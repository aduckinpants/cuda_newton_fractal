from __future__ import annotations

import argparse
import copy
import json
import math
import sys
import time
from collections import Counter
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from statistics import median
from typing import Any, Callable


REPO_ROOT = Path(__file__).resolve().parents[1]
TESTS_DIR = REPO_ROOT / "tests"


@dataclass(frozen=True)
class SdfWitnessScenario:
    name: str
    color_signal: str
    source_stack: tuple[dict[str, object], ...]
    fractal_type: str = "multibrot"
    palette: str = "cyclic_escape"
    grading: str = "escape_default"
    lens_downsample: int = 1
    is_sdf: bool = True


DEFAULT_SCENARIOS: tuple[SdfWitnessScenario, ...] = (
    SdfWitnessScenario(
        name="plain_smooth_escape",
        color_signal="smooth_escape",
        source_stack=(),
        is_sdf=False,
    ),
    SdfWitnessScenario(
        name="sdf_signed_distance_fullres",
        color_signal="sdf_signed_distance",
        source_stack=({"signal": "sdf_signed_distance", "scale": 0.05, "bias": 0.5, "blend_weight": 1.0},),
    ),
    SdfWitnessScenario(
        name="sdf_normal_angle_fullres",
        color_signal="sdf_normal_angle",
        source_stack=({"signal": "sdf_normal_angle", "scale": 1.0, "bias": 0.0, "blend_weight": 1.0},),
        palette="phase_wheel",
        grading="phase_default",
    ),
    SdfWitnessScenario(
        name="sdf_curvature_fullres",
        color_signal="sdf_curvature",
        source_stack=({"signal": "sdf_curvature", "scale": 1.0, "bias": 0.5, "blend_weight": 1.0},),
    ),
    SdfWitnessScenario(
        name="sdf_normal_angle_curvature_stack",
        color_signal="sdf_normal_angle",
        source_stack=(
            {"signal": "sdf_normal_angle", "scale": 1.0, "bias": 0.0, "blend_weight": 1.0},
            {"signal": "sdf_curvature", "scale": 1.0, "bias": 0.5, "blend_weight": 0.5},
        ),
        palette="phase_wheel",
        grading="phase_default",
    ),
    SdfWitnessScenario(
        name="sdf_signed_distance_downsample4",
        color_signal="sdf_signed_distance",
        source_stack=({"signal": "sdf_signed_distance", "scale": 0.05, "bias": 0.5, "blend_weight": 1.0},),
        lens_downsample=4,
    ),
    SdfWitnessScenario(
        name="lens_field_v2_fullres",
        color_signal="lens_field_v2_distance",
        source_stack=({"signal": "lens_field_v2_distance", "scale": 1.0, "bias": 0.0, "blend_weight": 1.0},),
    ),
    SdfWitnessScenario(
        name="sdf_pack_scene_signed_distance",
        color_signal="sdf_signed_distance",
        source_stack=({"signal": "sdf_signed_distance", "scale": 0.05, "bias": 0.5, "blend_weight": 1.0},),
        fractal_type="sdf_pack_scene",
    ),
)


def repo_absolute_path(path: Path) -> Path:
    return path if path.is_absolute() else REPO_ROOT / path


def _as_float(payload: dict[str, object], key: str, default: float = 0.0) -> float:
    value = payload.get(key, default)
    if isinstance(value, bool):
        return default
    if isinstance(value, (int, float)):
        result = float(value)
        return result if math.isfinite(result) else default
    return default


def _as_int(payload: dict[str, object], key: str, default: int = 0) -> int:
    value = payload.get(key, default)
    if isinstance(value, bool):
        return default
    if isinstance(value, (int, float)):
        return int(value)
    return default


def _classification(payload: dict[str, object], *, is_sdf: bool, field_ms: float, postprocess_ms: float, total_ms: float) -> str:
    if not is_sdf or payload.get("lens_sdf_color_pipeline_active") is not True:
        return "non_sdf_baseline"
    if payload.get("render_pacing_preview_active") is True or _as_int(payload, "lens_sdf_postprocess_pixel_step", 1) > 1:
        return "preview_quality_sample"
    if total_ms <= 0.0:
        return "mixed_or_inconclusive"

    field_fraction = field_ms / total_ms
    postprocess_fraction = postprocess_ms / total_ms
    if postprocess_ms >= max(1.0, field_ms * 1.5) and postprocess_fraction >= 0.55:
        return "postprocess_pressure"
    if field_ms >= max(1.0, postprocess_ms * 1.5) and field_fraction >= 0.55:
        return "field_generation_pressure"
    if total_ms < 2.0:
        return "low_sdf_cost"
    return "mixed_or_inconclusive"


def measurement_from_payload(
    name: str,
    phase: str,
    payload: dict[str, object],
    *,
    source_stack: list[str] | tuple[str, ...],
    lens_downsample: int,
    is_sdf: bool | None = None,
) -> dict[str, object]:
    field_ms = _as_float(payload, "lens_sdf_field_ms")
    postprocess_ms = _as_float(payload, "lens_sdf_postprocess_ms")
    total_ms = _as_float(payload, "lens_sdf_total_ms", field_ms + postprocess_ms)
    if total_ms <= 0.0 and field_ms + postprocess_ms > 0.0:
        total_ms = field_ms + postprocess_ms
    base_ms = _as_float(payload, "base_render_ms")
    last_render_ms = _as_float(payload, "last_render_ms")
    if is_sdf is None:
        is_sdf = any(str(signal).startswith("sdf_") for signal in source_stack)
    classification = _classification(
        payload,
        is_sdf=is_sdf,
        field_ms=field_ms,
        postprocess_ms=postprocess_ms,
        total_ms=total_ms,
    )
    field_fraction = field_ms / total_ms if total_ms > 0.0 else 0.0
    postprocess_fraction = postprocess_ms / total_ms if total_ms > 0.0 else 0.0
    rendered_hash = payload.get("rendered_frame_hash")
    if not isinstance(rendered_hash, str):
        rendered_hash = ""
    postprocess_backend = payload.get("lens_sdf_postprocess_backend_used", "unknown")
    if not isinstance(postprocess_backend, str):
        postprocess_backend = "unknown"

    return {
        "name": name,
        "phase": phase,
        "classification": classification,
        "source_stack": list(source_stack),
        "lens_downsample": int(lens_downsample),
        "current_fractal_type": str(payload.get("current_fractal_type", "")),
        "lens_sdf_field_source": str(payload.get("lens_sdf_field_source", "none")),
        "lens_sdf_field_producer_kind": str(payload.get("lens_sdf_field_producer_kind", "none")),
        "lens_sdf_supported_signals": [
            str(item) for item in payload.get("lens_sdf_supported_signals", [])
        ] if isinstance(payload.get("lens_sdf_supported_signals"), list) else [],
        "lens_sdf_field_capability_fail_closed_reason": payload.get("lens_sdf_field_capability_fail_closed_reason"),
        "rendered_frame_ready": payload.get("rendered_frame_ready") is True,
        "rendered_frame_hash": rendered_hash,
        "target_render_width": _as_int(payload, "target_render_width"),
        "target_render_height": _as_int(payload, "target_render_height"),
        "rendered_frame_width": _as_int(payload, "rendered_frame_width"),
        "rendered_frame_height": _as_int(payload, "rendered_frame_height"),
        "base_render_ms": base_ms,
        "lens_sdf_field_ms": field_ms,
        "lens_sdf_requested_equivalent_field_ms": _as_float(payload, "lens_sdf_requested_equivalent_field_ms", field_ms),
        "lens_sdf_field_cache_lookup_ms": _as_float(payload, "lens_sdf_field_cache_lookup_ms"),
        "lens_sdf_field_mask_downsample_ms": _as_float(payload, "lens_sdf_field_mask_downsample_ms"),
        "lens_sdf_field_backend_ms": _as_float(payload, "lens_sdf_field_backend_ms"),
        "lens_sdf_field_cache_store_ms": _as_float(payload, "lens_sdf_field_cache_store_ms"),
        "lens_sdf_postprocess_ms": postprocess_ms,
        "lens_sdf_total_ms": total_ms,
        "last_render_ms": last_render_ms,
        "field_fraction_of_sdf_total": field_fraction,
        "postprocess_fraction_of_sdf_total": postprocess_fraction,
        "lens_sdf_width": _as_int(payload, "lens_sdf_width"),
        "lens_sdf_height": _as_int(payload, "lens_sdf_height"),
        "lens_sdf_pixel_scale": _as_float(payload, "lens_sdf_pixel_scale", 1.0),
        "lens_sdf_requested_downsample": _as_int(payload, "lens_sdf_requested_downsample", int(lens_downsample)),
        "lens_sdf_effective_downsample": _as_int(payload, "lens_sdf_effective_downsample", int(lens_downsample)),
        "lens_sdf_quality_mode": str(payload.get("lens_sdf_quality_mode", "requested")),
        "lens_sdf_field_cache_status": str(payload.get("lens_sdf_field_cache_status", "disabled")),
        "lens_sdf_field_cache_hit": payload.get("lens_sdf_field_cache_hit") is True,
        "lens_sdf_field_cache_mask_bytes": _as_int(payload, "lens_sdf_field_cache_mask_bytes"),
        "lens_sdf_postprocess_pixel_step": _as_int(payload, "lens_sdf_postprocess_pixel_step", 1),
        "lens_sdf_postprocess_worker_count": _as_int(payload, "lens_sdf_postprocess_worker_count", 1),
        "lens_sdf_postprocess_backend_used": postprocess_backend,
        "lens_sdf_postprocess_backend_fallback_used": payload.get("lens_sdf_postprocess_backend_fallback_used") is True,
        "lens_sdf_postprocess_backend_buffer_reused": payload.get("lens_sdf_postprocess_backend_buffer_reused") is True,
        "lens_sdf_postprocess_backend_buffer_grew": payload.get("lens_sdf_postprocess_backend_buffer_grew") is True,
        "lens_sdf_postprocess_direct_sample_count": _as_int(payload, "lens_sdf_postprocess_direct_sample_count"),
        "lens_sdf_postprocess_neighborhood_sample_count": _as_int(payload, "lens_sdf_postprocess_neighborhood_sample_count"),
        "lens_sdf_postprocess_filled_pixel_count": _as_int(payload, "lens_sdf_postprocess_filled_pixel_count"),
        "render_pacing_preview_active": payload.get("render_pacing_preview_active") is True,
        "render_pacing_preview_scale": _as_float(payload, "render_pacing_preview_scale", 1.0),
    }


MEDIAN_FLOAT_KEYS = (
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
    "field_fraction_of_sdf_total",
    "postprocess_fraction_of_sdf_total",
    "lens_sdf_pixel_scale",
    "render_pacing_preview_scale",
)

MEDIAN_INT_KEYS = (
    "target_render_width",
    "target_render_height",
    "rendered_frame_width",
    "rendered_frame_height",
    "lens_sdf_width",
    "lens_sdf_height",
    "lens_sdf_requested_downsample",
    "lens_sdf_effective_downsample",
    "lens_sdf_field_cache_mask_bytes",
    "lens_sdf_postprocess_pixel_step",
    "lens_sdf_postprocess_worker_count",
    "lens_sdf_postprocess_direct_sample_count",
    "lens_sdf_postprocess_neighborhood_sample_count",
    "lens_sdf_postprocess_filled_pixel_count",
)

TIMING_SAMPLE_KEYS = (
    "base_render_ms",
    "lens_sdf_field_ms",
    "lens_sdf_field_mask_downsample_ms",
    "lens_sdf_field_backend_ms",
    "lens_sdf_field_cache_store_ms",
    "lens_sdf_postprocess_ms",
    "lens_sdf_total_ms",
    "last_render_ms",
)


def _measurement_group_key(item: dict[str, object]) -> tuple[object, ...]:
    return (
        item.get("name", ""),
        item.get("phase", ""),
        item.get("current_fractal_type", ""),
        tuple(item.get("source_stack", [])) if isinstance(item.get("source_stack"), list) else (),
        item.get("lens_downsample", 0),
    )


def _median_float(items: list[dict[str, object]], key: str) -> float:
    values = [float(item.get(key, 0.0)) for item in items]
    return float(median(values)) if values else 0.0


def _median_int(items: list[dict[str, object]], key: str) -> int:
    values = [int(item.get(key, 0)) for item in items]
    return int(median(values)) if values else 0


def _timing_sample(item: dict[str, object]) -> dict[str, object]:
    return {key: item.get(key, 0.0) for key in TIMING_SAMPLE_KEYS}


def _mode_string(items: list[dict[str, object]], key: str, default: str = "") -> str:
    values = [str(item.get(key, default)) for item in items]
    if not values:
        return default
    counts = Counter(values)
    return counts.most_common(1)[0][0]


def aggregate_measurement_samples(measurements: list[dict[str, object]]) -> list[dict[str, object]]:
    groups: dict[tuple[object, ...], list[dict[str, object]]] = {}
    order: list[tuple[object, ...]] = []
    for item in measurements:
        key = _measurement_group_key(item)
        if key not in groups:
            groups[key] = []
            order.append(key)
        groups[key].append(item)

    aggregated: list[dict[str, object]] = []
    for key in order:
        samples = groups[key]
        row = dict(samples[0])
        row["sample_count"] = len(samples)
        row["timing_samples"] = [_timing_sample(sample) for sample in samples]
        row["rendered_frame_hash_samples"] = [
            sample.get("rendered_frame_hash", "") for sample in samples
        ]
        row["lens_sdf_field_cache_status_samples"] = [
            str(sample.get("lens_sdf_field_cache_status", "")) for sample in samples
        ]
        row["lens_sdf_postprocess_backend_buffer_reused_samples"] = [
            sample.get("lens_sdf_postprocess_backend_buffer_reused") is True for sample in samples
        ]
        row["lens_sdf_postprocess_backend_buffer_grew_samples"] = [
            sample.get("lens_sdf_postprocess_backend_buffer_grew") is True for sample in samples
        ]
        row["lens_sdf_field_cache_status"] = _mode_string(samples, "lens_sdf_field_cache_status", "disabled")
        row["lens_sdf_field_cache_hit"] = sum(1 for sample in samples if sample.get("lens_sdf_field_cache_hit") is True) >= (
            (len(samples) // 2) + 1
        )
        row["lens_sdf_postprocess_backend_buffer_reused"] = sum(
            1 for sample in samples if sample.get("lens_sdf_postprocess_backend_buffer_reused") is True
        ) >= ((len(samples) // 2) + 1)
        row["lens_sdf_postprocess_backend_buffer_grew"] = any(
            sample.get("lens_sdf_postprocess_backend_buffer_grew") is True for sample in samples
        )
        for metric in MEDIAN_FLOAT_KEYS:
            row[metric] = _median_float(samples, metric)
        for metric in MEDIAN_INT_KEYS:
            row[metric] = _median_int(samples, metric)

        total_ms = float(row.get("lens_sdf_total_ms", 0.0))
        field_ms = float(row.get("lens_sdf_field_ms", 0.0))
        postprocess_ms = float(row.get("lens_sdf_postprocess_ms", 0.0))
        if total_ms <= 0.0 and field_ms + postprocess_ms > 0.0:
            total_ms = field_ms + postprocess_ms
            row["lens_sdf_total_ms"] = total_ms
        row["field_fraction_of_sdf_total"] = field_ms / total_ms if total_ms > 0.0 else 0.0
        row["postprocess_fraction_of_sdf_total"] = postprocess_ms / total_ms if total_ms > 0.0 else 0.0
        source_stack = row.get("source_stack", [])
        is_sdf = row.get("lens_sdf_field_producer_kind", "none") != "none" or (
            isinstance(source_stack, list) and any(str(signal).startswith("sdf_") for signal in source_stack)
        )
        row["lens_sdf_color_pipeline_active"] = is_sdf
        row["classification"] = _classification(
            row,
            is_sdf=is_sdf,
            field_ms=field_ms,
            postprocess_ms=postprocess_ms,
            total_ms=total_ms,
        )
        aggregated.append(row)
    return aggregated


def _recommendation(votes: Counter[str]) -> str:
    postprocess_votes = votes.get("postprocess_pressure", 0)
    field_votes = votes.get("field_generation_pressure", 0)
    if postprocess_votes >= 2 and postprocess_votes > field_votes:
        return "postprocess_optimization_candidate"
    if field_votes >= 2 and field_votes > postprocess_votes:
        return "field_generation_or_downsample_candidate"
    return "mixed_or_inconclusive_measurement_review_required"


def build_measurement_report(
    measurements: list[dict[str, object]],
    *,
    runtime_exe: str,
    persistent_viewer_launch_count: int = 0,
) -> dict[str, object]:
    aggregated_measurements = aggregate_measurement_samples(measurements)
    votes: Counter[str] = Counter(str(item.get("classification", "mixed_or_inconclusive")) for item in aggregated_measurements)
    sdf_measurements = [item for item in aggregated_measurements if item.get("classification") != "non_sdf_baseline"]
    full_quality = [item for item in aggregated_measurements if item.get("phase") == "full_quality"]
    preview = [item for item in aggregated_measurements if item.get("phase") == "preview"]
    max_postprocess_fraction = max(
        (float(item.get("postprocess_fraction_of_sdf_total", 0.0)) for item in sdf_measurements),
        default=0.0,
    )
    max_field_fraction = max(
        (float(item.get("field_fraction_of_sdf_total", 0.0)) for item in sdf_measurements),
        default=0.0,
    )
    field_cache_hit_count = sum(1 for item in sdf_measurements if item.get("lens_sdf_field_cache_hit") is True)

    return {
        "schema_version": 1,
        "generated_at_utc": datetime.now(timezone.utc).isoformat(),
        "runtime_exe": runtime_exe,
        "no_mouse_automation": True,
        "persistent_viewer_launch_count": int(persistent_viewer_launch_count),
        "summary": {
            "raw_sample_count": len(measurements),
            "scenario_count": len(aggregated_measurements),
            "sdf_scenario_count": len(sdf_measurements),
            "full_quality_sample_count": len(full_quality),
            "preview_sample_count": len(preview),
            "bottleneck_votes": {
                "field_generation_pressure": votes.get("field_generation_pressure", 0),
                "postprocess_pressure": votes.get("postprocess_pressure", 0),
                "preview_quality_sample": votes.get("preview_quality_sample", 0),
                "low_sdf_cost": votes.get("low_sdf_cost", 0),
                "mixed_or_inconclusive": votes.get("mixed_or_inconclusive", 0),
                "non_sdf_baseline": votes.get("non_sdf_baseline", 0),
            },
            "max_postprocess_fraction_of_sdf_total": max_postprocess_fraction,
            "max_field_fraction_of_sdf_total": max_field_fraction,
            "field_cache_hit_count": field_cache_hit_count,
            "recommendation": _recommendation(votes),
        },
        "scenarios": aggregated_measurements,
    }


def write_markdown_report(report: dict[str, object], out_path: Path) -> None:
    lines = [
        "# SDF Performance Witness",
        "",
        f"- Runtime: `{report.get('runtime_exe', '')}`",
        f"- Recommendation: `{report.get('summary', {}).get('recommendation', '')}`",
        f"- Persistent viewer launches: `{report.get('persistent_viewer_launch_count', '')}`",
        "",
        "| Scenario | Samples | Phase | Class | Backend | Fallback | Buffer Reuse | Buffer Grow | Field Cache | Req DS | Eff DS | Quality | Base ms | Field ms | Down ms | Backend ms | Store ms | Post ms | SDF total ms | Last ms | Step | Workers |",
        "|---|---:|---|---|---|---|---|---|---|---:|---:|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|",
    ]
    for item in report.get("scenarios", []):
        if not isinstance(item, dict):
            continue
        lines.append(
            "| {name} | {samples} | {phase} | {classification} | {backend} | {fallback} | {buffer_reuse} | {buffer_grow} | {cache} | {requested} | {effective} | {quality} | {base:.3f} | {field:.3f} | {down:.3f} | {backend_ms:.3f} | {store:.3f} | {post:.3f} | {total:.3f} | {last:.3f} | {step} | {workers} |".format(
                name=item.get("name", ""),
                samples=int(item.get("sample_count", 1)),
                phase=item.get("phase", ""),
                classification=item.get("classification", ""),
                backend=item.get("lens_sdf_postprocess_backend_used", "unknown"),
                fallback="yes" if item.get("lens_sdf_postprocess_backend_fallback_used") else "no",
                buffer_reuse="yes" if item.get("lens_sdf_postprocess_backend_buffer_reused") else "no",
                buffer_grow="yes" if item.get("lens_sdf_postprocess_backend_buffer_grew") else "no",
                cache=item.get("lens_sdf_field_cache_status", "disabled"),
                requested=int(item.get("lens_sdf_requested_downsample", item.get("lens_downsample", 0))),
                effective=int(item.get("lens_sdf_effective_downsample", item.get("lens_downsample", 0))),
                quality=item.get("lens_sdf_quality_mode", "requested"),
                base=float(item.get("base_render_ms", 0.0)),
                field=float(item.get("lens_sdf_field_ms", 0.0)),
                down=float(item.get("lens_sdf_field_mask_downsample_ms", 0.0)),
                backend_ms=float(item.get("lens_sdf_field_backend_ms", 0.0)),
                store=float(item.get("lens_sdf_field_cache_store_ms", 0.0)),
                post=float(item.get("lens_sdf_postprocess_ms", 0.0)),
                total=float(item.get("lens_sdf_total_ms", 0.0)),
                last=float(item.get("last_render_ms", 0.0)),
                step=int(item.get("lens_sdf_postprocess_pixel_step", 0)),
                workers=int(item.get("lens_sdf_postprocess_worker_count", 0)),
            )
        )
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def _base_state(runtime_exe: Path, *, width: int, height: int) -> dict[str, object]:
    if str(TESTS_DIR) not in sys.path:
        sys.path.insert(0, str(TESTS_DIR))
    from runtime_harness import run_headless_capture

    capture = run_headless_capture(
        str(runtime_exe),
        "--capture-diagnostic",
        "--fractal-type",
        "multibrot",
        "--width",
        str(width),
        "--height",
        str(height),
    )
    state = copy.deepcopy(capture["state"])
    state.setdefault("view", {})
    state.setdefault("params", {})
    state.setdefault("render", {})
    state.setdefault("lens", {})
    state["view"].update(
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
    state["params"].update(
        {
            "max_iter": 96,
            "coloring_mode": "smooth_escape",
            "multibrot_power_float": 2.0,
            "multibrot_power_imag": 0.0,
        }
    )
    state["render"].update(
        {
            "width": width,
            "height": height,
            "interaction_debounce_ms": 450,
            "preview_target_fps": 240.0,
            "preview_min_scale": 0.5,
            "sample_tier": "fast",
        }
    )
    state["lens"].update(
        {
            "enabled": False,
            "downsample": 1,
            "sdf_overlay_mode": "off",
            "sdf_overlay_opacity": 0.55,
            "sdf_overlay_band_px": 1.5,
        }
    )
    return state


def _state_for_scenario(base_state: dict[str, object], scenario: SdfWitnessScenario) -> dict[str, object]:
    state = copy.deepcopy(base_state)
    params = state["params"]
    state["fractal_type"] = scenario.fractal_type
    params["coloring_mode"] = "smooth_escape"
    params["color_signal"] = scenario.color_signal
    params["color_shape"] = "identity"
    params["color_palette"] = scenario.palette
    params["color_grading"] = scenario.grading
    params.pop("color_source_stack", None)
    if scenario.source_stack:
        params["color_source_stack"] = [dict(entry) for entry in scenario.source_stack]
    state["lens"]["downsample"] = scenario.lens_downsample
    return state


def _write_json(path: Path, payload: dict[str, object]) -> Path:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2), encoding="utf-8")
    return path


def _wait_for_report_predicate(
    viewer: object,
    predicate: Callable[[dict[str, object]], bool],
    *,
    timeout_seconds: float,
) -> dict[str, object]:
    deadline = time.monotonic() + timeout_seconds
    last_payload: dict[str, object] | None = None
    while time.monotonic() < deadline:
        proc = getattr(viewer, "proc", None)
        if proc is not None and proc.poll() is not None:
            raise RuntimeError(f"viewer exited while waiting for SDF witness report; returncode={proc.returncode}")
        payload = viewer._load_report()
        if payload is not None:
            last_payload = payload
            if payload.get("rendered_frame_ready") is True and predicate(payload):
                return payload
        time.sleep(0.05)
    raise RuntimeError(f"SDF witness report predicate was not satisfied; last_payload={last_payload!r}")


def collect_runtime_measurements(
    *,
    runtime_exe: Path,
    work_dir: Path,
    width: int,
    height: int,
    include_preview_sample: bool,
    repeat_count: int,
    timeout_seconds: float,
) -> tuple[list[dict[str, object]], int]:
    if str(TESTS_DIR) not in sys.path:
        sys.path.insert(0, str(TESTS_DIR))
    from runtime_harness import PersistentRuntimeViewerAutomation, runtime_automation_lock, write_state_bundle

    work_dir.mkdir(parents=True, exist_ok=True)
    base_state = _base_state(runtime_exe, width=width, height=height)
    scenario_paths: list[tuple[SdfWitnessScenario, Path]] = []
    for scenario in DEFAULT_SCENARIOS:
        scenario_state = _state_for_scenario(base_state, scenario)
        scenario_paths.append((scenario, write_state_bundle(work_dir / scenario.name, scenario_state)))

    measurements: list[dict[str, object]] = []
    first_scenario, first_path = scenario_paths[0]
    report_path = work_dir / "viewer_report.json"
    command_path = work_dir / "viewer_command.json"
    with runtime_automation_lock():
        with PersistentRuntimeViewerAutomation(
            exe_path=runtime_exe,
            state_path=first_path,
            report_path=report_path,
            command_path=command_path,
            open_color_pipeline=False,
        ) as viewer:
            viewer.wait_for_control("fractal_control.center_x.primary", timeout_seconds=timeout_seconds)
            first_payload = viewer.wait_for_report(timeout_seconds=timeout_seconds)
            measurements.append(
                measurement_from_payload(
                    first_scenario.name,
                    "full_quality",
                    first_payload,
                    source_stack=[str(entry.get("signal", "")) for entry in first_scenario.source_stack],
                    lens_downsample=first_scenario.lens_downsample,
                    is_sdf=first_scenario.is_sdf,
                )
            )
            for _repeat_index in range(1, max(1, repeat_count)):
                payload = viewer.load_state_json(first_path, expected_fractal_type=first_scenario.fractal_type, timeout_seconds=timeout_seconds)
                measurements.append(
                    measurement_from_payload(
                        first_scenario.name,
                        "full_quality",
                        payload,
                        source_stack=[str(entry.get("signal", "")) for entry in first_scenario.source_stack],
                        lens_downsample=first_scenario.lens_downsample,
                        is_sdf=first_scenario.is_sdf,
                    )
                )

            preview_path: Path | None = None
            preview_scenario: SdfWitnessScenario | None = None
            for scenario, state_path in scenario_paths[1:]:
                for _repeat_index in range(max(1, repeat_count)):
                    payload = viewer.load_state_json(state_path, expected_fractal_type=scenario.fractal_type, timeout_seconds=timeout_seconds)
                    measurements.append(
                        measurement_from_payload(
                            scenario.name,
                            "full_quality",
                            payload,
                            source_stack=[str(entry.get("signal", "")) for entry in scenario.source_stack],
                            lens_downsample=scenario.lens_downsample,
                            is_sdf=scenario.is_sdf,
                        )
                    )
                if scenario.name == "sdf_normal_angle_curvature_stack":
                    preview_path = state_path
                    preview_scenario = scenario

            if include_preview_sample and preview_path is not None and preview_scenario is not None:
                viewer.load_state_json(preview_path, expected_fractal_type=preview_scenario.fractal_type, timeout_seconds=timeout_seconds)
                preview_payload = viewer.set_control_value(
                    "fractal_control.center_x.primary",
                    -0.49,
                    timeout_seconds=timeout_seconds,
                )
                measurements.append(
                    measurement_from_payload(
                        f"{preview_scenario.name}_interaction_preview",
                        "preview",
                        preview_payload,
                        source_stack=[str(entry.get("signal", "")) for entry in preview_scenario.source_stack],
                        lens_downsample=preview_scenario.lens_downsample,
                        is_sdf=preview_scenario.is_sdf,
                    )
                )
                settled_payload = _wait_for_report_predicate(
                    viewer,
                    lambda payload: payload.get("ui_automation_command_sequence") == viewer.sequence
                    and payload.get("render_pacing_preview_active") is False
                    and float(payload.get("render_pacing_preview_scale", 0.0)) == 1.0
                    and int(payload.get("rendered_frame_width", 0)) >= int(payload.get("target_render_width", 1))
                    and int(payload.get("rendered_frame_height", 0)) >= int(payload.get("target_render_height", 1))
                    and int(payload.get("lens_sdf_postprocess_pixel_step", 0)) == 1,
                    timeout_seconds=timeout_seconds,
                )
                measurements.append(
                    measurement_from_payload(
                        f"{preview_scenario.name}_settled_full_quality",
                        "full_quality",
                        settled_payload,
                        source_stack=[str(entry.get("signal", "")) for entry in preview_scenario.source_stack],
                        lens_downsample=preview_scenario.lens_downsample,
                        is_sdf=preview_scenario.is_sdf,
                    )
                )
            return measurements, int(viewer.launch_count)


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Run a no-mouse SDF Color Pipeline performance witness against the published viewer.")
    parser.add_argument("--runtime-exe", type=Path, required=True)
    parser.add_argument("--out-json", type=Path, required=True)
    parser.add_argument("--out-md", type=Path)
    parser.add_argument("--work-dir", type=Path, default=REPO_ROOT / "artifacts" / "sdf_performance_witness")
    parser.add_argument("--width", type=int, default=640)
    parser.add_argument("--height", type=int, default=480)
    parser.add_argument("--repeat-count", type=int, default=1)
    parser.add_argument("--timeout-seconds", type=float, default=90.0)
    parser.add_argument("--include-preview-sample", action="store_true")
    return parser.parse_args(argv)


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    if args.width <= 0 or args.height <= 0:
        raise SystemExit("--width and --height must be positive")
    if args.repeat_count <= 0:
        raise SystemExit("--repeat-count must be positive")
    runtime_exe = repo_absolute_path(args.runtime_exe)
    out_json = repo_absolute_path(args.out_json)
    out_md = repo_absolute_path(args.out_md) if args.out_md is not None else None
    work_dir = repo_absolute_path(args.work_dir)
    if not runtime_exe.exists():
        raise SystemExit(f"runtime executable does not exist: {runtime_exe}")

    measurements, launch_count = collect_runtime_measurements(
        runtime_exe=runtime_exe,
        work_dir=work_dir,
        width=args.width,
        height=args.height,
        include_preview_sample=bool(args.include_preview_sample),
        repeat_count=int(args.repeat_count),
        timeout_seconds=float(args.timeout_seconds),
    )
    report = build_measurement_report(
        measurements,
        runtime_exe=str(runtime_exe),
        persistent_viewer_launch_count=launch_count,
    )
    out_json.parent.mkdir(parents=True, exist_ok=True)
    out_json.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    if out_md is not None:
        write_markdown_report(report, out_md)
    print(f"viewer_host_sdf_performance_witness: wrote {out_json}")
    print(f"viewer_host_sdf_performance_witness: recommendation={report['summary']['recommendation']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
