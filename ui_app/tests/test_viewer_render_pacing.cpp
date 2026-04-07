#include "../src/viewer_render_pacing.h"

#include <cstdlib>
#include <cmath>
#include <iostream>

namespace {

bool NearlyEqual(double a, double b, double eps = 1.0e-6) {
    return std::fabs(a - b) <= eps;
}

} // namespace

int main() {
    {
        RenderSettings render{};
        render.resolution = {2048, 1536};
        RenderStats stats{};
        stats.last_render_ms = 80.0f;
        ViewerRenderPacingConfig config{};
        ViewerRenderPacingState state{};

        NoteViewerInteraction(&state);
        ViewerRenderPacingDecision decision = AdvanceViewerRenderPacing(render, stats, 0.05, config, &state);

        if (!decision.preview_active) {
            std::cerr << "Expected slow interaction to enter preview mode\n";
            return 1;
        }
        if (decision.render_resolution.x >= render.resolution.x || decision.render_resolution.y >= render.resolution.y) {
            std::cerr << "Expected preview resolution to be smaller than the base resolution\n";
            return 1;
        }
        if (!state.settle_render_pending) {
            std::cerr << "Expected preview mode to require one restoring full-quality render\n";
            return 1;
        }
        const long long aspectError = std::llabs(
            static_cast<long long>(decision.render_resolution.x) * static_cast<long long>(render.resolution.y) -
            static_cast<long long>(decision.render_resolution.y) * static_cast<long long>(render.resolution.x));
        if (aspectError > render.resolution.x) {
            std::cerr << "Expected preview scaling to preserve the aspect ratio within integer rounding\n";
            return 1;
        }
    }

    {
        RenderSettings render{};
        render.resolution = {1024, 768};
        RenderStats stats{};
        stats.last_render_ms = 10.0f;
        ViewerRenderPacingConfig config{};
        ViewerRenderPacingState state{};

        NoteViewerInteraction(&state);
        ViewerRenderPacingDecision decision = AdvanceViewerRenderPacing(render, stats, 0.05, config, &state);

        if (decision.preview_active) {
            std::cerr << "Expected fast frames to stay at full resolution during interaction\n";
            return 1;
        }
        if (decision.full_quality_due) {
            std::cerr << "Did not expect an immediate settle render for full-resolution interaction frames\n";
            return 1;
        }
        if (state.settle_render_pending) {
            std::cerr << "Full-resolution interaction should not schedule an extra settle render\n";
            return 1;
        }
    }

    {
        RenderSettings render{};
        render.resolution = {2048, 1536};
        RenderStats stats{};
        stats.last_render_ms = 90.0f;
        ViewerRenderPacingConfig config{};
        ViewerRenderPacingState state{};

        NoteViewerInteraction(&state);
        ViewerRenderPacingDecision preview = AdvanceViewerRenderPacing(render, stats, 0.05, config, &state);
        if (!preview.preview_active || !state.settle_render_pending) {
            std::cerr << "Expected preview phase before debounce expiry\n";
            return 1;
        }

        ViewerRenderPacingDecision settle = AdvanceViewerRenderPacing(render, stats, 0.20, config, &state);
        if (settle.preview_active) {
            std::cerr << "Expected debounce expiry to end preview mode\n";
            return 1;
        }
        if (!settle.full_quality_due) {
            std::cerr << "Expected exactly one full-quality render when preview mode settles\n";
            return 1;
        }
        if (state.settle_render_pending) {
            std::cerr << "Expected settle render request to clear after it is issued\n";
            return 1;
        }
        if (!NearlyEqual(state.active_preview_scale, 1.0)) {
            std::cerr << "Expected settle render to reset the active preview scale\n";
            return 1;
        }
    }

    std::cout << "test_viewer_render_pacing: all passed\n";
    return 0;
}