from __future__ import annotations

import copy
import hashlib
import os
import json
import shutil
import subprocess
import sys
import tempfile
import uuid
from pathlib import Path
from typing import Any

import pytest
from PIL import Image

from tests.runtime_harness import (
    PersistentRuntimeViewerAutomation,
    RUNTIME_DIR,
    active_runtime_exe,
    run_headless_capture,
    runtime_automation_lock,
    write_state_bundle,
)
from tests.test_fractal_runtime_color_recipe_qualification import (
    _capture_with_actions,
    _decode_bmp_rgb,
    _headless_actions_from_graph_receipt,
    _image_metrics,
    _mean_abs_normalized_rgb_change,
)
from tools.blackbody_palette_evidence import verify_preservation


@pytest.fixture(autouse=True)
def _serialize_runtime_automation():
    with runtime_automation_lock():
        yield


REPO_ROOT = Path(__file__).resolve().parents[1]
ARTIFACT_ROOT = REPO_ROOT / "artifacts/blackbody_chromaticity_palette"
PRESERVATION_ARTIFACT = ARTIFACT_ROOT / "preservation/existing_recipe_parity.json"

RECIPE_ID = "blackbody_thermal_ridges"
EXPECTED_ROWS = (
    "source:smooth_escape_ramp",
    "shape:mirror_repeat",
    "palette:blackbody_palette_v1",
    "grading:contrast_lift",
)
EXPECTED_NODE_IDS = (
    "source.escape_signal",
    "shape.thermal_ridges",
    "palette.blackbody",
    "grading.thermal_finish",
)

QUALIFICATION_PERTURBATIONS = (
    ("shape_frequency_plus_10_percent", "set_param:shape:0:shape.frequency:number:4.4"),
    ("shape_phase_plus_0_10", "set_param:shape:0:shape.phase:number:0.1"),
    ("temperature_0_plus_10_percent", "set_param:palette:0:palette.temperature_0_k:number:1760"),
    ("temperature_1_plus_20_percent", "set_param:palette:0:palette.temperature_1_k:number:14400"),
    ("mapping_linear_kelvin", "set_param:palette:0:palette.temperature_mapping:enum:linear_kelvin"),
    ("exposure_plus_0_10", "set_param:grading:0:grade.exposure:number:1.1"),
    ("saturation_plus_0_10", "set_param:grading:0:grade.saturation:number:1.1"),
)


def _sha256_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def _state_sha256(state: dict[str, Any]) -> str:
    return hashlib.sha256(
        json.dumps(state, sort_keys=True, separators=(",", ":")).encode("utf-8")
    ).hexdigest()


def _configure_render_size(state: dict[str, Any], width: int, height: int) -> None:
    render = state.get("render")
    assert isinstance(render, dict), state
    render["width"] = width
    render["height"] = height
    render["aa_mode"] = "off"
    view = state.get("view")
    assert isinstance(view, dict), state
    view["auto_max_iter"] = False
    params = state.get("params")
    assert isinstance(params, dict), state
    params["max_iter"] = 256


def _new_mandelbrot_state(exe_path: Path, width: int, height: int) -> dict[str, Any]:
    capture = run_headless_capture(
        str(exe_path),
        "--fractal-type",
        "mandelbrot",
        "--width",
        str(width),
        "--height",
        str(height),
        "--capture-diagnostic",
    )
    state = copy.deepcopy(capture["state"])
    assert isinstance(state, dict)
    _configure_render_size(state, width, height)
    return state


def _apply_public_recipe(
    viewer: PersistentRuntimeViewerAutomation,
    recipe_id: str,
    *,
    timeout_seconds: float = 90.0,
) -> dict[str, Any]:
    selected = viewer.click_control(
        f"color_pipeline.recipe.{recipe_id}.select", timeout_seconds=timeout_seconds
    )
    assert selected.get("click_consumed") is True, selected
    applied = viewer.click_control(
        "color_pipeline.recipe.apply_selected", timeout_seconds=timeout_seconds
    )
    assert applied.get("click_consumed") is True, applied
    settled = viewer.click_control("render_once", timeout_seconds=timeout_seconds)
    assert settled.get("click_consumed") is True, settled
    return settled


def _param(row: dict[str, Any], path: str) -> dict[str, Any]:
    params = row.get("parameter_values")
    if not isinstance(params, list):
        params = row.get("params")
    assert isinstance(params, list), row
    for param in params:
        if isinstance(param, dict) and param.get("path") == path:
            return param
    raise AssertionError((path, row))


def _write_png(frame_bytes: bytes, path: Path) -> None:
    width, height, pixels = _decode_bmp_rgb(frame_bytes)
    path.parent.mkdir(parents=True, exist_ok=True)
    image = Image.new("RGB", (width, height))
    image.putdata(pixels)
    image.save(path, format="PNG", optimize=False)


def _recipe_actions(shape_id: str, t0: float, t1: float) -> list[str]:
    return [
        "select_function:source:0:smooth_escape_ramp",
        "set_param:source:0:signal.scale:number:1",
        "set_param:source:0:signal.bias:number:0",
        f"select_function:shape:0:{shape_id}",
        "set_param:shape:0:shape.frequency:number:4",
        "set_param:shape:0:shape.phase:number:0",
        "select_function:palette:0:blackbody_palette_v1",
        f"set_param:palette:0:palette.temperature_0_k:number:{t0}",
        f"set_param:palette:0:palette.temperature_1_k:number:{t1}",
        "set_param:palette:0:palette.temperature_mapping:enum:reciprocal_temperature",
        "select_function:grading:0:contrast_lift",
        "set_param:grading:0:grade.exposure:number:1",
        "set_param:grading:0:grade.saturation:number:1",
    ]


def _run_audition(exe_path: Path, state_path: Path) -> dict[str, Any]:
    cases = (
        ("repeat_forward", "repeat", 1600.0, 12000.0),
        ("repeat_reverse", "repeat", 12000.0, 1600.0),
        ("mirror_repeat_forward", "mirror_repeat", 1600.0, 12000.0),
        ("mirror_repeat_reverse", "mirror_repeat", 12000.0, 1600.0),
    )
    rows: list[dict[str, Any]] = []
    for case_id, shape_id, t0, t1 in cases:
        capture = _capture_with_actions(exe_path, state_path, _recipe_actions(shape_id, t0, t1))
        png_path = ARTIFACT_ROOT / "audition" / f"{case_id}.png"
        _write_png(capture["frame_bytes"], png_path)
        rows.append(
            {
                "id": case_id,
                "shape_id": shape_id,
                "temperature_0_k": t0,
                "temperature_1_k": t1,
                "mapping": "reciprocal_temperature",
                "frame_sha256": capture["frame_hash"],
                "state_sha256": _state_sha256(capture["state"]),
                "png_file": png_path.relative_to(REPO_ROOT).as_posix(),
                "png_sha256": _sha256_file(png_path),
                "image_metrics": _image_metrics(capture["frame_bytes"]),
            }
        )
    assert len({row["frame_sha256"] for row in rows}) == len(rows), rows
    artifact = {
        "schema_id": "viewer.blackbody_recipe_audition.v1",
        "classification": "evidence_only_recipe_unchanged",
        "fractal_type": "mandelbrot",
        "precision": "float32",
        "backend": "cuda_direct",
        "resolution": [1024, 768],
        "locked_recipe_choice": "mirror_repeat_forward",
        "cases": rows,
    }
    path = ARTIFACT_ROOT / "audition/blackbody_recipe_audition.json"
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(artifact, indent=2) + "\n", encoding="utf-8")
    return artifact


def _assert_application_report(report: dict[str, Any]) -> dict[str, Any]:
    assert tuple(report.get("lane_rows", [])) == EXPECTED_ROWS, report
    application = report.get("color_pipeline_recipe_application_report")
    assert isinstance(application, dict), report
    assert application.get("last_recipe_application_request") == RECIPE_ID, application
    assert application.get("current_recipe_match") == "exact", application
    receipt = application.get("receipt")
    assert isinstance(receipt, dict), application
    assert receipt.get("recipe_id") == RECIPE_ID, receipt
    assert receipt.get("recipe_version") == 1, receipt
    assert receipt.get("application_authority") == "recipe_v2_graph", receipt
    assert receipt.get("fallback_active") is False, receipt
    assert tuple(receipt.get("semantic_node_ids", [])) == EXPECTED_NODE_IDS, receipt
    assert receipt.get("committed_row_fingerprint") == receipt.get(
        "committed_live_row_fingerprint"
    ), receipt
    assert receipt.get("committed_rows") == receipt.get("committed_live_rows"), receipt
    assert receipt.get("approved_adapters") in ([], None), receipt
    return receipt


def _assert_measurement(report: dict[str, Any]) -> dict[str, Any]:
    measurement = report.get("color_source_measurement")
    assert isinstance(measurement, dict), report
    assert measurement.get("schema_id") == "viewer.color_source_measurement.v1", measurement
    assert measurement.get("requested") is True, measurement
    assert measurement.get("valid") is True, measurement
    assert measurement.get("source_id") == "smooth_escape", measurement
    graph_receipt = report.get("color_pipeline_graph_receipt")
    assert isinstance(graph_receipt, dict), report
    source_nodes = [
        node
        for node in graph_receipt.get("nodes", [])
        if isinstance(node, dict) and node.get("lane_id") == "source"
    ]
    assert len(source_nodes) == 1, graph_receipt
    assert source_nodes[0].get("function_id") == "smooth_escape_ramp", source_nodes[0]
    assert measurement.get("shape_id") == "mirror_repeat", measurement
    assert measurement.get("root_pattern_ref") == "none", measurement
    assert measurement.get("root_pattern_hash") is None, measurement
    shaped = measurement.get("shape_output")
    assert isinstance(shaped, dict), measurement
    assert int(shaped["finite_count"]) > 0, shaped
    assert int(shaped["nonfinite_count"]) == 0, shaped
    assert float(shaped["p95"]) - float(shaped["p05"]) >= 0.20, shaped
    histogram = shaped.get("histogram_32")
    assert isinstance(histogram, list) and len(histogram) == 32, shaped
    assert sum(int(count) > 0 for count in histogram) >= 8, shaped
    return measurement


def test_blackbody_thermal_ridges_public_apply_capture_replay_and_evidence(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Black Body recipe published-runtime proof is Windows-only")

    exe_path = active_runtime_exe()
    state = _new_mandelbrot_state(exe_path, 1024, 768)
    seed_path = write_state_bundle(tmp_path / "thermal_ridges_seed", state)
    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=seed_path,
        report_path=tmp_path / "thermal_ridges_report.json",
        command_path=tmp_path / "thermal_ridges_command.json",
        open_color_pipeline=True,
    ) as viewer:
        ready = viewer.wait_for_report(timeout_seconds=90.0)
        applicability = next(
            item
            for item in ready["color_pipeline_recipe_capability_report"]["recipe_applicability"]
            if item["recipe_id"] == RECIPE_ID
        )
        assert applicability.get("available") is True, applicability
        settled = _apply_public_recipe(viewer, RECIPE_ID)
        baseline_public_hash = settled["rendered_frame_hash"]
        receipt = _assert_application_report(settled)
        measurement = _assert_measurement(settled)
        unrelated = viewer.click_control(
            "color_pipeline.recipe.default_smooth_escape.select", timeout_seconds=90.0
        )
        assert unrelated["rendered_frame_hash"] == baseline_public_hash, unrelated

    graph_receipt = settled.get("color_pipeline_graph_receipt")
    assert isinstance(graph_receipt, dict), settled
    actions = _headless_actions_from_graph_receipt(graph_receipt)
    baseline = _capture_with_actions(exe_path, seed_path, actions)
    applied_state_path = write_state_bundle(
        tmp_path / "thermal_ridges_applied", copy.deepcopy(baseline["state"])
    )
    replay_a = run_headless_capture(
        str(exe_path), "--load-state-json", str(applied_state_path), "--capture-diagnostic"
    )
    replay_b = run_headless_capture(
        str(exe_path), "--load-state-json", str(applied_state_path), "--capture-diagnostic"
    )
    assert replay_a["frame_hash"] == baseline["frame_hash"], replay_a
    assert replay_b["frame_hash"] == baseline["frame_hash"], replay_b
    assert _mean_abs_normalized_rgb_change(baseline["frame_bytes"], replay_b["frame_bytes"]) == 0.0

    sensitivity: list[dict[str, Any]] = []
    for perturbation_id, action in QUALIFICATION_PERTURBATIONS:
        changed = _capture_with_actions(exe_path, applied_state_path, [action])
        delta = _mean_abs_normalized_rgb_change(baseline["frame_bytes"], changed["frame_bytes"])
        sensitivity.append(
            {
                "id": perturbation_id,
                "action": action,
                "frame_sha256": changed["frame_hash"],
                "mean_abs_normalized_rgb_change": delta,
            }
        )
    assert all(row["mean_abs_normalized_rgb_change"] >= 0.01 for row in sensitivity), sensitivity

    metrics = _image_metrics(baseline["frame_bytes"])
    assert metrics["finite_pixel_percentage"] == 100.0, metrics
    audition = _run_audition(exe_path, seed_path)

    group = f"pytest_blackbody_thermal_ridges_{uuid.uuid4().hex}"
    group_root = RUNTIME_DIR.parent / "findings" / group
    shutil.rmtree(group_root, ignore_errors=True)
    try:
        result = subprocess.run(
            [
                str(exe_path),
                "--load-state-json",
                str(applied_state_path),
                "--capture-finding",
                "--finding-group",
                group,
                "--finding-why",
                "Black Body Thermal Ridges graph recipe qualification",
            ],
            cwd=str(exe_path.parent),
            text=True,
            capture_output=True,
            check=False,
        )
        assert result.returncode == 0, result.stdout + result.stderr
        finding_states = sorted(group_root.rglob("state.json"))
        assert len(finding_states) == 1, finding_states
        finding_state = finding_states[0]
        sidecar = json.loads((finding_state.parent / "fractal-state.json").read_text(encoding="utf-8"))
        color_pipeline = sidecar.get("color_pipeline")
        assert isinstance(color_pipeline, dict), sidecar
        palette_stack = color_pipeline.get("color_palette_stack")
        assert isinstance(palette_stack, list) and len(palette_stack) == 1, color_pipeline
        assert palette_stack[0]["palette"] == "blackbody_palette_v1", palette_stack
        assert palette_stack[0]["blackbody_temperature_0_k"] == pytest.approx(1600.0)
        assert palette_stack[0]["blackbody_temperature_1_k"] == pytest.approx(12000.0)
        assert palette_stack[0]["blackbody_temperature_mapping"] == "reciprocal_temperature"
        assert isinstance(color_pipeline.get("graph_receipt"), dict), color_pipeline
        finding_replay_a = run_headless_capture(
            str(exe_path), "--load-state-json", str(finding_state), "--capture-diagnostic"
        )
        finding_replay_b = run_headless_capture(
            str(exe_path), "--load-state-json", str(finding_state), "--capture-diagnostic"
        )
        assert finding_replay_a["frame_hash"] == finding_replay_b["frame_hash"]
        replay_width, replay_height, replay_pixels = _decode_bmp_rgb(
            finding_replay_a["frame_bytes"]
        )
        replay_image = Image.new("RGB", (replay_width, replay_height))
        replay_image.putdata(replay_pixels)
        with Image.open(finding_state.parent / "frame.png") as archived_frame:
            archived_rgb = archived_frame.convert("RGB")
            assert archived_rgb.size == (replay_width, replay_height)
            assert archived_rgb.tobytes() == replay_image.tobytes()
    finally:
        shutil.rmtree(group_root, ignore_errors=True)

    verify_preservation(PRESERVATION_ARTIFACT, exe_path)
    qualification = {
        "schema_id": "viewer.blackbody_thermal_ridges_qualification.v1",
        "classification": "passed",
        "runtime_exe": str(exe_path),
        "runtime_exe_sha256": _sha256_file(exe_path),
        "fractal_type": "mandelbrot",
        "resolution": [1024, 768],
        "precision": "float32",
        "backend": "cuda_direct",
        "recipe_id": RECIPE_ID,
        "public_frame_sha256": baseline_public_hash,
        "headless_frame_sha256": baseline["frame_hash"],
        "stationary_mean_abs_normalized_rgb_change": 0.0,
        "unrelated_recipe_selection_changed_pixels": False,
        "application_receipt": receipt,
        "source_measurement": measurement,
        "image_metrics": metrics,
        "sensitivity": sensitivity,
        "capture_replay_frame_sha256": baseline["frame_hash"],
        "preservation_artifact": PRESERVATION_ARTIFACT.relative_to(REPO_ROOT).as_posix(),
        "audition_artifact": "artifacts/blackbody_chromaticity_palette/audition/blackbody_recipe_audition.json",
        "audition_case_count": len(audition["cases"]),
    }
    artifact_path = ARTIFACT_ROOT / "qualification/blackbody_qualification.json"
    artifact_path.parent.mkdir(parents=True, exist_ok=True)
    artifact_path.write_text(json.dumps(qualification, indent=2) + "\n", encoding="utf-8")


def test_blackbody_palette_interleaved_performance(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Black Body performance proof is Windows-only")

    test_root = Path(
        os.environ.get("SALT_FRACTAL_ROOT", r"D:\salt-fractal")
    ) / "cuda_newton_fractal_clone"
    benchmark_exe = test_root / "build_tests/test_blackbody_palette_cuda.exe"
    assert benchmark_exe.exists(), benchmark_exe

    artifact_path = ARTIFACT_ROOT / "performance/blackbody_performance.json"
    artifact_path.parent.mkdir(parents=True, exist_ok=True)
    result = subprocess.run(
        [str(benchmark_exe), "--benchmark-json", str(artifact_path)],
        cwd=REPO_ROOT,
        capture_output=True,
        text=True,
        timeout=180.0,
        check=False,
    )
    assert result.returncode == 0, {
        "returncode": result.returncode,
        "stdout": result.stdout,
        "stderr": result.stderr,
    }
    artifact = json.loads(artifact_path.read_text(encoding="utf-8"))
    assert artifact.get("schema_id") == "viewer.blackbody_palette_interleaved_performance.v1", artifact
    assert artifact.get("authority") == "cuda_event_actual_device_palette_evaluator", artifact
    assert artifact.get("classification") == "proven", artifact
    assert artifact.get("claim") == (
        "bounded incremental device color-path cost only; no viewer FPS or performance improvement claim"
    ), artifact
    sizes = artifact.get("sizes")
    assert isinstance(sizes, list) and len(sizes) == 2, artifact
    assert [row.get("resolution") for row in sizes] == [[1024, 768], [2048, 1536]], sizes
    assert all(row.get("classification") == "proven" for row in sizes), sizes