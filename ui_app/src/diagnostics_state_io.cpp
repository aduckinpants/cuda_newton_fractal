#include "diagnostics_state_io.h"

#define COLOR_PIPELINE_WINDOW_NO_IMGUI
#include "color_pipeline_window.h"
#undef COLOR_PIPELINE_WINDOW_NO_IMGUI
#include "enum_id_utils.h"
#include "explaino_seed.h"
#include "fractal_family_rules.h"
#include "json_min.h"

#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace {

bool ReadTextFile(const std::filesystem::path& path, std::string* outText, std::string* outError) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        if (outError) *outError = "Failed to open state file: " + path.string();
        return false;
    }

    std::ostringstream text;
    text << file.rdbuf();
    if (!file.good() && !file.eof()) {
        if (outError) *outError = "Failed to read state file: " + path.string();
        return false;
    }
    if (outText) *outText = text.str();
    return true;
}

bool GetRequiredObject(const json_min::Value& object, const char* key, const json_min::Value** outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_object()) {
        if (outError) *outError = std::string("Missing or invalid object field: ") + key;
        return false;
    }
    if (outValue) *outValue = value;
    return true;
}

bool GetRequiredArray(const json_min::Value& object, const char* key, const json_min::Value** outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_array()) {
        if (outError) *outError = std::string("Missing or invalid array field: ") + key;
        return false;
    }
    if (outValue) *outValue = value;
    return true;
}

bool GetRequiredNumber(const json_min::Value& object, const char* key, double* outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_number()) {
        if (outError) *outError = std::string("Missing or invalid number field: ") + key;
        return false;
    }
    if (outValue) *outValue = value->as_number();
    return true;
}

bool GetRequiredBool(const json_min::Value& object, const char* key, bool* outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_bool()) {
        if (outError) *outError = std::string("Missing or invalid bool field: ") + key;
        return false;
    }
    if (outValue) *outValue = value->as_bool();
    return true;
}

bool GetRequiredString(const json_min::Value& object, const char* key, std::string* outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_string()) {
        if (outError) *outError = std::string("Missing or invalid string field: ") + key;
        return false;
    }
    if (outValue) *outValue = value->as_string();
    return true;
}

bool TryGetOptionalString(const json_min::Value& object, const char* key, std::string* outValue) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_string()) return false;
    if (outValue) *outValue = value->as_string();
    return true;
}

bool GetOptionalNumber(const json_min::Value& object, const char* key, double* outValue, bool* outPresent, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value) {
        if (outPresent) *outPresent = false;
        return true;
    }
    if (!value->is_number()) {
        if (outError) *outError = std::string("Invalid optional number field: ") + key;
        return false;
    }
    if (outValue) *outValue = value->as_number();
    if (outPresent) *outPresent = true;
    return true;
}

bool GetOptionalBool(const json_min::Value& object, const char* key, bool* outValue, bool* outPresent, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value) {
        if (outPresent) *outPresent = false;
        return true;
    }
    if (!value->is_bool()) {
        if (outError) *outError = std::string("Invalid optional bool field: ") + key;
        return false;
    }
    if (outValue) *outValue = value->as_bool();
    if (outPresent) *outPresent = true;
    return true;
}

bool SidecarOrientationFieldsAreFinite(const SidecarOrientationVector& orientation) {
    return std::isfinite(orientation.field_embedding_stats) &&
           std::isfinite(orientation.slime_energy_delta) &&
           std::isfinite(orientation.busy_beaver_metrics) &&
           std::isfinite(orientation.decode_stability) &&
           std::isfinite(orientation.diff_magnitude);
}

bool ParseSidecarHashField(const json_min::Value& object,
    const char* key,
    std::uint64_t* outValue,
    std::string* outError) {
    constexpr double kMaxExactJsonInteger = 9007199254740991.0;

    const json_min::Value* value = object.get(key);
    if (!value) {
        if (outError) *outError = std::string("Missing sidecar_orientation field: ") + key;
        return false;
    }

    if (value->is_string()) {
        const std::string text = value->as_string();
        if (text.empty()) {
            if (outError) *outError = std::string("Invalid sidecar_orientation field: ") + key;
            return false;
        }

        std::size_t consumed = 0;
        try {
            const unsigned long long parsed = std::stoull(text, &consumed, 10);
            if (consumed != text.size()) {
                if (outError) *outError = std::string("Invalid sidecar_orientation field: ") + key;
                return false;
            }
            if (outValue) *outValue = static_cast<std::uint64_t>(parsed);
            return true;
        } catch (...) {
            if (outError) *outError = std::string("Invalid sidecar_orientation field: ") + key;
            return false;
        }
    }

    if (!value->is_number()) {
        if (outError) *outError = std::string("Invalid sidecar_orientation field: ") + key;
        return false;
    }

    const double numeric = value->as_number();
    if (numeric < 0.0 || std::floor(numeric) != numeric || numeric > kMaxExactJsonInteger) {
        if (outError) {
            *outError = std::string("sidecar_orientation.") + key +
                " must be an exact non-negative integer; use a quoted decimal string for full 64-bit values";
        }
        return false;
    }

    if (outValue) *outValue = static_cast<std::uint64_t>(numeric);
    return true;
}

bool ParseOptionalSidecarOrientation(const json_min::Value& root,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    std::string* outError) {
    if (outOrientation) *outOrientation = {};
    if (outHasOrientation) *outHasOrientation = false;

    const json_min::Value* orientationObject = root.get("sidecar_orientation");
    if (!orientationObject) return true;
    if (!orientationObject->is_object()) {
        if (outError) *outError = "Missing or invalid object field: sidecar_orientation";
        return false;
    }

    SidecarOrientationVector orientation{};
    if (!ParseSidecarHashField(*orientationObject, "import_signature", &orientation.import_signature, outError)) return false;
    if (!ParseSidecarHashField(*orientationObject, "pack_projection_hash", &orientation.pack_projection_hash, outError)) return false;
    if (!GetRequiredNumber(*orientationObject, "field_embedding_stats", &orientation.field_embedding_stats, outError)) return false;
    if (!GetRequiredNumber(*orientationObject, "slime_energy_delta", &orientation.slime_energy_delta, outError)) return false;
    if (!GetRequiredNumber(*orientationObject, "busy_beaver_metrics", &orientation.busy_beaver_metrics, outError)) return false;
    if (!GetRequiredNumber(*orientationObject, "decode_stability", &orientation.decode_stability, outError)) return false;
    if (!GetRequiredNumber(*orientationObject, "diff_magnitude", &orientation.diff_magnitude, outError)) return false;
    if (!SidecarOrientationFieldsAreFinite(orientation)) {
        if (outError) *outError = "sidecar_orientation must contain only finite values";
        return false;
    }

    if (outOrientation) *outOrientation = orientation;
    if (outHasOrientation) *outHasOrientation = true;
    return true;
}

bool ParseFractalType(const std::string& text, FractalType* outType) {
    return TryParseFractalTypeId(text, outType);
}

bool ParsePolyKind(int rawValue, PolyKind* outKind) {
    if (rawValue == static_cast<int>(PolyKind::z3_minus_1)) { if (outKind) *outKind = PolyKind::z3_minus_1; return true; }
    if (rawValue == static_cast<int>(PolyKind::z4_minus_1)) { if (outKind) *outKind = PolyKind::z4_minus_1; return true; }
    if (rawValue == static_cast<int>(PolyKind::custom)) { if (outKind) *outKind = PolyKind::custom; return true; }
    return false;
}

bool ParseColoringMode(const std::string& text, ColoringMode* outMode) {
    return TryParseColoringModeId(text, outMode);
}

bool ParseColorSignal(const std::string& text, ColorSignal* outSignal) {
    return TryParseColorSignalId(text, outSignal);
}

bool ParseColorPalette(const std::string& text, ColorPalette* outPalette) {
    return TryParseColorPaletteId(text, outPalette);
}

bool ParseColorGradingPreset(const std::string& text, ColorGradingPreset* outGrading) {
    return TryParseColorGradingPresetId(text, outGrading);
}

bool ParseColorPipelineShape(const std::string& text, ColorPipelineShape* outShape) {
    return TryParseColorPipelineShapeId(text, outShape);
}

bool ParseIntField(const json_min::Value& object, const char* key, int* outValue, std::string* outError) {
    double rawValue = 0.0;
    if (!GetRequiredNumber(object, key, &rawValue, outError)) return false;
    if (!std::isfinite(rawValue) || std::floor(rawValue) != rawValue) {
        if (outError) *outError = std::string("Invalid integer field: ") + key;
        return false;
    }
    if (rawValue < static_cast<double>(INT_MIN) || rawValue > static_cast<double>(INT_MAX)) {
        if (outError) *outError = std::string("Out-of-range integer field: ") + key;
        return false;
    }
    if (outValue) *outValue = static_cast<int>(rawValue);
    return true;
}

bool SidecarAutoDemoPolicyFieldsAreValid(const SidecarAutoDemoControllerPolicy& policy, std::string* outError) {
    if (!std::isfinite(policy.paced_loop_interval_seconds) || policy.paced_loop_interval_seconds <= 0.0) {
        if (outError) *outError = "sidecar_auto_demo_policy.paced_loop_interval_seconds must be > 0";
        return false;
    }
    if (!std::isfinite(policy.stop_demonstrated_fraction) ||
        policy.stop_demonstrated_fraction < 0.0 ||
        policy.stop_demonstrated_fraction > 1.0) {
        if (outError) *outError = "sidecar_auto_demo_policy.stop_demonstrated_fraction must be within [0, 1]";
        return false;
    }
    if (policy.stop_uncertain_count < 0) {
        if (outError) *outError = "sidecar_auto_demo_policy.stop_uncertain_count must be >= 0";
        return false;
    }
    return true;
}

bool ParseOptionalSidecarAutoDemoPolicy(const json_min::Value& root,
    SidecarAutoDemoControllerPolicy* outPolicy,
    bool* outHasPolicy,
    std::string* outError) {
    if (outPolicy) *outPolicy = {};
    if (outHasPolicy) *outHasPolicy = false;

    const json_min::Value* policyObject = root.get("sidecar_auto_demo_policy");
    if (!policyObject) return true;
    if (!policyObject->is_object()) {
        if (outError) *outError = "Missing or invalid object field: sidecar_auto_demo_policy";
        return false;
    }

    SidecarAutoDemoControllerPolicy policy{};
    if (!GetRequiredBool(*policyObject, "enabled", &policy.enabled, outError)) return false;
    if (!GetRequiredBool(*policyObject, "allow_runtime_mutation", &policy.allow_runtime_mutation, outError)) return false;
    if (!GetRequiredBool(*policyObject, "run_paced_loop", &policy.run_paced_loop, outError)) return false;
    if (!GetRequiredNumber(*policyObject, "paced_loop_interval_seconds", &policy.paced_loop_interval_seconds, outError)) return false;
    if (!GetRequiredNumber(*policyObject, "stop_demonstrated_fraction", &policy.stop_demonstrated_fraction, outError)) return false;
    if (!ParseIntField(*policyObject, "stop_uncertain_count", &policy.stop_uncertain_count, outError)) return false;
    if (!SidecarAutoDemoPolicyFieldsAreValid(policy, outError)) return false;

    if (outPolicy) *outPolicy = policy;
    if (outHasPolicy) *outHasPolicy = true;
    return true;
}

bool ParseSidecarMutationStringField(const json_min::Value& object,
    size_t index,
    const char* key,
    std::string* outValue,
    std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_string()) {
        if (outError) {
            *outError = "Invalid sidecar_mutation_history[" + std::to_string(index) + "]." + key;
        }
        return false;
    }
    if (outValue) *outValue = value->as_string();
    return true;
}

bool ParseSidecarMutationNumberField(const json_min::Value& object,
    size_t index,
    const char* key,
    double* outValue,
    std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_number() || !std::isfinite(value->as_number())) {
        if (outError) {
            *outError = "Invalid sidecar_mutation_history[" + std::to_string(index) + "]." + key;
        }
        return false;
    }
    if (outValue) *outValue = value->as_number();
    return true;
}

bool ParseOptionalSidecarMutationHistory(const json_min::Value& root,
    SidecarAutoDemoMutationHistory* outHistory,
    bool* outHasHistory,
    std::string* outError) {
    if (outHistory) outHistory->clear();
    if (outHasHistory) *outHasHistory = false;

    const json_min::Value* historyArray = root.get("sidecar_mutation_history");
    if (!historyArray) return true;
    if (!historyArray->is_array()) {
        if (outError) *outError = "Missing or invalid array field: sidecar_mutation_history";
        return false;
    }

    SidecarAutoDemoMutationHistory parsedHistory;
    parsedHistory.reserve(historyArray->as_array().size());
    for (size_t index = 0; index < historyArray->as_array().size(); ++index) {
        const json_min::Value& entry = historyArray->as_array()[index];
        if (!entry.is_object()) {
            if (outError) {
                *outError = "Invalid sidecar_mutation_history[" + std::to_string(index) + "]";
            }
            return false;
        }

        SidecarAutoDemoMutationRecord record;
        if (!ParseSidecarMutationStringField(entry, index, "label", &record.label, outError)) return false;
        if (!ParseSidecarMutationStringField(entry, index, "path", &record.path, outError)) return false;
        if (!ParseSidecarMutationStringField(entry, index, "type", &record.type, outError)) return false;
        if (!ParseSidecarMutationNumberField(entry, index, "target_value", &record.target_value, outError)) return false;
        if (!ParseSidecarMutationNumberField(entry, index, "utility", &record.utility, outError)) return false;
        parsedHistory.push_back(std::move(record));
    }

    if (outHistory) *outHistory = std::move(parsedHistory);
    if (outHasHistory) *outHasHistory = true;
    return true;
}

bool ParsePositiveUInt64Field(const json_min::Value& object,
    const char* key,
    std::uint64_t* outValue,
    std::string* outError) {
    constexpr double kMaxExactJsonInteger = 9007199254740991.0;

    const json_min::Value* value = object.get(key);
    if (!value || !value->is_number()) {
        if (outError) *outError = std::string("Missing or invalid integer field: ") + key;
        return false;
    }

    const double numeric = value->as_number();
    if (!std::isfinite(numeric) || numeric <= 0.0 || std::floor(numeric) != numeric || numeric > kMaxExactJsonInteger) {
        if (outError) *outError = std::string("Invalid positive integer field: ") + key;
        return false;
    }

    if (outValue) *outValue = static_cast<std::uint64_t>(numeric);
    return true;
}

bool FindColorPipelineLaneCatalogIndex(const std::string& laneId, std::size_t* outIndex) {
    const std::vector<ColorPipelineLaneCatalog>& catalogs = GetColorPipelineLaneCatalogs();
    for (std::size_t index = 0; index < catalogs.size(); ++index) {
        if (catalogs[index].lane_id == laneId) {
            if (outIndex) *outIndex = index;
            return true;
        }
    }
    return false;
}

bool FindColorPipelineParamDescriptorIndex(const FunctionDescriptor& descriptor,
    const std::string& path,
    std::size_t* outIndex) {
    for (std::size_t index = 0; index < descriptor.parameters.size(); ++index) {
        if (descriptor.parameters[index].path == path) {
            if (outIndex) *outIndex = index;
            return true;
        }
    }
    return false;
}

bool IsValidColorPipelineEnumValue(const FunctionParamDescriptor& descriptor, const std::string& value) {
    if (descriptor.type != "enum") {
        return false;
    }
    if (descriptor.options.empty()) {
        return true;
    }
    for (const UISchemaOption& option : descriptor.options) {
        if (option.id == value) {
            return true;
        }
    }
    return false;
}

bool ParseColorPipelineDraftParam(const json_min::Value& paramObject,
    const FunctionParamDescriptor& descriptor,
    ColorPipelineParamState* ioParam,
    std::string* outError) {
    if (!ioParam) {
        if (outError) *outError = "color_pipeline_draft parameter parse requires output storage";
        return false;
    }

    std::string path;
    std::string type;
    if (!GetRequiredString(paramObject, "path", &path, outError)) return false;
    if (!GetRequiredString(paramObject, "type", &type, outError)) return false;
    if (path != descriptor.path) {
        if (outError) *outError = "color_pipeline_draft parameter path mismatch for " + descriptor.path;
        return false;
    }
    if (type != descriptor.type) {
        if (outError) *outError = "color_pipeline_draft parameter type mismatch for " + descriptor.path;
        return false;
    }

    ioParam->path = path;
    ioParam->type = type;
    if (descriptor.type == "bool") {
        if (!GetRequiredBool(paramObject, "bool_value", &ioParam->bool_value, outError)) return false;
        return true;
    }
    if (descriptor.type == "enum") {
        if (!GetRequiredString(paramObject, "enum_value", &ioParam->enum_value, outError)) return false;
        if (!IsValidColorPipelineEnumValue(descriptor, ioParam->enum_value)) {
            if (outError) *outError = "Unknown color_pipeline_draft enum value for " + descriptor.path;
            return false;
        }
        return true;
    }

    double numberValue = 0.0;
    if (!GetRequiredNumber(paramObject, "number_value", &numberValue, outError)) return false;
    if (!std::isfinite(numberValue)) {
        if (outError) *outError = "color_pipeline_draft numeric parameter must be finite for " + descriptor.path;
        return false;
    }
    if (descriptor.type == "int") {
        const double rounded = std::round(numberValue);
        if (std::fabs(numberValue - rounded) > 1.0e-6) {
            if (outError) *outError = "color_pipeline_draft integer parameter must be integral for " + descriptor.path;
            return false;
        }
    }
    ioParam->number_value = numberValue;
    return true;
}

bool ParseColorPipelineDraftRow(const json_min::Value& rowObject,
    const ColorPipelineLaneCatalog& catalog,
    ColorPipelineRowState* outRow,
    std::string* outError) {
    if (!outRow) {
        if (outError) *outError = "color_pipeline_draft row parse requires output storage";
        return false;
    }

    std::uint64_t uiRowId = 0;
    bool enabled = true;
    std::string functionId;
    const json_min::Value* parameterValues = nullptr;
    if (!ParsePositiveUInt64Field(rowObject, "ui_row_id", &uiRowId, outError)) return false;
    if (!GetRequiredBool(rowObject, "enabled", &enabled, outError)) return false;
    if (!GetRequiredString(rowObject, "function_id", &functionId, outError)) return false;
    if (!GetRequiredArray(rowObject, "parameter_values", &parameterValues, outError)) return false;

    const FunctionDescriptor* descriptor = FindColorPipelineFunctionDescriptor(catalog, functionId);
    if (!descriptor) {
        if (outError) *outError = "Unknown advanced color function '" + functionId + "' for lane " + catalog.label;
        return false;
    }

    ColorPipelineRowState row;
    if (!BuildColorPipelineRowFromFunctionId(catalog, functionId.c_str(), uiRowId, &row, outError)) {
        return false;
    }
    row.enabled = enabled;

    if (parameterValues->as_array().size() != descriptor->parameters.size()) {
        if (outError) *outError = "color_pipeline_draft parameter count mismatch for function '" + functionId + "'";
        return false;
    }

    std::vector<bool> seenParams(descriptor->parameters.size(), false);
    for (const json_min::Value& paramValue : parameterValues->as_array()) {
        if (!paramValue.is_object()) {
            if (outError) *outError = "color_pipeline_draft parameter_values entries must be objects";
            return false;
        }

        std::string path;
        if (!GetRequiredString(paramValue, "path", &path, outError)) return false;
        std::size_t paramIndex = 0;
        if (!FindColorPipelineParamDescriptorIndex(*descriptor, path, &paramIndex)) {
            if (outError) *outError = "Unknown color_pipeline_draft parameter path '" + path + "' for function '" + functionId + "'";
            return false;
        }
        if (seenParams[paramIndex]) {
            if (outError) *outError = "Duplicate color_pipeline_draft parameter path '" + path + "' for function '" + functionId + "'";
            return false;
        }
        if (!ParseColorPipelineDraftParam(paramValue, descriptor->parameters[paramIndex], &row.parameter_values[paramIndex], outError)) {
            return false;
        }
        seenParams[paramIndex] = true;
    }

    for (std::size_t index = 0; index < seenParams.size(); ++index) {
        if (!seenParams[index]) {
            if (outError) *outError = "Missing color_pipeline_draft parameter path '" + descriptor->parameters[index].path + "' for function '" + functionId + "'";
            return false;
        }
    }

    *outRow = std::move(row);
    return true;
}

bool ParseColorPipelineDraftLane(const json_min::Value& laneObject,
    const ColorPipelineLaneCatalog& catalog,
    ColorPipelineLaneState* outLane,
    std::string* outError) {
    if (!outLane) {
        if (outError) *outError = "color_pipeline_draft lane parse requires output storage";
        return false;
    }

    std::string laneId;
    const json_min::Value* rowsValue = nullptr;
    if (!GetRequiredString(laneObject, "lane_id", &laneId, outError)) return false;
    if (laneId != catalog.lane_id) {
        if (outError) *outError = "color_pipeline_draft lane_id mismatch for lane '" + std::string(catalog.lane_id) + "'";
        return false;
    }

    const json_min::Value* labelValue = laneObject.get("label");
    if (labelValue && !labelValue->is_string()) {
        if (outError) *outError = "Invalid color_pipeline_draft lane label for lane '" + laneId + "'";
        return false;
    }

    if (!GetRequiredArray(laneObject, "rows", &rowsValue, outError)) return false;
    if (rowsValue->as_array().empty()) {
        if (outError) *outError = "color_pipeline_draft lane '" + laneId + "' must contain at least one row";
        return false;
    }

    ColorPipelineLaneState lane;
    lane.lane_id = catalog.lane_id;
    lane.label = catalog.label;
    lane.rows.reserve(rowsValue->as_array().size());
    for (const json_min::Value& rowValue : rowsValue->as_array()) {
        if (!rowValue.is_object()) {
            if (outError) *outError = "color_pipeline_draft rows entries must be objects";
            return false;
        }
        ColorPipelineRowState row;
        if (!ParseColorPipelineDraftRow(rowValue, catalog, &row, outError)) {
            return false;
        }
        lane.rows.push_back(std::move(row));
    }

    *outLane = std::move(lane);
    return true;
}

bool ParseOptionalColorPipelineDraft(const json_min::Value& root,
    FractalType liveFractalType,
    const KernelParams& liveParams,
    ColorPipelineWindowState* outColorPipelineWindow,
    std::string* outError) {
    if (outColorPipelineWindow) {
        *outColorPipelineWindow = {};
    }

    const json_min::Value* draftObject = root.get("color_pipeline_draft");
    if (!draftObject) {
        return true;
    }
    if (!draftObject->is_object()) {
        if (outError) *outError = "Missing or invalid object field: color_pipeline_draft";
        return false;
    }

    const std::vector<ColorPipelineLaneCatalog>& catalogs = GetColorPipelineLaneCatalogs();
    const json_min::Value* lanesValue = nullptr;
    std::uint64_t nextRowId = 0;
    if (!ParsePositiveUInt64Field(*draftObject, "next_row_id", &nextRowId, outError)) return false;
    if (!GetRequiredArray(*draftObject, "lanes", &lanesValue, outError)) return false;
    if (lanesValue->as_array().size() != catalogs.size()) {
        if (outError) *outError = "color_pipeline_draft must contain exactly the shipped programmable lanes";
        return false;
    }

    ColorPipelineWindowState parsedState;
    parsedState.initialized = true;
    parsedState.next_row_id = nextRowId;
    parsedState.lanes.resize(catalogs.size());
    std::vector<bool> seenLanes(catalogs.size(), false);
    std::uint64_t maxRowId = 0;

    for (const json_min::Value& laneValue : lanesValue->as_array()) {
        if (!laneValue.is_object()) {
            if (outError) *outError = "color_pipeline_draft lanes entries must be objects";
            return false;
        }

        std::string laneId;
        if (!GetRequiredString(laneValue, "lane_id", &laneId, outError)) return false;
        std::size_t catalogIndex = 0;
        if (!FindColorPipelineLaneCatalogIndex(laneId, &catalogIndex)) {
            if (outError) *outError = "Unknown color_pipeline_draft lane id: " + laneId;
            return false;
        }
        if (seenLanes[catalogIndex]) {
            if (outError) *outError = "Duplicate color_pipeline_draft lane id: " + laneId;
            return false;
        }

        ColorPipelineLaneState lane;
        if (!ParseColorPipelineDraftLane(laneValue, catalogs[catalogIndex], &lane, outError)) {
            return false;
        }
        for (const ColorPipelineRowState& row : lane.rows) {
            if (row.ui_row_id >= parsedState.next_row_id) {
                if (outError) *outError = "color_pipeline_draft.next_row_id must be greater than every ui_row_id";
                return false;
            }
            if (row.ui_row_id > maxRowId) {
                maxRowId = row.ui_row_id;
            }
        }
        parsedState.lanes[catalogIndex] = std::move(lane);
        seenLanes[catalogIndex] = true;
    }

    for (std::size_t index = 0; index < seenLanes.size(); ++index) {
        if (!seenLanes[index]) {
            if (outError) *outError = "Missing color_pipeline_draft lane id: " + std::string(catalogs[index].lane_id);
            return false;
        }
    }

    if (maxRowId + 1 > parsedState.next_row_id) {
        if (outError) *outError = "color_pipeline_draft.next_row_id must be greater than every ui_row_id";
        return false;
    }

    std::string snapshotError;
    if (!TryBuildColorPipelineLiveSnapshot(liveFractalType, liveParams, &parsedState.live_snapshot, &snapshotError)) {
        if (outError) *outError = "Saved color_pipeline_draft cannot be restored: " + snapshotError;
        return false;
    }
    if (!parsedState.live_snapshot.valid || !parsedState.live_snapshot.draft_import_supported) {
        if (outError) *outError = "Saved color_pipeline_draft is not supported for the saved fractal/color tuple";
        return false;
    }

    if (outColorPipelineWindow) {
        *outColorPipelineWindow = std::move(parsedState);
    }
    return true;
}

bool RequirePositiveIntField(int value, const char* key, std::string* outError) {
    if (value > 0) return true;
    if (outError) *outError = std::string(key) + " must be > 0";
    return false;
}

bool IsFindingStateRelativePathAllowed(const std::filesystem::path& relativePath) {
    if (relativePath.empty()) return false;
    if (relativePath.is_absolute() || relativePath.has_root_name() || relativePath.has_root_directory()) {
        return false;
    }
    const std::filesystem::path normalized = relativePath.lexically_normal();
    if (normalized.empty() || normalized == "." || normalized.filename().empty()) return false;
    for (const auto& part : normalized) {
        if (part == "..") return false;
    }
    return true;
}

} // namespace

bool LoadDiagnosticsStateJson(const std::string& text,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    std::string* outError) {
    return LoadDiagnosticsStateJson(text, ioView, ioParams, ioRender, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, outError);
}

bool LoadDiagnosticsStateJson(const std::string& text,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    ColorPipelineWindowState* outColorPipelineWindow,
    std::string* outError) {
    return LoadDiagnosticsStateJson(text, ioView, ioParams, ioRender, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, outColorPipelineWindow, outError);
}

bool LoadDiagnosticsStateJson(const std::string& text,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    std::string* outError) {
    return LoadDiagnosticsStateJson(
        text,
        ioView,
        ioParams,
        ioRender,
        outOrientation,
        outHasOrientation,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        outError);
}

bool LoadDiagnosticsStateJson(const std::string& text,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    SidecarAutoDemoControllerPolicy* outControllerPolicy,
    bool* outHasControllerPolicy,
    std::string* outError) {
    return LoadDiagnosticsStateJson(
        text,
        ioView,
        ioParams,
        ioRender,
        outOrientation,
        outHasOrientation,
        outControllerPolicy,
        outHasControllerPolicy,
        nullptr,
        nullptr,
        outError);
}

bool LoadDiagnosticsStateJson(const std::string& text,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    SidecarAutoDemoControllerPolicy* outControllerPolicy,
    bool* outHasControllerPolicy,
    SidecarAutoDemoMutationHistory* outMutationHistory,
    bool* outHasMutationHistory,
    std::string* outError) {
    return LoadDiagnosticsStateJson(
        text,
        ioView,
        ioParams,
        ioRender,
        outOrientation,
        outHasOrientation,
        outControllerPolicy,
        outHasControllerPolicy,
        outMutationHistory,
        outHasMutationHistory,
        nullptr,
        outError);
}

bool LoadDiagnosticsStateJson(const std::string& text,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    SidecarAutoDemoControllerPolicy* outControllerPolicy,
    bool* outHasControllerPolicy,
    SidecarAutoDemoMutationHistory* outMutationHistory,
    bool* outHasMutationHistory,
    ColorPipelineWindowState* outColorPipelineWindow,
    std::string* outError) {
    if (outError) outError->clear();
    if (outOrientation) *outOrientation = {};
    if (outHasOrientation) *outHasOrientation = false;
    if (outControllerPolicy) *outControllerPolicy = {};
    if (outHasControllerPolicy) *outHasControllerPolicy = false;
    if (outMutationHistory) outMutationHistory->clear();
    if (outHasMutationHistory) *outHasMutationHistory = false;
    if (outColorPipelineWindow) *outColorPipelineWindow = {};
    if (!ioView || !ioParams || !ioRender) {
        if (outError) *outError = "LoadDiagnosticsStateJson requires non-null output pointers";
        return false;
    }

    json_min::ParseResult parseResult = json_min::Parse(text);
    if (!parseResult.error.empty()) {
        if (outError) *outError = "JSON parse failed: " + parseResult.error;
        return false;
    }
    if (!parseResult.value.is_object()) {
        if (outError) *outError = "State JSON root must be an object";
        return false;
    }

    const json_min::Value& root = parseResult.value;
    if (!ParseOptionalSidecarOrientation(root, outOrientation, outHasOrientation, outError)) return false;
    if (!ParseOptionalSidecarAutoDemoPolicy(root, outControllerPolicy, outHasControllerPolicy, outError)) return false;
    if (!ParseOptionalSidecarMutationHistory(root, outMutationHistory, outHasMutationHistory, outError)) return false;
    int stateVersion = 0;
    if (!ParseIntField(root, "state_version", &stateVersion, outError)) return false;
    if (stateVersion != 1 && stateVersion != 2 && stateVersion != 3) {
        if (outError) *outError = "Unsupported state_version: " + std::to_string(stateVersion);
        return false;
    }

    std::string fractalTypeId;
    if (!GetRequiredString(root, "fractal_type", &fractalTypeId, outError)) return false;

    ViewState nextView = *ioView;
    KernelParams nextParams = *ioParams;
    RenderSettings nextRender = *ioRender;

    if (!ParseFractalType(fractalTypeId, &nextView.fractal_type)) {
        if (outError) *outError = "Unknown fractal_type: " + fractalTypeId;
        return false;
    }

    const json_min::Value* viewObject = nullptr;
    const json_min::Value* paramsObject = nullptr;
    const json_min::Value* renderObject = nullptr;
    if (!GetRequiredObject(root, "view", &viewObject, outError)) return false;
    if (!GetRequiredObject(root, "params", &paramsObject, outError)) return false;
    if (!GetRequiredObject(root, "render", &renderObject, outError)) return false;

    double centerX = 0.0;
    double centerY = 0.0;
    double zoom = 0.0;
    double rotationDegrees = 0.0;
    double centerHpX = 0.0;
    double centerHpY = 0.0;
    double log2Zoom = 0.0;
    double explainoPhase = 0.0;
    double explainoSeedDrift = 0.0;
    bool explainoSeedTween = true;
    bool autoIncrementSeed = nextView.auto_increment_seed;
    double explainoSeedRate = nextView.explaino_seed_rate;
    double explainoPhaseStrength = nextView.explaino_phase_strength;
    if (!GetRequiredNumber(*viewObject, "center_x", &centerX, outError)) return false;
    if (!GetRequiredNumber(*viewObject, "center_y", &centerY, outError)) return false;
    if (!GetRequiredNumber(*viewObject, "zoom", &zoom, outError)) return false;
    if (!GetRequiredNumber(*viewObject, "rotation_degrees", &rotationDegrees, outError)) return false;
    if (!GetRequiredNumber(*viewObject, "center_hp_x", &centerHpX, outError)) return false;
    if (!GetRequiredNumber(*viewObject, "center_hp_y", &centerHpY, outError)) return false;
    if (!GetRequiredNumber(*viewObject, "log2_zoom", &log2Zoom, outError)) return false;
    if (!GetRequiredNumber(*viewObject, "explaino_phase", &explainoPhase, outError)) return false;
    if (!GetRequiredNumber(*viewObject, "explaino_seed_drift", &explainoSeedDrift, outError)) return false;
    if (!GetRequiredBool(*viewObject, "explaino_seed_tween", &explainoSeedTween, outError)) return false;
    bool autoMaxIter = DefaultAutoMaxIterForFractal(nextView.fractal_type);
    if (!GetOptionalBool(*viewObject, "auto_max_iter", &autoMaxIter, nullptr, outError)) return false;
    if (!GetOptionalBool(*viewObject, "auto_increment_seed", &autoIncrementSeed, nullptr, outError)) return false;
    if (!GetOptionalNumber(*viewObject, "explaino_seed_rate", &explainoSeedRate, nullptr, outError)) return false;
    if (!GetOptionalNumber(*viewObject, "explaino_phase_strength", &explainoPhaseStrength, nullptr, outError)) return false;

    nextView.center.x = static_cast<float>(centerX);
    nextView.center.y = static_cast<float>(centerY);
    nextView.zoom = static_cast<float>(zoom);
    nextView.rotation_degrees = static_cast<float>(rotationDegrees);
    nextView.center_hp_x = centerHpX;
    nextView.center_hp_y = centerHpY;
    nextView.log2_zoom = log2Zoom;
    nextView.explaino_phase = static_cast<float>(explainoPhase);
    nextView.explaino_seed_drift = static_cast<float>(explainoSeedDrift);
    nextView.explaino_seed_tween = explainoSeedTween;
    nextView.auto_max_iter = autoMaxIter;
    nextView.auto_increment_seed = autoIncrementSeed;
    nextView.explaino_seed_rate = static_cast<float>(explainoSeedRate);
    nextView.explaino_phase_strength = static_cast<float>(explainoPhaseStrength);

    int maxIter = 0;
    int rawPolyKind = 0;
    double epsilon = 0.0;
    double exposure = 0.0;
    std::string coloringModeId;
    bool hasLegacyColoringModeId = false;
    std::string colorSignalId;
    std::string colorShapeId;
    std::string colorPaletteId;
    std::string colorGradingId;
    double novaAlpha = 0.0;
    double phoenixPReal = 0.0;
    double phoenixPImag = 0.0;
    int multibrotPower = 0;
    double multibrotPowerFloat = static_cast<double>(nextParams.multibrot_power_float);
    double lambdaReal = static_cast<double>(nextParams.lambda_real);
    double lambdaImag = static_cast<double>(nextParams.lambda_imag);
    double explainoSeed = 0.0;
    double explainoSeedB = nextParams.explaino_seed_b;
    double explainoMix = nextParams.explaino_mix;
    double explainoWarpStrength = 0.0;
    double explainoRootSpread = nextParams.explaino_root_spread;
    int explainoRootCount = 0;
    double joyCoupling = nextParams.joy_coupling;
    double foldCoupling = nextParams.fold_coupling;
    double bellCoupling = nextParams.bell_coupling;
    double rippleAmplitude = nextParams.ripple_amplitude;
    double spliceOffset = nextParams.splice_offset;
    double vortexStrength = nextParams.vortex_strength;
    double tensionStrength = nextParams.tension_strength;
    const json_min::Value* polyCoeffsArray = nullptr;
    if (!ParseIntField(*paramsObject, "max_iter", &maxIter, outError)) return false;
    if (!GetRequiredNumber(*paramsObject, "epsilon", &epsilon, outError)) return false;
    if (!GetRequiredNumber(*paramsObject, "exposure", &exposure, outError)) return false;
    if (!ParseIntField(*paramsObject, "poly_kind", &rawPolyKind, outError)) return false;
    if (stateVersion >= 2) {
        if (!GetRequiredNumber(*paramsObject, "nova_alpha", &novaAlpha, outError)) return false;
        if (!GetRequiredNumber(*paramsObject, "phoenix_p_real", &phoenixPReal, outError)) return false;
        if (!GetRequiredNumber(*paramsObject, "phoenix_p_imag", &phoenixPImag, outError)) return false;
        if (!ParseIntField(*paramsObject, "multibrot_power", &multibrotPower, outError)) return false;
        if (!GetOptionalNumber(*paramsObject, "multibrot_power_float", &multibrotPowerFloat, nullptr, outError)) return false;
        if (!GetOptionalNumber(*paramsObject, "lambda_real", &lambdaReal, nullptr, outError)) return false;
        if (!GetOptionalNumber(*paramsObject, "lambda_imag", &lambdaImag, nullptr, outError)) return false;
    }
    const bool hasColorSignalId = TryGetOptionalString(*paramsObject, "color_signal", &colorSignalId);
    const bool hasColorShapeId = TryGetOptionalString(*paramsObject, "color_shape", &colorShapeId);
    const bool hasColorPaletteId = TryGetOptionalString(*paramsObject, "color_palette", &colorPaletteId);
    const bool hasColorGradingId = TryGetOptionalString(*paramsObject, "color_grading", &colorGradingId);
    const bool hasAnyExplicitColorPipeline = hasColorSignalId || hasColorPaletteId || hasColorGradingId;
    hasLegacyColoringModeId = stateVersion >= 2 && TryGetOptionalString(*paramsObject, "coloring_mode", &coloringModeId);
    if (hasAnyExplicitColorPipeline && !(hasColorSignalId && hasColorPaletteId && hasColorGradingId)) {
        if (outError) *outError = "color_signal, color_palette, and color_grading must all be provided together";
        return false;
    }
    if (stateVersion >= 2 && !hasAnyExplicitColorPipeline && !hasLegacyColoringModeId) {
        if (outError) *outError = "Missing or invalid string field: coloring_mode";
        return false;
    }
    if (!GetRequiredNumber(*paramsObject, "explaino_seed", &explainoSeed, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "explaino_seed_b", &explainoSeedB, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "explaino_mix", &explainoMix, nullptr, outError)) return false;
    if (!GetRequiredNumber(*paramsObject, "explaino_warp_strength", &explainoWarpStrength, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "explaino_root_spread", &explainoRootSpread, nullptr, outError)) return false;
    if (!ParseIntField(*paramsObject, "explaino_root_count", &explainoRootCount, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "joy_coupling", &joyCoupling, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "fold_coupling", &foldCoupling, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "bell_coupling", &bellCoupling, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "ripple_amplitude", &rippleAmplitude, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "splice_offset", &spliceOffset, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "vortex_strength", &vortexStrength, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "tension_strength", &tensionStrength, nullptr, outError)) return false;
    if (!GetRequiredArray(*paramsObject, "poly_coeffs", &polyCoeffsArray, outError)) return false;
    if (polyCoeffsArray->as_array().size() != 5) {
        if (outError) *outError = "poly_coeffs must contain exactly 5 numbers";
        return false;
    }

    if (!ParsePolyKind(rawPolyKind, &nextParams.poly_kind)) {
        if (outError) *outError = "Unknown poly_kind: " + std::to_string(rawPolyKind);
        return false;
    }
    if (IsExplainoFamily(nextView.fractal_type) && nextParams.poly_kind != PolyKind::custom) {
        if (outError) *outError = "poly_kind must be custom for fractal_type " + fractalTypeId;
        return false;
    }
    if (stateVersion >= 2 && !hasAnyExplicitColorPipeline) {
        if (!ParseColoringMode(coloringModeId, &nextParams.coloring_mode)) {
            if (outError) *outError = "Unknown coloring_mode: " + coloringModeId;
            return false;
        }
        nextParams.nova_alpha = static_cast<float>(novaAlpha);
        nextParams.phoenix_p_real = static_cast<float>(phoenixPReal);
        nextParams.phoenix_p_imag = static_cast<float>(phoenixPImag);
        nextParams.multibrot_power = multibrotPower;
        nextParams.multibrot_power_float = static_cast<float>(multibrotPowerFloat);
        nextParams.lambda_real = static_cast<float>(lambdaReal);
        nextParams.lambda_imag = static_cast<float>(lambdaImag);
    } else {
        nextParams.coloring_mode = DefaultColoringModeForFractal(nextView.fractal_type);
    }
    if (stateVersion >= 2 && !paramsObject->get("multibrot_power_float")) {
        nextParams.multibrot_power_float = static_cast<float>(nextParams.multibrot_power);
    }
    if (!IsColoringModeAllowedForFractal(nextView.fractal_type, nextParams.coloring_mode)) {
        if (outError) {
            std::string coloringModeName = "unknown";
            switch (nextParams.coloring_mode) {
            case ColoringMode::root_basin: coloringModeName = "root_basin"; break;
            case ColoringMode::iteration_count: coloringModeName = "iteration_count"; break;
            case ColoringMode::smooth_escape: coloringModeName = "smooth_escape"; break;
            case ColoringMode::joy_basins: coloringModeName = "joy_basins"; break;
            case ColoringMode::phase: coloringModeName = "phase"; break;
            case ColoringMode::iteration_bands: coloringModeName = "iteration_bands"; break;
            }
            *outError = "coloring_mode " + coloringModeName + " is not allowed for fractal_type " + fractalTypeId;
        }
        return false;
    }
    if (hasAnyExplicitColorPipeline) {
        ColorPipelineSelection explicitPipeline{};
        if (!ParseColorSignal(colorSignalId, &explicitPipeline.signal)) {
            if (outError) *outError = "Unknown color_signal: " + colorSignalId;
            return false;
        }
        if (!ParseColorPalette(colorPaletteId, &explicitPipeline.palette)) {
            if (outError) *outError = "Unknown color_palette: " + colorPaletteId;
            return false;
        }
        if (!ParseColorGradingPreset(colorGradingId, &explicitPipeline.grading)) {
            if (outError) *outError = "Unknown color_grading: " + colorGradingId;
            return false;
        }
        if (hasColorShapeId && !ParseColorPipelineShape(colorShapeId, &nextParams.color_shape)) {
            if (outError) *outError = "Unknown color_shape: " + colorShapeId;
            return false;
        }
        ColoringMode derivedMode = ColoringMode::root_basin;
        if (!TryMirroredColoringModeForPipeline(explicitPipeline, &derivedMode)) {
            if (outError) *outError = "Unsupported split-color combination in saved state";
            return false;
        }
        if (!IsColoringModeAllowedForFractal(nextView.fractal_type, derivedMode)) {
            if (outError) *outError = "split-color combination is not allowed for fractal_type " + fractalTypeId;
            return false;
        }
        nextParams.color_pipeline = explicitPipeline;
        nextParams.coloring_mode = derivedMode;
    } else {
        nextParams.color_pipeline = stateVersion >= 2
            ? ColorPipelineForLegacyMode(nextParams.coloring_mode)
            : DefaultColorPipelineForFractal(nextView.fractal_type);
    }
    if (!RequirePositiveIntField(maxIter, "max_iter", outError)) return false;
    nextParams.max_iter = maxIter;
    nextParams.epsilon = static_cast<float>(epsilon);
    nextParams.exposure = static_cast<float>(exposure);
    nextParams.explaino_seed = explainoSeed;
    nextParams.explaino_seed_b = explainoSeedB;
    nextParams.explaino_mix = static_cast<float>(explainoMix);
    nextParams.explaino_warp_strength = static_cast<float>(explainoWarpStrength);
    nextParams.explaino_root_spread = static_cast<float>(explainoRootSpread);
    nextParams.explaino_root_count = explainoRootCount;
    nextParams.joy_coupling = static_cast<float>(joyCoupling);
    nextParams.fold_coupling = static_cast<float>(foldCoupling);
    nextParams.bell_coupling = static_cast<float>(bellCoupling);
    nextParams.ripple_amplitude = static_cast<float>(rippleAmplitude);
    nextParams.splice_offset = static_cast<float>(spliceOffset);
    nextParams.vortex_strength = static_cast<float>(vortexStrength);
    nextParams.tension_strength = static_cast<float>(tensionStrength);
    if (IsExplainoFamily(nextView.fractal_type)) {
        ExplainoSeedNormalize(nextView, nextParams);
    }

    // Color grading (v3+, optional for backward compat)
    double colorSaturation = nextParams.color_saturation;
    double colorContrast = nextParams.color_contrast;
    double colorTintR = nextParams.color_tint_r;
    double colorTintG = nextParams.color_tint_g;
    double colorTintB = nextParams.color_tint_b;
    double colorPhaseSignalOffset = nextParams.color_phase_signal_offset;
    double colorPhaseWrapCycles = nextParams.color_phase_wrap_cycles;
    double colorPhasePaletteOffset = nextParams.color_phase_palette_offset;
    double colorShapeOffset = nextParams.color_shape_offset;
    double colorShapeScale = nextParams.color_shape_scale;
    double colorShapeRepeatFrequency = nextParams.color_shape_repeat_frequency;
    double colorShapeRepeatPhase = nextParams.color_shape_repeat_phase;
    int colorShapePosterizeSteps = nextParams.color_shape_posterize_steps;
    double colorShapePosterizeStepsRaw = static_cast<double>(colorShapePosterizeSteps);
    double colorShapePosterizeMix = nextParams.color_shape_posterize_mix;
    double colorShapeBias = nextParams.color_shape_bias;
    double colorShapeGain = nextParams.color_shape_gain;
    double colorShapeWindowCenter = nextParams.color_shape_window_center;
    double colorShapeWindowWidth = nextParams.color_shape_window_width;
    double colorShapeWindowSoftness = nextParams.color_shape_window_softness;
    int colorIterationBandCount = nextParams.color_iteration_band_count;
    double colorIterationBandCountRaw = static_cast<double>(colorIterationBandCount);
    double colorIterationBandSoftness = nextParams.color_iteration_band_softness;
    double colorIterationBandEmphasis = nextParams.color_iteration_band_emphasis;
    double colorIterationBandPaletteOffset = nextParams.color_iteration_band_palette_offset;
    double colorSmoothEscapeScale = nextParams.color_smooth_escape_scale;
    double colorSmoothEscapeBias = nextParams.color_smooth_escape_bias;
    double colorEscapeMagnitudeScale = nextParams.color_escape_magnitude_scale;
    double colorEscapeMagnitudeBias = nextParams.color_escape_magnitude_bias;
    double colorOrbitStripeFrequency = nextParams.color_orbit_stripe_frequency;
    double colorOrbitStripePhase = nextParams.color_orbit_stripe_phase;
    double colorRootProximityScale = nextParams.color_root_proximity_scale;
    double colorRootProximityBias = nextParams.color_root_proximity_bias;
    double colorHeatmapCycleScale = nextParams.color_heatmap_cycle_scale;
    double colorHeatmapSaturation = nextParams.color_heatmap_saturation;
    double colorContrastLiftExposure = nextParams.color_contrast_lift_exposure;
    double colorContrastLiftSaturation = nextParams.color_contrast_lift_saturation;
    if (!GetOptionalNumber(*paramsObject, "color_saturation", &colorSaturation, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_contrast", &colorContrast, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_tint_r", &colorTintR, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_tint_g", &colorTintG, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_tint_b", &colorTintB, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_phase_signal_offset", &colorPhaseSignalOffset, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_phase_wrap_cycles", &colorPhaseWrapCycles, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_phase_palette_offset", &colorPhasePaletteOffset, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_shape_offset", &colorShapeOffset, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_shape_scale", &colorShapeScale, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_shape_repeat_frequency", &colorShapeRepeatFrequency, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_shape_repeat_phase", &colorShapeRepeatPhase, nullptr, outError)) return false;
    bool hasColorShapePosterizeSteps = false;
    if (!GetOptionalNumber(*paramsObject, "color_shape_posterize_steps", &colorShapePosterizeStepsRaw, &hasColorShapePosterizeSteps, outError)) return false;
    if (hasColorShapePosterizeSteps) {
        if (!std::isfinite(colorShapePosterizeStepsRaw) || std::floor(colorShapePosterizeStepsRaw) != colorShapePosterizeStepsRaw) {
            if (outError) *outError = "Invalid integer field: color_shape_posterize_steps";
            return false;
        }
        if (colorShapePosterizeStepsRaw < static_cast<double>(INT_MIN) ||
            colorShapePosterizeStepsRaw > static_cast<double>(INT_MAX)) {
            if (outError) *outError = "Out-of-range integer field: color_shape_posterize_steps";
            return false;
        }
        colorShapePosterizeSteps = static_cast<int>(colorShapePosterizeStepsRaw);
    }
    if (!GetOptionalNumber(*paramsObject, "color_shape_posterize_mix", &colorShapePosterizeMix, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_shape_bias", &colorShapeBias, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_shape_gain", &colorShapeGain, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_shape_window_center", &colorShapeWindowCenter, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_shape_window_width", &colorShapeWindowWidth, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_shape_window_softness", &colorShapeWindowSoftness, nullptr, outError)) return false;
    bool hasColorIterationBandCount = false;
    if (!GetOptionalNumber(*paramsObject, "color_iteration_band_count", &colorIterationBandCountRaw, &hasColorIterationBandCount, outError)) return false;
    if (hasColorIterationBandCount) {
        if (!std::isfinite(colorIterationBandCountRaw) || std::floor(colorIterationBandCountRaw) != colorIterationBandCountRaw) {
            if (outError) *outError = "Invalid integer field: color_iteration_band_count";
            return false;
        }
        if (colorIterationBandCountRaw < static_cast<double>(INT_MIN) ||
            colorIterationBandCountRaw > static_cast<double>(INT_MAX)) {
            if (outError) *outError = "Out-of-range integer field: color_iteration_band_count";
            return false;
        }
        colorIterationBandCount = static_cast<int>(colorIterationBandCountRaw);
    }
    if (!GetOptionalNumber(*paramsObject, "color_iteration_band_softness", &colorIterationBandSoftness, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_iteration_band_emphasis", &colorIterationBandEmphasis, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_iteration_band_palette_offset", &colorIterationBandPaletteOffset, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_smooth_escape_scale", &colorSmoothEscapeScale, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_smooth_escape_bias", &colorSmoothEscapeBias, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_escape_magnitude_scale", &colorEscapeMagnitudeScale, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_escape_magnitude_bias", &colorEscapeMagnitudeBias, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_orbit_stripe_frequency", &colorOrbitStripeFrequency, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_orbit_stripe_phase", &colorOrbitStripePhase, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_root_proximity_scale", &colorRootProximityScale, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_root_proximity_bias", &colorRootProximityBias, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_heatmap_cycle_scale", &colorHeatmapCycleScale, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_heatmap_saturation", &colorHeatmapSaturation, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_contrast_lift_exposure", &colorContrastLiftExposure, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_contrast_lift_saturation", &colorContrastLiftSaturation, nullptr, outError)) return false;
    nextParams.color_saturation = static_cast<float>(colorSaturation);
    nextParams.color_contrast = static_cast<float>(colorContrast);
    nextParams.color_tint_r = static_cast<float>(colorTintR);
    nextParams.color_tint_g = static_cast<float>(colorTintG);
    nextParams.color_tint_b = static_cast<float>(colorTintB);
    nextParams.color_phase_signal_offset = static_cast<float>(colorPhaseSignalOffset);
    nextParams.color_phase_wrap_cycles = static_cast<float>(colorPhaseWrapCycles);
    nextParams.color_phase_palette_offset = static_cast<float>(colorPhasePaletteOffset);
    nextParams.color_shape_offset = static_cast<float>(colorShapeOffset);
    nextParams.color_shape_scale = static_cast<float>(colorShapeScale);
    nextParams.color_shape_repeat_frequency = static_cast<float>(colorShapeRepeatFrequency);
    nextParams.color_shape_repeat_phase = static_cast<float>(colorShapeRepeatPhase);
    nextParams.color_shape_posterize_steps = colorShapePosterizeSteps;
    nextParams.color_shape_posterize_mix = static_cast<float>(colorShapePosterizeMix);
    nextParams.color_shape_bias = static_cast<float>(colorShapeBias);
    nextParams.color_shape_gain = static_cast<float>(colorShapeGain);
    nextParams.color_shape_window_center = static_cast<float>(colorShapeWindowCenter);
    nextParams.color_shape_window_width = static_cast<float>(colorShapeWindowWidth);
    nextParams.color_shape_window_softness = static_cast<float>(colorShapeWindowSoftness);
    nextParams.color_iteration_band_count = colorIterationBandCount;
    nextParams.color_iteration_band_softness = static_cast<float>(colorIterationBandSoftness);
    nextParams.color_iteration_band_emphasis = static_cast<float>(colorIterationBandEmphasis);
    nextParams.color_iteration_band_palette_offset = static_cast<float>(colorIterationBandPaletteOffset);
    nextParams.color_smooth_escape_scale = static_cast<float>(colorSmoothEscapeScale);
    nextParams.color_smooth_escape_bias = static_cast<float>(colorSmoothEscapeBias);
    nextParams.color_escape_magnitude_scale = static_cast<float>(colorEscapeMagnitudeScale);
    nextParams.color_escape_magnitude_bias = static_cast<float>(colorEscapeMagnitudeBias);
    nextParams.color_orbit_stripe_frequency = static_cast<float>(colorOrbitStripeFrequency);
    nextParams.color_orbit_stripe_phase = static_cast<float>(colorOrbitStripePhase);
    nextParams.color_root_proximity_scale = static_cast<float>(colorRootProximityScale);
    nextParams.color_root_proximity_bias = static_cast<float>(colorRootProximityBias);
    nextParams.color_heatmap_cycle_scale = static_cast<float>(colorHeatmapCycleScale);
    nextParams.color_heatmap_saturation = static_cast<float>(colorHeatmapSaturation);
    nextParams.color_contrast_lift_exposure = static_cast<float>(colorContrastLiftExposure);
    nextParams.color_contrast_lift_saturation = static_cast<float>(colorContrastLiftSaturation);

    // explaino_cluster_radius (optional for backward compat)
    double explainoClusterRadius = nextParams.explaino_cluster_radius;
    if (!GetOptionalNumber(*paramsObject, "explaino_cluster_radius", &explainoClusterRadius, nullptr, outError)) return false;
    nextParams.explaino_cluster_radius = static_cast<float>(explainoClusterRadius);

    // transcendental_func (optional for backward compat)
    {
        const json_min::Value* tfVal = paramsObject->get("transcendental_func");
        if (tfVal) {
            if (!tfVal->is_string()) {
                if (outError) *outError = "Invalid transcendental_func field";
                return false;
            }
            const std::string tfStr = tfVal->as_string();
            if (tfStr == "f_sin") nextParams.transcendental_func = TranscendentalFunc::f_sin;
            else if (tfStr == "f_exp_minus_1") nextParams.transcendental_func = TranscendentalFunc::f_exp_minus_1;
            else if (tfStr == "f_cosh") nextParams.transcendental_func = TranscendentalFunc::f_cosh;
            else {
                if (outError) *outError = "Unknown transcendental_func: " + tfStr;
                return false;
            }
        }
    }

    // momentum_beta (optional for backward compat)
    {
        const json_min::Value* mbVal = paramsObject->get("momentum_beta");
        if (mbVal) {
            if (!mbVal->is_number()) {
                if (outError) *outError = "Invalid momentum_beta field";
                return false;
            }
            nextParams.momentum_beta = static_cast<float>(mbVal->as_number());
        }
    }

    // mcmullen_preset (optional for backward compat)
    {
        const json_min::Value* mpVal = paramsObject->get("mcmullen_preset");
        if (mpVal) {
            if (!mpVal->is_string()) {
                if (outError) *outError = "Invalid mcmullen_preset field";
                return false;
            }
            std::string mpStr = mpVal->as_string();
            if (mpStr == "z3_z3") nextParams.mcmullen_preset = McMullenPreset::z3_z3;
            else if (mpStr == "z2_z2") nextParams.mcmullen_preset = McMullenPreset::z2_z2;
            else if (mpStr == "z4_z2") nextParams.mcmullen_preset = McMullenPreset::z4_z2;
            else if (mpStr == "z3_z2") nextParams.mcmullen_preset = McMullenPreset::z3_z2;
            else {
                if (outError) *outError = "Unknown mcmullen_preset: " + mpStr;
                return false;
            }
        }
    }

    for (size_t index = 0; index < 5; ++index) {
        const json_min::Value& coeff = polyCoeffsArray->as_array()[index];
        if (!coeff.is_number()) {
            if (outError) *outError = "poly_coeffs must contain only numbers";
            return false;
        }
        nextParams.poly_coeffs[index] = static_cast<float>(coeff.as_number());
    }

    int width = 0;
    int height = 0;
    int blockSize = 0;
    int deviceId = 0;
    double interactionDebounceMs = static_cast<double>(nextRender.interaction_debounce_ms);
    double previewTargetFps = nextRender.preview_target_fps;
    double previewMinScale = nextRender.preview_min_scale;
    if (!ParseIntField(*renderObject, "width", &width, outError)) return false;
    if (!ParseIntField(*renderObject, "height", &height, outError)) return false;
    if (!ParseIntField(*renderObject, "block_size", &blockSize, outError)) return false;
    if (!ParseIntField(*renderObject, "device_id", &deviceId, outError)) return false;
    if (!GetOptionalNumber(*renderObject, "interaction_debounce_ms", &interactionDebounceMs, nullptr, outError)) return false;
    if (!GetOptionalNumber(*renderObject, "preview_target_fps", &previewTargetFps, nullptr, outError)) return false;
    if (!GetOptionalNumber(*renderObject, "preview_min_scale", &previewMinScale, nullptr, outError)) return false;
    if (!RequirePositiveIntField(width, "width", outError)) return false;
    if (!RequirePositiveIntField(height, "height", outError)) return false;
    nextRender.resolution.x = width;
    nextRender.resolution.y = height;
    nextRender.block_size = blockSize;
    nextRender.device_id = deviceId;
    nextRender.interaction_debounce_ms = static_cast<int>(interactionDebounceMs);
    nextRender.preview_target_fps = static_cast<float>(previewTargetFps);
    nextRender.preview_min_scale = static_cast<float>(previewMinScale);

    ColorPipelineWindowState nextColorPipelineWindow;
    if (!ParseOptionalColorPipelineDraft(root, nextView.fractal_type, nextParams, &nextColorPipelineWindow, outError)) {
        return false;
    }

    *ioView = nextView;
    *ioParams = nextParams;
    *ioRender = nextRender;
    if (outColorPipelineWindow) *outColorPipelineWindow = std::move(nextColorPipelineWindow);
    return true;
}

bool LoadDiagnosticsStateFile(const std::string& path,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    std::string* outError) {
    return LoadDiagnosticsStateFile(path, ioView, ioParams, ioRender, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, outError);
}

bool LoadDiagnosticsStateFile(const std::string& path,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    ColorPipelineWindowState* outColorPipelineWindow,
    std::string* outError) {
    return LoadDiagnosticsStateFile(path, ioView, ioParams, ioRender, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, outColorPipelineWindow, outError);
}

bool LoadDiagnosticsStateFile(const std::string& path,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    std::string* outError) {
    return LoadDiagnosticsStateFile(
        path,
        ioView,
        ioParams,
        ioRender,
        outOrientation,
        outHasOrientation,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        outError);
}

bool LoadDiagnosticsStateFile(const std::string& path,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    SidecarAutoDemoControllerPolicy* outControllerPolicy,
    bool* outHasControllerPolicy,
    std::string* outError) {
    return LoadDiagnosticsStateFile(
        path,
        ioView,
        ioParams,
        ioRender,
        outOrientation,
        outHasOrientation,
        outControllerPolicy,
        outHasControllerPolicy,
        nullptr,
        nullptr,
        outError);
}

bool LoadDiagnosticsStateFile(const std::string& path,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    SidecarAutoDemoControllerPolicy* outControllerPolicy,
    bool* outHasControllerPolicy,
    SidecarAutoDemoMutationHistory* outMutationHistory,
    bool* outHasMutationHistory,
    std::string* outError) {
    return LoadDiagnosticsStateFile(
        path,
        ioView,
        ioParams,
        ioRender,
        outOrientation,
        outHasOrientation,
        outControllerPolicy,
        outHasControllerPolicy,
        outMutationHistory,
        outHasMutationHistory,
        nullptr,
        outError);
}

bool LoadDiagnosticsStateFile(const std::string& path,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    SidecarAutoDemoControllerPolicy* outControllerPolicy,
    bool* outHasControllerPolicy,
    SidecarAutoDemoMutationHistory* outMutationHistory,
    bool* outHasMutationHistory,
    ColorPipelineWindowState* outColorPipelineWindow,
    std::string* outError) {
    if (outError) outError->clear();
    std::string text;
    if (!ReadTextFile(std::filesystem::path(path), &text, outError)) return false;
    return LoadDiagnosticsStateJson(
        text,
        ioView,
        ioParams,
        ioRender,
        outOrientation,
        outHasOrientation,
        outControllerPolicy,
        outHasControllerPolicy,
        outMutationHistory,
        outHasMutationHistory,
        outColorPipelineWindow,
        outError);
}

bool ResolveFindingStateJsonPath(const std::string& selectedPath,
    std::string* outStateJsonPath,
    std::string* outError) {
    if (outError) outError->clear();

    const std::filesystem::path inputPath = std::filesystem::path(selectedPath).lexically_normal();
    if (!std::filesystem::exists(inputPath)) {
        if (outError) *outError = "Selected path does not exist: " + inputPath.string();
        return false;
    }

    if (inputPath.filename() == "state.json") {
        if (outStateJsonPath) *outStateJsonPath = inputPath.string();
        return true;
    }

    if (inputPath.filename() != "finding.json") {
        if (outError) *outError = "Select either state.json or finding.json";
        return false;
    }

    std::string text;
    if (!ReadTextFile(inputPath, &text, outError)) return false;

    json_min::ParseResult parseResult = json_min::Parse(text);
    if (!parseResult.error.empty()) {
        if (outError) *outError = "JSON parse failed: " + parseResult.error;
        return false;
    }
    if (!parseResult.value.is_object()) {
        if (outError) *outError = "Finding metadata must be a JSON object";
        return false;
    }

    std::string stateFileName;
    if (!GetRequiredString(parseResult.value, "state_file", &stateFileName, outError)) return false;
    const std::filesystem::path relativeStatePath(stateFileName);
    if (!IsFindingStateRelativePathAllowed(relativeStatePath)) {
        if (outError) *outError = "state_file must stay within the finding directory";
        return false;
    }
    const std::filesystem::path statePath = (inputPath.parent_path() / relativeStatePath).lexically_normal();
    if (!std::filesystem::exists(statePath)) {
        if (outError) *outError = "Finding metadata points to missing state file: " + statePath.string();
        return false;
    }
    if (outStateJsonPath) *outStateJsonPath = statePath.string();
    return true;
}