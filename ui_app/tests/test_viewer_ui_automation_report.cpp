#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

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
    Check(!probe.color_pipeline_active && probe.base_render_ms == 0.0f &&
            probe.field_ms == 0.0f && probe.postprocess_ms == 0.0f && probe.total_ms == 0.0f,
        "lens SDF automation probe reports stable timing defaults");
}

void TestLensSdfProbeTimingFields() {
    ViewerUiAutomationLensSdfProbe probe{};
    probe.color_pipeline_active = true;
    probe.base_render_ms = 3.0f;
    probe.field_ms = 2.0f;
    probe.postprocess_ms = 7.5f;
    probe.total_ms = probe.field_ms + probe.postprocess_ms;
    Check(probe.color_pipeline_active && probe.base_render_ms == 3.0f &&
            probe.field_ms == 2.0f && probe.postprocess_ms == 7.5f && probe.total_ms == 9.5f,
        "lens SDF automation probe carries separate field/postprocess timing");
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
    TestRenderPacingProbeReportsTimingAndDecision();
    TestRenderedFrameProbeHash();
    if (g_failed != 0) {
        std::printf("test_viewer_ui_automation_report: %d failure(s)\n", g_failed);
        return 1;
    }
    std::printf("test_viewer_ui_automation_report: all passed\n");
    return 0;
}
