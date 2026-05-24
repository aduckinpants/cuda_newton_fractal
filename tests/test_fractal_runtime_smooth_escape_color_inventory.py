from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

import pytest

from tests.runtime_harness import active_runtime_exe


def test_smooth_escape_color_inventory_runs_against_published_runtime(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("published viewer runtime inventory is Windows-only")

    exe_path = active_runtime_exe()
    out_dir = tmp_path / "smooth_escape_inventory"
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
            "mandelbrot",
            "--fractal-type",
            "magnet",
            "--runtime-lock",
        ],
        cwd=str(Path(__file__).resolve().parents[1]),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout

    report_path = out_dir / "inventory.json"
    markdown_path = out_dir / "inventory.md"
    assert report_path.exists()
    assert markdown_path.exists()

    report = json.loads(report_path.read_text(encoding="utf-8"))
    assert report["runtime_exe"] == str(exe_path)
    assert report["width"] == 48
    assert report["height"] == 36
    assert [case["fractal_type"] for case in report["cases"]] == ["mandelbrot", "magnet"]
    assert report["analysis"]["observed_color_tuple_count"] == 1
    cases_by_type = {case["fractal_type"]: case for case in report["cases"]}
    assert cases_by_type["magnet"]["frame_metrics"]["black_pixel_fraction"] < 0.50
    assert "magnet" not in report["analysis"]["high_black_fraction_cases"]
