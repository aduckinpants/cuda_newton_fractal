from __future__ import annotations

import json
import math
import shutil
import uuid
from pathlib import Path
from typing import Any, Mapping, MutableMapping

from .generic_sampler_gallery import write_generic_sample_gallery
from .probe_client import run_sample_request

AST_OPS = {
    "var_z",
    "var_z_conj",
    "const",
    "param",
    "complex_param",
    "add",
    "sub",
    "mul",
    "div",
    "neg",
    "conj",
    "abs",
    "sin",
    "cos",
    "exp",
    "log",
    "pow_int",
    "pow_real",
    "compose",
}

DEFAULT_METRICS = [
    "iterations",
    "status",
    "value",
    "abs2",
    "derivative",
    "summary_mean_iterations",
    "summary_diverged_fraction",
    "summary_converged_fraction",
]


def _repo_root() -> Path:
    return Path(__file__).resolve().parents[3]


def _require_mapping(value: object, context: str) -> Mapping[str, Any]:
    if not isinstance(value, Mapping):
        raise ValueError(f"{context} must be an object")
    return value


def _require_list(value: object, context: str) -> list[object]:
    if not isinstance(value, list):
        raise ValueError(f"{context} must be an array")
    return value


def _finite_float(value: object, context: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        raise ValueError(f"{context} must be numeric")
    result = float(value)
    if not math.isfinite(result):
        raise ValueError(f"{context} must be finite")
    return result


def _positive_finite_float(value: object, context: str) -> float:
    result = _finite_float(value, context)
    if result <= 0.0:
        raise ValueError(f"{context} must be positive")
    return result


def _positive_int(value: object, context: str) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise ValueError(f"{context} must be an integer")
    if value <= 0:
        raise ValueError(f"{context} must be positive")
    return value


def _validate_ast(node: object, context: str = "formula.ast", depth: int = 0) -> None:
    if depth > 64:
        raise ValueError(f"{context} is too deeply nested")
    mapping = _require_mapping(node, context)
    raw_op = mapping.get("op")
    if not isinstance(raw_op, str):
        raise ValueError(f"{context}.op must be a string")
    if raw_op not in AST_OPS:
        raise ValueError(f"{context}.op is unsupported: {raw_op}")

    if raw_op in {"var_z", "var_z_conj"}:
        return
    if raw_op == "const":
        _finite_float(mapping.get("value"), f"{context}.value")
        return
    if raw_op in {"param", "complex_param"}:
        if not isinstance(mapping.get("name"), str):
            raise ValueError(f"{context}.name must be a string")
        return
    if raw_op in {"neg", "conj", "abs", "sin", "cos", "exp", "log"}:
        _validate_ast(mapping.get("arg"), f"{context}.arg", depth + 1)
        return
    if raw_op in {"add", "sub", "mul", "div", "compose"}:
        args = _require_list(mapping.get("args"), f"{context}.args")
        if len(args) != 2:
            raise ValueError(f"{context}.args must contain exactly two nodes")
        _validate_ast(args[0], f"{context}.args[0]", depth + 1)
        _validate_ast(args[1], f"{context}.args[1]", depth + 1)
        return
    if raw_op in {"pow_int", "pow_real"}:
        _validate_ast(mapping.get("base"), f"{context}.base", depth + 1)
        has_exponent = "exponent" in mapping
        has_exponent_param = "exponent_param" in mapping
        if has_exponent == has_exponent_param:
            raise ValueError(f"{context} must define exactly one of exponent or exponent_param")
        if has_exponent:
            exponent = _finite_float(mapping.get("exponent"), f"{context}.exponent")
            if raw_op == "pow_int" and exponent != math.floor(exponent):
                raise ValueError(f"{context}.exponent must be an integer")
        elif not isinstance(mapping.get("exponent_param"), str):
            raise ValueError(f"{context}.exponent_param must be a string")


def _validate_pack(pack: Mapping[str, Any]) -> dict[str, Any]:
    allowed = {"schema_version", "pack_id", "name", "formula", "params", "controls", "epsilon", "escape_radius", "region"}
    unknown = sorted(set(pack) - allowed)
    if unknown:
        raise ValueError(f"unknown equation pack keys: {', '.join(unknown)}")
    if pack.get("schema_version") != 1:
        raise ValueError("schema_version must be 1")
    if not isinstance(pack.get("pack_id"), str) or not pack["pack_id"]:
        raise ValueError("pack_id must be a non-empty string")
    if not isinstance(pack.get("name"), str) or not pack["name"]:
        raise ValueError("name must be a non-empty string")

    formula = _require_mapping(pack.get("formula"), "formula")
    formula_unknown = sorted(set(formula) - {"kind", "ast", "iteration_param"})
    if formula_unknown:
        raise ValueError(f"unknown formula keys: {', '.join(formula_unknown)}")
    kind = formula.get("kind")
    if kind not in {"direct", "iterate_map"}:
        raise ValueError("formula.kind must be direct or iterate_map")
    if kind == "iterate_map" and not isinstance(formula.get("iteration_param"), str):
        raise ValueError("formula.iteration_param must be a string for iterate_map")
    _validate_ast(formula.get("ast"))

    params = _require_mapping(pack.get("params", {}), "params")
    normalized_params: dict[str, float] = {}
    for key, value in params.items():
        if not isinstance(key, str) or not key:
            raise ValueError("params keys must be non-empty strings")
        normalized_params[key] = _finite_float(value, f"params.{key}")

    controls = _require_list(pack.get("controls", []), "controls")
    seen_ids: set[str] = set()
    seen_params: set[str] = set()
    normalized_controls: list[dict[str, Any]] = []
    for index, raw_control in enumerate(controls):
        control = dict(_require_mapping(raw_control, f"controls[{index}]"))
        control_unknown = sorted(set(control) - {"id", "param", "label", "min", "max", "step", "default"})
        if control_unknown:
            raise ValueError(f"unknown controls[{index}] keys: {', '.join(control_unknown)}")
        control_id = control.get("id")
        control_param = control.get("param")
        if not isinstance(control_id, str) or not control_id:
            raise ValueError(f"controls[{index}].id must be a non-empty string")
        if not isinstance(control_param, str) or not control_param:
            raise ValueError(f"controls[{index}].param must be a non-empty string")
        if control_id in seen_ids:
            raise ValueError(f"duplicate control id: {control_id}")
        if control_param in seen_params:
            raise ValueError(f"duplicate control param: {control_param}")
        seen_ids.add(control_id)
        seen_params.add(control_param)
        if "label" in control and not isinstance(control["label"], str):
            raise ValueError(f"controls[{index}].label must be a string")
        for key in ("min", "max", "step", "default"):
            if key in control:
                control[key] = _finite_float(control[key], f"controls[{index}].{key}")
        normalized_controls.append(control)

    normalized = dict(pack)
    normalized["params"] = normalized_params
    normalized["controls"] = normalized_controls
    normalized["epsilon"] = _positive_finite_float(pack.get("epsilon", 1.0e-6), "epsilon")
    normalized["escape_radius"] = _positive_finite_float(pack.get("escape_radius", 1000.0), "escape_radius")

    if "region" in pack:
        region = dict(_require_mapping(pack["region"], "region"))
        region_unknown = sorted(set(region) - {"center_x", "center_y", "span_x", "span_y", "grid_width", "grid_height"})
        if region_unknown:
            raise ValueError(f"unknown region keys: {', '.join(region_unknown)}")
        normalized["region"] = {
            "center_x": _finite_float(region.get("center_x"), "region.center_x"),
            "center_y": _finite_float(region.get("center_y"), "region.center_y"),
            "span_x": _positive_finite_float(region.get("span_x"), "region.span_x"),
            "span_y": _positive_finite_float(region.get("span_y"), "region.span_y"),
            "grid_width": _positive_int(region.get("grid_width"), "region.grid_width"),
            "grid_height": _positive_int(region.get("grid_height"), "region.grid_height"),
        }

    return normalized


def load_equation_pack(pack_path: Path | str) -> dict[str, Any]:
    path = Path(pack_path)
    parsed = json.loads(path.read_text(encoding="utf-8"))
    return _validate_pack(_require_mapping(parsed, "equation pack"))


def _apply_control_overrides(pack: Mapping[str, Any], overrides: Mapping[str, float] | None) -> dict[str, float]:
    params = dict(_require_mapping(pack.get("params", {}), "params"))
    normalized = {str(key): _finite_float(value, f"params.{key}") for key, value in params.items()}
    if not overrides:
        return normalized

    control_to_param = {
        str(control["id"]): str(control["param"])
        for control in _require_list(pack.get("controls", []), "controls")
        if isinstance(control, Mapping) and "id" in control and "param" in control
    }
    for key, value in overrides.items():
        param_key = control_to_param.get(str(key), str(key))
        normalized[param_key] = _finite_float(value, f"override.{key}")
    return normalized


def build_generic_sample_request(
    pack: Mapping[str, Any],
    *,
    backend: str = "default",
    control_overrides: Mapping[str, float] | None = None,
    region_overrides: Mapping[str, object] | None = None,
    request_id: str | None = None,
) -> dict[str, Any]:
    normalized_pack = _validate_pack(pack)
    formula = _require_mapping(normalized_pack["formula"], "formula")
    function: dict[str, Any] = {
        "ast": formula["ast"],
        "params": _apply_control_overrides(normalized_pack, control_overrides),
        "epsilon": normalized_pack["epsilon"],
        "escape_radius": normalized_pack["escape_radius"],
    }
    if formula["kind"] == "iterate_map":
        function["iterate"] = {"count_param": formula["iteration_param"]}

    request: dict[str, Any] = {
        "request_version": 1,
        "request_id": request_id or f"equation-pack-{normalized_pack['pack_id']}-{uuid.uuid4().hex[:8]}",
        "function_id": "generic.sample",
        "function": function,
        "metrics": DEFAULT_METRICS,
    }

    region_source: MutableMapping[str, object] = dict(normalized_pack.get("region", {}))
    if region_overrides:
        region_source.update(region_overrides)
    if region_source:
        region = {
            "center_x": _finite_float(region_source.get("center_x"), "region.center_x"),
            "center_y": _finite_float(region_source.get("center_y"), "region.center_y"),
            "span_x": _positive_finite_float(region_source.get("span_x"), "region.span_x"),
            "span_y": _positive_finite_float(region_source.get("span_y"), "region.span_y"),
            "grid_width": _positive_int(region_source.get("grid_width"), "region.grid_width"),
            "grid_height": _positive_int(region_source.get("grid_height"), "region.grid_height"),
        }
        request["mode"] = "grid"
        request["region"] = region
    else:
        request["mode"] = "point_set"
        request["points"] = [{"x": 0.0, "y": 0.0}]

    if backend != "default":
        if backend not in {"cpu", "cuda"}:
            raise ValueError("backend must be cpu, cuda, or default")
        request["execution"] = {"backend_preference": backend}
    else:
        request["execution"] = {"backend_preference": "default"}

    return request


def run_equation_pack_workbench(
    pack_path: Path | str,
    out_dir: Path | str,
    *,
    backend: str = "default",
    control_overrides: Mapping[str, float] | None = None,
    region_overrides: Mapping[str, object] | None = None,
    exe_path: Path | None = None,
    timeout_seconds: float = 180.0,
    sample_runner=run_sample_request,
    repo_root_path: Path | None = None,
) -> dict[str, Any]:
    source_path = Path(pack_path)
    output = Path(out_dir)
    output.mkdir(parents=True, exist_ok=True)

    pack = load_equation_pack(source_path)
    request = build_generic_sample_request(
        pack,
        backend=backend,
        control_overrides=control_overrides,
        region_overrides=region_overrides,
    )

    shutil.copyfile(source_path, output / "pack.json")
    (output / "request.json").write_text(json.dumps(request, indent=2), encoding="utf-8")
    response = sample_runner(
        repo_root_path or _repo_root(),
        request,
        exe_path=exe_path,
        timeout_seconds=timeout_seconds,
    )
    (output / "response.json").write_text(json.dumps(response, indent=2), encoding="utf-8")
    manifest = write_generic_sample_gallery(response, output)

    return {
        "pack_json": str(output / "pack.json"),
        "request_json": str(output / "request.json"),
        "response_json": str(output / "response.json"),
        "gallery_manifest": str(output / "gallery_manifest.json"),
        "frame_count": manifest["frame_count"],
        "backend": response.get("runtime", {}).get("backend_used", ""),
    }
