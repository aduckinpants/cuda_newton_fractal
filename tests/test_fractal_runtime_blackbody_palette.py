from __future__ import annotations

import json
import shutil
import subprocess
import sys
import uuid
from pathlib import Path

import pytest

from tests.runtime_harness import (
    PersistentRuntimeViewerAutomation,
    RUNTIME_DIR,
    active_runtime_exe,
    run_headless_capture,
    runtime_automation_lock,
    write_state_bundle,
)


@pytest.fixture(autouse=True)
def _serialize_runtime_automation():
    with runtime_automation_lock():
        yield


def _palette_row(state: dict[str, object]) -> dict[str, object]:
    draft = state.get("color_pipeline_draft")
    assert isinstance(draft, dict), state
    lanes = draft.get("lanes")
    assert isinstance(lanes, list), draft
    for lane in lanes:
        if not isinstance(lane, dict) or lane.get("lane_id") != "palette":
            continue
        rows = lane.get("rows")
        assert isinstance(rows, list) and rows, lane
        row = rows[0]
        assert isinstance(row, dict), lane
        return row
    raise AssertionError("missing Palette lane")


def _param(row: dict[str, object], path: str) -> dict[str, object]:
    params = row.get("parameter_values")
    if not isinstance(params, list):
        params = row.get("params")
    assert isinstance(params, list), row
    for param in params:
        if isinstance(param, dict) and param.get("path") == path:
            return param
    raise AssertionError(f"missing Palette parameter {path}")


def test_blackbody_palette_non_default_capture_replay_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Black Body published-runtime proof is Windows-only")

    exe_path = active_runtime_exe()
    baseline = run_headless_capture(
        str(exe_path),
        "--fractal-type",
        "mandelbrot",
        "--width",
        "192",
        "--height",
        "144",
        "--capture-diagnostic",
    )
    default_blackbody = run_headless_capture(
        str(exe_path),
        "--fractal-type",
        "mandelbrot",
        "--width",
        "192",
        "--height",
        "144",
        "--color-pipeline-action",
        "select_function:source:0:smooth_escape_ramp",
        "--color-pipeline-action",
        "select_function:shape:0:identity",
        "--color-pipeline-action",
        "select_function:palette:0:blackbody_palette_v1",
        "--capture-diagnostic",
    )
    changed = run_headless_capture(
        str(exe_path),
        "--fractal-type",
        "mandelbrot",
        "--width",
        "192",
        "--height",
        "144",
        "--color-pipeline-action",
        "select_function:source:0:smooth_escape_ramp",
        "--color-pipeline-action",
        "select_function:shape:0:identity",
        "--color-pipeline-action",
        "select_function:palette:0:blackbody_palette_v1",
        "--color-pipeline-action",
        "set_param:palette:0:palette.temperature_0_k:number:2400",
        "--color-pipeline-action",
        "set_param:palette:0:palette.temperature_1_k:number:18000",
        "--color-pipeline-action",
        "set_param:palette:0:palette.temperature_mapping:enum:linear_kelvin",
        "--capture-diagnostic",
    )

    assert changed["frame_hash"] != baseline["frame_hash"], changed
    assert changed["frame_hash"] != default_blackbody["frame_hash"], changed
    state = changed.get("state")
    assert isinstance(state, dict), changed
    assert state.get("fractal_type") == "mandelbrot", state
    palette_row = _palette_row(state)
    assert palette_row.get("function_id") == "blackbody_palette_v1", palette_row
    assert _param(palette_row, "palette.temperature_0_k").get("number_value") == pytest.approx(2400.0)
    assert _param(palette_row, "palette.temperature_1_k").get("number_value") == pytest.approx(18000.0)
    assert _param(palette_row, "palette.temperature_mapping").get("enum_value") == "linear_kelvin"

    params = state.get("params")
    assert isinstance(params, dict), state
    stack = params.get("color_palette_stack")
    assert isinstance(stack, list) and len(stack) == 1, params
    assert stack[0].get("palette") == "blackbody_palette_v1", stack
    assert stack[0].get("blackbody_temperature_0_k") == pytest.approx(2400.0), stack
    assert stack[0].get("blackbody_temperature_1_k") == pytest.approx(18000.0), stack
    assert stack[0].get("blackbody_temperature_mapping") == "linear_kelvin", stack

    state_path = write_state_bundle(tmp_path / "blackbody_non_default", state)
    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / "blackbody_report.json",
        command_path=tmp_path / "blackbody_command.json",
        open_color_pipeline=True,
    ) as viewer:
        for control_id in (
            "color_pipeline.palette.blackbody_palette_v1.palette.temperature_0_k.primary",
            "color_pipeline.palette.blackbody_palette_v1.palette.temperature_1_k.primary",
            "color_pipeline.palette.blackbody_palette_v1.palette.temperature_mapping.primary",
        ):
            viewer.wait_for_control(control_id, timeout_seconds=30.0)
        report = viewer.wait_for_report(timeout_seconds=30.0)
    report_hash = report.get("rendered_frame_hash")
    assert isinstance(report_hash, str) and report_hash.startswith("fnv1a64:"), report
    receipt = report.get("color_pipeline_active_graph_receipt")
    assert isinstance(receipt, dict), report
    nodes = receipt.get("nodes")
    assert isinstance(nodes, list), receipt
    palette_nodes = [
        node
        for node in nodes
        if isinstance(node, dict)
        and node.get("id") == "palette.0"
        and node.get("function_id") == "blackbody_palette_v1"
    ]
    assert len(palette_nodes) == 1, receipt
    report_palette = palette_nodes[0]
    assert _param(report_palette, "palette.temperature_0_k").get("number_value") == pytest.approx(2400.0)
    assert _param(report_palette, "palette.temperature_1_k").get("number_value") == pytest.approx(18000.0)
    assert _param(report_palette, "palette.temperature_mapping").get("enum_value") == "linear_kelvin"

    group = f"pytest_blackbody_palette_{uuid.uuid4().hex}"
    group_root = RUNTIME_DIR.parent / "findings" / group
    shutil.rmtree(group_root, ignore_errors=True)
    try:
        result = subprocess.run(
            [
                str(exe_path),
                "--load-state-json",
                str(state_path),
                "--capture-finding",
                "--finding-group",
                group,
                "--finding-why",
                "Black Body non-default no-mouse replay proof",
            ],
            cwd=str(exe_path.parent),
            text=True,
            capture_output=True,
            check=False,
        )
        assert result.returncode == 0, result.stdout + result.stderr
        finding_state_paths = sorted(group_root.rglob("state.json"))
        assert len(finding_state_paths) == 1, finding_state_paths
        finding_dir = finding_state_paths[0].parent
        finding_state_text = finding_state_paths[0].read_text(encoding="utf-8")
        assert "blackbody_lut_x0" not in finding_state_text
        assert "blackbody_lut_x1" not in finding_state_text
        assert "blackbody_lut_dx" not in finding_state_text

        sidecar = json.loads((finding_dir / "fractal-state.json").read_text(encoding="utf-8"))
        color_pipeline = sidecar.get("color_pipeline")
        assert isinstance(color_pipeline, dict), sidecar
        sidecar_stack = color_pipeline.get("color_palette_stack")
        assert isinstance(sidecar_stack, list) and len(sidecar_stack) == 1, color_pipeline
        assert sidecar_stack[0].get("palette") == "blackbody_palette_v1", sidecar_stack
        assert sidecar_stack[0].get("blackbody_temperature_0_k") == pytest.approx(2400.0), sidecar_stack
        assert sidecar_stack[0].get("blackbody_temperature_1_k") == pytest.approx(18000.0), sidecar_stack
        assert sidecar_stack[0].get("blackbody_temperature_mapping") == "linear_kelvin", sidecar_stack

        replay_a = run_headless_capture(
            str(exe_path),
            "--load-state-json",
            str(finding_state_paths[0]),
            "--capture-diagnostic",
        )
        replay_b = run_headless_capture(
            str(exe_path),
            "--load-state-json",
            str(finding_state_paths[0]),
            "--capture-diagnostic",
        )
        assert replay_b["frame_hash"] == replay_a["frame_hash"], (replay_a, replay_b)
    finally:
        shutil.rmtree(group_root, ignore_errors=True)
