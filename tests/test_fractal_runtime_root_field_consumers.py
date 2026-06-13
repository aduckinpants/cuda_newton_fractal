from __future__ import annotations

import json
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Any

import pytest

from tests.runtime_harness import (
    PersistentRuntimeViewerAutomation,
    RUNTIME_DIR,
    active_runtime_exe,
    run_headless_capture,
    runtime_automation_lock,
    write_state_bundle,
)


@pytest.fixture(autouse=True)
def _serialize_runtime_automation():
    with runtime_automation_lock():
        yield


def _require_frame_hash(payload: dict[str, Any]) -> str:
    frame_hash = payload.get("rendered_frame_hash")
    assert isinstance(frame_hash, str) and frame_hash.startswith("fnv1a64:"), payload
    return frame_hash


def _require_root_hash(payload: dict[str, Any]) -> str:
    root_hash = payload.get("root_field_consumer_base_root_hash")
    assert isinstance(root_hash, str) and root_hash.startswith("fnv1a64:"), payload
    return root_hash


def _state_for_lane(exe_path: Path, lane_id: str) -> dict[str, Any]:
    capture = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        lane_id,
        "--width",
        "256",
        "--height",
        "192",
    )
    state = json.loads(json.dumps(capture["state"]))
    assert state.get("fractal_type") == lane_id, state
    return state


def _color_pipeline_row(state: dict[str, Any], lane_id: str, row_index: int) -> dict[str, Any]:
    draft = state.get("color_pipeline_draft")
    assert isinstance(draft, dict), "expected captured state to include color_pipeline_draft"
    lanes = draft.get("lanes")
    assert isinstance(lanes, list), "expected color_pipeline_draft.lanes to be a list"
    for lane in lanes:
        if isinstance(lane, dict) and lane.get("lane_id") == lane_id:
            rows = lane.get("rows")
            assert isinstance(rows, list), f"expected {lane_id} rows to be a list"
            row = rows[row_index]
            assert isinstance(row, dict), f"expected {lane_id} row {row_index} to be an object"
            return row
    raise AssertionError(f"missing color pipeline lane {lane_id!r}")


def _color_pipeline_number_param(row: dict[str, Any], path: str) -> float:
    values = row.get("parameter_values")
    assert isinstance(values, list), row
    for value in values:
        if isinstance(value, dict) and value.get("path") == path:
            number = value.get("number_value")
            assert isinstance(number, (int, float)), value
            return float(number)
    raise AssertionError(f"missing color pipeline param {path!r} in row {row!r}")


@pytest.mark.skipif(sys.platform != "win32", reason="Windows-only viewer runtime")
@pytest.mark.parametrize(
    ("lane_id", "base_lane"),
    [
        ("explaino_mandelbrot_root_trap", "mandelbrot"),
        ("explaino_magnet_root_well", "magnet"),
    ],
)
def test_root_field_consumer_lanes_report_and_mutate_no_mouse(
    tmp_path: Path,
    lane_id: str,
    base_lane: str,
) -> None:
    exe_path = active_runtime_exe()
    state = _state_for_lane(exe_path, lane_id)
    state_path = write_state_bundle(tmp_path / lane_id, state)

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / f"{lane_id}_report.json",
        command_path=tmp_path / f"{lane_id}_command.json",
    ) as viewer:
        viewer.wait_for_control("fractal_control.explaino_root_field_trap_strength.primary", timeout_seconds=20.0)
        viewer.wait_for_control("fractal_control.explaino_root_field_trap_scale.primary", timeout_seconds=20.0)

        baseline = viewer.wait_for_report(timeout_seconds=60.0)
        assert baseline.get("current_fractal_type") == lane_id, baseline
        assert baseline.get("root_field_consumer_active") is True, baseline
        assert baseline.get("root_field_consumer_kind") == lane_id, baseline
        assert baseline.get("root_field_consumer_base_fractal_type") == base_lane, baseline
        assert baseline.get("root_field_consumer_root_count") == 4, baseline
        assert baseline.get("root_field_consumer_fail_closed_reason") is None, baseline
        baseline_hash = _require_frame_hash(baseline)
        baseline_root_hash = _require_root_hash(baseline)

        neutral = viewer.set_control_value(
            "fractal_control.explaino_root_field_trap_strength.primary",
            0.0,
            timeout_seconds=20.0,
        )
        assert neutral.get("current_fractal_type") == lane_id, neutral
        assert neutral.get("root_field_consumer_active") is True, neutral
        neutral_hash = _require_frame_hash(neutral)

        restored = viewer.set_control_value(
            "fractal_control.explaino_root_field_trap_strength.primary",
            1.0,
            timeout_seconds=20.0,
        )
        assert restored.get("current_fractal_type") == lane_id, restored
        assert _require_frame_hash(restored) != neutral_hash, restored

        scaled = viewer.set_control_value(
            "fractal_control.explaino_root_field_trap_scale.primary",
            2.0,
            timeout_seconds=20.0,
        )
        assert scaled.get("current_fractal_type") == lane_id, scaled
        assert _require_frame_hash(scaled) != neutral_hash, scaled

        regular = viewer.set_enum_id(
            "fractal.params.explaino_generated_root_layout",
            "regular_ngon_v1",
            expected_fractal_type=lane_id,
            timeout_seconds=20.0,
        )
        assert regular.get("root_field_consumer_root_layout_kind") == "regular_ngon_v1", regular
        viewer.wait_for_control("fractal_control.explaino_generated_root_count.primary", timeout_seconds=20.0)
        root_count = viewer.set_control_value(
            "fractal_control.explaino_generated_root_count.primary",
            5.0,
            timeout_seconds=20.0,
        )
        assert root_count.get("root_field_consumer_root_count") == 5, root_count
        assert _require_root_hash(root_count) != baseline_root_hash, root_count
        assert _require_frame_hash(root_count) != baseline_hash, root_count


@pytest.mark.skipif(sys.platform != "win32", reason="Windows-only viewer runtime")
def test_mandelbrot_root_trap_root_phase_source_runtime_actions_change_frame(
    tmp_path: Path,
) -> None:
    exe_path = active_runtime_exe()
    lane_id = "explaino_mandelbrot_root_trap"
    state = _state_for_lane(exe_path, lane_id)
    state_path = write_state_bundle(tmp_path / "root_phase_source", state)

    baseline = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--capture-diagnostic",
    )
    root_phase = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--color-pipeline-action",
        "select_function:source:0:root_phase",
        "--color-pipeline-action",
        "select_function:palette:0:phase_wheel_palette",
        "--capture-diagnostic",
    )
    shifted = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--color-pipeline-action",
        "select_function:source:0:root_phase",
        "--color-pipeline-action",
        "select_function:palette:0:phase_wheel_palette",
        "--color-pipeline-action",
        "set_param:source:0:signal.phase_offset:number:1.57079632679",
        "--capture-diagnostic",
    )

    root_phase_state = root_phase["state"]
    params = root_phase_state.get("params")
    assert isinstance(params, dict), root_phase_state
    assert root_phase_state.get("fractal_type") == lane_id
    assert params.get("color_signal") == "root_phase", params
    assert params.get("color_palette") == "phase_wheel", params
    assert params.get("coloring_mode") == "phase", params

    root_phase_source_row = _color_pipeline_row(root_phase_state, "source", 0)
    assert root_phase_source_row.get("function_id") == "root_phase", root_phase_source_row
    root_phase_palette_row = _color_pipeline_row(root_phase_state, "palette", 0)
    assert root_phase_palette_row.get("function_id") == "phase_wheel_palette", root_phase_palette_row

    shifted_source_row = _color_pipeline_row(shifted["state"], "source", 0)
    assert shifted_source_row.get("function_id") == "root_phase", shifted_source_row
    assert _color_pipeline_number_param(shifted_source_row, "signal.phase_offset") == pytest.approx(
        1.57079632679,
        abs=1.0e-6,
    )

    assert root_phase["frame_hash"] != baseline["frame_hash"]
    assert shifted["frame_hash"] != root_phase["frame_hash"]


@pytest.mark.skipif(sys.platform != "win32", reason="Windows-only viewer runtime")
@pytest.mark.parametrize(
    ("lane_id", "base_lane"),
    [
        ("explaino_mandelbrot_root_trap", "mandelbrot"),
        ("explaino_magnet_root_well", "magnet"),
    ],
)
def test_root_field_consumer_capture_finding_sidecar_and_replay(
    tmp_path: Path,
    lane_id: str,
    base_lane: str,
) -> None:
    exe_path = active_runtime_exe()
    state = _state_for_lane(exe_path, lane_id)
    state["params"]["explaino_generated_root_layout"] = "regular_ngon_v1"
    state["params"]["explaino_generated_root_count"] = 5
    state["params"]["explaino_root_field_trap_strength"] = 1.0
    state["params"]["explaino_root_field_trap_scale"] = 1.75
    state_path = write_state_bundle(tmp_path / f"{lane_id}_capture", state)

    group = f"pytest_root_field_consumer_{lane_id}_{tmp_path.name}"
    group_root = RUNTIME_DIR.parent / "findings" / group
    if group_root.exists():
        shutil.rmtree(group_root)
    result = subprocess.run(
        [
            str(exe_path),
            "--load-state-json",
            str(state_path),
            "--capture-finding",
            "--finding-group",
            group,
            "--finding-why",
            "root-field consumer runtime sidecar proof",
        ],
        cwd=str(exe_path.parent),
        text=True,
        capture_output=True,
        check=False,
    )
    assert result.returncode == 0, result.stdout + result.stderr
    state_paths = sorted(group_root.rglob("state.json"))
    assert len(state_paths) == 1, state_paths
    finding_dir = state_paths[0].parent
    finding_state_path = finding_dir / "state.json"
    fractal_state_path = finding_dir / "fractal-state.json"
    finding_json_path = finding_dir / "finding.json"
    assert finding_state_path.exists(), finding_dir
    assert fractal_state_path.exists(), finding_dir
    assert finding_json_path.exists(), finding_dir

    finding_json = json.loads(finding_json_path.read_text(encoding="utf-8"))
    assert finding_json.get("fractal_state_file") == "fractal-state.json", finding_json
    sidecar = json.loads(fractal_state_path.read_text(encoding="utf-8"))
    capture_context = sidecar.get("capture_context")
    assert isinstance(capture_context, dict), sidecar
    assert capture_context.get("fractal_type") == lane_id, capture_context
    assert capture_context.get("selected_fractal_type") == lane_id, capture_context
    active_controls = sidecar.get("active_fractal_controls")
    assert isinstance(active_controls, dict), sidecar
    assert active_controls.get("explaino_generated_root_layout") == "regular_ngon_v1", active_controls
    assert active_controls.get("explaino_generated_root_count") == 5, active_controls
    assert active_controls.get("explaino_root_field_trap_strength") == pytest.approx(1.0), active_controls
    assert active_controls.get("explaino_root_field_trap_scale") == pytest.approx(1.75), active_controls
    derived = sidecar.get("derived_runtime_values")
    assert isinstance(derived, dict), sidecar
    consumer = derived.get("root_field_consumer")
    assert isinstance(consumer, dict), derived
    assert consumer.get("consumer_kind") == lane_id, consumer
    assert consumer.get("base_fractal_type") == base_lane, consumer
    assert consumer.get("root_layout_kind") == "regular_ngon_v1", consumer
    assert consumer.get("requested_generated_root_count") == 5, consumer
    assert consumer.get("root_count") == 5, consumer
    assert consumer.get("trap_strength") == pytest.approx(1.0), consumer
    assert consumer.get("trap_scale") == pytest.approx(1.75), consumer
    assert isinstance(consumer.get("base_root_hash"), str), consumer
    assert isinstance(consumer.get("effective_root_hash"), str), consumer

    replay_a = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(finding_state_path),
        "--capture-diagnostic",
    )
    replay_b = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(finding_state_path),
        "--capture-diagnostic",
    )
    assert replay_b["frame_hash"] == replay_a["frame_hash"]
