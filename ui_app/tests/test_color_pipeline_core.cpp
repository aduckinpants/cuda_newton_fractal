#include "../src/color_pipeline_core.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <string>
#include <vector>

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

bool Near(double actual, double expected, double eps = 1.0e-9) {
    return std::fabs(actual - expected) <= eps;
}

bool HasFunction(const ColorPipelineLaneCatalog& catalog, const char* id) {
    return color_pipeline_core::FindColorPipelineFunctionDescriptor(catalog, id) != nullptr;
}

bool CatalogIdsEqual(const ColorPipelineLaneCatalog& catalog, std::initializer_list<const char*> expected) {
    if (catalog.functions.size() != expected.size()) {
        return false;
    }
    std::size_t index = 0;
    for (const char* id : expected) {
        if (catalog.functions[index].id != id) return false;
        ++index;
    }
    return true;
}

bool RowNumber(const ColorPipelineRowState& row, const char* path, double expected) {
    double actual = 0.0;
    return color_pipeline_core::TryGetColorPipelineParamNumber(row, path, &actual) && Near(actual, expected);
}

bool RowEnum(const ColorPipelineRowState& row, const char* path, const char* expected) {
    std::string actual;
    return color_pipeline_core::TryGetColorPipelineParamEnum(row, path, &actual) && actual == expected;
}

const FunctionDescriptor* FindFunction(const ColorPipelineLaneCatalog& catalog, const char* id) {
    return color_pipeline_core::FindColorPipelineFunctionDescriptor(catalog, id ? id : "");
}

void TestFunctionIdMappingsRoundTrip() {
    ColorSignal signal = ColorSignal::smooth_escape;
    Check(std::string(color_pipeline_core::AdvancedColorSignalFunctionId(ColorSignal::root_index)) == "root_index",
        "TestFunctionIdMappingsRoundTrip_SignalId");
    Check(color_pipeline_core::TryParseAdvancedColorSignalFunctionId("orbit_stripe", &signal) && signal == ColorSignal::orbit_stripe,
        "TestFunctionIdMappingsRoundTrip_SignalParse");
    Check(!color_pipeline_core::TryParseAdvancedColorSignalFunctionId("missing_source", &signal) && signal == ColorSignal::orbit_stripe,
        "TestFunctionIdMappingsRoundTrip_SignalRejectPreservesValue");

    ColorPalette palette = ColorPalette::root_classic;
    Check(std::string(color_pipeline_core::AdvancedColorPaletteFunctionId(ColorPalette::explaino_cmap)) == "explaino_cmap",
        "TestFunctionIdMappingsRoundTrip_PaletteId");
    Check(color_pipeline_core::TryParseAdvancedColorPaletteFunctionId("joy_root_palette", &palette) && palette == ColorPalette::joy,
        "TestFunctionIdMappingsRoundTrip_PaletteParse");
    Check(!color_pipeline_core::TryParseAdvancedColorPaletteFunctionId("missing_palette", &palette) && palette == ColorPalette::joy,
        "TestFunctionIdMappingsRoundTrip_PaletteRejectPreservesValue");

    ColorGradingPreset grading = ColorGradingPreset::escape_default;
    Check(std::string(color_pipeline_core::AdvancedColorGradingFunctionId(ColorGradingPreset::basin_default)) == "basin_default",
        "TestFunctionIdMappingsRoundTrip_GradingIdIncludesBasinDefault");
    Check(std::string(color_pipeline_core::AdvancedColorGradingFunctionId(ColorGradingPreset::bands_default)) == "band_finish",
        "TestFunctionIdMappingsRoundTrip_GradingIdIncludesDraftBandFinish");
    Check(color_pipeline_core::TryParseAdvancedColorGradingFunctionId("basin_default", &grading) && grading == ColorGradingPreset::basin_default,
        "TestFunctionIdMappingsRoundTrip_GradingParseBasinDefault");
    Check(color_pipeline_core::TryParseAdvancedColorGradingFunctionId("phase_finish", &grading) && grading == ColorGradingPreset::phase_default,
        "TestFunctionIdMappingsRoundTrip_GradingParsePhaseFinish");
    Check(color_pipeline_core::TryParseAdvancedColorGradingFunctionId("neutral_finish", &grading) &&
            std::string(color_pipeline_core::AdvancedColorGradingFunctionId(grading)) == "neutral_finish",
        "TestFunctionIdMappingsRoundTrip_GradingParseNeutralFinish");
    Check(color_pipeline_core::TryParseAdvancedColorGradingFunctionId("tone_map_finish", &grading) &&
            std::string(color_pipeline_core::AdvancedColorGradingFunctionId(grading)) == "tone_map_finish",
        "TestFunctionIdMappingsRoundTrip_GradingParseToneMapFinish");
    Check(!color_pipeline_core::TryParseAdvancedColorGradingFunctionId("missing_grade", &grading) && grading == ColorGradingPreset::tone_map_default,
        "TestFunctionIdMappingsRoundTrip_GradingRejectPreservesValue");

    Check(std::string(color_pipeline_core::AdvancedColorShapeFunctionId(ColorPipelineShape::smooth_window)) == "smooth_window",
        "TestFunctionIdMappingsRoundTrip_ShapeId");
}

void TestLaneCatalogFiltersRuntimeBackedRows() {
    const std::vector<ColorPipelineLaneCatalog>& catalogs = color_pipeline_core::GetColorPipelineLaneCatalogs();
    Check(catalogs.size() == 4, "TestLaneCatalogFiltersRuntimeBackedRows_LaneCount");
    Check(catalogs[0].lane_id == std::string("source") && catalogs[1].lane_id == std::string("shape") &&
            catalogs[2].lane_id == std::string("palette") && catalogs[3].lane_id == std::string("grading"),
        "TestLaneCatalogFiltersRuntimeBackedRows_LaneOrder");

    const ColorPipelineLaneCatalog* source = color_pipeline_core::FindColorPipelineLaneCatalog("source");
    const ColorPipelineLaneCatalog* shape = color_pipeline_core::FindColorPipelineLaneCatalog("shape");
    const ColorPipelineLaneCatalog* palette = color_pipeline_core::FindColorPipelineLaneCatalog("palette");
    const ColorPipelineLaneCatalog* grading = color_pipeline_core::FindColorPipelineLaneCatalog("grading");
    Check(source && shape && palette && grading, "TestLaneCatalogFiltersRuntimeBackedRows_AllCatalogsDiscoverable");
    if (!source || !shape || !palette || !grading) return;

    Check(source->default_function_id == std::string("smooth_escape_ramp") && source->functions.size() == 7,
        "TestLaneCatalogFiltersRuntimeBackedRows_SourceShape");
    Check(HasFunction(*source, "smooth_escape_ramp") && HasFunction(*source, "phase_orbit") &&
            HasFunction(*source, "banded_signal") && HasFunction(*source, "escape_magnitude") &&
            HasFunction(*source, "orbit_stripe") && HasFunction(*source, "root_proximity") &&
            HasFunction(*source, "root_index"),
        "TestLaneCatalogFiltersRuntimeBackedRows_SourceFunctions");
    Check(shape->default_function_id == std::string("identity") && shape->functions.size() == 7 &&
            HasFunction(*shape, "smooth_window"),
        "TestLaneCatalogFiltersRuntimeBackedRows_ShapeFunctions");
    Check(palette->default_function_id == std::string("heatmap") && palette->functions.size() == 6 &&
            HasFunction(*palette, "explaino_cmap") && HasFunction(*palette, "root_classic_palette") &&
            HasFunction(*palette, "joy_root_palette"),
        "TestLaneCatalogFiltersRuntimeBackedRows_PaletteFunctions");
    Check(grading->default_function_id == std::string("contrast_lift") && grading->functions.size() == 6 &&
            HasFunction(*grading, "contrast_lift") && HasFunction(*grading, "phase_finish") && HasFunction(*grading, "band_finish") && HasFunction(*grading, "basin_default") && HasFunction(*grading, "neutral_finish") && HasFunction(*grading, "tone_map_finish"),
        "TestLaneCatalogFiltersRuntimeBackedRows_GradingShipsToneMapFinish");
    Check(CatalogIdsEqual(*source, {"smooth_escape_ramp", "phase_orbit", "banded_signal", "escape_magnitude", "orbit_stripe", "root_proximity", "root_index"}),
        "TestLaneCatalogFiltersRuntimeBackedRows_SourceFunctionOrder");
    Check(CatalogIdsEqual(*shape, {"identity", "offset_scale", "repeat", "posterize", "mirror_repeat", "bias_gain_curve", "smooth_window"}),
        "TestLaneCatalogFiltersRuntimeBackedRows_ShapeFunctionOrder");
    Check(CatalogIdsEqual(*palette, {"heatmap", "phase_wheel_palette", "banded_heatmap", "explaino_cmap", "root_classic_palette", "joy_root_palette"}),
        "TestLaneCatalogFiltersRuntimeBackedRows_PaletteFunctionOrder");
    Check(CatalogIdsEqual(*grading, {"contrast_lift", "phase_finish", "band_finish", "basin_default", "neutral_finish", "tone_map_finish"}),
        "TestLaneCatalogFiltersRuntimeBackedRows_GradingFunctionOrder");

    const std::vector<FunctionDescriptor> allGradeFunctions = color_pipeline_core::BuildColorPipelineGradeFunctions();
    bool rawGradeIncludesBandFinish = false;
    bool rawGradeIncludesBasinDefault = false;
    bool rawGradeIncludesNeutralFinish = false;
    bool rawGradeIncludesToneMapFinish = false;
    for (const FunctionDescriptor& descriptor : allGradeFunctions) {
        rawGradeIncludesBandFinish = rawGradeIncludesBandFinish || descriptor.id == "band_finish";
        rawGradeIncludesBasinDefault = rawGradeIncludesBasinDefault || descriptor.id == "basin_default";
        rawGradeIncludesNeutralFinish = rawGradeIncludesNeutralFinish || descriptor.id == "neutral_finish";
        rawGradeIncludesToneMapFinish = rawGradeIncludesToneMapFinish || descriptor.id == "tone_map_finish";
    }
    Check(rawGradeIncludesBandFinish, "TestLaneCatalogFiltersRuntimeBackedRows_RawGradeCatalogNamesDraftBandFinish");
    Check(rawGradeIncludesBasinDefault, "TestLaneCatalogFiltersRuntimeBackedRows_RawGradeCatalogNamesBasinDefault");
    Check(rawGradeIncludesNeutralFinish, "TestLaneCatalogFiltersRuntimeBackedRows_RawGradeCatalogNamesNeutralFinish");
    Check(rawGradeIncludesToneMapFinish, "TestLaneCatalogFiltersRuntimeBackedRows_RawGradeCatalogNamesToneMapFinish");
    Check(color_pipeline_core::IsColorPipelineFunctionRuntimeBacked("grading", "band_finish"),
        "TestLaneCatalogFiltersRuntimeBackedRows_BandFinishRuntimeBacked");
    Check(color_pipeline_core::IsColorPipelineFunctionRuntimeBacked("grading", "basin_default"),
        "TestLaneCatalogFiltersRuntimeBackedRows_BasinDefaultRuntimeBacked");
    Check(color_pipeline_core::IsColorPipelineFunctionRuntimeBacked("grading", "neutral_finish"),
        "TestLaneCatalogFiltersRuntimeBackedRows_NeutralFinishRuntimeBacked");
    Check(color_pipeline_core::IsColorPipelineFunctionRuntimeBacked("grading", "tone_map_finish"),
        "TestLaneCatalogFiltersRuntimeBackedRows_ToneMapFinishRuntimeBacked");
    Check(!color_pipeline_core::IsColorPipelineFunctionRuntimeBacked(nullptr, "heatmap") &&
            !color_pipeline_core::IsColorPipelineFunctionRuntimeBacked("unknown_lane", "heatmap"),
        "TestLaneCatalogFiltersRuntimeBackedRows_UnknownLaneFailsClosed");
    Check(color_pipeline_core::FindColorPipelineLaneCatalog("missing") == nullptr,
        "TestLaneCatalogFiltersRuntimeBackedRows_UnknownCatalogMissing");
}

void TestRowBuildersAndDefaults() {
    const ColorPipelineLaneCatalog* source = color_pipeline_core::FindColorPipelineLaneCatalog("source");
    const ColorPipelineLaneCatalog* shape = color_pipeline_core::FindColorPipelineLaneCatalog("shape");
    const ColorPipelineLaneCatalog* grading = color_pipeline_core::FindColorPipelineLaneCatalog("grading");
    Check(source && shape && grading, "TestRowBuildersAndDefaults_CatalogsPresent");
    if (!source || !shape || !grading) return;

    ColorPipelineRowState phaseRow;
    std::string error;
    Check(color_pipeline_core::BuildColorPipelineRowFromFunctionId(*source, "phase_orbit", 17, &phaseRow, &error),
        "TestRowBuildersAndDefaults_PhaseRowBuilds");
    Check(phaseRow.ui_row_id == 17 && phaseRow.enabled && phaseRow.function_id == "phase_orbit",
        "TestRowBuildersAndDefaults_PhaseRowIdentity");
    Check(phaseRow.parameter_values.size() == 3 && RowNumber(phaseRow, "signal.phase_offset", 0.0) &&
            RowNumber(phaseRow, "signal.wrap_cycles", 1.0) &&
            RowNumber(phaseRow, "signal.blend_weight", 1.0),
        "TestRowBuildersAndDefaults_PhaseRowDefaults");

    ColorPipelineLaneState repeatLane;
    Check(color_pipeline_core::BuildColorPipelineLaneWithSingleRow(*shape, "repeat", 22, &repeatLane, &error),
        "TestRowBuildersAndDefaults_RepeatLaneBuilds");
    Check(repeatLane.lane_id == "shape" && repeatLane.label == "Shape" && repeatLane.rows.size() == 1,
        "TestRowBuildersAndDefaults_RepeatLaneIdentity");
    Check(repeatLane.rows[0].ui_row_id == 22 && repeatLane.rows[0].function_id == "repeat" &&
            RowNumber(repeatLane.rows[0], "shape.frequency", 8.0) && RowNumber(repeatLane.rows[0], "shape.phase", 0.0),
        "TestRowBuildersAndDefaults_RepeatLaneDefaults");

    ColorPipelineRowState unknownRow;
    error.clear();
    Check(!color_pipeline_core::BuildColorPipelineRowFromFunctionId(*source, "not_a_function", 1, &unknownRow, &error) &&
            error.find("Unknown advanced color function") != std::string::npos,
        "TestRowBuildersAndDefaults_UnknownFunctionFailsClosed");
    ColorPipelineRowState bandFinishDefaultRow;
    error.clear();
    Check(color_pipeline_core::BuildColorPipelineRowFromFunctionId(*grading, "band_finish", 35, &bandFinishDefaultRow, &error) &&
            bandFinishDefaultRow.ui_row_id == 35 &&
            bandFinishDefaultRow.function_id == "band_finish" &&
            bandFinishDefaultRow.parameter_values.size() == 2 &&
            RowNumber(bandFinishDefaultRow, "grade.saturation", 1.15) &&
            RowNumber(bandFinishDefaultRow, "grade.contrast", 1.10),
        "TestRowBuildersAndDefaults_BandFinishBuildsFromRuntimeCatalog");
    ColorPipelineRowState basinDefaultRow;
    error.clear();
    Check(color_pipeline_core::BuildColorPipelineRowFromFunctionId(*grading, "basin_default", 36, &basinDefaultRow, &error) &&
            basinDefaultRow.ui_row_id == 36 &&
            basinDefaultRow.function_id == "basin_default" &&
            basinDefaultRow.parameter_values.empty(),
        "TestRowBuildersAndDefaults_BasinDefaultBuildsWithoutFakeControls");
    ColorPipelineRowState neutralFinishRow;
    error.clear();
    Check(color_pipeline_core::BuildColorPipelineRowFromFunctionId(*grading, "neutral_finish", 37, &neutralFinishRow, &error) &&
            neutralFinishRow.ui_row_id == 37 &&
            neutralFinishRow.function_id == "neutral_finish" &&
            neutralFinishRow.parameter_values.size() == 3 &&
            RowNumber(neutralFinishRow, "grade.exposure", 1.0) &&
            RowNumber(neutralFinishRow, "grade.saturation", 1.15) &&
            RowNumber(neutralFinishRow, "grade.contrast", 1.10),
        "TestRowBuildersAndDefaults_NeutralFinishBuildsFromRuntimeCatalog");
    ColorPipelineRowState toneMapFinishRow;
    error.clear();
    Check(color_pipeline_core::BuildColorPipelineRowFromFunctionId(*grading, "tone_map_finish", 38, &toneMapFinishRow, &error) &&
            toneMapFinishRow.ui_row_id == 38 &&
            toneMapFinishRow.function_id == "tone_map_finish" &&
            toneMapFinishRow.parameter_values.size() == 3 &&
            RowNumber(toneMapFinishRow, "grade.exposure", 1.0) &&
            RowNumber(toneMapFinishRow, "grade.saturation", 1.15) &&
            RowNumber(toneMapFinishRow, "grade.contrast", 1.10),
        "TestRowBuildersAndDefaults_ToneMapFinishBuildsFromRuntimeCatalog");
    Check(!color_pipeline_core::BuildColorPipelineRowFromFunctionId(*source, "", 1, &unknownRow, &error),
        "TestRowBuildersAndDefaults_EmptyFunctionFailsClosed");
    Check(!color_pipeline_core::BuildColorPipelineLaneWithSingleRow(*shape, "repeat", 1, nullptr, &error),
        "TestRowBuildersAndDefaults_NullLaneOutputFailsClosed");
}

void TestRowFunctionSwitchPreservesSharedParams() {
    const ColorPipelineLaneCatalog* shape = color_pipeline_core::FindColorPipelineLaneCatalog("shape");
    const ColorPipelineLaneCatalog* palette = color_pipeline_core::FindColorPipelineLaneCatalog("palette");
    const ColorPipelineLaneCatalog* grading = color_pipeline_core::FindColorPipelineLaneCatalog("grading");
    Check(shape && palette && grading, "TestRowFunctionSwitchPreservesSharedParams_CatalogsPresent");
    if (!shape || !palette || !grading) return;

    ColorPipelineRowState repeatRow;
    Check(color_pipeline_core::BuildColorPipelineRowFromFunctionId(*shape, "repeat", 41, &repeatRow) &&
            color_pipeline_core::SetColorPipelineParamNumber(&repeatRow, "shape.frequency", 6.0) &&
            color_pipeline_core::SetColorPipelineParamNumber(&repeatRow, "shape.phase", 0.25),
        "TestRowFunctionSwitchPreservesSharedParams_RepeatRowConfigured");
    const FunctionDescriptor* mirrorRepeat = FindFunction(*shape, "mirror_repeat");
    Check(mirrorRepeat && color_pipeline_core::SetColorPipelineRowFunction(&repeatRow, *mirrorRepeat),
        "TestRowFunctionSwitchPreservesSharedParams_SwitchesToMirrorRepeat");
    Check(repeatRow.ui_row_id == 41 && repeatRow.function_id == "mirror_repeat" &&
            RowNumber(repeatRow, "shape.frequency", 6.0) &&
            RowNumber(repeatRow, "shape.phase", 0.25),
        "TestRowFunctionSwitchPreservesSharedParams_ShapeSharedParamsSurvive");

    ColorPipelineRowState heatmapRow;
    Check(color_pipeline_core::BuildColorPipelineRowFromFunctionId(*palette, "heatmap", 42, &heatmapRow) &&
            color_pipeline_core::SetColorPipelineParamNumber(&heatmapRow, "palette.blend_weight", 0.37),
        "TestRowFunctionSwitchPreservesSharedParams_HeatmapBlendConfigured");
    const FunctionDescriptor* explainoCmap = FindFunction(*palette, "explaino_cmap");
    Check(explainoCmap && color_pipeline_core::SetColorPipelineRowFunction(&heatmapRow, *explainoCmap),
        "TestRowFunctionSwitchPreservesSharedParams_SwitchesToExplainoCmap");
    Check(heatmapRow.ui_row_id == 42 && heatmapRow.function_id == "explaino_cmap" &&
            RowNumber(heatmapRow, "palette.blend_weight", 0.37) &&
            RowNumber(heatmapRow, "palette.seed_scale", 1.0),
        "TestRowFunctionSwitchPreservesSharedParams_PaletteSharedParamsSurviveAndNewParamsDefault");

    ColorPipelineRowState invalidBlendModeRow;
    Check(color_pipeline_core::BuildColorPipelineRowFromFunctionId(*palette, "heatmap", 44, &invalidBlendModeRow) &&
            color_pipeline_core::SetColorPipelineParamEnum(&invalidBlendModeRow, "palette.blend_mode", "unsupported"),
        "TestRowFunctionSwitchPreservesSharedParams_InvalidBlendModeConfigured");
    Check(explainoCmap && color_pipeline_core::SetColorPipelineRowFunction(&invalidBlendModeRow, *explainoCmap),
        "TestRowFunctionSwitchPreservesSharedParams_SwitchesInvalidBlendModeRow");
    Check(RowEnum(invalidBlendModeRow, "palette.blend_mode", "normal"),
        "TestRowFunctionSwitchPreservesSharedParams_InvalidSharedEnumFallsBackToDefault");

    ColorPipelineRowState phaseFinishRow;
    Check(color_pipeline_core::BuildColorPipelineRowFromFunctionId(*grading, "phase_finish", 43, &phaseFinishRow) &&
            color_pipeline_core::SetColorPipelineParamNumber(&phaseFinishRow, "grade.saturation", 0.82) &&
            color_pipeline_core::SetColorPipelineParamNumber(&phaseFinishRow, "grade.contrast", 1.7),
        "TestRowFunctionSwitchPreservesSharedParams_PhaseFinishConfigured");
    const FunctionDescriptor* bandFinish = FindFunction(*grading, "band_finish");
    Check(bandFinish && color_pipeline_core::SetColorPipelineRowFunction(&phaseFinishRow, *bandFinish),
        "TestRowFunctionSwitchPreservesSharedParams_SwitchesToBandFinish");
    Check(phaseFinishRow.ui_row_id == 43 && phaseFinishRow.function_id == "band_finish" &&
            RowNumber(phaseFinishRow, "grade.saturation", 0.82) &&
            RowNumber(phaseFinishRow, "grade.contrast", 1.7),
        "TestRowFunctionSwitchPreservesSharedParams_GradingSharedParamsSurvive");
}

void TestImportAndApplySupportedParams() {
    const ColorPipelineLaneCatalog* source = color_pipeline_core::FindColorPipelineLaneCatalog("source");
    const ColorPipelineLaneCatalog* palette = color_pipeline_core::FindColorPipelineLaneCatalog("palette");
    const ColorPipelineLaneCatalog* grading = color_pipeline_core::FindColorPipelineLaneCatalog("grading");
    Check(source && palette && grading, "TestImportAndApplySupportedParams_CatalogsPresent");
    if (!source || !palette || !grading) return;

    ColorPipelineRowState phaseOrbitRow;
    Check(color_pipeline_core::BuildColorPipelineRowFromFunctionId(*source, "phase_orbit", 31, &phaseOrbitRow),
        "TestImportAndApplySupportedParams_PhaseOrbitBuilds");
    KernelParams importedSourceParams;
    importedSourceParams.color_phase_signal_offset = 0.75f;
    importedSourceParams.color_phase_wrap_cycles = 2.5f;
    Check(color_pipeline_core::ImportSupportedColorPipelineParamsFromLive(&phaseOrbitRow, importedSourceParams) &&
            RowNumber(phaseOrbitRow, "signal.phase_offset", 0.75) && RowNumber(phaseOrbitRow, "signal.wrap_cycles", 2.5),
        "TestImportAndApplySupportedParams_PhaseOrbitImportsLiveValues");

    ColorPipelineRowState heatmapRow;
    Check(color_pipeline_core::BuildColorPipelineRowFromFunctionId(*palette, "heatmap", 32, &heatmapRow) &&
            color_pipeline_core::SetColorPipelineParamNumber(&heatmapRow, "palette.cycle_scale", 2.0) &&
            color_pipeline_core::SetColorPipelineParamNumber(&heatmapRow, "palette.saturation", 0.5),
        "TestImportAndApplySupportedParams_HeatmapBuildsAndEdits");
    KernelParams paletteParams;
    paletteParams.color_phase_palette_offset = 1.0f;
    paletteParams.color_iteration_band_emphasis = 0.2f;
    paletteParams.color_iteration_band_palette_offset = 0.4f;
    paletteParams.color_explaino_palette_seed_scale = 2.5f;
    paletteParams.color_explaino_palette_seed_phase = 0.5f;
    paletteParams.color_explaino_palette_colorfulness = 0.25f;
    bool changed = false;
    Check(color_pipeline_core::ApplySupportedColorPipelineRowParamsToLive(heatmapRow, &paletteParams, &changed) && changed,
        "TestImportAndApplySupportedParams_HeatmapApplies");
    Check(Near(paletteParams.color_heatmap_cycle_scale, 2.0f, 1.0e-6) &&
            Near(paletteParams.color_heatmap_saturation, 0.5f, 1.0e-6) &&
            Near(paletteParams.color_phase_palette_offset, 0.0f, 1.0e-6) &&
            Near(paletteParams.color_iteration_band_emphasis, 1.0f, 1.0e-6) &&
            Near(paletteParams.color_iteration_band_palette_offset, 0.0f, 1.0e-6) &&
            Near(paletteParams.color_explaino_palette_seed_scale, 1.0f, 1.0e-6) &&
            Near(paletteParams.color_explaino_palette_seed_phase, 0.0f, 1.0e-6) &&
            Near(paletteParams.color_explaino_palette_colorfulness, 1.0f, 1.0e-6),
        "TestImportAndApplySupportedParams_HeatmapResetsCompetingPaletteOwners");

    ColorPipelineRowState phaseFinishRow;
    Check(color_pipeline_core::BuildColorPipelineRowFromFunctionId(*grading, "phase_finish", 33, &phaseFinishRow) &&
            color_pipeline_core::SetColorPipelineParamNumber(&phaseFinishRow, "grade.saturation", 1.6) &&
            color_pipeline_core::SetColorPipelineParamNumber(&phaseFinishRow, "grade.contrast", 2.1),
        "TestImportAndApplySupportedParams_PhaseFinishBuildsAndEdits");
    KernelParams gradeParams;
    changed = false;
    Check(color_pipeline_core::ApplySupportedColorPipelineRowParamsToLive(phaseFinishRow, &gradeParams, &changed) && changed,
        "TestImportAndApplySupportedParams_PhaseFinishApplies");
    Check(Near(gradeParams.color_saturation, 1.6f, 1.0e-6) && Near(gradeParams.color_contrast, 2.1f, 1.0e-6),
        "TestImportAndApplySupportedParams_PhaseFinishUsesLegacyRuntimeOwners");

    ColorPipelineRowState bandFinishRow;
    Check(color_pipeline_core::BuildColorPipelineRowFromFunctionId(*grading, "band_finish", 35, &bandFinishRow) &&
            color_pipeline_core::SetColorPipelineParamNumber(&bandFinishRow, "grade.saturation", 0.75) &&
            color_pipeline_core::SetColorPipelineParamNumber(&bandFinishRow, "grade.contrast", 1.8),
        "TestImportAndApplySupportedParams_BandFinishBuildsAndEdits");
    KernelParams bandGradeParams;
    changed = false;
    Check(color_pipeline_core::ApplySupportedColorPipelineRowParamsToLive(bandFinishRow, &bandGradeParams, &changed) && changed,
        "TestImportAndApplySupportedParams_BandFinishApplies");
    Check(Near(bandGradeParams.color_saturation, 0.75f, 1.0e-6) && Near(bandGradeParams.color_contrast, 1.8f, 1.0e-6),
        "TestImportAndApplySupportedParams_BandFinishUsesLegacyRuntimeOwners");

    ColorPipelineRowState toneMapImportedRow;
    Check(color_pipeline_core::BuildColorPipelineRowFromFunctionId(*grading, "tone_map_finish", 36, &toneMapImportedRow),
        "TestImportAndApplySupportedParams_ToneMapFinishBuildsForImport");
    KernelParams importedToneMapParams{};
    importedToneMapParams.exposure = 1.25f;
    importedToneMapParams.color_saturation = 0.75f;
    importedToneMapParams.color_contrast = 1.5f;
    Check(color_pipeline_core::ImportSupportedColorPipelineParamsFromLive(&toneMapImportedRow, importedToneMapParams) &&
            RowNumber(toneMapImportedRow, "grade.exposure", 1.25) &&
            RowNumber(toneMapImportedRow, "grade.saturation", 0.75) &&
            RowNumber(toneMapImportedRow, "grade.contrast", 1.5),
        "TestImportAndApplySupportedParams_ToneMapFinishImportsLiveValues");

    ColorPipelineRowState toneMapFinishRow;
    Check(color_pipeline_core::BuildColorPipelineRowFromFunctionId(*grading, "tone_map_finish", 37, &toneMapFinishRow) &&
            color_pipeline_core::SetColorPipelineParamNumber(&toneMapFinishRow, "grade.exposure", 1.35) &&
            color_pipeline_core::SetColorPipelineParamNumber(&toneMapFinishRow, "grade.saturation", 0.75) &&
            color_pipeline_core::SetColorPipelineParamNumber(&toneMapFinishRow, "grade.contrast", 1.6),
        "TestImportAndApplySupportedParams_ToneMapFinishBuildsAndEdits");
    KernelParams toneMapGradeParams;
    changed = false;
    Check(color_pipeline_core::ApplySupportedColorPipelineRowParamsToLive(toneMapFinishRow, &toneMapGradeParams, &changed) && changed,
        "TestImportAndApplySupportedParams_ToneMapFinishApplies");
    Check(Near(toneMapGradeParams.exposure, 1.35f, 1.0e-6) && Near(toneMapGradeParams.color_saturation, 0.75f, 1.0e-6) && Near(toneMapGradeParams.color_contrast, 1.6f, 1.0e-6),
        "TestImportAndApplySupportedParams_ToneMapFinishUsesLegacyRuntimeOwners");

    ColorPipelineRowState bandedRow;
    std::string error;
    Check(color_pipeline_core::BuildColorPipelineRowFromFunctionId(*source, "banded_signal", 34, &bandedRow),
        "TestImportAndApplySupportedParams_BandedSignalBuilds");
    Check(color_pipeline_core::SetColorPipelineParamNumber(&bandedRow, "signal.band_count", 7.5),
        "TestImportAndApplySupportedParams_BandedSignalCanSetFractionalForValidation");
    KernelParams bandParams;
    changed = true;
    Check(!color_pipeline_core::ApplySupportedColorPipelineRowParamsToLive(bandedRow, &bandParams, &changed, &error) &&
            error.find("must be an integer") != std::string::npos && !changed,
        "TestImportAndApplySupportedParams_FractionalBandCountFailsClosed");
    error.clear();
    Check(color_pipeline_core::SetColorPipelineParamNumber(&bandedRow, "signal.band_count", 7.0) &&
            color_pipeline_core::SetColorPipelineParamNumber(&bandedRow, "signal.softness", 0.2) &&
            color_pipeline_core::ApplySupportedColorPipelineRowParamsToLive(bandedRow, &bandParams, &changed, &error) &&
            bandParams.color_iteration_band_count == 7 && Near(bandParams.color_iteration_band_softness, 0.2f, 1.0e-6),
        "TestImportAndApplySupportedParams_IntegerBandCountApplies");

    ColorPipelineRowState invalidHeatmapRow = heatmapRow;
    Check(color_pipeline_core::SetColorPipelineParamNumber(&invalidHeatmapRow, "palette.saturation", 3.5),
        "TestImportAndApplySupportedParams_InvalidHeatmapCanSetForValidation");
    KernelParams invalidParams;
    invalidParams.color_heatmap_cycle_scale = 0.75f;
    invalidParams.color_heatmap_saturation = 0.8f;
    changed = true;
    error.clear();
    Check(!color_pipeline_core::ApplySupportedColorPipelineRowParamsToLive(invalidHeatmapRow, &invalidParams, &changed, &error) &&
            error.find("palette.saturation") != std::string::npos && !changed &&
            Near(invalidParams.color_heatmap_cycle_scale, 0.75f, 1.0e-6) && Near(invalidParams.color_heatmap_saturation, 0.8f, 1.0e-6),
        "TestImportAndApplySupportedParams_OutOfRangeHeatmapFailsBeforeMutation");

    ColorPipelineRowState missingHeatmapRow = heatmapRow;
    for (auto it = missingHeatmapRow.parameter_values.begin(); it != missingHeatmapRow.parameter_values.end(); ++it) {
        if (it->path == "palette.saturation") {
            missingHeatmapRow.parameter_values.erase(it);
            break;
        }
    }
    error.clear();
    Check(!color_pipeline_core::ApplySupportedColorPipelineRowParamsToLive(missingHeatmapRow, &invalidParams, &changed, &error) &&
            error.find("Missing advanced color parameter path") != std::string::npos,
        "TestImportAndApplySupportedParams_MissingParameterFailsClosed");
    Check(!color_pipeline_core::ApplySupportedColorPipelineRowParamsToLive(heatmapRow, nullptr, &changed, &error),
        "TestImportAndApplySupportedParams_NullKernelParamsFailsClosed");
}

void TestSelectionAndScheduleBridgeIds() {
    ColorPipelineSelection selection;
    ColoringMode mode = ColoringMode::smooth_escape;
    Check(color_pipeline_core::TryBuildColorPipelineSelectionFromLaneIds("root_index", "joy_root_palette", &selection, &mode) &&
            selection.signal == ColorSignal::root_index && selection.palette == ColorPalette::joy &&
            selection.grading == ColorGradingPreset::basin_default && mode == ColoringMode::joy_basins,
        "TestSelectionAndScheduleBridgeIds_JoyRootSelection");
    Check(color_pipeline_core::TryBuildColorPipelineSelectionFromLaneIds("escape_magnitude", "explaino_cmap", &selection, &mode) &&
            selection.signal == ColorSignal::escape_magnitude && selection.palette == ColorPalette::explaino_cmap &&
            selection.grading == ColorGradingPreset::escape_default && mode == ColoringMode::smooth_escape,
        "TestSelectionAndScheduleBridgeIds_EscapeMagnitudeExplainoSelection");
    Check(!color_pipeline_core::TryBuildColorPipelineSelectionFromLaneIds("phase_orbit", "heatmap", &selection, &mode),
        "TestSelectionAndScheduleBridgeIds_InvalidTupleFailsClosed");
    Check(!color_pipeline_core::TryBuildColorPipelineSelectionFromLaneIds(nullptr, "heatmap", &selection, &mode),
        "TestSelectionAndScheduleBridgeIds_NullSourceFailsClosed");

    const char* sourceFunction = nullptr;
    const char* paletteFunction = nullptr;
    const ColorPipelineSelection bandsPipeline = {ColorSignal::iteration_bands, ColorPalette::banded_escape, ColorGradingPreset::bands_default};
    Check(color_pipeline_core::TryBuildColorPipelineScheduleBridgeIds(bandsPipeline, &sourceFunction, &paletteFunction) &&
            std::string(sourceFunction ? sourceFunction : "") == "banded_signal" &&
            std::string(paletteFunction ? paletteFunction : "") == "banded_heatmap",
        "TestSelectionAndScheduleBridgeIds_BandsBridge");
    const ColorPipelineSelection rootPipeline = {ColorSignal::root_index, ColorPalette::root_classic, ColorGradingPreset::basin_default};
    Check(color_pipeline_core::TryBuildColorPipelineScheduleBridgeIds(rootPipeline, &sourceFunction, &paletteFunction) &&
            std::string(sourceFunction ? sourceFunction : "") == "root_index" &&
            std::string(paletteFunction ? paletteFunction : "") == "root_classic_palette",
        "TestSelectionAndScheduleBridgeIds_RootBridge");
    const ColorPipelineSelection unsupportedPipeline = {ColorSignal::root_index, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
    sourceFunction = "stale";
    paletteFunction = "stale";
    Check(!color_pipeline_core::TryBuildColorPipelineScheduleBridgeIds(unsupportedPipeline, &sourceFunction, &paletteFunction) &&
            sourceFunction == nullptr && paletteFunction == nullptr,
        "TestSelectionAndScheduleBridgeIds_UnsupportedBridgeClearsOutputs");
}

} // namespace

int main() {
    TestFunctionIdMappingsRoundTrip();
    TestLaneCatalogFiltersRuntimeBackedRows();
    TestRowBuildersAndDefaults();
    TestRowFunctionSwitchPreservesSharedParams();
    TestImportAndApplySupportedParams();
    TestSelectionAndScheduleBridgeIds();

    std::printf("test_color_pipeline_core: passed=%d failed=%d\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
