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
        ValidateMaterializedLanes(contract.lanes, &functionIds, signalTypeIds, outError) &&
        ValidateMaterializedCompatibility(contract.compatibility, functionIds, outError) &&
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
