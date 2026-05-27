#pragma once

#include "color_pipeline_sdf_postprocess.h"

bool ApplyLensSdfColorPipelinePostprocessCudaDirectScalar(
    const SdfFieldView& field,
    const RenderSettings& render,
    const KernelParams& params,
    std::uint32_t* ioRgba,
    std::string* outError = nullptr,
    SdfColorPipelinePostprocessStats* outStats = nullptr,
    const SdfColorPipelinePostprocessOptions* options = nullptr);
