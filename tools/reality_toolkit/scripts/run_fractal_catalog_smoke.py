from __future__ import annotations

import argparse
import json
from pathlib import Path
import sys


REPO_ROOT = Path(__file__).resolve().parents[3]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

from tools.reality_toolkit.fractal_explorer.fractal_catalog_smoke import (
    default_fractal_catalog_smoke_out_dir,
    run_fractal_catalog_smoke,
    write_fractal_catalog_smoke_report,
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run a sample-API smoke audit across every advertised fractal type")
    parser.add_argument("--out-dir", type=Path, default=None, help="Output directory for JSON/CSV smoke reports")
    parser.add_argument("--timeout-seconds", type=float, default=60.0, help="Per-fractal sample request timeout")
    parser.add_argument("--strict", action="store_true", help="Exit non-zero when any advertised fractal type fails")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    out_dir = args.out_dir or default_fractal_catalog_smoke_out_dir()
    report = run_fractal_catalog_smoke(repo_root=REPO_ROOT, timeout_seconds=args.timeout_seconds)
    json_path, csv_path = write_fractal_catalog_smoke_report(out_dir, report)
    print(json.dumps({
        "fractal_types_total": report["fractal_types_total"],
        "fractal_types_ok": report["fractal_types_ok"],
        "fractal_types_failed": report["fractal_types_failed"],
        "json_report": str(json_path),
        "csv_report": str(csv_path),
    }, indent=2))
    if args.strict and report["fractal_types_failed"]:
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
