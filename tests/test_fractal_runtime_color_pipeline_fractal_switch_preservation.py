from __future__ import annotations

import copy
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


def _multibrot_sdf_normal_angle_state(tmp_path: Path) -> Path:
    exe_path = active_runtime_exe()
    capture = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "multibrot",
        "--width",
        "192",
        "--height",
        "144",
    )
    state = copy.deepcopy(capture["state"])
    params = state["params"]
    params["coloring_mode"] = "phase"
    params["color_signal"] = "sdf_normal_angle"
    params["color_palette"] = "phase_wheel"
    params["color_grading"] = "phase_default"
    params["color_source_stack"] = [
        {
            "signal": "sdf_normal_angle",
            "scale": -0.40,
            "bias": 0.18,
            "blend_weight": 1.0,
        }
    ]
    state["lens"] = {
        "enabled": False,
        "downsample": 4,
        "sdf_overlay_mode": "off",
        "sdf_overlay_opacity": 0.55,
        "sdf_overlay_band_px": 1.5,
    }
    return write_state_bundle(tmp_path / "multibrot_sdf_normal_angle", state)


def test_color_pipeline_survives_compatible_fractal_switch_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("persistent viewer runtime proof is Windows-only")

    exe_path = active_runtime_exe()
    state_path = _multibrot_sdf_normal_angle_state(tmp_path)
    report_path = tmp_path / "color_pipeline_fractal_switch_report.json"
    command_path = tmp_path / "color_pipeline_fractal_switch_command.json"

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=report_path,
        command_path=command_path,
        open_color_pipeline=True,
    ) as viewer:
        initial = viewer.wait_for_report(timeout_seconds=30.0)
        assert initial.get("current_fractal_type") == "multibrot"
        assert "source:sdf_normal_angle" in initial.get("lane_rows", [])

        edited_before_switch = viewer.set_control_value(
            "color_pipeline.source.sdf_normal_angle.signal.scale.primary",
            -0.75,
            timeout_seconds=30.0,
        )
        assert edited_before_switch.get("current_fractal_type") == "multibrot"
        assert "source:sdf_normal_angle" in edited_before_switch.get("lane_rows", [])

        switched = viewer.set_enum_id(
            "fractal.view.fractal_type",
            "mandelbrot",
            expected_fractal_type="mandelbrot",
            timeout_seconds=30.0,
        )
        assert switched.get("current_fractal_type") == "mandelbrot"
        assert "source:sdf_normal_angle" in switched.get("lane_rows", [])
        assert "palette:phase_wheel_palette" in switched.get("lane_rows", [])
        switched_hash = switched.get("rendered_frame_hash")
        assert isinstance(switched_hash, str) and switched_hash

        edited_after_switch = viewer.set_control_value(
            "color_pipeline.source.sdf_normal_angle.signal.bias.primary",
            0.35,
            timeout_seconds=30.0,
        )
        assert edited_after_switch.get("current_fractal_type") == "mandelbrot"
        assert "source:sdf_normal_angle" in edited_after_switch.get("lane_rows", [])
        assert edited_after_switch.get("rendered_frame_hash") != switched_hash
