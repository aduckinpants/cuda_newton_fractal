#pragma once

#include "fractal_types.h"
#include "lens_sdf.h"

#include <cstdint>
#include <cstddef>
#include <string>

bool IsColorPipelineSdfSourceSignal(ColorSignal signal);
bool ColorPipelineUsesSdfSource(const KernelParams& params);
bool ColorPipelineSourceStackIsSdfOnly(const KernelParams& params, std::string* outError = nullptr);
bool ColorPipelineSdfPostprocessCanUseDirectSamples(const KernelParams& params);
int ResolveSdfColorPipelinePostprocessOutputPixelStep(const KernelParams& params, bool previewActive, double previewScale, bool forceFullQuality);

struct SdfColorPipelinePostprocessOptions {
    int output_pixel_step{1};
    int max_worker_threads{0};
};

struct SdfColorPipelinePostprocessStats {
    std::size_t direct_sample_count{0};
    std::size_t neighborhood_sample_count{0};
    std::size_t source_direct_sample_count{0};
    std::size_t source_neighborhood_sample_count{0};
    std::size_t filled_pixel_count{0};
    int output_pixel_step{1};
    int worker_count{1};
};

bool ApplyLensSdfColorPipelinePostprocess(
    const SdfFieldView& field,
    const RenderSettings& render,
    const KernelParams& params,
    std::uint32_t* ioRgba,
    std::string* outError = nullptr,
    SdfColorPipelinePostprocessStats* outStats = nullptr,
    const SdfColorPipelinePostprocessOptions* options = nullptr);
