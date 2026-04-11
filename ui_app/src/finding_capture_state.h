#pragma once

#include "fractal_types.h"

// Prepares derived runtime state used by finding capture.
// Returns true when capture preparation may have changed runtime-derived inputs,
// meaning dependent caches should be invalidated.
bool PrepareFindingCaptureRuntimeState(ViewState& view, KernelParams& params);