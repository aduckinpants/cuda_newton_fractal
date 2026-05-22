#include "../src/viewer_render_pacing.h"

#include <cmath>
#include <cstdlib>
#include <iostream>

namespace {

bool NearlyEqual(double a, double b, double eps = 1.0e-6) {
    return std::fabs(a - b) <= eps;
}

double DefaultTargetFrameMs() {
    return 1000.0 / static_cast<double>(RenderSettings::kDefaultPreviewTargetFps);
}

int ScaledDimension(int value, double scale) {
    int result = static_cast<int>(std::floor(static_cast<double>(value) * scale + 0.5));
    if (result < 64) result = 64;
    if (result > value) result = value;
    return result;
}

} // namespace

int main() {
    {
        RenderSettings render{};
        const ViewerRenderPacingConfig config = BuildViewerRenderPacingConfig(render);
        if (!NearlyEqual(config.debounce_seconds, 0.20)) {
            std::cerr << "Expected default render pacing config to preserve the 200ms interaction debounce\n";
            return 1;
        }
        if (!NearlyEqual(config.target_frame_ms, DefaultTargetFrameMs())) {
            std::cerr << "Expected default render pacing config to preserve the 30 FPS preview target\n";
            return 1;
        }
        if (!NearlyEqual(config.min_preview_scale, 0.50)) {
            std::cerr << "Expected default render pacing config to preserve the 0.5 preview floor\n";
            return 1;
        }
    }

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
        render.resolution = {2048, 1536};
        RenderStats stats{};
        stats.last_render_ms = 90.0f;
        const ViewerRenderPacingConfig config = BuildViewerRenderPacingConfig(render);
        ViewerRenderPacingState state{};

        NoteViewerInteraction(&state);
        ViewerRenderPacingDecision decision = AdvanceViewerRenderPacing(render, stats, 0.05, config, &state);
        const double expectedScale = std::sqrt(DefaultTargetFrameMs() / 90.0);

        if (!decision.preview_active) {
            std::cerr << "Expected slow interaction frames to enter preview immediately\n";
            return 1;
        }
        if (!NearlyEqual(decision.preview_scale, expectedScale, 1.0e-6)) {
            std::cerr << "Expected preview scale to come from sqrt(target_frame_ms / last_render_ms)\n";
            return 1;
        }
        if (decision.render_resolution.x != ScaledDimension(render.resolution.x, expectedScale) ||
            decision.render_resolution.y != ScaledDimension(render.resolution.y, expectedScale)) {
            std::cerr << "Expected budget scale to drive the preview render resolution\n";
            return 1;
        }
        if (!state.settle_render_pending) {
            std::cerr << "Expected preview mode to require one restoring full-quality render\n";
            return 1;
        }
    }

    {
        RenderSettings render{};
        render.resolution = {2048, 1536};
        RenderStats stats{};
        stats.last_render_ms = 0.0f;
        const ViewerRenderPacingConfig config = BuildViewerRenderPacingConfig(render);
        ViewerRenderPacingState state{};

        NoteViewerInteraction(&state);
        ViewerRenderPacingDecision decision = AdvanceViewerRenderPacing(render, stats, 0.01, config, &state);
        if (decision.preview_active) {
            std::cerr << "Expected unknown timing to stay full resolution until a slow frame is measured\n";
            return 1;
        }
        if (!NearlyEqual(decision.preview_scale, 1.0) ||
            decision.render_resolution.x != 2048 || decision.render_resolution.y != 1536) {
            std::cerr << "Expected unknown timing to preserve full-resolution output\n";
            return 1;
        }
        if (state.settle_render_pending) {
            std::cerr << "Expected unknown timing without preview to avoid scheduling a settle render\n";
            return 1;
        }
    }

    {
        RenderSettings render{};
        render.resolution = {2048, 1536};
        RenderStats stats{};
        stats.last_render_ms = 35.0f;
        ViewerRenderPacingConfig config = BuildViewerRenderPacingConfig(render);
        ViewerRenderPacingState state{};

        NoteViewerInteraction(&state);
        ViewerRenderPacingDecision decision = AdvanceViewerRenderPacing(render, stats, 0.05, config, &state);
        if (decision.preview_active) {
            std::cerr << "Expected near-budget full-resolution frames to stay full resolution during interaction\n";
            return 1;
        }
        if (decision.render_resolution.x != 2048 || decision.render_resolution.y != 1536) {
            std::cerr << "Expected near-budget interaction to preserve the base render resolution\n";
            return 1;
        }
        if (state.settle_render_pending) {
            std::cerr << "Expected near-budget interaction to avoid scheduling an extra settle render\n";
            return 1;
        }
    }

    {
        RenderSettings render{};
        render.resolution = {1600, 900};
        RenderStats stats{};
        stats.last_render_ms = 180.0f;
        ViewerRenderPacingConfig config = BuildViewerRenderPacingConfig(render);
        ViewerRenderPacingState state{};
        state.seconds_since_interaction = 0.02;
        state.active_preview_scale = 0.90;
        state.settle_render_pending = true;

        ViewerRenderPacingDecision decision = AdvanceViewerRenderPacing(render, stats, 0.02, config, &state);
        if (!decision.preview_active || !NearlyEqual(decision.preview_scale, 0.5)) {
            std::cerr << "Expected slow preview frames to drop directly to the computed/clamped budget scale\n";
            return 1;
        }
    }

    {
        RenderSettings render{};
        render.resolution = {1024, 768};
        RenderStats stats{};
        stats.last_render_ms = 8.0f;
        ViewerRenderPacingConfig config = BuildViewerRenderPacingConfig(render);
        ViewerRenderPacingState state{};
        state.seconds_since_interaction = 0.05;
        state.settle_render_pending = true;
        state.active_preview_scale = 0.50;

        ViewerRenderPacingDecision decision = AdvanceViewerRenderPacing(render, stats, 0.05, config, &state);
        if (!decision.preview_active || !NearlyEqual(decision.preview_scale, 0.625)) {
            std::cerr << "Expected fast preview timing to recover gradually instead of jumping to full quality\n";
            return 1;
        }
        if (!state.settle_render_pending) {
            std::cerr << "Expected gradual recovery to preserve the pending settle render\n";
            return 1;
        }
    }

    {
        RenderSettings render{};
        render.resolution = {1024, 768};
        RenderStats stats{};
        stats.last_render_ms = 10.0f;
        ViewerRenderPacingConfig config = BuildViewerRenderPacingConfig(render);
        ViewerRenderPacingState state{};

        NoteViewerInteraction(&state);
        ViewerRenderPacingDecision decision = AdvanceViewerRenderPacing(render, stats, 0.05, config, &state);
        if (decision.preview_active) {
            std::cerr << "Expected comfortably fast full-resolution frames to stay full resolution during interaction\n";
            return 1;
        }
        if (decision.full_quality_due || state.settle_render_pending) {
            std::cerr << "Expected full-resolution interaction to avoid scheduling an extra settle render\n";
            return 1;
        }
    }

    {
        RenderSettings render{};
        render.resolution = {2048, 1536};
        RenderStats stats{};
        stats.last_render_ms = 90.0f;
        ViewerRenderPacingConfig config = BuildViewerRenderPacingConfig(render);
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
        if (!NearlyEqual(state.active_preview_scale, 1.0) || state.preview_step_index != 0) {
            std::cerr << "Expected settle render to reset preview state\n";
            return 1;
        }

        ViewerRenderPacingDecision idle = AdvanceViewerRenderPacing(render, stats, 0.20, config, &state);
        if (idle.preview_active || idle.full_quality_due) {
            std::cerr << "Expected settle render to be issued only once\n";
            return 1;
        }
    }

    std::cout << "test_viewer_render_pacing: all passed\n";
    return 0;
}