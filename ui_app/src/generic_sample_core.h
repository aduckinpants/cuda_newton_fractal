#pragma once
// Host-callable API for generic function sampling on the GPU.
// Mirrors SampleFractalPoints() but for GenericFunctionDesc expression trees.
// No dependencies on fractal_types.h, ViewState, KernelParams, DX11, or ImGui.

#include "generic_function_types.h"

// Sample a user-defined function at arbitrary complex-plane coordinates.
// coords: array of numPoints (x,y) pairs.
// outResults: caller-allocated array of numPoints GenericSampleResult.
// outError: set to a message string on failure (caller does not free).
// Returns true on success, false on error.
bool SampleGenericFunction(
    const GFPoint* coords,
    int numPoints,
    const GenericFunctionDesc& func,
    double epsilon,
    double escape_radius,
    GenericSampleResult* outResults,
    const char** outError);
