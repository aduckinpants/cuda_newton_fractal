"""CLI runner for finding analysis.

Usage:
    python tools/reality_toolkit/scripts/run_fractal_finding_analyzer.py <finding_dir>
    python tools/reality_toolkit/scripts/run_fractal_finding_analyzer.py --finding-id 220722_840__explaino_dual --date 2026-04-05
"""
from __future__ import annotations

import sys
from pathlib import Path

# Add the toolkit package to the path
_toolkit_root = Path(__file__).resolve().parents[1]
if str(_toolkit_root) not in sys.path:
    sys.path.insert(0, str(_toolkit_root))

from fractal_explorer.finding_analyzer import main

if __name__ == "__main__":
    sys.exit(main())
