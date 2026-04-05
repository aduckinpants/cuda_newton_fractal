from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys


REPO_ROOT = Path(__file__).resolve().parents[3]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.reality_toolkit.fractal_explorer.paths import default_seed_sweep_out_dir, diagnostics_last_dir, runtime_root
from tools.reality_toolkit.fractal_explorer.seed_sweep import SweepConfig, build_seed_values, run_seed_sweep


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run a deterministic Explaino-family seed sweep against the local clone viewer app")
    parser.add_argument("--seed-start", type=float, default=0.70)
    parser.add_argument("--seed-stop", type=float, default=0.80)
    parser.add_argument("--seed-step", type=float, default=0.02)
    parser.add_argument("--seed", type=float, action="append", dest="explicit_seeds")
    parser.add_argument("--fractal-type", default="explaino", choices=["explaino", "explaino_y", "explaino_fp"])
    parser.add_argument("--width", type=int)
    parser.add_argument("--height", type=int)
    parser.add_argument("--explaino-phase", type=float)
    parser.add_argument("--explaino-warp-strength", type=float)
    parser.add_argument("--timeout-seconds", type=float, default=180.0)
    parser.add_argument("--no-validate-ui", action="store_true")
    parser.add_argument(
        "--out-dir",
        type=Path,
        default=None,
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    repo_root = REPO_ROOT
    out_dir = args.out_dir or default_seed_sweep_out_dir(repo_root)

    seeds = build_seed_values(
        seed_start=args.seed_start,
        seed_stop=args.seed_stop,
        seed_step=args.seed_step,
        explicit_seeds=args.explicit_seeds,
    )

    config = SweepConfig(
        repo_root=repo_root,
        exe_path=runtime_root(repo_root) / "fractal_ui.exe",
        diagnostics_last_dir=diagnostics_last_dir(repo_root),
        out_dir=out_dir,
        seeds=seeds,
        fractal_type=args.fractal_type,
        validate_ui=not args.no_validate_ui,
        timeout_seconds=args.timeout_seconds,
        width=args.width,
        height=args.height,
        explaino_phase=args.explaino_phase,
        explaino_warp_strength=args.explaino_warp_strength,
    )

    summary = run_seed_sweep(config)
    print(json.dumps({
        "runs_total": summary["runs_total"],
        "runs_ok": summary["runs_ok"],
        "best_edge_seed": summary["best_edge_seed"],
        "best_delta_seed": summary["best_delta_seed"],
        "out_dir": str(config.out_dir),
    }, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())