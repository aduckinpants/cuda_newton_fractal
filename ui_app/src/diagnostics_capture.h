#pragma once

#include "fractal_types.h"

#include <string>

struct DiagnosticsCaptureResult {
    std::string output_dir;
    std::string frame_bmp_path;
    std::string state_json_path;
};

bool CaptureDiagnosticsLastBundle(const std::string& exeDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    DiagnosticsCaptureResult* outResult,
    std::string* outError);