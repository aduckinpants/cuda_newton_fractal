#pragma once

#include "fractal_types.h"

#include <cstdint>

uint32_t HashU32(uint32_t x);
float Hash01(uint32_t x);

void SetPolyPreset(KernelParams& params);
void ApplyFractalPresetDefaults(const ViewState& view, KernelParams& params, bool* ioDirty);
void UpdateExplainoPolynomial(const ViewState& view, KernelParams& params, bool* ioDirty);
void ApplyFractalDerivedFieldsAndSyncHp(ViewState& view, KernelParams& params, bool* ioDirty,
    bool haveExplainoSeedOverride, double explainoSeedOverride);