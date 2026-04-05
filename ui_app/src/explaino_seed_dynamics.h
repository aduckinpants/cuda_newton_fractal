#pragma once

#include "fractal_types.h"

bool ApplyExplainoSeedDynamics(const RenderStats& stats,
    double deltaSeconds,
    ViewState& view,
    KernelParams& params);