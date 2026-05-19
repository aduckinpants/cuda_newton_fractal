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
    TestRenderedFrameProbeHash();
    if (g_failed != 0) {
        std::printf("test_viewer_ui_automation_report: %d failure(s)\n", g_failed);
        return 1;
    }
    std::printf("test_viewer_ui_automation_report: all passed\n");
    return 0;
}
