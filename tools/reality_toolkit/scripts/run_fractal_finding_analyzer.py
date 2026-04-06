"""Analyze a captured fractal finding: root structure, basin maps, charts, CSVs.

Produces a full output directory with PNG charts, CSV data, and a summary.md.

Usage:
    python tools/reality_toolkit/scripts/run_fractal_finding_analyzer.py <finding_dir>
    python tools/reality_toolkit/scripts/run_fractal_finding_analyzer.py --finding-id 220722_840__explaino_dual --date 2026-04-05
    python tools/reality_toolkit/scripts/run_fractal_finding_analyzer.py <finding_dir> --open
"""
from __future__ import annotations

import json
import os
import subprocess
import sys
from dataclasses import asdict
from pathlib import Path

# Add the toolkit package to the path
_toolkit_root = Path(__file__).resolve().parents[1]
if str(_toolkit_root) not in sys.path:
    sys.path.insert(0, str(_toolkit_root))

from fractal_explorer.finding_analyzer import analyze_finding, format_report
from fractal_explorer.finding_charts import generate_all
from fractal_explorer.paths import findings_root


def main(args=None):
    import argparse
    parser = argparse.ArgumentParser(
        description="Analyze a captured fractal finding and produce charts + CSVs")
    parser.add_argument("finding_dir", nargs="?",
                        help="Path to finding directory (contains state.json + frame.png)")
    parser.add_argument("--finding-id", help="Finding ID to locate under findings root")
    parser.add_argument("--date", help="Date subfolder (YYYY-MM-DD)")
    parser.add_argument("--group", default="manual_capture",
                        help="Finding group (default: manual_capture)")
    parser.add_argument("--sample-step", type=int, default=8,
                        help="Basin map sampling step (lower=slower, more detail)")
    parser.add_argument("--convergence-grid", type=int, default=200,
                        help="Convergence heatmap resolution (default 200)")
    parser.add_argument("--out-dir",
                        help="Output directory (default: <finding_dir>/analysis/)")
    parser.add_argument("--open", action="store_true",
                        help="Open output folder in Explorer after generation")
    parsed = parser.parse_args(args)

    # Resolve finding directory
    if parsed.finding_dir:
        finding_dir = Path(parsed.finding_dir)
    elif parsed.finding_id and parsed.date:
        repo_root = Path(__file__).resolve().parents[2]
        root = findings_root(repo_root)
        finding_dir = root / parsed.group / parsed.date / parsed.finding_id
    else:
        parser.error("Provide finding_dir or --finding-id + --date")
        return 1

    if not finding_dir.exists():
        print(f"ERROR: {finding_dir} does not exist", file=sys.stderr)
        return 1

    # Resolve output directory
    if parsed.out_dir:
        out_dir = Path(parsed.out_dir)
    else:
        out_dir = finding_dir / "analysis"

    print(f"Analyzing: {finding_dir}")
    print(f"Output to: {out_dir}")
    print()

    # Run analysis
    analysis = analyze_finding(finding_dir, sample_step=parsed.sample_step)

    # Print text report
    print(format_report(analysis))
    print()

    # Generate all outputs
    print("Generating charts and data exports...")
    manifest = generate_all(analysis, out_dir,
                            convergence_grid=parsed.convergence_grid)

    # Print manifest
    print()
    print("=" * 60)
    print("OUTPUT FILES")
    print("=" * 60)
    for key, path in sorted(manifest.items()):
        label = key.replace("_", " ").title()
        print(f"  {label:30s} {path}")
    print()
    print(f"Summary: {out_dir / 'summary.md'}")
    print(f"Output folder: {out_dir}")

    # Optionally open in explorer
    if parsed.open:
        if sys.platform == "win32":
            os.startfile(str(out_dir))
        elif sys.platform == "darwin":
            subprocess.run(["open", str(out_dir)])
        else:
            subprocess.run(["xdg-open", str(out_dir)])

    return 0


if __name__ == "__main__":
    sys.exit(main())
