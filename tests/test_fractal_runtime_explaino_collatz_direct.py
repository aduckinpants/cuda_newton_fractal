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


def test_explaino_collatz_direct_controls_change_live_output_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("viewer automation is Windows-only")

    exe_path = active_runtime_exe()
    state = _capture_state(exe_path, "explaino_collatz_direct")
    state["params"]["collatz_transition_strength"] = 1.0
    state["params"]["explaino_warp_strength"] = 0.0
    state["view"]["explaino_phase"] = 0.0
    state["view"]["explaino_phase_strength"] = 1.0
    state["params"]["max_iter"] = 500
    state["params"]["coloring_mode"] = "smooth_escape"
    state["params"]["color_signal"] = "smooth_escape"
    state["params"]["color_palette"] = "cyclic_escape"
    state["params"]["color_grading"] = "escape_default"
    state_path = write_state_bundle(tmp_path / "explaino_collatz_direct", state)

    with runtime_automation_lock(timeout_seconds=20.0):
        with PersistentRuntimeViewerAutomation(
            exe_path=exe_path,
            state_path=state_path,
            report_path=tmp_path / "explaino_collatz_direct_report.json",
            command_path=tmp_path / "explaino_collatz_direct_command.json",
        ) as viewer:
            baseline = viewer.wait_for_report(timeout_seconds=15.0)
            baseline_hash = baseline.get("rendered_frame_hash")
            assert baseline.get("current_fractal_type") == "explaino_collatz_direct"
            assert isinstance(baseline_hash, str) and baseline_hash

            transition_control = "fractal_control.collatz_transition_strength.primary"
            viewer.wait_for_control(transition_control, timeout_seconds=15.0)
            transition_edited = viewer.set_control_value(transition_control, 0.35, timeout_seconds=15.0)
            transition_hash = transition_edited.get("rendered_frame_hash")
            assert transition_edited.get("current_fractal_type") == "explaino_collatz_direct"
            assert transition_edited.get("set_value_consumed") is True
            assert isinstance(transition_hash, str) and transition_hash
            assert transition_hash != baseline_hash

            warp_control = "fractal_control.explaino_warp_strength.primary"
            viewer.wait_for_control(warp_control, timeout_seconds=15.0)
            warp_edited = viewer.set_control_value(warp_control, 0.35, timeout_seconds=15.0)
            warp_hash = warp_edited.get("rendered_frame_hash")
            assert warp_edited.get("current_fractal_type") == "explaino_collatz_direct"
            assert warp_edited.get("set_value_consumed") is True
            assert isinstance(warp_hash, str) and warp_hash
            assert warp_hash != transition_hash
