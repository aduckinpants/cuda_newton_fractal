#include "../src/color_pipeline_core.h"
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
    params.color_grading_stack_count = 4;
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

    std::cout << "test_runtime_reset: all passed\n";
    return 0;
}