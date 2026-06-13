from __future__ import annotations

import json
import sys
from pathlib import Path

import pytest

from tests.runtime_harness import (
    PersistentRuntimeViewerAutomation,
    active_runtime_exe,
    run_headless_capture,
    write_state_bundle,
)


def _require_rendered_frame_hash(payload: dict[str, object]) -> str:
    assert payload.get("rendered_frame_ready") is True, payload
    frame_hash = payload.get("rendered_frame_hash")
    assert isinstance(frame_hash, str) and frame_hash.startswith("fnv1a64:"), payload
    return frame_hash


def _payload_has_control(payload: dict[str, object], control_id: str) -> bool:
    controls = payload.get("controls")
    assert isinstance(controls, list), payload
    for control in controls:
        if isinstance(control, dict) and control.get("control_id") == control_id:
            return True
    return False


def test_live_aa_controls_are_default_off_and_no_mouse_settable(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("persistent runtime viewer harness is Windows-only")

    exe_path = active_runtime_exe()
    neutral_capture = run_headless_capture(
        str(exe_path), "--capture-diagnostic", "--fractal-type", "multibrot", "--width", "320", "--height", "240"
    )
    state = json.loads(json.dumps(neutral_capture["state"]))
    render = state.setdefault("render", {})
    assert isinstance(render, dict)
    render["width"] = 320
    render["height"] = 240
    render["aa_mode"] = "off"
    params = state.setdefault("params", {})
    assert isinstance(params, dict)
    params["max_iter"] = 180
    params["coloring_mode"] = "smooth_escape"
    params["color_signal"] = "smooth_escape"
    params["color_shape"] = "identity"
    params["color_palette"] = "cyclic_escape"
    params["color_grading"] = "escape_default"
    state_path = write_state_bundle(tmp_path / "live_aa_controls", state)

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "live_aa_report.json",
        command_path=tmp_path / "live_aa_command.json",
    ) as viewer:
        viewer.wait_for_control("fractal_control.aa_enabled.primary", timeout_seconds=20.0)
        baseline = viewer.wait_for_report(timeout_seconds=20.0)
        baseline_hash = _require_rendered_frame_hash(baseline)
        assert _payload_has_control(baseline, "fractal_control.aa_enabled.primary")
        assert not _payload_has_control(baseline, "fractal_control.aa_mode.primary"), baseline

        enabled = viewer.set_control_value("fractal_control.aa_enabled.primary", 1.0, timeout_seconds=20.0)
        enabled_hash = _require_rendered_frame_hash(enabled)
        assert enabled.get("set_value_consumed") is True, enabled
        assert enabled.get("current_fractal_type") == "multibrot", enabled
        viewer.wait_for_control("fractal_control.aa_mode.primary", timeout_seconds=20.0)
        assert enabled_hash != baseline_hash, "enabling SSAA should change the rendered frame hash"

        disabled = viewer.set_control_value("fractal_control.aa_enabled.primary", 0.0, timeout_seconds=20.0)
        disabled_hash = _require_rendered_frame_hash(disabled)
        assert disabled.get("set_value_consumed") is True, disabled
        assert disabled_hash == baseline_hash, "disabling live AA should return to the AA-off rendered frame"
        assert viewer.launch_count == 1
