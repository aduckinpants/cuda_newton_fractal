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
        render.interaction_debounce_ms = 450;
        render.preview_target_fps = 20.0f;
        render.preview_min_scale = 0.40f;

        const ViewerRenderPacingConfig config = BuildViewerRenderPacingConfig(render);
        if (!NearlyEqual(config.debounce_seconds, 0.45)) {
            std::cerr << "Expected render pacing config to convert debounce milliseconds to seconds\n";
            return 1;
        }
        if (!NearlyEqual(config.target_frame_ms, 50.0)) {
            std::cerr << "Expected render pacing config to convert preview target FPS into frame time\n";
            return 1;
        }
        if (!NearlyEqual(config.min_preview_scale, 0.40)) {
            std::cerr << "Expected render pacing config to preserve the preview minimum scale\n";
            return 1;
        }
    }

    {
        RenderSettings render{};
        render.resolution = {1024, 768};
        RenderStats stats{};
        stats.last_render_ms = 120.0f;
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
        if (state.preview_step_index != 1 || decision.preview_step_index != 1) {
            std::cerr << "Expected a stepped controller to enter the first preview level before dropping further\n";
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
        RenderStats slowStats{};
        slowStats.last_render_ms = 120.0f;
        RenderStats stillSlowPreviewStats{};
        stillSlowPreviewStats.last_render_ms = 90.0f;
        ViewerRenderPacingConfig config{};
        ViewerRenderPacingState state{};

        NoteViewerInteraction(&state);
        ViewerRenderPacingDecision first = AdvanceViewerRenderPacing(render, slowStats, 0.05, config, &state);
        ViewerRenderPacingDecision second = AdvanceViewerRenderPacing(render, stillSlowPreviewStats, 0.05, config, &state);

        if (!first.preview_active || !second.preview_active) {
            std::cerr << "Expected slow preview timing to keep preview mode active\n";
            return 1;
        }
        if (second.preview_step_index <= first.preview_step_index) {
            std::cerr << "Expected stepped preview controller to drop another level when preview frames stay slow\n";
            return 1;
        }
        if (second.render_resolution.x >= first.render_resolution.x || second.render_resolution.y >= first.render_resolution.y) {
            std::cerr << "Expected a deeper preview step to reduce resolution further\n";
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
        if (state.preview_step_index != 0) {
            std::cerr << "Expected settle render to reset the preview step index\n";
            return 1;
        }
    }

    {
        RenderSettings render{};
        render.resolution = {1024, 768};
        RenderStats stats{};
        stats.last_render_ms = 8.0f;
        ViewerRenderPacingConfig config{};
        ViewerRenderPacingState state{};
        state.seconds_since_interaction = 0.05;
        state.settle_render_pending = true;
        state.preview_step_index = 3;
        state.active_preview_scale = 0.625;

        ViewerRenderPacingDecision decision = AdvanceViewerRenderPacing(render, stats, 0.05, config, &state);
        if (!decision.preview_active) {
            std::cerr << "Expected fast preview timing during the debounce window to keep preview mode active while stepping up\n";
            return 1;
        }
        if (decision.preview_step_index >= 3 || state.preview_step_index >= 3) {
            std::cerr << "Expected stepped preview controller to move back toward higher resolution when preview timing is comfortably fast\n";
            return 1;
        }
    }

    std::cout << "test_viewer_render_pacing: all passed\n";
    return 0;
}