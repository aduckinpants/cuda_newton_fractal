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


def _configure_explaino_nova_state(state: dict[str, object]) -> None:
    view = state["view"]
    assert isinstance(view, dict)
    view["center_x"] = 0.0
    view["center_y"] = 0.0
    view["center_hp_x"] = 0.0
    view["center_hp_y"] = 0.0
    view["zoom"] = 1.0
    view["log2_zoom"] = math.log2(1.0)
    view["auto_max_iter"] = False
    view["explaino_phase"] = 1.1

    params = state["params"]
    assert isinstance(params, dict)
    params.update(
        {
            "epsilon": 1.0e-6,
            "max_iter": 500,
            "nova_alpha": 0.5,
            "explaino_warp_strength": 0.0,
            "explaino_damping": 1.0,
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


def test_explaino_nova_repaired_controls_change_live_frame_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("viewer automation is Windows-only")

    exe_path = active_runtime_exe()
    capture = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino_nova",
        "--width",
        "320",
        "--height",
        "240",
    )
    state = json.loads(json.dumps(capture["state"]))
    _configure_explaino_nova_state(state)
    state_path = write_state_bundle(tmp_path / "explaino_nova_branch_dead_controls", state)

    with runtime_automation_lock(timeout_seconds=20.0):
        with PersistentRuntimeViewerAutomation(
            exe_path=exe_path,
            state_path=state_path,
            report_path=tmp_path / "explaino_nova_branch_dead_report.json",
            command_path=tmp_path / "explaino_nova_branch_dead_command.json",
        ) as viewer:
            warp_control = "fractal_control.explaino_warp_strength.primary"
            damping_control = "fractal_control.explaino_damping.primary"
            viewer.wait_for_control(warp_control, timeout_seconds=15.0)
            viewer.wait_for_control(damping_control, timeout_seconds=15.0)
            baseline = viewer.wait_for_report(timeout_seconds=20.0)
            baseline_hash = _require_frame_hash(baseline)
            assert baseline.get("current_fractal_type") == "explaino_nova"

            warped = viewer.set_control_value(warp_control, 0.45, timeout_seconds=20.0)
            warped_hash = _require_frame_hash(warped)
            assert warped.get("current_fractal_type") == "explaino_nova"
            assert warped.get("set_value_consumed") is True
            assert warped_hash != baseline_hash

            damped = viewer.set_control_value(damping_control, 0.35, timeout_seconds=20.0)
            damped_hash = _require_frame_hash(damped)
            assert damped.get("current_fractal_type") == "explaino_nova"
            assert damped.get("set_value_consumed") is True
            assert damped_hash != warped_hash
            assert viewer.launch_count == 1
