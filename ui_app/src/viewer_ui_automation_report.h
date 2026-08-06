#pragma once

#include "color_pipeline_window.h"
#include "fractal_types.h"
#include "generic_equation_pack_workbench.h"
#include "render_capture_guard.h"
#include "sdf_field_capability.h"
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

struct ViewerUiAutomationRootPatternProbe {
    std::string ref = "dynamics_root_field";
    std::string label = "Dynamics Root Field";
    std::string layout_kind = "none";
    std::string source_kind = "none";
    int root_count = 0;
    int requested_generated_root_count = 0;
    double seed = 0.0;
    float root_spread = 0.0f;
    float phase = 0.0f;
    float phase_strength = 1.0f;
    std::uint64_t base_root_hash = 0;
    std::uint64_t effective_root_hash = 0;
    std::string fail_closed_reason;
};

struct ViewerUiAutomationRootPatternConsumerProbe {
    std::string consumer_kind = "none";
    std::string consumer_id = "none";
    std::string pattern_ref = "dynamics_root_field";
    std::string fail_closed_reason;
};

struct ViewerUiAutomationScalarDistributionProbe {
    std::uint64_t finite_count = 0;
    std::uint64_t nonfinite_count = 0;
    std::uint64_t below_histogram_range_count = 0;
    std::uint64_t above_histogram_range_count = 0;
    double minimum = 0.0;
    double maximum = 0.0;
    double mean = 0.0;
    double standard_deviation = 0.0;
    double p01 = 0.0;
    double p05 = 0.0;
    double p50 = 0.0;
    double p95 = 0.0;
    double p99 = 0.0;
    double histogram_minimum = 0.0;
    double histogram_maximum = 0.0;
    std::vector<std::uint64_t> histogram;
};

struct ViewerUiAutomationColorSourceMeasurementProbe {
    bool requested = false;
    bool valid = false;
    std::string error;
    std::string producer_id = "none";
    std::string source_id = "none";
    int row_index = -1;
    std::string shape_id = "none";
    std::string root_pattern_ref = "dynamics_root_field";
    std::uint64_t root_pattern_hash = 0;
    std::string evaluator_id = "unknown";
    std::string fractal_precision_tier = "unknown";
    std::string color_metric_arithmetic_tier = "unknown";
    std::string color_metric_narrowing = "unknown";
    ViewerUiAutomationScalarDistributionProbe source_raw;
    ViewerUiAutomationScalarDistributionProbe shape_output;
};

struct ViewerUiAutomationLensSdfProbe {
    bool enabled = false;
    bool valid = false;
    bool color_pipeline_active = false;
    std::string source_stack_kind = "non_sdf_only";
    bool mixed_source_signal_frame_used = false;
    std::string field_source = "none";
    std::string field_producer_kind = SdfFieldProducerKindId(SdfFieldProducerKind::none);
    std::vector<std::string> supported_signal_ids;
    std::string field_capability_fail_closed_reason;
    std::string field_source_pack_id;
    std::string field_source_error;
    bool root_field_consumer_active = false;
    std::string root_field_consumer_kind = "none";
    std::string root_field_consumer_base_fractal_type = "none";
    std::string root_field_consumer_root_layout_kind = "none";
    std::string root_field_consumer_root_source_kind = "none";
    int root_field_consumer_root_count = 0;
    int root_field_consumer_requested_generated_root_count = 0;
    float root_field_consumer_trap_strength = 0.0f;
    float root_field_consumer_trap_scale = 1.0f;
    float root_field_consumer_multibrot_power_float = 0.0f;
    float root_field_consumer_multibrot_power_imag = 0.0f;
    std::uint64_t root_field_consumer_base_root_hash = 0;
    std::uint64_t root_field_consumer_effective_root_hash = 0;
    std::string root_field_consumer_fail_closed_reason;
    std::vector<ViewerUiAutomationRootPatternProbe> root_patterns;
    std::vector<ViewerUiAutomationRootPatternConsumerProbe> root_pattern_consumers;
    int explaino_root_sdf_root_count = 0;
    int explaino_root_sdf_bridge_count = 0;
    std::string explaino_root_sdf_root_layout_kind = "none";
    int explaino_root_sdf_requested_generated_root_count = 0;
    std::string explaino_root_sdf_h_source = "none";
    std::uint64_t explaino_root_sdf_base_root_hash = 0;
    std::uint64_t explaino_root_sdf_effective_root_hash = 0;
    std::string backend_used = "none";
    std::string pack_backend_used = "none";
    bool pack_backend_fallback_used = false;
    bool pack_direct_grid_evaluation = false;
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
    ViewerUiAutomationColorSourceMeasurementProbe color_source_measurement;
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
ViewerUiAutomationColorSourceMeasurementProbe BuildViewerUiAutomationColorSourceMeasurementProbe(
    const float* rawValues,
    std::size_t valueCount,
    const KernelParams& params,
    int rowIndex,
    const char* sourceId);
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
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const ViewerRenderPacingDecision& renderPacing,
    const ViewerUiAutomationFrameProbe& frameProbe,
    const ViewerUiAutomationLensSdfProbe& lensSdfProbe,
    const ViewerUiAutomationEnumCommandReport& enumCommandReport,
    std::int64_t uiAutomationCommandSequence);
