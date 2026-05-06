#pragma once

#include "enum_id_utils.h"
#include "fractal_family_rules.h"
#include "function_descriptor.h"
#include "imgui.h"
#include "imgui_stack_editor.h"

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

struct ColorPipelineLaneState {
    std::string lane_id;
    std::string label;
    std::uint64_t ui_row_id = 0;
    std::string function_id;
    std::vector<ColorPipelineParamState> parameter_values;
};

struct ColorPipelineLiveSnapshot {
    bool valid = false;
    FractalType fractal_type = FractalType::explaino;
    ColoringMode coloring_mode = ColoringMode::root_basin;
    ColorPipelineSelection pipeline{};
    std::vector<ColorPipelineLaneState> lanes;
};

struct ColorPipelineWindowState {
    bool open = false;
    bool initialized = false;
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

inline std::vector<FunctionDescriptor> BuildColorPipelineSignalFunctions() {
    return {
        MakeColorPipelineFunction(
            "root_index",
            "Root Index",
            "Use root identity as the upstream signal before palette selection.",
            {
                MakeColorPipelineFloatParam("signal.root_mix", "Root Mix", "Blend between discrete root bands and a softened basin edge.", 0.0, 1.0, 0.01, 0.25),
            }),
        MakeColorPipelineFunction(
            "iteration_count",
            "Iteration Count",
            "Use raw iteration count as the upstream color signal.",
            {
                MakeColorPipelineFloatParam("signal.scale", "Scale", "Expand or compress the iteration-count ramp.", 0.25, 4.0, 0.01, 1.0),
            }),
        MakeColorPipelineFunction(
            "smooth_escape",
            "Smooth Escape",
            "Use the continuous escape estimate as the upstream color signal.",
            {
                MakeColorPipelineFloatParam("signal.scale", "Scale", "Expand or compress the smooth escape ramp.", 0.25, 4.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("signal.bias", "Bias", "Shift the smooth escape ramp before palette lookup.", -1.0, 1.0, 0.01, 0.0),
            }),
        MakeColorPipelineFunction(
            "phase_angle",
            "Phase Angle",
            "Use orbit phase as the upstream signal.",
            {
                MakeColorPipelineFloatParam("signal.phase_offset", "Phase Offset", "Rotate the sampled phase before downstream palette work.", -3.141592653589793, 3.141592653589793, 0.01, 0.0),
                MakeColorPipelineFloatParam("signal.wrap_cycles", "Wrap Cycles", "Control how many hue cycles appear across one full rotation.", 0.5, 6.0, 0.01, 1.0),
            }),
        MakeColorPipelineFunction(
            "iteration_bands",
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
            "root_classic",
            "Root Classic",
            "Map basin signals to the classic root palette.",
            {
                MakeColorPipelineFloatParam("palette.edge_mix", "Edge Mix", "Add a controlled amount of edge tinting to basin colors.", 0.0, 1.0, 0.01, 0.2),
            }),
        MakeColorPipelineFunction(
            "joy",
            "Joy",
            "Map basin signals to the brighter Joy palette.",
            {
                MakeColorPipelineFloatParam("palette.energy", "Energy", "Push or soften Joy palette intensity.", 0.0, 2.0, 0.01, 1.0),
            }),
        MakeColorPipelineFunction(
            "cyclic_escape",
            "Cyclic Escape",
            "Wrap escape signals through the cyclic escape palette.",
            {
                MakeColorPipelineFloatParam("palette.cycle_scale", "Cycle Scale", "Control how quickly the palette repeats across the signal.", 0.25, 4.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("palette.saturation", "Saturation", "Push or soften color separation in the cyclic palette.", 0.0, 2.0, 0.01, 1.0),
            }),
        MakeColorPipelineFunction(
            "phase_wheel",
            "Phase Wheel",
            "Wrap signal values around a phase wheel palette.",
            {
                MakeColorPipelineFloatParam("palette.phase_offset", "Phase Offset", "Rotate the wheel before it is applied to the incoming signal.", -3.141592653589793, 3.141592653589793, 0.01, 0.0),
                MakeColorPipelineFloatParam("palette.saturation", "Saturation", "Push or soften color separation in the wheel.", 0.0, 2.0, 0.01, 1.15),
            }),
        MakeColorPipelineFunction(
            "banded_escape",
            "Banded Escape",
            "Map banded escape signals through the runtime band palette.",
            {
                MakeColorPipelineFloatParam("palette.band_emphasis", "Band Emphasis", "Increase or relax the contrast between neighboring bands.", 0.0, 2.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("palette.phase_offset", "Phase Offset", "Offset the band palette without changing the source signal.", -3.141592653589793, 3.141592653589793, 0.01, 0.0),
            }),
    };
}

inline std::vector<FunctionDescriptor> BuildColorPipelineGradeFunctions() {
    return {
        MakeColorPipelineFunction(
            "basin_default",
            "Basin Default",
            "Apply the default basin grading profile.",
            {
                MakeColorPipelineFloatParam("grade.exposure", "Exposure", "Set the overall basin brightness.", 0.1, 3.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("grade.contrast", "Contrast", "Stretch basin mid-tones.", 0.0, 3.0, 0.01, 1.1),
            }),
        MakeColorPipelineFunction(
            "escape_default",
            "Escape Default",
            "Apply the default escape-time grading profile.",
            {
                MakeColorPipelineFloatParam("grade.exposure", "Exposure", "Set the overall escape brightness.", 0.1, 3.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("grade.saturation", "Saturation", "Push or soften the escape palette intensity.", 0.0, 2.0, 0.01, 1.0),
            }),
        MakeColorPipelineFunction(
            "phase_default",
            "Phase Default",
            "Apply the default phase grading profile.",
            {
                MakeColorPipelineFloatParam("grade.saturation", "Saturation", "Push or soften phase-wheel intensity.", 0.0, 2.0, 0.01, 1.15),
                MakeColorPipelineFloatParam("grade.contrast", "Contrast", "Stretch the phase palette mid-tones.", 0.0, 3.0, 0.01, 1.0),
            }),
        MakeColorPipelineFunction(
            "bands_default",
            "Bands Default",
            "Apply the default banded grading profile.",
            {
                MakeColorPipelineFloatParam("grade.band_emphasis", "Band Emphasis", "Increase or relax final band contrast.", 0.0, 2.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("grade.glow", "Glow", "Add controlled highlight bloom to the final output.", 0.0, 2.0, 0.01, 0.25),
            }),
    };
}

inline const std::vector<ColorPipelineLaneCatalog>& GetColorPipelineLaneCatalogs() {
    static const std::vector<ColorPipelineLaneCatalog> catalogs = {
        {"signal", "Signal", "root_index", BuildColorPipelineSignalFunctions()},
        {"palette", "Palette", "root_classic", BuildColorPipelinePaletteFunctions()},
        {"grade", "Grade", "basin_default", BuildColorPipelineGradeFunctions()},
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
    ColorPipelineLaneState* ioLane,
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

inline bool SetColorPipelineLaneFunction(
    ColorPipelineLaneState* ioLane,
    const FunctionDescriptor& descriptor) {
    if (!ioLane) {
        return false;
    }

    ioLane->function_id = descriptor.id;
    ioLane->parameter_values.clear();
    ioLane->parameter_values.reserve(descriptor.parameters.size());
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
        ioLane->parameter_values.push_back(std::move(value));
    }
    return true;
}

inline bool BuildColorPipelineLaneFromFunctionId(
    const ColorPipelineLaneCatalog& catalog,
    const char* functionId,
    std::uint64_t stableRowId,
    ColorPipelineLaneState* outLane,
    std::string* outError = nullptr) {
    if (!outLane || !functionId || functionId[0] == '\0') {
        if (outError) *outError = "Color pipeline lane builder requires a non-empty function id";
        return false;
    }

    const FunctionDescriptor* descriptor = FindColorPipelineFunctionDescriptor(catalog, functionId);
    if (!descriptor) {
        if (outError) {
            *outError = std::string("Unknown advanced color function '") + functionId + "' for lane " + catalog.label;
        }
        return false;
    }

    ColorPipelineLaneState lane;
    lane.lane_id = catalog.lane_id;
    lane.label = catalog.label;
    lane.ui_row_id = stableRowId;
    if (!SetColorPipelineLaneFunction(&lane, *descriptor)) {
        if (outError) *outError = std::string("Failed to initialize advanced color function '") + functionId + "'";
        return false;
    }
    *outLane = std::move(lane);
    return true;
}

inline void ApplyColorPipelineLaneTemplate(ColorPipelineLaneState* ioLane, const ColorPipelineLaneState& source) {
    if (!ioLane) {
        return;
    }
    const std::uint64_t stableRowId = ioLane->ui_row_id;
    *ioLane = source;
    ioLane->ui_row_id = stableRowId;
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

inline bool ColorPipelineLaneStatesEqual(
    const ColorPipelineLaneState& left,
    const ColorPipelineLaneState& right) {
    if (left.lane_id != right.lane_id ||
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

inline bool HasColorPipelineDraftEdits(const ColorPipelineWindowState& state) {
    if (!state.live_snapshot.valid || state.lanes.size() != state.live_snapshot.lanes.size()) {
        return false;
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
    if (ioState->lanes.size() != ioState->live_snapshot.lanes.size()) {
        ioState->lanes = ioState->live_snapshot.lanes;
    } else {
        for (std::size_t index = 0; index < ioState->lanes.size(); ++index) {
            ApplyColorPipelineLaneTemplate(&ioState->lanes[index], ioState->live_snapshot.lanes[index]);
        }
    }
    for (ColorPipelineLaneState& lane : ioState->lanes) {
        if (!EnsureImGuiStackEditorRowId(&lane.ui_row_id, &ioState->next_row_id)) {
            return false;
        }
    }
    return true;
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

    const char* laneFunctionIds[] = {
        ColorSignalId(liveParams.color_pipeline.signal),
        ColorPaletteId(liveParams.color_pipeline.palette),
        ColorGradingPresetId(liveParams.color_pipeline.grading),
    };

    ColorPipelineLiveSnapshot snapshot;
    snapshot.valid = true;
    snapshot.fractal_type = liveFractalType;
    snapshot.coloring_mode = liveParams.coloring_mode;
    snapshot.pipeline = liveParams.color_pipeline;
    const std::vector<ColorPipelineLaneCatalog>& catalogs = GetColorPipelineLaneCatalogs();
    snapshot.lanes.reserve(catalogs.size());
    for (std::size_t index = 0; index < catalogs.size(); ++index) {
        ColorPipelineLaneState lane;
        if (!BuildColorPipelineLaneFromFunctionId(catalogs[index], laneFunctionIds[index], 0, &lane, outError)) {
            return false;
        }
        if (!ImportSupportedColorPipelineParamsFromLive(&lane, liveParams, outError)) {
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
            if (!EnsureImGuiStackEditorRowId(&lane.ui_row_id, &ioState->next_row_id)) {
                return false;
            }
        }
        return true;
    }

    ioState->lanes.clear();
    ClearColorPipelineValidationMessages(ioState);
    for (const ColorPipelineLaneCatalog& catalog : GetColorPipelineLaneCatalogs()) {
        ColorPipelineLaneState lane;
        if (!BuildColorPipelineLaneFromFunctionId(catalog, catalog.default_function_id, 0, &lane)) {
            PushColorPipelineValidationMessage(ioState,
                std::string("Missing default advanced color function for lane: ") + catalog.label);
            return false;
        }
        if (!EnsureImGuiStackEditorRowId(&lane.ui_row_id, &ioState->next_row_id)) {
            return false;
        }
        ioState->lanes.push_back(std::move(lane));
    }
    ioState->initialized = true;
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

    const bool adoptIntoDraft = !ioState->live_snapshot.valid || !HasColorPipelineDraftEdits(*ioState);
    ColorPipelineLiveSnapshot nextSnapshot;
    std::string error;
    if (!TryBuildColorPipelineLiveSnapshot(liveFractalType, *liveParams, &nextSnapshot, &error)) {
        ioState->live_snapshot = {};
        PushColorPipelineValidationMessage(ioState, error);
        return false;
    }

    ioState->live_snapshot = std::move(nextSnapshot);
    if (adoptIntoDraft) {
        return ResetColorPipelineDraftFromLiveState(ioState);
    }
    return true;
}

inline bool SelectColorPipelineLaneFunction(
    ColorPipelineWindowState* ioState,
    std::size_t laneIndex,
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

    return SetColorPipelineLaneFunction(&lane, *descriptor);
}

inline bool IsLiveColorPipelineParamPath(const std::string& functionId, const std::string& path) {
    if (functionId == "phase_angle") {
        return path == "signal.phase_offset" || path == "signal.wrap_cycles";
    }
    if (functionId == "phase_wheel") {
        return path == "palette.phase_offset";
    }
    if (functionId == "iteration_bands") {
        return path == "signal.band_count" || path == "signal.softness";
    }
    if (functionId == "banded_escape") {
        return path == "palette.band_emphasis" || path == "palette.phase_offset";
    }
    return false;
}

inline bool TryGetColorPipelineParamNumber(
    const ColorPipelineLaneState& lane,
    const char* path,
    double* outValue,
    std::string* outError = nullptr) {
    if (!path || !outValue) {
        if (outError) *outError = "Advanced color parameter lookup requires a path and output storage";
        return false;
    }
    for (const ColorPipelineParamState& param : lane.parameter_values) {
        if (param.path == path) {
            *outValue = param.number_value;
            return true;
        }
    }
    if (outError) *outError = std::string("Missing advanced color parameter path '") + path + "' for function '" + lane.function_id + "'";
    return false;
}

inline bool SetColorPipelineParamNumber(
    ColorPipelineLaneState* ioLane,
    const char* path,
    double value,
    std::string* outError = nullptr) {
    if (!ioLane || !path) {
        if (outError) *outError = "Advanced color parameter import requires a lane and parameter path";
        return false;
    }
    for (ColorPipelineParamState& param : ioLane->parameter_values) {
        if (param.path == path) {
            param.number_value = value;
            return true;
        }
    }
    if (outError) *outError = std::string("Missing advanced color parameter path '") + path + "' for function '" + ioLane->function_id + "'";
    return false;
}

inline bool ImportSupportedColorPipelineParamsFromLive(
    ColorPipelineLaneState* ioLane,
    const KernelParams& liveParams,
    std::string* outError) {
    if (!ioLane) {
        if (outError) *outError = "Advanced color parameter import requires a lane";
        return false;
    }
    if (ioLane->function_id == "phase_angle") {
        return SetColorPipelineParamNumber(ioLane, "signal.phase_offset", liveParams.color_phase_signal_offset, outError) &&
            SetColorPipelineParamNumber(ioLane, "signal.wrap_cycles", liveParams.color_phase_wrap_cycles, outError);
    }
    if (ioLane->function_id == "phase_wheel") {
        return SetColorPipelineParamNumber(ioLane, "palette.phase_offset", liveParams.color_phase_palette_offset, outError);
    }
    if (ioLane->function_id == "iteration_bands") {
        return SetColorPipelineParamNumber(ioLane, "signal.band_count", static_cast<double>(liveParams.color_iteration_band_count), outError) &&
            SetColorPipelineParamNumber(ioLane, "signal.softness", liveParams.color_iteration_band_softness, outError);
    }
    if (ioLane->function_id == "banded_escape") {
        return SetColorPipelineParamNumber(ioLane, "palette.band_emphasis", liveParams.color_iteration_band_emphasis, outError) &&
            SetColorPipelineParamNumber(ioLane, "palette.phase_offset", liveParams.color_iteration_band_palette_offset, outError);
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
    const ColorPipelineLaneState& lane,
    const char* path,
    int* outValue,
    std::string* outError = nullptr) {
    double value = 0.0;
    if (!TryGetColorPipelineParamNumber(lane, path, &value, outError)) {
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
        if (lane.function_id == "phase_angle") {
            double phaseOffset = 0.0;
            double wrapCycles = 0.0;
            if (!TryGetColorPipelineParamNumber(lane, "signal.phase_offset", &phaseOffset, outError) ||
                !TryGetColorPipelineParamNumber(lane, "signal.wrap_cycles", &wrapCycles, outError) ||
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
        if (lane.function_id == "phase_wheel") {
            double paletteOffset = 0.0;
            if (!TryGetColorPipelineParamNumber(lane, "palette.phase_offset", &paletteOffset, outError) ||
                !ValidateColorPipelineParamRange("palette.phase_offset", paletteOffset, -3.141592653589793, 3.141592653589793, outError)) {
                return false;
            }
            if (std::fabs(ioParams->color_phase_palette_offset - static_cast<float>(paletteOffset)) > 1.0e-6f) {
                ioParams->color_phase_palette_offset = static_cast<float>(paletteOffset);
                changed = true;
            }
            continue;
        }
        if (lane.function_id == "iteration_bands") {
            int bandCount = 0;
            double softness = 0.0;
            if (!TryReadColorPipelineIntegerParam(lane, "signal.band_count", &bandCount, outError) ||
                !TryGetColorPipelineParamNumber(lane, "signal.softness", &softness, outError) ||
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
        if (lane.function_id == "banded_escape") {
            double emphasis = 0.0;
            double paletteOffset = 0.0;
            if (!TryGetColorPipelineParamNumber(lane, "palette.band_emphasis", &emphasis, outError) ||
                !TryGetColorPipelineParamNumber(lane, "palette.phase_offset", &paletteOffset, outError) ||
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

    if (outChanged) {
        *outChanged = changed;
    }
    return true;
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

    ColorPipelineSelection pipeline{};
    bool hasSignal = false;
    bool hasPalette = false;
    bool hasGrading = false;
    for (const ColorPipelineLaneState& lane : state.lanes) {
        if (lane.lane_id == "signal") {
            if (!TryParseColorSignalId(lane.function_id, &pipeline.signal)) {
                if (outError) *outError = std::string("Unknown advanced color signal function '") + lane.function_id + "'";
                return false;
            }
            hasSignal = true;
            continue;
        }
        if (lane.lane_id == "palette") {
            if (!TryParseColorPaletteId(lane.function_id, &pipeline.palette)) {
                if (outError) *outError = std::string("Unknown advanced color palette function '") + lane.function_id + "'";
                return false;
            }
            hasPalette = true;
            continue;
        }
        if (lane.lane_id == "grade") {
            if (!TryParseColorGradingPresetId(lane.function_id, &pipeline.grading)) {
                if (outError) *outError = std::string("Unknown advanced color grading function '") + lane.function_id + "'";
                return false;
            }
            hasGrading = true;
            continue;
        }
        if (outError) *outError = std::string("Unknown advanced color pipeline lane id: ") + lane.lane_id;
        return false;
    }

    if (!(hasSignal && hasPalette && hasGrading)) {
        if (outError) *outError = "Advanced color pipeline apply requires signal, palette, and grade lanes";
        return false;
    }

    ColoringMode mode = ColoringMode::root_basin;
    if (!TryLegacyColoringModeForPipeline(pipeline, &mode)) {
        if (outError) *outError = "Selected advanced color tuple does not map to a supported live runtime mode";
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
    if (!state.live_snapshot.valid) {
        return {
            ColorPipelineDraftApplyStatus::live_unavailable,
            "Live runtime selection is not available yet.",
        };
    }
    if (!HasColorPipelineDraftEdits(state)) {
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
            "Selected lane mix is preview-only in this slice; only exact supported live tuples can apply.",
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
        if (!ApplySupportedColorPipelineParamsToLive(state, &probe, nullptr, &error)) {
            return {
                ColorPipelineDraftApplyStatus::invalid_params,
                error,
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
    if (!SelectColorPipelineLaneFunction(&probeState, laneIndex, functionId)) {
        return {
            ColorPipelineDraftApplyStatus::unsupported_tuple,
            "Unsupported advanced color candidate.",
        };
    }
    return DescribeColorPipelineDraftApplyState(probeState, liveFractalType, liveParams);
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

inline bool RenderColorPipelineParamControl(
    const FunctionParamDescriptor& param,
    ColorPipelineParamState* ioValue) {
    if (!ioValue) {
        return false;
    }

    bool changed = false;
    ImGui::PushID(param.path.c_str());
    if (param.type == "float" || param.type == "double") {
        float value = static_cast<float>(ioValue->number_value);
        if (param.has_min && param.has_max) {
            changed = ImGui::SliderFloat(param.label.c_str(), &value,
                static_cast<float>(param.min_value),
                static_cast<float>(param.max_value));
        } else {
            const float step = param.has_step ? static_cast<float>(param.step_value) : 0.01f;
            changed = ImGui::DragFloat(param.label.c_str(), &value, step);
        }
        if (changed) {
            ioValue->number_value = value;
        }
    } else if (param.type == "int") {
        int value = static_cast<int>(std::lround(ioValue->number_value));
        if (param.has_min && param.has_max) {
            changed = ImGui::SliderInt(param.label.c_str(), &value,
                static_cast<int>(param.min_value),
                static_cast<int>(param.max_value));
        } else {
            const float step = param.has_step ? static_cast<float>(param.step_value) : 1.0f;
            changed = ImGui::DragInt(param.label.c_str(), &value, step);
        }
        if (changed) {
            ioValue->number_value = static_cast<double>(value);
        }
    } else if (param.type == "bool") {
        bool value = ioValue->bool_value;
        changed = ImGui::Checkbox(param.label.c_str(), &value);
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
    bool* ioDirty) {
    ImGui::TextWrapped("Live slot editor: applying Signal / Palette / Grade here updates the same runtime color tuple used by the main Color panel.");
    ImGui::TextDisabled("Only exact supported live tuples are executable in this slice; unsupported mixed lane combinations remain preview-only.");
    ImGui::TextDisabled("Within those tuples, only phase and iteration-bands tuning controls are live in this slice; unsupported function knobs remain preview-only.");
    ImGui::Separator();
    if (ioState && ioState->live_snapshot.valid) {
        const ColorPipelineDraftApplyState applyState = DescribeColorPipelineDraftApplyState(*ioState, liveFractalType, liveParams);
        ImGui::TextWrapped(
            "Live runtime: %s -> %s / %s / %s",
            ColoringModeId(ioState->live_snapshot.coloring_mode),
            ColorSignalId(ioState->live_snapshot.pipeline.signal),
            ColorPaletteId(ioState->live_snapshot.pipeline.palette),
            ColorGradingPresetId(ioState->live_snapshot.pipeline.grading));
        if (applyState.status == ColorPipelineDraftApplyStatus::matches_live) {
            ImGui::TextDisabled("%s", applyState.message.c_str());
        } else {
            const bool canApply = applyState.status == ColorPipelineDraftApplyStatus::can_apply;
            const ImVec4 statusColor = canApply
                ? ImVec4(0.95f, 0.83f, 0.40f, 1.0f)
                : ImVec4(1.0f, 0.62f, 0.48f, 1.0f);
            ImGui::TextColored(statusColor, "%s", applyState.message.c_str());
            if (!canApply) {
                ImGui::TextDisabled("Supported live tuples in this slice: Root Basin, Joy Basins, Iteration Count, Smooth Escape, Phase, and Iteration Bands.");
            }
            if (liveParams) {
                if (!canApply) {
                    ImGui::BeginDisabled();
                }
                if (ImGui::Button("Apply Selected Pipeline") && canApply) {
                    bool changed = false;
                    if (ApplyColorPipelineDraftToLiveState(ioState, liveFractalType, liveParams, &changed) && changed && ioDirty) {
                        *ioDirty = true;
                    }
                }
                if (!canApply) {
                    ImGui::EndDisabled();
                }
                ImGui::SameLine();
            }
            if (ImGui::Button("Reset Draft From Live")) {
                ResetColorPipelineDraftFromLiveState(ioState);
            }
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
    const KernelParams* liveParams) {
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

    const FunctionDescriptor* descriptor = FindColorPipelineFunctionDescriptor(*catalog, lane.function_id);
    if (!descriptor) {
        PushColorPipelineValidationMessage(ioState,
            std::string("Unknown advanced color function '") + lane.function_id + "' for lane " + lane.label);
        return;
    }

    const FunctionDescriptor* currentDescriptor = descriptor;
    const std::string headerLabel = lane.label + ": " + currentDescriptor->name;
    ImGuiStackEditorRowChromeSpec rowSpec;
    rowSpec.tree_node_id = lane.lane_id.c_str();
    rowSpec.header_label = headerLabel.c_str();
    rowSpec.stable_row_id = lane.ui_row_id;
    rowSpec.allow_remove = false;
    rowSpec.allow_move_up = false;
    rowSpec.allow_move_down = false;

    RenderImGuiStackEditorRowChrome(rowSpec, [&]() {
        const char* comboPreview = currentDescriptor->name.empty() ? "(select)" : currentDescriptor->name.c_str();
        if (ImGui::BeginCombo("Function", comboPreview)) {
            for (const FunctionDescriptor& candidate : catalog->functions) {
                const bool isSelected = (candidate.id == lane.function_id);
                const ColorPipelineDraftApplyState candidateState = DescribeColorPipelineCandidateApplyState(
                    *ioState,
                    laneIndex,
                    candidate.id.c_str(),
                    liveFractalType,
                    liveParams);
                std::string optionLabel = candidate.name;
                if (!isSelected && candidateState.status != ColorPipelineDraftApplyStatus::can_apply &&
                    candidateState.status != ColorPipelineDraftApplyStatus::matches_live) {
                    optionLabel += " (preview only)";
                }
                if (ImGui::Selectable(optionLabel.c_str(), isSelected)) {
                    if (SelectColorPipelineLaneFunction(ioState, laneIndex, candidate.id.c_str())) {
                        currentDescriptor = FindColorPipelineFunctionDescriptor(*catalog, lane.function_id);
                    }
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        if (!currentDescriptor) {
            return;
        }
        if (!currentDescriptor->description.empty()) {
            ImGui::TextWrapped("%s", currentDescriptor->description.c_str());
        }
        if (!currentDescriptor->parameters.empty()) {
            bool hasLiveParams = false;
            bool hasPreviewOnlyParams = false;
            for (std::size_t paramIndex = 0; paramIndex < currentDescriptor->parameters.size() &&
                    paramIndex < lane.parameter_values.size();
                 ++paramIndex) {
                const bool liveParam = IsLiveColorPipelineParamPath(
                    lane.function_id,
                    currentDescriptor->parameters[paramIndex].path);
                hasLiveParams = hasLiveParams || liveParam;
                hasPreviewOnlyParams = hasPreviewOnlyParams || !liveParam;
                if (!liveParam) {
                    ImGui::BeginDisabled();
                }
                RenderColorPipelineParamControl(currentDescriptor->parameters[paramIndex], &lane.parameter_values[paramIndex]);
                if (!liveParam) {
                    ImGui::EndDisabled();
                }
            }
            if (!hasLiveParams) {
                ImGui::TextDisabled("Parameter tuning preview only in this slice.");
            } else if (hasPreviewOnlyParams) {
                ImGui::TextDisabled("Only the enabled controls are live in this slice.");
            }
        }
    });
}

inline bool RenderColorPipelineWindow(
    ColorPipelineWindowState* ioState,
    FractalType liveFractalType,
    KernelParams* liveParams,
    bool* ioDirty = nullptr) {
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
        RenderColorPipelineWindowSummary(ioState, liveFractalType, liveParams, ioDirty);
        for (std::size_t laneIndex = 0; laneIndex < ioState->lanes.size(); ++laneIndex) {
            RenderColorPipelineWindowLane(ioState, laneIndex, liveFractalType, liveParams);
            ImGui::Spacing();
        }
    }
    ImGui::End();

    ioState->open = open;
    return true;
}

inline bool RenderColorPipelineWindow(ColorPipelineWindowState* ioState) {
    return RenderColorPipelineWindow(ioState, FractalType::explaino, nullptr);
}
