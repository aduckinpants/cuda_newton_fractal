from __future__ import annotations

import json
import subprocess
import sys
from pathlib import Path

import pytest

from tests.runtime_harness import active_runtime_exe


def test_sdf_performance_witness_runs_against_published_runtime_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("persistent runtime viewer harness is Windows-only")

    exe_path = active_runtime_exe()
    out_json = tmp_path / "sdf_performance_witness.json"
    out_md = tmp_path / "sdf_performance_witness.md"
    work_dir = tmp_path / "witness_work"

    result = subprocess.run(
        [
            sys.executable,
            "tools/viewer_host_sdf_performance_witness.py",
            "--runtime-exe",
            str(exe_path),
            "--out-json",
            str(out_json),
            "--out-md",
            str(out_md),
            "--work-dir",
            str(work_dir),
            "--width",
            "320",
            "--height",
            "240",
            "--include-preview-sample",
        ],
        cwd=Path(__file__).resolve().parents[1],
        text=True,
        capture_output=True,
        check=False,
        timeout=240,
    )

    assert result.returncode == 0, result.stderr or result.stdout
    assert out_json.exists()
    assert out_md.exists()
    report = json.loads(out_json.read_text(encoding="utf-8"))
    assert report["schema_version"] == 1
    assert report["no_mouse_automation"] is True
    assert report["persistent_viewer_launch_count"] == 1
    assert report["summary"]["scenario_count"] >= 5
    assert report["summary"]["sdf_scenario_count"] >= 4
    assert report["summary"]["preview_sample_count"] >= 1
    assert report["summary"]["full_quality_sample_count"] >= 4
    assert report["summary"]["recommendation"] in {
        "postprocess_optimization_candidate",
        "field_generation_or_downsample_candidate",
        "mixed_or_inconclusive_measurement_review_required",
    }
    for scenario in report["scenarios"]:
        assert scenario["rendered_frame_hash"].startswith("fnv1a64:")
        assert scenario["rendered_frame_width"] > 0
        assert scenario["rendered_frame_height"] > 0
        assert scenario["classification"] in {
            "non_sdf_baseline",
            "field_generation_pressure",
            "postprocess_pressure",
            "preview_quality_sample",
            "low_sdf_cost",
            "mixed_or_inconclusive",
        }
        if scenario["name"].endswith("_settled_full_quality"):
            assert scenario["rendered_frame_width"] >= scenario["target_render_width"], scenario
            assert scenario["rendered_frame_height"] >= scenario["target_render_height"], scenario
            assert scenario["lens_sdf_postprocess_pixel_step"] == 1, scenario
