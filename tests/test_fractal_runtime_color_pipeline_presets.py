from __future__ import annotations

import json
import sys
from pathlib import Path

import pytest

from tests.runtime_harness import (
    PersistentRuntimeViewerAutomation,
    active_runtime_exe,
    run_headless_capture,
    runtime_automation_lock,
    write_state_bundle,
)


@pytest.fixture(autouse=True)
def _serialize_runtime_automation():
    with runtime_automation_lock():
        yield


def test_color_pipeline_recipe_presets_are_visible_and_apply_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Color Pipeline preset runtime regression is Windows-only")

    exe_path = active_runtime_exe()
    neutral_capture = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "mandelbrot",
        "--width",
        "160",
        "--height",
        "120",
    )
    state_path = write_state_bundle(
        tmp_path / "color_pipeline_preset_seed",
        json.loads(json.dumps(neutral_capture["state"])),
    )
    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "color_pipeline_presets_report.json",
        command_path=tmp_path / "color_pipeline_presets_command.json",
        open_color_pipeline=True,
    ) as viewer:
        ready_report = viewer.wait_for_report(timeout_seconds=30.0)
        base_hash = ready_report.get("rendered_frame_hash")
        assert isinstance(base_hash, str), ready_report
        viewer.wait_for_control("color_pipeline.recipe.default_smooth_escape.apply", timeout_seconds=20.0)
        viewer.wait_for_control("color_pipeline.recipe.phase_orbit_wheel.apply", timeout_seconds=20.0)
        viewer.wait_for_control("color_pipeline.recipe.sdf_normal_angle_diagnostic.apply", timeout_seconds=20.0)
        applied = viewer.click_control("color_pipeline.recipe.sdf_normal_angle_diagnostic.apply", timeout_seconds=60.0)

    assert applied.get("click_consumed") is True, applied
    assert "source:sdf_normal_angle" in applied.get("lane_rows", []), applied
    assert "palette:phase_wheel_palette" in applied.get("lane_rows", []), applied
    assert "grading:phase_finish" in applied.get("lane_rows", []), applied
    assert applied.get("rendered_frame_hash") != base_hash, applied
