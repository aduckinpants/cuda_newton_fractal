from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

import pytest

from tests.runtime_harness import active_runtime_exe


LOW_UNIQUE_TARGETS = [
    "nova",
    "mcmullen",
    "magnet",
    "explaino_nova",
    "explaino_rational_escape",
]


def test_remaining_smooth_escape_low_unique_rows_clear_published_inventory(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("published viewer runtime inventory is Windows-only")

    exe_path = active_runtime_exe()
    out_dir = tmp_path / "smooth_escape_low_unique"
    command = [
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
        "--runtime-lock",
    ]
    for fractal_type in LOW_UNIQUE_TARGETS:
        command.extend(["--fractal-type", fractal_type])

    result = subprocess.run(
        command,
        cwd=str(Path(__file__).resolve().parents[1]),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stderr or result.stdout

    report = json.loads((out_dir / "inventory.json").read_text(encoding="utf-8"))
    assert report["fractal_types"] == LOW_UNIQUE_TARGETS
    assert report["analysis"]["low_unique_color_cases"] == []

    cases_by_type = {case["fractal_type"]: case for case in report["cases"]}
    for fractal_type in LOW_UNIQUE_TARGETS:
        metrics = cases_by_type[fractal_type]["frame_metrics"]
        assert metrics["unique_rgb_count"] >= 200, (fractal_type, metrics)
