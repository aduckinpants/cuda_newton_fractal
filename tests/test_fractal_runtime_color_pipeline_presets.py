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
        viewer.wait_for_control("color_pipeline.recipe.selector", timeout_seconds=20.0)
        viewer.wait_for_control("color_pipeline.recipe.apply_selected", timeout_seconds=20.0)
        controls_report = viewer.wait_for_report(timeout_seconds=20.0)
        visible_controls = {
            control.get("control_id")
            for control in controls_report.get("controls", [])
            if isinstance(control, dict)
        }
        for recipe_id in (
            "default_smooth_escape",
            "phase_orbit_wheel",
            "root_phase_wheel",
            "root_proximity_heatmap",
            "sdf_normal_angle_diagnostic",
            "sdf_normal_angle_beauty",
        ):
            assert f"color_pipeline.recipe.{recipe_id}.apply" not in visible_controls, controls_report

        selected = viewer.click_control("color_pipeline.recipe.sdf_normal_angle_diagnostic.select", timeout_seconds=60.0)
        assert selected.get("click_consumed") is True, selected
        applied = viewer.click_control("color_pipeline.recipe.apply_selected", timeout_seconds=60.0)
        selected_beauty = viewer.click_control("color_pipeline.recipe.sdf_normal_angle_beauty.select", timeout_seconds=60.0)
        assert selected_beauty.get("click_consumed") is True, selected_beauty
        beauty = viewer.click_control("color_pipeline.recipe.apply_selected", timeout_seconds=60.0)
        viewer.wait_for_control(
            "color_pipeline.source.sdf_normal_angle.signal.sdf_gate_width_px.primary",
            timeout_seconds=20.0,
        )
        beauty_width = viewer.set_control_value(
            "color_pipeline.source.sdf_normal_angle.signal.sdf_gate_width_px.primary",
            2.0,
            timeout_seconds=60.0,
        )
        selected_diagnostic_again = viewer.click_control(
            "color_pipeline.recipe.sdf_normal_angle_diagnostic.select",
            timeout_seconds=60.0,
        )
        assert selected_diagnostic_again.get("click_consumed") is True, selected_diagnostic_again
        diagnostic_again = viewer.click_control(
            "color_pipeline.recipe.apply_selected",
            timeout_seconds=60.0,
        )

    assert applied.get("click_consumed") is True, applied
    assert "source:sdf_normal_angle" in applied.get("lane_rows", []), applied
    assert "palette:phase_wheel_palette" in applied.get("lane_rows", []), applied
    assert "grading:phase_finish" in applied.get("lane_rows", []), applied
    assert applied.get("rendered_frame_hash") != base_hash, applied
    assert beauty.get("click_consumed") is True, beauty
    assert "source:sdf_normal_angle" in beauty.get("lane_rows", []), beauty
    assert "source:lens_field_v2_distance" in beauty.get("lane_rows", []), beauty
    assert "palette:phase_wheel_palette" in beauty.get("lane_rows", []), beauty
    assert beauty.get("rendered_frame_hash") != applied.get("rendered_frame_hash"), beauty
    assert beauty_width.get("set_value_consumed") is True, beauty_width
    assert beauty_width.get("rendered_frame_hash") != beauty.get("rendered_frame_hash"), beauty_width
    assert diagnostic_again.get("click_consumed") is True, diagnostic_again
    assert "source:lens_field_v2_distance" not in diagnostic_again.get("lane_rows", []), diagnostic_again
    assert diagnostic_again.get("rendered_frame_hash") == applied.get("rendered_frame_hash"), diagnostic_again



def test_color_pipeline_source_stack_graph_receipt_is_reported_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Color Pipeline graph receipt runtime regression is Windows-only")

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
        tmp_path / "color_pipeline_graph_receipt_seed",
        json.loads(json.dumps(neutral_capture["state"])),
    )
    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "color_pipeline_graph_receipt_report.json",
        command_path=tmp_path / "color_pipeline_graph_receipt_command.json",
        open_color_pipeline=True,
    ) as viewer:
        viewer.wait_for_control("color_pipeline.recipe.selector", timeout_seconds=20.0)
        selected = viewer.click_control("color_pipeline.recipe.sdf_normal_angle_beauty.select", timeout_seconds=60.0)
        assert selected.get("click_consumed") is True, selected
        applied = viewer.click_control("color_pipeline.recipe.apply_selected", timeout_seconds=60.0)

    assert applied.get("click_consumed") is True, applied
    receipt = applied.get("color_pipeline_graph_receipt")
    assert isinstance(receipt, dict), applied
    assert receipt.get("schema_id") == "viewer.color_pipeline_graph_receipt.v1", receipt
    assert receipt.get("execution_authority") == "linear_row_stack", receipt
    assert receipt.get("ui_projection") == "linear_color_stack", receipt
    assert receipt.get("source_stack_kind") == "sdf_only", receipt
    nodes = receipt.get("nodes")
    assert isinstance(nodes, list), receipt
    node_by_id = {
        node.get("id"): node
        for node in nodes
        if isinstance(node, dict)
    }
    assert node_by_id.get("source.0", {}).get("function_id") == "sdf_normal_angle", receipt
    assert node_by_id.get("source.1", {}).get("function_id") == "lens_field_v2_distance", receipt
    assert node_by_id.get("shape.0", {}).get("function_id") == "identity", receipt
    assert node_by_id.get("palette.0", {}).get("function_id") == "phase_wheel_palette", receipt
    source_0 = node_by_id["source.0"]
    assert source_0.get("enabled") is True and source_0.get("active_execution") is True, source_0
    assert source_0.get("sdf_applicator") == "boundary_band", source_0
    assert source_0.get("sdf_field_downsample") in {"0", "1", "2", "4", "8", "16"}, source_0
    edges = receipt.get("edges")
    assert isinstance(edges, list), receipt
    edge_ids = {
        edge.get("id")
        for edge in edges
        if isinstance(edge, dict)
    }
    assert "source.0->source.1" in edge_ids, receipt
    assert "source.1->shape.0" in edge_ids, receipt
    unsupported = receipt.get("unsupported_routes")
    assert isinstance(unsupported, list), receipt


def _graph_source_root_pattern_ref(report: dict[str, object]) -> str:
    receipt = report.get("color_pipeline_graph_receipt")
    assert isinstance(receipt, dict), report
    nodes = receipt.get("nodes")
    assert isinstance(nodes, list), receipt
    for node in nodes:
        if not isinstance(node, dict):
            continue
        if node.get("id") == "source.0":
            value = node.get("root_pattern_ref")
            assert isinstance(value, str), node
            return value
    raise AssertionError(f"source.0 node missing from receipt: {receipt!r}")


def test_color_pipeline_root_pattern_authority_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Color Pipeline root-pattern authority runtime regression is Windows-only")

    exe_path = active_runtime_exe()
    neutral_capture = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino_magnet_root_well",
        "--width",
        "160",
        "--height",
        "120",
    )
    poisoned_state = json.loads(json.dumps(neutral_capture["state"]))
    params = poisoned_state.get("params")
    assert isinstance(params, dict), poisoned_state
    params["explaino_root_field_pattern_ref"] = "color_root_field"
    state_path = write_state_bundle(
        tmp_path / "color_pipeline_root_pattern_authority_seed",
        poisoned_state,
    )
    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "color_pipeline_root_pattern_authority_report.json",
        command_path=tmp_path / "color_pipeline_root_pattern_authority_command.json",
        open_color_pipeline=True,
    ) as viewer:
        viewer.wait_for_control("color_pipeline.recipe.selector", timeout_seconds=20.0)
        selected_phase = viewer.click_control("color_pipeline.recipe.root_phase_wheel.select", timeout_seconds=60.0)
        assert selected_phase.get("click_consumed") is True, selected_phase
        applied_phase = viewer.click_control("color_pipeline.recipe.apply_selected", timeout_seconds=60.0)
        selected_proximity = viewer.click_control("color_pipeline.recipe.root_proximity_heatmap.select", timeout_seconds=60.0)
        assert selected_proximity.get("click_consumed") is True, selected_proximity
        applied_proximity = viewer.click_control("color_pipeline.recipe.apply_selected", timeout_seconds=60.0)

    assert applied_phase.get("click_consumed") is True, applied_phase
    assert "source:root_phase" in applied_phase.get("lane_rows", []), applied_phase
    assert _graph_source_root_pattern_ref(applied_phase) == "dynamics_root_field"
    phase_params = applied_phase.get("params")
    if isinstance(phase_params, dict):
        source_stack = phase_params.get("color_source_stack")
        assert isinstance(source_stack, list) and source_stack, applied_phase
        assert source_stack[0].get("root_pattern_ref") == "dynamics_root_field", applied_phase

    assert applied_proximity.get("click_consumed") is True, applied_proximity
    assert "source:root_proximity" in applied_proximity.get("lane_rows", []), applied_proximity
    assert _graph_source_root_pattern_ref(applied_proximity) == "dynamics_root_field"
    proximity_params = applied_proximity.get("params")
    if isinstance(proximity_params, dict):
        source_stack = proximity_params.get("color_source_stack")
        assert isinstance(source_stack, list) and source_stack, applied_proximity
        assert source_stack[0].get("root_pattern_ref") == "dynamics_root_field", applied_proximity


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


def _capture_shape_row(
    *,
    exe_path: Path,
    state_path: Path,
    shape_function_id: str,
    param_actions: list[str],
) -> dict[str, object]:
    args = [
        str(exe_path),
        "--load-state-json",
        str(state_path),
        "--color-pipeline-action",
        f"select_function:shape:0:{shape_function_id}",
    ]
    for action in param_actions:
        args.extend(["--color-pipeline-action", action])
    args.append("--capture-diagnostic")
    return run_headless_capture(*args)


def test_color_pipeline_function_library_batch1_shapes_are_runtime_backed_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Color Pipeline runtime regression is Windows-only")

    exe_path = active_runtime_exe()
    neutral_capture = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "mandelbrot",
        "--width",
        "192",
        "--height",
        "144",
    )
    state_path = write_state_bundle(
        tmp_path / "color_pipeline_batch1_shape_seed",
        json.loads(json.dumps(neutral_capture["state"])),
    )

    baseline_hash = neutral_capture["frame_hash"]
    assert isinstance(baseline_hash, str), neutral_capture

    log_capture = _capture_shape_row(
        exe_path=exe_path,
        state_path=state_path,
        shape_function_id="log_compress",
        param_actions=["set_param:shape:0:shape.scale:number:6.0"],
    )
    smoothstep_capture = _capture_shape_row(
        exe_path=exe_path,
        state_path=state_path,
        shape_function_id="smoothstep_range",
        param_actions=[
            "set_param:shape:0:shape.center:number:0.35",
            "set_param:shape:0:shape.width:number:0.35",
            "set_param:shape:0:shape.softness:number:1.0",
        ],
    )

    for capture, expected_shape in (
        (log_capture, "log_compress"),
        (smoothstep_capture, "smoothstep_range"),
    ):
        params = capture["state"]["params"]
        assert isinstance(params, dict), capture
        shape_stack = params.get("color_shape_stack")
        assert isinstance(shape_stack, list) and shape_stack, capture
        assert shape_stack[0].get("shape") == expected_shape, capture
        frame_hash = capture["frame_hash"]
        assert isinstance(frame_hash, str), capture
        assert frame_hash != baseline_hash, capture

    assert log_capture["frame_hash"] != smoothstep_capture["frame_hash"]
