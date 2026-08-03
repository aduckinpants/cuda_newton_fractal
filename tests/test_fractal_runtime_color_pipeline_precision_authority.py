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


def _balance_void_parameter(state: dict[str, object]) -> dict[str, object]:
    draft = state.get("color_pipeline_draft")
    assert isinstance(draft, dict)
    lanes = draft.get("lanes")
    assert isinstance(lanes, list)
    for lane in lanes:
        if not isinstance(lane, dict) or lane.get("lane_id") != "grading":
            continue
        rows = lane.get("rows")
        assert isinstance(rows, list) and len(rows) == 1
        row = rows[0]
        assert isinstance(row, dict) and row.get("function_id") == "balance_void_grade"
        values = row.get("parameter_values")
        assert isinstance(values, list)
        for value in values:
            if isinstance(value, dict) and value.get("path") == "grade.balance_void":
                return value
    raise AssertionError("missing balance_void_grade grade.balance_void in captured draft")


def _authoring_state(state: dict[str, object]) -> dict[str, object]:
    result = copy.deepcopy(state)
    result.pop("stats", None)
    return result


def test_published_runtime_normalizes_color_pipeline_float_once_and_replays(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Color Pipeline precision authority regression is Windows-only")

    exe_path = active_runtime_exe()
    base = run_headless_capture(
        str(exe_path),
        "--fractal-type",
        "mandelbrot",
        "--width",
        "160",
        "--height",
        "120",
        "--color-pipeline-action",
        "select_function:grading:0:balance_void_grade",
        "--color-pipeline-action",
        "set_param:grading:0:grade.balance_void:number:0.0",
        "--capture-diagnostic",
    )

    requested = 0.123456789012345
    expected_runtime = _float32(requested)
    draft_only_state = copy.deepcopy(base["state"])
    assert isinstance(draft_only_state, dict)
    _balance_void_parameter(draft_only_state)["number_value"] = requested
    draft_only_path = write_state_bundle(tmp_path / "draft_only", draft_only_state)

    ordinary = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(draft_only_path),
        "--capture-diagnostic",
    )
    assert _balance_void_parameter(ordinary["state"])["number_value"] == requested
    assert ordinary["state"]["params"]["color_balance_void"] == 0.0
    assert ordinary["frame_hash"] == base["frame_hash"]

    applied = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(draft_only_path),
        "--apply-loaded-color-pipeline-draft",
        "--capture-diagnostic",
    )
    assert applied["state"]["params"]["color_balance_void"] == expected_runtime
    assert _balance_void_parameter(applied["state"])["number_value"] == expected_runtime
    assert _balance_void_parameter(applied["state"])["number_value"] != requested
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
