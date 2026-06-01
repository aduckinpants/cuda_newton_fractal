#include "color_pipeline_metadata_contract.h"

#include "json_min.h"

#include <cmath>
#include <fstream>
#include <set>
#include <sstream>

namespace {

bool SetError(std::string* outError, const std::string& message) {
    if (outError) {
        *outError = message;
    }
    return false;
}

const json_min::Value* RequiredField(const json_min::Value& object, const char* key, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value) {
        SetError(outError, std::string("Missing required field '") + key + "'");
    }
    return value;
}

bool ReadString(const json_min::Value& object, const char* key, std::string* outValue, std::string* outError) {
    const json_min::Value* value = RequiredField(object, key, outError);
    if (!value) {
        return false;
    }
    if (!value->is_string()) {
        return SetError(outError, std::string("Field '") + key + "' must be a string");
    }
    *outValue = value->as_string();
    return true;
}

bool ReadOptionalString(const json_min::Value& object, const char* key, std::string* outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || value->is_null()) {
        outValue->clear();
        return true;
    }
    if (!value->is_string()) {
        return SetError(outError, std::string("Field '") + key + "' must be a string");
    }
    *outValue = value->as_string();
    return true;
}

bool ReadBool(const json_min::Value& object, const char* key, bool* outValue, std::string* outError) {
    const json_min::Value* value = RequiredField(object, key, outError);
    if (!value) {
        return false;
    }
    if (!value->is_bool()) {
        return SetError(outError, std::string("Field '") + key + "' must be a bool");
    }
    *outValue = value->as_bool();
    return true;
}

bool ReadOptionalBool(const json_min::Value& object, const char* key, bool* outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || value->is_null()) {
        *outValue = false;
        return true;
    }
    if (!value->is_bool()) {
        return SetError(outError, std::string("Field '") + key + "' must be a bool");
    }
    *outValue = value->as_bool();
    return true;
}

bool ReadNumber(const json_min::Value& object, const char* key, double* outValue, std::string* outError) {
    const json_min::Value* value = RequiredField(object, key, outError);
    if (!value) {
        return false;
    }
    if (!value->is_number() || !std::isfinite(value->as_number())) {
        return SetError(outError, std::string("Field '") + key + "' must be a finite number");
    }
    *outValue = value->as_number();
    return true;
}

bool ReadOptionalNumber(
    const json_min::Value& object,
    const char* key,
    bool* outHasValue,
    double* outValue,
    std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || value->is_null()) {
        *outHasValue = false;
        *outValue = 0.0;
        return true;
    }
    if (!value->is_number() || !std::isfinite(value->as_number())) {
        return SetError(outError, std::string("Field '") + key + "' must be a finite number or null");
    }
    *outHasValue = true;
    *outValue = value->as_number();
    return true;
}

bool ReadStringArray(
    const json_min::Value& object,
    const char* key,
    std::vector<std::string>* outValues,
    std::string* outError) {
    const json_min::Value* value = RequiredField(object, key, outError);
    if (!value) {
        return false;
    }
    if (!value->is_array()) {
        return SetError(outError, std::string("Field '") + key + "' must be an array");
    }
    outValues->clear();
    for (const json_min::Value& item : value->as_array()) {
        if (!item.is_string()) {
            return SetError(outError, std::string("Field '") + key + "' entries must be strings");
        }
        outValues->push_back(item.as_string());
    }
    return true;
}

bool StringInList(const std::string& value, const char* const* items, std::size_t count) {
    for (std::size_t index = 0; index < count; ++index) {
        if (value == items[index]) {
            return true;
        }
    }
    return false;
}

bool StartsWith(const std::string& value, const char* prefix) {
    const std::string prefixValue = prefix ? prefix : "";
    return value.rfind(prefixValue, 0) == 0;
}

bool IsGenericPortType(const std::string& type) {
    return StartsWith(type, "generic.") && type.size() > 8;
}

bool ReadPositiveInteger(const json_min::Value& object, const char* key, int* outValue, std::string* outError) {
    double numericValue = 0.0;
    if (!ReadNumber(object, key, &numericValue, outError)) {
        return false;
    }
    const double rounded = std::floor(numericValue);
    if (numericValue != rounded || numericValue < 1.0) {
        return SetError(outError, std::string("Field '") + key + "' must be a positive integer");
    }
    *outValue = static_cast<int>(numericValue);
    return true;
}

bool ReadNonNegativeInteger(const json_min::Value& object, const char* key, int* outValue, std::string* outError) {
    double numericValue = 0.0;
    if (!ReadNumber(object, key, &numericValue, outError)) {
        return false;
    }
    const double rounded = std::floor(numericValue);
    if (numericValue != rounded || numericValue < 0.0) {
        return SetError(outError, std::string("Field '") + key + "' must be a non-negative integer");
    }
    *outValue = static_cast<int>(numericValue);
    return true;
}

bool ReadDefaultValue(
    const json_min::Value& object,
    MaterializedColorPipelineParam* outParam,
    std::string* outError) {
    const json_min::Value* value = object.get("default");
    if (!value || value->is_null()) {
        outParam->has_default = false;
        outParam->default_kind.clear();
        return true;
    }
    outParam->has_default = true;
    if (value->is_number()) {
        if (!std::isfinite(value->as_number())) {
            return SetError(outError, "Param default must be finite");
        }
        outParam->default_kind = "number";
        outParam->number_default = value->as_number();
        return true;
    }
    if (value->is_bool()) {
        outParam->default_kind = "bool";
        outParam->bool_default = value->as_bool();
        return true;
    }
    if (value->is_string()) {
        outParam->default_kind = "string";
        outParam->string_default = value->as_string();
        return true;
    }
    return SetError(outError, "Param default must be number, bool, string, or null");
}

bool ReadSignalType(
    const json_min::Value& value,
    MaterializedSignalType* outType,
    std::string* outError) {
    if (!value.is_object()) {
        return SetError(outError, "Signal type entry must be an object");
    }
    MaterializedSignalType type;
    if (!ReadString(value, "id", &type.id, outError) ||
        !ReadString(value, "kind", &type.kind, outError) ||
        !ReadString(value, "domain", &type.domain, outError) ||
        !ReadString(value, "topology", &type.topology, outError) ||
        !ReadPositiveInteger(value, "arity", &type.arity, outError) ||
        !ReadString(value, "default_adapter_policy", &type.default_adapter_policy, outError) ||
        !ReadOptionalString(value, "units", &type.units, outError) ||
        !ReadOptionalNumber(value, "period", &type.has_period, &type.period, outError) ||
        !ReadOptionalString(value, "color_space", &type.color_space, outError) ||
        !ReadOptionalString(value, "coordinate_space", &type.coordinate_space, outError)) {
        return false;
    }
    *outType = std::move(type);
    return true;
}

bool ReadParam(
    const json_min::Value& value,
    MaterializedColorPipelineParam* outParam,
    std::string* outError) {
    if (!value.is_object()) {
        return SetError(outError, "Param entry must be an object");
    }
    MaterializedColorPipelineParam param;
    if (!ReadString(value, "path", &param.path, outError) ||
        !ReadString(value, "type", &param.type, outError) ||
        !ReadString(value, "label", &param.label, outError) ||
        !ReadOptionalNumber(value, "min", &param.has_min, &param.min_value, outError) ||
        !ReadOptionalNumber(value, "max", &param.has_max, &param.max_value, outError) ||
        !ReadOptionalNumber(value, "step", &param.has_step, &param.step_value, outError) ||
        !ReadDefaultValue(value, &param, outError)) {
        return false;
    }
    if (param.has_min && param.has_max && param.min_value > param.max_value) {
        return SetError(outError, "Param min cannot exceed max");
    }
    if (const json_min::Value* options = value.get("options")) {
        if (!options->is_array()) {
            return SetError(outError, "Param options must be an array");
        }
        for (const json_min::Value& option : options->as_array()) {
            if (!option.is_string()) {
                return SetError(outError, "Param option must be a string");
            }
            param.enum_options.push_back(option.as_string());
        }
    }
    *outParam = std::move(param);
    return true;
}

bool ReadPort(
    const json_min::Value& value,
    MaterializedColorPipelinePort* outPort,
    std::string* outError) {
    if (!value.is_object()) {
        return SetError(outError, "Port entry must be an object");
    }
    MaterializedColorPipelinePort port;
    if (!ReadString(value, "direction", &port.direction, outError) ||
        !ReadString(value, "id", &port.id, outError) ||
        !ReadString(value, "type", &port.type, outError) ||
        !ReadOptionalBool(value, "canonical", &port.canonical, outError) ||
        !ReadOptionalString(value, "generic_group", &port.generic_group, outError)) {
        return false;
    }
    *outPort = std::move(port);
    return true;
}

bool ReadAdapter(
    const json_min::Value& value,
    MaterializedColorPipelineAdapter* outAdapter,
    std::string* outError) {
    if (!value.is_object()) {
        return SetError(outError, "Adapter entry must be an object");
    }
    MaterializedColorPipelineAdapter adapter;
    if (!ReadString(value, "id", &adapter.id, outError) ||
        !ReadString(value, "source", &adapter.source, outError) ||
        !ReadString(value, "target", &adapter.target, outError) ||
        !ReadString(value, "policy", &adapter.policy, outError) ||
        !ReadBool(value, "lossy", &adapter.lossy, outError) ||
        !ReadBool(value, "reversible", &adapter.reversible, outError) ||
        !ReadNonNegativeInteger(value, "cost", &adapter.cost, outError) ||
        !ReadOptionalString(value, "fail_closed_reason", &adapter.fail_closed_reason, outError)) {
        return false;
    }
    *outAdapter = std::move(adapter);
    return true;
}

bool ReadEdgePolicy(
    const json_min::Value& value,
    MaterializedColorPipelineEdgePolicy* outPolicy,
    std::string* outError) {
    if (!value.is_object()) {
        return SetError(outError, "Edge policy entry must be an object");
    }
    MaterializedColorPipelineEdgePolicy policy;
    if (!ReadString(value, "id", &policy.id, outError) ||
        !ReadNonNegativeInteger(value, "max_adapter_hops", &policy.max_adapter_hops, outError) ||
        !ReadBool(value, "allow_lossy", &policy.allow_lossy, outError) ||
        !ReadBool(value, "allow_visible_default", &policy.allow_visible_default, outError) ||
        !ReadBool(value, "allow_explicit", &policy.allow_explicit, outError) ||
        !ReadBool(value, "allow_diagnostic", &policy.allow_diagnostic, outError) ||
        !ReadBool(value, "fail_closed_default", &policy.fail_closed_default, outError)) {
        return false;
    }
    *outPolicy = std::move(policy);
    return true;
}

bool ReadEdgeLink(
    const json_min::Value& value,
    MaterializedColorPipelineEdgeLink* outEdge,
    std::string* outError) {
    if (!value.is_object()) {
        return SetError(outError, "Edge link entry must be an object");
    }
    MaterializedColorPipelineEdgeLink edge;
    if (!ReadString(value, "id", &edge.id, outError) ||
        !ReadString(value, "from_lane", &edge.from_lane, outError) ||
        !ReadString(value, "to_lane", &edge.to_lane, outError) ||
        !ReadString(value, "from_port", &edge.from_port, outError) ||
        !ReadString(value, "to_port", &edge.to_port, outError) ||
        !ReadString(value, "fail_closed_reason", &edge.fail_closed_reason, outError)) {
        return false;
    }
    *outEdge = std::move(edge);
    return true;
}

bool ReadRouteEdge(
    const json_min::Value& value,
    MaterializedColorPipelineRouteEdge* outRouteEdge,
    std::string* outError) {
    if (!value.is_object()) {
        return SetError(outError, "Resolution route edge entry must be an object");
    }
    MaterializedColorPipelineRouteEdge routeEdge;
    if (!ReadString(value, "edge_id", &routeEdge.edge_id, outError) ||
        !ReadString(value, "from_function", &routeEdge.from_function, outError) ||
        !ReadString(value, "to_function", &routeEdge.to_function, outError) ||
        !ReadString(value, "from_type", &routeEdge.from_type, outError) ||
        !ReadString(value, "to_type", &routeEdge.to_type, outError) ||
        !ReadString(value, "output_type", &routeEdge.output_type, outError) ||
        !ReadString(value, "status", &routeEdge.status, outError) ||
        !ReadStringArray(value, "adapters", &routeEdge.adapters, outError) ||
        !ReadNonNegativeInteger(value, "adapter_hops", &routeEdge.adapter_hops, outError) ||
        !ReadNonNegativeInteger(value, "adapter_cost", &routeEdge.adapter_cost, outError)) {
        return false;
    }
    *outRouteEdge = std::move(routeEdge);
    return true;
}

bool ReadResolutionCase(
    const json_min::Value& value,
    MaterializedColorPipelineResolutionCase* outCase,
    std::string* outError) {
    if (!value.is_object()) {
        return SetError(outError, "Resolution case entry must be an object");
    }
    MaterializedColorPipelineResolutionCase resolutionCase;
    if (!ReadString(value, "id", &resolutionCase.id, outError) ||
        !ReadString(value, "source", &resolutionCase.source, outError) ||
        !ReadString(value, "shape", &resolutionCase.shape, outError) ||
        !ReadString(value, "palette", &resolutionCase.palette, outError) ||
        !ReadString(value, "grading", &resolutionCase.grading, outError) ||
        !ReadString(value, "expected_status", &resolutionCase.expected_status, outError) ||
        !ReadString(value, "status", &resolutionCase.status, outError) ||
        !ReadBool(value, "allow_lossy", &resolutionCase.allow_lossy, outError) ||
        !ReadBool(value, "allow_visible_default", &resolutionCase.allow_visible_default, outError) ||
        !ReadBool(value, "explicit_adapter_consent", &resolutionCase.explicit_adapter_consent, outError) ||
        !ReadBool(value, "diagnostic_adapter_consent", &resolutionCase.diagnostic_adapter_consent, outError) ||
        !ReadStringArray(value, "chosen_adapters", &resolutionCase.chosen_adapters, outError) ||
        !ReadNonNegativeInteger(value, "adapter_hops", &resolutionCase.adapter_hops, outError) ||
        !ReadNonNegativeInteger(value, "adapter_cost", &resolutionCase.adapter_cost, outError) ||
        !ReadString(value, "tie_break_rule", &resolutionCase.tie_break_rule, outError) ||
        !ReadStringArray(value, "policy_blockers", &resolutionCase.policy_blockers, outError) ||
        !ReadOptionalString(value, "fail_closed_reason", &resolutionCase.fail_closed_reason, outError)) {
        return false;
    }
    const json_min::Value* routeEdges = RequiredField(value, "route_edges", outError);
    if (!routeEdges || !routeEdges->is_array()) {
        return SetError(outError, "resolution_case.route_edges must be an array");
    }
    for (const json_min::Value& routeValue : routeEdges->as_array()) {
        MaterializedColorPipelineRouteEdge routeEdge;
        if (!ReadRouteEdge(routeValue, &routeEdge, outError)) {
            return false;
        }
        resolutionCase.route_edges.push_back(std::move(routeEdge));
    }
    *outCase = std::move(resolutionCase);
    return true;
}

bool ReadFunction(
    const json_min::Value& value,
    MaterializedColorPipelineFunction* outFunction,
    std::string* outError) {
    if (!value.is_object()) {
        return SetError(outError, "Function entry must be an object");
    }
    MaterializedColorPipelineFunction function;
    if (!ReadString(value, "id", &function.id, outError) ||
        !ReadString(value, "label", &function.label, outError) ||
        !ReadOptionalString(value, "description", &function.description, outError) ||
        !ReadString(value, "taxonomy_group", &function.taxonomy_group, outError) ||
        !ReadBool(value, "runtime_backed", &function.runtime_backed, outError) ||
        !ReadString(value, "input_kind", &function.input_kind, outError) ||
        !ReadString(value, "output_kind", &function.output_kind, outError)) {
        return false;
    }
    if (!ReadOptionalString(value, "signal_kind", &function.signal_kind, outError) ||
        !ReadOptionalString(value, "typed_signal", &function.typed_signal, outError)) {
        return false;
    }

    const json_min::Value* params = RequiredField(value, "params", outError);
    if (!params) {
        return false;
    }
    if (!params->is_array()) {
        return SetError(outError, "Function params must be an array");
    }
    for (const json_min::Value& paramValue : params->as_array()) {
        MaterializedColorPipelineParam param;
        if (!ReadParam(paramValue, &param, outError)) {
            return false;
        }
        function.params.push_back(std::move(param));
    }

    if (const json_min::Value* ports = value.get("ports")) {
        if (!ports->is_array()) {
            return SetError(outError, "Function ports must be an array");
        }
        for (const json_min::Value& portValue : ports->as_array()) {
            MaterializedColorPipelinePort port;
            if (!ReadPort(portValue, &port, outError)) {
                return false;
            }
            function.ports.push_back(std::move(port));
        }
    }

    *outFunction = std::move(function);
    return true;
}

bool ReadLane(
    const json_min::Value& value,
    MaterializedColorPipelineLane* outLane,
    std::string* outError) {
    if (!value.is_object()) {
        return SetError(outError, "Lane entry must be an object");
    }
    MaterializedColorPipelineLane lane;
    if (!ReadString(value, "id", &lane.id, outError) ||
        !ReadString(value, "label", &lane.label, outError) ||
        !ReadString(value, "default", &lane.default_function_id, outError)) {
        return false;
    }
    const json_min::Value* functions = RequiredField(value, "functions", outError);
    if (!functions) {
        return false;
    }
    if (!functions->is_array()) {
        return SetError(outError, "Lane functions must be an array");
    }
    for (const json_min::Value& functionValue : functions->as_array()) {
        MaterializedColorPipelineFunction function;
        if (!ReadFunction(functionValue, &function, outError)) {
            return false;
        }
        lane.functions.push_back(std::move(function));
    }
    *outLane = std::move(lane);
    return true;
}

bool ReadCompatibility(
    const json_min::Value& value,
    MaterializedColorPipelineCompatibility* outCompatibility,
    std::string* outError) {
    if (!value.is_object()) {
        return SetError(outError, "Compatibility entry must be an object");
    }
    MaterializedColorPipelineCompatibility compatibility;
    if (!ReadString(value, "source", &compatibility.source, outError) ||
        !ReadString(value, "palette", &compatibility.palette, outError) ||
        !ReadString(value, "signal", &compatibility.signal, outError) ||
        !ReadString(value, "palette_runtime", &compatibility.palette_runtime, outError) ||
        !ReadString(value, "grading", &compatibility.grading, outError) ||
        !ReadString(value, "mode", &compatibility.mode, outError) ||
        !ReadOptionalString(value, "reason", &compatibility.reason, outError)) {
        return false;
    }
    *outCompatibility = std::move(compatibility);
    return true;
}

bool ReadCompatOverride(
    const json_min::Value& value,
    MaterializedColorPipelineCompatOverride* outOverride,
    std::string* outError) {
    if (!value.is_object()) {
        return SetError(outError, "Compat override entry must be an object");
    }
    MaterializedColorPipelineCompatOverride compatOverride;
    if (!ReadString(value, "id", &compatOverride.id, outError) ||
        !ReadString(value, "source", &compatOverride.source, outError) ||
        !ReadString(value, "palette", &compatOverride.palette, outError) ||
        !ReadString(value, "grading", &compatOverride.grading, outError) ||
        !ReadString(value, "classification", &compatOverride.classification, outError) ||
        !ReadString(value, "owner_seam", &compatOverride.owner_seam, outError) ||
        !ReadString(value, "reason", &compatOverride.reason, outError) ||
        !ReadString(value, "proof", &compatOverride.proof, outError)) {
        return false;
    }
    *outOverride = std::move(compatOverride);
    return true;
}

bool ReadCompatibilityAudit(
    const json_min::Value& value,
    MaterializedColorPipelineCompatibilityAudit* outAudit,
    std::string* outError) {
    if (!value.is_object()) {
        return SetError(outError, "Compatibility audit entry must be an object");
    }
    MaterializedColorPipelineCompatibilityAudit audit;
    if (!ReadString(value, "source", &audit.source, outError) ||
        !ReadString(value, "palette", &audit.palette, outError) ||
        !ReadString(value, "grading", &audit.grading, outError) ||
        !ReadString(value, "mode", &audit.mode, outError) ||
        !ReadString(value, "classification", &audit.classification, outError) ||
        !ReadString(value, "route_case_id", &audit.route_case_id, outError) ||
        !ReadString(value, "override_id", &audit.override_id, outError) ||
        !ReadString(value, "reason", &audit.reason, outError)) {
        return false;
    }
    *outAudit = std::move(audit);
    return true;
}

bool ReadRecipe(
    const json_min::Value& value,
    MaterializedColorPipelineRecipe* outRecipe,
    std::string* outError) {
    if (!value.is_object()) {
        return SetError(outError, "Recipe entry must be an object");
    }
    MaterializedColorPipelineRecipe recipe;
    if (!ReadString(value, "id", &recipe.id, outError) ||
        !ReadString(value, "label", &recipe.label, outError) ||
        !ReadString(value, "source", &recipe.source, outError) ||
        !ReadString(value, "shape", &recipe.shape, outError) ||
        !ReadString(value, "palette", &recipe.palette, outError) ||
        !ReadString(value, "grading", &recipe.grading, outError) ||
        !ReadOptionalString(value, "fail_closed_reason", &recipe.fail_closed_reason, outError)) {
        return false;
    }
    *outRecipe = std::move(recipe);
    return true;
}

bool ReadRowApplicator(
    const json_min::Value& value,
    MaterializedColorPipelineRowApplicator* outApplicator,
    std::string* outError) {
    if (!value.is_object()) {
        return SetError(outError, "Row applicator entry must be an object");
    }
    MaterializedColorPipelineRowApplicator applicator;
    if (!ReadString(value, "id", &applicator.id, outError) ||
        !ReadString(value, "label", &applicator.label, outError) ||
        !ReadString(value, "target_lane", &applicator.target_lane, outError) ||
        !ReadString(value, "required_signal_kind", &applicator.required_signal_kind, outError) ||
        !ReadBool(value, "requires_sdf_field", &applicator.requires_sdf_field, outError) ||
        !ReadString(value, "storage_param", &applicator.storage_param, outError) ||
        !ReadOptionalString(value, "width_param", &applicator.width_param, outError) ||
        !ReadString(value, "fail_closed_reason", &applicator.fail_closed_reason, outError)) {
        return false;
    }
    *outApplicator = std::move(applicator);
    return true;
}

bool ReadExplainoEntry(
    const json_min::Value& value,
    MaterializedExplainoContractEntry* outEntry,
    std::string* outError) {
    if (!value.is_object()) {
        return SetError(outError, "Explaino contract entry must be an object");
    }
    MaterializedExplainoContractEntry entry;
    if (!ReadString(value, "id", &entry.id, outError) ||
        !ReadString(value, "hypothesis_space", &entry.hypothesis_space, outError) ||
        !ReadString(value, "authority", &entry.authority, outError) ||
        !ReadString(value, "lens", &entry.lens, outError) ||
        !ReadString(value, "invariant", &entry.invariant, outError) ||
        !ReadString(value, "proof", &entry.proof, outError) ||
        !ReadString(value, "fallback", &entry.fallback, outError) ||
        !ReadBool(value, "product_facing", &entry.product_facing, outError) ||
        !ReadBool(value, "diagnostic", &entry.diagnostic, outError)) {
        return false;
    }
    if (entry.hypothesis_space.empty() || entry.authority.empty() || entry.lens.empty() ||
        entry.invariant.empty() || entry.proof.empty() || entry.fallback.empty()) {
        return SetError(outError, std::string("Explaino contract entry '") + entry.id + "' has empty proof metadata");
    }
    *outEntry = std::move(entry);
    return true;
}

bool ValidateMaterializedSignalTypes(
    const std::vector<MaterializedSignalType>& types,
    std::set<std::string>* outTypeIds,
    std::string* outError) {
    static const char* const kValidKinds[] = {"scalar", "phase", "category", "palette", "mask", "color", "field"};
    static const char* const kValidTopologies[] = {"linear", "circular", "discrete", "mask", "color", "field"};
    static const char* const kValidPolicies[] = {"safe", "visible_default", "explicit_only", "diagnostic_only", "forbidden"};

    for (const MaterializedSignalType& type : types) {
        if (type.id.empty()) {
            return SetError(outError, "Materialized signal type contains an empty id");
        }
        if (!outTypeIds->insert(type.id).second) {
            return SetError(outError, std::string("Duplicate materialized signal type id '") + type.id + "'");
        }
        if (!StringInList(type.kind, kValidKinds, sizeof(kValidKinds) / sizeof(kValidKinds[0]))) {
            return SetError(outError, std::string("Materialized signal type '") + type.id + "' has invalid kind");
        }
        if (!StringInList(type.topology, kValidTopologies, sizeof(kValidTopologies) / sizeof(kValidTopologies[0]))) {
            return SetError(outError, std::string("Materialized signal type '") + type.id + "' has invalid topology");
        }
        if (!StringInList(type.default_adapter_policy, kValidPolicies, sizeof(kValidPolicies) / sizeof(kValidPolicies[0]))) {
            return SetError(outError, std::string("Materialized signal type '") + type.id + "' has invalid default_adapter_policy");
        }
        if (type.kind == "category" && type.domain == "discrete_index") {
            return SetError(outError, "palette discrete_index must use kind palette");
        }
        if (type.kind == "palette" && type.domain != "discrete_index") {
            return SetError(outError, std::string("Materialized signal type '") + type.id + "' palette kind requires discrete_index domain");
        }
        if (type.kind == "phase" && type.topology != "circular") {
            return SetError(outError, std::string("Materialized signal type '") + type.id + "' phase kind requires circular topology");
        }
        if (type.kind == "phase" && (!type.has_period || type.period <= 0.0)) {
            return SetError(outError, std::string("Materialized signal type '") + type.id + "' phase kind requires positive period");
        }
    }
    return !types.empty() || SetError(outError, "Materialized contract must contain at least one signal type");
}

bool ValidateMaterializedPorts(
    const MaterializedColorPipelineFunction& function,
    const std::string& laneId,
    const std::set<std::string>& signalTypeIds,
    std::string* outError) {
    if (function.ports.empty()) {
        return true;
    }
    std::set<std::string> portKeys;
    int canonicalOutputCount = 0;
    int genericInputCount = 0;
    int genericOutputCount = 0;
    std::string genericInputType;
    std::string genericOutputType;
    for (const MaterializedColorPipelinePort& port : function.ports) {
        if (port.direction != "input" && port.direction != "output") {
            return SetError(outError, std::string("Materialized function '") + function.id + "' has invalid port direction");
        }
        if (port.id.empty()) {
            return SetError(outError, std::string("Materialized function '") + function.id + "' contains an empty port id");
        }
        const std::string key = port.direction + "\n" + port.id;
        if (!portKeys.insert(key).second) {
            return SetError(outError, std::string("Duplicate materialized port '") + port.id + "' for function '" + function.id + "'");
        }
        if (port.type == "any") {
            return SetError(outError, "Materialized port type 'any' is forbidden");
        }
        const bool isGeneric = IsGenericPortType(port.type);
        if (isGeneric) {
            if (port.generic_group.empty()) {
                return SetError(outError, std::string("Materialized function '") + function.id + "' generic port requires generic_group");
            }
            if (port.generic_group == "any") {
                return SetError(outError, "Materialized generic_group 'any' is forbidden");
            }
            if (port.type != std::string("generic.") + port.generic_group) {
                return SetError(outError, std::string("Materialized function '") + function.id + "' generic_group must match port type");
            }
        } else {
            if (signalTypeIds.find(port.type) == signalTypeIds.end()) {
                return SetError(outError, std::string("Materialized function '") + function.id + "' port references unknown signal type '" + port.type + "'");
            }
            if (!port.generic_group.empty()) {
                return SetError(outError, std::string("Materialized function '") + function.id + "' non-generic port cannot declare generic_group");
            }
        }
        if (port.canonical) {
            if (port.direction != "output") {
                return SetError(outError, std::string("Materialized function '") + function.id + "' canonical port must be an output");
            }
            ++canonicalOutputCount;
        }
        if (laneId == "source" && port.direction == "input") {
            return SetError(outError, std::string("Materialized source function '") + function.id + "' cannot declare input ports");
        }
        if (laneId == "shape" && function.id != "identity" && !StartsWith(port.type, "scalar.")) {
            return SetError(outError, std::string("Materialized shape function '") + function.id + "' only supports scalar ports in Slice B");
        }
        if (function.id == "identity" && isGeneric) {
            if (port.direction == "input") {
                ++genericInputCount;
                genericInputType = port.type;
            } else if (port.direction == "output") {
                ++genericOutputCount;
                genericOutputType = port.type;
            }
        }
    }
    if (canonicalOutputCount != 1) {
        return SetError(outError, std::string("Materialized function '") + function.id + "' port signatures require exactly one canonical output");
    }
    if (function.id == "identity" &&
        (function.ports.size() != 2 || genericInputCount != 1 || genericOutputCount != 1 || genericInputType != genericOutputType)) {
        return SetError(outError, "Materialized identity ports must declare exactly one matching generic input and output");
    }
    return true;
}

bool IsScalarType(const std::string& typeId) {
    return StartsWith(typeId, "scalar.");
}

bool IsCategoryType(const std::string& typeId) {
    return StartsWith(typeId, "category.");
}

bool ValidateMaterializedAdapters(
    const std::vector<MaterializedColorPipelineAdapter>& adapters,
    const std::set<std::string>& signalTypeIds,
    std::string* outError) {
    static const char* const kValidPolicies[] = {
        "safe", "visible_default", "explicit_only", "diagnostic_only", "forbidden"};
    std::set<std::string> adapterIds;
    for (const MaterializedColorPipelineAdapter& adapter : adapters) {
        if (adapter.id.empty()) {
            return SetError(outError, "Materialized adapter id must be non-empty");
        }
        if (!adapterIds.insert(adapter.id).second) {
            return SetError(outError, std::string("Duplicate materialized adapter id '") + adapter.id + "'");
        }
        if (signalTypeIds.find(adapter.source) == signalTypeIds.end()) {
            return SetError(outError, std::string("Materialized adapter '") + adapter.id +
                "' references unknown source type");
        }
        if (signalTypeIds.find(adapter.target) == signalTypeIds.end()) {
            return SetError(outError, std::string("Materialized adapter '") + adapter.id +
                "' references unknown target type");
        }
        if (!StringInList(adapter.policy, kValidPolicies, sizeof(kValidPolicies) / sizeof(kValidPolicies[0]))) {
            return SetError(outError, std::string("Materialized adapter '") + adapter.id + "' has invalid policy");
        }
        if (adapter.lossy && adapter.policy == "safe") {
            return SetError(outError, std::string("Materialized adapter '") + adapter.id +
                "' lossy adapters cannot use safe policy");
        }
        if (adapter.policy != "safe" && adapter.fail_closed_reason.empty()) {
            return SetError(outError, std::string("Materialized adapter '") + adapter.id +
                "' requires fail_closed_reason for non-safe policy");
        }
        if (IsCategoryType(adapter.source) && IsScalarType(adapter.target) &&
            (adapter.policy == "safe" || adapter.policy == "visible_default")) {
            return SetError(outError, std::string("Materialized adapter '") + adapter.id +
                "' category-to-scalar adapters cannot be safe or visible_default");
        }
        if (adapter.source == "color.linear_rgb" && IsScalarType(adapter.target) &&
            (adapter.policy == "safe" || adapter.policy == "visible_default")) {
            return SetError(outError, std::string("Materialized adapter '") + adapter.id +
                "' color-to-scalar adapters cannot be safe or visible_default");
        }
        if ((adapter.source == "scalar.signed" || adapter.source == "scalar.sdf_signed_distance") &&
            adapter.target == "scalar.unit" &&
            (adapter.policy == "safe" || adapter.policy == "visible_default")) {
            return SetError(outError, std::string("Materialized adapter '") + adapter.id +
                "' signed-to-unit adapters require explicit normalization policy");
        }
    }
    return true;
}

bool ValidateMaterializedEdgeResolution(
    const MaterializedColorPipelineContract& contract,
    const std::set<std::string>& functionIds,
    const std::set<std::string>& signalTypeIds,
    std::string* outError) {
    const bool hasPolicy = !contract.edge_policy.id.empty();
    const bool hasEdges = !contract.edge_links.empty();
    const bool hasCases = !contract.resolution_cases.empty();
    if (!hasPolicy && !hasEdges && !hasCases) {
        return true;
    }
    if (!hasPolicy || !hasEdges || !hasCases) {
        return SetError(outError, "Materialized edge resolution requires policy, edges, and audit cases together");
    }

    std::set<std::string> laneIds;
    for (const MaterializedColorPipelineLane& lane : contract.lanes) {
        laneIds.insert(lane.id);
    }
    std::set<std::string> edgeIds;
    for (const MaterializedColorPipelineEdgeLink& edge : contract.edge_links) {
        if (edge.id.empty()) {
            return SetError(outError, "Materialized edge link id must be non-empty");
        }
        if (!edgeIds.insert(edge.id).second) {
            return SetError(outError, std::string("Duplicate materialized edge link id '") + edge.id + "'");
        }
        if (laneIds.find(edge.from_lane) == laneIds.end() || laneIds.find(edge.to_lane) == laneIds.end()) {
            return SetError(outError, std::string("Materialized edge link '") + edge.id + "' references unknown lane");
        }
        if (edge.from_port.empty() || edge.to_port.empty()) {
            return SetError(outError, std::string("Materialized edge link '") + edge.id + "' has an empty port id");
        }
        if (edge.fail_closed_reason.empty()) {
            return SetError(outError, std::string("Materialized edge link '") + edge.id + "' is missing fail_closed_reason");
        }
    }

    std::set<std::string> adapterIds;
    for (const MaterializedColorPipelineAdapter& adapter : contract.adapters) {
        adapterIds.insert(adapter.id);
    }
    static const char* const kValidCaseStatuses[] = {"resolved", "fail_closed"};
    static const char* const kValidRouteStatuses[] = {"direct", "adapted"};
    std::set<std::string> caseIds;
    for (const MaterializedColorPipelineResolutionCase& resolutionCase : contract.resolution_cases) {
        if (!caseIds.insert(resolutionCase.id).second) {
            return SetError(outError, std::string("Duplicate materialized resolution case id '") + resolutionCase.id + "'");
        }
        if (!StringInList(resolutionCase.expected_status, kValidCaseStatuses, 2) ||
            !StringInList(resolutionCase.status, kValidCaseStatuses, 2)) {
            return SetError(outError, std::string("Materialized resolution case '") + resolutionCase.id + "' has invalid status");
        }
        if (resolutionCase.expected_status != resolutionCase.status) {
            return SetError(outError, std::string("Materialized resolution case '") + resolutionCase.id + "' status does not match expected_status");
        }
        if (functionIds.find(resolutionCase.source) == functionIds.end() ||
            functionIds.find(resolutionCase.shape) == functionIds.end() ||
            functionIds.find(resolutionCase.palette) == functionIds.end() ||
            functionIds.find(resolutionCase.grading) == functionIds.end()) {
            return SetError(outError, std::string("Materialized resolution case '") + resolutionCase.id + "' references a missing function");
        }
        if (resolutionCase.status == "resolved" && resolutionCase.route_edges.empty()) {
            return SetError(outError, std::string("Materialized resolved case '") + resolutionCase.id + "' requires route_edges");
        }
        if (resolutionCase.status == "resolved" && !resolutionCase.fail_closed_reason.empty()) {
            return SetError(outError, std::string("Materialized resolved case '") + resolutionCase.id + "' must not include fail_closed_reason");
        }
        if (resolutionCase.status == "fail_closed" && resolutionCase.fail_closed_reason.empty()) {
            return SetError(outError, std::string("Materialized fail_closed case '") + resolutionCase.id + "' requires fail_closed_reason");
        }
        if (resolutionCase.tie_break_rule.empty()) {
            return SetError(outError, std::string("Materialized resolution case '") + resolutionCase.id + "' requires tie_break_rule");
        }
        if (resolutionCase.status == "resolved" && !resolutionCase.policy_blockers.empty()) {
            return SetError(outError, std::string("Materialized resolved case '") + resolutionCase.id + "' must not include policy_blockers");
        }
        if (resolutionCase.status == "fail_closed" && resolutionCase.policy_blockers.empty()) {
            return SetError(outError, std::string("Materialized fail_closed case '") + resolutionCase.id + "' requires policy_blockers");
        }
        for (const std::string& adapterId : resolutionCase.chosen_adapters) {
            if (adapterIds.find(adapterId) == adapterIds.end()) {
                return SetError(outError, std::string("Materialized resolution case '") + resolutionCase.id + "' references unknown adapter");
            }
        }
        int routeAdapterHops = 0;
        int routeAdapterCost = 0;
        for (const MaterializedColorPipelineRouteEdge& routeEdge : resolutionCase.route_edges) {
            if (edgeIds.find(routeEdge.edge_id) == edgeIds.end()) {
                return SetError(outError, std::string("Materialized resolution case '") + resolutionCase.id + "' route references unknown edge");
            }
            if (functionIds.find(routeEdge.from_function) == functionIds.end() ||
                functionIds.find(routeEdge.to_function) == functionIds.end()) {
                return SetError(outError, std::string("Materialized resolution case '") + resolutionCase.id + "' route references missing function");
            }
            if (signalTypeIds.find(routeEdge.from_type) == signalTypeIds.end() ||
                signalTypeIds.find(routeEdge.to_type) == signalTypeIds.end() ||
                signalTypeIds.find(routeEdge.output_type) == signalTypeIds.end()) {
                return SetError(outError, std::string("Materialized resolution case '") + resolutionCase.id + "' route references unknown signal type");
            }
            if (!StringInList(routeEdge.status, kValidRouteStatuses, 2)) {
                return SetError(outError, std::string("Materialized resolution case '") + resolutionCase.id + "' route has invalid status");
            }
            if (routeEdge.adapter_hops != static_cast<int>(routeEdge.adapters.size())) {
                return SetError(outError, std::string("Materialized resolution case '") + resolutionCase.id + "' route adapter_hops does not match adapters");
            }
            for (const std::string& adapterId : routeEdge.adapters) {
                if (adapterIds.find(adapterId) == adapterIds.end()) {
                    return SetError(outError, std::string("Materialized resolution case '") + resolutionCase.id + "' route references unknown adapter");
                }
            }
            routeAdapterHops += routeEdge.adapter_hops;
            routeAdapterCost += routeEdge.adapter_cost;
        }
        if (resolutionCase.adapter_hops != routeAdapterHops || resolutionCase.adapter_cost != routeAdapterCost) {
            return SetError(outError, std::string("Materialized resolution case '") + resolutionCase.id + "' route totals do not match route_edges");
        }
    }
    return true;
}

bool ValidateMaterializedParams(
    const MaterializedColorPipelineFunction& function,
    std::string* outError) {
    std::set<std::string> paramPaths;
    for (const MaterializedColorPipelineParam& param : function.params) {
        if (param.path.empty()) {
            return SetError(outError, std::string("Materialized function '") + function.id + "' contains an empty parameter path");
        }
        if (!paramPaths.insert(param.path).second) {
            return SetError(outError, std::string("Duplicate materialized parameter path '") + param.path + "' for function '" + function.id + "'");
        }
        if (param.has_min && param.has_max && param.min_value > param.max_value) {
            return SetError(outError, std::string("Invalid materialized parameter range for '") + param.path + "'");
        }
    }
    return true;
}

bool ValidateMaterializedLanes(
    const std::vector<MaterializedColorPipelineLane>& lanes,
    std::set<std::string>* outFunctionIds,
    const std::set<std::string>& signalTypeIds,
    std::string* outError) {
    std::set<std::string> laneIds;
    for (const MaterializedColorPipelineLane& lane : lanes) {
        if (lane.id.empty()) {
            return SetError(outError, "Materialized contract contains an empty lane id");
        }
        if (!laneIds.insert(lane.id).second) {
            return SetError(outError, std::string("Duplicate materialized lane id '") + lane.id + "'");
        }
        bool defaultFound = false;
        std::set<std::string> laneFunctionIds;
        for (const MaterializedColorPipelineFunction& function : lane.functions) {
            if (function.id.empty()) {
                return SetError(outError, std::string("Materialized lane '") + lane.id + "' contains an empty function id");
            }
            if (function.taxonomy_group.empty()) {
                return SetError(outError, std::string("Materialized function '") + function.id + "' contains an empty taxonomy_group");
            }
            if (!laneFunctionIds.insert(function.id).second || !outFunctionIds->insert(function.id).second) {
                return SetError(outError, std::string("Duplicate materialized function id '") + function.id + "'");
            }
            if (!function.typed_signal.empty() && signalTypeIds.find(function.typed_signal) == signalTypeIds.end()) {
                return SetError(outError, std::string("Materialized function '") + function.id + "' typed_signal references unknown signal type '" + function.typed_signal + "'");
            }
            defaultFound = defaultFound || function.id == lane.default_function_id;
            if (!ValidateMaterializedParams(function, outError) ||
                !ValidateMaterializedPorts(function, lane.id, signalTypeIds, outError)) {
                return false;
            }
        }
        if (!defaultFound) {
            return SetError(outError, std::string("Materialized lane '") + lane.id + "' default references missing function '" + lane.default_function_id + "'");
        }
    }
    return !laneIds.empty() || SetError(outError, "Materialized contract must contain at least one lane");
}

bool RequireKnownFunction(
    const std::set<std::string>& functionIds,
    const std::string& functionId,
    const char* role,
    std::string* outError) {
    if (functionIds.find(functionId) != functionIds.end()) {
        return true;
    }
    return SetError(outError, std::string("Compatibility references missing ") + role + " function '" + functionId + "'");
}

std::string CompatibilityKey(const std::string& source, const std::string& palette, const std::string& grading) {
    return source + "\n" + palette + "\n" + grading;
}

bool ValidateMaterializedCompatibility(
    const std::vector<MaterializedColorPipelineCompatibility>& rows,
    const std::set<std::string>& functionIds,
    std::string* outError) {
    std::set<std::string> compatibilityPairs;
    for (const MaterializedColorPipelineCompatibility& compatibility : rows) {
        if (!RequireKnownFunction(functionIds, compatibility.source, "source", outError) ||
            !RequireKnownFunction(functionIds, compatibility.palette, "palette", outError) ||
            !RequireKnownFunction(functionIds, compatibility.signal, "signal", outError) ||
            !RequireKnownFunction(functionIds, compatibility.palette_runtime, "runtime palette", outError) ||
            !RequireKnownFunction(functionIds, compatibility.grading, "grading", outError)) {
            return false;
        }
        const std::string pairKey = compatibility.source + "\n" + compatibility.palette;
        if (!compatibilityPairs.insert(pairKey).second) {
            return SetError(outError, std::string("Duplicate compatibility pair '") + compatibility.source + "' + '" + compatibility.palette + "'");
        }
    }
    return true;
}

bool ValidateMaterializedCompatibilityAudit(
    const MaterializedColorPipelineContract& contract,
    const std::set<std::string>& functionIds,
    std::string* outError) {
    if (contract.compat_overrides.empty() && contract.compatibility_audit.empty()) {
        return true;
    }
    if (contract.compatibility_audit.size() != contract.compatibility.size()) {
        return SetError(outError, "Compatibility audit must classify every compatibility row");
    }

    std::set<std::string> compatibilityKeys;
    for (const MaterializedColorPipelineCompatibility& compatibility : contract.compatibility) {
        compatibilityKeys.insert(CompatibilityKey(compatibility.source, compatibility.palette, compatibility.grading));
    }
    std::set<std::string> routeCaseIds;
    for (const MaterializedColorPipelineResolutionCase& resolutionCase : contract.resolution_cases) {
        routeCaseIds.insert(resolutionCase.id);
    }

    std::set<std::string> overrideIds;
    std::set<std::string> overrideKeys;
    for (const MaterializedColorPipelineCompatOverride& compatOverride : contract.compat_overrides) {
        if (compatOverride.id.empty()) {
            return SetError(outError, "Materialized compat override id must be non-empty");
        }
        if (!overrideIds.insert(compatOverride.id).second) {
            return SetError(outError, std::string("Duplicate materialized compat override id '") + compatOverride.id + "'");
        }
        const std::string key = CompatibilityKey(compatOverride.source, compatOverride.palette, compatOverride.grading);
        if (!overrideKeys.insert(key).second) {
            return SetError(outError, std::string("Duplicate materialized compat override route '") +
                compatOverride.source + "' + '" + compatOverride.palette + "' + '" + compatOverride.grading + "'");
        }
        if (compatibilityKeys.find(key) == compatibilityKeys.end()) {
            return SetError(outError, std::string("Materialized compat override '") + compatOverride.id +
                "' does not map to a compatibility row");
        }
        if (!RequireKnownFunction(functionIds, compatOverride.source, "source", outError) ||
            !RequireKnownFunction(functionIds, compatOverride.palette, "palette", outError) ||
            !RequireKnownFunction(functionIds, compatOverride.grading, "grading", outError)) {
            return false;
        }
        if (compatOverride.classification != "runtime_legacy_override") {
            return SetError(outError, std::string("Materialized compat override '") + compatOverride.id +
                "' has invalid classification");
        }
        if (compatOverride.owner_seam.empty() || compatOverride.reason.empty() || compatOverride.proof.empty()) {
            return SetError(outError, std::string("Materialized compat override '") + compatOverride.id +
                "' requires owner_seam, reason, and proof");
        }
    }

    std::set<std::string> auditKeys;
    std::set<std::string> usedOverrideIds;
    for (const MaterializedColorPipelineCompatibilityAudit& audit : contract.compatibility_audit) {
        const std::string key = CompatibilityKey(audit.source, audit.palette, audit.grading);
        if (!auditKeys.insert(key).second) {
            return SetError(outError, std::string("Duplicate materialized compatibility audit row '") +
                audit.source + "' + '" + audit.palette + "' + '" + audit.grading + "'");
        }
        if (compatibilityKeys.find(key) == compatibilityKeys.end()) {
            return SetError(outError, std::string("Materialized compatibility audit row '") +
                audit.source + "' + '" + audit.palette + "' + '" + audit.grading +
                "' does not map to a compatibility row");
        }
        if (audit.reason.empty()) {
            return SetError(outError, "Materialized compatibility audit row requires a reason");
        }
        if (audit.classification == "typed_resolved") {
            if (audit.route_case_id.empty() || !audit.override_id.empty()) {
                return SetError(outError, "Materialized typed_resolved compatibility audit row has invalid route/override fields");
            }
            if (routeCaseIds.find(audit.route_case_id) == routeCaseIds.end()) {
                return SetError(outError, std::string("Materialized compatibility audit references unknown route case '") +
                    audit.route_case_id + "'");
            }
        } else if (audit.classification == "runtime_legacy_override") {
            if (!audit.route_case_id.empty() || audit.override_id.empty()) {
                return SetError(outError, "Materialized runtime_legacy_override compatibility audit row has invalid route/override fields");
            }
            if (overrideIds.find(audit.override_id) == overrideIds.end()) {
                return SetError(outError, std::string("Materialized compatibility audit references unknown compat override '") +
                    audit.override_id + "'");
            }
            usedOverrideIds.insert(audit.override_id);
        } else {
            return SetError(outError, std::string("Materialized compatibility audit row has invalid classification '") +
                audit.classification + "'");
        }
    }
    for (const MaterializedColorPipelineCompatOverride& compatOverride : contract.compat_overrides) {
        if (usedOverrideIds.find(compatOverride.id) == usedOverrideIds.end()) {
            return SetError(outError, std::string("Materialized compat override '") + compatOverride.id +
                "' is not referenced by compatibility audit");
        }
    }
    return true;
}

bool ValidateMaterializedRecipes(
    const std::vector<MaterializedColorPipelineRecipe>& recipes,
    const std::set<std::string>& functionIds,
    std::string* outError) {
    std::set<std::string> recipeIds;
    for (const MaterializedColorPipelineRecipe& recipe : recipes) {
        if (!recipeIds.insert(recipe.id).second) {
            return SetError(outError, std::string("Duplicate materialized recipe id '") + recipe.id + "'");
        }
        if (functionIds.find(recipe.source) == functionIds.end() ||
            functionIds.find(recipe.shape) == functionIds.end() ||
            functionIds.find(recipe.palette) == functionIds.end() ||
            functionIds.find(recipe.grading) == functionIds.end()) {
            return SetError(outError, std::string("Materialized recipe '") + recipe.id + "' references a missing function");
        }
    }
    return true;
}

bool ValidateMaterializedRowApplicators(
    const std::vector<MaterializedColorPipelineRowApplicator>& applicators,
    std::string* outError) {
    std::set<std::string> applicatorIds;
    for (const MaterializedColorPipelineRowApplicator& applicator : applicators) {
        if (!applicatorIds.insert(applicator.id).second) {
            return SetError(outError, std::string("Duplicate materialized row applicator id '") + applicator.id + "'");
        }
        if (applicator.target_lane != "source") {
            return SetError(outError, std::string("Materialized row applicator '") + applicator.id + "' has invalid target_lane");
        }
        if (applicator.required_signal_kind != "any" &&
            applicator.required_signal_kind != "scalar" &&
            applicator.required_signal_kind != "phase" &&
            applicator.required_signal_kind != "categorical") {
            return SetError(outError, std::string("Materialized row applicator '") + applicator.id + "' has invalid required_signal_kind");
        }
        if (applicator.storage_param.empty()) {
            return SetError(outError, std::string("Materialized row applicator '") + applicator.id + "' is missing storage_param");
        }
        if (applicator.fail_closed_reason.empty()) {
            return SetError(outError, std::string("Materialized row applicator '") + applicator.id + "' is missing fail_closed_reason");
        }
    }
    return !applicators.empty() || SetError(outError, "Materialized contract must contain at least one row applicator");
}


bool ValidateMaterializedExplainoEntries(
    const std::vector<MaterializedExplainoContractEntry>& entries,
    std::string* outError) {
    std::set<std::string> explainoIds;
    for (const MaterializedExplainoContractEntry& entry : entries) {
        if (!explainoIds.insert(entry.id).second) {
            return SetError(outError, std::string("Duplicate Explaino contract entry '") + entry.id + "'");
        }
    }
    return true;
}

bool ValidateLoadedContract(const MaterializedColorPipelineContract& contract, std::string* outError) {
    std::set<std::string> functionIds;
    std::set<std::string> signalTypeIds;
    return ValidateMaterializedSignalTypes(contract.signal_types, &signalTypeIds, outError) &&
        ValidateMaterializedAdapters(contract.adapters, signalTypeIds, outError) &&
        ValidateMaterializedLanes(contract.lanes, &functionIds, signalTypeIds, outError) &&
        ValidateMaterializedEdgeResolution(contract, functionIds, signalTypeIds, outError) &&
        ValidateMaterializedCompatibility(contract.compatibility, functionIds, outError) &&
        ValidateMaterializedCompatibilityAudit(contract, functionIds, outError) &&
        ValidateMaterializedRecipes(contract.recipes, functionIds, outError) &&
        ValidateMaterializedRowApplicators(contract.row_applicators, outError) &&
        ValidateMaterializedExplainoEntries(contract.explaino_entries, outError);
}

} // namespace

bool LoadColorPipelineMaterializedContractJson(
    const std::string& path,
    MaterializedColorPipelineContract* outContract,
    std::string* outError) {
    if (!outContract) {
        return SetError(outError, "Materialized contract load requires output storage");
    }
    std::ifstream input(path, std::ios::in | std::ios::binary);
    if (!input) {
        return SetError(outError, std::string("Unable to open materialized contract '") + path + "'");
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();

    json_min::ParseResult parsed = json_min::Parse(buffer.str());
    if (!parsed.error.empty()) {
        return SetError(outError, std::string("Failed to parse materialized contract: ") + parsed.error);
    }
    if (!parsed.value.is_object()) {
        return SetError(outError, "Materialized contract root must be an object");
    }

    MaterializedColorPipelineContract contract;
    double schemaVersion = 0.0;
    if (!ReadNumber(parsed.value, "schema_version", &schemaVersion, outError)) {
        return false;
    }
    if (schemaVersion != 1.0) {
        return SetError(outError, "Unsupported materialized contract schema_version");
    }
    contract.schema_version = static_cast<int>(schemaVersion);
    if (!ReadString(parsed.value, "source_path", &contract.source_path, outError)) {
        return false;
    }

    const json_min::Value* signalTypeRegistry = RequiredField(parsed.value, "signal_type_registry", outError);
    const json_min::Value* adapterLibrary = parsed.value.get("adapter_library_contract");
    const json_min::Value* edgeResolutionContract = parsed.value.get("edge_resolution_contract");
    const json_min::Value* resolutionAudit = parsed.value.get("color_pipeline_resolution_audit");
    const json_min::Value* functionLibrary = RequiredField(parsed.value, "function_library", outError);
    const json_min::Value* compositionContract = RequiredField(parsed.value, "composition_recipe_contract", outError);
    const json_min::Value* explainoContract = RequiredField(parsed.value, "explaino_contract", outError);
    if (!signalTypeRegistry || !functionLibrary || !compositionContract || !explainoContract) {
        return false;
    }

    const json_min::Value* signalTypes = RequiredField(*signalTypeRegistry, "types", outError);
    if (!signalTypes || !signalTypes->is_array()) {
        return SetError(outError, "signal_type_registry.types must be an array");
    }
    for (const json_min::Value& signalTypeValue : signalTypes->as_array()) {
        MaterializedSignalType signalType;
        if (!ReadSignalType(signalTypeValue, &signalType, outError)) {
            return false;
        }
        contract.signal_types.push_back(std::move(signalType));
    }

    if (adapterLibrary) {
        if (!adapterLibrary->is_object()) {
            return SetError(outError, "adapter_library_contract must be an object");
        }
        const json_min::Value* adapters = RequiredField(*adapterLibrary, "adapters", outError);
        if (!adapters || !adapters->is_array()) {
            return SetError(outError, "adapter_library_contract.adapters must be an array");
        }
        for (const json_min::Value& adapterValue : adapters->as_array()) {
            MaterializedColorPipelineAdapter adapter;
            if (!ReadAdapter(adapterValue, &adapter, outError)) {
                return false;
            }
            contract.adapters.push_back(std::move(adapter));
        }
    }

    if ((edgeResolutionContract == nullptr) != (resolutionAudit == nullptr)) {
        return SetError(outError, "edge_resolution_contract and color_pipeline_resolution_audit must be present together");
    }
    if (edgeResolutionContract) {
        if (!edgeResolutionContract->is_object()) {
            return SetError(outError, "edge_resolution_contract must be an object");
        }
        if (!resolutionAudit->is_object()) {
            return SetError(outError, "color_pipeline_resolution_audit must be an object");
        }
        const json_min::Value* policy = RequiredField(*edgeResolutionContract, "policy", outError);
        if (!policy || !ReadEdgePolicy(*policy, &contract.edge_policy, outError)) {
            return false;
        }
        const json_min::Value* edges = RequiredField(*edgeResolutionContract, "edges", outError);
        if (!edges || !edges->is_array()) {
            return SetError(outError, "edge_resolution_contract.edges must be an array");
        }
        for (const json_min::Value& edgeValue : edges->as_array()) {
            MaterializedColorPipelineEdgeLink edge;
            if (!ReadEdgeLink(edgeValue, &edge, outError)) {
                return false;
            }
            contract.edge_links.push_back(std::move(edge));
        }
        const json_min::Value* cases = RequiredField(*resolutionAudit, "cases", outError);
        if (!cases || !cases->is_array()) {
            return SetError(outError, "color_pipeline_resolution_audit.cases must be an array");
        }
        for (const json_min::Value& caseValue : cases->as_array()) {
            MaterializedColorPipelineResolutionCase resolutionCase;
            if (!ReadResolutionCase(caseValue, &resolutionCase, outError)) {
                return false;
            }
            contract.resolution_cases.push_back(std::move(resolutionCase));
        }
    }

    const json_min::Value* lanes = RequiredField(*functionLibrary, "lanes", outError);
    if (!lanes || !lanes->is_array()) {
        return SetError(outError, "function_library.lanes must be an array");
    }
    for (const json_min::Value& laneValue : lanes->as_array()) {
        MaterializedColorPipelineLane lane;
        if (!ReadLane(laneValue, &lane, outError)) {
            return false;
        }
        contract.lanes.push_back(std::move(lane));
    }

    const json_min::Value* compatibility = RequiredField(*compositionContract, "compatibility", outError);
    if (!compatibility || !compatibility->is_array()) {
        return SetError(outError, "composition_recipe_contract.compatibility must be an array");
    }
    for (const json_min::Value& compatibilityValue : compatibility->as_array()) {
        MaterializedColorPipelineCompatibility row;
        if (!ReadCompatibility(compatibilityValue, &row, outError)) {
            return false;
        }
        contract.compatibility.push_back(std::move(row));
    }

    if (const json_min::Value* compatOverrides = compositionContract->get("compat_overrides")) {
        if (!compatOverrides->is_array()) {
            return SetError(outError, "composition_recipe_contract.compat_overrides must be an array");
        }
        for (const json_min::Value& overrideValue : compatOverrides->as_array()) {
            MaterializedColorPipelineCompatOverride compatOverride;
            if (!ReadCompatOverride(overrideValue, &compatOverride, outError)) {
                return false;
            }
            contract.compat_overrides.push_back(std::move(compatOverride));
        }
    }
    if (const json_min::Value* compatibilityAudit = compositionContract->get("compatibility_audit")) {
        if (!compatibilityAudit->is_array()) {
            return SetError(outError, "composition_recipe_contract.compatibility_audit must be an array");
        }
        for (const json_min::Value& auditValue : compatibilityAudit->as_array()) {
            MaterializedColorPipelineCompatibilityAudit audit;
            if (!ReadCompatibilityAudit(auditValue, &audit, outError)) {
                return false;
            }
            contract.compatibility_audit.push_back(std::move(audit));
        }
    }

    const json_min::Value* recipes = RequiredField(*compositionContract, "recipes", outError);
    if (!recipes || !recipes->is_array()) {
        return SetError(outError, "composition_recipe_contract.recipes must be an array");
    }
    for (const json_min::Value& recipeValue : recipes->as_array()) {
        MaterializedColorPipelineRecipe recipe;
        if (!ReadRecipe(recipeValue, &recipe, outError)) {
            return false;
        }
        contract.recipes.push_back(std::move(recipe));
    }

    const json_min::Value* rowApplicators = RequiredField(*compositionContract, "row_applicators", outError);
    if (!rowApplicators || !rowApplicators->is_array()) {
        return SetError(outError, "composition_recipe_contract.row_applicators must be an array");
    }
    for (const json_min::Value& applicatorValue : rowApplicators->as_array()) {
        MaterializedColorPipelineRowApplicator applicator;
        if (!ReadRowApplicator(applicatorValue, &applicator, outError)) {
            return false;
        }
        contract.row_applicators.push_back(std::move(applicator));
    }

    const json_min::Value* entries = RequiredField(*explainoContract, "entries", outError);
    if (!entries || !entries->is_array()) {
        return SetError(outError, "explaino_contract.entries must be an array");
    }
    for (const json_min::Value& entryValue : entries->as_array()) {
        MaterializedExplainoContractEntry entry;
        if (!ReadExplainoEntry(entryValue, &entry, outError)) {
            return false;
        }
        contract.explaino_entries.push_back(std::move(entry));
    }
    if (contract.explaino_entries.empty()) {
        return SetError(outError, "Materialized contract must include at least one Explaino contract entry");
    }
    if (!ValidateLoadedContract(contract, outError)) {
        return false;
    }

    *outContract = std::move(contract);
    return true;
}

const MaterializedColorPipelineLane* FindMaterializedColorPipelineLane(
    const MaterializedColorPipelineContract& contract,
    const std::string& laneId) {
    for (const MaterializedColorPipelineLane& lane : contract.lanes) {
        if (lane.id == laneId) {
            return &lane;
        }
    }
    return nullptr;
}

const MaterializedColorPipelineFunction* FindMaterializedColorPipelineFunction(
    const MaterializedColorPipelineLane& lane,
    const std::string& functionId) {
    for (const MaterializedColorPipelineFunction& function : lane.functions) {
        if (function.id == functionId) {
            return &function;
        }
    }
    return nullptr;
}

const MaterializedColorPipelineCompatibility* FindMaterializedColorPipelineCompatibility(
    const MaterializedColorPipelineContract& contract,
    const std::string& sourceFunctionId,
    const std::string& paletteFunctionId) {
    for (const MaterializedColorPipelineCompatibility& compatibility : contract.compatibility) {
        if (compatibility.source == sourceFunctionId && compatibility.palette == paletteFunctionId) {
            return &compatibility;
        }
    }
    return nullptr;
}
