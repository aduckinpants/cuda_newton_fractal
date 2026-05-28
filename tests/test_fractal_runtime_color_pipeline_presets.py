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


@pytest.fixture(autouse=True)
def _serialize_runtime_automation():
    with runtime_automation_lock():
        yield


NON_SDF_SOURCE_ROWS = (
    ("smooth_escape_ramp", "heatmap", "smooth_escape", "cyclic_escape"),
    ("phase_orbit", "phase_wheel_palette", "phase_angle", "phase_wheel"),
    ("banded_signal", "banded_heatmap", "iteration_bands", "banded_escape"),
    ("escape_magnitude", "heatmap", "escape_magnitude", "cyclic_escape"),
    ("orbit_stripe", "phase_wheel_palette", "orbit_stripe", "phase_wheel"),
    ("root_proximity", "heatmap", "root_proximity", "cyclic_escape"),
    ("root_index", "root_classic_palette", "root_index", "root_classic"),
)


def _capture_non_sdf_source_row(
    *,
    exe_path: Path,
    state_path: Path,
    source_function_id: str,
    palette_function_id: str,
) -> dict[str, object]:
    return run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--color-pipeline-action",
        f"select_function:source:0:{source_function_id}",
        "--color-pipeline-action",
        f"select_function:palette:0:{palette_function_id}",
        "--capture-diagnostic",
    )


def _assert_color_pipeline_state(
    capture: dict[str, object],
    *,
    expected_signal: str,
    expected_palette: str,
) -> None:
    state = capture["state"]
    assert isinstance(state, dict)
    params = state.get("params")
    assert isinstance(params, dict)
    assert params.get("color_signal") == expected_signal
    assert params.get("color_palette") == expected_palette


def test_color_pipeline_recipe_presets_are_visible_and_apply_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Color Pipeline preset runtime regression is Windows-only")

    exe_path = active_runtime_exe()
    neutral_capture = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "mandelbrot",
        "--width",
        "160",
        "--height",
        "120",
    )
    state_path = write_state_bundle(
        tmp_path / "color_pipeline_preset_seed",
        json.loads(json.dumps(neutral_capture["state"])),
    )
    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "color_pipeline_presets_report.json",
        command_path=tmp_path / "color_pipeline_presets_command.json",
        open_color_pipeline=True,
    ) as viewer:
        ready_report = viewer.wait_for_report(timeout_seconds=30.0)
        base_hash = ready_report.get("rendered_frame_hash")
        assert isinstance(base_hash, str), ready_report
        viewer.wait_for_control("color_pipeline.recipe.default_smooth_escape.apply", timeout_seconds=20.0)
        viewer.wait_for_control("color_pipeline.recipe.phase_orbit_wheel.apply", timeout_seconds=20.0)
        viewer.wait_for_control("color_pipeline.recipe.sdf_normal_angle_diagnostic.apply", timeout_seconds=20.0)
        viewer.wait_for_control("color_pipeline.recipe.sdf_normal_angle_beauty.apply", timeout_seconds=20.0)
        applied = viewer.click_control("color_pipeline.recipe.sdf_normal_angle_diagnostic.apply", timeout_seconds=60.0)
        beauty = viewer.click_control("color_pipeline.recipe.sdf_normal_angle_beauty.apply", timeout_seconds=60.0)
        viewer.wait_for_control(
            "color_pipeline.source.sdf_normal_angle.signal.sdf_gate_width_px.primary",
            timeout_seconds=20.0,
        )
        beauty_width = viewer.set_control_value(
            "color_pipeline.source.sdf_normal_angle.signal.sdf_gate_width_px.primary",
            2.0,
            timeout_seconds=60.0,
        )

    assert applied.get("click_consumed") is True, applied
    assert "source:sdf_normal_angle" in applied.get("lane_rows", []), applied
    assert "palette:phase_wheel_palette" in applied.get("lane_rows", []), applied
    assert "grading:phase_finish" in applied.get("lane_rows", []), applied
    assert applied.get("rendered_frame_hash") != base_hash, applied
    assert beauty.get("click_consumed") is True, beauty
    assert "source:sdf_normal_angle" in beauty.get("lane_rows", []), beauty
    assert "palette:phase_wheel_palette" in beauty.get("lane_rows", []), beauty
    assert beauty.get("rendered_frame_hash") != applied.get("rendered_frame_hash"), beauty
    assert beauty_width.get("set_value_consumed") is True, beauty_width
    assert beauty_width.get("rendered_frame_hash") != beauty.get("rendered_frame_hash"), beauty_width


def test_non_sdf_source_rows_do_not_alias_smooth_escape_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Color Pipeline source-row runtime regression is Windows-only")

    exe_path = active_runtime_exe()
    neutral_capture = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "newton",
        "--width",
        "192",
        "--height",
        "144",
    )
    state_path = write_state_bundle(
        tmp_path / "non_sdf_source_distinctness_seed",
        json.loads(json.dumps(neutral_capture["state"])),
    )

    captures: dict[str, dict[str, object]] = {}
    for source_function_id, palette_function_id, expected_signal, expected_palette in NON_SDF_SOURCE_ROWS:
        capture = _capture_non_sdf_source_row(
            exe_path=exe_path,
            state_path=state_path,
            source_function_id=source_function_id,
            palette_function_id=palette_function_id,
        )
        _assert_color_pipeline_state(
            capture,
            expected_signal=expected_signal,
            expected_palette=expected_palette,
        )
        captures[source_function_id] = capture

    smooth_hash = captures["smooth_escape_ramp"]["frame_hash"]
    assert isinstance(smooth_hash, str), captures["smooth_escape_ramp"]
    for source_function_id, capture in captures.items():
        frame_hash = capture["frame_hash"]
        assert isinstance(frame_hash, str), capture
        if source_function_id == "smooth_escape_ramp":
            continue
        assert frame_hash != smooth_hash, (
            f"expected non-SDF Source row {source_function_id!r} to render distinctly from smooth_escape_ramp; "
            f"both produced {frame_hash}"
        )

    hash_owners: dict[str, str] = {}
    for source_function_id, capture in captures.items():
        frame_hash = capture["frame_hash"]
        assert isinstance(frame_hash, str), capture
        previous_owner = hash_owners.setdefault(frame_hash, source_function_id)
        assert previous_owner == source_function_id, (
            f"expected shipped non-SDF Source row {source_function_id!r} to render distinctly from "
            f"{previous_owner!r}; both produced {frame_hash}"
        )
