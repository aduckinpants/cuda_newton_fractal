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
    "edge_resolution",
    "resolution_audit",
    "compat_override_audit",
}
VALID_SIGNAL_KINDS = {"scalar", "phase", "categorical"}
VALID_SIGNAL_TYPE_KINDS = {"scalar", "phase", "category", "palette", "mask", "color", "field"}
VALID_SIGNAL_TYPE_TOPOLOGIES = {"linear", "circular", "discrete", "mask", "color", "field"}
VALID_ADAPTER_POLICIES = {"safe", "visible_default", "explicit_only", "diagnostic_only", "forbidden"}
VALID_APPLICATOR_SIGNAL_KINDS = VALID_SIGNAL_KINDS | {"any"}
VALID_APPLICATOR_TARGET_LANES = {"source"}
VALID_PARAM_TYPES = {"float", "double", "int", "bool", "enum"}
VALID_PORT_DIRECTIONS = {"input", "output"}
VALID_RESOLUTION_EXPECTATIONS = {"resolved", "fail_closed"}
VALID_COMPAT_OVERRIDE_CLASSIFICATIONS = {"runtime_legacy_override"}
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
    "edge_policy",
    "edge_link",
    "resolution_case",
    "compat_override",
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


@dataclass
class AdapterRoute:
    adapters: list[dict[str, Any]]
    sort_key: tuple[Any, ...]


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


def _build_edge_policy(kwargs: dict[str, Any]) -> dict[str, Any]:
    policy_id = _require_string(kwargs, "id", statement="edge_policy")
    return {
        "id": policy_id,
        "max_adapter_hops": _require_non_negative_int(kwargs, "max_adapter_hops", statement=f"edge_policy {policy_id}"),
        "allow_lossy": _require_bool(kwargs, "allow_lossy", statement=f"edge_policy {policy_id}"),
        "allow_visible_default": _require_bool(kwargs, "allow_visible_default", statement=f"edge_policy {policy_id}"),
        "allow_explicit": _require_bool(kwargs, "allow_explicit", statement=f"edge_policy {policy_id}"),
        "allow_diagnostic": _require_bool(kwargs, "allow_diagnostic", statement=f"edge_policy {policy_id}"),
        "fail_closed_default": _require_bool(kwargs, "fail_closed_default", statement=f"edge_policy {policy_id}"),
    }


def _build_edge_link(kwargs: dict[str, Any], lane_ids: set[str]) -> dict[str, Any]:
    edge_id = _require_string(kwargs, "id", statement="edge_link")
    from_lane = _require_string(kwargs, "from_lane", statement=f"edge_link {edge_id}")
    to_lane = _require_string(kwargs, "to_lane", statement=f"edge_link {edge_id}")
    if from_lane not in lane_ids:
        raise MaterializerError(f"edge_link {edge_id} references unknown from_lane '{from_lane}'")
    if to_lane not in lane_ids:
        raise MaterializerError(f"edge_link {edge_id} references unknown to_lane '{to_lane}'")
    return {
        "id": edge_id,
        "from_lane": from_lane,
        "to_lane": to_lane,
        "from_port": _require_string(kwargs, "from_port", statement=f"edge_link {edge_id}"),
        "to_port": _require_string(kwargs, "to_port", statement=f"edge_link {edge_id}"),
        "fail_closed_reason": _require_string(kwargs, "fail_closed_reason", statement=f"edge_link {edge_id}"),
    }


def _build_resolution_case(kwargs: dict[str, Any]) -> dict[str, Any]:
    case_id = _require_string(kwargs, "id", statement="resolution_case")
    expectation = _require_string(kwargs, "expect", statement=f"resolution_case {case_id}")
    if expectation not in VALID_RESOLUTION_EXPECTATIONS:
        raise MaterializerError(f"resolution_case {case_id} invalid expect '{expectation}'")
    return {
        "id": case_id,
        "source": _require_string(kwargs, "source", statement=f"resolution_case {case_id}"),
        "shape": _require_string(kwargs, "shape", statement=f"resolution_case {case_id}"),
        "palette": _require_string(kwargs, "palette", statement=f"resolution_case {case_id}"),
        "grading": _require_string(kwargs, "grading", statement=f"resolution_case {case_id}"),
        "expected_status": expectation,
        "allow_lossy": _optional_bool(kwargs, "allow_lossy", False),
        "allow_visible_default": _optional_bool(kwargs, "allow_visible_default", False),
        "explicit_adapter_consent": _optional_bool(kwargs, "explicit_adapter_consent", False),
        "diagnostic_adapter_consent": _optional_bool(kwargs, "diagnostic_adapter_consent", False),
        "fail_closed_reason": _optional_string(kwargs, "fail_closed_reason", ""),
    }


def _build_compat_override(kwargs: dict[str, Any]) -> dict[str, Any]:
    override_id = _require_string(kwargs, "id", statement="compat_override")
    classification = _require_string(kwargs, "classification", statement=f"compat_override {override_id}")
    if classification not in VALID_COMPAT_OVERRIDE_CLASSIFICATIONS:
        raise MaterializerError(f"compat_override {override_id} invalid classification '{classification}'")
    return {
        "id": override_id,
        "source": _require_string(kwargs, "source", statement=f"compat_override {override_id}"),
        "palette": _require_string(kwargs, "palette", statement=f"compat_override {override_id}"),
        "grading": _require_string(kwargs, "grading", statement=f"compat_override {override_id}"),
        "classification": classification,
        "owner_seam": _require_string(kwargs, "owner_seam", statement=f"compat_override {override_id}"),
        "reason": _require_string(kwargs, "reason", statement=f"compat_override {override_id}"),
        "proof": _require_string(kwargs, "proof", statement=f"compat_override {override_id}"),
    }


def _canonical_output_port(function: dict[str, Any]) -> dict[str, Any] | None:
    for port in function.get("ports", []):
        if port["direction"] == "output" and bool(port.get("canonical", False)):
            return port
    return None


def _first_input_port(function: dict[str, Any], port_id: str) -> dict[str, Any] | None:
    for port in function.get("ports", []):
        if port["direction"] == "input" and port["id"] == port_id:
            return port
    return None


def _adapter_block_reason(adapter: dict[str, Any], policy: dict[str, Any], case: dict[str, Any]) -> str:
    if adapter["policy"] == "forbidden":
        return f"adapter {adapter['id']} is forbidden"
    if adapter["lossy"] and not case["allow_lossy"] and not policy["allow_lossy"]:
        return f"adapter {adapter['id']} is lossy and allow_lossy is false"
    if adapter["policy"] == "visible_default" and not (case["allow_visible_default"] or policy["allow_visible_default"]):
        return f"adapter {adapter['id']} requires visible_default adapter permission"
    if adapter["policy"] == "explicit_only" and not (case["explicit_adapter_consent"] or policy["allow_explicit"]):
        return f"adapter {adapter['id']} requires explicit adapter consent"
    if adapter["policy"] == "diagnostic_only" and not (case["diagnostic_adapter_consent"] or policy["allow_diagnostic"]):
        return f"adapter {adapter['id']} requires diagnostic adapter consent"
    return ""


def _adapter_policy_rank(adapter: dict[str, Any]) -> int:
    return {
        "safe": 0,
        "visible_default": 1,
        "explicit_only": 2,
        "diagnostic_only": 3,
        "forbidden": 4,
    }.get(adapter["policy"], 99)


def _find_adapter_route(
    source_type: str,
    target_type: str,
    adapters: list[dict[str, Any]],
    policy: dict[str, Any],
    case: dict[str, Any],
) -> tuple[AdapterRoute | None, str]:
    if source_type == target_type:
        return AdapterRoute(adapters=[], sort_key=(0, 0, 0, 0, ())), ""

    max_hops = policy["max_adapter_hops"]
    adapter_order = {adapter["id"]: index for index, adapter in enumerate(adapters)}
    candidates: list[AdapterRoute] = []
    blocked_reasons: list[str] = []
    frontier: list[tuple[str, list[dict[str, Any]], set[str]]] = [(source_type, [], {source_type})]

    while frontier:
        current_type, path, visited_types = frontier.pop(0)
        if len(path) >= max_hops:
            continue
        for adapter in adapters:
            if adapter["source"] != current_type:
                continue
            reason = _adapter_block_reason(adapter, policy, case)
            if reason:
                blocked_reasons.append(reason)
                continue
            next_type = adapter["target"]
            if next_type in visited_types:
                continue
            next_path = path + [adapter]
            if next_type == target_type:
                worst_policy = max(_adapter_policy_rank(item) for item in next_path)
                any_lossy = int(any(bool(item["lossy"]) for item in next_path))
                total_cost = sum(int(item["cost"]) for item in next_path)
                route_order = tuple(adapter_order[item["id"]] for item in next_path)
                candidates.append(
                    AdapterRoute(
                        adapters=next_path,
                        sort_key=(1, worst_policy, any_lossy, total_cost, len(next_path), route_order),
                    )
                )
                continue
            frontier.append((next_type, next_path, visited_types | {next_type}))

    if candidates:
        candidates.sort(key=lambda item: item.sort_key)
        return candidates[0], ""
    if blocked_reasons:
        return None, blocked_reasons[0]
    return None, f"no adapter route from {source_type} to {target_type}"


def _resolve_edge(
    edge: dict[str, Any],
    from_function: dict[str, Any],
    to_function: dict[str, Any],
    current_type: str,
    adapters: list[dict[str, Any]],
    policy: dict[str, Any],
    case: dict[str, Any],
) -> tuple[dict[str, Any] | None, str, str]:
    input_port = _first_input_port(to_function, edge["to_port"])
    if input_port is None:
        return None, "", f"function {to_function['id']} lacks input port '{edge['to_port']}'"

    target_type = input_port["type"]
    chosen_adapters: list[dict[str, Any]] = []
    if _is_generic_type(target_type):
        resolved_input_type = current_type
        route_status = "direct"
    else:
        route, reason = _find_adapter_route(current_type, target_type, adapters, policy, case)
        if route is None:
            return None, "", reason or edge["fail_closed_reason"]
        chosen_adapters = route.adapters
        resolved_input_type = target_type
        route_status = "direct" if not chosen_adapters else "adapted"

    output_port = _canonical_output_port(to_function)
    if output_port is None:
        return None, "", f"function {to_function['id']} lacks canonical output port"
    output_type = output_port["type"]
    if _is_generic_type(output_type):
        output_type = resolved_input_type

    route_edge = {
        "edge_id": edge["id"],
        "from_function": from_function["id"],
        "to_function": to_function["id"],
        "from_type": current_type,
        "to_type": resolved_input_type,
        "output_type": output_type,
        "status": route_status,
        "adapters": [adapter["id"] for adapter in chosen_adapters],
        "adapter_hops": len(chosen_adapters),
        "adapter_cost": sum(int(adapter["cost"]) for adapter in chosen_adapters),
    }
    return route_edge, output_type, ""


def _resolve_case(
    case: dict[str, Any],
    functions: dict[str, FunctionRecord],
    edge_links: list[dict[str, Any]],
    adapters: list[dict[str, Any]],
    policy: dict[str, Any],
) -> dict[str, Any]:
    function_ids = [case["source"], case["shape"], case["palette"], case["grading"]]
    for function_id in function_ids:
        if function_id not in functions:
            raise MaterializerError(f"resolution_case {case['id']} references unknown function '{function_id}'")

    if len(edge_links) != 3:
        raise MaterializerError("resolution_audit requires exactly three linear edge_link statements")

    source_function = functions[case["source"]].function
    output_port = _canonical_output_port(source_function)
    if output_port is None:
        raise MaterializerError(f"resolution_case {case['id']} source lacks canonical output port")
    current_type = output_port["type"]

    route_edges: list[dict[str, Any]] = []
    chosen_adapters: list[str] = []
    ordered_functions = [
        source_function,
        functions[case["shape"]].function,
        functions[case["palette"]].function,
        functions[case["grading"]].function,
    ]

    fail_reason = ""
    for index, edge in enumerate(edge_links):
        route_edge, next_type, reason = _resolve_edge(
            edge,
            ordered_functions[index],
            ordered_functions[index + 1],
            current_type,
            adapters,
            policy,
            case,
        )
        if route_edge is None:
            fail_reason = case["fail_closed_reason"] or reason or edge["fail_closed_reason"]
            break
        route_edges.append(route_edge)
        chosen_adapters.extend(route_edge["adapters"])
        current_type = next_type

    status = "fail_closed" if fail_reason else "resolved"
    if status == "fail_closed" and not fail_reason:
        fail_reason = case["fail_closed_reason"] or "typed resolver failed closed"

    if case["expected_status"] == "resolved" and status != "resolved":
        raise MaterializerError(
            f"resolution_case {case['id']} expected resolved but got fail_closed: {fail_reason}"
        )
    if case["expected_status"] == "fail_closed" and status == "resolved":
        raise MaterializerError(f"resolution_case {case['id']} expected fail_closed but resolved")
    if case["expected_status"] == "fail_closed" and not case["fail_closed_reason"]:
        raise MaterializerError(f"resolution_case {case['id']} expected fail_closed requires fail_closed_reason")

    adapter_hops = sum(int(edge["adapter_hops"]) for edge in route_edges)
    adapter_cost = sum(int(edge["adapter_cost"]) for edge in route_edges)

    return {
        "id": case["id"],
        "source": case["source"],
        "shape": case["shape"],
        "palette": case["palette"],
        "grading": case["grading"],
        "expected_status": case["expected_status"],
        "status": status,
        "allow_lossy": case["allow_lossy"],
        "allow_visible_default": case["allow_visible_default"],
        "explicit_adapter_consent": case["explicit_adapter_consent"],
        "diagnostic_adapter_consent": case["diagnostic_adapter_consent"],
        "chosen_adapters": chosen_adapters,
        "adapter_hops": adapter_hops,
        "adapter_cost": adapter_cost,
        "tie_break_rule": "exact_identity_safe_non_lossy_lower_cost_fewer_hops_declaration_order",
        "policy_blockers": [] if status == "resolved" else [fail_reason],
        "route_edges": route_edges,
        "fail_closed_reason": "" if status == "resolved" else fail_reason,
    }


def _build_compatibility_audit(
    compatibility: list[dict[str, Any]],
    compat_overrides: list[dict[str, Any]],
    resolved_cases: list[dict[str, Any]],
    function_ids: set[str],
) -> list[dict[str, Any]]:
    typed_routes: dict[tuple[str, str, str], dict[str, Any]] = {}
    for resolution_case in resolved_cases:
        if (
            resolution_case["status"] == "resolved"
            and resolution_case["shape"] == "identity"
            and not resolution_case["chosen_adapters"]
        ):
            typed_routes[(resolution_case["source"], resolution_case["palette"], resolution_case["grading"])] = resolution_case

    override_routes: dict[tuple[str, str, str], dict[str, Any]] = {}
    override_ids: set[str] = set()
    for override in compat_overrides:
        if override["id"] in override_ids:
            raise MaterializerError(f"duplicate compat_override id '{override['id']}'")
        override_ids.add(override["id"])
        for role in ("source", "palette", "grading"):
            if override[role] not in function_ids:
                raise MaterializerError(f"compat_override {override['id']} references unknown {role} '{override[role]}'")
        key = (override["source"], override["palette"], override["grading"])
        if key in override_routes:
            raise MaterializerError(
                f"duplicate compat_override route {override['source']} + {override['palette']} + {override['grading']}"
            )
        override_routes[key] = override

    audit: list[dict[str, Any]] = []
    used_override_keys: set[tuple[str, str, str]] = set()
    for row in compatibility:
        key = (row["source"], row["palette"], row["grading"])
        if key in typed_routes:
            route = typed_routes[key]
            audit.append({
                "source": row["source"],
                "palette": row["palette"],
                "grading": row["grading"],
                "mode": row["mode"],
                "classification": "typed_resolved",
                "route_case_id": route["id"],
                "override_id": "",
                "reason": "typed resolver route covers this compatibility row",
            })
            continue
        if key in override_routes:
            override = override_routes[key]
            used_override_keys.add(key)
            audit.append({
                "source": row["source"],
                "palette": row["palette"],
                "grading": row["grading"],
                "mode": row["mode"],
                "classification": override["classification"],
                "route_case_id": "",
                "override_id": override["id"],
                "reason": override["reason"],
            })
            continue
        raise MaterializerError(
            f"compat row {row['source']} + {row['palette']} + {row['grading']} is not typed-resolved and has no compat_override"
        )
    for key, override in override_routes.items():
        if key not in used_override_keys:
            raise MaterializerError(f"compat_override {override['id']} does not map to a compatibility row")
    return audit


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
    compat_override_audit_declared = False
    compat_overrides: list[dict[str, Any]] = []
    edge_policy: dict[str, Any] | None = None
    edge_links: list[dict[str, Any]] = []
    edge_link_ids: set[str] = set()
    resolution_cases: list[dict[str, Any]] = []
    resolution_case_ids: set[str] = set()

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
            if kind == "compat_override_audit":
                compat_override_audit_declared = True
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
        elif name == "compat_override":
            _check_known_args(name, kwargs, {"id", "source", "palette", "grading", "classification", "owner_seam", "reason", "proof"})
            compat_overrides.append(_build_compat_override(kwargs))
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
        elif name == "edge_policy":
            _check_known_args(
                name,
                kwargs,
                {
                    "id",
                    "max_adapter_hops",
                    "allow_lossy",
                    "allow_visible_default",
                    "allow_explicit",
                    "allow_diagnostic",
                    "fail_closed_default",
                },
            )
            if edge_policy is not None:
                raise MaterializerError("duplicate edge_policy")
            edge_policy = _build_edge_policy(kwargs)
        elif name == "edge_link":
            _check_known_args(
                name,
                kwargs,
                {"id", "from_lane", "to_lane", "from_port", "to_port", "fail_closed_reason"},
            )
            edge_link = _build_edge_link(kwargs, lane_ids)
            if edge_link["id"] in edge_link_ids:
                raise MaterializerError(f"duplicate edge_link id '{edge_link['id']}'")
            edge_link_ids.add(edge_link["id"])
            edge_links.append(edge_link)
        elif name == "resolution_case":
            _check_known_args(
                name,
                kwargs,
                {
                    "id",
                    "source",
                    "shape",
                    "palette",
                    "grading",
                    "expect",
                    "allow_lossy",
                    "allow_visible_default",
                    "explicit_adapter_consent",
                    "diagnostic_adapter_consent",
                    "fail_closed_reason",
                },
            )
            resolution_case = _build_resolution_case(kwargs)
            if resolution_case["id"] in resolution_case_ids:
                raise MaterializerError(f"duplicate resolution_case id '{resolution_case['id']}'")
            resolution_case_ids.add(resolution_case["id"])
            resolution_cases.append(resolution_case)
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
    if (edge_policy is None) != (not edge_links and not resolution_cases):
        raise MaterializerError("edge_resolution requires edge_policy, edge_link, and resolution_case statements together")

    resolved_cases: list[dict[str, Any]] = []
    if edge_policy is not None:
        if not edge_links:
            raise MaterializerError("edge_resolution requires at least one edge_link")
        if not resolution_cases:
            raise MaterializerError("resolution_audit requires at least one resolution_case")
        resolved_cases = [
            _resolve_case(case, functions, edge_links, adapters, edge_policy)
            for case in resolution_cases
        ]
    compatibility_audit: list[dict[str, Any]] = []
    if compat_override_audit_declared:
        compatibility_audit = _build_compatibility_audit(
            compatibility,
            compat_overrides,
            resolved_cases,
            set(functions.keys()),
        )

    payload = {
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
    if compat_override_audit_declared:
        payload["composition_recipe_contract"]["compat_overrides"] = compat_overrides
        payload["composition_recipe_contract"]["compatibility_audit"] = compatibility_audit
    if edge_policy is not None:
        payload["edge_resolution_contract"] = {
            "policy": edge_policy,
            "edges": edge_links,
        }
        payload["color_pipeline_resolution_audit"] = {
            "cases": resolved_cases,
        }
    return payload


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
