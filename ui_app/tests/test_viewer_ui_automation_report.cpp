#include <cstdio>
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "../src/color_pipeline_graph_receipt.h"
#include "../src/viewer_ui_automation_report.h"

namespace {

int g_failed = 0;

void Check(bool condition, const char* message) {
    if (!condition) {
        ++g_failed;
        std::printf("  FAIL: %s\n", message);
    }
}

void TestJsonStringEscaping() {
    const std::string raw = std::string("quote=\" slash=\\ newline=\n control=") + char(1);
    const std::string escaped = JsonEscapeAutomationReportString(raw);
    Check(escaped.find("\\\"") != std::string::npos, "quotes are escaped");
    Check(escaped.find("\\\\") != std::string::npos, "backslashes are escaped");
    Check(escaped.find("\\n") != std::string::npos, "newlines are escaped");
    Check(escaped.find("\\u0001") != std::string::npos, "control characters are escaped as unicode");

    std::ostringstream out;
    WriteAutomationReportString(out, raw);
    Check(out.str().size() >= 2 && out.str().front() == '"' && out.str().back() == '"',
        "report string writer wraps escaped content in quotes");
}

void TestVisibleControlLookupAndFailClosedErrors() {
    std::vector<ViewerUiAutomationRect> schemaRects;
    ViewerUiAutomationRect schemaRect;
    schemaRect.control_id = "fractal_control.width.primary";
    schemaRects.push_back(schemaRect);

    ColorPipelineWindowState state{};
    state.ui_automation_set_pending = true;
    state.ui_automation_set_control_id = "fractal_control.width.primary";
    Check(ViewerUiAutomationControlIdVisible(schemaRects, state, state.ui_automation_set_control_id),
        "schema rect lookup sees visible controls");
    FailClosedPendingUiAutomationSetValue(schemaRects, state);
    Check(state.ui_automation_set_error.find("did not support visible control") != std::string::npos,
        "visible but unsupported schema control fails closed");

    ColorPipelineWindowState missingState{};
    missingState.ui_automation_set_pending = true;
    missingState.ui_automation_set_control_id = "fractal_control.missing.primary";
    FailClosedPendingUiAutomationSetValue(schemaRects, missingState);
    Check(missingState.ui_automation_set_error.find("not visible or unsupported") != std::string::npos,
        "missing non-color control fails closed");

    ColorPipelineWindowState deferredColorState{};
    deferredColorState.ui_automation_set_pending = true;
    deferredColorState.ui_automation_set_control_id = "color_pipeline.source.smooth_escape_ramp.signal.scale.primary";
    FailClosedPendingUiAutomationSetValue(schemaRects, deferredColorState);
    Check(deferredColorState.ui_automation_set_error.empty(),
        "color pipeline set-value waits until the color window publishes controls");

    ColorPipelineWindowState readyColorState{};
    readyColorState.open = true;
    readyColorState.initialized = true;
    readyColorState.ui_automation_set_pending = true;
    readyColorState.ui_automation_set_control_id = "color_pipeline.source.smooth_escape_ramp.signal.scale.primary";
    ColorPipelineUiAutomationRect colorRect;
    colorRect.control_id = "color_pipeline.other.primary";
    readyColorState.ui_automation_rects.push_back(colorRect);
    FailClosedPendingUiAutomationSetValue(schemaRects, readyColorState);
    Check(readyColorState.ui_automation_set_error.find("not visible or unsupported") != std::string::npos,
        "ready color pipeline control set fails closed when requested control is absent");
}

void TestLensSdfProbeDefaults() {
    ViewerUiAutomationLensSdfProbe probe{};
    Check(probe.overlay_mode == "off" && !probe.overlay_active && probe.overlay_opacity > 0.5f,
        "lens SDF automation probe reports stable overlay defaults");
    Check(probe.field_producer_kind == "none" &&
            probe.supported_signal_ids.empty() &&
            probe.field_capability_fail_closed_reason.empty(),
        "lens SDF automation probe reports stable capability defaults");
    Check(!probe.color_pipeline_active && probe.base_render_ms == 0.0f &&
            probe.field_ms == 0.0f && probe.requested_equivalent_field_ms == 0.0f &&
            probe.postprocess_ms == 0.0f && probe.total_ms == 0.0f,
        "lens SDF automation probe reports stable timing defaults");
    Check(probe.field_cache_lookup_ms == 0.0f &&
            probe.field_mask_downsample_ms == 0.0f &&
            probe.field_backend_ms == 0.0f &&
            probe.field_cache_store_ms == 0.0f,
        "lens SDF automation probe reports stable field-generation stage defaults");
    Check(probe.postprocess_backend_used == "cpu" && !probe.postprocess_backend_fallback_used,
        "lens SDF automation probe reports stable postprocess backend defaults");
    Check(!probe.postprocess_backend_buffer_reused && !probe.postprocess_backend_buffer_grew,
        "lens SDF automation probe reports stable postprocess buffer defaults");
    Check(probe.requested_downsample == 1 &&
            probe.effective_downsample == 1 &&
            probe.quality_mode == "requested",
        "lens SDF automation probe reports stable requested/effective quality defaults");
    Check(!probe.field_cache_hit &&
            probe.field_cache_status == "disabled" &&
            probe.field_cache_mask_bytes == 0,
        "lens SDF automation probe reports stable field cache defaults");
    Check(!probe.pack_direct_grid_evaluation,
        "lens SDF automation probe reports stable authored-pack direct-grid default");
    Check(probe.field_group_count == 0 && probe.field_groups.empty(),
        "lens SDF automation probe reports stable empty field-group defaults");
}

void TestLensSdfProbeTimingFields() {
    ViewerUiAutomationLensSdfProbe probe{};
    probe.color_pipeline_active = true;
    probe.requested_downsample = 1;
    probe.effective_downsample = 4;
    probe.quality_mode = "interactive_adaptive";
    probe.base_render_ms = 3.0f;
    probe.field_ms = 2.0f;
    probe.requested_equivalent_field_ms = 8.0f;
    probe.field_cache_lookup_ms = 0.25f;
    probe.field_mask_downsample_ms = 0.5f;
    probe.field_backend_ms = 1.0f;
    probe.field_cache_store_ms = 0.25f;
    probe.postprocess_ms = 7.5f;
    probe.total_ms = probe.field_ms + probe.postprocess_ms;
    probe.postprocess_worker_count = 3;
    probe.postprocess_backend_used = "cuda_direct_scalar";
    probe.postprocess_backend_fallback_used = true;
    probe.postprocess_backend_buffer_reused = true;
    probe.postprocess_backend_buffer_grew = false;
    probe.postprocess_source_direct_sample_count = 11;
    probe.postprocess_source_neighborhood_sample_count = 22;
    probe.field_cache_status = "hit";
    probe.field_cache_hit = true;
    probe.field_cache_mask_bytes = 76800;
    probe.field_producer_kind = "lens_field_v2";
    probe.root_field_consumer_active = true;
    probe.root_field_consumer_kind = "explaino_mandelbrot_root_trap";
    probe.root_field_consumer_base_fractal_type = "mandelbrot";
    probe.root_field_consumer_root_layout_kind = "regular_ngon_v1";
    probe.root_field_consumer_root_source_kind = "generated";
    probe.root_field_consumer_root_count = 5;
    probe.root_field_consumer_requested_generated_root_count = 5;
    probe.root_field_consumer_trap_strength = 0.75f;
    probe.root_field_consumer_trap_scale = 1.5f;
    probe.root_field_consumer_multibrot_power_float = 2.5f;
    probe.root_field_consumer_multibrot_power_imag = 0.65f;
    probe.root_field_consumer_base_root_hash = 0x9012ull;
    probe.root_field_consumer_effective_root_hash = 0x3456ull;
    ViewerUiAutomationRootPatternProbe primaryPattern{};
    primaryPattern.ref = "dynamics_root_field";
    primaryPattern.label = "Dynamics Root Field";
    primaryPattern.layout_kind = "regular_ngon_v1";
    primaryPattern.source_kind = "generated";
    primaryPattern.root_count = 11;
    primaryPattern.requested_generated_root_count = 11;
    primaryPattern.base_root_hash = 0xaaa1ull;
    primaryPattern.effective_root_hash = 0xaaa2ull;
    probe.root_patterns.push_back(primaryPattern);
    ViewerUiAutomationRootPatternProbe secondaryPattern{};
    secondaryPattern.ref = "color_root_field";
    secondaryPattern.label = "Color Root Field";
    secondaryPattern.layout_kind = "legacy_quartic_v1";
    secondaryPattern.source_kind = "generated";
    secondaryPattern.root_count = 4;
    secondaryPattern.requested_generated_root_count = 4;
    secondaryPattern.base_root_hash = 0xbbb1ull;
    secondaryPattern.effective_root_hash = 0xbbb2ull;
    probe.root_patterns.push_back(secondaryPattern);
    ViewerUiAutomationRootPatternConsumerProbe dynamicsConsumer{};
    dynamicsConsumer.consumer_kind = "root_field_consumer";
    dynamicsConsumer.consumer_id = "explaino_magnet_root_well";
    dynamicsConsumer.pattern_ref = "dynamics_root_field";
    probe.root_pattern_consumers.push_back(dynamicsConsumer);
    ViewerUiAutomationRootPatternConsumerProbe rowConsumer{};
    rowConsumer.consumer_kind = "color_source_row";
    rowConsumer.consumer_id = "root_phase";
    rowConsumer.pattern_ref = "color_root_field";
    probe.root_pattern_consumers.push_back(rowConsumer);
    probe.explaino_root_sdf_root_count = 4;
    probe.explaino_root_sdf_bridge_count = 2;
    probe.explaino_root_sdf_root_layout_kind = "regular_ngon_v1";
    probe.explaino_root_sdf_requested_generated_root_count = 8;
    probe.explaino_root_sdf_h_source = "phase_sine";
    probe.explaino_root_sdf_base_root_hash = 0x1234ull;
    probe.explaino_root_sdf_effective_root_hash = 0x5678ull;
    probe.pack_direct_grid_evaluation = true;
    probe.supported_signal_ids = {
        "sdf_signed_distance",
        "sdf_inside_outside",
        "sdf_boundary_band",
        "sdf_normal_angle",
        "sdf_curvature",
        "lens_field_v2_distance",
    };
    probe.field_capability_fail_closed_reason = "mixed Source rows require renderer-backed non-SDF source signals";
    ViewerUiAutomationLensSdfFieldGroupProbe group{};
    group.group_index = 0;
    group.requested_downsample = 1;
    group.effective_downsample = 4;
    group.row_count = 2;
    group.has_inherited_row = true;
    group.has_explicit_row = true;
    group.cache_status = "hit";
    group.cache_hit = true;
    group.width = 80;
    group.height = 60;
    group.pixel_scale = 4.0f;
    group.field_ms = 1.5f;
    group.mask_downsample_ms = 0.25f;
    group.backend_ms = 1.0f;
    group.cache_lookup_ms = 0.1f;
    group.cache_store_ms = 0.0f;
    probe.field_groups.push_back(group);
    probe.field_group_count = static_cast<int>(probe.field_groups.size());
    Check(probe.color_pipeline_active && probe.base_render_ms == 3.0f &&
            probe.field_ms == 2.0f && probe.requested_equivalent_field_ms == 8.0f &&
            probe.postprocess_ms == 7.5f && probe.total_ms == 9.5f,
        "lens SDF automation probe carries separate field/postprocess timing");
    Check(probe.field_cache_lookup_ms == 0.25f &&
            probe.field_mask_downsample_ms == 0.5f &&
            probe.field_backend_ms == 1.0f &&
            probe.field_cache_store_ms == 0.25f,
        "lens SDF automation probe carries field-generation stage timings");
    Check(probe.postprocess_worker_count == 3,
        "lens SDF automation probe carries postprocess worker count");
    Check(probe.postprocess_backend_used == "cuda_direct_scalar" && probe.postprocess_backend_fallback_used,
        "lens SDF automation probe carries actual postprocess backend");
    Check(probe.postprocess_backend_buffer_reused && !probe.postprocess_backend_buffer_grew,
        "lens SDF automation probe carries postprocess buffer reuse status");
    Check(probe.postprocess_source_direct_sample_count == 11 &&
            probe.postprocess_source_neighborhood_sample_count == 22,
        "lens SDF automation probe carries per-row source sample counts");
    Check(probe.requested_downsample == 1 &&
            probe.effective_downsample == 4 &&
            probe.quality_mode == "interactive_adaptive",
        "lens SDF automation probe carries requested/effective field quality");
    Check(probe.field_cache_status == "hit" &&
            probe.field_cache_hit &&
            probe.field_cache_mask_bytes == 76800,
        "lens SDF automation probe carries field cache status");
    Check(probe.field_producer_kind == "lens_field_v2" &&
            probe.supported_signal_ids.size() == kSdfFieldCapabilitySignalIds.size() &&
            SdfFieldCapabilitySupportsSignalId(probe.supported_signal_ids.back()) &&
            !probe.field_capability_fail_closed_reason.empty(),
        "lens SDF automation probe carries field capability identity and fail-closed reason");
    Check(probe.field_group_count == 1 &&
            probe.field_groups[0].requested_downsample == 1 &&
            probe.field_groups[0].effective_downsample == 4 &&
            probe.field_groups[0].has_inherited_row &&
            probe.field_groups[0].has_explicit_row &&
            probe.field_groups[0].cache_status == "hit" &&
            probe.field_groups[0].width == 80,
        "lens SDF automation probe carries field-group detail");
    Check(probe.pack_direct_grid_evaluation,
        "lens SDF automation probe carries authored-pack direct-grid status");

    const std::filesystem::path reportPath =
        std::filesystem::temp_directory_path() / "test_viewer_ui_automation_report_root_sdf.json";
    std::vector<ViewerUiAutomationRect> viewerRects;
    ColorPipelineWindowState colorPipelineWindow{};
    ViewState view{};
    RenderSettings render{};
    RenderStats stats{};
    ViewerRenderPacingDecision pacing{};
    ViewerUiAutomationFrameProbe frameProbe{};
    ViewerUiAutomationEnumCommandReport enumReport{};
    HWND hwnd = CreateWindowExA(
        0,
        "STATIC",
        "automation-report-test",
        WS_OVERLAPPED,
        0,
        0,
        1,
        1,
        nullptr,
        nullptr,
        GetModuleHandleA(nullptr),
        nullptr);
    Check(hwnd != nullptr, "automation report test creates a valid window handle");
    WriteColorPipelineUiAutomationReport(
        reportPath.string(),
        hwnd,
        viewerRects,
        colorPipelineWindow,
        nullptr,
        nullptr,
        view,
        render,
        stats,
        pacing,
        frameProbe,
        probe,
        enumReport,
        17);
    if (hwnd) {
        DestroyWindow(hwnd);
    }
    std::ifstream in(reportPath, std::ios::binary);
    std::ostringstream buffer;
    buffer << in.rdbuf();
    const std::string json = buffer.str();
    Check(json.find("\"explaino_root_sdf_root_count\": 4") != std::string::npos &&
            json.find("\"explaino_root_sdf_bridge_count\": 2") != std::string::npos &&
            json.find("\"explaino_root_sdf_root_layout_kind\": \"regular_ngon_v1\"") != std::string::npos &&
            json.find("\"explaino_root_sdf_requested_generated_root_count\": 8") != std::string::npos &&
            json.find("\"explaino_root_sdf_h_source\": \"phase_sine\"") != std::string::npos,
        "automation report writes ExplainO root-SDF layout, root count, bridge count, and h source");
    Check(json.find("\"explaino_root_sdf_base_root_hash\": \"fnv1a64:0000000000001234\"") != std::string::npos &&
            json.find("\"explaino_root_sdf_effective_root_hash\": \"fnv1a64:0000000000005678\"") != std::string::npos,
        "automation report writes ExplainO root-SDF root hashes");
    Check(json.find("\"root_field_consumer_active\": true") != std::string::npos &&
            json.find("\"root_field_consumer_kind\": \"explaino_mandelbrot_root_trap\"") != std::string::npos &&
            json.find("\"root_field_consumer_base_fractal_type\": \"mandelbrot\"") != std::string::npos &&
            json.find("\"root_field_consumer_root_layout_kind\": \"regular_ngon_v1\"") != std::string::npos &&
            json.find("\"root_field_consumer_root_source_kind\": \"generated\"") != std::string::npos &&
            json.find("\"root_field_consumer_root_count\": 5") != std::string::npos &&
            json.find("\"root_field_consumer_requested_generated_root_count\": 5") != std::string::npos,
        "automation report writes root-field consumer identity and root-layout diagnostics");
    Check(json.find("\"root_field_consumer_base_root_hash\": \"fnv1a64:0000000000009012\"") != std::string::npos &&
            json.find("\"root_field_consumer_effective_root_hash\": \"fnv1a64:0000000000003456\"") != std::string::npos,
        "automation report writes root-field consumer root hashes");
    Check(json.find("\"root_field_consumer_multibrot_power_float\": 2.5") != std::string::npos &&
            json.find("\"root_field_consumer_multibrot_power_imag\": 0.649") != std::string::npos,
        "automation report writes Multibrot exponent values for root-field consumers");
    Check(json.find("\"active_root_field\":") != std::string::npos &&
            json.find("\"ref\": \"dynamics_root_field\"") != std::string::npos &&
            json.find("\"label\": \"Dynamics Root Field\"") != std::string::npos &&
            json.find("\"root_count\": 11") != std::string::npos,
        "automation report writes Dynamics Root Field as the active root field");
    Check(json.find("\"root_patterns\":") != std::string::npos &&
            json.find("\"ref\": \"dynamics_root_field\"") != std::string::npos &&
            json.find("\"root_count\": 11") != std::string::npos &&
            json.find("\"base_root_hash\": \"fnv1a64:000000000000aaa1\"") != std::string::npos &&
            json.find("\"ref\": \"color_root_field\"") != std::string::npos &&
            json.find("\"label\": \"Color Root Field\"") != std::string::npos &&
            json.find("\"layout_kind\": \"legacy_quartic_v1\"") != std::string::npos,
        "automation report writes scoped root-pattern authority slots");
    Check(json.find("\"root_pattern_consumers\":") != std::string::npos &&
            json.find("\"consumer_kind\": \"root_field_consumer\"") != std::string::npos &&
            json.find("\"consumer_id\": \"explaino_magnet_root_well\"") != std::string::npos &&
            json.find("\"consumer_kind\": \"color_source_row\"") != std::string::npos &&
            json.find("\"pattern_ref\": \"color_root_field\"") != std::string::npos &&
            json.find("\"pattern_ref\": \"primary\"") == std::string::npos,
        "automation report writes explicit scoped root-pattern consumer refs");
}


void TestColorPipelineGraphReceiptBuilderReportsLinearStackTruth() {
    ColorPipelineLaneState sourceLane{};
    sourceLane.lane_id = "source";
    sourceLane.label = "Source";
    ColorPipelineRowState smoothRow{};
    smoothRow.ui_row_id = 10;
    smoothRow.enabled = true;
    smoothRow.function_id = "smooth_escape_ramp";
    smoothRow.parameter_values.push_back({"signal.blend_weight", "float", 1.0, false, ""});
    sourceLane.rows.push_back(smoothRow);

    ColorPipelineRowState sdfRow{};
    sdfRow.ui_row_id = 11;
    sdfRow.enabled = false;
    sdfRow.function_id = "sdf_signed_distance";
    sdfRow.parameter_values.push_back({"signal.sdf_gate", "enum", 0.0, false, "sdf_inside"});
    sdfRow.parameter_values.push_back({"signal.sdf_gate_width_px", "float", 4.0, false, ""});
    sdfRow.parameter_values.push_back({"signal.sdf_field_downsample", "enum", 0.0, false, "4"});
    sdfRow.parameter_values.push_back({"signal.root_pattern_ref", "enum", 0.0, false, "color_root_field"});
    sdfRow.parameter_values.push_back({"signal.blend_weight", "float", 0.35, false, ""});
    sourceLane.rows.push_back(sdfRow);

    ColorPipelineLaneState shapeLane{};
    shapeLane.lane_id = "shape";
    shapeLane.label = "Shape";
    ColorPipelineRowState shapeRow{};
    shapeRow.ui_row_id = 12;
    shapeRow.enabled = true;
    shapeRow.function_id = "identity";
    shapeLane.rows.push_back(shapeRow);

    const std::vector<ColorPipelineLaneState> lanes = {sourceLane, shapeLane};
    const std::vector<std::string> messages = {"Selected Source / Shape / Palette recipe is draft-only"};
    const std::string json = color_pipeline_graph_receipt::BuildColorPipelineGraphReceiptJson(lanes, messages, "mixed");
    Check(json.find("\"schema_id\": \"viewer.color_pipeline_graph_receipt.v1\"") != std::string::npos,
        "graph receipt declares schema id");
    Check(json.find("\"execution_authority\": \"linear_row_stack\"") != std::string::npos &&
            json.find("\"ui_projection\": \"linear_color_stack\"") != std::string::npos,
        "graph receipt declares report-only linear authority");
    Check(json.find("\"source_stack_kind\": \"mixed\"") != std::string::npos,
        "graph receipt reports source stack kind");
    Check(json.find("\"id\": \"source.0\"") != std::string::npos &&
            json.find("\"id\": \"source.1\"") != std::string::npos &&
            json.find("\"id\": \"shape.0\"") != std::string::npos,
        "graph receipt emits stable row node ids");
    Check(json.find("\"function_id\": \"sdf_signed_distance\"") != std::string::npos &&
            json.find("\"enabled\": false") != std::string::npos &&
            json.find("\"active_execution\": false") != std::string::npos &&
            json.find("\"fail_closed_reason\": \"row disabled\"") != std::string::npos,
        "graph receipt reports disabled rows without claiming execution");
    Check(json.find("\"sdf_applicator\": \"sdf_inside\"") != std::string::npos &&
            json.find("\"sdf_gate_width_px\": 4") != std::string::npos &&
            json.find("\"sdf_field_downsample\": \"4\"") != std::string::npos &&
            json.find("\"root_pattern_ref\": \"color_root_field\"") != std::string::npos &&
            json.find("\"blend_weight\": 0.35") != std::string::npos,
        "graph receipt preserves SDF applicator, downsample, root ref, and blend params");
    Check(json.find("source.0->source.1") == std::string::npos &&
            json.find("source.0->shape.0") != std::string::npos,
        "graph receipt active edges skip disabled rows and preserve lane projection");
    Check(json.find("\"unsupported_routes\":") != std::string::npos &&
            json.find("draft-only") != std::string::npos,
        "graph receipt carries unsupported route diagnostics");
}

void TestAutomationReportIncludesColorPipelineGraphReceipt() {
    const std::filesystem::path reportPath =
        std::filesystem::temp_directory_path() / "test_viewer_ui_automation_graph_receipt.json";
    ColorPipelineWindowState colorPipelineWindow{};
    colorPipelineWindow.initialized = true;
    ColorPipelineLaneState sourceLane{};
    sourceLane.lane_id = "source";
    ColorPipelineRowState row{};
    row.ui_row_id = 21;
    row.function_id = "sdf_normal_angle";
    row.parameter_values.push_back({"signal.sdf_gate", "enum", 0.0, false, "boundary_band"});
    row.parameter_values.push_back({"signal.sdf_field_downsample", "enum", 0.0, false, "8"});
    sourceLane.rows.push_back(row);
    colorPipelineWindow.lanes.push_back(sourceLane);
    colorPipelineWindow.validation_messages.push_back("unsupported_source_for_producer");

    ViewerUiAutomationLensSdfProbe probe{};
    probe.source_stack_kind = "sdf_only";
    ViewState view{};
    RenderSettings render{};
    RenderStats stats{};
    ViewerRenderPacingDecision pacing{};
    ViewerUiAutomationFrameProbe frameProbe{};
    ViewerUiAutomationEnumCommandReport enumReport{};
    HWND hwnd = CreateWindowExA(
        0,
        "STATIC",
        "automation-report-graph-test",
        WS_OVERLAPPED,
        0,
        0,
        1,
        1,
        nullptr,
        nullptr,
        GetModuleHandleA(nullptr),
        nullptr);
    Check(hwnd != nullptr, "automation graph receipt test creates a valid window handle");
    WriteColorPipelineUiAutomationReport(
        reportPath.string(),
        hwnd,
        {},
        colorPipelineWindow,
        nullptr,
        nullptr,
        view,
        render,
        stats,
        pacing,
        frameProbe,
        probe,
        enumReport,
        19);
    if (hwnd) {
        DestroyWindow(hwnd);
    }
    std::ifstream in(reportPath, std::ios::binary);
    std::ostringstream buffer;
    buffer << in.rdbuf();
    const std::string json = buffer.str();
    Check(json.find("\"color_pipeline_graph_receipt\":") != std::string::npos &&
            json.find("\"schema_id\": \"viewer.color_pipeline_graph_receipt.v1\"") != std::string::npos &&
            json.find("\"source_stack_kind\": \"sdf_only\"") != std::string::npos &&
            json.find("\"id\": \"source.0\"") != std::string::npos &&
            json.find("\"function_id\": \"sdf_normal_angle\"") != std::string::npos &&
            json.find("unsupported_source_for_producer") != std::string::npos,
        "automation report emits color_pipeline_graph_receipt with row and unsupported-route truth");
}

void TestRenderPacingProbeReportsTimingAndDecision() {
    RenderSettings render{};
    render.resolution = {2048, 1536};
    RenderStats stats{};
    stats.last_render_ms = 40.0f;
    ViewerRenderPacingDecision pacing{};
    pacing.preview_active = true;
    pacing.preview_scale = 0.5;
    pacing.full_quality_due = false;
    pacing.render_resolution = {1024, 768};

    ViewerUiAutomationRenderPacingProbe probe = BuildViewerUiAutomationRenderPacingProbe(render, stats, pacing);
    Check(probe.target_width == 2048 && probe.target_height == 1536,
        "render pacing probe reports target render dimensions");
    Check(probe.has_last_render_fps && probe.last_render_fps > 24.9 && probe.last_render_fps < 25.1,
        "render pacing probe reports measured FPS from last_render_ms");
    Check(probe.pacing_preview_active && probe.pacing_render_width == 1024 && probe.pacing_render_height == 768,
        "render pacing probe reports preview decision dimensions");
}

void TestRenderedFrameProbeHash() {
    RenderedFrameState frame{};
    std::vector<uint32_t> rgba = {0xff000000u, 0xff112233u, 0xff445566u, 0xff778899u};

    ViewerUiAutomationFrameProbe missing = BuildViewerUiAutomationFrameProbe(rgba, frame);
    Check(!missing.ready && missing.hash == 0,
        "frame probe stays unavailable until the renderer reports a ready frame");

    frame.ready = true;
    frame.width = 2;
    frame.height = 2;
    ViewerUiAutomationFrameProbe first = BuildViewerUiAutomationFrameProbe(rgba, frame);
    Check(first.ready && first.width == 2 && first.height == 2 && first.hash != 0,
        "frame probe hashes ready rendered RGBA data");

    rgba[2] ^= 0x00010101u;
    ViewerUiAutomationFrameProbe changed = BuildViewerUiAutomationFrameProbe(rgba, frame);
    Check(changed.ready && changed.hash != first.hash,
        "frame probe hash changes when rendered pixels change");
}

} // namespace

int main() {
    TestJsonStringEscaping();
    TestVisibleControlLookupAndFailClosedErrors();
    TestLensSdfProbeDefaults();
    TestLensSdfProbeTimingFields();
    TestColorPipelineGraphReceiptBuilderReportsLinearStackTruth();
    TestAutomationReportIncludesColorPipelineGraphReceipt();
    TestRenderPacingProbeReportsTimingAndDecision();
    TestRenderedFrameProbeHash();
    if (g_failed != 0) {
        std::printf("test_viewer_ui_automation_report: %d failure(s)\n", g_failed);
        return 1;
    }
    std::printf("test_viewer_ui_automation_report: all passed\n");
    return 0;
}
