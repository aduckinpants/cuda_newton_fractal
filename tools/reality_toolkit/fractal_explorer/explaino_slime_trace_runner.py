from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
from pathlib import Path
from typing import Any, Sequence

from .finding_analyzer import RootResolution, resolve_analysis_roots

TRACE_SCHEMA_ID = "viewer.explaino_slime_trace.v1"
ROOT_SAMPLE_SCHEMA_ID = "viewer.explaino_slime_trace.root_sample.v1"
MEASUREMENT_SCHEMA_ID = "viewer.explaino_slime_trace.measurement_sample.v1"
DEFAULT_POLICY_ID = "deterministic_non_root_explaino_controls_v1"
DEFAULT_STEP_SCALE = 0.03125
MAX_TRACE_STEPS = 10000

TRACE_ARTIFACTS = {
    "manifest": "slime_trace_manifest.json",
    "initial_state": "initial_state.json",
    "final_state": "final_state.json",
    "mutation_trace": "mutation_trace.jsonl",
    "root_samples": "root_samples.jsonl",
    "measurement_samples": "measurement_samples.jsonl",
    "summary": "trace_summary.json",
}

# Keep this first runner away from root-authority parameters. It is a receipt
# generator for bounded parameter traversal, not a runtime root recomputation path.
NON_ROOT_EXPLAINO_PARAM_PRIORITY: tuple[str, ...] = (
    "ripple_amplitude",
    "splice_offset",
    "vortex_strength",
    "tension_strength",
    "balance_void",
    "symmetry_tension",
    "field_curvature",
    "explaino_warp",
    "newton_damping",
    "explaino_damping",
    "epsilon",
)


def _canonical_json_bytes(value: Any) -> bytes:
    return json.dumps(value, sort_keys=True, separators=(",", ":"), allow_nan=False).encode("utf-8")


def stable_json_hash(value: Any) -> str:
    return "sha256:" + hashlib.sha256(_canonical_json_bytes(value)).hexdigest()


def _write_json(path: Path, value: Any) -> None:
    path.write_text(json.dumps(value, indent=2, sort_keys=True, allow_nan=False) + "\n", encoding="utf-8")


def _write_jsonl(path: Path, rows: Sequence[dict[str, Any]]) -> None:
    text = "".join(json.dumps(row, sort_keys=True, allow_nan=False) + "\n" for row in rows)
    path.write_text(text, encoding="utf-8")


def _load_state(path: Path) -> dict[str, Any]:
    if not path.exists():
        raise FileNotFoundError(f"state JSON does not exist: {path}")
    state = json.loads(path.read_text(encoding="utf-8"))
    if not isinstance(state, dict):
        raise ValueError("state JSON root must be an object")
    return state


def _fractal_type_id(state: dict[str, Any]) -> str:
    value = state.get("fractal_type")
    if isinstance(value, str) and value:
        return value
    view = state.get("view")
    if isinstance(view, dict):
        nested = view.get("fractal_type")
        if isinstance(nested, str) and nested:
            return nested
    return ""


def _require_explaino_state(state: dict[str, Any]) -> str:
    fractal_type = _fractal_type_id(state)
    if not fractal_type.startswith("explaino"):
        raise ValueError("ExplainO slime trace requires an ExplainO fractal_type")
    return fractal_type


def _state_params(state: dict[str, Any]) -> dict[str, Any]:
    params = state.get("params")
    if not isinstance(params, dict):
        raise ValueError("state JSON must contain object field: params")
    return params


def _is_finite_number(value: Any) -> bool:
    return not isinstance(value, bool) and isinstance(value, (int, float)) and math.isfinite(float(value))


def _candidate_param_paths(params: dict[str, Any]) -> list[str]:
    paths: list[str] = []
    for name in NON_ROOT_EXPLAINO_PARAM_PRIORITY:
        if _is_finite_number(params.get(name)):
            paths.append(f"params.{name}")
    return paths


def _get_path_value(state: dict[str, Any], path: str) -> float:
    if not path.startswith("params."):
        raise ValueError(f"unsupported trace path: {path}")
    params = _state_params(state)
    key = path.removeprefix("params.")
    value = params.get(key)
    if not _is_finite_number(value):
        raise ValueError(f"trace path is not a finite numeric parameter: {path}")
    return float(value)


def _set_path_value(state: dict[str, Any], path: str, value: float) -> None:
    if not path.startswith("params."):
        raise ValueError(f"unsupported trace path: {path}")
    _state_params(state)[path.removeprefix("params.")] = value


def _roots_as_dicts(root_resolution: RootResolution) -> list[dict[str, float]]:
    return [{"x": float(root.x), "y": float(root.y)} for root in root_resolution.roots]


def _resolve_roots_for_state(state: dict[str, Any]) -> RootResolution:
    params = _state_params(state)
    coeffs = params.get("poly_coeffs", [])
    if not isinstance(coeffs, list):
        coeffs = []
    return resolve_analysis_roots(params, coeffs)


def _target_value(previous_value: float, step_index: int, *, step_scale: float) -> float:
    if not math.isfinite(step_scale) or step_scale <= 0.0:
        raise ValueError("step_scale must be a finite positive number")
    return round(previous_value + (step_scale * step_index), 12)


def _build_measurement_sample(
    *,
    step_index: int,
    path: str,
    previous_value: float,
    applied_value: float,
    pre_state_hash: str,
    post_state_hash: str,
    root_resolution: RootResolution,
    root_count: int,
    policy_id: str,
) -> dict[str, Any]:
    delta = applied_value - previous_value
    payload: dict[str, Any] = {
        "schema_id": MEASUREMENT_SCHEMA_ID,
        "step_index": step_index,
        "path": path,
        "measurement_kind": "state_hash_delta",
        "previous_value": previous_value,
        "applied_value": applied_value,
        "delta": delta,
        "absolute_delta": abs(delta),
        "utility": round(abs(delta) / (1.0 + abs(previous_value)), 12),
        "pre_state_hash": pre_state_hash,
        "post_state_hash": post_state_hash,
        "root_source": root_resolution.source,
        "root_authority": root_resolution.authority,
        "root_count": root_count,
        "policy_id": policy_id,
    }
    payload["measurement_hash"] = stable_json_hash(payload)
    return payload


def _build_root_sample(
    *,
    step_index: int,
    path: str,
    state_hash: str,
    root_resolution: RootResolution,
    roots: list[dict[str, float]],
) -> dict[str, Any]:
    return {
        "schema_id": ROOT_SAMPLE_SCHEMA_ID,
        "step_index": step_index,
        "path": path,
        "state_hash": state_hash,
        "root_source": root_resolution.source,
        "root_authority": root_resolution.authority,
        "root_source_note": root_resolution.note,
        "root_count": len(roots),
        "roots": roots,
    }


def run_explaino_slime_trace(
    *,
    state_path: Path,
    out_dir: Path,
    steps: int,
    policy_id: str = DEFAULT_POLICY_ID,
    rng_seed: int | None = None,
    step_scale: float = DEFAULT_STEP_SCALE,
) -> dict[str, Any]:
    if steps <= 0:
        raise ValueError("steps must be > 0")
    if steps > MAX_TRACE_STEPS:
        raise ValueError(f"steps must be <= {MAX_TRACE_STEPS}")
    if not policy_id:
        raise ValueError("policy_id must be non-empty")

    original_state = _load_state(state_path)
    fractal_type = _require_explaino_state(original_state)
    params = _state_params(original_state)
    root_resolution = _resolve_roots_for_state(original_state)
    roots = _roots_as_dicts(root_resolution)
    candidate_paths = _candidate_param_paths(params)
    if not candidate_paths:
        raise ValueError("ExplainO slime trace found no finite non-root parameter controls to mutate")

    out_dir.mkdir(parents=True, exist_ok=True)
    current_state = copy.deepcopy(original_state)
    initial_state_hash = stable_json_hash(original_state)
    scene_id = f"{fractal_type}:{initial_state_hash.removeprefix('sha256:')[:12]}"

    mutation_rows: list[dict[str, Any]] = []
    root_rows: list[dict[str, Any]] = []
    measurement_rows: list[dict[str, Any]] = []

    for step_index in range(1, steps + 1):
        path = candidate_paths[(step_index - 1) % len(candidate_paths)]
        previous_value = _get_path_value(current_state, path)
        pre_state_hash = stable_json_hash(current_state)
        target_value = _target_value(previous_value, step_index, step_scale=step_scale)
        _set_path_value(current_state, path, target_value)
        applied_value = _get_path_value(current_state, path)
        post_state_hash = stable_json_hash(current_state)

        step_root_resolution = _resolve_roots_for_state(current_state)
        step_roots = _roots_as_dicts(step_root_resolution)
        root_row = _build_root_sample(
            step_index=step_index,
            path=path,
            state_hash=post_state_hash,
            root_resolution=step_root_resolution,
            roots=step_roots,
        )
        measurement_row = _build_measurement_sample(
            step_index=step_index,
            path=path,
            previous_value=previous_value,
            applied_value=applied_value,
            pre_state_hash=pre_state_hash,
            post_state_hash=post_state_hash,
            root_resolution=step_root_resolution,
            root_count=len(step_roots),
            policy_id=policy_id,
        )
        mutation_rows.append({
            "schema_id": TRACE_SCHEMA_ID,
            "step_index": step_index,
            "path": path,
            "type": "float",
            "previous_value": previous_value,
            "target_value": target_value,
            "applied_value": applied_value,
            "utility": measurement_row["utility"],
            "selection_reason": "deterministic_priority_order_non_root_control",
            "pre_state_hash": pre_state_hash,
            "post_state_hash": post_state_hash,
            "measurement_hash": measurement_row["measurement_hash"],
            "root_source": step_root_resolution.source,
            "root_authority": step_root_resolution.authority,
            "roots_at_step": step_roots,
            "scene_id": scene_id,
            "rng_seed": rng_seed,
            "policy_id": policy_id,
        })
        root_rows.append(root_row)
        measurement_rows.append(measurement_row)

    final_state_hash = stable_json_hash(current_state)
    trace_hash = stable_json_hash({
        "mutation_trace": mutation_rows,
        "root_samples": root_rows,
        "measurement_samples": measurement_rows,
        "final_state_hash": final_state_hash,
    })
    summary: dict[str, Any] = {
        "schema_id": "viewer.explaino_slime_trace.summary.v1",
        "ok": True,
        "fractal_type": fractal_type,
        "scene_id": scene_id,
        "policy_id": policy_id,
        "rng_seed": rng_seed,
        "steps_requested": steps,
        "steps_applied": len(mutation_rows),
        "candidate_count": len(candidate_paths),
        "candidate_paths": candidate_paths,
        "root_source": root_resolution.source,
        "root_authority": root_resolution.authority,
        "initial_state_hash": initial_state_hash,
        "final_state_hash": final_state_hash,
        "trace_hash": trace_hash,
        "deferred_boundaries": [
            "seed_hunting",
            "root_history_charts",
            "fits_flashlight_reuse",
            "stopping_policy",
            "genetic_algorithm",
            "runtime_rendering",
            "sdf_work",
        ],
    }
    manifest: dict[str, Any] = {
        "schema_id": TRACE_SCHEMA_ID,
        "source_state_json": str(state_path),
        "fractal_type": fractal_type,
        "scene_id": scene_id,
        "policy_id": policy_id,
        "rng_seed": rng_seed,
        "root_source": root_resolution.source,
        "root_authority": root_resolution.authority,
        "steps_requested": steps,
        "steps_applied": len(mutation_rows),
        "trace_hash": trace_hash,
        "artifacts": TRACE_ARTIFACTS,
    }

    _write_json(out_dir / TRACE_ARTIFACTS["initial_state"], original_state)
    _write_json(out_dir / TRACE_ARTIFACTS["final_state"], current_state)
    _write_jsonl(out_dir / TRACE_ARTIFACTS["mutation_trace"], mutation_rows)
    _write_jsonl(out_dir / TRACE_ARTIFACTS["root_samples"], root_rows)
    _write_jsonl(out_dir / TRACE_ARTIFACTS["measurement_samples"], measurement_rows)
    _write_json(out_dir / TRACE_ARTIFACTS["summary"], summary)
    _write_json(out_dir / TRACE_ARTIFACTS["manifest"], manifest)
    return summary


def build_arg_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="Run a bounded headless ExplainO parameter-space slime trace")
    parser.add_argument("--state-json", required=True, type=Path, help="Input viewer state.json")
    parser.add_argument("--out-dir", required=True, type=Path, help="Output trace artifact directory")
    parser.add_argument("--steps", type=int, default=8, help="Number of bounded deterministic mutation steps")
    parser.add_argument("--policy-id", default=DEFAULT_POLICY_ID, help="Policy identity written into trace receipts")
    parser.add_argument("--rng-seed", type=int, default=None, help="Optional seed identity; v1 policy is deterministic")
    parser.add_argument("--step-scale", type=float, default=DEFAULT_STEP_SCALE, help="Positive numeric delta scale")
    return parser


def main(argv: Sequence[str] | None = None) -> int:
    parser = build_arg_parser()
    args = parser.parse_args(argv)
    summary = run_explaino_slime_trace(
        state_path=args.state_json,
        out_dir=args.out_dir,
        steps=args.steps,
        policy_id=args.policy_id,
        rng_seed=args.rng_seed,
        step_scale=args.step_scale,
    )
    print(json.dumps(summary, indent=2, sort_keys=True, allow_nan=False))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
