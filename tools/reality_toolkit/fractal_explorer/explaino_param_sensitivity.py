from __future__ import annotations

import csv
import json
import math
from dataclasses import dataclass
from datetime import datetime
from pathlib import Path
from typing import Callable, Sequence

from .paths import repo_publish_root
from .probe_client import run_sample_request


@dataclass(frozen=True)
class ExplainoVariantSensitivitySpec:
    variant_name: str
    fractal_type: str
    param_name: str
    param_path: str
    default_value: float
    baseline_case_id: str
    zero_case_id: str
    default_case_id: str


EXPLAINO_VARIANT_SENSITIVITY_SPECS: tuple[ExplainoVariantSensitivitySpec, ...] = (
    ExplainoVariantSensitivitySpec(
        variant_name="explaino_ripple",
        fractal_type="explaino_ripple",
        param_name="ripple_amplitude",
        param_path="fractal.params.ripple_amplitude",
        default_value=0.15,
        baseline_case_id="explaino_baseline",
        zero_case_id="explaino_ripple_zero",
        default_case_id="explaino_ripple_default",
    ),
    ExplainoVariantSensitivitySpec(
        variant_name="explaino_splice",
        fractal_type="explaino_splice",
        param_name="splice_offset",
        param_path="fractal.params.splice_offset",
        default_value=0.5,
        baseline_case_id="explaino_baseline",
        zero_case_id="explaino_splice_zero",
        default_case_id="explaino_splice_default",
    ),
    ExplainoVariantSensitivitySpec(
        variant_name="explaino_vortex",
        fractal_type="explaino_vortex",
        param_name="vortex_strength",
        param_path="fractal.params.vortex_strength",
        default_value=0.3,
        baseline_case_id="explaino_baseline",
        zero_case_id="explaino_vortex_zero",
        default_case_id="explaino_vortex_default",
    ),
    ExplainoVariantSensitivitySpec(
        variant_name="explaino_tension",
        fractal_type="explaino_tension",
        param_name="tension_strength",
        param_path="fractal.params.tension_strength",
        default_value=0.02,
        baseline_case_id="explaino_baseline",
        zero_case_id="explaino_tension_zero",
        default_case_id="explaino_tension_default",
    ),
)

_SPEC_BY_VARIANT = {spec.variant_name: spec for spec in EXPLAINO_VARIANT_SENSITIVITY_SPECS}

DEFAULT_SENSITIVITY_METRICS = [
    "summary_mean_iterations",
    "summary_escape_fraction",
    "summary_converged_fraction",
    "summary_nonfinite_fraction",
    "summary_pole_fraction",
    "root_index",
]


def default_explaino_param_sensitivity_out_dir(repo_root: Path) -> Path:
    stamp = datetime.now().strftime("%Y-%m-%d_%H%M%S")
    return repo_publish_root(repo_root) / "artifacts" / f"explaino_param_sensitivity_{stamp}"


def resolve_variant_sensitivity_specs(variants: Sequence[str] | None = None) -> list[ExplainoVariantSensitivitySpec]:
    if not variants:
        return list(EXPLAINO_VARIANT_SENSITIVITY_SPECS)

    resolved: list[ExplainoVariantSensitivitySpec] = []
    seen: set[str] = set()
    for variant_name in variants:
        spec = _SPEC_BY_VARIANT.get(variant_name)
        if spec is None:
            raise ValueError(f"Unknown Explaino sensitivity variant: {variant_name}")
        if variant_name in seen:
            continue
        seen.add(variant_name)
        resolved.append(spec)
    return resolved


def build_variant_sensitivity_ticks(default_value: float, ticks: int) -> list[tuple[int, float, float]]:
    if ticks <= 0:
        raise ValueError("ticks must be > 0")

    tick_values: list[tuple[int, float, float]] = []
    for index in range(ticks):
        t = index / max(1, ticks - 1) if ticks > 1 else 0.0
        tick_values.append((index, t, default_value * t))
    return tick_values


def build_variant_sensitivity_request(
    *,
    spec: ExplainoVariantSensitivitySpec,
    ticks: int,
    center_x: float,
    center_y: float,
    span_x: float,
    span_y: float,
    grid_width: int,
    grid_height: int,
    base_state_path: Path | None = None,
    metrics: Sequence[str] | None = None,
) -> tuple[dict[str, object], list[tuple[int, float, float]]]:
    if grid_width <= 0 or grid_height <= 0:
        raise ValueError("grid_width and grid_height must be > 0")

    tick_values = build_variant_sensitivity_ticks(spec.default_value, ticks)
    request: dict[str, object] = {
        "request_version": 1,
        "request_id": f"explaino-param-sensitivity-{spec.variant_name}",
        "function_id": "fractal.sample",
        "mode": "sequence_grid",
        "overrides": [
            {"path": "fractal.view.fractal_type", "value": spec.fractal_type},
        ],
        "region": {
            "center_x": center_x,
            "center_y": center_y,
            "span_x": span_x,
            "span_y": span_y,
            "grid_width": grid_width,
            "grid_height": grid_height,
        },
        "sequence": {
            "zip_paths": True,
            "vary": [
                {
                    "path": spec.param_path,
                    "values": [value for _index, _t, value in tick_values],
                }
            ],
        },
        "metrics": list(metrics or DEFAULT_SENSITIVITY_METRICS),
        "operator_context": {
            "source": "reality_toolkit",
            "operator": "run_explaino_param_sensitivity",
            "why": f"Sweep {spec.variant_name} from zero axis to shipped default strength",
        },
    }
    if base_state_path is not None:
        request["base_state"] = {"load_state_json": str(base_state_path)}

    return request, tick_values


def _coerce_int(value: object) -> int | None:
    if isinstance(value, bool):
        return None
    if isinstance(value, int):
        return value
    if isinstance(value, float) and value.is_integer():
        return int(value)
    return None


def compute_root_index_entropy(samples: Sequence[dict[str, object]]) -> tuple[float, int, int]:
    counts: dict[int, int] = {}
    for sample in samples:
        if not isinstance(sample, dict):
            continue
        root_index = _coerce_int(sample.get("root_index"))
        if root_index is None:
            continue
        counts[root_index] = counts.get(root_index, 0) + 1

    root_sample_count = sum(counts.values())
    if root_sample_count == 0:
        return 0.0, 0, 0

    entropy_bits = 0.0
    for count in counts.values():
        probability = count / root_sample_count
        entropy_bits -= probability * math.log2(probability)
    return entropy_bits, root_sample_count, len(counts)


def _group_samples_by_sequence(samples: Sequence[object]) -> dict[int, list[dict[str, object]]]:
    grouped: dict[int, list[dict[str, object]]] = {}
    for sample in samples:
        if not isinstance(sample, dict):
            continue
        sequence_index = _coerce_int(sample.get("sequence_index"))
        if sequence_index is None:
            continue
        grouped.setdefault(sequence_index, []).append(sample)
    return grouped


def _sequence_summary_map(sequence_results: Sequence[object]) -> dict[int, dict[str, object]]:
    summaries: dict[int, dict[str, object]] = {}
    for result in sequence_results:
        if not isinstance(result, dict):
            continue
        sequence_index = _coerce_int(result.get("sequence_index"))
        if sequence_index is None:
            continue
        summary = result.get("summary")
        summaries[sequence_index] = summary if isinstance(summary, dict) else {}
    return summaries


def _build_variant_rows(
    spec: ExplainoVariantSensitivitySpec,
    tick_values: Sequence[tuple[int, float, float]],
    sequence_results: Sequence[object],
    samples: Sequence[object],
) -> list[dict[str, object]]:
    summary_by_sequence = _sequence_summary_map(sequence_results)
    samples_by_sequence = _group_samples_by_sequence(samples)

    rows: list[dict[str, object]] = []
    for tick, t, param_value in tick_values:
        sequence_summary = summary_by_sequence.get(tick, {})
        entropy_bits, root_sample_count, unique_root_count = compute_root_index_entropy(
            samples_by_sequence.get(tick, [])
        )
        rows.append({
            "variant_name": spec.variant_name,
            "fractal_type": spec.fractal_type,
            "param_name": spec.param_name,
            "param_path": spec.param_path,
            "default_value": spec.default_value,
            "baseline_case_id": spec.baseline_case_id,
            "zero_case_id": spec.zero_case_id,
            "default_case_id": spec.default_case_id,
            "tick": tick,
            "sequence_index": tick,
            "t": t,
            "param_value": param_value,
            "zero_axis": tick == 0,
            "default_tick": tick == len(tick_values) - 1,
            "mean_iterations": sequence_summary.get("mean_iterations", ""),
            "escape_fraction": sequence_summary.get("escape_fraction", ""),
            "converged_fraction": sequence_summary.get("converged_fraction", ""),
            "nonfinite_fraction": sequence_summary.get("nonfinite_fraction", ""),
            "pole_fraction": sequence_summary.get("pole_fraction", ""),
            "root_entropy_bits": entropy_bits,
            "root_sample_count": root_sample_count,
            "unique_root_count": unique_root_count,
        })
    return rows


_CSV_COLUMNS = [
    "variant_name",
    "fractal_type",
    "param_name",
    "param_path",
    "default_value",
    "baseline_case_id",
    "zero_case_id",
    "default_case_id",
    "tick",
    "sequence_index",
    "t",
    "param_value",
    "zero_axis",
    "default_tick",
    "mean_iterations",
    "escape_fraction",
    "converged_fraction",
    "nonfinite_fraction",
    "pole_fraction",
    "root_entropy_bits",
    "root_sample_count",
    "unique_root_count",
]


def write_variant_sensitivity_manifest(out_path: Path, rows: Sequence[dict[str, object]]) -> Path:
    with out_path.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.DictWriter(handle, fieldnames=_CSV_COLUMNS)
        writer.writeheader()
        for row in rows:
            writer.writerow({column: row.get(column, "") for column in _CSV_COLUMNS})
    return out_path


def _peak_entropy_row(rows: Sequence[dict[str, object]]) -> dict[str, object] | None:
    if not rows:
        return None
    return max(rows, key=lambda row: float(row.get("root_entropy_bits", 0.0) or 0.0))


def run_variant_sensitivity_sweep(
    *,
    repo_root: Path,
    spec: ExplainoVariantSensitivitySpec,
    ticks: int,
    out_dir: Path,
    center_x: float,
    center_y: float,
    span_x: float,
    span_y: float,
    grid_width: int,
    grid_height: int,
    base_state_path: Path | None = None,
    metrics: Sequence[str] | None = None,
    timeout_seconds: float = 180.0,
    exe_path: Path | None = None,
    dry_run: bool = False,
    sample_runner: Callable[..., dict[str, object]] = run_sample_request,
) -> tuple[dict[str, object], list[dict[str, object]]]:
    out_dir.mkdir(parents=True, exist_ok=True)

    request, tick_values = build_variant_sensitivity_request(
        spec=spec,
        ticks=ticks,
        center_x=center_x,
        center_y=center_y,
        span_x=span_x,
        span_y=span_y,
        grid_width=grid_width,
        grid_height=grid_height,
        base_state_path=base_state_path,
        metrics=metrics,
    )

    request_path = out_dir / "probe_request.json"
    request_path.write_text(json.dumps(request, indent=2), encoding="utf-8")

    response: dict[str, object] | None = None
    response_path = out_dir / "probe_response.json"
    if not dry_run:
        response = sample_runner(repo_root, request, timeout_seconds=timeout_seconds, exe_path=exe_path)
        response_path.write_text(json.dumps(response, indent=2), encoding="utf-8")

    sequence_results = response.get("sequence_results", []) if response is not None else []
    samples = response.get("samples", []) if response is not None else []
    rows = _build_variant_rows(
        spec,
        tick_values,
        sequence_results if isinstance(sequence_results, Sequence) else [],
        samples if isinstance(samples, Sequence) else [],
    )
    manifest_path = write_variant_sensitivity_manifest(out_dir / "probe_manifest.csv", rows)

    peak_row = _peak_entropy_row(rows)
    summary: dict[str, object] = {
        "variant_name": spec.variant_name,
        "fractal_type": spec.fractal_type,
        "param_name": spec.param_name,
        "param_path": spec.param_path,
        "default_value": spec.default_value,
        "baseline_case_id": spec.baseline_case_id,
        "zero_case_id": spec.zero_case_id,
        "default_case_id": spec.default_case_id,
        "ticks": ticks,
        "mode": "sample_sequence_grid",
        "dry_run": dry_run,
        "manifest": str(manifest_path),
        "request_path": str(request_path),
        "response_path": str(response_path) if response is not None else None,
        "runtime": response.get("runtime") if response is not None else None,
        "response_summary": response.get("summary") if response is not None else None,
        "peak_entropy_tick": peak_row.get("tick") if peak_row is not None else None,
        "peak_entropy_param_value": peak_row.get("param_value") if peak_row is not None else None,
        "peak_entropy_bits": peak_row.get("root_entropy_bits") if peak_row is not None else None,
    }
    summary_path = out_dir / "probe_summary.json"
    summary_path.write_text(json.dumps(summary, indent=2), encoding="utf-8")
    return summary, rows


def run_explaino_param_sensitivity_sweep(
    *,
    repo_root: Path,
    out_dir: Path,
    ticks: int,
    variants: Sequence[str] | None = None,
    center_x: float,
    center_y: float,
    span_x: float,
    span_y: float,
    grid_width: int,
    grid_height: int,
    base_state_path: Path | None = None,
    metrics: Sequence[str] | None = None,
    timeout_seconds: float = 180.0,
    exe_path: Path | None = None,
    dry_run: bool = False,
    sample_runner: Callable[..., dict[str, object]] = run_sample_request,
) -> dict[str, object]:
    out_dir.mkdir(parents=True, exist_ok=True)
    specs = resolve_variant_sensitivity_specs(variants)

    all_rows: list[dict[str, object]] = []
    variant_summaries: list[dict[str, object]] = []
    for spec in specs:
        variant_summary, variant_rows = run_variant_sensitivity_sweep(
            repo_root=repo_root,
            spec=spec,
            ticks=ticks,
            out_dir=out_dir / spec.variant_name,
            center_x=center_x,
            center_y=center_y,
            span_x=span_x,
            span_y=span_y,
            grid_width=grid_width,
            grid_height=grid_height,
            base_state_path=base_state_path,
            metrics=metrics,
            timeout_seconds=timeout_seconds,
            exe_path=exe_path,
            dry_run=dry_run,
            sample_runner=sample_runner,
        )
        variant_summaries.append(variant_summary)
        all_rows.extend(variant_rows)

    manifest_path = write_variant_sensitivity_manifest(out_dir / "explaino_variant_sensitivity.csv", all_rows)
    summary: dict[str, object] = {
        "timestamp": datetime.now().isoformat(),
        "out_dir": str(out_dir),
        "manifest": str(manifest_path),
        "mode": "sample_sequence_grid",
        "dry_run": dry_run,
        "ticks": ticks,
        "variant_count": len(specs),
        "variants": [spec.variant_name for spec in specs],
        "variant_summaries": variant_summaries,
        "grid_width": grid_width,
        "grid_height": grid_height,
        "base_state_path": str(base_state_path) if base_state_path is not None else None,
    }
    summary_path = out_dir / "explaino_variant_sensitivity_summary.json"
    summary_path.write_text(json.dumps(summary, indent=2), encoding="utf-8")
    return summary