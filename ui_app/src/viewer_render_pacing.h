#pragma once

#include "fractal_types.h"

struct ViewerRenderPacingConfig {
    double debounce_seconds{0.20};
    double target_frame_ms{33.3333333333};
    double min_preview_scale{0.50};
    int preview_step_count{4};
    double step_down_hysteresis{1.10};
    double step_up_hysteresis{0.75};
};

struct ViewerRenderPacingState {
    double seconds_since_interaction{1.0e30};
    bool settle_render_pending{false};
    double active_preview_scale{1.0};
    int preview_step_index{0};
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