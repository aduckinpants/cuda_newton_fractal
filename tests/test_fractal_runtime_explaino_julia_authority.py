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


def _capture_state(exe_path: Path, fractal_type: str) -> dict[str, object]:
    capture = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        fractal_type,
        "--width",
        "320",
        "--height",
        "240",
    )
    return json.loads(json.dumps(capture["state"]))


def test_explaino_julia_custom_constants_change_live_output_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("viewer automation is Windows-only")

    exe_path = active_runtime_exe()
    state = _capture_state(exe_path, "explaino_julia")
    state["params"]["explaino_julia_constant_mode"] = "custom"
    state["params"]["explaino_julia_c_real"] = -0.7
    state["params"]["explaino_julia_c_imag"] = 0.27015
    state["params"]["max_iter"] = 700
    state["params"]["coloring_mode"] = "smooth_escape"
    state["params"]["color_signal"] = "smooth_escape"
    state["params"]["color_palette"] = "cyclic_escape"
    state["params"]["color_grading"] = "escape_default"
    state_path = write_state_bundle(tmp_path / "explaino_julia_authority", state)

    with runtime_automation_lock(timeout_seconds=20.0):
        with PersistentRuntimeViewerAutomation(
            exe_path=exe_path,
            state_path=state_path,
            report_path=tmp_path / "explaino_julia_authority_report.json",
            command_path=tmp_path / "explaino_julia_authority_command.json",
        ) as viewer:
            baseline = viewer.wait_for_report(timeout_seconds=15.0)
            baseline_hash = baseline.get("rendered_frame_hash")
            assert baseline.get("current_fractal_type") == "explaino_julia"
            assert isinstance(baseline_hash, str) and baseline_hash

            real_control = "fractal_control.explaino_julia_c_real.primary"
            viewer.wait_for_control(real_control, timeout_seconds=15.0)
            real_edited = viewer.set_control_value(real_control, 0.285, timeout_seconds=15.0)
            real_hash = real_edited.get("rendered_frame_hash")
            assert real_edited.get("current_fractal_type") == "explaino_julia"
            assert real_edited.get("set_value_consumed") is True
            assert isinstance(real_hash, str) and real_hash
            assert real_hash != baseline_hash

            imag_control = "fractal_control.explaino_julia_c_imag.primary"
            viewer.wait_for_control(imag_control, timeout_seconds=15.0)
            imag_edited = viewer.set_control_value(imag_control, 0.01, timeout_seconds=15.0)
            imag_hash = imag_edited.get("rendered_frame_hash")
            assert imag_edited.get("current_fractal_type") == "explaino_julia"
            assert imag_edited.get("set_value_consumed") is True
            assert isinstance(imag_hash, str) and imag_hash
            assert imag_hash != real_hash
