#pragma once

#include "fractal_types.h"
#include "sweep_player.h"

bool ApplySweepPlayback(const SweepPlayerConfig& config,
    bool paused,
    bool singleStep,
    double deltaSeconds,
    SweepPlayerState* ioState,
    ViewState* ioView,
    KernelParams* ioParams,
    bool* outDirty);