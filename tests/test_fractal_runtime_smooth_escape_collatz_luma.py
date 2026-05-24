from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

import pytest

from tests.runtime_harness import active_runtime_exe


def test_collatz_family_smooth_escape_luma_runs_against_published_runtime(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("published viewer runtime inventory is Windows-only")

    exe_path = active_runtime_exe()
    out_dir = tmp_path / "smooth_escape_collatz_luma"
    result = subprocess.run(
        [
            sys.executable,
            "tools/smooth_escape_color_inventory.py",
            "--runtime-exe",
            str(exe_path),
            "--out-dir",
            str(out_dir),
            "--width",
            "48",
            "--height",
            "36",
            "--fractal-type",
            "collatz",
            "--fractal-type",
            "explaino_collatz_direct",
            "--runtime-lock",
        ],
        cwd=str(Path(__file__).resolve().parents[1]),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout

    report = json.loads((out_dir / "inventory.json").read_text(encoding="utf-8"))
    cases_by_type = {case["fractal_type"]: case for case in report["cases"]}
    collatz_metrics = cases_by_type["collatz"]["frame_metrics"]
    explaino_direct_metrics = cases_by_type["explaino_collatz_direct"]["frame_metrics"]

    assert collatz_metrics["luma_max"] - collatz_metrics["luma_min"] >= 90.0
    assert collatz_metrics["black_pixel_fraction"] < 0.05
    assert explaino_direct_metrics["black_pixel_fraction"] < 0.05
    assert "collatz" not in report["analysis"]["low_luma_span_cases"]
