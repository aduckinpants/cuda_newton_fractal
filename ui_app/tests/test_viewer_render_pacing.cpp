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
        render.resolution = {4096, 4096};
        RenderStats stats{};
        stats.last_render_ms = 55.0f;
        ViewerRenderPacingConfig config = BuildViewerRenderPacingConfig(render);
        ViewerRenderPacingState state{};

        NoteViewerInteraction(&state);
        ViewerRenderPacingDecision decision = AdvanceViewerRenderPacing(render, stats, 0.05, config, &state);
        if (decision.preview_active) {
            std::cerr << "Expected moderately slow full-resolution frames to stay full resolution until the FPS loss is material\n";
            return 1;
        }
        if (decision.render_resolution.x != 4096 || decision.render_resolution.y != 4096) {
            std::cerr << "Expected moderate f32 interaction to preserve the base render resolution\n";
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
        render.preview_min_scale = 0.50f;
        RenderStats stats{};
        stats.last_render_ms = 240.0f;
        const ViewerRenderPacingConfig config = BuildViewerRenderPacingConfig(render);
        ViewerRenderPacingState state{};

        NoteViewerInteraction(&state);
        ViewerRenderPacingDecision decision = AdvanceViewerRenderPacing(render, stats, 0.02, config, &state);
        const double expectedScale = std::sqrt(DefaultTargetFrameMs() / 240.0);
        if (!decision.preview_active || !NearlyEqual(decision.preview_scale, expectedScale, 1.0e-6)) {
            std::cerr << "Expected adaptive preview floor to drop below the configured 0.5 floor when measured frames require it\n";
            return 1;
        }
        if (decision.render_resolution.x != ScaledDimension(render.resolution.x, expectedScale) ||
            decision.render_resolution.y != ScaledDimension(render.resolution.y, expectedScale)) {
            std::cerr << "Expected adaptive preview floor to drive budget-derived dimensions\n";
            return 1;
        }
    }

    {
        RenderSettings render{};
        render.resolution = {2048, 1536};
        render.preview_min_scale = 0.50f;
        RenderStats stats{};
        stats.last_render_ms = 1500.0f;
        const ViewerRenderPacingConfig config = BuildViewerRenderPacingConfig(render);
        ViewerRenderPacingState state{};

        NoteViewerInteraction(&state);
        ViewerRenderPacingDecision decision = AdvanceViewerRenderPacing(render, stats, 1.50, config, &state);
        const double expectedScale = std::sqrt(DefaultTargetFrameMs() / 1500.0);

        if (!decision.preview_active) {
            std::cerr << "Expected a newly observed interaction to enter preview even when the previous frame was very slow\n";
            return 1;
        }
        if (!NearlyEqual(decision.preview_scale, expectedScale, 1.0e-6)) {
            std::cerr << "Expected severe slow interaction to use the measured budget scale instead of the stale 0.5 floor\n";
            return 1;
        }
        if (decision.render_resolution.x != ScaledDimension(render.resolution.x, expectedScale) ||
            decision.render_resolution.y != ScaledDimension(render.resolution.y, expectedScale)) {
            std::cerr << "Expected severe slow interaction to render at the emergency budget dimensions\n";
            return 1;
        }
        if (decision.full_quality_due) {
            std::cerr << "Expected stale frame time not to force an immediate full-quality settle on new input\n";
            return 1;
        }
    }

    {
        RenderSettings render{};
        render.resolution = {2048, 1536};
        render.preview_min_scale = 0.50f;
        RenderStats stats{};
        stats.last_render_ms = 1500.0f;
        const ViewerRenderPacingConfig config = BuildViewerRenderPacingConfig(render);
        ViewerRenderPacingState state{};
        state.seconds_since_interaction = 0.02;
        state.active_preview_scale = 1.0;

        ViewerRenderPacingDecision decision = AdvanceViewerRenderPacing(render, stats, 0.02, config, &state);
        if (!decision.preview_active || decision.preview_scale >= 0.25) {
            std::cerr << "Expected severe active interaction to bypass the normal preview floor when 0.5 remains too slow\n";
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
        const double expectedScale = 0.90 * std::sqrt(DefaultTargetFrameMs() / 180.0);
        if (!decision.preview_active || !NearlyEqual(decision.preview_scale, expectedScale, 1.0e-6)) {
            std::cerr << "Expected slow active preview frames to keep dropping below the normal floor when timing is still over budget\n";
            return 1;
        }
    }


    {
        RenderSettings render{};
        render.resolution = {2048, 1536};
        render.preview_min_scale = 0.50f;
        RenderStats stats{};
        stats.last_render_ms = 72.0f;
        const ViewerRenderPacingConfig config = BuildViewerRenderPacingConfig(render);
        ViewerRenderPacingState state{};
        state.seconds_since_interaction = 0.02;
        state.active_preview_scale = 0.15;
        state.settle_render_pending = true;

        ViewerRenderPacingDecision decision = AdvanceViewerRenderPacing(render, stats, 0.02, config, &state);
        if (!decision.preview_active || decision.preview_scale > 0.151) {
            std::cerr << "Expected severe active preview to hold its emergency scale instead of snapping back to the normal floor\n";
            return 1;
        }
    }

    {
        RenderSettings render{};
        render.resolution = {1024, 768};
        RenderStats stats{};
        stats.last_render_ms = 0.0f;
        ViewerRenderPacingConfig config = BuildViewerRenderPacingConfig(render);
        ViewerRenderPacingState state{};
        state.seconds_since_interaction = 0.05;
        state.active_preview_scale = 0.50;
        state.settle_render_pending = true;

        ViewerRenderPacingDecision decision = AdvanceViewerRenderPacing(render, stats, 0.02, config, &state);
        if (!decision.preview_active || !NearlyEqual(decision.preview_scale, 0.50)) {
            std::cerr << "Expected active preview with unknown timing to hold the current preview scale\n";
            return 1;
        }
    }

    {
        RenderSettings render{};
        render.resolution = {1024, 768};
        RenderStats stats{};
        stats.last_render_ms = 30.0f;
        ViewerRenderPacingConfig config = BuildViewerRenderPacingConfig(render);
        ViewerRenderPacingState state{};
        state.seconds_since_interaction = 0.05;
        state.active_preview_scale = 0.50;
        state.settle_render_pending = true;

        ViewerRenderPacingDecision decision = AdvanceViewerRenderPacing(render, stats, 0.02, config, &state);
        if (!decision.preview_active || !NearlyEqual(decision.preview_scale, 0.50)) {
            std::cerr << "Expected active preview to hold scale inside the recovery hysteresis band\n";
            return 1;
        }
    }

    {
        RenderSettings render{};
        render.resolution = {1024, 768};
        RenderStats stats{};
        stats.last_render_ms = 45.0f;
        ViewerRenderPacingConfig config = BuildViewerRenderPacingConfig(render);
        ViewerRenderPacingState state{};
        state.seconds_since_interaction = 0.05;
        state.active_preview_scale = 0.50;
        state.settle_render_pending = true;

        ViewerRenderPacingDecision decision = AdvanceViewerRenderPacing(render, stats, 0.02, config, &state);
        if (!decision.preview_active || !NearlyEqual(decision.preview_scale, 0.50)) {
            std::cerr << "Expected slow active-preview timing to stay at the current bounded preview scale\n";
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
        render.resolution = {4096, 2542};
        render.interaction_debounce_ms = 200;
        RenderStats fullStats{};
        fullStats.last_render_ms = 4987.0f;
        RenderStats previewStats{};
        previewStats.last_render_ms = 30.0f;
        ViewerRenderPacingConfig config = BuildViewerRenderPacingConfig(render);
        ViewerRenderPacingState state{};

        NoteViewerInteraction(&state);
        ViewerRenderPacingDecision preview = AdvanceViewerRenderPacing(render, fullStats, 0.0, config, &state);
        if (!preview.preview_active || !state.settle_render_pending) {
            std::cerr << "Expected very slow full-quality timing to enter preview during interaction\n";
            return 1;
        }
        if (state.slow_full_quality_settle_seconds < 1.9) {
            std::cerr << "Expected very slow full-quality timing to extend the settle window\n";
            return 1;
        }

        ViewerRenderPacingDecision hold = AdvanceViewerRenderPacing(render, previewStats, 0.25, config, &state);
        if (!hold.preview_active || hold.full_quality_due) {
            std::cerr << "Expected SDF-heavy preview to stay active past the fixed debounce instead of flickering to full quality\n";
            return 1;
        }

        ViewerRenderPacingDecision settle = AdvanceViewerRenderPacing(render, previewStats, 2.0, config, &state);
        if (settle.preview_active || !settle.full_quality_due) {
            std::cerr << "Expected extended settle window to still allow one full-quality settle render\n";
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
