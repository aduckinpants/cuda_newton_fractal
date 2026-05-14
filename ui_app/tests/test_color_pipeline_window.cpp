#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#define COLOR_PIPELINE_WINDOW_NO_IMGUI
#include "../src/color_pipeline_window.h"
#undef COLOR_PIPELINE_WINDOW_NO_IMGUI

namespace {

int g_passed = 0;
int g_failed = 0;

void Check(bool condition, const char* message) {
    if (condition) {
        ++g_passed;
    } else {
        ++g_failed;
        std::printf("  FAIL: %s\n", message);
    }
}

bool Near(double actual, double expected, double eps = 1.0e-6) {
    return std::fabs(actual - expected) <= eps;
}

ColorPipelineLaneState* FindLane(ColorPipelineWindowState* state, const char* laneId) {
    if (!state || !laneId) {
        return nullptr;
    }
    for (ColorPipelineLaneState& lane : state->lanes) {
        if (lane.lane_id == laneId) {
            return &lane;
        }
    }
    return nullptr;
}

const ColorPipelineLaneState* FindLane(const ColorPipelineWindowState& state, const char* laneId) {
    for (const ColorPipelineLaneState& lane : state.lanes) {
        if (lane.lane_id == laneId) {
            return &lane;
        }
    }
    return nullptr;
}

bool SetRowNumber(ColorPipelineRowState& row, const char* path, double value) {
    return SetColorPipelineParamNumber(&row, path, value);
}

bool RowNumber(const ColorPipelineRowState& row, const char* path, double expected) {
    double actual = 0.0;
    return TryGetColorPipelineParamNumber(row, path, &actual) && Near(actual, expected);
}

KernelParams SmoothEscapeParams() {
    KernelParams params{};
    params.coloring_mode = ColoringMode::smooth_escape;
    params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
    params.color_shape = ColorPipelineShape::identity;
    params.color_shape_stack_count = 0;
    return params;
}

void TestInitializationAndLiveSync() {
    ColorPipelineWindowState state{};
    Check(!EnsureColorPipelineWindowInitialized(nullptr), "TestInitializationAndLiveSync_NullInitFails");
    Check(EnsureColorPipelineWindowInitialized(&state), "TestInitializationAndLiveSync_Initializes");
    Check(state.initialized && state.lanes.size() == 4 && state.next_row_id == 5,
        "TestInitializationAndLiveSync_DefaultLaneShape");
    Check(state.lanes[0].lane_id == "source" && state.lanes[1].lane_id == "shape" &&
            state.lanes[2].lane_id == "palette" && state.lanes[3].lane_id == "grading",
        "TestInitializationAndLiveSync_DefaultLaneOrder");
    Check(state.lanes[0].rows[0].ui_row_id == 1 && state.lanes[1].rows[0].ui_row_id == 2 &&
            state.lanes[2].rows[0].ui_row_id == 3 && state.lanes[3].rows[0].ui_row_id == 4,
        "TestInitializationAndLiveSync_StableInitialRowIds");
    Check(IsColorPipelineStarterDraft(state), "TestInitializationAndLiveSync_StarterDraftRecognized");

    KernelParams params = SmoothEscapeParams();
    params.color_shape = ColorPipelineShape::offset_scale;
    params.color_shape_offset = 0.25f;
    params.color_shape_scale = 1.5f;
    Check(SyncColorPipelineWindowFromLiveState(&state, FractalType::newton, &params),
        "TestInitializationAndLiveSync_SyncSupportedLiveTuple");
    Check(state.live_snapshot.valid && state.live_snapshot.draft_import_supported && state.live_snapshot.lanes.size() == 4,
        "TestInitializationAndLiveSync_LiveSnapshotSupported");
    Check(state.lanes[1].rows[0].function_id == "offset_scale" &&
            RowNumber(state.lanes[1].rows[0], "shape.offset", 0.25) &&
            RowNumber(state.lanes[1].rows[0], "shape.scale", 1.5),
        "TestInitializationAndLiveSync_ImportsShapeOwnerValues");
    const std::uint64_t sourceRowId = state.lanes[0].rows[0].ui_row_id;
    const std::uint64_t shapeRowId = state.lanes[1].rows[0].ui_row_id;
    Check(SyncColorPipelineWindowFromLiveState(&state, FractalType::newton, &params),
        "TestInitializationAndLiveSync_SecondSyncSucceeds");
    Check(state.lanes[0].rows[0].ui_row_id == sourceRowId && state.lanes[1].rows[0].ui_row_id == shapeRowId,
        "TestInitializationAndLiveSync_SecondSyncPreservesRowIds");
}

void TestRootSelectionCoSwitchesAndApplies() {
    ColorPipelineWindowState state{};
    KernelParams params = SmoothEscapeParams();
    Check(SyncColorPipelineWindowFromLiveState(&state, FractalType::newton, &params),
        "TestRootSelectionCoSwitchesAndApplies_SyncStartsSupported");

    const ColorPipelineDraftApplyState candidateState = DescribeColorPipelineCandidateApplyState(
        state,
        0,
        "root_index",
        FractalType::newton,
        &params);
    Check(candidateState.status == ColorPipelineDraftApplyStatus::can_apply,
        "TestRootSelectionCoSwitchesAndApplies_RootCandidateCanApply");
    Check(SelectColorPipelineLaneFunction(&state, 0, "root_index"),
        "TestRootSelectionCoSwitchesAndApplies_SelectRootIndex");
    Check(state.lanes[0].rows[0].function_id == "root_index" &&
            state.lanes[2].rows[0].function_id == "root_classic_palette" &&
            state.lanes[3].rows[0].function_id == "basin_default" &&
            state.lanes[3].rows[0].parameter_values.empty(),
        "TestRootSelectionCoSwitchesAndApplies_CoSwitchesRootPaletteAndBasinGrading");
    Check(HasColorPipelineDraftEdits(state), "TestRootSelectionCoSwitchesAndApplies_DraftEditDetected");

    bool changed = false;
    Check(ApplyColorPipelineDraftToLiveState(&state, FractalType::newton, &params, &changed) && changed,
        "TestRootSelectionCoSwitchesAndApplies_ApplyChangesRuntimeTuple");
    Check(params.coloring_mode == ColoringMode::root_basin &&
            params.color_pipeline.signal == ColorSignal::root_index &&
            params.color_pipeline.palette == ColorPalette::root_classic &&
            params.color_pipeline.grading == ColorGradingPreset::basin_default &&
            params.color_grading_stack_count == 1 &&
            params.color_grading_stack[0].grading == ColorGradingPreset::basin_default,
        "TestRootSelectionCoSwitchesAndApplies_RuntimeTupleIsRootClassic");
    Check(state.live_snapshot.valid && state.live_snapshot.draft_import_supported &&
            state.live_snapshot.lanes.size() == 4 &&
            state.live_snapshot.lanes[3].rows.size() == 1 &&
            state.live_snapshot.lanes[3].rows[0].function_id == "basin_default" &&
            state.live_snapshot.lanes[3].rows[0].parameter_values.empty() &&
            !HasColorPipelineDraftEdits(state),
        "TestRootSelectionCoSwitchesAndApplies_ApplyResyncsSupportedSnapshotWithBasinGrading");
    Check(!SelectColorPipelineLaneFunction(&state, 0, "missing_source") &&
            state.lanes[0].rows[0].function_id == "root_index" && !state.validation_messages.empty(),
        "TestRootSelectionCoSwitchesAndApplies_UnknownFunctionFailsClosed");
}

void TestShapeStackApplyAndValidation() {
    ColorPipelineWindowState state{};
    KernelParams params = SmoothEscapeParams();
    Check(EnsureColorPipelineWindowInitialized(&state), "TestShapeStackApplyAndValidation_Initializes");
    Check(SelectColorPipelineLaneFunction(&state, 1, "offset_scale") &&
            SetRowNumber(state.lanes[1].rows[0], "shape.offset", 0.25) &&
            SetRowNumber(state.lanes[1].rows[0], "shape.scale", 1.5),
        "TestShapeStackApplyAndValidation_ConfiguresOffsetScale");
    Check(AddColorPipelineLaneRow(&state, 1, "repeat") && state.lanes[1].rows.size() == 2,
        "TestShapeStackApplyAndValidation_AddsRepeatRow");
    Check(SetRowNumber(state.lanes[1].rows[1], "shape.frequency", 6.0) &&
            SetRowNumber(state.lanes[1].rows[1], "shape.phase", 0.2),
        "TestShapeStackApplyAndValidation_ConfiguresRepeatRow");
    Check(MoveColorPipelineLaneRow(&state, 1, 1, -1) && state.lanes[1].rows[0].function_id == "repeat",
        "TestShapeStackApplyAndValidation_MoveRepeatUp");
    Check(MoveColorPipelineLaneRow(&state, 1, 0, 1) && state.lanes[1].rows[1].function_id == "repeat",
        "TestShapeStackApplyAndValidation_MoveRepeatDown");

    bool changed = false;
    Check(ApplyColorPipelineDraftToLiveState(&state, FractalType::newton, &params, &changed) && changed,
        "TestShapeStackApplyAndValidation_ApplyStackSucceeds");
    Check(params.color_shape_stack_count == 2 &&
            params.color_shape_stack[0].shape == ColorPipelineShape::offset_scale &&
            Near(params.color_shape_stack[0].params.offset, 0.25) &&
            Near(params.color_shape_stack[0].params.scale, 1.5) &&
            params.color_shape_stack[1].shape == ColorPipelineShape::repeat &&
            Near(params.color_shape_stack[1].params.repeat_frequency, 6.0) &&
            Near(params.color_shape_stack[1].params.repeat_phase, 0.2),
        "TestShapeStackApplyAndValidation_RuntimeShapeStackWritten");
    Check(params.color_shape == ColorPipelineShape::repeat &&
            Near(params.color_shape_repeat_frequency, 6.0) && Near(params.color_shape_repeat_phase, 0.2),
        "TestShapeStackApplyAndValidation_LegacyMirrorUsesLastShapeRow");
    Check(state.live_snapshot.valid && state.live_snapshot.lanes[1].rows.size() == 2 && !HasColorPipelineDraftEdits(state),
        "TestShapeStackApplyAndValidation_ApplyResyncsShapeStackDraft");

    Check(RemoveColorPipelineLaneRow(&state, 1, 1) && state.lanes[1].rows.size() == 1,
        "TestShapeStackApplyAndValidation_RemoveSecondShapeRow");
    Check(!RemoveColorPipelineLaneRow(&state, 1, 0),
        "TestShapeStackApplyAndValidation_CannotRemoveLastShapeRow");

    ColorPipelineWindowState invalidState{};
    KernelParams invalidParams = SmoothEscapeParams();
    Check(EnsureColorPipelineWindowInitialized(&invalidState) &&
            SelectColorPipelineLaneFunction(&invalidState, 1, "offset_scale") &&
            SetRowNumber(invalidState.lanes[1].rows[0], "shape.scale", 0.01),
        "TestShapeStackApplyAndValidation_ConfiguresInvalidScale");
    const ColorPipelineDraftApplyState invalidApplyState = DescribeColorPipelineDraftApplyState(
        invalidState,
        FractalType::newton,
        &invalidParams);
    Check(invalidApplyState.status == ColorPipelineDraftApplyStatus::invalid_params,
        "TestShapeStackApplyAndValidation_InvalidScaleClassified");
    Check(!ApplyColorPipelineDraftToLiveState(&invalidState, FractalType::newton, &invalidParams) &&
            !invalidState.validation_messages.empty() &&
            invalidParams.color_shape == ColorPipelineShape::identity,
        "TestShapeStackApplyAndValidation_InvalidScaleFailsBeforeMutation");
}

void TestIterationBandsLiveImportShipsBandFinish() {
    ColorPipelineWindowState state{};
    KernelParams params{};
    params.coloring_mode = ColoringMode::iteration_bands;
    params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::iteration_bands);
    params.color_shape = ColorPipelineShape::identity;
    params.color_saturation = 0.75f;
    params.color_contrast = 1.8f;

    Check(SyncColorPipelineWindowFromLiveState(&state, FractalType::newton, &params),
        "TestIterationBandsLiveImportShipsBandFinish_SyncObservesLiveTuple");
    Check(state.live_snapshot.valid && state.live_snapshot.draft_import_supported && state.live_snapshot.lanes.size() == 4,
        "TestIterationBandsLiveImportShipsBandFinish_SnapshotMarkedDraftSupported");
    Check(state.lanes.size() == 4 &&
            state.lanes[0].rows[0].function_id == "banded_signal" &&
            state.lanes[2].rows[0].function_id == "banded_heatmap" &&
            state.lanes[3].rows[0].function_id == "band_finish" &&
            state.lanes[3].rows[0].parameter_values.size() == 2 &&
            state.lanes[3].rows[0].parameter_values[0].path == "grade.saturation" &&
            Near(state.lanes[3].rows[0].parameter_values[0].number_value, 0.75) &&
            state.lanes[3].rows[0].parameter_values[1].path == "grade.contrast" &&
            Near(state.lanes[3].rows[0].parameter_values[1].number_value, 1.8),
        "TestIterationBandsLiveImportShipsBandFinish_DraftImportsBandRows");
    Check(ResetColorPipelineDraftFromLiveState(&state) && state.validation_messages.empty(),
        "TestIterationBandsLiveImportShipsBandFinish_ResetFromLiveImportsBandRows");

    ColorPipelineLiveSnapshot missingOutput;
    std::string error;
    Check(!TryBuildColorPipelineLiveSnapshot(FractalType::newton, params, nullptr, &error) &&
            error.find("requires an output") != std::string::npos,
        "TestIterationBandsLiveImportShipsBandFinish_LiveSnapshotRequiresOutput");
    params.coloring_mode = ColoringMode::phase;
    params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
    Check(!TryBuildColorPipelineLiveSnapshot(FractalType::newton, params, &missingOutput, &error) &&
            error.find("out of sync") != std::string::npos,
        "TestIterationBandsLiveImportShipsBandFinish_OutOfSyncLiveFailsClosed");
}


void TestGradingStackApplyAndLiveImport() {
    ColorPipelineWindowState state{};
    KernelParams params = SmoothEscapeParams();
    Check(EnsureColorPipelineWindowInitialized(&state), "TestGradingStackApplyAndLiveImport_Initializes");
    Check(SelectColorPipelineLaneFunction(&state, 3, "contrast_lift") &&
            SetRowNumber(state.lanes[3].rows[0], "grade.exposure", 1.4) &&
            SetRowNumber(state.lanes[3].rows[0], "grade.saturation", 1.2),
        "TestGradingStackApplyAndLiveImport_ConfiguresContrastLift");
    Check(AddColorPipelineLaneRow(&state, 3, "phase_finish") && state.lanes[3].rows.size() == 2,
        "TestGradingStackApplyAndLiveImport_AddsPhaseFinishRow");
    Check(SetRowNumber(state.lanes[3].rows[1], "grade.saturation", 0.8) &&
            SetRowNumber(state.lanes[3].rows[1], "grade.contrast", 1.6),
        "TestGradingStackApplyAndLiveImport_ConfiguresPhaseFinish");

    bool changed = false;
    Check(ApplyColorPipelineDraftToLiveState(&state, FractalType::newton, &params, &changed) && changed,
        "TestGradingStackApplyAndLiveImport_ApplyStackSucceeds");
    Check(params.color_grading_stack_count == 2 &&
            params.color_grading_stack[0].grading == ColorGradingPreset::escape_default &&
            Near(params.color_grading_stack[0].params.exposure, 1.4) &&
            Near(params.color_grading_stack[0].params.saturation, 1.2) &&
            params.color_grading_stack[1].grading == ColorGradingPreset::phase_default &&
            Near(params.color_grading_stack[1].params.saturation, 0.8) &&
            Near(params.color_grading_stack[1].params.contrast, 1.6),
        "TestGradingStackApplyAndLiveImport_RuntimeGradingStackWritten");
    Check(params.color_pipeline.grading == ColorGradingPreset::escape_default,
        "TestGradingStackApplyAndLiveImport_PipelineTupleKeepsPrimaryGradingBridge");
    Check(Near(params.color_saturation, 0.8) && Near(params.color_contrast, 1.6),
        "TestGradingStackApplyAndLiveImport_LegacyMirrorUsesLastGradingRow");
    Check(state.live_snapshot.valid && state.live_snapshot.lanes[3].rows.size() == 2 &&
            state.live_snapshot.lanes[3].rows[0].function_id == "contrast_lift" &&
            state.live_snapshot.lanes[3].rows[1].function_id == "phase_finish" &&
            !HasColorPipelineDraftEdits(state),
        "TestGradingStackApplyAndLiveImport_ApplyResyncsGradingStackDraft");

    Check(RemoveColorPipelineLaneRow(&state, 3, 1) && state.lanes[3].rows.size() == 1,
        "TestGradingStackApplyAndLiveImport_RemoveSecondGradingRow");
    Check(!RemoveColorPipelineLaneRow(&state, 3, 0),
        "TestGradingStackApplyAndLiveImport_CannotRemoveLastGradingRow");

    ColorGradingPreset neutralGrading = ColorGradingPreset::escape_default;
    const bool neutralConfigured = SelectColorPipelineLaneFunction(&state, 3, "neutral_finish") &&
            SetRowNumber(state.lanes[3].rows[0], "grade.exposure", 1.25) &&
            SetRowNumber(state.lanes[3].rows[0], "grade.saturation", 0.85) &&
            SetRowNumber(state.lanes[3].rows[0], "grade.contrast", 1.4);
    Check(neutralConfigured,
        "TestGradingStackApplyAndLiveImport_ConfiguresNeutralFinish");
    if (neutralConfigured) {
        changed = false;
        Check(ApplyColorPipelineDraftToLiveState(&state, FractalType::newton, &params, &changed) && changed,
            "TestGradingStackApplyAndLiveImport_ApplyNeutralFinish");
        Check(color_pipeline_core::TryParseAdvancedColorGradingFunctionId("neutral_finish", &neutralGrading) &&
                params.color_grading_stack_count == 1 &&
                params.color_grading_stack[0].grading == neutralGrading &&
                Near(params.color_grading_stack[0].params.exposure, 1.25) &&
                Near(params.color_grading_stack[0].params.saturation, 0.85) &&
                Near(params.color_grading_stack[0].params.contrast, 1.4),
            "TestGradingStackApplyAndLiveImport_RuntimeWritesNeutralFinish");
        Check(params.color_pipeline.grading == neutralGrading && Near(params.exposure, 1.25) && Near(params.color_saturation, 0.85) && Near(params.color_contrast, 1.4),
            "TestGradingStackApplyAndLiveImport_NeutralFinishMirrorsLegacyOwners");
        Check(state.live_snapshot.valid && state.live_snapshot.lanes[3].rows.size() == 1 &&
                state.live_snapshot.lanes[3].rows[0].function_id == "neutral_finish" &&
                state.live_snapshot.lanes[3].rows[0].parameter_values.size() == 3,
            "TestGradingStackApplyAndLiveImport_ApplyResyncsNeutralFinish");
    }

    ColorGradingPreset toneMapGrading = ColorGradingPreset::escape_default;
    const bool toneMapConfigured = SelectColorPipelineLaneFunction(&state, 3, "tone_map_finish") &&
            SetRowNumber(state.lanes[3].rows[0], "grade.exposure", 1.35) &&
            SetRowNumber(state.lanes[3].rows[0], "grade.saturation", 0.75) &&
            SetRowNumber(state.lanes[3].rows[0], "grade.contrast", 1.6);
    Check(toneMapConfigured,
        "TestGradingStackApplyAndLiveImport_ConfiguresToneMapFinish");
    if (toneMapConfigured) {
        changed = false;
        Check(ApplyColorPipelineDraftToLiveState(&state, FractalType::newton, &params, &changed) && changed,
            "TestGradingStackApplyAndLiveImport_ApplyToneMapFinish");
        Check(color_pipeline_core::TryParseAdvancedColorGradingFunctionId("tone_map_finish", &toneMapGrading) &&
                params.color_grading_stack_count == 1 &&
                params.color_grading_stack[0].grading == toneMapGrading &&
                Near(params.color_grading_stack[0].params.exposure, 1.35) &&
                Near(params.color_grading_stack[0].params.saturation, 0.75) &&
                Near(params.color_grading_stack[0].params.contrast, 1.6),
            "TestGradingStackApplyAndLiveImport_RuntimeWritesToneMapFinish");
        Check(params.color_pipeline.grading == toneMapGrading && Near(params.exposure, 1.35) && Near(params.color_saturation, 0.75) && Near(params.color_contrast, 1.6),
            "TestGradingStackApplyAndLiveImport_ToneMapFinishMirrorsLegacyOwners");
        Check(state.live_snapshot.valid && state.live_snapshot.lanes[3].rows.size() == 1 &&
                state.live_snapshot.lanes[3].rows[0].function_id == "tone_map_finish" &&
                state.live_snapshot.lanes[3].rows[0].parameter_values.size() == 3,
            "TestGradingStackApplyAndLiveImport_ApplyResyncsToneMapFinish");
    }

    ColorGradingPreset gradeGlowGrading = ColorGradingPreset::escape_default;
    const bool gradeGlowConfigured = SelectColorPipelineLaneFunction(&state, 3, "grade_glow") &&
            SetRowNumber(state.lanes[3].rows[0], "grade.exposure", 1.2) &&
            SetRowNumber(state.lanes[3].rows[0], "grade.saturation", 0.8) &&
            SetRowNumber(state.lanes[3].rows[0], "grade.contrast", 1.5) &&
            SetRowNumber(state.lanes[3].rows[0], "grade.glow", 0.6);
    Check(gradeGlowConfigured,
        "TestGradingStackApplyAndLiveImport_ConfiguresGradeGlow");
    if (gradeGlowConfigured) {
        changed = false;
        Check(ApplyColorPipelineDraftToLiveState(&state, FractalType::newton, &params, &changed) && changed,
            "TestGradingStackApplyAndLiveImport_ApplyGradeGlow");
        Check(color_pipeline_core::TryParseAdvancedColorGradingFunctionId("grade_glow", &gradeGlowGrading) &&
                params.color_grading_stack_count == 1 &&
                params.color_grading_stack[0].grading == gradeGlowGrading &&
                Near(params.color_grading_stack[0].params.exposure, 1.2) &&
                Near(params.color_grading_stack[0].params.saturation, 0.8) &&
                Near(params.color_grading_stack[0].params.contrast, 1.5) &&
                Near(params.color_grading_stack[0].params.glow, 0.6),
            "TestGradingStackApplyAndLiveImport_RuntimeWritesGradeGlow");
        Check(params.color_pipeline.grading == gradeGlowGrading && Near(params.exposure, 1.2) && Near(params.color_saturation, 0.8) && Near(params.color_contrast, 1.5) && Near(params.color_glow, 0.6),
            "TestGradingStackApplyAndLiveImport_GradeGlowMirrorsLegacyOwners");
        Check(state.live_snapshot.valid && state.live_snapshot.lanes[3].rows.size() == 1 &&
                state.live_snapshot.lanes[3].rows[0].function_id == "grade_glow" &&
                state.live_snapshot.lanes[3].rows[0].parameter_values.size() == 4,
            "TestGradingStackApplyAndLiveImport_ApplyResyncsGradeGlow");
    }

    ColorGradingPreset balanceVoidGrading = ColorGradingPreset::escape_default;
    const bool balanceVoidConfigured = SelectColorPipelineLaneFunction(&state, 3, "balance_void_grade") &&
            SetRowNumber(state.lanes[3].rows[0], "grade.balance_void", 0.35) &&
            SetRowNumber(state.lanes[3].rows[0], "grade.chroma_tension", 0.6) &&
            SetRowNumber(state.lanes[3].rows[0], "grade.accent_bias", -0.25);
    Check(balanceVoidConfigured,
        "TestGradingStackApplyAndLiveImport_ConfiguresBalanceVoidGrade");
    if (balanceVoidConfigured) {
        changed = false;
        Check(ApplyColorPipelineDraftToLiveState(&state, FractalType::newton, &params, &changed) && changed,
            "TestGradingStackApplyAndLiveImport_ApplyBalanceVoidGrade");
        Check(color_pipeline_core::TryParseAdvancedColorGradingFunctionId("balance_void_grade", &balanceVoidGrading) &&
                params.color_grading_stack_count == 1 &&
                params.color_grading_stack[0].grading == balanceVoidGrading &&
                Near(params.color_grading_stack[0].params.balance_void, 0.35) &&
                Near(params.color_grading_stack[0].params.chroma_tension, 0.6) &&
                Near(params.color_grading_stack[0].params.accent_bias, -0.25),
            "TestGradingStackApplyAndLiveImport_RuntimeWritesBalanceVoidGrade");
        Check(params.color_pipeline.grading == balanceVoidGrading && Near(params.color_balance_void, 0.35) && Near(params.color_chroma_tension, 0.6) && Near(params.color_accent_bias, -0.25),
            "TestGradingStackApplyAndLiveImport_BalanceVoidGradeMirrorsDedicatedOwners");
        Check(state.live_snapshot.valid && state.live_snapshot.lanes[3].rows.size() == 1 &&
                state.live_snapshot.lanes[3].rows[0].function_id == "balance_void_grade" &&
                state.live_snapshot.lanes[3].rows[0].parameter_values.size() == 3,
            "TestGradingStackApplyAndLiveImport_ApplyResyncsBalanceVoidGrade");
    }
}

void TestPaletteSwitchPreservesSupportedBalanceVoidGrade() {
    ColorPipelineWindowState state{};
    KernelParams params = SmoothEscapeParams();
    Check(SyncColorPipelineWindowFromLiveState(&state, FractalType::newton, &params),
        "TestPaletteSwitchPreservesSupportedBalanceVoidGrade_SyncStartsSupported");
    Check(SelectColorPipelineLaneFunction(&state, 3, "balance_void_grade") &&
            SetRowNumber(state.lanes[3].rows[0], "grade.balance_void", 0.35) &&
            SetRowNumber(state.lanes[3].rows[0], "grade.chroma_tension", 0.6) &&
            SetRowNumber(state.lanes[3].rows[0], "grade.accent_bias", -0.25),
        "TestPaletteSwitchPreservesSupportedBalanceVoidGrade_ConfiguresBalanceVoidGrade");

    bool changed = false;
    Check(ApplyColorPipelineDraftToLiveState(&state, FractalType::newton, &params, &changed) && changed,
        "TestPaletteSwitchPreservesSupportedBalanceVoidGrade_ApplyBalanceVoidGrade");
    const std::uint64_t gradingRowId = state.lanes[3].rows[0].ui_row_id;
    ColorGradingPreset balanceVoidGrading = ColorGradingPreset::escape_default;
    Check(color_pipeline_core::TryParseAdvancedColorGradingFunctionId("balance_void_grade", &balanceVoidGrading),
        "TestPaletteSwitchPreservesSupportedBalanceVoidGrade_ParsesBalanceVoidGradingId");

    Check(SelectColorPipelineLaneFunction(&state, 2, "explaino_cmap"),
        "TestPaletteSwitchPreservesSupportedBalanceVoidGrade_SwitchesPaletteToExplaino");
    Check(state.lanes[3].rows.size() == 1 &&
            state.lanes[3].rows[0].ui_row_id == gradingRowId &&
            state.lanes[3].rows[0].function_id == "balance_void_grade" &&
            RowNumber(state.lanes[3].rows[0], "grade.balance_void", 0.35) &&
            RowNumber(state.lanes[3].rows[0], "grade.chroma_tension", 0.6) &&
            RowNumber(state.lanes[3].rows[0], "grade.accent_bias", -0.25),
        "TestPaletteSwitchPreservesSupportedBalanceVoidGrade_DraftKeepsSupportedGradingRowAndValues");

    changed = false;
    Check(ApplyColorPipelineDraftToLiveState(&state, FractalType::newton, &params, &changed) && changed,
        "TestPaletteSwitchPreservesSupportedBalanceVoidGrade_ApplySwitchedPalette");
    Check(params.color_pipeline.palette == ColorPalette::explaino_cmap &&
            params.color_pipeline.grading == balanceVoidGrading &&
            Near(params.color_balance_void, 0.35) && Near(params.color_chroma_tension, 0.6) && Near(params.color_accent_bias, -0.25),
        "TestPaletteSwitchPreservesSupportedBalanceVoidGrade_RuntimeKeepsSupportedGradingOwners");
}


void TestRootBasinPairScheduleBridge() {
    ColorPipelineWindowState state{};
    KernelParams params = SmoothEscapeParams();
    Check(EnsureColorPipelineWindowInitialized(&state), "TestRootBasinPairScheduleBridge_Initializes");
    Check(SelectColorPipelineLaneFunction(&state, 0, "root_index") &&
            SelectColorPipelineLaneFunction(&state, 2, "root_classic_palette") &&
            AddColorPipelineLaneRow(&state, 0, "root_index") &&
            AddColorPipelineLaneRow(&state, 2, "joy_root_palette"),
        "TestRootBasinPairScheduleBridge_BuildsTwoRowPairDraft");
    Check(state.lanes[0].rows.size() == 2 && state.lanes[2].rows.size() == 2,
        "TestRootBasinPairScheduleBridge_RowCountsMatch");
    bool changed = false;
    Check(ApplyColorPipelineDraftToLiveState(&state, FractalType::newton, &params, &changed) && changed,
        "TestRootBasinPairScheduleBridge_AppliesTwoRowPairDraft");
    Check(params.color_root_basin_pair_count == 2 &&
            params.color_root_basin_pairs[0].palette == ColorPalette::root_classic &&
            params.color_root_basin_pairs[1].palette == ColorPalette::joy &&
            params.color_grading_stack_count == 1 &&
            params.color_grading_stack[0].grading == ColorGradingPreset::basin_default &&
            params.coloring_mode == ColoringMode::joy_basins,
        "TestRootBasinPairScheduleBridge_RuntimePairSchedulePreservesBasinGrading");
    Check(HasCoherentRootBasinPairSchedule(params),
        "TestRootBasinPairScheduleBridge_RuntimePairScheduleCoherent");
    Check(state.live_snapshot.valid && state.live_snapshot.draft_import_supported &&
            state.live_snapshot.lanes.size() == 4 &&
            state.live_snapshot.lanes[0].rows.size() == 2 && state.live_snapshot.lanes[2].rows.size() == 2 &&
            state.live_snapshot.lanes[3].rows.size() == 1 &&
            state.live_snapshot.lanes[3].rows[0].function_id == "basin_default" &&
            state.live_snapshot.lanes[3].rows[0].parameter_values.empty() &&
            !HasColorPipelineDraftEdits(state),
        "TestRootBasinPairScheduleBridge_LiveSnapshotImportsPairsWithBasinGrading");

    ColorPipelineWindowState mismatchState{};
    Check(EnsureColorPipelineWindowInitialized(&mismatchState) &&
            SelectColorPipelineLaneFunction(&mismatchState, 0, "root_index") &&
            SelectColorPipelineLaneFunction(&mismatchState, 2, "root_classic_palette") &&
            AddColorPipelineLaneRow(&mismatchState, 2, "joy_root_palette"),
        "TestRootBasinPairScheduleBridge_BuildsMismatchedPairDraft");
    ColorPipelineSelection pipeline{};
    ColoringMode mode = ColoringMode::smooth_escape;
    std::string error;
    Check(!TryBuildColorPipelineSelectionFromDraft(mismatchState, &pipeline, &mode, &error) &&
            error.find("row counts match") != std::string::npos,
        "TestRootBasinPairScheduleBridge_MismatchedPairCountsFailClosed");
}

void TestSourcePalettePresetCheckboxesTogglePairedRows() {
    ColorPipelineWindowState state{};
    KernelParams params = SmoothEscapeParams();
    Check(SyncColorPipelineWindowFromLiveState(&state, FractalType::newton, &params),
        "TestSourcePalettePresetCheckboxesTogglePairedRows_SyncStartsSupported");
    Check(SelectColorPipelineLaneFunction(&state, 0, "root_index") &&
            SelectColorPipelineLaneFunction(&state, 2, "root_classic_palette") &&
            AddColorPipelineLaneRow(&state, 0, "root_index") &&
            AddColorPipelineLaneRow(&state, 2, "joy_root_palette"),
        "TestSourcePalettePresetCheckboxesTogglePairedRows_BuildsTwoPresetPairs");

    bool changed = false;
    Check(ApplyColorPipelineDraftToLiveState(&state, FractalType::newton, &params, &changed) && changed,
        "TestSourcePalettePresetCheckboxesTogglePairedRows_AppliesInitialPairs");
    Check(params.color_root_basin_pair_count == 2 && params.color_pipeline.palette == ColorPalette::joy,
        "TestSourcePalettePresetCheckboxesTogglePairedRows_InitialPairsUseLastEnabledPreset");

    Check(SetColorPipelineRowEnabledFromUi(&state, 2, 1, false),
        "TestSourcePalettePresetCheckboxesTogglePairedRows_DisablesPalettePresetRow");
    Check(!state.lanes[0].rows[1].enabled && !state.lanes[2].rows[1].enabled,
        "TestSourcePalettePresetCheckboxesTogglePairedRows_DisablesMatchingSourceRow");
    Check(SetColorPipelineRowEnabledFromUi(&state, 2, 1, true),
        "TestSourcePalettePresetCheckboxesTogglePairedRows_ReEnablesPalettePresetRow");
    Check(state.lanes[0].rows[1].enabled && state.lanes[2].rows[1].enabled,
        "TestSourcePalettePresetCheckboxesTogglePairedRows_ReEnablesMatchingSourceRow");

    Check(SetColorPipelineRowEnabledFromUi(&state, 0, 0, false),
        "TestSourcePalettePresetCheckboxesTogglePairedRows_DisablesSourcePresetRow");
    Check(!state.lanes[0].rows[0].enabled && !state.lanes[2].rows[0].enabled &&
            state.lanes[0].rows[1].enabled && state.lanes[2].rows[1].enabled,
        "TestSourcePalettePresetCheckboxesTogglePairedRows_DisablesMatchingPaletteRowOnly");

    changed = false;
    Check(ApplyColorPipelineDraftToLiveState(&state, FractalType::newton, &params, &changed) && changed,
        "TestSourcePalettePresetCheckboxesTogglePairedRows_AppliesSingleRemainingPair");
    Check(params.color_root_basin_pair_count == 1 &&
            params.color_pipeline.signal == ColorSignal::root_index &&
            params.color_pipeline.palette == ColorPalette::joy,
        "TestSourcePalettePresetCheckboxesTogglePairedRows_RuntimeUsesRemainingPresetPair");

    ClearColorPipelineValidationMessages(&state);
    Check(SetColorPipelineRowEnabledFromUi(&state, 2, 0, false),
        "TestSourcePalettePresetCheckboxesTogglePairedRows_LastPairDisableIsHandled");
    Check(state.lanes[0].rows[0].enabled && state.lanes[2].rows[0].enabled && !state.validation_messages.empty(),
        "TestSourcePalettePresetCheckboxesTogglePairedRows_LastPairStaysEnabled");
}

void TestDisablePreservesSupportedGradingRowAcrossApplyResync() {
    ColorPipelineWindowState state{};
    KernelParams params = SmoothEscapeParams();
    Check(SyncColorPipelineWindowFromLiveState(&state, FractalType::newton, &params),
        "TestDisablePreservesSupportedGradingRowAcrossApplyResync_SyncStartsSupported");
    Check(AddColorPipelineLaneRow(&state, 3, "phase_finish") && state.lanes[3].rows.size() == 2,
        "TestDisablePreservesSupportedGradingRowAcrossApplyResync_AddsSecondGradingRow");
    Check(SetRowNumber(state.lanes[3].rows[1], "grade.saturation", 0.8) &&
            SetRowNumber(state.lanes[3].rows[1], "grade.contrast", 1.6),
        "TestDisablePreservesSupportedGradingRowAcrossApplyResync_ConfiguresSecondRow");

    const std::uint64_t secondRowId = state.lanes[3].rows[1].ui_row_id;
    bool changed = false;
    Check(ApplyColorPipelineDraftToLiveState(&state, FractalType::newton, &params, &changed) && changed,
        "TestDisablePreservesSupportedGradingRowAcrossApplyResync_AppliesTwoRowStack");
    Check(state.lanes[3].rows.size() == 2 && state.lanes[3].rows[1].ui_row_id == secondRowId &&
            state.lanes[3].rows[1].enabled && params.color_grading_stack_count == 2,
        "TestDisablePreservesSupportedGradingRowAcrossApplyResync_ApplyKeepsSecondRowIdentity");

    Check(SetColorPipelineRowEnabledFromUi(&state, 3, 1, false),
        "TestDisablePreservesSupportedGradingRowAcrossApplyResync_DisablesSecondRow");
    const ColorPipelineDraftApplyState disabledApplyState = DescribeColorPipelineDraftApplyState(
        state,
        FractalType::newton,
        &params);
    Check(disabledApplyState.status == ColorPipelineDraftApplyStatus::can_apply,
        "TestDisablePreservesSupportedGradingRowAcrossApplyResync_DisabledRowCanStillApply");
    changed = false;
    Check(ApplyColorPipelineDraftToLiveState(&state, FractalType::newton, &params, &changed),
        "TestDisablePreservesSupportedGradingRowAcrossApplyResync_ApplyDisabledRowState");
    Check(changed,
        "TestDisablePreservesSupportedGradingRowAcrossApplyResync_DisablingChangesLiveStack");
    Check(state.lanes[3].rows.size() == 2 && state.lanes[3].rows[1].ui_row_id == secondRowId &&
            state.lanes[3].rows[1].function_id == "phase_finish" && !state.lanes[3].rows[1].enabled,
        "TestDisablePreservesSupportedGradingRowAcrossApplyResync_DisabledRowStaysVisibleAfterResync");
    Check(state.live_snapshot.valid && state.live_snapshot.lanes[3].rows.size() == 1 &&
            params.color_grading_stack_count == 1,
        "TestDisablePreservesSupportedGradingRowAcrossApplyResync_LiveSnapshotDropsDisabledRowOnly");
    Check(HasColorPipelineDraftEdits(state),
        "TestDisablePreservesSupportedGradingRowAcrossApplyResync_DisabledRowRemainsDraftDelta");

    Check(SetColorPipelineRowEnabledFromUi(&state, 3, 1, true),
        "TestDisablePreservesSupportedGradingRowAcrossApplyResync_ReEnablesSecondRow");
    const ColorPipelineDraftApplyState reenabledApplyState = DescribeColorPipelineDraftApplyState(
        state,
        FractalType::newton,
        &params);
    Check(reenabledApplyState.status == ColorPipelineDraftApplyStatus::can_apply,
        "TestDisablePreservesSupportedGradingRowAcrossApplyResync_ReEnabledRowCanApply");
    changed = false;
    Check(ApplyColorPipelineDraftToLiveState(&state, FractalType::newton, &params, &changed) && changed,
        "TestDisablePreservesSupportedGradingRowAcrossApplyResync_ReApplyRestoresTwoRowStack");
    Check(state.lanes[3].rows.size() == 2 && state.lanes[3].rows[1].ui_row_id == secondRowId &&
            state.lanes[3].rows[1].enabled && params.color_grading_stack_count == 2,
        "TestDisablePreservesSupportedGradingRowAcrossApplyResync_ReEnableRestoresSameRow");

    Check(RemoveColorPipelineLaneRow(&state, 3, 1),
        "TestDisablePreservesSupportedGradingRowAcrossApplyResync_ExplicitRemoveDeletesSecondRow");
    Check(state.lanes[3].rows.size() == 1,
        "TestDisablePreservesSupportedGradingRowAcrossApplyResync_RemoveShrinksDraftOnlyOnExplicitRemove");
    changed = false;
    Check(ApplyColorPipelineDraftToLiveState(&state, FractalType::newton, &params, &changed) && changed,
        "TestDisablePreservesSupportedGradingRowAcrossApplyResync_ApplyAfterExplicitRemove");
    Check(state.lanes[3].rows.size() == 1 && params.color_grading_stack_count == 1,
        "TestDisablePreservesSupportedGradingRowAcrossApplyResync_RemoveKeepsExplicitDeleteSeparateFromDisable");
}

void TestDisablePreservesUnsupportedPaletteRowAcrossApplyResync() {
    ColorPipelineWindowState state{};
    KernelParams params = SmoothEscapeParams();
    Check(SyncColorPipelineWindowFromLiveState(&state, FractalType::newton, &params),
        "TestDisablePreservesUnsupportedPaletteRowAcrossApplyResync_SyncStartsSupported");
    Check(AddColorPipelineLaneRow(&state, 2, "root_classic_palette") && state.lanes[2].rows.size() == 2,
        "TestDisablePreservesUnsupportedPaletteRowAcrossApplyResync_AddsUnsupportedPaletteRow");

    const std::uint64_t unsupportedRowId = state.lanes[2].rows[1].ui_row_id;
    const ColorPipelineDraftApplyState unsupportedApplyState = DescribeColorPipelineDraftApplyState(
        state,
        FractalType::newton,
        &params);
    Check(unsupportedApplyState.status == ColorPipelineDraftApplyStatus::unsupported_tuple,
        "TestDisablePreservesUnsupportedPaletteRowAcrossApplyResync_ExtraRootPaletteStartsUnsupported");

    Check(SetColorPipelineRowEnabledFromUi(&state, 2, 1, false),
        "TestDisablePreservesUnsupportedPaletteRowAcrossApplyResync_DisablesUnsupportedRow");
    const ColorPipelineDraftApplyState disabledApplyState = DescribeColorPipelineDraftApplyState(
        state,
        FractalType::newton,
        &params);
    Check(disabledApplyState.status == ColorPipelineDraftApplyStatus::matches_live,
        "TestDisablePreservesUnsupportedPaletteRowAcrossApplyResync_DisabledUnsupportedRowMatchesEnabledLiveTuple");

    bool changed = true;
    Check(ApplyColorPipelineDraftToLiveState(&state, FractalType::newton, &params, &changed),
        "TestDisablePreservesUnsupportedPaletteRowAcrossApplyResync_ApplyDisabledUnsupportedRow");
    Check(!changed,
        "TestDisablePreservesUnsupportedPaletteRowAcrossApplyResync_DisablingUnsupportedRowDoesNotChangeLiveTuple");
    Check(state.lanes[2].rows.size() == 2 && state.lanes[2].rows[1].ui_row_id == unsupportedRowId &&
            state.lanes[2].rows[1].function_id == "root_classic_palette" && !state.lanes[2].rows[1].enabled,
        "TestDisablePreservesUnsupportedPaletteRowAcrossApplyResync_DisabledUnsupportedRowStaysVisibleAfterResync");
    Check(state.live_snapshot.valid && state.live_snapshot.lanes[2].rows.size() == 1 &&
            state.live_snapshot.lanes[2].rows[0].function_id == "heatmap",
        "TestDisablePreservesUnsupportedPaletteRowAcrossApplyResync_LiveSnapshotKeepsOnlySupportedEnabledPalette");
    Check(HasColorPipelineDraftEdits(state),
        "TestDisablePreservesUnsupportedPaletteRowAcrossApplyResync_DisabledUnsupportedRowRemainsDraftDelta");
    Check(state.validation_messages.empty(),
        "TestDisablePreservesUnsupportedPaletteRowAcrossApplyResync_DisabledUnsupportedRowClearsValidationNoise");
}

void TestWindowUtilityContracts() {
    ColorPipelineWindowState state{};
    PushColorPipelineValidationMessage(&state, "first");
    PushColorPipelineValidationMessage(&state, "");
    Check(state.validation_messages.size() == 1, "TestWindowUtilityContracts_PushSkipsEmptyMessages");
    ClearColorPipelineValidationMessages(&state);
    Check(state.validation_messages.empty(), "TestWindowUtilityContracts_ClearValidationMessages");

    state.ui_automation_rects.push_back({"stale", 1, 2, 3, 4});
    ClearColorPipelineUiAutomationRects(&state);
    Check(state.ui_automation_rects.empty(), "TestWindowUtilityContracts_ClearAutomationRects");
    NoteColorPipelineUiAutomationRect(&state, "ignored_without_imgui");
    Check(state.ui_automation_rects.empty(), "TestWindowUtilityContracts_NoImguiRectRecorderIsNoOp");

    const FunctionParamDescriptor param = MakeColorPipelineFloatParam(
        "shape.scale",
        "Scale",
        "",
        0.1,
        8.0,
        0.1,
        1.0);
    Check(BuildColorPipelinePrimaryControlId("shape", "offset_scale", param) ==
            "color_pipeline.shape.offset_scale.shape.scale.primary",
        "TestWindowUtilityContracts_PrimaryControlIdIsStable");
    Check(!ShouldDisableLegacyColorPanelControlWhileAdvancedWindowOpen(state, "fractal.params.coloring_mode"),
        "TestWindowUtilityContracts_LegacyControlsAvailableWhenClosed");
    state.open = true;
    Check(ShouldDisableLegacyColorPanelControlWhileAdvancedWindowOpen(state, "fractal.params.coloring_mode") &&
            ShouldDisableLegacyColorPanelControlWhileAdvancedWindowOpen(state, "fractal.params.color_grading") &&
            !ShouldDisableLegacyColorPanelControlWhileAdvancedWindowOpen(state, "fractal.params.color_saturation"),
        "TestWindowUtilityContracts_OnlyOwnedLegacyControlsDisableWhenOpen");

    ColorPipelineRenderInteractionState interaction{};
    NoteColorPipelineInteractionSnapshot(false, false, false, false, &interaction);
    Check(!interaction.interacted && !interaction.has_active_item,
        "TestWindowUtilityContracts_NoInteractionSnapshotStaysEmpty");
    NoteColorPipelineInteractionSnapshot(false, true, true, false, &interaction);
    Check(interaction.interacted && interaction.has_active_item,
        "TestWindowUtilityContracts_InteractionSnapshotTracksActiveItem");
}

} // namespace

int main() {
    TestInitializationAndLiveSync();
    TestRootSelectionCoSwitchesAndApplies();
    TestShapeStackApplyAndValidation();
    TestIterationBandsLiveImportShipsBandFinish();
    TestGradingStackApplyAndLiveImport();
    TestRootBasinPairScheduleBridge();
    TestPaletteSwitchPreservesSupportedBalanceVoidGrade();
    TestSourcePalettePresetCheckboxesTogglePairedRows();
    TestDisablePreservesSupportedGradingRowAcrossApplyResync();
    TestDisablePreservesUnsupportedPaletteRowAcrossApplyResync();
    TestWindowUtilityContracts();

    std::printf("test_color_pipeline_window: passed=%d failed=%d\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
