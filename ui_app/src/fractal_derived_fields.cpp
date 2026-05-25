#include "fractal_derived_fields.h"

#include "explaino_seed.h"
#include "explaino_seed_curve.h"
#include "fractal_catalog.h"
#include "fractal_family_rules.h"
#include "view_hp_sync.h"

#include <cmath>

static inline float ClampF(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

uint32_t HashU32(uint32_t x) {
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

float Hash01(uint32_t x) {
    return static_cast<float>(HashU32(x) & 0x00ffffffU) / static_cast<float>(0x01000000U);
}

static inline float LerpF(float a, float b, float t) {
    return a + (b - a) * t;
}

void ApplyFractalViewPresetDefaults(ViewState& view, bool* ioDirty) {
    const FractalCatalogViewDefaults* catalogDefaults = FindFractalCatalogViewDefaults(view.fractal_type);
    const FractalCatalogViewDefaults fallback = FractalCatalogViewDefaultsFor(view.fractal_type);
    const FractalCatalogViewDefaults& defaults = catalogDefaults ? *catalogDefaults : fallback;
    view.auto_max_iter = defaults.auto_max_iter;
    view.center = defaults.center;
    view.zoom = defaults.zoom;
    view.rotation_degrees = defaults.rotation_degrees;
    if (ioDirty) *ioDirty = true;
}

void SetPolyPreset(KernelParams& params) {
    if (params.poly_kind == PolyKind::z3_minus_1) {
        params.poly_coeffs[0] = -1.0f;
        params.poly_coeffs[1] = 0.0f;
        params.poly_coeffs[2] = 0.0f;
        params.poly_coeffs[3] = 1.0f;
        params.poly_coeffs[4] = 0.0f;
    } else if (params.poly_kind == PolyKind::z4_minus_1) {
        params.poly_coeffs[0] = -1.0f;
        params.poly_coeffs[1] = 0.0f;
        params.poly_coeffs[2] = 0.0f;
        params.poly_coeffs[3] = 0.0f;
        params.poly_coeffs[4] = 1.0f;
    }
}

static void SyncCounterfactualPairRootFamilyPreset(KernelParams& params) {
    switch (params.counterfactual_pair_root_family) {
    case CounterfactualPairRootFamily::cubic_unit_roots:
        params.poly_kind = PolyKind::z3_minus_1;
        break;
    case CounterfactualPairRootFamily::quartic_unit_roots:
        params.poly_kind = PolyKind::z4_minus_1;
        break;
    }
    SetPolyPreset(params);
}

static void SyncProjectionAndFlowRootFamilyPreset(KernelParams& params) {
    switch (params.projection_and_flow_root_family) {
    case ProjectionAndFlowRootFamily::cubic_unit_roots:
        params.poly_kind = PolyKind::z3_minus_1;
        break;
    case ProjectionAndFlowRootFamily::quartic_unit_roots:
        params.poly_kind = PolyKind::z4_minus_1;
        break;
    }
    SetPolyPreset(params);
}

static void MarkDirty(bool* ioDirty) {
    if (ioDirty) *ioDirty = true;
}

static void ApplyCommonPresetDefaults(KernelParams& params) {
    params.multibrot_power = 3;
    params.multibrot_power_float = 3.0f;
    params.multibrot_power_imag = 0.0f;
    params.lambda_real = 2.9685855f;
    params.lambda_imag = -0.27446103f;
    params.burning_ship_fold_mix = 1.0f;
    params.celtic_abs_mix = 1.0f;
    params.perpendicular_fold_mix = 1.0f;
    params.julia_c_real = -0.7f;
    params.julia_c_imag = 0.27015f;
    params.explaino_julia_constant_mode = ExplainoJuliaConstantMode::seeded;
    params.explaino_julia_c_real = -0.7f;
    params.explaino_julia_c_imag = 0.27015f;
    params.magnet_seed_real = 0.0f;
    params.magnet_seed_imag = 0.0f;
    params.magnet_relaxation = 1.0f;
    params.magnet_bailout = 12.0f;
    params.color_saturation = 1.15f;
    params.color_contrast = 1.10f;
    params.color_tint_r = 1.0f;
    params.color_tint_g = 1.0f;
    params.color_tint_b = 1.0f;
    params.color_phase_signal_offset = 0.0f;
    params.color_phase_wrap_cycles = 1.0f;
    params.color_phase_palette_offset = 0.0f;
    params.color_source_stack_count = 0;
    for (ColorPipelineSourceStackEntry& sourceEntry : params.color_source_stack) {
        sourceEntry = {};
    }
    params.color_shape = ColorPipelineShape::identity;
    params.color_shape_stack_count = 0;
    for (ColorPipelineShapeStackEntry& shapeEntry : params.color_shape_stack) {
        shapeEntry = {};
    }
    params.color_palette_stack_count = 0;
    for (ColorPipelinePaletteStackEntry& paletteEntry : params.color_palette_stack) {
        paletteEntry = {};
    }
    params.color_grading_stack_count = 0;
    for (ColorPipelineGradingStackEntry& gradingEntry : params.color_grading_stack) {
        gradingEntry = {};
    }
    params.color_shape_offset = 0.0f;
    params.color_shape_scale = 1.0f;
    params.color_shape_repeat_frequency = 8.0f;
    params.color_shape_repeat_phase = 0.0f;
    params.color_shape_posterize_steps = 6;
    params.color_shape_posterize_mix = 1.0f;
    params.color_shape_bias = 0.5f;
    params.color_shape_gain = 0.5f;
    params.color_shape_window_center = 0.5f;
    params.color_shape_window_width = 1.0f;
    params.color_shape_window_softness = 0.0f;
    params.color_iteration_band_count = 8;
    params.color_iteration_band_softness = 0.35f;
    params.color_iteration_band_emphasis = 1.0f;
    params.color_iteration_band_palette_offset = 0.0f;
    params.color_smooth_escape_scale = 1.0f;
    params.color_smooth_escape_bias = 0.0f;
    params.color_smooth_escape_interior_strength = 0.2f;
    params.color_heatmap_cycle_scale = 1.0f;
    params.color_heatmap_saturation = 1.0f;
    params.color_explaino_palette_seed_scale = 1.0f;
    params.color_explaino_palette_seed_phase = 0.0f;
    params.color_explaino_palette_colorfulness = 1.0f;
    params.color_contrast_lift_exposure = 1.0f;
    params.color_contrast_lift_saturation = 1.0f;
}

static void ApplyDefaultColoringSelection(FractalType fractalType, KernelParams& params) {
    params.coloring_mode = DefaultColoringModeForFractal(fractalType);
    params.color_pipeline = DefaultColorPipelineForFractal(fractalType);
}

struct ColorPipelineRuntimeSnapshot {
    ColoringMode coloring_mode{ColoringMode::root_basin};
    ColorPipelineSelection color_pipeline{};
    float exposure{1.0f};
    float color_saturation{1.15f};
    float color_contrast{1.10f};
    float color_glow{0.25f};
    float color_balance_void{0.0f};
    float color_chroma_tension{0.0f};
    float color_accent_bias{0.0f};
    float color_tint_r{1.0f};
    float color_tint_g{1.0f};
    float color_tint_b{1.0f};
    float color_phase_signal_offset{0.0f};
    float color_phase_wrap_cycles{1.0f};
    float color_phase_palette_offset{0.0f};
    int color_source_stack_count{0};
    ColorPipelineSourceStackEntry color_source_stack[kColorPipelineMaxSourceStackCount]{};
    ColorPipelineShape color_shape{ColorPipelineShape::identity};
    int color_shape_stack_count{0};
    ColorPipelineShapeStackEntry color_shape_stack[kColorPipelineMaxShapeStackCount]{};
    int color_root_basin_pair_count{0};
    ColorPipelineSelection color_root_basin_pairs[kColorPipelineMaxRootBasinPairCount]{};
    int color_palette_stack_count{0};
    ColorPipelinePaletteStackEntry color_palette_stack[kColorPipelineMaxPaletteStackCount]{};
    int color_grading_stack_count{0};
    ColorPipelineGradingStackEntry color_grading_stack[kColorPipelineMaxGradingStackCount]{};
    float color_shape_offset{0.0f};
    float color_shape_scale{1.0f};
    float color_shape_repeat_frequency{8.0f};
    float color_shape_repeat_phase{0.0f};
    int color_shape_posterize_steps{6};
    float color_shape_posterize_mix{1.0f};
    float color_shape_bias{0.5f};
    float color_shape_gain{0.5f};
    float color_shape_window_center{0.5f};
    float color_shape_window_width{1.0f};
    float color_shape_window_softness{0.0f};
    int color_iteration_band_count{8};
    float color_iteration_band_softness{0.35f};
    float color_iteration_band_emphasis{1.0f};
    float color_iteration_band_palette_offset{0.0f};
    float color_smooth_escape_scale{1.0f};
    float color_smooth_escape_bias{0.0f};
    float color_smooth_escape_interior_strength{0.2f};
    float color_escape_magnitude_scale{1.0f};
    float color_escape_magnitude_bias{0.0f};
    float color_orbit_stripe_frequency{1.0f};
    float color_orbit_stripe_phase{0.0f};
    float color_root_proximity_scale{1.0f};
    float color_root_proximity_bias{0.0f};
    float color_heatmap_cycle_scale{1.0f};
    float color_heatmap_saturation{1.0f};
    float color_explaino_palette_seed_scale{1.0f};
    float color_explaino_palette_seed_phase{0.0f};
    float color_explaino_palette_colorfulness{1.0f};
    float color_contrast_lift_exposure{1.0f};
    float color_contrast_lift_saturation{1.0f};
};

static int ClampColorPipelineCount(int count, int maxCount) {
    if (count < 0) return 0;
    if (count > maxCount) return maxCount;
    return count;
}

static ColorPipelineRuntimeSnapshot CaptureColorPipelineRuntimeSnapshot(const KernelParams& params) {
    ColorPipelineRuntimeSnapshot snapshot{};
    snapshot.coloring_mode = params.coloring_mode;
    snapshot.color_pipeline = params.color_pipeline;
    snapshot.exposure = params.exposure;
    snapshot.color_saturation = params.color_saturation;
    snapshot.color_contrast = params.color_contrast;
    snapshot.color_glow = params.color_glow;
    snapshot.color_balance_void = params.color_balance_void;
    snapshot.color_chroma_tension = params.color_chroma_tension;
    snapshot.color_accent_bias = params.color_accent_bias;
    snapshot.color_tint_r = params.color_tint_r;
    snapshot.color_tint_g = params.color_tint_g;
    snapshot.color_tint_b = params.color_tint_b;
    snapshot.color_phase_signal_offset = params.color_phase_signal_offset;
    snapshot.color_phase_wrap_cycles = params.color_phase_wrap_cycles;
    snapshot.color_phase_palette_offset = params.color_phase_palette_offset;
    snapshot.color_source_stack_count = ClampColorPipelineCount(params.color_source_stack_count, kColorPipelineMaxSourceStackCount);
    for (int index = 0; index < snapshot.color_source_stack_count; ++index) {
        snapshot.color_source_stack[index] = params.color_source_stack[index];
    }
    snapshot.color_shape = params.color_shape;
    snapshot.color_shape_stack_count = ClampColorPipelineCount(params.color_shape_stack_count, kColorPipelineMaxShapeStackCount);
    for (int index = 0; index < snapshot.color_shape_stack_count; ++index) {
        snapshot.color_shape_stack[index] = params.color_shape_stack[index];
    }
    snapshot.color_root_basin_pair_count = ClampColorPipelineCount(params.color_root_basin_pair_count, kColorPipelineMaxRootBasinPairCount);
    for (int index = 0; index < snapshot.color_root_basin_pair_count; ++index) {
        snapshot.color_root_basin_pairs[index] = params.color_root_basin_pairs[index];
    }
    snapshot.color_palette_stack_count = ClampColorPipelineCount(params.color_palette_stack_count, kColorPipelineMaxPaletteStackCount);
    for (int index = 0; index < snapshot.color_palette_stack_count; ++index) {
        snapshot.color_palette_stack[index] = params.color_palette_stack[index];
    }
    snapshot.color_grading_stack_count = ClampColorPipelineCount(params.color_grading_stack_count, kColorPipelineMaxGradingStackCount);
    for (int index = 0; index < snapshot.color_grading_stack_count; ++index) {
        snapshot.color_grading_stack[index] = params.color_grading_stack[index];
    }
    snapshot.color_shape_offset = params.color_shape_offset;
    snapshot.color_shape_scale = params.color_shape_scale;
    snapshot.color_shape_repeat_frequency = params.color_shape_repeat_frequency;
    snapshot.color_shape_repeat_phase = params.color_shape_repeat_phase;
    snapshot.color_shape_posterize_steps = params.color_shape_posterize_steps;
    snapshot.color_shape_posterize_mix = params.color_shape_posterize_mix;
    snapshot.color_shape_bias = params.color_shape_bias;
    snapshot.color_shape_gain = params.color_shape_gain;
    snapshot.color_shape_window_center = params.color_shape_window_center;
    snapshot.color_shape_window_width = params.color_shape_window_width;
    snapshot.color_shape_window_softness = params.color_shape_window_softness;
    snapshot.color_iteration_band_count = params.color_iteration_band_count;
    snapshot.color_iteration_band_softness = params.color_iteration_band_softness;
    snapshot.color_iteration_band_emphasis = params.color_iteration_band_emphasis;
    snapshot.color_iteration_band_palette_offset = params.color_iteration_band_palette_offset;
    snapshot.color_smooth_escape_scale = params.color_smooth_escape_scale;
    snapshot.color_smooth_escape_bias = params.color_smooth_escape_bias;
    snapshot.color_smooth_escape_interior_strength = params.color_smooth_escape_interior_strength;
    snapshot.color_escape_magnitude_scale = params.color_escape_magnitude_scale;
    snapshot.color_escape_magnitude_bias = params.color_escape_magnitude_bias;
    snapshot.color_orbit_stripe_frequency = params.color_orbit_stripe_frequency;
    snapshot.color_orbit_stripe_phase = params.color_orbit_stripe_phase;
    snapshot.color_root_proximity_scale = params.color_root_proximity_scale;
    snapshot.color_root_proximity_bias = params.color_root_proximity_bias;
    snapshot.color_heatmap_cycle_scale = params.color_heatmap_cycle_scale;
    snapshot.color_heatmap_saturation = params.color_heatmap_saturation;
    snapshot.color_explaino_palette_seed_scale = params.color_explaino_palette_seed_scale;
    snapshot.color_explaino_palette_seed_phase = params.color_explaino_palette_seed_phase;
    snapshot.color_explaino_palette_colorfulness = params.color_explaino_palette_colorfulness;
    snapshot.color_contrast_lift_exposure = params.color_contrast_lift_exposure;
    snapshot.color_contrast_lift_saturation = params.color_contrast_lift_saturation;
    return snapshot;
}

static bool CanRestoreColorPipelineRuntimeSnapshot(FractalType fractalType, const ColorPipelineRuntimeSnapshot& snapshot) {
    if (!IsColorPipelineAllowedForFractal(fractalType, snapshot.color_pipeline)) {
        return false;
    }
    for (int index = 0; index < snapshot.color_source_stack_count; ++index) {
        if (!IsColorSignalAllowedForFractal(fractalType, snapshot.color_source_stack[index].signal)) {
            return false;
        }
    }
    for (int index = 0; index < snapshot.color_root_basin_pair_count; ++index) {
        if (!IsColorPipelineAllowedForFractal(fractalType, snapshot.color_root_basin_pairs[index])) {
            return false;
        }
    }
    return true;
}

static void RestoreColorPipelineRuntimeSnapshot(const ColorPipelineRuntimeSnapshot& snapshot, KernelParams& params) {
    ColoringMode mirroredMode = snapshot.coloring_mode;
    if (TryMirroredColoringModeForPipeline(snapshot.color_pipeline, &mirroredMode)) {
        params.coloring_mode = mirroredMode;
    } else {
        params.coloring_mode = snapshot.coloring_mode;
    }
    params.color_pipeline = snapshot.color_pipeline;
    params.exposure = snapshot.exposure;
    params.color_saturation = snapshot.color_saturation;
    params.color_contrast = snapshot.color_contrast;
    params.color_glow = snapshot.color_glow;
    params.color_balance_void = snapshot.color_balance_void;
    params.color_chroma_tension = snapshot.color_chroma_tension;
    params.color_accent_bias = snapshot.color_accent_bias;
    params.color_tint_r = snapshot.color_tint_r;
    params.color_tint_g = snapshot.color_tint_g;
    params.color_tint_b = snapshot.color_tint_b;
    params.color_phase_signal_offset = snapshot.color_phase_signal_offset;
    params.color_phase_wrap_cycles = snapshot.color_phase_wrap_cycles;
    params.color_phase_palette_offset = snapshot.color_phase_palette_offset;
    params.color_source_stack_count = snapshot.color_source_stack_count;
    for (int index = 0; index < kColorPipelineMaxSourceStackCount; ++index) {
        params.color_source_stack[index] = snapshot.color_source_stack[index];
    }
    params.color_shape = snapshot.color_shape;
    params.color_shape_stack_count = snapshot.color_shape_stack_count;
    for (int index = 0; index < kColorPipelineMaxShapeStackCount; ++index) {
        params.color_shape_stack[index] = snapshot.color_shape_stack[index];
    }
    params.color_root_basin_pair_count = snapshot.color_root_basin_pair_count;
    for (int index = 0; index < kColorPipelineMaxRootBasinPairCount; ++index) {
        params.color_root_basin_pairs[index] = snapshot.color_root_basin_pairs[index];
    }
    params.color_palette_stack_count = snapshot.color_palette_stack_count;
    for (int index = 0; index < kColorPipelineMaxPaletteStackCount; ++index) {
        params.color_palette_stack[index] = snapshot.color_palette_stack[index];
    }
    params.color_grading_stack_count = snapshot.color_grading_stack_count;
    for (int index = 0; index < kColorPipelineMaxGradingStackCount; ++index) {
        params.color_grading_stack[index] = snapshot.color_grading_stack[index];
    }
    params.color_shape_offset = snapshot.color_shape_offset;
    params.color_shape_scale = snapshot.color_shape_scale;
    params.color_shape_repeat_frequency = snapshot.color_shape_repeat_frequency;
    params.color_shape_repeat_phase = snapshot.color_shape_repeat_phase;
    params.color_shape_posterize_steps = snapshot.color_shape_posterize_steps;
    params.color_shape_posterize_mix = snapshot.color_shape_posterize_mix;
    params.color_shape_bias = snapshot.color_shape_bias;
    params.color_shape_gain = snapshot.color_shape_gain;
    params.color_shape_window_center = snapshot.color_shape_window_center;
    params.color_shape_window_width = snapshot.color_shape_window_width;
    params.color_shape_window_softness = snapshot.color_shape_window_softness;
    params.color_iteration_band_count = snapshot.color_iteration_band_count;
    params.color_iteration_band_softness = snapshot.color_iteration_band_softness;
    params.color_iteration_band_emphasis = snapshot.color_iteration_band_emphasis;
    params.color_iteration_band_palette_offset = snapshot.color_iteration_band_palette_offset;
    params.color_smooth_escape_scale = snapshot.color_smooth_escape_scale;
    params.color_smooth_escape_bias = snapshot.color_smooth_escape_bias;
    params.color_smooth_escape_interior_strength = snapshot.color_smooth_escape_interior_strength;
    params.color_escape_magnitude_scale = snapshot.color_escape_magnitude_scale;
    params.color_escape_magnitude_bias = snapshot.color_escape_magnitude_bias;
    params.color_orbit_stripe_frequency = snapshot.color_orbit_stripe_frequency;
    params.color_orbit_stripe_phase = snapshot.color_orbit_stripe_phase;
    params.color_root_proximity_scale = snapshot.color_root_proximity_scale;
    params.color_root_proximity_bias = snapshot.color_root_proximity_bias;
    params.color_heatmap_cycle_scale = snapshot.color_heatmap_cycle_scale;
    params.color_heatmap_saturation = snapshot.color_heatmap_saturation;
    params.color_explaino_palette_seed_scale = snapshot.color_explaino_palette_seed_scale;
    params.color_explaino_palette_seed_phase = snapshot.color_explaino_palette_seed_phase;
    params.color_explaino_palette_colorfulness = snapshot.color_explaino_palette_colorfulness;
    params.color_contrast_lift_exposure = snapshot.color_contrast_lift_exposure;
    params.color_contrast_lift_saturation = snapshot.color_contrast_lift_saturation;
}

static void ApplyNewtonLikePresetDefaults(FractalType fractalType, KernelParams& params) {
    params.max_iter = fractalType == FractalType::nova ? 300 : 500;
    params.epsilon = 1e-6f;
    params.nova_alpha = 0.50f;
    params.projection_and_flow_root_family = ProjectionAndFlowRootFamily::cubic_unit_roots;
    params.projection_and_flow_target_radius = 1.0f;
    params.projection_and_flow_pressure_threshold = 1.0f;
    SyncProjectionAndFlowRootFamilyPreset(params);
    ApplyDefaultColoringSelection(fractalType, params);
    params.exposure = 1.0f;
    params.phoenix_p_real = -0.50f;
    params.phoenix_p_imag = 0.0f;
}

static void ApplyCounterfactualPairPresetDefaults(KernelParams& params) {
    params.max_iter = 96;
    params.epsilon = 1e-6f;
    params.counterfactual_pair_root_family = CounterfactualPairRootFamily::cubic_unit_roots;
    params.counterfactual_pair_frame = CounterfactualPairFrame::world_absolute;
    params.counterfactual_pair_offset_x = 0.16f;
    params.counterfactual_pair_offset_y = 0.08f;
    params.counterfactual_pair_reconvergence_ratio = 0.60f;
    SyncCounterfactualPairRootFamilyPreset(params);
    ApplyDefaultColoringSelection(FractalType::counterfactual_pair, params);
    params.exposure = 1.0f;
    params.phoenix_p_real = 0.0f;
    params.phoenix_p_imag = 0.0f;
}

static void ApplyProjectionAndFlowPresetDefaults(KernelParams& params) {
    params.max_iter = 96;
    params.epsilon = 1e-6f;
    params.projection_and_flow_root_family = ProjectionAndFlowRootFamily::cubic_unit_roots;
    params.projection_and_flow_target_radius = 1.0f;
    params.projection_and_flow_pressure_threshold = 1.0f;
    SyncProjectionAndFlowRootFamilyPreset(params);
    ApplyDefaultColoringSelection(FractalType::projection_and_flow, params);
    params.exposure = 1.0f;
    params.phoenix_p_real = 0.0f;
    params.phoenix_p_imag = 0.0f;
}

static void ApplyExplainoPresetDefaults(FractalType fractalType, KernelParams& params);

static void ApplyExplainoProjectionAndFlowPresetDefaults(KernelParams& params) {
    ApplyExplainoPresetDefaults(FractalType::explaino_projection_and_flow, params);
    params.max_iter = 96;
    params.projection_and_flow_root_family = ProjectionAndFlowRootFamily::cubic_unit_roots;
    params.projection_and_flow_target_radius = 1.0f;
    params.projection_and_flow_pressure_threshold = 1.0f;
    params.explaino_warp_strength = 0.25f;
    params.explaino_damping = 0.75f;
    params.explaino_root_count = 0;
    SyncProjectionAndFlowRootFamilyPreset(params);
    ApplyDefaultColoringSelection(FractalType::explaino_projection_and_flow, params);
}

static bool IsExplainoPresetFractal(FractalType fractalType) {
    switch (fractalType) {
    case FractalType::explaino_all:
    case FractalType::explaino:
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
    case FractalType::explaino_counterfactual_pair:
    case FractalType::explaino_projection_and_flow:
        return true;
    default:
        return false;
    }
}

static int DefaultExplainoMaxIter(FractalType fractalType) {
    switch (fractalType) {
    case FractalType::explaino_all:
    case FractalType::explaino:
    case FractalType::explaino_halley:
    case FractalType::explaino_dual:
    case FractalType::explaino_mult:
    case FractalType::explaino_phoenix:
    case FractalType::explaino_transcendental:
    case FractalType::explaino_inertial:
    case FractalType::explaino_rational:
    case FractalType::explaino_collatz:
    case FractalType::explaino_collatz_direct:
    case FractalType::explaino_joy:
    case FractalType::explaino_fold:
    case FractalType::explaino_bell:
    case FractalType::explaino_ripple:
    case FractalType::explaino_splice:
    case FractalType::explaino_vortex:
    case FractalType::explaino_tension:
    case FractalType::explaino_balance_void:
    case FractalType::explaino_counterfactual_pair:
        return 500;
    case FractalType::explaino_nova:
        return 300;
    case FractalType::explaino_julia:
    case FractalType::explaino_lambda:
    case FractalType::explaino_rational_escape:
        return 1200;
    default:
        return 650;
    }
}

static void ApplyExplainoPresetDefaults(FractalType fractalType, KernelParams& params) {
    params.max_iter = DefaultExplainoMaxIter(fractalType);
    params.epsilon = 1e-6f;
    params.nova_alpha = 0.50f;
    params.poly_kind = PolyKind::custom;
    ApplyDefaultColoringSelection(fractalType, params);
    params.exposure = 1.0f;
    params.phoenix_p_real = 0.0f;
    params.phoenix_p_imag = 0.0f;
    params.explaino_seed_b = 1.0;
    params.explaino_mix = 0.5f;
    params.explaino_warp_strength = 0.0f;
    params.explaino_cluster_radius = 0.0f;
    params.explaino_root_authority = ExplainoRootAuthority::generated;
    params.explaino_root_count = 0;
    ApplyExplainoCouplingRegistryDefaults(fractalType, params);
    ApplyExplainoAxisRegistryDefaults(fractalType, params);
    ApplyPhoenixStepCarrierDefaults(fractalType, params);
    ApplyExplainoClusterRadiusDefaults(fractalType, params);
    if (fractalType == FractalType::explaino_transcendental ||
        fractalType == FractalType::explaino_julia ||
        fractalType == FractalType::explaino_lambda) {
        params.explaino_warp_strength = 0.25f;
    }
    if (fractalType == FractalType::explaino_lambda) {
        params.exposure = 1.4f;
    }
    if (fractalType == FractalType::explaino_rational_escape) {
        params.exposure = 1.2f;
    }
}

static void ApplyExplainoCounterfactualPairPresetDefaults(KernelParams& params) {
    ApplyExplainoPresetDefaults(FractalType::explaino_counterfactual_pair, params);
    params.max_iter = 96;
    params.counterfactual_pair_root_family = CounterfactualPairRootFamily::cubic_unit_roots;
    params.counterfactual_pair_frame = CounterfactualPairFrame::world_absolute;
    params.counterfactual_pair_offset_x = 0.16f;
    params.counterfactual_pair_offset_y = 0.08f;
    params.counterfactual_pair_reconvergence_ratio = 0.60f;
    ApplyDefaultColoringSelection(FractalType::explaino_counterfactual_pair, params);
}

static void ApplyEscapeTimePresetDefaults(FractalType fractalType, KernelParams& params, int maxIter, float exposure) {
    params.max_iter = maxIter;
    ApplyDefaultColoringSelection(fractalType, params);
    params.exposure = exposure;
}

static void ApplyHalleyPresetDefaults(FractalType fractalType, KernelParams& params) {
    params.max_iter = 500;
    params.epsilon = 1e-6f;
    params.poly_kind = PolyKind::z3_minus_1;
    SetPolyPreset(params);
    ApplyDefaultColoringSelection(fractalType, params);
    params.exposure = 1.0f;
}

static void ApplyPhoenixPresetDefaults(FractalType fractalType, KernelParams& params) {
    params.max_iter = 1200;
    params.epsilon = 1e-6f;
    params.nova_alpha = 0.50f;
    params.phoenix_p_real = 0.0f;
    params.explaino_cluster_radius = 0.0f;
    ApplyPhoenixStepCarrierDefaults(fractalType, params);
    ApplyExplainoClusterRadiusDefaults(fractalType, params);
    params.phoenix_p_imag = 0.0f;
    ApplyDefaultColoringSelection(fractalType, params);
    params.exposure = 1.6f;
}

static void ApplyLambdaPresetDefaults(FractalType fractalType, KernelParams& params) {
    ApplyEscapeTimePresetDefaults(fractalType, params, 1200, 1.4f);
    params.lambda_real = 2.9685855f;
    params.lambda_imag = -0.27446103f;
}

static void ApplyCollatzPresetDefaults(FractalType fractalType, KernelParams& params) {
    params.max_iter = 200;
    ApplyDefaultColoringSelection(fractalType, params);
    params.exposure = 1.0f;
    params.collatz_transition_strength = 1.0f;
}

static void ApplyExplainoCollatzDirectPresetDefaults(KernelParams& params) {
    ApplyExplainoPresetDefaults(FractalType::explaino_collatz_direct, params);
    params.max_iter = 300;
    params.exposure = 1.0f;
    params.collatz_transition_strength = 1.0f;
    params.explaino_warp_strength = 0.25f;
    ApplyDefaultColoringSelection(FractalType::explaino_collatz_direct, params);
}

static void ApplyMcMullenPresetDefaults(FractalType fractalType, KernelParams& params) {
    params.max_iter = 500;
    ApplyDefaultColoringSelection(fractalType, params);
    params.exposure = 1.2f;
    params.mcmullen_preset = McMullenPreset::z3_z3;
}

static void ApplyMagnetPresetDefaults(FractalType fractalType, KernelParams& params) {
    params.max_iter = 900;
    ApplyDefaultColoringSelection(fractalType, params);
    params.exposure = 1.35f;
    params.magnet_seed_real = 0.0f;
    params.magnet_seed_imag = 0.0f;
    params.magnet_relaxation = 1.0f;
    params.magnet_bailout = 12.0f;
}

void ApplyFractalPresetDefaults(const ViewState& view, KernelParams& params, bool* ioDirty) {
    ApplyCommonPresetDefaults(params);

    switch (view.fractal_type) {
    case FractalType::newton:
    case FractalType::nova:
        ApplyNewtonLikePresetDefaults(view.fractal_type, params);
        break;
    case FractalType::projection_and_flow:
        ApplyProjectionAndFlowPresetDefaults(params);
        break;
    case FractalType::counterfactual_pair:
        ApplyCounterfactualPairPresetDefaults(params);
        break;
    case FractalType::explaino_counterfactual_pair:
        ApplyExplainoCounterfactualPairPresetDefaults(params);
        break;
    case FractalType::explaino_projection_and_flow:
        ApplyExplainoProjectionAndFlowPresetDefaults(params);
        break;
    case FractalType::multicorn:
        ApplyEscapeTimePresetDefaults(view.fractal_type, params, 1200, 1.5f);
        params.multibrot_power = 2;
        params.multibrot_power_float = 2.0f;
        params.multibrot_power_imag = 0.0f;
        break;
    case FractalType::halley:
        ApplyHalleyPresetDefaults(view.fractal_type, params);
        break;
    case FractalType::phoenix:
        ApplyPhoenixPresetDefaults(view.fractal_type, params);
        break;
    case FractalType::mandelbrot:
    case FractalType::spider:
    case FractalType::celtic_mandelbrot:
    case FractalType::perpendicular_burning_ship:
    case FractalType::burning_ship:
        ApplyEscapeTimePresetDefaults(view.fractal_type, params, 1200, 1.5f);
        break;
    case FractalType::julia:
    case FractalType::multibrot:
        ApplyEscapeTimePresetDefaults(view.fractal_type, params, 1000, 1.4f);
        break;
    case FractalType::lambda_map:
        ApplyLambdaPresetDefaults(view.fractal_type, params);
        break;
    case FractalType::magnet:
        ApplyMagnetPresetDefaults(view.fractal_type, params);
        break;
    case FractalType::collatz:
        ApplyCollatzPresetDefaults(view.fractal_type, params);
        break;
    case FractalType::explaino_collatz_direct:
        ApplyExplainoCollatzDirectPresetDefaults(params);
        break;
    case FractalType::mcmullen:
        ApplyMcMullenPresetDefaults(view.fractal_type, params);
        break;
    default:
        if (IsExplainoPresetFractal(view.fractal_type)) {
            ApplyExplainoPresetDefaults(view.fractal_type, params);
            break;
        }
        params.max_iter = 800;
        ApplyDefaultColoringSelection(view.fractal_type, params);
        params.exposure = 1.0f;
        break;
    }

    MarkDirty(ioDirty);
}

void ApplyFractalPresetDefaultsForFractalSwitch(const ViewState& view, KernelParams& params, bool* ioDirty) {
    const ColorPipelineRuntimeSnapshot previousColor = CaptureColorPipelineRuntimeSnapshot(params);
    const bool canPreserveColor = CanRestoreColorPipelineRuntimeSnapshot(view.fractal_type, previousColor);
    ApplyFractalPresetDefaults(view, params, ioDirty);
    if (canPreserveColor) {
        RestoreColorPipelineRuntimeSnapshot(previousColor, params);
    }
}

struct ExplainoSeedShape {
    float a;
    float b;
    float c;
    float d;
};

static ExplainoSeedShape LerpExplainoShape(const ExplainoSeedShape& left, const ExplainoSeedShape& right, float mix) {
    ExplainoSeedShape out{};
    out.a = LerpF(left.a, right.a, mix);
    out.b = LerpF(left.b, right.b, mix);
    out.c = LerpF(left.c, right.c, mix);
    out.d = LerpF(left.d, right.d, mix);
    return out;
}

static ExplainoSeedShape ExplainoShapeForSeed(uint32_t seed, float phase, float spread, float phaseStrength) {
    const float r0 = Hash01(seed ^ 0x13579bdu);
    const float r1 = Hash01(seed ^ 0x2468aceu);
    const float r2 = Hash01(seed ^ 0xdeadbeefu);
    const float r3 = Hash01(seed ^ 0x9e3779b9u);

    const float baseR = 0.85f + 0.95f * spread;
    const float ps = 0.35f * phaseStrength;
    const float aAngle = (r0 * 6.2831853f) + ps * std::sin(phase * (0.15f + 0.20f * r2));
    const float cAngle = (r1 * 6.2831853f) + ps * std::cos(phase * (0.13f + 0.23f * r3));

    ExplainoSeedShape out{};
    out.a = baseR * std::cos(aAngle);
    out.b = (0.25f + 0.95f * std::fabs(std::sin(aAngle + 0.7f))) * (0.65f + 0.45f * r2);
    out.c = baseR * std::cos(cAngle);
    out.d = (0.25f + 0.95f * std::fabs(std::sin(cAngle - 0.4f))) * (0.65f + 0.45f * r3);
    return out;
}

static ExplainoSeedShape ExplainoShapeForCombinedSeed(double combinedSeed, bool seedTween, float phase, float spread, float phaseStrength) {
    if (!std::isfinite(combinedSeed)) combinedSeed = 0.0;

    const double seedFloor = std::floor(combinedSeed);
    float driftFrac = static_cast<float>(combinedSeed - seedFloor);
    if (!std::isfinite(driftFrac)) driftFrac = 0.0f;
    driftFrac = ClampF(driftFrac, 0.0f, 1.0f);

    const int seedBase = static_cast<int>(seedFloor);
    const uint32_t s0 = static_cast<uint32_t>(seedBase);
    const uint32_t s1 = static_cast<uint32_t>(seedBase + 1);

    ExplainoSeedShape shape0 = ExplainoShapeForSeed(s0, phase, spread, phaseStrength);
    if (!seedTween || driftFrac <= 0.0f) {
        return shape0;
    }

    ExplainoSeedShape shape1 = ExplainoShapeForSeed(s1, phase, spread, phaseStrength);
    float tweenFrac = static_cast<float>(ExplainoWedgeTween(static_cast<double>(driftFrac)));
    return LerpExplainoShape(shape0, shape1, tweenFrac);
}

static ExplainoSeedShape ResolveExplainoSeedShape(const ViewState& view, const KernelParams& params,
                                                  float phase, float spread, float phaseStrength) {
    ExplainoSeedShape shape = ExplainoShapeForCombinedSeed(
        ExplainoSeedCombined(view, params),
        view.explaino_seed_tween,
        phase,
        spread,
        phaseStrength);
    if (view.fractal_type != FractalType::explaino_dual) {
        return shape;
    }

    float mix = params.explaino_mix;
    if (!std::isfinite(mix)) mix = 0.5f;
    mix = ClampF(mix, 0.0f, 1.0f);
    ExplainoSeedShape shapeB = ExplainoShapeForCombinedSeed(
        params.explaino_seed_b,
        view.explaino_seed_tween,
        phase,
        spread,
        phaseStrength);
    return LerpExplainoShape(shape, shapeB, mix);
}

static void SetExplainoRootsForShape(FractalType fractalType, float clusterRadius,
                                     const ExplainoSeedShape& shape, KernelParams& params) {
    if (fractalType == FractalType::explaino_mult) {
        const float halfClusterRadius = ClampF(clusterRadius, 0.0f, 2.0f) * 0.5f;
        params.explaino_roots[0] = {shape.a + halfClusterRadius, shape.b};
        params.explaino_roots[1] = {shape.a - halfClusterRadius, shape.b};
        params.explaino_roots[2] = {shape.c, shape.d + halfClusterRadius};
        params.explaino_roots[3] = {shape.c, shape.d - halfClusterRadius};
        return;
    }

    params.explaino_roots[0] = {shape.a, shape.b};
    params.explaino_roots[1] = {shape.a, -shape.b};
    params.explaino_roots[2] = {shape.c, shape.d};
    params.explaino_roots[3] = {shape.c, -shape.d};
}

static void SetDegree4PolynomialCoefficientsFromRoots(const Float2 roots[4], float coeffs[5]) {
    const float r0x = roots[0].x, r0y = roots[0].y;
    const float r1x = roots[1].x, r1y = roots[1].y;
    const float r2x = roots[2].x, r2y = roots[2].y;
    const float r3x = roots[3].x, r3y = roots[3].y;

    const float s01x = r0x + r1x, s01y = r0y + r1y;
    const float p01x = r0x * r1x - r0y * r1y, p01y = r0x * r1y + r0y * r1x;
    const float s23x = r2x + r3x, s23y = r2y + r3y;
    const float p23x = r2x * r3x - r2y * r3y, p23y = r2x * r3y + r2y * r3x;

    coeffs[4] = 1.0f;
    coeffs[3] = -(s01x + s23x);
    coeffs[2] = p01x + (s01x * s23x - s01y * s23y) + p23x;
    coeffs[1] = -(p01x * s23x - p01y * s23y + s01x * p23x - s01y * p23y);
    coeffs[0] = p01x * p23x - p01y * p23y;
}

static void SetDegree3PolynomialCoefficientsFromRoots(const Float2 roots[3], float coeffs[5]) {
    const float a = roots[0].x;
    const float b = roots[0].y;
    const float c = roots[2].x;
    const float pairProduct = a * a + b * b;
    const float pairwiseSum = pairProduct + 2.0f * a * c;
    const float rootSum = 2.0f * a + c;
    const float tripleProduct = pairProduct * c;

    coeffs[4] = 0.0f;
    coeffs[3] = 1.0f;
    coeffs[2] = -rootSum;
    coeffs[1] = pairwiseSum;
    coeffs[0] = -tripleProduct;
}

static void SetExplainoProjectionAndFlowRootsForShape(ProjectionAndFlowRootFamily rootFamily,
                                                      const ExplainoSeedShape& shape,
                                                      KernelParams& params) {
    for (Float2& root : params.explaino_roots) {
        root = {0.0f, 0.0f};
    }

    if (rootFamily == ProjectionAndFlowRootFamily::quartic_unit_roots) {
        params.explaino_root_count = 4;
        SetExplainoRootsForShape(FractalType::explaino_projection_and_flow, params.explaino_cluster_radius, shape, params);
        SetDegree4PolynomialCoefficientsFromRoots(params.explaino_roots, params.poly_coeffs);
        return;
    }

    params.explaino_root_count = 3;
    params.explaino_roots[0] = {shape.a, shape.b};
    params.explaino_roots[1] = {shape.a, -shape.b};
    params.explaino_roots[2] = {
        ClampF(shape.c + 0.35f * std::copysign(shape.d, shape.c == 0.0f ? 1.0f : shape.c), -4.0f, 4.0f),
        0.0f,
    };
    SetDegree3PolynomialCoefficientsFromRoots(params.explaino_roots, params.poly_coeffs);
}

static void SetExplainoCounterfactualPairRootsForShape(CounterfactualPairRootFamily rootFamily,
                                                       const ExplainoSeedShape& shape,
                                                       KernelParams& params) {
    for (Float2& root : params.explaino_roots) {
        root = {0.0f, 0.0f};
    }

    if (rootFamily == CounterfactualPairRootFamily::quartic_unit_roots) {
        params.explaino_root_count = 4;
        SetExplainoRootsForShape(FractalType::explaino_counterfactual_pair, params.explaino_cluster_radius, shape, params);
        SetDegree4PolynomialCoefficientsFromRoots(params.explaino_roots, params.poly_coeffs);
        return;
    }

    params.explaino_root_count = 3;
    params.explaino_roots[0] = {shape.a, shape.b};
    params.explaino_roots[1] = {shape.a, -shape.b};
    params.explaino_roots[2] = {ClampF(shape.c, -4.0f, 4.0f), 0.0f};
    SetDegree3PolynomialCoefficientsFromRoots(params.explaino_roots, params.poly_coeffs);
}

static void UpdateExplainoSplicePolynomial(const ViewState& view, const KernelParams& params,
                                           float phase, float spread, float phaseStrength,
                                           float coeffs[5]) {
    const double offsetSeed = ExplainoSeedCombined(view, params) + (double)params.splice_offset;
    const ExplainoSeedShape shape = ExplainoShapeForCombinedSeed(
        offsetSeed,
        view.explaino_seed_tween,
        phase,
        spread,
        phaseStrength);
    const Float2 roots[4] = {
        {shape.a, shape.b},
        {shape.a, -shape.b},
        {shape.c, shape.d},
        {shape.c, -shape.d},
    };
    SetDegree4PolynomialCoefficientsFromRoots(roots, coeffs);
}

static void ClearPolynomialCoefficients(float coeffs[5]) {
    for (int i = 0; i < 5; ++i) {
        coeffs[i] = 0.0f;
    }
}

static bool IsExplainoRootEditorFractalType(FractalType fractalType) {
    switch (fractalType) {
    case FractalType::explaino_julia:
    case FractalType::explaino_lambda:
    case FractalType::explaino_rational_escape:
    case FractalType::explaino_collatz_direct:
    case FractalType::explaino_counterfactual_pair:
    case FractalType::explaino_projection_and_flow:
        return false;
    default:
        return IsExplainoFamily(fractalType);
    }
}

static int ClampExplainoCustomRootCount(int rootCount) {
    return rootCount == 3 ? 3 : 4;
}

static bool IsExplainoComposedVariantType(FractalType fractalType) {
    switch (fractalType) {
    case FractalType::explaino_all:
    case FractalType::explaino_ripple:
    case FractalType::explaino_splice:
    case FractalType::explaino_vortex:
    case FractalType::explaino_tension:
        return true;
    default:
        return false;
    }
}

void UpdateExplainoPolynomial(const ViewState& view, KernelParams& params, bool* ioDirty) {
    if (!IsExplainoFamily(view.fractal_type)) {
        params.explaino_root_count = 0;
        return;
    }

    if (!UsesExplainoCustomPolynomialAuthority(view.fractal_type)) {
        params.explaino_root_authority = ExplainoRootAuthority::generated;
        params.explaino_root_count = 0;
        ClearPolynomialCoefficients(params.poly_coeffs_b);
        SyncProjectionAndFlowRootFamilyPreset(params);
        if (ioDirty) *ioDirty = true;
        return;
    }

    if (params.explaino_root_authority == ExplainoRootAuthority::custom &&
        IsExplainoRootEditorFractalType(view.fractal_type)) {
        params.poly_kind = PolyKind::custom;
        params.explaino_root_count = ClampExplainoCustomRootCount(params.explaino_root_count);
        if (params.explaino_root_count == 3) {
            SetDegree3PolynomialCoefficientsFromRoots(params.explaino_roots, params.poly_coeffs);
        } else {
            SetDegree4PolynomialCoefficientsFromRoots(params.explaino_roots, params.poly_coeffs);
        }
        ClearPolynomialCoefficients(params.poly_coeffs_b);
        if (ioDirty) *ioDirty = true;
        return;
    }

    params.explaino_root_authority = ExplainoRootAuthority::generated;
    params.poly_kind = PolyKind::custom;
    params.explaino_root_count = 4;

    const float phase = view.explaino_phase;
    const float spread = std::fmax(0.0f, std::fmin(3.0f, params.explaino_root_spread));
    const float phaseStrength = view.explaino_phase_strength;

    const ExplainoSeedShape shape = ResolveExplainoSeedShape(view, params, phase, spread, phaseStrength);
    if (view.fractal_type == FractalType::explaino_projection_and_flow) {
        params.poly_kind = PolyKind::custom;
        SetExplainoProjectionAndFlowRootsForShape(params.projection_and_flow_root_family, shape, params);
        ClearPolynomialCoefficients(params.poly_coeffs_b);
        if (ioDirty) *ioDirty = true;
        return;
    }
    if (view.fractal_type == FractalType::explaino_counterfactual_pair) {
        params.poly_kind = PolyKind::custom;
        SetExplainoCounterfactualPairRootsForShape(params.counterfactual_pair_root_family, shape, params);
        ClearPolynomialCoefficients(params.poly_coeffs_b);
        if (ioDirty) *ioDirty = true;
        return;
    }

    SetExplainoRootsForShape(view.fractal_type, params.explaino_cluster_radius, shape, params);
    SetDegree4PolynomialCoefficientsFromRoots(params.explaino_roots, params.poly_coeffs);

    const bool needsSplicePolynomial =
        view.fractal_type == FractalType::explaino_splice ||
        (IsExplainoComposedVariantType(view.fractal_type) && params.splice_offset != 0.0f);
    if (needsSplicePolynomial) {
        UpdateExplainoSplicePolynomial(view, params, phase, spread, phaseStrength, params.poly_coeffs_b);
    } else {
        ClearPolynomialCoefficients(params.poly_coeffs_b);
    }

    if (ioDirty) *ioDirty = true;
}

void ApplyFractalDerivedFieldsAndSyncHp(ViewState& view, KernelParams& params, bool* ioDirty,
    bool haveExplainoSeedOverride, double explainoSeedOverride) {
    ApplyFractalPresetDefaults(view, params, ioDirty);
    if (haveExplainoSeedOverride) ExplainoSeedSetCombined(view, params, explainoSeedOverride);
    UpdateExplainoPolynomial(view, params, ioDirty);
    SyncViewHpFromUi(view);
}
