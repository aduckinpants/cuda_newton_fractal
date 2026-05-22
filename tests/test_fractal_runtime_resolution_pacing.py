from __future__ import annotations

import json
import sys
from pathlib import Path

import pytest

from tests.runtime_harness import (
    PersistentRuntimeViewerAutomation,
    active_runtime_exe,
    run_headless_capture,
    write_state_bundle,
)


def _configure_resolution_state(state: dict[str, object]) -> None:
    render = state["render"]
    assert isinstance(render, dict)
    render["width"] = 640
    render["height"] = 480
    render["interaction_debounce_ms"] = 0
    render["preview_target_fps"] = 30.0
    render["preview_min_scale"] = 0.5


def _assert_rendered_resolution(payload: dict[str, object], width: int, height: int) -> None:
    assert payload.get("rendered_frame_ready") is True, payload
    assert payload.get("rendered_frame_width") == width, payload
    assert payload.get("rendered_frame_height") == height, payload
    frame_hash = payload.get("rendered_frame_hash")
    assert isinstance(frame_hash, str) and frame_hash.startswith("fnv1a64:"), payload


def test_resolution_aspect_and_long_edge_controls_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("persistent runtime viewer harness is Windows-only")

    exe_path = active_runtime_exe()
    neutral_capture = run_headless_capture(
        str(exe_path), "--capture-diagnostic", "--fractal-type", "explaino_all", "--width", "320", "--height", "240"
    )
    state = json.loads(json.dumps(neutral_capture["state"]))
    _configure_resolution_state(state)
    state_path = write_state_bundle(tmp_path / "resolution_pacing", state)

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "resolution_pacing_report.json",
        command_path=tmp_path / "resolution_pacing_command.json",
    ) as viewer:
        viewer.wait_for_control("fractal_control.resolution_aspect_preset.primary", timeout_seconds=15.0)
        viewer.wait_for_control("fractal_control.resolution_long_edge.primary", timeout_seconds=15.0)
        _assert_rendered_resolution(viewer.wait_for_report(timeout_seconds=15.0), 640, 480)

        aspect_payload = viewer.set_enum_id(
            "fractal.render.resolution.aspect_preset",
            "16:9",
            expected_fractal_type="explaino_all",
            timeout_seconds=15.0,
        )
        _assert_rendered_resolution(aspect_payload, 640, 360)

        long_edge_payload = viewer.set_control_value(
            "fractal_control.resolution_long_edge.primary",
            800,
            timeout_seconds=15.0,
        )
        _assert_rendered_resolution(long_edge_payload, 800, 450)

        square_aspect_payload = viewer.set_enum_id(
            "fractal.render.resolution.aspect_preset",
            "1:1",
            expected_fractal_type="explaino_all",
            timeout_seconds=15.0,
        )
        _assert_rendered_resolution(square_aspect_payload, 800, 800)

        square_long_edge_payload = viewer.set_control_value(
            "fractal_control.resolution_long_edge.primary",
            512,
            timeout_seconds=15.0,
        )
        _assert_rendered_resolution(square_long_edge_payload, 512, 512)

        assert viewer.launch_count == 1
        assert aspect_payload.get("ui_automation_command_sequence") == 1
        assert long_edge_payload.get("ui_automation_command_sequence") == 2
        assert square_aspect_payload.get("ui_automation_command_sequence") == 3
        assert square_long_edge_payload.get("ui_automation_command_sequence") == 4
