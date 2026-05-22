from __future__ import annotations

import json
import math
import sys
import time
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


def _configure_slow_camera_pacing_state(state: dict[str, object]) -> None:
    state["fractal_type"] = "multibrot"
    view = state["view"]
    assert isinstance(view, dict)
    view.update(
        {
            "center_x": -0.15000000596046448,
            "center_y": 0.75,
            "center_hp_x": -0.15000000596046448,
            "center_hp_y": 0.75,
            "zoom": 4.5,
            "log2_zoom": math.log2(4.5),
            "auto_max_iter": False,
        }
    )
    params = state["params"]
    assert isinstance(params, dict)
    params.update(
        {
            "max_iter": 2000,
            "coloring_mode": "smooth_escape",
            "color_signal": "smooth_escape",
            "color_shape": "identity",
            "color_palette": "cyclic_escape",
            "color_grading": "escape_default",
            "multibrot_power_float": 2.0,
            "multibrot_power_imag": 0.0,
        }
    )
    render = state["render"]
    assert isinstance(render, dict)
    render.update(
        {
            "width": 1280,
            "height": 960,
            "interaction_debounce_ms": 800,
            "preview_target_fps": 240.0,
            "preview_min_scale": 0.5,
            "sample_tier": "standard",
        }
    )


def _configure_fast_moderate_f32_pacing_state(state: dict[str, object]) -> None:
    state["fractal_type"] = "multibrot"
    view = state["view"]
    assert isinstance(view, dict)
    view.update(
        {
            "center_x": -0.5,
            "center_y": 0.0,
            "center_hp_x": -0.5,
            "center_hp_y": 0.0,
            "zoom": 1.0,
            "log2_zoom": 0.0,
            "auto_max_iter": False,
        }
    )
    params = state["params"]
    assert isinstance(params, dict)
    params.update(
        {
            "max_iter": 868,
            "coloring_mode": "smooth_escape",
            "color_signal": "smooth_escape",
            "color_shape": "identity",
            "color_palette": "cyclic_escape",
            "color_grading": "escape_default",
            "multibrot_power_float": 2.0,
            "multibrot_power_imag": 0.0,
        }
    )
    render = state["render"]
    assert isinstance(render, dict)
    render.update(
        {
            "width": 4096,
            "height": 4096,
            "interaction_debounce_ms": 800,
            "preview_target_fps": 30.0,
            "preview_min_scale": 0.5,
            "sample_tier": "fast",
        }
    )


def _wait_for_pacing_payload(viewer: PersistentRuntimeViewerAutomation, predicate, *, timeout_seconds: float) -> dict[str, object]:
    deadline = time.monotonic() + timeout_seconds
    last_payload: dict[str, object] | None = None
    while time.monotonic() < deadline:
        if viewer.proc is not None and viewer.proc.poll() is not None:
            raise AssertionError(f"viewer exited while waiting for pacing report; returncode={viewer.proc.returncode}")
        payload = viewer._load_report()
        if payload is not None:
            last_payload = payload
            if payload.get("rendered_frame_ready") is True and predicate(payload):
                return payload
        time.sleep(0.05)
    raise AssertionError(f"pacing report predicate was not satisfied; last_payload={last_payload!r}")


def _assert_pacing_report_fields(payload: dict[str, object], *, target_width: int = 1280, target_height: int = 960) -> None:
    assert payload.get("target_render_width") == target_width, payload
    assert payload.get("target_render_height") == target_height, payload
    assert isinstance(payload.get("last_render_ms"), (int, float)), payload
    assert isinstance(payload.get("last_render_fps"), (int, float)), payload
    assert "render_pacing_preview_active" in payload, payload
    assert isinstance(payload.get("render_pacing_preview_scale"), (int, float)), payload
    assert isinstance(payload.get("render_pacing_render_width"), int), payload
    assert isinstance(payload.get("render_pacing_render_height"), int), payload


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


def test_camera_center_edits_enter_preview_when_measured_frames_are_slow_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("persistent runtime viewer harness is Windows-only")

    exe_path = active_runtime_exe()
    neutral_capture = run_headless_capture(
        str(exe_path), "--capture-diagnostic", "--fractal-type", "multibrot", "--width", "320", "--height", "240"
    )
    state = json.loads(json.dumps(neutral_capture["state"]))
    _configure_slow_camera_pacing_state(state)
    state_path = write_state_bundle(tmp_path / "camera_pacing", state)

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "camera_pacing_report.json",
        command_path=tmp_path / "camera_pacing_command.json",
    ) as viewer:
        viewer.wait_for_control("fractal_control.center_x.primary", timeout_seconds=20.0)
        viewer.wait_for_control("fractal_control.center_y.primary", timeout_seconds=20.0)
        baseline = viewer.wait_for_report(timeout_seconds=30.0)
        _assert_pacing_report_fields(baseline)
        assert baseline.get("render_pacing_preview_active") is False, baseline
        slow_threshold_ms = (1000.0 / 240.0) * 1.10
        assert float(baseline["last_render_ms"]) > slow_threshold_ms, (
            "pacing runtime proof workload did not produce a measured slow full-resolution frame",
            baseline,
        )

        center_x_payload = viewer.set_control_value(
            "fractal_control.center_x.primary",
            -0.14500000596046448,
            timeout_seconds=45.0,
        )
        _assert_pacing_report_fields(center_x_payload)
        assert center_x_payload.get("ui_automation_command_sequence") == 1, center_x_payload
        assert center_x_payload.get("render_pacing_preview_active") is True, center_x_payload
        assert float(center_x_payload["render_pacing_preview_scale"]) < 0.999, center_x_payload
        assert int(center_x_payload["rendered_frame_width"]) < int(center_x_payload["target_render_width"]), center_x_payload
        assert int(center_x_payload["rendered_frame_height"]) < int(center_x_payload["target_render_height"]), center_x_payload

        center_y_payload = viewer.set_control_value(
            "fractal_control.center_y.primary",
            0.755,
            timeout_seconds=45.0,
        )
        _assert_pacing_report_fields(center_y_payload)
        assert center_y_payload.get("ui_automation_command_sequence") == 2, center_y_payload
        assert center_y_payload.get("render_pacing_preview_active") is True, center_y_payload

        settled_payload = _wait_for_pacing_payload(
            viewer,
            lambda payload: payload.get("ui_automation_command_sequence") == 2
            and payload.get("render_pacing_preview_active") is False
            and float(payload.get("render_pacing_preview_scale", 0.0)) == 1.0,
            timeout_seconds=30.0,
        )
        _assert_pacing_report_fields(settled_payload)
        assert int(settled_payload["rendered_frame_width"]) >= int(center_y_payload["rendered_frame_width"]), settled_payload
        assert viewer.launch_count == 1


def test_default_target_fast_f32_stays_full_but_slow_f64_enters_preview_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("persistent runtime viewer harness is Windows-only")

    exe_path = active_runtime_exe()
    neutral_capture = run_headless_capture(
        str(exe_path), "--capture-diagnostic", "--fractal-type", "multibrot", "--width", "320", "--height", "240"
    )
    fast_state = json.loads(json.dumps(neutral_capture["state"]))
    slow_state = json.loads(json.dumps(neutral_capture["state"]))
    _configure_fast_moderate_f32_pacing_state(fast_state)
    _configure_slow_camera_pacing_state(slow_state)
    slow_render = slow_state["render"]
    assert isinstance(slow_render, dict)
    slow_render["width"] = 2048
    slow_render["height"] = 1536
    slow_render["preview_target_fps"] = 30.0

    fast_state_path = write_state_bundle(tmp_path / "fast_f32_default_pacing", fast_state)
    slow_state_path = write_state_bundle(tmp_path / "slow_f64_default_pacing", slow_state)

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=fast_state_path,
        report_path=tmp_path / "default_pacing_report.json",
        command_path=tmp_path / "default_pacing_command.json",
    ) as viewer:
        viewer.wait_for_control("fractal_control.center_x.primary", timeout_seconds=20.0)
        fast_baseline = viewer.wait_for_report(timeout_seconds=60.0)
        _assert_pacing_report_fields(fast_baseline, target_width=4096, target_height=4096)
        old_trigger_ms = (1000.0 / 30.0) * 1.10
        assert float(fast_baseline["last_render_ms"]) > old_trigger_ms, fast_baseline

        fast_payload = viewer.set_control_value(
            "fractal_control.center_x.primary",
            -0.49,
            timeout_seconds=60.0,
        )
        _assert_pacing_report_fields(fast_payload, target_width=4096, target_height=4096)
        assert fast_payload.get("render_pacing_preview_active") is False, fast_payload
        assert float(fast_payload["render_pacing_preview_scale"]) == 1.0, fast_payload
        assert int(fast_payload["rendered_frame_width"]) == 4096, fast_payload
        assert int(fast_payload["rendered_frame_height"]) == 4096, fast_payload

        slow_load = viewer.load_state_json(slow_state_path, expected_fractal_type="multibrot", timeout_seconds=60.0)
        _assert_pacing_report_fields(slow_load, target_width=2048, target_height=1536)
        slow_threshold_ms = (1000.0 / 30.0) * 2.0
        assert float(slow_load["last_render_ms"]) > slow_threshold_ms, slow_load
        slow_payload = viewer.set_control_value(
            "fractal_control.center_x.primary",
            -0.14500000596046448,
            timeout_seconds=90.0,
        )
        _assert_pacing_report_fields(slow_payload, target_width=2048, target_height=1536)
        assert slow_payload.get("render_pacing_preview_active") is True, slow_payload
        assert float(slow_payload["render_pacing_preview_scale"]) < 0.999, slow_payload
        assert int(slow_payload["rendered_frame_width"]) < int(slow_payload["target_render_width"]), slow_payload
        assert int(slow_payload["rendered_frame_height"]) < int(slow_payload["target_render_height"]), slow_payload
        assert viewer.launch_count == 1
