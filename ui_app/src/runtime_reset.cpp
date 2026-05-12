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
    params.momentum_beta = 0.0f;
    ApplyFractalPresetDefaults(view, params, ioDirty);
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