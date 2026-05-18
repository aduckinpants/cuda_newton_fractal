#include "../src/color_pipeline_core.h"
#include "../src/fractal_family_rules.h"
#include "../src/runtime_reset.h"

#include <iostream>

static bool NearlyEqual(float a, float b, float eps = 1.0e-6f) {
    float d = a - b;
    return d < eps && d > -eps;
}

int main() {
    {
        ViewState startupView{};
        if (startupView.fractal_type != FractalType::explaino) {
            std::cerr << "Fresh runtime state should default startup fractal selection to Explaino\n";
            return 1;
        }

        RenderSettings startupRender{};
        if (startupRender.resolution.x != 2048 || startupRender.resolution.y != 1536) {
            std::cerr << "Fresh runtime render state should default startup resolution to 2048x1536\n";
            return 1;
        }
    }

    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    LensSettings lens{};
    bool dirty = false;
    ColorGradingPreset neutralGrading = ColorGradingPreset::escape_default;
    if (!color_pipeline_core::TryParseAdvancedColorGradingFunctionId("neutral_finish", &neutralGrading)) {
        std::cerr << "Expected neutral_finish to parse before reset proof\n";
        return 1;
    }
    ColorGradingPreset toneMapGrading = ColorGradingPreset::escape_default;
    if (!color_pipeline_core::TryParseAdvancedColorGradingFunctionId("tone_map_finish", &toneMapGrading)) {
        std::cerr << "Expected tone_map_finish to parse before reset proof\n";
        return 1;
    }
    ColorGradingPreset gradeGlowGrading = ColorGradingPreset::escape_default;
    if (!color_pipeline_core::TryParseAdvancedColorGradingFunctionId("grade_glow", &gradeGlowGrading)) {
        std::cerr << "Expected grade_glow to parse before reset proof\n";
        return 1;
    }
    ColorGradingPreset balanceVoidGrading = ColorGradingPreset::escape_default;
    if (!color_pipeline_core::TryParseAdvancedColorGradingFunctionId("balance_void_grade", &balanceVoidGrading)) {
        std::cerr << "Expected balance_void_grade to parse before reset proof\n";
        return 1;
    }

    view.fractal_type = FractalType::explaino_halley;
    view.explaino_phase = 2.0f;
    view.explaino_seed_drift = 0.75f;
    view.auto_increment_seed = true;
    view.explaino_seed_rate = 1.25f;
    view.explaino_phase_strength = 3.0f;
    params.explaino_seed = 9.0;
    params.explaino_warp_strength = 0.9f;
    params.explaino_root_spread = 2.25f;
    params.explaino_damping = 0.4f;
    params.color_phase_signal_offset = 1.25f;
    params.color_phase_wrap_cycles = 2.5f;
    params.color_phase_palette_offset = -0.75f;
    params.color_source_stack_count = 2;
    params.color_source_stack[0].signal = ColorSignal::smooth_escape;
    params.color_source_stack[0].params.scale = 0.5f;
    params.color_source_stack[0].params.bias = 0.25f;
    params.color_source_stack[0].params.blend_weight = 1.0f;
    params.color_source_stack[1].signal = ColorSignal::escape_magnitude;
    params.color_source_stack[1].params.magnitude_scale = 1.5f;
    params.color_source_stack[1].params.magnitude_bias = -0.25f;
    params.color_source_stack[1].params.blend_weight = 0.25f;
    params.color_shape = ColorPipelineShape::offset_scale;
    params.color_shape_stack_count = 2;
    params.color_shape_stack[0].shape = ColorPipelineShape::offset_scale;
    params.color_shape_stack[0].params.offset = 0.35f;
    params.color_shape_stack[0].params.scale = 1.8f;
    params.color_shape_stack[1].shape = ColorPipelineShape::repeat;
    params.color_shape_stack[1].params.repeat_frequency = 6.0f;
    params.color_shape_stack[1].params.repeat_phase = 0.2f;
    params.color_root_basin_pair_count = 2;
    params.color_root_basin_pairs[0] = {ColorSignal::root_index, ColorPalette::root_classic, ColorGradingPreset::basin_default};
    params.color_root_basin_pairs[1] = {ColorSignal::root_index, ColorPalette::joy, ColorGradingPreset::basin_default};
    params.color_palette_stack_count = 2;
    params.color_palette_stack[0].palette = ColorPalette::cyclic_escape;
    params.color_palette_stack[0].params.cycle_scale = 1.25f;
    params.color_palette_stack[0].params.saturation = 0.9f;
    params.color_palette_stack[1].palette = ColorPalette::explaino_cmap;
    params.color_palette_stack[1].params.seed_scale = 1.5f;
    params.color_palette_stack[1].params.seed_phase = 0.25f;
    params.color_palette_stack[1].params.colorfulness = 0.8f;
    params.color_palette_stack[1].params.blend_weight = 0.35f;
    params.color_grading_stack_count = 5;
    params.color_grading_stack[0].grading = ColorGradingPreset::escape_default;
    params.color_grading_stack[0].params.exposure = 1.4f;
    params.color_grading_stack[0].params.saturation = 1.2f;
    params.color_grading_stack[1].grading = neutralGrading;
    params.color_grading_stack[1].params.exposure = 1.25f;
    params.color_grading_stack[1].params.saturation = 0.85f;
    params.color_grading_stack[1].params.contrast = 1.4f;
    params.color_grading_stack[2].grading = toneMapGrading;
    params.color_grading_stack[2].params.exposure = 1.35f;
    params.color_grading_stack[2].params.saturation = 0.75f;
    params.color_grading_stack[2].params.contrast = 1.6f;
    params.color_grading_stack[3].grading = gradeGlowGrading;
    params.color_grading_stack[3].params.exposure = 1.2f;
    params.color_grading_stack[3].params.saturation = 0.8f;
    params.color_grading_stack[3].params.contrast = 1.5f;
    params.color_grading_stack[3].params.glow = 0.6f;
    params.color_grading_stack[4].grading = balanceVoidGrading;
    params.color_grading_stack[4].params.balance_void = 0.35f;
    params.color_grading_stack[4].params.chroma_tension = 0.6f;
    params.color_grading_stack[4].params.accent_bias = -0.25f;
    params.color_shape_offset = 0.35f;
    params.color_shape_scale = 1.8f;
    params.color_shape_repeat_frequency = 6.0f;
    params.color_shape_repeat_phase = 0.2f;
    params.color_shape_posterize_steps = 5;
    params.color_shape_posterize_mix = 0.65f;
    params.color_shape_bias = 0.2f;
    params.color_shape_gain = 0.8f;
    params.color_shape_window_center = 0.35f;
    params.color_shape_window_width = 0.4f;
    params.color_shape_window_softness = 0.05f;
    params.color_iteration_band_count = 5;
    params.color_iteration_band_softness = 0.8f;
    params.color_iteration_band_emphasis = 1.6f;
    params.color_iteration_band_palette_offset = 0.4f;
    params.color_smooth_escape_scale = 2.0f;
    params.color_smooth_escape_bias = -0.25f;
    params.exposure = 1.25f;
    params.color_saturation = 0.85f;
    params.color_contrast = 1.4f;
    params.color_glow = 0.6f;
    params.color_balance_void = 0.35f;
    params.color_chroma_tension = 0.6f;
    params.color_accent_bias = -0.25f;
    params.color_tint_r = 0.4f;
    params.color_tint_g = 1.7f;
    params.color_tint_b = 0.25f;
    params.color_heatmap_cycle_scale = 1.75f;
    params.color_heatmap_saturation = 1.4f;
    params.color_explaino_palette_seed_scale = 1.5f;
    params.color_explaino_palette_seed_phase = 0.25f;
    params.color_explaino_palette_colorfulness = 0.8f;
    params.color_contrast_lift_exposure = 1.8f;
    params.color_contrast_lift_saturation = 1.3f;
    render.resolution = {2048, 1024};
    render.block_size = 512;
    render.device_id = 3;
    render.benchmark = true;
    lens.enabled = true;
    lens.downsample = 16;

    ResetRuntimeStateForCurrentFractal(view, params, render, lens, &dirty);

    if (!dirty) {
        std::cerr << "Reset should mark dirty\n";
        return 1;
    }
    if (view.auto_refresh) {
        std::cerr << "Reset should leave continuous render disabled by default\n";
        return 1;
    }
    if (!NearlyEqual(view.explaino_phase, 0.0f) || !NearlyEqual(view.explaino_seed_drift, 0.0f)) {
        std::cerr << "Reset should clear explaino phase/drift\n";
        return 1;
    }
    if (view.auto_increment_seed || !NearlyEqual(view.explaino_seed_rate, 0.001f) || !NearlyEqual(view.explaino_phase_strength, 1.0f)) {
        std::cerr << "Reset should restore explaino transport defaults\n";
        return 1;
    }
    if (!NearlyEqual(static_cast<float>(params.explaino_seed), 0.0f) || !NearlyEqual(params.explaino_warp_strength, 0.0f) ||
        !NearlyEqual(params.explaino_root_spread, 0.5f) || !NearlyEqual(params.explaino_damping, 1.0f)) {
        std::cerr << "Reset should restore explaino param defaults\n";
        return 1;
    }
    if (!NearlyEqual(params.color_phase_signal_offset, 0.0f) ||
        !NearlyEqual(params.color_phase_wrap_cycles, 1.0f) ||
        !NearlyEqual(params.color_phase_palette_offset, 0.0f) ||
        params.color_source_stack_count != 0 ||
        params.color_root_basin_pair_count != 0 ||
        params.color_shape != ColorPipelineShape::identity ||
        params.color_shape_stack_count != 0 ||
        params.color_palette_stack_count != 0 ||
        params.color_grading_stack_count != 0 ||
        !NearlyEqual(params.color_shape_offset, 0.0f) ||
        !NearlyEqual(params.color_shape_scale, 1.0f) ||
        !NearlyEqual(params.color_shape_repeat_frequency, 8.0f) ||
        !NearlyEqual(params.color_shape_repeat_phase, 0.0f) ||
        params.color_shape_posterize_steps != 6 ||
        !NearlyEqual(params.color_shape_posterize_mix, 1.0f) ||
        !NearlyEqual(params.color_shape_bias, 0.5f) ||
        !NearlyEqual(params.color_shape_gain, 0.5f) ||
        !NearlyEqual(params.color_shape_window_center, 0.5f) ||
        !NearlyEqual(params.color_shape_window_width, 1.0f) ||
        !NearlyEqual(params.color_shape_window_softness, 0.0f) ||
        params.color_iteration_band_count != 8 ||
        !NearlyEqual(params.color_iteration_band_softness, 0.35f) ||
        !NearlyEqual(params.color_iteration_band_emphasis, 1.0f) ||
        !NearlyEqual(params.color_iteration_band_palette_offset, 0.0f) ||
        !NearlyEqual(params.color_smooth_escape_scale, 1.0f) ||
        !NearlyEqual(params.color_smooth_escape_bias, 0.0f) ||
        !NearlyEqual(params.exposure, 1.0f) ||
        !NearlyEqual(params.color_saturation, 1.15f) ||
        !NearlyEqual(params.color_contrast, 1.10f) ||
        !NearlyEqual(params.color_glow, 0.25f) ||
        !NearlyEqual(params.color_balance_void, 0.0f) ||
        !NearlyEqual(params.color_chroma_tension, 0.0f) ||
        !NearlyEqual(params.color_accent_bias, 0.0f) ||
        !NearlyEqual(params.color_tint_r, 1.0f) ||
        !NearlyEqual(params.color_tint_g, 1.0f) ||
        !NearlyEqual(params.color_tint_b, 1.0f) ||
        !NearlyEqual(params.color_heatmap_cycle_scale, 1.0f) ||
        !NearlyEqual(params.color_heatmap_saturation, 1.0f) ||
        !NearlyEqual(params.color_explaino_palette_seed_scale, 1.0f) ||
        !NearlyEqual(params.color_explaino_palette_seed_phase, 0.0f) ||
        !NearlyEqual(params.color_explaino_palette_colorfulness, 1.0f) ||
        !NearlyEqual(params.color_contrast_lift_exposure, 1.0f) ||
        !NearlyEqual(params.color_contrast_lift_saturation, 1.0f)) {
        std::cerr << "Reset should restore the base Color-panel defaults alongside the phase, band, and advanced programmable color parameter defaults\n";
        return 1;
    }
    if (render.resolution.x != 2048 || render.resolution.y != 1536 || render.block_size != 256 || render.device_id != 0 || render.benchmark) {
        std::cerr << "Reset should restore render defaults\n";
        return 1;
    }
    if (params.color_grading_stack[2].grading != ColorGradingPreset::escape_default ||
        !NearlyEqual(params.color_grading_stack[2].params.exposure, 1.0f) ||
        !NearlyEqual(params.color_grading_stack[2].params.saturation, 1.0f) ||
        !NearlyEqual(params.color_grading_stack[2].params.contrast, 1.0f)) {
        std::cerr << "Reset should clear tone_map_finish grading stack storage back to the default grading-entry state\n";
        return 1;
    }
    if (params.color_grading_stack[3].grading != ColorGradingPreset::escape_default ||
        !NearlyEqual(params.color_grading_stack[3].params.exposure, 1.0f) ||
        !NearlyEqual(params.color_grading_stack[3].params.saturation, 1.0f) ||
        !NearlyEqual(params.color_grading_stack[3].params.contrast, 1.0f) ||
        !NearlyEqual(params.color_grading_stack[3].params.glow, 0.25f)) {
        std::cerr << "Reset should clear grade_glow grading stack storage back to the default grading-entry state\n";
        return 1;
    }
    if (params.color_grading_stack[4].grading != ColorGradingPreset::escape_default ||
        !NearlyEqual(params.color_grading_stack[4].params.balance_void, 0.0f) ||
        !NearlyEqual(params.color_grading_stack[4].params.chroma_tension, 0.0f) ||
        !NearlyEqual(params.color_grading_stack[4].params.accent_bias, 0.0f)) {
        std::cerr << "Reset should clear balance_void_grade grading stack storage back to the default grading-entry state\n";
        return 1;
    }
    if (render.interaction_debounce_ms != 200 || !NearlyEqual(render.preview_target_fps, 30.0f) || !NearlyEqual(render.preview_min_scale, 0.5f)) {
        std::cerr << "Reset should restore adaptive preview pacing defaults\n";
        return 1;
    }
    if (lens.enabled || lens.downsample != 2) {
        std::cerr << "Reset should restore lens defaults\n";
        return 1;
    }

    {
        ViewState dualView{};
        KernelParams dualParams{};
        RenderSettings dualRender{};
        LensSettings dualLens{};
        bool dualDirty = false;

        dualView.fractal_type = FractalType::explaino_dual;
        dualParams.explaino_seed = 3.0;
        dualParams.explaino_seed_b = 8.0;
        dualParams.explaino_mix = 0.9f;

        ResetRuntimeStateForCurrentFractal(dualView, dualParams, dualRender, dualLens, &dualDirty);

        if (!dualDirty) {
            std::cerr << "Dual-seed reset should mark dirty\n";
            return 1;
        }
        if (!NearlyEqual(static_cast<float>(dualParams.explaino_seed_b), 1.0f) || !NearlyEqual(dualParams.explaino_mix, 0.5f)) {
            std::cerr << "Dual-seed reset should restore deterministic seed_b/mix defaults\n";
            return 1;
        }
    }

    {
        ViewState composedView{};
        KernelParams composedParams{};
        RenderSettings composedRender{};
        LensSettings composedLens{};
        bool composedDirty = false;

        composedView.fractal_type = FractalType::explaino_vortex;
        composedParams.balance_void = 0.4f;
        composedParams.symmetry_tension = -0.3f;
        composedParams.field_curvature = 0.25f;
        composedParams.ripple_amplitude = 0.15f;
        composedParams.splice_offset = 0.5f;
        composedParams.vortex_strength = 0.75f;
        composedParams.tension_strength = 0.02f;

        ResetRuntimeStateForCurrentFractal(composedView, composedParams, composedRender, composedLens, &composedDirty);

        if (!composedDirty) {
            std::cerr << "Legacy Explaino composed reset should mark dirty\n";
            return 1;
        }
        if (composedView.fractal_type != FractalType::explaino_vortex) {
            std::cerr << "Legacy Explaino composed reset should preserve the explicit Explaino-Vortex selector while restoring its preset vector\n";
            return 1;
        }
        if (!NearlyEqual(composedParams.ripple_amplitude, 0.0f) ||
            !NearlyEqual(composedParams.splice_offset, 0.0f) ||
            !NearlyEqual(composedParams.vortex_strength, 0.3f) ||
            !NearlyEqual(composedParams.tension_strength, 0.0f) ||
            !NearlyEqual(composedParams.balance_void, 0.0f) ||
            !NearlyEqual(composedParams.symmetry_tension, 0.0f) ||
            !NearlyEqual(composedParams.field_curvature, 0.0f)) {
            std::cerr << "Legacy Explaino composed reset should restore only the explicit Explaino-Vortex preset vector\n";
            return 1;
        }
    }

    {
        ViewState balanceVoidView{};
        KernelParams balanceVoidParams{};
        RenderSettings balanceVoidRender{};
        LensSettings balanceVoidLens{};
        bool balanceVoidDirty = false;

        balanceVoidView.fractal_type = FractalType::explaino_balance_void;
        balanceVoidParams.balance_void = 0.4f;
        balanceVoidParams.symmetry_tension = -0.3f;
        balanceVoidParams.field_curvature = 0.25f;
        balanceVoidParams.ripple_amplitude = 0.15f;
        balanceVoidParams.splice_offset = 0.5f;
        balanceVoidParams.vortex_strength = 0.3f;
        balanceVoidParams.tension_strength = 0.02f;

        ResetRuntimeStateForCurrentFractal(balanceVoidView, balanceVoidParams, balanceVoidRender, balanceVoidLens, &balanceVoidDirty);

        if (!balanceVoidDirty) {
            std::cerr << "ExplainO-BalanceVoid reset should mark dirty\n";
            return 1;
        }
        if (balanceVoidView.fractal_type != FractalType::explaino_balance_void) {
            std::cerr << "ExplainO-BalanceVoid reset should preserve the explicit public selector identity\n";
            return 1;
        }
        if (!NearlyEqual(balanceVoidParams.balance_void, 0.0f) ||
            !NearlyEqual(balanceVoidParams.symmetry_tension, 0.0f) ||
            !NearlyEqual(balanceVoidParams.field_curvature, 0.0f)) {
            std::cerr << "ExplainO-BalanceVoid reset should preserve its dedicated family-axis defaults at the neutral collapse point\n";
            return 1;
        }
        if (!NearlyEqual(balanceVoidParams.ripple_amplitude, 0.0f) ||
            !NearlyEqual(balanceVoidParams.splice_offset, 0.0f) ||
            !NearlyEqual(balanceVoidParams.vortex_strength, 0.0f) ||
            !NearlyEqual(balanceVoidParams.tension_strength, 0.0f)) {
            std::cerr << "ExplainO-BalanceVoid reset should clear unrelated composed-variant axes without widening into deferred classes\n";
            return 1;
        }
    }

    {
        for (const auto& coupling : kExplainoCouplingRegistry) {
            ViewState couplingView{};
            KernelParams couplingParams{};
            RenderSettings couplingRender{};
            LensSettings couplingLens{};
            bool couplingDirty = false;

            couplingView.fractal_type = coupling.carrier_fractal_type;
            couplingParams.momentum_beta = 0.8f;
            couplingParams.joy_coupling = 0.9f;
            couplingParams.fold_coupling = 0.7f;
            couplingParams.bell_coupling = 0.6f;
            couplingParams.ripple_amplitude = 0.15f;
            couplingParams.splice_offset = 0.5f;
            couplingParams.vortex_strength = 0.3f;
            couplingParams.tension_strength = 0.02f;
            couplingParams.balance_void = 0.4f;
            couplingParams.symmetry_tension = -0.3f;
            couplingParams.field_curvature = 0.25f;
            couplingParams.phoenix_p_real = 0.5667f;
            couplingParams.explaino_seed_b = 7.0;
            couplingParams.explaino_mix = 0.9f;
            couplingParams.explaino_cluster_radius = 0.8f;

            ResetRuntimeStateForCurrentFractal(couplingView, couplingParams, couplingRender, couplingLens, &couplingDirty);

            if (!couplingDirty) {
                std::cerr << "Deferred Explaino coupling reset should mark dirty for every legacy carrier\n";
                return 1;
            }
            if (couplingView.fractal_type != coupling.carrier_fractal_type) {
                std::cerr << "Deferred Explaino coupling reset should preserve the owning legacy selector identity instead of canonicalizing it\n";
                return 1;
            }
            for (const auto& expectedCoupling : kExplainoCouplingRegistry) {
                const float* value = ResolveExplainoCouplingValue(static_cast<const KernelParams&>(couplingParams), expectedCoupling.slot);
                const float expected = expectedCoupling.carrier_fractal_type == coupling.carrier_fractal_type
                    ? expectedCoupling.default_value
                    : expectedCoupling.neutral_value;
                if (!value || !NearlyEqual(*value, expected)) {
                    std::cerr << "Deferred Explaino coupling reset should restore only the owning coupling default\n";
                    return 1;
                }
            }
            for (const auto& axis : kExplainoAxisRegistry) {
                const float* axisValue = ResolveExplainoAxisValue(static_cast<const KernelParams&>(couplingParams), axis.slot);
                if (!axisValue || !NearlyEqual(*axisValue, 0.0f)) {
                    std::cerr << "Deferred Explaino coupling reset should keep the canonical Explaino-all axis registry neutral\n";
                    return 1;
                }
            }
            if (!NearlyEqual(couplingParams.phoenix_p_real, 0.0f) ||
                !NearlyEqual(static_cast<float>(couplingParams.explaino_seed_b), 1.0f) ||
                !NearlyEqual(couplingParams.explaino_mix, 0.5f) ||
                !NearlyEqual(couplingParams.explaino_cluster_radius, 0.0f)) {
                std::cerr << "Deferred Explaino coupling reset must not auto-promote the other deferred parameter classes\n";
                return 1;
            }
        }
    }

    {
        for (const FractalType phoenixViewType : {
                FractalType::phoenix,
                FractalType::explaino_phoenix,
                FractalType::explaino_joy,
                FractalType::explaino_fold,
                FractalType::explaino_bell,
            }) {
            ViewState phoenixView{};
            KernelParams phoenixParams{};
            RenderSettings phoenixRender{};
            LensSettings phoenixLens{};
            bool phoenixDirty = false;

            phoenixView.fractal_type = phoenixViewType;
            phoenixParams.phoenix_p_real = 0.5667f;
            phoenixParams.explaino_cluster_radius = 0.8f;
            phoenixParams.momentum_beta = 0.8f;
            phoenixParams.joy_coupling = 0.9f;
            phoenixParams.fold_coupling = 0.7f;
            phoenixParams.bell_coupling = 0.6f;
            phoenixParams.explaino_seed_b = 7.0;
            phoenixParams.explaino_mix = 0.9f;

            ResetRuntimeStateForCurrentFractal(phoenixView, phoenixParams, phoenixRender, phoenixLens, &phoenixDirty);

            if (!phoenixDirty || phoenixView.fractal_type != phoenixViewType) {
                std::cerr << "phoenix_p_real follow-up reset should preserve the owning phoenix-step carrier identity and mark dirty\n";
                return 1;
            }
            const PhoenixStepCarrierSelectorDescriptor* phoenixCarrier = FindPhoenixStepCarrierSelectorDescriptor(phoenixViewType);
            const float expectedPhoenix = phoenixCarrier
                ? static_cast<float>(phoenixCarrier->default_value)
                : 0.0f;
            const ExplainoCouplingDescriptor* coupling = FindExplainoCouplingDescriptor(phoenixViewType);
            const float expectedMomentum = coupling && coupling->slot == ExplainoCouplingParamSlot::momentum_beta
                ? coupling->default_value
                : 0.0f;
            const float expectedJoy = coupling && coupling->slot == ExplainoCouplingParamSlot::joy_coupling
                ? coupling->default_value
                : 0.0f;
            const float expectedFold = coupling && coupling->slot == ExplainoCouplingParamSlot::fold_coupling
                ? coupling->default_value
                : 0.0f;
            const float expectedBell = coupling && coupling->slot == ExplainoCouplingParamSlot::bell_coupling
                ? coupling->default_value
                : 0.0f;
            if (!NearlyEqual(phoenixParams.phoenix_p_real, expectedPhoenix) ||
                !NearlyEqual(phoenixParams.explaino_cluster_radius, 0.0f)) {
                std::cerr << "phoenix_p_real follow-up reset should restore only the owning phoenix-step carrier default\n";
                return 1;
            }
            if (!NearlyEqual(phoenixParams.momentum_beta, expectedMomentum) ||
                !NearlyEqual(phoenixParams.joy_coupling, expectedJoy) ||
                !NearlyEqual(phoenixParams.fold_coupling, expectedFold) ||
                !NearlyEqual(phoenixParams.bell_coupling, expectedBell) ||
                !NearlyEqual(static_cast<float>(phoenixParams.explaino_seed_b), 1.0f) ||
                !NearlyEqual(phoenixParams.explaino_mix, 0.5f)) {
                std::cerr << "phoenix_p_real follow-up reset must preserve the closed coupling defaults and must not auto-promote dual-seed params\n";
                return 1;
            }
        }

        for (const FractalType projectionViewType : {
                FractalType::explaino_ripple,
                FractalType::explaino_splice,
                FractalType::explaino_vortex,
                FractalType::explaino_tension,
            }) {
            ViewState projectionView{};
            KernelParams projectionParams{};
            RenderSettings projectionRender{};
            LensSettings projectionLens{};
            bool projectionDirty = false;

            projectionView.fractal_type = projectionViewType;
            projectionParams.phoenix_p_real = 0.5667f;
            projectionParams.explaino_cluster_radius = 0.8f;
            projectionParams.ripple_amplitude = 0.41f;
            projectionParams.splice_offset = 0.61f;
            projectionParams.vortex_strength = 0.71f;
            projectionParams.tension_strength = 0.81f;

            ResetRuntimeStateForCurrentFractal(projectionView, projectionParams, projectionRender, projectionLens, &projectionDirty);

            if (!projectionDirty || projectionView.fractal_type != projectionViewType) {
                std::cerr << "Phoenix-step projection carriers should preserve their explicit public selector identity on reset\n";
                return 1;
            }
            const ExplainoAxisDescriptor* axis = FindExplainoSingleAxisProjectionDescriptor(projectionViewType);
            if (!axis) {
                std::cerr << "Expected projection carrier to resolve through the closed Explaino axis registry\n";
                return 1;
            }
            const float* axisValue = ResolveExplainoAxisValue(static_cast<const KernelParams&>(projectionParams), axis->slot);
            if (!axisValue || !NearlyEqual(*axisValue, axis->default_value) ||
                !NearlyEqual(projectionParams.phoenix_p_real, 0.0f) ||
                !NearlyEqual(projectionParams.explaino_cluster_radius, 0.0f) ||
                !NearlyEqual(static_cast<float>(projectionParams.explaino_seed_b), 1.0f) ||
                !NearlyEqual(projectionParams.explaino_mix, 0.5f)) {
                std::cerr << "Phoenix-step projection carriers should keep the axis reset answer while leaving phoenix_p_real fenced off the canonical reset path\n";
                return 1;
            }
        }

        for (const FractalType structuralViewType : {
                FractalType::explaino_all,
                FractalType::explaino_mult,
                FractalType::explaino_rational,
            }) {
            ViewState structuralView{};
            KernelParams structuralParams{};
            RenderSettings structuralRender{};
            LensSettings structuralLens{};
            bool structuralDirty = false;

            structuralView.fractal_type = structuralViewType;
            structuralParams.phoenix_p_real = 0.5667f;
            structuralParams.explaino_cluster_radius = 0.8f;
            structuralParams.momentum_beta = 0.8f;
            structuralParams.joy_coupling = 0.9f;
            structuralParams.fold_coupling = 0.7f;
            structuralParams.bell_coupling = 0.6f;
            structuralParams.explaino_seed_b = 7.0;
            structuralParams.explaino_mix = 0.9f;

            ResetRuntimeStateForCurrentFractal(structuralView, structuralParams, structuralRender, structuralLens, &structuralDirty);

            if (!structuralDirty || structuralView.fractal_type != structuralViewType) {
                std::cerr << "Structural/root-pack reset should preserve the owning carrier identity and mark dirty in the phoenix_p_real follow-up\n";
                return 1;
            }
            const ExplainoClusterRadiusSelectorDescriptor* structuralCarrier = FindExplainoClusterRadiusSelectorDescriptor(structuralViewType);
            const float expectedCluster = structuralCarrier
                ? static_cast<float>(structuralCarrier->default_value)
                : 0.0f;
            if (!NearlyEqual(structuralParams.phoenix_p_real, 0.0f) ||
                !NearlyEqual(structuralParams.explaino_cluster_radius, expectedCluster)) {
                std::cerr << "Cluster-radius reset should restore only the split selector defaults and keep phoenix_p_real out of scope\n";
                return 1;
            }
            if (!NearlyEqual(structuralParams.momentum_beta, 0.0f) ||
                !NearlyEqual(structuralParams.joy_coupling, 0.0f) ||
                !NearlyEqual(structuralParams.fold_coupling, 0.0f) ||
                !NearlyEqual(structuralParams.bell_coupling, 0.0f) ||
                !NearlyEqual(static_cast<float>(structuralParams.explaino_seed_b), 1.0f) ||
                !NearlyEqual(structuralParams.explaino_mix, 0.5f)) {
                std::cerr << "The phoenix_p_real follow-up must leave resolved couplings and dual-seed params untouched during structural reset\n";
                return 1;
            }
        }
    }

    std::cout << "test_runtime_reset: all passed\n";
    return 0;
}
