#pragma once

#include <cmath>

#include "fractal_types.h"

// Explaino seed model.
// - params.explaino_seed stores the integral base.
// - view.explaino_seed_drift stores the fractional drift in [0,1).
// - The combined value is base + drift.

double ExplainoSeedCombined(const ViewState& view, const KernelParams& params);
void ExplainoSeedSetCombined(ViewState& view, KernelParams& params, double combined);
void ExplainoSeedNormalize(ViewState& view, KernelParams& params);