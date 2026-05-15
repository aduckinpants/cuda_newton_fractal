#pragma once

#include "fractal_types.h"

#include <string_view>

#if defined(__CUDACC__)
#define FRACTAL_FAMILY_RULES_HD __host__ __device__
#else
#define FRACTAL_FAMILY_RULES_HD
#endif

enum class ExplainoAxisParamSlot : int {
    ripple_amplitude = 0,
    splice_offset = 1,
    vortex_strength = 2,
    tension_strength = 3,
    balance_void = 4,
    symmetry_tension = 5,
    field_curvature = 6,
};

struct ExplainoAxisDescriptor {
    const char* axis_id;
    const char* binding_path;
    FractalType carrier_fractal_type;
    float default_value;
    ExplainoAxisParamSlot slot;
};

enum class ExplainoCouplingParamSlot : int {
    momentum_beta = 0,
    joy_coupling = 1,
    fold_coupling = 2,
    bell_coupling = 3,
};

enum class ExplainoCouplingOwnership : int {
    canonical_axis = 0,
    legacy_only = 1,
    different_ownership_model = 2,
};

enum class ExplainoCouplingModel : int {
    inertial_memory = 0,
    phoenix_step_variant = 1,
};

struct ExplainoCouplingDescriptor {
    const char* param_id;
    const char* binding_path;
    FractalType carrier_fractal_type;
    float default_value;
    float neutral_value;
    ExplainoCouplingParamSlot slot;
    ExplainoCouplingOwnership ownership;
    ExplainoCouplingModel model;
    bool zero_collapses_to_baseline;
    bool requires_root_pack_modifier;
};

enum class ExplainoDualSeedOwnership : int {
    canonical_axis = 0,
    legacy_only = 1,
    different_ownership_model = 2,
};

enum class ExplainoDualSeedModel : int {
    secondary_seed_surface = 0,
    blend_gate = 1,
};

struct ExplainoDualSeedDescriptor {
    const char* param_id;
    const char* binding_path;
    FractalType carrier_fractal_type;
    double default_value;
    double neutral_value;
    ExplainoDualSeedOwnership ownership;
    ExplainoDualSeedModel model;
    bool zero_collapses_to_baseline;
    bool default_collapses_to_baseline;
};

enum class PhoenixStepCarrierParamSlot : int {
    phoenix_p_real = 0,
};

enum class PhoenixStepCarrierOwnership : int {
    canonical_axis = 0,
    legacy_only = 1,
    different_ownership_model = 2,
};

enum class PhoenixStepCarrierModel : int {
    shared_memory_term = 0,
};

enum class PhoenixStepCarrierRuntimeRole : int {
    phoenix_step_memory_term = 0,
};

struct PhoenixStepCarrierDescriptor {
    const char* param_id;
    const char* binding_path;
    PhoenixStepCarrierParamSlot slot;
    PhoenixStepCarrierOwnership ownership;
    PhoenixStepCarrierModel model;
    bool shared_with_explaino_legacy_selectors;
    bool zero_is_noncanonical_carrier_neutral;
};

struct PhoenixStepCarrierSelectorDescriptor {
    FractalType carrier_fractal_type;
    PhoenixStepCarrierParamSlot slot;
    double default_value;
    double neutral_value;
    PhoenixStepCarrierRuntimeRole runtime_role;
    bool zero_collapses_to_baseline;
    bool default_collapses_to_baseline;
};

enum class ExplainoStructuralParamSlot : int {
    explaino_cluster_radius = 0,
};

enum class ExplainoStructuralOwnership : int {
    canonical_axis = 0,
    legacy_only = 1,
    different_ownership_model = 2,
};

enum class ExplainoStructuralModel : int {
    root_pack_modifier = 0,
};

enum class ExplainoStructuralRuntimeRole : int {
    multi_root_cluster_split = 0,
    rational_laurent_term = 1,
};

struct ExplainoStructuralDescriptor {
    const char* param_id;
    const char* binding_path;
    ExplainoStructuralParamSlot slot;
    ExplainoStructuralOwnership ownership;
    ExplainoStructuralModel model;
    bool shared_with_non_explaino_carrier;
    bool requires_carrier_specific_zero_collapse;
};

struct ExplainoStructuralCarrierDescriptor {
    FractalType carrier_fractal_type;
    ExplainoStructuralParamSlot slot;
    double default_value;
    double neutral_value;
    ExplainoStructuralRuntimeRole runtime_role;
    bool zero_collapses_to_baseline;
    bool default_collapses_to_baseline;
};

FRACTAL_FAMILY_RULES_HD inline constexpr FractalType ExplainoCanonicalFractalType() {
    return FractalType::explaino_all;
}

enum class ExplainoSelectorRole : int {
    baseline = 0,
    canonical_public = 1,
    legacy_family_nonprojection = 2,
    legacy_projection_single_axis = 3,
    legacy_projection_multi_axis = 4,
};

struct ExplainoSelectorDescriptor {
    FractalType fractal_type;
    const char* fractal_type_id;
    ExplainoSelectorRole role;
};

inline constexpr ExplainoSelectorDescriptor kExplainoSelectorRegistry[] = {
    {FractalType::explaino, "explaino", ExplainoSelectorRole::baseline},
    {FractalType::explaino_all, "explaino_all", ExplainoSelectorRole::canonical_public},
    {FractalType::explaino_y, "explaino_y", ExplainoSelectorRole::legacy_family_nonprojection},
    {FractalType::explaino_fp, "explaino_fp", ExplainoSelectorRole::legacy_family_nonprojection},
    {FractalType::explaino_nova, "explaino_nova", ExplainoSelectorRole::legacy_family_nonprojection},
    {FractalType::explaino_halley, "explaino_halley", ExplainoSelectorRole::legacy_family_nonprojection},
    {FractalType::explaino_dual, "explaino_dual", ExplainoSelectorRole::legacy_family_nonprojection},
    {FractalType::explaino_mult, "explaino_mult", ExplainoSelectorRole::legacy_family_nonprojection},
    {FractalType::explaino_phoenix, "explaino_phoenix", ExplainoSelectorRole::legacy_family_nonprojection},
    {FractalType::explaino_transcendental, "explaino_transcendental", ExplainoSelectorRole::legacy_family_nonprojection},
    {FractalType::explaino_inertial, "explaino_inertial", ExplainoSelectorRole::legacy_family_nonprojection},
    {FractalType::explaino_julia, "explaino_julia", ExplainoSelectorRole::legacy_family_nonprojection},
    {FractalType::explaino_rational, "explaino_rational", ExplainoSelectorRole::legacy_family_nonprojection},
    {FractalType::explaino_collatz, "explaino_collatz", ExplainoSelectorRole::legacy_family_nonprojection},
    {FractalType::explaino_lambda, "explaino_lambda", ExplainoSelectorRole::legacy_family_nonprojection},
    {FractalType::explaino_rational_escape, "explaino_rational_escape", ExplainoSelectorRole::legacy_family_nonprojection},
    {FractalType::explaino_joy, "explaino_joy", ExplainoSelectorRole::legacy_family_nonprojection},
    {FractalType::explaino_fold, "explaino_fold", ExplainoSelectorRole::legacy_family_nonprojection},
    {FractalType::explaino_bell, "explaino_bell", ExplainoSelectorRole::legacy_family_nonprojection},
    {FractalType::explaino_ripple, "explaino_ripple", ExplainoSelectorRole::legacy_projection_single_axis},
    {FractalType::explaino_splice, "explaino_splice", ExplainoSelectorRole::legacy_projection_single_axis},
    {FractalType::explaino_vortex, "explaino_vortex", ExplainoSelectorRole::legacy_projection_single_axis},
    {FractalType::explaino_tension, "explaino_tension", ExplainoSelectorRole::legacy_projection_single_axis},
    {FractalType::explaino_balance_void, "explaino_balance_void", ExplainoSelectorRole::legacy_projection_multi_axis},
};

inline constexpr const ExplainoSelectorDescriptor* FindExplainoSelectorDescriptor(FractalType fractalType) {
    for (const auto& selector : kExplainoSelectorRegistry) {
        if (selector.fractal_type == fractalType) {
            return &selector;
        }
    }
    return nullptr;
}

inline const ExplainoSelectorDescriptor* FindExplainoSelectorDescriptor(std::string_view fractalTypeId) {
    for (const auto& selector : kExplainoSelectorRegistry) {
        if (fractalTypeId == selector.fractal_type_id) {
            return &selector;
        }
    }
    return nullptr;
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool IsExplainoSingleAxisProjectionSelector(FractalType fractalType) {
    switch (fractalType) {
    case FractalType::explaino_ripple:
    case FractalType::explaino_splice:
    case FractalType::explaino_vortex:
    case FractalType::explaino_tension:
        return true;
    default:
        return false;
    }
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool IsExplainoLegacyProjectionSelector(FractalType fractalType) {
    return IsExplainoSingleAxisProjectionSelector(fractalType) ||
        fractalType == FractalType::explaino_balance_void;
}

FRACTAL_FAMILY_RULES_HD inline constexpr FractalType ResolveExplainoPublicFractalType(FractalType fractalType) {
    return IsExplainoLegacyProjectionSelector(fractalType)
        ? ExplainoCanonicalFractalType()
        : fractalType;
}

inline constexpr ExplainoAxisDescriptor kExplainoAxisRegistry[] = {
    {"ripple_amplitude", "fractal.params.ripple_amplitude", FractalType::explaino_ripple, 0.15f, ExplainoAxisParamSlot::ripple_amplitude},
    {"splice_offset", "fractal.params.splice_offset", FractalType::explaino_splice, 0.5f, ExplainoAxisParamSlot::splice_offset},
    {"vortex_strength", "fractal.params.vortex_strength", FractalType::explaino_vortex, 0.3f, ExplainoAxisParamSlot::vortex_strength},
    {"tension_strength", "fractal.params.tension_strength", FractalType::explaino_tension, 0.02f, ExplainoAxisParamSlot::tension_strength},
    {"balance_void", "fractal.params.balance_void", FractalType::explaino_balance_void, 0.0f, ExplainoAxisParamSlot::balance_void},
    {"symmetry_tension", "fractal.params.symmetry_tension", FractalType::explaino_balance_void, 0.0f, ExplainoAxisParamSlot::symmetry_tension},
    {"field_curvature", "fractal.params.field_curvature", FractalType::explaino_balance_void, 0.0f, ExplainoAxisParamSlot::field_curvature},
};

inline constexpr ExplainoCouplingDescriptor kExplainoCouplingRegistry[] = {
    {"momentum_beta", "fractal.params.momentum_beta", FractalType::explaino_inertial, 0.15f, 0.0f, ExplainoCouplingParamSlot::momentum_beta, ExplainoCouplingOwnership::different_ownership_model, ExplainoCouplingModel::inertial_memory, true, false},
    {"joy_coupling", "fractal.params.joy_coupling", FractalType::explaino_joy, 0.3f, 0.0f, ExplainoCouplingParamSlot::joy_coupling, ExplainoCouplingOwnership::different_ownership_model, ExplainoCouplingModel::phoenix_step_variant, false, true},
    {"fold_coupling", "fractal.params.fold_coupling", FractalType::explaino_fold, 0.5f, 0.0f, ExplainoCouplingParamSlot::fold_coupling, ExplainoCouplingOwnership::different_ownership_model, ExplainoCouplingModel::phoenix_step_variant, false, true},
    {"bell_coupling", "fractal.params.bell_coupling", FractalType::explaino_bell, 0.5f, 0.0f, ExplainoCouplingParamSlot::bell_coupling, ExplainoCouplingOwnership::different_ownership_model, ExplainoCouplingModel::phoenix_step_variant, false, true},
};

inline constexpr ExplainoDualSeedDescriptor kExplainoDualSeedRegistry[] = {
    {"explaino_seed_b", "fractal.params.explaino_seed_b", FractalType::explaino_dual, 1.0, 0.0, ExplainoDualSeedOwnership::different_ownership_model, ExplainoDualSeedModel::secondary_seed_surface, false, false},
    {"explaino_mix", "fractal.params.explaino_mix", FractalType::explaino_dual, 0.5, 0.0, ExplainoDualSeedOwnership::different_ownership_model, ExplainoDualSeedModel::blend_gate, true, false},
};

inline constexpr PhoenixStepCarrierDescriptor kPhoenixStepCarrierRegistry[] = {
    {"phoenix_p_real", "fractal.params.phoenix_p_real", PhoenixStepCarrierParamSlot::phoenix_p_real, PhoenixStepCarrierOwnership::different_ownership_model, PhoenixStepCarrierModel::shared_memory_term, true, true},
};

inline constexpr PhoenixStepCarrierSelectorDescriptor kPhoenixStepCarrierSelectorRegistry[] = {
    {FractalType::phoenix, PhoenixStepCarrierParamSlot::phoenix_p_real, 0.5667, 0.0, PhoenixStepCarrierRuntimeRole::phoenix_step_memory_term, false, false},
    {FractalType::explaino_phoenix, PhoenixStepCarrierParamSlot::phoenix_p_real, 0.12, 0.0, PhoenixStepCarrierRuntimeRole::phoenix_step_memory_term, false, false},
    {FractalType::explaino_joy, PhoenixStepCarrierParamSlot::phoenix_p_real, 0.0, 0.0, PhoenixStepCarrierRuntimeRole::phoenix_step_memory_term, false, false},
    {FractalType::explaino_fold, PhoenixStepCarrierParamSlot::phoenix_p_real, 0.0, 0.0, PhoenixStepCarrierRuntimeRole::phoenix_step_memory_term, false, false},
    {FractalType::explaino_bell, PhoenixStepCarrierParamSlot::phoenix_p_real, 0.0, 0.0, PhoenixStepCarrierRuntimeRole::phoenix_step_memory_term, false, false},
    {FractalType::explaino_ripple, PhoenixStepCarrierParamSlot::phoenix_p_real, 0.0, 0.0, PhoenixStepCarrierRuntimeRole::phoenix_step_memory_term, false, false},
    {FractalType::explaino_splice, PhoenixStepCarrierParamSlot::phoenix_p_real, 0.0, 0.0, PhoenixStepCarrierRuntimeRole::phoenix_step_memory_term, false, false},
    {FractalType::explaino_vortex, PhoenixStepCarrierParamSlot::phoenix_p_real, 0.0, 0.0, PhoenixStepCarrierRuntimeRole::phoenix_step_memory_term, false, false},
    {FractalType::explaino_tension, PhoenixStepCarrierParamSlot::phoenix_p_real, 0.0, 0.0, PhoenixStepCarrierRuntimeRole::phoenix_step_memory_term, false, false},
};

inline constexpr ExplainoStructuralDescriptor kExplainoStructuralRegistry[] = {
    {"explaino_cluster_radius", "fractal.params.explaino_cluster_radius", ExplainoStructuralParamSlot::explaino_cluster_radius, ExplainoStructuralOwnership::different_ownership_model, ExplainoStructuralModel::root_pack_modifier, false, true},
};

inline constexpr ExplainoStructuralCarrierDescriptor kExplainoStructuralCarrierRegistry[] = {
    {FractalType::explaino_mult, ExplainoStructuralParamSlot::explaino_cluster_radius, 0.0, 0.0, ExplainoStructuralRuntimeRole::multi_root_cluster_split, false, false},
    {FractalType::explaino_rational, ExplainoStructuralParamSlot::explaino_cluster_radius, 0.1, 0.0, ExplainoStructuralRuntimeRole::rational_laurent_term, false, false},
};

inline constexpr const ExplainoAxisDescriptor* FindExplainoSingleAxisProjectionDescriptor(FractalType fractalType) {
    if (!IsExplainoSingleAxisProjectionSelector(fractalType)) {
        return nullptr;
    }
    for (const auto& axis : kExplainoAxisRegistry) {
        if (axis.carrier_fractal_type == fractalType) {
            return &axis;
        }
    }
    return nullptr;
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool IsExplainoComposedAxisCarrier(FractalType fractalType) {
    return IsExplainoSingleAxisProjectionSelector(fractalType);
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool HasExplainoComposedAxisPerturbation(const KernelParams& params) {
    return params.ripple_amplitude != 0.0f ||
        params.splice_offset != 0.0f ||
        params.vortex_strength != 0.0f ||
        params.tension_strength != 0.0f;
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool HasExplainoBalanceVoidPerturbation(const KernelParams& params) {
    return params.balance_void != 0.0f ||
        params.symmetry_tension != 0.0f ||
        params.field_curvature != 0.0f;
}

FRACTAL_FAMILY_RULES_HD inline constexpr FractalType ResolveExplainoRuntimeFractalType(
    FractalType fractalType,
    const KernelParams& params) {
    if (fractalType == ExplainoCanonicalFractalType()) {
        if (HasExplainoComposedAxisPerturbation(params)) {
            return FractalType::explaino_ripple;
        }
        if (HasExplainoBalanceVoidPerturbation(params)) {
            return FractalType::explaino_balance_void;
        }
        return FractalType::explaino;
    }
    if (IsExplainoComposedAxisCarrier(fractalType) && !HasExplainoComposedAxisPerturbation(params)) {
        return FractalType::explaino;
    }
    if (fractalType == FractalType::explaino_balance_void && !HasExplainoBalanceVoidPerturbation(params)) {
        return FractalType::explaino;
    }
    return fractalType;
}

inline const ExplainoAxisDescriptor* FindExplainoAxisDescriptor(std::string_view axisId) {
    for (const auto& axis : kExplainoAxisRegistry) {
        if (axisId == axis.axis_id) {
            return &axis;
        }
    }
    return nullptr;
}

inline constexpr const ExplainoCouplingDescriptor* FindExplainoCouplingDescriptor(FractalType fractalType) {
    for (const auto& coupling : kExplainoCouplingRegistry) {
        if (coupling.carrier_fractal_type == fractalType) {
            return &coupling;
        }
    }
    return nullptr;
}

inline const ExplainoCouplingDescriptor* FindExplainoCouplingDescriptor(std::string_view paramId) {
    for (const auto& coupling : kExplainoCouplingRegistry) {
        if (paramId == coupling.param_id) {
            return &coupling;
        }
    }
    return nullptr;
}

inline const ExplainoCouplingDescriptor* FindExplainoCouplingDescriptorByBindingPath(std::string_view bindingPath) {
    for (const auto& coupling : kExplainoCouplingRegistry) {
        if (bindingPath == coupling.binding_path) {
            return &coupling;
        }
    }
    return nullptr;
}

inline const ExplainoDualSeedDescriptor* FindExplainoDualSeedDescriptor(std::string_view paramId) {
    for (const auto& dualSeed : kExplainoDualSeedRegistry) {
        if (paramId == dualSeed.param_id) {
            return &dualSeed;
        }
    }
    return nullptr;
}

inline const ExplainoDualSeedDescriptor* FindExplainoDualSeedDescriptorByBindingPath(std::string_view bindingPath) {
    for (const auto& dualSeed : kExplainoDualSeedRegistry) {
        if (bindingPath == dualSeed.binding_path) {
            return &dualSeed;
        }
    }
    return nullptr;
}

inline const PhoenixStepCarrierDescriptor* FindPhoenixStepCarrierDescriptor(std::string_view paramId) {
    for (const auto& carrier : kPhoenixStepCarrierRegistry) {
        if (paramId == carrier.param_id) {
            return &carrier;
        }
    }
    return nullptr;
}

inline const PhoenixStepCarrierDescriptor* FindPhoenixStepCarrierDescriptorByBindingPath(std::string_view bindingPath) {
    for (const auto& carrier : kPhoenixStepCarrierRegistry) {
        if (bindingPath == carrier.binding_path) {
            return &carrier;
        }
    }
    return nullptr;
}

inline constexpr const PhoenixStepCarrierSelectorDescriptor* FindPhoenixStepCarrierSelectorDescriptor(FractalType fractalType) {
    for (const auto& carrier : kPhoenixStepCarrierSelectorRegistry) {
        if (carrier.carrier_fractal_type == fractalType) {
            return &carrier;
        }
    }
    return nullptr;
}

inline const ExplainoStructuralDescriptor* FindExplainoStructuralDescriptor(std::string_view paramId) {
    for (const auto& structural : kExplainoStructuralRegistry) {
        if (paramId == structural.param_id) {
            return &structural;
        }
    }
    return nullptr;
}

inline const ExplainoStructuralDescriptor* FindExplainoStructuralDescriptorByBindingPath(std::string_view bindingPath) {
    for (const auto& structural : kExplainoStructuralRegistry) {
        if (bindingPath == structural.binding_path) {
            return &structural;
        }
    }
    return nullptr;
}

inline constexpr const ExplainoStructuralCarrierDescriptor* FindExplainoStructuralCarrierDescriptor(FractalType fractalType) {
    for (const auto& carrier : kExplainoStructuralCarrierRegistry) {
        if (carrier.carrier_fractal_type == fractalType) {
            return &carrier;
        }
    }
    return nullptr;
}

inline float* ResolveExplainoAxisValue(KernelParams& params, ExplainoAxisParamSlot slot) {
    switch (slot) {
    case ExplainoAxisParamSlot::ripple_amplitude:
        return &params.ripple_amplitude;
    case ExplainoAxisParamSlot::splice_offset:
        return &params.splice_offset;
    case ExplainoAxisParamSlot::vortex_strength:
        return &params.vortex_strength;
    case ExplainoAxisParamSlot::tension_strength:
        return &params.tension_strength;
    case ExplainoAxisParamSlot::balance_void:
        return &params.balance_void;
    case ExplainoAxisParamSlot::symmetry_tension:
        return &params.symmetry_tension;
    case ExplainoAxisParamSlot::field_curvature:
        return &params.field_curvature;
    }
    return nullptr;
}

inline const float* ResolveExplainoAxisValue(const KernelParams& params, ExplainoAxisParamSlot slot) {
    switch (slot) {
    case ExplainoAxisParamSlot::ripple_amplitude:
        return &params.ripple_amplitude;
    case ExplainoAxisParamSlot::splice_offset:
        return &params.splice_offset;
    case ExplainoAxisParamSlot::vortex_strength:
        return &params.vortex_strength;
    case ExplainoAxisParamSlot::tension_strength:
        return &params.tension_strength;
    case ExplainoAxisParamSlot::balance_void:
        return &params.balance_void;
    case ExplainoAxisParamSlot::symmetry_tension:
        return &params.symmetry_tension;
    case ExplainoAxisParamSlot::field_curvature:
        return &params.field_curvature;
    }
    return nullptr;
}

inline float* ResolveExplainoCouplingValue(KernelParams& params, ExplainoCouplingParamSlot slot) {
    switch (slot) {
    case ExplainoCouplingParamSlot::momentum_beta:
        return &params.momentum_beta;
    case ExplainoCouplingParamSlot::joy_coupling:
        return &params.joy_coupling;
    case ExplainoCouplingParamSlot::fold_coupling:
        return &params.fold_coupling;
    case ExplainoCouplingParamSlot::bell_coupling:
        return &params.bell_coupling;
    }
    return nullptr;
}

inline const float* ResolveExplainoCouplingValue(const KernelParams& params, ExplainoCouplingParamSlot slot) {
    switch (slot) {
    case ExplainoCouplingParamSlot::momentum_beta:
        return &params.momentum_beta;
    case ExplainoCouplingParamSlot::joy_coupling:
        return &params.joy_coupling;
    case ExplainoCouplingParamSlot::fold_coupling:
        return &params.fold_coupling;
    case ExplainoCouplingParamSlot::bell_coupling:
        return &params.bell_coupling;
    }
    return nullptr;
}

inline float* ResolvePhoenixStepCarrierValue(KernelParams& params, PhoenixStepCarrierParamSlot slot) {
    switch (slot) {
    case PhoenixStepCarrierParamSlot::phoenix_p_real:
        return &params.phoenix_p_real;
    }
    return nullptr;
}

inline const float* ResolvePhoenixStepCarrierValue(const KernelParams& params, PhoenixStepCarrierParamSlot slot) {
    switch (slot) {
    case PhoenixStepCarrierParamSlot::phoenix_p_real:
        return &params.phoenix_p_real;
    }
    return nullptr;
}

inline float* ResolveExplainoStructuralValue(KernelParams& params, ExplainoStructuralParamSlot slot) {
    switch (slot) {
    case ExplainoStructuralParamSlot::explaino_cluster_radius:
        return &params.explaino_cluster_radius;
    }
    return nullptr;
}

inline const float* ResolveExplainoStructuralValue(const KernelParams& params, ExplainoStructuralParamSlot slot) {
    switch (slot) {
    case ExplainoStructuralParamSlot::explaino_cluster_radius:
        return &params.explaino_cluster_radius;
    }
    return nullptr;
}

inline void ResetExplainoAxisRegistryValues(KernelParams& params) {
    for (const auto& axis : kExplainoAxisRegistry) {
        float* value = ResolveExplainoAxisValue(params, axis.slot);
        if (value) {
            *value = 0.0f;
        }
    }
}

inline void ApplyExplainoAxisRegistryDefaults(FractalType fractalType, KernelParams& params) {
    ResetExplainoAxisRegistryValues(params);
    for (const auto& axis : kExplainoAxisRegistry) {
        if (axis.carrier_fractal_type == fractalType) {
            float* value = ResolveExplainoAxisValue(params, axis.slot);
            if (value) {
                *value = axis.default_value;
            }
        }
    }
}

inline void ResetExplainoCouplingRegistryValues(KernelParams& params) {
    for (const auto& coupling : kExplainoCouplingRegistry) {
        float* value = ResolveExplainoCouplingValue(params, coupling.slot);
        if (value) {
            *value = coupling.neutral_value;
        }
    }
}

inline void ApplyExplainoCouplingRegistryDefaults(FractalType fractalType, KernelParams& params) {
    ResetExplainoCouplingRegistryValues(params);
    const ExplainoCouplingDescriptor* coupling = FindExplainoCouplingDescriptor(fractalType);
    if (!coupling) {
        return;
    }
    float* value = ResolveExplainoCouplingValue(params, coupling->slot);
    if (value) {
        *value = coupling->default_value;
    }
}

inline void ResetPhoenixStepCarrierValues(KernelParams& params) {
    for (const auto& carrier : kPhoenixStepCarrierRegistry) {
        float* value = ResolvePhoenixStepCarrierValue(params, carrier.slot);
        if (value) {
            *value = 0.0f;
        }
    }
}

inline void ApplyPhoenixStepCarrierDefaults(FractalType fractalType, KernelParams& params) {
    ResetPhoenixStepCarrierValues(params);
    const PhoenixStepCarrierSelectorDescriptor* carrier = FindPhoenixStepCarrierSelectorDescriptor(fractalType);
    if (!carrier) {
        return;
    }
    float* value = ResolvePhoenixStepCarrierValue(params, carrier->slot);
    if (value) {
        *value = static_cast<float>(carrier->default_value);
    }
}

inline void ResetExplainoStructuralRegistryValues(KernelParams& params) {
    for (const auto& structural : kExplainoStructuralRegistry) {
        float* value = ResolveExplainoStructuralValue(params, structural.slot);
        if (value) {
            *value = 0.0f;
        }
    }
}

inline void ApplyExplainoStructuralRegistryDefaults(FractalType fractalType, KernelParams& params) {
    ResetExplainoStructuralRegistryValues(params);
    const ExplainoStructuralCarrierDescriptor* carrier = FindExplainoStructuralCarrierDescriptor(fractalType);
    if (!carrier) {
        return;
    }
    float* value = ResolveExplainoStructuralValue(params, carrier->slot);
    if (value) {
        *value = static_cast<float>(carrier->default_value);
    }
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool IsExplainoFamily(FractalType fractalType) {
    switch (fractalType) {
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
    case FractalType::explaino_collatz:
    case FractalType::explaino_lambda:
    case FractalType::explaino_rational_escape:
    case FractalType::explaino_joy:
    case FractalType::explaino_fold:
    case FractalType::explaino_bell:
    case FractalType::explaino_ripple:
    case FractalType::explaino_splice:
    case FractalType::explaino_vortex:
    case FractalType::explaino_tension:
    case FractalType::explaino_balance_void:
        return true;
    default:
        return false;
    }
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool SupportsBasinColoring(FractalType fractalType) {
    return fractalType == FractalType::newton || fractalType == ExplainoCanonicalFractalType() ||
        fractalType == FractalType::explaino ||
        fractalType == FractalType::explaino_y || fractalType == FractalType::explaino_fp ||
        fractalType == FractalType::explaino_halley || fractalType == FractalType::explaino_dual ||
        fractalType == FractalType::explaino_mult || fractalType == FractalType::explaino_phoenix ||
        fractalType == FractalType::explaino_transcendental || fractalType == FractalType::explaino_inertial ||
        fractalType == FractalType::explaino_rational || fractalType == FractalType::explaino_collatz ||
        fractalType == FractalType::explaino_joy || fractalType == FractalType::explaino_fold ||
        fractalType == FractalType::explaino_bell || fractalType == FractalType::explaino_ripple ||
        fractalType == FractalType::explaino_splice || fractalType == FractalType::explaino_vortex ||
        fractalType == FractalType::explaino_tension || fractalType == FractalType::explaino_balance_void ||
        fractalType == FractalType::halley;
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool IsEscapeTimeFamily(FractalType fractalType) {
    return fractalType == FractalType::nova || fractalType == FractalType::mandelbrot ||
        fractalType == FractalType::julia || fractalType == FractalType::burning_ship ||
        fractalType == FractalType::multibrot || fractalType == FractalType::phoenix ||
        fractalType == FractalType::explaino_nova || fractalType == FractalType::explaino_julia ||
        fractalType == FractalType::multicorn || fractalType == FractalType::collatz ||
        fractalType == FractalType::mcmullen || fractalType == FractalType::lambda_map ||
        fractalType == FractalType::explaino_lambda ||
        fractalType == FractalType::explaino_rational_escape ||
        fractalType == FractalType::spider ||
        fractalType == FractalType::celtic_mandelbrot ||
        fractalType == FractalType::perpendicular_burning_ship;
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool LensMaskInsideForFractal(FractalType fractalType, bool converged, bool escaped) {
    if (SupportsBasinColoring(fractalType)) return converged;
    if (IsEscapeTimeFamily(fractalType)) return !escaped;
    return false;
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool IsColoringModeAllowedForFractal(FractalType fractalType, ColoringMode mode) {
    if (SupportsBasinColoring(fractalType)) return true;
    return mode != ColoringMode::root_basin && mode != ColoringMode::joy_basins;
}

FRACTAL_FAMILY_RULES_HD inline constexpr ColoringMode DefaultColoringModeForFractal(FractalType fractalType) {
    return SupportsBasinColoring(fractalType) ? ColoringMode::joy_basins : ColoringMode::smooth_escape;
}

FRACTAL_FAMILY_RULES_HD inline constexpr ColorPipelineSelection ColorPipelineForLegacyMode(ColoringMode mode) {
    switch (mode) {
    case ColoringMode::root_basin:
        return {ColorSignal::root_index, ColorPalette::root_classic, ColorGradingPreset::basin_default};
    case ColoringMode::iteration_count:
        return {ColorSignal::iteration_count, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
    case ColoringMode::smooth_escape:
        return {ColorSignal::smooth_escape, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
    case ColoringMode::joy_basins:
        return {ColorSignal::root_index, ColorPalette::joy, ColorGradingPreset::basin_default};
    case ColoringMode::phase:
        return {ColorSignal::phase_angle, ColorPalette::phase_wheel, ColorGradingPreset::phase_default};
    case ColoringMode::iteration_bands:
        return {ColorSignal::iteration_bands, ColorPalette::banded_escape, ColorGradingPreset::bands_default};
    }
    return {};
}

FRACTAL_FAMILY_RULES_HD inline constexpr ColorPipelineSelection DefaultColorPipelineForFractal(FractalType fractalType) {
    return ColorPipelineForLegacyMode(DefaultColoringModeForFractal(fractalType));
}

inline constexpr ColorPipelineSelection kSelectableColorPipelines[] = {
    ColorPipelineForLegacyMode(ColoringMode::root_basin),
    ColorPipelineForLegacyMode(ColoringMode::iteration_count),
    ColorPipelineForLegacyMode(ColoringMode::smooth_escape),
    ColorPipelineForLegacyMode(ColoringMode::joy_basins),
    ColorPipelineForLegacyMode(ColoringMode::phase),
    ColorPipelineForLegacyMode(ColoringMode::iteration_bands),
    {ColorSignal::escape_magnitude, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default},
    {ColorSignal::orbit_stripe, ColorPalette::phase_wheel, ColorGradingPreset::phase_default},
    {ColorSignal::root_proximity, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default},
    {ColorSignal::smooth_escape, ColorPalette::explaino_cmap, ColorGradingPreset::escape_default},
    {ColorSignal::escape_magnitude, ColorPalette::explaino_cmap, ColorGradingPreset::escape_default},
    {ColorSignal::root_proximity, ColorPalette::explaino_cmap, ColorGradingPreset::escape_default},
};

FRACTAL_FAMILY_RULES_HD inline constexpr bool TryLegacyColoringModeForPipeline(
    const ColorPipelineSelection& pipeline,
    ColoringMode* outMode) {
    if (pipeline.signal == ColorSignal::root_index &&
        pipeline.palette == ColorPalette::root_classic &&
        pipeline.grading == ColorGradingPreset::basin_default) {
        if (outMode) *outMode = ColoringMode::root_basin;
        return true;
    }
    const bool isEscapeLikeGrading = pipeline.grading == ColorGradingPreset::escape_default ||
        pipeline.grading == ColorGradingPreset::neutral_default ||
        pipeline.grading == ColorGradingPreset::tone_map_default ||
        pipeline.grading == ColorGradingPreset::glow_default ||
        pipeline.grading == ColorGradingPreset::balance_void_default;
    const bool isPhaseLikeGrading = pipeline.grading == ColorGradingPreset::phase_default ||
        pipeline.grading == ColorGradingPreset::neutral_default ||
        pipeline.grading == ColorGradingPreset::balance_void_default;
    const bool isBandLikeGrading = pipeline.grading == ColorGradingPreset::bands_default ||
        pipeline.grading == ColorGradingPreset::neutral_default ||
        pipeline.grading == ColorGradingPreset::balance_void_default;
    if (pipeline.signal == ColorSignal::iteration_count &&
        pipeline.palette == ColorPalette::cyclic_escape &&
        isEscapeLikeGrading) {
        if (outMode) *outMode = ColoringMode::iteration_count;
        return true;
    }
    if (pipeline.signal == ColorSignal::smooth_escape &&
        pipeline.palette == ColorPalette::cyclic_escape &&
        isEscapeLikeGrading) {
        if (outMode) *outMode = ColoringMode::smooth_escape;
        return true;
    }
    if (pipeline.signal == ColorSignal::root_index &&
        pipeline.palette == ColorPalette::joy &&
        pipeline.grading == ColorGradingPreset::basin_default) {
        if (outMode) *outMode = ColoringMode::joy_basins;
        return true;
    }
    if (pipeline.signal == ColorSignal::phase_angle &&
        pipeline.palette == ColorPalette::phase_wheel &&
        isPhaseLikeGrading) {
        if (outMode) *outMode = ColoringMode::phase;
        return true;
    }
    if (pipeline.signal == ColorSignal::iteration_bands &&
        pipeline.palette == ColorPalette::banded_escape &&
        isBandLikeGrading) {
        if (outMode) *outMode = ColoringMode::iteration_bands;
        return true;
    }
    return false;
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool IsColorSignalAllowedForFractal(
    FractalType fractalType,
    ColorSignal signal) {
    if (signal == ColorSignal::root_index || signal == ColorSignal::root_proximity) {
        return SupportsBasinColoring(fractalType);
    }
    return true;
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool TryMirroredColoringModeForPipeline(
    const ColorPipelineSelection& pipeline,
    ColoringMode* outMode) {
    if (TryLegacyColoringModeForPipeline(pipeline, outMode)) {
        return true;
    }
    const bool isEscapeLikeGrading = pipeline.grading == ColorGradingPreset::escape_default ||
        pipeline.grading == ColorGradingPreset::neutral_default ||
        pipeline.grading == ColorGradingPreset::tone_map_default ||
        pipeline.grading == ColorGradingPreset::glow_default ||
        pipeline.grading == ColorGradingPreset::balance_void_default;
    const bool isPhaseLikeGrading = pipeline.grading == ColorGradingPreset::phase_default ||
        pipeline.grading == ColorGradingPreset::neutral_default ||
        pipeline.grading == ColorGradingPreset::balance_void_default;
    if (pipeline.signal == ColorSignal::escape_magnitude &&
        pipeline.palette == ColorPalette::cyclic_escape &&
        isEscapeLikeGrading) {
        if (outMode) *outMode = ColoringMode::smooth_escape;
        return true;
    }
    if (pipeline.signal == ColorSignal::orbit_stripe &&
        pipeline.palette == ColorPalette::phase_wheel &&
        isPhaseLikeGrading) {
        if (outMode) *outMode = ColoringMode::phase;
        return true;
    }
    if (pipeline.signal == ColorSignal::root_proximity &&
        pipeline.palette == ColorPalette::cyclic_escape &&
        isEscapeLikeGrading) {
        if (outMode) *outMode = ColoringMode::smooth_escape;
        return true;
    }
    if (pipeline.signal == ColorSignal::smooth_escape &&
        pipeline.palette == ColorPalette::explaino_cmap &&
        isEscapeLikeGrading) {
        if (outMode) *outMode = ColoringMode::smooth_escape;
        return true;
    }
    if (pipeline.signal == ColorSignal::escape_magnitude &&
        pipeline.palette == ColorPalette::explaino_cmap &&
        isEscapeLikeGrading) {
        if (outMode) *outMode = ColoringMode::smooth_escape;
        return true;
    }
    if (pipeline.signal == ColorSignal::root_proximity &&
        pipeline.palette == ColorPalette::explaino_cmap &&
        isEscapeLikeGrading) {
        if (outMode) *outMode = ColoringMode::smooth_escape;
        return true;
    }
    return false;
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool IsColorPipelineAllowedForFractal(
    FractalType fractalType,
    const ColorPipelineSelection& pipeline) {
    ColoringMode mode = ColoringMode::root_basin;
    return IsColorSignalAllowedForFractal(fractalType, pipeline.signal) &&
        TryMirroredColoringModeForPipeline(pipeline, &mode) &&
        IsColoringModeAllowedForFractal(fractalType, mode);
}

FRACTAL_FAMILY_RULES_HD inline constexpr bool DefaultAutoMaxIterForFractal(FractalType fractalType) {
    return fractalType == FractalType::nova || fractalType == FractalType::explaino_nova;
}

// Auto max-iter: scale iterations with zoom depth and fractal family.
// Escape-time fractals need more iterations at depth; basin types less so.
inline int ComputeAutoMaxIter(double log2_zoom, FractalType fractalType) {
    double depth = log2_zoom < 0.0 ? -log2_zoom : log2_zoom;
    int base, scale;
    if (fractalType == FractalType::collatz || fractalType == FractalType::explaino_collatz) {
        base = 300; scale = 80;
    } else if (fractalType == FractalType::nova || fractalType == FractalType::explaino_nova) {
        base = 400; scale = 120;
    } else if (IsEscapeTimeFamily(fractalType)) {
        base = 200; scale = 50;
    } else {
        base = 150; scale = 30;
    }
    int result = base + static_cast<int>(scale * depth);
    if (result < 100) result = 100;
    if (result > 5000) result = 5000;
    return result;
}

#undef FRACTAL_FAMILY_RULES_HD
