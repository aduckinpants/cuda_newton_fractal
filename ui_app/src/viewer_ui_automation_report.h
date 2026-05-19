#pragma once

#include "color_pipeline_window.h"
#include "fractal_types.h"
#include "render_capture_guard.h"

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

std::string JsonEscapeAutomationReportString(const std::string& value);
void WriteAutomationReportString(std::ostream& out, const std::string& value);
ViewerUiAutomationFrameProbe BuildViewerUiAutomationFrameProbe(
    const std::vector<uint32_t>& rgba,
    const RenderedFrameState& renderedFrame);
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
    const ViewState& view,
    const ViewerUiAutomationFrameProbe& frameProbe);
