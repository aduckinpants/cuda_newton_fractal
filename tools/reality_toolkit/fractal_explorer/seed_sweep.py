from __future__ import annotations

import csv
import json
import math
import re
import shutil
import struct
import subprocess
from dataclasses import asdict, dataclass
from decimal import Decimal
from pathlib import Path
from typing import Iterable, Sequence

from .finding_capture import archive_finding_bundle
from .paths import findings_root


def format_seed_label(seed: float) -> str:
    return f"{seed:.6f}"


def build_seed_values(
    *,
    seed_start: float | None = None,
    seed_stop: float | None = None,
    seed_step: float | None = None,
    explicit_seeds: Sequence[float] | None = None,
) -> list[float]:
    if explicit_seeds is not None:
        values = [float(seed) for seed in explicit_seeds]
        if not values:
            raise ValueError("explicit_seeds must not be empty")
        if not all(math.isfinite(seed) for seed in values):
            raise ValueError("explicit seeds must be finite")
        return values

    if seed_start is None or seed_stop is None or seed_step is None:
        raise ValueError("seed_start, seed_stop, and seed_step are required when explicit_seeds is not provided")
    if not math.isfinite(seed_start) or not math.isfinite(seed_stop) or not math.isfinite(seed_step):
        raise ValueError("seed_start, seed_stop, and seed_step must be finite")
    if seed_step == 0:
        raise ValueError("seed_step must be non-zero")

    start = Decimal(str(seed_start))
    stop = Decimal(str(seed_stop))
    step = Decimal(str(seed_step))
    if (stop > start and step < 0) or (stop < start and step > 0):
        raise ValueError("seed_step direction does not reach stop")

    values: list[float] = []
    current = start
    if step > 0:
        while current <= stop:
            values.append(float(current))
            current += step
    else:
        while current >= stop:
            values.append(float(current))
            current += step
    if not values:
        raise ValueError("seed sweep produced no values")
    return values


@dataclass(slots=True)
class SweepConfig:
    repo_root: Path
    exe_path: Path
    diagnostics_last_dir: Path
    out_dir: Path
    seeds: list[float]
    fractal_type: str = "explaino"
    validate_ui: bool = True
    timeout_seconds: float = 180.0
    width: int | None = None
    height: int | None = None
    explaino_phase: float | None = None
    explaino_warp_strength: float | None = None
    archive_findings: bool = False
    finding_group: str | None = None
    finding_out_root: Path | None = None


def _run_command(command: Sequence[str], *, cwd: Path, timeout_seconds: float) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        list(command),
        cwd=str(cwd),
        text=True,
        capture_output=True,
        timeout=timeout_seconds,
        check=False,
    )


def _runtime_command(exe_path: Path, *args: str) -> list[str]:
    command = [str(exe_path), *args]
    if exe_path.suffix.lower() in {".cmd", ".bat"}:
        return ["cmd.exe", "/d", "/c", *command]
    return command


def _run_validate(config: SweepConfig) -> None:
    result = _run_command(_runtime_command(config.exe_path, "--validate-ui"), cwd=config.repo_root, timeout_seconds=config.timeout_seconds)
    if result.returncode != 0:
        raise RuntimeError(f"validate-ui failed with exit code {result.returncode}: {result.stderr or result.stdout}")


def _copy_diagnostics_bundle(source_dir: Path, destination_dir: Path) -> None:
    if destination_dir.exists():
        shutil.rmtree(destination_dir)
    shutil.copytree(source_dir, destination_dir)


def _load_bmp24_pixels(path: Path) -> tuple[int, int, bytes]:
    data = path.read_bytes()
    if len(data) < 54 or data[:2] != b"BM":
        raise ValueError(f"Unsupported BMP header in {path}")

    pixel_offset = struct.unpack_from("<I", data, 10)[0]
    dib_size = struct.unpack_from("<I", data, 14)[0]
    if dib_size < 40:
        raise ValueError(f"Unsupported DIB header in {path}")

    width = struct.unpack_from("<i", data, 18)[0]
    height = struct.unpack_from("<i", data, 22)[0]
    bits_per_pixel = struct.unpack_from("<H", data, 28)[0]
    compression = struct.unpack_from("<I", data, 30)[0]
    if bits_per_pixel != 24 or compression != 0:
        raise ValueError(f"Only 24-bit BI_RGB BMPs are supported, got {bits_per_pixel}-bit compression={compression}")

    abs_height = abs(height)
    row_stride = width * 3
    row_padded = (row_stride + 3) & ~3
    pixel_bytes = bytearray(width * abs_height * 3)

    top_down = height < 0
    for row_index in range(abs_height):
        source_row = row_index if top_down else (abs_height - 1 - row_index)
        source_offset = pixel_offset + source_row * row_padded
        target_offset = row_index * row_stride
        pixel_bytes[target_offset:target_offset + row_stride] = data[source_offset:source_offset + row_stride]

    return width, abs_height, bytes(pixel_bytes)


def _compute_frame_metrics(frame_path: Path, previous_pixels: bytes | None) -> tuple[dict[str, float | None], bytes]:
    width, height, pixels = _load_bmp24_pixels(frame_path)
    count = width * height
    if count == 0:
        return {"mean_luma": None, "std_luma": None, "edge_energy": None, "delta_prev_mean_abs": None}, pixels

    luma_values = [0.0] * count
    mean_sum = 0.0
    for index in range(count):
        base = index * 3
        blue = pixels[base + 0]
        green = pixels[base + 1]
        red = pixels[base + 2]
        luma = 0.0722 * blue + 0.7152 * green + 0.2126 * red
        luma_values[index] = luma
        mean_sum += luma

    mean_luma = mean_sum / count
    variance = 0.0
    edge_sum = 0.0
    for y in range(height):
        row_offset = y * width
        for x in range(width):
            index = row_offset + x
            value = luma_values[index]
            variance += (value - mean_luma) * (value - mean_luma)
            if x + 1 < width:
                edge_sum += abs(value - luma_values[index + 1])
            if y + 1 < height:
                edge_sum += abs(value - luma_values[index + width])

    std_luma = math.sqrt(variance / count)
    edge_denominator = max(1, (width - 1) * height + (height - 1) * width)
    edge_energy = edge_sum / edge_denominator

    delta_prev_mean_abs: float | None = None
    if previous_pixels is not None and len(previous_pixels) == len(pixels):
        delta_sum = 0.0
        for index in range(len(pixels)):
            delta_sum += abs(pixels[index] - previous_pixels[index])
        delta_prev_mean_abs = delta_sum / len(pixels)

    return {
        "mean_luma": mean_luma,
        "std_luma": std_luma,
        "edge_energy": edge_energy,
        "delta_prev_mean_abs": delta_prev_mean_abs,
    }, pixels


def _state_fractal_type(state_json_path: Path) -> str | None:
    if not state_json_path.exists():
        return None
    try:
        data = json.loads(state_json_path.read_text(encoding="utf-8"))
    except Exception:
        return None
    value = data.get("fractal_type")
    return value if isinstance(value, str) else None


def _run_one_seed(config: SweepConfig, seed: float, previous_pixels: bytes | None) -> tuple[dict[str, object], bytes | None]:
    seed_label = format_seed_label(seed)
    run_dir = config.out_dir / f"seed_{seed_label}"
    run_dir.mkdir(parents=True, exist_ok=True)

    command = _runtime_command(
        config.exe_path,
        "--capture-diagnostic",
        "--fractal-type",
        config.fractal_type,
        "--explaino-seed",
        seed_label,
    )
    if config.width is not None:
        command.extend(["--width", str(config.width)])
    if config.height is not None:
        command.extend(["--height", str(config.height)])
    if config.explaino_phase is not None:
        command.extend(["--explaino-phase", f"{config.explaino_phase:.12g}"])
    if config.explaino_warp_strength is not None:
        command.extend(["--explaino-warp-strength", f"{config.explaino_warp_strength:.12g}"])

    result = _run_command(command, cwd=config.repo_root, timeout_seconds=config.timeout_seconds)

    diagnostics_dest = run_dir / "diagnostics_last"
    frame_path = diagnostics_dest / "frame.bmp"
    state_path = diagnostics_dest / "state.json"
    error_text = (result.stderr or result.stdout or "").strip() or None

    metrics = {
        "mean_luma": None,
        "std_luma": None,
        "edge_energy": None,
        "delta_prev_mean_abs": None,
    }
    pixels: bytes | None = None

    ok = result.returncode == 0 and config.diagnostics_last_dir.exists()
    if ok:
        _copy_diagnostics_bundle(config.diagnostics_last_dir, diagnostics_dest)
        metrics, pixels = _compute_frame_metrics(frame_path, previous_pixels)

    state_fractal_type = _state_fractal_type(state_path)
    entry: dict[str, object] = {
        "seed": seed,
        "seed_label": seed_label,
        "mode": "capture_diagnostic",
        "ok": ok,
        "returncode": result.returncode,
        "fractal_type": config.fractal_type,
        "state_fractal_type": state_fractal_type,
        "frame_path": str(frame_path) if frame_path.exists() else None,
        "state_path": str(state_path) if state_path.exists() else None,
        "stdout": result.stdout,
        "stderr": result.stderr,
        "error": error_text,
        **metrics,
    }
    return entry, pixels


def _write_csv(path: Path, rows: Iterable[dict[str, object]]) -> None:
    rows = list(rows)
    fieldnames = [
        "seed",
        "seed_label",
        "mode",
        "ok",
        "returncode",
        "fractal_type",
        "state_fractal_type",
        "mean_luma",
        "std_luma",
        "edge_energy",
        "delta_prev_mean_abs",
        "frame_path",
        "state_path",
        "error",
        "finding_dir",
        "finding_error",
    ]
    with path.open("w", encoding="utf-8", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=fieldnames)
        writer.writeheader()
        for row in rows:
            writer.writerow({name: row.get(name) for name in fieldnames})


def _sanitize_folder_label(text: str) -> str:
    cleaned = re.sub(r"[^A-Za-z0-9._-]+", "_", text.strip())
    cleaned = cleaned.strip("._-")
    return cleaned or "finding_batch"


def _finding_batch_dir(config: SweepConfig) -> Path:
    out_root = config.finding_out_root or findings_root(config.repo_root)
    if config.finding_group:
        return out_root / _sanitize_folder_label(config.finding_group) / config.out_dir.name
    return out_root / config.out_dir.name


def _repro_command(config: SweepConfig, seed: float) -> str:
    parts = [
        str(config.exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        config.fractal_type,
        "--explaino-seed",
        format_seed_label(seed),
    ]
    if config.width is not None:
        parts.extend(["--width", str(config.width)])
    if config.height is not None:
        parts.extend(["--height", str(config.height)])
    if config.explaino_phase is not None:
        parts.extend(["--explaino-phase", f"{config.explaino_phase:.12g}"])
    if config.explaino_warp_strength is not None:
        parts.extend(["--explaino-warp-strength", f"{config.explaino_warp_strength:.12g}"])
    return " ".join(parts)


def run_seed_sweep(config: SweepConfig) -> dict[str, object]:
    config.out_dir.mkdir(parents=True, exist_ok=True)
    finding_batch_dir = _finding_batch_dir(config) if config.archive_findings else None

    if config.validate_ui:
        _run_validate(config)

    runs: list[dict[str, object]] = []
    previous_pixels: bytes | None = None
    for seed in config.seeds:
        entry, previous_pixels = _run_one_seed(config, seed, previous_pixels)
        if config.archive_findings and entry.get("ok"):
            try:
                diagnostics_dir = Path(str(entry["frame_path"])).parent
                finding_id = f"{len(runs) + 1:03d}__seed_{entry['seed_label']}"
                output_dir = finding_batch_dir / finding_id
                archive_finding_bundle(
                    diagnostics_dir=diagnostics_dir,
                    output_dir=output_dir,
                    finding_id=finding_id,
                    why=f"Automated sweep finding for {config.fractal_type} seed {entry['seed_label']}.",
                    repro_command=_repro_command(config, seed),
                )
                entry["finding_dir"] = str(output_dir)
                entry["finding_error"] = None
            except Exception as exc:
                entry["finding_dir"] = None
                entry["finding_error"] = str(exc)
        runs.append(entry)

    summary = {
        "config": {
            **asdict(config),
            "repo_root": str(config.repo_root),
            "exe_path": str(config.exe_path),
            "diagnostics_last_dir": str(config.diagnostics_last_dir),
            "out_dir": str(config.out_dir),
        },
        "runs_total": len(runs),
        "runs_ok": sum(1 for row in runs if row["ok"]),
        "best_edge_seed": None,
        "best_delta_seed": None,
        "finding_batch_dir": str(finding_batch_dir) if finding_batch_dir is not None else None,
        "runs": runs,
    }

    edge_candidates = [row for row in runs if isinstance(row.get("edge_energy"), (int, float))]
    if edge_candidates:
        summary["best_edge_seed"] = max(edge_candidates, key=lambda row: float(row["edge_energy"]))["seed"]

    delta_candidates = [row for row in runs if isinstance(row.get("delta_prev_mean_abs"), (int, float))]
    if delta_candidates:
        summary["best_delta_seed"] = max(delta_candidates, key=lambda row: float(row["delta_prev_mean_abs"]))["seed"]

    summary_path = config.out_dir / "summary.json"
    summary_path.write_text(json.dumps(summary, indent=2), encoding="utf-8")
    _write_csv(config.out_dir / "summary.csv", runs)
    return summary