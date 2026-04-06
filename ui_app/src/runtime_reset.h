#pragma once

#include "fractal_types.h"

void ResetRuntimeStateForCurrentFractal(
    ViewState& view,
    KernelParams& params,
    RenderSettings& render,
    LensSettings& lens,
    bool* ioDirty);