#pragma once

#include "fractal_types.h"

#include <string>

bool LoadFindingSelectionIntoRuntime(const std::string& selectedPath,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    std::string* outResolvedStatePath,
    std::string* outError);