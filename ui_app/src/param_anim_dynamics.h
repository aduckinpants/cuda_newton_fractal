#pragma once

#include "fractal_types.h"

// Applies one frame of parameter animation based on the active target.
// Returns true if any parameter was modified.
bool ApplyParamAnimDynamics(double deltaSeconds, ViewState& view, KernelParams& params);
