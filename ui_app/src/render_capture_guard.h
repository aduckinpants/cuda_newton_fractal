#pragma once

#include "fractal_types.h"

#include <cstddef>
#include <string>

struct RenderedFrameState {
    bool ready{false};
    int width{0};
    int height{0};
};

bool ShouldDispatchRender(bool autoRefresh,
    bool dirty,
    bool renderOnceRequested,
    bool captureDiagnosticRequested,
    bool captureFindingRequested);

void InvalidateRenderedFrame(RenderedFrameState* ioState);

void MarkRenderedFrameReady(const RenderSettings& render, RenderedFrameState* ioState);

bool HasExactRenderPixelCount(const RenderSettings& render, std::size_t pixelCount, std::string* outError);

bool CanCaptureRenderedFrame(const RenderSettings& render,
    std::size_t pixelCount,
    const RenderedFrameState& frame,
    std::string* outError);