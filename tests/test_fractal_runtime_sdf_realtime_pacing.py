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
    runtime_automation_lock,
    write_state_bundle,
)


@pytest.fixture(autouse=True)
def _serialize_runtime_automation():
    with runtime_automation_lock():
        yield


def _capture_sdf_scalar_source_stack(*, exe_path: Path, state_path: Path) -> dict[str, object]:
    return run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--color-pipeline-action",
        "select_function:source:0:sdf_signed_distance",
        "--capture-diagnostic",
    )


def _capture_sdf_heavy_source_stack(*, exe_path: Path, state_path: Path) -> dict[str, object]:
    return run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--color-pipeline-action",
        "select_function:source:0:sdf_signed_distance",
        "--color-pipeline-action",
        "add_row:source:sdf_inside_outside",
        "--color-pipeline-action",
        "add_row:source:sdf_boundary_band",
        "--color-pipeline-action",
        "add_row:source:sdf_normal_angle",
        "--color-pipeline-action",
        "add_row:source:sdf_curvature",
        "--capture-diagnostic",
    )


def _configure_sdf_heavy_realtime_state(state: dict[str, object]) -> None:
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
            "max_iter": 80,
            "coloring_mode": "smooth_escape",
            "color_signal": "sdf_curvature",
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
            "sample_tier": "fast",
        }
    )
    lens = state.setdefault("lens", {})
    assert isinstance(lens, dict)
    lens.update(
        {
            "enabled": False,
            "downsample": 1,
            "sdf_overlay_mode": "off",
        }
    )


def _wait_for_pacing_payload(viewer: PersistentRuntimeViewerAutomation, predicate, *, timeout_seconds: float) -> dict[str, object]:
    deadline = time.monotonic() + timeout_seconds
    last_payload: dict[str, object] | None = None
    while time.monotonic() < deadline:
        if viewer.proc is not None and viewer.proc.poll() is not None:
            raise AssertionError(f"viewer exited while waiting for SDF pacing report; returncode={viewer.proc.returncode}")
        payload = viewer._load_report()
        if payload is not None:
            last_payload = payload
            if payload.get("rendered_frame_ready") is True and predicate(payload):
                return payload
        time.sleep(0.05)
    raise AssertionError(f"SDF pacing report predicate was not satisfied; last_payload={last_payload!r}")


def _assert_sdf_timing_payload(payload: dict[str, object]) -> None:
    assert payload.get("lens_sdf_valid") is True, payload
    assert payload.get("lens_sdf_color_pipeline_active") is True, payload
    for key in ("base_render_ms", "lens_sdf_field_ms", "lens_sdf_postprocess_ms", "lens_sdf_total_ms", "last_render_ms"):
        assert isinstance(payload.get(key), (int, float)), payload
        assert math.isfinite(float(payload[key])), payload
    assert float(payload["lens_sdf_field_ms"]) >= 0.0, payload
    assert float(payload["lens_sdf_postprocess_ms"]) > 0.0, payload
    assert float(payload["lens_sdf_total_ms"]) >= float(payload["lens_sdf_postprocess_ms"]), payload
    assert float(payload["last_render_ms"]) >= float(payload["base_render_ms"]) + float(payload["lens_sdf_total_ms"]) * 0.95, payload


def test_sdf_color_pipeline_cost_drives_realtime_preview_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("persistent runtime viewer harness is Windows-only")

    exe_path = active_runtime_exe()
    neutral_capture = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "multibrot",
        "--width",
        "320",
        "--height",
        "240",
    )
    seed_state_path = write_state_bundle(tmp_path / "sdf_realtime_seed", neutral_capture["state"])
    scalar_stack_capture = _capture_sdf_scalar_source_stack(exe_path=exe_path, state_path=seed_state_path)
    scalar_state = json.loads(json.dumps(scalar_stack_capture["state"]))
    _configure_sdf_heavy_realtime_state(scalar_state)
    scalar_params = scalar_state["params"]
    assert isinstance(scalar_params, dict)
    scalar_params["color_signal"] = "sdf_signed_distance"
    scalar_state_path = write_state_bundle(tmp_path / "sdf_scalar_realtime_pacing", scalar_state)

    sdf_stack_capture = _capture_sdf_heavy_source_stack(exe_path=exe_path, state_path=seed_state_path)
    state = json.loads(json.dumps(sdf_stack_capture["state"]))
    _configure_sdf_heavy_realtime_state(state)
    state_path = write_state_bundle(tmp_path / "sdf_realtime_pacing", state)

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=scalar_state_path,
        report_path=tmp_path / "sdf_realtime_pacing_report.json",
        command_path=tmp_path / "sdf_realtime_pacing_command.json",
    ) as viewer:
        viewer.wait_for_control("fractal_control.center_x.primary", timeout_seconds=20.0)
        scalar_baseline = viewer.wait_for_report(timeout_seconds=60.0)
        _assert_sdf_timing_payload(scalar_baseline)
        assert scalar_baseline.get("render_pacing_preview_active") is False, scalar_baseline
        assert int(scalar_baseline["lens_sdf_width"]) == int(scalar_baseline["rendered_frame_width"]), scalar_baseline
        assert int(scalar_baseline["lens_sdf_height"]) == int(scalar_baseline["rendered_frame_height"]), scalar_baseline

        baseline = viewer.load_state_json(state_path, expected_fractal_type="multibrot", timeout_seconds=60.0)
        _assert_sdf_timing_payload(baseline)
        assert baseline.get("render_pacing_preview_active") is False, baseline
        slow_threshold_ms = (1000.0 / 240.0) * 2.0
        assert float(baseline["last_render_ms"]) > slow_threshold_ms, baseline

        edited = viewer.set_control_value(
            "fractal_control.center_x.primary",
            -0.49,
            timeout_seconds=90.0,
        )
        _assert_sdf_timing_payload(edited)
        assert edited.get("ui_automation_command_sequence") == viewer.sequence, edited
        assert edited.get("render_pacing_preview_active") is True, edited
        assert int(edited["rendered_frame_width"]) < int(edited["target_render_width"]), edited
        assert int(edited["rendered_frame_height"]) < int(edited["target_render_height"]), edited
        assert int(edited["lens_sdf_width"]) == int(edited["rendered_frame_width"]), edited
        assert int(edited["lens_sdf_height"]) == int(edited["rendered_frame_height"]), edited
        assert float(edited["last_render_ms"]) < float(baseline["last_render_ms"]), edited

        settled = _wait_for_pacing_payload(
            viewer,
            lambda payload: payload.get("ui_automation_command_sequence") == viewer.sequence
            and payload.get("render_pacing_preview_active") is False
            and float(payload.get("render_pacing_preview_scale", 0.0)) == 1.0,
            timeout_seconds=45.0,
        )
        _assert_sdf_timing_payload(settled)
        assert int(settled["rendered_frame_width"]) >= int(settled["target_render_width"]), settled
        assert int(settled["rendered_frame_height"]) >= int(settled["target_render_height"]), settled
        assert int(settled["lens_sdf_width"]) == int(settled["rendered_frame_width"]), settled
        assert int(settled["lens_sdf_height"]) == int(settled["rendered_frame_height"]), settled
        assert viewer.launch_count == 1
