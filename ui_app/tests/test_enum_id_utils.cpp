#include "../src/enum_id_utils.h"

#include <cstdio>

static int g_passed = 0;
static int g_failed = 0;

static void Check(bool cond, const char* name) {
    if (cond) {
        std::printf("  PASS: %s\n", name);
        ++g_passed;
    } else {
        std::printf("  FAIL: %s\n", name);
        ++g_failed;
    }
}

enum class ExampleMode {
    alpha,
    beta,
};

static constexpr enum_id_utils::EnumIdPair<ExampleMode> kExampleModeIds[] = {
    {ExampleMode::alpha, "alpha"},
    {ExampleMode::beta, "beta"},
};

static void TestGenericEnumLookupAndParseHelpers() {
    ExampleMode parsed = ExampleMode::alpha;

    Check(enum_id_utils::LookupEnumId(ExampleMode::beta, kExampleModeIds) == std::string_view("beta"),
        "TestGenericEnumLookupAndParseHelpers_LookupKnownValue");
    Check(enum_id_utils::LookupEnumId(static_cast<ExampleMode>(99), kExampleModeIds) == nullptr,
        "TestGenericEnumLookupAndParseHelpers_LookupUnknownValue");
    Check(enum_id_utils::TryParseEnumId("alpha", kExampleModeIds, &parsed) && parsed == ExampleMode::alpha,
        "TestGenericEnumLookupAndParseHelpers_ParseKnownValue");
    Check(enum_id_utils::TryParseEnumId("beta", kExampleModeIds, static_cast<ExampleMode*>(nullptr)),
        "TestGenericEnumLookupAndParseHelpers_ParseKnownValueWithoutOutput");
    Check(!enum_id_utils::TryParseEnumId("gamma", kExampleModeIds, &parsed),
        "TestGenericEnumLookupAndParseHelpers_ParseUnknownValue");
}

static void TestConcreteEnumIdWrappers() {
    PolyKind polyKind = PolyKind::custom;
    FractalType fractalType = FractalType::newton;
    ColoringMode coloringMode = ColoringMode::root_basin;
    CameraBehavior cameraBehavior = CameraBehavior::manual;
    SampleTier sampleTier = SampleTier::fast;
    ColorPipelineShape shape = ColorPipelineShape::identity;
    CounterfactualPairRootFamily rootFamily = CounterfactualPairRootFamily::cubic_unit_roots;
    CounterfactualPairFrame pairFrame = CounterfactualPairFrame::world_absolute;
    ProjectionAndFlowRootFamily projectionFlowRootFamily = ProjectionAndFlowRootFamily::cubic_unit_roots;

    Check(std::string_view(PolyKindId(PolyKind::z3_minus_1)) == "z3_minus_1",
        "TestConcreteEnumIdWrappers_PolyKindLookup");
    Check(TryParsePolyKindId("custom", &polyKind) && polyKind == PolyKind::custom,
        "TestConcreteEnumIdWrappers_PolyKindParse");

    Check(std::string_view(FractalTypeId(FractalType::explaino_all)) == "explaino_all",
        "TestConcreteEnumIdWrappers_FractalTypeCanonicalExplainoAllLookup");
    Check(TryParseFractalTypeId("explaino_all", &fractalType) && fractalType == FractalType::explaino_all,
        "TestConcreteEnumIdWrappers_FractalTypeCanonicalExplainoAllParse");
    Check(std::string_view(FractalTypeId(FractalType::counterfactual_pair)) == "counterfactual_pair",
        "TestConcreteEnumIdWrappers_FractalTypeCounterfactualPairLookup");
    Check(TryParseFractalTypeId("counterfactual_pair", &fractalType) && fractalType == FractalType::counterfactual_pair,
        "TestConcreteEnumIdWrappers_FractalTypeCounterfactualPairParse");
    Check(std::string_view(FractalTypeId(FractalType::explaino_counterfactual_pair)) == "explaino_counterfactual_pair",
        "TestConcreteEnumIdWrappers_FractalTypeExplainoCounterfactualPairLookup");
    Check(TryParseFractalTypeId("explaino_counterfactual_pair", &fractalType) && fractalType == FractalType::explaino_counterfactual_pair,
        "TestConcreteEnumIdWrappers_FractalTypeExplainoCounterfactualPairParse");
    Check(std::string_view(FractalTypeId(FractalType::projection_and_flow)) == "projection_and_flow",
        "TestConcreteEnumIdWrappers_FractalTypeProjectionAndFlowLookup");
    Check(TryParseFractalTypeId("projection_and_flow", &fractalType) && fractalType == FractalType::projection_and_flow,
        "TestConcreteEnumIdWrappers_FractalTypeProjectionAndFlowParse");
    Check(std::string_view(FractalTypeId(FractalType::explaino_projection_and_flow)) == "explaino_projection_and_flow",
        "TestConcreteEnumIdWrappers_FractalTypeExplainoProjectionAndFlowLookup");
    Check(TryParseFractalTypeId("explaino_projection_and_flow", &fractalType) &&
            fractalType == FractalType::explaino_projection_and_flow,
        "TestConcreteEnumIdWrappers_FractalTypeExplainoProjectionAndFlowParse");
    Check(!TryParseFractalTypeId("meta_basin", &fractalType),
        "TestConcreteEnumIdWrappers_FractalTypeRejectsStandaloneMetaBasin");
    Check(!TryParseFractalTypeId("explaino_meta_basin", &fractalType),
        "TestConcreteEnumIdWrappers_FractalTypeRejectsExplainoMetaBasin");
    Check(std::string_view(FractalTypeId(FractalType::explaino_lambda)) == "explaino_lambda",
        "TestConcreteEnumIdWrappers_FractalTypeLookup");
    Check(std::string_view(FractalTypeId(FractalType::magnet)) == "magnet",
        "TestConcreteEnumIdWrappers_FractalTypeMagnetLookup");
    Check(TryParseFractalTypeId("magnet", &fractalType) && fractalType == FractalType::magnet,
        "TestConcreteEnumIdWrappers_FractalTypeMagnetParse");
    Check(std::string_view(FractalTypeId(FractalType::generic_equation_pack)) == "generic_equation_pack",
        "TestConcreteEnumIdWrappers_FractalTypeGenericEquationPackLookup");
    Check(TryParseFractalTypeId("generic_equation_pack", &fractalType) && fractalType == FractalType::generic_equation_pack,
        "TestConcreteEnumIdWrappers_FractalTypeGenericEquationPackParse");
    Check(std::string_view(FractalTypeId(FractalType::explaino_collatz_direct)) == "explaino_collatz_direct",
        "TestConcreteEnumIdWrappers_FractalTypeExplainoCollatzDirectLookup");
    Check(TryParseFractalTypeId("explaino_collatz_direct", &fractalType) &&
            fractalType == FractalType::explaino_collatz_direct,
        "TestConcreteEnumIdWrappers_FractalTypeExplainoCollatzDirectParse");
    Check(TryParseFractalTypeId("lambda", &fractalType) && fractalType == FractalType::lambda_map,
        "TestConcreteEnumIdWrappers_FractalTypeParseAlias");

    Check(std::string_view(ColoringModeId(ColoringMode::smooth_escape)) == "smooth_escape",
        "TestConcreteEnumIdWrappers_ColoringModeLookup");
    Check(TryParseColoringModeId("joy_basins", &coloringMode) && coloringMode == ColoringMode::joy_basins,
        "TestConcreteEnumIdWrappers_ColoringModeParse");
    Check(!TryParseColoringModeId("not_a_mode", &coloringMode),
        "TestConcreteEnumIdWrappers_ColoringModeRejectsUnknown");

    Check(std::string_view(CameraBehaviorId(CameraBehavior::entropy)) == "entropy",
        "TestConcreteEnumIdWrappers_CameraBehaviorLookup");
    Check(TryParseCameraBehaviorId("orbit", &cameraBehavior) && cameraBehavior == CameraBehavior::orbit,
        "TestConcreteEnumIdWrappers_CameraBehaviorParse");

    Check(std::string_view(SampleTierId(SampleTier::tier_auto)) == "tier_auto",
        "TestConcreteEnumIdWrappers_SampleTierLookup");
    Check(TryParseSampleTierId("standard", &sampleTier) && sampleTier == SampleTier::standard,
        "TestConcreteEnumIdWrappers_SampleTierParse");

    Check(std::string_view(ColorPipelineShapeId(ColorPipelineShape::smooth_window)) == "smooth_window",
        "TestConcreteEnumIdWrappers_ColorPipelineShapeLookup");
    Check(TryParseColorPipelineShapeId("mirror_repeat", &shape) && shape == ColorPipelineShape::mirror_repeat,
        "TestConcreteEnumIdWrappers_ColorPipelineShapeParse");

    Check(std::string_view(CounterfactualPairRootFamilyId(CounterfactualPairRootFamily::cubic_unit_roots)) == "cubic_unit_roots",
        "TestConcreteEnumIdWrappers_CounterfactualPairRootFamilyLookup");
    Check(TryParseCounterfactualPairRootFamilyId("quartic_unit_roots", &rootFamily) && rootFamily == CounterfactualPairRootFamily::quartic_unit_roots,
        "TestConcreteEnumIdWrappers_CounterfactualPairRootFamilyParse");

    Check(std::string_view(CounterfactualPairFrameId(CounterfactualPairFrame::world_absolute)) == "world_absolute",
        "TestConcreteEnumIdWrappers_CounterfactualPairFrameLookup");
    Check(TryParseCounterfactualPairFrameId("view_relative", &pairFrame) && pairFrame == CounterfactualPairFrame::view_relative,
        "TestConcreteEnumIdWrappers_CounterfactualPairFrameParse");

    Check(std::string_view(ProjectionAndFlowRootFamilyId(ProjectionAndFlowRootFamily::cubic_unit_roots)) == "cubic_unit_roots",
        "TestConcreteEnumIdWrappers_ProjectionAndFlowRootFamilyLookup");
    Check(TryParseProjectionAndFlowRootFamilyId("quartic_unit_roots", &projectionFlowRootFamily) &&
            projectionFlowRootFamily == ProjectionAndFlowRootFamily::quartic_unit_roots,
        "TestConcreteEnumIdWrappers_ProjectionAndFlowRootFamilyParse");
}

int main() {
    TestGenericEnumLookupAndParseHelpers();
    TestConcreteEnumIdWrappers();

    std::printf("test_enum_id_utils: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}
