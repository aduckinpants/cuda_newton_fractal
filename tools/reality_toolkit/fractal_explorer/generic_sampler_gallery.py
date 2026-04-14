from __future__ import annotations

import colorsys
import json
import math
from pathlib import Path
from typing import Any, Mapping

from .finding_capture import _write_png_rgb


def _require_mapping(value: object, context: str) -> Mapping[str, object]:
    if not isinstance(value, Mapping):
        raise ValueError(f"{context} must be an object")
    return value


def _require_sequence(value: object, context: str) -> list[object]:
    if not isinstance(value, list):
        raise ValueError(f"{context} must be an array")
    return value


def _require_int(value: object, context: str) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise ValueError(f"{context} must be an integer")
    return value


def _require_float(value: object, context: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ValueError(f"{context} must be numeric")
    return float(value)


def _clamp_byte(value: float) -> int:
    return max(0, min(255, int(round(value))))


def _sample_iterations(sample: Mapping[str, object]) -> int:
    iterations_raw = sample.get("iterations", 0)
    if iterations_raw is None:
        return 0
    iterations = _require_int(iterations_raw, "sample.iterations")
    return max(0, iterations)


def _sample_to_rgb(sample: Mapping[str, object]) -> tuple[int, int, int]:
    status = str(sample.get("status") or "bounded")
    if status == "nonfinite":
        return (255, 255, 255)
    if status == "pole":
        return (255, 0, 255)
    if status == "invalid_param":
        return (255, 64, 64)

    value_x = _require_float(sample.get("value_x", 0.0), "sample.value_x")
    value_y = _require_float(sample.get("value_y", 0.0), "sample.value_y")
    abs2_raw = sample.get("abs2", value_x * value_x + value_y * value_y)
    abs2 = 0.0 if abs2_raw is None else _require_float(abs2_raw, "sample.abs2")
    if not math.isfinite(abs2) or abs2 < 0.0:
        abs2 = 0.0

    iterations = _sample_iterations(sample)
    iter_log = math.log1p(float(iterations))
    iter_norm = iter_log / (1.0 + iter_log)
    iter_band = 0.5 + 0.5 * math.sin(float(iterations) * 0.55)

    phase = (math.atan2(value_y, value_x) + math.pi) / (2.0 * math.pi)
    hue = phase
    mag = math.log1p(abs2)
    brightness = 0.20 + 0.80 * (mag / (1.0 + mag))
    saturation = 0.90

    if status == "converged":
        hue = (phase + 0.16 * iter_norm) % 1.0
        saturation = min(1.0, 0.78 + 0.20 * iter_norm)
        brightness = max(brightness, min(0.98, 0.42 + 0.28 * iter_norm + 0.18 * iter_band))
    elif status == "escaped":
        hue = (phase + 0.08 * iter_norm) % 1.0
        saturation = min(1.0, 0.70 + 0.18 * iter_norm)
        brightness = max(brightness, min(0.98, 0.50 + 0.20 * iter_norm))
    elif status == "bounded":
        hue = (phase + 0.04 * iter_norm) % 1.0
        saturation = 0.85
        brightness = max(brightness, min(0.95, 0.30 + 0.12 * iter_norm))

    red, green, blue = colorsys.hsv_to_rgb(hue, saturation, brightness)
    return (
        _clamp_byte(red * 255.0),
        _clamp_byte(green * 255.0),
        _clamp_byte(blue * 255.0),
    )


def write_generic_sample_gallery(response: Mapping[str, object], out_dir: Path) -> dict[str, object]:
    payload = _require_mapping(response, "response")
    function_id = payload.get("function_id")
    if function_id != "generic.sample":
        raise ValueError("response.function_id must be generic.sample")

    samples = _require_sequence(payload.get("samples"), "response.samples")
    if not samples:
        raise ValueError("response.samples must not be empty")

    sequence_results_raw = payload.get("sequence_results", [])
    sequence_results = _require_sequence(sequence_results_raw, "response.sequence_results")
    applied_by_sequence: dict[int, Mapping[str, object]] = {}
    for entry in sequence_results:
        entry_mapping = _require_mapping(entry, "response.sequence_results[]")
        sequence_index = _require_int(entry_mapping.get("sequence_index"), "response.sequence_results[].sequence_index")
        applied = entry_mapping.get("applied", {})
        applied_by_sequence[sequence_index] = _require_mapping(applied, "response.sequence_results[].applied")

    grouped: dict[int, dict[tuple[int, int], Mapping[str, object]]] = {}
    dims: dict[int, tuple[int, int]] = {}
    for sample in samples:
        sample_mapping = _require_mapping(sample, "response.samples[]")
        sequence_index = _require_int(sample_mapping.get("sequence_index", 0), "response.samples[].sequence_index")
        grid_x = _require_int(sample_mapping.get("grid_x"), "response.samples[].grid_x")
        grid_y = _require_int(sample_mapping.get("grid_y"), "response.samples[].grid_y")
        if grid_x < 0 or grid_y < 0:
            raise ValueError("response.samples must contain non-negative grid coordinates")
        grouped.setdefault(sequence_index, {})[(grid_x, grid_y)] = sample_mapping
        prev_w, prev_h = dims.get(sequence_index, (0, 0))
        dims[sequence_index] = (max(prev_w, grid_x + 1), max(prev_h, grid_y + 1))

    out_dir.mkdir(parents=True, exist_ok=True)
    (out_dir / "response.json").write_text(json.dumps(payload, indent=2), encoding="utf-8")

    manifest_frames: list[dict[str, object]] = []
    for sequence_index in sorted(grouped):
        width, height = dims[sequence_index]
        cells = grouped[sequence_index]
        rgb = bytearray()
        for grid_y in range(height):
            for grid_x in range(width):
                sample_mapping = cells.get((grid_x, grid_y))
                if sample_mapping is None:
                    raise ValueError(f"missing grid sample at sequence_index={sequence_index} grid=({grid_x},{grid_y})")
                rgb.extend(_sample_to_rgb(sample_mapping))

        frame_name = f"frame_{sequence_index:04d}.png"
        frame_path = out_dir / frame_name
        _write_png_rgb(frame_path, width, height, bytes(rgb))
        manifest_frames.append({
            "sequence_index": sequence_index,
            "applied": dict(applied_by_sequence.get(sequence_index, {})),
            "width": width,
            "height": height,
            "frame_png": frame_name,
        })

    manifest = {
        "tool": "generic_sampler_gallery",
        "request_id": payload.get("request_id", ""),
        "function_id": "generic.sample",
        "frame_count": len(manifest_frames),
        "frames": manifest_frames,
    }
    (out_dir / "gallery_manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    return manifest