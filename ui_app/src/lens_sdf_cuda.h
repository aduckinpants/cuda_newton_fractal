#pragma once

#include "lens_sdf.h"

bool ComputeLensSdfFieldCudaJfa(
    const uint8_t* mask,
    int width,
    int height,
    SdfFieldResult& outField);
