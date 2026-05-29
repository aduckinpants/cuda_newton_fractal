#include "../src/fractal_catalog.h"

#include "../src/enum_id_utils.h"
#include "../src/fractal_family_rules.h"
#include "../src/fractal_probe_runner.h"
#include "../src/perturbation_reference_orbit.h"

#include <cstddef>
#include <iostream>
#include <set>
#include <string>
#include <string_view>

namespace {

int g_failed = 0;

void Check(bool condition, const char* message) {
    if (!condition) {
        ++g_failed;
        std::cerr << "FAIL: " << message << "\n";
    }
}

template <typename T, std::size_t N>
constexpr std::size_t ArraySize(const T (&)[N]) {
    return N;
}

bool HasEnumId(FractalType type, std::string_view selectorId) {
    for (const auto& pair : enum_id_utils::kFractalTypeIds) {
        if (pair.value == type && std::string_view(pair.id) == selectorId) {
            return true;
        }
    }
    return false;
}

} // namespace

int main() {
    const std::size_t enumCount = ArraySize(enum_id_utils::kFractalTypeIds);

    Check(FractalCatalogCount() == enumCount,
        "Fractal catalog must cover every checked-in FractalType id exactly once");

    std::set<int> seenTypes;
    std::set<std::string> seenIds;
    for (const FractalCatalogEntry& entry : kFractalCatalog) {
        Check(entry.selector_id && std::string_view(entry.selector_id).size() > 0,
            "Catalog selector ids must be non-empty");
        Check(entry.display_name && std::string_view(entry.display_name).size() > 0,
            "Catalog display names must be non-empty");
        Check(HasEnumId(entry.type, entry.selector_id),
            "Every catalog row must map back to the enum id authority");
        Check(seenTypes.insert(static_cast<int>(entry.type)).second,
            "Catalog rows must not duplicate FractalType values");
        Check(seenIds.insert(std::string(entry.selector_id)).second,
            "Catalog rows must not duplicate selector ids");
        Check(HasFractalCatalogCapabilityFlag(entry, FractalCatalogCapabilityFlag::schema_control_surface),
            "Catalog rows must declare schema/control-surface capability explicitly");
        Check(HasFractalCatalogCapabilityFlag(entry, FractalCatalogCapabilityFlag::param_animation_surface),
            "Catalog rows must declare animation applicability explicitly");
        Check(HasFractalCatalogCapabilityFlag(entry, FractalCatalogCapabilityFlag::color_pipeline_frame_coloring),
            "Catalog rows must declare Color Pipeline frame-coloring capability explicitly");
    }

    for (const auto& pair : enum_id_utils::kFractalTypeIds) {
        const FractalCatalogEntry* entry = FindFractalCatalogEntry(pair.value);
        Check(entry != nullptr, "Every enum id must have a catalog row");
        if (!entry) {
            continue;
        }

        Check(entry->type == pair.value, "Catalog type lookup must preserve FractalType identity");
        Check(std::string_view(entry->selector_id) == std::string_view(pair.id),
            "Catalog selector id must match enum_id_utils");
        Check(FindFractalCatalogEntryById(pair.id) == entry,
            "Catalog id lookup must resolve to the same row as type lookup");
        Check(HasFractalCatalogCapabilityFlag(*entry, FractalCatalogCapabilityFlag::sample_probe) ==
                IsProbeSamplingImplementedForFractalTypeId(pair.id),
            "Catalog sample/probe capability must mirror the shipped fractal.sample support surface");

        Check(HasFractalCatalogRuntimeFlag(*entry, FractalCatalogRuntimeFlag::escape_time) == IsEscapeTimeFamily(pair.value),
            "Catalog escape-time flag must mirror current family rules");
        Check(HasFractalCatalogRuntimeFlag(*entry, FractalCatalogRuntimeFlag::basin_coloring) == SupportsBasinColoring(pair.value),
            "Catalog basin-coloring flag must mirror current family rules");
        Check(HasFractalCatalogRuntimeFlag(*entry, FractalCatalogRuntimeFlag::explaino_family) == IsExplainoFamily(pair.value),
            "Catalog Explaino flag must mirror current family rules");
        Check(HasFractalCatalogRuntimeFlag(*entry, FractalCatalogRuntimeFlag::perturbation_reference_orbit) ==
                SupportsPerturbationReferenceOrbit(pair.value),
            "Catalog perturbation eligibility flag must mirror the current perturbation seam");
        Check(entry->default_view.auto_max_iter == DefaultAutoMaxIterForFractal(pair.value),
            "Catalog auto max-iter default must mirror current family rules");
        Check(FindFractalCatalogViewDefaults(pair.value) == &entry->default_view,
            "Catalog view default lookup must resolve to the owning catalog row");
        Check(HasFractalCatalogCapabilityFlag(*entry, FractalCatalogCapabilityFlag::smooth_escape_coloring) ==
                IsColoringModeAllowedForFractal(pair.value, ColoringMode::smooth_escape),
            "Catalog smooth-escape coloring capability must mirror current family rules");
        Check(HasFractalCatalogCapabilityFlag(*entry, FractalCatalogCapabilityFlag::root_basin_coloring) ==
                SupportsBasinColoring(pair.value),
            "Catalog root/basin coloring capability must mirror current family rules");
        Check(HasFractalCatalogCapabilityFlag(*entry, FractalCatalogCapabilityFlag::generic_equation_pack) ==
                (pair.value == FractalType::generic_equation_pack),
            "Only the generic equation pack row should carry the generic-equation-pack capability flag");
        Check(HasFractalCatalogCapabilityFlag(*entry, FractalCatalogCapabilityFlag::sdf_pack_scene) ==
                (pair.value == FractalType::sdf_pack_scene),
            "Only the SDF pack scene row should carry the SDF-pack-scene capability flag");

        if (IsExplainoFamily(pair.value)) {
            Check(entry->category == FractalCatalogCategory::explaino,
                "Every Explaino-family fractal should be categorized as Explaino");
            Check(entry->family == FractalCatalogFamily::explaino,
                "Every Explaino-family fractal should share the Explaino catalog family for Slice A");
        }
    }

    const FractalCatalogEntry* sdfPackScene = FindFractalCatalogEntry(FractalType::sdf_pack_scene);
    Check(sdfPackScene != nullptr, "SDF pack scene must have a catalog row");
    if (sdfPackScene) {
        Check(sdfPackScene->category == FractalCatalogCategory::sdf,
            "SDF pack scene should stay in the SDF category");
        Check(sdfPackScene->family == FractalCatalogFamily::sdf_pack_scene,
            "SDF pack scene should have a distinct catalog family");
        Check(sdfPackScene->formula_growth_surface == FractalCatalogFormulaGrowthSurface::sdf_pack_scene,
            "SDF pack scene should keep its SDF-pack formula growth surface");
    }

    const FractalCatalogEntry* generic = FindFractalCatalogEntry(FractalType::generic_equation_pack);
    Check(generic != nullptr, "Generic equation pack must have a catalog row");
    if (generic) {
        Check(generic->category == FractalCatalogCategory::custom,
            "Generic equation pack should stay in the custom/workbench category");
        Check(generic->family == FractalCatalogFamily::generic_equation_pack,
            "Generic equation pack should have a distinct catalog family");
        Check(generic->formula_growth_surface == FractalCatalogFormulaGrowthSurface::generic_equation_pack,
            "Generic equation pack should keep its generic-pack formula growth surface");
    }

    for (FractalType unsupportedProbeType : {
            FractalType::counterfactual_pair,
            FractalType::explaino_counterfactual_pair,
            FractalType::projection_and_flow,
            FractalType::generic_equation_pack,
            FractalType::sdf_pack_scene,
        }) {
        const FractalCatalogEntry* entry = FindFractalCatalogEntry(unsupportedProbeType);
        Check(entry && !HasFractalCatalogCapabilityFlag(*entry, FractalCatalogCapabilityFlag::sample_probe),
            "Unsupported fractal.sample lanes must not advertise sample/probe catalog capability");
    }

    const FractalCatalogEntry* mandelbrot = FindFractalCatalogEntry(FractalType::mandelbrot);
    const FractalCatalogEntry* julia = FindFractalCatalogEntry(FractalType::julia);
    if (mandelbrot && julia) {
        Check(HasFractalCatalogRuntimeFlag(*mandelbrot, FractalCatalogRuntimeFlag::perturbation_reference_orbit),
            "Mandelbrot should remain perturbation-reference eligible");
        Check(HasFractalCatalogRuntimeFlag(*julia, FractalCatalogRuntimeFlag::perturbation_reference_orbit),
            "Julia should remain perturbation-reference eligible");
    }

    if (g_failed != 0) {
        std::cerr << "test_fractal_catalog_authority failed=" << g_failed << "\n";
        return 1;
    }

    std::cout << "test_fractal_catalog_authority: passed\n";
    return 0;
}
