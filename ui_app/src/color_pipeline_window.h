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

struct ColorPipelineWindowState {
    bool open = false;
    bool initialized = false;
    bool force_open_for_automation = false;
    std::uint64_t next_row_id = 1;
    std::vector<ColorPipelineLaneState> lanes;
    ColorPipelineLiveSnapshot live_snapshot;
    std::vector<std::string> validation_messages;
    std::vector<struct ColorPipelineUiAutomationRect> ui_automation_rects;
};

struct ColorPipelineUiAutomationRect {
    std::string control_id;
    int client_left = 0;
    int client_top = 0;
    int client_right = 0;
    int client_bottom = 0;
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

inline ColorPipelineDraftApplyState DescribeColorPipelineDraftApplyState(
    const ColorPipelineWindowState& state,
    FractalType liveFractalType,
    const KernelParams* liveParams);

inline void ClearColorPipelineUiAutomationRects(ColorPipelineWindowState* ioState) {
    if (!ioState) {
        return;
    }
    ioState->ui_automation_rects.clear();
}

inline std::string BuildColorPipelinePrimaryControlId(
    const std::string& laneId,
    const std::string& functionId,
    const FunctionParamDescriptor& param) {
    return std::string("color_pipeline.") + laneId + "." + functionId + "." + param.path + ".primary";
}

#ifndef COLOR_PIPELINE_WINDOW_NO_IMGUI
inline void NoteColorPipelineUiAutomationRect(
    ColorPipelineWindowState* ioState,
    const char* controlId) {
    if (!ioState || !controlId || controlId[0] == '\0') {
        return;
    }
    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    if (max.x <= min.x || max.y <= min.y) {
        return;
    }
    ColorPipelineUiAutomationRect rect;
    rect.control_id = controlId;
    rect.client_left = static_cast<int>(std::lround(min.x));
    rect.client_top = static_cast<int>(std::lround(min.y));
    rect.client_right = static_cast<int>(std::lround(max.x));
    rect.client_bottom = static_cast<int>(std::lround(max.y));
    ioState->ui_automation_rects.push_back(std::move(rect));
}
#else
inline void NoteColorPipelineUiAutomationRect(
    ColorPipelineWindowState* ioState,
    const char* controlId) {
    (void)ioState;
    (void)controlId;
}
#endif

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

inline const char* ColorPaletteBlendModeId(ColorPaletteBlendMode value) {
    return color_pipeline_core::ColorPaletteBlendModeId(value);
}

inline bool TryParseColorPaletteBlendModeId(const std::string& id, ColorPaletteBlendMode* outValue) {
    return color_pipeline_core::TryParseColorPaletteBlendModeId(id, outValue);
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
    return color_pipeline_core::ResolveColorPipelineNumericDefault(param);
}

inline bool ResolveColorPipelineBoolDefault(const FunctionParamDescriptor& param) {
    return color_pipeline_core::ResolveColorPipelineBoolDefault(param);
}

inline std::string ResolveColorPipelineEnumDefault(const FunctionParamDescriptor& param) {
    return color_pipeline_core::ResolveColorPipelineEnumDefault(param);
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
    return color_pipeline_core::SetColorPipelineRowFunction(ioRow, descriptor);
}

inline bool BuildColorPipelineRowFromFunctionId(
    const ColorPipelineLaneCatalog& catalog,
    const char* functionId,
    std::uint64_t stableRowId,
    ColorPipelineRowState* outRow,
    std::string* outError = nullptr) {
    return color_pipeline_core::BuildColorPipelineRowFromFunctionId(
        catalog,
        functionId,
        stableRowId,
        outRow,
        outError);
}

inline bool BuildColorPipelineLaneWithSingleRow(
    const ColorPipelineLaneCatalog& catalog,
    const char* functionId,
    std::uint64_t stableRowId,
    ColorPipelineLaneState* outLane,
    std::string* outError = nullptr) {
    return color_pipeline_core::BuildColorPipelineLaneWithSingleRow(
        catalog,
        functionId,
        stableRowId,
        outLane,
        outError);
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

inline bool ImportSupportedColorPipelineParamsFromShapeStackEntry(
    ColorPipelineRowState* ioRow,
    const ColorPipelineShapeStackEntry& shapeEntry,
    std::string* outError = nullptr);

inline bool ImportSupportedColorPipelineParamsFromGradingStackEntry(
    ColorPipelineRowState* ioRow,
    const ColorPipelineGradingStackEntry& gradingEntry,
    std::string* outError = nullptr);

inline bool CollectEnabledColorPipelineRows(
    const ColorPipelineWindowState& state,
    const char* laneId,
    std::vector<const ColorPipelineRowState*>* outRows,
    std::string* outError = nullptr);

inline bool IsSupportedColorPipelineShapeFunctionId(const std::string& functionId);

inline const ColorPipelineLaneState* FindColorPipelineLaneState(
    const ColorPipelineWindowState& state,
    const char* laneId);

inline bool IsSupportedColorPipelineGradingFunctionId(const std::string& functionId);

inline bool ColorPipelineShapeRuntimeParamsEqual(
    const ColorPipelineShapeRuntimeParams& left,
    const ColorPipelineShapeRuntimeParams& right);

inline bool ColorPipelineShapeStackEntriesEqual(
    const ColorPipelineShapeStackEntry& left,
    const ColorPipelineShapeStackEntry& right);

inline bool ColorPipelineGradingStackEntriesEqual(
    const ColorPipelineGradingStackEntry& left,
    const ColorPipelineGradingStackEntry& right);

inline bool TryBuildColorPipelineShapeStackEntryFromRow(
    const ColorPipelineRowState& row,
    ColorPipelineShapeStackEntry* outEntry,
    std::string* outError = nullptr);

inline bool TryBuildColorPipelineShapeLaneFromLive(
    const KernelParams& liveParams,
    ColorPipelineLaneState* outLane,
    bool* outDraftImportSupported,
    std::string* outError = nullptr);

inline bool TryBuildColorPipelineGradingLaneFromLive(
    const KernelParams& liveParams,
    ColorPipelineLaneState* outLane,
    bool* outDraftImportSupported,
    std::string* outError = nullptr);

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
    const std::vector<ColorPipelineLaneState> previousLanes = ioState->lanes;
    if (ioState->lanes.size() != ioState->live_snapshot.lanes.size()) {
        ioState->lanes = ioState->live_snapshot.lanes;
    } else {
        for (std::size_t index = 0; index < ioState->lanes.size(); ++index) {
            ioState->lanes[index] = ioState->live_snapshot.lanes[index];
        }
    }

    const std::size_t sharedLaneCount = (std::min)(ioState->lanes.size(), previousLanes.size());
    for (std::size_t laneIndex = 0; laneIndex < sharedLaneCount; ++laneIndex) {
        ColorPipelineLaneState& lane = ioState->lanes[laneIndex];
        const ColorPipelineLaneState& previousLane = previousLanes[laneIndex];
        if (lane.lane_id != previousLane.lane_id) {
            continue;
        }
        const std::size_t sharedRowCount = (std::min)(lane.rows.size(), previousLane.rows.size());
        for (std::size_t rowIndex = 0; rowIndex < sharedRowCount; ++rowIndex) {
            lane.rows[rowIndex].ui_row_id = previousLane.rows[rowIndex].ui_row_id;
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

inline bool TryBuildRootBasinPairLanesFromLive(
    const KernelParams& liveParams,
    ColorPipelineLaneState* outSourceLane,
    ColorPipelineLaneState* outPaletteLane,
    bool* outDraftImportSupported,
    std::string* outError = nullptr);

inline bool HasCoherentRootBasinPairSchedule(const KernelParams& liveParams);

inline bool TryBuildColorPipelinePaletteLaneFromLive(
    const KernelParams& liveParams,
    ColorPipelineLaneState* outLane,
    bool* outDraftImportSupported,
    std::string* outError = nullptr);

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

    ColorPipelineLiveSnapshot snapshot;
    snapshot.valid = true;
    snapshot.fractal_type = liveFractalType;
    snapshot.coloring_mode = liveParams.coloring_mode;
    snapshot.pipeline = liveParams.color_pipeline;

    const char* gradingFunctionId = AdvancedColorGradingFunctionId(liveParams.color_pipeline.grading);
    const ColorPipelineLaneCatalog* gradingCatalog = FindColorPipelineLaneCatalog("grading");
    if (gradingFunctionId && gradingFunctionId[0] != '\0') {
        if (!gradingCatalog) {
            if (outError) *outError = "Missing advanced color Grading lane catalog";
            return false;
        }
        if (!FindColorPipelineFunctionDescriptor(*gradingCatalog, gradingFunctionId)) {
            snapshot.draft_import_supported = false;
            *outSnapshot = std::move(snapshot);
            return true;
        }
    }

    const ColorPipelineLaneCatalog* sourceCatalog = FindColorPipelineLaneCatalog("source");
    const ColorPipelineLaneCatalog* paletteCatalog = FindColorPipelineLaneCatalog("palette");
    if (!sourceCatalog || !paletteCatalog) {
        if (outError) *outError = "Missing advanced color Source or Palette lane catalog";
        return false;
    }

    ColorPipelineLaneState sourceLane;
    ColorPipelineLaneState paletteLane;
    bool sourcePaletteDraftImportSupported = true;
    if (HasCoherentRootBasinPairSchedule(liveParams)) {
        if (!TryBuildRootBasinPairLanesFromLive(
                liveParams,
                &sourceLane,
                &paletteLane,
                &sourcePaletteDraftImportSupported,
                outError)) {
            return false;
        }
    } else {
        const char* sourceFunctionId = nullptr;
        const char* paletteFunctionId = nullptr;
        sourcePaletteDraftImportSupported = TryBuildColorPipelineScheduleBridgeIds(
            liveParams.color_pipeline,
            &sourceFunctionId,
            &paletteFunctionId);
        if (!sourcePaletteDraftImportSupported) {
            *outSnapshot = std::move(snapshot);
            return true;
        }
        if (!BuildColorPipelineLaneWithSingleRow(*sourceCatalog, sourceFunctionId, 0, &sourceLane, outError) ||
            !ImportSupportedColorPipelineParamsFromLive(&sourceLane.rows.front(), liveParams, outError)) {
            return false;
        }
        if (!TryBuildColorPipelinePaletteLaneFromLive(
                liveParams,
                &paletteLane,
                &sourcePaletteDraftImportSupported,
                outError)) {
            return false;
        }
    }

    ColorPipelineLaneState shapeLane;
    bool shapeDraftImportSupported = true;
    if (!TryBuildColorPipelineShapeLaneFromLive(liveParams, &shapeLane, &shapeDraftImportSupported, outError)) {
        return false;
    }

    ColorPipelineLaneState gradingLane;
    bool gradingDraftImportSupported = true;
    bool hasGradingLane = false;
    if (gradingFunctionId && gradingFunctionId[0] != '\0') {
        if (!TryBuildColorPipelineGradingLaneFromLive(liveParams, &gradingLane, &gradingDraftImportSupported, outError)) {
            return false;
        }
        hasGradingLane = true;
    }

    snapshot.draft_import_supported = sourcePaletteDraftImportSupported && shapeDraftImportSupported && gradingDraftImportSupported;
    snapshot.lanes.push_back(std::move(sourceLane));
    snapshot.lanes.push_back(std::move(shapeLane));
    snapshot.lanes.push_back(std::move(paletteLane));
    if (hasGradingLane) {
        snapshot.lanes.push_back(std::move(gradingLane));
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
    ioState->live_snapshot = std::move(nextSnapshot);

    const ColorPipelineDraftApplyState applyState = DescribeColorPipelineDraftApplyState(*ioState, liveFractalType, liveParams);
    const bool draftDisallowedForFamily =
        applyState.status == ColorPipelineDraftApplyStatus::disallowed_for_family;
    const bool adoptIntoDraft =
        !liveSnapshotWasValid ||
        (liveSnapshotChanged && !draftHasEdits) ||
        (liveSnapshotChanged && !liveSnapshotWasImportSupported && draftMatchesStarter) ||
        draftDisallowedForFamily;
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
                } else if (lane.rows[rowIndex].function_id == "joy_root_palette") {
                    companionLaneId = "source";
                    companionFunctionId = "root_index";
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

        ColorPipelineLaneState* editableSourceLane = nullptr;
        ColorPipelineLaneState* editablePaletteLane = nullptr;
        ColorPipelineLaneState* gradingLane = nullptr;
        for (ColorPipelineLaneState& candidateLane : ioState->lanes) {
            if (candidateLane.lane_id == "source") {
                editableSourceLane = &candidateLane;
            } else if (candidateLane.lane_id == "palette") {
                editablePaletteLane = &candidateLane;
            } else if (candidateLane.lane_id == "grading") {
                gradingLane = &candidateLane;
            }
        }

        if (editableSourceLane && editablePaletteLane && gradingLane &&
            editableSourceLane->rows.size() == 1 &&
            editablePaletteLane->rows.size() == 1 &&
            gradingLane->rows.size() == 1) {
            ColorPipelineSelection supportedPipeline{};
            ColoringMode supportedMode = ColoringMode::root_basin;
            if (TryBuildColorPipelineSelectionFromLaneIds(
                    editableSourceLane->rows[0].function_id.c_str(),
                    editablePaletteLane->rows[0].function_id.c_str(),
                    &supportedPipeline,
                    &supportedMode)) {
                const char* requiredGradingFunctionId = AdvancedColorGradingFunctionId(supportedPipeline.grading);
                if (requiredGradingFunctionId && requiredGradingFunctionId[0] != '\0') {
                    const ColorPipelineLaneCatalog* gradingCatalog = FindColorPipelineLaneCatalog(gradingLane->lane_id);
                    if (gradingCatalog) {
                        const FunctionDescriptor* gradingDescriptor = FindColorPipelineFunctionDescriptor(*gradingCatalog, requiredGradingFunctionId);
                        if (gradingDescriptor && gradingLane->rows[0].function_id != requiredGradingFunctionId) {
                            SetColorPipelineRowFunction(&gradingLane->rows[0], *gradingDescriptor);
                        }
                    }
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
        return path == "palette.cycle_scale" || path == "palette.saturation" ||
            path == "palette.blend_weight" || path == "palette.blend_mode";
    }
    if (functionId == "contrast_lift") {
        return path == "grade.exposure" || path == "grade.saturation";
    }
    if (functionId == "phase_finish") {
        return path == "grade.saturation" || path == "grade.contrast";
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
        return path == "palette.phase_offset" || path == "palette.saturation" ||
            path == "palette.blend_weight" || path == "palette.blend_mode";
    }
    if (functionId == "explaino_cmap") {
        return path == "palette.seed_scale" || path == "palette.seed_phase" || path == "palette.colorfulness" ||
            path == "palette.blend_weight" || path == "palette.blend_mode";
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
        return path == "palette.band_emphasis" || path == "palette.phase_offset" ||
            path == "palette.blend_weight" || path == "palette.blend_mode";
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
    return color_pipeline_core::TryGetColorPipelineParamNumber(row, path, outValue, outError);
}

inline bool SetColorPipelineParamNumber(
    ColorPipelineRowState* ioRow,
    const char* path,
    double value,
    std::string* outError = nullptr) {
    return color_pipeline_core::SetColorPipelineParamNumber(ioRow, path, value, outError);
}

inline bool TryGetColorPipelineParamEnum(
    const ColorPipelineRowState& row,
    const char* path,
    std::string* outValue,
    std::string* outError = nullptr) {
    return color_pipeline_core::TryGetColorPipelineParamEnum(row, path, outValue, outError);
}

inline bool ImportSupportedColorPipelineParamsFromLive(
    ColorPipelineRowState* ioRow,
    const KernelParams& liveParams,
    std::string* outError) {
    return color_pipeline_core::ImportSupportedColorPipelineParamsFromLive(ioRow, liveParams, outError);
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

inline bool ImportSupportedColorPipelineParamsFromShapeStackEntry(
    ColorPipelineRowState* ioRow,
    const ColorPipelineShapeStackEntry& shapeEntry,
    std::string* outError) {
    if (!ioRow) {
        if (outError) *outError = "Advanced color Shape-stack import requires a row";
        return false;
    }
    if (ioRow->function_id == "offset_scale") {
        return SetColorPipelineParamNumber(ioRow, "shape.offset", shapeEntry.params.offset, outError) &&
            SetColorPipelineParamNumber(ioRow, "shape.scale", shapeEntry.params.scale, outError);
    }
    if (ioRow->function_id == "repeat") {
        return SetColorPipelineParamNumber(ioRow, "shape.frequency", shapeEntry.params.repeat_frequency, outError) &&
            SetColorPipelineParamNumber(ioRow, "shape.phase", shapeEntry.params.repeat_phase, outError);
    }
    if (ioRow->function_id == "posterize") {
        return SetColorPipelineParamNumber(ioRow, "shape.steps", static_cast<double>(shapeEntry.params.posterize_steps), outError) &&
            SetColorPipelineParamNumber(ioRow, "shape.mix", shapeEntry.params.posterize_mix, outError);
    }
    if (ioRow->function_id == "mirror_repeat") {
        return SetColorPipelineParamNumber(ioRow, "shape.frequency", shapeEntry.params.repeat_frequency, outError) &&
            SetColorPipelineParamNumber(ioRow, "shape.phase", shapeEntry.params.repeat_phase, outError);
    }
    if (ioRow->function_id == "bias_gain_curve") {
        return SetColorPipelineParamNumber(ioRow, "shape.bias", shapeEntry.params.bias, outError) &&
            SetColorPipelineParamNumber(ioRow, "shape.gain", shapeEntry.params.gain, outError);
    }
    if (ioRow->function_id == "smooth_window") {
        return SetColorPipelineParamNumber(ioRow, "shape.center", shapeEntry.params.window_center, outError) &&
            SetColorPipelineParamNumber(ioRow, "shape.width", shapeEntry.params.window_width, outError) &&
            SetColorPipelineParamNumber(ioRow, "shape.softness", shapeEntry.params.window_softness, outError);
    }
    return true;
}

inline bool CollectEnabledColorPipelineRows(
    const ColorPipelineWindowState& state,
    const char* laneId,
    std::vector<const ColorPipelineRowState*>* outRows,
    std::string* outError) {
    if (!outRows) {
        if (outError) *outError = "Advanced color row collection requires output storage";
        return false;
    }
    outRows->clear();
    const ColorPipelineLaneState* lane = nullptr;
    for (const ColorPipelineLaneState& candidateLane : state.lanes) {
        if (candidateLane.lane_id == laneId) {
            lane = &candidateLane;
            break;
        }
    }
    if (!lane) {
        if (outError) *outError = std::string("Unknown advanced color pipeline lane id: ") + (laneId ? laneId : "");
        return false;
    }
    for (const ColorPipelineRowState& row : lane->rows) {
        if (row.enabled) {
            outRows->push_back(&row);
        }
    }
    if (outRows->empty()) {
        if (outError) {
            *outError = std::string("Current live bridge requires one enabled row in the ") + lane->label + " lane.";
        }
        return false;
    }
    return true;
}

inline bool IsSupportedColorPipelineShapeFunctionId(const std::string& functionId) {
    return functionId == "identity" ||
        functionId == "offset_scale" ||
        functionId == "repeat" ||
        functionId == "posterize" ||
        functionId == "mirror_repeat" ||
        functionId == "bias_gain_curve" ||
        functionId == "smooth_window";
}

inline bool ColorPipelineShapeRuntimeParamsEqual(
    const ColorPipelineShapeRuntimeParams& left,
    const ColorPipelineShapeRuntimeParams& right) {
    return std::fabs(left.offset - right.offset) <= 1.0e-6f &&
        std::fabs(left.scale - right.scale) <= 1.0e-6f &&
        std::fabs(left.repeat_frequency - right.repeat_frequency) <= 1.0e-6f &&
        std::fabs(left.repeat_phase - right.repeat_phase) <= 1.0e-6f &&
        left.posterize_steps == right.posterize_steps &&
        std::fabs(left.posterize_mix - right.posterize_mix) <= 1.0e-6f &&
        std::fabs(left.bias - right.bias) <= 1.0e-6f &&
        std::fabs(left.gain - right.gain) <= 1.0e-6f &&
        std::fabs(left.window_center - right.window_center) <= 1.0e-6f &&
        std::fabs(left.window_width - right.window_width) <= 1.0e-6f &&
        std::fabs(left.window_softness - right.window_softness) <= 1.0e-6f;
}

inline bool ColorPipelineShapeStackEntriesEqual(
    const ColorPipelineShapeStackEntry& left,
    const ColorPipelineShapeStackEntry& right) {
    return left.shape == right.shape &&
        ColorPipelineShapeRuntimeParamsEqual(left.params, right.params);
}

inline bool IsSupportedColorPipelineGradingFunctionId(const std::string& functionId) {
    return functionId == "contrast_lift" ||
        functionId == "phase_finish" ||
        functionId == "band_finish";
}

inline bool ColorPipelineGradingRuntimeParamsEqual(
    const ColorPipelineGradingRuntimeParams& left,
    const ColorPipelineGradingRuntimeParams& right) {
    return std::fabs(left.exposure - right.exposure) <= 1.0e-6f &&
        std::fabs(left.saturation - right.saturation) <= 1.0e-6f &&
        std::fabs(left.contrast - right.contrast) <= 1.0e-6f;
}

inline bool ColorPipelineGradingStackEntriesEqual(
    const ColorPipelineGradingStackEntry& left,
    const ColorPipelineGradingStackEntry& right) {
    return left.grading == right.grading &&
        ColorPipelineGradingRuntimeParamsEqual(left.params, right.params);
}


inline bool IsSupportedColorPipelinePaletteStackFunctionId(const std::string& functionId) {
    return functionId == "heatmap" ||
        functionId == "phase_wheel_palette" ||
        functionId == "banded_heatmap" ||
        functionId == "explaino_cmap";
}

inline bool ColorPipelinePaletteRuntimeParamsEqual(
    const ColorPipelinePaletteRuntimeParams& left,
    const ColorPipelinePaletteRuntimeParams& right) {
    return std::fabs(left.cycle_scale - right.cycle_scale) <= 1.0e-6f &&
        std::fabs(left.saturation - right.saturation) <= 1.0e-6f &&
        std::fabs(left.phase_offset - right.phase_offset) <= 1.0e-6f &&
        std::fabs(left.band_emphasis - right.band_emphasis) <= 1.0e-6f &&
        std::fabs(left.seed_scale - right.seed_scale) <= 1.0e-6f &&
        std::fabs(left.seed_phase - right.seed_phase) <= 1.0e-6f &&
        std::fabs(left.colorfulness - right.colorfulness) <= 1.0e-6f &&
        std::fabs(left.blend_weight - right.blend_weight) <= 1.0e-6f &&
        left.blend_mode == right.blend_mode;
}

inline bool ColorPipelinePaletteStackEntriesEqual(
    const ColorPipelinePaletteStackEntry& left,
    const ColorPipelinePaletteStackEntry& right) {
    return left.palette == right.palette &&
        ColorPipelinePaletteRuntimeParamsEqual(left.params, right.params);
}

inline bool TryBuildColorPipelinePaletteStackEntryFromRow(
    const ColorPipelineRowState& row,
    ColorPipelinePaletteStackEntry* outEntry,
    std::string* outError) {
    if (!outEntry) {
        if (outError) *outError = "Advanced color Palette-stack apply requires output storage";
        return false;
    }
    if (!IsSupportedColorPipelinePaletteStackFunctionId(row.function_id)) {
        if (outError) {
            *outError = "Current live bridge only supports Heatmap, Phase Wheel, Banded Heatmap, and ExplainO CMap in the Palette stack.";
        }
        return false;
    }

    ColorPalette palette = ColorPalette::cyclic_escape;
    if (!TryParseAdvancedColorPaletteFunctionId(row.function_id.c_str(), &palette)) {
        if (outError) {
            *outError = std::string("Unknown advanced color Palette row id: ") + row.function_id;
        }
        return false;
    }

    ColorPipelinePaletteStackEntry entry;
    entry.palette = palette;
    double blendWeight = entry.params.blend_weight;
    std::string blendModeId;
    if (!TryGetColorPipelineParamNumber(row, "palette.blend_weight", &blendWeight, outError) ||
        !TryGetColorPipelineParamEnum(row, "palette.blend_mode", &blendModeId, outError) ||
        !ValidateColorPipelineParamRange("palette.blend_weight", blendWeight, 0.0, 1.0, outError) ||
        !TryParseColorPaletteBlendModeId(blendModeId, &entry.params.blend_mode)) {
        if (outError && outError->empty()) {
            *outError = std::string("Unknown advanced color Palette blend mode: ") + blendModeId;
        }
        return false;
    }
    entry.params.blend_weight = static_cast<float>(blendWeight);
    if (row.function_id == "heatmap") {
        double cycleScale = 0.0;
        double saturation = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "palette.cycle_scale", &cycleScale, outError) ||
            !TryGetColorPipelineParamNumber(row, "palette.saturation", &saturation, outError) ||
            !ValidateColorPipelineParamRange("palette.cycle_scale", cycleScale, 0.25, 4.0, outError) ||
            !ValidateColorPipelineParamRange("palette.saturation", saturation, 0.0, 2.0, outError)) {
            return false;
        }
        entry.params.cycle_scale = static_cast<float>(cycleScale);
        entry.params.saturation = static_cast<float>(saturation);
    } else if (row.function_id == "phase_wheel_palette") {
        double phaseOffset = 0.0;
        double saturation = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "palette.phase_offset", &phaseOffset, outError) ||
            !TryGetColorPipelineParamNumber(row, "palette.saturation", &saturation, outError) ||
            !ValidateColorPipelineParamRange("palette.phase_offset", phaseOffset, -3.141592653589793, 3.141592653589793, outError) ||
            !ValidateColorPipelineParamRange("palette.saturation", saturation, 0.0, 2.0, outError)) {
            return false;
        }
        entry.params.phase_offset = static_cast<float>(phaseOffset);
        entry.params.saturation = static_cast<float>(saturation);
    } else if (row.function_id == "banded_heatmap") {
        double emphasis = 0.0;
        double phaseOffset = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "palette.band_emphasis", &emphasis, outError) ||
            !TryGetColorPipelineParamNumber(row, "palette.phase_offset", &phaseOffset, outError) ||
            !ValidateColorPipelineParamRange("palette.band_emphasis", emphasis, 0.0, 2.0, outError) ||
            !ValidateColorPipelineParamRange("palette.phase_offset", phaseOffset, -3.141592653589793, 3.141592653589793, outError)) {
            return false;
        }
        entry.params.band_emphasis = static_cast<float>(emphasis);
        entry.params.phase_offset = static_cast<float>(phaseOffset);
    } else if (row.function_id == "explaino_cmap") {
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
        entry.params.seed_scale = static_cast<float>(seedScale);
        entry.params.seed_phase = static_cast<float>(seedPhase);
        entry.params.colorfulness = static_cast<float>(colorfulness);
    }

    *outEntry = entry;
    return true;
}

inline bool ImportSupportedColorPipelineParamsFromPaletteStackEntry(
    ColorPipelineRowState* ioRow,
    const ColorPipelinePaletteStackEntry& paletteEntry,
    std::string* outError) {
    if (!ioRow) {
        if (outError) *outError = "Advanced color Palette-stack import requires a row";
        return false;
    }
    const char* functionId = AdvancedColorPaletteFunctionId(paletteEntry.palette);
    if (!functionId || ioRow->function_id != functionId) {
        if (outError) *outError = "Advanced color Palette-stack import row does not match the saved palette entry";
        return false;
    }
    const char* blendModeId = ColorPaletteBlendModeId(paletteEntry.params.blend_mode);
    if (!blendModeId ||
        !color_pipeline_core::SetColorPipelineParamNumber(ioRow, "palette.blend_weight", paletteEntry.params.blend_weight, outError) ||
        !color_pipeline_core::SetColorPipelineParamEnum(ioRow, "palette.blend_mode", blendModeId, outError)) {
        return false;
    }
    if (ioRow->function_id == "heatmap") {
        return SetColorPipelineParamNumber(ioRow, "palette.cycle_scale", paletteEntry.params.cycle_scale, outError) &&
            SetColorPipelineParamNumber(ioRow, "palette.saturation", paletteEntry.params.saturation, outError);
    }
    if (ioRow->function_id == "phase_wheel_palette") {
        return SetColorPipelineParamNumber(ioRow, "palette.phase_offset", paletteEntry.params.phase_offset, outError) &&
            SetColorPipelineParamNumber(ioRow, "palette.saturation", paletteEntry.params.saturation, outError);
    }
    if (ioRow->function_id == "banded_heatmap") {
        return SetColorPipelineParamNumber(ioRow, "palette.band_emphasis", paletteEntry.params.band_emphasis, outError) &&
            SetColorPipelineParamNumber(ioRow, "palette.phase_offset", paletteEntry.params.phase_offset, outError);
    }
    if (ioRow->function_id == "explaino_cmap") {
        return SetColorPipelineParamNumber(ioRow, "palette.seed_scale", paletteEntry.params.seed_scale, outError) &&
            SetColorPipelineParamNumber(ioRow, "palette.seed_phase", paletteEntry.params.seed_phase, outError) &&
            SetColorPipelineParamNumber(ioRow, "palette.colorfulness", paletteEntry.params.colorfulness, outError);
    }
    return true;
}

inline bool IsSupportedRootBasinPaletteFunctionId(const std::string& functionId) {
    return functionId == "root_classic_palette" ||
        functionId == "joy_root_palette";
}

inline ColorPipelineLaneState* FindMutableColorPipelineLaneState(
    ColorPipelineWindowState* ioState,
    const char* laneId) {
    if (!ioState || !laneId || laneId[0] == '\0') {
        return nullptr;
    }
    for (ColorPipelineLaneState& lane : ioState->lanes) {
        if (lane.lane_id == laneId) {
            return &lane;
        }
    }
    return nullptr;
}

inline std::size_t CountEnabledColorPipelineLaneRows(const ColorPipelineLaneState& lane) {
    std::size_t count = 0;
    for (const ColorPipelineRowState& row : lane.rows) {
        if (row.enabled) {
            ++count;
        }
    }
    return count;
}

inline bool IsRootBasinPairRowAtIndex(
    const ColorPipelineLaneState& sourceLane,
    const ColorPipelineLaneState& paletteLane,
    std::size_t rowIndex) {
    return rowIndex < sourceLane.rows.size() &&
        rowIndex < paletteLane.rows.size() &&
        sourceLane.rows[rowIndex].function_id == "root_index" &&
        IsSupportedRootBasinPaletteFunctionId(paletteLane.rows[rowIndex].function_id);
}

inline std::size_t CountEnabledRootBasinPairRows(
    const ColorPipelineLaneState& sourceLane,
    const ColorPipelineLaneState& paletteLane) {
    const std::size_t rowCount = (std::min)(sourceLane.rows.size(), paletteLane.rows.size());
    std::size_t count = 0;
    for (std::size_t rowIndex = 0; rowIndex < rowCount; ++rowIndex) {
        if (sourceLane.rows[rowIndex].enabled &&
            paletteLane.rows[rowIndex].enabled &&
            IsRootBasinPairRowAtIndex(sourceLane, paletteLane, rowIndex)) {
            ++count;
        }
    }
    return count;
}

inline bool SetColorPipelineRowEnabledFromUi(
    ColorPipelineWindowState* ioState,
    std::size_t laneIndex,
    std::size_t rowIndex,
    bool enabled) {
    if (!ioState || laneIndex >= ioState->lanes.size()) {
        return false;
    }
    ColorPipelineLaneState& lane = ioState->lanes[laneIndex];
    if (rowIndex >= lane.rows.size()) {
        return false;
    }

    bool changed = lane.rows[rowIndex].enabled != enabled;
    lane.rows[rowIndex].enabled = enabled;

    if (lane.lane_id == "source" || lane.lane_id == "palette") {
        ColorPipelineLaneState* sourceLane = FindMutableColorPipelineLaneState(ioState, "source");
        ColorPipelineLaneState* paletteLane = FindMutableColorPipelineLaneState(ioState, "palette");
        if (sourceLane && paletteLane && IsRootBasinPairRowAtIndex(*sourceLane, *paletteLane, rowIndex)) {
            ColorPipelineRowState& sourceRow = sourceLane->rows[rowIndex];
            ColorPipelineRowState& paletteRow = paletteLane->rows[rowIndex];
            if (sourceRow.enabled != enabled) {
                sourceRow.enabled = enabled;
                changed = true;
            }
            if (paletteRow.enabled != enabled) {
                paletteRow.enabled = enabled;
                changed = true;
            }
            if (!enabled && CountEnabledRootBasinPairRows(*sourceLane, *paletteLane) == 0) {
                sourceRow.enabled = true;
                paletteRow.enabled = true;
                PushColorPipelineValidationMessage(ioState,
                    "At least one root-basin Source / Palette preset pair must stay enabled.");
                return true;
            }
            return changed;
        }
    }

    if (!enabled && CountEnabledColorPipelineLaneRows(lane) == 0) {
        lane.rows[rowIndex].enabled = true;
        PushColorPipelineValidationMessage(ioState,
            std::string("At least one ") + lane.label + " row must stay enabled.");
        return true;
    }
    return changed;
}

inline bool IsRootBasinPairCandidateRows(
    const std::vector<const ColorPipelineRowState*>& sourceRows,
    const std::vector<const ColorPipelineRowState*>& paletteRows) {
    if (sourceRows.empty() || paletteRows.empty() || sourceRows.size() != paletteRows.size()) {
        return false;
    }
    for (const ColorPipelineRowState* row : sourceRows) {
        if (!row || row->function_id != "root_index") {
            return false;
        }
    }
    for (const ColorPipelineRowState* row : paletteRows) {
        if (!row || !IsSupportedRootBasinPaletteFunctionId(row->function_id)) {
            return false;
        }
    }
    return true;
}

inline bool TryBuildRootBasinPairSelectionsFromRows(
    const std::vector<const ColorPipelineRowState*>& sourceRows,
    const std::vector<const ColorPipelineRowState*>& paletteRows,
    std::vector<ColorPipelineSelection>* outSelections,
    ColoringMode* outFinalMode,
    std::string* outError) {
    if (!outSelections || !outFinalMode) {
        if (outError) *outError = "Advanced color root-basin pair build requires output storage";
        return false;
    }
    outSelections->clear();
    if (sourceRows.empty() || paletteRows.empty() || sourceRows.size() != paletteRows.size()) {
        if (outError) {
            *outError = "Current live bridge only supports row-indexed root-basin schedules when enabled Source and Palette row counts match.";
        }
        return false;
    }
    if (sourceRows.size() > static_cast<std::size_t>(kColorPipelineMaxRootBasinPairCount)) {
        if (outError) {
            *outError = "Current live bridge only supports a bounded number of enabled root-basin Source / Palette pairs.";
        }
        return false;
    }
    for (std::size_t index = 0; index < sourceRows.size(); ++index) {
        const ColorPipelineRowState* sourceRow = sourceRows[index];
        const ColorPipelineRowState* paletteRow = paletteRows[index];
        if (!sourceRow || !paletteRow || sourceRow->function_id != "root_index" || !IsSupportedRootBasinPaletteFunctionId(paletteRow->function_id)) {
            if (outError) {
                *outError = "Current live bridge only supports row-indexed root-basin pairs of root_index with root_classic_palette or joy_root_palette.";
            }
            return false;
        }
        ColorPipelineSelection selection{};
        ColoringMode mode = ColoringMode::root_basin;
        if (!color_pipeline_core::TryBuildColorPipelineSelectionFromLaneIds(
                sourceRow->function_id.c_str(),
                paletteRow->function_id.c_str(),
                &selection,
                &mode)) {
            if (outError) {
                *outError = "Selected root-basin pair is not runtime-backed by the current Source / Palette bridge.";
            }
            return false;
        }
        outSelections->push_back(selection);
        *outFinalMode = mode;
    }
    return !outSelections->empty();
}

inline bool HasCoherentRootBasinPairSchedule(const KernelParams& liveParams) {
    if (liveParams.color_root_basin_pair_count <= 0 ||
        liveParams.color_root_basin_pair_count > kColorPipelineMaxRootBasinPairCount) {
        return false;
    }
    const ColorPipelineSelection& activePair =
        liveParams.color_root_basin_pairs[liveParams.color_root_basin_pair_count - 1];
    if (!ColorPipelineSelectionsEqual(activePair, liveParams.color_pipeline)) {
        return false;
    }
    const char* sourceFunctionId = nullptr;
    const char* paletteFunctionId = nullptr;
    return TryBuildColorPipelineScheduleBridgeIds(activePair, &sourceFunctionId, &paletteFunctionId) &&
        sourceFunctionId &&
        paletteFunctionId &&
        std::strcmp(sourceFunctionId, "root_index") == 0 &&
        IsSupportedRootBasinPaletteFunctionId(paletteFunctionId);
}

inline bool TryBuildRootBasinPairLanesFromLive(
    const KernelParams& liveParams,
    ColorPipelineLaneState* outSourceLane,
    ColorPipelineLaneState* outPaletteLane,
    bool* outDraftImportSupported,
    std::string* outError) {
    if (!outSourceLane || !outPaletteLane || !outDraftImportSupported) {
        if (outError) *outError = "Advanced color root-basin pair snapshot requires output storage";
        return false;
    }

    const ColorPipelineLaneCatalog* sourceCatalog = FindColorPipelineLaneCatalog("source");
    const ColorPipelineLaneCatalog* paletteCatalog = FindColorPipelineLaneCatalog("palette");
    if (!sourceCatalog || !paletteCatalog) {
        if (outError) *outError = "Missing advanced color Source or Palette lane catalog";
        return false;
    }

    ColorPipelineLaneState sourceLane;
    sourceLane.lane_id = sourceCatalog->lane_id;
    sourceLane.label = sourceCatalog->label;
    ColorPipelineLaneState paletteLane;
    paletteLane.lane_id = paletteCatalog->lane_id;
    paletteLane.label = paletteCatalog->label;
    *outDraftImportSupported = true;

    int pairCount = liveParams.color_root_basin_pair_count;
    if (pairCount > kColorPipelineMaxRootBasinPairCount) {
        pairCount = kColorPipelineMaxRootBasinPairCount;
    }
    for (int index = 0; index < pairCount; ++index) {
        const ColorPipelineSelection& pairSelection = liveParams.color_root_basin_pairs[index];
        const char* sourceFunctionId = nullptr;
        const char* paletteFunctionId = nullptr;
        if (!TryBuildColorPipelineScheduleBridgeIds(pairSelection, &sourceFunctionId, &paletteFunctionId) ||
            !sourceFunctionId ||
            !paletteFunctionId ||
            std::strcmp(sourceFunctionId, "root_index") != 0 ||
            !IsSupportedRootBasinPaletteFunctionId(paletteFunctionId)) {
            *outDraftImportSupported = false;
            *outSourceLane = std::move(sourceLane);
            *outPaletteLane = std::move(paletteLane);
            return true;
        }

        ColorPipelineRowState sourceRow;
        if (!BuildColorPipelineRowFromFunctionId(*sourceCatalog, sourceFunctionId, index, &sourceRow, outError) ||
            !ImportSupportedColorPipelineParamsFromLive(&sourceRow, liveParams, outError)) {
            return false;
        }
        sourceLane.rows.push_back(std::move(sourceRow));

        ColorPipelineRowState paletteRow;
        if (!BuildColorPipelineRowFromFunctionId(*paletteCatalog, paletteFunctionId, index, &paletteRow, outError) ||
            !ImportSupportedColorPipelineParamsFromLive(&paletteRow, liveParams, outError)) {
            return false;
        }
        paletteLane.rows.push_back(std::move(paletteRow));
    }

    *outSourceLane = std::move(sourceLane);
    *outPaletteLane = std::move(paletteLane);
    return true;
}


inline bool TryBuildColorPipelinePaletteLaneFromLive(
    const KernelParams& liveParams,
    ColorPipelineLaneState* outLane,
    bool* outDraftImportSupported,
    std::string* outError) {
    if (!outLane || !outDraftImportSupported) {
        if (outError) *outError = "Advanced color Palette lane snapshot requires output storage";
        return false;
    }
    const ColorPipelineLaneCatalog* catalog = FindColorPipelineLaneCatalog("palette");
    if (!catalog) {
        if (outError) *outError = "Missing advanced color Palette lane catalog";
        return false;
    }

    ColorPipelineLaneState lane;
    lane.lane_id = catalog->lane_id;
    lane.label = catalog->label;
    *outDraftImportSupported = true;

    int paletteStackCount = liveParams.color_palette_stack_count;
    if (paletteStackCount > kColorPipelineMaxPaletteStackCount) {
        paletteStackCount = kColorPipelineMaxPaletteStackCount;
    }
    if (paletteStackCount > 0) {
        for (int index = 0; index < paletteStackCount; ++index) {
            const ColorPipelinePaletteStackEntry& paletteEntry = liveParams.color_palette_stack[index];
            const char* functionId = AdvancedColorPaletteFunctionId(paletteEntry.palette);
            if (!functionId || !IsSupportedColorPipelinePaletteStackFunctionId(functionId)) {
                *outDraftImportSupported = false;
                *outLane = std::move(lane);
                return true;
            }
            ColorPipelineRowState row;
            if (!BuildColorPipelineRowFromFunctionId(*catalog, functionId, index, &row, outError) ||
                !ImportSupportedColorPipelineParamsFromPaletteStackEntry(&row, paletteEntry, outError)) {
                return false;
            }
            lane.rows.push_back(std::move(row));
        }
        if (lane.rows.empty()) {
            *outDraftImportSupported = false;
        }
        *outLane = std::move(lane);
        return true;
    }

    const char* functionId = AdvancedColorPaletteFunctionId(liveParams.color_pipeline.palette);
    if (!functionId) {
        *outDraftImportSupported = false;
        *outLane = std::move(lane);
        return true;
    }
    if (IsSupportedRootBasinPaletteFunctionId(functionId)) {
        ColorPipelineRowState row;
        if (!BuildColorPipelineRowFromFunctionId(*catalog, functionId, 0, &row, outError) ||
            !ImportSupportedColorPipelineParamsFromLive(&row, liveParams, outError)) {
            return false;
        }
        lane.rows.push_back(std::move(row));
        *outLane = std::move(lane);
        return true;
    }
    ColorPipelinePaletteStackEntry livePaletteEntry;
    livePaletteEntry.palette = liveParams.color_pipeline.palette;
    livePaletteEntry.params.cycle_scale = liveParams.color_heatmap_cycle_scale;
    livePaletteEntry.params.saturation = liveParams.color_heatmap_saturation;
    livePaletteEntry.params.phase_offset = liveParams.color_phase_palette_offset;
    livePaletteEntry.params.band_emphasis = liveParams.color_iteration_band_emphasis;
    if (livePaletteEntry.palette == ColorPalette::banded_escape) {
        livePaletteEntry.params.phase_offset = liveParams.color_iteration_band_palette_offset;
    }
    livePaletteEntry.params.seed_scale = liveParams.color_explaino_palette_seed_scale;
    livePaletteEntry.params.seed_phase = liveParams.color_explaino_palette_seed_phase;
    livePaletteEntry.params.colorfulness = liveParams.color_explaino_palette_colorfulness;
    ColorPipelineRowState row;
    if (!BuildColorPipelineRowFromFunctionId(*catalog, functionId, 0, &row, outError) ||
        !ImportSupportedColorPipelineParamsFromPaletteStackEntry(&row, livePaletteEntry, outError)) {
        return false;
    }
    lane.rows.push_back(std::move(row));
    *outLane = std::move(lane);
    return true;
}

inline bool TryBuildColorPipelineGradingStackEntryFromRow(
    const ColorPipelineRowState& row,
    ColorPipelineGradingStackEntry* outEntry,
    std::string* outError) {
    if (!outEntry) {
        if (outError) *outError = "Advanced color Grading-stack apply requires output storage";
        return false;
    }
    if (!IsSupportedColorPipelineGradingFunctionId(row.function_id)) {
        if (outError) {
            *outError = "Current live bridge only supports contrast_lift, phase_finish, and band_finish in the Grading stack.";
        }
        return false;
    }

    ColorGradingPreset grading = ColorGradingPreset::escape_default;
    if (!TryParseAdvancedColorGradingFunctionId(row.function_id.c_str(), &grading)) {
        if (outError) {
            *outError = std::string("Unknown advanced color Grading row id: ") + row.function_id;
        }
        return false;
    }

    ColorPipelineGradingStackEntry entry;
    entry.grading = grading;
    if (row.function_id == "contrast_lift") {
        double exposure = 0.0;
        double saturation = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "grade.exposure", &exposure, outError) ||
            !TryGetColorPipelineParamNumber(row, "grade.saturation", &saturation, outError) ||
            !ValidateColorPipelineParamRange("grade.exposure", exposure, 0.1, 3.0, outError) ||
            !ValidateColorPipelineParamRange("grade.saturation", saturation, 0.0, 2.0, outError)) {
            return false;
        }
        entry.params.exposure = static_cast<float>(exposure);
        entry.params.saturation = static_cast<float>(saturation);
        entry.params.contrast = 1.0f;
    } else if (row.function_id == "phase_finish" || row.function_id == "band_finish") {
        double saturation = 0.0;
        double contrast = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "grade.saturation", &saturation, outError) ||
            !TryGetColorPipelineParamNumber(row, "grade.contrast", &contrast, outError) ||
            !ValidateColorPipelineParamRange("grade.saturation", saturation, 0.0, 2.0, outError) ||
            !ValidateColorPipelineParamRange("grade.contrast", contrast, 0.0, 3.0, outError)) {
            return false;
        }
        entry.params.exposure = 1.0f;
        entry.params.saturation = static_cast<float>(saturation);
        entry.params.contrast = static_cast<float>(contrast);
    }

    *outEntry = entry;
    return true;
}

inline bool TryBuildColorPipelineShapeStackEntryFromRow(
    const ColorPipelineRowState& row,
    ColorPipelineShapeStackEntry* outEntry,
    std::string* outError) {
    if (!outEntry) {
        if (outError) *outError = "Advanced color Shape-stack apply requires output storage";
        return false;
    }
    if (!IsSupportedColorPipelineShapeFunctionId(row.function_id)) {
        if (outError) {
            *outError = "Current live bridge only supports the Identity, Offset + Scale, Repeat, Posterize, Mirror Repeat, Bias + Gain Curve, and Smooth Window Shape rows; stacked or remapped Shape recipes stay draft-only until custom runtime integration lands.";
        }
        return false;
    }

    ColorPipelineShape shape = ColorPipelineShape::identity;
    if (!TryParseColorPipelineShapeId(row.function_id, &shape)) {
        if (outError) {
            *outError = std::string("Unknown advanced color Shape row id: ") + row.function_id;
        }
        return false;
    }

    ColorPipelineShapeStackEntry entry;
    entry.shape = shape;
    if (row.function_id == "offset_scale") {
        double offset = 0.0;
        double scale = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "shape.offset", &offset, outError) ||
            !TryGetColorPipelineParamNumber(row, "shape.scale", &scale, outError) ||
            !ValidateColorPipelineParamRange("shape.offset", offset, -2.0, 2.0, outError) ||
            !ValidateColorPipelineParamRange("shape.scale", scale, 0.1, 8.0, outError)) {
            return false;
        }
        entry.params.offset = static_cast<float>(offset);
        entry.params.scale = static_cast<float>(scale);
    } else if (row.function_id == "repeat" || row.function_id == "mirror_repeat") {
        double frequency = 0.0;
        double phase = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "shape.frequency", &frequency, outError) ||
            !TryGetColorPipelineParamNumber(row, "shape.phase", &phase, outError) ||
            !ValidateColorPipelineParamRange("shape.frequency", frequency, 0.25, 24.0, outError) ||
            !ValidateColorPipelineParamRange("shape.phase", phase, -1.0, 1.0, outError)) {
            return false;
        }
        entry.params.repeat_frequency = static_cast<float>(frequency);
        entry.params.repeat_phase = static_cast<float>(phase);
    } else if (row.function_id == "posterize") {
        int steps = 0;
        double mix = 0.0;
        if (!TryReadColorPipelineIntegerParam(row, "shape.steps", &steps, outError) ||
            !TryGetColorPipelineParamNumber(row, "shape.mix", &mix, outError) ||
            !ValidateColorPipelineParamRange("shape.steps", static_cast<double>(steps), 2.0, 24.0, outError) ||
            !ValidateColorPipelineParamRange("shape.mix", mix, 0.0, 1.0, outError)) {
            return false;
        }
        entry.params.posterize_steps = steps;
        entry.params.posterize_mix = static_cast<float>(mix);
    } else if (row.function_id == "bias_gain_curve") {
        double bias = 0.0;
        double gain = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "shape.bias", &bias, outError) ||
            !TryGetColorPipelineParamNumber(row, "shape.gain", &gain, outError) ||
            !ValidateColorPipelineParamRange("shape.bias", bias, 0.0, 1.0, outError) ||
            !ValidateColorPipelineParamRange("shape.gain", gain, 0.0, 1.0, outError)) {
            return false;
        }
        entry.params.bias = static_cast<float>(bias);
        entry.params.gain = static_cast<float>(gain);
    } else if (row.function_id == "smooth_window") {
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
        entry.params.window_center = static_cast<float>(center);
        entry.params.window_width = static_cast<float>(width);
        entry.params.window_softness = static_cast<float>(softness);
    }

    *outEntry = entry;
    return true;
}

inline bool ImportSupportedColorPipelineParamsFromGradingStackEntry(
    ColorPipelineRowState* ioRow,
    const ColorPipelineGradingStackEntry& gradingEntry,
    std::string* outError) {
    if (!ioRow) {
        if (outError) *outError = "Advanced color Grading-stack import requires a row";
        return false;
    }
    const char* functionId = AdvancedColorGradingFunctionId(gradingEntry.grading);
    if (!functionId || ioRow->function_id != functionId) {
        if (outError) *outError = "Advanced color Grading-stack import row does not match the saved grading entry";
        return false;
    }
    if (ioRow->function_id == "contrast_lift") {
        return SetColorPipelineParamNumber(ioRow, "grade.exposure", gradingEntry.params.exposure, outError) &&
            SetColorPipelineParamNumber(ioRow, "grade.saturation", gradingEntry.params.saturation, outError);
    }
    if (ioRow->function_id == "phase_finish" || ioRow->function_id == "band_finish") {
        return SetColorPipelineParamNumber(ioRow, "grade.saturation", gradingEntry.params.saturation, outError) &&
            SetColorPipelineParamNumber(ioRow, "grade.contrast", gradingEntry.params.contrast, outError);
    }
    return true;
}

inline bool TryBuildColorPipelineGradingLaneFromLive(
    const KernelParams& liveParams,
    ColorPipelineLaneState* outLane,
    bool* outDraftImportSupported,
    std::string* outError) {
    if (!outLane || !outDraftImportSupported) {
        if (outError) *outError = "Advanced color Grading lane snapshot requires output storage";
        return false;
    }
    const ColorPipelineLaneCatalog* catalog = FindColorPipelineLaneCatalog("grading");
    if (!catalog) {
        if (outError) *outError = "Missing advanced color Grading lane catalog";
        return false;
    }

    ColorPipelineLaneState lane;
    lane.lane_id = catalog->lane_id;
    lane.label = catalog->label;
    *outDraftImportSupported = true;

    int gradingStackCount = liveParams.color_grading_stack_count;
    if (gradingStackCount > kColorPipelineMaxGradingStackCount) {
        gradingStackCount = kColorPipelineMaxGradingStackCount;
    }
    if (gradingStackCount > 0) {
        for (int index = 0; index < gradingStackCount; ++index) {
            const ColorPipelineGradingStackEntry& gradingEntry = liveParams.color_grading_stack[index];
            const char* functionId = AdvancedColorGradingFunctionId(gradingEntry.grading);
            if (!functionId || !IsSupportedColorPipelineGradingFunctionId(functionId)) {
                *outDraftImportSupported = false;
                *outLane = std::move(lane);
                return true;
            }
            ColorPipelineRowState row;
            if (!BuildColorPipelineRowFromFunctionId(*catalog, functionId, index, &row, outError) ||
                !ImportSupportedColorPipelineParamsFromGradingStackEntry(&row, gradingEntry, outError)) {
                return false;
            }
            lane.rows.push_back(std::move(row));
        }
        if (lane.rows.empty()) {
            *outDraftImportSupported = false;
        }
        *outLane = std::move(lane);
        return true;
    }

    const char* functionId = AdvancedColorGradingFunctionId(liveParams.color_pipeline.grading);
    if (!functionId || !IsSupportedColorPipelineGradingFunctionId(functionId)) {
        *outDraftImportSupported = false;
        *outLane = std::move(lane);
        return true;
    }
    ColorPipelineGradingStackEntry liveGradingEntry;
    liveGradingEntry.grading = liveParams.color_pipeline.grading;
    if (liveGradingEntry.grading == ColorGradingPreset::escape_default) {
        liveGradingEntry.params.exposure = liveParams.color_contrast_lift_exposure;
        liveGradingEntry.params.saturation = liveParams.color_contrast_lift_saturation;
        liveGradingEntry.params.contrast = 1.0f;
    } else {
        liveGradingEntry.params.exposure = 1.0f;
        liveGradingEntry.params.saturation = liveParams.color_saturation;
        liveGradingEntry.params.contrast = liveParams.color_contrast;
    }
    ColorPipelineRowState row;
    if (!BuildColorPipelineRowFromFunctionId(*catalog, functionId, 0, &row, outError) ||
        !ImportSupportedColorPipelineParamsFromGradingStackEntry(&row, liveGradingEntry, outError)) {
        return false;
    }
    lane.rows.push_back(std::move(row));
    *outLane = std::move(lane);
    return true;
}

inline bool TryBuildColorPipelineShapeLaneFromLive(
    const KernelParams& liveParams,
    ColorPipelineLaneState* outLane,
    bool* outDraftImportSupported,
    std::string* outError) {
    if (!outLane || !outDraftImportSupported) {
        if (outError) *outError = "Advanced color Shape lane snapshot requires output storage";
        return false;
    }
    const ColorPipelineLaneCatalog* catalog = FindColorPipelineLaneCatalog("shape");
    if (!catalog) {
        if (outError) *outError = "Missing advanced color Shape lane catalog";
        return false;
    }

    ColorPipelineLaneState lane;
    lane.lane_id = catalog->lane_id;
    lane.label = catalog->label;
    *outDraftImportSupported = true;

    int shapeStackCount = liveParams.color_shape_stack_count;
    if (shapeStackCount > kColorPipelineMaxShapeStackCount) {
        shapeStackCount = kColorPipelineMaxShapeStackCount;
    }
    if (shapeStackCount > 0) {
        for (int index = 0; index < shapeStackCount; ++index) {
            const ColorPipelineShapeStackEntry& shapeEntry = liveParams.color_shape_stack[index];
            const char* functionId = AdvancedColorShapeFunctionId(shapeEntry.shape);
            if (!functionId) {
                *outDraftImportSupported = false;
                *outLane = std::move(lane);
                return true;
            }
            ColorPipelineRowState row;
            if (!BuildColorPipelineRowFromFunctionId(*catalog, functionId, 0, &row, outError) ||
                !ImportSupportedColorPipelineParamsFromShapeStackEntry(&row, shapeEntry, outError)) {
                return false;
            }
            lane.rows.push_back(std::move(row));
        }
        if (lane.rows.empty()) {
            *outDraftImportSupported = false;
        }
        *outLane = std::move(lane);
        return true;
    }

    const char* functionId = AdvancedColorShapeFunctionId(liveParams.color_shape);
    if (!functionId) {
        *outDraftImportSupported = false;
        *outLane = std::move(lane);
        return true;
    }
    ColorPipelineRowState row;
    ColorPipelineShapeStackEntry liveShapeEntry;
    liveShapeEntry.shape = liveParams.color_shape;
    liveShapeEntry.params.offset = liveParams.color_shape_offset;
    liveShapeEntry.params.scale = liveParams.color_shape_scale;
    liveShapeEntry.params.repeat_frequency = liveParams.color_shape_repeat_frequency;
    liveShapeEntry.params.repeat_phase = liveParams.color_shape_repeat_phase;
    liveShapeEntry.params.posterize_steps = liveParams.color_shape_posterize_steps;
    liveShapeEntry.params.posterize_mix = liveParams.color_shape_posterize_mix;
    liveShapeEntry.params.bias = liveParams.color_shape_bias;
    liveShapeEntry.params.gain = liveParams.color_shape_gain;
    liveShapeEntry.params.window_center = liveParams.color_shape_window_center;
    liveShapeEntry.params.window_width = liveParams.color_shape_window_width;
    liveShapeEntry.params.window_softness = liveParams.color_shape_window_softness;
    if (!BuildColorPipelineRowFromFunctionId(*catalog, functionId, 0, &row, outError) ||
        !ImportSupportedColorPipelineParamsFromShapeStackEntry(&row, liveShapeEntry, outError)) {
        return false;
    }
    lane.rows.push_back(std::move(row));
    *outLane = std::move(lane);
    return true;
}

inline bool ApplySupportedColorPipelineParamsToLive(
    const ColorPipelineWindowState& state,
    KernelParams* ioParams,
    bool* outChanged = nullptr,
    std::string* outError = nullptr,
    const ColorPipelineSelection* targetPipeline = nullptr) {
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

    std::vector<const ColorPipelineRowState*> sourceRowsForStacks;
    if (!CollectEnabledColorPipelineRows(state, "source", &sourceRowsForStacks, outError)) {
        return false;
    }
    std::vector<const ColorPipelineRowState*> paletteRows;
    if (!CollectEnabledColorPipelineRows(state, "palette", &paletteRows, outError)) {
        return false;
    }
    const bool rootBasinPairSchedule = IsRootBasinPairCandidateRows(sourceRowsForStacks, paletteRows);

    std::vector<const ColorPipelineRowState*> shapeRows;
    if (!CollectEnabledColorPipelineRows(state, "shape", &shapeRows, outError)) {
        return false;
    }
    if (shapeRows.size() > static_cast<std::size_t>(kColorPipelineMaxShapeStackCount)) {
        if (outError) {
            *outError = "Current live bridge only supports a bounded number of enabled Shape rows in the schedule lane.";
        }
        return false;
    }

    std::vector<const ColorPipelineRowState*> gradingRows;
    const ColorPipelineSelection& effectivePipeline = targetPipeline ? *targetPipeline : ioParams->color_pipeline;
    const bool targetUsesGradingLane = AdvancedColorGradingFunctionId(effectivePipeline.grading) != nullptr;
    const ColorPipelineLaneState* gradingLane = FindColorPipelineLaneState(state, "grading");
    if (targetUsesGradingLane && gradingLane && !CollectEnabledColorPipelineRows(state, "grading", &gradingRows, outError)) {
        return false;
    }
    if (gradingRows.size() > static_cast<std::size_t>(kColorPipelineMaxGradingStackCount)) {
        if (outError) {
            *outError = "Current live bridge only supports a bounded number of enabled Grading rows in the schedule lane.";
        }
        return false;
    }

    std::vector<ColorPipelineShapeStackEntry> nextShapeStack;
    nextShapeStack.reserve(shapeRows.size());
    for (const ColorPipelineRowState* shapeRow : shapeRows) {
        ColorPipelineShapeStackEntry shapeEntry;
        if (!shapeRow || !TryBuildColorPipelineShapeStackEntryFromRow(*shapeRow, &shapeEntry, outError)) {
            return false;
        }
        nextShapeStack.push_back(shapeEntry);
    }

    if (ioParams->color_shape_stack_count != static_cast<int>(nextShapeStack.size())) {
        ioParams->color_shape_stack_count = static_cast<int>(nextShapeStack.size());
        changed = true;
    }
    for (int index = 0; index < kColorPipelineMaxShapeStackCount; ++index) {
        ColorPipelineShapeStackEntry nextEntry;
        if (index < static_cast<int>(nextShapeStack.size())) {
            nextEntry = nextShapeStack[static_cast<std::size_t>(index)];
        }
        if (!ColorPipelineShapeStackEntriesEqual(ioParams->color_shape_stack[index], nextEntry)) {
            ioParams->color_shape_stack[index] = nextEntry;
            changed = true;
        }
    }

    std::vector<ColorPipelinePaletteStackEntry> nextPaletteStack;
    if (!rootBasinPairSchedule) {
        if (paletteRows.size() > static_cast<std::size_t>(kColorPipelineMaxPaletteStackCount)) {
            if (outError) {
                *outError = "Current live bridge only supports a bounded number of enabled Palette rows in the schedule lane.";
            }
            return false;
        }
        nextPaletteStack.reserve(paletteRows.size());
        for (const ColorPipelineRowState* paletteRow : paletteRows) {
            ColorPipelinePaletteStackEntry paletteEntry;
            if (!paletteRow || !TryBuildColorPipelinePaletteStackEntryFromRow(*paletteRow, &paletteEntry, outError)) {
                return false;
            }
            nextPaletteStack.push_back(paletteEntry);
        }
    }
    if (ioParams->color_palette_stack_count != static_cast<int>(nextPaletteStack.size())) {
        ioParams->color_palette_stack_count = static_cast<int>(nextPaletteStack.size());
        changed = true;
    }
    for (int index = 0; index < kColorPipelineMaxPaletteStackCount; ++index) {
        ColorPipelinePaletteStackEntry nextEntry;
        if (index < static_cast<int>(nextPaletteStack.size())) {
            nextEntry = nextPaletteStack[static_cast<std::size_t>(index)];
        }
        if (!ColorPipelinePaletteStackEntriesEqual(ioParams->color_palette_stack[index], nextEntry)) {
            ioParams->color_palette_stack[index] = nextEntry;
            changed = true;
        }
    }

    std::vector<ColorPipelineGradingStackEntry> nextGradingStack;
    nextGradingStack.reserve(gradingRows.size());
    for (const ColorPipelineRowState* gradingRow : gradingRows) {
        ColorPipelineGradingStackEntry gradingEntry;
        if (!gradingRow || !TryBuildColorPipelineGradingStackEntryFromRow(*gradingRow, &gradingEntry, outError)) {
            return false;
        }
        nextGradingStack.push_back(gradingEntry);
    }
    if (ioParams->color_grading_stack_count != static_cast<int>(nextGradingStack.size())) {
        ioParams->color_grading_stack_count = static_cast<int>(nextGradingStack.size());
        changed = true;
    }
    for (int index = 0; index < kColorPipelineMaxGradingStackCount; ++index) {
        ColorPipelineGradingStackEntry nextEntry;
        if (index < static_cast<int>(nextGradingStack.size())) {
            nextEntry = nextGradingStack[static_cast<std::size_t>(index)];
        }
        if (!ColorPipelineGradingStackEntriesEqual(ioParams->color_grading_stack[index], nextEntry)) {
            ioParams->color_grading_stack[index] = nextEntry;
            changed = true;
        }
    }

    ColorPipelineShapeStackEntry legacyMirrorEntry;
    if (!nextShapeStack.empty()) {
        legacyMirrorEntry = nextShapeStack.back();
    }
    if (ioParams->color_shape != legacyMirrorEntry.shape) {
        ioParams->color_shape = legacyMirrorEntry.shape;
        changed = true;
    }
    resetShapeOffsetScale();
    resetShapeRepeat();
    resetShapePosterize();
    resetShapeBiasGain();
    resetShapeSmoothWindow();
    switch (legacyMirrorEntry.shape) {
    case ColorPipelineShape::offset_scale:
        assignShapeFloat(&ioParams->color_shape_offset, legacyMirrorEntry.params.offset);
        assignShapeFloat(&ioParams->color_shape_scale, legacyMirrorEntry.params.scale);
        break;
    case ColorPipelineShape::repeat:
    case ColorPipelineShape::mirror_repeat:
        assignShapeFloat(&ioParams->color_shape_repeat_frequency, legacyMirrorEntry.params.repeat_frequency);
        assignShapeFloat(&ioParams->color_shape_repeat_phase, legacyMirrorEntry.params.repeat_phase);
        break;
    case ColorPipelineShape::posterize:
        assignShapeInt(&ioParams->color_shape_posterize_steps, legacyMirrorEntry.params.posterize_steps);
        assignShapeFloat(&ioParams->color_shape_posterize_mix, legacyMirrorEntry.params.posterize_mix);
        break;
    case ColorPipelineShape::bias_gain_curve:
        assignShapeFloat(&ioParams->color_shape_bias, legacyMirrorEntry.params.bias);
        assignShapeFloat(&ioParams->color_shape_gain, legacyMirrorEntry.params.gain);
        break;
    case ColorPipelineShape::smooth_window:
        assignShapeFloat(&ioParams->color_shape_window_center, legacyMirrorEntry.params.window_center);
        assignShapeFloat(&ioParams->color_shape_window_width, legacyMirrorEntry.params.window_width);
        assignShapeFloat(&ioParams->color_shape_window_softness, legacyMirrorEntry.params.window_softness);
        break;
    case ColorPipelineShape::identity:
    default:
        break;
    }

    if (!nextPaletteStack.empty()) {
        const ColorPipelinePaletteStackEntry& paletteMirrorEntry = nextPaletteStack.back();
        resetPaletteHeatmap();
        resetPalettePhaseWheel();
        resetPaletteBandedHeatmap();
        resetPaletteExplaino();
        if (paletteMirrorEntry.palette == ColorPalette::cyclic_escape) {
            assignShapeFloat(&ioParams->color_heatmap_cycle_scale, paletteMirrorEntry.params.cycle_scale);
            assignShapeFloat(&ioParams->color_heatmap_saturation, paletteMirrorEntry.params.saturation);
        } else if (paletteMirrorEntry.palette == ColorPalette::phase_wheel) {
            assignShapeFloat(&ioParams->color_phase_palette_offset, paletteMirrorEntry.params.phase_offset);
        } else if (paletteMirrorEntry.palette == ColorPalette::banded_escape) {
            assignShapeFloat(&ioParams->color_iteration_band_emphasis, paletteMirrorEntry.params.band_emphasis);
            assignShapeFloat(&ioParams->color_iteration_band_palette_offset, paletteMirrorEntry.params.phase_offset);
        } else if (paletteMirrorEntry.palette == ColorPalette::explaino_cmap) {
            assignShapeFloat(&ioParams->color_explaino_palette_seed_scale, paletteMirrorEntry.params.seed_scale);
            assignShapeFloat(&ioParams->color_explaino_palette_seed_phase, paletteMirrorEntry.params.seed_phase);
            assignShapeFloat(&ioParams->color_explaino_palette_colorfulness, paletteMirrorEntry.params.colorfulness);
        }
    }

    if (!nextGradingStack.empty()) {
        const ColorPipelineGradingStackEntry& gradingMirrorEntry = nextGradingStack.back();
        if (gradingMirrorEntry.grading == ColorGradingPreset::escape_default) {
            assignShapeFloat(&ioParams->color_contrast_lift_exposure, gradingMirrorEntry.params.exposure);
            assignShapeFloat(&ioParams->color_contrast_lift_saturation, gradingMirrorEntry.params.saturation);
        } else if (gradingMirrorEntry.grading == ColorGradingPreset::phase_default ||
                   gradingMirrorEntry.grading == ColorGradingPreset::bands_default) {
            assignShapeFloat(&ioParams->color_saturation, gradingMirrorEntry.params.saturation);
            assignShapeFloat(&ioParams->color_contrast, gradingMirrorEntry.params.contrast);
        }
    }

    for (const ColorPipelineLaneState& lane : state.lanes) {
        if (lane.lane_id == "shape" || lane.lane_id == "palette" || lane.lane_id == "grading") {
            continue;
        }
        for (const ColorPipelineRowState& row : lane.rows) {
        if (!row.enabled) {
            continue;
        }
        bool rowChanged = false;
        if (!color_pipeline_core::ApplySupportedColorPipelineRowParamsToLive(
                row,
                ioParams,
                &rowChanged,
                outError)) {
            return false;
        }
        if (rowChanged) {
            changed = true;
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
    return color_pipeline_core::TryBuildColorPipelineSelectionFromLaneIds(
        sourceFunctionId,
        paletteFunctionId,
        outPipeline,
        outMode);
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

    std::vector<const ColorPipelineRowState*> sourceRows;
    if (!CollectEnabledColorPipelineRows(state, "source", &sourceRows, outError)) {
        return false;
    }
    std::vector<const ColorPipelineRowState*> shapeRows;
    if (!CollectEnabledColorPipelineRows(state, "shape", &shapeRows, outError)) {
        return false;
    }
    std::vector<const ColorPipelineRowState*> paletteRows;
    if (!CollectEnabledColorPipelineRows(state, "palette", &paletteRows, outError)) {
        return false;
    }
    if (sourceRows.size() == 1 &&
        !IsRootBasinPairCandidateRows(sourceRows, paletteRows) &&
        paletteRows.size() > static_cast<std::size_t>(kColorPipelineMaxPaletteStackCount)) {
        if (outError) {
            *outError = "Current live bridge only supports a bounded number of enabled Palette rows in the schedule lane.";
        }
        return false;
    }
    if (shapeRows.size() > static_cast<std::size_t>(kColorPipelineMaxShapeStackCount)) {
        if (outError) {
            *outError = "Current live bridge only supports a bounded number of enabled Shape rows in the schedule lane.";
        }
        return false;
    }
    for (const ColorPipelineRowState* shapeRow : shapeRows) {
        if (!shapeRow || !IsSupportedColorPipelineShapeFunctionId(shapeRow->function_id)) {
            if (outError) {
                *outError = "Current live bridge only supports the Identity, Offset + Scale, Repeat, Posterize, Mirror Repeat, Bias + Gain Curve, and Smooth Window Shape rows; stacked or remapped Shape recipes stay draft-only until custom runtime integration lands.";
            }
            return false;
        }
    }

    if (IsRootBasinPairCandidateRows(sourceRows, paletteRows)) {
        std::vector<ColorPipelineSelection> rootBasinPairs;
        ColoringMode mode = ColoringMode::root_basin;
        if (!TryBuildRootBasinPairSelectionsFromRows(sourceRows, paletteRows, &rootBasinPairs, &mode, outError)) {
            return false;
        }
        *outPipeline = rootBasinPairs.back();
        *outMode = mode;
        return true;
    }

    bool hasPartialRootBasinPairFamily = false;
    for (const ColorPipelineRowState* sourceRow : sourceRows) {
        hasPartialRootBasinPairFamily = hasPartialRootBasinPairFamily || (sourceRow && sourceRow->function_id == "root_index");
    }
    for (const ColorPipelineRowState* paletteRow : paletteRows) {
        hasPartialRootBasinPairFamily = hasPartialRootBasinPairFamily || (paletteRow && IsSupportedRootBasinPaletteFunctionId(paletteRow->function_id));
    }
    if (hasPartialRootBasinPairFamily) {
        if (outError) {
            *outError = "Current live bridge only supports one enabled Source row and one enabled Palette row unless every enabled Source / Palette row participates in the bounded row-indexed root-basin pair family.";
        }
        return false;
    }

    if (sourceRows.size() != 1) {
        if (outError) {
            *outError = "Current live bridge only supports one enabled Source row unless every enabled Source / Palette row participates in the bounded row-indexed root-basin pair family.";
        }
        return false;
    }
    for (const ColorPipelineRowState* paletteRow : paletteRows) {
        if (!paletteRow || !IsSupportedColorPipelinePaletteStackFunctionId(paletteRow->function_id)) {
            if (outError) {
                *outError = "Current live bridge only supports Heatmap, Phase Wheel, Banded Heatmap, and ExplainO CMap in the Palette stack.";
            }
            return false;
        }
    }

    ColorPipelineSelection pipeline{};
    ColoringMode mode = ColoringMode::root_basin;
    for (const ColorPipelineRowState* paletteRow : paletteRows) {
        ColorPipelineSelection rowPipeline{};
        ColoringMode rowMode = ColoringMode::root_basin;
        if (!TryBuildColorPipelineSelectionFromLaneIds(
                sourceRows.front()->function_id.c_str(),
                paletteRow->function_id.c_str(),
                &rowPipeline,
                &rowMode)) {
            if (outError) {
                *outError = "Selected Source / Shape / Palette recipe is draft-only until custom pipeline runtime integration lands or you choose a matching supported Source / Palette pair.";
            }
            return false;
        }
        pipeline = rowPipeline;
        mode = rowMode;
    }

    const char* gradingFunctionId = AdvancedColorGradingFunctionId(pipeline.grading);
    if (gradingFunctionId && gradingFunctionId[0] != '\0') {
        const ColorPipelineLaneCatalog* gradingCatalog = FindColorPipelineLaneCatalog("grading");
        if (!gradingCatalog || !FindColorPipelineFunctionDescriptor(*gradingCatalog, gradingFunctionId)) {
            if (outError) {
                *outError = "Selected Source / Shape / Palette recipe still depends on an unshipped Grading row and stays draft-only until that grading runtime integration lands.";
            }
            return false;
        }
        const ColorPipelineLaneState* gradingLane = FindColorPipelineLaneState(state, "grading");
        if (gradingLane) {
            std::vector<const ColorPipelineRowState*> gradingRows;
            if (!CollectEnabledColorPipelineRows(state, "grading", &gradingRows, outError)) {
                return false;
            }
            if (gradingRows.empty()) {
                if (outError) {
                    *outError = "Selected Source / Shape / Palette recipe requires at least one shipped Grading row.";
                }
                return false;
            }
            if (gradingRows.size() > static_cast<std::size_t>(kColorPipelineMaxGradingStackCount)) {
                if (outError) {
                    *outError = "Current live bridge only supports a bounded number of enabled Grading rows in the schedule lane.";
                }
                return false;
            }
            if (!gradingRows.front() || gradingRows.front()->function_id != gradingFunctionId) {
                if (outError) {
                    *outError = "Selected Source / Shape / Palette recipe requires the first shipped Grading row to match its runtime bridge.";
                }
                return false;
            }
            for (const ColorPipelineRowState* gradingRow : gradingRows) {
                if (!gradingRow || !IsSupportedColorPipelineGradingFunctionId(gradingRow->function_id)) {
                    if (outError) {
                        *outError = "Current live bridge only supports contrast_lift, phase_finish, and band_finish in the Grading stack.";
                    }
                    return false;
                }
            }
        } else if (gradingCatalog->default_function_id != std::string(gradingFunctionId)) {
            if (outError) {
                *outError = "Selected Source / Shape / Palette recipe requires a shipped Grading row that is not available in the current draft state.";
            }
            return false;
        }
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
        if (!ApplySupportedColorPipelineParamsToLive(state, &probe, &paramChanged, &error, &nextPipeline)) {
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

    std::vector<const ColorPipelineRowState*> sourceRows;
    if (!CollectEnabledColorPipelineRows(*ioState, "source", &sourceRows, &error)) {
        PushColorPipelineValidationMessage(ioState, error);
        return false;
    }
    std::vector<const ColorPipelineRowState*> paletteRows;
    if (!CollectEnabledColorPipelineRows(*ioState, "palette", &paletteRows, &error)) {
        PushColorPipelineValidationMessage(ioState, error);
        return false;
    }
    std::vector<ColorPipelineSelection> nextRootBasinPairs;
    if (IsRootBasinPairCandidateRows(sourceRows, paletteRows) &&
        !TryBuildRootBasinPairSelectionsFromRows(sourceRows, paletteRows, &nextRootBasinPairs, &nextMode, &error)) {
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
    if (!ApplySupportedColorPipelineParamsToLive(*ioState, ioParams, &paramChanged, &error, &nextPipeline)) {
        PushColorPipelineValidationMessage(ioState, error);
        return false;
    }

    if (ioParams->color_root_basin_pair_count != static_cast<int>(nextRootBasinPairs.size())) {
        ioParams->color_root_basin_pair_count = static_cast<int>(nextRootBasinPairs.size());
        paramChanged = true;
    }
    for (int index = 0; index < kColorPipelineMaxRootBasinPairCount; ++index) {
        ColorPipelineSelection nextPair{};
        if (index < static_cast<int>(nextRootBasinPairs.size())) {
            nextPair = nextRootBasinPairs[static_cast<std::size_t>(index)];
        }
        if (!ColorPipelineSelectionsEqual(ioParams->color_root_basin_pairs[index], nextPair)) {
            ioParams->color_root_basin_pairs[index] = nextPair;
            paramChanged = true;
        }
    }

    ioParams->coloring_mode = nextMode;
    ioParams->color_pipeline = nextPipeline;
    if (!SyncColorPipelineWindowFromLiveState(ioState, liveFractalType, ioParams)) {
        return false;
    }
    if (ioState->live_snapshot.draft_import_supported &&
        !ResetColorPipelineDraftFromLiveState(ioState)) {
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
    ColorPipelineRenderInteractionState* ioInteraction = nullptr,
    const char* primaryControlId = nullptr) {
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
        NoteColorPipelineUiAutomationRect(ioState, primaryControlId);
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
        NoteColorPipelineUiAutomationRect(ioState, primaryControlId);
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
        NoteColorPipelineUiAutomationRect(ioState, primaryControlId);
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
        NoteColorPipelineUiAutomationRect(ioState, primaryControlId);
        NoteColorPipelineCurrentItemInteraction(changed, ioInteraction);
        if (changed) {
            ioValue->bool_value = value;
            TryApplySupportedColorPipelineDraftFromControl(ioState, liveFractalType, liveParams, ioDirty, ioInteraction);
        }
    } else if (param.type == "enum") {
        const char* preview = ioValue->enum_value.empty() ? "(select)" : ioValue->enum_value.c_str();
        const bool comboOpen = ImGui::BeginCombo(param.label.c_str(), preview);
        NoteColorPipelineUiAutomationRect(ioState, primaryControlId);
        if (comboOpen) {
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
                ImGui::TextDisabled("Supported live bridge recipes right now: Smooth Escape / Escape Magnitude / Root Proximity with Heatmap or Explaino Cmap, Phase Orbit / Orbit Stripe with Phase Wheel, Iteration Bands with Banded Heatmap, and Root Index with Root Classic Palette or Joy Root Palette.");
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
        if (ioState->force_open_for_automation) {
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);
        }
        const bool rowEnabledBefore = row.enabled;

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
                const std::string primaryControlId = BuildColorPipelinePrimaryControlId(lane.lane_id, row.function_id, currentDescriptor->parameters[paramIndex]);
                RenderColorPipelineParamControl(
                    ioState,
                    liveFractalType,
                    liveParams,
                    currentDescriptor->parameters[paramIndex],
                    &row.parameter_values[paramIndex],
                    ioDirty,
                    ioInteraction,
                    primaryControlId.c_str());
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

        if (row.enabled != rowEnabledBefore) {
            SetColorPipelineRowEnabledFromUi(ioState, laneIndex, rowIndex, row.enabled);
            TryApplySupportedColorPipelineDraftFromControl(ioState, liveFractalType, liveParams, ioDirty, ioInteraction);
        }
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
    if (!ioState) {
        return false;
    }

    ClearColorPipelineUiAutomationRects(ioState);
    if (!ioState->open) {
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
    if (ioState->force_open_for_automation) {
        ImGui::SetNextWindowCollapsed(false, ImGuiCond_Always);
    }
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
