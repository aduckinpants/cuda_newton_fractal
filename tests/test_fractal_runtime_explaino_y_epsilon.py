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
    runtime_automation_lock,
    write_state_bundle,
)


def _require_frame_hash(payload: dict[str, object]) -> str:
    assert payload.get("rendered_frame_ready") is True
    assert payload.get("target_render_width") == 320
    assert payload.get("target_render_height") == 240
    assert payload.get("rendered_frame_width") == 320
    assert payload.get("rendered_frame_height") == 240
    assert payload.get("render_pacing_preview_active") is False
    assert float(payload.get("render_pacing_preview_scale", 0.0)) == pytest.approx(1.0)
    frame_hash = payload.get("rendered_frame_hash")
    assert isinstance(frame_hash, str) and frame_hash.startswith("fnv1a64:")
    return frame_hash


def _configure_explaino_y_tuned_state(state: dict[str, object]) -> None:
    view = state["view"]
    assert isinstance(view, dict)
    view["center_x"] = 0.25
    view["center_y"] = -0.1
    view["center_hp_x"] = 0.25
    view["center_hp_y"] = -0.1
    view["zoom"] = 2.0
    view["log2_zoom"] = math.log2(2.0)
    view["auto_max_iter"] = False

    params = state["params"]
    assert isinstance(params, dict)
    params.update(
        {
            "epsilon": 1.0e-6,
            "max_iter": 64,
            "coloring_mode": "smooth_escape",
            "color_signal": "smooth_escape",
            "color_shape": "identity",
            "color_palette": "cyclic_escape",
            "color_grading": "escape_default",
            "color_smooth_escape_scale": 1.0,
            "color_smooth_escape_bias": 0.0,
        }
    )

    render = state["render"]
    assert isinstance(render, dict)
    render["resolution"] = {"x": 320, "y": 240}
    render["preview_min_scale"] = 1.0


def test_explaino_y_epsilon_tuned_witness_changes_live_frame_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("viewer automation is Windows-only")

    exe_path = active_runtime_exe()
    capture = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino_y",
        "--width",
        "320",
        "--height",
        "240",
    )
    state = json.loads(json.dumps(capture["state"]))
    _configure_explaino_y_tuned_state(state)
    state_path = write_state_bundle(tmp_path / "explaino_y_epsilon_tuned", state)

    with runtime_automation_lock(timeout_seconds=20.0):
        with PersistentRuntimeViewerAutomation(
            exe_path=exe_path,
            state_path=state_path,
            report_path=tmp_path / "explaino_y_epsilon_report.json",
            command_path=tmp_path / "explaino_y_epsilon_command.json",
        ) as viewer:
            epsilon_control = "fractal_control.epsilon.primary"
            viewer.wait_for_control(epsilon_control, timeout_seconds=15.0)
            baseline = viewer.wait_for_report(timeout_seconds=15.0)
            baseline_hash = _require_frame_hash(baseline)
            assert baseline.get("current_fractal_type") == "explaino_y"

            edited = viewer.set_control_value(epsilon_control, 0.01, timeout_seconds=15.0)
            edited_hash = _require_frame_hash(edited)
            assert edited.get("current_fractal_type") == "explaino_y"
            assert edited.get("set_value_consumed") is True
            assert edited_hash != baseline_hash
            assert viewer.launch_count == 1
