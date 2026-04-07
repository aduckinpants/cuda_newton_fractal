#include "viewer_render_pacing.h"

#include <cmath>

namespace {

constexpr double kNoRecentInteraction = 1.0e30;

double ClampPreviewScale(double value, double minScale) {
    if (!std::isfinite(value)) return 1.0;
    if (!std::isfinite(minScale)) minScale = 0.5;
    if (minScale < 0.125) minScale = 0.125;
    if (minScale > 1.0) minScale = 1.0;
    if (value < minScale) value = minScale;
    if (value > 1.0) value = 1.0;
    return value;
}

double ComputePreviewScale(float lastRenderMs, double targetFrameMs, double minScale) {
    if (!std::isfinite(targetFrameMs) || targetFrameMs <= 0.0) return 1.0;
    if (!std::isfinite(lastRenderMs) || lastRenderMs <= 0.0f) return 1.0;
    if (static_cast<double>(lastRenderMs) <= targetFrameMs) return 1.0;

    const double scale = std::sqrt(targetFrameMs / static_cast<double>(lastRenderMs));
    return ClampPreviewScale(scale, minScale);
}

int ScaleDimension(int value, double scale) {
    if (value <= 0) return 0;
    const double scaled = std::floor(static_cast<double>(value) * scale + 0.5);
    int result = static_cast<int>(scaled);
    if (result < 64) result = 64;
    if (result > value) result = value;
    return result;
}

} // namespace

void NoteViewerInteraction(ViewerRenderPacingState* ioState) {
    if (!ioState) return;
    ioState->seconds_since_interaction = 0.0;
}

ViewerRenderPacingDecision AdvanceViewerRenderPacing(
    const RenderSettings& baseRender,
    const RenderStats& lastStats,
    double deltaSeconds,
    const ViewerRenderPacingConfig& config,
    ViewerRenderPacingState* ioState) {
    ViewerRenderPacingDecision decision{};
    decision.render_resolution = baseRender.resolution;

    if (!ioState) return decision;
    if (ioState->seconds_since_interaction >= kNoRecentInteraction) return decision;

    const double debounceSeconds = std::fmax(0.0, config.debounce_seconds);
    const double delta = (std::isfinite(deltaSeconds) && deltaSeconds > 0.0) ? deltaSeconds : 0.0;
    const double ageAfterFrame = ioState->seconds_since_interaction + delta;

    if (debounceSeconds <= 0.0 || ageAfterFrame >= debounceSeconds) {
        ioState->seconds_since_interaction = kNoRecentInteraction;
        ioState->active_preview_scale = 1.0;
        if (ioState->settle_render_pending) {
            decision.full_quality_due = true;
            ioState->settle_render_pending = false;
        }
        return decision;
    }

    ioState->seconds_since_interaction = ageAfterFrame;

    double previewScale = 1.0;
    if (ioState->settle_render_pending && ioState->active_preview_scale < 0.999) {
        previewScale = ioState->active_preview_scale;
    } else {
        previewScale = ComputePreviewScale(lastStats.last_render_ms, config.target_frame_ms, config.min_preview_scale);
        ioState->active_preview_scale = previewScale;
    }

    if (previewScale < 0.999) {
        decision.preview_active = true;
        decision.preview_scale = previewScale;
        decision.render_resolution = {
            ScaleDimension(baseRender.resolution.x, previewScale),
            ScaleDimension(baseRender.resolution.y, previewScale)};
        ioState->settle_render_pending = true;
    } else {
        ioState->active_preview_scale = 1.0;
    }

    return decision;
}