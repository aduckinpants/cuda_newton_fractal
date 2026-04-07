#include "render_capture_guard.h"

namespace {

std::size_t ExpectedRenderPixelCount(const RenderSettings& render) {
    if (render.resolution.x <= 0 || render.resolution.y <= 0) return 0;
    return static_cast<std::size_t>(render.resolution.x) * static_cast<std::size_t>(render.resolution.y);
}

} // namespace

bool ShouldDispatchRender(bool continuousRender,
    bool dirty,
    bool renderOnceRequested,
    bool captureDiagnosticRequested,
    bool captureFindingRequested,
    bool fullQualityRequested) {
    return continuousRender || dirty || renderOnceRequested || captureDiagnosticRequested || captureFindingRequested || fullQualityRequested;
}

void InvalidateRenderedFrame(RenderedFrameState* ioState) {
    if (!ioState) return;
    ioState->ready = false;
    ioState->width = 0;
    ioState->height = 0;
}

void MarkRenderedFrameReady(const RenderSettings& render, RenderedFrameState* ioState) {
    if (!ioState) return;
    ioState->ready = render.resolution.x > 0 && render.resolution.y > 0;
    ioState->width = render.resolution.x;
    ioState->height = render.resolution.y;
}

bool HasExactRenderPixelCount(const RenderSettings& render, std::size_t pixelCount, std::string* outError) {
    const std::size_t expected = ExpectedRenderPixelCount(render);
    if (expected == 0) {
        if (outError) *outError = "render resolution must be positive before capture";
        return false;
    }
    if (pixelCount != expected) {
        if (outError) *outError = "capture pixel count does not match render resolution";
        return false;
    }
    return true;
}

bool CanCaptureRenderedFrame(const RenderSettings& render,
    std::size_t pixelCount,
    const RenderedFrameState& frame,
    std::string* outError) {
    if (!frame.ready) {
        if (outError) *outError = "no current rendered frame is available for capture";
        return false;
    }
    if (frame.width != render.resolution.x || frame.height != render.resolution.y) {
        if (outError) *outError = "current rendered frame resolution does not match the requested capture resolution";
        return false;
    }
    return HasExactRenderPixelCount(render, pixelCount, outError);
}