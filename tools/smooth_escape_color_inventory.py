from __future__ import annotations

import argparse
from contextlib import contextmanager, nullcontext
import hashlib
import json
import math
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time
from typing import Any, Iterator


REPO_ROOT = Path(__file__).resolve().parents[1]
RUNTIME_DIR = Path(r"D:\salt-fractal\cuda_newton_fractal_clone\runtime")
ACTIVE_RUNTIME_FILE = RUNTIME_DIR / "fractal_ui_active.txt"
RUNTIME_AUTOMATION_LOCK_FILE = RUNTIME_DIR / ".runtime_ui_automation.lock"
RUNTIME_AUTOMATION_LOCK_STALE_SECONDS = 300.0

DEFAULT_FRACTAL_TYPES = [
    "mandelbrot",
    "julia",
    "multibrot",
    "burning_ship",
    "phoenix",
    "nova",
    "collatz",
    "mcmullen",
    "lambda",
    "spider",
    "celtic_mandelbrot",
    "perpendicular_burning_ship",
    "magnet",
    "explaino_nova",
    "explaino_julia",
    "explaino_lambda",
    "explaino_rational_escape",
    "explaino_collatz_direct",
]


@contextmanager
def runtime_automation_lock(*, timeout_seconds: float = 120.0) -> Iterator[None]:
    RUNTIME_DIR.mkdir(parents=True, exist_ok=True)
    deadline = time.monotonic() + timeout_seconds
    fd: int | None = None
    while time.monotonic() < deadline:
        try:
            fd = os.open(str(RUNTIME_AUTOMATION_LOCK_FILE), os.O_CREAT | os.O_EXCL | os.O_RDWR)
            os.write(fd, f"pid={os.getpid()} acquired_at={time.time():.6f}\n".encode("ascii"))
            break
        except FileExistsError:
            try:
                lock_age = time.time() - RUNTIME_AUTOMATION_LOCK_FILE.stat().st_mtime
            except OSError:
                lock_age = 0.0
            if lock_age > RUNTIME_AUTOMATION_LOCK_STALE_SECONDS:
                try:
                    RUNTIME_AUTOMATION_LOCK_FILE.unlink()
                    continue
                except OSError:
                    pass
            time.sleep(0.1)
    if fd is None:
        raise RuntimeError(f"runtime automation lock was not acquired: {RUNTIME_AUTOMATION_LOCK_FILE}")
    try:
        yield
    finally:
        os.close(fd)
        try:
            RUNTIME_AUTOMATION_LOCK_FILE.unlink()
        except FileNotFoundError:
            pass


def active_runtime_exe() -> Path:
    if not ACTIVE_RUNTIME_FILE.exists():
        raise FileNotFoundError(f"missing active runtime metadata: {ACTIVE_RUNTIME_FILE}")
    active_name = ACTIVE_RUNTIME_FILE.read_text(encoding="utf-8").strip()
    if not active_name:
        raise RuntimeError(f"empty active runtime metadata: {ACTIVE_RUNTIME_FILE}")
    exe_path = RUNTIME_DIR / active_name
    if not exe_path.exists():
        raise FileNotFoundError(f"active runtime missing: {exe_path}")
    return exe_path


def _read_i32_le(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 4], "little", signed=True)


def _read_u16_le(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 2], "little", signed=False)


def _read_u32_le(data: bytes, offset: int) -> int:
    return int.from_bytes(data[offset : offset + 4], "little", signed=False)


def read_bmp_rgb(path: Path) -> dict[str, Any]:
    data = path.read_bytes()
    if len(data) < 54 or data[:2] != b"BM":
        raise ValueError(f"not a BMP file: {path}")
    pixel_offset = _read_u32_le(data, 10)
    dib_size = _read_u32_le(data, 14)
    if dib_size < 40:
        raise ValueError(f"unsupported BMP DIB header in {path}")
    width = _read_i32_le(data, 18)
    raw_height = _read_i32_le(data, 22)
    planes = _read_u16_le(data, 26)
    bits_per_pixel = _read_u16_le(data, 28)
    compression = _read_u32_le(data, 30)
    if width <= 0 or raw_height == 0 or planes != 1 or compression != 0:
        raise ValueError(f"unsupported BMP shape in {path}")
    if bits_per_pixel not in (24, 32):
        raise ValueError(f"unsupported BMP bit depth {bits_per_pixel} in {path}")

    height = abs(raw_height)
    bytes_per_pixel = bits_per_pixel // 8
    row_stride = ((width * bits_per_pixel + 31) // 32) * 4
    expected_size = pixel_offset + row_stride * height
    if len(data) < expected_size:
        raise ValueError(f"truncated BMP pixel data in {path}")

    pixels: list[tuple[int, int, int]] = []
    for y in range(height):
        source_y = y if raw_height < 0 else height - 1 - y
        row_offset = pixel_offset + source_y * row_stride
        for x in range(width):
            offset = row_offset + x * bytes_per_pixel
            blue = data[offset]
            green = data[offset + 1]
            red = data[offset + 2]
            pixels.append((red, green, blue))
    return {"width": width, "height": height, "pixels": pixels}


def _luma(pixel: tuple[int, int, int]) -> float:
    red, green, blue = pixel
    return 0.299 * red + 0.587 * green + 0.114 * blue


def compute_frame_metrics(frame: dict[str, Any], *, black_threshold: int = 3) -> dict[str, Any]:
    pixels = list(frame["pixels"])
    pixel_count = len(pixels)
    if pixel_count <= 0:
        raise ValueError("frame has no pixels")
    lumas = [_luma(pixel) for pixel in pixels]
    mean_luma = sum(lumas) / pixel_count
    variance = sum((value - mean_luma) * (value - mean_luma) for value in lumas) / pixel_count
    black_count = sum(
        1
        for red, green, blue in pixels
        if red <= black_threshold and green <= black_threshold and blue <= black_threshold
    )
    reds = [pixel[0] for pixel in pixels]
    greens = [pixel[1] for pixel in pixels]
    blues = [pixel[2] for pixel in pixels]
    return {
        "width": int(frame["width"]),
        "height": int(frame["height"]),
        "pixel_count": pixel_count,
        "unique_rgb_count": len(set(pixels)),
        "black_pixel_count": black_count,
        "black_pixel_fraction": black_count / pixel_count,
        "luma_min": min(lumas),
        "luma_max": max(lumas),
        "luma_mean": mean_luma,
        "luma_stddev": math.sqrt(variance),
        "red_min": min(reds),
        "red_max": max(reds),
        "green_min": min(greens),
        "green_max": max(greens),
        "blue_min": min(blues),
        "blue_max": max(blues),
    }


def _black_fraction_class(fraction: float) -> str:
    if fraction >= 0.50:
        return "high"
    if fraction >= 0.10:
        return "moderate"
    return "low"


def _read_json(path: Path) -> dict[str, Any]:
    payload = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(payload, dict):
        raise ValueError(f"expected JSON object: {path}")
    return payload


def build_case_summary(fractal_type: str, bundle_dir: Path) -> dict[str, Any]:
    state_path = bundle_dir / "state.json"
    frame_path = bundle_dir / "frame.bmp"
    if not state_path.exists():
        raise FileNotFoundError(f"missing state.json in capture bundle: {bundle_dir}")
    if not frame_path.exists():
        raise FileNotFoundError(f"missing frame.bmp in capture bundle: {bundle_dir}")

    state = _read_json(state_path)
    params = state.get("params", {})
    render = state.get("render", {})
    stats = state.get("stats", {})
    if not isinstance(params, dict) or not isinstance(render, dict) or not isinstance(stats, dict):
        raise ValueError(f"state.json has invalid params/render/stats shape: {state_path}")

    frame = read_bmp_rgb(frame_path)
    metrics = compute_frame_metrics(frame)
    frame_bytes = frame_path.read_bytes()
    return {
        "fractal_type": fractal_type,
        "reported_fractal_type": state.get("fractal_type", "unknown"),
        "bundle_dir": str(bundle_dir),
        "frame_sha256": hashlib.sha256(frame_bytes).hexdigest(),
        "color": {
            "mode": params.get("coloring_mode", "unknown"),
            "signal": params.get("color_signal", "unknown"),
            "palette": params.get("color_palette", "unknown"),
            "grading": params.get("color_grading", "unknown"),
            "smooth_escape_scale": params.get("color_smooth_escape_scale"),
            "smooth_escape_bias": params.get("color_smooth_escape_bias"),
            "heatmap_cycle_scale": params.get("color_heatmap_cycle_scale"),
            "heatmap_saturation": params.get("color_heatmap_saturation"),
        },
        "render": {
            "width": render.get("width"),
            "height": render.get("height"),
            "sample_tier": render.get("sample_tier", "unknown"),
        },
        "stats": {
            "last_render_ms": stats.get("last_render_ms"),
            "last_iters_avg": stats.get("last_iters_avg"),
            "last_iters_sum": stats.get("last_iters_sum"),
            "last_pixel_count": stats.get("last_pixel_count"),
            "resolved_backend": stats.get("resolved_backend", "unknown"),
            "resolved_strategy": stats.get("resolved_strategy", "unknown"),
        },
        "frame_metrics": metrics,
        "classification": {
            "observed_black_pixel_fraction": _black_fraction_class(metrics["black_pixel_fraction"]),
            "interior_treatment_note": "black pixels are measured from the final frame; this report does not infer exact interior membership",
            "tuning_authority": "current runtime uses saved/global color params; per-family smooth-escape tuning metadata is not applied by this measurement tool",
        },
    }


def _safe_case_dir_name(fractal_type: str) -> str:
    return "".join(ch if ch.isalnum() or ch in ("_", "-") else "_" for ch in fractal_type)


def _prepare_empty_dir(path: Path, *, allowed_root: Path) -> None:
    resolved_path = path.resolve()
    resolved_root = allowed_root.resolve()
    if resolved_path == resolved_root:
        raise ValueError(f"refusing to delete output root directly: {path}")
    if resolved_root not in resolved_path.parents:
        raise ValueError(f"refusing to delete path outside output root: {path}")
    if path.exists():
        shutil.rmtree(path)
    path.mkdir(parents=True, exist_ok=True)


def resolve_inventory_out_dir(out_dir: str | Path) -> Path:
    return Path(out_dir).resolve()


def run_capture_case(exe_path: Path, fractal_type: str, bundle_dir: Path, *, width: int, height: int) -> None:
    command = [
        str(exe_path),
        "--capture-diagnostic",
        "--out-dir",
        str(bundle_dir),
        "--fractal-type",
        fractal_type,
        "--width",
        str(width),
        "--height",
        str(height),
    ]
    result = subprocess.run(
        command,
        cwd=str(RUNTIME_DIR),
        text=True,
        capture_output=True,
        check=False,
    )
    if result.returncode != 0:
        raise RuntimeError(
            "capture failed for "
            + fractal_type
            + "\nstdout:\n"
            + result.stdout
            + "\nstderr:\n"
            + result.stderr
        )


def write_markdown_report(report: dict[str, Any], path: Path) -> None:
    lines = [
        "# Smooth-Escape Color Inventory",
        "",
        f"- runtime: `{report['runtime_exe']}`",
        f"- resolution: `{report['width']}x{report['height']}`",
        f"- cases: `{len(report['cases'])}`",
        "",
        "| Fractal | Color | Backend | ms | Avg iters | Black frac | Unique RGB | Luma range |",
        "| --- | --- | --- | ---: | ---: | ---: | ---: | --- |",
    ]
    for case in report["cases"]:
        color = case["color"]
        stats = case["stats"]
        metrics = case["frame_metrics"]
        lines.append(
            "| {fractal} | {signal}/{palette}/{grading} | {backend} | {ms} | {iters} | {black:.3f} | {unique} | {lmin:.1f}-{lmax:.1f} |".format(
                fractal=case["fractal_type"],
                signal=color["signal"],
                palette=color["palette"],
                grading=color["grading"],
                backend=stats["resolved_backend"],
                ms=stats["last_render_ms"],
                iters=stats["last_iters_avg"],
                black=metrics["black_pixel_fraction"],
                unique=metrics["unique_rgb_count"],
                lmin=metrics["luma_min"],
                lmax=metrics["luma_max"],
            )
        )
    lines.append("")
    lines.append("## Aggregate Flags")
    lines.append("")
    analysis = report["analysis"]
    lines.append(
        "- shared color tuple: `{}`".format(
            "yes" if analysis["all_cases_share_color_tuple"] else "no"
        )
    )
    lines.append(
        "- high black-fraction cases: `{}`".format(
            ", ".join(analysis["high_black_fraction_cases"]) or "none"
        )
    )
    lines.append(
        "- low luma-span cases: `{}`".format(
            ", ".join(analysis["low_luma_span_cases"]) or "none"
        )
    )
    lines.append(
        "- low unique-color cases: `{}`".format(
            ", ".join(analysis["low_unique_color_cases"]) or "none"
        )
    )
    lines.append("")
    lines.append("Notes:")
    lines.append("- This is a measurement artifact only; it does not change renderer or Color Pipeline behavior.")
    lines.append("- Black-pixel fraction is measured from the final rendered frame and is not exact interior membership.")
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def build_inventory_analysis(cases: list[dict[str, Any]]) -> dict[str, Any]:
    color_tuples = {
        (
            case["color"]["signal"],
            case["color"]["palette"],
            case["color"]["grading"],
            case["color"]["smooth_escape_scale"],
            case["color"]["smooth_escape_bias"],
            case["color"]["heatmap_cycle_scale"],
        )
        for case in cases
    }
    high_black_cases = [
        case["fractal_type"]
        for case in cases
        if case["frame_metrics"]["black_pixel_fraction"] >= 0.50
    ]
    moderate_black_cases = [
        case["fractal_type"]
        for case in cases
        if 0.10 <= case["frame_metrics"]["black_pixel_fraction"] < 0.50
    ]
    low_luma_span_cases = [
        case["fractal_type"]
        for case in cases
        if case["frame_metrics"]["luma_max"] - case["frame_metrics"]["luma_min"] < 80.0
    ]
    low_unique_color_cases = [
        case["fractal_type"]
        for case in cases
        if case["frame_metrics"]["unique_rgb_count"] < 200
    ]
    return {
        "all_cases_share_color_tuple": len(color_tuples) == 1,
        "observed_color_tuple_count": len(color_tuples),
        "high_black_fraction_cases": high_black_cases,
        "moderate_black_fraction_cases": moderate_black_cases,
        "low_luma_span_cases": low_luma_span_cases,
        "low_unique_color_cases": low_unique_color_cases,
        "later_tuning_candidate_cases": sorted(
            set(high_black_cases + moderate_black_cases + low_luma_span_cases + low_unique_color_cases)
        ),
        "thresholds": {
            "high_black_fraction_min": 0.50,
            "moderate_black_fraction_min": 0.10,
            "low_luma_span_max": 80.0,
            "low_unique_color_max": 199,
        },
    }


def run_inventory(args: argparse.Namespace) -> dict[str, Any]:
    out_dir = resolve_inventory_out_dir(args.out_dir)
    captures_dir = out_dir / "captures"
    captures_dir.mkdir(parents=True, exist_ok=True)
    exe_path = Path(args.runtime_exe) if args.runtime_exe else active_runtime_exe()
    cases: list[dict[str, Any]] = []
    for fractal_type in args.fractal_type:
        case_dir = captures_dir / _safe_case_dir_name(fractal_type)
        _prepare_empty_dir(case_dir, allowed_root=out_dir)
        run_capture_case(exe_path, fractal_type, case_dir, width=args.width, height=args.height)
        cases.append(build_case_summary(fractal_type, case_dir))

    report = {
        "version": 1,
        "out_dir": str(out_dir),
        "runtime_exe": str(exe_path),
        "width": args.width,
        "height": args.height,
        "fractal_types": list(args.fractal_type),
        "cases": cases,
        "analysis": build_inventory_analysis(cases),
    }
    (out_dir / "inventory.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    write_markdown_report(report, out_dir / "inventory.md")
    return report


def build_arg_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Measure current smooth-escape color output from published runtime captures.")
    parser.add_argument("--out-dir", required=True, help="Directory for inventory.json, inventory.md, and capture bundles")
    parser.add_argument("--runtime-exe", help="Runtime executable; defaults to fractal_ui_active.txt")
    parser.add_argument("--width", type=int, default=96)
    parser.add_argument("--height", type=int, default=72)
    parser.add_argument("--fractal-type", action="append", default=[], help="Fractal id to measure; repeat to override defaults")
    parser.add_argument("--runtime-lock", action="store_true", help="Use the shared no-mouse runtime automation lock")
    return parser


def main(argv: list[str] | None = None) -> int:
    parser = build_arg_parser()
    args = parser.parse_args(argv)
    if args.width <= 0 or args.height <= 0:
        parser.error("--width and --height must be positive")
    if not args.fractal_type:
        args.fractal_type = list(DEFAULT_FRACTAL_TYPES)
    context = runtime_automation_lock() if args.runtime_lock else nullcontext()
    with context:
        report = run_inventory(args)
    print(json.dumps({"ok": True, "out_dir": report["out_dir"], "cases": len(report["cases"])}, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
