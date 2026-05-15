#include "runtime_reset.h"

#include "fractal_derived_fields.h"
#include "fractal_family_rules.h"
#include "view_hp_sync.h"

void ResetRuntimeStateForCurrentFractal(
    ViewState& view,
    KernelParams& params,
    RenderSettings& render,
    LensSettings& lens,
    bool* ioDirty) {
    const FractalType selectedFractalType = view.fractal_type;
    const bool canonicalizeLegacyProjection = IsExplainoLegacyProjectionSelector(selectedFractalType);
    if (canonicalizeLegacyProjection) {
        view.fractal_type = ExplainoCanonicalFractalType();
    }

    view.auto_refresh = false;
    view.camera_behavior = CameraBehavior::complexity;
    view.auto_dive = false;
    view.dive_speed = 1.0f;
    view.explaino_alive = false;
    view.explaino_seed_tween = true;
    view.explaino_phase = 0.0f;
    view.explaino_seed_drift = 0.0f;
    view.auto_increment_seed = false;
    view.explaino_seed_rate = 0.001f;
    view.explaino_phase_strength = 1.0f;
    ApplyFractalViewPresetDefaults(view, ioDirty);

    params.explaino_seed = 0.0;
    params.explaino_seed_b = 1.0;
    params.explaino_mix = 0.5f;
    params.explaino_warp_strength = 0.0f;
    params.explaino_root_spread = 0.5f;
    params.explaino_damping = 1.0f;
    params.explaino_cluster_radius = 0.0f;
    params.balance_void = 0.0f;
    params.symmetry_tension = 0.0f;
    params.field_curvature = 0.0f;
    params.ripple_amplitude = 0.0f;
    params.splice_offset = 0.0f;
    params.vortex_strength = 0.0f;
    params.tension_strength = 0.0f;
    ResetExplainoStructuralRegistryValues(params);
    ResetExplainoCouplingRegistryValues(params);
    ApplyFractalPresetDefaults(view, params, ioDirty);
    if (canonicalizeLegacyProjection) {
        ApplyExplainoAxisRegistryDefaults(selectedFractalType, params);
    }
    params.color_source_stack_count = 0;
    for (ColorPipelineSourceStackEntry& sourceEntry : params.color_source_stack) {
        sourceEntry = {};
    }
    params.color_root_basin_pair_count = 0;
    for (ColorPipelineSelection& pairSelection : params.color_root_basin_pairs) {
        pairSelection = {};
    }
    params.color_palette_stack_count = 0;
    for (ColorPipelinePaletteStackEntry& paletteEntry : params.color_palette_stack) {
        paletteEntry = {};
    }
    params.color_grading_stack_count = 0;
    for (ColorPipelineGradingStackEntry& gradingEntry : params.color_grading_stack) {
        gradingEntry = {};
    }
    params.color_glow = 0.25f;
    params.color_balance_void = 0.0f;
    params.color_chroma_tension = 0.0f;
    params.color_accent_bias = 0.0f;
    if (IsExplainoFamily(view.fractal_type)) {
        UpdateExplainoPolynomial(view, params, nullptr);
    }
    SyncViewHpFromUi(view);

    render.resolution = {RenderSettings::kDefaultWidth, RenderSettings::kDefaultHeight};
    render.block_size = 256;
    render.device_id = 0;
    render.benchmark = false;
    render.interaction_debounce_ms = RenderSettings::kDefaultInteractionDebounceMs;
    render.preview_target_fps = RenderSettings::kDefaultPreviewTargetFps;
    render.preview_min_scale = RenderSettings::kDefaultPreviewMinScale;

    lens.enabled = false;
    lens.downsample = 2;

    if (ioDirty) *ioDirty = true;
}
