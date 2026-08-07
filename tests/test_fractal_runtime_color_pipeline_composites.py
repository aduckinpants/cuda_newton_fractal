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


FROZEN_MANUAL_UNIT_CONTOURS_HASH = "571ab0012afe909fe874b98f73ea3f83905f5119c947f192e1e8fa90f7e378b1"


@pytest.fixture(autouse=True)
def _serialize_runtime_automation():
    with runtime_automation_lock():
        yield


def _capture_with_actions(exe_path: Path, actions: list[str]) -> dict[str, object]:
    args = [
        str(exe_path),
        "--fractal-type",
        "mandelbrot",
        "--width",
        "192",
        "--height",
        "144",
    ]
    for action in actions:
        args.extend(["--color-pipeline-action", action])
    args.append("--capture-diagnostic")
    return run_headless_capture(*args)


def _common_actions() -> list[str]:
    return [
        "select_function:source:0:smooth_escape_ramp",
        "select_function:palette:0:gradient_three_stop_v1",
        "select_function:grading:0:levels_gamma_v1",
    ]


def _manual_unit_contours_actions() -> list[str]:
    return _common_actions() + [
        "select_function:shape:0:repeat",
        "set_param:shape:0:shape.frequency:number:6.0",
        "set_param:shape:0:shape.phase:number:0.1",
        "add_row:shape:smooth_window",
        "set_param:shape:1:shape.center:number:0.5",
        "set_param:shape:1:shape.width:number:0.35",
        "set_param:shape:1:shape.softness:number:0.08",
    ]


def _composite_unit_contours_actions() -> list[str]:
    return _common_actions() + [
        "select_function:shape:0:unit_contours_v1",
        "set_param:shape:0:composite.frequency:number:6.0",
        "set_param:shape:0:composite.offset:number:0.1",
        "set_param:shape:0:composite.window_width:number:0.35",
        "set_param:shape:0:composite.softness:number:0.08",
    ]


def _lane_rows(lanes: object, lane_id: str) -> list[dict[str, object]]:
    assert isinstance(lanes, list), lanes
    for lane in lanes:
        if isinstance(lane, dict) and lane.get("lane_id") == lane_id:
            rows = lane.get("rows")
            assert isinstance(rows, list), lane
            return [row for row in rows if isinstance(row, dict)]
    raise AssertionError(f"missing lane {lane_id!r}: {lanes!r}")


def test_unit_contours_composite_public_paths_match_exact_manual_stack(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Color Pipeline composite runtime proof is Windows-only")

    exe_path = active_runtime_exe()
    manual = _capture_with_actions(exe_path, _manual_unit_contours_actions())
    composite = _capture_with_actions(exe_path, _composite_unit_contours_actions())

    assert manual["frame_hash"] == FROZEN_MANUAL_UNIT_CONTOURS_HASH, manual
    assert composite["frame_hash"] == manual["frame_hash"], (manual, composite)

    composite_state = composite.get("state")
    assert isinstance(composite_state, dict), composite
    expanded_shape_rows = _lane_rows(
        composite_state.get("color_pipeline_draft", {}).get("lanes"),
        "shape",
    )
    assert [row.get("function_id") for row in expanded_shape_rows] == [
        "repeat",
        "smooth_window",
    ]
    projection = composite_state.get("color_pipeline_composite_projection")
    assert isinstance(projection, dict), composite_state
    assert projection.get("schema_id") == "viewer.color_pipeline_composite_projection.v1"
    projection_rows = projection.get("items")
    assert isinstance(projection_rows, list) and len(projection_rows) == 1, projection
    assert projection_rows[0].get("composite_id") == "unit_contours_v1"

    replay_state_path = write_state_bundle(
        tmp_path / "composite_replay",
        json.loads(json.dumps(composite_state)),
    )
    replay = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(replay_state_path),
        "--capture-diagnostic",
    )
    assert replay["frame_hash"] == composite["frame_hash"], (composite, replay)

    manual_state = manual.get("state")
    assert isinstance(manual_state, dict), manual
    manual_state_path = write_state_bundle(
        tmp_path / "manual_ui_reference",
        json.loads(json.dumps(manual_state)),
    )
    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=manual_state_path,
        report_path=tmp_path / "manual_ui_report.json",
        command_path=tmp_path / "manual_ui_command.json",
        open_color_pipeline=True,
    ) as manual_viewer:
        manual_viewer.wait_for_report(timeout_seconds=30.0)
        manual_ui_settled = manual_viewer.click_control("render_once", timeout_seconds=60.0)
        manual_ui_hash = manual_ui_settled.get("rendered_frame_hash")
        assert isinstance(manual_ui_hash, str) and manual_ui_hash.startswith("fnv1a64:"), manual_ui_settled

    neutral = _capture_with_actions(
        exe_path,
        _common_actions() + ["select_function:shape:0:identity"],
    )
    neutral_state = neutral.get("state")
    assert isinstance(neutral_state, dict), neutral
    neutral_state_path = write_state_bundle(
        tmp_path / "public_ui_seed",
        json.loads(json.dumps(neutral_state)),
    )
    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=neutral_state_path,
        report_path=tmp_path / "public_ui_report.json",
        command_path=tmp_path / "public_ui_command.json",
        open_color_pipeline=True,
    ) as viewer:
        ready = viewer.wait_for_report(timeout_seconds=30.0)
        controls = ready.get("controls")
        assert isinstance(controls, list), ready
        picker_ids = [
            item.get("control_id")
            for item in controls
            if isinstance(item, dict)
            and isinstance(item.get("control_id"), str)
            and item["control_id"].startswith("color_pipeline.shape.")
            and item["control_id"].endswith(".function")
        ]
        assert len(picker_ids) == 1, picker_ids
        selected = viewer.click_control(
            f"{picker_ids[0]}.unit_contours_v1.select",
            timeout_seconds=60.0,
        )
        assert selected.get("click_consumed") is True, selected

        default_settled = viewer.click_control("render_once", timeout_seconds=60.0)
        default_hash = default_settled.get("rendered_frame_hash")
        assert default_hash == manual_ui_hash, default_settled

        param_controls = {
            "color_pipeline.shape.unit_contours_v1.composite.frequency.primary": (7.0, 6.0),
            "color_pipeline.shape.unit_contours_v1.composite.offset.primary": (0.2, 0.1),
            "color_pipeline.shape.unit_contours_v1.composite.window_width.primary": (0.5, 0.35),
            "color_pipeline.shape.unit_contours_v1.composite.softness.primary": (0.15, 0.08),
        }
        for control_id, (probe_value, restore_value) in param_controls.items():
            probed = viewer.set_control_value(control_id, probe_value, timeout_seconds=60.0)
            assert probed.get("set_value_consumed") is True, probed
            restored = viewer.set_control_value(control_id, restore_value, timeout_seconds=60.0)
            assert restored.get("set_value_consumed") is True, restored

        settled = viewer.click_control("render_once", timeout_seconds=60.0)
        assert settled.get("rendered_frame_hash") == manual_ui_hash, settled

    composite_report = settled.get("color_pipeline_composite_application_report")
    assert isinstance(composite_report, dict), settled
    active_execution = composite_report.get("active_execution")
    draft_projection = composite_report.get("draft_projection")
    committed_receipt = composite_report.get("composite_receipt")
    assert isinstance(active_execution, dict), composite_report
    assert isinstance(draft_projection, dict), composite_report
    assert isinstance(committed_receipt, dict), composite_report
    assert active_execution.get("authority") == "expanded_primitive_rows"
    assert [row.get("function_id") for row in _lane_rows(active_execution.get("rows"), "shape")] == [
        "repeat",
        "smooth_window",
    ]
    assert draft_projection.get("authority") == "composite_wrapper"
    assert draft_projection.get("status") == "ready"
    assert [row.get("function_id") for row in _lane_rows(draft_projection.get("wrapper_rows"), "shape")] == [
        "unit_contours_v1",
    ]
    assert committed_receipt.get("application_status") == "committed"
    assert committed_receipt.get("fail_closed_reason") is None
