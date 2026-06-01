#pragma once

#include "sdf_pack_runtime_types.h"

bool SampleSdfPackCuda(
    const SdfPackGpuPoint* points,
    int pointCount,
    const SdfPackRuntimeDesc& desc,
    SdfPackGpuSample* outSamples,
    const char** outError);

bool SampleSdfPackGridCuda(
    int width,
    int height,
    double centerX,
    double centerY,
    double halfWidth,
    double halfHeight,
    double pixelScale,
    const SdfPackRuntimeDesc& desc,
    float* outSignedDistancePx,
    const char** outError);
