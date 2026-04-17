#include "viewer_state_init.h"

#include "explaino_seed.h"
#include "explaino_seed_dynamics.h"
#include "finding_state_actions.h"
#include "fractal_derived_fields.h"
#include "fractal_family_rules.h"
#include "view_hp_sync.h"

namespace {

bool CliOverridesLoadedStateSnapshot(const ViewerCliArgs& cli) {
    return cli.have_fractal_type ||
        cli.have_explaino_seed ||
        cli.have_explaino_phase ||
        cli.have_explaino_seed_drift ||
        cli.have_explaino_seed_b ||
        cli.have_explaino_mix ||
        cli.have_explaino_warp_strength ||
        cli.have_lambda_real ||
        cli.have_lambda_imag ||
        cli.have_width ||
        cli.have_height ||
        cli.sweep_config.enabled;
}

} // namespace

int ApplyCliOverrides(const ViewerCliArgs& cli,
                      ViewState& view, KernelParams& params,
                      RenderSettings& render,
                      SidecarAutoDemoControllerPolicy* ioSidecarControllerPolicy,
                      SidecarOrientationVector* outLoadedOrientation,
                      bool* outHasLoadedOrientation,
                      SidecarAutoDemoMutationHistory* outLoadedMutationHistory,
                      bool* outHasLoadedMutationHistory,
                      std::string* outResolvedLoadedStatePath,
                      bool* dirty) {
    if (outLoadedOrientation) *outLoadedOrientation = {};
    if (outHasLoadedOrientation) *outHasLoadedOrientation = false;
    if (outLoadedMutationHistory) outLoadedMutationHistory->clear();
    if (outHasLoadedMutationHistory) *outHasLoadedMutationHistory = false;
    if (outResolvedLoadedStatePath) outResolvedLoadedStatePath->clear();

    bool loadedState = false;

    if (cli.have_load_state_json) {
        std::string loadError;
        std::string loadedStatePath;
        SidecarOrientationVector loadedOrientation{};
        bool hasLoadedOrientation = false;
        SidecarAutoDemoControllerPolicy loadedControllerPolicy{};
        bool hasLoadedControllerPolicy = false;
        SidecarAutoDemoMutationHistory loadedMutationHistory;
        bool hasLoadedMutationHistory = false;
        if (!LoadFindingSelectionIntoRuntime(
                cli.load_state_json,
                &view,
                &params,
                &render,
                &loadedOrientation,
                &hasLoadedOrientation,
                &loadedControllerPolicy,
                &hasLoadedControllerPolicy,
            &loadedMutationHistory,
            &hasLoadedMutationHistory,
                &loadedStatePath,
                &loadError)) {
            return 1;
        }
        loadedState = true;
        if (outResolvedLoadedStatePath) *outResolvedLoadedStatePath = loadedStatePath;
        if (ioSidecarControllerPolicy) {
            *ioSidecarControllerPolicy = hasLoadedControllerPolicy ? loadedControllerPolicy : SidecarAutoDemoControllerPolicy{};
        }
        if (outLoadedOrientation) *outLoadedOrientation = loadedOrientation;
        if (outHasLoadedOrientation) *outHasLoadedOrientation = hasLoadedOrientation;
        if (outLoadedMutationHistory) *outLoadedMutationHistory = loadedMutationHistory;
        if (outHasLoadedMutationHistory) *outHasLoadedMutationHistory = hasLoadedMutationHistory;
        if (dirty) *dirty = true;
    }

    if (cli.have_fractal_type) {
        view.fractal_type = cli.fractal_type;
        ApplyFractalViewPresetDefaults(view, dirty);
        if (dirty) *dirty = true;
    } else if (cli.have_explaino_seed) {
        view.fractal_type = FractalType::explaino;
        ApplyFractalViewPresetDefaults(view, dirty);
        if (dirty) *dirty = true;
    } else if (cli.sweep_config.enabled) {
        view.fractal_type = FractalType::explaino;
        ApplyFractalViewPresetDefaults(view, dirty);
        if (dirty) *dirty = true;
    }

    if (cli.sweep_config.enabled && !IsExplainoFamily(view.fractal_type)) {
        return 1;
    }

    if (cli.have_width) render.resolution.x = cli.width;
    if (cli.have_height) render.resolution.y = cli.height;

    const bool needPresetDerivedFields = !loadedState || cli.have_fractal_type || cli.have_explaino_seed || cli.sweep_config.enabled;
    if (needPresetDerivedFields) {
        ApplyFractalDerivedFieldsAndSyncHp(view, params, dirty, cli.have_explaino_seed, cli.explaino_seed);
    } else {
        if (IsExplainoFamily(view.fractal_type)) {
            UpdateExplainoPolynomial(view, params, dirty);
        }
        SyncViewUiFromHp(view);
    }

    if (cli.have_explaino_seed) ExplainoSeedSetCombined(view, params, cli.explaino_seed);
    if (cli.have_explaino_phase) view.explaino_phase = (float)cli.explaino_phase;
    if (cli.have_explaino_seed_drift) view.explaino_seed_drift = (float)cli.explaino_seed_drift;
    if (cli.have_explaino_seed_b) params.explaino_seed_b = cli.explaino_seed_b;
    if (cli.have_explaino_mix) params.explaino_mix = (float)cli.explaino_mix;
    if (cli.have_explaino_warp_strength) params.explaino_warp_strength = (float)cli.explaino_warp_strength;
    if (cli.have_lambda_real) params.lambda_real = (float)cli.lambda_real;
    if (cli.have_lambda_imag) params.lambda_imag = (float)cli.lambda_imag;

    if (IsExplainoFamily(view.fractal_type)) {
        UpdateExplainoPolynomial(view, params, dirty);
    }

    if (loadedState && CliOverridesLoadedStateSnapshot(cli)) {
        if (outLoadedOrientation) *outLoadedOrientation = {};
        if (outHasLoadedOrientation) *outHasLoadedOrientation = false;
        if (outLoadedMutationHistory) outLoadedMutationHistory->clear();
        if (outHasLoadedMutationHistory) *outHasLoadedMutationHistory = false;
    }

    return 0;
}

int ApplyCliOverrides(const ViewerCliArgs& cli,
                      ViewState& view, KernelParams& params,
                      RenderSettings& render, bool* dirty) {
    return ApplyCliOverrides(cli, view, params, render, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, dirty);
}
