#pragma once

#include "fractal_types.h"

#include <string>
#include <string_view>

std::string BuildFractalPresetCoreJson(
    const ViewState& view,
    const KernelParams& params,
    const LensSettings& lens,
    std::string* outError = nullptr);

bool ApplyFractalPresetCoreJson(
    std::string_view json,
    ViewState* outView,
    KernelParams* outParams,
    LensSettings* outLens,
    std::string* outError = nullptr);
