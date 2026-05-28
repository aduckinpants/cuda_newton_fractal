from __future__ import annotations

import copy
from pathlib import Path

import pytest

from runtime_harness import (
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


def _base_multibrot_state(exe_path: Path) -> dict[str, object]:
    capture = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "multibrot",
        "--width",
        "320",
        "--height",
        "240",
    )
    state = copy.deepcopy(capture["state"])
    state["fractal_type"] = "multibrot"
    state["render"]["width"] = 320
    state["render"]["height"] = 240
    state["render"]["sample_tier"] = "fast"
    state["params"]["max_iter"] = 96
    state["params"]["multibrot_power_float"] = 3.0
    state["params"]["multibrot_power_imag"] = 0.0
    state["params"]["coloring_mode"] = "smooth_escape"
    state["params"]["color_signal"] = "sdf_boundary_band"
    state["params"]["color_shape"] = "identity"
    state["params"]["color_palette"] = "cyclic_escape"
    state["params"]["color_grading"] = "escape_default"
    state["lens"] = {
        "enabled": False,
        "downsample": 2,
        "sdf_overlay_mode": "off",
        "sdf_overlay_opacity": 0.55,
        "sdf_overlay_band_px": 1.5,
    }
    return state


def _two_row_state(
    base_state: dict[str, object],
    *,
    first_row_downsample: int | None,
    second_row_downsample: int | None,
) -> dict[str, object]:
    state = copy.deepcopy(base_state)
    first_row: dict[str, object] = {
        "signal": "sdf_signed_distance",
        "scale": 0.05,
        "bias": 0.5,
        "blend_weight": 1.0,
    }
    if first_row_downsample is not None:
        first_row["sdf_field_downsample"] = first_row_downsample
    second_row: dict[str, object] = {
        "signal": "sdf_boundary_band",
        "scale": 1.0,
        "bias": 0.0,
        "blend_weight": 1.0,
        "sdf_boundary_width_px": 2.0,
    }
    if second_row_downsample is not None:
        second_row["sdf_field_downsample"] = second_row_downsample
    state["params"]["color_source_stack"] = [
        first_row,
        second_row,
    ]
    return state


def _field_groups_by_effective(report: dict[str, object]) -> dict[int, dict[str, object]]:
    groups = report.get("lens_sdf_field_groups")
    assert isinstance(groups, list), report
    result: dict[int, dict[str, object]] = {}
    for group in groups:
        assert isinstance(group, dict), report
        result[int(group["effective_downsample"])] = group
    return result


def test_sdf_source_rows_can_use_distinct_field_resolution_no_mouse(tmp_path: Path) -> None:
    exe_path = active_runtime_exe()
    base_state = _base_multibrot_state(exe_path)
    distinct_state = _two_row_state(base_state, first_row_downsample=1, second_row_downsample=4)
    shared_state = _two_row_state(base_state, first_row_downsample=None, second_row_downsample=None)
    distinct_path = write_state_bundle(tmp_path / "distinct_row_fields", distinct_state)
    shared_path = write_state_bundle(tmp_path / "shared_row_fields", shared_state)

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=distinct_path,
        report_path=tmp_path / "sdf_row_field_resolution_report.json",
        command_path=tmp_path / "sdf_row_field_resolution_command.json",
        open_color_pipeline=False,
    ) as viewer:
        distinct_report = viewer.wait_for_report(timeout_seconds=60.0)
        assert distinct_report.get("current_fractal_type") == "multibrot", distinct_report
        assert distinct_report.get("lens_sdf_valid") is True, distinct_report
        assert distinct_report.get("lens_sdf_color_pipeline_active") is True, distinct_report
        assert int(distinct_report.get("lens_sdf_field_group_count", 0)) == 2, distinct_report
        distinct_groups = _field_groups_by_effective(distinct_report)
        assert set(distinct_groups) == {1, 4}, distinct_report
        assert distinct_groups[1]["has_explicit_row"] is True
        assert distinct_groups[4]["has_explicit_row"] is True
        assert distinct_report.get("lens_sdf_postprocess_backend_used") == "cpu", distinct_report
        distinct_hash = distinct_report["rendered_frame_hash"]

        shared_report = viewer.load_state_json(shared_path, expected_fractal_type="multibrot", timeout_seconds=60.0)
        assert int(shared_report.get("lens_sdf_field_group_count", 0)) == 1, shared_report
        shared_groups = _field_groups_by_effective(shared_report)
        assert set(shared_groups) == {2}, shared_report
        assert shared_groups[2]["row_count"] == 2, shared_report
        assert shared_report.get("lens_sdf_postprocess_backend_used") in {
            "cuda_direct_scalar",
            "cuda_field_signal",
        }, shared_report
        assert shared_report["rendered_frame_hash"] != distinct_hash, shared_report


def test_visible_sdf_row_field_downsample_controls_drive_distinct_fields_no_mouse(tmp_path: Path) -> None:
    exe_path = active_runtime_exe()
    base_state = _base_multibrot_state(exe_path)
    inherited_state = _two_row_state(base_state, first_row_downsample=None, second_row_downsample=None)
    inherited_path = write_state_bundle(tmp_path / "visible_inherited_row_fields", inherited_state)

    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=inherited_path,
        report_path=tmp_path / "visible_sdf_row_field_resolution_report.json",
        command_path=tmp_path / "visible_sdf_row_field_resolution_command.json",
        open_color_pipeline=True,
    ) as viewer:
        ready_report = viewer.wait_for_report(timeout_seconds=60.0)
        assert int(ready_report.get("lens_sdf_field_group_count", 0)) == 1, ready_report
        ready_hash = ready_report["rendered_frame_hash"]
        viewer.wait_for_control(
            "color_pipeline.source.sdf_signed_distance.signal.sdf_field_downsample.primary",
            timeout_seconds=20.0,
        )
        viewer.wait_for_control(
            "color_pipeline.source.sdf_boundary_band.signal.sdf_field_downsample.primary",
            timeout_seconds=20.0,
        )

        high_detail_report = viewer.set_control_value(
            "color_pipeline.source.sdf_signed_distance.signal.sdf_field_downsample.primary",
            1.0,
            timeout_seconds=60.0,
        )
        assert high_detail_report.get("set_value_consumed") is True, high_detail_report
        assert int(high_detail_report.get("lens_sdf_field_group_count", 0)) == 2, high_detail_report
        high_detail_groups = _field_groups_by_effective(high_detail_report)
        assert set(high_detail_groups) == {1, 2}, high_detail_report
        assert high_detail_groups[1]["has_explicit_row"] is True, high_detail_report
        assert high_detail_groups[2]["has_inherited_row"] is True, high_detail_report

        distinct_report = viewer.set_control_value(
            "color_pipeline.source.sdf_boundary_band.signal.sdf_field_downsample.primary",
            4.0,
            timeout_seconds=60.0,
        )
        assert distinct_report.get("set_value_consumed") is True, distinct_report
        assert distinct_report.get("rendered_frame_hash") != ready_hash, distinct_report
        assert int(distinct_report.get("lens_sdf_field_group_count", 0)) == 2, distinct_report
        distinct_groups = _field_groups_by_effective(distinct_report)
        assert set(distinct_groups) == {1, 4}, distinct_report
        assert distinct_groups[1]["has_explicit_row"] is True, distinct_report
        assert distinct_groups[4]["has_explicit_row"] is True, distinct_report
