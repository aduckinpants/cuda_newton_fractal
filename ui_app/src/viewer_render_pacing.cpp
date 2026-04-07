#include "viewer_render_pacing.h"

#include <cmath>

namespace {

constexpr double kNoRecentInteraction = 1.0e30;
constexpr int kDefaultPreviewStepCount = 4;

double ClampFinite(double value, double minValue, double maxValue, double fallback) {
    if (!std::isfinite(value)) return fallback;
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

int ClampStepIndex(int stepIndex, int maxStepCount) {
    if (stepIndex < 0) return 0;
    if (stepIndex > maxStepCount) return maxStepCount;
    return stepIndex;
}

double ClampPreviewScale(double value, double minScale) {
    if (!std::isfinite(value)) return 1.0;
    if (!std::isfinite(minScale)) minScale = 0.5;
    if (minScale < 0.125) minScale = 0.125;
    if (minScale > 1.0) minScale = 1.0;
    if (value < minScale) value = minScale;
    if (value > 1.0) value = 1.0;
    return value;
}

double PreviewScaleForStep(int stepIndex, int stepCount, double minScale) {
    if (stepIndex <= 0) return 1.0;
    if (stepCount <= 0) return 1.0;
    if (stepIndex >= stepCount) return ClampPreviewScale(minScale, minScale);

    const double t = static_cast<double>(stepIndex) / static_cast<double>(stepCount);
    const double logMinScale = std::log(ClampPreviewScale(minScale, minScale));
    return std::exp(logMinScale * t);
}

int AdvancePreviewStep(int currentStep,
    float lastRenderMs,
    const ViewerRenderPacingConfig& config) {
    const int maxStepCount = (config.preview_step_count > 0) ? config.preview_step_count : kDefaultPreviewStepCount;
    currentStep = ClampStepIndex(currentStep, maxStepCount);

    if (!std::isfinite(config.target_frame_ms) || config.target_frame_ms <= 0.0) {
        return 0;
    }
    if (!std::isfinite(lastRenderMs) || lastRenderMs <= 0.0f) {
        return currentStep;
    }

    const double lastFrameMs = static_cast<double>(lastRenderMs);
    const double stepDownThreshold = config.target_frame_ms * ClampFinite(config.step_down_hysteresis, 1.0, 4.0, 1.10);
    const double stepUpThreshold = config.target_frame_ms * ClampFinite(config.step_up_hysteresis, 0.10, 1.0, 0.75);

    if (lastFrameMs > stepDownThreshold) {
        return ClampStepIndex(currentStep + 1, maxStepCount);
    }
    if (lastFrameMs < stepUpThreshold) {
        return ClampStepIndex(currentStep - 1, maxStepCount);
    }
    return currentStep;
}

int ScaleDimension(int value, double scale) {
    if (value <= 0) return 0;
    const double scaled = std::floor(static_cast<double>(value) * scale + 0.5);
    int result = static_cast<int>(scaled);
    if (result < 64) result = 64;
    if (result > value) result = value;
    return result;
}

bool IsPositiveResolution(const Int2& resolution) {
    return resolution.x > 0 && resolution.y > 0;
}

} // namespace

ViewerRenderPacingConfig BuildViewerRenderPacingConfig(const RenderSettings& render) {
    ViewerRenderPacingConfig config{};
    config.debounce_seconds = ClampFinite(static_cast<double>(render.interaction_debounce_ms) / 1000.0, 0.0, 5.0, 0.20);
    const double previewTargetFps = ClampFinite(static_cast<double>(render.preview_target_fps), 1.0, 240.0, 30.0);
    config.target_frame_ms = 1000.0 / previewTargetFps;
    config.min_preview_scale = ClampPreviewScale(static_cast<double>(render.preview_min_scale), 0.125);
    config.preview_step_count = kDefaultPreviewStepCount;
    return config;
}

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
    const int previewStepCount = (config.preview_step_count > 0) ? config.preview_step_count : kDefaultPreviewStepCount;

    if (debounceSeconds <= 0.0 || ageAfterFrame >= debounceSeconds) {
        ioState->seconds_since_interaction = kNoRecentInteraction;
        ioState->active_preview_scale = 1.0;
        ioState->preview_step_index = 0;
        if (ioState->settle_render_pending) {
            decision.full_quality_due = true;
            ioState->settle_render_pending = false;
        }
        return decision;
    }

    ioState->seconds_since_interaction = ageAfterFrame;
    ioState->preview_step_index = AdvancePreviewStep(ioState->preview_step_index, lastStats.last_render_ms, config);
    decision.preview_step_index = ioState->preview_step_index;

    if (ioState->preview_step_index > 0) {
        const double previewScale = PreviewScaleForStep(ioState->preview_step_index, previewStepCount, config.min_preview_scale);
        decision.preview_active = true;
        decision.preview_scale = previewScale;
        decision.render_resolution = {
            ScaleDimension(baseRender.resolution.x, previewScale),
            ScaleDimension(baseRender.resolution.y, previewScale)};
        ioState->settle_render_pending = true;
        ioState->active_preview_scale = previewScale;
    } else {
        ioState->active_preview_scale = 1.0;
        ioState->settle_render_pending = false;
    }

    return decision;
}