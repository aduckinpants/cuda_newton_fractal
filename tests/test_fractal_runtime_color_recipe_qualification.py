from __future__ import annotations

import hashlib
import json
import math
import statistics
import struct
import sys
from dataclasses import dataclass
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


@dataclass(frozen=True)
class RecipeCase:
    recipe_id: str
    fractal_type: str
    expected_rows: tuple[str, ...]
    perturbations: tuple[tuple[str, float], ...]
    timing_baseline_recipe_id: str
    timing_baseline_median_ms: float
    timing_baseline_mad_ms: float


CASES = (
    RecipeCase(
        "lens_topography",
        "mandelbrot",
        (
            "source:lens_field_v2_distance",
            "shape:identity",
            "palette:heatmap",
            "grading:contrast_lift",
        ),
        (
            ("set_param:source:0:signal.sign_contrast:number:0.75", 0.75),
            ("set_param:source:0:signal.bias:number:0.1", 0.1),
            ("set_param:palette:0:palette.cycle_scale:number:1.6", 1.6),
        ),
        "default_smooth_escape",
        16.1378231048584,
        0.31641626358032227,
    ),
    RecipeCase(
        "curvature_relief",
        "mandelbrot",
        (
            "source:sdf_curvature",
            "shape:signed_unit_map_v1",
            "palette:heatmap",
            "grading:contrast_lift",
        ),
        (
            ("set_param:source:0:signal.bias:number:0.1", 0.1),
            ("set_param:shape:0:shape.scale:number:1.65", 1.65),
            ("set_param:palette:0:palette.cycle_scale:number:1.6", 1.6),
        ),
        "default_smooth_escape",
        16.1378231048584,
        0.31641626358032227,
    ),
)

ROOT_GLOW_CANDIDATE_ACTIONS = (
    "select_function:source:0:root_log_proximity_v1",
    "set_param:source:0:signal.proximity_scale:number:1.0",
    "set_param:source:0:signal.proximity_bias:number:0.0",
    "set_param:source:0:signal.root_pattern_ref:enum:dynamics_root_field",
    "set_param:source:0:signal.blend_weight:number:1.0",
    "select_function:shape:0:signed_unit_map_v1",
    "set_param:shape:0:shape.scale:number:0.18",
    "set_param:shape:0:shape.bias:number:0.0",
    "select_function:palette:0:heatmap",
    "set_param:palette:0:palette.cycle_scale:number:1.0",
    "set_param:palette:0:palette.saturation:number:1.0",
    "set_param:palette:0:palette.blend_weight:number:1.0",
    "set_param:palette:0:palette.blend_mode:enum:normal",
    "select_function:grading:0:grade_glow",
    "set_param:grading:0:grade.exposure:number:1.0",
    "set_param:grading:0:grade.saturation:number:1.0",
    "set_param:grading:0:grade.contrast:number:1.0",
    "set_param:grading:0:grade.glow:number:0.45",
)

ROOT_GLOW_PERTURBATIONS = (
    ("set_param:source:0:signal.proximity_bias:number:0.1", 0.1),
    ("set_param:shape:0:shape.scale:number:0.198", 0.198),
    ("set_param:grading:0:grade.glow:number:0.55", 0.55),
)


def _headless_actions_from_graph_receipt(receipt: dict[str, object]) -> list[str]:
    nodes = receipt.get("nodes")
    assert isinstance(nodes, list), receipt
    actions: list[str] = []
    for lane_id in ("source", "shape", "palette", "grading"):
        lane_nodes = sorted(
            (
                node
                for node in nodes
                if isinstance(node, dict)
                and node.get("lane_id") == lane_id
                and node.get("enabled") is True
            ),
            key=lambda node: int(node["row_index"]),
        )
        assert lane_nodes, (lane_id, receipt)
        for row_index, node in enumerate(lane_nodes):
            function_id = node.get("function_id")
            assert isinstance(function_id, str), node
            actions.append(
                f"select_function:{lane_id}:0:{function_id}"
                if row_index == 0
                else f"add_row:{lane_id}:{function_id}"
            )
            params = node.get("params")
            assert isinstance(params, list), node
            for param in params:
                assert isinstance(param, dict), node
                path = param.get("path")
                param_type = param.get("type")
                assert isinstance(path, str) and isinstance(param_type, str), param
                if param_type == "enum":
                    value_kind = "enum"
                    value = param.get("enum_value")
                elif param_type == "bool":
                    value_kind = "bool"
                    value = "true" if param.get("bool_value") is True else "false"
                else:
                    value_kind = "number"
                    value = param.get("number_value")
                actions.append(
                    f"set_param:{lane_id}:{row_index}:{path}:{value_kind}:{value}"
                )
    return actions


def _capture_with_actions(
    exe_path: Path,
    state_path: Path,
    actions: list[str],
) -> dict[str, object]:
    args = [str(exe_path), "--load-state-json", str(state_path)]
    for action in actions:
        args.extend(["--color-pipeline-action", action])
    args.append("--capture-diagnostic")
    return run_headless_capture(*args)


def _decode_bmp_rgb(frame_bytes: bytes) -> tuple[int, int, list[tuple[int, int, int]]]:
    assert frame_bytes[:2] == b"BM"
    pixel_offset = struct.unpack_from("<I", frame_bytes, 10)[0]
    dib_size = struct.unpack_from("<I", frame_bytes, 14)[0]
    assert dib_size >= 40
    width, signed_height = struct.unpack_from("<ii", frame_bytes, 18)
    planes, bits_per_pixel = struct.unpack_from("<HH", frame_bytes, 26)
    compression = struct.unpack_from("<I", frame_bytes, 30)[0]
    assert width > 0 and signed_height != 0 and planes == 1
    assert bits_per_pixel in (24, 32) and compression == 0
    height = abs(signed_height)
    bytes_per_pixel = bits_per_pixel // 8
    row_stride = ((width * bytes_per_pixel + 3) // 4) * 4
    pixels: list[tuple[int, int, int]] = []
    for output_y in range(height):
        source_y = output_y if signed_height < 0 else height - 1 - output_y
        row_start = pixel_offset + source_y * row_stride
        for x in range(width):
            offset = row_start + x * bytes_per_pixel
            blue, green, red = frame_bytes[offset : offset + 3]
            pixels.append((red, green, blue))
    assert len(pixels) == width * height
    return width, height, pixels


def _quantile(values: list[float], q: float) -> float:
    ordered = sorted(values)
    return ordered[round((len(ordered) - 1) * q)]


def _image_metrics(frame_bytes: bytes) -> dict[str, object]:
    width, height, pixels = _decode_bmp_rgb(frame_bytes)
    luminance = [
        (0.2126 * red + 0.7152 * green + 0.0722 * blue) / 255.0
        for red, green, blue in pixels
    ]
    bins = [0] * 32
    for value in luminance:
        bins[min(31, int(value * 32.0))] += 1
    terminal = bins[0] + bins[-1]
    return {
        "width": width,
        "height": height,
        "finite_pixel_percentage": 100.0
        if all(math.isfinite(value) for value in luminance)
        else 0.0,
        "normalized_luminance_p05": _quantile(luminance, 0.05),
        "normalized_luminance_p95": _quantile(luminance, 0.95),
        "normalized_scalar_proxy_spread": _quantile(luminance, 0.95)
        - _quantile(luminance, 0.05),
        "occupied_palette_proxy_bins": sum(1 for count in bins if count > 0),
        "terminal_palette_proxy_fraction": terminal / len(luminance),
        "palette_proxy_histogram_32": bins,
    }


def _mean_abs_normalized_rgb_change(left: bytes, right: bytes) -> float:
    left_width, left_height, left_pixels = _decode_bmp_rgb(left)
    right_width, right_height, right_pixels = _decode_bmp_rgb(right)
    assert (left_width, left_height) == (right_width, right_height)
    total = 0
    for left_pixel, right_pixel in zip(left_pixels, right_pixels, strict=True):
        total += sum(abs(a - b) for a, b in zip(left_pixel, right_pixel, strict=True))
    return total / (len(left_pixels) * 3.0 * 255.0)


def _timing_reference_receipt(
    *,
    measured_median_ms: float,
    baseline_recipe_id: str,
    baseline_median_ms: float,
    baseline_mad_ms: float,
) -> dict[str, object]:
    allowed_fraction = max(0.10, 3.0 * baseline_mad_ms / baseline_median_ms)
    limit_ms = baseline_median_ms * (1.0 + allowed_fraction)
    return {
        "authority": "slice0_same_machine_non_equivalent_reference",
        "comparison_classification": "non_equivalent_reference_only",
        "regression_claim": "not_authorized",
        "baseline_recipe_id": baseline_recipe_id,
        "baseline_median_ms": baseline_median_ms,
        "baseline_mad_ms": baseline_mad_ms,
        "allowed_fraction": allowed_fraction,
        "limit_ms": limit_ms,
    }

def _recipe_capability(report: dict[str, object], recipe_id: str) -> dict[str, object]:
    capability_report = report.get("color_pipeline_recipe_capability_report")
    assert isinstance(capability_report, dict), report
    for item in capability_report.get("recipe_applicability", []):
        if isinstance(item, dict) and item.get("recipe_id") == recipe_id:
            return item
    raise AssertionError((recipe_id, capability_report))


def test_curated_color_recipe_qualification_no_mouse(tmp_path: Path) -> None:
    if sys.platform != "win32":
        pytest.skip("Curated Color Recipe qualification is Windows-only")

    exe_path = active_runtime_exe()
    results: list[dict[str, object]] = []
    for case in CASES:
        neutral_capture = run_headless_capture(
            str(exe_path),
            "--capture-diagnostic",
            "--fractal-type",
            case.fractal_type,
            "--width",
            "256",
            "--height",
            "192",
        )
        seed_state_path = write_state_bundle(
            tmp_path / f"{case.recipe_id}_seed",
            json.loads(json.dumps(neutral_capture["state"])),
        )
        with PersistentRuntimeViewerAutomation(
            exe_path=exe_path,
            state_path=seed_state_path,
            report_path=tmp_path / f"{case.recipe_id}_report.json",
            command_path=tmp_path / f"{case.recipe_id}_command.json",
            open_color_pipeline=True,
        ) as viewer:
            ready = viewer.wait_for_report(timeout_seconds=60.0)
            applicability = _recipe_capability(ready, case.recipe_id)
            assert applicability.get("available") is True, applicability
            selected = viewer.click_control(
                f"color_pipeline.recipe.{case.recipe_id}.select",
                timeout_seconds=60.0,
            )
            assert selected.get("click_consumed") is True, selected
            applied = viewer.click_control(
                "color_pipeline.recipe.apply_selected",
                timeout_seconds=60.0,
            )
            assert applied.get("click_consumed") is True, applied
            settled = viewer.click_control("render_once", timeout_seconds=60.0)
            assert settled.get("click_consumed") is True, settled
            unrelated = viewer.click_control(
                "color_pipeline.recipe.default_smooth_escape.select",
                timeout_seconds=60.0,
            )
            assert unrelated.get("click_consumed") is True, unrelated
            assert unrelated.get("rendered_frame_hash") == settled.get(
                "rendered_frame_hash"
            ), unrelated

        assert tuple(settled.get("lane_rows", [])) == case.expected_rows, settled
        application_report = settled.get("color_pipeline_recipe_application_report")
        assert isinstance(application_report, dict), settled
        assert application_report.get("current_recipe_match") == "exact", application_report
        receipt = application_report.get("receipt")
        assert isinstance(receipt, dict), application_report
        assert receipt.get("recipe_id") == case.recipe_id, receipt
        assert receipt.get("application_authority") == "recipe_v2_graph", receipt
        assert receipt.get("fallback_active") is False, receipt
        assert receipt.get("committed_row_fingerprint") == receipt.get(
            "committed_live_row_fingerprint"
        ), receipt
        assert receipt.get("committed_rows") == receipt.get("committed_live_rows"), receipt

        capability_report = settled.get("color_pipeline_recipe_capability_report")
        assert isinstance(capability_report, dict), settled
        snapshot = capability_report.get("snapshot")
        assert isinstance(snapshot, dict), capability_report
        assert snapshot.get("color_metric_arithmetic_tier") == "float32", snapshot
        assert isinstance(snapshot.get("evaluator_id"), str), snapshot
        if case.recipe_id == "root_glow":
            assert settled.get("root_patterns"), settled
            assert settled.get("root_pattern_consumers"), settled

        graph_receipt = settled.get("color_pipeline_graph_receipt")
        assert isinstance(graph_receipt, dict), settled
        recipe_actions = _headless_actions_from_graph_receipt(graph_receipt)
        baseline = _capture_with_actions(exe_path, seed_state_path, recipe_actions)
        applied_state_path = write_state_bundle(
            tmp_path / f"{case.recipe_id}_applied",
            json.loads(json.dumps(baseline["state"])),
        )
        replay = run_headless_capture(
            str(exe_path),
            "--load-state-json",
            str(applied_state_path),
            "--capture-diagnostic",
        )
        stationary = run_headless_capture(
            str(exe_path),
            "--load-state-json",
            str(applied_state_path),
            "--capture-diagnostic",
        )
        assert replay["frame_hash"] == baseline["frame_hash"], (baseline, replay)
        assert stationary["frame_hash"] == baseline["frame_hash"], stationary
        assert _mean_abs_normalized_rgb_change(
            baseline["frame_bytes"], stationary["frame_bytes"]
        ) == 0.0

        sensitivity: list[dict[str, object]] = []
        for action, actual_value in case.perturbations:
            perturbed = _capture_with_actions(exe_path, applied_state_path, [action])
            delta = _mean_abs_normalized_rgb_change(
                baseline["frame_bytes"], perturbed["frame_bytes"]
            )
            sensitivity.append(
                {
                    "action": action,
                    "actual_value": actual_value,
                    "mean_abs_normalized_rgb_change": delta,
                }
            )

        for _ in range(5):
            warmup = run_headless_capture(
                str(exe_path),
                "--load-state-json",
                str(applied_state_path),
                "--capture-diagnostic",
            )
            assert warmup["frame_hash"] == baseline["frame_hash"], warmup
        timing_values: list[float] = []
        for _ in range(20):
            measured = run_headless_capture(
                str(exe_path),
                "--load-state-json",
                str(applied_state_path),
                "--capture-diagnostic",
            )
            assert measured["frame_hash"] == baseline["frame_hash"], measured
            stats = measured["state"].get("stats")
            assert isinstance(stats, dict), measured
            timing_values.append(float(stats["last_render_ms"]))

        metrics = _image_metrics(baseline["frame_bytes"])
        assert metrics["finite_pixel_percentage"] == 100.0, metrics
        assert metrics["normalized_scalar_proxy_spread"] >= 0.05, metrics
        assert metrics["occupied_palette_proxy_bins"] >= 8, metrics
        assert metrics["terminal_palette_proxy_fraction"] < 0.90, metrics
        assert all(
            item["mean_abs_normalized_rgb_change"] >= 0.01
            for item in sensitivity
        ), sensitivity

        ordered_timing = sorted(timing_values)
        timing_comparison = _timing_reference_receipt(
            measured_median_ms=statistics.median(timing_values),
            baseline_recipe_id=case.timing_baseline_recipe_id,
            baseline_median_ms=case.timing_baseline_median_ms,
            baseline_mad_ms=case.timing_baseline_mad_ms,
        )
        results.append(
            {
                "recipe_id": case.recipe_id,
                "fractal_type": case.fractal_type,
                "classification": "enabled",
                "frame_sha256": baseline["frame_hash"],
                "state_sha256": hashlib.sha256(
                    json.dumps(
                        baseline["state"], sort_keys=True, separators=(",", ":")
                    ).encode("utf-8")
                ).hexdigest(),
                "metrics": metrics,
                "sensitivity": sensitivity,
                "stationary_mean_abs_normalized_rgb_change": 0.0,
                "unrelated_ui_frame_hash_unchanged": True,
                "capability_snapshot": snapshot,
                "application_receipt": receipt,
                "timing_ms": {
                    "warmups": 5,
                    "samples": 20,
                    "median": statistics.median(timing_values),
                    "p95": ordered_timing[18],
                    "values": timing_values,
                    "comparison": timing_comparison,
                },
            }
        )

    root_neutral = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        "explaino_magnet_root_well",
        "--width",
        "256",
        "--height",
        "192",
    )
    root_seed_state_path = write_state_bundle(
        tmp_path / "root_glow_seed",
        json.loads(json.dumps(root_neutral["state"])),
    )
    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=root_seed_state_path,
        report_path=tmp_path / "root_glow_report.json",
        command_path=tmp_path / "root_glow_command.json",
        open_color_pipeline=True,
    ) as viewer:
        root_ready = viewer.wait_for_report(timeout_seconds=60.0)
        root_applicability = _recipe_capability(root_ready, "root_glow")
        assert root_applicability.get("available") is False, root_applicability
        assert root_applicability.get("reason_code") == "recipe_qualification_failed", root_applicability
        assert root_applicability.get("missing_capability_ids") == [], root_applicability
        assert root_ready.get("root_patterns"), root_ready
        assert root_ready.get("root_pattern_consumers"), root_ready
        root_selected = viewer.click_control(
            "color_pipeline.recipe.root_glow.select",
            timeout_seconds=60.0,
        )
        assert root_selected.get("click_consumed") is True, root_selected
        root_rejected = viewer.click_control(
            "color_pipeline.recipe.apply_selected",
            timeout_seconds=60.0,
        )
        assert root_rejected.get("click_consumed") is True, root_rejected
        assert root_rejected.get("rendered_frame_hash") == root_ready.get(
            "rendered_frame_hash"
        ), root_rejected
        assert root_rejected.get("lane_rows") == root_ready.get("lane_rows"), root_rejected
        assert any(
            "recipe_qualification_failed" in message
            for message in root_rejected.get("validation_messages", [])
        ), root_rejected

    root_candidate = _capture_with_actions(
        exe_path,
        root_seed_state_path,
        list(ROOT_GLOW_CANDIDATE_ACTIONS),
    )
    root_candidate_state_path = write_state_bundle(
        tmp_path / "root_glow_candidate",
        json.loads(json.dumps(root_candidate["state"])),
    )
    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=root_candidate_state_path,
        report_path=tmp_path / "root_glow_candidate_report.json",
        command_path=tmp_path / "root_glow_candidate_command.json",
        open_color_pipeline=True,
    ) as viewer:
        root_candidate_report = viewer.wait_for_report(timeout_seconds=60.0)
        assert any(
            consumer.get("consumer_kind") == "color_source_row"
            and consumer.get("consumer_id") == "root_log_proximity_v1"
            and consumer.get("pattern_ref") == "dynamics_root_field"
            for consumer in root_candidate_report.get("root_pattern_consumers", [])
            if isinstance(consumer, dict)
        ), root_candidate_report
    root_replay = run_headless_capture(
        str(exe_path),
        "--load-state-json",
        str(root_candidate_state_path),
        "--capture-diagnostic",
    )
    assert root_replay["frame_hash"] == root_candidate["frame_hash"], root_replay

    root_sensitivity: list[dict[str, object]] = []
    for action, actual_value in ROOT_GLOW_PERTURBATIONS:
        perturbed = _capture_with_actions(exe_path, root_candidate_state_path, [action])
        root_sensitivity.append(
            {
                "action": action,
                "actual_value": actual_value,
                "mean_abs_normalized_rgb_change": _mean_abs_normalized_rgb_change(
                    root_candidate["frame_bytes"], perturbed["frame_bytes"]
                ),
            }
        )
    root_metrics = _image_metrics(root_candidate["frame_bytes"])
    assert root_metrics["finite_pixel_percentage"] == 100.0, root_metrics
    assert root_metrics["normalized_scalar_proxy_spread"] >= 0.05, root_metrics
    assert root_metrics["terminal_palette_proxy_fraction"] < 0.90, root_metrics
    assert root_metrics["occupied_palette_proxy_bins"] < 8, root_metrics
    assert any(
        item["mean_abs_normalized_rgb_change"] < 0.01
        for item in root_sensitivity
    ), root_sensitivity

    for _ in range(5):
        root_warmup = run_headless_capture(
            str(exe_path),
            "--load-state-json",
            str(root_candidate_state_path),
            "--capture-diagnostic",
        )
        assert root_warmup["frame_hash"] == root_candidate["frame_hash"], root_warmup
    root_timing_values: list[float] = []
    for _ in range(20):
        root_measured = run_headless_capture(
            str(exe_path),
            "--load-state-json",
            str(root_candidate_state_path),
            "--capture-diagnostic",
        )
        assert root_measured["frame_hash"] == root_candidate["frame_hash"], root_measured
        root_stats = root_measured["state"].get("stats")
        assert isinstance(root_stats, dict), root_measured
        root_timing_values.append(float(root_stats["last_render_ms"]))
    ordered_root_timing = sorted(root_timing_values)
    root_timing_comparison = _timing_reference_receipt(
        measured_median_ms=statistics.median(root_timing_values),
        baseline_recipe_id="root_proximity_heatmap",
        baseline_median_ms=14.236847877502441,
        baseline_mad_ms=0.21484804153442383,
    )
    root_capability_report = root_ready.get("color_pipeline_recipe_capability_report")
    assert isinstance(root_capability_report, dict), root_ready
    root_snapshot = root_capability_report.get("snapshot")
    assert isinstance(root_snapshot, dict), root_capability_report
    results.append(
        {
            "recipe_id": "root_glow",
            "fractal_type": "explaino_magnet_root_well",
            "classification": "visible_disabled_recipe_specific",
            "qualification_reason_code": "recipe_qualification_failed",
            "candidate_measurement_authority": "locked_recipe_rows_v1",
            "frame_sha256": root_candidate["frame_hash"],
            "state_sha256": hashlib.sha256(
                json.dumps(
                    root_candidate["state"], sort_keys=True, separators=(",", ":")
                ).encode("utf-8")
            ).hexdigest(),
            "metrics": root_metrics,
            "sensitivity": root_sensitivity,
            "stationary_mean_abs_normalized_rgb_change": 0.0,
            "unavailable_apply_preserved_frame_and_rows": True,
            "capability_snapshot": root_snapshot,
            "applicability": root_applicability,
            "timing_ms": {
                "warmups": 5,
                "samples": 20,
                "median": statistics.median(root_timing_values),
                "p95": ordered_root_timing[18],
                "values": root_timing_values,
                "comparison": root_timing_comparison,
            },
        }
    )

    artifact_path = Path(
        "artifacts/curated_color_recipe_authority_campaign/qualification/curated_recipes.json"
    )
    artifact_path.parent.mkdir(parents=True, exist_ok=True)
    artifact_path.write_text(
        json.dumps(
            {
                "schema_id": "viewer.curated_color_recipe_qualification.v1",
                "runtime_exe": str(exe_path),
                "observable_scalar_contract": (
                    "normalized Rec.709 luminance is the bounded image-domain scalar proxy; "
                    "source-field scalar export is not added by this slice"
                ),
                "cases": results,
            },
            indent=2,
        )
        + "\n",
        encoding="utf-8",
    )
