from __future__ import annotations

import copy
import hashlib
import json
from pathlib import Path

import pytest

from tests.runtime_harness import (
    PersistentRuntimeViewerAutomation,
    active_runtime_exe,
    run_headless_capture,
    runtime_automation_lock,
    write_state_bundle,
)
from tests.test_fractal_runtime_color_recipe_qualification import (
    ROOT_GLOW_CANDIDATE_ACTIONS,
    ROOT_GLOW_PERTURBATIONS,
    _capture_with_actions,
    _image_metrics,
    _mean_abs_normalized_rgb_change,
)


@pytest.fixture(autouse=True)
def _serialize_runtime_automation():
    with runtime_automation_lock():
        yield


SCENES = (
    ("magnet_legacy_quartic", "explaino_magnet_root_well", "legacy_quartic_v1", 4),
    ("magnet_regular_11", "explaino_magnet_root_well", "regular_ngon_v1", 11),
    ("mandelbrot_trap_legacy", "explaino_mandelbrot_root_trap", "legacy_quartic_v1", 4),
)

MEASUREMENT_PERTURBATIONS = (
    ("source_scale_plus_25_percent", "set_param:source:0:signal.proximity_scale:number:1.25"),
    ("source_bias_plus_0_25", "set_param:source:0:signal.proximity_bias:number:0.25"),
    ("shape_scale_plus_0_05", "set_param:shape:0:shape.scale:number:0.15"),
    ("shape_bias_plus_0_10", "set_param:shape:0:shape.bias:number:0.25"),
)


def _state_sha256(state: dict[str, object]) -> str:
    return hashlib.sha256(
        json.dumps(state, sort_keys=True, separators=(",", ":")).encode("utf-8")
    ).hexdigest()


def _measurement_from_report(report: dict[str, object]) -> dict[str, object]:
    measurement = report.get("color_source_measurement")
    assert isinstance(measurement, dict), report
    assert measurement.get("schema_id") == "viewer.color_source_measurement.v1", measurement
    assert measurement.get("requested") is True and measurement.get("valid") is True, measurement
    assert measurement.get("source_id") == "root_log_proximity_v1", measurement
    assert measurement.get("shape_id") == "signed_unit_map_v1", measurement
    assert measurement.get("root_pattern_ref") == "dynamics_root_field", measurement
    assert isinstance(measurement.get("root_pattern_hash"), str), measurement
    assert measurement.get("color_metric_arithmetic_tier") == "float32", measurement
    assert measurement.get("fractal_precision_tier") in {"float32", "float64"}, measurement
    raw = measurement.get("source_raw")
    shaped = measurement.get("shape_output")
    assert isinstance(raw, dict) and isinstance(shaped, dict), measurement
    for distribution, expected_range in ((raw, (-16.0, 16.0)), (shaped, (0.0, 1.0))):
        assert distribution.get("finite_count") == 256 * 192, distribution
        assert distribution.get("nonfinite_count") == 0, distribution
        assert distribution.get("histogram_minimum") == expected_range[0], distribution
        assert distribution.get("histogram_maximum") == expected_range[1], distribution
        histogram = distribution.get("histogram_32")
        assert isinstance(histogram, list) and len(histogram) == 32, distribution
        accounted = (
            sum(int(value) for value in histogram)
            + int(distribution.get("below_histogram_range_count", 0))
            + int(distribution.get("above_histogram_range_count", 0))
        )
        assert accounted == distribution["finite_count"], distribution
    assert 0.0 <= float(shaped["minimum"]) <= float(shaped["maximum"]) <= 1.0, shaped
    return measurement


def _report_for_state(
    exe_path: Path,
    state_path: Path,
    tmp_path: Path,
    label: str,
) -> dict[str, object]:
    with PersistentRuntimeViewerAutomation(
        exe_path=exe_path,
        state_path=state_path,
        report_path=tmp_path / f"{label}.report.json",
        command_path=tmp_path / f"{label}.command.json",
        open_color_pipeline=True,
    ) as viewer:
        report = viewer.wait_for_report(timeout_seconds=60.0)
        assert report.get("current_fractal_type") == json.loads(
            state_path.read_text(encoding="utf-8")
        )["fractal_type"], report
        return report


def _candidate_scene(
    exe_path: Path,
    tmp_path: Path,
    label: str,
    fractal_type: str,
    layout: str,
    root_count: int,
) -> tuple[dict[str, object], Path, dict[str, object]]:
    neutral = run_headless_capture(
        str(exe_path),
        "--capture-diagnostic",
        "--fractal-type",
        fractal_type,
        "--width",
        "256",
        "--height",
        "192",
    )
    state = copy.deepcopy(neutral["state"])
    assert isinstance(state, dict)
    render = state.get("render")
    params = state.get("params")
    assert isinstance(render, dict) and isinstance(params, dict), state
    render["width"] = 256
    render["height"] = 192
    render["aa_mode"] = "off"
    params["explaino_generated_root_layout"] = layout
    params["explaino_generated_root_count"] = root_count
    seed_path = write_state_bundle(tmp_path / f"{label}.seed", state)
    candidate = _capture_with_actions(
        exe_path,
        seed_path,
        list(ROOT_GLOW_CANDIDATE_ACTIONS),
    )
    candidate_state = copy.deepcopy(candidate["state"])
    assert isinstance(candidate_state, dict)
    candidate_path = write_state_bundle(
        tmp_path / f"{label}.candidate", candidate_state
    )
    report = _report_for_state(exe_path, candidate_path, tmp_path, label)
    assert isinstance(report.get("rendered_frame_hash"), str), report
    assert str(report["rendered_frame_hash"]).startswith("fnv1a64:"), report
    return candidate, candidate_path, report


def _occupied_histogram_bins(distribution: dict[str, object]) -> int:
    histogram = distribution.get("histogram_32")
    assert isinstance(histogram, list), distribution
    return sum(1 for value in histogram if int(value) > 0)


def test_root_glow_source_measurement_no_mouse(tmp_path: Path) -> None:
    exe_path = active_runtime_exe()
    scene_results: list[dict[str, object]] = []
    baseline_candidate: dict[str, object] | None = None
    baseline_state_path: Path | None = None
    baseline_measurement: dict[str, object] | None = None

    for label, fractal_type, layout, root_count in SCENES:
        candidate, state_path, report = _candidate_scene(
            exe_path, tmp_path, label, fractal_type, layout, root_count
        )
        measurement = _measurement_from_report(report)
        raw = measurement["source_raw"]
        shaped = measurement["shape_output"]
        assert isinstance(raw, dict) and isinstance(shaped, dict)
        image_metrics = _image_metrics(candidate["frame_bytes"])
        qualification_sensitivity: list[dict[str, object]] = []
        for action, actual_value in ROOT_GLOW_PERTURBATIONS:
            perturbed = _capture_with_actions(exe_path, state_path, [action])
            qualification_sensitivity.append(
                {
                    "action": action,
                    "actual_value": actual_value,
                    "mean_abs_normalized_rgb_change": _mean_abs_normalized_rgb_change(
                        candidate["frame_bytes"], perturbed["frame_bytes"]
                    ),
                }
            )
        assert image_metrics["finite_pixel_percentage"] == 100.0, image_metrics
        assert image_metrics["normalized_scalar_proxy_spread"] >= 0.05, image_metrics
        assert image_metrics["occupied_palette_proxy_bins"] >= 8, image_metrics
        assert image_metrics["terminal_palette_proxy_fraction"] < 0.90, image_metrics
        assert all(
            item["mean_abs_normalized_rgb_change"] >= 0.01
            for item in qualification_sensitivity
        ), qualification_sensitivity
        scene_results.append(
            {
                "scene_id": label,
                "fractal_type": fractal_type,
                "layout": layout,
                "root_count": root_count,
                "frame_sha256": candidate["frame_hash"],
                "state_sha256": _state_sha256(candidate["state"]),
                "measurement": measurement,
                "image_metrics": image_metrics,
                "qualification_sensitivity": qualification_sensitivity,
                "raw_p05_p95_spread": float(raw["p95"]) - float(raw["p05"]),
                "shaped_p05_p95_spread": float(shaped["p95"]) - float(shaped["p05"]),
                "raw_occupied_histogram_bins": _occupied_histogram_bins(raw),
                "shaped_occupied_histogram_bins": _occupied_histogram_bins(shaped),
            }
        )
        if label == "magnet_legacy_quartic":
            baseline_candidate = candidate
            baseline_state_path = state_path
            baseline_measurement = measurement

    assert baseline_candidate is not None
    assert baseline_state_path is not None
    assert baseline_measurement is not None

    perturbation_results: list[dict[str, object]] = []
    for label, action in MEASUREMENT_PERTURBATIONS:
        perturbed = _capture_with_actions(exe_path, baseline_state_path, [action])
        perturbed_path = write_state_bundle(
            tmp_path / f"perturbation.{label}", copy.deepcopy(perturbed["state"])
        )
        report = _report_for_state(
            exe_path, perturbed_path, tmp_path, f"perturbation.{label}"
        )
        measurement = _measurement_from_report(report)
        raw = measurement["source_raw"]
        shaped = measurement["shape_output"]
        base_raw = baseline_measurement["source_raw"]
        base_shaped = baseline_measurement["shape_output"]
        assert isinstance(raw, dict) and isinstance(shaped, dict)
        assert isinstance(base_raw, dict) and isinstance(base_shaped, dict)
        perturbation_results.append(
            {
                "id": label,
                "action": action,
                "frame_sha256": perturbed["frame_hash"],
                "mean_abs_normalized_rgb_change": _mean_abs_normalized_rgb_change(
                    baseline_candidate["frame_bytes"], perturbed["frame_bytes"]
                ),
                "raw_mean_delta": float(raw["mean"]) - float(base_raw["mean"]),
                "raw_p50_delta": float(raw["p50"]) - float(base_raw["p50"]),
                "shaped_mean_delta": float(shaped["mean"]) - float(base_shaped["mean"]),
                "shaped_p50_delta": float(shaped["p50"]) - float(base_shaped["p50"]),
                "measurement": measurement,
            }
        )

    raw_spreads = [float(row["raw_p05_p95_spread"]) for row in scene_results]
    shaped_spreads = [float(row["shaped_p05_p95_spread"]) for row in scene_results]
    shaped_occupancy = [int(row["shaped_occupied_histogram_bins"]) for row in scene_results]
    source_bias = next(row for row in perturbation_results if row["id"] == "source_bias_plus_0_25")
    rgb_sensitivities = [
        float(row["mean_abs_normalized_rgb_change"]) for row in perturbation_results
    ]
    raw_metric_is_useful = min(raw_spreads) >= 0.5 and abs(float(source_bias["raw_mean_delta"])) >= 0.05
    sensitive_parameter_count = sum(value >= 0.01 for value in rgb_sensitivities)
    existing_mapping_is_useful = (
        min(shaped_spreads) >= 0.2
        and min(shaped_occupancy) >= 8
        and sensitive_parameter_count >= 2
    )
    if existing_mapping_is_useful:
        classification = "existing_mapping_viable"
    elif raw_metric_is_useful:
        classification = "transfer_mapping_needed"
    else:
        classification = "source_metric_revision_needed"

    artifact = {
        "schema_id": "viewer.root_glow_source_measurement.v1",
        "runtime_exe": str(exe_path),
        "measurement_authority": "renderer_source_signal_sidecar_before_palette_and_grading",
        "scene_contract": {
            "width": 256,
            "height": 192,
            "aa_mode": "off",
            "raw_histogram_range": [-16.0, 16.0],
            "shaped_histogram_range": [0.0, 1.0],
        },
        "decision_thresholds": {
            "minimum_raw_p05_p95_spread": 0.5,
            "minimum_source_bias_raw_mean_delta": 0.05,
            "minimum_shaped_p05_p95_spread": 0.2,
            "minimum_shaped_occupied_bins": 8,
            "minimum_rgb_sensitivity": 0.01,
            "minimum_sensitive_parameter_count": 2,
            "qualification_scene_count": 3,
            "all_visible_controls_must_pass_each_scene": True,
        },
        "classification": classification,
        "scenes": scene_results,
        "perturbations": perturbation_results,
    }
    artifact_path = Path(
        "artifacts/root_glow_source_measurement/root_glow_source_measurement.json"
    )
    artifact_path.parent.mkdir(parents=True, exist_ok=True)
    artifact_path.write_text(json.dumps(artifact, indent=2) + "\n", encoding="utf-8")

    assert classification in {
        "existing_mapping_viable",
        "transfer_mapping_needed",
        "source_metric_revision_needed",
    }, artifact
