#pragma once

#include "color_pipeline_core.h"
#include "enum_id_utils.h"
#include "fractal_family_rules.h"
#include "schema_binding.h"

#include <cmath>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#ifndef COLOR_PIPELINE_WINDOW_NO_IMGUI
#include "imgui.h"
#include "imgui_stack_editor.h"
#else
inline bool EnsureImGuiStackEditorRowId(std::uint64_t* ioRowId, std::uint64_t* ioNextRowId) {
    if (!ioRowId || !ioNextRowId) {
        return false;
    }
    if (*ioNextRowId == 0) {
        *ioNextRowId = 1;
    }
    if (*ioRowId == 0) {
        *ioRowId = *ioNextRowId;
        ++(*ioNextRowId);
        return true;
    }
    if (*ioNextRowId <= *ioRowId) {
        *ioNextRowId = *ioRowId + 1;
    }
    return true;
}
#endif

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

inline bool IsLegacyColorPanelControlBindingPath(const std::string& bindingPath) {
    return bindingPath == "fractal.params.coloring_mode" ||
        bindingPath == "fractal.params.color_grading";
}

inline bool ShouldDisableLegacyColorPanelControlWhileAdvancedWindowOpen(
    const ColorPipelineWindowState& state,
    const std::string& bindingPath) {
    return state.open && IsLegacyColorPanelControlBindingPath(bindingPath);
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
    return color_pipeline_core::MakeColorPipelineFloatParam(
        path,
        label,
        help,
        minValue,
        maxValue,
        stepValue,
        defaultValue);
}

inline FunctionParamDescriptor MakeColorPipelineIntParam(
    const char* path,
    const char* label,
    const char* help,
    int minValue,
    int maxValue,
    int stepValue,
    int defaultValue) {
    return color_pipeline_core::MakeColorPipelineIntParam(
        path,
        label,
        help,
        minValue,
        maxValue,
        stepValue,
        defaultValue);
}

inline FunctionParamDescriptor MakeColorPipelineBoolParam(
    const char* path,
    const char* label,
    const char* help,
    bool defaultValue) {
    return color_pipeline_core::MakeColorPipelineBoolParam(path, label, help, defaultValue);
}

inline FunctionDescriptor MakeColorPipelineFunction(
    const char* id,
    const char* name,
    const char* description,
    std::vector<FunctionParamDescriptor> parameters) {
    return color_pipeline_core::MakeColorPipelineFunction(
        id,
        name,
        description,
        std::move(parameters));
}

inline const char* AdvancedColorSignalFunctionId(ColorSignal value) {
    return color_pipeline_core::AdvancedColorSignalFunctionId(value);
}

inline bool TryParseAdvancedColorSignalFunctionId(const std::string& functionId, ColorSignal* outValue) {
    return color_pipeline_core::TryParseAdvancedColorSignalFunctionId(functionId, outValue);
}

inline const char* AdvancedColorPaletteFunctionId(ColorPalette value) {
    return color_pipeline_core::AdvancedColorPaletteFunctionId(value);
}

inline bool TryParseAdvancedColorPaletteFunctionId(const std::string& functionId, ColorPalette* outValue) {
    return color_pipeline_core::TryParseAdvancedColorPaletteFunctionId(functionId, outValue);
}

inline const char* AdvancedColorGradingFunctionId(ColorGradingPreset value) {
    return color_pipeline_core::AdvancedColorGradingFunctionId(value);
}

inline bool TryParseAdvancedColorGradingFunctionId(const std::string& functionId, ColorGradingPreset* outValue) {
    return color_pipeline_core::TryParseAdvancedColorGradingFunctionId(functionId, outValue);
}

inline const char* AdvancedColorShapeFunctionId(ColorPipelineShape value) {
    return color_pipeline_core::AdvancedColorShapeFunctionId(value);
}

inline const std::vector<ColorPipelineLaneCatalog>& GetColorPipelineLaneCatalogs() {
    return color_pipeline_core::GetColorPipelineLaneCatalogs();
}

inline const ColorPipelineLaneCatalog* FindColorPipelineLaneCatalog(const std::string& laneId) {
    return color_pipeline_core::FindColorPipelineLaneCatalog(laneId);
}

inline const FunctionDescriptor* FindColorPipelineFunctionDescriptor(
    const ColorPipelineLaneCatalog& catalog,
    const std::string& functionId) {
    return color_pipeline_core::FindColorPipelineFunctionDescriptor(catalog, functionId);
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

inline bool TryBuildColorPipelineSelectionFromLaneIds(
    const char* sourceFunctionId,
    const char* paletteFunctionId,
    ColorPipelineSelection* outPipeline,
    ColoringMode* outMode);

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

inline bool ColorPipelineSelectionsEqual(
    const ColorPipelineSelection& left,
    const ColorPipelineSelection& right) {
    return left.signal == right.signal &&
        left.palette == right.palette &&
        left.grading == right.grading;
}

inline bool ColorPipelineLiveSnapshotsEqual(
    const ColorPipelineLiveSnapshot& left,
    const ColorPipelineLiveSnapshot& right) {
    if (left.valid != right.valid ||
        left.draft_import_supported != right.draft_import_supported ||
        left.fractal_type != right.fractal_type ||
        left.coloring_mode != right.coloring_mode ||
        !ColorPipelineSelectionsEqual(left.pipeline, right.pipeline) ||
        left.lanes.size() != right.lanes.size()) {
        return false;
    }
    for (std::size_t index = 0; index < left.lanes.size(); ++index) {
        if (!ColorPipelineLaneStatesEqual(left.lanes[index], right.lanes[index])) {
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
    return color_pipeline_core::TryBuildColorPipelineScheduleBridgeIds(
        pipeline,
        outSourceFunctionId,
        outPaletteFunctionId);
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
    if (!TryMirroredColoringModeForPipeline(liveParams.color_pipeline, &inferredMode)) {
        if (outError) *outError = "Live runtime color pipeline does not map to a supported mirrored mode";
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
    const bool liveSnapshotWasImportSupported = ioState->live_snapshot.draft_import_supported;
    const bool draftHasEdits = HasColorPipelineDraftEdits(*ioState);
    const bool draftMatchesStarter = IsColorPipelineStarterDraft(*ioState);
    ColorPipelineLiveSnapshot nextSnapshot;
    std::string error;
    if (!TryBuildColorPipelineLiveSnapshot(liveFractalType, *liveParams, &nextSnapshot, &error)) {
        ioState->live_snapshot = {};
        PushColorPipelineValidationMessage(ioState, error);
        return false;
    }

    const bool liveSnapshotChanged =
        !liveSnapshotWasValid ||
        !ColorPipelineLiveSnapshotsEqual(ioState->live_snapshot, nextSnapshot);
    const bool adoptIntoDraft =
        !liveSnapshotWasValid ||
        (liveSnapshotChanged && !draftHasEdits) ||
        (liveSnapshotChanged && !liveSnapshotWasImportSupported && draftMatchesStarter);

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

    if (!SetColorPipelineRowFunction(&lane.rows[rowIndex], *descriptor)) {
        return false;
    }

    if (rowIndex == 0 && lane.rows.size() == 1 &&
        (lane.lane_id == "source" || lane.lane_id == "palette")) {
        const ColorPipelineLaneState* sourceLane = nullptr;
        const ColorPipelineLaneState* paletteLane = nullptr;
        for (const ColorPipelineLaneState& candidateLane : ioState->lanes) {
            if (candidateLane.lane_id == "source") {
                sourceLane = &candidateLane;
            } else if (candidateLane.lane_id == "palette") {
                paletteLane = &candidateLane;
            }
        }

        ColorPipelineSelection pipeline{};
        ColoringMode mode = ColoringMode::root_basin;
        const char* companionLaneId = nullptr;
        const char* companionFunctionId = nullptr;
        const bool singleSourcePaletteRows =
            sourceLane && paletteLane &&
            sourceLane->rows.size() == 1 &&
            paletteLane->rows.size() == 1;
        const bool pairAlreadySupported =
            singleSourcePaletteRows &&
            TryBuildColorPipelineSelectionFromLaneIds(
                sourceLane->rows[0].function_id.c_str(),
                paletteLane->rows[0].function_id.c_str(),
                &pipeline,
                &mode);
        if (!pairAlreadySupported) {
            if (lane.lane_id == "source") {
                if (lane.rows[rowIndex].function_id == "smooth_escape_ramp" ||
                    lane.rows[rowIndex].function_id == "escape_magnitude" ||
                    lane.rows[rowIndex].function_id == "root_proximity") {
                    companionLaneId = "palette";
                    companionFunctionId = "heatmap";
                } else if (lane.rows[rowIndex].function_id == "phase_orbit" ||
                           lane.rows[rowIndex].function_id == "orbit_stripe") {
                    companionLaneId = "palette";
                    companionFunctionId = "phase_wheel_palette";
                } else if (lane.rows[rowIndex].function_id == "banded_signal") {
                    companionLaneId = "palette";
                    companionFunctionId = "banded_heatmap";
                } else if (lane.rows[rowIndex].function_id == "root_index") {
                    companionLaneId = "palette";
                    companionFunctionId = "root_classic_palette";
                }
            } else if (lane.lane_id == "palette") {
                if (lane.rows[rowIndex].function_id == "heatmap" ||
                    lane.rows[rowIndex].function_id == "explaino_cmap") {
                    companionLaneId = "source";
                    companionFunctionId = "smooth_escape_ramp";
                } else if (lane.rows[rowIndex].function_id == "phase_wheel_palette") {
                    companionLaneId = "source";
                    companionFunctionId = "phase_orbit";
                } else if (lane.rows[rowIndex].function_id == "banded_heatmap") {
                    companionLaneId = "source";
                    companionFunctionId = "banded_signal";
                } else if (lane.rows[rowIndex].function_id == "root_classic_palette") {
                    companionLaneId = "source";
                    companionFunctionId = "root_index";
                }
            }

            if (companionLaneId && companionFunctionId) {
                for (ColorPipelineLaneState& companionLane : ioState->lanes) {
                    if (companionLane.lane_id != companionLaneId) {
                        continue;
                    }
                    if (companionLane.rows.size() != 1) {
                        break;
                    }
                    const ColorPipelineLaneCatalog* companionCatalog = FindColorPipelineLaneCatalog(companionLane.lane_id);
                    if (!companionCatalog) {
                        break;
                    }
                    const FunctionDescriptor* companionDescriptor = FindColorPipelineFunctionDescriptor(*companionCatalog, companionFunctionId);
                    if (!companionDescriptor) {
                        break;
                    }
                    SetColorPipelineRowFunction(&companionLane.rows[0], *companionDescriptor);
                    break;
                }
            }
        }
    }

    return true;
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
    if (functionId == "escape_magnitude") {
        return path == "signal.magnitude_scale" || path == "signal.magnitude_bias";
    }
    if (functionId == "orbit_stripe") {
        return path == "signal.stripe_frequency" || path == "signal.phase_offset";
    }
    if (functionId == "root_proximity") {
        return path == "signal.proximity_scale" || path == "signal.proximity_bias";
    }
    if (functionId == "phase_wheel_palette") {
        return path == "palette.phase_offset";
    }
    if (functionId == "explaino_cmap") {
        return path == "palette.seed_scale" || path == "palette.seed_phase" || path == "palette.colorfulness";
    }
    if (functionId == "offset_scale") {
        return path == "shape.offset" || path == "shape.scale";
    }
    if (functionId == "repeat") {
        return path == "shape.frequency" || path == "shape.phase";
    }
    if (functionId == "posterize") {
        return path == "shape.steps" || path == "shape.mix";
    }
    if (functionId == "mirror_repeat") {
        return path == "shape.frequency" || path == "shape.phase";
    }
    if (functionId == "bias_gain_curve") {
        return path == "shape.bias" || path == "shape.gain";
    }
    if (functionId == "smooth_window") {
        return path == "shape.center" || path == "shape.width" || path == "shape.softness";
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
    if (ioRow->function_id == "escape_magnitude") {
        return SetColorPipelineParamNumber(ioRow, "signal.magnitude_scale", liveParams.color_escape_magnitude_scale, outError) &&
            SetColorPipelineParamNumber(ioRow, "signal.magnitude_bias", liveParams.color_escape_magnitude_bias, outError);
    }
    if (ioRow->function_id == "orbit_stripe") {
        return SetColorPipelineParamNumber(ioRow, "signal.stripe_frequency", liveParams.color_orbit_stripe_frequency, outError) &&
            SetColorPipelineParamNumber(ioRow, "signal.phase_offset", liveParams.color_orbit_stripe_phase, outError);
    }
    if (ioRow->function_id == "root_proximity") {
        return SetColorPipelineParamNumber(ioRow, "signal.proximity_scale", liveParams.color_root_proximity_scale, outError) &&
            SetColorPipelineParamNumber(ioRow, "signal.proximity_bias", liveParams.color_root_proximity_bias, outError);
    }
    if (ioRow->function_id == "phase_wheel_palette") {
        return SetColorPipelineParamNumber(ioRow, "palette.phase_offset", liveParams.color_phase_palette_offset, outError);
    }
    if (ioRow->function_id == "explaino_cmap") {
        return SetColorPipelineParamNumber(ioRow, "palette.seed_scale", liveParams.color_explaino_palette_seed_scale, outError) &&
            SetColorPipelineParamNumber(ioRow, "palette.seed_phase", liveParams.color_explaino_palette_seed_phase, outError) &&
            SetColorPipelineParamNumber(ioRow, "palette.colorfulness", liveParams.color_explaino_palette_colorfulness, outError);
    }
    if (ioRow->function_id == "offset_scale") {
        return SetColorPipelineParamNumber(ioRow, "shape.offset", liveParams.color_shape_offset, outError) &&
            SetColorPipelineParamNumber(ioRow, "shape.scale", liveParams.color_shape_scale, outError);
    }
    if (ioRow->function_id == "repeat") {
        return SetColorPipelineParamNumber(ioRow, "shape.frequency", liveParams.color_shape_repeat_frequency, outError) &&
            SetColorPipelineParamNumber(ioRow, "shape.phase", liveParams.color_shape_repeat_phase, outError);
    }
    if (ioRow->function_id == "posterize") {
        return SetColorPipelineParamNumber(ioRow, "shape.steps", static_cast<double>(liveParams.color_shape_posterize_steps), outError) &&
            SetColorPipelineParamNumber(ioRow, "shape.mix", liveParams.color_shape_posterize_mix, outError);
    }
    if (ioRow->function_id == "mirror_repeat") {
        return SetColorPipelineParamNumber(ioRow, "shape.frequency", liveParams.color_shape_repeat_frequency, outError) &&
            SetColorPipelineParamNumber(ioRow, "shape.phase", liveParams.color_shape_repeat_phase, outError);
    }
    if (ioRow->function_id == "bias_gain_curve") {
        return SetColorPipelineParamNumber(ioRow, "shape.bias", liveParams.color_shape_bias, outError) &&
            SetColorPipelineParamNumber(ioRow, "shape.gain", liveParams.color_shape_gain, outError);
    }
    if (ioRow->function_id == "smooth_window") {
        return SetColorPipelineParamNumber(ioRow, "shape.center", liveParams.color_shape_window_center, outError) &&
            SetColorPipelineParamNumber(ioRow, "shape.width", liveParams.color_shape_window_width, outError) &&
            SetColorPipelineParamNumber(ioRow, "shape.softness", liveParams.color_shape_window_softness, outError);
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
    const auto assignShapeInt = [&](int* target, int value) {
        if (*target != value) {
            *target = value;
            changed = true;
        }
    };
    const auto assignShapeFloat = [&](float* target, float value) {
        if (std::fabs(*target - value) > 1.0e-6f) {
            *target = value;
            changed = true;
        }
    };
    const auto resetShapeOffsetScale = [&]() {
        assignShapeFloat(&ioParams->color_shape_offset, 0.0f);
        assignShapeFloat(&ioParams->color_shape_scale, 1.0f);
    };
    const auto resetShapeRepeat = [&]() {
        assignShapeFloat(&ioParams->color_shape_repeat_frequency, 8.0f);
        assignShapeFloat(&ioParams->color_shape_repeat_phase, 0.0f);
    };
    const auto resetShapePosterize = [&]() {
        assignShapeInt(&ioParams->color_shape_posterize_steps, 6);
        assignShapeFloat(&ioParams->color_shape_posterize_mix, 1.0f);
    };
    const auto resetShapeBiasGain = [&]() {
        assignShapeFloat(&ioParams->color_shape_bias, 0.5f);
        assignShapeFloat(&ioParams->color_shape_gain, 0.5f);
    };
    const auto resetShapeSmoothWindow = [&]() {
        assignShapeFloat(&ioParams->color_shape_window_center, 0.5f);
        assignShapeFloat(&ioParams->color_shape_window_width, 1.0f);
        assignShapeFloat(&ioParams->color_shape_window_softness, 0.0f);
    };
    const auto resetPaletteHeatmap = [&]() {
        assignShapeFloat(&ioParams->color_heatmap_cycle_scale, 1.0f);
        assignShapeFloat(&ioParams->color_heatmap_saturation, 1.0f);
    };
    const auto resetPalettePhaseWheel = [&]() {
        assignShapeFloat(&ioParams->color_phase_palette_offset, 0.0f);
    };
    const auto resetPaletteBandedHeatmap = [&]() {
        assignShapeFloat(&ioParams->color_iteration_band_emphasis, 1.0f);
        assignShapeFloat(&ioParams->color_iteration_band_palette_offset, 0.0f);
    };
    const auto resetPaletteExplaino = [&]() {
        assignShapeFloat(&ioParams->color_explaino_palette_seed_scale, 1.0f);
        assignShapeFloat(&ioParams->color_explaino_palette_seed_phase, 0.0f);
        assignShapeFloat(&ioParams->color_explaino_palette_colorfulness, 1.0f);
    };
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
            resetPalettePhaseWheel();
            resetPaletteBandedHeatmap();
            resetPaletteExplaino();
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
        if (row.function_id == "escape_magnitude") {
            double magnitudeScale = 0.0;
            double magnitudeBias = 0.0;
            if (!TryGetColorPipelineParamNumber(row, "signal.magnitude_scale", &magnitudeScale, outError) ||
                !TryGetColorPipelineParamNumber(row, "signal.magnitude_bias", &magnitudeBias, outError) ||
                !ValidateColorPipelineParamRange("signal.magnitude_scale", magnitudeScale, 0.25, 4.0, outError) ||
                !ValidateColorPipelineParamRange("signal.magnitude_bias", magnitudeBias, -1.0, 1.0, outError)) {
                return false;
            }
            if (std::fabs(ioParams->color_escape_magnitude_scale - static_cast<float>(magnitudeScale)) > 1.0e-6f) {
                ioParams->color_escape_magnitude_scale = static_cast<float>(magnitudeScale);
                changed = true;
            }
            if (std::fabs(ioParams->color_escape_magnitude_bias - static_cast<float>(magnitudeBias)) > 1.0e-6f) {
                ioParams->color_escape_magnitude_bias = static_cast<float>(magnitudeBias);
                changed = true;
            }
            continue;
        }
        if (row.function_id == "orbit_stripe") {
            double stripeFrequency = 0.0;
            double stripePhase = 0.0;
            if (!TryGetColorPipelineParamNumber(row, "signal.stripe_frequency", &stripeFrequency, outError) ||
                !TryGetColorPipelineParamNumber(row, "signal.phase_offset", &stripePhase, outError) ||
                !ValidateColorPipelineParamRange("signal.stripe_frequency", stripeFrequency, 0.25, 12.0, outError) ||
                !ValidateColorPipelineParamRange("signal.phase_offset", stripePhase, -3.141592653589793, 3.141592653589793, outError)) {
                return false;
            }
            if (std::fabs(ioParams->color_orbit_stripe_frequency - static_cast<float>(stripeFrequency)) > 1.0e-6f) {
                ioParams->color_orbit_stripe_frequency = static_cast<float>(stripeFrequency);
                changed = true;
            }
            if (std::fabs(ioParams->color_orbit_stripe_phase - static_cast<float>(stripePhase)) > 1.0e-6f) {
                ioParams->color_orbit_stripe_phase = static_cast<float>(stripePhase);
                changed = true;
            }
            continue;
        }
        if (row.function_id == "root_proximity") {
            double proximityScale = 0.0;
            double proximityBias = 0.0;
            if (!TryGetColorPipelineParamNumber(row, "signal.proximity_scale", &proximityScale, outError) ||
                !TryGetColorPipelineParamNumber(row, "signal.proximity_bias", &proximityBias, outError) ||
                !ValidateColorPipelineParamRange("signal.proximity_scale", proximityScale, 0.25, 8.0, outError) ||
                !ValidateColorPipelineParamRange("signal.proximity_bias", proximityBias, -1.0, 1.0, outError)) {
                return false;
            }
            if (std::fabs(ioParams->color_root_proximity_scale - static_cast<float>(proximityScale)) > 1.0e-6f) {
                ioParams->color_root_proximity_scale = static_cast<float>(proximityScale);
                changed = true;
            }
            if (std::fabs(ioParams->color_root_proximity_bias - static_cast<float>(proximityBias)) > 1.0e-6f) {
                ioParams->color_root_proximity_bias = static_cast<float>(proximityBias);
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
            resetPaletteHeatmap();
            resetPaletteBandedHeatmap();
            resetPaletteExplaino();
            continue;
        }
        if (row.function_id == "explaino_cmap") {
            double seedScale = 0.0;
            double seedPhase = 0.0;
            double colorfulness = 0.0;
            if (!TryGetColorPipelineParamNumber(row, "palette.seed_scale", &seedScale, outError) ||
                !TryGetColorPipelineParamNumber(row, "palette.seed_phase", &seedPhase, outError) ||
                !TryGetColorPipelineParamNumber(row, "palette.colorfulness", &colorfulness, outError) ||
                !ValidateColorPipelineParamRange("palette.seed_scale", seedScale, 0.25, 4.0, outError) ||
                !ValidateColorPipelineParamRange("palette.seed_phase", seedPhase, -1.0, 1.0, outError) ||
                !ValidateColorPipelineParamRange("palette.colorfulness", colorfulness, 0.0, 1.0, outError)) {
                return false;
            }
            if (std::fabs(ioParams->color_explaino_palette_seed_scale - static_cast<float>(seedScale)) > 1.0e-6f) {
                ioParams->color_explaino_palette_seed_scale = static_cast<float>(seedScale);
                changed = true;
            }
            if (std::fabs(ioParams->color_explaino_palette_seed_phase - static_cast<float>(seedPhase)) > 1.0e-6f) {
                ioParams->color_explaino_palette_seed_phase = static_cast<float>(seedPhase);
                changed = true;
            }
            if (std::fabs(ioParams->color_explaino_palette_colorfulness - static_cast<float>(colorfulness)) > 1.0e-6f) {
                ioParams->color_explaino_palette_colorfulness = static_cast<float>(colorfulness);
                changed = true;
            }
            resetPaletteHeatmap();
            resetPalettePhaseWheel();
            resetPaletteBandedHeatmap();
            continue;
        }
        if (row.function_id == "identity") {
            if (ioParams->color_shape != ColorPipelineShape::identity) {
                ioParams->color_shape = ColorPipelineShape::identity;
                changed = true;
            }
            resetShapeOffsetScale();
            resetShapeRepeat();
            resetShapePosterize();
            resetShapeBiasGain();
            resetShapeSmoothWindow();
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
            assignShapeFloat(&ioParams->color_shape_offset, static_cast<float>(offset));
            assignShapeFloat(&ioParams->color_shape_scale, static_cast<float>(scale));
            resetShapeRepeat();
            resetShapePosterize();
            resetShapeBiasGain();
            resetShapeSmoothWindow();
            continue;
        }
        if (row.function_id == "repeat") {
            double frequency = 0.0;
            double phase = 0.0;
            if (!TryGetColorPipelineParamNumber(row, "shape.frequency", &frequency, outError) ||
                !TryGetColorPipelineParamNumber(row, "shape.phase", &phase, outError) ||
                !ValidateColorPipelineParamRange("shape.frequency", frequency, 0.25, 24.0, outError) ||
                !ValidateColorPipelineParamRange("shape.phase", phase, -1.0, 1.0, outError)) {
                return false;
            }
            if (ioParams->color_shape != ColorPipelineShape::repeat) {
                ioParams->color_shape = ColorPipelineShape::repeat;
                changed = true;
            }
            resetShapeOffsetScale();
            assignShapeFloat(&ioParams->color_shape_repeat_frequency, static_cast<float>(frequency));
            assignShapeFloat(&ioParams->color_shape_repeat_phase, static_cast<float>(phase));
            resetShapePosterize();
            resetShapeBiasGain();
            resetShapeSmoothWindow();
            continue;
        }
        if (row.function_id == "posterize") {
            int steps = 0;
            double mix = 0.0;
            if (!TryReadColorPipelineIntegerParam(row, "shape.steps", &steps, outError) ||
                !TryGetColorPipelineParamNumber(row, "shape.mix", &mix, outError) ||
                !ValidateColorPipelineParamRange("shape.steps", static_cast<double>(steps), 2.0, 24.0, outError) ||
                !ValidateColorPipelineParamRange("shape.mix", mix, 0.0, 1.0, outError)) {
                return false;
            }
            if (ioParams->color_shape != ColorPipelineShape::posterize) {
                ioParams->color_shape = ColorPipelineShape::posterize;
                changed = true;
            }
            resetShapeOffsetScale();
            resetShapeRepeat();
            assignShapeInt(&ioParams->color_shape_posterize_steps, steps);
            assignShapeFloat(&ioParams->color_shape_posterize_mix, static_cast<float>(mix));
            resetShapeBiasGain();
            resetShapeSmoothWindow();
            continue;
        }
        if (row.function_id == "mirror_repeat") {
            double frequency = 0.0;
            double phase = 0.0;
            if (!TryGetColorPipelineParamNumber(row, "shape.frequency", &frequency, outError) ||
                !TryGetColorPipelineParamNumber(row, "shape.phase", &phase, outError) ||
                !ValidateColorPipelineParamRange("shape.frequency", frequency, 0.25, 24.0, outError) ||
                !ValidateColorPipelineParamRange("shape.phase", phase, -1.0, 1.0, outError)) {
                return false;
            }
            if (ioParams->color_shape != ColorPipelineShape::mirror_repeat) {
                ioParams->color_shape = ColorPipelineShape::mirror_repeat;
                changed = true;
            }
            resetShapeOffsetScale();
            resetShapePosterize();
            assignShapeFloat(&ioParams->color_shape_repeat_frequency, static_cast<float>(frequency));
            assignShapeFloat(&ioParams->color_shape_repeat_phase, static_cast<float>(phase));
            resetShapeBiasGain();
            resetShapeSmoothWindow();
            continue;
        }
        if (row.function_id == "bias_gain_curve") {
            double bias = 0.0;
            double gain = 0.0;
            if (!TryGetColorPipelineParamNumber(row, "shape.bias", &bias, outError) ||
                !TryGetColorPipelineParamNumber(row, "shape.gain", &gain, outError) ||
                !ValidateColorPipelineParamRange("shape.bias", bias, 0.0, 1.0, outError) ||
                !ValidateColorPipelineParamRange("shape.gain", gain, 0.0, 1.0, outError)) {
                return false;
            }
            if (ioParams->color_shape != ColorPipelineShape::bias_gain_curve) {
                ioParams->color_shape = ColorPipelineShape::bias_gain_curve;
                changed = true;
            }
            resetShapeOffsetScale();
            resetShapeRepeat();
            resetShapePosterize();
            assignShapeFloat(&ioParams->color_shape_bias, static_cast<float>(bias));
            assignShapeFloat(&ioParams->color_shape_gain, static_cast<float>(gain));
            resetShapeSmoothWindow();
            continue;
        }
        if (row.function_id == "smooth_window") {
            double center = 0.0;
            double width = 0.0;
            double softness = 0.0;
            if (!TryGetColorPipelineParamNumber(row, "shape.center", &center, outError) ||
                !TryGetColorPipelineParamNumber(row, "shape.width", &width, outError) ||
                !TryGetColorPipelineParamNumber(row, "shape.softness", &softness, outError) ||
                !ValidateColorPipelineParamRange("shape.center", center, 0.0, 1.0, outError) ||
                !ValidateColorPipelineParamRange("shape.width", width, 0.0, 1.0, outError) ||
                !ValidateColorPipelineParamRange("shape.softness", softness, 0.0, 1.0, outError)) {
                return false;
            }
            if (ioParams->color_shape != ColorPipelineShape::smooth_window) {
                ioParams->color_shape = ColorPipelineShape::smooth_window;
                changed = true;
            }
            resetShapeOffsetScale();
            resetShapeRepeat();
            resetShapePosterize();
            resetShapeBiasGain();
            assignShapeFloat(&ioParams->color_shape_window_center, static_cast<float>(center));
            assignShapeFloat(&ioParams->color_shape_window_width, static_cast<float>(width));
            assignShapeFloat(&ioParams->color_shape_window_softness, static_cast<float>(softness));
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
            resetPaletteHeatmap();
            resetPalettePhaseWheel();
            resetPaletteExplaino();
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

inline bool TryBuildColorPipelineSelectionFromLaneIds(
    const char* sourceFunctionId,
    const char* paletteFunctionId,
    ColorPipelineSelection* outPipeline,
    ColoringMode* outMode) {
    if (!sourceFunctionId || sourceFunctionId[0] == '\0' ||
        !paletteFunctionId || paletteFunctionId[0] == '\0' ||
        !outPipeline || !outMode) {
        return false;
    }

    if (std::strcmp(sourceFunctionId, "smooth_escape_ramp") == 0 && std::strcmp(paletteFunctionId, "heatmap") == 0) {
        *outPipeline = {ColorSignal::smooth_escape, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
        *outMode = ColoringMode::smooth_escape;
        return true;
    }
    if (std::strcmp(sourceFunctionId, "smooth_escape_ramp") == 0 && std::strcmp(paletteFunctionId, "explaino_cmap") == 0) {
        *outPipeline = {ColorSignal::smooth_escape, ColorPalette::explaino_cmap, ColorGradingPreset::escape_default};
        *outMode = ColoringMode::smooth_escape;
        return true;
    }
    if (std::strcmp(sourceFunctionId, "phase_orbit") == 0 && std::strcmp(paletteFunctionId, "phase_wheel_palette") == 0) {
        *outPipeline = {ColorSignal::phase_angle, ColorPalette::phase_wheel, ColorGradingPreset::phase_default};
        *outMode = ColoringMode::phase;
        return true;
    }
    if (std::strcmp(sourceFunctionId, "banded_signal") == 0 && std::strcmp(paletteFunctionId, "banded_heatmap") == 0) {
        *outPipeline = {ColorSignal::iteration_bands, ColorPalette::banded_escape, ColorGradingPreset::bands_default};
        *outMode = ColoringMode::iteration_bands;
        return true;
    }
    if (std::strcmp(sourceFunctionId, "escape_magnitude") == 0 && std::strcmp(paletteFunctionId, "heatmap") == 0) {
        *outPipeline = {ColorSignal::escape_magnitude, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
        *outMode = ColoringMode::smooth_escape;
        return true;
    }
    if (std::strcmp(sourceFunctionId, "escape_magnitude") == 0 && std::strcmp(paletteFunctionId, "explaino_cmap") == 0) {
        *outPipeline = {ColorSignal::escape_magnitude, ColorPalette::explaino_cmap, ColorGradingPreset::escape_default};
        *outMode = ColoringMode::smooth_escape;
        return true;
    }
    if (std::strcmp(sourceFunctionId, "orbit_stripe") == 0 && std::strcmp(paletteFunctionId, "phase_wheel_palette") == 0) {
        *outPipeline = {ColorSignal::orbit_stripe, ColorPalette::phase_wheel, ColorGradingPreset::phase_default};
        *outMode = ColoringMode::phase;
        return true;
    }
    if (std::strcmp(sourceFunctionId, "root_proximity") == 0 && std::strcmp(paletteFunctionId, "heatmap") == 0) {
        *outPipeline = {ColorSignal::root_proximity, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
        *outMode = ColoringMode::smooth_escape;
        return true;
    }
    if (std::strcmp(sourceFunctionId, "root_proximity") == 0 && std::strcmp(paletteFunctionId, "explaino_cmap") == 0) {
        *outPipeline = {ColorSignal::root_proximity, ColorPalette::explaino_cmap, ColorGradingPreset::escape_default};
        *outMode = ColoringMode::smooth_escape;
        return true;
    }
    if (std::strcmp(sourceFunctionId, "root_index") == 0 && std::strcmp(paletteFunctionId, "root_classic_palette") == 0) {
        *outPipeline = {ColorSignal::root_index, ColorPalette::root_classic, ColorGradingPreset::basin_default};
        *outMode = ColoringMode::root_basin;
        return true;
    }

    return false;
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

    if (shapeRow->function_id != "identity" &&
        shapeRow->function_id != "offset_scale" &&
        shapeRow->function_id != "repeat" &&
        shapeRow->function_id != "posterize" &&
        shapeRow->function_id != "mirror_repeat" &&
        shapeRow->function_id != "bias_gain_curve" &&
        shapeRow->function_id != "smooth_window") {
        if (outError) {
            *outError = "Current live bridge only supports the Identity, Offset + Scale, Repeat, Posterize, Mirror Repeat, Bias + Gain Curve, and Smooth Window Shape rows; stacked or remapped Shape recipes stay draft-only until custom runtime integration lands.";
        }
        return false;
    }

    ColorPipelineSelection pipeline{};
    ColoringMode mode = ColoringMode::root_basin;
    if (!TryBuildColorPipelineSelectionFromLaneIds(
            sourceRow->function_id.c_str(),
            paletteRow->function_id.c_str(),
            &pipeline,
            &mode)) {
        if (outError) {
            *outError = "Selected Source / Shape / Palette recipe is draft-only until custom pipeline runtime integration lands or you choose a matching supported Source / Palette pair.";
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

inline void NoteColorPipelineInteractionSnapshot(
    bool changed,
    bool itemActivated,
    bool itemActive,
    bool itemDeactivatedAfterEdit,
    ColorPipelineRenderInteractionState* ioState) {
    if (!ioState) {
        return;
    }
    if (changed || itemActivated || itemActive || itemDeactivatedAfterEdit) {
        ioState->interacted = true;
        ioState->has_active_item = ioState->has_active_item || itemActive;
    }
}

#ifndef COLOR_PIPELINE_WINDOW_NO_IMGUI
inline void NoteColorPipelineCurrentItemInteraction(
    bool changed,
    ColorPipelineRenderInteractionState* ioState) {
    NoteColorPipelineInteractionSnapshot(
        changed,
        ImGui::IsItemActivated(),
        ImGui::IsItemActive(),
        ImGui::IsItemDeactivatedAfterEdit(),
        ioState);
}

inline bool ShouldAutoApplySupportedColorPipelineDraft(
    const ColorPipelineWindowState& state,
    const ColorPipelineDraftApplyState& applyState,
    const ColorPipelineRenderInteractionState& interactionState,
    const KernelParams* liveParams = nullptr) {
    (void)state;
    return liveParams &&
        applyState.status == ColorPipelineDraftApplyStatus::can_apply &&
        interactionState.interacted &&
        !interactionState.has_active_item;
}

inline bool TryApplySupportedColorPipelineDraftFromControl(
    ColorPipelineWindowState* ioState,
    FractalType liveFractalType,
    KernelParams* liveParams,
    bool* ioDirty,
    ColorPipelineRenderInteractionState* ioInteraction) {
    if (!ioState || !liveParams) {
        return false;
    }
    const ColorPipelineDraftApplyState applyState = DescribeColorPipelineDraftApplyState(*ioState, liveFractalType, liveParams);
    if (applyState.status != ColorPipelineDraftApplyStatus::can_apply) {
        return false;
    }

    bool changed = false;
    if (!ApplyColorPipelineDraftToLiveState(ioState, liveFractalType, liveParams, &changed)) {
        return false;
    }
    if (changed && ioDirty) {
        *ioDirty = true;
    }
    if (changed && ioInteraction) {
        ioInteraction->interacted = true;
    }
    return changed;
}

inline bool RenderColorPipelineParamControl(
    ColorPipelineWindowState* ioState,
    FractalType liveFractalType,
    KernelParams* liveParams,
    const FunctionParamDescriptor& param,
    ColorPipelineParamState* ioValue,
    bool* ioDirty = nullptr,
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
        bool sliderChanged = false;
        if (range.has_widget_min && range.has_widget_max) {
            sliderChanged = ImGui::SliderFloat(param.label.c_str(), &value, minValue, maxValue, "%.5f");
        } else {
            const float step = param.has_step ? static_cast<float>(param.step_value) : 0.01f;
            const float dragMin = dragBounds.has_bounds ? static_cast<float>(dragBounds.min) : 0.0f;
            const float dragMax = dragBounds.has_bounds ? static_cast<float>(dragBounds.max) : 0.0f;
            sliderChanged = ImGui::DragFloat(param.label.c_str(), &value, step, dragMin, dragMax, "%.3f");
        }
        NoteColorPipelineCurrentItemInteraction(sliderChanged, ioInteraction);
        ImGui::SameLine();
        const bool typedChanged = ImGui::InputFloat("##value_input", &value, 0.0f, 0.0f, "%.5f");
        changed = sliderChanged || typedChanged;
        if (changed) {
            ClampColorPipelineNumericValue(&value, range);
            ioValue->number_value = value;
            TryApplySupportedColorPipelineDraftFromControl(ioState, liveFractalType, liveParams, ioDirty, ioInteraction);
        }
        NoteColorPipelineCurrentItemInteraction(typedChanged, ioInteraction);
    } else if (param.type == "double") {
        double value = ioValue->number_value;
        const NumericControlRange range = ResolveColorPipelineNumericControlRange(param);
        const NumericDragWidgetBounds dragBounds = ResolveColorPipelineNumericDragWidgetBounds(param);
        const double minValue = range.has_widget_min ? range.widget_min : 0.0;
        const double maxValue = range.has_widget_max ? range.widget_max : 1.0;
        bool sliderChanged = false;
        if (range.has_widget_min && range.has_widget_max) {
            sliderChanged = ImGui::SliderScalar(param.label.c_str(), ImGuiDataType_Double, &value, &minValue, &maxValue, "%.6f");
        } else {
            const double* dragMin = dragBounds.has_bounds ? &dragBounds.min : nullptr;
            const double* dragMax = dragBounds.has_bounds ? &dragBounds.max : nullptr;
            const double step = param.has_step ? param.step_value : 0.001;
            sliderChanged = ImGui::DragScalar(param.label.c_str(), ImGuiDataType_Double, &value, static_cast<float>(step), dragMin, dragMax, "%.6f");
        }
        NoteColorPipelineCurrentItemInteraction(sliderChanged, ioInteraction);
        ImGui::SameLine();
        const bool typedChanged = ImGui::InputDouble("##value_input", &value, 0.0, 0.0, "%.6f");
        changed = sliderChanged || typedChanged;
        if (changed) {
            ClampColorPipelineNumericValue(&value, range);
            ioValue->number_value = value;
            TryApplySupportedColorPipelineDraftFromControl(ioState, liveFractalType, liveParams, ioDirty, ioInteraction);
        }
        NoteColorPipelineCurrentItemInteraction(typedChanged, ioInteraction);
    } else if (param.type == "int") {
        int value = static_cast<int>(std::lround(ioValue->number_value));
        const NumericControlRange range = ResolveColorPipelineNumericControlRange(param);
        const NumericDragWidgetBounds dragBounds = ResolveColorPipelineNumericDragWidgetBounds(param);
        const int minValue = range.has_widget_min ? static_cast<int>(range.widget_min) : 0;
        const int maxValue = range.has_widget_max ? static_cast<int>(range.widget_max) : 100;
        bool sliderChanged = false;
        if (range.has_widget_min && range.has_widget_max) {
            sliderChanged = ImGui::SliderInt(param.label.c_str(), &value, minValue, maxValue);
        } else {
            const float step = param.has_step ? static_cast<float>(param.step_value) : 1.0f;
            const int dragMin = dragBounds.has_bounds ? static_cast<int>(dragBounds.min) : 0;
            const int dragMax = dragBounds.has_bounds ? static_cast<int>(dragBounds.max) : 0;
            sliderChanged = ImGui::DragInt(param.label.c_str(), &value, step, dragMin, dragMax);
        }
        NoteColorPipelineCurrentItemInteraction(sliderChanged, ioInteraction);
        ImGui::SameLine();
        const bool typedChanged = ImGui::InputInt("##value_input", &value, 0, 0);
        changed = sliderChanged || typedChanged;
        if (changed) {
            ClampColorPipelineNumericValue(&value, range);
            ioValue->number_value = static_cast<double>(value);
            TryApplySupportedColorPipelineDraftFromControl(ioState, liveFractalType, liveParams, ioDirty, ioInteraction);
        }
        NoteColorPipelineCurrentItemInteraction(typedChanged, ioInteraction);
    } else if (param.type == "bool") {
        bool value = ioValue->bool_value;
        changed = ImGui::Checkbox(param.label.c_str(), &value);
        NoteColorPipelineCurrentItemInteraction(changed, ioInteraction);
        if (changed) {
            ioValue->bool_value = value;
            TryApplySupportedColorPipelineDraftFromControl(ioState, liveFractalType, liveParams, ioDirty, ioInteraction);
        }
    } else if (param.type == "enum") {
        const char* preview = ioValue->enum_value.empty() ? "(select)" : ioValue->enum_value.c_str();
        if (ImGui::BeginCombo(param.label.c_str(), preview)) {
            for (const UISchemaOption& option : param.options) {
                const bool isSelected = (option.id == ioValue->enum_value);
                if (ImGui::Selectable(option.label.c_str(), isSelected)) {
                    ioValue->enum_value = option.id;
                    changed = true;
                    TryApplySupportedColorPipelineDraftFromControl(ioState, liveFractalType, liveParams, ioDirty, ioInteraction);
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
    (void)ioInteraction;
    ImGui::TextWrapped("Draft Source / Shape / Palette recipes here. The legacy Color mode and grading controls stay in the main Color panel during the schedule-editor transition.");
    ImGui::TextDisabled("This window now models three typed lane stacks instead of a fixed Signal / Palette / Grade trio.");
    ImGui::TextDisabled("Current live apply bridge supports one enabled Source row, one enabled Palette row, and one live-backed Shape row (Identity, Offset + Scale, Repeat, Posterize, Mirror Repeat, Bias + Gain Curve, or Smooth Window).");
    ImGui::TextDisabled("Some Source / Palette rows are fixed presets with no tunable parameters; choosing the row is itself the live change.");
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
                ImGui::TextDisabled("Supported live bridge recipes right now: Smooth Escape / Escape Magnitude / Root Proximity with Heatmap or Explaino Cmap, Phase Orbit / Orbit Stripe with Phase Wheel, Iteration Bands with Banded Heatmap, and Root Index with Root Classic Palette.");
            }
        }
        if (!ioState->live_snapshot.valid || !ioState->live_snapshot.draft_import_supported) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Reset Draft From Live")) {
            ResetColorPipelineDraftFromLiveState(ioState);
        }
        if (!ioState->live_snapshot.valid || !ioState->live_snapshot.draft_import_supported) {
            ImGui::EndDisabled();
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
    KernelParams* liveParams,
    bool* ioDirty = nullptr,
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
                RenderColorPipelineParamControl(ioState, liveFractalType, liveParams, currentDescriptor->parameters[paramIndex], &row.parameter_values[paramIndex], ioDirty, ioInteraction);
            }
            if (currentDescriptor->parameters.empty() &&
                (lane.lane_id == "source" || lane.lane_id == "palette")) {
                ImGui::TextDisabled("This fixed %s row has no tunable parameters; choosing it changes the live bridge tuple directly.", lane.label.c_str());
            } else if (!currentDescriptor->parameters.empty() && renderableParamIndexes.empty()) {
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
            RenderColorPipelineWindowLane(ioState, laneIndex, liveFractalType, liveParams, ioDirty, &interactionState);
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
#endif
