#pragma once

#include "fractal_types.h"
#include "lens_sdf.h"

#include <cstdint>
#include <string>

bool IsColorPipelineSdfSourceSignal(ColorSignal signal);
bool ColorPipelineUsesSdfSource(const KernelParams& params);
bool ColorPipelineSourceStackIsSdfOnly(const KernelParams& params, std::string* outError = nullptr);

bool ApplyLensSdfColorPipelinePostprocess(
    const SdfFieldView& field,
    const RenderSettings& render,
    const KernelParams& params,
    std::uint32_t* ioRgba,
    std::string* outError = nullptr);
