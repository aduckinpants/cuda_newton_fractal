#pragma once

#include "fractal_types.h"

struct ViewerRenderPacingConfig {
    double debounce_seconds{static_cast<double>(RenderSettings::kDefaultInteractionDebounceMs) / 1000.0};
    double target_frame_ms{1000.0 / static_cast<double>(RenderSettings::kDefaultPreviewTargetFps)};
    double min_preview_scale{static_cast<double>(RenderSettings::kDefaultPreviewMinScale)};
    int preview_step_count{4};
    double step_down_hysteresis{2.0};
    double step_up_hysteresis{0.75};
};

struct ViewerRenderPacingState {
    double seconds_since_interaction{1.0e30};
    bool settle_render_pending{false};
    double active_preview_scale{1.0};
    double slow_full_quality_settle_seconds{0.0};
    int preview_step_index{0};
    bool interaction_just_noted{false};
};

struct ViewerRenderPacingDecision {
    Int2 render_resolution{0, 0};
    double preview_scale{1.0};
    bool preview_active{false};
    bool full_quality_due{false};
    int preview_step_index{0};
};

ViewerRenderPacingConfig BuildViewerRenderPacingConfig(const RenderSettings& render);

void NoteViewerInteraction(ViewerRenderPacingState* ioState);

ViewerRenderPacingDecision AdvanceViewerRenderPacing(
    const RenderSettings& baseRender,
    const RenderStats& lastStats,
    double deltaSeconds,
    const ViewerRenderPacingConfig& config,
    ViewerRenderPacingState* ioState);
