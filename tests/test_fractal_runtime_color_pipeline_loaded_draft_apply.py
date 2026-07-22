from __future__ import annotations

import copy
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


def _grading_saturation_parameter(state: dict[str, object]) -> dict[str, object]:
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
        assert isinstance(row, dict) and row.get("function_id") == "neutral_finish"
        values = row.get("parameter_values")
        assert isinstance(values, list)
        for value in values:
            if isinstance(value, dict) and value.get("path") == "grade.saturation":
                return value
    raise AssertionError("missing neutral_finish grade.saturation in captured draft")


def test_explicit_loaded_draft_application_materializes_and_replays(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("loaded Color Pipeline draft runtime regression is Windows-only")

    exe_path = active_runtime_exe()
    base = run_headless_capture(
        str(exe_path),
        "--fractal-type",
        "explaino_multibrot_root_trap",
        "--width",
        "160",
        "--height",
        "120",
        "--color-pipeline-action",
        "select_function:source:0:banded_signal",
        "--color-pipeline-action",
        "select_function:palette:0:banded_heatmap",
        "--color-pipeline-action",
        "select_function:grading:0:neutral_finish",
        "--color-pipeline-action",
        "set_param:grading:0:grade.saturation:number:1.0",
        "--capture-diagnostic",
    )

    draft_only_state = copy.deepcopy(base["state"])
    assert isinstance(draft_only_state, dict)
    saturation = _grading_saturation_parameter(draft_only_state)
    saturation["number_value"] = 0.25
    draft_only_path = write_state_bundle(tmp_path / "draft_only", draft_only_state)

    ordinary_load = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(draft_only_path),
        "--capture-diagnostic",
    )
    ordinary_params = ordinary_load["state"]["params"]
    assert isinstance(ordinary_params, dict)
    assert ordinary_params["color_saturation"] == pytest.approx(1.0, abs=1e-6)
    assert _grading_saturation_parameter(ordinary_load["state"])["number_value"] == pytest.approx(0.25, abs=1e-6)
    assert ordinary_load["frame_hash"] == base["frame_hash"], (
        "ordinary state loading must preserve a draft without implicitly applying it"
    )

    applied = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(draft_only_path),
        "--apply-loaded-color-pipeline-draft",
        "--capture-diagnostic",
    )
    applied_params = applied["state"]["params"]
    assert isinstance(applied_params, dict)
    assert applied_params["color_saturation"] == pytest.approx(0.25, abs=1e-6)
    grading_stack = applied_params.get("color_grading_stack")
    assert isinstance(grading_stack, list) and len(grading_stack) == 1
    assert grading_stack[0]["saturation"] == pytest.approx(0.25, abs=1e-6)
    assert applied["frame_hash"] != base["frame_hash"], (
        "explicit draft application must change decoded capture bytes for this controlled fixture"
    )

    replay_path = write_state_bundle(tmp_path / "replay", applied["state"])
    replay = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(replay_path),
        "--capture-diagnostic",
    )
    assert replay["state"]["params"] == applied["state"]["params"]
    assert replay["state"]["color_pipeline_draft"] == applied["state"]["color_pipeline_draft"]
    assert replay["frame_hash"] == applied["frame_hash"]
