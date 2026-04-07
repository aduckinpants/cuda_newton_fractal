#include "../src/render_capture_guard.h"

#include <iostream>

int main() {
    {
        if (ShouldDispatchRender(false, false, false, false, false, false)) {
            std::cerr << "Expected no render dispatch when nothing requested\n";
            return 1;
        }
        if (!ShouldDispatchRender(true, false, false, false, false, false) ||
            !ShouldDispatchRender(false, true, false, false, false, false) ||
            !ShouldDispatchRender(false, false, true, false, false, false) ||
            !ShouldDispatchRender(false, false, false, true, false, false) ||
            !ShouldDispatchRender(false, false, false, false, true, false) ||
            !ShouldDispatchRender(false, false, false, false, false, true)) {
            std::cerr << "Expected render dispatch for each explicit trigger\n";
            return 1;
        }
    }

    {
        RenderSettings render{};
        render.resolution = {2048, 1536};

        RenderedFrameState frame{};
        std::string error;
        if (CanCaptureRenderedFrame(render, static_cast<size_t>(render.resolution.x) * static_cast<size_t>(render.resolution.y), frame, &error)) {
            std::cerr << "Expected capture guard to reject missing rendered frames\n";
            return 1;
        }

        MarkRenderedFrameReady(render, &frame);
        if (!CanCaptureRenderedFrame(render, static_cast<size_t>(render.resolution.x) * static_cast<size_t>(render.resolution.y), frame, &error)) {
            std::cerr << "Expected capture guard to accept matching rendered frames: " << error << "\n";
            return 1;
        }
        if (CanCaptureRenderedFrame(render, static_cast<size_t>(render.resolution.x) * static_cast<size_t>(render.resolution.y - 1), frame, &error)) {
            std::cerr << "Expected capture guard to reject mismatched pixel counts\n";
            return 1;
        }

        render.resolution = {1536, 1536};
        if (CanCaptureRenderedFrame(render, static_cast<size_t>(render.resolution.x) * static_cast<size_t>(render.resolution.y), frame, &error)) {
            std::cerr << "Expected capture guard to reject stale rendered dimensions\n";
            return 1;
        }
    }

    std::cout << "test_render_capture_guard: all passed\n";
    return 0;
}