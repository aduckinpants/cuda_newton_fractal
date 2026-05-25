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


def test_sdf_viewport_overlay_is_visible_controlled_and_does_not_require_lens_window_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("SDF viewport overlay runtime regression is Windows-only")

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
    state = json.loads(json.dumps(neutral_capture["state"]))
    lens = state.setdefault("lens", {})
    assert isinstance(lens, dict)
    lens["enabled"] = False
    lens["downsample"] = 2
    state_path = write_state_bundle(tmp_path / "sdf_viewport_overlay_seed", state)

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "sdf_viewport_overlay_report.json",
        command_path=tmp_path / "sdf_viewport_overlay_command.json",
    ) as viewer:
        ready_report = viewer.wait_for_report(timeout_seconds=30.0)
        assert ready_report.get("lens_sdf_enabled") is False, ready_report
        assert ready_report.get("lens_sdf_overlay_mode") == "off", ready_report
        assert ready_report.get("lens_sdf_overlay_active") is False, ready_report
        viewer.wait_for_control("fractal_control.lens_sdf_overlay_mode.primary", timeout_seconds=20.0)
        overlay_report = viewer.set_enum_id(
            "fractal.lens.sdf_overlay_mode",
            "field_debug",
            timeout_seconds=60.0,
        )
        assert overlay_report.get("lens_sdf_enabled") is False, overlay_report
        assert overlay_report.get("lens_sdf_valid") is True, overlay_report
        assert overlay_report.get("lens_sdf_overlay_mode") == "field_debug", overlay_report
        assert overlay_report.get("lens_sdf_overlay_active") is True, overlay_report
        assert overlay_report.get("rendered_frame_hash") == ready_report.get("rendered_frame_hash"), (
            "the overlay should be a viewport presentation layer, not a mutation of the captured base frame"
        )
        viewer.wait_for_control("fractal_control.lens_downsample.primary", timeout_seconds=20.0)
        edited_downsample_report = viewer.set_control_value(
            "fractal_control.lens_downsample.primary",
            4.0,
            timeout_seconds=60.0,
        )

    assert edited_downsample_report.get("lens_sdf_overlay_active") is True, edited_downsample_report
    assert edited_downsample_report.get("lens_sdf_pixel_scale") == pytest.approx(4.0), edited_downsample_report