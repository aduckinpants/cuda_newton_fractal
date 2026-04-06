"""Parameter probe sweep for the fractal viewer.

Generates state.json variants with systematically modulated parameters,
then launches the viewer for each to capture frames. Follows the nine repo
probe sweep pattern: probe definitions as tuples, modulation functions,
and structured output (CSV + JSON summary).

Usage:
    py -3.14 tools/reality_toolkit/scripts/run_fractal_param_probe_sweep.py \
        --probe phase_strength_sine --ticks 30 --out-dir D:/salt-fractal/probes/run1

    py -3.14 tools/reality_toolkit/scripts/run_fractal_param_probe_sweep.py \
        --list-probes
"""
from __future__ import annotations

import argparse
import csv
import json
import math
import shutil
import subprocess
import sys
from datetime import datetime
from pathlib import Path
from typing import Callable

REPO_ROOT = Path(__file__).resolve().parents[3]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.reality_toolkit.fractal_explorer.paths import publish_root, runtime_root


# ---------------------------------------------------------------------------
# Modulation functions: t in [0, 1] -> parameter value
# ---------------------------------------------------------------------------

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


# ---------------------------------------------------------------------------
# Probe definitions: (label, param_path, modulation_fn, description)
# param_path uses dot notation: "view.explaino_phase_strength" or "params.explaino_damping"
# ---------------------------------------------------------------------------

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


def _default_base_state() -> dict:
    """Minimal default state suitable for explaino probing."""
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


def _set_nested(state: dict, dotpath: str, value: float) -> None:
    """Set a value in state dict using dot notation like 'view.explaino_phase'."""
    parts = dotpath.split(".")
    obj = state
    for part in parts[:-1]:
        obj = obj[part]
    obj[parts[-1]] = value


def generate_probe_states(
    probe_name: str,
    ticks: int,
    base_state: dict | None = None,
) -> list[tuple[int, float, float, dict]]:
    """Generate (tick_index, t, param_value, state_dict) for each tick."""
    if probe_name not in PROBES:
        raise ValueError(f"Unknown probe: {probe_name}. Use --list-probes.")

    label, param_path, mod_fn, _desc = PROBES[probe_name]
    base = base_state or _default_base_state()

    results = []
    for i in range(ticks):
        t = i / max(1, ticks - 1) if ticks > 1 else 0.0
        value = mod_fn(t)
        state = json.loads(json.dumps(base))  # deep copy
        _set_nested(state, param_path, value)
        results.append((i, t, value, state))
    return results


def write_probe_manifest(
    out_dir: Path,
    probe_name: str,
    ticks: list[tuple[int, float, float]],
) -> Path:
    """Write a CSV manifest of the probe sweep."""
    manifest_path = out_dir / "probe_manifest.csv"
    with open(manifest_path, "w", newline="", encoding="utf-8") as f:
        writer = csv.writer(f)
        writer.writerow(["tick", "t", "param_value", "state_json"])
        for idx, t, val in ticks:
            writer.writerow([idx, f"{t:.6f}", f"{val:.8g}", f"tick_{idx:04d}/state.json"])
    return manifest_path


def run_probe_sweep(
    probe_name: str,
    ticks: int,
    out_dir: Path,
    base_state_path: Path | None = None,
    dry_run: bool = False,
) -> Path:
    """Run a full parameter probe sweep, generating state files and optionally launching the viewer."""
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

    manifest = write_probe_manifest(out_dir, probe_name, manifest_ticks)

    summary = {
        "probe_name": probe_name,
        "ticks": ticks,
        "timestamp": datetime.now().isoformat(),
        "out_dir": str(out_dir),
        "manifest": str(manifest),
        "description": PROBES[probe_name][3],
        "param_path": PROBES[probe_name][1],
        "dry_run": dry_run,
    }
    summary_path = out_dir / "probe_summary.json"
    summary_path.write_text(json.dumps(summary, indent=2), encoding="utf-8")

    if not dry_run:
        launcher = runtime_root(REPO_ROOT) / "fractal_ui.cmd"
        if not launcher.exists():
            launcher = runtime_root(REPO_ROOT) / "fractal_ui.exe"
        if launcher.exists():
            for idx, t, val, state in states:
                tick_dir = out_dir / f"tick_{idx:04d}"
                state_path = tick_dir / "state.json"
                cmd = [str(launcher), "--load-state-json", str(state_path), "--capture-diagnostic"]
                print(f"  tick {idx:4d}/{ticks}  t={t:.4f}  val={val:.6g} ...", end="", flush=True)
                result = subprocess.run(cmd, capture_output=True, text=True, timeout=60)
                if result.returncode == 0:
                    # Copy captured frame if available
                    diag_frame = runtime_root(REPO_ROOT) / "diagnostics" / "last" / "frame.bmp"
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


def main() -> int:
    parser = argparse.ArgumentParser(description="Parameter probe sweep for fractal viewer")
    parser.add_argument("--probe", help="Probe name to run")
    parser.add_argument("--ticks", type=int, default=30, help="Number of ticks in the sweep (default: 30)")
    parser.add_argument("--out-dir", help="Output directory for sweep results")
    parser.add_argument("--base-state", help="Path to a base state.json to modulate from")
    parser.add_argument("--dry-run", action="store_true", help="Generate state files without launching viewer")
    parser.add_argument("--list-probes", action="store_true", help="List available probe definitions")
    args = parser.parse_args()

    if args.list_probes:
        print("Available probes:")
        for name, (label, path, _fn, desc) in sorted(PROBES.items()):
            print(f"  {name:30s}  {path:40s}  {desc}")
        return 0

    if not args.probe:
        parser.error("--probe is required (use --list-probes to see available probes)")

    stamp = datetime.now().strftime("%Y-%m-%d_%H%M%S")
    out_dir = Path(args.out_dir) if args.out_dir else publish_root() / "probes" / f"{args.probe}_{stamp}"
    base_state = Path(args.base_state) if args.base_state else None

    run_probe_sweep(
        probe_name=args.probe,
        ticks=args.ticks,
        out_dir=out_dir,
        base_state_path=base_state,
        dry_run=args.dry_run,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
