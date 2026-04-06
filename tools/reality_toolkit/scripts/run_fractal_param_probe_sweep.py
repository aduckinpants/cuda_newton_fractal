"""Parameter probe sweep for the fractal runtime.

The default path is the fast sample API (`sequence_grid`) so numeric sweeps can
run without state-file churn or diagnostic frame capture. Legacy
capture-diagnostic mode remains available when visual frame output is still
needed.

Usage:
    py -3.14 tools/reality_toolkit/scripts/run_fractal_param_probe_sweep.py \
        --probe phase_strength_sine --ticks 30 --out-dir D:/salt-fractal/probes/run1

    py -3.14 tools/reality_toolkit/scripts/run_fractal_param_probe_sweep.py \
        --mode capture-diagnostic --probe phase_strength_sine --ticks 30

    py -3.14 tools/reality_toolkit/scripts/run_fractal_param_probe_sweep.py \
        --list-probes
"""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.reality_toolkit.fractal_explorer.param_probe_sweep import (
    DEFAULT_SAMPLE_METRICS,
    DEFAULT_SAMPLE_POINT_METRICS,
    PROBES,
    default_probe_out_dir,
    run_capture_probe_sweep,
    run_sample_probe_sweep,
)


def main() -> int:
    parser = argparse.ArgumentParser(description="Parameter probe sweep for the fractal runtime")
    parser.add_argument("--probe", help="Probe name to run")
    parser.add_argument("--ticks", type=int, default=30, help="Number of ticks in the sweep (default: 30)")
    parser.add_argument("--out-dir", help="Output directory for sweep results")
    parser.add_argument("--base-state", help="Path to a base state.json to modulate from")
    parser.add_argument("--dry-run", action="store_true", help="Generate state files without launching viewer")
    parser.add_argument("--list-probes", action="store_true", help="List available probe definitions")
    parser.add_argument(
        "--mode",
        choices=["sample-sequence-grid", "capture-diagnostic"],
        default="sample-sequence-grid",
        help="Execution path for the sweep (default: sample-sequence-grid)",
    )
    parser.add_argument("--fractal-type", default=None, help="Fractal type override for sample-sequence-grid mode")
    parser.add_argument("--center-x", type=float, default=0.0, help="Grid center x for sample-sequence-grid mode")
    parser.add_argument("--center-y", type=float, default=0.0, help="Grid center y for sample-sequence-grid mode")
    parser.add_argument("--span-x", type=float, default=0.2, help="Grid span x for sample-sequence-grid mode")
    parser.add_argument("--span-y", type=float, default=0.2, help="Grid span y for sample-sequence-grid mode")
    parser.add_argument("--grid-width", type=int, default=16, help="Grid width for sample-sequence-grid mode")
    parser.add_argument("--grid-height", type=int, default=16, help="Grid height for sample-sequence-grid mode")
    parser.add_argument("--include-samples", action="store_true", help="Include per-sample metrics in sample-sequence-grid mode")
    parser.add_argument("--metric", action="append", dest="metrics", help="Explicit sample API metric; may be repeated")
    parser.add_argument("--timeout-seconds", type=float, default=180.0, help="Probe timeout for sample-sequence-grid mode")
    args = parser.parse_args()

    if args.list_probes:
        print("Available probes:")
        for name, (label, path, _fn, desc) in sorted(PROBES.items()):
            print(f"  {name:30s}  {path:40s}  {desc}")
        return 0

    if not args.probe:
        parser.error("--probe is required (use --list-probes to see available probes)")

    out_dir = Path(args.out_dir) if args.out_dir else default_probe_out_dir(args.probe)
    base_state = Path(args.base_state) if args.base_state else None

    if args.mode == "capture-diagnostic":
        run_capture_probe_sweep(
            repo_root=REPO_ROOT,
            probe_name=args.probe,
            ticks=args.ticks,
            out_dir=out_dir,
            base_state_path=base_state,
            dry_run=args.dry_run,
        )
        return 0

    metrics = list(args.metrics) if args.metrics else list(DEFAULT_SAMPLE_METRICS)
    if args.include_samples:
        for metric in DEFAULT_SAMPLE_POINT_METRICS:
            if metric not in metrics:
                metrics.append(metric)

    summary = run_sample_probe_sweep(
        repo_root=REPO_ROOT,
        probe_name=args.probe,
        ticks=args.ticks,
        out_dir=out_dir,
        fractal_type=args.fractal_type,
        center_x=args.center_x,
        center_y=args.center_y,
        span_x=args.span_x,
        span_y=args.span_y,
        grid_width=args.grid_width,
        grid_height=args.grid_height,
        base_state_path=base_state,
        metrics=metrics,
        timeout_seconds=args.timeout_seconds,
        dry_run=args.dry_run,
    )
    print(json.dumps(summary, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
