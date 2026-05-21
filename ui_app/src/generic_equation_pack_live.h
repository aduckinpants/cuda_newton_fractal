#pragma once

#include "fractal_types.h"
#include "generic_equation_pack.h"

#include <cstdint>
#include <string>

bool RenderGenericEquationPackLiveFrame(
    const GenericEquationPack& pack,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    std::uint32_t* outRGBA,
    std::uint8_t* outMask,
    RenderStats* outStats,
    std::string* outError);
