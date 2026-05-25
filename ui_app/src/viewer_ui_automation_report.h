#pragma once

#include "color_pipeline_window.h"
#include "fractal_types.h"
#include "generic_equation_pack_workbench.h"
#include "render_capture_guard.h"
#include "viewer_render_pacing.h"

#include <Windows.h>

#include <iosfwd>
#include <cstdint>
#include <string>
#include <vector>

struct ViewerUiAutomationRect {
    std::string control_id;
    int client_left = 0;
    int client_top = 0;
    int client_right = 0;
    int client_bottom = 0;
};

struct ViewerUiAutomationFrameProbe {
    bool ready = false;
    int width = 0;
    int height = 0;
    std::uint64_t hash = 0;
};

struct ViewerUiAutomationRenderPacingProbe {
    int target_width = 0;
    int target_height = 0;
    float last_render_ms = 0.0f;
    bool has_last_render_fps = false;
    double last_render_fps = 0.0;
    bool pacing_preview_active = false;
    double pacing_preview_scale = 1.0;
    bool pacing_full_quality_due = false;
    int pacing_render_width = 0;
    int pacing_render_height = 0;
};

struct ViewerUiAutomationLensSdfProbe {
    bool enabled = false;
    bool valid = false;
    bool color_pipeline_active = false;
    std::string backend_used = "none";
    bool fallback_used = false;
    int width = 0;
    int height = 0;
    float pixel_scale = 1.0f;
    float base_render_ms = 0.0f;
    float field_ms = 0.0f;
    float postprocess_ms = 0.0f;
    float total_ms = 0.0f;
    std::string overlay_mode = "off";
    bool overlay_active = false;
    float overlay_opacity = 0.55f;
};

struct ViewerUiAutomationEnumCommandReport {
    std::string requested_enum_path;
    std::string requested_enum_id;
    bool enum_consumed = false;
    std::string enum_error;
};

std::string JsonEscapeAutomationReportString(const std::string& value);
void WriteAutomationReportString(std::ostream& out, const std::string& value);
ViewerUiAutomationFrameProbe BuildViewerUiAutomationFrameProbe(
    const std::vector<uint32_t>& rgba,
    const RenderedFrameState& renderedFrame);
ViewerUiAutomationRenderPacingProbe BuildViewerUiAutomationRenderPacingProbe(
    const RenderSettings& render,
    const RenderStats& stats,
    const ViewerRenderPacingDecision& renderPacing);
bool ViewerUiAutomationControlIdVisible(
    const std::vector<ViewerUiAutomationRect>& viewerUiAutomationRects,
    const ColorPipelineWindowState& colorPipelineWindow,
    const std::string& controlId);
void FailClosedPendingUiAutomationSetValue(
    const std::vector<ViewerUiAutomationRect>& viewerUiAutomationRects,
    ColorPipelineWindowState& colorPipelineWindow);
void WriteColorPipelineUiAutomationReport(
    const std::string& reportPath,
    HWND hwnd,
    const std::vector<ViewerUiAutomationRect>& viewerUiAutomationRects,
    const ColorPipelineWindowState& colorPipelineWindow,
    const GenericEquationPackWorkbenchAutomationReport* equationPackWorkbench,
    const ViewState& view,
    const RenderSettings& render,
    const RenderStats& stats,
    const ViewerRenderPacingDecision& renderPacing,
    const ViewerUiAutomationFrameProbe& frameProbe,
    const ViewerUiAutomationLensSdfProbe& lensSdfProbe,
    const ViewerUiAutomationEnumCommandReport& enumCommandReport,
    std::int64_t uiAutomationCommandSequence);
