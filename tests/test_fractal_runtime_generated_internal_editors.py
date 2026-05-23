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


def _capture_state(exe_path: Path, fractal_type: str) -> dict[str, object]:
    capture = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        fractal_type,
        "--width",
        "320",
        "--height",
        "240",
    )
    return json.loads(json.dumps(capture["state"]))


def _require_frame_hash(payload: dict[str, object]) -> str:
    frame_hash = payload.get("rendered_frame_hash")
    assert isinstance(frame_hash, str) and frame_hash.startswith("fnv1a64:"), payload
    return frame_hash


def test_explaino_custom_root_editor_changes_live_output_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("viewer automation is Windows-only")

    exe_path = active_runtime_exe()
    state = _capture_state(exe_path, "explaino")
    state["params"]["max_iter"] = 180
    state["params"]["explaino_root_authority"] = "custom"
    state["params"]["explaino_root_count"] = 4
    state["params"]["explaino_roots"] = [
        {"x": 1.20, "y": 0.10},
        {"x": 0.10, "y": 1.20},
        {"x": -1.15, "y": -0.05},
        {"x": 0.05, "y": -0.85},
    ]
    state_path = write_state_bundle(tmp_path / "generated_internal_editors", state)

    with runtime_automation_lock(timeout_seconds=20.0):
        with PersistentRuntimeViewerAutomation(
            exe_path=exe_path,
            state_path=state_path,
            report_path=tmp_path / "generated_internal_editors_report.json",
            command_path=tmp_path / "generated_internal_editors_command.json",
        ) as viewer:
            baseline = viewer.wait_for_report(timeout_seconds=20.0)
            baseline_hash = _require_frame_hash(baseline)
            assert baseline.get("current_fractal_type") == "explaino"

            viewer.wait_for_control("fractal_control.explaino_root_authority.primary", timeout_seconds=20.0)

            root_x_control = "fractal_control.explaino_root_0_x.primary"
            viewer.wait_for_control(root_x_control, timeout_seconds=20.0)
            root_x_edited = viewer.set_control_value(root_x_control, 1.65, timeout_seconds=20.0)
            root_x_hash = _require_frame_hash(root_x_edited)
            assert root_x_edited.get("current_fractal_type") == "explaino"
            assert root_x_edited.get("set_value_consumed") is True
            assert root_x_hash != baseline_hash

            root_count_control = "fractal_control.explaino_custom_root_count.primary"
            viewer.wait_for_control(root_count_control, timeout_seconds=20.0)
            root_count_edited = viewer.set_control_value(root_count_control, 3.0, timeout_seconds=20.0)
            root_count_hash = _require_frame_hash(root_count_edited)
            assert root_count_edited.get("current_fractal_type") == "explaino"
            assert root_count_edited.get("set_value_consumed") is True
            assert root_count_hash != root_x_hash
