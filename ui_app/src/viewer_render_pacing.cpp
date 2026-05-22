#include "viewer_render_pacing.h"

#include <cmath>

namespace {

constexpr double kNoRecentInteraction = 1.0e30;
constexpr int kDefaultPreviewStepCount = 4;
constexpr double kPreviewRecoveryBlend = 0.25;
constexpr int kLargeRenderPixelThreshold = 1024 * 768;

double ClampFinite(double value, double minValue, double maxValue, double fallback) {
    if (!std::isfinite(value)) return fallback;
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

double ClampPreviewScale(double value, double minScale) {
    if (!std::isfinite(value)) return 1.0;
    if (!std::isfinite(minScale)) minScale = static_cast<double>(RenderSettings::kDefaultPreviewMinScale);
    if (minScale < 0.125) minScale = 0.125;
    if (minScale > 1.0) minScale = 1.0;
    if (value < minScale) value = minScale;
    if (value > 1.0) value = 1.0;
    return value;
}

int ScaleDimension(int value, double scale) {
    if (value <= 0) return 0;
    const double scaled = std::floor(static_cast<double>(value) * scale + 0.5);
    int result = static_cast<int>(scaled);
    if (result < 64) result = 64;
    if (result > value) result = value;
    return result;
}

bool IsLargeRenderResolution(const Int2& resolution) {
    if (resolution.x <= 0 || resolution.y <= 0) return false;
    return static_cast<long long>(resolution.x) * static_cast<long long>(resolution.y) >= kLargeRenderPixelThreshold;
}

bool HasUsableRenderTiming(float renderMs) {
    return std::isfinite(renderMs) && renderMs > 0.0f;
}

double PreviewScaleFromTiming(float renderMs, double targetFrameMs, double minScale) {
    const double target = std::sqrt(targetFrameMs / static_cast<double>(renderMs));
    return ClampPreviewScale(target, minScale);
}

double TargetPreviewScale(const RenderSettings& baseRender, const RenderStats& lastStats, const ViewerRenderPacingConfig& config) {
    const double minScale = ClampPreviewScale(config.min_preview_scale, 0.125);
    if (!std::isfinite(config.target_frame_ms) || config.target_frame_ms <= 0.0) return 1.0;
    if (!HasUsableRenderTiming(lastStats.last_render_ms)) {
        return IsLargeRenderResolution(baseRender.resolution) ? minScale : 1.0;
    }
    return PreviewScaleFromTiming(lastStats.last_render_ms, config.target_frame_ms, minScale);
}

double ClampRecoveredPreviewScale(double value, double minScale) {
    if (value > 0.995) return 1.0;
    return ClampPreviewScale(value, minScale);
}

double RecoverPreviewScale(double currentScale, double targetScale, double minScale) {
    currentScale = ClampPreviewScale(currentScale, minScale);
    targetScale = ClampPreviewScale(targetScale, minScale);
    if (targetScale < currentScale) return targetScale;
    const double recovered = currentScale + (targetScale - currentScale) * kPreviewRecoveryBlend;
    return ClampRecoveredPreviewScale(recovered, minScale);
}

double DefaultDebounceSeconds() {
    return static_cast<double>(RenderSettings::kDefaultInteractionDebounceMs) / 1000.0;
}

double DefaultPreviewTargetFps() {
    return static_cast<double>(RenderSettings::kDefaultPreviewTargetFps);
}

} // namespace

ViewerRenderPacingConfig BuildViewerRenderPacingConfig(const RenderSettings& render) {
    ViewerRenderPacingConfig config{};
    config.debounce_seconds = ClampFinite(static_cast<double>(render.interaction_debounce_ms) / 1000.0, 0.0, 5.0, DefaultDebounceSeconds());
    const double previewTargetFps = ClampFinite(static_cast<double>(render.preview_target_fps), 1.0, 240.0, DefaultPreviewTargetFps());
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
    const double minScale = ClampPreviewScale(config.min_preview_scale, 0.125);
    const double targetScale = TargetPreviewScale(baseRender, lastStats, config);
    const double previewScale = RecoverPreviewScale(ioState->active_preview_scale, targetScale, minScale);
    ioState->active_preview_scale = previewScale;

    if (previewScale < 0.999) {
        decision.preview_active = true;
        decision.preview_scale = previewScale;
        decision.preview_step_index = 1;
        decision.render_resolution = {
            ScaleDimension(baseRender.resolution.x, previewScale),
            ScaleDimension(baseRender.resolution.y, previewScale)};
        ioState->preview_step_index = 1;
        ioState->settle_render_pending = true;
    } else {
        decision.preview_scale = 1.0;
        ioState->preview_step_index = 0;
    }

    return decision;
}