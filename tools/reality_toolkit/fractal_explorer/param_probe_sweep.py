from __future__ import annotations

import csv
import json
import math
import shutil
import subprocess
import sys
from datetime import datetime
from pathlib import Path
from typing import Callable, Sequence

from .paths import publish_root, runtime_root
from .probe_client import run_sample_request


def mod_linear(lo: float, hi: float) -> Callable[[float], float]:
    return lambda t: lo + (hi - lo) * t


def mod_sine(lo: float, hi: float) -> Callable[[float], float]:
    mid = (lo + hi) / 2.0
    amp = (hi - lo) / 2.0
    return lambda t: mid + amp * math.sin(2.0 * math.pi * t)


def mod_log_sweep(lo: float, hi: float) -> Callable[[float], float]:
    if lo <= 0:
        lo = 1e-6
    log_lo = math.log(lo)
    log_hi = math.log(hi)
    return lambda t: math.exp(log_lo + (log_hi - log_lo) * t)


ProbeSpec = tuple[str, str, Callable[[float], float], str]

PROBES: dict[str, ProbeSpec] = {
    "phase_strength_sine": (
        "phase_strength_sine",
        "view.explaino_phase_strength",
        mod_sine(0.0, 20.0),
        "Sinusoidal sweep of phase strength 0-20",
    ),
    "phase_strength_linear": (
        "phase_strength_linear",
        "view.explaino_phase_strength",
        mod_linear(0.0, 20.0),
        "Linear ramp of phase strength 0-20",
    ),
    "warp_strength_sine": (
        "warp_strength_sine",
        "params.explaino_warp_strength",
        mod_sine(0.0, 5.0),
        "Sinusoidal sweep of warp strength 0-5",
    ),
    "warp_strength_linear": (
        "warp_strength_linear",
        "params.explaino_warp_strength",
        mod_linear(0.0, 5.0),
        "Linear ramp of warp strength 0-5",
    ),
    "damping_log": (
        "damping_log",
        "params.explaino_damping",
        mod_log_sweep(0.01, 10.0),
        "Logarithmic sweep of damping 0.01-10",
    ),
    "damping_sine": (
        "damping_sine",
        "params.explaino_damping",
        mod_sine(0.1, 5.0),
        "Sinusoidal sweep of damping 0.1-5",
    ),
    "root_spread_linear": (
        "root_spread_linear",
        "params.explaino_root_spread",
        mod_linear(0.0, 10.0),
        "Linear ramp of root spread 0-10",
    ),
    "cluster_radius_linear": (
        "cluster_radius_linear",
        "params.explaino_cluster_radius",
        mod_linear(0.0, 5.0),
        "Linear ramp of cluster radius 0-5 (explaino_mult)",
    ),
    "phase_linear": (
        "phase_linear",
        "view.explaino_phase",
        mod_linear(-math.pi, math.pi),
        "Linear sweep of explaino phase -pi to pi",
    ),
    "seed_linear": (
        "seed_linear",
        "params.explaino_seed",
        mod_linear(0.0, 10.0),
        "Linear sweep of explaino seed 0-10",
    ),
    "nova_alpha_linear": (
        "nova_alpha_linear",
        "params.nova_alpha",
        mod_linear(0.01, 5.0),
        "Linear sweep of nova alpha 0.01-5",
    ),
}

DEFAULT_SAMPLE_METRICS = [
    "summary_mean_iterations",
    "summary_escape_fraction",
    "summary_converged_fraction",
    "summary_nonfinite_fraction",
    "summary_pole_fraction",
    "summary_best_sequence_index",
]

DEFAULT_SAMPLE_POINT_METRICS = [
    "iterations",
    "status",
    "final_abs2",
]


def _default_base_state() -> dict[str, object]:
    return {
        "state_version": 3,
        "fractal_type": "explaino",
        "view": {
            "center_x": 0.0,
            "center_y": 0.0,
            "zoom": 1.0,
            "rotation_degrees": 0.0,
            "center_hp_x": 0.0,
            "center_hp_y": 0.0,
            "log2_zoom": 0.0,
            "explaino_phase": 0.0,
            "explaino_seed_drift": 0.0,
            "explaino_seed_tween": True,
            "auto_increment_seed": False,
            "explaino_seed_rate": 0.05,
            "explaino_phase_strength": 1.0,
        },
        "params": {
            "max_iter": 500,
            "epsilon": 1e-6,
            "exposure": 1.0,
            "poly_kind": 2,
            "coloring_mode": "joy_basins",
            "nova_alpha": 0.5,
            "phoenix_p_real": -0.5,
            "phoenix_p_imag": 0.0,
            "multibrot_power": 3,
            "explaino_seed": 0.0,
            "explaino_seed_b": 1.0,
            "explaino_mix": 0.5,
            "explaino_warp_strength": 0.0,
            "explaino_root_count": 0,
            "explaino_cluster_radius": 0.1,
            "explaino_damping": 1.0,
            "explaino_root_spread": 0.5,
            "transcendental_func": "f_sin",
            "poly_coeffs": [-1.0, 0.0, 0.0, 1.0, 0.0],
            "color_saturation": 1.15,
            "color_contrast": 1.1,
            "color_tint_r": 1.0,
            "color_tint_g": 1.0,
            "color_tint_b": 1.0,
        },
        "render": {
            "width": 1024,
            "height": 768,
            "block_size": 256,
            "device_id": 0,
        },
    }


def _normalize_probe_binding_path(path: str) -> str:
    return path if path.startswith("fractal.") else f"fractal.{path}"


def _set_nested(state: dict[str, object], dotpath: str, value: float) -> None:
    parts = dotpath.split(".")
    obj: dict[str, object] = state
    for part in parts[:-1]:
        obj = obj[part]  # type: ignore[index]
    obj[parts[-1]] = value


def generate_probe_states(
    probe_name: str,
    ticks: int,
    base_state: dict[str, object] | None = None,
) -> list[tuple[int, float, float, dict[str, object]]]:
    if probe_name not in PROBES:
        raise ValueError(f"Unknown probe: {probe_name}. Use --list-probes.")

    _label, param_path, mod_fn, _desc = PROBES[probe_name]
    base = base_state or _default_base_state()

    results: list[tuple[int, float, float, dict[str, object]]] = []
    for i in range(ticks):
        t = i / max(1, ticks - 1) if ticks > 1 else 0.0
        value = mod_fn(t)
        state = json.loads(json.dumps(base))
        _set_nested(state, param_path, value)
        results.append((i, t, value, state))
    return results


def generate_probe_ticks(probe_name: str, ticks: int) -> list[tuple[int, float, float]]:
    return [(index, t, value) for index, t, value, _state in generate_probe_states(probe_name, ticks)]


def write_capture_probe_manifest(
    out_dir: Path,
    ticks: list[tuple[int, float, float]],
) -> Path:
    manifest_path = out_dir / "probe_manifest.csv"
    with manifest_path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle)
        writer.writerow(["tick", "t", "param_value", "state_json"])
        for idx, t, val in ticks:
            writer.writerow([idx, f"{t:.6f}", f"{val:.8g}", f"tick_{idx:04d}/state.json"])
    return manifest_path


def build_sequence_grid_request(
    *,
    probe_name: str,
    ticks: int,
    fractal_type: str | None,
    center_x: float,
    center_y: float,
    span_x: float,
    span_y: float,
    grid_width: int,
    grid_height: int,
    base_state_path: Path | None = None,
    metrics: Sequence[str] | None = None,
) -> tuple[dict[str, object], list[tuple[int, float, float]]]:
    if probe_name not in PROBES:
        raise ValueError(f"Unknown probe: {probe_name}. Use --list-probes.")
    if ticks <= 0:
        raise ValueError("ticks must be > 0")
    if grid_width <= 0 or grid_height <= 0:
        raise ValueError("grid_width and grid_height must be > 0")

    _label, param_path, mod_fn, description = PROBES[probe_name]
    tick_values: list[tuple[int, float, float]] = []
    values: list[float] = []
    for index in range(ticks):
        t = index / max(1, ticks - 1) if ticks > 1 else 0.0
        value = mod_fn(t)
        tick_values.append((index, t, value))
        values.append(value)

    effective_fractal_type = fractal_type
    if effective_fractal_type is None and base_state_path is None:
        effective_fractal_type = "explaino"

    request: dict[str, object] = {
        "request_version": 1,
        "request_id": f"probe-{probe_name}",
        "function_id": "fractal.sample",
        "mode": "sequence_grid",
        "overrides": [],
        "region": {
            "center_x": center_x,
            "center_y": center_y,
            "span_x": span_x,
            "span_y": span_y,
            "grid_width": grid_width,
            "grid_height": grid_height,
        },
        "sequence": {
            "zip_paths": True,
            "vary": [
                {
                    "path": _normalize_probe_binding_path(param_path),
                    "values": values,
                }
            ],
        },
        "metrics": list(metrics or DEFAULT_SAMPLE_METRICS),
        "operator_context": {
            "source": "reality_toolkit",
            "operator": "run_fractal_param_probe_sweep",
            "why": description,
        },
    }
    if effective_fractal_type is not None:
        request["overrides"].append({"path": "fractal.view.fractal_type", "value": effective_fractal_type})  # type: ignore[union-attr]
    if base_state_path is not None:
        request["base_state"] = {"load_state_json": str(base_state_path)}

    return request, tick_values


def _write_sample_probe_manifest(
    out_dir: Path,
    tick_values: list[tuple[int, float, float]],
    sequence_results: Sequence[dict[str, object]],
) -> Path:
    summaries = {
        int(result.get("sequence_index", -1)): result.get("summary", {})
        for result in sequence_results
        if isinstance(result, dict)
    }

    manifest_path = out_dir / "probe_manifest.csv"
    with manifest_path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle)
        writer.writerow([
            "tick",
            "sequence_index",
            "t",
            "param_value",
            "mean_iterations",
            "escape_fraction",
            "converged_fraction",
            "nonfinite_fraction",
            "pole_fraction",
        ])
        for tick, t, value in tick_values:
            summary = summaries.get(tick, {})
            writer.writerow([
                tick,
                tick,
                f"{t:.6f}",
                f"{value:.8g}",
                summary.get("mean_iterations", ""),
                summary.get("escape_fraction", ""),
                summary.get("converged_fraction", ""),
                summary.get("nonfinite_fraction", ""),
                summary.get("pole_fraction", ""),
            ])
    return manifest_path


def run_sample_probe_sweep(
    *,
    repo_root: Path,
    probe_name: str,
    ticks: int,
    out_dir: Path,
    fractal_type: str | None,
    center_x: float,
    center_y: float,
    span_x: float,
    span_y: float,
    grid_width: int,
    grid_height: int,
    base_state_path: Path | None = None,
    metrics: Sequence[str] | None = None,
    timeout_seconds: float = 180.0,
    exe_path: Path | None = None,
    dry_run: bool = False,
    sample_runner: Callable[..., dict[str, object]] = run_sample_request,
) -> dict[str, object]:
    out_dir.mkdir(parents=True, exist_ok=True)

    request, tick_values = build_sequence_grid_request(
        probe_name=probe_name,
        ticks=ticks,
        fractal_type=fractal_type,
        center_x=center_x,
        center_y=center_y,
        span_x=span_x,
        span_y=span_y,
        grid_width=grid_width,
        grid_height=grid_height,
        base_state_path=base_state_path,
        metrics=metrics,
    )

    request_path = out_dir / "probe_request.json"
    request_path.write_text(json.dumps(request, indent=2), encoding="utf-8")

    response: dict[str, object] | None = None
    response_path = out_dir / "probe_response.json"
    if not dry_run:
        response = sample_runner(repo_root, request, timeout_seconds=timeout_seconds, exe_path=exe_path)
        response_path.write_text(json.dumps(response, indent=2), encoding="utf-8")

    sequence_results = response.get("sequence_results", []) if response is not None else []
    manifest_path = _write_sample_probe_manifest(out_dir, tick_values, sequence_results if isinstance(sequence_results, Sequence) else [])

    best_sequence_index: int | None = None
    best_tick: int | None = None
    best_param_value: float | None = None
    if response is not None:
        summary = response.get("summary", {})
        if isinstance(summary, dict) and isinstance(summary.get("best_sequence_index"), int):
            best_sequence_index = int(summary["best_sequence_index"])
            if 0 <= best_sequence_index < len(tick_values):
                best_tick = tick_values[best_sequence_index][0]
                best_param_value = tick_values[best_sequence_index][2]

    summary_json: dict[str, object] = {
        "probe_name": probe_name,
        "ticks": ticks,
        "timestamp": datetime.now().isoformat(),
        "out_dir": str(out_dir),
        "manifest": str(manifest_path),
        "request_path": str(request_path),
        "response_path": str(response_path) if response is not None else None,
        "description": PROBES[probe_name][3],
        "param_path": _normalize_probe_binding_path(PROBES[probe_name][1]),
        "mode": "sample_sequence_grid",
        "dry_run": dry_run,
        "best_sequence_index": best_sequence_index,
        "best_tick": best_tick,
        "best_param_value": best_param_value,
        "runtime": response.get("runtime") if response is not None else None,
        "response_summary": response.get("summary") if response is not None else None,
    }
    summary_path = out_dir / "probe_summary.json"
    summary_path.write_text(json.dumps(summary_json, indent=2), encoding="utf-8")
    return summary_json


def run_capture_probe_sweep(
    *,
    repo_root: Path,
    probe_name: str,
    ticks: int,
    out_dir: Path,
    base_state_path: Path | None = None,
    dry_run: bool = False,
) -> Path:
    base_state = None
    if base_state_path:
        base_state = json.loads(base_state_path.read_text(encoding="utf-8"))

    states = generate_probe_states(probe_name, ticks, base_state)
    out_dir.mkdir(parents=True, exist_ok=True)

    manifest_ticks = []
    for idx, t, val, state in states:
        tick_dir = out_dir / f"tick_{idx:04d}"
        tick_dir.mkdir(parents=True, exist_ok=True)
        state_path = tick_dir / "state.json"
        state_path.write_text(json.dumps(state, indent=2), encoding="utf-8")
        manifest_ticks.append((idx, t, val))

    manifest = write_capture_probe_manifest(out_dir, manifest_ticks)

    summary = {
        "probe_name": probe_name,
        "ticks": ticks,
        "timestamp": datetime.now().isoformat(),
        "out_dir": str(out_dir),
        "manifest": str(manifest),
        "description": PROBES[probe_name][3],
        "param_path": PROBES[probe_name][1],
        "mode": "capture_diagnostic",
        "dry_run": dry_run,
    }
    summary_path = out_dir / "probe_summary.json"
    summary_path.write_text(json.dumps(summary, indent=2), encoding="utf-8")

    if not dry_run:
        launcher = runtime_root(repo_root) / "fractal_ui.cmd"
        if not launcher.exists():
            launcher = runtime_root(repo_root) / "fractal_ui.exe"
        if launcher.exists():
            for idx, t, val, _state in states:
                tick_dir = out_dir / f"tick_{idx:04d}"
                state_path = tick_dir / "state.json"
                cmd = [str(launcher), "--load-state-json", str(state_path), "--capture-diagnostic"]
                print(f"  tick {idx:4d}/{ticks}  t={t:.4f}  val={val:.6g} ...", end="", flush=True)
                result = subprocess.run(cmd, capture_output=True, text=True, timeout=60)
                if result.returncode == 0:
                    diag_frame = runtime_root(repo_root) / "diagnostics" / "last" / "frame.bmp"
                    if diag_frame.exists():
                        shutil.copy2(diag_frame, tick_dir / "frame.bmp")
                    print(" OK")
                else:
                    print(f" FAIL (exit {result.returncode})")
        else:
            print(f"WARNING: Viewer launcher not found at {launcher}. State files generated but no captures taken.", file=sys.stderr)

    print(f"\nProbe sweep complete: {out_dir}")
    print(f"  Manifest: {manifest}")
    print(f"  Summary: {summary_path}")
    return out_dir


def default_probe_out_dir(probe_name: str) -> Path:
    stamp = datetime.now().strftime("%Y-%m-%d_%H%M%S")
    return publish_root() / "probes" / f"{probe_name}_{stamp}"
