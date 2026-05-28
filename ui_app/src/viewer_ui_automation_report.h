#pragma once

#include "color_pipeline_window.h"
#include "fractal_types.h"
#include "generic_equation_pack_workbench.h"
#include "render_capture_guard.h"
#include "sdf_pack_viewer_ui.h"
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

struct ViewerUiAutomationLensSdfFieldGroupProbe {
    int group_index = 0;
    int requested_downsample = 1;
    int effective_downsample = 1;
    int row_count = 0;
    bool has_inherited_row = false;
    bool has_explicit_row = false;
    std::string cache_status = "disabled";
    bool cache_hit = false;
    int width = 0;
    int height = 0;
    float pixel_scale = 1.0f;
    float field_ms = 0.0f;
    float mask_downsample_ms = 0.0f;
    float backend_ms = 0.0f;
    float cache_lookup_ms = 0.0f;
    float cache_store_ms = 0.0f;
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
    int requested_downsample = 1;
    int effective_downsample = 1;
    std::string quality_mode = "requested";
    std::string field_cache_status = "disabled";
    bool field_cache_hit = false;
    std::uint64_t field_cache_mask_bytes = 0;
    int field_group_count = 0;
    std::vector<ViewerUiAutomationLensSdfFieldGroupProbe> field_groups;
    float base_render_ms = 0.0f;
    float field_ms = 0.0f;
    float requested_equivalent_field_ms = 0.0f;
    float field_cache_lookup_ms = 0.0f;
    float field_mask_downsample_ms = 0.0f;
    float field_backend_ms = 0.0f;
    float field_cache_store_ms = 0.0f;
    float postprocess_ms = 0.0f;
    float total_ms = 0.0f;
    int postprocess_pixel_step = 1;
    int postprocess_worker_count = 1;
    std::string postprocess_backend_used = "cpu";
    bool postprocess_backend_fallback_used = false;
    bool postprocess_backend_buffer_reused = false;
    bool postprocess_backend_buffer_grew = false;
    std::uint64_t postprocess_direct_sample_count = 0;
    std::uint64_t postprocess_neighborhood_sample_count = 0;
    std::uint64_t postprocess_source_direct_sample_count = 0;
    std::uint64_t postprocess_source_neighborhood_sample_count = 0;
    std::uint64_t postprocess_filled_pixel_count = 0;
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
    const SdfPackViewerAutomationReport* sdfPackViewer,
    const ViewState& view,
    const RenderSettings& render,
    const RenderStats& stats,
    const ViewerRenderPacingDecision& renderPacing,
    const ViewerUiAutomationFrameProbe& frameProbe,
    const ViewerUiAutomationLensSdfProbe& lensSdfProbe,
    const ViewerUiAutomationEnumCommandReport& enumCommandReport,
    std::int64_t uiAutomationCommandSequence);
