#pragma once

#include "color_pipeline_core.h"
#include "enum_id_utils.h"
#include "fractal_family_rules.h"
#include "schema_binding.h"

#include <cmath>
#include <cstdint>
#include <string>
#include <type_traits>
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
    std::string ui_automation_click_control_id;
    bool ui_automation_click_pending = false;
    bool ui_automation_click_consumed = false;
    std::string ui_automation_set_control_id;
    double ui_automation_set_control_value = 0.0;
    bool ui_automation_set_pending = false;
    bool ui_automation_set_consumed = false;
    std::string ui_automation_set_error;
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

inline std::string BuildColorPipelineRowEnabledControlId(
    const std::string& laneId,
    std::uint64_t rowId) {
    return std::string("color_pipeline.") + laneId + "." + std::to_string(rowId) + ".enabled";
}

inline std::string BuildColorPipelineRowRemoveControlId(
    const std::string& laneId,
    std::uint64_t rowId) {
    return std::string("color_pipeline.") + laneId + "." + std::to_string(rowId) + ".remove";
}

inline std::string BuildColorPipelineSdfFieldDownsampleControlId() {
    return "color_pipeline.source.sdf_field.downsample.primary";
}

inline std::string BuildColorPipelineRecipeApplyControlId(const std::string& recipeId) {
    return std::string("color_pipeline.recipe.") + recipeId + ".apply";
}

inline const char* ColorPipelineWindowDraftRecipesIntroText() {
    return "Preset Source / Shape / Palette / Grading recipes are materialized from the UI-Salt contract and apply through the normal row editor.";
}

inline const char* ColorPipelineWindowLaneModelSummaryText() {
    return "This window now models four typed editor lanes instead of a fixed Signal / Palette / Grade trio.";
}

inline const char* ColorPipelineWindowBridgeBoundarySummaryText() {
    return "Supported preset application covers the bounded shipped Source / Shape / Palette / Grading recipes shown here; root_index with root_classic_palette or joy_root_palette stays on the separate row-indexed root-basin schedule.";
}

inline const char* ColorPipelineWindowSupportedPresetSummaryText() {
    return "Available presets: Smooth Escape, Phase Orbit Wheel, and SDF Normal Angle Diagnostic.";
}

inline const char* ColorPipelineWindowFixedPresetHelpText() {
    return "Some Source / Palette rows are fixed presets with no tunable parameters; choosing the row is itself the live change.";
}

inline const char* ColorPipelineUnsupportedShapeRowsMessage() {
    return "Current runtime application only supports the shipped Identity, Offset + Scale, Repeat, Posterize, Mirror Repeat, Bias + Gain Curve, and Smooth Window Shape rows in bounded ordered Shape stacks; unshipped custom Shape recipes stay unsupported until custom runtime integration lands.";
}

inline const char* ColorPipelineShapeRowBridgeHelpText() {
    return "Shape rows edit the authored row stack; shipped ordered Shape stacks participate in runtime application, and the last enabled Shape row still mirrors into the legacy single-shape owners.";
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
inline void NoteColorPipelineUiAutomationRectFromStackEditor(
    void* userData,
    const char* controlId) {
    NoteColorPipelineUiAutomationRect(static_cast<ColorPipelineWindowState*>(userData), controlId);
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
    const char* taxonomyGroup,
    std::vector<FunctionParamDescriptor> parameters) {
    return color_pipeline_core::MakeColorPipelineFunction(
        id,
        name,
        description,
        taxonomyGroup,
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
    const FunctionDescriptor& descriptor,
    bool preserveExistingValues = true) {
    return color_pipeline_core::SetColorPipelineRowFunction(ioRow, descriptor, preserveExistingValues);
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

inline bool ImportSupportedColorPipelineParamsFromSourceStackEntry(
    ColorPipelineRowState* ioRow,
    const ColorPipelineSourceStackEntry& sourceEntry,
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

inline bool IsSupportedColorPipelineSourceStackFunctionId(const std::string& functionId);

inline bool IsSdfColorPipelineSourceFunctionId(const std::string& functionId);

inline bool IsSupportedColorPipelineShapeFunctionId(const std::string& functionId);

inline const ColorPipelineLaneState* FindColorPipelineLaneState(
    const ColorPipelineWindowState& state,
    const char* laneId);

inline bool IsSupportedColorPipelineGradingFunctionId(const std::string& functionId);

inline bool ColorPipelineShapeRuntimeParamsEqual(
    const ColorPipelineShapeRuntimeParams& left,
    const ColorPipelineShapeRuntimeParams& right);

inline bool ColorPipelineSourceRuntimeParamsEqual(
    const ColorPipelineSourceRuntimeParams& left,
    const ColorPipelineSourceRuntimeParams& right);

inline bool ColorPipelineSourceStackEntriesEqual(
    const ColorPipelineSourceStackEntry& left,
    const ColorPipelineSourceStackEntry& right);

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

inline bool TryBuildColorPipelineSourceStackEntryFromRow(
    const ColorPipelineRowState& row,
    ColorPipelineSourceStackEntry* outEntry,
    std::string* outError = nullptr);

inline bool TryBuildColorPipelineSourceLaneFromLive(
    const KernelParams& liveParams,
    ColorPipelineLaneState* outLane,
    bool* outDraftImportSupported,
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
            "Current color selection is outside the shipped advanced catalog; keep editing the authored row stack or switch the simple Color panel first.");
        return false;
    }
    const std::vector<ColorPipelineLaneState> previousLanes = ioState->lanes;
    ioState->lanes = ioState->live_snapshot.lanes;

    const std::size_t sharedLaneCount = (std::min)(ioState->lanes.size(), previousLanes.size());
    for (std::size_t laneIndex = 0; laneIndex < sharedLaneCount; ++laneIndex) {
        ColorPipelineLaneState& lane = ioState->lanes[laneIndex];
        const ColorPipelineLaneState& previousLane = previousLanes[laneIndex];
        if (lane.lane_id != previousLane.lane_id) {
            continue;
        }

        std::vector<ColorPipelineRowState> mergedRows;
        mergedRows.reserve(lane.rows.size() + previousLane.rows.size());
        std::size_t liveRowIndex = 0;
        for (const ColorPipelineRowState& previousRow : previousLane.rows) {
            if (previousRow.enabled) {
                if (liveRowIndex >= lane.rows.size()) {
                    continue;
                }
                ColorPipelineRowState liveRow = lane.rows[liveRowIndex++];
                liveRow.ui_row_id = previousRow.ui_row_id;
                mergedRows.push_back(std::move(liveRow));
                continue;
            }
            mergedRows.push_back(previousRow);
        }
        while (liveRowIndex < lane.rows.size()) {
            mergedRows.push_back(lane.rows[liveRowIndex++]);
        }
        lane.rows = std::move(mergedRows);
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
        if (sourceFunctionId && std::strcmp(sourceFunctionId, "root_index") == 0) {
            if (!BuildColorPipelineLaneWithSingleRow(*sourceCatalog, sourceFunctionId, 0, &sourceLane, outError) ||
                !ImportSupportedColorPipelineParamsFromLive(&sourceLane.rows.front(), liveParams, outError)) {
                return false;
            }
        } else if (!TryBuildColorPipelineSourceLaneFromLive(
                liveParams,
                &sourceLane,
                &sourcePaletteDraftImportSupported,
                outError)) {
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
        std::string companionLaneId;
        std::string companionFunctionId;
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
            color_pipeline_core::TrySuggestColorPipelineCompanionFunction(
                lane.lane_id.c_str(),
                lane.rows[rowIndex].function_id.c_str(),
                &companionLaneId,
                &companionFunctionId);

            if (!companionLaneId.empty() && !companionFunctionId.empty()) {
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
                    const FunctionDescriptor* companionDescriptor = FindColorPipelineFunctionDescriptor(*companionCatalog, companionFunctionId.c_str());
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
                    const FractalType supportedFractalType = ioState->live_snapshot.valid
                        ? ioState->live_snapshot.fractal_type
                        : FractalType::explaino;
                    bool preserveCurrentGrading = false;
                    ColorGradingPreset currentGrading = ColorGradingPreset::escape_default;
                    if (TryParseAdvancedColorGradingFunctionId(gradingLane->rows[0].function_id, &currentGrading)) {
                        ColorPipelineSelection preservedPipeline = supportedPipeline;
                        preservedPipeline.grading = currentGrading;
                        preserveCurrentGrading = IsColorPipelineAllowedForFractal(supportedFractalType, preservedPipeline);
                    }
                    if (!preserveCurrentGrading) {
                        const ColorPipelineLaneCatalog* gradingCatalog = FindColorPipelineLaneCatalog(gradingLane->lane_id);
                        if (gradingCatalog) {
                            const FunctionDescriptor* gradingDescriptor = FindColorPipelineFunctionDescriptor(*gradingCatalog, requiredGradingFunctionId);
                            if (gradingDescriptor && gradingLane->rows[0].function_id != requiredGradingFunctionId) {
                                SetColorPipelineRowFunction(&gradingLane->rows[0], *gradingDescriptor, false);
                            }
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
    const char* functionId);

inline bool ApplyColorPipelineRecipeToDraft(
    ColorPipelineWindowState* ioState,
    const char* recipeId) {
    if (!ioState || !recipeId || recipeId[0] == '\0') {
        return false;
    }
    if (!EnsureColorPipelineWindowInitialized(ioState)) {
        return false;
    }

    const MaterializedColorPipelineRecipe* recipe =
        color_pipeline_core::FindActiveColorPipelineRecipe(recipeId);
    if (!recipe) {
        PushColorPipelineValidationMessage(ioState, std::string("Unknown Color Pipeline recipe: ") + recipeId);
        return false;
    }

    struct RecipeLaneSpec {
        const char* lane_id;
        const std::string* function_id;
    };
    const RecipeLaneSpec laneSpecs[] = {
        {"source", &recipe->source},
        {"shape", &recipe->shape},
        {"palette", &recipe->palette},
        {"grading", &recipe->grading},
    };

    ColorPipelineWindowState probe = *ioState;
    for (const RecipeLaneSpec& laneSpec : laneSpecs) {
        bool foundLane = false;
        for (std::size_t laneIndex = 0; laneIndex < probe.lanes.size(); ++laneIndex) {
            ColorPipelineLaneState& lane = probe.lanes[laneIndex];
            if (lane.lane_id != laneSpec.lane_id) {
                continue;
            }
            foundLane = true;
            while (lane.rows.size() > 1) {
                lane.rows.pop_back();
            }
            if (lane.rows.empty() && !AddColorPipelineLaneRow(&probe, laneIndex, laneSpec.function_id->c_str())) {
                return false;
            }
            if (!SelectColorPipelineLaneFunction(&probe, laneIndex, laneSpec.function_id->c_str())) {
                return false;
            }
            break;
        }
        if (!foundLane) {
            PushColorPipelineValidationMessage(ioState, std::string("Missing Color Pipeline recipe lane: ") + laneSpec.lane_id);
            return false;
        }
    }

    *ioState = std::move(probe);
    return true;
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
        return path == "signal.scale" || path == "signal.bias" || path == "signal.blend_weight";
    }
    if (IsSdfColorPipelineSourceFunctionId(functionId)) {
        if (functionId == "sdf_boundary_band") {
            return path == "signal.scale" || path == "signal.bias" ||
                path == "signal.blend_weight" || path == "signal.boundary_width_px" ||
                path == "signal.sdf_gate" || path == "signal.sdf_gate_width_px" ||
                path == "signal.sdf_sample_step";
        }
        return path == "signal.scale" || path == "signal.bias" || path == "signal.blend_weight" ||
            path == "signal.sdf_gate" || path == "signal.sdf_gate_width_px" ||
            path == "signal.sdf_sample_step";
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
    if (functionId == "neutral_finish" || functionId == "tone_map_finish") {
        return path == "grade.exposure" || path == "grade.saturation" || path == "grade.contrast";
    }
    if (functionId == "grade_glow") {
        return path == "grade.exposure" || path == "grade.saturation" || path == "grade.contrast" || path == "grade.glow";
    }
    if (functionId == "balance_void_grade") {
        return path == "grade.balance_void" || path == "grade.chroma_tension" || path == "grade.accent_bias";
    }
    if (functionId == "phase_orbit") {
        return path == "signal.phase_offset" || path == "signal.wrap_cycles" || path == "signal.blend_weight";
    }
    if (functionId == "escape_magnitude") {
        return path == "signal.magnitude_scale" || path == "signal.magnitude_bias" || path == "signal.blend_weight";
    }
    if (functionId == "orbit_stripe") {
        return path == "signal.stripe_frequency" || path == "signal.phase_offset" || path == "signal.blend_weight";
    }
    if (functionId == "root_proximity") {
        return path == "signal.proximity_scale" || path == "signal.proximity_bias" || path == "signal.blend_weight";
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
        return path == "signal.band_count" || path == "signal.softness" || path == "signal.blend_weight";
    }
    if (functionId == "banded_heatmap") {
        return path == "palette.band_emphasis" || path == "palette.phase_offset" ||
            path == "palette.blend_weight" || path == "palette.blend_mode";
    }
    return false;
}

inline bool IsRenderableColorPipelineParam(
    const char* laneId,
    const char* functionId,
    const char* path) {
    (void)laneId;
    return functionId && path && IsLiveColorPipelineParamPath(functionId, path);
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

inline bool SetColorPipelineParamEnum(
    ColorPipelineRowState* ioRow,
    const char* path,
    const char* value,
    std::string* outError = nullptr) {
    return color_pipeline_core::SetColorPipelineParamEnum(ioRow, path, value, outError);
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

inline bool ImportSupportedColorPipelineParamsFromSourceStackEntry(
    ColorPipelineRowState* ioRow,
    const ColorPipelineSourceStackEntry& sourceEntry,
    std::string* outError) {
    if (!ioRow) {
        if (outError) *outError = "Advanced color Source-stack import requires a row";
        return false;
    }
    const char* functionId = AdvancedColorSignalFunctionId(sourceEntry.signal);
    if (!functionId || std::strcmp(functionId, "root_index") == 0 || ioRow->function_id != functionId) {
        if (outError) *outError = "Advanced color Source-stack import row does not match the saved Source entry";
        return false;
    }
    if (!SetColorPipelineParamNumber(ioRow, "signal.blend_weight", sourceEntry.params.blend_weight, outError)) {
        return false;
    }
    if (ioRow->function_id == "smooth_escape_ramp") {
        return SetColorPipelineParamNumber(ioRow, "signal.scale", sourceEntry.params.scale, outError) &&
            SetColorPipelineParamNumber(ioRow, "signal.bias", sourceEntry.params.bias, outError);
    }
    if (IsSdfColorPipelineSourceFunctionId(ioRow->function_id)) {
        if (!SetColorPipelineParamNumber(ioRow, "signal.scale", sourceEntry.params.scale, outError) ||
            !SetColorPipelineParamNumber(ioRow, "signal.bias", sourceEntry.params.bias, outError)) {
            return false;
        }
        if (ioRow->function_id == "sdf_boundary_band") {
            if (!SetColorPipelineParamNumber(ioRow, "signal.boundary_width_px", sourceEntry.params.sdf_boundary_width_px, outError)) {
                return false;
            }
        }
        const char* gateId = color_pipeline_core::ColorPipelineSdfGateModeId(sourceEntry.params.sdf_gate);
        return SetColorPipelineParamEnum(ioRow, "signal.sdf_gate", gateId ? gateId : "none", outError) &&
            SetColorPipelineParamNumber(ioRow, "signal.sdf_gate_width_px", sourceEntry.params.sdf_gate_width_px, outError) &&
            SetColorPipelineParamNumber(ioRow, "signal.sdf_sample_step", static_cast<double>(sourceEntry.params.sdf_sample_step), outError);
    }
    if (ioRow->function_id == "phase_orbit") {
        return SetColorPipelineParamNumber(ioRow, "signal.phase_offset", sourceEntry.params.phase_offset, outError) &&
            SetColorPipelineParamNumber(ioRow, "signal.wrap_cycles", sourceEntry.params.wrap_cycles, outError);
    }
    if (ioRow->function_id == "banded_signal") {
        return SetColorPipelineParamNumber(ioRow, "signal.band_count", static_cast<double>(sourceEntry.params.band_count), outError) &&
            SetColorPipelineParamNumber(ioRow, "signal.softness", sourceEntry.params.softness, outError);
    }
    if (ioRow->function_id == "escape_magnitude") {
        return SetColorPipelineParamNumber(ioRow, "signal.magnitude_scale", sourceEntry.params.magnitude_scale, outError) &&
            SetColorPipelineParamNumber(ioRow, "signal.magnitude_bias", sourceEntry.params.magnitude_bias, outError);
    }
    if (ioRow->function_id == "orbit_stripe") {
        return SetColorPipelineParamNumber(ioRow, "signal.stripe_frequency", sourceEntry.params.stripe_frequency, outError) &&
            SetColorPipelineParamNumber(ioRow, "signal.phase_offset", sourceEntry.params.stripe_phase, outError);
    }
    if (ioRow->function_id == "root_proximity") {
        return SetColorPipelineParamNumber(ioRow, "signal.proximity_scale", sourceEntry.params.proximity_scale, outError) &&
            SetColorPipelineParamNumber(ioRow, "signal.proximity_bias", sourceEntry.params.proximity_bias, outError);
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
            *outError = std::string("Current row editor application requires one enabled row in the ") + lane->label + " lane.";
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

inline bool IsSupportedColorPipelineSourceStackFunctionId(const std::string& functionId) {
    return functionId == "smooth_escape_ramp" ||
        functionId == "phase_orbit" ||
        functionId == "banded_signal" ||
        functionId == "escape_magnitude" ||
        functionId == "orbit_stripe" ||
        functionId == "root_proximity" ||
        functionId == "sdf_signed_distance" ||
        functionId == "sdf_inside_outside" ||
        functionId == "sdf_boundary_band" ||
        functionId == "sdf_normal_angle" ||
        functionId == "sdf_curvature" ||
        functionId == "lens_field_v2_distance";
}

inline bool IsSdfColorPipelineSourceFunctionId(const std::string& functionId) {
    return functionId == "sdf_signed_distance" ||
        functionId == "sdf_inside_outside" ||
        functionId == "sdf_boundary_band" ||
        functionId == "sdf_normal_angle" ||
        functionId == "sdf_curvature" ||
        functionId == "lens_field_v2_distance";
}

inline bool ColorPipelineSourceRuntimeParamsEqual(
    const ColorPipelineSourceRuntimeParams& left,
    const ColorPipelineSourceRuntimeParams& right) {
    return std::fabs(left.scale - right.scale) <= 1.0e-6f &&
        std::fabs(left.bias - right.bias) <= 1.0e-6f &&
        std::fabs(left.phase_offset - right.phase_offset) <= 1.0e-6f &&
        std::fabs(left.wrap_cycles - right.wrap_cycles) <= 1.0e-6f &&
        left.band_count == right.band_count &&
        std::fabs(left.softness - right.softness) <= 1.0e-6f &&
        std::fabs(left.magnitude_scale - right.magnitude_scale) <= 1.0e-6f &&
        std::fabs(left.magnitude_bias - right.magnitude_bias) <= 1.0e-6f &&
        std::fabs(left.stripe_frequency - right.stripe_frequency) <= 1.0e-6f &&
        std::fabs(left.stripe_phase - right.stripe_phase) <= 1.0e-6f &&
        std::fabs(left.proximity_scale - right.proximity_scale) <= 1.0e-6f &&
        std::fabs(left.proximity_bias - right.proximity_bias) <= 1.0e-6f &&
        std::fabs(left.sdf_boundary_width_px - right.sdf_boundary_width_px) <= 1.0e-6f &&
        left.sdf_gate == right.sdf_gate &&
        std::fabs(left.sdf_gate_width_px - right.sdf_gate_width_px) <= 1.0e-6f &&
        left.sdf_sample_step == right.sdf_sample_step &&
        std::fabs(left.blend_weight - right.blend_weight) <= 1.0e-6f;
}

inline bool ColorPipelineSourceStackEntriesEqual(
    const ColorPipelineSourceStackEntry& left,
    const ColorPipelineSourceStackEntry& right) {
    return left.signal == right.signal &&
        ColorPipelineSourceRuntimeParamsEqual(left.params, right.params);
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
        functionId == "band_finish" ||
        functionId == "basin_default" ||
        functionId == "neutral_finish" ||
        functionId == "tone_map_finish" ||
        functionId == "grade_glow" ||
        functionId == "balance_void_grade";
}

inline bool ColorPipelineGradingRuntimeParamsEqual(
    const ColorPipelineGradingRuntimeParams& left,
    const ColorPipelineGradingRuntimeParams& right) {
    return std::fabs(left.exposure - right.exposure) <= 1.0e-6f &&
        std::fabs(left.saturation - right.saturation) <= 1.0e-6f &&
        std::fabs(left.contrast - right.contrast) <= 1.0e-6f &&
        std::fabs(left.glow - right.glow) <= 1.0e-6f &&
        std::fabs(left.balance_void - right.balance_void) <= 1.0e-6f &&
        std::fabs(left.chroma_tension - right.chroma_tension) <= 1.0e-6f &&
        std::fabs(left.accent_bias - right.accent_bias) <= 1.0e-6f;
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

inline bool TryBuildColorPipelineSourceStackEntryFromRow(
    const ColorPipelineRowState& row,
    ColorPipelineSourceStackEntry* outEntry,
    std::string* outError) {
    if (!outEntry) {
        if (outError) *outError = "Advanced color Source-stack apply requires output storage";
        return false;
    }
    if (!IsSupportedColorPipelineSourceStackFunctionId(row.function_id)) {
        if (outError) {
            *outError = "Current row editor application only supports shipped non-basin Source rows in the Source stack; root_index stays on the separate root-basin pair schedule.";
        }
        return false;
    }

    ColorSignal signal = ColorSignal::smooth_escape;
    if (!TryParseAdvancedColorSignalFunctionId(row.function_id.c_str(), &signal) || signal == ColorSignal::root_index) {
        if (outError) {
            *outError = std::string("Unknown advanced color Source row id: ") + row.function_id;
        }
        return false;
    }

    ColorPipelineSourceStackEntry entry;
    entry.signal = signal;
    double blendWeight = entry.params.blend_weight;
    if (!TryGetColorPipelineParamNumber(row, "signal.blend_weight", &blendWeight, outError) ||
        !ValidateColorPipelineParamRange("signal.blend_weight", blendWeight, 0.0, 1.0, outError)) {
        return false;
    }
    entry.params.blend_weight = static_cast<float>(blendWeight);

    if (row.function_id == "smooth_escape_ramp") {
        double scale = 0.0;
        double bias = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "signal.scale", &scale, outError) ||
            !TryGetColorPipelineParamNumber(row, "signal.bias", &bias, outError) ||
            !ValidateColorPipelineParamRange("signal.scale", scale, 0.25, 4.0, outError) ||
            !ValidateColorPipelineParamRange("signal.bias", bias, -1.0, 1.0, outError)) {
            return false;
        }
        entry.params.scale = static_cast<float>(scale);
        entry.params.bias = static_cast<float>(bias);
    } else if (IsSdfColorPipelineSourceFunctionId(row.function_id)) {
        double scale = 0.0;
        double bias = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "signal.scale", &scale, outError) ||
            !TryGetColorPipelineParamNumber(row, "signal.bias", &bias, outError) ||
            !ValidateColorPipelineParamRange("signal.scale", scale, -2.0, 2.0, outError) ||
            !ValidateColorPipelineParamRange("signal.bias", bias, -2.0, 2.0, outError)) {
            return false;
        }
        entry.params.scale = static_cast<float>(scale);
        entry.params.bias = static_cast<float>(bias);
        if (row.function_id == "sdf_boundary_band") {
            double boundaryWidthPx = entry.params.sdf_boundary_width_px;
            if (!TryGetColorPipelineParamNumber(row, "signal.boundary_width_px", &boundaryWidthPx, outError) ||
                !ValidateColorPipelineParamRange("signal.boundary_width_px", boundaryWidthPx, 0.25, 16.0, outError)) {
                return false;
            }
            entry.params.sdf_boundary_width_px = static_cast<float>(boundaryWidthPx);
        }
        std::string gateId = "none";
        double gateWidthPx = entry.params.sdf_gate_width_px;
        int sampleStep = entry.params.sdf_sample_step;
        if (!TryGetColorPipelineParamEnum(row, "signal.sdf_gate", &gateId, outError) ||
            !color_pipeline_core::TryParseColorPipelineSdfGateModeId(gateId, &entry.params.sdf_gate) ||
            !TryGetColorPipelineParamNumber(row, "signal.sdf_gate_width_px", &gateWidthPx, outError) ||
            !ValidateColorPipelineParamRange("signal.sdf_gate_width_px", gateWidthPx, 0.25, 16.0, outError) ||
            !TryReadColorPipelineIntegerParam(row, "signal.sdf_sample_step", &sampleStep, outError) ||
            !ValidateColorPipelineParamRange("signal.sdf_sample_step", static_cast<double>(sampleStep), 1.0, 8.0, outError)) {
            if (outError && outError->empty()) {
                *outError = "Invalid SDF Source boundary-gate parameters";
            }
            return false;
        }
        entry.params.sdf_gate_width_px = static_cast<float>(gateWidthPx);
        entry.params.sdf_sample_step = sampleStep;
    } else if (row.function_id == "phase_orbit") {
        double phaseOffset = 0.0;
        double wrapCycles = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "signal.phase_offset", &phaseOffset, outError) ||
            !TryGetColorPipelineParamNumber(row, "signal.wrap_cycles", &wrapCycles, outError) ||
            !ValidateColorPipelineParamRange("signal.phase_offset", phaseOffset, -3.141592653589793, 3.141592653589793, outError) ||
            !ValidateColorPipelineParamRange("signal.wrap_cycles", wrapCycles, 0.5, 6.0, outError)) {
            return false;
        }
        entry.params.phase_offset = static_cast<float>(phaseOffset);
        entry.params.wrap_cycles = static_cast<float>(wrapCycles);
    } else if (row.function_id == "banded_signal") {
        int bandCount = 0;
        double softness = 0.0;
        if (!TryReadColorPipelineIntegerParam(row, "signal.band_count", &bandCount, outError) ||
            !TryGetColorPipelineParamNumber(row, "signal.softness", &softness, outError) ||
            !ValidateColorPipelineParamRange("signal.band_count", static_cast<double>(bandCount), 2.0, 24.0, outError) ||
            !ValidateColorPipelineParamRange("signal.softness", softness, 0.0, 1.0, outError)) {
            return false;
        }
        entry.params.band_count = bandCount;
        entry.params.softness = static_cast<float>(softness);
    } else if (row.function_id == "escape_magnitude") {
        double magnitudeScale = 0.0;
        double magnitudeBias = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "signal.magnitude_scale", &magnitudeScale, outError) ||
            !TryGetColorPipelineParamNumber(row, "signal.magnitude_bias", &magnitudeBias, outError) ||
            !ValidateColorPipelineParamRange("signal.magnitude_scale", magnitudeScale, 0.25, 4.0, outError) ||
            !ValidateColorPipelineParamRange("signal.magnitude_bias", magnitudeBias, -1.0, 1.0, outError)) {
            return false;
        }
        entry.params.magnitude_scale = static_cast<float>(magnitudeScale);
        entry.params.magnitude_bias = static_cast<float>(magnitudeBias);
    } else if (row.function_id == "orbit_stripe") {
        double stripeFrequency = 0.0;
        double stripePhase = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "signal.stripe_frequency", &stripeFrequency, outError) ||
            !TryGetColorPipelineParamNumber(row, "signal.phase_offset", &stripePhase, outError) ||
            !ValidateColorPipelineParamRange("signal.stripe_frequency", stripeFrequency, 0.25, 12.0, outError) ||
            !ValidateColorPipelineParamRange("signal.phase_offset", stripePhase, -3.141592653589793, 3.141592653589793, outError)) {
            return false;
        }
        entry.params.stripe_frequency = static_cast<float>(stripeFrequency);
        entry.params.stripe_phase = static_cast<float>(stripePhase);
    } else if (row.function_id == "root_proximity") {
        double proximityScale = 0.0;
        double proximityBias = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "signal.proximity_scale", &proximityScale, outError) ||
            !TryGetColorPipelineParamNumber(row, "signal.proximity_bias", &proximityBias, outError) ||
            !ValidateColorPipelineParamRange("signal.proximity_scale", proximityScale, 0.25, 8.0, outError) ||
            !ValidateColorPipelineParamRange("signal.proximity_bias", proximityBias, -1.0, 1.0, outError)) {
            return false;
        }
        entry.params.proximity_scale = static_cast<float>(proximityScale);
        entry.params.proximity_bias = static_cast<float>(proximityBias);
    }

    *outEntry = entry;
    return true;
}

inline bool ColorPipelineSourceDraftUsesSdf(const ColorPipelineWindowState& state) {
    const ColorPipelineLaneState* sourceLane = nullptr;
    for (const ColorPipelineLaneState& lane : state.lanes) {
        if (lane.lane_id == "source") {
            sourceLane = &lane;
            break;
        }
    }
    if (!sourceLane) {
        return false;
    }
    for (const ColorPipelineRowState& row : sourceLane->rows) {
        if (row.enabled && IsSdfColorPipelineSourceFunctionId(row.function_id)) {
            return true;
        }
    }
    return false;
}

inline bool ColorPipelineSdfFieldDownsampleControlVisible(
    const ColorPipelineWindowState& state,
    const LensSettings* lens) {
    return lens && ColorPipelineSourceDraftUsesSdf(state);
}

inline int NormalizeColorPipelineSdfFieldDownsampleValue(int value) {
    if (value <= 1) return 1;
    if (value <= 2) return 2;
    if (value <= 4) return 4;
    if (value <= 8) return 8;
    return 16;
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
            *outError = "Current row editor application only supports Heatmap, Phase Wheel, Banded Heatmap, and ExplainO CMap in the Palette stack.";
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
            *outError = "Current row editor application only supports row-indexed root-basin schedules when enabled Source and Palette row counts match.";
        }
        return false;
    }
    if (sourceRows.size() > static_cast<std::size_t>(kColorPipelineMaxRootBasinPairCount)) {
        if (outError) {
            *outError = "Current row editor application only supports a bounded number of enabled root-basin Source / Palette pairs.";
        }
        return false;
    }
    for (std::size_t index = 0; index < sourceRows.size(); ++index) {
        const ColorPipelineRowState* sourceRow = sourceRows[index];
        const ColorPipelineRowState* paletteRow = paletteRows[index];
        if (!sourceRow || !paletteRow || sourceRow->function_id != "root_index" || !IsSupportedRootBasinPaletteFunctionId(paletteRow->function_id)) {
            if (outError) {
                *outError = "Current row editor application only supports row-indexed root-basin pairs of root_index with root_classic_palette or joy_root_palette.";
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

inline bool TryBuildColorPipelineSourceLaneFromLive(
    const KernelParams& liveParams,
    ColorPipelineLaneState* outLane,
    bool* outDraftImportSupported,
    std::string* outError) {
    if (!outLane || !outDraftImportSupported) {
        if (outError) *outError = "Advanced color Source lane snapshot requires output storage";
        return false;
    }
    const ColorPipelineLaneCatalog* catalog = FindColorPipelineLaneCatalog("source");
    if (!catalog) {
        if (outError) *outError = "Missing advanced color Source lane catalog";
        return false;
    }

    ColorPipelineLaneState lane;
    lane.lane_id = catalog->lane_id;
    lane.label = catalog->label;
    *outDraftImportSupported = true;

    int sourceStackCount = liveParams.color_source_stack_count;
    if (sourceStackCount > kColorPipelineMaxSourceStackCount) {
        sourceStackCount = kColorPipelineMaxSourceStackCount;
    }
    if (sourceStackCount > 0) {
        for (int index = 0; index < sourceStackCount; ++index) {
            const ColorPipelineSourceStackEntry& sourceEntry = liveParams.color_source_stack[index];
            const char* functionId = AdvancedColorSignalFunctionId(sourceEntry.signal);
            if (!functionId || !IsSupportedColorPipelineSourceStackFunctionId(functionId)) {
                *outDraftImportSupported = false;
                *outLane = std::move(lane);
                return true;
            }
            ColorPipelineRowState row;
            if (!BuildColorPipelineRowFromFunctionId(*catalog, functionId, index, &row, outError) ||
                !ImportSupportedColorPipelineParamsFromSourceStackEntry(&row, sourceEntry, outError)) {
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

    const char* functionId = AdvancedColorSignalFunctionId(liveParams.color_pipeline.signal);
    if (!functionId || !IsSupportedColorPipelineSourceStackFunctionId(functionId)) {
        *outDraftImportSupported = false;
        *outLane = std::move(lane);
        return true;
    }
    ColorPipelineSourceStackEntry liveSourceEntry;
    liveSourceEntry.signal = liveParams.color_pipeline.signal;
    liveSourceEntry.params.scale = liveParams.color_smooth_escape_scale;
    liveSourceEntry.params.bias = liveParams.color_smooth_escape_bias;
    liveSourceEntry.params.phase_offset = liveParams.color_phase_signal_offset;
    liveSourceEntry.params.wrap_cycles = liveParams.color_phase_wrap_cycles;
    liveSourceEntry.params.band_count = liveParams.color_iteration_band_count;
    liveSourceEntry.params.softness = liveParams.color_iteration_band_softness;
    liveSourceEntry.params.magnitude_scale = liveParams.color_escape_magnitude_scale;
    liveSourceEntry.params.magnitude_bias = liveParams.color_escape_magnitude_bias;
    liveSourceEntry.params.stripe_frequency = liveParams.color_orbit_stripe_frequency;
    liveSourceEntry.params.stripe_phase = liveParams.color_orbit_stripe_phase;
    liveSourceEntry.params.proximity_scale = liveParams.color_root_proximity_scale;
    liveSourceEntry.params.proximity_bias = liveParams.color_root_proximity_bias;
    liveSourceEntry.params.blend_weight = 1.0f;
    ColorPipelineRowState row;
    if (!BuildColorPipelineRowFromFunctionId(*catalog, functionId, 0, &row, outError) ||
        !ImportSupportedColorPipelineParamsFromSourceStackEntry(&row, liveSourceEntry, outError)) {
        return false;
    }
    lane.rows.push_back(std::move(row));
    *outLane = std::move(lane);
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
            *outError = "Current row editor application only supports contrast_lift, phase_finish, band_finish, basin_default, neutral_finish, tone_map_finish, grade_glow, and balance_void_grade in the Grading stack.";
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
    } else if (row.function_id == "neutral_finish" || row.function_id == "tone_map_finish") {
        double exposure = 0.0;
        double saturation = 0.0;
        double contrast = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "grade.exposure", &exposure, outError) ||
            !TryGetColorPipelineParamNumber(row, "grade.saturation", &saturation, outError) ||
            !TryGetColorPipelineParamNumber(row, "grade.contrast", &contrast, outError) ||
            !ValidateColorPipelineParamRange("grade.exposure", exposure, 0.1, 3.0, outError) ||
            !ValidateColorPipelineParamRange("grade.saturation", saturation, 0.0, 2.0, outError) ||
            !ValidateColorPipelineParamRange("grade.contrast", contrast, 0.0, 3.0, outError)) {
            return false;
        }
        entry.params.exposure = static_cast<float>(exposure);
        entry.params.saturation = static_cast<float>(saturation);
        entry.params.contrast = static_cast<float>(contrast);
    } else if (row.function_id == "grade_glow") {
        double exposure = 0.0;
        double saturation = 0.0;
        double contrast = 0.0;
        double glow = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "grade.exposure", &exposure, outError) ||
            !TryGetColorPipelineParamNumber(row, "grade.saturation", &saturation, outError) ||
            !TryGetColorPipelineParamNumber(row, "grade.contrast", &contrast, outError) ||
            !TryGetColorPipelineParamNumber(row, "grade.glow", &glow, outError) ||
            !ValidateColorPipelineParamRange("grade.exposure", exposure, 0.1, 3.0, outError) ||
            !ValidateColorPipelineParamRange("grade.saturation", saturation, 0.0, 2.0, outError) ||
            !ValidateColorPipelineParamRange("grade.contrast", contrast, 0.0, 3.0, outError) ||
            !ValidateColorPipelineParamRange("grade.glow", glow, 0.0, 2.0, outError)) {
            return false;
        }
        entry.params.exposure = static_cast<float>(exposure);
        entry.params.saturation = static_cast<float>(saturation);
        entry.params.contrast = static_cast<float>(contrast);
        entry.params.glow = static_cast<float>(glow);
    } else if (row.function_id == "balance_void_grade") {
        double balanceVoid = 0.0;
        double chromaTension = 0.0;
        double accentBias = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "grade.balance_void", &balanceVoid, outError) ||
            !TryGetColorPipelineParamNumber(row, "grade.chroma_tension", &chromaTension, outError) ||
            !TryGetColorPipelineParamNumber(row, "grade.accent_bias", &accentBias, outError) ||
            !ValidateColorPipelineParamRange("grade.balance_void", balanceVoid, -1.0, 1.0, outError) ||
            !ValidateColorPipelineParamRange("grade.chroma_tension", chromaTension, -1.0, 1.0, outError) ||
            !ValidateColorPipelineParamRange("grade.accent_bias", accentBias, -1.0, 1.0, outError)) {
            return false;
        }
        entry.params.balance_void = static_cast<float>(balanceVoid);
        entry.params.chroma_tension = static_cast<float>(chromaTension);
        entry.params.accent_bias = static_cast<float>(accentBias);
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
            *outError = ColorPipelineUnsupportedShapeRowsMessage();
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
    if (ioRow->function_id == "neutral_finish" || ioRow->function_id == "tone_map_finish") {
        return SetColorPipelineParamNumber(ioRow, "grade.exposure", gradingEntry.params.exposure, outError) &&
            SetColorPipelineParamNumber(ioRow, "grade.saturation", gradingEntry.params.saturation, outError) &&
            SetColorPipelineParamNumber(ioRow, "grade.contrast", gradingEntry.params.contrast, outError);
    }
    if (ioRow->function_id == "grade_glow") {
        return SetColorPipelineParamNumber(ioRow, "grade.exposure", gradingEntry.params.exposure, outError) &&
            SetColorPipelineParamNumber(ioRow, "grade.saturation", gradingEntry.params.saturation, outError) &&
            SetColorPipelineParamNumber(ioRow, "grade.contrast", gradingEntry.params.contrast, outError) &&
            SetColorPipelineParamNumber(ioRow, "grade.glow", gradingEntry.params.glow, outError);
    }
    if (ioRow->function_id == "balance_void_grade") {
        return SetColorPipelineParamNumber(ioRow, "grade.balance_void", color_pipeline_core::NormalizeImportedColorPipelineNumber(gradingEntry.params.balance_void), outError) &&
            SetColorPipelineParamNumber(ioRow, "grade.chroma_tension", color_pipeline_core::NormalizeImportedColorPipelineNumber(gradingEntry.params.chroma_tension), outError) &&
            SetColorPipelineParamNumber(ioRow, "grade.accent_bias", color_pipeline_core::NormalizeImportedColorPipelineNumber(gradingEntry.params.accent_bias), outError);
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
    } else if (liveGradingEntry.grading == ColorGradingPreset::neutral_default ||
               liveGradingEntry.grading == ColorGradingPreset::tone_map_default) {
        liveGradingEntry.params.exposure = liveParams.exposure;
        liveGradingEntry.params.saturation = liveParams.color_saturation;
        liveGradingEntry.params.contrast = liveParams.color_contrast;
    } else if (liveGradingEntry.grading == ColorGradingPreset::glow_default) {
        liveGradingEntry.params.exposure = liveParams.exposure;
        liveGradingEntry.params.saturation = liveParams.color_saturation;
        liveGradingEntry.params.contrast = liveParams.color_contrast;
        liveGradingEntry.params.glow = liveParams.color_glow;
    } else if (liveGradingEntry.grading == ColorGradingPreset::balance_void_default) {
        liveGradingEntry.params.balance_void = liveParams.color_balance_void;
        liveGradingEntry.params.chroma_tension = liveParams.color_chroma_tension;
        liveGradingEntry.params.accent_bias = liveParams.color_accent_bias;
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
    const auto resetSourceSmoothEscape = [&]() {
        assignShapeFloat(&ioParams->color_smooth_escape_scale, 1.0f);
        assignShapeFloat(&ioParams->color_smooth_escape_bias, 0.0f);
    };
    const auto resetSourcePhaseOrbit = [&]() {
        assignShapeFloat(&ioParams->color_phase_signal_offset, 0.0f);
        assignShapeFloat(&ioParams->color_phase_wrap_cycles, 1.0f);
    };
    const auto resetSourceIterationBands = [&]() {
        assignShapeInt(&ioParams->color_iteration_band_count, 8);
        assignShapeFloat(&ioParams->color_iteration_band_softness, 0.35f);
    };
    const auto resetSourceEscapeMagnitude = [&]() {
        assignShapeFloat(&ioParams->color_escape_magnitude_scale, 1.0f);
        assignShapeFloat(&ioParams->color_escape_magnitude_bias, 0.0f);
    };
    const auto resetSourceOrbitStripe = [&]() {
        assignShapeFloat(&ioParams->color_orbit_stripe_frequency, 1.0f);
        assignShapeFloat(&ioParams->color_orbit_stripe_phase, 0.0f);
    };
    const auto resetSourceRootProximity = [&]() {
        assignShapeFloat(&ioParams->color_root_proximity_scale, 1.0f);
        assignShapeFloat(&ioParams->color_root_proximity_bias, 0.0f);
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
    if (!rootBasinPairSchedule && sourceRowsForStacks.size() > static_cast<std::size_t>(kColorPipelineMaxSourceStackCount)) {
        if (outError) {
            *outError = "Current row editor application only supports a bounded number of enabled Source rows in the Source stack.";
        }
        return false;
    }
    if (shapeRows.size() > static_cast<std::size_t>(kColorPipelineMaxShapeStackCount)) {
        if (outError) {
            *outError = "Current row editor application only supports a bounded number of enabled Shape rows in the schedule lane.";
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
            *outError = "Current row editor application only supports a bounded number of enabled Grading rows in the schedule lane.";
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

    std::vector<ColorPipelineSourceStackEntry> nextSourceStack;
    if (!rootBasinPairSchedule) {
        nextSourceStack.reserve(sourceRowsForStacks.size());
        for (const ColorPipelineRowState* sourceRow : sourceRowsForStacks) {
            ColorPipelineSourceStackEntry sourceEntry;
            if (!sourceRow || !TryBuildColorPipelineSourceStackEntryFromRow(*sourceRow, &sourceEntry, outError)) {
                return false;
            }
            nextSourceStack.push_back(sourceEntry);
        }
    }

    bool materializeSourceStack = true;
    if (!rootBasinPairSchedule &&
        ColorPipelineSelectionsEqual(ioParams->color_pipeline, effectivePipeline) &&
        ioParams->color_source_stack_count == 0 &&
        nextSourceStack.size() == 1) {
        ColorPipelineLaneState liveSourceLane;
        bool liveSourceImportSupported = true;
        std::string liveSourceError;
        ColorPipelineSourceStackEntry liveImplicitEntry;
        if (TryBuildColorPipelineSourceLaneFromLive(
                *ioParams,
                &liveSourceLane,
                &liveSourceImportSupported,
                &liveSourceError) &&
            liveSourceImportSupported &&
            liveSourceLane.rows.size() == 1 &&
            TryBuildColorPipelineSourceStackEntryFromRow(
                liveSourceLane.rows.front(),
                &liveImplicitEntry,
                &liveSourceError) &&
            ColorPipelineSourceStackEntriesEqual(liveImplicitEntry, nextSourceStack.front())) {
            materializeSourceStack = false;
        }
    }
    const int nextSourceStackCount = materializeSourceStack
        ? static_cast<int>(nextSourceStack.size())
        : 0;
    if (ioParams->color_source_stack_count != nextSourceStackCount) {
        ioParams->color_source_stack_count = nextSourceStackCount;
        changed = true;
    }
    for (int index = 0; index < kColorPipelineMaxSourceStackCount; ++index) {
        ColorPipelineSourceStackEntry nextEntry;
        if (materializeSourceStack && index < nextSourceStackCount) {
            nextEntry = nextSourceStack[static_cast<std::size_t>(index)];
        }
        if (!ColorPipelineSourceStackEntriesEqual(ioParams->color_source_stack[index], nextEntry)) {
            ioParams->color_source_stack[index] = nextEntry;
            changed = true;
        }
    }

    bool materializeShapeStack = true;
    if (ColorPipelineSelectionsEqual(ioParams->color_pipeline, effectivePipeline) &&
        ioParams->color_shape_stack_count == 0 &&
        nextShapeStack.size() == 1) {
        ColorPipelineLaneState liveShapeLane;
        bool liveShapeImportSupported = true;
        std::string liveShapeError;
        ColorPipelineShapeStackEntry liveImplicitEntry;
        if (TryBuildColorPipelineShapeLaneFromLive(
                *ioParams,
                &liveShapeLane,
                &liveShapeImportSupported,
                &liveShapeError) &&
            liveShapeImportSupported &&
            liveShapeLane.rows.size() == 1 &&
            TryBuildColorPipelineShapeStackEntryFromRow(
                liveShapeLane.rows.front(),
                &liveImplicitEntry,
                &liveShapeError) &&
            ColorPipelineShapeStackEntriesEqual(liveImplicitEntry, nextShapeStack.front())) {
            materializeShapeStack = false;
        }
    }
    const int nextShapeStackCount = materializeShapeStack
        ? static_cast<int>(nextShapeStack.size())
        : 0;
    if (ioParams->color_shape_stack_count != nextShapeStackCount) {
        ioParams->color_shape_stack_count = nextShapeStackCount;
        changed = true;
    }
    for (int index = 0; index < kColorPipelineMaxShapeStackCount; ++index) {
        ColorPipelineShapeStackEntry nextEntry;
        if (materializeShapeStack && index < nextShapeStackCount) {
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
                *outError = "Current row editor application only supports a bounded number of enabled Palette rows in the schedule lane.";
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
    bool materializePaletteStack = true;
    if (!rootBasinPairSchedule &&
        ColorPipelineSelectionsEqual(ioParams->color_pipeline, effectivePipeline) &&
        ioParams->color_palette_stack_count == 0 &&
        nextPaletteStack.size() == 1) {
        ColorPipelineLaneState livePaletteLane;
        bool livePaletteImportSupported = true;
        std::string livePaletteError;
        ColorPipelinePaletteStackEntry liveImplicitEntry;
        if (TryBuildColorPipelinePaletteLaneFromLive(
                *ioParams,
                &livePaletteLane,
                &livePaletteImportSupported,
                &livePaletteError) &&
            livePaletteImportSupported &&
            livePaletteLane.rows.size() == 1 &&
            TryBuildColorPipelinePaletteStackEntryFromRow(
                livePaletteLane.rows.front(),
                &liveImplicitEntry,
                &livePaletteError) &&
            ColorPipelinePaletteStackEntriesEqual(liveImplicitEntry, nextPaletteStack.front())) {
            materializePaletteStack = false;
        }
    }
    const int nextPaletteStackCount = materializePaletteStack
        ? static_cast<int>(nextPaletteStack.size())
        : 0;
    if (ioParams->color_palette_stack_count != nextPaletteStackCount) {
        ioParams->color_palette_stack_count = nextPaletteStackCount;
        changed = true;
    }
    for (int index = 0; index < kColorPipelineMaxPaletteStackCount; ++index) {
        ColorPipelinePaletteStackEntry nextEntry;
        if (materializePaletteStack && index < nextPaletteStackCount) {
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
    bool materializeGradingStack = true;
    if (ColorPipelineSelectionsEqual(ioParams->color_pipeline, effectivePipeline) &&
        ioParams->color_grading_stack_count == 0 &&
        nextGradingStack.size() == 1) {
        ColorPipelineLaneState liveGradingLane;
        bool liveGradingImportSupported = true;
        std::string liveGradingError;
        ColorPipelineGradingStackEntry liveImplicitEntry;
        if (TryBuildColorPipelineGradingLaneFromLive(
                *ioParams,
                &liveGradingLane,
                &liveGradingImportSupported,
                &liveGradingError) &&
            liveGradingImportSupported &&
            liveGradingLane.rows.size() == 1 &&
            TryBuildColorPipelineGradingStackEntryFromRow(
                liveGradingLane.rows.front(),
                &liveImplicitEntry,
                &liveGradingError) &&
            ColorPipelineGradingStackEntriesEqual(liveImplicitEntry, nextGradingStack.front())) {
            materializeGradingStack = false;
        }
    }
    const int nextGradingStackCount = materializeGradingStack
        ? static_cast<int>(nextGradingStack.size())
        : 0;
    if (ioParams->color_grading_stack_count != nextGradingStackCount) {
        ioParams->color_grading_stack_count = nextGradingStackCount;
        changed = true;
    }
    for (int index = 0; index < kColorPipelineMaxGradingStackCount; ++index) {
        ColorPipelineGradingStackEntry nextEntry;
        if (materializeGradingStack && index < nextGradingStackCount) {
            nextEntry = nextGradingStack[static_cast<std::size_t>(index)];
        }
        if (!ColorPipelineGradingStackEntriesEqual(ioParams->color_grading_stack[index], nextEntry)) {
            ioParams->color_grading_stack[index] = nextEntry;
            changed = true;
        }
    }

    ColorPipelineSourceStackEntry legacySourceEntry;
    if (!nextSourceStack.empty()) {
        legacySourceEntry = nextSourceStack.back();
    }
    if (!rootBasinPairSchedule && ioParams->color_pipeline.signal != legacySourceEntry.signal) {
        ioParams->color_pipeline.signal = legacySourceEntry.signal;
        changed = true;
    }
    resetSourceSmoothEscape();
    resetSourcePhaseOrbit();
    resetSourceIterationBands();
    resetSourceEscapeMagnitude();
    resetSourceOrbitStripe();
    resetSourceRootProximity();
    if (legacySourceEntry.signal == ColorSignal::smooth_escape) {
        assignShapeFloat(&ioParams->color_smooth_escape_scale, legacySourceEntry.params.scale);
        assignShapeFloat(&ioParams->color_smooth_escape_bias, legacySourceEntry.params.bias);
    } else if (legacySourceEntry.signal == ColorSignal::phase_angle) {
        assignShapeFloat(&ioParams->color_phase_signal_offset, legacySourceEntry.params.phase_offset);
        assignShapeFloat(&ioParams->color_phase_wrap_cycles, legacySourceEntry.params.wrap_cycles);
    } else if (legacySourceEntry.signal == ColorSignal::iteration_bands) {
        assignShapeInt(&ioParams->color_iteration_band_count, legacySourceEntry.params.band_count);
        assignShapeFloat(&ioParams->color_iteration_band_softness, legacySourceEntry.params.softness);
    } else if (legacySourceEntry.signal == ColorSignal::escape_magnitude) {
        assignShapeFloat(&ioParams->color_escape_magnitude_scale, legacySourceEntry.params.magnitude_scale);
        assignShapeFloat(&ioParams->color_escape_magnitude_bias, legacySourceEntry.params.magnitude_bias);
    } else if (legacySourceEntry.signal == ColorSignal::orbit_stripe) {
        assignShapeFloat(&ioParams->color_orbit_stripe_frequency, legacySourceEntry.params.stripe_frequency);
        assignShapeFloat(&ioParams->color_orbit_stripe_phase, legacySourceEntry.params.stripe_phase);
    } else if (legacySourceEntry.signal == ColorSignal::root_proximity) {
        assignShapeFloat(&ioParams->color_root_proximity_scale, legacySourceEntry.params.proximity_scale);
        assignShapeFloat(&ioParams->color_root_proximity_bias, legacySourceEntry.params.proximity_bias);
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
        assignShapeFloat(&ioParams->color_glow, 0.25f);
        assignShapeFloat(&ioParams->color_balance_void, 0.0f);
        assignShapeFloat(&ioParams->color_chroma_tension, 0.0f);
        assignShapeFloat(&ioParams->color_accent_bias, 0.0f);
        if (gradingMirrorEntry.grading == ColorGradingPreset::escape_default) {
            assignShapeFloat(&ioParams->color_contrast_lift_exposure, gradingMirrorEntry.params.exposure);
            assignShapeFloat(&ioParams->color_contrast_lift_saturation, gradingMirrorEntry.params.saturation);
        } else if (gradingMirrorEntry.grading == ColorGradingPreset::phase_default ||
                   gradingMirrorEntry.grading == ColorGradingPreset::bands_default) {
            assignShapeFloat(&ioParams->color_saturation, gradingMirrorEntry.params.saturation);
            assignShapeFloat(&ioParams->color_contrast, gradingMirrorEntry.params.contrast);
        } else if (gradingMirrorEntry.grading == ColorGradingPreset::neutral_default ||
                   gradingMirrorEntry.grading == ColorGradingPreset::tone_map_default) {
            assignShapeFloat(&ioParams->exposure, gradingMirrorEntry.params.exposure);
            assignShapeFloat(&ioParams->color_saturation, gradingMirrorEntry.params.saturation);
            assignShapeFloat(&ioParams->color_contrast, gradingMirrorEntry.params.contrast);
        } else if (gradingMirrorEntry.grading == ColorGradingPreset::glow_default) {
            assignShapeFloat(&ioParams->exposure, gradingMirrorEntry.params.exposure);
            assignShapeFloat(&ioParams->color_saturation, gradingMirrorEntry.params.saturation);
            assignShapeFloat(&ioParams->color_contrast, gradingMirrorEntry.params.contrast);
            assignShapeFloat(&ioParams->color_glow, gradingMirrorEntry.params.glow);
        } else if (gradingMirrorEntry.grading == ColorGradingPreset::balance_void_default) {
            assignShapeFloat(&ioParams->color_balance_void, gradingMirrorEntry.params.balance_void);
            assignShapeFloat(&ioParams->color_chroma_tension, gradingMirrorEntry.params.chroma_tension);
            assignShapeFloat(&ioParams->color_accent_bias, gradingMirrorEntry.params.accent_bias);
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
                *outError = std::string("Current row editor application only supports one enabled row in the ") + lane->label + " lane.";
            }
            return nullptr;
        }
        singleRow = &row;
    }
    if (!singleRow) {
        if (outError) {
            *outError = std::string("Current row editor application requires one enabled row in the ") + lane->label + " lane.";
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
    bool hasRootBasinPairFamily = false;
    bool hasSdfSourceRows = false;
    bool hasNonSdfSourceRows = false;
    for (const ColorPipelineRowState* sourceRow : sourceRows) {
        hasRootBasinPairFamily = hasRootBasinPairFamily || (sourceRow && sourceRow->function_id == "root_index");
        hasSdfSourceRows = hasSdfSourceRows || (sourceRow && IsSdfColorPipelineSourceFunctionId(sourceRow->function_id));
        hasNonSdfSourceRows = hasNonSdfSourceRows || (sourceRow && sourceRow->function_id != "root_index" && !IsSdfColorPipelineSourceFunctionId(sourceRow->function_id));
    }
    for (const ColorPipelineRowState* paletteRow : paletteRows) {
        hasRootBasinPairFamily = hasRootBasinPairFamily || (paletteRow && IsSupportedRootBasinPaletteFunctionId(paletteRow->function_id));
    }
    if (!hasRootBasinPairFamily &&
        paletteRows.size() > static_cast<std::size_t>(kColorPipelineMaxPaletteStackCount)) {
        if (outError) {
            *outError = "Current row editor application only supports a bounded number of enabled Palette rows in the schedule lane.";
        }
        return false;
    }
    if (shapeRows.size() > static_cast<std::size_t>(kColorPipelineMaxShapeStackCount)) {
        if (outError) {
            *outError = "Current row editor application only supports a bounded number of enabled Shape rows in the schedule lane.";
        }
        return false;
    }
    for (const ColorPipelineRowState* shapeRow : shapeRows) {
        if (!shapeRow || !IsSupportedColorPipelineShapeFunctionId(shapeRow->function_id)) {
            if (outError) {
                *outError = ColorPipelineUnsupportedShapeRowsMessage();
            }
            return false;
        }
    }
    if (hasSdfSourceRows && hasNonSdfSourceRows) {
        if (outError) {
            *outError = "Current live SDF color bridge requires enabled Source rows to be all SDF rows or all non-SDF rows.";
        }
        return false;
    }

    if (hasRootBasinPairFamily) {
        std::vector<ColorPipelineSelection> rootBasinPairs;
        ColoringMode mode = ColoringMode::root_basin;
        if (!TryBuildRootBasinPairSelectionsFromRows(sourceRows, paletteRows, &rootBasinPairs, &mode, outError)) {
            return false;
        }
        *outPipeline = rootBasinPairs.back();
        *outMode = mode;
        return true;
    }

    if (sourceRows.size() > static_cast<std::size_t>(kColorPipelineMaxSourceStackCount)) {
        if (outError) {
            *outError = "Current row editor application only supports a bounded number of enabled Source rows in the Source stack.";
        }
        return false;
    }
    for (const ColorPipelineRowState* sourceRow : sourceRows) {
        if (!sourceRow || !IsSupportedColorPipelineSourceStackFunctionId(sourceRow->function_id)) {
            if (outError) {
                *outError = "Current row editor application only supports shipped non-basin Source rows in the Source stack; root_index stays on the separate root-basin pair schedule.";
            }
            return false;
        }
    }
    for (const ColorPipelineRowState* paletteRow : paletteRows) {
        if (!paletteRow || !IsSupportedColorPipelinePaletteStackFunctionId(paletteRow->function_id)) {
            if (outError) {
                *outError = "Current row editor application only supports Heatmap, Phase Wheel, Banded Heatmap, and ExplainO CMap in the Palette stack.";
            }
            return false;
        }
    }

    const ColorPipelineRowState* sourceCompatibilityRow = sourceRows.back();
    if (hasSdfSourceRows && !hasNonSdfSourceRows) {
        // SDF postprocess uses the first Source row as the base signal; later SDF rows blend into it.
        sourceCompatibilityRow = sourceRows.front();
    }

    ColorPipelineSelection pipeline{};
    ColoringMode mode = ColoringMode::root_basin;
    for (const ColorPipelineRowState* paletteRow : paletteRows) {
        ColorPipelineSelection rowPipeline{};
        ColoringMode rowMode = ColoringMode::root_basin;
        if (!TryBuildColorPipelineSelectionFromLaneIds(
                sourceCompatibilityRow->function_id.c_str(),
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

    const ColorPipelineLaneCatalog* gradingCatalog = FindColorPipelineLaneCatalog("grading");
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
                *outError = "Current row editor application only supports a bounded number of enabled Grading rows in the schedule lane.";
            }
            return false;
        }
        for (const ColorPipelineRowState* gradingRow : gradingRows) {
            if (!gradingRow || !IsSupportedColorPipelineGradingFunctionId(gradingRow->function_id)) {
                if (outError) {
                    *outError = "Current row editor application only supports contrast_lift, phase_finish, band_finish, basin_default, neutral_finish, tone_map_finish, grade_glow, and balance_void_grade in the Grading stack.";
                }
                return false;
            }
        }
        if (!gradingRows.front() ||
            !TryParseAdvancedColorGradingFunctionId(gradingRows.front()->function_id, &pipeline.grading)) {
            if (outError) {
                *outError = "Selected Source / Shape / Palette recipe requires the first shipped Grading row to map to a runtime grading id.";
            }
            return false;
        }
    }
    const char* gradingFunctionId = AdvancedColorGradingFunctionId(pipeline.grading);
    if (gradingFunctionId && gradingFunctionId[0] != '\0') {
        if (!gradingCatalog || !FindColorPipelineFunctionDescriptor(*gradingCatalog, gradingFunctionId)) {
            if (outError) {
                *outError = "Selected Source / Shape / Palette recipe still depends on an unshipped Grading row and stays draft-only until that grading runtime integration lands.";
            }
            return false;
        }
        if (!gradingLane && gradingCatalog->default_function_id != std::string(gradingFunctionId)) {
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
            "Row stack matches the live runtime selection.",
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
                "Row stack matches the live runtime selection.",
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
        "Row stack differs from the live runtime selection.",
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

inline bool ShouldColorPipelineCandidateUseDraftOnlyLabel(
    const ColorPipelineWindowState& state,
    std::size_t laneIndex,
    std::size_t rowIndex,
    const char* functionId,
    FractalType liveFractalType,
    const KernelParams* liveParams = nullptr) {
    if (!functionId || functionId[0] == '\0' || laneIndex >= state.lanes.size()) {
        return true;
    }
    const ColorPipelineDraftApplyState candidateState = DescribeColorPipelineCandidateApplyState(
        state,
        laneIndex,
        rowIndex,
        functionId,
        liveFractalType,
        liveParams);
    if (candidateState.status == ColorPipelineDraftApplyStatus::can_apply ||
        candidateState.status == ColorPipelineDraftApplyStatus::matches_live) {
        return false;
    }
    const char* laneId = state.lanes[laneIndex].lane_id.c_str();
    if (!color_pipeline_core::IsColorPipelineFunctionRuntimeBacked(laneId, functionId)) {
        return true;
    }
    const ColorPipelineDraftApplyState baseState = DescribeColorPipelineDraftApplyState(state, liveFractalType, liveParams);
    return baseState.status == ColorPipelineDraftApplyStatus::can_apply ||
        baseState.status == ColorPipelineDraftApplyStatus::matches_live;
}

inline bool ShouldColorPipelineCandidateUseDraftOnlyLabel(
    const ColorPipelineWindowState& state,
    std::size_t laneIndex,
    const char* functionId,
    FractalType liveFractalType,
    const KernelParams* liveParams = nullptr) {
    return ShouldColorPipelineCandidateUseDraftOnlyLabel(state, laneIndex, 0, functionId, liveFractalType, liveParams);
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
    if (outChanged) {
        *outChanged = changed || paramChanged;
    }
    return true;
}

#ifndef COLOR_PIPELINE_WINDOW_NO_IMGUI
inline bool TryApplySupportedColorPipelineDraftFromControl(
    ColorPipelineWindowState* ioState,
    FractalType liveFractalType,
    KernelParams* liveParams,
    bool* ioDirty,
    ColorPipelineRenderInteractionState* ioInteraction);
#endif

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

template <typename T>
inline bool CommitColorPipelineNumericParamEdit(
    ColorPipelineWindowState* ioState,
    FractalType liveFractalType,
    KernelParams* liveParams,
    const NumericControlRange& range,
    T value,
    ColorPipelineParamState* ioValue,
    bool changed,
    bool itemActivated,
    bool itemActive,
    bool itemDeactivatedAfterEdit,
    bool* ioDirty = nullptr,
    ColorPipelineRenderInteractionState* ioInteraction = nullptr) {
    if (!ioValue) {
        return false;
    }
    NoteColorPipelineInteractionSnapshot(
        changed,
        itemActivated,
        itemActive,
        itemDeactivatedAfterEdit,
        ioInteraction);
    if (!changed) {
        return false;
    }
    ClampColorPipelineNumericValue(&value, range);
    ioValue->number_value = static_cast<double>(value);
    return TryApplySupportedColorPipelineDraftFromControl(
        ioState,
        liveFractalType,
        liveParams,
        ioDirty,
        ioInteraction);
}

template <typename T>
inline bool CommitColorPipelineNumericParamEditFromCurrentItem(
    ColorPipelineWindowState* ioState,
    FractalType liveFractalType,
    KernelParams* liveParams,
    const NumericControlRange& range,
    T value,
    ColorPipelineParamState* ioValue,
    bool changed,
    bool* ioDirty = nullptr,
    ColorPipelineRenderInteractionState* ioInteraction = nullptr) {
    return CommitColorPipelineNumericParamEdit(
        ioState,
        liveFractalType,
        liveParams,
        range,
        value,
        ioValue,
        changed,
        ImGui::IsItemActivated(),
        ImGui::IsItemActive(),
        ImGui::IsItemDeactivatedAfterEdit(),
        ioDirty,
        ioInteraction);
}

template <typename T>
inline bool TryApplyColorPipelineUiAutomationSetValue(
    ColorPipelineWindowState* ioState,
    const char* primaryControlId,
    FractalType liveFractalType,
    KernelParams* liveParams,
    const NumericControlRange& range,
    T* ioWidgetValue,
    ColorPipelineParamState* ioValue,
    bool* ioDirty = nullptr,
    ColorPipelineRenderInteractionState* ioInteraction = nullptr) {
    if (!ioState || !primaryControlId || !ioState->ui_automation_set_pending ||
        ioState->ui_automation_set_consumed || ioState->ui_automation_set_control_id != primaryControlId) {
        return false;
    }

    T requestedValue{};
    if constexpr (std::is_integral<T>::value) {
        requestedValue = static_cast<T>(std::lround(ioState->ui_automation_set_control_value));
    } else {
        requestedValue = static_cast<T>(ioState->ui_automation_set_control_value);
    }

    const bool applied = CommitColorPipelineNumericParamEdit(
        ioState,
        liveFractalType,
        liveParams,
        range,
        requestedValue,
        ioValue,
        true,
        false,
        false,
        true,
        ioDirty,
        ioInteraction);
    if (!applied) {
        ioState->ui_automation_set_error = std::string("color pipeline row editor rejected visible control: ") + primaryControlId;
        return false;
    }
    if (ioWidgetValue) {
        *ioWidgetValue = requestedValue;
    }
    ioState->ui_automation_set_consumed = true;
    ioState->ui_automation_set_error.clear();
    return true;
}

inline bool ApplyColorPipelineSdfFieldDownsampleValue(
    ColorPipelineWindowState* ioState,
    LensSettings* liveLens,
    int requestedValue,
    bool* ioDirty = nullptr,
    ColorPipelineRenderInteractionState* ioInteraction = nullptr) {
    if (!liveLens) {
        if (ioState) {
            ioState->ui_automation_set_error = "SDF Field Downsample requires live Lens settings";
        }
        return false;
    }
    const int nextValue = NormalizeColorPipelineSdfFieldDownsampleValue(requestedValue);
    if (liveLens->downsample != nextValue) {
        liveLens->downsample = nextValue;
        if (ioDirty) {
            *ioDirty = true;
        }
        if (ioInteraction) {
            ioInteraction->interacted = true;
        }
    }
    if (ioState) {
        ioState->ui_automation_set_error.clear();
    }
    return true;
}

#ifndef COLOR_PIPELINE_WINDOW_NO_IMGUI
inline bool TryApplyColorPipelineSdfFieldDownsampleAutomation(
    ColorPipelineWindowState* ioState,
    LensSettings* liveLens,
    bool* ioDirty = nullptr,
    ColorPipelineRenderInteractionState* ioInteraction = nullptr) {
    const std::string controlId = BuildColorPipelineSdfFieldDownsampleControlId();
    if (!ioState || !ioState->ui_automation_set_pending || ioState->ui_automation_set_consumed ||
        ioState->ui_automation_set_control_id != controlId) {
        return false;
    }
    const int requestedValue = static_cast<int>(std::lround(ioState->ui_automation_set_control_value));
    if (!ApplyColorPipelineSdfFieldDownsampleValue(ioState, liveLens, requestedValue, ioDirty, ioInteraction)) {
        return false;
    }
    ioState->ui_automation_set_consumed = true;
    return true;
}
#endif

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
        const bool automationChanged = TryApplyColorPipelineUiAutomationSetValue(
            ioState,
            primaryControlId,
            liveFractalType,
            liveParams,
            range,
            &value,
            ioValue,
            ioDirty,
            ioInteraction);
        CommitColorPipelineNumericParamEditFromCurrentItem(
            ioState,
            liveFractalType,
            liveParams,
            range,
            value,
            ioValue,
            sliderChanged,
            ioDirty,
            ioInteraction);
        ImGui::SameLine();
        const bool typedChanged = ImGui::InputFloat("##value_input", &value, 0.0f, 0.0f, "%.5f");
        CommitColorPipelineNumericParamEditFromCurrentItem(
            ioState,
            liveFractalType,
            liveParams,
            range,
            value,
            ioValue,
            typedChanged,
            ioDirty,
            ioInteraction);
        changed = sliderChanged || typedChanged || automationChanged;
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
        const bool automationChanged = TryApplyColorPipelineUiAutomationSetValue(
            ioState,
            primaryControlId,
            liveFractalType,
            liveParams,
            range,
            &value,
            ioValue,
            ioDirty,
            ioInteraction);
        CommitColorPipelineNumericParamEditFromCurrentItem(
            ioState,
            liveFractalType,
            liveParams,
            range,
            value,
            ioValue,
            sliderChanged,
            ioDirty,
            ioInteraction);
        ImGui::SameLine();
        const bool typedChanged = ImGui::InputDouble("##value_input", &value, 0.0, 0.0, "%.6f");
        CommitColorPipelineNumericParamEditFromCurrentItem(
            ioState,
            liveFractalType,
            liveParams,
            range,
            value,
            ioValue,
            typedChanged,
            ioDirty,
            ioInteraction);
        changed = sliderChanged || typedChanged || automationChanged;
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
        const bool automationChanged = TryApplyColorPipelineUiAutomationSetValue(
            ioState,
            primaryControlId,
            liveFractalType,
            liveParams,
            range,
            &value,
            ioValue,
            ioDirty,
            ioInteraction);
        CommitColorPipelineNumericParamEditFromCurrentItem(
            ioState,
            liveFractalType,
            liveParams,
            range,
            value,
            ioValue,
            sliderChanged,
            ioDirty,
            ioInteraction);
        ImGui::SameLine();
        const bool typedChanged = ImGui::InputInt("##value_input", &value, 0, 0);
        CommitColorPipelineNumericParamEditFromCurrentItem(
            ioState,
            liveFractalType,
            liveParams,
            range,
            value,
            ioValue,
            typedChanged,
            ioDirty,
            ioInteraction);
        changed = sliderChanged || typedChanged || automationChanged;
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

inline bool ConsumeColorPipelineAutomationClick(
    ColorPipelineWindowState* ioState,
    const std::string& controlId) {
    if (!ioState ||
        !ioState->ui_automation_click_pending ||
        ioState->ui_automation_click_consumed ||
        ioState->ui_automation_click_control_id != controlId) {
        return false;
    }
    ioState->ui_automation_click_consumed = true;
    return true;
}

inline bool ApplyColorPipelineRecipePresetToLive(
    ColorPipelineWindowState* ioState,
    const char* recipeId,
    FractalType liveFractalType,
    KernelParams* liveParams,
    bool* ioDirty,
    ColorPipelineRenderInteractionState* ioInteraction = nullptr) {
    if (!ApplyColorPipelineRecipeToDraft(ioState, recipeId)) {
        return false;
    }
    bool changed = false;
    if (liveParams && ApplyColorPipelineDraftToLiveState(ioState, liveFractalType, liveParams, &changed)) {
        if (changed && ioDirty) {
            *ioDirty = true;
        }
        if (changed && ioInteraction) {
            ioInteraction->interacted = true;
        }
    } else if (ioInteraction) {
        ioInteraction->interacted = true;
    }
    return true;
}

inline void RenderColorPipelineRecipePresetControls(
    ColorPipelineWindowState* ioState,
    FractalType liveFractalType,
    KernelParams* liveParams,
    bool* ioDirty,
    ColorPipelineRenderInteractionState* ioInteraction = nullptr) {
    if (!ioState) {
        return;
    }
    const std::vector<MaterializedColorPipelineRecipe>& recipes = color_pipeline_core::GetActiveColorPipelineRecipes();
    if (recipes.empty()) {
        return;
    }
    ImGui::TextDisabled(ColorPipelineWindowSupportedPresetSummaryText());
    for (std::size_t index = 0; index < recipes.size(); ++index) {
        const MaterializedColorPipelineRecipe& recipe = recipes[index];
        const std::string controlId = BuildColorPipelineRecipeApplyControlId(recipe.id);
        if (index > 0) {
            ImGui::SameLine();
        }
        ImGui::PushID(recipe.id.c_str());
        const bool clicked = ImGui::Button(recipe.label.c_str());
        NoteColorPipelineUiAutomationRect(ioState, controlId.c_str());
        const bool automationClicked = ConsumeColorPipelineAutomationClick(ioState, controlId);
        if (clicked || automationClicked) {
            ApplyColorPipelineRecipePresetToLive(
                ioState,
                recipe.id.c_str(),
                liveFractalType,
                liveParams,
                ioDirty,
                ioInteraction);
        }
        ImGui::PopID();
    }
}

inline void RenderColorPipelineWindowSummary(
    ColorPipelineWindowState* ioState,
    FractalType liveFractalType,
    KernelParams* liveParams,
    bool* ioDirty,
    ColorPipelineRenderInteractionState* ioInteraction = nullptr) {
    (void)ioDirty;
    (void)ioInteraction;
    ImGui::TextWrapped(ColorPipelineWindowDraftRecipesIntroText());
    ImGui::TextDisabled(ColorPipelineWindowLaneModelSummaryText());
    ImGui::TextDisabled(ColorPipelineWindowBridgeBoundarySummaryText());
    RenderColorPipelineRecipePresetControls(ioState, liveFractalType, liveParams, ioDirty, ioInteraction);
    ImGui::TextDisabled(ColorPipelineWindowFixedPresetHelpText());
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
                "Current color selection: %s -> %s / %s / %s",
                ColoringModeId(ioState->live_snapshot.coloring_mode),
                signalId ? signalId : "(unsupported signal)",
                shapeId,
                paletteId ? paletteId : "(unsupported palette)");
        } else if (ioState->live_snapshot.valid) {
            ImGui::TextWrapped(
                "Current color selection: %s (outside the current Source / Shape / Palette preset path)",
                ColoringModeId(ioState->live_snapshot.coloring_mode));
            ImGui::TextDisabled("The row editor keeps its own starter state until the current color selection maps onto a supported Source / Shape / Palette preset.");
        } else {
            ImGui::TextWrapped("Current color selection is invalid or out of sync with the row editor.");
            ImGui::TextDisabled("The current supported row stack can still repair the selection; reset from current color returns once the state is coherent again.");
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
                ImGui::TextDisabled("Supported selections right now: Smooth Escape / Escape Magnitude / Root Proximity with Heatmap or Explaino Cmap, Phase Orbit / Orbit Stripe with Phase Wheel, Iteration Bands with Banded Heatmap, and Root Index with Root Classic Palette or Joy Root Palette.");
            }
        }
        if (!ioState->live_snapshot.valid || !ioState->live_snapshot.draft_import_supported) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Reset From Current Color")) {
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

inline void RenderColorPipelineSdfFieldDownsampleAlias(
    ColorPipelineWindowState* ioState,
    LensSettings* liveLens,
    bool* ioDirty = nullptr,
    ColorPipelineRenderInteractionState* ioInteraction = nullptr) {
    if (!ioState || !ColorPipelineSdfFieldDownsampleControlVisible(*ioState, liveLens)) {
        return;
    }
    const std::string controlId = BuildColorPipelineSdfFieldDownsampleControlId();
    int value = NormalizeColorPipelineSdfFieldDownsampleValue(liveLens->downsample);
    const int values[] = {1, 2, 4, 8, 16};
    const char* labels[] = {"1x (Full)", "2x", "4x", "8x", "16x"};
    int currentIndex = 0;
    for (int index = 0; index < 5; ++index) {
        if (values[index] == value) {
            currentIndex = index;
            break;
        }
    }
    bool changed = false;
    if (ImGui::BeginCombo("SDF Field Downsample", labels[currentIndex])) {
        for (int index = 0; index < 5; ++index) {
            const bool selected = (index == currentIndex);
            if (ImGui::Selectable(labels[index], selected)) {
                value = values[index];
                changed = true;
                currentIndex = index;
            }
            if (selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    NoteColorPipelineUiAutomationRect(ioState, controlId.c_str());
    const bool automationChanged = TryApplyColorPipelineSdfFieldDownsampleAutomation(
        ioState,
        liveLens,
        ioDirty,
        ioInteraction);
    if (changed) {
        ApplyColorPipelineSdfFieldDownsampleValue(ioState, liveLens, value, ioDirty, ioInteraction);
    }
    if ((changed || automationChanged) && ioInteraction) {
        ioInteraction->interacted = true;
    }
}

inline void RenderColorPipelineWindowLane(
    ColorPipelineWindowState* ioState,
    std::size_t laneIndex,
    FractalType liveFractalType,
    KernelParams* liveParams,
    LensSettings* liveLens = nullptr,
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

    if (lane.lane_id == "source") {
        RenderColorPipelineSdfFieldDownsampleAlias(ioState, liveLens, ioDirty, ioInteraction);
    }

    for (std::size_t rowIndex = 0; rowIndex < lane.rows.size(); ++rowIndex) {
        ColorPipelineRowState& row = lane.rows[rowIndex];
        const FunctionDescriptor* descriptor = FindColorPipelineFunctionDescriptor(*catalog, row.function_id);
        const char* rowLabel = descriptor ? descriptor->name.c_str() : row.function_id.c_str();

        const bool rowEnabledBefore = row.enabled;
        bool requestedEnabled = rowEnabledBefore;

        ImGuiStackEditorRowChromeSpec rowSpec;
        rowSpec.tree_node_id = lane.lane_id.c_str();
        rowSpec.header_label = rowLabel;
        rowSpec.stable_row_id = row.ui_row_id;
        rowSpec.enabled = &requestedEnabled;
        const std::string enabledControlId = BuildColorPipelineRowEnabledControlId(lane.lane_id, row.ui_row_id);
        const std::string removeControlId = BuildColorPipelineRowRemoveControlId(lane.lane_id, row.ui_row_id);
        rowSpec.enabled_control_id = enabledControlId.c_str();
        rowSpec.remove_control_id = removeControlId.c_str();
        rowSpec.note_item_rect = NoteColorPipelineUiAutomationRectFromStackEditor;
        rowSpec.note_item_rect_user_data = ioState;
        rowSpec.emulate_activate_control_id = ioState->ui_automation_click_pending
            ? ioState->ui_automation_click_control_id.c_str()
            : nullptr;
        rowSpec.emulate_activate_consumed = ioState->ui_automation_click_pending
            ? &ioState->ui_automation_click_consumed
            : nullptr;
        rowSpec.allow_remove = lane.rows.size() > 1;
        rowSpec.allow_move_up = rowIndex > 0;
        rowSpec.allow_move_down = rowIndex + 1 < lane.rows.size();
        if (ioState->force_open_for_automation) {
            ImGui::SetNextItemOpen(true, ImGuiCond_Always);
        }
        const ImGuiStackEditorRowChromeResult rowResult = RenderImGuiStackEditorRowChrome(rowSpec, [&]() {
            const FunctionDescriptor* currentDescriptor = FindColorPipelineFunctionDescriptor(*catalog, row.function_id);
            const char* comboPreview = (currentDescriptor && !currentDescriptor->name.empty())
                ? currentDescriptor->name.c_str()
                : "(select)";
            if (ImGui::BeginCombo("Function", comboPreview)) {
                for (const FunctionDescriptor& candidate : catalog->functions) {
                    const bool isSelected = (candidate.id == row.function_id);
                    const bool candidateDraftOnly = ShouldColorPipelineCandidateUseDraftOnlyLabel(
                        *ioState,
                        laneIndex,
                        rowIndex,
                        candidate.id.c_str(),
                        liveFractalType,
                        liveParams);
                    std::string optionLabel = candidate.name;
                    if (!isSelected && candidateDraftOnly) {
                        optionLabel += " (unsupported for current selection)";
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
                ImGui::TextDisabled("This fixed %s row has no tunable parameters; choosing it changes the current Color Pipeline selection directly.", lane.label.c_str());
            } else if (!currentDescriptor->parameters.empty() && renderableParamIndexes.empty()) {
                ImGui::TextDisabled("Parameter tuning preview only in this slice.");
            } else if (hasHiddenParams) {
                ImGui::TextDisabled("Only the visible controls are live in this slice.");
            }
            if (!currentDescriptor->parameters.empty() && lane.lane_id == "shape") {
                ImGui::TextDisabled(ColorPipelineShapeRowBridgeHelpText());
            }
        });

        if (requestedEnabled != rowEnabledBefore) {
            SetColorPipelineRowEnabledFromUi(ioState, laneIndex, rowIndex, requestedEnabled);
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
    LensSettings* liveLens = nullptr,
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
            RenderColorPipelineWindowLane(ioState, laneIndex, liveFractalType, liveParams, liveLens, ioDirty, &interactionState);
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
    return RenderColorPipelineWindow(ioState, FractalType::explaino, nullptr, nullptr);
}
#endif

