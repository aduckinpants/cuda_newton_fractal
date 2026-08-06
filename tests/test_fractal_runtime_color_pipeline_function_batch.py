from __future__ import annotations

import sys
from pathlib import Path

import pytest

from tests.runtime_harness import (
    active_runtime_exe,
    run_headless_capture,
    runtime_automation_lock,
    write_state_bundle,
)


@pytest.fixture(autouse=True)
def _serialize_runtime_automation():
    with runtime_automation_lock():
        yield


def _capture(exe_path: Path, *, fractal_type: str, actions: list[str]) -> dict[str, object]:
    args = [
        str(exe_path),
        "--fractal-type",
        fractal_type,
        "--width",
        "192",
        "--height",
        "144",
    ]
    for action in actions:
        args.extend(["--color-pipeline-action", action])
    args.append("--capture-diagnostic")
    return run_headless_capture(*args)


def _lane_function(state: dict[str, object], lane_id: str) -> str:
    draft = state.get("color_pipeline_draft")
    assert isinstance(draft, dict), state
    lanes = draft.get("lanes")
    assert isinstance(lanes, list), draft
    for lane in lanes:
        if not isinstance(lane, dict) or lane.get("lane_id") != lane_id:
            continue
        rows = lane.get("rows")
        assert isinstance(rows, list) and rows, lane
        row = rows[0]
        assert isinstance(row, dict), lane
        function_id = row.get("function_id")
        assert isinstance(function_id, str), row
        return function_id
    raise AssertionError(f"missing lane {lane_id}")


def test_low_risk_color_pipeline_function_batch_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Color Pipeline runtime function batch proof is Windows-only")

    exe_path = active_runtime_exe()
    cases = (
        (
            "invert_unit_v1",
            "mandelbrot",
            ["select_function:source:0:smooth_escape_ramp", "select_function:shape:0:identity", "select_function:palette:0:gradient_three_stop_v1"],
            ["select_function:source:0:smooth_escape_ramp", "select_function:shape:0:invert_unit_v1", "select_function:palette:0:gradient_three_stop_v1"],
            "shape",
        ),
        (
            "fold_centered_v1",
            "mandelbrot",
            ["select_function:source:0:smooth_escape_ramp", "select_function:shape:0:fold_centered_v1", "set_param:shape:0:shape.mix:number:0.0", "select_function:palette:0:gradient_three_stop_v1"],
            ["select_function:source:0:smooth_escape_ramp", "select_function:shape:0:fold_centered_v1", "set_param:shape:0:shape.mix:number:1.0", "select_function:palette:0:gradient_three_stop_v1"],
            "shape",
        ),
        (
            "phase_offset_v1",
            "mandelbrot",
            ["select_function:source:0:phase_orbit", "select_function:shape:0:phase_offset_v1", "set_param:shape:0:shape.offset_turns:number:0.0", "select_function:palette:0:phase_wheel_palette"],
            ["select_function:source:0:phase_orbit", "select_function:shape:0:phase_offset_v1", "set_param:shape:0:shape.offset_turns:number:0.25", "select_function:palette:0:phase_wheel_palette"],
            "shape",
        ),
        (
            "phase_repeat_v1",
            "mandelbrot",
            ["select_function:source:0:phase_orbit", "select_function:shape:0:phase_repeat_v1", "set_param:shape:0:shape.cycles:number:1.0", "select_function:palette:0:phase_wheel_palette"],
            ["select_function:source:0:phase_orbit", "select_function:shape:0:phase_repeat_v1", "set_param:shape:0:shape.cycles:number:3.0", "select_function:palette:0:phase_wheel_palette"],
            "shape",
        ),
        (
            "phase_mirror_v1",
            "mandelbrot",
            ["select_function:source:0:phase_orbit", "select_function:shape:0:phase_mirror_v1", "set_param:shape:0:shape.mix:number:0.0", "select_function:palette:0:phase_wheel_palette"],
            ["select_function:source:0:phase_orbit", "select_function:shape:0:phase_mirror_v1", "set_param:shape:0:shape.mix:number:1.0", "select_function:palette:0:phase_wheel_palette"],
            "shape",
        ),
        (
            "diverging_signed_palette_v1",
            "explaino_magnet_root_well",
            ["select_function:source:0:root_log_proximity_v1", "select_function:shape:0:identity", "select_function:palette:0:diverging_signed_palette_v1", "set_param:palette:0:palette.contrast:number:1.0"],
            ["select_function:source:0:root_log_proximity_v1", "select_function:shape:0:identity", "select_function:palette:0:diverging_signed_palette_v1", "set_param:palette:0:palette.contrast:number:3.0"],
            "palette",
        ),
        (
            "inside_outside_two_tone_v1",
            "mandelbrot",
            ["select_function:source:0:sdf_inside_outside", "select_function:shape:0:identity", "select_function:palette:0:inside_outside_two_tone_v1", "set_param:palette:0:palette.inside_r:number:0.95"],
            ["select_function:source:0:sdf_inside_outside", "select_function:shape:0:identity", "select_function:palette:0:inside_outside_two_tone_v1", "set_param:palette:0:palette.inside_r:number:0.15"],
            "palette",
        ),
        (
            "gradient_three_stop_v1",
            "mandelbrot",
            ["select_function:source:0:smooth_escape_ramp", "select_function:shape:0:identity", "select_function:palette:0:gradient_three_stop_v1", "set_param:palette:0:palette.midpoint:number:0.5"],
            ["select_function:source:0:smooth_escape_ramp", "select_function:shape:0:identity", "select_function:palette:0:gradient_three_stop_v1", "set_param:palette:0:palette.midpoint:number:0.2"],
            "palette",
        ),
        (
            "levels_gamma_v1",
            "mandelbrot",
            ["select_function:source:0:smooth_escape_ramp", "select_function:shape:0:identity", "select_function:palette:0:gradient_three_stop_v1", "select_function:grading:0:levels_gamma_v1", "set_param:grading:0:grade.gamma:number:1.0"],
            ["select_function:source:0:smooth_escape_ramp", "select_function:shape:0:identity", "select_function:palette:0:gradient_three_stop_v1", "select_function:grading:0:levels_gamma_v1", "set_param:grading:0:grade.gamma:number:2.0"],
            "grading",
        ),
        (
            "hue_rotate_v1",
            "mandelbrot",
            ["select_function:source:0:smooth_escape_ramp", "select_function:shape:0:identity", "select_function:palette:0:gradient_three_stop_v1", "select_function:grading:0:hue_rotate_v1", "set_param:grading:0:grade.hue_turns:number:0.0"],
            ["select_function:source:0:smooth_escape_ramp", "select_function:shape:0:identity", "select_function:palette:0:gradient_three_stop_v1", "select_function:grading:0:hue_rotate_v1", "set_param:grading:0:grade.hue_turns:number:0.2"],
            "grading",
        ),
    )

    for function_id, fractal_type, baseline_actions, changed_actions, owning_lane in cases:
        baseline = _capture(exe_path, fractal_type=fractal_type, actions=list(baseline_actions))
        changed = _capture(exe_path, fractal_type=fractal_type, actions=list(changed_actions))
        baseline_state = baseline.get("state")
        changed_state = changed.get("state")
        assert isinstance(baseline_state, dict) and isinstance(changed_state, dict), function_id
        assert baseline_state.get("fractal_type") == fractal_type, baseline_state
        assert changed_state.get("fractal_type") == fractal_type, changed_state
        assert _lane_function(changed_state, owning_lane) == function_id, changed_state
        assert baseline["frame_hash"] != changed["frame_hash"], (
            function_id,
            baseline["frame_hash"],
            changed["frame_hash"],
        )

        replay_path = write_state_bundle(tmp_path / function_id, changed_state)
        replay = run_headless_capture(
            str(exe_path),
            "--load-state-json",
            str(replay_path),
            "--capture-diagnostic",
        )
        assert replay["frame_hash"] == changed["frame_hash"], (function_id, changed, replay)
