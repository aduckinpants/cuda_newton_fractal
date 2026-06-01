from __future__ import annotations

import argparse
import ast
import json
import math
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any


VALID_CONTRACT_KINDS = {
    "function_library",
    "composition_recipe",
    "explaino",
    "signal_type_registry",
    "adapter_library",
}
VALID_SIGNAL_KINDS = {"scalar", "phase", "categorical"}
VALID_SIGNAL_TYPE_KINDS = {"scalar", "phase", "category", "palette", "mask", "color", "field"}
VALID_SIGNAL_TYPE_TOPOLOGIES = {"linear", "circular", "discrete", "mask", "color", "field"}
VALID_ADAPTER_POLICIES = {"safe", "visible_default", "explicit_only", "diagnostic_only", "forbidden"}
VALID_APPLICATOR_SIGNAL_KINDS = VALID_SIGNAL_KINDS | {"any"}
VALID_APPLICATOR_TARGET_LANES = {"source"}
VALID_PARAM_TYPES = {"float", "double", "int", "bool", "enum"}
VALID_PORT_DIRECTIONS = {"input", "output"}
VALID_STATEMENTS = {
    "contract",
    "signal_type",
    "adapter",
    "lane",
    "function",
    "param",
    "port",
    "compat",
    "recipe",
    "row_applicator",
    "explaino_contract",
}


class MaterializerError(ValueError):
    pass


@dataclass
class FunctionRecord:
    lane: str
    function: dict[str, Any]
    seen_params: set[str] = field(default_factory=set)
    seen_ports: set[tuple[str, str]] = field(default_factory=set)


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


def _require_bool(kwargs: dict[str, Any], key: str, *, statement: str) -> bool:
    if key not in kwargs:
        raise MaterializerError(f"{statement} requires {key}")
    value = kwargs[key]
    if not isinstance(value, bool):
        raise MaterializerError(f"{statement} {key} must be a bool")
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



def _is_generic_type(type_id: str) -> bool:
    return type_id.startswith("generic.") and len(type_id) > len("generic.")


def _append_port(record: FunctionRecord, port: dict[str, Any]) -> None:
    key = (port["direction"], port["id"])
    if key in record.seen_ports:
        raise MaterializerError(
            f"duplicate {port['direction']} port id '{port['id']}' for function '{record.function['id']}'"
        )
    record.seen_ports.add(key)
    record.function.setdefault("ports", []).append(port)


def _build_port(function_id: str, kwargs: dict[str, Any], signal_type_ids: set[str]) -> dict[str, Any]:
    direction = _require_string(kwargs, "direction", statement="port")
    if direction not in VALID_PORT_DIRECTIONS:
        raise MaterializerError(f"port for {function_id} invalid direction '{direction}'")
    port_id = _require_string(kwargs, "id", statement="port")
    type_id = _require_string(kwargs, "type", statement="port")
    if type_id == "any":
        raise MaterializerError("port type 'any' is forbidden")
    generic_group = _optional_string(kwargs, "generic_group", "")
    if _is_generic_type(type_id):
        if not generic_group:
            raise MaterializerError(f"port {function_id}.{port_id} generic type requires generic_group")
        if generic_group == "any":
            raise MaterializerError("generic_group 'any' is forbidden")
        if type_id != f"generic.{generic_group}":
            raise MaterializerError(f"port {function_id}.{port_id} generic_group must match type '{type_id}'")
    elif type_id not in signal_type_ids:
        raise MaterializerError(f"port references unknown signal type '{type_id}'")
    elif generic_group:
        raise MaterializerError(f"port {function_id}.{port_id} non-generic type cannot declare generic_group")
    port = {
        "direction": direction,
        "id": port_id,
        "type": type_id,
    }
    if generic_group:
        port["generic_group"] = generic_group
    if _optional_bool(kwargs, "canonical", False):
        port["canonical"] = True
    return port


def _require_non_negative_int(kwargs: dict[str, Any], key: str, *, statement: str) -> int:
    value = kwargs.get(key)
    if not isinstance(value, int) or isinstance(value, bool) or value < 0:
        raise MaterializerError(f"{statement} requires non-negative integer {key}")
    return value


def _is_scalar_type(type_id: str) -> bool:
    return type_id.startswith("scalar.")


def _is_category_type(type_id: str) -> bool:
    return type_id.startswith("category.")


def _validate_adapter_semantics(adapter: dict[str, Any]) -> None:
    adapter_id = adapter["id"]
    source = adapter["source"]
    target = adapter["target"]
    policy = adapter["policy"]
    lossy = bool(adapter["lossy"])
    if lossy and policy == "safe":
        raise MaterializerError(f"adapter {adapter_id} lossy adapters cannot use safe policy")
    if policy != "safe" and not adapter["fail_closed_reason"]:
        raise MaterializerError(f"adapter {adapter_id} requires fail_closed_reason for non-safe policy")
    if _is_category_type(source) and _is_scalar_type(target) and policy in {"safe", "visible_default"}:
        raise MaterializerError(f"adapter {adapter_id} category-to-scalar adapters cannot be safe or visible_default")
    if source == "color.linear_rgb" and _is_scalar_type(target) and policy in {"safe", "visible_default"}:
        raise MaterializerError(f"adapter {adapter_id} color-to-scalar adapters cannot be safe or visible_default")
    if source in {"scalar.signed", "scalar.sdf_signed_distance"} and target == "scalar.unit" and policy in {"safe", "visible_default"}:
        raise MaterializerError(f"adapter {adapter_id} signed-to-unit adapters require explicit normalization policy")


def _build_adapter(kwargs: dict[str, Any], signal_type_ids: set[str]) -> dict[str, Any]:
    adapter_id = _require_string(kwargs, "id", statement="adapter")
    source = _require_string(kwargs, "source", statement=f"adapter {adapter_id}")
    target = _require_string(kwargs, "target", statement=f"adapter {adapter_id}")
    if source not in signal_type_ids:
        raise MaterializerError(f"adapter {adapter_id} references unknown source type '{source}'")
    if target not in signal_type_ids:
        raise MaterializerError(f"adapter {adapter_id} references unknown target type '{target}'")
    policy = _require_string(kwargs, "policy", statement=f"adapter {adapter_id}")
    if policy not in VALID_ADAPTER_POLICIES:
        raise MaterializerError(f"adapter {adapter_id} has invalid policy '{policy}'")
    adapter = {
        "id": adapter_id,
        "source": source,
        "target": target,
        "policy": policy,
        "lossy": _require_bool(kwargs, "lossy", statement=f"adapter {adapter_id}"),
        "reversible": _require_bool(kwargs, "reversible", statement=f"adapter {adapter_id}"),
        "cost": _require_non_negative_int(kwargs, "cost", statement=f"adapter {adapter_id}"),
        "fail_closed_reason": _optional_string(kwargs, "fail_closed_reason", ""),
    }
    _validate_adapter_semantics(adapter)
    return adapter


def _validate_function_ports(record: FunctionRecord) -> None:
    ports = record.function.get("ports", [])
    if not ports:
        return
    function_id = record.function["id"]
    if record.lane == "source" and any(port["direction"] == "input" for port in ports):
        raise MaterializerError(f"source function {function_id} cannot declare input ports")
    canonical_outputs = [
        port for port in ports if port["direction"] == "output" and bool(port.get("canonical", False))
    ]
    if len(canonical_outputs) != 1:
        raise MaterializerError(f"function {function_id} port signatures require exactly one canonical output")
    if function_id == "identity":
        generic_inputs = [port for port in ports if port["direction"] == "input" and _is_generic_type(port["type"])]
        generic_outputs = [port for port in ports if port["direction"] == "output" and _is_generic_type(port["type"])]
        if len(ports) != 2 or len(generic_inputs) != 1 or len(generic_outputs) != 1 or generic_inputs[0]["type"] != generic_outputs[0]["type"]:
            raise MaterializerError("identity ports must declare exactly one matching generic input and output")
    if record.lane == "shape" and function_id != "identity":
        for port in ports:
            if not port["type"].startswith("scalar."):
                raise MaterializerError(f"shape function {function_id} only supports scalar ports in Slice B")


def materialize_text(text: str, *, source_path: str = "") -> dict[str, Any]:
    contracts: list[dict[str, Any]] = []
    lanes: list[dict[str, Any]] = []
    functions: dict[str, FunctionRecord] = {}
    compatibility: list[dict[str, Any]] = []
    recipes: list[dict[str, Any]] = []
    explaino_entries: list[dict[str, Any]] = []
    row_applicators: list[dict[str, Any]] = []
    row_applicator_ids: set[str] = set()
    lane_ids: set[str] = set()
    contract_keys: set[tuple[str, str]] = set()
    signal_types: list[dict[str, Any]] = []
    signal_type_ids: set[str] = set()
    adapter_library_declared = False
    adapters: list[dict[str, Any]] = []
    adapter_ids: set[str] = set()

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
            if kind == "adapter_library":
                adapter_library_declared = True
        elif name == "signal_type":
            _check_known_args(
                name,
                kwargs,
                {
                    "id",
                    "kind",
                    "domain",
                    "topology",
                    "arity",
                    "default_adapter_policy",
                    "units",
                    "period",
                    "color_space",
                    "coordinate_space",
                },
            )
            type_id = _require_string(kwargs, "id", statement=name)
            if type_id in signal_type_ids:
                raise MaterializerError(f"duplicate signal_type id '{type_id}'")
            signal_type_ids.add(type_id)
            kind = _require_string(kwargs, "kind", statement=f"signal_type {type_id}")
            if kind not in VALID_SIGNAL_TYPE_KINDS:
                raise MaterializerError(f"signal_type {type_id} invalid signal_type kind '{kind}'")
            domain = _require_string(kwargs, "domain", statement=f"signal_type {type_id}")
            topology = _require_string(kwargs, "topology", statement=f"signal_type {type_id}")
            if topology not in VALID_SIGNAL_TYPE_TOPOLOGIES:
                raise MaterializerError(f"signal_type {type_id} invalid topology '{topology}'")
            if kind == "category" and domain == "discrete_index":
                raise MaterializerError("palette discrete_index must use kind palette")
            if kind == "palette" and domain != "discrete_index":
                raise MaterializerError(f"signal_type {type_id} palette kind requires discrete_index domain")
            arity = kwargs.get("arity")
            if not isinstance(arity, int) or isinstance(arity, bool) or arity <= 0:
                raise MaterializerError(f"signal_type {type_id} requires positive integer arity")
            adapter_policy = _require_string(kwargs, "default_adapter_policy", statement=f"signal_type {type_id}")
            if adapter_policy not in VALID_ADAPTER_POLICIES:
                raise MaterializerError(f"signal_type {type_id} invalid default_adapter_policy '{adapter_policy}'")
            signal_type = {
                "id": type_id,
                "kind": kind,
                "domain": domain,
                "topology": topology,
                "arity": arity,
                "default_adapter_policy": adapter_policy,
            }
            for optional_key in ("units", "color_space", "coordinate_space"):
                if optional_key in kwargs:
                    signal_type[optional_key] = _optional_string(kwargs, optional_key, "")
            if "period" in kwargs:
                _check_finite(kwargs["period"], f"signal_type {type_id} period")
                if not isinstance(kwargs["period"], (int, float)) or isinstance(kwargs["period"], bool) or float(kwargs["period"]) <= 0.0:
                    raise MaterializerError(f"signal_type {type_id} period must be positive")
                signal_type["period"] = kwargs["period"]
            if kind == "phase" and topology != "circular":
                raise MaterializerError(f"signal_type {type_id} phase kind requires circular topology")
            signal_types.append(signal_type)
        elif name == "adapter":
            _check_known_args(
                name,
                kwargs,
                {
                    "id",
                    "source",
                    "target",
                    "policy",
                    "lossy",
                    "reversible",
                    "cost",
                    "fail_closed_reason",
                },
            )
            adapter = _build_adapter(kwargs, signal_type_ids)
            adapter_id = adapter["id"]
            if adapter_id in adapter_ids:
                raise MaterializerError(f"duplicate adapter id '{adapter_id}'")
            adapter_ids.add(adapter_id)
            adapters.append(adapter)
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
            _check_known_args(name, kwargs, {"lane", "id", "label", "description", "taxonomy_group", "signal_kind", "typed_signal", "runtime_backed", "input_kind", "output_kind", "cost_hint", "params"})
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
            typed_signal = _optional_string(kwargs, "typed_signal", "")
            if typed_signal:
                function["typed_signal"] = typed_signal
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
        elif name == "port":
            _check_known_args(name, kwargs, {"function", "direction", "id", "type", "canonical", "generic_group"})
            function_id = _require_string(kwargs, "function", statement=name)
            if function_id not in functions:
                raise MaterializerError(f"port references unknown function '{function_id}'")
            _append_port(functions[function_id], _build_port(function_id, kwargs, signal_type_ids))
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
        elif name == "row_applicator":
            _check_known_args(
                name,
                kwargs,
                {
                    "id",
                    "label",
                    "target_lane",
                    "required_signal_kind",
                    "requires_sdf_field",
                    "storage_param",
                    "width_param",
                    "fail_closed_reason",
                },
            )
            applicator_id = _require_string(kwargs, "id", statement=name)
            if applicator_id in row_applicator_ids:
                raise MaterializerError(f"duplicate row_applicator id '{applicator_id}'")
            row_applicator_ids.add(applicator_id)
            target_lane = _require_string(kwargs, "target_lane", statement=f"row_applicator {applicator_id}")
            if target_lane not in VALID_APPLICATOR_TARGET_LANES:
                raise MaterializerError(f"row_applicator {applicator_id} has invalid target_lane '{target_lane}'")
            required_signal_kind = _require_string(kwargs, "required_signal_kind", statement=f"row_applicator {applicator_id}")
            if required_signal_kind not in VALID_APPLICATOR_SIGNAL_KINDS:
                raise MaterializerError(f"row_applicator {applicator_id} has invalid required_signal_kind '{required_signal_kind}'")
            if "requires_sdf_field" not in kwargs:
                raise MaterializerError(f"row_applicator {applicator_id} requires requires_sdf_field")
            row_applicators.append({
                "id": applicator_id,
                "label": _require_string(kwargs, "label", statement=name),
                "target_lane": target_lane,
                "required_signal_kind": required_signal_kind,
                "requires_sdf_field": _optional_bool(kwargs, "requires_sdf_field", False),
                "storage_param": _require_string(kwargs, "storage_param", statement=f"row_applicator {applicator_id}"),
                "width_param": _optional_string(kwargs, "width_param", ""),
                "fail_closed_reason": _require_string(kwargs, "fail_closed_reason", statement=f"row_applicator {applicator_id}"),
            })
        elif name == "explaino_contract":
            required = ("id", "hypothesis_space", "authority", "lens", "invariant", "proof", "fallback")
            _check_known_args(name, kwargs, set(required) | {"product_facing", "diagnostic"})
            entry = {key: _require_string(kwargs, key, statement=name) for key in required}
            entry["product_facing"] = _optional_bool(kwargs, "product_facing", False)
            entry["diagnostic"] = _optional_bool(kwargs, "diagnostic", True)
            explaino_entries.append(entry)

    if not signal_types:
        raise MaterializerError("at least one signal_type is required")
    for record in functions.values():
        if record.lane not in lane_ids:
            raise MaterializerError(f"function '{record.function['id']}' references unknown lane '{record.lane}'")
        typed_signal = record.function.get("typed_signal", "")
        if typed_signal and typed_signal not in signal_type_ids:
            raise MaterializerError(
                f"function {record.function['id']} typed_signal references unknown signal type '{typed_signal}'"
            )
        _validate_function_ports(record)
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
    if not row_applicators:
        raise MaterializerError("at least one row_applicator is required")
    if adapter_library_declared and not adapters:
        raise MaterializerError("adapter_library contract requires at least one adapter")

    return {
        "schema_version": 1,
        "source_path": source_path,
        "contracts": contracts,
        "signal_type_registry": {"types": signal_types},
        "adapter_library_contract": {"adapters": adapters},
        "function_library": {"lanes": lanes},
        "composition_recipe_contract": {
            "compatibility": compatibility,
            "row_applicators": row_applicators,
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
