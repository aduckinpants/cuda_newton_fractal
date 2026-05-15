#include "../src/fractal_family_rules.h"
#include "../src/enum_id_utils.h"

#include <cstddef>
#include <iostream>
#include <string_view>

int main() {
    {
        if (!IsEscapeTimeFamily(FractalType::nova)) {
            std::cerr << "Nova should be classified as an escape-time family\n";
            return 1;
        }
        if (SupportsBasinColoring(FractalType::nova)) {
            std::cerr << "Nova should not support basin coloring\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::nova) != ColoringMode::smooth_escape) {
            std::cerr << "Nova should default to smooth_escape\n";
            return 1;
        }
        if (IsColoringModeAllowedForFractal(FractalType::nova, ColoringMode::root_basin)) {
            std::cerr << "Nova should reject root_basin coloring\n";
            return 1;
        }
        if (IsColoringModeAllowedForFractal(FractalType::nova, ColoringMode::joy_basins)) {
            std::cerr << "Nova should reject joy_basins coloring\n";
            return 1;
        }
        if (!IsColoringModeAllowedForFractal(FractalType::nova, ColoringMode::iteration_count) ||
            !IsColoringModeAllowedForFractal(FractalType::nova, ColoringMode::smooth_escape)) {
            std::cerr << "Nova should allow iteration_count and smooth_escape\n";
            return 1;
        }
        if (!DefaultAutoMaxIterForFractal(FractalType::nova)) {
            std::cerr << "Nova should default auto_max_iter on\n";
            return 1;
        }
        if (ComputeAutoMaxIter(10.0, FractalType::nova) != 1600) {
            std::cerr << "Nova auto max-iter curve should be tuned for medium-depth escape-time zooms\n";
            return 1;
        }
    }

    {
        if (!SupportsBasinColoring(FractalType::newton)) {
            std::cerr << "Newton should support basin coloring\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::newton) != ColoringMode::joy_basins) {
            std::cerr << "Newton should default to joy_basins\n";
            return 1;
        }
        if (!LensMaskInsideForFractal(FractalType::newton, true, false)) {
            std::cerr << "Newton lens mask should treat converged pixels as inside\n";
            return 1;
        }
        if (LensMaskInsideForFractal(FractalType::newton, false, false)) {
            std::cerr << "Newton lens mask should reject non-converged pixels\n";
            return 1;
        }
        const ColorPipelineSelection pipeline = DefaultColorPipelineForFractal(FractalType::newton);
        if (pipeline.signal != ColorSignal::root_index) {
            std::cerr << "Newton should default to the root_index signal\n";
            return 1;
        }
        if (pipeline.palette != ColorPalette::joy) {
            std::cerr << "Newton should default to the joy palette\n";
            return 1;
        }
        if (pipeline.grading != ColorGradingPreset::basin_default) {
            std::cerr << "Newton should default to basin_default grading\n";
            return 1;
        }
    }

    {
        if (!SupportsBasinColoring(FractalType::explaino_fp)) {
            std::cerr << "Explaino FP should support basin coloring\n";
            return 1;
        }
        if (!IsExplainoFamily(FractalType::explaino_y)) {
            std::cerr << "Explaino Y should stay in the Explaino family\n";
            return 1;
        }
        if (!IsExplainoFamily(FractalType::explaino_all)) {
            std::cerr << "Explaino-all should already be a real Explaino family identity, not just a planned selector label\n";
            return 1;
        }
        if (!SupportsBasinColoring(FractalType::explaino_all)) {
            std::cerr << "Explaino-all should inherit the basin-capable Explaino family baseline\n";
            return 1;
        }
        if (ExplainoCanonicalFractalType() != FractalType::explaino_all) {
            std::cerr << "Explaino-all should be the canonical Explaino family identity\n";
            return 1;
        }
        if (!IsExplainoLegacyProjectionSelector(FractalType::explaino_ripple) ||
            !IsExplainoLegacyProjectionSelector(FractalType::explaino_splice) ||
            !IsExplainoLegacyProjectionSelector(FractalType::explaino_vortex) ||
            !IsExplainoLegacyProjectionSelector(FractalType::explaino_tension) ||
            !IsExplainoLegacyProjectionSelector(FractalType::explaino_balance_void) ||
            IsExplainoLegacyProjectionSelector(FractalType::explaino_dual)) {
            std::cerr << "Explaino-all slice 2 should fence projection selectors to the seven-axis legacy family only\n";
            return 1;
        }
        if (ResolveExplainoPublicFractalType(FractalType::explaino_ripple) != FractalType::explaino_all ||
            ResolveExplainoPublicFractalType(FractalType::explaino_balance_void) != FractalType::explaino_all ||
            ResolveExplainoPublicFractalType(FractalType::explaino_dual) != FractalType::explaino_dual) {
            std::cerr << "Explaino-all slice 2 should project only the legacy seven-axis selectors back to the canonical explaino_all public identity\n";
            return 1;
        }
        struct ExpectedAxis {
            const char* axis_id;
            const char* binding_path;
            FractalType carrier_fractal_type;
            float default_value;
        };
        constexpr ExpectedAxis kExpectedAxes[] = {
            {"ripple_amplitude", "fractal.params.ripple_amplitude", FractalType::explaino_ripple, 0.15f},
            {"splice_offset", "fractal.params.splice_offset", FractalType::explaino_splice, 0.5f},
            {"vortex_strength", "fractal.params.vortex_strength", FractalType::explaino_vortex, 0.3f},
            {"tension_strength", "fractal.params.tension_strength", FractalType::explaino_tension, 0.02f},
            {"balance_void", "fractal.params.balance_void", FractalType::explaino_balance_void, 0.0f},
            {"symmetry_tension", "fractal.params.symmetry_tension", FractalType::explaino_balance_void, 0.0f},
            {"field_curvature", "fractal.params.field_curvature", FractalType::explaino_balance_void, 0.0f},
        };
        if ((sizeof(kExplainoAxisRegistry) / sizeof(kExplainoAxisRegistry[0])) != (sizeof(kExpectedAxes) / sizeof(kExpectedAxes[0]))) {
            std::cerr << "Explaino-all should expose exactly one canonical seven-axis registry\n";
            return 1;
        }
        for (std::size_t index = 0; index < (sizeof(kExpectedAxes) / sizeof(kExpectedAxes[0])); ++index) {
            const auto& axis = kExplainoAxisRegistry[index];
            if (std::string_view(axis.axis_id) != kExpectedAxes[index].axis_id ||
                std::string_view(axis.binding_path) != kExpectedAxes[index].binding_path ||
                axis.carrier_fractal_type != kExpectedAxes[index].carrier_fractal_type ||
                axis.default_value != kExpectedAxes[index].default_value) {
                std::cerr << "Explaino-all canonical axis registry entry " << index << " drifted from the expected shared authority\n";
                return 1;
            }
        }

        std::size_t explainoSelectorRegistryMatches = 0;
        std::size_t projectionSelectorCount = 0;
        std::size_t singleAxisProjectionCount = 0;
        for (const auto& pair : enum_id_utils::kFractalTypeIds) {
            const std::string_view fractalTypeId = pair.id;
            const bool isExplainoSelector = fractalTypeId == "explaino" ||
                fractalTypeId.compare(0, 9, "explaino_") == 0;
            if (!isExplainoSelector) {
                continue;
            }

            const ExplainoSelectorDescriptor* descriptor = FindExplainoSelectorDescriptor(pair.value);
            if (!descriptor || std::string_view(descriptor->fractal_type_id) != fractalTypeId) {
                std::cerr << "Every checked-in Explaino selector enum id should be classified by one canonical Explaino selector registry\n";
                return 1;
            }
            ++explainoSelectorRegistryMatches;
            const bool expectLegacyProjection =
                descriptor->role == ExplainoSelectorRole::legacy_projection_single_axis ||
                descriptor->role == ExplainoSelectorRole::legacy_projection_multi_axis;
            const bool expectSingleAxisProjection =
                descriptor->role == ExplainoSelectorRole::legacy_projection_single_axis;
            if (!IsExplainoFamily(pair.value) ||
                IsExplainoLegacyProjectionSelector(pair.value) != expectLegacyProjection ||
                IsExplainoSingleAxisProjectionSelector(pair.value) != expectSingleAxisProjection ||
                IsExplainoComposedAxisCarrier(pair.value) != expectSingleAxisProjection) {
                std::cerr << "Explaino-all slice 3 should keep the CUDA-safe Explaino classifiers in lockstep with the canonical selector registry\n";
                return 1;
            }
            if (expectLegacyProjection) {
                ++projectionSelectorCount;
            }
            if (expectSingleAxisProjection) {
                const ExplainoAxisDescriptor* axis = FindExplainoSingleAxisProjectionDescriptor(pair.value);
                if (!axis || axis->carrier_fractal_type != pair.value) {
                    std::cerr << "Every single-axis Explaino projection selector should derive helper coverage from the canonical axis registry\n";
                    return 1;
                }
                ++singleAxisProjectionCount;
            }
        }
        if (explainoSelectorRegistryMatches != (sizeof(kExplainoSelectorRegistry) / sizeof(kExplainoSelectorRegistry[0]))) {
            std::cerr << "Explaino selector classification should cover every checked-in Explaino selector exactly once\n";
            return 1;
        }
        if (projectionSelectorCount != 5 || singleAxisProjectionCount != 4 ||
            FindExplainoSingleAxisProjectionDescriptor(FractalType::explaino_balance_void) != nullptr) {
            std::cerr << "Explaino-all slice 3 should distinguish the four single-axis projection selectors from the multi-axis ExplainO-BalanceVoid projection selector\n";
            return 1;
        }
    }

    {
        struct ExpectedCoupling {
            const char* param_id;
            const char* binding_path;
            FractalType carrier_fractal_type;
            float default_value;
            ExplainoCouplingOwnership ownership;
            ExplainoCouplingModel model;
            bool zero_collapses_to_baseline;
            bool requires_root_pack_modifier;
        };
        constexpr ExpectedCoupling kExpectedCouplings[] = {
            {"momentum_beta", "fractal.params.momentum_beta", FractalType::explaino_inertial, 0.15f, ExplainoCouplingOwnership::different_ownership_model, ExplainoCouplingModel::inertial_memory, true, false},
            {"joy_coupling", "fractal.params.joy_coupling", FractalType::explaino_joy, 0.3f, ExplainoCouplingOwnership::different_ownership_model, ExplainoCouplingModel::phoenix_step_variant, false, true},
            {"fold_coupling", "fractal.params.fold_coupling", FractalType::explaino_fold, 0.5f, ExplainoCouplingOwnership::different_ownership_model, ExplainoCouplingModel::phoenix_step_variant, false, true},
            {"bell_coupling", "fractal.params.bell_coupling", FractalType::explaino_bell, 0.5f, ExplainoCouplingOwnership::different_ownership_model, ExplainoCouplingModel::phoenix_step_variant, false, true},
        };
        if ((sizeof(kExplainoCouplingRegistry) / sizeof(kExplainoCouplingRegistry[0])) !=
            (sizeof(kExpectedCouplings) / sizeof(kExpectedCouplings[0]))) {
            std::cerr << "Deferred Explaino couplings should carry one explicit checked-in ownership registry\n";
            return 1;
        }
        KernelParams couplingParams{};
        std::size_t phoenixCouplingCount = 0;
        for (std::size_t index = 0; index < (sizeof(kExpectedCouplings) / sizeof(kExpectedCouplings[0])); ++index) {
            const auto& coupling = kExplainoCouplingRegistry[index];
            if (std::string_view(coupling.param_id) != kExpectedCouplings[index].param_id ||
                std::string_view(coupling.binding_path) != kExpectedCouplings[index].binding_path ||
                coupling.carrier_fractal_type != kExpectedCouplings[index].carrier_fractal_type ||
                coupling.default_value != kExpectedCouplings[index].default_value ||
                coupling.neutral_value != 0.0f ||
                coupling.ownership != kExpectedCouplings[index].ownership ||
                coupling.model != kExpectedCouplings[index].model ||
                coupling.zero_collapses_to_baseline != kExpectedCouplings[index].zero_collapses_to_baseline ||
                coupling.requires_root_pack_modifier != kExpectedCouplings[index].requires_root_pack_modifier) {
                std::cerr << "Deferred Explaino coupling registry entry " << index << " drifted from the bounded ownership answer\n";
                return 1;
            }
            const ExplainoCouplingDescriptor* byParam = FindExplainoCouplingDescriptor(coupling.param_id);
            const ExplainoCouplingDescriptor* byBinding = FindExplainoCouplingDescriptorByBindingPath(coupling.binding_path);
            const ExplainoCouplingDescriptor* byCarrier = FindExplainoCouplingDescriptor(coupling.carrier_fractal_type);
            const ExplainoSelectorDescriptor* carrierSelector = FindExplainoSelectorDescriptor(coupling.carrier_fractal_type);
            if (byParam != &coupling || byBinding != &coupling || byCarrier != &coupling ||
                !carrierSelector || carrierSelector->role != ExplainoSelectorRole::legacy_family_nonprojection ||
                ResolveExplainoPublicFractalType(coupling.carrier_fractal_type) != coupling.carrier_fractal_type) {
                std::cerr << "Deferred Explaino couplings should stay legacy-family carriers with one canonical ownership table\n";
                return 1;
            }
            float* value = ResolveExplainoCouplingValue(couplingParams, coupling.slot);
            if (!value) {
                std::cerr << "Deferred Explaino coupling registry should resolve every KernelParams slot\n";
                return 1;
            }
            *value = coupling.default_value;
            const float* constValue = ResolveExplainoCouplingValue(static_cast<const KernelParams&>(couplingParams), coupling.slot);
            if (!constValue || *constValue != coupling.default_value) {
                std::cerr << "Deferred Explaino coupling registry should expose both mutable and const KernelParams slot access\n";
                return 1;
            }
            if (coupling.requires_root_pack_modifier) {
                ++phoenixCouplingCount;
            }
        }
        ResetExplainoCouplingRegistryValues(couplingParams);
        for (const auto& coupling : kExplainoCouplingRegistry) {
            const float* value = ResolveExplainoCouplingValue(static_cast<const KernelParams&>(couplingParams), coupling.slot);
            if (!value || *value != 0.0f) {
                std::cerr << "Deferred Explaino coupling reset should collapse every owned slot back to neutral zero\n";
                return 1;
            }
        }
        if (phoenixCouplingCount != 3u) {
            std::cerr << "Deferred Explaino couplings should split into one inertial-memory entry plus three phoenix-step entries\n";
            return 1;
        }
    }

    {
        if (!IsExplainoFamily(FractalType::explaino_balance_void)) {
            std::cerr << "Explaino-BalanceVoid should stay in the Explaino family surface instead of existing only as generic grading intent\n";
            return 1;
        }
        if (!SupportsBasinColoring(FractalType::explaino_balance_void)) {
            std::cerr << "Explaino-BalanceVoid should support basin coloring as a root-finding family track\n";
            return 1;
        }
        if (IsEscapeTimeFamily(FractalType::explaino_balance_void)) {
            std::cerr << "Explaino-BalanceVoid should remain a basin-capable Explaino family track, not an escape-time generic grading alias\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::explaino_balance_void) != ColoringMode::joy_basins) {
            std::cerr << "Explaino-BalanceVoid should inherit the neutral Explaino basin-default coloring mode\n";
            return 1;
        }
        const ColorPipelineSelection pipeline = DefaultColorPipelineForFractal(FractalType::explaino_balance_void);
        if (pipeline.signal != ColorSignal::root_index ||
            pipeline.palette != ColorPalette::joy ||
            pipeline.grading != ColorGradingPreset::basin_default) {
            std::cerr << "Explaino-BalanceVoid should default to the neutral Explaino basin pipeline instead of widening into generic balance_void_grade ownership\n";
            return 1;
        }
    }

    {
        if (!IsExplainoFamily(FractalType::explaino_nova)) {
            std::cerr << "Explaino-Nova should stay in the Explaino family surface\n";
            return 1;
        }
        if (!IsEscapeTimeFamily(FractalType::explaino_nova)) {
            std::cerr << "Explaino-Nova should be classified as an escape-time family\n";
            return 1;
        }
        if (SupportsBasinColoring(FractalType::explaino_nova)) {
            std::cerr << "Explaino-Nova should not support basin coloring\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::explaino_nova) != ColoringMode::smooth_escape) {
            std::cerr << "Explaino-Nova should default to smooth_escape\n";
            return 1;
        }
        if (IsColoringModeAllowedForFractal(FractalType::explaino_nova, ColoringMode::root_basin) ||
            IsColoringModeAllowedForFractal(FractalType::explaino_nova, ColoringMode::joy_basins)) {
            std::cerr << "Explaino-Nova should reject basin coloring modes\n";
            return 1;
        }
        if (!IsColoringModeAllowedForFractal(FractalType::explaino_nova, ColoringMode::iteration_count) ||
            !IsColoringModeAllowedForFractal(FractalType::explaino_nova, ColoringMode::smooth_escape)) {
            std::cerr << "Explaino-Nova should allow escape-time coloring modes\n";
            return 1;
        }
        if (!DefaultAutoMaxIterForFractal(FractalType::explaino_nova)) {
            std::cerr << "Explaino-Nova should default auto_max_iter on\n";
            return 1;
        }
        if (ComputeAutoMaxIter(10.0, FractalType::explaino_nova) != 1600) {
            std::cerr << "Explaino-Nova auto max-iter curve should match Nova\n";
            return 1;
        }
    }

    {
        ColoringMode mirroredMode = ColoringMode::root_basin;
        const ColorPipelineSelection smoothBalanceVoid = {ColorSignal::smooth_escape, ColorPalette::cyclic_escape, ColorGradingPreset::balance_void_default};
        if (!TryLegacyColoringModeForPipeline(smoothBalanceVoid, &mirroredMode) || mirroredMode != ColoringMode::smooth_escape) {
            std::cerr << "balance_void_grade should count as an escape-like grading owner on smooth escape tuples\n";
            return 1;
        }
        const ColorPipelineSelection phaseBalanceVoid = {ColorSignal::phase_angle, ColorPalette::phase_wheel, ColorGradingPreset::balance_void_default};
        if (!TryLegacyColoringModeForPipeline(phaseBalanceVoid, &mirroredMode) || mirroredMode != ColoringMode::phase) {
            std::cerr << "balance_void_grade should remain reusable on phase tuples instead of being trapped in escape-only grading logic\n";
            return 1;
        }
        const ColorPipelineSelection bandBalanceVoid = {ColorSignal::iteration_bands, ColorPalette::banded_escape, ColorGradingPreset::balance_void_default};
        if (!TryLegacyColoringModeForPipeline(bandBalanceVoid, &mirroredMode) || mirroredMode != ColoringMode::iteration_bands) {
            std::cerr << "balance_void_grade should remain reusable on banded tuples instead of collapsing the band runtime bridge\n";
            return 1;
        }
        const ColorPipelineSelection explainoBalanceVoid = {ColorSignal::smooth_escape, ColorPalette::explaino_cmap, ColorGradingPreset::balance_void_default};
        if (!TryMirroredColoringModeForPipeline(explainoBalanceVoid, &mirroredMode) || mirroredMode != ColoringMode::smooth_escape) {
            std::cerr << "balance_void_grade should remain reusable on explaino_cmap smooth-escape tuples through the mirrored runtime bridge\n";
            return 1;
        }
        if (!IsColorPipelineAllowedForFractal(FractalType::explaino_nova, explainoBalanceVoid)) {
            std::cerr << "balance_void_grade should stay available to escape-time Explaino tuples without widening into ExplainO-BalanceVoid family work\n";
            return 1;
        }
    }

    {
        if (!IsEscapeTimeFamily(FractalType::phoenix)) {
            std::cerr << "Phoenix should stay in the escape-time family\n";
            return 1;
        }
        if (IsColoringModeAllowedForFractal(FractalType::phoenix, ColoringMode::joy_basins)) {
            std::cerr << "Phoenix should reject joy_basins coloring\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::phoenix) != ColoringMode::smooth_escape) {
            std::cerr << "Phoenix should default to smooth_escape\n";
            return 1;
        }
        if (!LensMaskInsideForFractal(FractalType::phoenix, false, false)) {
            std::cerr << "Phoenix lens mask should treat bounded pixels as inside\n";
            return 1;
        }
        if (LensMaskInsideForFractal(FractalType::phoenix, false, true)) {
            std::cerr << "Phoenix lens mask should reject escaped pixels\n";
            return 1;
        }
        const ColorPipelineSelection pipeline = DefaultColorPipelineForFractal(FractalType::phoenix);
        if (pipeline.signal != ColorSignal::smooth_escape) {
            std::cerr << "Phoenix should default to the smooth_escape signal\n";
            return 1;
        }
        if (pipeline.palette != ColorPalette::cyclic_escape) {
            std::cerr << "Phoenix should default to the cyclic escape palette\n";
            return 1;
        }
        if (pipeline.grading != ColorGradingPreset::escape_default) {
            std::cerr << "Phoenix should default to escape_default grading\n";
            return 1;
        }
    }

    {
        if (!IsExplainoFamily(FractalType::explaino_halley)) {
            std::cerr << "Explaino-Halley should be in the Explaino family\n";
            return 1;
        }
        if (!SupportsBasinColoring(FractalType::explaino_halley)) {
            std::cerr << "Explaino-Halley should support basin coloring\n";
            return 1;
        }
        if (IsEscapeTimeFamily(FractalType::explaino_halley)) {
            std::cerr << "Explaino-Halley should not be escape-time\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::explaino_halley) != ColoringMode::joy_basins) {
            std::cerr << "Explaino-Halley should default to joy_basins\n";
            return 1;
        }
    }

    {
        struct ExpectedDualSeedParam {
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
        constexpr ExpectedDualSeedParam kExpectedDualSeedParams[] = {
            {"explaino_seed_b", "fractal.params.explaino_seed_b", FractalType::explaino_dual, 1.0, 0.0, ExplainoDualSeedOwnership::different_ownership_model, ExplainoDualSeedModel::secondary_seed_surface, false, false},
            {"explaino_mix", "fractal.params.explaino_mix", FractalType::explaino_dual, 0.5, 0.0, ExplainoDualSeedOwnership::different_ownership_model, ExplainoDualSeedModel::blend_gate, true, false},
        };
        if ((sizeof(kExplainoDualSeedRegistry) / sizeof(kExplainoDualSeedRegistry[0])) !=
            (sizeof(kExpectedDualSeedParams) / sizeof(kExpectedDualSeedParams[0]))) {
            std::cerr << "Deferred Explaino dual-seed controls should carry one explicit checked-in ownership registry\n";
            return 1;
        }
        const ExplainoSelectorDescriptor* dualCarrier = FindExplainoSelectorDescriptor(FractalType::explaino_dual);
        if (!dualCarrier || dualCarrier->role != ExplainoSelectorRole::legacy_family_nonprojection ||
            ResolveExplainoPublicFractalType(FractalType::explaino_dual) != FractalType::explaino_dual) {
            std::cerr << "Explaino dual-seed ownership should stay tied to the explaino_dual carrier until canonical proof says otherwise\n";
            return 1;
        }
        for (std::size_t index = 0; index < (sizeof(kExpectedDualSeedParams) / sizeof(kExpectedDualSeedParams[0])); ++index) {
            const auto& dualSeed = kExplainoDualSeedRegistry[index];
            if (std::string_view(dualSeed.param_id) != kExpectedDualSeedParams[index].param_id ||
                std::string_view(dualSeed.binding_path) != kExpectedDualSeedParams[index].binding_path ||
                dualSeed.carrier_fractal_type != kExpectedDualSeedParams[index].carrier_fractal_type ||
                dualSeed.default_value != kExpectedDualSeedParams[index].default_value ||
                dualSeed.neutral_value != kExpectedDualSeedParams[index].neutral_value ||
                dualSeed.ownership != kExpectedDualSeedParams[index].ownership ||
                dualSeed.model != kExpectedDualSeedParams[index].model ||
                dualSeed.zero_collapses_to_baseline != kExpectedDualSeedParams[index].zero_collapses_to_baseline ||
                dualSeed.default_collapses_to_baseline != kExpectedDualSeedParams[index].default_collapses_to_baseline) {
                std::cerr << "Deferred Explaino dual-seed registry entry " << index << " drifted from the bounded ownership answer\n";
                return 1;
            }
            const ExplainoDualSeedDescriptor* byParam = FindExplainoDualSeedDescriptor(dualSeed.param_id);
            const ExplainoDualSeedDescriptor* byBinding = FindExplainoDualSeedDescriptorByBindingPath(dualSeed.binding_path);
            if (byParam != &dualSeed || byBinding != &dualSeed) {
                std::cerr << "Deferred Explaino dual-seed ownership should resolve through one checked-in authority surface\n";
                return 1;
            }
        }
        for (const char* outOfScopeParamId : {
                "phoenix_p_real",
                "explaino_cluster_radius",
                "momentum_beta",
                "joy_coupling",
                "fold_coupling",
                "bell_coupling",
            }) {
            if (FindExplainoDualSeedDescriptor(outOfScopeParamId) != nullptr) {
                std::cerr << "Deferred Explaino dual-seed ownership must stay bounded and leave structural modifiers plus resolved couplings out of scope\n";
                return 1;
            }
        }
        for (const char* outOfScopeBindingPath : {
                "fractal.params.phoenix_p_real",
                "fractal.params.explaino_cluster_radius",
                "fractal.params.momentum_beta",
                "fractal.params.joy_coupling",
                "fractal.params.fold_coupling",
                "fractal.params.bell_coupling",
            }) {
            if (FindExplainoDualSeedDescriptorByBindingPath(outOfScopeBindingPath) != nullptr) {
                std::cerr << "Deferred Explaino dual-seed binding-path ownership must stay bounded and leave other deferred classes out of scope\n";
                return 1;
            }
        }
    }

    {
        if (!IsExplainoFamily(FractalType::explaino_dual)) {
            std::cerr << "Explaino-DualSeed should be in the Explaino family\n";
            return 1;
        }
        if (!SupportsBasinColoring(FractalType::explaino_dual)) {
            std::cerr << "Explaino-DualSeed should support basin coloring\n";
            return 1;
        }
        if (IsEscapeTimeFamily(FractalType::explaino_dual)) {
            std::cerr << "Explaino-DualSeed should not be escape-time\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::explaino_dual) != ColoringMode::joy_basins) {
            std::cerr << "Explaino-DualSeed should default to joy_basins\n";
            return 1;
        }
    }

    // Explaino-Julia: explaino family + escape-time + no basin coloring
    {
        if (!IsExplainoFamily(FractalType::explaino_julia)) {
            std::cerr << "Explaino-Julia should be in the Explaino family\n";
            return 1;
        }
        if (!IsEscapeTimeFamily(FractalType::explaino_julia)) {
            std::cerr << "Explaino-Julia should be escape-time\n";
            return 1;
        }
        if (SupportsBasinColoring(FractalType::explaino_julia)) {
            std::cerr << "Explaino-Julia should not support basin coloring\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::explaino_julia) != ColoringMode::smooth_escape) {
            std::cerr << "Explaino-Julia should default to smooth_escape\n";
            return 1;
        }
        if (IsColoringModeAllowedForFractal(FractalType::explaino_julia, ColoringMode::root_basin) ||
            IsColoringModeAllowedForFractal(FractalType::explaino_julia, ColoringMode::joy_basins)) {
            std::cerr << "Explaino-Julia should reject basin coloring modes\n";
            return 1;
        }
    }

    // Explaino-Rational: explaino family + basin coloring + not escape-time
    {
        if (!IsExplainoFamily(FractalType::explaino_rational)) {
            std::cerr << "Explaino-Rational should be in the Explaino family\n";
            return 1;
        }
        if (!SupportsBasinColoring(FractalType::explaino_rational)) {
            std::cerr << "Explaino-Rational should support basin coloring\n";
            return 1;
        }
        if (IsEscapeTimeFamily(FractalType::explaino_rational)) {
            std::cerr << "Explaino-Rational should not be escape-time\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::explaino_rational) != ColoringMode::joy_basins) {
            std::cerr << "Explaino-Rational should default to joy_basins\n";
            return 1;
        }
    }

    // Multicorn: not explaino + escape-time + no basin coloring
    {
        if (IsExplainoFamily(FractalType::multicorn)) {
            std::cerr << "Multicorn should not be in the Explaino family\n";
            return 1;
        }
        if (!IsEscapeTimeFamily(FractalType::multicorn)) {
            std::cerr << "Multicorn should be escape-time\n";
            return 1;
        }
        if (SupportsBasinColoring(FractalType::multicorn)) {
            std::cerr << "Multicorn should not support basin coloring\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::multicorn) != ColoringMode::smooth_escape) {
            std::cerr << "Multicorn should default to smooth_escape\n";
            return 1;
        }
    }

    {
        if (IsExplainoFamily(FractalType::spider)) {
            std::cerr << "Spider should not be in the Explaino family\n";
            return 1;
        }
        if (!IsEscapeTimeFamily(FractalType::spider)) {
            std::cerr << "Spider should be escape-time\n";
            return 1;
        }
        if (SupportsBasinColoring(FractalType::spider)) {
            std::cerr << "Spider should not support basin coloring\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::spider) != ColoringMode::smooth_escape) {
            std::cerr << "Spider should default to smooth_escape\n";
            return 1;
        }
    }

    {
        if (IsExplainoFamily(FractalType::celtic_mandelbrot)) {
            std::cerr << "Celtic Mandelbrot should not be in the Explaino family\n";
            return 1;
        }
        if (!IsEscapeTimeFamily(FractalType::celtic_mandelbrot)) {
            std::cerr << "Celtic Mandelbrot should be escape-time\n";
            return 1;
        }
        if (SupportsBasinColoring(FractalType::celtic_mandelbrot)) {
            std::cerr << "Celtic Mandelbrot should not support basin coloring\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::celtic_mandelbrot) != ColoringMode::smooth_escape) {
            std::cerr << "Celtic Mandelbrot should default to smooth_escape\n";
            return 1;
        }
    }

    {
        if (IsExplainoFamily(FractalType::perpendicular_burning_ship)) {
            std::cerr << "Perpendicular Burning Ship should not be in the Explaino family\n";
            return 1;
        }
        if (!IsEscapeTimeFamily(FractalType::perpendicular_burning_ship)) {
            std::cerr << "Perpendicular Burning Ship should be escape-time\n";
            return 1;
        }
        if (SupportsBasinColoring(FractalType::perpendicular_burning_ship)) {
            std::cerr << "Perpendicular Burning Ship should not support basin coloring\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::perpendicular_burning_ship) != ColoringMode::smooth_escape) {
            std::cerr << "Perpendicular Burning Ship should default to smooth_escape\n";
            return 1;
        }
    }

    // Halley: not explaino + basin coloring + not escape-time
    {
        if (IsExplainoFamily(FractalType::halley)) {
            std::cerr << "Halley should not be in the Explaino family\n";
            return 1;
        }
        if (!SupportsBasinColoring(FractalType::halley)) {
            std::cerr << "Halley should support basin coloring\n";
            return 1;
        }
        if (IsEscapeTimeFamily(FractalType::halley)) {
            std::cerr << "Halley should not be escape-time\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::halley) != ColoringMode::joy_basins) {
            std::cerr << "Halley should default to joy_basins\n";
            return 1;
        }
        if (!IsColoringModeAllowedForFractal(FractalType::halley, ColoringMode::root_basin)) {
            std::cerr << "Halley should allow root_basin coloring\n";
            return 1;
        }
    }

    // Collatz: not explaino + escape-time + no basin coloring
    {
        if (IsExplainoFamily(FractalType::collatz)) {
            std::cerr << "Collatz should not be in the Explaino family\n";
            return 1;
        }
        if (!IsEscapeTimeFamily(FractalType::collatz)) {
            std::cerr << "Collatz should be escape-time\n";
            return 1;
        }
        if (SupportsBasinColoring(FractalType::collatz)) {
            std::cerr << "Collatz should not support basin coloring\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::collatz) != ColoringMode::smooth_escape) {
            std::cerr << "Collatz should default to smooth_escape\n";
            return 1;
        }
        if (IsColoringModeAllowedForFractal(FractalType::collatz, ColoringMode::joy_basins)) {
            std::cerr << "Collatz should reject joy_basins coloring\n";
            return 1;
        }
    }

    // Explaino-Collatz: explaino family + basin coloring + not escape-time
    {
        if (!IsExplainoFamily(FractalType::explaino_collatz)) {
            std::cerr << "Explaino-Collatz should be in the Explaino family\n";
            return 1;
        }
        if (!SupportsBasinColoring(FractalType::explaino_collatz)) {
            std::cerr << "Explaino-Collatz should support basin coloring\n";
            return 1;
        }
        if (IsEscapeTimeFamily(FractalType::explaino_collatz)) {
            std::cerr << "Explaino-Collatz should not be escape-time\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::explaino_collatz) != ColoringMode::joy_basins) {
            std::cerr << "Explaino-Collatz should default to joy_basins\n";
            return 1;
        }
        if (!IsColoringModeAllowedForFractal(FractalType::explaino_collatz, ColoringMode::root_basin)) {
            std::cerr << "Explaino-Collatz should allow root_basin coloring\n";
            return 1;
        }
    }

    // McMullen: not explaino + escape-time + no basin coloring
    {
        if (IsExplainoFamily(FractalType::mcmullen)) {
            std::cerr << "McMullen should not be in the Explaino family\n";
            return 1;
        }
        if (!IsEscapeTimeFamily(FractalType::mcmullen)) {
            std::cerr << "McMullen should be escape-time\n";
            return 1;
        }
        if (SupportsBasinColoring(FractalType::mcmullen)) {
            std::cerr << "McMullen should not support basin coloring\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::mcmullen) != ColoringMode::smooth_escape) {
            std::cerr << "McMullen should default to smooth_escape\n";
            return 1;
        }
        if (IsColoringModeAllowedForFractal(FractalType::mcmullen, ColoringMode::joy_basins)) {
            std::cerr << "McMullen should reject joy_basins coloring\n";
            return 1;
        }
    }

    // Lambda: not explaino + escape-time + no basin coloring
    {
        if (IsExplainoFamily(FractalType::lambda_map)) {
            std::cerr << "Lambda should not be in the Explaino family\n";
            return 1;
        }
        if (!IsEscapeTimeFamily(FractalType::lambda_map)) {
            std::cerr << "Lambda should be escape-time\n";
            return 1;
        }
        if (SupportsBasinColoring(FractalType::lambda_map)) {
            std::cerr << "Lambda should not support basin coloring\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::lambda_map) != ColoringMode::smooth_escape) {
            std::cerr << "Lambda should default to smooth_escape\n";
            return 1;
        }
        if (IsColoringModeAllowedForFractal(FractalType::lambda_map, ColoringMode::joy_basins)) {
            std::cerr << "Lambda should reject joy_basins coloring\n";
            return 1;
        }
    }

    // Explaino-Lambda: explaino family + escape-time + no basin coloring
    {
        if (!IsExplainoFamily(FractalType::explaino_lambda)) {
            std::cerr << "Explaino-Lambda should be in the Explaino family\n";
            return 1;
        }
        if (!IsEscapeTimeFamily(FractalType::explaino_lambda)) {
            std::cerr << "Explaino-Lambda should be escape-time\n";
            return 1;
        }
        if (SupportsBasinColoring(FractalType::explaino_lambda)) {
            std::cerr << "Explaino-Lambda should not support basin coloring\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::explaino_lambda) != ColoringMode::smooth_escape) {
            std::cerr << "Explaino-Lambda should default to smooth_escape\n";
            return 1;
        }
        if (IsColoringModeAllowedForFractal(FractalType::explaino_lambda, ColoringMode::root_basin) ||
            IsColoringModeAllowedForFractal(FractalType::explaino_lambda, ColoringMode::joy_basins)) {
            std::cerr << "Explaino-Lambda should reject basin coloring modes\n";
            return 1;
        }
        if (!IsColoringModeAllowedForFractal(FractalType::explaino_lambda, ColoringMode::iteration_count) ||
            !IsColoringModeAllowedForFractal(FractalType::explaino_lambda, ColoringMode::smooth_escape)) {
            std::cerr << "Explaino-Lambda should allow escape-time coloring modes\n";
            return 1;
        }
        if (!LensMaskInsideForFractal(FractalType::explaino_lambda, false, false)) {
            std::cerr << "Explaino-Lambda lens mask should treat non-escaped pixels as inside\n";
            return 1;
        }
    }

    // Explaino-Rational-Escape: explaino family + escape-time + no basin coloring
    {
        if (!IsExplainoFamily(FractalType::explaino_rational_escape)) {
            std::cerr << "Explaino-Rational-Escape should be in the Explaino family\n";
            return 1;
        }
        if (!IsEscapeTimeFamily(FractalType::explaino_rational_escape)) {
            std::cerr << "Explaino-Rational-Escape should be escape-time\n";
            return 1;
        }
        if (SupportsBasinColoring(FractalType::explaino_rational_escape)) {
            std::cerr << "Explaino-Rational-Escape should not support basin coloring\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::explaino_rational_escape) != ColoringMode::smooth_escape) {
            std::cerr << "Explaino-Rational-Escape should default to smooth_escape\n";
            return 1;
        }
        if (IsColoringModeAllowedForFractal(FractalType::explaino_rational_escape, ColoringMode::root_basin) ||
            IsColoringModeAllowedForFractal(FractalType::explaino_rational_escape, ColoringMode::joy_basins)) {
            std::cerr << "Explaino-Rational-Escape should reject basin coloring modes\n";
            return 1;
        }
        if (!IsColoringModeAllowedForFractal(FractalType::explaino_rational_escape, ColoringMode::iteration_count) ||
            !IsColoringModeAllowedForFractal(FractalType::explaino_rational_escape, ColoringMode::smooth_escape)) {
            std::cerr << "Explaino-Rational-Escape should allow escape-time coloring modes\n";
            return 1;
        }
    }

    // ComputeAutoMaxIter tests
    {
        // At zoom 0 (no zoom): basin type gets base 150
        int iters = ComputeAutoMaxIter(0.0, FractalType::newton);
        if (iters != 150) {
            std::cerr << "Auto max iter for newton at zoom 0 should be 150, got " << iters << "\n";
            return 1;
        }

        // At zoom 0: escape type gets base 200
        iters = ComputeAutoMaxIter(0.0, FractalType::mandelbrot);
        if (iters != 200) {
            std::cerr << "Auto max iter for mandelbrot at zoom 0 should be 200, got " << iters << "\n";
            return 1;
        }

        // At zoom 0: collatz gets base 300
        iters = ComputeAutoMaxIter(0.0, FractalType::collatz);
        if (iters != 300) {
            std::cerr << "Auto max iter for collatz at zoom 0 should be 300, got " << iters << "\n";
            return 1;
        }

        // Deeper zoom increases iterations
        int shallow = ComputeAutoMaxIter(-2.0, FractalType::mandelbrot);
        int deep = ComputeAutoMaxIter(-20.0, FractalType::mandelbrot);
        if (deep <= shallow) {
            std::cerr << "Deeper zoom should produce more iterations\n";
            return 1;
        }

        // Clamped to 5000 max
        iters = ComputeAutoMaxIter(-200.0, FractalType::mandelbrot);
        if (iters != 5000) {
            std::cerr << "Auto max iter should clamp to 5000, got " << iters << "\n";
            return 1;
        }

        // Clamped to 100 min (shouldn't happen at positive values, but test the floor)
        iters = ComputeAutoMaxIter(0.0, FractalType::newton);
        if (iters < 100) {
            std::cerr << "Auto max iter should be at least 100, got " << iters << "\n";
            return 1;
        }
    }

    return 0;
}
