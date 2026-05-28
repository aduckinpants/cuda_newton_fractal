from __future__ import annotations

import json
import math
import sys
import time
from pathlib import Path

import pytest

from tests.runtime_harness import (
    PersistentRuntimeViewerAutomation,
    RuntimeReportTiming,
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


def _configure_sdf_animation_realtime_state(state: dict[str, object]) -> None:
    _configure_sdf_heavy_realtime_state(state)
    state["fractal_type"] = "explaino_all"
    view = state["view"]
    assert isinstance(view, dict)
    view.update(
        {
            "center_x": 0.0,
            "center_y": 0.0,
            "center_hp_x": 0.0,
            "center_hp_y": 0.0,
            "zoom": 1.0,
            "log2_zoom": 0.0,
        }
    )
    params = state["params"]
    assert isinstance(params, dict)
    params.update(
        {
            "max_iter": 150,
            "ripple_amplitude": 0.15,
            "color_signal": "sdf_curvature",
            "color_shape": "identity",
            "color_palette": "cyclic_escape",
            "color_grading": "escape_default",
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


def _wait_for_pacing_timing(
    viewer: PersistentRuntimeViewerAutomation,
    command_issue_epoch: float,
    command_sequence: int,
    predicate,
    *,
    timeout_seconds: float,
) -> RuntimeReportTiming:
    return viewer.wait_for_report_timing(
        command_issue_epoch,
        command_sequence,
        lambda payload: payload.get("ui_automation_command_sequence") == command_sequence and predicate(payload),
        timeout_seconds=timeout_seconds,
    )


def _assert_sdf_timing_payload(payload: dict[str, object]) -> None:
    assert payload.get("lens_sdf_valid") is True, payload
    assert payload.get("lens_sdf_color_pipeline_active") is True, payload
    for key in (
        "base_render_ms",
        "lens_sdf_field_ms",
        "lens_sdf_requested_equivalent_field_ms",
        "lens_sdf_field_cache_lookup_ms",
        "lens_sdf_field_mask_downsample_ms",
        "lens_sdf_field_backend_ms",
        "lens_sdf_field_cache_store_ms",
        "lens_sdf_postprocess_ms",
        "lens_sdf_total_ms",
        "last_render_ms",
    ):
        assert isinstance(payload.get(key), (int, float)), payload
        assert math.isfinite(float(payload[key])), payload
    assert float(payload["lens_sdf_field_ms"]) >= 0.0, payload
    assert float(payload["lens_sdf_field_cache_lookup_ms"]) >= 0.0, payload
    assert float(payload["lens_sdf_field_mask_downsample_ms"]) >= 0.0, payload
    assert float(payload["lens_sdf_field_backend_ms"]) >= 0.0, payload
    assert float(payload["lens_sdf_field_cache_store_ms"]) >= 0.0, payload
    assert float(payload["lens_sdf_requested_equivalent_field_ms"]) >= float(payload["lens_sdf_field_ms"]), payload
    assert float(payload["lens_sdf_postprocess_ms"]) > 0.0, payload
    assert float(payload["lens_sdf_total_ms"]) >= float(payload["lens_sdf_postprocess_ms"]), payload
    assert float(payload["last_render_ms"]) >= float(payload["base_render_ms"]) + float(payload["lens_sdf_total_ms"]) * 0.95, payload
    assert int(payload.get("lens_sdf_postprocess_pixel_step", 0)) >= 1, payload
    assert int(payload.get("lens_sdf_postprocess_filled_pixel_count", 0)) >= 0, payload
    assert int(payload.get("lens_sdf_postprocess_direct_sample_count", 0)) >= 0, payload
    assert int(payload.get("lens_sdf_postprocess_neighborhood_sample_count", 0)) >= 0, payload
    requested_downsample = int(payload.get("lens_sdf_requested_downsample", 0))
    effective_downsample = int(payload.get("lens_sdf_effective_downsample", 0))
    assert requested_downsample in {1, 2, 4, 8, 16}, payload
    assert effective_downsample in {1, 2, 4, 8, 16}, payload
    assert effective_downsample >= requested_downsample, payload
    assert float(payload.get("lens_sdf_pixel_scale", 0.0)) == pytest.approx(float(effective_downsample)), payload
    quality_mode = payload.get("lens_sdf_quality_mode")
    assert quality_mode in {"requested", "interactive_adaptive"}, payload
    if quality_mode == "requested":
        assert effective_downsample == requested_downsample, payload
    else:
        assert effective_downsample > requested_downsample, payload


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
    heavy_render = state["render"]
    assert isinstance(heavy_render, dict)
    heavy_render["preview_target_fps"] = 1200.0
    heavy_preview_target_fps = float(heavy_render["preview_target_fps"])
    state_path = write_state_bundle(tmp_path / "sdf_realtime_pacing", state)

    explaino_seed_capture = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino_all",
        "--width",
        "320",
        "--height",
        "240",
    )
    explaino_seed_state_path = write_state_bundle(tmp_path / "sdf_animation_seed", explaino_seed_capture["state"])
    animation_stack_capture = _capture_sdf_heavy_source_stack(exe_path=exe_path, state_path=explaino_seed_state_path)
    animation_state = json.loads(json.dumps(animation_stack_capture["state"]))
    _configure_sdf_animation_realtime_state(animation_state)
    animation_state_path = write_state_bundle(tmp_path / "sdf_animation_pacing", animation_state)

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
        assert scalar_baseline.get("lens_sdf_quality_mode") == "requested", scalar_baseline
        assert int(scalar_baseline["lens_sdf_requested_downsample"]) == 1, scalar_baseline
        assert int(scalar_baseline["lens_sdf_effective_downsample"]) == 1, scalar_baseline
        assert int(scalar_baseline["lens_sdf_postprocess_pixel_step"]) == 1, scalar_baseline
        assert int(scalar_baseline["lens_sdf_postprocess_filled_pixel_count"]) == int(scalar_baseline["rendered_frame_width"]) * int(scalar_baseline["rendered_frame_height"]), scalar_baseline
        assert int(scalar_baseline["lens_sdf_width"]) == int(scalar_baseline["rendered_frame_width"]), scalar_baseline
        assert int(scalar_baseline["lens_sdf_height"]) == int(scalar_baseline["rendered_frame_height"]), scalar_baseline

        scalar_edited = viewer.set_control_value(
            "fractal_control.center_x.primary",
            -0.49,
            timeout_seconds=90.0,
        )
        _assert_sdf_timing_payload(scalar_edited)
        assert scalar_edited.get("ui_automation_command_sequence") == viewer.sequence, scalar_edited
        assert scalar_edited.get("render_pacing_preview_active") is True, scalar_edited
        assert int(scalar_edited["rendered_frame_width"]) < int(scalar_edited["target_render_width"]), scalar_edited
        assert int(scalar_edited["lens_sdf_postprocess_pixel_step"]) == 1, scalar_edited
        scalar_edited_pixels = int(scalar_edited["rendered_frame_width"]) * int(scalar_edited["rendered_frame_height"])
        scalar_edited_field_pixels = int(scalar_edited["lens_sdf_width"]) * int(scalar_edited["lens_sdf_height"])
        assert int(scalar_edited["lens_sdf_postprocess_filled_pixel_count"]) == scalar_edited_pixels, scalar_edited
        assert int(scalar_edited["lens_sdf_postprocess_direct_sample_count"]) == scalar_edited_field_pixels, scalar_edited
        assert int(scalar_edited["lens_sdf_postprocess_neighborhood_sample_count"]) == 0, scalar_edited

        scalar_settled = _wait_for_pacing_payload(
            viewer,
            lambda payload: payload.get("ui_automation_command_sequence") == viewer.sequence
            and payload.get("render_pacing_preview_active") is False
            and float(payload.get("render_pacing_preview_scale", 0.0)) == 1.0
            and int(payload.get("rendered_frame_width", 0)) >= int(payload.get("target_render_width", 1))
            and int(payload.get("rendered_frame_height", 0)) >= int(payload.get("target_render_height", 1)),
            timeout_seconds=45.0,
        )
        _assert_sdf_timing_payload(scalar_settled)
        assert scalar_settled.get("lens_sdf_quality_mode") == "requested", scalar_settled
        assert int(scalar_settled["lens_sdf_effective_downsample"]) == int(scalar_settled["lens_sdf_requested_downsample"]) == 1, scalar_settled
        assert int(scalar_settled["lens_sdf_postprocess_pixel_step"]) == 1, scalar_settled
        assert int(scalar_settled["lens_sdf_postprocess_filled_pixel_count"]) == int(scalar_settled["rendered_frame_width"]) * int(scalar_settled["rendered_frame_height"]), scalar_settled

        baseline = viewer.load_state_json(state_path, expected_fractal_type="multibrot", timeout_seconds=60.0)
        _assert_sdf_timing_payload(baseline)
        assert baseline.get("render_pacing_preview_active") is False, baseline
        assert baseline.get("lens_sdf_quality_mode") == "requested", baseline
        assert int(baseline["lens_sdf_effective_downsample"]) == int(baseline["lens_sdf_requested_downsample"]) == 1, baseline
        assert int(baseline["lens_sdf_postprocess_pixel_step"]) == 1, baseline
        assert int(baseline["lens_sdf_postprocess_filled_pixel_count"]) == int(baseline["rendered_frame_width"]) * int(baseline["rendered_frame_height"]), baseline
        slow_threshold_ms = (1000.0 / heavy_preview_target_fps) * 1.5
        assert float(baseline["last_render_ms"]) > slow_threshold_ms, baseline

        edited_timing = viewer.set_control_value_timing(
            "fractal_control.center_x.primary",
            -0.49,
            timeout_seconds=90.0,
        )
        edited = edited_timing.payload
        _assert_sdf_timing_payload(edited)
        assert edited.get("ui_automation_command_sequence") == viewer.sequence, edited
        assert edited.get("render_pacing_preview_active") is True, edited
        assert edited.get("lens_sdf_quality_mode") == "interactive_adaptive", edited
        assert int(edited["lens_sdf_requested_downsample"]) == 1, edited
        assert int(edited["lens_sdf_effective_downsample"]) > 1, edited
        assert float(edited["lens_sdf_requested_equivalent_field_ms"]) > float(edited["lens_sdf_field_ms"]), edited
        assert int(edited["rendered_frame_width"]) < int(edited["target_render_width"]), edited
        assert int(edited["rendered_frame_height"]) < int(edited["target_render_height"]), edited
        assert int(edited["lens_sdf_width"]) < int(edited["rendered_frame_width"]), edited
        assert int(edited["lens_sdf_height"]) < int(edited["rendered_frame_height"]), edited
        assert int(edited["lens_sdf_postprocess_pixel_step"]) >= 2, edited
        edited_pixels = int(edited["rendered_frame_width"]) * int(edited["rendered_frame_height"])
        edited_samples = int(edited["lens_sdf_postprocess_direct_sample_count"]) + int(edited["lens_sdf_postprocess_neighborhood_sample_count"])
        assert int(edited["lens_sdf_postprocess_filled_pixel_count"]) == edited_pixels, edited
        assert 0 < edited_samples < edited_pixels, edited
        assert float(edited["last_render_ms"]) < float(baseline["last_render_ms"]), edited
        assert edited_timing.command_to_publish_ms <= edited_timing.command_to_report_ms + 5.0, (
            edited_timing.command_to_publish_ms,
            edited_timing.command_to_report_ms,
        )
        assert edited_timing.publish_to_observed_ms < 250.0, edited_timing.publish_to_observed_ms
        assert edited_timing.command_to_report_ms < max(150.0, float(baseline["last_render_ms"]) * 2.0), (
            edited_timing.command_to_report_ms,
            baseline,
            edited,
        )

        settled_timing = _wait_for_pacing_timing(
            viewer,
            edited_timing.command_issue_epoch,
            edited_timing.command_sequence,
            lambda payload: payload.get("ui_automation_command_sequence") == viewer.sequence
            and payload.get("render_pacing_preview_active") is False
            and float(payload.get("render_pacing_preview_scale", 0.0)) == 1.0
            and int(payload.get("rendered_frame_width", 0)) >= int(payload.get("target_render_width", 1))
            and int(payload.get("rendered_frame_height", 0)) >= int(payload.get("target_render_height", 1)),
            timeout_seconds=45.0,
        )
        settled = settled_timing.payload
        _assert_sdf_timing_payload(settled)
        assert int(settled["rendered_frame_width"]) >= int(settled["target_render_width"]), settled
        assert int(settled["rendered_frame_height"]) >= int(settled["target_render_height"]), settled
        assert settled.get("lens_sdf_quality_mode") == "requested", settled
        assert int(settled["lens_sdf_effective_downsample"]) == int(settled["lens_sdf_requested_downsample"]) == 1, settled
        assert int(settled["lens_sdf_width"]) == int(settled["rendered_frame_width"]), settled
        assert int(settled["lens_sdf_height"]) == int(settled["rendered_frame_height"]), settled
        assert int(settled["lens_sdf_postprocess_pixel_step"]) == 1, settled
        assert int(settled["lens_sdf_postprocess_filled_pixel_count"]) == int(settled["rendered_frame_width"]) * int(settled["rendered_frame_height"]), settled
        assert settled_timing.command_to_report_ms >= edited_timing.command_to_report_ms, (
            edited_timing.command_to_report_ms,
            settled_timing.command_to_report_ms,
        )

        animation_baseline = viewer.load_state_json(
            animation_state_path,
            expected_fractal_type="explaino_all",
            timeout_seconds=60.0,
        )
        _assert_sdf_timing_payload(animation_baseline)
        assert animation_baseline.get("render_pacing_preview_active") is False, animation_baseline
        assert animation_baseline.get("lens_sdf_quality_mode") == "requested", animation_baseline
        assert int(animation_baseline["lens_sdf_postprocess_pixel_step"]) == 1, animation_baseline
        animation_slow_threshold_ms = (1000.0 / 240.0) * 2.0
        assert float(animation_baseline["last_render_ms"]) > animation_slow_threshold_ms, animation_baseline

        animation_enable = viewer.set_enum_id(
            "fractal.view.param_anim_target",
            "ripple_amplitude",
            expected_fractal_type="explaino_all",
            timeout_seconds=60.0,
        )
        _assert_sdf_timing_payload(animation_enable)
        animation_sequence = viewer.sequence
        animation_wait_started = time.monotonic()
        animation_preview = _wait_for_pacing_payload(
            viewer,
            lambda payload: payload.get("ui_automation_command_sequence") == animation_sequence
            and (time.monotonic() - animation_wait_started) > 1.0
            and payload.get("render_pacing_preview_active") is True
            and float(payload.get("render_pacing_preview_scale", 1.0)) < 0.999
            and int(payload.get("rendered_frame_width", 0)) < int(payload.get("target_render_width", 0))
            and int(payload.get("lens_sdf_postprocess_pixel_step", 1)) >= 1,
            timeout_seconds=15.0,
        )
        _assert_sdf_timing_payload(animation_preview)
        assert int(animation_preview["lens_sdf_effective_downsample"]) >= int(animation_preview["lens_sdf_requested_downsample"]), animation_preview

        animation_disable = viewer.set_enum_id(
            "fractal.view.param_anim_target",
            "none",
            expected_fractal_type="explaino_all",
            timeout_seconds=60.0,
        )
        _assert_sdf_timing_payload(animation_disable)
        disable_sequence = viewer.sequence
        animation_settled = _wait_for_pacing_payload(
            viewer,
            lambda payload: payload.get("ui_automation_command_sequence") == disable_sequence
            and payload.get("render_pacing_preview_active") is False
            and float(payload.get("render_pacing_preview_scale", 0.0)) == 1.0
            and int(payload.get("rendered_frame_width", 0)) >= int(payload.get("target_render_width", 1))
            and int(payload.get("lens_sdf_postprocess_pixel_step", 0)) == 1,
            timeout_seconds=45.0,
        )
        _assert_sdf_timing_payload(animation_settled)
        assert animation_settled.get("lens_sdf_quality_mode") == "requested", animation_settled
        assert viewer.launch_count == 1
