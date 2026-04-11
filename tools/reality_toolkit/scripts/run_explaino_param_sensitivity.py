from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[3]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.reality_toolkit.fractal_explorer.explaino_param_sensitivity import (
    DEFAULT_SENSITIVITY_METRICS,
    EXPLAINO_VARIANT_SENSITIVITY_SPECS,
    default_explaino_param_sensitivity_out_dir,
    run_explaino_param_sensitivity_sweep,
)


def main() -> int:
    parser = argparse.ArgumentParser(description="Sweep Explaino variant strengths from zero axis to shipped defaults")
    parser.add_argument("--variant", action="append", dest="variants", help="Variant name to sweep; may be repeated. Defaults to all variants.")
    parser.add_argument("--list-variants", action="store_true", help="List the available Explaino variants and exit")
    parser.add_argument("--ticks", type=int, default=5, help="Number of ticks per variant (default: 5)")
    parser.add_argument("--out-dir", help="Output directory for sweep results")
    parser.add_argument("--base-state", help="Optional base state JSON to load before applying overrides")
    parser.add_argument("--center-x", type=float, default=0.0, help="Grid center x (default: 0.0)")
    parser.add_argument("--center-y", type=float, default=0.0, help="Grid center y (default: 0.0)")
    parser.add_argument("--span-x", type=float, default=0.2, help="Grid span x (default: 0.2)")
    parser.add_argument("--span-y", type=float, default=0.2, help="Grid span y (default: 0.2)")
    parser.add_argument("--grid-width", type=int, default=16, help="Grid width (default: 16)")
    parser.add_argument("--grid-height", type=int, default=16, help="Grid height (default: 16)")
    parser.add_argument("--metric", action="append", dest="metrics", help="Explicit sample metric; may be repeated")
    parser.add_argument("--timeout-seconds", type=float, default=180.0, help="Sample request timeout in seconds")
    parser.add_argument("--exe-path", help="Optional explicit runtime path")
    parser.add_argument("--dry-run", action="store_true", help="Write request/manifests without launching the runtime")
    args = parser.parse_args()

    if args.list_variants:
        print("Available variants:")
        for spec in EXPLAINO_VARIANT_SENSITIVITY_SPECS:
            print(f"  {spec.variant_name:18s}  {spec.param_path:34s}  default={spec.default_value:g}")
        return 0

    out_dir = Path(args.out_dir) if args.out_dir else default_explaino_param_sensitivity_out_dir(REPO_ROOT)
    base_state = Path(args.base_state) if args.base_state else None
    exe_path = Path(args.exe_path) if args.exe_path else None
    metrics = list(args.metrics) if args.metrics else list(DEFAULT_SENSITIVITY_METRICS)

    summary = run_explaino_param_sensitivity_sweep(
        repo_root=REPO_ROOT,
        out_dir=out_dir,
        ticks=args.ticks,
        variants=args.variants,
        center_x=args.center_x,
        center_y=args.center_y,
        span_x=args.span_x,
        span_y=args.span_y,
        grid_width=args.grid_width,
        grid_height=args.grid_height,
        base_state_path=base_state,
        metrics=metrics,
        timeout_seconds=args.timeout_seconds,
        exe_path=exe_path,
        dry_run=args.dry_run,
    )
    print(json.dumps(summary, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())