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
    view.auto_refresh = true;
    view.camera_behavior = CameraBehavior::complexity;
    view.auto_dive = false;
    view.dive_speed = 1.0f;
    view.explaino_alive = false;
    view.explaino_seed_tween = true;
    view.explaino_phase = 0.0f;
    view.explaino_seed_drift = 0.0f;
    view.auto_increment_seed = false;
    view.explaino_seed_rate = 0.05f;
    view.explaino_phase_strength = 1.0f;
    ApplyFractalViewPresetDefaults(view, ioDirty);

    params.explaino_seed = 0.0;
    params.explaino_warp_strength = 0.0f;
    params.explaino_root_spread = 0.5f;
    params.explaino_damping = 1.0f;
    ApplyFractalPresetDefaults(view, params, ioDirty);
    if (IsExplainoFamily(view.fractal_type)) {
        UpdateExplainoPolynomial(view, params, nullptr);
    }
    SyncViewHpFromUi(view);

    render.resolution = {1024, 768};
    render.block_size = 256;
    render.device_id = 0;
    render.benchmark = false;

    lens.enabled = false;
    lens.downsample = 2;

    if (ioDirty) *ioDirty = true;
}