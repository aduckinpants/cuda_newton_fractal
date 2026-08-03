from __future__ import annotations

import argparse
import hashlib
import json
import re
import subprocess
from collections import Counter
from pathlib import Path
from typing import Any


SCHEMA_VERSION = "viewer_host.precision_authority_inventory.v1"
UI_SCHEMA_PATH = Path("ui/fractal_binding_surface_v1.ui_schema.json")
PIPELINE_CONTRACT_PATH = Path("docs/ui_salt/generated/color_pipeline_function_library.contract.v1.json")
SCHEMA_BINDING_PATH = Path("ui_app/src/schema_binding.cpp")
STATE_IO_PATH = Path("ui_app/src/diagnostics_state_io.cpp")
STATE_CAPTURE_PATH = Path("ui_app/src/diagnostics_capture.cpp")
FRACTAL_TYPES_PATH = Path("ui_app/src/fractal_types.h")
EXPLAINO_SEED_PATH = Path("ui_app/src/explaino_seed.cpp")
COLOR_PIPELINE_CORE_PATH = Path("ui_app/src/color_pipeline_core.h")
COLOR_PIPELINE_WINDOW_PATH = Path("ui_app/src/color_pipeline_window.h")
TIER_RESOLVER_PATH = Path("ui_app/src/sample_tier_resolver.cpp")
SAMPLE_DEVICE_PATH = Path("ui_app/src/fractal_sample_device.inl")
ENUM_IDS_PATH = Path("ui_app/src/enum_id_utils.h")
FAMILY_RULES_PATH = Path("ui_app/src/fractal_family_rules.h")
SPECIALIZED_FORMULAS_PATH = Path("ui_app/src/escape_time_specialized_formulas.h")

NUMERIC_CONTROL_TYPES = {
    "drag_float",
    "slider_float",
    "drag_double",
    "slider_double",
    "drag_int",
    "slider_int",
}


def _read_text(repo_root: Path, relative: Path) -> str:
    return (repo_root / relative).read_text(encoding="utf-8")


def _read_json(repo_root: Path, relative: Path) -> dict[str, Any]:
    return json.loads(_read_text(repo_root, relative))


def _sha256(repo_root: Path, relative: Path) -> str:
    return hashlib.sha256((repo_root / relative).read_bytes()).hexdigest()


def _source_commit(repo_root: Path) -> str:
    result = subprocess.run(
        ["git", "rev-parse", "HEAD"],
        cwd=repo_root,
        check=True,
        capture_output=True,
        text=True,
    )
    return result.stdout.strip()


def _function_body(source: str, signature: str) -> str:
    start = source.find(signature)
    if start < 0:
        raise ValueError(f"missing required source signature: {signature}")
    opening = source.find("{", start)
    if opening < 0:
        raise ValueError(f"missing opening brace for: {signature}")
    depth = 0
    for index in range(opening, len(source)):
        char = source[index]
        if char == "{":
            depth += 1
        elif char == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:index]
    raise ValueError(f"unterminated source function: {signature}")


def _quoted_paths(source: str) -> set[str]:
    return set(re.findall(r'"(fractal\.[^"]+)"', source))


def _binding_path_types(schema_binding: str) -> dict[str, str]:
    float_signatures = (
        "bool BindExplainoRootCoordinate",
        "bool BindColorPanelFloat",
        "bool BindExplainoRootSdfFloat",
        "bool BindViewFloat",
        "bool BindingContext::BindFloat",
    )
    typed_paths: dict[str, str] = {}
    for signature in float_signatures:
        for path in _quoted_paths(_function_body(schema_binding, signature)):
            typed_paths[path] = "float"
    for path in _quoted_paths(_function_body(schema_binding, "bool BindingContext::BindDouble")):
        typed_paths[path] = "double"
    for path in _quoted_paths(_function_body(schema_binding, "bool BindingContext::BindInt")):
        typed_paths[path] = "int"
    return typed_paths


def _general_double_input_format(schema_binding: str) -> str:
    body = _function_body(schema_binding, "const char* GeneralDoubleControlRoundTripFormat")
    match = re.search(r'return\s+"([^"]+)"\s*;', body)
    if not match:
        raise ValueError("general double round-trip format is not source-proven")
    return match.group(1)


def _numeric_controls(ui_schema: dict[str, Any], schema_binding: str) -> list[dict[str, Any]]:
    binding_types = _binding_path_types(schema_binding)
    special_routes = _special_numeric_routes(schema_binding)
    general_double_input_format = _general_double_input_format(schema_binding)
    controls: list[dict[str, Any]] = []
    for panel in ui_schema.get("panels", []):
        for control in panel.get("controls", []):
            control_type = str(control.get("type", ""))
            if control_type not in NUMERIC_CONTROL_TYPES:
                continue
            binding = control.get("binding") or {}
            path = str(binding.get("path", ""))
            storage = binding_types.get(path, "unresolved")
            input_format = _input_format(control, path, general_double_input_format)
            route = special_routes.get(
                path,
                {
                    "edit_route": "direct_binding",
                    "authoritative_storage": storage,
                    "editor_carrier": storage,
                },
            )
            classification, basis = _authoring_classification(
                control,
                storage,
                input_format,
                route,
            )
            controls.append(
                {
                    "panel_id": str(panel.get("id", "")),
                    "control_id": str(control.get("id", "")),
                    "control_type": control_type,
                    "declared_value_type": str(control.get("value_type", "")),
                    "binding_path": path,
                    "binding_storage": storage,
                    "authoritative_storage": route["authoritative_storage"],
                    "editor_carrier": route["editor_carrier"],
                    "edit_route": route["edit_route"],
                    "authorability_status": "schema_ui_numeric_authoring_route",
                    "declared_applicability": control.get("visible_if", "always_visible_in_panel"),
                    "declared_range": {
                        key: control[key]
                        for key in ("min", "max", "ui_min", "ui_max")
                        if key in control
                    },
                    "input_format": input_format,
                    "step": control.get("step"),
                    "logarithmic": bool(control.get("logarithmic", False)),
                    "classification": classification,
                    "classification_basis": basis,
                    "classification_confidence": "source_proven_authoring_route",
                    "state_load_conversion": "not_joined_requires_phase3_owner_trace",
                    "state_save_policy": "global_max_digits10_serializer_path_membership_not_joined",
                    "runtime_consumption": "not_proven_by_ui_binding",
                    "authority_ref": f"{UI_SCHEMA_PATH.as_posix()}#{panel.get('id')}/{control.get('id')}",
                }
            )
    return controls


def _special_numeric_routes(schema_binding: str) -> dict[str, dict[str, str]]:
    display_body = _function_body(schema_binding, "bool TryGetFloatControlDisplayValue")
    edit_body = _function_body(schema_binding, "bool ApplyFloatControlEdit")
    double_edit_body = _function_body(schema_binding, "bool ApplyDoubleControlEdit")
    zoom_render_body = _function_body(schema_binding, "bool RenderCameraZoomControl")
    get_int_body = _function_body(schema_binding, "bool BindingContext::GetIntValue")
    set_int_body = _function_body(schema_binding, "bool BindingContext::SetIntValue")
    routes: dict[str, dict[str, str]] = {}
    for axis in ("x", "y"):
        path = f"fractal.view.center.{axis}"
        hp_member = f"center_hp_{axis}"
        if (
            f'binding.path == "{path}"' in display_body
            and hp_member in display_body
            and f'binding.path == "{path}"' in edit_body
            and hp_member in edit_body
            and "ImGui::InputFloat" in schema_binding
        ):
            routes[path] = {
                "edit_route": "camera_hp_double_via_float_editor",
                "authoritative_storage": "double",
                "editor_carrier": "float",
            }
    if (
        'binding.path == "fractal.view.zoom"' in display_body
        and "log2_zoom" in display_body
        and 'binding.path == "fractal.view.zoom"' in edit_body
        and "log2_zoom" in edit_body
        and "ImGui::InputDouble" in zoom_render_body
    ):
        routes["fractal.view.zoom"] = {
            "edit_route": "camera_log2_double_via_double_editor",
            "authoritative_storage": "double_log2",
            "editor_carrier": "double",
        }
    if (
        "IsResolutionLongEdgePath(path)" in get_int_body
        and "ResolutionLongEdge(*render)" in get_int_body
        and "IsResolutionLongEdgePath(path)" in set_int_body
        and "ApplyResolutionLongEdge(*render, value)" in set_int_body
    ):
        routes["fractal.render.resolution.long_edge"] = {
            "edit_route": "resolution_long_edge_to_int2",
            "authoritative_storage": "int2",
            "editor_carrier": "int",
        }
    if "ExplainoSeedSetCombined(*ctx.view, *ctx.params, nextValue)" in double_edit_body:
        for path in (
            "fractal.params.explaino_seed",
            "fractal.root_pattern.dynamics.seed",
        ):
            if f'binding.path == "{path}"' in double_edit_body:
                routes[path] = {
                    "edit_route": "combined_explaino_seed_double",
                    "authoritative_storage": "double_plus_derived_float_fields",
                    "editor_carrier": "double",
                }
    return routes


def _input_format(control: dict[str, Any], path: str, general_double_input_format: str) -> str:
    control_type = str(control.get("type", ""))
    if control_type in {"slider_double", "drag_double"}:
        return general_double_input_format
    if control_type in {"slider_float", "drag_float"}:
        if path == "fractal.view.zoom" or bool(control.get("logarithmic", False)):
            return "%.9g"
        return "%.5f"
    return "integer"


def _authoring_classification(
    control: dict[str, Any],
    storage: str,
    input_format: str,
    route: dict[str, str],
) -> tuple[str, str]:
    declared = str(control.get("value_type", ""))
    control_id = str(control.get("id", ""))
    if route["edit_route"] == "combined_explaino_seed_double":
        return (
            "INTENTIONAL_MIXED_PRECISION",
            "roundtrip_double_editor_projects_fraction_to_float_backed_seed_drift",
        )
    if route["edit_route"] == "resolution_long_edge_to_int2":
        return (
            "INTENTIONAL_MIXED_PRECISION",
            "single_integer_authoring_projects_to_aspect_preserving_int2_resolution",
        )
    if route["edit_route"] == "camera_hp_double_via_float_editor":
        return (
            "AUTHORING_IDENTITY_LOSS",
            "camera_hp_double_authority_roundtrips_through_float_editor_and_fixed_five_decimal_input",
        )
    if route["edit_route"] == "camera_log2_double_via_double_editor":
        return (
            "AUTHORING_IDENTITY_LOSS",
            "camera_log2_double_authority_uses_nine_significant_digit_linear_zoom_input",
        )
    if storage == "unresolved":
        return "UNSUPPORTED_OR_NONAUTHORABLE", "binding_path_did_not_resolve_in_static_binding_owner"
    if declared and declared != storage:
        return "AUTHORING_IDENTITY_LOSS", "schema_value_type_and_binding_storage_disagree"
    if storage == "double" and input_format == "%.6f":
        if control_id == "explaino_seed":
            return "AUTHORING_IDENTITY_LOSS", "combined_double_seed_uses_fixed_six_decimal_edit_format"
        return "AUTHORING_IDENTITY_LOSS", "double_storage_uses_fixed_six_decimal_edit_format"
    if storage == "float" and input_format != "%.9g":
        return "AUTHORING_IDENTITY_LOSS", "float_storage_edit_format_is_not_roundtrip_precision"
    if storage == "float":
        return "TRUTHFUL_FLOAT32", "float_input_uses_nine_significant_digits"
    if storage == "double":
        return "TRUTHFUL_FLOAT64", "double_binding_and_roundtrip_capable_input"
    return "INTENTIONAL_MIXED_PRECISION", "integer_authoring_is_exact_but_outside_float_width_audit"


def _pipeline_inventory(
    contract: dict[str, Any],
    core_source: str,
    window_source: str,
    fractal_types_source: str,
) -> dict[str, Any]:
    struct_fields = _struct_field_types(fractal_types_source)
    runtime_owner_names = (
        "ColorPipelineSourceRuntimeParams",
        "ColorPipelineShapeRuntimeParams",
        "ColorPipelinePaletteRuntimeParams",
        "ColorPipelineGradingRuntimeParams",
        "KernelParams",
    )
    runtime_consumer_owners: list[dict[str, Any]] = []
    runtime_numeric_types: set[str] = set()
    for owner_name in runtime_owner_names:
        numeric_fields = []
        for field_name, field_type in sorted(struct_fields.get(owner_name, {}).items()):
            if field_type not in {"float", "int", "double"}:
                continue
            if owner_name == "KernelParams" and not _color_pipeline_kernel_field(field_name):
                continue
            numeric_fields.append({"field": field_name, "storage_type": field_type})
            runtime_numeric_types.add(field_type)
        if numeric_fields:
            runtime_consumer_owners.append({
                "owner_struct": owner_name,
                "numeric_fields": numeric_fields,
            })

    format_match = re.search(
        r'kColorPipelineFloatInputFormat\s*=\s*"([^"]+)"',
        core_source,
    )
    float_editor_format = format_match.group(1) if format_match else "unresolved"
    import_signature = "inline double PreserveImportedColorPipelineFloat"
    if import_signature in core_source:
        import_body = _function_body(core_source, import_signature)
    elif "inline double NormalizeImportedColorPipelineNumber" in core_source:
        import_body = _function_body(core_source, "inline double NormalizeImportedColorPipelineNumber")
    else:
        import_body = ""
    exact_float_readback = (
        "static_cast<double>(value)" in import_body
        and "round" not in import_body
    )
    exact_float_identity = (
        "left.number_value == right.number_value" in window_source
        and "if (*target != value)" in window_source
        and "if (*target != value)" in core_source
    )
    float_lowering_is_source_proven = (
        "ApplySupportedColorPipelineParamsToLive" in window_source
        and "static_cast<float>" in window_source
        and "float" in runtime_numeric_types
        and "double" not in runtime_numeric_types
    )

    parameters: list[dict[str, Any]] = []
    lanes = contract.get("function_library", {}).get("lanes", [])
    for lane in lanes:
        for function in lane.get("functions", []):
            for index, parameter in enumerate(function.get("params", [])):
                declared_type = str(parameter.get("type", "unknown"))
                if declared_type == "float":
                    classification = (
                        "TRUTHFUL_FLOAT32"
                        if float_editor_format == "%.9g" and exact_float_readback and exact_float_identity and float_lowering_is_source_proven
                        else "AUTHORING_IDENTITY_LOSS"
                    )
                    editor_carrier = "float"
                    runtime_consumption = (
                        "float_backed_color_pipeline_runtime_owner"
                        if float_lowering_is_source_proven
                        else "unresolved_float_runtime_owner"
                    )
                elif declared_type == "int":
                    classification = "TRUTHFUL_INTEGER"
                    editor_carrier = "int"
                    runtime_consumption = "int_backed_color_pipeline_runtime_owner"
                elif declared_type == "enum":
                    classification = "TRUTHFUL_ENUM"
                    editor_carrier = "enum"
                    runtime_consumption = "enum_backed_color_pipeline_runtime_owner"
                else:
                    classification = "UNSUPPORTED_OR_NONAUTHORABLE"
                    editor_carrier = "unresolved"
                    runtime_consumption = "unresolved"
                parameters.append({
                    "lane_id": lane.get("id"),
                    "function_id": function.get("id"),
                    "parameter_index": index,
                    "path": parameter.get("path"),
                    "declared_type": declared_type,
                    "minimum": parameter.get("min"),
                    "maximum": parameter.get("max"),
                    "step": parameter.get("step"),
                    "default": parameter.get("default"),
                    "draft_carrier": "double" if declared_type in {"float", "int"} else declared_type,
                    "editor_carrier": editor_carrier,
                    "authorability_status": "compiled_contract_parameter",
                    "classification": classification,
                    "classification_confidence": "compiled_contract_and_shared_source_owners_proven",
                    "state_load_conversion": (
                        "runtime_owner_normalizes_on_explicit_apply"
                        if declared_type == "float"
                        else "typed_contract_validation"
                    ),
                    "runtime_consumption": runtime_consumption,
                })

    type_counts = Counter(str(parameter.get("declared_type", "unknown")) for parameter in parameters)
    carrier = "double" if re.search(r"\bdouble\s+number_value\s*=", core_source) else "unresolved"
    classifications = Counter(parameter["classification"] for parameter in parameters)
    all_supported = not classifications.get("UNSUPPORTED_OR_NONAUTHORABLE")
    all_float_truthful = classifications.get("TRUTHFUL_FLOAT32", 0) == type_counts.get("float", 0)
    return {
        "authority_kind": "compiled_ui_salt_contract",
        "contract_parameter_count": len(parameters),
        "parameter_type_counts": dict(sorted(type_counts.items())),
        "compiled_double_parameter_count": type_counts.get("double", 0),
        "runtime_number_carrier": carrier,
        "float_editor_format": float_editor_format,
        "float_identity_comparison": (
            "exact_binary32" if exact_float_identity else "fixed_decimal_tolerance_loss"
        ),
        "float_readback": (
            "exact_binary32_promotion"
            if exact_float_readback
            else "decimal_rounding_identity_loss"
        ),
        "runtime_numeric_storage_types": sorted(runtime_numeric_types),
        "runtime_consumer_owner_count": len(runtime_consumer_owners),
        "runtime_consumer_owners": runtime_consumer_owners,
        "parameters": parameters,
        "classification_counts": dict(sorted(classifications.items())),
        "classification": (
            "INTENTIONAL_MIXED_PRECISION"
            if carrier == "double" and all_supported and all_float_truthful
            else "AUTHORING_IDENTITY_LOSS"
        ),
        "classification_basis": (
            "compiled types select shared typed editor and runtime owners; the double draft carrier "
            "preserves input until explicit application normalizes float descriptors to binary32"
        ),
    }


def _color_pipeline_kernel_field(field_name: str) -> bool:
    return field_name.startswith("color_") or field_name == "exposure"


def _brace_range(source: str, opening: int) -> tuple[int, int]:
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return opening, index
    raise ValueError("unterminated source brace block")


def _struct_field_types(fractal_types_source: str) -> dict[str, dict[str, str]]:
    structs: dict[str, dict[str, str]] = {}
    for match in re.finditer(r"(?m)^struct\s+([A-Za-z_]\w*)\s*\{", fractal_types_source):
        struct_name = match.group(1)
        opening = fractal_types_source.find("{", match.start())
        _, closing = _brace_range(fractal_types_source, opening)
        body = fractal_types_source[opening + 1:closing]
        fields: dict[str, str] = {}
        for raw_line in body.splitlines():
            line = raw_line.strip()
            if not line or line.startswith(("//", "static ")):
                continue
            field_match = re.match(
                r"([A-Za-z_]\w*(?:::\w+)*)\s+([A-Za-z_]\w*)(?:\[[^\]]+\])?\s*(?:\{|;)",
                line,
            )
            if field_match:
                fields[field_match.group(2)] = field_match.group(1)
        structs[struct_name] = fields
    return structs


def _loader_function_ranges(state_io_source: str) -> list[dict[str, Any]]:
    ranges: list[dict[str, Any]] = []
    signature_pattern = re.compile(r"(?m)^(?:bool|void)\s+([A-Za-z_]\w*)\s*\(")
    for match in signature_pattern.finditer(state_io_source):
        opening = state_io_source.find("{", match.end())
        if opening < 0:
            continue
        _, closing = _brace_range(state_io_source, opening)
        ranges.append({"name": match.group(1), "start": match.start(), "end": closing, "source": state_io_source[match.start():closing + 1]})
    return ranges


def _function_variable_types(function_source: str, struct_fields: dict[str, dict[str, str]]) -> dict[str, str]:
    known_types = sorted({"float", "double", *struct_fields}, key=len, reverse=True)
    type_pattern = "|".join(re.escape(type_name) for type_name in known_types)
    variables: dict[str, str] = {}
    for match in re.finditer(rf"\b({type_pattern})\s*(?:const\s+)?(?:\*|&)?\s*([A-Za-z_]\w*)", function_source):
        variables[match.group(2)] = match.group(1)
    return variables


def _assignment_destination_type(raw_line: str, variables: dict[str, str], struct_fields: dict[str, dict[str, str]]) -> tuple[str, str]:
    assignment = raw_line.split("=", 1)[0].strip().lstrip("*").strip()
    normalized = re.sub(r"\[[^\]]+\]", "", assignment.replace("->", "."))
    parts = [part for part in normalized.split(".") if part]
    if not parts or parts[0] not in variables:
        return assignment, "unresolved"
    destination_type = variables[parts[0]]
    for field_name in parts[1:]:
        owner_fields = struct_fields.get(destination_type)
        if owner_fields is None or field_name not in owner_fields:
            return assignment, "unresolved"
        destination_type = owner_fields[field_name]
    return assignment, destination_type


def _float_cast_sites(state_io_source: str, fractal_types_source: str) -> list[dict[str, Any]]:
    struct_fields = _struct_field_types(fractal_types_source)
    function_ranges = _loader_function_ranges(state_io_source)
    sites: list[dict[str, Any]] = []
    offset = 0
    for line_number, raw_line in enumerate(state_io_source.splitlines(keepends=True), start=1):
        line_start = offset
        offset += len(raw_line)
        if "static_cast<float>" not in raw_line:
            continue
        owners = [item for item in function_ranges if item["start"] <= line_start <= item["end"]]
        if not owners:
            owner_name = "unresolved"
            variables: dict[str, str] = {}
        else:
            owner = min(owners, key=lambda item: item["end"] - item["start"])
            owner_name = str(owner["name"])
            variables = _function_variable_types(str(owner["source"]), struct_fields)
        target, destination_type = _assignment_destination_type(raw_line, variables, struct_fields)
        scalar_type = "float" if destination_type in {"float", "Float2"} else "unresolved"
        sites.append({
            "line": line_number,
            "snippet": raw_line.strip(),
            "owner_function": owner_name,
            "assignment_target": target,
            "destination_type": destination_type,
            "destination_scalar_type": scalar_type,
            "classification": "TRUTHFUL_FLOAT32" if scalar_type == "float" else "NEEDS_RUNTIME_WITNESS",
        })
    return sites


def _seed_normalized_state_members(explaino_seed_source: str) -> set[tuple[str, str]]:
    normalize_body = _function_body(explaino_seed_source, "void ExplainoSeedNormalize")
    if "ExplainoSeedSetCombined" not in normalize_body:
        return set()
    setter_body = _function_body(explaino_seed_source, "void ExplainoSeedSetCombined")
    members: set[tuple[str, str]] = set()
    for variable_name, struct_name in (("view", "ViewState"), ("params", "KernelParams")):
        for field_name in re.findall(rf"\b{variable_name}\.([A-Za-z_]\w*)\s*=", setter_body):
            members.add((struct_name, field_name))
    return members


def _serialized_double_state_owners(state_io_source: str, capture_source: str, fractal_types_source: str, explaino_seed_source: str) -> list[dict[str, str]]:
    struct_fields = _struct_field_types(fractal_types_source)
    normalized_members = _seed_normalized_state_members(explaino_seed_source)
    owners: list[dict[str, str]] = []
    for struct_name, load_prefix, save_prefix in (("ViewState", "nextView", "view"), ("KernelParams", "nextParams", "params")):
        for field_name, field_type in sorted(struct_fields.get(struct_name, {}).items()):
            if field_type != "double":
                continue
            load_assignment = f"{load_prefix}.{field_name} ="
            save_reference = f"{save_prefix}.{field_name}"
            if load_assignment not in state_io_source or save_reference not in capture_source:
                continue
            owners.append({
                "owner_struct": struct_name,
                "owner_member": field_name,
                "storage_type": "double",
                "load_assignment": load_assignment,
                "save_reference": save_reference,
                "classification": (
                    "INTENTIONAL_MIXED_PRECISION"
                    if (struct_name, field_name) in normalized_members
                    else "TRUTHFUL_FLOAT64"
                ),
                "normalization_owner": (
                    "ExplainoSeedNormalize"
                    if (struct_name, field_name) in normalized_members
                    else "none"
                ),
            })
    return owners


def _state_io_inventory(state_io_source: str, capture_source: str, fractal_types_source: str, explaino_seed_source: str) -> dict[str, Any]:
    cast_sites = _float_cast_sites(state_io_source, fractal_types_source)
    owner_counts = Counter(site["owner_function"] for site in cast_sites)
    unresolved_sites = [site for site in cast_sites if site["destination_scalar_type"] == "unresolved"]
    serialized_double_owners = _serialized_double_state_owners(state_io_source, capture_source, fractal_types_source, explaino_seed_source)
    truthful_double_owners = [item for item in serialized_double_owners if item["classification"] == "TRUTHFUL_FLOAT64"]
    normalized_double_owners = [item for item in serialized_double_owners if item["classification"] == "INTENTIONAL_MIXED_PRECISION"]
    serializer = "unresolved"
    marker = "std::setprecision(std::numeric_limits<double>::max_digits10)"
    if marker in capture_source:
        serializer = "std::numeric_limits<double>::max_digits10"
    classification = "INTENTIONAL_MIXED_PRECISION" if not unresolved_sites else "NEEDS_RUNTIME_WITNESS"
    return {
        "serializer_precision": serializer,
        "float_cast_site_count": len(cast_sites),
        "float_conversion_owner_function_count": len(owner_counts),
        "float_conversion_owner_counts": dict(sorted(owner_counts.items())),
        "float_cast_sites": cast_sites,
        "unresolved_float_cast_site_count": len(unresolved_sites),
        "serialized_double_state_owner_count": len(serialized_double_owners),
        "truthful_double_state_owner_count": len(truthful_double_owners),
        "normalized_double_state_owner_count": len(normalized_double_owners),
        "double_state_owners": serialized_double_owners,
        "classification": classification,
        "classification_basis": (
            "every explicit float conversion resolves to float-backed destination storage; five serialized double owners bypass normalization and one is canonicalized by the mixed-precision ExplainO seed owner"
            if not unresolved_sites
            else "one or more explicit float conversions could not be joined mechanically to destination storage"
        ),
    }
def _enum_id_map(enum_source: str) -> dict[str, str]:
    pairs = re.findall(r'\{FractalType::([A-Za-z0-9_]+),\s*"([^"]+)"\}', enum_source)
    return {enum_name: public_id for enum_name, public_id in pairs}


def _selector_ids_from_condition(
    condition: str,
    enum_map: dict[str, str],
    helper_sources: tuple[str, ...],
) -> tuple[list[str], list[str]]:
    enum_names = set(re.findall(r"FractalType::([A-Za-z0-9_]+)", condition))
    helper_names: list[str] = []
    for helper_name in re.findall(r"\b([A-Z][A-Za-z0-9_]*)\([^)]*\)", condition):
        helper_body = None
        for source in helper_sources:
            signature = re.search(rf"\b{re.escape(helper_name)}\s*\([^{{;]*\)\s*\{{", source)
            if signature:
                helper_body = _brace_block(source, source.find("{", signature.start()))
                break
        if helper_body is None:
            continue
        helper_names.append(helper_name)
        enum_names.update(re.findall(r"FractalType::([A-Za-z0-9_]+)", helper_body))
    return sorted(enum_map.get(name, name) for name in enum_names), sorted(set(helper_names))


def _top_level_sampler_branches(
    sample_source: str,
    enum_map: dict[str, str],
    helper_sources: tuple[str, ...],
) -> list[dict[str, Any]]:
    pattern = re.compile(
        r"(?ms)^    (?:\}\s*)?(?:if|else if)\s*"
        r"\(([^{}]*?\b(?:ft|fractalType)\b[^{}]*?)\)\s*\{"
    )
    branches: list[dict[str, Any]] = []
    for match in pattern.finditer(sample_source):
        opening = sample_source.find("{", match.start())
        block = _brace_block(sample_source, opening)
        condition = " ".join(match.group(1).split())
        direct_enum_names = sorted(set(re.findall(r"FractalType::([A-Za-z0-9_]+)", condition)))
        selectors, helper_names = _selector_ids_from_condition(condition, enum_map, helper_sources)
        marker_present = "usedFloat64IterationArithmetic = true" in block
        if helper_names and not direct_enum_names:
            owner_id = f"predicate:{'+'.join(helper_names)}"
        elif len(selectors) == 1:
            owner_id = f"branch:{selectors[0]}"
        else:
            owner_id = f"branch:line-{sample_source.count(chr(10), 0, match.start()) + 1}"
        branches.append(
            {
                "owner_id": owner_id,
                "line": sample_source.count("\n", 0, match.start()) + 1,
                "condition": condition,
                "enum_names": direct_enum_names,
                "helper_names": helper_names,
                "selectors": selectors,
                "float64_iteration_execution_marker": marker_present,
                "classification_confidence": "static_execution_marker_only",
            }
        )
    return branches


def _fallback_dispatch_owner(sample_source: str, selectors: list[str], claimed: set[str]) -> dict[str, Any]:
    marker = "// Escape-time family."
    marker_index = sample_source.find(marker)
    if marker_index < 0:
        raise ValueError("missing final escape-time fallback marker")
    opening = sample_source.rfind("{", 0, marker_index)
    block = _brace_block(sample_source, opening)
    return {
        "owner_id": "fallback:escape_time_family",
        "line": sample_source.count("\n", 0, opening) + 1,
        "condition": "final else escape-time fallback after selector normalization",
        "enum_names": [],
        "helper_names": [],
        "selectors": sorted(set(selectors) - claimed),
        "float64_iteration_execution_marker": "usedFloat64IterationArithmetic = true" in block,
        "classification_confidence": "static_execution_marker_only",
    }


def _brace_block(source: str, opening: int) -> str:
    if opening < 0 or source[opening] != "{":
        raise ValueError("invalid sampler branch opening brace")
    depth = 0
    for index in range(opening, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[opening:index + 1]
    raise ValueError("unterminated sampler branch")


def _fractal_selector_ids(ui_schema: dict[str, Any]) -> list[str]:
    for panel in ui_schema.get("panels", []):
        for control in panel.get("controls", []):
            if control.get("id") == "fractal_type":
                return [str(option["id"]) for option in control.get("options", [])]
    raise ValueError("fractal_type selector is missing from UI schema")


def _runtime_inventory(
    ui_schema: dict[str, Any],
    tier_source: str,
    sample_source: str,
    enum_source: str,
    family_rules_source: str,
    specialized_formulas_source: str,
) -> dict[str, Any]:
    selectors = _fractal_selector_ids(ui_schema)
    branches = _top_level_sampler_branches(
        sample_source,
        _enum_id_map(enum_source),
        (family_rules_source, specialized_formulas_source),
    )
    claimed = {selector for branch in branches for selector in branch["selectors"]}
    dispatch_owners = [*branches, _fallback_dispatch_owner(sample_source, selectors, claimed)]
    witnessed_selectors = {
        selector
        for owner in dispatch_owners
        if owner["float64_iteration_execution_marker"]
        for selector in owner["selectors"]
    }
    support_policy = "unresolved"
    if "if (!SupportsBasinColoring(ft))" in tier_source and "flags |= kSupport_Standard" in tier_source:
        support_policy = "advertised_for_all_selectors"
    standard_resolves = "unresolved"
    if "return {NumericBackend::float64, IterationStrategy::direct};" in tier_source:
        standard_resolves = "float64_direct"
    return {
        "selector_count": len(selectors),
        "selectors": selectors,
        "standard_support_policy": support_policy,
        "standard_resolves_to": standard_resolves,
        "branch_records": branches,
        "dispatch_owner_count": len(dispatch_owners),
        "dispatch_owner_records": dispatch_owners,
        "dispatch_owners_needing_execution_witness": sorted(
            owner["owner_id"]
            for owner in dispatch_owners
            if not owner["float64_iteration_execution_marker"]
        ),
        "selectors_with_static_execution_owner": sorted(set(selectors) & witnessed_selectors),
        "selectors_without_static_execution_owner": sorted(set(selectors) - witnessed_selectors),
        "static_evidence_disclaimer": (
            "A canonical execution-evidence assignment is only static dispatch-owner evidence. It prevents "
            "selector counts from being mistaken for repair counts, but native execution witnesses remain "
            "authoritative for whether the iteration arithmetic actually ran at float64."
        ),
    }


def _authority(repo_root: Path) -> dict[str, Any]:
    paths = (
        UI_SCHEMA_PATH,
        PIPELINE_CONTRACT_PATH,
        SCHEMA_BINDING_PATH,
        STATE_IO_PATH,
        STATE_CAPTURE_PATH,
        FRACTAL_TYPES_PATH,
        EXPLAINO_SEED_PATH,
        COLOR_PIPELINE_CORE_PATH,
        COLOR_PIPELINE_WINDOW_PATH,
        TIER_RESOLVER_PATH,
        SAMPLE_DEVICE_PATH,
        ENUM_IDS_PATH,
        FAMILY_RULES_PATH,
        SPECIALIZED_FORMULAS_PATH,
    )
    records = {path.as_posix(): _sha256(repo_root, path) for path in paths}
    return {
        "source_commit": _source_commit(repo_root),
        "ui_schema": {"path": UI_SCHEMA_PATH.as_posix(), "sha256": records[UI_SCHEMA_PATH.as_posix()]},
        "color_pipeline_contract": {
            "path": PIPELINE_CONTRACT_PATH.as_posix(),
            "sha256": records[PIPELINE_CONTRACT_PATH.as_posix()],
        },
        "source_files": records,
    }


def build_inventory(repo_root: Path) -> dict[str, Any]:
    ui_schema = _read_json(repo_root, UI_SCHEMA_PATH)
    pipeline_contract = _read_json(repo_root, PIPELINE_CONTRACT_PATH)
    schema_binding = _read_text(repo_root, SCHEMA_BINDING_PATH)
    numeric_controls = _numeric_controls(ui_schema, schema_binding)
    classifications = Counter(item["classification"] for item in numeric_controls)
    return {
        "schema_version": SCHEMA_VERSION,
        "authority": _authority(repo_root),
        "general_schema": {
            "numeric_control_count": len(numeric_controls),
            "classification_counts": dict(sorted(classifications.items())),
            "numeric_controls": numeric_controls,
        },
        "color_pipeline": _pipeline_inventory(
            pipeline_contract,
            _read_text(repo_root, COLOR_PIPELINE_CORE_PATH),
            _read_text(repo_root, COLOR_PIPELINE_WINDOW_PATH),
            _read_text(repo_root, FRACTAL_TYPES_PATH),
        ),
        "state_io": _state_io_inventory(
            _read_text(repo_root, STATE_IO_PATH),
            _read_text(repo_root, STATE_CAPTURE_PATH),
            _read_text(repo_root, FRACTAL_TYPES_PATH),
            _read_text(repo_root, EXPLAINO_SEED_PATH),
        ),
        "runtime_tiers": _runtime_inventory(
            ui_schema,
            _read_text(repo_root, TIER_RESOLVER_PATH),
            _read_text(repo_root, SAMPLE_DEVICE_PATH),
            _read_text(repo_root, ENUM_IDS_PATH),
            _read_text(repo_root, FAMILY_RULES_PATH),
            _read_text(repo_root, SPECIALIZED_FORMULAS_PATH),
        ),
    }


def render_markdown(inventory: dict[str, Any]) -> str:
    general = inventory["general_schema"]
    pipeline = inventory["color_pipeline"]
    state_io = inventory["state_io"]
    runtime = inventory["runtime_tiers"]
    lines = [
        "# Precision Authority Inventory",
        "",
        f"Schema: `{inventory['schema_version']}`",
        f"Source commit: `{inventory['authority']['source_commit']}`",
        "",
        "## Summary",
        "",
        f"- General numeric controls: {general['numeric_control_count']}",
        f"- Authoring identity losses: {general['classification_counts'].get('AUTHORING_IDENTITY_LOSS', 0)}",
        f"- Color Pipeline contract parameters: {pipeline['contract_parameter_count']}",
        f"- State-load float cast sites: {state_io['float_cast_site_count']}",
        f"- Live selectors: {runtime['selector_count']}",
        f"- Runtime dispatch owners: {runtime['dispatch_owner_count']}",
        f"- Dispatch owners without static execution markers: {len(runtime['dispatch_owners_needing_execution_witness'])}",
        "",
        "## General Schema Authoring Risks",
        "",
        "| Control | Path | Storage | Input format | Classification |",
        "|---|---|---|---|---|",
    ]
    for item in general["numeric_controls"]:
        if item["classification"] != "AUTHORING_IDENTITY_LOSS":
            continue
        lines.append(
            f"| `{item['control_id']}` | `{item['binding_path']}` | {item['binding_storage']} | "
            f"`{item['input_format']}` | {item['classification']} |"
        )
    lines.extend(
        [
            "",
            "## Color Pipeline",
            "",
            f"- Authority: {pipeline['authority_kind']}",
            f"- Runtime numeric carrier: {pipeline['runtime_number_carrier']}",
            f"- Float editor format: `{pipeline['float_editor_format']}`",
            f"- Float readback: {pipeline['float_readback']}",
            f"- Runtime consumer owners: {pipeline['runtime_consumer_owner_count']}",
            f"- Classification: {pipeline['classification']}",
            "",
            "## State I/O",
            "",
            f"- Serializer precision: `{state_io['serializer_precision']}`",
            f"- Float conversion sites: {state_io['float_cast_site_count']}",
            f"- Conversion owner functions: {state_io['float_conversion_owner_function_count']}",
            f"- Unresolved destinations: {state_io['unresolved_float_cast_site_count']}",
            f"- Serialized double state owners: {state_io['serialized_double_state_owner_count']}",
            f"- Exact binary64 state owners: {state_io['truthful_double_state_owner_count']}",
            f"- Normalized mixed-precision double owners: {state_io['normalized_double_state_owner_count']}",
            f"- Classification: {state_io['classification']}",
            "",
            "## Runtime Tier Truth",
            "",
            f"- Standard support policy: `{runtime['standard_support_policy']}`",
            f"- Standard resolves to: `{runtime['standard_resolves_to']}`",
            "- Static evidence is not runtime proof.",
            "- Dispatch owners without canonical float64 execution markers:",
        ]
    )
    if runtime["dispatch_owners_needing_execution_witness"]:
        for owner_id in runtime["dispatch_owners_needing_execution_witness"]:
            lines.append(f"  - `{owner_id}`")
    else:
        lines.append("  - none")
    return "\n".join(lines) + "\n"


def write_outputs(inventory: dict[str, Any], *, out_json: Path, out_md: Path) -> None:
    out_json.parent.mkdir(parents=True, exist_ok=True)
    out_md.parent.mkdir(parents=True, exist_ok=True)
    out_json.write_text(json.dumps(inventory, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    out_md.write_text(render_markdown(inventory), encoding="utf-8")


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Build the deterministic viewer-host precision authority inventory")
    parser.add_argument("--repo-root", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--out-json", type=Path, required=True)
    parser.add_argument("--out-md", type=Path, required=True)
    args = parser.parse_args(argv)
    inventory = build_inventory(args.repo_root.resolve())
    write_outputs(inventory, out_json=args.out_json, out_md=args.out_md)
    print(
        "precision_authority_inventory: "
        f"controls={inventory['general_schema']['numeric_control_count']} "
        f"pipeline_params={inventory['color_pipeline']['contract_parameter_count']} "
        f"selectors={inventory['runtime_tiers']['selector_count']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
