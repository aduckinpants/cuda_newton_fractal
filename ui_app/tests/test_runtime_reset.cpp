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
    if (render.resolution.x != 2048 || render.resolution.y != 1536 || render.block_size != 256 || render.device_id != 0 || render.benchmark) {
        std::cerr << "Reset should restore render defaults\n";
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