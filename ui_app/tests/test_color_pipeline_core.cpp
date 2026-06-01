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

bool HasEnumOption(const FunctionDescriptor& function, const char* path, const char* optionId) {
    for (const FunctionParamDescriptor& param : function.parameters) {
        if (param.path != path) continue;
        for (const UISchemaOption& option : param.options) {
            if (option.id == optionId) return true;
        }
    }
    return false;
}

const FunctionDescriptor* FindFunction(const ColorPipelineLaneCatalog& catalog, const char* id) {
    return color_pipeline_core::FindColorPipelineFunctionDescriptor(catalog, id ? id : "");
}

bool FileExists(const char* path) {
    std::ifstream input(path, std::ios::in | std::ios::binary);
    return input.good();
}

const MaterializedSignalType* FindSignalType(
    const MaterializedColorPipelineContract& contract,
    const char* id) {
    for (const MaterializedSignalType& type : contract.signal_types) {
        if (type.id == id) {
            return &type;
        }
    }
    return nullptr;
}

const MaterializedColorPipelineAdapter* FindAdapter(
    const MaterializedColorPipelineContract& contract,
    const char* id) {
    for (const MaterializedColorPipelineAdapter& adapter : contract.adapters) {
        if (adapter.id == id) {
            return &adapter;
        }
    }
    return nullptr;
}

const MaterializedColorPipelineResolutionCase* FindResolutionCase(
    const MaterializedColorPipelineContract& contract,
    const char* id) {
    for (const MaterializedColorPipelineResolutionCase& resolutionCase : contract.resolution_cases) {
        if (resolutionCase.id == id) {
            return &resolutionCase;
        }
    }
    return nullptr;
}

const MaterializedColorPipelineCompatibilityAudit* FindCompatibilityAudit(
    const MaterializedColorPipelineContract& contract,
    const char* source,
    const char* palette,
    const char* grading) {
    for (const MaterializedColorPipelineCompatibilityAudit& audit : contract.compatibility_audit) {
        if (audit.source == source && audit.palette == palette && audit.grading == grading) {
            return &audit;
        }
    }
    return nullptr;
}

const MaterializedColorPipelineCompatOverride* FindCompatOverride(
    const MaterializedColorPipelineContract& contract,
    const char* id) {
    for (const MaterializedColorPipelineCompatOverride& compatOverride : contract.compat_overrides) {
        if (compatOverride.id == id) {
            return &compatOverride;
        }
    }
    return nullptr;
}

const MaterializedColorPipelineSdfSourceCapability* FindSdfSourceCapability(
    const MaterializedColorPipelineContract& contract,
    const char* functionId) {
    for (const MaterializedColorPipelineSdfSourceCapability& capability : contract.sdf_source_capabilities) {
        if (capability.function == functionId) {
            return &capability;
        }
    }
    return nullptr;
}

const MaterializedColorPipelineRecipeV2* FindRecipeV2(
    const MaterializedColorPipelineContract& contract,
    const char* recipeId) {
    for (const MaterializedColorPipelineRecipeV2& recipe : contract.recipe_v2) {
        if (recipe.id == recipeId) {
            return &recipe;
        }
    }
    return nullptr;
}

void CheckMaterializedPort(
    const MaterializedColorPipelineContract& contract,
    const char* laneId,
    const char* functionId,
    std::size_t portIndex,
    std::size_t expectedPortCount,
    const char* direction,
    const char* portId,
    const char* type,
    bool canonical,
    const char* genericGroup) {
    const std::string prefix = std::string("TestMaterializedUiSaltMetadataShadowsCurrentCatalog_Port_") +
        (laneId ? laneId : "") + "_" + (functionId ? functionId : "") + "_" +
        (direction ? direction : "") + "_" + (portId ? portId : "");
    const MaterializedColorPipelineLane* lane = FindMaterializedColorPipelineLane(contract, laneId ? laneId : "");
    Check(lane != nullptr, (prefix + "_LanePresent").c_str());
    if (!lane) return;
    const MaterializedColorPipelineFunction* function = FindMaterializedColorPipelineFunction(*lane, functionId ? functionId : "");
    Check(function != nullptr, (prefix + "_FunctionPresent").c_str());
    if (!function) return;
    Check(function->ports.size() == expectedPortCount, (prefix + "_Count").c_str());
    if (function->ports.size() <= portIndex) {
        Check(false, (prefix + "_IndexPresent").c_str());
        return;
    }
    const MaterializedColorPipelinePort& port = function->ports[portIndex];
    Check(port.direction == (direction ? direction : ""), (prefix + "_Direction").c_str());
    Check(port.id == (portId ? portId : ""), (prefix + "_Id").c_str());
    Check(port.type == (type ? type : ""), (prefix + "_Type").c_str());
    Check(port.canonical == canonical, (prefix + "_Canonical").c_str());
    Check(port.generic_group == (genericGroup ? genericGroup : ""), (prefix + "_GenericGroup").c_str());
}

const char* ExpectedTypedSignalForFunction(const char* functionId) {
    if (!functionId) return "";
    if (std::strcmp(functionId, "smooth_escape_ramp") == 0) return "scalar.unit";
    if (std::strcmp(functionId, "phase_orbit") == 0) return "phase.radians";
    if (std::strcmp(functionId, "banded_signal") == 0) return "scalar.unit";
    if (std::strcmp(functionId, "escape_magnitude") == 0) return "scalar.unit";
    if (std::strcmp(functionId, "orbit_stripe") == 0) return "phase.radians";
    if (std::strcmp(functionId, "root_proximity") == 0) return "scalar.unit";
    if (std::strcmp(functionId, "root_index") == 0) return "category.root_index";
    if (std::strcmp(functionId, "sdf_signed_distance") == 0) return "scalar.sdf_signed_distance";
    if (std::strcmp(functionId, "sdf_inside_outside") == 0) return "category.inside_outside";
    if (std::strcmp(functionId, "sdf_boundary_band") == 0) return "scalar.unit";
    if (std::strcmp(functionId, "sdf_normal_angle") == 0) return "phase.radians";
    if (std::strcmp(functionId, "sdf_curvature") == 0) return "scalar.signed";
    if (std::strcmp(functionId, "lens_field_v2_distance") == 0) return "scalar.sdf_signed_distance";
    return "";
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
    Check(std::string(color_pipeline_core::AdvancedColorSignalFunctionId(ColorSignal::lens_field_v2_distance)) == "lens_field_v2_distance",
        "TestFunctionIdMappingsRoundTrip_LensFieldV2SignalId");
    Check(color_pipeline_core::TryParseAdvancedColorSignalFunctionId("sdf_curvature", &signal) && signal == ColorSignal::sdf_curvature,
        "TestFunctionIdMappingsRoundTrip_SdfSignalParse");
    Check(color_pipeline_core::TryParseAdvancedColorSignalFunctionId("lens_field_v2_distance", &signal) && signal == ColorSignal::lens_field_v2_distance,
        "TestFunctionIdMappingsRoundTrip_LensFieldV2SignalParse");
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
    Check(std::string(color_pipeline_core::AdvancedColorShapeFunctionId(ColorPipelineShape::log_compress)) == "log_compress",
        "TestFunctionIdMappingsRoundTrip_LogCompressShapeId");
    Check(std::string(color_pipeline_core::AdvancedColorShapeFunctionId(ColorPipelineShape::smoothstep_range)) == "smoothstep_range",
        "TestFunctionIdMappingsRoundTrip_SmoothstepRangeShapeId");
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

    Check(source->default_function_id == std::string("smooth_escape_ramp") && source->functions.size() == 13,
        "TestLaneCatalogFiltersRuntimeBackedRows_SourceShape");
    Check(HasFunction(*source, "smooth_escape_ramp") && HasFunction(*source, "phase_orbit") &&
            HasFunction(*source, "banded_signal") && HasFunction(*source, "escape_magnitude") &&
            HasFunction(*source, "orbit_stripe") && HasFunction(*source, "root_proximity") &&
            HasFunction(*source, "root_index") &&
            HasFunction(*source, "sdf_signed_distance") &&
            HasFunction(*source, "sdf_inside_outside") &&
            HasFunction(*source, "sdf_boundary_band") &&
            HasFunction(*source, "sdf_normal_angle") &&
            HasFunction(*source, "sdf_curvature") &&
            HasFunction(*source, "lens_field_v2_distance"),
        "TestLaneCatalogFiltersRuntimeBackedRows_SourceFunctions");
    Check(shape->default_function_id == std::string("identity") && shape->functions.size() == 9 &&
            HasFunction(*shape, "smooth_window") &&
            HasFunction(*shape, "log_compress") &&
            HasFunction(*shape, "smoothstep_range"),
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
            "sdf_curvature",
            "lens_field_v2_distance"}),
        "TestLaneCatalogFiltersRuntimeBackedRows_SourceFunctionOrder");
    Check(CatalogIdsEqual(*shape, {
            "identity",
            "offset_scale",
            "repeat",
            "posterize",
            "mirror_repeat",
            "bias_gain_curve",
            "smooth_window",
            "log_compress",
            "smoothstep_range"}),
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
        ColorSignal::lens_field_v2_distance,
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
    Check(color_pipeline_core::ColorPipelineSourceSignalKindForFunctionId("lens_field_v2_distance") ==
            ColorPipelineSourceSignalKind::scalar,
        "TestSourceSignalKindMetadata_LensFieldV2FunctionIsScalar");
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
    Check(color_pipeline_core::TryBuildColorPipelineSelectionFromLaneIds("lens_field_v2_distance", "heatmap", &selection, &mode) &&
            selection.signal == ColorSignal::lens_field_v2_distance && selection.palette == ColorPalette::cyclic_escape &&
            selection.grading == ColorGradingPreset::escape_default && mode == ColoringMode::smooth_escape,
        "TestSelectionAndScheduleBridgeIds_LensFieldV2HeatmapSelection");
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
    const ColorPipelineSelection lensFieldV2Pipeline = {ColorSignal::lens_field_v2_distance, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
    Check(color_pipeline_core::TryBuildColorPipelineScheduleBridgeIds(lensFieldV2Pipeline, &sourceFunction, &paletteFunction) &&
            std::string(sourceFunction ? sourceFunction : "") == "lens_field_v2_distance" &&
            std::string(paletteFunction ? paletteFunction : "") == "heatmap",
        "TestSelectionAndScheduleBridgeIds_LensFieldV2Bridge");
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
        {"lens_field_v2_distance", 1.0, 0.0},
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
        Check(HasParam(*descriptor, "signal.sdf_gate") &&
                HasParam(*descriptor, "signal.sdf_gate_width_px") &&
                HasParam(*descriptor, "signal.sdf_field_downsample") &&
                HasEnumOption(*descriptor, "signal.sdf_gate", "none") &&
                HasEnumOption(*descriptor, "signal.sdf_gate", "boundary_band") &&
                HasEnumOption(*descriptor, "signal.sdf_gate", "sdf_inside") &&
                HasEnumOption(*descriptor, "signal.sdf_gate", "sdf_outside") &&
                HasEnumOption(*descriptor, "signal.sdf_field_downsample", "0") &&
                HasEnumOption(*descriptor, "signal.sdf_field_downsample", "1") &&
                HasEnumOption(*descriptor, "signal.sdf_field_downsample", "2") &&
                HasEnumOption(*descriptor, "signal.sdf_field_downsample", "4") &&
                HasEnumOption(*descriptor, "signal.sdf_field_downsample", "8") &&
                HasEnumOption(*descriptor, "signal.sdf_field_downsample", "16"),
            "TestSdfSourceRowsAreRuntimeBackedCatalogRows_SdfRowsExposeBoundaryGateAndFieldParams");
        Check((std::string(expected.id) == "sdf_boundary_band") == HasParam(*descriptor, "signal.boundary_width_px"),
            "TestSdfSourceRowsAreRuntimeBackedCatalogRows_BoundaryWidthIsBoundaryBandOnly");
        Check((std::string(expected.id) == "lens_field_v2_distance") == HasParam(*descriptor, "signal.sign_contrast"),
            "TestSdfSourceRowsAreRuntimeBackedCatalogRows_LensFieldV2SignContrastIsLensOnly");

        ColorPipelineRowState row;
        std::string error;
        Check(color_pipeline_core::BuildColorPipelineRowFromFunctionId(*source, expected.id, 41, &row, &error) &&
                row.function_id == expected.id &&
                RowNumber(row, "signal.scale", expected.scale) &&
                RowNumber(row, "signal.bias", expected.bias) &&
                RowNumber(row, "signal.blend_weight", 1.0) &&
                RowEnum(row, "signal.sdf_gate", "none") &&
                RowEnum(row, "signal.sdf_field_downsample", "0") &&
                RowNumber(row, "signal.sdf_gate_width_px", 2.0) &&
                (std::string(expected.id) != "sdf_boundary_band" || RowNumber(row, "signal.boundary_width_px", 2.0)) &&
                (std::string(expected.id) != "lens_field_v2_distance" || RowNumber(row, "signal.sign_contrast", 0.35)),
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
    Check(contract.compatibility.size() == 22,
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_CompatibilityCount");
    Check(contract.compat_overrides.size() == 18,
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_CompatOverrideCount");
    Check(contract.compatibility_audit.size() == 22,
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_CompatibilityAuditCount");
    Check(contract.recipes.size() == 4, "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_RecipeCount");
    Check(contract.has_recipe_v2, "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_RecipeV2Present");
    Check(contract.recipe_v2.size() == 4, "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_RecipeV2Count");
    Check(contract.row_applicators.size() == 4, "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_RowApplicatorCount");
    Check(contract.sdf_source_capabilities.size() == 6,
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_SdfSourceCapabilityCount");
    Check(contract.signal_types.size() == 10, "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_SignalTypeCount");
    Check(contract.adapters.size() == 11, "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_AdapterCount");
    Check(contract.edge_policy.id == "current_linear_color_stack" &&
            contract.edge_policy.max_adapter_hops == 2 &&
            !contract.edge_policy.allow_lossy &&
            contract.edge_policy.allow_visible_default &&
            !contract.edge_policy.allow_explicit &&
            !contract.edge_policy.allow_diagnostic &&
            contract.edge_policy.fail_closed_default,
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_EdgePolicy");
    Check(contract.edge_links.size() == 3, "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_EdgeLinkCount");
    if (contract.edge_links.size() == 3) {
        Check(contract.edge_links[0].id == "source_to_shape" &&
                contract.edge_links[1].id == "shape_to_palette" &&
                contract.edge_links[2].id == "palette_to_grading",
            "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_EdgeLinkOrder");
    }
    Check(contract.resolution_cases.size() == 10,
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_ResolutionCaseCount");
    const MaterializedColorPipelineCompatibilityAudit* smoothAudit =
        FindCompatibilityAudit(contract, "smooth_escape_ramp", "heatmap", "contrast_lift");
    const MaterializedColorPipelineCompatibilityAudit* rootAudit =
        FindCompatibilityAudit(contract, "root_index", "root_classic_palette", "basin_default");
    const MaterializedColorPipelineCompatibilityAudit* sdfAudit =
        FindCompatibilityAudit(contract, "sdf_signed_distance", "heatmap", "contrast_lift");
    const MaterializedColorPipelineCompatibilityAudit* explainoAudit =
        FindCompatibilityAudit(contract, "smooth_escape_ramp", "explaino_cmap", "contrast_lift");
    Check(smoothAudit && smoothAudit->classification == "typed_resolved" &&
            smoothAudit->route_case_id == "smooth_escape_heatmap" &&
            smoothAudit->override_id.empty(),
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_SmoothCompatAuditTypedResolved");
    Check(rootAudit && rootAudit->classification == "typed_resolved" &&
            rootAudit->route_case_id == "root_classic",
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_RootCompatAuditTypedResolved");
    Check(sdfAudit && sdfAudit->classification == "runtime_legacy_override" &&
            sdfAudit->override_id == "legacy_sdf_signed_distance_heatmap_contrast_lift",
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_SdfCompatAuditOverride");
    Check(explainoAudit && explainoAudit->classification == "runtime_legacy_override" &&
            explainoAudit->override_id == "legacy_smooth_escape_ramp_explaino_cmap_contrast_lift",
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_ExplainoCompatAuditOverride");
    const MaterializedColorPipelineCompatOverride* sdfOverride =
        FindCompatOverride(contract, "legacy_sdf_signed_distance_heatmap_contrast_lift");
    Check(sdfOverride && !sdfOverride->owner_seam.empty() &&
            !sdfOverride->reason.empty() &&
            !sdfOverride->proof.empty(),
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_SdfCompatOverrideMetadata");
    const MaterializedColorPipelineResolutionCase* smoothRoute =
        FindResolutionCase(contract, "smooth_escape_heatmap");
    const MaterializedColorPipelineResolutionCase* sdfRoute =
        FindResolutionCase(contract, "sdf_signed_distance_normalized_heatmap");
    const MaterializedColorPipelineResolutionCase* rootBad =
        FindResolutionCase(contract, "root_repeat_heatmap_bad");
    const MaterializedColorPipelineResolutionCase* phaseBad =
        FindResolutionCase(contract, "phase_root_palette_bad");
    Check(smoothRoute && smoothRoute->status == "resolved" &&
            smoothRoute->route_edges.size() == 3 &&
            smoothRoute->chosen_adapters.empty() &&
            smoothRoute->adapter_hops == 0 &&
            smoothRoute->adapter_cost == 0 &&
            smoothRoute->policy_blockers.empty(),
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_SmoothRouteResolvedDirect");
    Check(sdfRoute && sdfRoute->status == "resolved" &&
            sdfRoute->allow_lossy &&
            sdfRoute->explicit_adapter_consent &&
            sdfRoute->chosen_adapters.size() == 1 &&
            sdfRoute->chosen_adapters[0] == "normalize.sdf_signed_distance.unit" &&
            sdfRoute->adapter_hops == 1 &&
            sdfRoute->adapter_cost == 2 &&
            sdfRoute->tie_break_rule == "exact_identity_safe_non_lossy_lower_cost_fewer_hops_declaration_order" &&
            !sdfRoute->route_edges.empty() &&
            !sdfRoute->route_edges[0].adapters.empty() &&
            sdfRoute->route_edges[0].adapter_hops == 1 &&
            sdfRoute->route_edges[0].adapter_cost == 2,
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_SdfRouteRequiresExplicitAdapter");
    Check(rootBad && rootBad->status == "fail_closed" &&
            rootBad->fail_closed_reason.find("root category") != std::string::npos &&
            !rootBad->policy_blockers.empty(),
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_RootBadRouteFailsClosed");
    Check(phaseBad && phaseBad->status == "fail_closed" &&
            phaseBad->fail_closed_reason.find("phase") != std::string::npos &&
            !phaseBad->policy_blockers.empty(),
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_PhaseBadRouteFailsClosed");
    const MaterializedSignalType* scalarSdf = FindSignalType(contract, "scalar.sdf_signed_distance");
    const MaterializedSignalType* fieldSdf = FindSignalType(contract, "field.sdf_signed_distance");
    const MaterializedSignalType* normalPhase = FindSignalType(contract, "phase.radians");
    const MaterializedSignalType* rootCategory = FindSignalType(contract, "category.root_index");
    const MaterializedSignalType* insideOutsideCategory = FindSignalType(contract, "category.inside_outside");
    const MaterializedSignalType* paletteIndex = FindSignalType(contract, "palette.discrete_index");
    Check(scalarSdf && scalarSdf->kind == "scalar" && scalarSdf->domain == "signed_distance",
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_SampledSdfIsScalar");
    Check(fieldSdf && fieldSdf->kind == "field" && fieldSdf->topology == "field",
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_RawSdfIsField");
    Check(normalPhase && normalPhase->kind == "phase" && normalPhase->topology == "circular" && normalPhase->has_period,
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_NormalAngleIsPhase");
    Check(rootCategory && rootCategory->kind == "category" && rootCategory->domain == "root_index",
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_RootIndexIsCategory");
    Check(insideOutsideCategory && insideOutsideCategory->kind == "category" && insideOutsideCategory->domain == "inside_outside",
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_InsideOutsideIsCategory");
    Check(paletteIndex && paletteIndex->kind == "palette" && paletteIndex->domain == "discrete_index",
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_DiscreteIndexIsPaletteDomain");

    const MaterializedColorPipelineAdapter* identityScalar = FindAdapter(contract, "identity.scalar_unit");
    const MaterializedColorPipelineAdapter* normalizeSdf = FindAdapter(contract, "normalize.sdf_signed_distance.unit");
    const MaterializedColorPipelineAdapter* rootPalette = FindAdapter(contract, "root_index.palette_discrete_index");
    const MaterializedColorPipelineAdapter* fieldMask = FindAdapter(contract, "field.sdf_signed_distance.boundary_mask");
    Check(identityScalar && identityScalar->source == "scalar.unit" && identityScalar->target == "scalar.unit" &&
            identityScalar->policy == "safe" && !identityScalar->lossy && identityScalar->reversible &&
            identityScalar->cost == 0,
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_IdentityScalarAdapter");
    Check(normalizeSdf && normalizeSdf->source == "scalar.sdf_signed_distance" &&
            normalizeSdf->target == "scalar.unit" && normalizeSdf->policy == "explicit_only" &&
            normalizeSdf->lossy && !normalizeSdf->fail_closed_reason.empty(),
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_SdfNormalizeAdapter");
    Check(rootPalette && rootPalette->source == "category.root_index" &&
            rootPalette->target == "palette.discrete_index" && rootPalette->policy == "visible_default" &&
            !rootPalette->lossy && !rootPalette->fail_closed_reason.empty(),
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_RootPaletteAdapter");
    Check(fieldMask && fieldMask->source == "field.sdf_signed_distance" &&
            fieldMask->target == "mask.alpha" && fieldMask->policy == "explicit_only" &&
            fieldMask->lossy && !fieldMask->reversible,
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_FieldMaskAdapter");

    CheckMaterializedPort(contract, "source", "smooth_escape_ramp", 0, 1, "output", "signal", "scalar.unit", true, "");
    CheckMaterializedPort(contract, "source", "phase_orbit", 0, 1, "output", "signal", "phase.radians", true, "");
    CheckMaterializedPort(contract, "source", "root_index", 0, 1, "output", "signal", "category.root_index", true, "");
    CheckMaterializedPort(contract, "source", "sdf_signed_distance", 0, 1, "output", "signal", "scalar.sdf_signed_distance", true, "");
    CheckMaterializedPort(contract, "source", "sdf_normal_angle", 0, 1, "output", "signal", "phase.radians", true, "");
    CheckMaterializedPort(contract, "source", "sdf_inside_outside", 0, 1, "output", "signal", "category.inside_outside", true, "");
    CheckMaterializedPort(contract, "shape", "identity", 0, 2, "input", "signal", "generic.T", false, "T");
    CheckMaterializedPort(contract, "shape", "identity", 1, 2, "output", "signal", "generic.T", true, "T");
    CheckMaterializedPort(contract, "shape", "repeat", 0, 2, "input", "signal", "scalar.unit", false, "");
    CheckMaterializedPort(contract, "shape", "repeat", 1, 2, "output", "signal", "scalar.unit", true, "");
    CheckMaterializedPort(contract, "shape", "bias_gain_curve", 0, 2, "input", "signal", "scalar.unit", false, "");
    CheckMaterializedPort(contract, "shape", "bias_gain_curve", 1, 2, "output", "signal", "scalar.unit", true, "");
    CheckMaterializedPort(contract, "shape", "log_compress", 0, 2, "input", "signal", "scalar.unit", false, "");
    CheckMaterializedPort(contract, "shape", "log_compress", 1, 2, "output", "signal", "scalar.unit", true, "");
    CheckMaterializedPort(contract, "shape", "smoothstep_range", 0, 2, "input", "signal", "scalar.unit", false, "");
    CheckMaterializedPort(contract, "shape", "smoothstep_range", 1, 2, "output", "signal", "scalar.unit", true, "");
    CheckMaterializedPort(contract, "palette", "heatmap", 0, 2, "input", "signal", "scalar.unit", false, "");
    CheckMaterializedPort(contract, "palette", "heatmap", 1, 2, "output", "color", "color.linear_rgb", true, "");
    CheckMaterializedPort(contract, "palette", "phase_wheel_palette", 0, 2, "input", "signal", "phase.radians", false, "");
    CheckMaterializedPort(contract, "palette", "phase_wheel_palette", 1, 2, "output", "color", "color.linear_rgb", true, "");
    CheckMaterializedPort(contract, "palette", "root_classic_palette", 0, 2, "input", "signal", "category.root_index", false, "");
    CheckMaterializedPort(contract, "palette", "root_classic_palette", 1, 2, "output", "color", "color.linear_rgb", true, "");
    CheckMaterializedPort(contract, "grading", "contrast_lift", 0, 2, "input", "color", "color.linear_rgb", false, "");
    CheckMaterializedPort(contract, "grading", "contrast_lift", 1, 2, "output", "color", "color.linear_rgb", true, "");
    CheckMaterializedPort(contract, "grading", "phase_finish", 0, 2, "input", "color", "color.linear_rgb", false, "");
    CheckMaterializedPort(contract, "grading", "phase_finish", 1, 2, "output", "color", "color.linear_rgb", true, "");
    CheckMaterializedPort(contract, "grading", "basin_default", 0, 2, "input", "color", "color.linear_rgb", false, "");
    CheckMaterializedPort(contract, "grading", "basin_default", 1, 2, "output", "color", "color.linear_rgb", true, "");
    CheckMaterializedPort(contract, "grading", "neutral_finish", 0, 2, "input", "color", "color.linear_rgb", false, "");
    CheckMaterializedPort(contract, "grading", "neutral_finish", 1, 2, "output", "color", "color.linear_rgb", true, "");

    const std::vector<std::string> expectedApplicatorIds = {
        "none",
        "sdf_boundary_band",
        "sdf_inside",
        "sdf_outside",
    };
    for (std::size_t applicatorIndex = 0;
         applicatorIndex < expectedApplicatorIds.size() && applicatorIndex < contract.row_applicators.size();
         ++applicatorIndex) {
        const MaterializedColorPipelineRowApplicator& applicator = contract.row_applicators[applicatorIndex];
        Check(applicator.id == expectedApplicatorIds[applicatorIndex],
            "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_RowApplicatorOrder");
        Check(applicator.target_lane == "source",
            "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_RowApplicatorTargetLane");
        Check(applicator.required_signal_kind == "any",
            "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_RowApplicatorSignalKind");
        Check(applicator.storage_param == "signal.sdf_gate",
            "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_RowApplicatorStorageParam");
        Check(!applicator.fail_closed_reason.empty(),
            "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_RowApplicatorFailClosedReason");
    }
    if (contract.row_applicators.size() == 4) {
        Check(contract.row_applicators[0].storage_value == "none" &&
                contract.row_applicators[1].storage_value == "boundary_band" &&
                contract.row_applicators[2].storage_value == "sdf_inside" &&
                contract.row_applicators[3].storage_value == "sdf_outside",
            "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_RowApplicatorStorageValues");
    }
    Check(contract.row_applicators.size() < 2 ||
            contract.row_applicators[1].width_param == "signal.sdf_gate_width_px",
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_BoundaryApplicatorWidthParam");
    const MaterializedColorPipelineSdfSourceCapability* sdfSignedCapability =
        FindSdfSourceCapability(contract, "sdf_signed_distance");
    const MaterializedColorPipelineSdfSourceCapability* lensV2Capability =
        FindSdfSourceCapability(contract, "lens_field_v2_distance");
    Check(sdfSignedCapability && sdfSignedCapability->field_source == "lens_sdf" &&
            sdfSignedCapability->requires_sdf_field &&
            sdfSignedCapability->supports_applicators &&
            sdfSignedCapability->supported_applicators.size() == 4 &&
            sdfSignedCapability->gate_param == "signal.sdf_gate" &&
            sdfSignedCapability->gate_width_param == "signal.sdf_gate_width_px" &&
            sdfSignedCapability->sample_step_param == "signal.sdf_sample_step" &&
            sdfSignedCapability->field_downsample_param == "signal.sdf_field_downsample",
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_SdfCapabilityStorageKeys");
    Check(lensV2Capability && lensV2Capability->field_source == "lens_field_v2",
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_LensV2CapabilityFieldSource");
    Check(FindSdfSourceCapability(contract, "smooth_escape_ramp") == nullptr,
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_NonSdfSourceHasNoSdfCapability");
    Check(!contract.explaino_entries.empty(), "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_ExplainoEntriesPresent");

    const ColorPipelineMetadataParityReport parity = ValidateColorPipelineMetadataParity(contract);
    Check(parity.ok && parity.errors.empty(),
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_ReusableParityReportOk");
    Check(parity.lane_count == 4 && parity.function_count == 36 &&
            parity.compatibility_count == 22 && parity.recipe_count == 4 &&
            parity.taxonomy_group_count == 24 && parity.unsupported_pair_count > 0,
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
            Check(actualFunction.taxonomy_group == expectedFunction.taxonomy_group && !actualFunction.taxonomy_group.empty(),
                "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_FunctionTaxonomyGroup");
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
                Check(actualFunction.typed_signal == ExpectedTypedSignalForFunction(expectedFunction.id.c_str()),
                    "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_SourceTypedSignal");
            } else {
                Check(actualFunction.signal_kind.empty(),
                    "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_NonSourceHasNoSignalKind");
                Check(actualFunction.typed_signal.empty(),
                    "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_NonSourceHasNoTypedSignal");
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
        const MaterializedColorPipelineRecipeV2* recipeV2 = FindRecipeV2(contract, recipe.id.c_str());
        Check(recipeV2 != nullptr,
            "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_RecipeV2PresentForRecipe");
        if (recipeV2) {
            Check(recipeV2->source_recipe_id == recipe.id &&
                    recipeV2->label == recipe.label &&
                    recipeV2->ui_projection == "linear_color_stack" &&
                    recipeV2->shadow_only &&
                    recipeV2->live_authority == "recipe" &&
                    recipeV2->status == "resolved" &&
                    recipeV2->fail_closed_reason.empty(),
                "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_RecipeV2ShadowMetadata");
            Check(recipeV2->nodes.size() == 4 && recipeV2->edges.size() == 3,
                "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_RecipeV2LinearProjectionShape");
            if (recipeV2->nodes.size() == 4) {
                Check(recipeV2->nodes[0].id == "source" &&
                        recipeV2->nodes[0].lane == "source" &&
                        recipeV2->nodes[0].function == recipe.source &&
                        recipeV2->nodes[1].id == "shape" &&
                        recipeV2->nodes[1].lane == "shape" &&
                        recipeV2->nodes[1].function == recipe.shape &&
                        recipeV2->nodes[2].id == "palette" &&
                        recipeV2->nodes[2].lane == "palette" &&
                        recipeV2->nodes[2].function == recipe.palette &&
                        recipeV2->nodes[3].id == "grading" &&
                        recipeV2->nodes[3].lane == "grading" &&
                        recipeV2->nodes[3].function == recipe.grading,
                    "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_RecipeV2NodesMatchRecipe");
            }
            if (recipeV2->edges.size() == 3) {
                Check(recipeV2->edges[0].from_node == "source" &&
                        recipeV2->edges[0].to_node == "shape" &&
                        recipeV2->edges[1].from_node == "shape" &&
                        recipeV2->edges[1].to_node == "palette" &&
                        recipeV2->edges[2].from_node == "palette" &&
                        recipeV2->edges[2].to_node == "grading",
                    "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_RecipeV2EdgesAreLinear");
            }
        }
    }

    const MaterializedColorPipelineRecipeV2* defaultRecipeV2 = FindRecipeV2(contract, "default_smooth_escape");
    Check(defaultRecipeV2 && defaultRecipeV2->chosen_adapters.empty() &&
            defaultRecipeV2->adapter_hops == 0 &&
            defaultRecipeV2->adapter_cost == 0 &&
            !defaultRecipeV2->edges.empty() &&
            defaultRecipeV2->edges[0].status == "direct" &&
            defaultRecipeV2->edges[0].adapters.empty(),
        "TestMaterializedUiSaltMetadataShadowsCurrentCatalog_DefaultRecipeV2DirectRoute");

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

void TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup() {
    color_pipeline_core::ClearColorPipelineMetadataCatalogForTests();
    Check(!color_pipeline_core::IsColorPipelineMetadataCompatibilityActive(),
        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_StartsHardcoded");
    Check(!color_pipeline_core::IsColorPipelineCompatibilityDiagnosticsActive(),
        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_StartsDiagnosticsInactive");
    Check(color_pipeline_core::ColorPipelineCompatibilityAuthorityId() == std::string("hardcoded"),
        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_StartsHardcodedAuthority");
    color_pipeline_core::ColorPipelineCompatibilityRouteExplanation inactiveExplanation;
    Check(!color_pipeline_core::TryExplainColorPipelineCompatibilityRoute(
            "smooth_escape_ramp",
            "heatmap",
            "contrast_lift",
            &inactiveExplanation) &&
            !inactiveExplanation.metadata_active &&
            inactiveExplanation.reason.find("not active") != std::string::npos,
        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_DiagnosticsInactiveExplains");
    Check(color_pipeline_core::CountActiveColorPipelineCompatibilityRows() == color_pipeline_core::CountHardcodedColorPipelineCompatibilityRows(),
        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_HardcodedFallbackCountIsTruthful");

    MaterializedColorPipelineContract contract;
    std::string error;
    const std::string path = ResolveMaterializedContractPath();
    Check(LoadColorPipelineMaterializedContractJson(path, &contract, &error),
        (std::string("TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_Loads: ") + error).c_str());
    if (!error.empty() || contract.schema_version != 1) {
        Check(false, "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_ContractLoadOrVersion");
        return;
    }

    Check(color_pipeline_core::TryInstallColorPipelineMetadataCatalog(contract, &error),
        (std::string("TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_Install: ") + error).c_str());
    Check(color_pipeline_core::IsColorPipelineMetadataCompatibilityActive(),
        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_MetadataActive");
    Check(color_pipeline_core::ColorPipelineCompatibilityAuthorityId() == std::string("materialized_json"),
        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_Authority");
    Check(color_pipeline_core::ColorPipelineCompatibilityRuntimeAuthorityIdForLaneIds(
            "smooth_escape_ramp",
            "heatmap") == std::string("typed_resolver_pilot"),
        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_TypedPilotAuthority");
    Check(color_pipeline_core::IsColorPipelineCompatibilityDiagnosticsActive() &&
            color_pipeline_core::ColorPipelineCompatibilityDiagnosticsAuthorityId() == std::string("materialized_json_diagnostic"),
        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_DiagnosticsActive");
    Check(color_pipeline_core::CountActiveColorPipelineCompatibilityRows() == 22,
        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_Count");

    color_pipeline_core::ColorPipelineCompatibilityRouteExplanation smoothExplanation;
    Check(color_pipeline_core::TryExplainColorPipelineCompatibilityRoute(
            "smooth_escape_ramp",
            "heatmap",
            "contrast_lift",
            &smoothExplanation) &&
            smoothExplanation.metadata_active &&
            smoothExplanation.supported &&
            smoothExplanation.classification == "typed_resolved" &&
            smoothExplanation.route_case_id == "smooth_escape_heatmap" &&
            smoothExplanation.override_id.empty(),
        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_DiagnosticsTypedResolved");
    ColorPipelineSelection typedPilotSelection;
    ColoringMode typedPilotMode = ColoringMode::phase;
    ColorPipelineSelection hardcodedPilotSelection;
    ColoringMode hardcodedPilotMode = ColoringMode::phase;
    Check(color_pipeline_core::TryBuildTypedResolverPilotColorPipelineSelectionFromLaneIds(
            "smooth_escape_ramp",
            "heatmap",
            &typedPilotSelection,
            &typedPilotMode) &&
            color_pipeline_core::TryBuildHardcodedColorPipelineSelectionFromLaneIds(
                "smooth_escape_ramp",
                "heatmap",
                &hardcodedPilotSelection,
                &hardcodedPilotMode) &&
            typedPilotSelection.signal == hardcodedPilotSelection.signal &&
            typedPilotSelection.palette == hardcodedPilotSelection.palette &&
            typedPilotSelection.grading == hardcodedPilotSelection.grading &&
            typedPilotMode == hardcodedPilotMode,
        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_TypedPilotPreservesRuntimeTuple");
    color_pipeline_core::SetColorPipelineTypedCompatibilityPilotEnabledForTests(false);
    Check(color_pipeline_core::ColorPipelineCompatibilityRuntimeAuthorityIdForLaneIds(
            "smooth_escape_ramp",
            "heatmap") == std::string("materialized_json"),
        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_TypedPilotKillSwitchFallsBack");
    ColorPipelineSelection fallbackPilotSelection;
    ColoringMode fallbackPilotMode = ColoringMode::phase;
    Check(color_pipeline_core::TryBuildColorPipelineSelectionFromLaneIds(
            "smooth_escape_ramp",
            "heatmap",
            &fallbackPilotSelection,
            &fallbackPilotMode) &&
            fallbackPilotSelection.signal == hardcodedPilotSelection.signal &&
            fallbackPilotSelection.palette == hardcodedPilotSelection.palette &&
            fallbackPilotSelection.grading == hardcodedPilotSelection.grading &&
            fallbackPilotMode == hardcodedPilotMode,
        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_TypedPilotKillSwitchPreservesRuntimeTuple");
    color_pipeline_core::SetColorPipelineTypedCompatibilityPilotEnabledForTests(true);
    Check(color_pipeline_core::ColorPipelineCompatibilityRuntimeAuthorityIdForLaneIds(
            "phase_orbit",
            "phase_wheel_palette") == std::string("materialized_json"),
        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_OnlyPilotRouteSwitches");
    color_pipeline_core::ColorPipelineCompatibilityRouteExplanation sdfExplanation;
    Check(color_pipeline_core::TryExplainColorPipelineCompatibilityRoute(
            "sdf_signed_distance",
            "heatmap",
            "contrast_lift",
            &sdfExplanation) &&
            sdfExplanation.metadata_active &&
            sdfExplanation.supported &&
            sdfExplanation.classification == "runtime_legacy_override" &&
            sdfExplanation.override_id == "legacy_sdf_signed_distance_heatmap_contrast_lift" &&
            sdfExplanation.route_case_id.empty(),
        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_DiagnosticsCompatOverride");
    color_pipeline_core::ColorPipelineCompatibilityRouteExplanation unsupportedExplanation;
    Check(color_pipeline_core::TryExplainColorPipelineCompatibilityRoute(
            "phase_orbit",
            "heatmap",
            "contrast_lift",
            &unsupportedExplanation) &&
            unsupportedExplanation.metadata_active &&
            !unsupportedExplanation.supported &&
            unsupportedExplanation.classification == "unsupported" &&
            unsupportedExplanation.reason.find("no materialized compatibility audit row") != std::string::npos,
        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_DiagnosticsUnsupported");

    ColorPipelineSelection unsupportedLiveSelection;
    ColoringMode unsupportedLiveMode = ColoringMode::smooth_escape;
    Check(!color_pipeline_core::TryBuildColorPipelineSelectionFromLaneIds(
            "phase_orbit",
            "heatmap",
            &unsupportedLiveSelection,
            &unsupportedLiveMode),
        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_DiagnosticsDoesNotSwitchLiveBehavior");

    const ColorPipelineLaneCatalog* sourceLane = color_pipeline_core::FindColorPipelineLaneCatalog("source");
    const ColorPipelineLaneCatalog* paletteLane = color_pipeline_core::FindColorPipelineLaneCatalog("palette");
    Check(sourceLane != nullptr && paletteLane != nullptr,
        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_SourcePaletteCatalogsPresent");
    if (sourceLane && paletteLane) {
        for (const FunctionDescriptor& sourceFunction : sourceLane->functions) {
            for (const FunctionDescriptor& paletteFunction : paletteLane->functions) {
                ColorPipelineSelection expectedSelection;
                ColoringMode expectedMode = ColoringMode::smooth_escape;
                const bool expectedSupported = color_pipeline_core::TryBuildHardcodedColorPipelineSelectionFromLaneIds(
                    sourceFunction.id.c_str(),
                    paletteFunction.id.c_str(),
                    &expectedSelection,
                    &expectedMode);
                ColorPipelineSelection actualSelection;
                ColoringMode actualMode = ColoringMode::smooth_escape;
                const bool actualSupported = color_pipeline_core::TryBuildColorPipelineSelectionFromLaneIds(
                    sourceFunction.id.c_str(),
                    paletteFunction.id.c_str(),
                    &actualSelection,
                    &actualMode);
                Check(actualSupported == expectedSupported,
                    "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_AllowDeny");
                if (actualSupported && expectedSupported) {
                    Check(actualSelection.signal == expectedSelection.signal,
                        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_Signal");
                    Check(actualSelection.palette == expectedSelection.palette,
                        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_Palette");
                    Check(actualSelection.grading == expectedSelection.grading,
                        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_Grading");
                    Check(actualMode == expectedMode,
                        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_Mode");
                }
            }
        }
    }

    ColorPipelineSelection rejectedSelection;
    ColoringMode rejectedMode = ColoringMode::smooth_escape;
    Check(!color_pipeline_core::TryBuildColorPipelineSelectionFromLaneIds(
            "sdf_normal_angle",
            "heatmap",
            &rejectedSelection,
            &rejectedMode),
        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_PhaseSdfHeatmapStillDenied");
    Check(!color_pipeline_core::TryBuildColorPipelineSelectionFromLaneIds(
            "sdf_signed_distance",
            "phase_wheel_palette",
            &rejectedSelection,
            &rejectedMode),
        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_ScalarSdfPhaseWheelStillDenied");

    color_pipeline_core::ClearColorPipelineMetadataCatalogForTests();
    Check(!color_pipeline_core::IsColorPipelineMetadataCompatibilityActive(),
        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_ClearRestoresHardcoded");
    Check(!color_pipeline_core::IsColorPipelineCompatibilityDiagnosticsActive(),
        "TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup_ClearRestoresDiagnosticsInactive");
}


void TestMaterializedUiSaltMetadataCanOwnCompanionSuggestions() {
    color_pipeline_core::ClearColorPipelineMetadataCatalogForTests();
    Check(!color_pipeline_core::IsColorPipelineMetadataCompanionSuggestionActive(),
        "TestMaterializedUiSaltMetadataCanOwnCompanionSuggestions_StartsHardcoded");
    Check(color_pipeline_core::ColorPipelineCompanionSuggestionAuthorityId() == std::string("hardcoded"),
        "TestMaterializedUiSaltMetadataCanOwnCompanionSuggestions_StartsHardcodedAuthority");
    Check(color_pipeline_core::CountActiveColorPipelineCompanionSuggestions() == color_pipeline_core::CountHardcodedColorPipelineCompanionSuggestions(),
        "TestMaterializedUiSaltMetadataCanOwnCompanionSuggestions_HardcodedFallbackCountIsTruthful");

    std::string companionLane;
    std::string companionFunction;
    Check(color_pipeline_core::TrySuggestColorPipelineCompanionFunction("source", "sdf_normal_angle", &companionLane, &companionFunction) &&
            companionLane == "palette" && companionFunction == "phase_wheel_palette",
        "TestMaterializedUiSaltMetadataCanOwnCompanionSuggestions_HardcodedPhaseSdfSuggestion");
    Check(color_pipeline_core::TrySuggestColorPipelineCompanionFunction("palette", "root_classic_palette", &companionLane, &companionFunction) &&
            companionLane == "source" && companionFunction == "root_index",
        "TestMaterializedUiSaltMetadataCanOwnCompanionSuggestions_HardcodedRootPaletteSuggestion");

    MaterializedColorPipelineContract contract;
    std::string error;
    const std::string path = ResolveMaterializedContractPath();
    Check(LoadColorPipelineMaterializedContractJson(path, &contract, &error),
        (std::string("TestMaterializedUiSaltMetadataCanOwnCompanionSuggestions_Loads: ") + error).c_str());
    if (!error.empty() || contract.schema_version != 1) {
        Check(false, "TestMaterializedUiSaltMetadataCanOwnCompanionSuggestions_ContractLoadOrVersion");
        return;
    }

    Check(color_pipeline_core::TryInstallColorPipelineMetadataCatalog(contract, &error),
        (std::string("TestMaterializedUiSaltMetadataCanOwnCompanionSuggestions_Install: ") + error).c_str());
    Check(color_pipeline_core::IsColorPipelineMetadataCompanionSuggestionActive(),
        "TestMaterializedUiSaltMetadataCanOwnCompanionSuggestions_MetadataActive");
    Check(color_pipeline_core::ColorPipelineCompanionSuggestionAuthorityId() == std::string("materialized_json"),
        "TestMaterializedUiSaltMetadataCanOwnCompanionSuggestions_Authority");
    Check(color_pipeline_core::CountActiveColorPipelineCompanionSuggestions() == color_pipeline_core::CountHardcodedColorPipelineCompanionSuggestions(),
        "TestMaterializedUiSaltMetadataCanOwnCompanionSuggestions_Count");

    const ColorPipelineLaneCatalog* sourceLane = color_pipeline_core::FindColorPipelineLaneCatalog("source");
    const ColorPipelineLaneCatalog* paletteLane = color_pipeline_core::FindColorPipelineLaneCatalog("palette");
    Check(sourceLane != nullptr && paletteLane != nullptr,
        "TestMaterializedUiSaltMetadataCanOwnCompanionSuggestions_SourcePaletteCatalogsPresent");
    if (sourceLane && paletteLane) {
        for (const FunctionDescriptor& sourceFunction : sourceLane->functions) {
            std::string expectedLane;
            std::string expectedFunction;
            const bool expectedSupported = color_pipeline_core::TrySuggestHardcodedColorPipelineCompanionFunction(
                "source",
                sourceFunction.id.c_str(),
                &expectedLane,
                &expectedFunction);
            std::string actualLane;
            std::string actualFunction;
            const bool actualSupported = color_pipeline_core::TrySuggestColorPipelineCompanionFunction(
                "source",
                sourceFunction.id.c_str(),
                &actualLane,
                &actualFunction);
            Check(actualSupported == expectedSupported,
                "TestMaterializedUiSaltMetadataCanOwnCompanionSuggestions_SourceAllowDeny");
            if (actualSupported && expectedSupported) {
                Check(actualLane == expectedLane && actualFunction == expectedFunction,
                    "TestMaterializedUiSaltMetadataCanOwnCompanionSuggestions_SourceTuple");
            }
        }
        for (const FunctionDescriptor& paletteFunction : paletteLane->functions) {
            std::string expectedLane;
            std::string expectedFunction;
            const bool expectedSupported = color_pipeline_core::TrySuggestHardcodedColorPipelineCompanionFunction(
                "palette",
                paletteFunction.id.c_str(),
                &expectedLane,
                &expectedFunction);
            std::string actualLane;
            std::string actualFunction;
            const bool actualSupported = color_pipeline_core::TrySuggestColorPipelineCompanionFunction(
                "palette",
                paletteFunction.id.c_str(),
                &actualLane,
                &actualFunction);
            Check(actualSupported == expectedSupported,
                "TestMaterializedUiSaltMetadataCanOwnCompanionSuggestions_PaletteAllowDeny");
            if (actualSupported && expectedSupported) {
                Check(actualLane == expectedLane && actualFunction == expectedFunction,
                    "TestMaterializedUiSaltMetadataCanOwnCompanionSuggestions_PaletteTuple");
            }
        }
    }

    Check(!color_pipeline_core::TrySuggestColorPipelineCompanionFunction("shape", "identity", &companionLane, &companionFunction),
        "TestMaterializedUiSaltMetadataCanOwnCompanionSuggestions_NonSourcePaletteDenied");

    color_pipeline_core::ClearColorPipelineMetadataCatalogForTests();
    Check(!color_pipeline_core::IsColorPipelineMetadataCompanionSuggestionActive(),
        "TestMaterializedUiSaltMetadataCanOwnCompanionSuggestions_ClearRestoresHardcoded");
}


void TestMaterializedUiSaltMetadataCanOwnRecipeExpansion() {
    color_pipeline_core::ClearColorPipelineMetadataCatalogForTests();
    Check(!color_pipeline_core::IsColorPipelineMetadataRecipeExpansionActive(),
        "TestMaterializedUiSaltMetadataCanOwnRecipeExpansion_StartsHardcoded");
    Check(color_pipeline_core::ColorPipelineRecipeExpansionAuthorityId() == std::string("hardcoded"),
        "TestMaterializedUiSaltMetadataCanOwnRecipeExpansion_StartsHardcodedAuthority");
    Check(color_pipeline_core::CountHardcodedColorPipelineRecipes() == 4,
        "TestMaterializedUiSaltMetadataCanOwnRecipeExpansion_HardcodedRecipeCount");
    Check(color_pipeline_core::CountActiveColorPipelineRecipes() == color_pipeline_core::CountHardcodedColorPipelineRecipes(),
        "TestMaterializedUiSaltMetadataCanOwnRecipeExpansion_HardcodedFallbackCountIsTruthful");

    const std::vector<MaterializedColorPipelineRecipe>& hardcodedRecipes =
        color_pipeline_core::GetHardcodedColorPipelineRecipes();
    Check(hardcodedRecipes.size() == 4,
        "TestMaterializedUiSaltMetadataCanOwnRecipeExpansion_HardcodedRecipeVectorCount");
    Check(color_pipeline_core::FindHardcodedColorPipelineRecipe("sdf_normal_angle_beauty") != nullptr,
        "TestMaterializedUiSaltMetadataCanOwnRecipeExpansion_HardcodedBeautyRecipeExists");
    for (const MaterializedColorPipelineRecipe& expectedRecipe : hardcodedRecipes) {
        const MaterializedColorPipelineRecipe* activeRecipe =
            color_pipeline_core::FindActiveColorPipelineRecipe(expectedRecipe.id);
        Check(activeRecipe != nullptr,
            "TestMaterializedUiSaltMetadataCanOwnRecipeExpansion_HardcodedRecipeFindsActive");
        if (activeRecipe) {
            Check(activeRecipe->source == expectedRecipe.source &&
                    activeRecipe->shape == expectedRecipe.shape &&
                    activeRecipe->palette == expectedRecipe.palette &&
                    activeRecipe->grading == expectedRecipe.grading,
                "TestMaterializedUiSaltMetadataCanOwnRecipeExpansion_HardcodedRecipeTuple");
        }
    }

    MaterializedColorPipelineContract contract;
    std::string error;
    const std::string path = ResolveMaterializedContractPath();
    Check(LoadColorPipelineMaterializedContractJson(path, &contract, &error),
        (std::string("TestMaterializedUiSaltMetadataCanOwnRecipeExpansion_Loads: ") + error).c_str());
    if (!error.empty() || contract.schema_version != 1) {
        Check(false, "TestMaterializedUiSaltMetadataCanOwnRecipeExpansion_ContractLoadOrVersion");
        return;
    }

    Check(color_pipeline_core::TryInstallColorPipelineMetadataCatalog(contract, &error),
        (std::string("TestMaterializedUiSaltMetadataCanOwnRecipeExpansion_Install: ") + error).c_str());
    Check(color_pipeline_core::IsColorPipelineMetadataRecipeExpansionActive(),
        "TestMaterializedUiSaltMetadataCanOwnRecipeExpansion_MetadataActive");
    Check(color_pipeline_core::ColorPipelineRecipeExpansionAuthorityId() == std::string("materialized_json"),
        "TestMaterializedUiSaltMetadataCanOwnRecipeExpansion_Authority");
    Check(color_pipeline_core::CountActiveColorPipelineRecipes() == color_pipeline_core::CountHardcodedColorPipelineRecipes(),
        "TestMaterializedUiSaltMetadataCanOwnRecipeExpansion_Count");

    for (const MaterializedColorPipelineRecipe& expectedRecipe : hardcodedRecipes) {
        const MaterializedColorPipelineRecipe* actualRecipe =
            color_pipeline_core::FindActiveColorPipelineRecipe(expectedRecipe.id);
        Check(actualRecipe != nullptr,
            "TestMaterializedUiSaltMetadataCanOwnRecipeExpansion_MetadataRecipeFindsActive");
        if (!actualRecipe) {
            continue;
        }
        Check(actualRecipe->label == expectedRecipe.label &&
                actualRecipe->source == expectedRecipe.source &&
                actualRecipe->shape == expectedRecipe.shape &&
                actualRecipe->palette == expectedRecipe.palette &&
                actualRecipe->grading == expectedRecipe.grading,
            "TestMaterializedUiSaltMetadataCanOwnRecipeExpansion_MetadataRecipeTuple");

        std::vector<ColorPipelineLaneState> recipeLanes;
        error.clear();
        Check(color_pipeline_core::TryBuildColorPipelineRecipeLanes(
                actualRecipe->id,
                &recipeLanes,
                &error),
            (std::string("TestMaterializedUiSaltMetadataCanOwnRecipeExpansion_BuildRecipeLanes: ") + error).c_str());
        Check(recipeLanes.size() == 4,
            "TestMaterializedUiSaltMetadataCanOwnRecipeExpansion_LaneCount");
        if (recipeLanes.size() == 4) {
            Check(recipeLanes[0].lane_id == "source" && recipeLanes[0].rows.size() == 1 &&
                    recipeLanes[0].rows[0].function_id == expectedRecipe.source,
                "TestMaterializedUiSaltMetadataCanOwnRecipeExpansion_SourceLane");
            Check(recipeLanes[1].lane_id == "shape" && recipeLanes[1].rows.size() == 1 &&
                    recipeLanes[1].rows[0].function_id == expectedRecipe.shape,
                "TestMaterializedUiSaltMetadataCanOwnRecipeExpansion_ShapeLane");
            Check(recipeLanes[2].lane_id == "palette" && recipeLanes[2].rows.size() == 1 &&
                    recipeLanes[2].rows[0].function_id == expectedRecipe.palette,
                "TestMaterializedUiSaltMetadataCanOwnRecipeExpansion_PaletteLane");
            Check(recipeLanes[3].lane_id == "grading" && recipeLanes[3].rows.size() == 1 &&
                    recipeLanes[3].rows[0].function_id == expectedRecipe.grading,
                "TestMaterializedUiSaltMetadataCanOwnRecipeExpansion_GradingLane");
        }
    }

    std::vector<ColorPipelineLaneState> rejectedLanes;
    error.clear();
    Check(!color_pipeline_core::TryBuildColorPipelineRecipeLanes("missing_recipe", &rejectedLanes, &error) &&
            rejectedLanes.empty() && error.find("Unknown Color Pipeline recipe") != std::string::npos,
        "TestMaterializedUiSaltMetadataCanOwnRecipeExpansion_UnknownRecipeFailsClosed");

    color_pipeline_core::ClearColorPipelineMetadataCatalogForTests();
    Check(!color_pipeline_core::IsColorPipelineMetadataRecipeExpansionActive(),
        "TestMaterializedUiSaltMetadataCanOwnRecipeExpansion_ClearRestoresHardcoded");
}

void TestMaterializedContractLoaderRejectsTamperedJson() {
    auto CheckTamperedAdapterJson = [](const char* adapterJson, const char* fileName, const char* expectedError, const char* checkName) {
        const std::string json = std::string(R"json({
  "schema_version": 1,
  "source_path": "tampered.ui.salt",
  "signal_type_registry": {"types": [
    {"id": "scalar.unit", "kind": "scalar", "domain": "unit", "topology": "linear", "arity": 1, "default_adapter_policy": "safe"},
    {"id": "scalar.signed", "kind": "scalar", "domain": "signed", "topology": "linear", "arity": 1, "default_adapter_policy": "explicit_only"},
    {"id": "scalar.sdf_signed_distance", "kind": "scalar", "domain": "signed_distance", "topology": "linear", "arity": 1, "default_adapter_policy": "explicit_only"},
    {"id": "category.root_index", "kind": "category", "domain": "root_index", "topology": "discrete", "arity": 1, "default_adapter_policy": "forbidden"},
    {"id": "palette.discrete_index", "kind": "palette", "domain": "discrete_index", "topology": "discrete", "arity": 1, "default_adapter_policy": "explicit_only"}
  ]},
  "adapter_library_contract": {"adapters": [
)json") + adapterJson + R"json(
  ]},
  "function_library": {
    "lanes": [
      {
        "id": "source",
        "label": "Source",
        "default": "smooth_escape_ramp",
        "functions": [
          {"id": "smooth_escape_ramp", "label": "Smooth Escape Ramp", "description": "", "taxonomy_group": "escape", "runtime_backed": true, "input_kind": "scalar", "output_kind": "scalar", "signal_kind": "scalar", "typed_signal": "scalar.unit", "params": []}
        ]
      }
    ]
  },
  "composition_recipe_contract": {"compatibility": [], "row_applicators": [{"id": "none", "label": "None", "target_lane": "source", "required_signal_kind": "any", "requires_sdf_field": false, "storage_param": "signal.sdf_gate", "width_param": "", "fail_closed_reason": "ungated"}], "recipes": []},
  "explaino_contract": {"entries": [
    {"id": "x", "hypothesis_space": "space", "authority": "owner", "lens": "lens", "invariant": "invariant", "proof": "proof", "fallback": "fail_closed", "product_facing": false, "diagnostic": true}
  ]}
})json";
        const std::string path = TempContractPath(fileName);
        Check(WriteTextFile(path, json.c_str()), (std::string(checkName) + "_WriteFixture").c_str());
        MaterializedColorPipelineContract contract;
        std::string error;
        Check(!LoadColorPipelineMaterializedContractJson(path, &contract, &error) &&
                error.find(expectedError) != std::string::npos,
            checkName);
        std::remove(path.c_str());
    };

    const char* duplicateSignalTypeJson = R"json({
  "schema_version": 1,
  "source_path": "tampered.ui.salt",
  "signal_type_registry": {"types": [
    {"id": "scalar.unit", "kind": "scalar", "domain": "unit", "topology": "linear", "arity": 1, "default_adapter_policy": "safe"},
    {"id": "scalar.unit", "kind": "scalar", "domain": "unit", "topology": "linear", "arity": 1, "default_adapter_policy": "safe"}
  ]},
  "function_library": {
    "lanes": [
      {
        "id": "source",
        "label": "Source",
        "default": "smooth_escape_ramp",
        "functions": [
          {"id": "smooth_escape_ramp", "label": "Smooth Escape Ramp", "description": "", "taxonomy_group": "escape", "runtime_backed": true, "input_kind": "scalar", "output_kind": "scalar", "signal_kind": "scalar", "typed_signal": "scalar.unit", "params": []}
        ]
      }
    ]
  },
  "composition_recipe_contract": {"compatibility": [], "row_applicators": [{"id": "none", "label": "None", "target_lane": "source", "required_signal_kind": "any", "requires_sdf_field": false, "storage_param": "signal.sdf_gate", "width_param": "", "fail_closed_reason": "ungated"}], "recipes": []},
  "explaino_contract": {"entries": [
    {"id": "x", "hypothesis_space": "space", "authority": "owner", "lens": "lens", "invariant": "invariant", "proof": "proof", "fallback": "fail_closed", "product_facing": false, "diagnostic": true}
  ]}
})json";

    const std::string duplicateSignalTypePath = TempContractPath("ui_salt_contract_duplicate_signal_type.json");
    Check(WriteTextFile(duplicateSignalTypePath, duplicateSignalTypeJson),
        "TestMaterializedContractLoaderRejectsTamperedJson_WriteDuplicateSignalTypeFixture");
    MaterializedColorPipelineContract contract;
    std::string error;
    Check(!LoadColorPipelineMaterializedContractJson(duplicateSignalTypePath, &contract, &error) &&
            error.find("Duplicate materialized signal type id") != std::string::npos,
        "TestMaterializedContractLoaderRejectsTamperedJson_DuplicateSignalTypeRejected");
    std::remove(duplicateSignalTypePath.c_str());

    const char* unknownTypedSignalJson = R"json({
  "schema_version": 1,
  "source_path": "tampered.ui.salt",
  "signal_type_registry": {"types": [
    {"id": "scalar.unit", "kind": "scalar", "domain": "unit", "topology": "linear", "arity": 1, "default_adapter_policy": "safe"}
  ]},
  "function_library": {
    "lanes": [
      {
        "id": "source",
        "label": "Source",
        "default": "smooth_escape_ramp",
        "functions": [
          {"id": "smooth_escape_ramp", "label": "Smooth Escape Ramp", "description": "", "taxonomy_group": "escape", "runtime_backed": true, "input_kind": "scalar", "output_kind": "scalar", "signal_kind": "scalar", "typed_signal": "scalar.missing", "params": []}
        ]
      }
    ]
  },
  "composition_recipe_contract": {"compatibility": [], "row_applicators": [{"id": "none", "label": "None", "target_lane": "source", "required_signal_kind": "any", "requires_sdf_field": false, "storage_param": "signal.sdf_gate", "width_param": "", "fail_closed_reason": "ungated"}], "recipes": []},
  "explaino_contract": {"entries": [
    {"id": "x", "hypothesis_space": "space", "authority": "owner", "lens": "lens", "invariant": "invariant", "proof": "proof", "fallback": "fail_closed", "product_facing": false, "diagnostic": true}
  ]}
})json";

    const std::string unknownTypedSignalPath = TempContractPath("ui_salt_contract_unknown_typed_signal.json");
    Check(WriteTextFile(unknownTypedSignalPath, unknownTypedSignalJson),
        "TestMaterializedContractLoaderRejectsTamperedJson_WriteUnknownTypedSignalFixture");
    error.clear();
    Check(!LoadColorPipelineMaterializedContractJson(unknownTypedSignalPath, &contract, &error) &&
            error.find("typed_signal references unknown signal type") != std::string::npos,
        "TestMaterializedContractLoaderRejectsTamperedJson_UnknownTypedSignalRejected");
    std::remove(unknownTypedSignalPath.c_str());

    const char* partialRecipeV2Json = R"json({
  "schema_version": 1,
  "source_path": "tampered.ui.salt",
  "signal_type_registry": {"types": [
    {"id": "scalar.unit", "kind": "scalar", "domain": "unit", "topology": "linear", "arity": 1, "default_adapter_policy": "safe"},
    {"id": "color.linear_rgb", "kind": "color", "domain": "linear_rgb", "topology": "color", "arity": 3, "default_adapter_policy": "forbidden"}
  ]},
  "function_library": {
    "lanes": [
      {"id": "source", "label": "Source", "default": "smooth_escape_ramp", "functions": [{"id": "smooth_escape_ramp", "label": "Smooth Escape Ramp", "description": "", "taxonomy_group": "escape", "runtime_backed": true, "input_kind": "scalar", "output_kind": "scalar", "params": []}]},
      {"id": "shape", "label": "Shape", "default": "identity", "functions": [{"id": "identity", "label": "Identity", "description": "", "taxonomy_group": "identity", "runtime_backed": true, "input_kind": "scalar", "output_kind": "scalar", "params": []}]},
      {"id": "palette", "label": "Palette", "default": "heatmap", "functions": [{"id": "heatmap", "label": "Heatmap", "description": "", "taxonomy_group": "palette_escape", "runtime_backed": true, "input_kind": "scalar", "output_kind": "color", "params": []}]},
      {"id": "grading", "label": "Grading", "default": "contrast_lift", "functions": [{"id": "contrast_lift", "label": "Contrast Lift", "description": "", "taxonomy_group": "grade_escape", "runtime_backed": true, "input_kind": "color", "output_kind": "color", "params": []}]}
    ]
  },
  "composition_recipe_contract": {
    "compatibility": [],
    "row_applicators": [{"id": "none", "label": "None", "target_lane": "source", "required_signal_kind": "any", "requires_sdf_field": false, "storage_param": "signal.sdf_gate", "width_param": "", "fail_closed_reason": "ungated"}],
    "recipes": [{"id": "default_smooth_escape", "label": "Default Smooth Escape", "source": "smooth_escape_ramp", "shape": "identity", "palette": "heatmap", "grading": "contrast_lift"}],
    "recipe_v2": []
  },
  "explaino_contract": {"entries": [
    {"id": "x", "hypothesis_space": "space", "authority": "owner", "lens": "lens", "invariant": "invariant", "proof": "proof", "fallback": "fail_closed", "product_facing": false, "diagnostic": true}
  ]}
})json";

    const std::string partialRecipeV2Path = TempContractPath("ui_salt_contract_partial_recipe_v2.json");
    Check(WriteTextFile(partialRecipeV2Path, partialRecipeV2Json),
        "TestMaterializedContractLoaderRejectsTamperedJson_WritePartialRecipeV2Fixture");
    error.clear();
    Check(!LoadColorPipelineMaterializedContractJson(partialRecipeV2Path, &contract, &error) &&
            error.find("recipe_v2 must mirror every materialized recipe") != std::string::npos,
        "TestMaterializedContractLoaderRejectsTamperedJson_PartialRecipeV2Rejected");
    std::remove(partialRecipeV2Path.c_str());

    const char* duplicateEdgeLinkJson = R"json({
  "schema_version": 1,
  "source_path": "tampered.ui.salt",
  "signal_type_registry": {"types": [
    {"id": "scalar.unit", "kind": "scalar", "domain": "unit", "topology": "linear", "arity": 1, "default_adapter_policy": "safe"},
    {"id": "color.linear_rgb", "kind": "color", "domain": "linear_rgb", "topology": "color", "arity": 3, "default_adapter_policy": "forbidden"}
  ]},
  "edge_resolution_contract": {
    "policy": {"id": "current_linear_color_stack", "max_adapter_hops": 2, "allow_lossy": false, "allow_visible_default": true, "allow_explicit": false, "allow_diagnostic": false, "fail_closed_default": true},
    "edges": [
      {"id": "source_to_shape", "from_lane": "source", "to_lane": "shape", "from_port": "signal", "to_port": "signal", "fail_closed_reason": "source output cannot feed selected shape"},
      {"id": "source_to_shape", "from_lane": "shape", "to_lane": "palette", "from_port": "signal", "to_port": "signal", "fail_closed_reason": "shape output cannot feed selected palette"}
    ]
  },
  "color_pipeline_resolution_audit": {"cases": [
    {"id": "known_bad", "source": "smooth_escape_ramp", "shape": "identity", "palette": "heatmap", "grading": "contrast_lift", "expected_status": "fail_closed", "status": "fail_closed", "allow_lossy": false, "allow_visible_default": false, "explicit_adapter_consent": false, "diagnostic_adapter_consent": false, "chosen_adapters": [], "adapter_hops": 0, "adapter_cost": 0, "tie_break_rule": "exact_identity_safe_non_lossy_lower_cost_fewer_hops_declaration_order", "policy_blockers": ["expected failure"], "route_edges": [], "fail_closed_reason": "expected failure"}
  ]},
  "function_library": {
    "lanes": [
      {"id": "source", "label": "Source", "default": "smooth_escape_ramp", "functions": [{"id": "smooth_escape_ramp", "label": "Smooth Escape Ramp", "description": "", "taxonomy_group": "escape", "runtime_backed": true, "input_kind": "scalar", "output_kind": "scalar", "params": []}]},
      {"id": "shape", "label": "Shape", "default": "identity", "functions": [{"id": "identity", "label": "Identity", "description": "", "taxonomy_group": "identity", "runtime_backed": true, "input_kind": "scalar", "output_kind": "scalar", "params": []}]},
      {"id": "palette", "label": "Palette", "default": "heatmap", "functions": [{"id": "heatmap", "label": "Heatmap", "description": "", "taxonomy_group": "palette_escape", "runtime_backed": true, "input_kind": "scalar", "output_kind": "color", "params": []}]},
      {"id": "grading", "label": "Grading", "default": "contrast_lift", "functions": [{"id": "contrast_lift", "label": "Contrast Lift", "description": "", "taxonomy_group": "grade_escape", "runtime_backed": true, "input_kind": "color", "output_kind": "color", "params": []}]}
    ]
  },
  "composition_recipe_contract": {"compatibility": [], "row_applicators": [{"id": "none", "label": "None", "target_lane": "source", "required_signal_kind": "any", "requires_sdf_field": false, "storage_param": "signal.sdf_gate", "width_param": "", "fail_closed_reason": "ungated"}], "recipes": []},
  "explaino_contract": {"entries": [
    {"id": "x", "hypothesis_space": "space", "authority": "owner", "lens": "lens", "invariant": "invariant", "proof": "proof", "fallback": "fail_closed", "product_facing": false, "diagnostic": true}
  ]}
})json";

    const std::string duplicateEdgeLinkPath = TempContractPath("ui_salt_contract_duplicate_edge_link.json");
    Check(WriteTextFile(duplicateEdgeLinkPath, duplicateEdgeLinkJson),
        "TestMaterializedContractLoaderRejectsTamperedJson_WriteDuplicateEdgeLinkFixture");
    error.clear();
    Check(!LoadColorPipelineMaterializedContractJson(duplicateEdgeLinkPath, &contract, &error) &&
            error.find("Duplicate materialized edge link id") != std::string::npos,
        "TestMaterializedContractLoaderRejectsTamperedJson_DuplicateEdgeLinkRejected");
    std::remove(duplicateEdgeLinkPath.c_str());

    const char* unknownPortTypeJson = R"json({
  "schema_version": 1,
  "source_path": "tampered.ui.salt",
  "signal_type_registry": {"types": [
    {"id": "scalar.unit", "kind": "scalar", "domain": "unit", "topology": "linear", "arity": 1, "default_adapter_policy": "safe"}
  ]},
  "function_library": {
    "lanes": [
      {
        "id": "source",
        "label": "Source",
        "default": "smooth_escape_ramp",
        "functions": [
          {"id": "smooth_escape_ramp", "label": "Smooth Escape Ramp", "description": "", "taxonomy_group": "escape", "runtime_backed": true, "input_kind": "scalar", "output_kind": "scalar", "signal_kind": "scalar", "typed_signal": "scalar.unit", "params": [], "ports": [{"direction": "output", "id": "signal", "type": "scalar.missing", "canonical": true}]}
        ]
      }
    ]
  },
  "composition_recipe_contract": {"compatibility": [], "row_applicators": [{"id": "none", "label": "None", "target_lane": "source", "required_signal_kind": "any", "requires_sdf_field": false, "storage_param": "signal.sdf_gate", "width_param": "", "fail_closed_reason": "ungated"}], "recipes": []},
  "explaino_contract": {"entries": [
    {"id": "x", "hypothesis_space": "space", "authority": "owner", "lens": "lens", "invariant": "invariant", "proof": "proof", "fallback": "fail_closed", "product_facing": false, "diagnostic": true}
  ]}
})json";

    const std::string unknownPortTypePath = TempContractPath("ui_salt_contract_unknown_port_type.json");
    Check(WriteTextFile(unknownPortTypePath, unknownPortTypeJson),
        "TestMaterializedContractLoaderRejectsTamperedJson_WriteUnknownPortTypeFixture");
    error.clear();
    Check(!LoadColorPipelineMaterializedContractJson(unknownPortTypePath, &contract, &error) &&
            error.find("port references unknown signal type") != std::string::npos,
        "TestMaterializedContractLoaderRejectsTamperedJson_UnknownPortTypeRejected");
    std::remove(unknownPortTypePath.c_str());

    CheckTamperedAdapterJson(
        R"json(    {"id": "", "source": "scalar.unit", "target": "scalar.unit", "policy": "safe", "lossy": false, "reversible": true, "cost": 0})json",
        "ui_salt_contract_empty_adapter_id.json",
        "Materialized adapter id must be non-empty",
        "TestMaterializedContractLoaderRejectsTamperedJson_EmptyAdapterIdRejected");
    CheckTamperedAdapterJson(
        R"json(    {"id": "bad.unknown", "source": "scalar.missing", "target": "scalar.unit", "policy": "explicit_only", "lossy": false, "reversible": false, "cost": 1, "fail_closed_reason": "missing source"})json",
        "ui_salt_contract_unknown_adapter_source.json",
        "references unknown source type",
        "TestMaterializedContractLoaderRejectsTamperedJson_UnknownAdapterSourceRejected");
    CheckTamperedAdapterJson(
        R"json(    {"id": "bad.lossy.safe", "source": "scalar.sdf_signed_distance", "target": "scalar.unit", "policy": "safe", "lossy": true, "reversible": false, "cost": 1})json",
        "ui_salt_contract_lossy_safe_adapter.json",
        "lossy adapters cannot use safe policy",
        "TestMaterializedContractLoaderRejectsTamperedJson_LossySafeAdapterRejected");
    CheckTamperedAdapterJson(
        R"json(    {"id": "bad.category.scalar", "source": "category.root_index", "target": "scalar.unit", "policy": "visible_default", "lossy": false, "reversible": false, "cost": 1, "fail_closed_reason": "category projection"})json",
        "ui_salt_contract_category_scalar_adapter.json",
        "category-to-scalar adapters cannot be safe or visible_default",
        "TestMaterializedContractLoaderRejectsTamperedJson_CategoryScalarAdapterRejected");
    CheckTamperedAdapterJson(
        R"json(    {"id": "bad.signed.unit", "source": "scalar.signed", "target": "scalar.unit", "policy": "visible_default", "lossy": false, "reversible": false, "cost": 1, "fail_closed_reason": "signed projection"})json",
        "ui_salt_contract_signed_unit_adapter.json",
        "signed-to-unit adapters require explicit normalization policy",
        "TestMaterializedContractLoaderRejectsTamperedJson_SignedUnitAdapterRejected");

    const char* anyPortTypeJson = R"json({
  "schema_version": 1,
  "source_path": "tampered.ui.salt",
  "signal_type_registry": {"types": [
    {"id": "scalar.unit", "kind": "scalar", "domain": "unit", "topology": "linear", "arity": 1, "default_adapter_policy": "safe"}
  ]},
  "function_library": {
    "lanes": [
      {
        "id": "source",
        "label": "Source",
        "default": "smooth_escape_ramp",
        "functions": [
          {"id": "smooth_escape_ramp", "label": "Smooth Escape Ramp", "description": "", "taxonomy_group": "escape", "runtime_backed": true, "input_kind": "scalar", "output_kind": "scalar", "signal_kind": "scalar", "typed_signal": "scalar.unit", "params": [], "ports": [{"direction": "output", "id": "signal", "type": "any", "canonical": true}]}
        ]
      }
    ]
  },
  "composition_recipe_contract": {"compatibility": [], "row_applicators": [{"id": "none", "label": "None", "target_lane": "source", "required_signal_kind": "any", "requires_sdf_field": false, "storage_param": "signal.sdf_gate", "width_param": "", "fail_closed_reason": "ungated"}], "recipes": []},
  "explaino_contract": {"entries": [
    {"id": "x", "hypothesis_space": "space", "authority": "owner", "lens": "lens", "invariant": "invariant", "proof": "proof", "fallback": "fail_closed", "product_facing": false, "diagnostic": true}
  ]}
})json";

    const std::string anyPortTypePath = TempContractPath("ui_salt_contract_any_port_type.json");
    Check(WriteTextFile(anyPortTypePath, anyPortTypeJson),
        "TestMaterializedContractLoaderRejectsTamperedJson_WriteAnyPortTypeFixture");
    error.clear();
    Check(!LoadColorPipelineMaterializedContractJson(anyPortTypePath, &contract, &error) &&
            error.find("port type 'any' is forbidden") != std::string::npos,
        "TestMaterializedContractLoaderRejectsTamperedJson_AnyPortTypeRejected");
    std::remove(anyPortTypePath.c_str());

    const char* genericAnyPortTypeJson = R"json({
  "schema_version": 1,
  "source_path": "tampered.ui.salt",
  "signal_type_registry": {"types": [
    {"id": "scalar.unit", "kind": "scalar", "domain": "unit", "topology": "linear", "arity": 1, "default_adapter_policy": "safe"}
  ]},
  "function_library": {
    "lanes": [
      {
        "id": "shape",
        "label": "Shape",
        "default": "identity",
        "functions": [
          {"id": "identity", "label": "Identity", "description": "", "taxonomy_group": "identity", "runtime_backed": true, "input_kind": "scalar", "output_kind": "scalar", "params": [], "ports": [{"direction": "input", "id": "signal", "type": "generic.any", "generic_group": "any"}, {"direction": "output", "id": "signal", "type": "generic.any", "generic_group": "any", "canonical": true}]}
        ]
      }
    ]
  },
  "composition_recipe_contract": {"compatibility": [], "row_applicators": [{"id": "none", "label": "None", "target_lane": "source", "required_signal_kind": "any", "requires_sdf_field": false, "storage_param": "signal.sdf_gate", "width_param": "", "fail_closed_reason": "ungated"}], "recipes": []},
  "explaino_contract": {"entries": [
    {"id": "x", "hypothesis_space": "space", "authority": "owner", "lens": "lens", "invariant": "invariant", "proof": "proof", "fallback": "fail_closed", "product_facing": false, "diagnostic": true}
  ]}
})json";

    const std::string genericAnyPortTypePath = TempContractPath("ui_salt_contract_generic_any_port_type.json");
    Check(WriteTextFile(genericAnyPortTypePath, genericAnyPortTypeJson),
        "TestMaterializedContractLoaderRejectsTamperedJson_WriteGenericAnyPortTypeFixture");
    error.clear();
    Check(!LoadColorPipelineMaterializedContractJson(genericAnyPortTypePath, &contract, &error) &&
            error.find("generic_group 'any' is forbidden") != std::string::npos,
        "TestMaterializedContractLoaderRejectsTamperedJson_GenericAnyPortTypeRejected");
    std::remove(genericAnyPortTypePath.c_str());

    const char* duplicateFunctionJson = R"json({
  "schema_version": 1,
  "source_path": "tampered.ui.salt",
  "signal_type_registry": {"types": [
    {"id": "scalar.unit", "kind": "scalar", "domain": "unit", "topology": "linear", "arity": 1, "default_adapter_policy": "safe"}
  ]},
  "function_library": {
    "lanes": [
      {
        "id": "source",
        "label": "Source",
        "default": "smooth_escape_ramp",
        "functions": [
          {"id": "smooth_escape_ramp", "label": "Smooth Escape Ramp", "description": "", "taxonomy_group": "escape", "runtime_backed": true, "input_kind": "scalar", "output_kind": "scalar", "signal_kind": "scalar", "typed_signal": "scalar.unit", "params": []},
          {"id": "smooth_escape_ramp", "label": "Duplicate", "description": "", "taxonomy_group": "escape", "runtime_backed": true, "input_kind": "scalar", "output_kind": "scalar", "signal_kind": "scalar", "typed_signal": "scalar.unit", "params": []}
        ]
      }
    ]
  },
  "composition_recipe_contract": {"compatibility": [], "row_applicators": [{"id": "none", "label": "None", "target_lane": "source", "required_signal_kind": "any", "requires_sdf_field": false, "storage_param": "signal.sdf_gate", "width_param": "", "fail_closed_reason": "ungated"}], "recipes": []},
  "explaino_contract": {"entries": [
    {"id": "x", "hypothesis_space": "space", "authority": "owner", "lens": "lens", "invariant": "invariant", "proof": "proof", "fallback": "fail_closed", "product_facing": false, "diagnostic": true}
  ]}
})json";

    const std::string duplicatePath = TempContractPath("ui_salt_contract_duplicate_function.json");
    Check(WriteTextFile(duplicatePath, duplicateFunctionJson),
        "TestMaterializedContractLoaderRejectsTamperedJson_WriteDuplicateFixture");
    error.clear();
    Check(!LoadColorPipelineMaterializedContractJson(duplicatePath, &contract, &error) &&
            error.find("Duplicate materialized function id") != std::string::npos,
        "TestMaterializedContractLoaderRejectsTamperedJson_DuplicateFunctionRejected");
    std::remove(duplicatePath.c_str());

    const char* danglingCompatibilityJson = R"json({
  "schema_version": 1,
  "source_path": "tampered.ui.salt",
  "signal_type_registry": {"types": [
    {"id": "scalar.unit", "kind": "scalar", "domain": "unit", "topology": "linear", "arity": 1, "default_adapter_policy": "safe"}
  ]},
  "function_library": {
    "lanes": [
      {
        "id": "source",
        "label": "Source",
        "default": "smooth_escape_ramp",
        "functions": [
          {"id": "smooth_escape_ramp", "label": "Smooth Escape Ramp", "description": "", "taxonomy_group": "escape", "runtime_backed": true, "input_kind": "scalar", "output_kind": "scalar", "signal_kind": "scalar", "typed_signal": "scalar.unit", "params": []}
        ]
      },
      {
        "id": "palette",
        "label": "Palette",
        "default": "heatmap",
        "functions": [
          {"id": "heatmap", "label": "Heatmap", "description": "", "taxonomy_group": "palette_escape", "runtime_backed": true, "input_kind": "scalar", "output_kind": "rgb", "params": []}
        ]
      },
      {
        "id": "grading",
        "label": "Grading",
        "default": "contrast_lift",
        "functions": [
          {"id": "contrast_lift", "label": "Contrast Lift", "description": "", "taxonomy_group": "grade_escape", "runtime_backed": true, "input_kind": "rgb", "output_kind": "rgb", "params": []}
        ]
      }
    ]
  },
  "composition_recipe_contract": {
    "compatibility": [
      {"source": "missing_source", "palette": "heatmap", "signal": "smooth_escape_ramp", "palette_runtime": "heatmap", "grading": "contrast_lift", "mode": "smooth_escape", "reason": "tampered"}
    ],
    "row_applicators": [{"id": "none", "label": "None", "target_lane": "source", "required_signal_kind": "any", "requires_sdf_field": false, "storage_param": "signal.sdf_gate", "width_param": "", "fail_closed_reason": "ungated"}],
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

    const char* duplicateCompatOverrideJson = R"json({
  "schema_version": 1,
  "source_path": "tampered.ui.salt",
  "signal_type_registry": {"types": [
    {"id": "scalar.unit", "kind": "scalar", "domain": "unit", "topology": "linear", "arity": 1, "default_adapter_policy": "safe"}
  ]},
  "function_library": {
    "lanes": [
      {"id": "source", "label": "Source", "default": "smooth_escape_ramp", "functions": [{"id": "smooth_escape_ramp", "label": "Smooth Escape Ramp", "description": "", "taxonomy_group": "escape", "runtime_backed": true, "input_kind": "scalar", "output_kind": "scalar", "signal_kind": "scalar", "typed_signal": "scalar.unit", "params": []}]},
      {"id": "palette", "label": "Palette", "default": "heatmap", "functions": [{"id": "heatmap", "label": "Heatmap", "description": "", "taxonomy_group": "palette_escape", "runtime_backed": true, "input_kind": "scalar", "output_kind": "color", "params": []}]},
      {"id": "grading", "label": "Grading", "default": "contrast_lift", "functions": [{"id": "contrast_lift", "label": "Contrast Lift", "description": "", "taxonomy_group": "grade_escape", "runtime_backed": true, "input_kind": "color", "output_kind": "color", "params": []}]}
    ]
  },
  "composition_recipe_contract": {
    "compatibility": [
      {"source": "smooth_escape_ramp", "palette": "heatmap", "signal": "smooth_escape_ramp", "palette_runtime": "heatmap", "grading": "contrast_lift", "mode": "smooth_escape", "reason": "tampered"}
    ],
    "compat_overrides": [
      {"id": "duplicate_override", "source": "smooth_escape_ramp", "palette": "heatmap", "grading": "contrast_lift", "classification": "runtime_legacy_override", "owner_seam": "test", "reason": "test", "proof": "test"},
      {"id": "duplicate_override", "source": "smooth_escape_ramp", "palette": "heatmap", "grading": "contrast_lift", "classification": "runtime_legacy_override", "owner_seam": "test", "reason": "test", "proof": "test"}
    ],
    "compatibility_audit": [
      {"source": "smooth_escape_ramp", "palette": "heatmap", "grading": "contrast_lift", "mode": "smooth_escape", "classification": "runtime_legacy_override", "route_case_id": "", "override_id": "duplicate_override", "reason": "test"}
    ],
    "row_applicators": [{"id": "none", "label": "None", "target_lane": "source", "required_signal_kind": "any", "requires_sdf_field": false, "storage_param": "signal.sdf_gate", "width_param": "", "fail_closed_reason": "ungated"}],
    "recipes": []
  },
  "explaino_contract": {"entries": [
    {"id": "x", "hypothesis_space": "space", "authority": "owner", "lens": "lens", "invariant": "invariant", "proof": "proof", "fallback": "fail_closed", "product_facing": false, "diagnostic": true}
  ]}
})json";

    const std::string duplicateCompatOverridePath = TempContractPath("ui_salt_contract_duplicate_compat_override.json");
    Check(WriteTextFile(duplicateCompatOverridePath, duplicateCompatOverrideJson),
        "TestMaterializedContractLoaderRejectsTamperedJson_WriteDuplicateCompatOverrideFixture");
    error.clear();
    Check(!LoadColorPipelineMaterializedContractJson(duplicateCompatOverridePath, &contract, &error) &&
            error.find("Duplicate materialized compat override id") != std::string::npos,
        "TestMaterializedContractLoaderRejectsTamperedJson_DuplicateCompatOverrideRejected");
    std::remove(duplicateCompatOverridePath.c_str());
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
    TestMaterializedUiSaltMetadataCanOwnCompatibilityLookup();
    TestMaterializedUiSaltMetadataCanOwnCompanionSuggestions();
    TestMaterializedUiSaltMetadataCanOwnRecipeExpansion();
    TestMaterializedContractLoaderRejectsTamperedJson();

    std::printf("test_color_pipeline_core: passed=%d failed=%d\n", g_passed, g_failed);
    return g_failed == 0 ? 0 : 1;
}
