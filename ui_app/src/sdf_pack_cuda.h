#pragma once

#include "sdf_pack_runtime_types.h"

bool SampleSdfPackCuda(
    const SdfPackGpuPoint* points,
    int pointCount,
    const SdfPackRuntimeDesc& desc,
    SdfPackGpuSample* outSamples,
    const char** outError);
