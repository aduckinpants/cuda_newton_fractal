#include "../src/color_pipeline_core.h"
#include "../src/color_pipeline_metadata_catalog.h"
#include "../src/color_pipeline_metadata_contract.h"
#include "../src/color_pipeline_metadata_parity.h"
#include "../src/enum_id_utils.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
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

bool HasParam(const FunctionDescriptor& function, const char* path) {
    for (const FunctionParamDescriptor& param : function.parameters) {
        if (param.path == path) return true;
    }
    return false;
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

bool FileExists(const char* path) {
    std::ifstream input(path, std::ios::in | std::ios::binary);
    return input.good();
}

std::string ResolveMaterializedContractPath() {
    const char* candidates[] = {
        "..\\docs\\ui_salt\\generated\\color_pipeline_function_library.contract.v1.json",
        "docs\\ui_salt\\generated\\color_pipeline_function_library.contract.v1.json",
    };
    for (const char* candidate : candidates) {
        if (FileExists(candidate)) {
            return candidate;
        }
    }
    return candidates[0];
}

std::string TempContractPath(const char* fileName) {
    const char* tempDir = std::getenv("TEMP");
    std::string base = (tempDir && tempDir[0] != '\0') ? tempDir : ".";
    const char last = base.empty() ? '\0' : base.back();
    if (last != '\\' && last != '/') {
        base.push_back('\\');
    }
    base += fileName ? fileName : "ui_salt_contract_invalid.json";
    return base;
}

bool WriteTextFile(const std::string& path, const char* text) {
    std::ofstream output(path, std::ios::out | std::ios::binary);
    if (!output) {
        return false;
    }
    output << (text ? text : "");
    return output.good();
}

void CheckMaterializedParamMatches(
    const FunctionParamDescriptor& expected,
    const MaterializedColorPipelineParam& actual,
    const char* messagePrefix) {
    Check(actual.path == expected.path, (std::string(messagePrefix) + "_Path").c_str());
    Check(actual.type == expected.type, (std::string(messagePrefix) + "_Type").c_str());
    Check(actual.label == expected.label, (std::string(messagePrefix) + "_Label").c_str());
    Check(actual.has_min == expected.has_min, (std::string(messagePrefix) + "_HasMin").c_str());
    if (actual.has_min && expected.has_min) {
        Check(Near(actual.min_value, expected.min_value), (std::string(messagePrefix) + "_Min").c_str());
    }
    Check(actual.has_max == expected.has_max, (std::string(messagePrefix) + "_HasMax").c_str());
    if (actual.has_max && expected.has_max) {
        Check(Near(actual.max_value, expected.max_value), (std::string(messagePrefix) + "_Max").c_str());
    }
    Check(actual.has_step == expected.has_step, (std::string(messagePrefix) + "_HasStep").c_str());
    if (actual.has_step && expected.has_step) {
        Check(Near(actual.step_value, expected.step_value), (std::string(messagePrefix) + "_Step").c_str());
    }
    Check(actual.has_default == expected.has_default, (std::string(messagePrefix) + "_HasDefault").c_str());
    if (expected.has_default) {
        if (expected.default_value.is_number()) {
            Check(actual.default_kind == "number" && Near(actual.number_default, expected.default_value.as_number()),
                (std::string(messagePrefix) + "_NumericDefault").c_str());
        } else if (expected.default_value.is_bool()) {
            Check(actual.default_kind == "bool" && actual.bool_default == expected.default_value.as_bool(),
                (std::string(messagePrefix) + "_BoolDefault").c_str());
        } else if (expected.default_value.is_string()) {
            Check(actual.default_kind == "string" && actual.string_default == expected.default_value.as_string(),
                (std::string(messagePrefix) + "_StringDefault").c_str());
        } else {
            Check(false, (std::string(messagePrefix) + "_UnsupportedDefaultKind").c_str());
        }
    }
    Check(actual.enum_options.size() == expected.options.size(), (std::string(messagePrefix) + "_EnumOptionCount").c_str());
    const std::size_t optionCount = std::min(actual.enum_options.size(), expected.options.size());
    for (std::size_t index = 0; index < optionCount; ++index) {
        Check(actual.enum_options[index] == expected.options[index].id,
            (std::string(messagePrefix) + "_EnumOption").c_str());
    }
}

void TestFunctionIdMappingsRoundTrip() {
    ColorSignal signal = ColorSignal::smooth_escape;
    Check(std::string(color_pipeline_core::AdvancedColorSignalFunctionId(ColorSignal::root_index)) == "root_index",
        "TestFunctionIdMappingsRoundTrip_SignalId");
    Check(color_pipeline_core::TryParseAdvancedColorSignalFunctionId("orbit_stripe", &signal) && signal == ColorSignal::orbit_stripe,
        "TestFunctionIdMappingsRoundTrip_SignalParse");
    Check(std::string(color_pipeline_core::AdvancedColorSignalFunctionId(ColorSignal::sdf_signed_distance)) == "sdf_signed_distance",
        "TestFunctionIdMappingsRoundTrip_SdfSignalId");
    Check(color_pipeline_core::TryParseAdvancedColorSignalFunctionId("sdf_curvature", &signal) && signal == ColorSignal::sdf_curvature,
        "TestFunctionIdMappingsRoundTrip_SdfSignalParse");
    signal = ColorSignal::orbit_stripe;
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
    Check(std::string(color_pipeline_core::AdvancedColorGradingFunctionId(ColorGradingPreset::glow_default)) == "grade_glow",
        "TestFunctionIdMappingsRoundTrip_GradingIdIncludesGradeGlow");
    Check(color_pipeline_core::TryParseAdvancedColorGradingFunctionId("grade_glow", &grading) &&
            std::string(color_pipeline_core::AdvancedColorGradingFunctionId(grading)) == "grade_glow",
        "TestFunctionIdMappingsRoundTrip_GradingParseGradeGlow");
    Check(std::string(color_pipeline_core::AdvancedColorGradingFunctionId(ColorGradingPreset::balance_void_default)) == "balance_void_grade",
        "TestFunctionIdMappingsRoundTrip_GradingIdIncludesBalanceVoidGrade");
    Check(color_pipeline_core::TryParseAdvancedColorGradingFunctionId("balance_void_grade", &grading) &&
            std::string(color_pipeline_core::AdvancedColorGradingFunctionId(grading)) == "balance_void_grade",
        "TestFunctionIdMappingsRoundTrip_GradingParseBalanceVoidGrade");
    Check(!color_pipeline_core::TryParseAdvancedColorGradingFunctionId("missing_grade", &grading) && grading == ColorGradingPreset::balance_void_default,
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

    Check(source->default_function_id == std::string("smooth_escape_ramp") && source->functions.size() == 12,
        "TestLaneCatalogFiltersRuntimeBackedRows_SourceShape");
    Check(HasFunction(*source, "smooth_escape_ramp") && HasFunction(*source, "phase_orbit") &&
            HasFunction(*source, "banded_signal") && HasFunction(*source, "escape_magnitude") &&
            HasFunction(*source, "orbit_stripe") && HasFunction(*source, "root_proximity") &&
            HasFunction(*source, "root_index") &&
            HasFunction(*source, "sdf_signed_distance") &&
            HasFunction(*source, "sdf_inside_outside") &&
            HasFunction(*source, "sdf_boundary_band") &&
            HasFunction(*source, "sdf_normal_angle") &&
            HasFunction(*source, "sdf_curvature"),
        "TestLaneCatalogFiltersRuntimeBackedRows_SourceFunctions");
    Check(shape->default_function_id == std::string("identity") && shape->functions.size() == 7 &&
            HasFunction(*shape, "smooth_window"),
        "TestLaneCatalogFiltersRuntimeBackedRows_ShapeFunctions");
    Check(palette->default_function_id == std::string("heatmap") && palette->functions.size() == 6 &&
            HasFunction(*palette, "explaino_cmap") && HasFunction(*palette, "root_classic_palette") &&
            HasFunction(*palette, "joy_root_palette"),
        "TestLaneCatalogFiltersRuntimeBackedRows_PaletteFunctions");
    Check(grading->default_function_id == std::string("contrast_lift") && grading->functions.size() == 8 &&
            HasFunction(*grading, "contrast_lift") && HasFunction(*grading, "phase_finish") && HasFunction(*grading, "band_finish") && HasFunction(*grading, "basin_default") && HasFunction(*grading, "neutral_finish") && HasFunction(*grading, "tone_map_finish") && HasFunction(*grading, "grade_glow") && HasFunction(*grading, "balance_void_grade"),
        "TestLaneCatalogFiltersRuntimeBackedRows_GradingShipsBalanceVoidGrade");
    Check(CatalogIdsEqual(*source, {
            "smooth_escape_ramp",
            "phase_orbit",
            "banded_signal",
            "escape_magnitude",
            "orbit_stripe",
            "root_proximity",
            "root_index",
            "sdf_signed_distance",
            "sdf_inside_outside",
            "sdf_boundary_band",
            "sdf_normal_angle",
            "sdf_curvature"}),
        "TestLaneCatalogFiltersRuntimeBackedRows_SourceFunctionOrder");
    Check(CatalogIdsEqual(*shape, {"identity", "offset_scale", "repeat", "posterize", "mirror_repeat", "bias_gain_curve", "smooth_window"}),
        "TestLaneCatalogFiltersRuntimeBackedRows_ShapeFunctionOrder");
    Check(CatalogIdsEqual(*palette, {"heatmap", "phase_wheel_palette", "banded_heatmap", "explaino_cmap", "root_classic_palette", "joy_root_palette"}),
        "TestLaneCatalogFiltersRuntimeBackedRows_PaletteFunctionOrder");
    Check(CatalogIdsEqual(*grading, {"contrast_lift", "phase_finish", "band_finish", "basin_default", "neutral_finish", "tone_map_finish", "grade_glow", "balance_void_grade"}),
        "TestLaneCatalogFiltersRuntimeBackedRows_GradingFunctionOrder");

    const std::vector<FunctionDescriptor> allGradeFunctions = color_pipeline_core::BuildColorPipelineGradeFunctions();
    bool rawGradeIncludesBandFinish = false;
    bool rawGradeIncludesBasinDefault = false;
    bool rawGradeIncludesNeutralFinish = false;
    bool rawGradeIncludesToneMapFinish = false;
    bool rawGradeIncludesGradeGlow = false;
    bool rawGradeIncludesBalanceVoidGrade = false;
    for (const FunctionDescriptor& descriptor : allGradeFunctions) {
        rawGradeIncludesBandFinish = rawGradeIncludesBandFinish || descriptor.id == "band_finish";
        rawGradeIncludesBasinDefault = rawGradeIncludesBasinDefault || descriptor.id == "basin_default";
        rawGradeIncludesNeutralFinish = rawGradeIncludesNeutralFinish || descriptor.id == "neutral_finish";
        rawGradeIncludesToneMapFinish = rawGradeIncludesToneMapFinish || descriptor.id == "tone_map_finish";
        rawGradeIncludesGradeGlow = rawGradeIncludesGradeGlow || descriptor.id == "grade_glow";
        rawGradeIncludesBalanceVoidGrade = rawGradeIncludesBalanceVoidGrade || descriptor.id == "balance_void_grade";
    }
    Check(rawGradeIncludesBandFinish, "TestLaneCatalogFiltersRuntimeBackedRows_RawGradeCatalogNamesDraftBandFinish");
    Check(rawGradeIncludesBasinDefault, "TestLaneCatalogFiltersRuntimeBackedRows_RawGradeCatalogNamesBasinDefault");
    Check(rawGradeIncludesNeutralFinish, "TestLaneCatalogFiltersRuntimeBackedRows_RawGradeCatalogNamesNeutralFinish");
    Check(rawGradeIncludesToneMapFinish, "TestLaneCatalogFiltersRuntimeBackedRows_RawGradeCatalogNamesToneMapFinish");
    Check(rawGradeIncludesGradeGlow, "TestLaneCatalogFiltersRuntimeBackedRows_RawGradeCatalogNamesGradeGlow");
    Check(rawGradeIncludesBalanceVoidGrade, "TestLaneCatalogFiltersRuntimeBackedRows_RawGradeCatalogNamesBalanceVoidGrade");
    Check(color_pipeline_core::IsColorPipelineFunctionRuntimeBacked("grading", "band_finish"),
        "TestLaneCatalogFiltersRuntimeBackedRows_BandFinishRuntimeBacked");
    Check(color_pipeline_core::IsColorPipelineFunctionRuntimeBacked("grading", "basin_default"),
        "TestLaneCatalogFiltersRuntimeBackedRows_BasinDefaultRuntimeBacked");
    Check(color_pipeline_core::IsColorPipelineFunctionRuntimeBacked("grading", "neutral_finish"),
        "TestLaneCatalogFiltersRuntimeBackedRows_NeutralFinishRuntimeBacked");
    Check(color_pipeline_core::IsColorPipelineFunctionRuntimeBacked("grading", "tone_map_finish"),
        "TestLaneCatalogFiltersRuntimeBackedRows_ToneMapFinishRuntimeBacked");
    Check(color_pipeline_core::IsColorPipelineFunctionRuntimeBacked("grading", "grade_glow"),
        "TestLaneCatalogFiltersRuntimeBackedRows_GradeGlowRuntimeBacked");
    Check(color_pipeline_core::IsColorPipelineFunctionRuntimeBacked("grading", "balance_void_grade"),
        "TestLaneCatalogFiltersRuntimeBackedRows_BalanceVoidGradeRuntimeBacked");
    Check(!color_pipeline_core::IsColorPipelineFunctionRuntimeBacked(nullptr, "heatmap") &&
            !color_pipeline_core::IsColorPipelineFunctionRuntimeBacked("unknown_lane", "heatmap"),
        "TestLaneCatalogFiltersRuntimeBackedRows_UnknownLaneFailsClosed");
    Check(color_pipeline_core::FindColorPipelineLaneCatalog("missing") == nullptr,
        "TestLaneCatalogFiltersRuntimeBackedRows_UnknownCatalogMissing");
}

void TestSourceSignalKindMetadata() {
    using color_pipeline_core::ColorPipelineSourceSignalKind;
    Check(std::string(color_pipeline_core::ColorPipelineSourceSignalKindId(ColorPipelineSourceSignalKind::scalar)) == "scalar",
        "TestSourceSignalKindMetadata_ScalarId");
    Check(std::string(color_pipeline_core::ColorPipelineSourceSignalKindId(ColorPipelineSourceSignalKind::phase)) == "phase",
        "TestSourceSignalKindMetadata_PhaseId");
    Check(std::string(color_pipeline_core::ColorPipelineSourceSignalKindId(ColorPipelineSourceSignalKind::categorical)) == "categorical",
        "TestSourceSignalKindMetadata_CategoricalId");

    Check(color_pipeline_core::ColorPipelineSourceSignalKindForSignal(ColorSignal::sdf_normal_angle) ==
            ColorPipelineSourceSignalKind::phase,
        "TestSourceSignalKindMetadata_SdfNormalAngleIsPhase");
    Check(color_pipeline_core::ColorPipelineSourceSignalKindForFunctionId("sdf_normal_angle") ==
            ColorPipelineSourceSignalKind::phase,
        "TestSourceSignalKindMetadata_SdfNormalAngleFunctionIsPhase");
    Check(color_pipeline_core::ColorPipelineSourceFunctionIsPhaseSignal("sdf_normal_angle"),
        "TestSourceSignalKindMetadata_SdfNormalAnglePhasePredicate");

    Check(color_pipeline_core::ColorPipelineSourceSignalKindForSignal(ColorSignal::phase_angle) ==
            ColorPipelineSourceSignalKind::phase,
        "TestSourceSignalKindMetadata_PhaseOrbitIsPhase");
    Check(color_pipeline_core::ColorPipelineSourceSignalKindForFunctionId("orbit_stripe") ==
            ColorPipelineSourceSignalKind::phase,
        "TestSourceSignalKindMetadata_OrbitStripeIsPhase");
    Check(color_pipeline_core::ColorPipelineSourceSignalKindForSignal(ColorSignal::root_index) ==
            ColorPipelineSourceSignalKind::categorical,
        "TestSourceSignalKindMetadata_RootIndexIsCategorical");
    Check(color_pipeline_core::ColorPipelineSourceSignalKindForFunctionId("sdf_inside_outside") ==
            ColorPipelineSourceSignalKind::categorical,
        "TestSourceSignalKindMetadata_SdfInsideOutsideIsCategorical");

    const ColorSignal scalarSignals[] = {
        ColorSignal::smooth_escape,
        ColorSignal::iteration_count,
        ColorSignal::iteration_bands,
        ColorSignal::escape_magnitude,
        ColorSignal::root_proximity,
        ColorSignal::sdf_signed_distance,
        ColorSignal::sdf_boundary_band,
        ColorSignal::sdf_curvature,
    };
    for (ColorSignal signal : scalarSignals) {
        Check(color_pipeline_core::ColorPipelineSourceSignalKindForSignal(signal) ==
                ColorPipelineSourceSignalKind::scalar,
            "TestSourceSignalKindMetadata_ExpectedScalarSignal");
    }
    Check(color_pipeline_core::ColorPipelineSourceSignalKindForFunctionId("smooth_escape_ramp") ==
            ColorPipelineSourceSignalKind::scalar,
        "TestSourceSignalKindMetadata_SmoothEscapeFunctionIsScalar");
    Check(color_pipeline_core::ColorPipelineSourceSignalKindForFunctionId("sdf_signed_distance") ==
            ColorPipelineSourceSignalKind::scalar,
        "TestSourceSignalKindMetadata_SdfSignedDistanceFunctionIsScalar");
    Check(color_pipeline_core::ColorPipelineSourceSignalKindForFunctionId("sdf_boundary_band") ==
            ColorPipelineSourceSignalKind::scalar,
        "TestSourceSignalKindMetadata_SdfBoundaryBandFunctionIsScalar");
    Check(color_pipeline_core::ColorPipelineSourceSignalKindForFunctionId("sdf_curvature") ==
            ColorPipelineSourceSignalKind::scalar,
        "TestSourceSignalKindMetadata_SdfCurvatureFunctionIsScalar");
    Check(color_pipeline_core::ColorPipelineSourceSignalKindForFunctionId("missing_source") ==
            ColorPipelineSourceSignalKind::scalar,
        "TestSourceSignalKindMetadata_UnknownDefaultsScalar");
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
    ColorPipelineRowState gradeGlowRow;
    error.clear();
    Check(color_pipeline_core::BuildColorPipelineRowFromFunctionId(*grading, "grade_glow", 39, &gradeGlowRow, &error) &&
            gradeGlowRow.ui_row_id == 39 &&
            gradeGlowRow.function_id == "grade_glow" &&
            gradeGlowRow.parameter_values.size() == 4 &&
            RowNumber(gradeGlowRow, "grade.exposure", 1.0) &&
            RowNumber(gradeGlowRow, "grade.saturation", 1.15) &&
            RowNumber(gradeGlowRow, "grade.contrast", 1.10) &&
            RowNumber(gradeGlowRow, "grade.glow", 0.25),
        "TestRowBuildersAndDefaults_GradeGlowBuildsFromRuntimeCatalog");
    ColorPipelineRowState balanceVoidGradeRow;
    error.clear();
    Check(color_pipeline_core::BuildColorPipelineRowFromFunctionId(*grading, "balance_void_grade", 40, &balanceVoidGradeRow, &error) &&
            balanceVoidGradeRow.ui_row_id == 40 &&
            balanceVoidGradeRow.function_id == "balance_void_grade" &&
            balanceVoidGradeRow.parameter_values.size() == 3 &&
            RowNumber(balanceVoidGradeRow, "grade.balance_void", 0.0) &&
            RowNumber(balanceVoidGradeRow, "grade.chroma_tension", 0.0) &&
            RowNumber(balanceVoidGradeRow, "grade.accent_bias", 0.0),
        "TestRowBuildersAndDefaults_BalanceVoidGradeBuildsFromRuntimeCatalog");
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

    ColorPipelineRowState gradeGlowImportedRow;
    Check(color_pipeline_core::BuildColorPipelineRowFromFunctionId(*grading, "grade_glow", 38, &gradeGlowImportedRow),
        "TestImportAndApplySupportedParams_GradeGlowBuildsForImport");
    KernelParams importedGradeGlowParams{};
    importedGradeGlowParams.exposure = 1.25f;
    importedGradeGlowParams.color_saturation = 0.75f;
    importedGradeGlowParams.color_contrast = 1.5f;
    importedGradeGlowParams.color_glow = 0.625f;
    Check(color_pipeline_core::ImportSupportedColorPipelineParamsFromLive(&gradeGlowImportedRow, importedGradeGlowParams) &&
            RowNumber(gradeGlowImportedRow, "grade.exposure", 1.25) &&
            RowNumber(gradeGlowImportedRow, "grade.saturation", 0.75) &&
            RowNumber(gradeGlowImportedRow, "grade.contrast", 1.5) &&
            RowNumber(gradeGlowImportedRow, "grade.glow", 0.625),
        "TestImportAndApplySupportedParams_GradeGlowImportsLiveValues");

    ColorPipelineRowState gradeGlowAppliedRow;
    Check(color_pipeline_core::BuildColorPipelineRowFromFunctionId(*grading, "grade_glow", 39, &gradeGlowAppliedRow) &&
            color_pipeline_core::SetColorPipelineParamNumber(&gradeGlowAppliedRow, "grade.exposure", 1.25) &&
            color_pipeline_core::SetColorPipelineParamNumber(&gradeGlowAppliedRow, "grade.saturation", 0.75) &&
            color_pipeline_core::SetColorPipelineParamNumber(&gradeGlowAppliedRow, "grade.contrast", 1.5) &&
            color_pipeline_core::SetColorPipelineParamNumber(&gradeGlowAppliedRow, "grade.glow", 0.625),
        "TestImportAndApplySupportedParams_GradeGlowBuildsAndEdits");
    KernelParams gradeGlowParams;
    changed = false;
    Check(color_pipeline_core::ApplySupportedColorPipelineRowParamsToLive(gradeGlowAppliedRow, &gradeGlowParams, &changed) && changed,
        "TestImportAndApplySupportedParams_GradeGlowApplies");
    Check(Near(gradeGlowParams.exposure, 1.25f, 1.0e-6) && Near(gradeGlowParams.color_saturation, 0.75f, 1.0e-6) && Near(gradeGlowParams.color_contrast, 1.5f, 1.0e-6) && Near(gradeGlowParams.color_glow, 0.625f, 1.0e-6),
        "TestImportAndApplySupportedParams_GradeGlowUsesLegacyRuntimeOwners");

    ColorPipelineRowState balanceVoidImportedRow;
    Check(color_pipeline_core::BuildColorPipelineRowFromFunctionId(*grading, "balance_void_grade", 40, &balanceVoidImportedRow),
        "TestImportAndApplySupportedParams_BalanceVoidGradeBuildsForImport");
    KernelParams importedBalanceVoidParams{};
    importedBalanceVoidParams.color_balance_void = 0.35f;
    importedBalanceVoidParams.color_chroma_tension = 0.6f;
    importedBalanceVoidParams.color_accent_bias = -0.25f;
    Check(color_pipeline_core::ImportSupportedColorPipelineParamsFromLive(&balanceVoidImportedRow, importedBalanceVoidParams) &&
            RowNumber(balanceVoidImportedRow, "grade.balance_void", 0.35) &&
            RowNumber(balanceVoidImportedRow, "grade.chroma_tension", 0.6) &&
            RowNumber(balanceVoidImportedRow, "grade.accent_bias", -0.25),
        "TestImportAndApplySupportedParams_BalanceVoidGradeImportsLiveValues");

    ColorPipelineRowState balanceVoidAppliedRow;
    Check(color_pipeline_core::BuildColorPipelineRowFromFunctionId(*grading, "balance_void_grade", 41, &balanceVoidAppliedRow) &&
            color_pipeline_core::SetColorPipelineParamNumber(&balanceVoidAppliedRow, "grade.balance_void", 0.35) &&
            color_pipeline_core::SetColorPipelineParamNumber(&balanceVoidAppliedRow, "grade.chroma_tension", 0.6) &&
            color_pipeline_core::SetColorPipelineParamNumber(&balanceVoidAppliedRow, "grade.accent_bias", -0.25),
        "TestImportAndApplySupportedParams_BalanceVoidGradeBuildsAndEdits");
    KernelParams balanceVoidGradeParams;
    changed = false;
    Check(color_pipeline_core::ApplySupportedColorPipelineRowParamsToLive(balanceVoidAppliedRow, &balanceVoidGradeParams, &changed) && changed,
        "TestImportAndApplySupportedParams_BalanceVoidGradeApplies");
    Check(Near(balanceVoidGradeParams.color_balance_void, 0.35f, 1.0e-6) && Near(balanceVoidGradeParams.color_chroma_tension, 0.6f, 1.0e-6) && Near(balanceVoidGradeParams.color_accent_bias, -0.25f, 1.0e-6),
        "TestImportAndApplySupportedParams_BalanceVoidGradeUsesDedicatedRuntimeOwners");

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
    Check(color_pipeline_core::TryBuildColorPipelineSelectionFromLaneIds("sdf_signed_distance", "heatmap", &selection, &mode) &&
            selection.signal == ColorSignal::sdf_signed_distance && selection.palette == ColorPalette::cyclic_escape &&
            selection.grading == ColorGradingPreset::escape_default && mode == ColoringMode::smooth_escape,
        "TestSelectionAndScheduleBridgeIds_SdfSignedDistanceHeatmapSelection");
    Check(color_pipeline_core::TryBuildColorPipelineSelectionFromLaneIds("sdf_normal_angle", "phase_wheel_palette", &selection, &mode) &&
            selection.signal == ColorSignal::sdf_normal_angle && selection.palette == ColorPalette::phase_wheel &&
            selection.grading == ColorGradingPreset::phase_default && mode == ColoringMode::phase,
        "TestSelectionAndScheduleBridgeIds_SdfNormalAnglePhaseSelection");
    Check(!color_pipeline_core::TryBuildColorPipelineSelectionFromLaneIds("phase_orbit", "heatmap", &selection, &mode),
        "TestSelectionAndScheduleBridgeIds_InvalidTupleFailsClosed");
    Check(!color_pipeline_core::TryBuildColorPipelineSelectionFromLaneIds(nullptr, "heatmap", &selection, &mode),
        "TestSelectionAndScheduleBridgeIds_NullSourceFailsClosed");

    const char* sourceFunction = nullptr;
    const char* paletteFunction = nullptr;
    const ColorGradingPreset smoothEscapeGradingRows[] = {
        ColorGradingPreset::escape_default,
        ColorGradingPreset::neutral_default,
        ColorGradingPreset::tone_map_default,
        ColorGradingPreset::glow_default,
        ColorGradingPreset::balance_void_default,
    };
    for (ColorGradingPreset grading : smoothEscapeGradingRows) {
        sourceFunction = nullptr;
        paletteFunction = nullptr;
        const ColorPipelineSelection smoothEscapePipeline = {ColorSignal::smooth_escape, ColorPalette::cyclic_escape, grading};
        Check(color_pipeline_core::TryBuildColorPipelineScheduleBridgeIds(smoothEscapePipeline, &sourceFunction, &paletteFunction) &&
                std::string(sourceFunction ? sourceFunction : "") == "smooth_escape_ramp" &&
                std::string(paletteFunction ? paletteFunction : "") == "heatmap",
            "TestSelectionAndScheduleBridgeIds_ShippedSmoothEscapeGradingRowsBridge");
    }
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
    const ColorPipelineSelection sdfHeatmapPipeline = {ColorSignal::sdf_boundary_band, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
    Check(color_pipeline_core::TryBuildColorPipelineScheduleBridgeIds(sdfHeatmapPipeline, &sourceFunction, &paletteFunction) &&
            std::string(sourceFunction ? sourceFunction : "") == "sdf_boundary_band" &&
            std::string(paletteFunction ? paletteFunction : "") == "heatmap",
        "TestSelectionAndScheduleBridgeIds_SdfHeatmapBridge");
    const ColorPipelineSelection sdfNormalAnglePipeline = {ColorSignal::sdf_normal_angle, ColorPalette::phase_wheel, ColorGradingPreset::phase_default};
    Check(color_pipeline_core::TryBuildColorPipelineScheduleBridgeIds(sdfNormalAnglePipeline, &sourceFunction, &paletteFunction) &&
            std::string(sourceFunction ? sourceFunction : "") == "sdf_normal_angle" &&
            std::string(paletteFunction ? paletteFunction : "") == "phase_wheel_palette",
        "TestSelectionAndScheduleBridgeIds_SdfNormalAngleBridge");
    const ColorPipelineSelection unsupportedPipeline = {ColorSignal::root_index, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
    sourceFunction = "stale";
    paletteFunction = "stale";
    Check(!color_pipeline_core::TryBuildColorPipelineScheduleBridgeIds(unsupportedPipeline, &sourceFunction, &paletteFunction) &&
            sourceFunction == nullptr && paletteFunction == nullptr,
        "TestSelectionAndScheduleBridgeIds_UnsupportedBridgeClearsOutputs");
}

void TestSdfSourceRowsAreRuntimeBackedCatalogRows() {
    const ColorPipelineLaneCatalog* source = color_pipeline_core::FindColorPipelineLaneCatalog("source");
    Check(source != nullptr, "TestSdfSourceRowsAreRuntimeBackedCatalogRows_SourceCatalogPresent");
    if (!source) return;

    struct ExpectedSdfSource {
        const char* id;
        double scale;
        double bias;
    };
    const ExpectedSdfSource sdfSourceIds[] = {
        {"sdf_signed_distance", 0.05, 0.5},
        {"sdf_inside_outside", 1.0, 0.0},
        {"sdf_boundary_band", 1.0, 0.0},
        {"sdf_normal_angle", 1.0, 0.0},
        {"sdf_curvature", 0.25, 0.5},
    };
    for (const ExpectedSdfSource& expected : sdfSourceIds) {
        const FunctionDescriptor* descriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*source, expected.id);
        Check(descriptor != nullptr, "TestSdfSourceRowsAreRuntimeBackedCatalogRows_SourceCatalogIncludesSdfRow");
        if (!descriptor) continue;
        Check(color_pipeline_core::IsColorPipelineFunctionRuntimeBacked("source", expected.id),
            "TestSdfSourceRowsAreRuntimeBackedCatalogRows_SdfRowRuntimeBacked");
        Check(HasParam(*descriptor, "signal.scale") && HasParam(*descriptor, "signal.bias") &&
                HasParam(*descriptor, "signal.blend_weight"),
            "TestSdfSourceRowsAreRuntimeBackedCatalogRows_SdfRowsExposeCommonLiveParams");
        Check((std::string(expected.id) == "sdf_boundary_band") == HasParam(*descriptor, "signal.boundary_width_px"),
            "TestSdfSourceRowsAreRuntimeBackedCatalogRows_BoundaryWidthIsBoundaryBandOnly");

        ColorPipelineRowState row;
        std::string error;
        Check(color_pipeline_core::BuildColorPipelineRowFromFunctionId(*source, expected.id, 41, &row, &error) &&
                row.function_id == expected.id &&
                RowNumber(row, "signal.scale", expected.scale) &&
                RowNumber(row, "signal.bias", expected.bias) &&
                RowNumber(row, "signal.blend_weight", 1.0) &&
                (std::string(expected.id) != "sdf_boundary_band" || RowNumber(row, "signal.boundary_width_px", 2.0)),
            "TestSdfSourceRowsAreRuntimeBackedCatalogRows_SdfRowBuildsWithVisibleTuningControls");
    }
}

void TestMaterializedUiSaltMetadataShadowsCurrentCatalog() {
    MaterializedColorPipelineContract contract;
    std::string error;
    const std::string path = ResolveMaterializedContractPath();
    Check(LoadColorPipelineMaterializedContractJson(path, &contract, &error),
        (std::string("TestMaterializedUiSaltMetadataShadowsCurrentCatalog_Loads: ") + error).c_str());
    if (!error.empty() || contract.schema_version != 1) {
        Check(false, "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_ContractLoadOrVersion");
        return;
    }

    Check(contract.lanes.size() == color_pipeline_core::GetColorPipelineLaneCatalogs().size(),
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_LaneCount");
    Check(contract.compatibility.size() == 20,
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_CompatibilityCount");
    Check(!contract.recipes.empty(), "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_RecipesPresent");
    Check(!contract.explaino_entries.empty(), "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_ExplainoEntriesPresent");

    const ColorPipelineMetadataParityReport parity = ValidateColorPipelineMetadataParity(contract);
    Check(parity.ok && parity.errors.empty(),
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_ReusableParityReportOk");
    Check(parity.lane_count == 4 && parity.function_count == 33 &&
            parity.compatibility_count == 20 && parity.unsupported_pair_count > 0,
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_ReusableParityReportCounts");

    const std::vector<ColorPipelineLaneCatalog>& catalogs = color_pipeline_core::GetColorPipelineLaneCatalogs();
    for (std::size_t laneIndex = 0; laneIndex < catalogs.size(); ++laneIndex) {
        const ColorPipelineLaneCatalog& expectedLane = catalogs[laneIndex];
        const MaterializedColorPipelineLane* actualLane = FindMaterializedColorPipelineLane(contract, expectedLane.lane_id);
        Check(actualLane != nullptr, "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_LanePresent");
        if (!actualLane) {
            continue;
        }
        Check(contract.lanes[laneIndex].id == expectedLane.lane_id,
            "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_LaneOrder");
        Check(actualLane->label == expectedLane.label,
            "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_LaneLabel");
        Check(actualLane->default_function_id == expectedLane.default_function_id,
            "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_LaneDefault");
        Check(actualLane->functions.size() == expectedLane.functions.size(),
            "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_FunctionCount");
        const std::size_t functionCount = std::min(actualLane->functions.size(), expectedLane.functions.size());
        for (std::size_t functionIndex = 0; functionIndex < functionCount; ++functionIndex) {
            const FunctionDescriptor& expectedFunction = expectedLane.functions[functionIndex];
            const MaterializedColorPipelineFunction& actualFunction = actualLane->functions[functionIndex];
            Check(actualFunction.id == expectedFunction.id,
                "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_FunctionOrder");
            Check(actualFunction.label == expectedFunction.name,
                "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_FunctionLabel");
            Check(actualFunction.description == expectedFunction.description,
                "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_FunctionDescription");
            Check(actualFunction.runtime_backed,
                "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_FunctionRuntimeBacked");
            Check(actualFunction.params.size() == expectedFunction.parameters.size(),
                "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_ParamCount");
            const std::size_t paramCount = std::min(actualFunction.params.size(), expectedFunction.parameters.size());
            for (std::size_t paramIndex = 0; paramIndex < paramCount; ++paramIndex) {
                CheckMaterializedParamMatches(
                    expectedFunction.parameters[paramIndex],
                    actualFunction.params[paramIndex],
                    "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_Param");
            }
            if (std::string(expectedLane.lane_id) == "source") {
                Check(actualFunction.signal_kind ==
                        color_pipeline_core::ColorPipelineSourceSignalKindId(
                            color_pipeline_core::ColorPipelineSourceSignalKindForFunctionId(expectedFunction.id.c_str())),
                    "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_SourceSignalKind");
            } else {
                Check(actualFunction.signal_kind.empty(),
                    "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_NonSourceHasNoSignalKind");
            }
        }
    }

    const ColorPipelineLaneCatalog* sourceLane = color_pipeline_core::FindColorPipelineLaneCatalog("source");
    const ColorPipelineLaneCatalog* paletteLane = color_pipeline_core::FindColorPipelineLaneCatalog("palette");
    Check(sourceLane != nullptr && paletteLane != nullptr,
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_SourcePaletteCatalogsPresent");
    if (sourceLane && paletteLane) {
        for (const FunctionDescriptor& sourceFunction : sourceLane->functions) {
            for (const FunctionDescriptor& paletteFunction : paletteLane->functions) {
                ColorPipelineSelection selection;
                ColoringMode mode = ColoringMode::smooth_escape;
                const bool runtimeSupported = color_pipeline_core::TryBuildColorPipelineSelectionFromLaneIds(
                    sourceFunction.id.c_str(),
                    paletteFunction.id.c_str(),
                    &selection,
                    &mode);
                const MaterializedColorPipelineCompatibility* materializedCompatibility =
                    FindMaterializedColorPipelineCompatibility(contract, sourceFunction.id, paletteFunction.id);
                Check(runtimeSupported == (materializedCompatibility != nullptr),
                    "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_CompatibilityAllowDeny");
                if (runtimeSupported && materializedCompatibility) {
                    Check(materializedCompatibility->signal == color_pipeline_core::AdvancedColorSignalFunctionId(selection.signal),
                        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_CompatibilitySignal");
                    Check(materializedCompatibility->palette_runtime == color_pipeline_core::AdvancedColorPaletteFunctionId(selection.palette),
                        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_CompatibilityPalette");
                    Check(materializedCompatibility->grading == color_pipeline_core::AdvancedColorGradingFunctionId(selection.grading),
                        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_CompatibilityGrading");
                    Check(materializedCompatibility->mode == ColoringModeId(mode),
                        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_CompatibilityMode");
                    Check(!materializedCompatibility->reason.empty(),
                        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_CompatibilityReason");
                }
            }
        }
    }

    for (const MaterializedColorPipelineRecipe& recipe : contract.recipes) {
        Check(FindMaterializedColorPipelineCompatibility(contract, recipe.source, recipe.palette) != nullptr,
            "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_RecipeHasSupportedBridge");
        const MaterializedColorPipelineLane* shapeLane = FindMaterializedColorPipelineLane(contract, "shape");
        const MaterializedColorPipelineLane* gradingLane = FindMaterializedColorPipelineLane(contract, "grading");
        Check(shapeLane && FindMaterializedColorPipelineFunction(*shapeLane, recipe.shape),
            "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_RecipeShapeBacked");
        Check(gradingLane && FindMaterializedColorPipelineFunction(*gradingLane, recipe.grading),
            "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_RecipeGradingBacked");
    }

    bool explainoPaletteFound = false;
    bool balanceVoidFound = false;
    for (const MaterializedExplainoContractEntry& entry : contract.explaino_entries) {
        Check(!entry.product_facing && entry.diagnostic,
            "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_ExplainoDiagnosticOnly");
        Check(!entry.hypothesis_space.empty() && !entry.authority.empty() && !entry.lens.empty() &&
                !entry.invariant.empty() && !entry.proof.empty() && !entry.fallback.empty(),
            "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_ExplainoProofFields");
        explainoPaletteFound = explainoPaletteFound || entry.id == "color_pipeline.explaino_cmap";
        balanceVoidFound = balanceVoidFound || entry.id == "color_pipeline.balance_void_grade";
    }
    Check(explainoPaletteFound && balanceVoidFound,
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_ExpectedExplainoEntriesPresent");
}

void TestMaterializedUiSaltMetadataCanOwnPublicCatalog() {
    color_pipeline_core::ClearColorPipelineMetadataCatalogForTests();
    Check(!color_pipeline_core::IsColorPipelineMetadataCatalogActive(),
        "TestMaterializedUiSaltMetadataCanOwnPublicCatalog_StartsHardcoded");

    MaterializedColorPipelineContract contract;
    std::string error;
    const std::string path = ResolveMaterializedContractPath();
    Check(LoadColorPipelineMaterializedContractJson(path, &contract, &error),
        (std::string("TestMaterializedUiSaltMetadataCanOwnPublicCatalog_Loads: ") + error).c_str());
    if (!error.empty() || contract.schema_version != 1) {
        Check(false, "TestMaterializedUiSaltMetadataCanOwnPublicCatalog_ContractLoadOrVersion");
        return;
    }

    const std::vector<ColorPipelineLaneCatalog>& hardcoded =
        color_pipeline_core::GetHardcodedColorPipelineLaneCatalogs();
    Check(color_pipeline_core::TryInstallColorPipelineMetadataCatalog(contract, &error),
        (std::string("TestMaterializedUiSaltMetadataCanOwnPublicCatalog_Install: ") + error).c_str());
    Check(color_pipeline_core::IsColorPipelineMetadataCatalogActive(),
        "TestMaterializedUiSaltMetadataCanOwnPublicCatalog_MetadataActive");
    Check(color_pipeline_core::ColorPipelineCatalogAuthorityId() == std::string("materialized_json"),
        "TestMaterializedUiSaltMetadataCanOwnPublicCatalog_Authority");

    const std::vector<ColorPipelineLaneCatalog>& active =
        color_pipeline_core::GetColorPipelineLaneCatalogs();
    Check(active.size() == hardcoded.size(),
        "TestMaterializedUiSaltMetadataCanOwnPublicCatalog_LaneCount");
    for (std::size_t laneIndex = 0; laneIndex < active.size() && laneIndex < hardcoded.size(); ++laneIndex) {
        const ColorPipelineLaneCatalog& expectedLane = hardcoded[laneIndex];
        const ColorPipelineLaneCatalog& actualLane = active[laneIndex];
        Check(std::string(actualLane.lane_id) == expectedLane.lane_id,
            "TestMaterializedUiSaltMetadataCanOwnPublicCatalog_LaneId");
        Check(std::string(actualLane.label) == expectedLane.label,
            "TestMaterializedUiSaltMetadataCanOwnPublicCatalog_LaneLabel");
        Check(std::string(actualLane.default_function_id) == expectedLane.default_function_id,
            "TestMaterializedUiSaltMetadataCanOwnPublicCatalog_LaneDefault");
        Check(actualLane.functions.size() == expectedLane.functions.size(),
            "TestMaterializedUiSaltMetadataCanOwnPublicCatalog_FunctionCount");
        for (std::size_t functionIndex = 0;
                functionIndex < actualLane.functions.size() && functionIndex < expectedLane.functions.size();
                ++functionIndex) {
            Check(actualLane.functions[functionIndex].id == expectedLane.functions[functionIndex].id,
                "TestMaterializedUiSaltMetadataCanOwnPublicCatalog_FunctionOrder");
            Check(actualLane.functions[functionIndex].parameters.size() ==
                    expectedLane.functions[functionIndex].parameters.size(),
                "TestMaterializedUiSaltMetadataCanOwnPublicCatalog_ParamCount");
            for (std::size_t paramIndex = 0;
                    paramIndex < actualLane.functions[functionIndex].parameters.size() &&
                    paramIndex < expectedLane.functions[functionIndex].parameters.size();
                    ++paramIndex) {
                const FunctionParamDescriptor& actualParam =
                    actualLane.functions[functionIndex].parameters[paramIndex];
                const FunctionParamDescriptor& expectedParam =
                    expectedLane.functions[functionIndex].parameters[paramIndex];
                Check(actualParam.help == expectedParam.help,
                    "TestMaterializedUiSaltMetadataCanOwnPublicCatalog_ParamHelpPreserved");
                Check(actualParam.options.size() == expectedParam.options.size(),
                    "TestMaterializedUiSaltMetadataCanOwnPublicCatalog_ParamOptionCount");
                for (std::size_t optionIndex = 0;
                        optionIndex < actualParam.options.size() && optionIndex < expectedParam.options.size();
                        ++optionIndex) {
                    Check(actualParam.options[optionIndex].label == expectedParam.options[optionIndex].label,
                        "TestMaterializedUiSaltMetadataCanOwnPublicCatalog_ParamOptionLabelPreserved");
                }
            }
        }
    }

    color_pipeline_core::ClearColorPipelineMetadataCatalogForTests();
    Check(!color_pipeline_core::IsColorPipelineMetadataCatalogActive(),
        "TestMaterializedUiSaltMetadataCanOwnPublicCatalog_ClearRestoresHardcoded");
}

void TestMaterializedContractLoaderRejectsTamperedJson() {
    const char* duplicateFunctionJson = R"json({
  "schema_version": 1,
  "source_path": "tampered.ui.salt",
  "function_library": {
    "lanes": [
      {
        "id": "source",
        "label": "Source",
        "default": "smooth_escape_ramp",
        "functions": [
          {"id": "smooth_escape_ramp", "label": "Smooth Escape Ramp", "description": "", "runtime_backed": true, "input_kind": "scalar", "output_kind": "scalar", "signal_kind": "scalar", "params": []},
          {"id": "smooth_escape_ramp", "label": "Duplicate", "description": "", "runtime_backed": true, "input_kind": "scalar", "output_kind": "scalar", "signal_kind": "scalar", "params": []}
        ]
      }
    ]
  },
  "composition_recipe_contract": {"compatibility": [], "recipes": []},
  "explaino_contract": {"entries": [
    {"id": "x", "hypothesis_space": "space", "authority": "owner", "lens": "lens", "invariant": "invariant", "proof": "proof", "fallback": "fail_closed", "product_facing": false, "diagnostic": true}
  ]}
})json";

    const std::string duplicatePath = TempContractPath("ui_salt_contract_duplicate_function.json");
    Check(WriteTextFile(duplicatePath, duplicateFunctionJson),
        "TestMaterializedContractLoaderRejectsTamperedJson_WriteDuplicateFixture");
    MaterializedColorPipelineContract contract;
    std::string error;
    Check(!LoadColorPipelineMaterializedContractJson(duplicatePath, &contract, &error) &&
            error.find("Duplicate materialized function id") != std::string::npos,
        "TestMaterializedContractLoaderRejectsTamperedJson_DuplicateFunctionRejected");
    std::remove(duplicatePath.c_str());

    const char* danglingCompatibilityJson = R"json({
  "schema_version": 1,
  "source_path": "tampered.ui.salt",
  "function_library": {
    "lanes": [
      {
        "id": "source",
        "label": "Source",
        "default": "smooth_escape_ramp",
        "functions": [
          {"id": "smooth_escape_ramp", "label": "Smooth Escape Ramp", "description": "", "runtime_backed": true, "input_kind": "scalar", "output_kind": "scalar", "signal_kind": "scalar", "params": []}
        ]
      },
      {
        "id": "palette",
        "label": "Palette",
        "default": "heatmap",
        "functions": [
          {"id": "heatmap", "label": "Heatmap", "description": "", "runtime_backed": true, "input_kind": "scalar", "output_kind": "rgb", "params": []}
        ]
      },
      {
        "id": "grading",
        "label": "Grading",
        "default": "contrast_lift",
        "functions": [
          {"id": "contrast_lift", "label": "Contrast Lift", "description": "", "runtime_backed": true, "input_kind": "rgb", "output_kind": "rgb", "params": []}
        ]
      }
    ]
  },
  "composition_recipe_contract": {
    "compatibility": [
      {"source": "missing_source", "palette": "heatmap", "signal": "smooth_escape_ramp", "palette_runtime": "heatmap", "grading": "contrast_lift", "mode": "smooth_escape", "reason": "tampered"}
    ],
    "recipes": []
  },
  "explaino_contract": {"entries": [
    {"id": "x", "hypothesis_space": "space", "authority": "owner", "lens": "lens", "invariant": "invariant", "proof": "proof", "fallback": "fail_closed", "product_facing": false, "diagnostic": true}
  ]}
})json";

    const std::string danglingPath = TempContractPath("ui_salt_contract_dangling_compatibility.json");
    Check(WriteTextFile(danglingPath, danglingCompatibilityJson),
        "TestMaterializedContractLoaderRejectsTamperedJson_WriteDanglingFixture");
    error.clear();
    Check(!LoadColorPipelineMaterializedContractJson(danglingPath, &contract, &error) &&
            error.find("Compatibility references missing source function") != std::string::npos,
        "TestMaterializedContractLoaderRejectsTamperedJson_DanglingCompatibilityRejected");
    std::remove(danglingPath.c_str());
}

} // namespace

int main() {
    TestFunctionIdMappingsRoundTrip();
    TestLaneCatalogFiltersRuntimeBackedRows();
    TestSourceSignalKindMetadata();
    TestRowBuildersAndDefaults();
    TestRowFunctionSwitchPreservesSharedParams();
    TestImportAndApplySupportedParams();
    TestSelectionAndScheduleBridgeIds();
    TestSdfSourceRowsAreRuntimeBackedCatalogRows();
    TestMaterializedUiSaltMetadataShadowsCurrentCatalog();
    TestMaterializedUiSaltMetadataCanOwnPublicCatalog();
    TestMaterializedContractLoaderRejectsTamperedJson();

    std::printf("test_color_pipeline_core: passed=%d failed=%d\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
