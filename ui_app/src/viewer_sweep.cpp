#include "viewer_sweep.h"

#include "explaino_seed.h"
#include "fractal_derived_fields.h"

bool ApplySweepPlayback(const SweepPlayerConfig& config,
    bool paused,
    bool singleStep,
    double deltaSeconds,
    SweepPlayerState* ioState,
    ViewState* ioView,
    KernelParams* ioParams,
    bool* outDirty) {
    if (outDirty) *outDirty = false;
    if (!ioState || !ioView || !ioParams) return false;
    if (paused && !singleStep) return true;

    bool sweepChanged = false;
    const double effectiveDeltaSeconds = singleStep ? config.dwell_seconds : deltaSeconds;
    if (!AdvanceSweepPlayer(config, effectiveDeltaSeconds, ioState, &sweepChanged)) {
        return false;
    }
    if (!sweepChanged) {
        return true;
    }

    double currentSweepSeed = 0.0;
    if (!SweepPlayerCurrentSeed(*ioState, &currentSweepSeed)) {
        return false;
    }

    ExplainoSeedSetCombined(*ioView, *ioParams, currentSweepSeed);
    UpdateExplainoPolynomial(*ioView, *ioParams, nullptr);
    if (outDirty) *outDirty = true;
    return true;
}