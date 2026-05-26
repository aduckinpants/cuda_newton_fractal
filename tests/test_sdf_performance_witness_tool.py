from __future__ import annotations

from pathlib import Path

from tools import viewer_host_sdf_performance_witness as witness


def test_repo_absolute_path_resolves_relative_paths_from_repo_root() -> None:
    resolved = witness.repo_absolute_path(Path("artifacts/sdf_performance_witness/report.json"))

    assert resolved.is_absolute()
    assert resolved == witness.REPO_ROOT / "artifacts" / "sdf_performance_witness" / "report.json"


def _payload(
    *,
    name: str,
    field_ms: float,
    postprocess_ms: float,
    total_ms: float | None = None,
    preview: bool = False,
    pixel_step: int = 1,
    direct_samples: int = 1000,
    neighborhood_samples: int = 0,
) -> dict[str, object]:
    sdf_total = field_ms + postprocess_ms if total_ms is None else total_ms
    return {
        "current_fractal_type": "multibrot",
        "rendered_frame_ready": True,
        "rendered_frame_hash": f"fnv1a64:{name:0>16}"[-24:],
        "rendered_frame_width": 320,
        "rendered_frame_height": 240,
        "target_render_width": 320,
        "target_render_height": 240,
        "base_render_ms": 4.0,
        "lens_sdf_valid": True,
        "lens_sdf_color_pipeline_active": True,
        "lens_sdf_field_ms": field_ms,
        "lens_sdf_postprocess_ms": postprocess_ms,
        "lens_sdf_total_ms": sdf_total,
        "last_render_ms": 4.0 + sdf_total,
        "lens_sdf_width": 320,
        "lens_sdf_height": 240,
        "lens_sdf_pixel_scale": 1.0,
        "lens_sdf_postprocess_pixel_step": pixel_step,
        "lens_sdf_postprocess_direct_sample_count": direct_samples,
        "lens_sdf_postprocess_neighborhood_sample_count": neighborhood_samples,
        "lens_sdf_postprocess_filled_pixel_count": 320 * 240,
        "render_pacing_preview_active": preview,
        "render_pacing_preview_scale": 0.5 if preview else 1.0,
    }


def test_sdf_measurement_report_classifies_field_and_postprocess_pressure() -> None:
    measurements = [
        witness.measurement_from_payload(
            "field_heavy",
            "full_quality",
            _payload(name="field", field_ms=12.0, postprocess_ms=2.0),
            source_stack=["sdf_signed_distance"],
            lens_downsample=1,
        ),
        witness.measurement_from_payload(
            "postprocess_heavy",
            "full_quality",
            _payload(name="post", field_ms=1.0, postprocess_ms=18.0, neighborhood_samples=4000),
            source_stack=["sdf_normal_angle", "sdf_curvature"],
            lens_downsample=1,
        ),
        witness.measurement_from_payload(
            "preview_heavy",
            "preview",
            _payload(name="preview", field_ms=1.0, postprocess_ms=5.0, preview=True, pixel_step=3),
            source_stack=["sdf_normal_angle"],
            lens_downsample=1,
        ),
    ]

    report = witness.build_measurement_report(
        measurements,
        runtime_exe="D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe",
        persistent_viewer_launch_count=1,
    )

    assert report["schema_version"] == 1
    assert report["no_mouse_automation"] is True
    assert report["persistent_viewer_launch_count"] == 1
    assert report["summary"]["scenario_count"] == 3
    assert report["summary"]["preview_sample_count"] == 1
    assert report["summary"]["bottleneck_votes"]["field_generation_pressure"] == 1
    assert report["summary"]["bottleneck_votes"]["postprocess_pressure"] == 1
    assert report["summary"]["bottleneck_votes"]["preview_quality_sample"] == 1
    assert report["scenarios"][0]["classification"] == "field_generation_pressure"
    assert report["scenarios"][1]["classification"] == "postprocess_pressure"
    assert report["scenarios"][2]["classification"] == "preview_quality_sample"
    assert report["summary"]["recommendation"] == "mixed_or_inconclusive_measurement_review_required"


def test_sdf_measurement_report_recommends_postprocess_when_votes_dominate() -> None:
    measurements = [
        witness.measurement_from_payload(
            "normal_angle",
            "full_quality",
            _payload(name="angle", field_ms=1.0, postprocess_ms=12.0, neighborhood_samples=4000),
            source_stack=["sdf_normal_angle"],
            lens_downsample=1,
        ),
        witness.measurement_from_payload(
            "curvature",
            "full_quality",
            _payload(name="curvature", field_ms=1.0, postprocess_ms=14.0, neighborhood_samples=5000),
            source_stack=["sdf_curvature"],
            lens_downsample=1,
        ),
    ]

    report = witness.build_measurement_report(measurements, runtime_exe="runtime/fractal_ui.exe")

    assert report["summary"]["bottleneck_votes"]["postprocess_pressure"] == 2
    assert report["summary"]["recommendation"] == "postprocess_optimization_candidate"
