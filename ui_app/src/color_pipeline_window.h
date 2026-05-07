#pragma once

#include "enum_id_utils.h"
#include "fractal_family_rules.h"
#include "function_descriptor.h"
#include "imgui.h"
#include "imgui_stack_editor.h"
#include "schema_binding.h"

#include <cmath>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

struct ColorPipelineParamState {
    std::string path;
    std::string type;
    double number_value = 0.0;
    bool bool_value = false;
    std::string enum_value;
};

struct ColorPipelineRowState {
    std::uint64_t ui_row_id = 0;
    bool enabled = true;
    std::string function_id;
    std::vector<ColorPipelineParamState> parameter_values;
};

struct ColorPipelineLaneState {
    std::string lane_id;
    std::string label;
    std::vector<ColorPipelineRowState> rows;
};

struct ColorPipelineLiveSnapshot {
    bool valid = false;
    bool draft_import_supported = false;
    FractalType fractal_type = FractalType::explaino;
    ColoringMode coloring_mode = ColoringMode::root_basin;
    ColorPipelineSelection pipeline{};
    std::vector<ColorPipelineLaneState> lanes;
};

struct ColorPipelineWindowState {
    bool open = false;
    bool initialized = false;
    bool auto_apply_supported_recipe = true;
    std::uint64_t next_row_id = 1;
    std::vector<ColorPipelineLaneState> lanes;
    ColorPipelineLiveSnapshot live_snapshot;
    std::vector<std::string> validation_messages;
};

enum class ColorPipelineDraftApplyStatus {
    live_unavailable,
    matches_live,
    can_apply,
    unsupported_tuple,
    disallowed_for_family,
    invalid_params,
};

struct ColorPipelineDraftApplyState {
    ColorPipelineDraftApplyStatus status = ColorPipelineDraftApplyStatus::live_unavailable;
    std::string message;
};

struct ColorPipelineRenderInteractionState {
    bool has_active_item = false;
    bool interacted = false;
};

struct ColorPipelineLaneCatalog {
    const char* lane_id = "";
    const char* label = "";
    const char* default_function_id = "";
    std::vector<FunctionDescriptor> functions;
};

inline json_min::Value MakeColorPipelineNumberValue(double value) {
    json_min::Value out;
    out.v = value;
    return out;
}

inline json_min::Value MakeColorPipelineBoolValue(bool value) {
    json_min::Value out;
    out.v = value;
    return out;
}

template <typename T>
inline void ClampColorPipelineNumericValue(T* value, const NumericControlRange& range) {
    if (!value) {
        return;
    }
    if (range.has_hard_min && *value < static_cast<T>(range.hard_min)) {
        *value = static_cast<T>(range.hard_min);
    }
    if (range.has_hard_max && *value > static_cast<T>(range.hard_max)) {
        *value = static_cast<T>(range.hard_max);
    }
}

inline NumericControlRange ResolveColorPipelineNumericControlRange(const FunctionParamDescriptor& param) {
    NumericControlRange range;
    if (param.has_min) {
        range.widget_min = param.min_value;
        range.hard_min = param.min_value;
        range.has_widget_min = true;
        range.has_hard_min = true;
    }
    if (param.has_max) {
        range.widget_max = param.max_value;
        range.hard_max = param.max_value;
        range.has_widget_max = true;
        range.has_hard_max = true;
    }
    return range;
}

inline NumericDragWidgetBounds ResolveColorPipelineNumericDragWidgetBounds(const FunctionParamDescriptor& param) {
    NumericDragWidgetBounds bounds;
    const NumericControlRange range = ResolveColorPipelineNumericControlRange(param);
    if (range.has_hard_min && range.has_hard_max && range.hard_max > range.hard_min) {
        bounds.min = range.hard_min;
        bounds.max = range.hard_max;
        bounds.has_bounds = true;
    }
    return bounds;
}

inline FunctionParamDescriptor MakeColorPipelineFloatParam(
    const char* path,
    const char* label,
    const char* help,
    double minValue,
    double maxValue,
    double stepValue,
    double defaultValue) {
    FunctionParamDescriptor param;
    param.path = path ? path : "";
    param.type = "float";
    param.label = label ? label : "";
    param.help = help ? help : "";
    param.has_min = true;
    param.min_value = minValue;
    param.has_max = true;
    param.max_value = maxValue;
    param.has_step = true;
    param.step_value = stepValue;
    param.has_default = true;
    param.default_value = MakeColorPipelineNumberValue(defaultValue);
    return param;
}

inline FunctionParamDescriptor MakeColorPipelineIntParam(
    const char* path,
    const char* label,
    const char* help,
    int minValue,
    int maxValue,
    int stepValue,
    int defaultValue) {
    FunctionParamDescriptor param;
    param.path = path ? path : "";
    param.type = "int";
    param.label = label ? label : "";
    param.help = help ? help : "";
    param.has_min = true;
    param.min_value = (double)minValue;
    param.has_max = true;
    param.max_value = (double)maxValue;
    param.has_step = true;
    param.step_value = (double)stepValue;
    param.has_default = true;
    param.default_value = MakeColorPipelineNumberValue((double)defaultValue);
    return param;
}

inline FunctionParamDescriptor MakeColorPipelineBoolParam(
    const char* path,
    const char* label,
    const char* help,
    bool defaultValue) {
    FunctionParamDescriptor param;
    param.path = path ? path : "";
    param.type = "bool";
    param.label = label ? label : "";
    param.help = help ? help : "";
    param.has_default = true;
    param.default_value = MakeColorPipelineBoolValue(defaultValue);
    return param;
}

inline FunctionDescriptor MakeColorPipelineFunction(
    const char* id,
    const char* name,
    const char* description,
    std::vector<FunctionParamDescriptor> parameters) {
    FunctionDescriptor descriptor;
    descriptor.id = id ? id : "";
    descriptor.name = name ? name : "";
    descriptor.description = description ? description : "";
    descriptor.parameters = std::move(parameters);
    return descriptor;
}

inline const char* AdvancedColorSignalFunctionId(ColorSignal value) {
    switch (value) {
    case ColorSignal::smooth_escape:
        return "smooth_escape_ramp";
    case ColorSignal::phase_angle:
        return "phase_orbit";
    case ColorSignal::iteration_bands:
        return "banded_signal";
    }
    return nullptr;
}

inline bool TryParseAdvancedColorSignalFunctionId(const std::string& functionId, ColorSignal* outValue) {
    if (functionId == "smooth_escape_ramp") {
        if (outValue) *outValue = ColorSignal::smooth_escape;
        return true;
    }
    if (functionId == "phase_orbit") {
        if (outValue) *outValue = ColorSignal::phase_angle;
        return true;
    }
    if (functionId == "banded_signal") {
        if (outValue) *outValue = ColorSignal::iteration_bands;
        return true;
    }
    return false;
}

inline const char* AdvancedColorPaletteFunctionId(ColorPalette value) {
    switch (value) {
    case ColorPalette::cyclic_escape:
        return "heatmap";
    case ColorPalette::phase_wheel:
        return "phase_wheel_palette";
    case ColorPalette::banded_escape:
        return "banded_heatmap";
    }
    return nullptr;
}

inline bool TryParseAdvancedColorPaletteFunctionId(const std::string& functionId, ColorPalette* outValue) {
    if (functionId == "heatmap") {
        if (outValue) *outValue = ColorPalette::cyclic_escape;
        return true;
    }
    if (functionId == "phase_wheel_palette") {
        if (outValue) *outValue = ColorPalette::phase_wheel;
        return true;
    }
    if (functionId == "banded_heatmap") {
        if (outValue) *outValue = ColorPalette::banded_escape;
        return true;
    }
    return false;
}

inline const char* AdvancedColorGradingFunctionId(ColorGradingPreset value) {
    switch (value) {
    case ColorGradingPreset::escape_default:
        return "contrast_lift";
    case ColorGradingPreset::phase_default:
        return "phase_finish";
    case ColorGradingPreset::bands_default:
        return "band_finish";
    }
    return nullptr;
}

inline bool TryParseAdvancedColorGradingFunctionId(const std::string& functionId, ColorGradingPreset* outValue) {
    if (functionId == "contrast_lift") {
        if (outValue) *outValue = ColorGradingPreset::escape_default;
        return true;
    }
    if (functionId == "phase_finish") {
        if (outValue) *outValue = ColorGradingPreset::phase_default;
        return true;
    }
    if (functionId == "band_finish") {
        if (outValue) *outValue = ColorGradingPreset::bands_default;
        return true;
    }
    return false;
}

inline const char* AdvancedColorShapeFunctionId(ColorPipelineShape value) {
    switch (value) {
    case ColorPipelineShape::identity:
        return "identity";
    case ColorPipelineShape::offset_scale:
        return "offset_scale";
    }
    return nullptr;
}

inline std::vector<FunctionDescriptor> BuildColorPipelineSignalFunctions() {
    return {
        MakeColorPipelineFunction(
            "smooth_escape_ramp",
            "Smooth Escape Ramp",
            "Use the continuous escape estimate as the upstream color signal.",
            {
                MakeColorPipelineFloatParam("signal.scale", "Scale", "Expand or compress the smooth escape ramp.", 0.25, 4.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("signal.bias", "Bias", "Shift the smooth escape ramp before palette lookup.", -1.0, 1.0, 0.01, 0.0),
            }),
        MakeColorPipelineFunction(
            "phase_orbit",
            "Phase Orbit",
            "Use orbit phase as the upstream signal.",
            {
                MakeColorPipelineFloatParam("signal.phase_offset", "Phase Offset", "Rotate the sampled phase before downstream palette work.", -3.141592653589793, 3.141592653589793, 0.01, 0.0),
                MakeColorPipelineFloatParam("signal.wrap_cycles", "Wrap Cycles", "Control how many hue cycles appear across one full rotation.", 0.5, 6.0, 0.01, 1.0),
            }),
        MakeColorPipelineFunction(
            "banded_signal",
            "Iteration Bands",
            "Quantize the escape signal into stepped iteration bands.",
            {
                MakeColorPipelineIntParam("signal.band_count", "Band Count", "Choose how many bands to carve out of the escape signal.", 2, 24, 1, 8),
                MakeColorPipelineFloatParam("signal.softness", "Softness", "Blend between hard posterization and soft band transitions.", 0.0, 1.0, 0.01, 0.35),
            }),
    };
}

inline std::vector<FunctionDescriptor> BuildColorPipelinePaletteFunctions() {
    return {
        MakeColorPipelineFunction(
            "heatmap",
            "Heatmap",
            "Wrap escape signals through the cyclic escape palette.",
            {
                MakeColorPipelineFloatParam("palette.cycle_scale", "Cycle Scale", "Control how quickly the palette repeats across the signal.", 0.25, 4.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("palette.saturation", "Saturation", "Push or soften color separation in the cyclic palette.", 0.0, 2.0, 0.01, 1.0),
            }),
        MakeColorPipelineFunction(
            "phase_wheel_palette",
            "Phase Wheel",
            "Wrap signal values around a phase wheel palette.",
            {
                MakeColorPipelineFloatParam("palette.phase_offset", "Phase Offset", "Rotate the wheel before it is applied to the incoming signal.", -3.141592653589793, 3.141592653589793, 0.01, 0.0),
                MakeColorPipelineFloatParam("palette.saturation", "Saturation", "Push or soften color separation in the wheel.", 0.0, 2.0, 0.01, 1.15),
            }),
        MakeColorPipelineFunction(
            "banded_heatmap",
            "Banded Heatmap",
            "Map banded escape signals through the runtime band palette.",
            {
                MakeColorPipelineFloatParam("palette.band_emphasis", "Band Emphasis", "Increase or relax the contrast between neighboring bands.", 0.0, 2.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("palette.phase_offset", "Phase Offset", "Offset the band palette without changing the source signal.", -3.141592653589793, 3.141592653589793, 0.01, 0.0),
            }),
    };
}

inline std::vector<FunctionDescriptor> BuildColorPipelineShapeFunctions() {
    return {
        MakeColorPipelineFunction(
            "identity",
            "Identity",
            "Keep the incoming source signal unchanged before palette materialization.",
            {}),
        MakeColorPipelineFunction(
            "offset_scale",
            "Offset + Scale",
            "Shift and scale the incoming signal before palette materialization.",
            {
                MakeColorPipelineFloatParam("shape.offset", "Offset", "Shift the incoming signal before downstream palette work.", -2.0, 2.0, 0.01, 0.0),
                MakeColorPipelineFloatParam("shape.scale", "Scale", "Expand or compress the incoming signal.", 0.1, 8.0, 0.01, 1.0),
            }),
        MakeColorPipelineFunction(
            "repeat",
            "Repeat",
            "Tile the incoming signal into repeating bands.",
            {
                MakeColorPipelineFloatParam("shape.frequency", "Frequency", "Control how often the signal repeats.", 0.25, 24.0, 0.01, 8.0),
                MakeColorPipelineFloatParam("shape.phase", "Phase", "Offset the repeated pattern without changing the source.", -1.0, 1.0, 0.01, 0.0),
            }),
        MakeColorPipelineFunction(
            "posterize",
            "Posterize",
            "Quantize the incoming signal into stepped bands.",
            {
                MakeColorPipelineIntParam("shape.steps", "Steps", "Choose how many discrete levels to keep.", 2, 24, 1, 6),
                MakeColorPipelineFloatParam("shape.mix", "Mix", "Blend between the original signal and the stepped version.", 0.0, 1.0, 0.01, 1.0),
            }),
    };
}

inline std::vector<FunctionDescriptor> BuildColorPipelineGradeFunctions() {
    return {
        MakeColorPipelineFunction(
            "contrast_lift",
            "Contrast Lift",
            "Apply the default escape-time grading profile.",
            {
                MakeColorPipelineFloatParam("grade.exposure", "Exposure", "Set the overall escape brightness.", 0.1, 3.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("grade.saturation", "Saturation", "Push or soften the escape palette intensity.", 0.0, 2.0, 0.01, 1.0),
            }),
        MakeColorPipelineFunction(
            "phase_finish",
            "Phase Finish",
            "Apply the default phase grading profile.",
            {
                MakeColorPipelineFloatParam("grade.saturation", "Saturation", "Push or soften phase-wheel intensity.", 0.0, 2.0, 0.01, 1.15),
                MakeColorPipelineFloatParam("grade.contrast", "Contrast", "Stretch the phase palette mid-tones.", 0.0, 3.0, 0.01, 1.0),
            }),
        MakeColorPipelineFunction(
            "band_finish",
            "Band Finish",
            "Apply the default banded grading profile.",
            {
                MakeColorPipelineFloatParam("grade.band_emphasis", "Band Emphasis", "Increase or relax final band contrast.", 0.0, 2.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("grade.glow", "Glow", "Add controlled highlight bloom to the final output.", 0.0, 2.0, 0.01, 0.25),
            }),
    };
}

inline bool IsColorPipelineFunctionRuntimeBacked(const char* laneId, const std::string& functionId) {
    if (!laneId || laneId[0] == '\0') {
        return false;
    }

    if (std::string(laneId) == "source") {
        return functionId == "smooth_escape_ramp" ||
            functionId == "phase_orbit" ||
            functionId == "banded_signal";
    }
    if (std::string(laneId) == "shape") {
        return functionId == "identity" ||
            functionId == "offset_scale";
    }
    if (std::string(laneId) == "palette") {
        return functionId == "heatmap" ||
            functionId == "phase_wheel_palette" ||
            functionId == "banded_heatmap";
    }
    return false;
}

inline std::vector<FunctionDescriptor> FilterRuntimeBackedColorPipelineFunctions(
    const char* laneId,
    std::vector<FunctionDescriptor> functions) {
    std::vector<FunctionDescriptor> filtered;
    filtered.reserve(functions.size());
    for (FunctionDescriptor& descriptor : functions) {
        if (IsColorPipelineFunctionRuntimeBacked(laneId, descriptor.id)) {
            filtered.push_back(std::move(descriptor));
        }
    }
    return filtered;
}

inline const std::vector<ColorPipelineLaneCatalog>& GetColorPipelineLaneCatalogs() {
    static const std::vector<ColorPipelineLaneCatalog> catalogs = {
        {"source", "Source", "smooth_escape_ramp", FilterRuntimeBackedColorPipelineFunctions("source", BuildColorPipelineSignalFunctions())},
        {"shape", "Shape", "identity", FilterRuntimeBackedColorPipelineFunctions("shape", BuildColorPipelineShapeFunctions())},
        {"palette", "Palette", "heatmap", FilterRuntimeBackedColorPipelineFunctions("palette", BuildColorPipelinePaletteFunctions())},
    };
    return catalogs;
}

inline const ColorPipelineLaneCatalog* FindColorPipelineLaneCatalog(const std::string& laneId) {
    for (const ColorPipelineLaneCatalog& catalog : GetColorPipelineLaneCatalogs()) {
        if (catalog.lane_id == laneId) {
            return &catalog;
        }
    }
    return nullptr;
}

inline const FunctionDescriptor* FindColorPipelineFunctionDescriptor(
    const ColorPipelineLaneCatalog& catalog,
    const std::string& functionId) {
    for (const FunctionDescriptor& descriptor : catalog.functions) {
        if (descriptor.id == functionId) {
            return &descriptor;
        }
    }
    return nullptr;
}

inline double ResolveColorPipelineNumericDefault(const FunctionParamDescriptor& param) {
    if (param.has_default && param.default_value.is_number()) {
        return param.default_value.as_number();
    }
    if (param.has_min && param.has_max) {
        return (param.min_value + param.max_value) * 0.5;
    }
    return 0.0;
}

inline bool ResolveColorPipelineBoolDefault(const FunctionParamDescriptor& param) {
    if (param.has_default && param.default_value.is_bool()) {
        return param.default_value.as_bool();
    }
    return false;
}

inline std::string ResolveColorPipelineEnumDefault(const FunctionParamDescriptor& param) {
    if (param.has_default && param.default_value.is_string()) {
        return param.default_value.as_string();
    }
    if (!param.options.empty()) {
        return param.options.front().id;
    }
    return {};
}

inline bool ImportSupportedColorPipelineParamsFromLive(
    ColorPipelineRowState* ioRow,
    const KernelParams& liveParams,
    std::string* outError = nullptr);

inline void ClearColorPipelineValidationMessages(ColorPipelineWindowState* ioState) {
    if (!ioState) {
        return;
    }
    ioState->validation_messages.clear();
}

inline void PushColorPipelineValidationMessage(ColorPipelineWindowState* ioState, const std::string& message) {
    if (!ioState || message.empty()) {
        return;
    }
    ioState->validation_messages.push_back(message);
}

inline bool SetColorPipelineRowFunction(
    ColorPipelineRowState* ioRow,
    const FunctionDescriptor& descriptor) {
    if (!ioRow) {
        return false;
    }

    ioRow->function_id = descriptor.id;
    ioRow->parameter_values.clear();
    ioRow->parameter_values.reserve(descriptor.parameters.size());
    for (const FunctionParamDescriptor& param : descriptor.parameters) {
        ColorPipelineParamState value;
        value.path = param.path;
        value.type = param.type;
        if (param.type == "bool") {
            value.bool_value = ResolveColorPipelineBoolDefault(param);
        } else if (param.type == "enum") {
            value.enum_value = ResolveColorPipelineEnumDefault(param);
        } else {
            value.number_value = ResolveColorPipelineNumericDefault(param);
        }
        ioRow->parameter_values.push_back(std::move(value));
    }
    return true;
}

inline bool BuildColorPipelineRowFromFunctionId(
    const ColorPipelineLaneCatalog& catalog,
    const char* functionId,
    std::uint64_t stableRowId,
    ColorPipelineRowState* outRow,
    std::string* outError = nullptr) {
    if (!outRow || !functionId || functionId[0] == '\0') {
        if (outError) *outError = "Color pipeline row builder requires a non-empty function id";
        return false;
    }

    const FunctionDescriptor* descriptor = FindColorPipelineFunctionDescriptor(catalog, functionId);
    if (!descriptor) {
        if (outError) {
            *outError = std::string("Unknown advanced color function '") + functionId + "' for lane " + catalog.label;
        }
        return false;
    }

    ColorPipelineRowState row;
    row.ui_row_id = stableRowId;
    if (!SetColorPipelineRowFunction(&row, *descriptor)) {
        if (outError) *outError = std::string("Failed to initialize advanced color function '") + functionId + "'";
        return false;
    }
    *outRow = std::move(row);
    return true;
}

inline bool BuildColorPipelineLaneWithSingleRow(
    const ColorPipelineLaneCatalog& catalog,
    const char* functionId,
    std::uint64_t stableRowId,
    ColorPipelineLaneState* outLane,
    std::string* outError = nullptr) {
    if (!outLane) {
        if (outError) *outError = "Color pipeline lane builder requires an output lane";
        return false;
    }

    ColorPipelineLaneState lane;
    lane.lane_id = catalog.lane_id;
    lane.label = catalog.label;

    ColorPipelineRowState row;
    if (!BuildColorPipelineRowFromFunctionId(catalog, functionId, stableRowId, &row, outError)) {
        return false;
    }
    lane.rows.push_back(std::move(row));
    *outLane = std::move(lane);
    return true;
}

inline void ApplyColorPipelineRowTemplate(ColorPipelineRowState* ioRow, const ColorPipelineRowState& source) {
    if (!ioRow) {
        return;
    }
    const std::uint64_t stableRowId = ioRow->ui_row_id;
    *ioRow = source;
    ioRow->ui_row_id = stableRowId;
}

inline bool EnsureColorPipelineLaneRowsInitialized(ColorPipelineLaneState* ioLane, std::uint64_t* ioNextRowId) {
    if (!ioLane || !ioNextRowId) {
        return false;
    }
    for (ColorPipelineRowState& row : ioLane->rows) {
        if (!EnsureImGuiStackEditorRowId(&row.ui_row_id, ioNextRowId)) {
            return false;
        }
    }
    return true;
}

inline bool ColorPipelineParamStatesEqual(
    const ColorPipelineParamState& left,
    const ColorPipelineParamState& right) {
    return left.path == right.path &&
        left.type == right.type &&
        std::fabs(left.number_value - right.number_value) <= 1e-6 &&
        left.bool_value == right.bool_value &&
        left.enum_value == right.enum_value;
}

inline bool ColorPipelineRowStatesEqual(
    const ColorPipelineRowState& left,
    const ColorPipelineRowState& right) {
    if (left.enabled != right.enabled ||
        left.function_id != right.function_id ||
        left.parameter_values.size() != right.parameter_values.size()) {
        return false;
    }
    for (std::size_t index = 0; index < left.parameter_values.size(); ++index) {
        if (!ColorPipelineParamStatesEqual(left.parameter_values[index], right.parameter_values[index])) {
            return false;
        }
    }
    return true;
}

inline bool ColorPipelineLaneStatesEqual(
    const ColorPipelineLaneState& left,
    const ColorPipelineLaneState& right) {
    if (left.lane_id != right.lane_id ||
        left.label != right.label ||
        left.rows.size() != right.rows.size()) {
        return false;
    }
    for (std::size_t index = 0; index < left.rows.size(); ++index) {
        if (!ColorPipelineRowStatesEqual(left.rows[index], right.rows[index])) {
            return false;
        }
    }
    return true;
}

inline bool HasColorPipelineDraftEdits(const ColorPipelineWindowState& state) {
    if (!state.live_snapshot.valid) {
        return false;
    }
    if (state.lanes.size() != state.live_snapshot.lanes.size()) {
        return true;
    }
    for (std::size_t index = 0; index < state.lanes.size(); ++index) {
        if (!ColorPipelineLaneStatesEqual(state.lanes[index], state.live_snapshot.lanes[index])) {
            return true;
        }
    }
    return false;
}

inline bool ResetColorPipelineDraftFromLiveState(ColorPipelineWindowState* ioState) {
    if (!ioState || !ioState->live_snapshot.valid) {
        return false;
    }
    if (!ioState->live_snapshot.draft_import_supported || ioState->live_snapshot.lanes.empty()) {
        PushColorPipelineValidationMessage(ioState,
            "Current live runtime tuple is outside the shipped advanced catalog; keep editing the programmable draft or switch the simple Color panel first.");
        return false;
    }
    if (ioState->lanes.size() != ioState->live_snapshot.lanes.size()) {
        ioState->lanes = ioState->live_snapshot.lanes;
    } else {
        for (std::size_t index = 0; index < ioState->lanes.size(); ++index) {
            ioState->lanes[index] = ioState->live_snapshot.lanes[index];
        }
    }
    for (ColorPipelineLaneState& lane : ioState->lanes) {
        if (!EnsureColorPipelineLaneRowsInitialized(&lane, &ioState->next_row_id)) {
            return false;
        }
    }
    return true;
}

inline bool TryBuildColorPipelineScheduleBridgeIds(
    const ColorPipelineSelection& pipeline,
    const char** outSourceFunctionId,
    const char** outPaletteFunctionId) {
    if (outSourceFunctionId) {
        *outSourceFunctionId = nullptr;
    }
    if (outPaletteFunctionId) {
        *outPaletteFunctionId = nullptr;
    }

    if (pipeline.signal == ColorSignal::smooth_escape &&
        pipeline.palette == ColorPalette::cyclic_escape &&
        pipeline.grading == ColorGradingPreset::escape_default) {
        if (outSourceFunctionId) *outSourceFunctionId = "smooth_escape_ramp";
        if (outPaletteFunctionId) *outPaletteFunctionId = "heatmap";
        return true;
    }
    if (pipeline.signal == ColorSignal::phase_angle &&
        pipeline.palette == ColorPalette::phase_wheel &&
        pipeline.grading == ColorGradingPreset::phase_default) {
        if (outSourceFunctionId) *outSourceFunctionId = "phase_orbit";
        if (outPaletteFunctionId) *outPaletteFunctionId = "phase_wheel_palette";
        return true;
    }
    if (pipeline.signal == ColorSignal::iteration_bands &&
        pipeline.palette == ColorPalette::banded_escape &&
        pipeline.grading == ColorGradingPreset::bands_default) {
        if (outSourceFunctionId) *outSourceFunctionId = "banded_signal";
        if (outPaletteFunctionId) *outPaletteFunctionId = "banded_heatmap";
        return true;
    }
    return false;
}

inline bool TryBuildColorPipelineLiveSnapshot(
    FractalType liveFractalType,
    const KernelParams& liveParams,
    ColorPipelineLiveSnapshot* outSnapshot,
    std::string* outError = nullptr) {
    if (!outSnapshot) {
        if (outError) *outError = "Advanced color pipeline live snapshot requires an output struct";
        return false;
    }

    ColoringMode inferredMode = ColoringMode::root_basin;
    if (!TryLegacyColoringModeForPipeline(liveParams.color_pipeline, &inferredMode)) {
        if (outError) *outError = "Live runtime color pipeline does not map to an exact supported legacy mode";
        return false;
    }
    if (!IsColorPipelineAllowedForFractal(liveFractalType, liveParams.color_pipeline)) {
        if (outError) *outError = "Live runtime color pipeline is not allowed for the current fractal family";
        return false;
    }
    if (liveParams.coloring_mode != inferredMode) {
        if (outError) *outError = "Live coloring_mode and live color_pipeline are out of sync";
        return false;
    }

    const char* sourceFunctionId = nullptr;
    const char* shapeFunctionId = AdvancedColorShapeFunctionId(liveParams.color_shape);
    const char* paletteFunctionId = nullptr;
    ColorPipelineLiveSnapshot snapshot;
    snapshot.valid = true;
    snapshot.fractal_type = liveFractalType;
    snapshot.coloring_mode = liveParams.coloring_mode;
    snapshot.pipeline = liveParams.color_pipeline;
    snapshot.draft_import_supported = TryBuildColorPipelineScheduleBridgeIds(
        liveParams.color_pipeline,
        &sourceFunctionId,
        &paletteFunctionId);
    if (!shapeFunctionId) {
        snapshot.draft_import_supported = false;
    }
    if (!snapshot.draft_import_supported) {
        *outSnapshot = std::move(snapshot);
        return true;
    }
    const std::vector<ColorPipelineLaneCatalog>& catalogs = GetColorPipelineLaneCatalogs();
    snapshot.lanes.reserve(catalogs.size());
    const char* laneFunctionIds[] = {
        sourceFunctionId,
        shapeFunctionId,
        paletteFunctionId,
    };
    for (std::size_t index = 0; index < catalogs.size(); ++index) {
        ColorPipelineLaneState lane;
        if (!BuildColorPipelineLaneWithSingleRow(catalogs[index], laneFunctionIds[index], 0, &lane, outError)) {
            return false;
        }
        if (!lane.rows.empty() && !ImportSupportedColorPipelineParamsFromLive(&lane.rows.front(), liveParams, outError)) {
            return false;
        }
        snapshot.lanes.push_back(std::move(lane));
    }
    *outSnapshot = std::move(snapshot);
    return true;
}

inline bool EnsureColorPipelineWindowInitialized(ColorPipelineWindowState* ioState) {
    if (!ioState) {
        return false;
    }

    if (ioState->initialized) {
        for (ColorPipelineLaneState& lane : ioState->lanes) {
            if (!EnsureColorPipelineLaneRowsInitialized(&lane, &ioState->next_row_id)) {
                return false;
            }
        }
        return true;
    }

    ioState->lanes.clear();
    ClearColorPipelineValidationMessages(ioState);
    for (const ColorPipelineLaneCatalog& catalog : GetColorPipelineLaneCatalogs()) {
        ColorPipelineLaneState lane;
        if (!BuildColorPipelineLaneWithSingleRow(catalog, catalog.default_function_id, 0, &lane)) {
            PushColorPipelineValidationMessage(ioState,
                std::string("Missing default advanced color function for lane: ") + catalog.label);
            return false;
        }
        if (!EnsureColorPipelineLaneRowsInitialized(&lane, &ioState->next_row_id)) {
            return false;
        }
        ioState->lanes.push_back(std::move(lane));
    }
    ioState->initialized = true;
    return true;
}

inline bool IsColorPipelineStarterDraft(const ColorPipelineWindowState& state) {
    const std::vector<ColorPipelineLaneCatalog>& catalogs = GetColorPipelineLaneCatalogs();
    if (state.lanes.size() != catalogs.size()) {
        return false;
    }

    for (std::size_t index = 0; index < catalogs.size(); ++index) {
        ColorPipelineLaneState expectedLane;
        if (!BuildColorPipelineLaneWithSingleRow(catalogs[index], catalogs[index].default_function_id, 0, &expectedLane)) {
            return false;
        }
        if (!ColorPipelineLaneStatesEqual(state.lanes[index], expectedLane)) {
            return false;
        }
    }
    return true;
}

inline bool SyncColorPipelineWindowFromLiveState(
    ColorPipelineWindowState* ioState,
    FractalType liveFractalType,
    const KernelParams* liveParams) {
    if (!ioState || !liveParams) {
        return false;
    }
    if (!EnsureColorPipelineWindowInitialized(ioState)) {
        return false;
    }

    const bool liveSnapshotWasValid = ioState->live_snapshot.valid;
    const bool draftHasEdits = HasColorPipelineDraftEdits(*ioState);
    const bool draftMatchesStarter = IsColorPipelineStarterDraft(*ioState);
    const bool adoptIntoDraft =
        !liveSnapshotWasValid ||
        !draftHasEdits ||
        (!ioState->live_snapshot.draft_import_supported && draftMatchesStarter);
    ColorPipelineLiveSnapshot nextSnapshot;
    std::string error;
    if (!TryBuildColorPipelineLiveSnapshot(liveFractalType, *liveParams, &nextSnapshot, &error)) {
        ioState->live_snapshot = {};
        PushColorPipelineValidationMessage(ioState, error);
        return false;
    }

    ioState->live_snapshot = std::move(nextSnapshot);
    if (adoptIntoDraft && ioState->live_snapshot.draft_import_supported) {
        return ResetColorPipelineDraftFromLiveState(ioState);
    }
    return true;
}

inline bool SelectColorPipelineRowFunction(
    ColorPipelineWindowState* ioState,
    std::size_t laneIndex,
    std::size_t rowIndex,
    const char* functionId) {
    if (!ioState || !functionId || functionId[0] == '\0') {
        return false;
    }
    if (!EnsureColorPipelineWindowInitialized(ioState)) {
        return false;
    }
    if (laneIndex >= ioState->lanes.size()) {
        PushColorPipelineValidationMessage(ioState, "Advanced color pipeline lane index was out of range.");
        return false;
    }

    ColorPipelineLaneState& lane = ioState->lanes[laneIndex];
    if (rowIndex >= lane.rows.size()) {
        PushColorPipelineValidationMessage(ioState, "Advanced color pipeline row index was out of range.");
        return false;
    }
    const ColorPipelineLaneCatalog* catalog = FindColorPipelineLaneCatalog(lane.lane_id);
    if (!catalog) {
        PushColorPipelineValidationMessage(ioState,
            std::string("Unknown advanced color pipeline lane id: ") + lane.lane_id);
        return false;
    }

    const FunctionDescriptor* descriptor = FindColorPipelineFunctionDescriptor(*catalog, functionId);
    if (!descriptor) {
        PushColorPipelineValidationMessage(ioState,
            std::string("Unknown advanced color function '") + functionId + "' for lane " + lane.label);
        return false;
    }

    return SetColorPipelineRowFunction(&lane.rows[rowIndex], *descriptor);
}

inline bool SelectColorPipelineLaneFunction(
    ColorPipelineWindowState* ioState,
    std::size_t laneIndex,
    const char* functionId) {
    return SelectColorPipelineRowFunction(ioState, laneIndex, 0, functionId);
}

inline bool AddColorPipelineLaneRow(
    ColorPipelineWindowState* ioState,
    std::size_t laneIndex,
    const char* functionId = nullptr) {
    if (!ioState || laneIndex >= ioState->lanes.size()) {
        return false;
    }
    if (!EnsureColorPipelineWindowInitialized(ioState)) {
        return false;
    }

    ColorPipelineLaneState& lane = ioState->lanes[laneIndex];
    const ColorPipelineLaneCatalog* catalog = FindColorPipelineLaneCatalog(lane.lane_id);
    if (!catalog) {
        PushColorPipelineValidationMessage(ioState,
            std::string("Unknown advanced color pipeline lane id: ") + lane.lane_id);
        return false;
    }

    const char* nextFunctionId = (functionId && functionId[0] != '\0') ? functionId : catalog->default_function_id;
    ColorPipelineRowState row;
    std::string error;
    if (!BuildColorPipelineRowFromFunctionId(*catalog, nextFunctionId, 0, &row, &error)) {
        PushColorPipelineValidationMessage(ioState, error);
        return false;
    }
    if (!EnsureImGuiStackEditorRowId(&row.ui_row_id, &ioState->next_row_id)) {
        return false;
    }
    lane.rows.push_back(std::move(row));
    return true;
}

inline bool MoveColorPipelineLaneRow(
    ColorPipelineWindowState* ioState,
    std::size_t laneIndex,
    std::size_t rowIndex,
    int direction) {
    if (!ioState || laneIndex >= ioState->lanes.size() || direction == 0) {
        return false;
    }
    ColorPipelineLaneState& lane = ioState->lanes[laneIndex];
    if (rowIndex >= lane.rows.size()) {
        return false;
    }
    if (direction < 0) {
        if (rowIndex == 0) {
            return false;
        }
        std::swap(lane.rows[rowIndex], lane.rows[rowIndex - 1]);
        return true;
    }
    if (rowIndex + 1 >= lane.rows.size()) {
        return false;
    }
    std::swap(lane.rows[rowIndex], lane.rows[rowIndex + 1]);
    return true;
}

inline bool RemoveColorPipelineLaneRow(
    ColorPipelineWindowState* ioState,
    std::size_t laneIndex,
    std::size_t rowIndex) {
    if (!ioState || laneIndex >= ioState->lanes.size()) {
        return false;
    }
    ColorPipelineLaneState& lane = ioState->lanes[laneIndex];
    if (lane.rows.size() <= 1 || rowIndex >= lane.rows.size()) {
        return false;
    }
    lane.rows.erase(lane.rows.begin() + static_cast<std::ptrdiff_t>(rowIndex));
    return true;
}

inline bool IsLiveColorPipelineParamPath(const std::string& functionId, const std::string& path) {
    if (functionId == "smooth_escape_ramp") {
        return path == "signal.scale" || path == "signal.bias";
    }
    if (functionId == "heatmap") {
        return path == "palette.cycle_scale" || path == "palette.saturation";
    }
    if (functionId == "contrast_lift") {
        return path == "grade.exposure" || path == "grade.saturation";
    }
    if (functionId == "phase_orbit") {
        return path == "signal.phase_offset" || path == "signal.wrap_cycles";
    }
    if (functionId == "phase_wheel_palette") {
        return path == "palette.phase_offset";
    }
    if (functionId == "offset_scale") {
        return path == "shape.offset" || path == "shape.scale";
    }
    if (functionId == "banded_signal") {
        return path == "signal.band_count" || path == "signal.softness";
    }
    if (functionId == "banded_heatmap") {
        return path == "palette.band_emphasis" || path == "palette.phase_offset";
    }
    return false;
}

inline bool CollectRenderableColorPipelineParamIndexes(
    const ColorPipelineRowState& row,
    std::vector<std::size_t>* outIndexes,
    bool* outHasHiddenParams = nullptr) {
    if (!outIndexes) {
        return false;
    }
    outIndexes->clear();
    bool hasHiddenParams = false;
    for (std::size_t index = 0; index < row.parameter_values.size(); ++index) {
        const bool liveParam = IsLiveColorPipelineParamPath(
            row.function_id,
            row.parameter_values[index].path);
        if (liveParam) {
            outIndexes->push_back(index);
        } else {
            hasHiddenParams = true;
        }
    }
    if (outHasHiddenParams) {
        *outHasHiddenParams = hasHiddenParams;
    }
    return true;
}

inline bool TryGetColorPipelineParamNumber(
    const ColorPipelineRowState& row,
    const char* path,
    double* outValue,
    std::string* outError = nullptr) {
    if (!path || !outValue) {
        if (outError) *outError = "Advanced color parameter lookup requires a path and output storage";
        return false;
    }
    for (const ColorPipelineParamState& param : row.parameter_values) {
        if (param.path == path) {
            *outValue = param.number_value;
            return true;
        }
    }
    if (outError) *outError = std::string("Missing advanced color parameter path '") + path + "' for function '" + row.function_id + "'";
    return false;
}

inline bool SetColorPipelineParamNumber(
    ColorPipelineRowState* ioRow,
    const char* path,
    double value,
    std::string* outError = nullptr) {
    if (!ioRow || !path) {
        if (outError) *outError = "Advanced color parameter import requires a row and parameter path";
        return false;
    }
    for (ColorPipelineParamState& param : ioRow->parameter_values) {
        if (param.path == path) {
            param.number_value = value;
            return true;
        }
    }
    if (outError) *outError = std::string("Missing advanced color parameter path '") + path + "' for function '" + ioRow->function_id + "'";
    return false;
}

inline bool ImportSupportedColorPipelineParamsFromLive(
    ColorPipelineRowState* ioRow,
    const KernelParams& liveParams,
    std::string* outError) {
    if (!ioRow) {
        if (outError) *outError = "Advanced color parameter import requires a row";
        return false;
    }
    if (ioRow->function_id == "smooth_escape_ramp") {
        return SetColorPipelineParamNumber(ioRow, "signal.scale", liveParams.color_smooth_escape_scale, outError) &&
            SetColorPipelineParamNumber(ioRow, "signal.bias", liveParams.color_smooth_escape_bias, outError);
    }
    if (ioRow->function_id == "heatmap") {
        return SetColorPipelineParamNumber(ioRow, "palette.cycle_scale", liveParams.color_heatmap_cycle_scale, outError) &&
            SetColorPipelineParamNumber(ioRow, "palette.saturation", liveParams.color_heatmap_saturation, outError);
    }
    if (ioRow->function_id == "contrast_lift") {
        return SetColorPipelineParamNumber(ioRow, "grade.exposure", liveParams.color_contrast_lift_exposure, outError) &&
            SetColorPipelineParamNumber(ioRow, "grade.saturation", liveParams.color_contrast_lift_saturation, outError);
    }
    if (ioRow->function_id == "phase_orbit") {
        return SetColorPipelineParamNumber(ioRow, "signal.phase_offset", liveParams.color_phase_signal_offset, outError) &&
            SetColorPipelineParamNumber(ioRow, "signal.wrap_cycles", liveParams.color_phase_wrap_cycles, outError);
    }
    if (ioRow->function_id == "phase_wheel_palette") {
        return SetColorPipelineParamNumber(ioRow, "palette.phase_offset", liveParams.color_phase_palette_offset, outError);
    }
    if (ioRow->function_id == "offset_scale") {
        return SetColorPipelineParamNumber(ioRow, "shape.offset", liveParams.color_shape_offset, outError) &&
            SetColorPipelineParamNumber(ioRow, "shape.scale", liveParams.color_shape_scale, outError);
    }
    if (ioRow->function_id == "banded_signal") {
        return SetColorPipelineParamNumber(ioRow, "signal.band_count", static_cast<double>(liveParams.color_iteration_band_count), outError) &&
            SetColorPipelineParamNumber(ioRow, "signal.softness", liveParams.color_iteration_band_softness, outError);
    }
    if (ioRow->function_id == "banded_heatmap") {
        return SetColorPipelineParamNumber(ioRow, "palette.band_emphasis", liveParams.color_iteration_band_emphasis, outError) &&
            SetColorPipelineParamNumber(ioRow, "palette.phase_offset", liveParams.color_iteration_band_palette_offset, outError);
    }
    return true;
}

inline bool ValidateColorPipelineParamRange(
    const char* path,
    double value,
    double minValue,
    double maxValue,
    std::string* outError = nullptr) {
    if (value < minValue || value > maxValue) {
        if (outError) {
            *outError = std::string("Advanced color parameter '") + path + "' is outside its supported range";
        }
        return false;
    }
    return true;
}

inline bool TryReadColorPipelineIntegerParam(
    const ColorPipelineRowState& row,
    const char* path,
    int* outValue,
    std::string* outError = nullptr) {
    double value = 0.0;
    if (!TryGetColorPipelineParamNumber(row, path, &value, outError)) {
        return false;
    }
    const double rounded = std::round(value);
    if (std::fabs(value - rounded) > 1.0e-6) {
        if (outError) *outError = std::string("Advanced color parameter '") + path + "' must be an integer";
        return false;
    }
    *outValue = static_cast<int>(rounded);
    return true;
}

inline bool ApplySupportedColorPipelineParamsToLive(
    const ColorPipelineWindowState& state,
    KernelParams* ioParams,
    bool* outChanged = nullptr,
    std::string* outError = nullptr) {
    if (outChanged) {
        *outChanged = false;
    }
    if (!ioParams) {
        if (outError) *outError = "Advanced color parameter apply requires live KernelParams";
        return false;
    }

    bool changed = false;
    for (const ColorPipelineLaneState& lane : state.lanes) {
        for (const ColorPipelineRowState& row : lane.rows) {
        if (!row.enabled) {
            continue;
        }
        if (row.function_id == "smooth_escape_ramp") {
            double scale = 0.0;
            double bias = 0.0;
            if (!TryGetColorPipelineParamNumber(row, "signal.scale", &scale, outError) ||
                !TryGetColorPipelineParamNumber(row, "signal.bias", &bias, outError) ||
                !ValidateColorPipelineParamRange("signal.scale", scale, 0.25, 4.0, outError) ||
                !ValidateColorPipelineParamRange("signal.bias", bias, -1.0, 1.0, outError)) {
                return false;
            }
            if (std::fabs(ioParams->color_smooth_escape_scale - static_cast<float>(scale)) > 1.0e-6f) {
                ioParams->color_smooth_escape_scale = static_cast<float>(scale);
                changed = true;
            }
            if (std::fabs(ioParams->color_smooth_escape_bias - static_cast<float>(bias)) > 1.0e-6f) {
                ioParams->color_smooth_escape_bias = static_cast<float>(bias);
                changed = true;
            }
            continue;
        }
        if (row.function_id == "heatmap") {
            double cycleScale = 0.0;
            double saturation = 0.0;
            if (!TryGetColorPipelineParamNumber(row, "palette.cycle_scale", &cycleScale, outError) ||
                !TryGetColorPipelineParamNumber(row, "palette.saturation", &saturation, outError) ||
                !ValidateColorPipelineParamRange("palette.cycle_scale", cycleScale, 0.25, 4.0, outError) ||
                !ValidateColorPipelineParamRange("palette.saturation", saturation, 0.0, 2.0, outError)) {
                return false;
            }
            if (std::fabs(ioParams->color_heatmap_cycle_scale - static_cast<float>(cycleScale)) > 1.0e-6f) {
                ioParams->color_heatmap_cycle_scale = static_cast<float>(cycleScale);
                changed = true;
            }
            if (std::fabs(ioParams->color_heatmap_saturation - static_cast<float>(saturation)) > 1.0e-6f) {
                ioParams->color_heatmap_saturation = static_cast<float>(saturation);
                changed = true;
            }
            continue;
        }
        if (row.function_id == "contrast_lift") {
            double exposure = 0.0;
            double saturation = 0.0;
            if (!TryGetColorPipelineParamNumber(row, "grade.exposure", &exposure, outError) ||
                !TryGetColorPipelineParamNumber(row, "grade.saturation", &saturation, outError) ||
                !ValidateColorPipelineParamRange("grade.exposure", exposure, 0.1, 3.0, outError) ||
                !ValidateColorPipelineParamRange("grade.saturation", saturation, 0.0, 2.0, outError)) {
                return false;
            }
            if (std::fabs(ioParams->color_contrast_lift_exposure - static_cast<float>(exposure)) > 1.0e-6f) {
                ioParams->color_contrast_lift_exposure = static_cast<float>(exposure);
                changed = true;
            }
            if (std::fabs(ioParams->color_contrast_lift_saturation - static_cast<float>(saturation)) > 1.0e-6f) {
                ioParams->color_contrast_lift_saturation = static_cast<float>(saturation);
                changed = true;
            }
            continue;
        }
        if (row.function_id == "phase_orbit") {
            double phaseOffset = 0.0;
            double wrapCycles = 0.0;
            if (!TryGetColorPipelineParamNumber(row, "signal.phase_offset", &phaseOffset, outError) ||
                !TryGetColorPipelineParamNumber(row, "signal.wrap_cycles", &wrapCycles, outError) ||
                !ValidateColorPipelineParamRange("signal.phase_offset", phaseOffset, -3.141592653589793, 3.141592653589793, outError) ||
                !ValidateColorPipelineParamRange("signal.wrap_cycles", wrapCycles, 0.5, 6.0, outError)) {
                return false;
            }
            if (std::fabs(ioParams->color_phase_signal_offset - static_cast<float>(phaseOffset)) > 1.0e-6f) {
                ioParams->color_phase_signal_offset = static_cast<float>(phaseOffset);
                changed = true;
            }
            if (std::fabs(ioParams->color_phase_wrap_cycles - static_cast<float>(wrapCycles)) > 1.0e-6f) {
                ioParams->color_phase_wrap_cycles = static_cast<float>(wrapCycles);
                changed = true;
            }
            continue;
        }
        if (row.function_id == "phase_wheel_palette") {
            double paletteOffset = 0.0;
            if (!TryGetColorPipelineParamNumber(row, "palette.phase_offset", &paletteOffset, outError) ||
                !ValidateColorPipelineParamRange("palette.phase_offset", paletteOffset, -3.141592653589793, 3.141592653589793, outError)) {
                return false;
            }
            if (std::fabs(ioParams->color_phase_palette_offset - static_cast<float>(paletteOffset)) > 1.0e-6f) {
                ioParams->color_phase_palette_offset = static_cast<float>(paletteOffset);
                changed = true;
            }
            continue;
        }
        if (row.function_id == "identity") {
            if (ioParams->color_shape != ColorPipelineShape::identity) {
                ioParams->color_shape = ColorPipelineShape::identity;
                changed = true;
            }
            if (std::fabs(ioParams->color_shape_offset) > 1.0e-6f) {
                ioParams->color_shape_offset = 0.0f;
                changed = true;
            }
            if (std::fabs(ioParams->color_shape_scale - 1.0f) > 1.0e-6f) {
                ioParams->color_shape_scale = 1.0f;
                changed = true;
            }
            continue;
        }
        if (row.function_id == "offset_scale") {
            double offset = 0.0;
            double scale = 0.0;
            if (!TryGetColorPipelineParamNumber(row, "shape.offset", &offset, outError) ||
                !TryGetColorPipelineParamNumber(row, "shape.scale", &scale, outError) ||
                !ValidateColorPipelineParamRange("shape.offset", offset, -2.0, 2.0, outError) ||
                !ValidateColorPipelineParamRange("shape.scale", scale, 0.1, 8.0, outError)) {
                return false;
            }
            if (ioParams->color_shape != ColorPipelineShape::offset_scale) {
                ioParams->color_shape = ColorPipelineShape::offset_scale;
                changed = true;
            }
            if (std::fabs(ioParams->color_shape_offset - static_cast<float>(offset)) > 1.0e-6f) {
                ioParams->color_shape_offset = static_cast<float>(offset);
                changed = true;
            }
            if (std::fabs(ioParams->color_shape_scale - static_cast<float>(scale)) > 1.0e-6f) {
                ioParams->color_shape_scale = static_cast<float>(scale);
                changed = true;
            }
            continue;
        }
        if (row.function_id == "banded_signal") {
            int bandCount = 0;
            double softness = 0.0;
            if (!TryReadColorPipelineIntegerParam(row, "signal.band_count", &bandCount, outError) ||
                !TryGetColorPipelineParamNumber(row, "signal.softness", &softness, outError) ||
                !ValidateColorPipelineParamRange("signal.band_count", static_cast<double>(bandCount), 2.0, 24.0, outError) ||
                !ValidateColorPipelineParamRange("signal.softness", softness, 0.0, 1.0, outError)) {
                return false;
            }
            if (ioParams->color_iteration_band_count != bandCount) {
                ioParams->color_iteration_band_count = bandCount;
                changed = true;
            }
            if (std::fabs(ioParams->color_iteration_band_softness - static_cast<float>(softness)) > 1.0e-6f) {
                ioParams->color_iteration_band_softness = static_cast<float>(softness);
                changed = true;
            }
            continue;
        }
        if (row.function_id == "banded_heatmap") {
            double emphasis = 0.0;
            double paletteOffset = 0.0;
            if (!TryGetColorPipelineParamNumber(row, "palette.band_emphasis", &emphasis, outError) ||
                !TryGetColorPipelineParamNumber(row, "palette.phase_offset", &paletteOffset, outError) ||
                !ValidateColorPipelineParamRange("palette.band_emphasis", emphasis, 0.0, 2.0, outError) ||
                !ValidateColorPipelineParamRange("palette.phase_offset", paletteOffset, -3.141592653589793, 3.141592653589793, outError)) {
                return false;
            }
            if (std::fabs(ioParams->color_iteration_band_emphasis - static_cast<float>(emphasis)) > 1.0e-6f) {
                ioParams->color_iteration_band_emphasis = static_cast<float>(emphasis);
                changed = true;
            }
            if (std::fabs(ioParams->color_iteration_band_palette_offset - static_cast<float>(paletteOffset)) > 1.0e-6f) {
                ioParams->color_iteration_band_palette_offset = static_cast<float>(paletteOffset);
                changed = true;
            }
            continue;
        }
        }
    }

    if (outChanged) {
        *outChanged = changed;
    }
    return true;
}

inline const ColorPipelineLaneState* FindColorPipelineLaneState(
    const ColorPipelineWindowState& state,
    const char* laneId) {
    if (!laneId || laneId[0] == '\0') {
        return nullptr;
    }
    for (const ColorPipelineLaneState& lane : state.lanes) {
        if (lane.lane_id == laneId) {
            return &lane;
        }
    }
    return nullptr;
}

inline const ColorPipelineRowState* FindSingleEnabledColorPipelineRow(
    const ColorPipelineWindowState& state,
    const char* laneId,
    std::string* outError = nullptr) {
    const ColorPipelineLaneState* lane = FindColorPipelineLaneState(state, laneId);
    if (!lane) {
        if (outError) *outError = std::string("Unknown advanced color pipeline lane id: ") + (laneId ? laneId : "");
        return nullptr;
    }

    const ColorPipelineRowState* singleRow = nullptr;
    for (const ColorPipelineRowState& row : lane->rows) {
        if (!row.enabled) {
            continue;
        }
        if (singleRow) {
            if (outError) {
                *outError = std::string("Current live bridge only supports one enabled row in the ") + lane->label + " lane.";
            }
            return nullptr;
        }
        singleRow = &row;
    }
    if (!singleRow) {
        if (outError) {
            *outError = std::string("Current live bridge requires one enabled row in the ") + lane->label + " lane.";
        }
        return nullptr;
    }
    return singleRow;
}

inline bool TryBuildColorPipelineSelectionFromDraft(
    const ColorPipelineWindowState& state,
    ColorPipelineSelection* outPipeline,
    ColoringMode* outMode,
    std::string* outError = nullptr) {
    if (!outPipeline || !outMode) {
        if (outError) *outError = "Advanced color pipeline apply requires output storage";
        return false;
    }

    const ColorPipelineRowState* sourceRow = FindSingleEnabledColorPipelineRow(state, "source", outError);
    if (!sourceRow) {
        return false;
    }
    const ColorPipelineRowState* shapeRow = FindSingleEnabledColorPipelineRow(state, "shape", outError);
    if (!shapeRow) {
        return false;
    }
    const ColorPipelineRowState* paletteRow = FindSingleEnabledColorPipelineRow(state, "palette", outError);
    if (!paletteRow) {
        return false;
    }

    if (shapeRow->function_id != "identity" && shapeRow->function_id != "offset_scale") {
        if (outError) {
            *outError = "Current live bridge only supports the Identity and Offset + Scale Shape rows; stacked or remapped Shape recipes stay draft-only until custom runtime integration lands.";
        }
        return false;
    }

    ColorPipelineSelection pipeline{};
    ColoringMode mode = ColoringMode::root_basin;
    if (sourceRow->function_id == "smooth_escape_ramp" && paletteRow->function_id == "heatmap") {
        pipeline = {ColorSignal::smooth_escape, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
        mode = ColoringMode::smooth_escape;
    } else if (sourceRow->function_id == "phase_orbit" && paletteRow->function_id == "phase_wheel_palette") {
        pipeline = {ColorSignal::phase_angle, ColorPalette::phase_wheel, ColorGradingPreset::phase_default};
        mode = ColoringMode::phase;
    } else if (sourceRow->function_id == "banded_signal" && paletteRow->function_id == "banded_heatmap") {
        pipeline = {ColorSignal::iteration_bands, ColorPalette::banded_escape, ColorGradingPreset::bands_default};
        mode = ColoringMode::iteration_bands;
    } else {
        if (outError) {
            *outError = "Selected Source / Shape / Palette recipe is draft-only until custom pipeline runtime integration lands.";
        }
        return false;
    }

    *outPipeline = pipeline;
    *outMode = mode;
    return true;
}

inline ColorPipelineDraftApplyState DescribeColorPipelineDraftApplyState(
    const ColorPipelineWindowState& state,
    FractalType liveFractalType,
    const KernelParams* liveParams = nullptr) {
    if (!liveParams && !state.live_snapshot.valid) {
        return {
            ColorPipelineDraftApplyStatus::live_unavailable,
            "Live runtime selection is not available yet.",
        };
    }

    if (!liveParams && !HasColorPipelineDraftEdits(state)) {
        return {
            ColorPipelineDraftApplyStatus::matches_live,
            "Draft matches the live runtime selection.",
        };
    }

    ColorPipelineSelection nextPipeline{};
    ColoringMode nextMode = ColoringMode::root_basin;
    std::string error;
    if (!TryBuildColorPipelineSelectionFromDraft(state, &nextPipeline, &nextMode, &error)) {
        return {
            ColorPipelineDraftApplyStatus::unsupported_tuple,
            error.empty()
                ? "Selected lane mix is preview-only in this slice; only exact supported live tuples can apply."
                : error,
        };
    }
    if (!IsColorPipelineAllowedForFractal(liveFractalType, nextPipeline)) {
        return {
            ColorPipelineDraftApplyStatus::disallowed_for_family,
            "Selected advanced color tuple is not allowed for the current fractal family.",
        };
    }
    if (liveParams) {
        KernelParams probe = *liveParams;
        bool paramChanged = false;
        if (!ApplySupportedColorPipelineParamsToLive(state, &probe, &paramChanged, &error)) {
            return {
                ColorPipelineDraftApplyStatus::invalid_params,
                error,
            };
        }

        const bool tupleChanged =
            liveParams->coloring_mode != nextMode ||
            liveParams->color_pipeline.signal != nextPipeline.signal ||
            liveParams->color_pipeline.palette != nextPipeline.palette ||
            liveParams->color_pipeline.grading != nextPipeline.grading ||
            paramChanged;
        if (!tupleChanged) {
            return {
                ColorPipelineDraftApplyStatus::matches_live,
                "Draft matches the live runtime selection.",
            };
        }

        if (!state.live_snapshot.valid) {
            return {
                ColorPipelineDraftApplyStatus::can_apply,
                "Current live runtime selection is invalid or out of sync; the supported draft can repair it.",
            };
        }
    }
    return {
        ColorPipelineDraftApplyStatus::can_apply,
        "Draft diverges from the live runtime selection.",
    };
}

inline ColorPipelineDraftApplyState DescribeColorPipelineCandidateApplyState(
    const ColorPipelineWindowState& state,
    std::size_t laneIndex,
    std::size_t rowIndex,
    const char* functionId,
    FractalType liveFractalType,
    const KernelParams* liveParams = nullptr) {
    if (!functionId || functionId[0] == '\0' || laneIndex >= state.lanes.size()) {
        return {
            ColorPipelineDraftApplyStatus::unsupported_tuple,
            "Unsupported advanced color candidate.",
        };
    }
    ColorPipelineWindowState probeState = state;
    if (!SelectColorPipelineRowFunction(&probeState, laneIndex, rowIndex, functionId)) {
        return {
            ColorPipelineDraftApplyStatus::unsupported_tuple,
            "Unsupported advanced color candidate.",
        };
    }
    return DescribeColorPipelineDraftApplyState(probeState, liveFractalType, liveParams);
}

inline ColorPipelineDraftApplyState DescribeColorPipelineCandidateApplyState(
    const ColorPipelineWindowState& state,
    std::size_t laneIndex,
    const char* functionId,
    FractalType liveFractalType,
    const KernelParams* liveParams = nullptr) {
    return DescribeColorPipelineCandidateApplyState(state, laneIndex, 0, functionId, liveFractalType, liveParams);
}

inline bool ApplyColorPipelineDraftToLiveState(
    ColorPipelineWindowState* ioState,
    FractalType liveFractalType,
    KernelParams* ioParams,
    bool* outChanged = nullptr) {
    if (outChanged) {
        *outChanged = false;
    }
    if (!ioState || !ioParams) {
        return false;
    }
    if (!EnsureColorPipelineWindowInitialized(ioState)) {
        return false;
    }

    ColorPipelineSelection nextPipeline{};
    ColoringMode nextMode = ColoringMode::root_basin;
    std::string error;
    if (!TryBuildColorPipelineSelectionFromDraft(*ioState, &nextPipeline, &nextMode, &error)) {
        PushColorPipelineValidationMessage(ioState, error);
        return false;
    }
    if (!IsColorPipelineAllowedForFractal(liveFractalType, nextPipeline)) {
        PushColorPipelineValidationMessage(ioState,
            "Selected advanced color tuple is not allowed for the current fractal family");
        return false;
    }

    const bool changed =
        ioParams->coloring_mode != nextMode ||
        ioParams->color_pipeline.signal != nextPipeline.signal ||
        ioParams->color_pipeline.palette != nextPipeline.palette ||
        ioParams->color_pipeline.grading != nextPipeline.grading;

    bool paramChanged = false;
    if (!ApplySupportedColorPipelineParamsToLive(*ioState, ioParams, &paramChanged, &error)) {
        PushColorPipelineValidationMessage(ioState, error);
        return false;
    }

    ioParams->coloring_mode = nextMode;
    ioParams->color_pipeline = nextPipeline;
    if (!SyncColorPipelineWindowFromLiveState(ioState, liveFractalType, ioParams)) {
        return false;
    }
    if (outChanged) {
        *outChanged = changed || paramChanged;
    }
    return true;
}

inline void NoteColorPipelineCurrentItemInteraction(
    bool changed,
    ColorPipelineRenderInteractionState* ioState) {
    if (!ioState) {
        return;
    }
    if (changed || ImGui::IsItemActivated() || ImGui::IsItemActive() || ImGui::IsItemDeactivatedAfterEdit()) {
        ioState->interacted = true;
        ioState->has_active_item = ioState->has_active_item || ImGui::IsItemActive();
    }
}

inline bool ShouldAutoApplySupportedColorPipelineDraft(
    const ColorPipelineWindowState& state,
    const ColorPipelineDraftApplyState& applyState,
    const ColorPipelineRenderInteractionState& interactionState,
    const KernelParams* liveParams = nullptr) {
    (void)interactionState;
    return liveParams &&
        state.auto_apply_supported_recipe &&
        applyState.status == ColorPipelineDraftApplyStatus::can_apply;
}

inline bool RenderColorPipelineParamControl(
    const FunctionParamDescriptor& param,
    ColorPipelineParamState* ioValue,
    ColorPipelineRenderInteractionState* ioInteraction = nullptr) {
    if (!ioValue) {
        return false;
    }

    bool changed = false;
    ImGui::PushID(param.path.c_str());
    if (param.type == "float") {
        float value = static_cast<float>(ioValue->number_value);
        const NumericControlRange range = ResolveColorPipelineNumericControlRange(param);
        const NumericDragWidgetBounds dragBounds = ResolveColorPipelineNumericDragWidgetBounds(param);
        const float minValue = range.has_widget_min ? static_cast<float>(range.widget_min) : 0.0f;
        const float maxValue = range.has_widget_max ? static_cast<float>(range.widget_max) : 1.0f;
        if (range.has_widget_min && range.has_widget_max) {
            changed = ImGui::SliderFloat(param.label.c_str(), &value, minValue, maxValue, "%.5f");
        } else {
            const float step = param.has_step ? static_cast<float>(param.step_value) : 0.01f;
            const float dragMin = dragBounds.has_bounds ? static_cast<float>(dragBounds.min) : 0.0f;
            const float dragMax = dragBounds.has_bounds ? static_cast<float>(dragBounds.max) : 0.0f;
            changed = ImGui::DragFloat(param.label.c_str(), &value, step, dragMin, dragMax, "%.3f");
        }
        ImGui::SameLine();
        const bool typedChanged = ImGui::InputFloat("##value_input", &value, 0.0f, 0.0f, "%.5f");
        if (changed || typedChanged) {
            ClampColorPipelineNumericValue(&value, range);
            changed = true;
            ioValue->number_value = value;
        }
        NoteColorPipelineCurrentItemInteraction(changed, ioInteraction);
    } else if (param.type == "double") {
        double value = ioValue->number_value;
        const NumericControlRange range = ResolveColorPipelineNumericControlRange(param);
        const NumericDragWidgetBounds dragBounds = ResolveColorPipelineNumericDragWidgetBounds(param);
        const double minValue = range.has_widget_min ? range.widget_min : 0.0;
        const double maxValue = range.has_widget_max ? range.widget_max : 1.0;
        if (range.has_widget_min && range.has_widget_max) {
            changed = ImGui::SliderScalar(param.label.c_str(), ImGuiDataType_Double, &value, &minValue, &maxValue, "%.6f");
        } else {
            const double* dragMin = dragBounds.has_bounds ? &dragBounds.min : nullptr;
            const double* dragMax = dragBounds.has_bounds ? &dragBounds.max : nullptr;
            const double step = param.has_step ? param.step_value : 0.001;
            changed = ImGui::DragScalar(param.label.c_str(), ImGuiDataType_Double, &value, static_cast<float>(step), dragMin, dragMax, "%.6f");
        }
        ImGui::SameLine();
        const bool typedChanged = ImGui::InputDouble("##value_input", &value, 0.0, 0.0, "%.6f");
        if (changed || typedChanged) {
            ClampColorPipelineNumericValue(&value, range);
            changed = true;
            ioValue->number_value = value;
        }
        NoteColorPipelineCurrentItemInteraction(changed, ioInteraction);
    } else if (param.type == "int") {
        int value = static_cast<int>(std::lround(ioValue->number_value));
        const NumericControlRange range = ResolveColorPipelineNumericControlRange(param);
        const NumericDragWidgetBounds dragBounds = ResolveColorPipelineNumericDragWidgetBounds(param);
        const int minValue = range.has_widget_min ? static_cast<int>(range.widget_min) : 0;
        const int maxValue = range.has_widget_max ? static_cast<int>(range.widget_max) : 100;
        if (range.has_widget_min && range.has_widget_max) {
            changed = ImGui::SliderInt(param.label.c_str(), &value, minValue, maxValue);
        } else {
            const float step = param.has_step ? static_cast<float>(param.step_value) : 1.0f;
            const int dragMin = dragBounds.has_bounds ? static_cast<int>(dragBounds.min) : 0;
            const int dragMax = dragBounds.has_bounds ? static_cast<int>(dragBounds.max) : 0;
            changed = ImGui::DragInt(param.label.c_str(), &value, step, dragMin, dragMax);
        }
        ImGui::SameLine();
        const bool typedChanged = ImGui::InputInt("##value_input", &value, 0, 0);
        if (changed || typedChanged) {
            ClampColorPipelineNumericValue(&value, range);
            changed = true;
            ioValue->number_value = static_cast<double>(value);
        }
        NoteColorPipelineCurrentItemInteraction(changed, ioInteraction);
    } else if (param.type == "bool") {
        bool value = ioValue->bool_value;
        changed = ImGui::Checkbox(param.label.c_str(), &value);
        NoteColorPipelineCurrentItemInteraction(changed, ioInteraction);
        if (changed) {
            ioValue->bool_value = value;
        }
    } else if (param.type == "enum") {
        const char* preview = ioValue->enum_value.empty() ? "(select)" : ioValue->enum_value.c_str();
        if (ImGui::BeginCombo(param.label.c_str(), preview)) {
            for (const UISchemaOption& option : param.options) {
                const bool isSelected = (option.id == ioValue->enum_value);
                if (ImGui::Selectable(option.label.c_str(), isSelected)) {
                    ioValue->enum_value = option.id;
                    changed = true;
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        NoteColorPipelineCurrentItemInteraction(changed, ioInteraction);
    } else {
        ImGui::TextUnformatted(param.label.c_str());
        ImGui::SameLine();
        ImGui::TextDisabled("Unsupported param type: %s", param.type.c_str());
    }

    if (!param.help.empty()) {
        ImGui::TextDisabled("%s", param.help.c_str());
    }
    ImGui::PopID();
    return changed;
}

inline void OpenColorPipelineWindow(ColorPipelineWindowState* ioState) {
    if (!ioState) {
        return;
    }
    ioState->open = true;
}

inline void RenderColorPipelineWindowSummary(
    ColorPipelineWindowState* ioState,
    FractalType liveFractalType,
    KernelParams* liveParams,
    bool* ioDirty,
    ColorPipelineRenderInteractionState* ioInteraction = nullptr) {
    (void)ioDirty;
    ImGui::TextWrapped("Draft Source / Shape / Palette recipes here. The legacy Color mode and grading controls stay in the main Color panel during the schedule-editor transition.");
    ImGui::TextDisabled("This window now models three typed lane stacks instead of a fixed Signal / Palette / Grade trio.");
    ImGui::TextDisabled("Current live apply bridge supports one enabled Source row, one live-backed Shape row (Identity or Offset + Scale), and one enabled Palette row. Stacked Shape recipes remain draft-only until custom runtime integration lands.");
    ImGui::Separator();
    if (ioState && liveParams) {
        ColorPipelineDraftApplyState applyState = DescribeColorPipelineDraftApplyState(*ioState, liveFractalType, liveParams);
        if (ioState->live_snapshot.valid && ioState->live_snapshot.draft_import_supported) {
            const char* signalId = nullptr;
            const char* shapeId = "(unsupported shape)";
            const char* paletteId = nullptr;
            TryBuildColorPipelineScheduleBridgeIds(ioState->live_snapshot.pipeline, &signalId, &paletteId);
            if (ioState->live_snapshot.lanes.size() > 1 && !ioState->live_snapshot.lanes[1].rows.empty()) {
                shapeId = ioState->live_snapshot.lanes[1].rows[0].function_id.c_str();
            }
            ImGui::TextWrapped(
                "Live bridge: %s -> %s / %s / %s",
                ColoringModeId(ioState->live_snapshot.coloring_mode),
                signalId ? signalId : "(unsupported signal)",
                shapeId,
                paletteId ? paletteId : "(unsupported palette)");
        } else if (ioState->live_snapshot.valid) {
            ImGui::TextWrapped(
                "Live bridge: %s (outside the current Source / Shape / Palette bridge)",
                ColoringModeId(ioState->live_snapshot.coloring_mode));
            ImGui::TextDisabled("The schedule editor keeps its own starter draft until live runtime maps onto a supported Source / Shape / Palette bridge recipe.");
        } else {
            ImGui::TextWrapped("Live bridge: current runtime color state is invalid or out of sync with the programmable bridge.");
            ImGui::TextDisabled("The current supported draft can still repair the runtime; the live import/reset path will return once the state is coherent again.");
        }
        const bool canApply = applyState.status == ColorPipelineDraftApplyStatus::can_apply;
        if (applyState.status == ColorPipelineDraftApplyStatus::matches_live) {
            ImGui::TextDisabled("%s", applyState.message.c_str());
        } else {
            const ImVec4 statusColor = canApply
                ? ImVec4(0.95f, 0.83f, 0.40f, 1.0f)
                : ImVec4(1.0f, 0.62f, 0.48f, 1.0f);
            ImGui::TextColored(statusColor, "%s", applyState.message.c_str());
            if (!canApply) {
                ImGui::TextDisabled("Supported live bridge recipes right now: Smooth Escape + Heatmap, Phase Orbit + Phase Wheel, and Iteration Bands + Banded Heatmap with Identity or Offset + Scale in Shape.");
            }
        }
        const bool autoApplyChanged = ImGui::Checkbox("Auto-apply supported recipe", &ioState->auto_apply_supported_recipe);
        NoteColorPipelineCurrentItemInteraction(autoApplyChanged, ioInteraction);
        ImGui::SameLine();
        if (!ioState->live_snapshot.valid || !ioState->live_snapshot.draft_import_supported) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Reset Draft From Live")) {
            NoteColorPipelineCurrentItemInteraction(true, ioInteraction);
            ResetColorPipelineDraftFromLiveState(ioState);
        }
        if (!ioState->live_snapshot.valid || !ioState->live_snapshot.draft_import_supported) {
            ImGui::EndDisabled();
        }
        if (ioState->auto_apply_supported_recipe && !canApply && applyState.status != ColorPipelineDraftApplyStatus::matches_live) {
            ImGui::TextDisabled("Auto-apply stays armed and will resume as soon as the draft returns to a supported live bridge recipe.");
        }
    } else {
        ImGui::TextDisabled("Live runtime selection is not available yet.");
    }
    if (ioState && !ioState->validation_messages.empty()) {
        ImGui::Spacing();
        RenderImGuiStackEditorValidationBox("Advanced Color Pipeline", ioState->validation_messages);
        ImGui::Spacing();
    }
}

inline void RenderColorPipelineWindowLane(
    ColorPipelineWindowState* ioState,
    std::size_t laneIndex,
    FractalType liveFractalType,
    const KernelParams* liveParams,
    ColorPipelineRenderInteractionState* ioInteraction = nullptr) {
    if (!ioState || laneIndex >= ioState->lanes.size()) {
        return;
    }

    ColorPipelineLaneState& lane = ioState->lanes[laneIndex];
    const ColorPipelineLaneCatalog* catalog = FindColorPipelineLaneCatalog(lane.lane_id);
    if (!catalog) {
        PushColorPipelineValidationMessage(ioState,
            std::string("Unknown advanced color pipeline lane id: ") + lane.lane_id);
        return;
    }

    ImGui::PushID(lane.lane_id.c_str());
    ImGuiStackEditorHeaderSpec headerSpec;
    headerSpec.add_button_label = "+";
    const ImGuiStackEditorHeaderResult headerResult = RenderImGuiStackEditorHeader(headerSpec);
    ImGui::SameLine();
    ImGui::TextUnformatted(lane.label.c_str());
    if (headerResult.add_requested) {
        if (ioInteraction) {
            ioInteraction->interacted = true;
        }
        AddColorPipelineLaneRow(ioState, laneIndex, catalog->default_function_id);
    }

    for (std::size_t rowIndex = 0; rowIndex < lane.rows.size(); ++rowIndex) {
        ColorPipelineRowState& row = lane.rows[rowIndex];
        const FunctionDescriptor* descriptor = FindColorPipelineFunctionDescriptor(*catalog, row.function_id);
        const char* rowLabel = descriptor ? descriptor->name.c_str() : row.function_id.c_str();

        ImGuiStackEditorRowChromeSpec rowSpec;
        rowSpec.tree_node_id = lane.lane_id.c_str();
        rowSpec.header_label = rowLabel;
        rowSpec.stable_row_id = row.ui_row_id;
        rowSpec.enabled = &row.enabled;
        rowSpec.allow_remove = lane.rows.size() > 1;
        rowSpec.allow_move_up = rowIndex > 0;
        rowSpec.allow_move_down = rowIndex + 1 < lane.rows.size();

        const ImGuiStackEditorRowChromeResult rowResult = RenderImGuiStackEditorRowChrome(rowSpec, [&]() {
            const FunctionDescriptor* currentDescriptor = FindColorPipelineFunctionDescriptor(*catalog, row.function_id);
            const char* comboPreview = (currentDescriptor && !currentDescriptor->name.empty())
                ? currentDescriptor->name.c_str()
                : "(select)";
            if (ImGui::BeginCombo("Function", comboPreview)) {
                for (const FunctionDescriptor& candidate : catalog->functions) {
                    const bool isSelected = (candidate.id == row.function_id);
                    const ColorPipelineDraftApplyState candidateState = DescribeColorPipelineCandidateApplyState(
                        *ioState,
                        laneIndex,
                        rowIndex,
                        candidate.id.c_str(),
                        liveFractalType,
                        liveParams);
                    std::string optionLabel = candidate.name;
                    if (!isSelected && candidateState.status != ColorPipelineDraftApplyStatus::can_apply &&
                        candidateState.status != ColorPipelineDraftApplyStatus::matches_live) {
                        optionLabel += " (draft only)";
                    }
                    if (ImGui::Selectable(optionLabel.c_str(), isSelected)) {
                        if (ioInteraction) {
                            ioInteraction->interacted = true;
                        }
                        SelectColorPipelineRowFunction(ioState, laneIndex, rowIndex, candidate.id.c_str());
                    }
                    if (isSelected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            currentDescriptor = FindColorPipelineFunctionDescriptor(*catalog, row.function_id);
            if (!currentDescriptor) {
                PushColorPipelineValidationMessage(ioState,
                    std::string("Unknown advanced color function '") + row.function_id + "' for lane " + lane.label);
                return;
            }
            if (!currentDescriptor->description.empty()) {
                ImGui::TextWrapped("%s", currentDescriptor->description.c_str());
            }
            std::vector<std::size_t> renderableParamIndexes;
            bool hasHiddenParams = false;
            CollectRenderableColorPipelineParamIndexes(row, &renderableParamIndexes, &hasHiddenParams);
            for (std::size_t renderIndex = 0; renderIndex < renderableParamIndexes.size(); ++renderIndex) {
                const std::size_t paramIndex = renderableParamIndexes[renderIndex];
                if (paramIndex >= currentDescriptor->parameters.size() || paramIndex >= row.parameter_values.size()) {
                    continue;
                }
                RenderColorPipelineParamControl(currentDescriptor->parameters[paramIndex], &row.parameter_values[paramIndex], ioInteraction);
            }
            if (!currentDescriptor->parameters.empty() && renderableParamIndexes.empty()) {
                ImGui::TextDisabled("Parameter tuning preview only in this slice.");
            } else if (hasHiddenParams) {
                ImGui::TextDisabled("Only the visible controls are live in this slice.");
            }
            if (!currentDescriptor->parameters.empty() && lane.lane_id == "shape") {
                ImGui::TextDisabled("Shape rows edit the schedule draft now; only one live-backed Shape row participates in the current bridge at a time.");
            }
        });

        if (rowResult.changed && ioInteraction) {
            ioInteraction->interacted = true;
        }

        if (rowResult.remove_requested) {
            RemoveColorPipelineLaneRow(ioState, laneIndex, rowIndex);
            ImGui::Spacing();
            break;
        }
        if (rowResult.move_up_requested) {
            MoveColorPipelineLaneRow(ioState, laneIndex, rowIndex, -1);
            ImGui::Spacing();
            break;
        }
        if (rowResult.move_down_requested) {
            MoveColorPipelineLaneRow(ioState, laneIndex, rowIndex, 1);
            ImGui::Spacing();
            break;
        }

        ImGui::Spacing();
    }
    ImGui::PopID();
}

inline bool RenderColorPipelineWindow(
    ColorPipelineWindowState* ioState,
    FractalType liveFractalType,
    KernelParams* liveParams,
    bool* ioDirty = nullptr,
    bool* ioInteracted = nullptr) {
    if (!ioState || !ioState->open) {
        return false;
    }
    if (!EnsureColorPipelineWindowInitialized(ioState)) {
        return false;
    }

    ClearColorPipelineValidationMessages(ioState);
    if (liveParams) {
        SyncColorPipelineWindowFromLiveState(ioState, liveFractalType, liveParams);
    }

    bool open = ioState->open;
    ImGui::SetNextWindowSize(ImVec2(720.0f, 520.0f), ImGuiCond_FirstUseEver);
    const bool began = ImGui::Begin("Color Pipeline", &open);
    if (began) {
        ColorPipelineRenderInteractionState interactionState;
        RenderColorPipelineWindowSummary(ioState, liveFractalType, liveParams, ioDirty, &interactionState);
        for (std::size_t laneIndex = 0; laneIndex < ioState->lanes.size(); ++laneIndex) {
            RenderColorPipelineWindowLane(ioState, laneIndex, liveFractalType, liveParams, &interactionState);
            ImGui::Spacing();
        }
        const ColorPipelineDraftApplyState applyState = DescribeColorPipelineDraftApplyState(*ioState, liveFractalType, liveParams);
        if (ShouldAutoApplySupportedColorPipelineDraft(*ioState, applyState, interactionState, liveParams)) {
            bool changed = false;
            if (ApplyColorPipelineDraftToLiveState(ioState, liveFractalType, liveParams, &changed)) {
                if (changed && ioDirty) {
                    *ioDirty = true;
                }
                if (changed) {
                    interactionState.interacted = true;
                }
            }
        }
        if (interactionState.interacted && ioInteracted) {
            *ioInteracted = true;
        }
    }
    ImGui::End();

    ioState->open = open;
    return true;
}

inline bool RenderColorPipelineWindow(ColorPipelineWindowState* ioState) {
    return RenderColorPipelineWindow(ioState, FractalType::explaino, nullptr);
}
