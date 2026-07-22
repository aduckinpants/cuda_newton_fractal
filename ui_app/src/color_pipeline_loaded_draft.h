#pragma once

#include "fractal_types.h"

#include <string>

struct ColorPipelineWindowState;

struct ColorPipelineLoadedDraftApplyResult {
    bool changed = false;
};

// Deliberately promotes one already-deserialized Color Pipeline draft into the
// effective runtime state. Ordinary state loading does not call this operation.
bool ApplyLoadedColorPipelineDraftToRuntime(
    ColorPipelineWindowState* ioDraft,
    FractalType liveFractalType,
    KernelParams* ioParams,
    ColorPipelineLoadedDraftApplyResult* outResult,
    std::string* outError);
