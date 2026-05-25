from __future__ import annotations

import json
import sys
from pathlib import Path

import pytest

from tests.runtime_harness import (
    active_runtime_exe,
    run_headless_capture,
    runtime_automation_lock,
    write_state_bundle,
)


SDF_SOURCE_ROWS = (
    ("sdf_signed_distance", "smooth_escape", "cyclic_escape", 0.05, 0.5, 0.65, 0.2),
    ("sdf_inside_outside", "smooth_escape", "cyclic_escape", 1.0, 0.0, -1.0, 0.5),
    ("sdf_boundary_band", "smooth_escape", "cyclic_escape", 1.0, 0.0, 1.75, -0.25),
    ("sdf_normal_angle", "phase", "phase_wheel", 1.0, 0.0, -0.75, 0.35),
    ("sdf_curvature", "smooth_escape", "cyclic_escape", 0.25, 0.5, 1.25, -0.35),
)


@pytest.fixture(autouse=True)
def _serialize_runtime_automation():
    with runtime_automation_lock():
        yield


def _first_color_pipeline_row(state: dict[str, object], lane_id: str) -> dict[str, object]:
    draft = state.get("color_pipeline_draft")
    assert isinstance(draft, dict), "expected captured state to include color_pipeline_draft"
    lanes = draft.get("lanes")
    assert isinstance(lanes, list), "expected color_pipeline_draft.lanes to be a list"
    for lane in lanes:
        if not isinstance(lane, dict) or lane.get("lane_id") != lane_id:
            continue
        rows = lane.get("rows")
        assert isinstance(rows, list) and rows, f"expected lane {lane_id} to include rows"
        row = rows[0]
        assert isinstance(row, dict), f"expected lane {lane_id} first row to be an object"
        return row
    raise AssertionError(f"missing color pipeline lane: {lane_id}")


def _row_number(row: dict[str, object], path: str) -> float:
    params = row.get("parameter_values")
    assert isinstance(params, list), "expected parameter_values to be a list"
    for param in params:
        if not isinstance(param, dict) or param.get("path") != path:
            continue
        value = param.get("number_value")
        assert isinstance(value, (int, float)), f"expected numeric value for {path}"
        return float(value)
    raise AssertionError(f"missing color pipeline row param: {path}")


def _capture_sdf_source_row(
    *,
    exe_path: Path,
    state_path: Path,
    function_id: str,
    scale: float,
    bias: float,
) -> dict[str, object]:
    return run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--color-pipeline-action",
        f"select_function:source:0:{function_id}",
        "--color-pipeline-action",
        f"set_param:source:0:signal.scale:number:{scale}",
        "--color-pipeline-action",
        f"set_param:source:0:signal.bias:number:{bias}",
        "--capture-diagnostic",
    )


def _assert_sdf_capture_state(
    capture: dict[str, object],
    *,
    function_id: str,
    expected_mode: str,
    expected_palette: str,
    scale: float,
    bias: float,
) -> None:
    state = capture["state"]
    assert isinstance(state, dict)
    params = state.get("params")
    assert isinstance(params, dict)
    assert params.get("coloring_mode") == expected_mode
    assert params.get("color_signal") == function_id
    assert params.get("color_palette") == expected_palette

    source_stack = params.get("color_source_stack")
    assert isinstance(source_stack, list) and len(source_stack) == 1
    source_entry = source_stack[0]
    assert isinstance(source_entry, dict)
    assert source_entry.get("signal") == function_id
    assert source_entry.get("scale") == pytest.approx(scale, abs=1e-6)
    assert source_entry.get("bias") == pytest.approx(bias, abs=1e-6)

    source_row = _first_color_pipeline_row(state, "source")
    assert source_row.get("function_id") == function_id
    assert _row_number(source_row, "signal.scale") == pytest.approx(scale, abs=1e-6)
    assert _row_number(source_row, "signal.bias") == pytest.approx(bias, abs=1e-6)


def test_color_pipeline_sdf_source_rows_are_live_backed_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Color Pipeline SDF runtime regression is Windows-only")

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
    neutral_state = json.loads(json.dumps(neutral_capture["state"]))
    state_path = write_state_bundle(tmp_path / "color_pipeline_sdf_rows", neutral_state)

    for function_id, expected_mode, expected_palette, baseline_scale, baseline_bias, edited_scale, edited_bias in SDF_SOURCE_ROWS:
        baseline = _capture_sdf_source_row(
            exe_path=exe_path,
            state_path=state_path,
            function_id=function_id,
            scale=baseline_scale,
            bias=baseline_bias,
        )
        edited = _capture_sdf_source_row(
            exe_path=exe_path,
            state_path=state_path,
            function_id=function_id,
            scale=edited_scale,
            bias=edited_bias,
        )

        _assert_sdf_capture_state(
            baseline,
            function_id=function_id,
            expected_mode=expected_mode,
            expected_palette=expected_palette,
            scale=baseline_scale,
            bias=baseline_bias,
        )
        _assert_sdf_capture_state(
            edited,
            function_id=function_id,
            expected_mode=expected_mode,
            expected_palette=expected_palette,
            scale=edited_scale,
            bias=edited_bias,
        )
        assert baseline["frame_hash"] != neutral_capture["frame_hash"], (
            f"expected {function_id} to use the live Lens SDF field and change the base frame"
        )
        assert edited["frame_hash"] != baseline["frame_hash"], (
            f"expected {function_id} signal.scale/signal.bias edits to change the live rendered frame"
        )
