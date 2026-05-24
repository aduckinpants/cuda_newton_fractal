#pragma once

#include "fractal_family_rules.h"
#include "fractal_types.h"
#include "perturbation_reference_orbit.h"

#include <cstddef>
#include <cstdint>
#include <string_view>

enum class FractalCatalogCategory : uint8_t {
    root_finding,
    escape_time,
    explaino,
    analysis,
    custom,
};

enum class FractalCatalogFamily : uint8_t {
    newton,
    nova,
    mandelbrot,
    julia,
    burning_ship,
    multibrot,
    phoenix,
    explaino,
    multicorn,
    halley,
    collatz,
    mcmullen,
    lambda_map,
    spider,
    celtic_mandelbrot,
    perpendicular_burning_ship,
    counterfactual_pair,
    projection_and_flow,
    magnet,
    generic_equation_pack,
};

enum class FractalCatalogViewPolicy : uint8_t {
    root_basin_unit,
    escape_tuned_region,
    explaino_family_region,
    analysis_pair_region,
    custom_workbench_region,
};

enum class FractalCatalogDefaultParamPolicy : uint8_t {
    root_polynomial,
    escape_direct,
    explaino_family,
    analysis_pair,
    custom_workbench,
};

struct FractalCatalogViewDefaults {
    Float2 center;
    float zoom;
    float rotation_degrees;
    bool auto_max_iter;
};

enum class FractalCatalogFormulaGrowthSurface : uint8_t {
    native_2d_formula,
    native_composite_formula,
    generic_equation_pack,
};

enum class FractalCatalogRuntimeFlag : uint32_t {
    escape_time = 1u << 0,
    basin_coloring = 1u << 1,
    explaino_family = 1u << 2,
    perturbation_reference_orbit = 1u << 3,
};

enum class FractalCatalogCapabilityFlag : uint32_t {
    sample_probe = 1u << 0,
    schema_control_surface = 1u << 1,
    param_animation_surface = 1u << 2,
    smooth_escape_coloring = 1u << 3,
    color_pipeline_frame_coloring = 1u << 4,
    generic_equation_pack = 1u << 5,
    root_basin_coloring = 1u << 6,
};

struct FractalCatalogEntry {
    FractalType type;
    const char* selector_id;
    const char* display_name;
    FractalCatalogCategory category;
    FractalCatalogFamily family;
    FractalCatalogViewPolicy view_policy;
    FractalCatalogDefaultParamPolicy default_param_policy;
    FractalCatalogFormulaGrowthSurface formula_growth_surface;
    uint32_t capability_flags;
    uint32_t runtime_flags;
    FractalCatalogViewDefaults default_view;
};

inline constexpr uint32_t FractalCatalogRuntimeFlagMask(FractalCatalogRuntimeFlag flag) {
    return static_cast<uint32_t>(flag);
}

inline constexpr uint32_t FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag flag) {
    return static_cast<uint32_t>(flag);
}

inline constexpr bool FractalCatalogSupportsSampleProbe(FractalType fractalType) {
    switch (fractalType) {
    case FractalType::newton:
    case FractalType::nova:
    case FractalType::mandelbrot:
    case FractalType::julia:
    case FractalType::burning_ship:
    case FractalType::multibrot:
    case FractalType::phoenix:
    case FractalType::explaino:
    case FractalType::explaino_all:
    case FractalType::explaino_y:
    case FractalType::explaino_fp:
    case FractalType::explaino_nova:
    case FractalType::explaino_halley:
    case FractalType::explaino_dual:
    case FractalType::explaino_mult:
    case FractalType::explaino_phoenix:
    case FractalType::explaino_transcendental:
    case FractalType::explaino_inertial:
    case FractalType::explaino_julia:
    case FractalType::explaino_rational:
    case FractalType::multicorn:
    case FractalType::halley:
    case FractalType::collatz:
    case FractalType::explaino_collatz:
    case FractalType::explaino_collatz_direct:
    case FractalType::mcmullen:
    case FractalType::lambda_map:
    case FractalType::explaino_lambda:
    case FractalType::explaino_rational_escape:
    case FractalType::spider:
    case FractalType::celtic_mandelbrot:
    case FractalType::perpendicular_burning_ship:
    case FractalType::explaino_joy:
    case FractalType::explaino_fold:
    case FractalType::explaino_bell:
    case FractalType::explaino_ripple:
    case FractalType::explaino_splice:
    case FractalType::explaino_vortex:
    case FractalType::explaino_tension:
    case FractalType::explaino_balance_void:
    case FractalType::explaino_projection_and_flow:
    case FractalType::magnet:
        return true;
    default:
        return false;
    }
}

inline constexpr uint32_t FractalCatalogCapabilityFlagsFor(FractalType fractalType) {
    return (FractalCatalogSupportsSampleProbe(fractalType)
                ? FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::sample_probe)
                : 0u) |
        FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::schema_control_surface) |
        FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::param_animation_surface) |
        FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::color_pipeline_frame_coloring) |
        (IsColoringModeAllowedForFractal(fractalType, ColoringMode::smooth_escape)
                ? FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::smooth_escape_coloring)
                : 0u) |
        (SupportsBasinColoring(fractalType) ? FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::root_basin_coloring) : 0u) |
        (fractalType == FractalType::generic_equation_pack
                ? FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::generic_equation_pack)
                : 0u);
}

inline constexpr uint32_t FractalCatalogRuntimeFlagsFor(FractalType fractalType) {
    return (IsEscapeTimeFamily(fractalType) ? FractalCatalogRuntimeFlagMask(FractalCatalogRuntimeFlag::escape_time) : 0u) |
        (SupportsBasinColoring(fractalType) ? FractalCatalogRuntimeFlagMask(FractalCatalogRuntimeFlag::basin_coloring) : 0u) |
        (IsExplainoFamily(fractalType) ? FractalCatalogRuntimeFlagMask(FractalCatalogRuntimeFlag::explaino_family) : 0u) |
        (SupportsPerturbationReferenceOrbit(fractalType)
                ? FractalCatalogRuntimeFlagMask(FractalCatalogRuntimeFlag::perturbation_reference_orbit)
                : 0u);
}

inline constexpr FractalCatalogViewDefaults FractalCatalogViewDefaultsFor(FractalType fractalType) {
    switch (fractalType) {
    case FractalType::mandelbrot:
        return {{-0.745f, 0.186f}, 38.0f, 0.0f, DefaultAutoMaxIterForFractal(fractalType)};
    case FractalType::julia:
        return {{0.0f, 0.0f}, 1.5f, 0.0f, DefaultAutoMaxIterForFractal(fractalType)};
    case FractalType::burning_ship:
        return {{-1.762f, -0.028f}, 25.0f, 0.0f, DefaultAutoMaxIterForFractal(fractalType)};
    case FractalType::spider:
        return {{-0.12f, 0.75f}, 4.0f, 0.0f, DefaultAutoMaxIterForFractal(fractalType)};
    case FractalType::celtic_mandelbrot:
        return {{-0.45f, 0.42f}, 3.2f, 0.0f, DefaultAutoMaxIterForFractal(fractalType)};
    case FractalType::perpendicular_burning_ship:
        return {{-1.785f, -0.012f}, 18.0f, 0.0f, DefaultAutoMaxIterForFractal(fractalType)};
    case FractalType::multibrot:
        return {{-0.15f, 0.75f}, 4.5f, 0.0f, DefaultAutoMaxIterForFractal(fractalType)};
    case FractalType::multicorn:
        return {{-0.3f, 0.0f}, 1.5f, 0.0f, DefaultAutoMaxIterForFractal(fractalType)};
    case FractalType::lambda_map:
    case FractalType::explaino_lambda:
        return {{0.5f, 0.0f}, 4.5f, 0.0f, DefaultAutoMaxIterForFractal(fractalType)};
    case FractalType::magnet:
        return {{-0.08f, 0.0f}, 2.2f, 0.0f, DefaultAutoMaxIterForFractal(fractalType)};
    case FractalType::explaino_rational_escape:
        return {{0.0f, 0.0f}, 1.8f, 0.0f, DefaultAutoMaxIterForFractal(fractalType)};
    case FractalType::phoenix:
        return {{0.36f, -0.1f}, 2.8f, 0.0f, DefaultAutoMaxIterForFractal(fractalType)};
    default:
        return {{0.0f, 0.0f}, 1.0f, 0.0f, DefaultAutoMaxIterForFractal(fractalType)};
    }
}

#define FRACTAL_CATALOG_ENTRY(enum_name, selector, label, category_name, family_name, view_name, defaults_name, growth_name) \
    {FractalType::enum_name, selector, label, FractalCatalogCategory::category_name, FractalCatalogFamily::family_name, \
        FractalCatalogViewPolicy::view_name, FractalCatalogDefaultParamPolicy::defaults_name, \
        FractalCatalogFormulaGrowthSurface::growth_name, FractalCatalogCapabilityFlagsFor(FractalType::enum_name), \
        FractalCatalogRuntimeFlagsFor(FractalType::enum_name), FractalCatalogViewDefaultsFor(FractalType::enum_name)}

inline constexpr FractalCatalogEntry kFractalCatalog[] = {
    FRACTAL_CATALOG_ENTRY(newton, "newton", "Newton", root_finding, newton, root_basin_unit, root_polynomial, native_2d_formula),
    FRACTAL_CATALOG_ENTRY(nova, "nova", "Nova", escape_time, nova, escape_tuned_region, root_polynomial, native_2d_formula),
    FRACTAL_CATALOG_ENTRY(mandelbrot, "mandelbrot", "Mandelbrot", escape_time, mandelbrot, escape_tuned_region, escape_direct, native_2d_formula),
    FRACTAL_CATALOG_ENTRY(julia, "julia", "Julia", escape_time, julia, escape_tuned_region, escape_direct, native_2d_formula),
    FRACTAL_CATALOG_ENTRY(burning_ship, "burning_ship", "Burning Ship", escape_time, burning_ship, escape_tuned_region, escape_direct, native_2d_formula),
    FRACTAL_CATALOG_ENTRY(multibrot, "multibrot", "Multibrot", escape_time, multibrot, escape_tuned_region, escape_direct, native_2d_formula),
    FRACTAL_CATALOG_ENTRY(phoenix, "phoenix", "Phoenix", escape_time, phoenix, escape_tuned_region, escape_direct, native_2d_formula),
    FRACTAL_CATALOG_ENTRY(explaino, "explaino", "Explaino", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(explaino_all, "explaino_all", "Explaino All", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(explaino_y, "explaino_y", "Explaino Y", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(explaino_fp, "explaino_fp", "Explaino FP", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(explaino_nova, "explaino_nova", "Explaino Nova", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(explaino_halley, "explaino_halley", "Explaino Halley", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(explaino_dual, "explaino_dual", "Explaino Dual Seed", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(explaino_mult, "explaino_mult", "Explaino Multi Root", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(explaino_phoenix, "explaino_phoenix", "Explaino Phoenix", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(explaino_transcendental, "explaino_transcendental", "Explaino Transcendental", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(explaino_inertial, "explaino_inertial", "Explaino Inertial", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(explaino_julia, "explaino_julia", "Explaino Julia", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(explaino_rational, "explaino_rational", "Explaino Rational", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(multicorn, "multicorn", "Multicorn", escape_time, multicorn, escape_tuned_region, escape_direct, native_2d_formula),
    FRACTAL_CATALOG_ENTRY(halley, "halley", "Halley", root_finding, halley, root_basin_unit, root_polynomial, native_2d_formula),
    FRACTAL_CATALOG_ENTRY(collatz, "collatz", "Collatz", escape_time, collatz, escape_tuned_region, escape_direct, native_2d_formula),
    FRACTAL_CATALOG_ENTRY(explaino_collatz, "explaino_collatz", "Explaino Collatz", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(explaino_collatz_direct, "explaino_collatz_direct", "Explaino Collatz Direct", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(mcmullen, "mcmullen", "McMullen", escape_time, mcmullen, escape_tuned_region, escape_direct, native_2d_formula),
    FRACTAL_CATALOG_ENTRY(lambda_map, "lambda", "Lambda", escape_time, lambda_map, escape_tuned_region, escape_direct, native_2d_formula),
    FRACTAL_CATALOG_ENTRY(explaino_lambda, "explaino_lambda", "Explaino Lambda", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(explaino_rational_escape, "explaino_rational_escape", "Explaino Rational Escape", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(spider, "spider", "Spider", escape_time, spider, escape_tuned_region, escape_direct, native_2d_formula),
    FRACTAL_CATALOG_ENTRY(celtic_mandelbrot, "celtic_mandelbrot", "Celtic Mandelbrot", escape_time, celtic_mandelbrot, escape_tuned_region, escape_direct, native_2d_formula),
    FRACTAL_CATALOG_ENTRY(perpendicular_burning_ship, "perpendicular_burning_ship", "Perpendicular Burning Ship", escape_time, perpendicular_burning_ship, escape_tuned_region, escape_direct, native_2d_formula),
    FRACTAL_CATALOG_ENTRY(explaino_joy, "explaino_joy", "Explaino Joy", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(explaino_fold, "explaino_fold", "Explaino Fold", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(explaino_bell, "explaino_bell", "Explaino Bell", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(explaino_ripple, "explaino_ripple", "Explaino Ripple", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(explaino_splice, "explaino_splice", "Explaino Splice", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(explaino_vortex, "explaino_vortex", "Explaino Vortex", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(explaino_tension, "explaino_tension", "Explaino Tension", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(explaino_balance_void, "explaino_balance_void", "Explaino Balance Void", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(counterfactual_pair, "counterfactual_pair", "Counterfactual Pair", analysis, counterfactual_pair, analysis_pair_region, analysis_pair, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(explaino_counterfactual_pair, "explaino_counterfactual_pair", "Explaino Counterfactual Pair", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(projection_and_flow, "projection_and_flow", "Projection and Flow", analysis, projection_and_flow, analysis_pair_region, analysis_pair, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(explaino_projection_and_flow, "explaino_projection_and_flow", "Explaino Projection and Flow", explaino, explaino, explaino_family_region, explaino_family, native_composite_formula),
    FRACTAL_CATALOG_ENTRY(magnet, "magnet", "Magnet Type I", escape_time, magnet, escape_tuned_region, escape_direct, native_2d_formula),
    FRACTAL_CATALOG_ENTRY(generic_equation_pack, "generic_equation_pack", "Generic Equation Pack", custom, generic_equation_pack, custom_workbench_region, custom_workbench, generic_equation_pack),
};

#undef FRACTAL_CATALOG_ENTRY

template <typename T, std::size_t N>
inline constexpr std::size_t FractalCatalogArraySize(const T (&)[N]) {
    return N;
}

inline constexpr std::size_t FractalCatalogCount() {
    return FractalCatalogArraySize(kFractalCatalog);
}

inline constexpr bool HasFractalCatalogRuntimeFlag(
    const FractalCatalogEntry& entry,
    FractalCatalogRuntimeFlag flag) {
    return (entry.runtime_flags & FractalCatalogRuntimeFlagMask(flag)) != 0u;
}

inline constexpr bool HasFractalCatalogCapabilityFlag(
    const FractalCatalogEntry& entry,
    FractalCatalogCapabilityFlag flag) {
    return (entry.capability_flags & FractalCatalogCapabilityFlagMask(flag)) != 0u;
}

inline const FractalCatalogEntry* FindFractalCatalogEntry(FractalType fractalType) {
    for (const FractalCatalogEntry& entry : kFractalCatalog) {
        if (entry.type == fractalType) {
            return &entry;
        }
    }
    return nullptr;
}

inline const FractalCatalogEntry* FindFractalCatalogEntryById(std::string_view selectorId) {
    for (const FractalCatalogEntry& entry : kFractalCatalog) {
        if (std::string_view(entry.selector_id) == selectorId) {
            return &entry;
        }
    }
    return nullptr;
}

inline const FractalCatalogViewDefaults* FindFractalCatalogViewDefaults(FractalType fractalType) {
    const FractalCatalogEntry* entry = FindFractalCatalogEntry(fractalType);
    return entry ? &entry->default_view : nullptr;
}
