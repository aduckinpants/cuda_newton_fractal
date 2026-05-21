from __future__ import annotations

import json
import math
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
    assert payload.get("rendered_frame_ready") is True, (
        f"automation report did not include a ready rendered frame: {payload!r}"
    )
    frame_hash = payload.get("rendered_frame_hash")
    assert isinstance(frame_hash, str) and frame_hash.startswith("fnv1a64:"), (
        f"automation report did not include a renderer-owned frame hash: {payload!r}"
    )
    return frame_hash


def _configure_multibrot_state(state: dict[str, object]) -> None:
    view = state["view"]
    assert isinstance(view, dict)
    view["center_x"] = -0.48
    view["center_y"] = 0.0
    view["center_hp_x"] = -0.48
    view["center_hp_y"] = 0.0
    view["zoom"] = 1.0
    view["log2_zoom"] = math.log2(1.0)
    view["auto_max_iter"] = False

    params = state["params"]
    assert isinstance(params, dict)
    params.update(
        {
            "max_iter": 160,
            "multibrot_power_float": 3.0,
            "multibrot_power_imag": 0.0,
            "coloring_mode": "smooth_escape",
            "color_signal": "smooth_escape",
            "color_shape": "identity",
            "color_palette": "cyclic_escape",
            "color_grading": "escape_default",
            "color_smooth_escape_scale": 1.0,
            "color_smooth_escape_bias": 0.0,
        }
    )


def test_multibrot_real_and_imaginary_exponent_controls_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("persistent runtime viewer harness is Windows-only")

    exe_path = active_runtime_exe()
    neutral_capture = run_headless_capture(
        str(exe_path), "--capture-diagnostic", "--fractal-type", "multibrot", "--width", "320", "--height", "240"
    )
    state = json.loads(json.dumps(neutral_capture["state"]))
    _configure_multibrot_state(state)
    state_path = write_state_bundle(tmp_path / "multibrot_exponent", state)

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "multibrot_exponent_report.json",
        command_path=tmp_path / "multibrot_exponent_command.json",
    ) as viewer:
        viewer.wait_for_control("fractal_control.multibrot_power_float.primary", timeout_seconds=15.0)
        viewer.wait_for_control("fractal_control.multibrot_power_imag.primary", timeout_seconds=15.0)
        baseline_hash = _require_rendered_frame_hash(viewer.wait_for_report())

        low_real_payload = viewer.set_control_value("fractal_control.multibrot_power_float.primary", 1.5, timeout_seconds=15.0)
        assert low_real_payload.get("current_fractal_type") == "multibrot"
        low_real_hash = _require_rendered_frame_hash(low_real_payload)

        high_real_payload = viewer.set_control_value("fractal_control.multibrot_power_float.primary", 16.0, timeout_seconds=15.0)
        assert high_real_payload.get("current_fractal_type") == "multibrot"
        high_real_hash = _require_rendered_frame_hash(high_real_payload)

        imag_payload = viewer.set_control_value("fractal_control.multibrot_power_imag.primary", 0.75, timeout_seconds=15.0)
        assert imag_payload.get("current_fractal_type") == "multibrot"
        imag_hash = _require_rendered_frame_hash(imag_payload)

        assert viewer.launch_count == 1
        assert low_real_payload.get("ui_automation_command_sequence") == 1
        assert high_real_payload.get("ui_automation_command_sequence") == 2
        assert imag_payload.get("ui_automation_command_sequence") == 3

    assert low_real_hash != baseline_hash, "below-two Multibrot real exponent edit should change the rendered frame"
    assert high_real_hash != low_real_hash, "above-old-cap Multibrot real exponent edit should change the rendered frame"
    assert imag_hash != high_real_hash, "Multibrot imaginary exponent edit should change the rendered frame"
