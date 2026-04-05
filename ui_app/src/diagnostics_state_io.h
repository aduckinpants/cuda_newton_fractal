#pragma once

#include "fractal_types.h"

#include <string>

bool LoadDiagnosticsStateJson(const std::string& text,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    std::string* outError);

bool LoadDiagnosticsStateFile(const std::string& path,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    std::string* outError);

bool ResolveFindingStateJsonPath(const std::string& selectedPath,
    std::string* outStateJsonPath,
    std::string* outError);