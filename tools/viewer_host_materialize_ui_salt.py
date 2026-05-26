from __future__ import annotations

import argparse
import ast
import json
import math
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any


VALID_CONTRACT_KINDS = {"function_library", "composition_recipe", "explaino"}
VALID_SIGNAL_KINDS = {"scalar", "phase", "categorical"}
VALID_PARAM_TYPES = {"float", "double", "int", "bool", "enum"}
VALID_STATEMENTS = {"contract", "lane", "function", "param", "compat", "recipe", "explaino_contract"}


class MaterializerError(ValueError):
    pass


@dataclass
class FunctionRecord:
    lane: str
    function: dict[str, Any]
    seen_params: set[str] = field(default_factory=set)


def _strip_comment(line: str) -> str:
    in_string = False
    quote = ""
    escaped = False
    for index, char in enumerate(line):
        if escaped:
            escaped = False
            continue
        if char == "\\" and in_string:
            escaped = True
            continue
        if char in ("'", '"'):
            if in_string and char == quote:
                in_string = False
                quote = ""
            elif not in_string:
                in_string = True
                quote = char
            continue
        if not in_string and char == "#":
            return line[:index]
    return line


def _literal_from_ast(node: ast.AST) -> Any:
    if isinstance(node, ast.Constant):
        return node.value
    if isinstance(node, ast.List):
        return [_literal_from_ast(item) for item in node.elts]
    if isinstance(node, ast.Tuple):
        return [_literal_from_ast(item) for item in node.elts]
    if isinstance(node, ast.UnaryOp) and isinstance(node.op, ast.USub):
        value = _literal_from_ast(node.operand)
        if isinstance(value, (int, float)) and not isinstance(value, bool):
            return -value
        raise MaterializerError("unary minus is only allowed on numeric literals")
    if isinstance(node, ast.Dict):
        keys = [_literal_from_ast(key) for key in node.keys]
        values = [_literal_from_ast(value) for value in node.values]
        return dict(zip(keys, values))
    raise MaterializerError(f"unsupported literal in ui.salt statement: {ast.dump(node)}")


def _parse_statement(line: str, line_number: int) -> tuple[str, dict[str, Any]] | None:
    line = _strip_comment(line).strip()
    if not line:
        return None
    try:
        expr = ast.parse(line, mode="eval").body
    except SyntaxError as exc:
        raise MaterializerError(f"line {line_number}: invalid statement syntax: {exc.msg}") from exc
    if not isinstance(expr, ast.Call) or not isinstance(expr.func, ast.Name):
        raise MaterializerError(f"line {line_number}: expected statement_name(key=value, ...)")
    name = expr.func.id
    if name not in VALID_STATEMENTS:
        raise MaterializerError(f"line {line_number}: unknown statement '{name}'")
    if expr.args:
        raise MaterializerError(f"line {line_number}: positional arguments are not allowed")

    kwargs: dict[str, Any] = {}
    for keyword in expr.keywords:
        if keyword.arg is None:
            raise MaterializerError(f"line {line_number}: **kwargs are not allowed")
        if keyword.arg in kwargs:
            raise MaterializerError(f"line {line_number}: duplicate argument '{keyword.arg}'")
        kwargs[keyword.arg] = _literal_from_ast(keyword.value)
    return name, kwargs


def _require_string(kwargs: dict[str, Any], key: str, *, statement: str) -> str:
    value = kwargs.get(key)
    if not isinstance(value, str) or not value.strip():
        raise MaterializerError(f"{statement} requires {key}")
    return value


def _optional_string(kwargs: dict[str, Any], key: str, default: str = "") -> str:
    value = kwargs.get(key, default)
    if value is None:
        return default
    if not isinstance(value, str):
        raise MaterializerError(f"{key} must be a string")
    return value


def _optional_bool(kwargs: dict[str, Any], key: str, default: bool) -> bool:
    value = kwargs.get(key, default)
    if not isinstance(value, bool):
        raise MaterializerError(f"{key} must be a bool")
    return value


def _check_known_args(statement: str, kwargs: dict[str, Any], allowed: set[str]) -> None:
    for key in kwargs:
        if key not in allowed:
            raise MaterializerError(f"{statement} has unknown field '{key}'")


def _check_finite(value: Any, field_name: str) -> None:
    if isinstance(value, (int, float)) and not isinstance(value, bool):
        if not math.isfinite(float(value)):
            raise MaterializerError(f"{field_name} must be finite")


def _build_param_from_sequence(function_id: str, raw: Any) -> dict[str, Any]:
    if not isinstance(raw, list) or len(raw) not in (7, 8):
        raise MaterializerError(
            f"param shorthand for {function_id} must be [path, type, label, min, max, step, default] or include options"
        )
    path, type_id, label, min_value, max_value, step_value, default_value = raw[:7]
    options = raw[7] if len(raw) == 8 else []
    if not isinstance(path, str) or not path:
        raise MaterializerError(f"param for {function_id} requires path")
    if not isinstance(type_id, str) or type_id not in VALID_PARAM_TYPES:
        raise MaterializerError(f"param {path} has invalid type")
    if not isinstance(label, str):
        raise MaterializerError(f"param {path} requires label")
    param = {
        "path": path,
        "type": type_id,
        "label": label,
    }
    for key, value in (("min", min_value), ("max", max_value), ("step", step_value)):
        if value is not None:
            _check_finite(value, f"param {path} {key}")
            param[key] = value
    if default_value is not None:
        _check_finite(default_value, f"param {path} default")
        param["default"] = default_value
    if options:
        if not isinstance(options, list) or not all(isinstance(item, str) and item for item in options):
            raise MaterializerError(f"param {path} options must be a list of strings")
        param["options"] = options
    _validate_param(function_id, param)
    return param


def _validate_param(function_id: str, param: dict[str, Any]) -> None:
    path = param.get("path")
    if not isinstance(path, str) or not path:
        raise MaterializerError(f"param for {function_id} requires path")
    type_id = param.get("type")
    if not isinstance(type_id, str) or type_id not in VALID_PARAM_TYPES:
        raise MaterializerError(f"param {path} has invalid type")
    for key in ("min", "max", "step", "default"):
        if key in param:
            _check_finite(param[key], f"param {path} {key}")
    if "min" in param and "max" in param and float(param["min"]) > float(param["max"]):
        raise MaterializerError(f"param {path} min greater than max")
    if "default" in param and isinstance(param["default"], (int, float)) and not isinstance(param["default"], bool):
        if "min" in param and float(param["default"]) < float(param["min"]):
            raise MaterializerError(f"param {path} default below min")
        if "max" in param and float(param["default"]) > float(param["max"]):
            raise MaterializerError(f"param {path} default above max")
    if type_id == "enum":
        options = param.get("options")
        if not isinstance(options, list) or not options:
            raise MaterializerError(f"enum param {path} requires options")
        if "default" in param and param["default"] not in options:
            raise MaterializerError(f"enum param {path} default not in options")


def _append_param(record: FunctionRecord, param: dict[str, Any]) -> None:
    path = param["path"]
    if path in record.seen_params:
        raise MaterializerError(f"duplicate param path '{path}' for function '{record.function['id']}'")
    record.seen_params.add(path)
    record.function.setdefault("params", []).append(param)


def materialize_text(text: str, *, source_path: str = "") -> dict[str, Any]:
    contracts: list[dict[str, Any]] = []
    lanes: list[dict[str, Any]] = []
    functions: dict[str, FunctionRecord] = {}
    compatibility: list[dict[str, Any]] = []
    recipes: list[dict[str, Any]] = []
    explaino_entries: list[dict[str, Any]] = []
    lane_ids: set[str] = set()
    contract_keys: set[tuple[str, str]] = set()

    for line_number, raw_line in enumerate(text.splitlines(), start=1):
        parsed = _parse_statement(raw_line, line_number)
        if parsed is None:
            continue
        name, kwargs = parsed
        if name == "contract":
            _check_known_args(name, kwargs, {"kind", "contract_id", "version"})
            kind = _require_string(kwargs, "kind", statement=name)
            if kind not in VALID_CONTRACT_KINDS:
                raise MaterializerError(f"contract has invalid kind '{kind}'")
            contract_id = _require_string(kwargs, "contract_id", statement=name)
            version = kwargs.get("version", 1)
            if not isinstance(version, int) or isinstance(version, bool) or version != 1:
                raise MaterializerError("contract version must be integer 1")
            key = (kind, contract_id)
            if key in contract_keys:
                raise MaterializerError(f"duplicate contract '{contract_id}'")
            contract_keys.add(key)
            contracts.append({"kind": kind, "contract_id": contract_id, "version": version})
        elif name == "lane":
            _check_known_args(name, kwargs, {"id", "label", "default"})
            lane_id = _require_string(kwargs, "id", statement=name)
            if lane_id in lane_ids:
                raise MaterializerError(f"duplicate lane id '{lane_id}'")
            lane_ids.add(lane_id)
            lanes.append({
                "id": lane_id,
                "label": _require_string(kwargs, "label", statement=name),
                "default": _require_string(kwargs, "default", statement=name),
                "functions": [],
            })
        elif name == "function":
            _check_known_args(name, kwargs, {"lane", "id", "label", "description", "taxonomy_group", "signal_kind", "runtime_backed", "input_kind", "output_kind", "cost_hint", "params"})
            lane_id = _require_string(kwargs, "lane", statement=name)
            function_id = _require_string(kwargs, "id", statement=name)
            if function_id in functions:
                raise MaterializerError(f"duplicate function id '{function_id}'")
            taxonomy_group = _require_string(kwargs, "taxonomy_group", statement=f"function {function_id}")
            signal_kind = _optional_string(kwargs, "signal_kind", "")
            if signal_kind and signal_kind not in VALID_SIGNAL_KINDS:
                raise MaterializerError(f"function {function_id} invalid signal_kind '{signal_kind}'")
            runtime_backed = _optional_bool(kwargs, "runtime_backed", True)
            function = {
                "id": function_id,
                "label": _require_string(kwargs, "label", statement=name),
                "description": _optional_string(kwargs, "description", ""),
                "taxonomy_group": taxonomy_group,
                "runtime_backed": runtime_backed,
                "input_kind": _optional_string(kwargs, "input_kind", "scalar"),
                "output_kind": _optional_string(kwargs, "output_kind", "scalar"),
                "params": [],
            }
            if signal_kind:
                function["signal_kind"] = signal_kind
            if "cost_hint" in kwargs:
                _check_finite(kwargs["cost_hint"], f"function {function_id} cost_hint")
                function["cost_hint"] = kwargs["cost_hint"]
            record = FunctionRecord(lane=lane_id, function=function)
            for raw_param in kwargs.get("params", []) or []:
                _append_param(record, _build_param_from_sequence(function_id, raw_param))
            functions[function_id] = record
        elif name == "param":
            _check_known_args(name, kwargs, {"function", "path", "type", "label", "help", "min", "max", "step", "default", "options"})
            function_id = _require_string(kwargs, "function", statement=name)
            if function_id not in functions:
                raise MaterializerError(f"param references unknown function '{function_id}'")
            param = {
                "path": _require_string(kwargs, "path", statement=name),
                "type": _require_string(kwargs, "type", statement=name),
                "label": _optional_string(kwargs, "label", ""),
            }
            for key in ("help", "min", "max", "step", "default", "options"):
                if key in kwargs:
                    param[key] = kwargs[key]
            _validate_param(function_id, param)
            _append_param(functions[function_id], param)
        elif name == "compat":
            _check_known_args(name, kwargs, {"source", "palette", "signal", "palette_runtime", "grading", "mode", "reason"})
            compatibility.append({
                "source": _require_string(kwargs, "source", statement=name),
                "palette": _require_string(kwargs, "palette", statement=name),
                "signal": _require_string(kwargs, "signal", statement=name),
                "palette_runtime": _require_string(kwargs, "palette_runtime", statement=name),
                "grading": _require_string(kwargs, "grading", statement=name),
                "mode": _require_string(kwargs, "mode", statement=name),
                "reason": _optional_string(kwargs, "reason", "supported_runtime_bridge"),
            })
        elif name == "recipe":
            _check_known_args(name, kwargs, {"id", "label", "source", "shape", "palette", "grading", "fail_closed_reason"})
            recipes.append({
                "id": _require_string(kwargs, "id", statement=name),
                "label": _require_string(kwargs, "label", statement=name),
                "source": _require_string(kwargs, "source", statement=name),
                "shape": _require_string(kwargs, "shape", statement=name),
                "palette": _require_string(kwargs, "palette", statement=name),
                "grading": _require_string(kwargs, "grading", statement=name),
                "fail_closed_reason": _optional_string(kwargs, "fail_closed_reason", ""),
            })
        elif name == "explaino_contract":
            required = ("id", "hypothesis_space", "authority", "lens", "invariant", "proof", "fallback")
            _check_known_args(name, kwargs, set(required) | {"product_facing", "diagnostic"})
            entry = {key: _require_string(kwargs, key, statement=name) for key in required}
            entry["product_facing"] = _optional_bool(kwargs, "product_facing", False)
            entry["diagnostic"] = _optional_bool(kwargs, "diagnostic", True)
            explaino_entries.append(entry)

    for record in functions.values():
        if record.lane not in lane_ids:
            raise MaterializerError(f"function '{record.function['id']}' references unknown lane '{record.lane}'")
    lane_by_id = {lane["id"]: lane for lane in lanes}
    for function_id, record in functions.items():
        lane_by_id[record.lane]["functions"].append(record.function)
    for lane in lanes:
        if not any(function["id"] == lane["default"] for function in lane["functions"]):
            raise MaterializerError(f"lane '{lane['id']}' default references unknown function '{lane['default']}'")
    for item in compatibility:
        if item["source"] not in functions:
            raise MaterializerError(f"compat references unknown source '{item['source']}'")
        if item["palette"] not in functions:
            raise MaterializerError(f"compat references unknown palette '{item['palette']}'")
    if not explaino_entries:
        raise MaterializerError("at least one explaino_contract is required")

    return {
        "schema_version": 1,
        "source_path": source_path,
        "contracts": contracts,
        "function_library": {"lanes": lanes},
        "composition_recipe_contract": {
            "compatibility": compatibility,
            "recipes": recipes,
        },
        "explaino_contract": {"entries": explaino_entries},
    }


def stable_source_path(path: Path) -> str:
    try:
        return str(path.resolve().relative_to(Path.cwd().resolve()))
    except ValueError:
        return str(path)


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Materialize viewer-local ui.salt metadata into strict JSON contracts")
    parser.add_argument("--ui-salt", required=True, help="Input viewer ui.salt file")
    parser.add_argument("--out", required=True, help="Output materialized JSON path")
    args = parser.parse_args(argv)

    source = Path(args.ui_salt)
    out = Path(args.out)
    try:
        text = source.read_text(encoding="utf-8")
        payload = materialize_text(text, source_path=stable_source_path(source))
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(json.dumps(payload, indent=2, sort_keys=False) + "\n", encoding="utf-8")
    except (OSError, MaterializerError) as exc:
        sys.stderr.write(str(exc) + "\n")
        return 2
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
