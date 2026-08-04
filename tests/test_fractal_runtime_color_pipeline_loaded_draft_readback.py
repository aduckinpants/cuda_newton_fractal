from __future__ import annotations

import copy
import struct
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


def _float32(value: float) -> float:
    return struct.unpack("<f", struct.pack("<f", value))[0]


def _lane_row(state: dict[str, object], lane_id: str, function_id: str) -> dict[str, object]:
    draft = state.get("color_pipeline_draft")
    assert isinstance(draft, dict)
    lanes = draft.get("lanes")
    assert isinstance(lanes, list)
    for lane in lanes:
        if not isinstance(lane, dict) or lane.get("lane_id") != lane_id:
            continue
        rows = lane.get("rows")
        assert isinstance(rows, list) and len(rows) == 1
        row = rows[0]
        assert isinstance(row, dict) and row.get("function_id") == function_id
        return row
    raise AssertionError(f"missing {lane_id} row {function_id}")


def _number_parameter(row: dict[str, object], path: str) -> dict[str, object]:
    values = row.get("parameter_values")
    assert isinstance(values, list)
    for value in values:
        if isinstance(value, dict) and value.get("path") == path:
            return value
    raise AssertionError(f"missing parameter {path}")


def _authoring_state(state: dict[str, object]) -> dict[str, object]:
    result = copy.deepcopy(state)
    result.pop("stats", None)
    return result


def test_published_runtime_applies_fixture_f_four_lane_draft_and_replays(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("loaded Color Pipeline draft runtime regression is Windows-only")

    exe_path = active_runtime_exe()
    base = run_headless_capture(
        str(exe_path),
        "--fractal-type",
        "explaino_balance_void",
        "--width",
        "160",
        "--height",
        "120",
        "--color-pipeline-action",
        "select_function:source:0:banded_signal",
        "--color-pipeline-action",
        "select_function:shape:0:offset_scale",
        "--color-pipeline-action",
        "set_param:shape:0:shape.offset:number:0.30939",
        "--color-pipeline-action",
        "set_param:shape:0:shape.scale:number:4.46989",
        "--color-pipeline-action",
        "select_function:palette:0:banded_heatmap",
        "--color-pipeline-action",
        "set_param:palette:0:palette.band_emphasis:number:1.5",
        "--color-pipeline-action",
        "select_function:grading:0:balance_void_grade",
        "--color-pipeline-action",
        "set_param:grading:0:grade.balance_void:number:0.5",
        "--color-pipeline-action",
        "set_param:grading:0:grade.chroma_tension:number:-0.3",
        "--color-pipeline-action",
        "set_param:grading:0:grade.accent_bias:number:-0.67403",
        "--capture-diagnostic",
    )

    assert _lane_row(base["state"], "source", "banded_signal")
    assert _lane_row(base["state"], "shape", "offset_scale")
    palette_row = _lane_row(base["state"], "palette", "banded_heatmap")
    assert _lane_row(base["state"], "grading", "balance_void_grade")
    assert _number_parameter(palette_row, "palette.band_emphasis")["number_value"] == pytest.approx(1.5)

    requested = 1.8
    expected_runtime = _float32(requested)
    draft_only_state = copy.deepcopy(base["state"])
    assert isinstance(draft_only_state, dict)
    draft_palette = _lane_row(draft_only_state, "palette", "banded_heatmap")
    _number_parameter(draft_palette, "palette.band_emphasis")["number_value"] = requested
    draft_only_path = write_state_bundle(tmp_path / "draft_only", draft_only_state)

    ordinary = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(draft_only_path),
        "--capture-diagnostic",
    )
    assert ordinary["state"]["params"]["color_iteration_band_emphasis"] == pytest.approx(1.5)
    ordinary_palette = _lane_row(ordinary["state"], "palette", "banded_heatmap")
    assert _number_parameter(ordinary_palette, "palette.band_emphasis")["number_value"] == requested
    assert ordinary["frame_hash"] == base["frame_hash"]

    applied = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(draft_only_path),
        "--apply-loaded-color-pipeline-draft",
        "--capture-diagnostic",
    )
    applied_params = applied["state"]["params"]
    assert applied_params["color_iteration_band_emphasis"] == expected_runtime
    palette_stack = applied_params.get("color_palette_stack")
    assert isinstance(palette_stack, list) and len(palette_stack) == 1
    assert palette_stack[0]["band_emphasis"] == expected_runtime
    applied_palette = _lane_row(applied["state"], "palette", "banded_heatmap")
    assert _number_parameter(applied_palette, "palette.band_emphasis")["number_value"] == expected_runtime
    assert applied["frame_hash"] != base["frame_hash"]

    replay_path = write_state_bundle(tmp_path / "replay", applied["state"])
    replay = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(replay_path),
        "--capture-diagnostic",
    )
    assert _authoring_state(replay["state"]) == _authoring_state(applied["state"])
    assert replay["frame_hash"] == applied["frame_hash"]
