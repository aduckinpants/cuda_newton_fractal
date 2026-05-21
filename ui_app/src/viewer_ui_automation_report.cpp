#include "viewer_ui_automation_report.h"

#include "enum_id_utils.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <ostream>
#include <sstream>

namespace {

std::uint64_t HashAutomationFrameBytes(const std::vector<uint32_t>& rgba, std::size_t pixelCount) {
    std::uint64_t hash = 1469598103934665603ull;
    const auto* bytes = reinterpret_cast<const unsigned char*>(rgba.data());
    const std::size_t byteCount = pixelCount * sizeof(uint32_t);
    for (std::size_t index = 0; index < byteCount; ++index) {
        hash ^= static_cast<std::uint64_t>(bytes[index]);
        hash *= 1099511628211ull;
    }
    return hash;
}

} // namespace

std::string JsonEscapeAutomationReportString(const std::string& value) {
    std::string out;
    out.reserve(value.size() + 8);
    for (char c : value) {
        switch (c) {
        case '\\': out += "\\\\"; break;
        case '"': out += "\\\""; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if (static_cast<unsigned char>(c) < 0x20) {
                constexpr char kHex[] = "0123456789abcdef";
                const unsigned char byte = static_cast<unsigned char>(c);
                out += "\\u00";
                out += kHex[(byte >> 4) & 0x0f];
                out += kHex[byte & 0x0f];
            } else {
                out += c;
            }
            break;
        }
    }
    return out;
}

void WriteAutomationReportString(std::ostream& out, const std::string& value) {
    out << '"' << JsonEscapeAutomationReportString(value) << '"';
}

ViewerUiAutomationFrameProbe BuildViewerUiAutomationFrameProbe(
    const std::vector<uint32_t>& rgba,
    const RenderedFrameState& renderedFrame) {
    ViewerUiAutomationFrameProbe probe{};
    probe.ready = renderedFrame.ready && renderedFrame.width > 0 && renderedFrame.height > 0;
    probe.width = renderedFrame.width;
    probe.height = renderedFrame.height;
    if (!probe.ready) {
        return probe;
    }
    const std::size_t pixelCount =
        static_cast<std::size_t>(renderedFrame.width) * static_cast<std::size_t>(renderedFrame.height);
    if (rgba.size() < pixelCount) {
        probe.ready = false;
        probe.hash = 0;
        return probe;
    }
    probe.hash = HashAutomationFrameBytes(rgba, pixelCount);
    return probe;
}

bool ViewerUiAutomationControlIdVisible(
    const std::vector<ViewerUiAutomationRect>& viewerUiAutomationRects,
    const ColorPipelineWindowState& colorPipelineWindow,
    const std::string& controlId) {
    for (const ViewerUiAutomationRect& rect : viewerUiAutomationRects) {
        if (rect.control_id == controlId) {
            return true;
        }
    }
    for (const ColorPipelineUiAutomationRect& rect : colorPipelineWindow.ui_automation_rects) {
        if (rect.control_id == controlId) {
            return true;
        }
    }
    return false;
}

void FailClosedPendingUiAutomationSetValue(
    const std::vector<ViewerUiAutomationRect>& viewerUiAutomationRects,
    ColorPipelineWindowState& colorPipelineWindow) {
    if (!colorPipelineWindow.ui_automation_set_pending ||
        colorPipelineWindow.ui_automation_set_consumed ||
        !colorPipelineWindow.ui_automation_set_error.empty() ||
        colorPipelineWindow.ui_automation_set_control_id.empty()) {
        return;
    }

    const std::string& controlId = colorPipelineWindow.ui_automation_set_control_id;
    if (ViewerUiAutomationControlIdVisible(viewerUiAutomationRects, colorPipelineWindow, controlId)) {
        colorPipelineWindow.ui_automation_set_error = std::string("set-value automation did not support visible control: ") + controlId;
        return;
    }

    const bool wantsColorPipeline = controlId.rfind("color_pipeline.", 0) == 0;
    const bool colorPipelineReady = colorPipelineWindow.open &&
        colorPipelineWindow.initialized &&
        !colorPipelineWindow.ui_automation_rects.empty();
    if (!wantsColorPipeline || colorPipelineReady) {
        colorPipelineWindow.ui_automation_set_error = std::string("requested set-value control is not visible or unsupported: ") + controlId;
    }
}

void WriteColorPipelineUiAutomationReport(
    const std::string& reportPath,
    HWND hwnd,
    const std::vector<ViewerUiAutomationRect>& viewerUiAutomationRects,
    const ColorPipelineWindowState& colorPipelineWindow,
    const GenericEquationPackWorkbenchAutomationReport* equationPackWorkbench,
    const ViewState& view,
    const ViewerUiAutomationFrameProbe& frameProbe,
    std::int64_t uiAutomationCommandSequence) {
    if (reportPath.empty() || !hwnd) {
        return;
    }

    std::error_code ignoredError;
    const std::filesystem::path finalPath(reportPath);
    if (finalPath.has_parent_path()) {
        std::filesystem::create_directories(finalPath.parent_path(), ignoredError);
    }

    POINT clientOrigin{0, 0};
    if (!ClientToScreen(hwnd, &clientOrigin)) {
        return;
    }

    std::filesystem::path tempPath = finalPath;
    tempPath += ".tmp";
    std::ofstream out(tempPath, std::ios::binary | std::ios::trunc);
    if (!out) {
        return;
    }

    out << "{\n";
    out << "  \"window_open\": " << (colorPipelineWindow.open ? "true" : "false") << ",\n";
    out << "  \"initialized\": " << (colorPipelineWindow.initialized ? "true" : "false") << ",\n";
    out << "  \"force_open_for_automation\": " << (colorPipelineWindow.force_open_for_automation ? "true" : "false") << ",\n";
    out << "  \"ui_automation_command_sequence\": ";
    if (uiAutomationCommandSequence >= 0) {
        out << uiAutomationCommandSequence;
    } else {
        out << "null";
    }
    out << ",\n";
    const char* currentFractalTypeId = enum_id_utils::LookupEnumId(view.fractal_type, enum_id_utils::kFractalTypeIds);
    out << "  \"current_fractal_type\": ";
    WriteAutomationReportString(out, currentFractalTypeId ? currentFractalTypeId : "");
    out << ",\n";
    out << "  \"equation_pack_workbench\": {\n";
    const GenericEquationPackWorkbenchAutomationReport emptyEquationPackReport;
    const GenericEquationPackWorkbenchAutomationReport& equationPack =
        equationPackWorkbench ? *equationPackWorkbench : emptyEquationPackReport;
    out << "    \"window_open\": " << (equationPack.window_open ? "true" : "false") << ",\n";
    out << "    \"initialized\": " << (equationPack.initialized ? "true" : "false") << ",\n";
    out << "    \"force_open_for_automation\": " << (equationPack.force_open_for_automation ? "true" : "false") << ",\n";
    out << "    \"have_pack\": " << (equationPack.have_pack ? "true" : "false") << ",\n";
    out << "    \"pack_path\": ";
    WriteAutomationReportString(out, equationPack.pack_path);
    out << ",\n";
    out << "    \"pack_id\": ";
    WriteAutomationReportString(out, equationPack.pack_id);
    out << ",\n";
    out << "    \"pack_name\": ";
    WriteAutomationReportString(out, equationPack.pack_name);
    out << ",\n";
    out << "    \"preview_ok\": " << (equationPack.preview_ok ? "true" : "false") << ",\n";
    out << "    \"preview_error\": ";
    if (equationPack.preview_error.empty()) {
        out << "null";
    } else {
        WriteAutomationReportString(out, equationPack.preview_error);
    }
    out << ",\n";
    out << "    \"preview_backend_used\": ";
    WriteAutomationReportString(out, equationPack.preview_backend_used);
    out << ",\n";
    out << "    \"preview_sample_count\": " << equationPack.preview_sample_count << ",\n";
    out << "    \"preview_converged_count\": " << equationPack.preview_converged_count << ",\n";
    out << "    \"preview_escaped_count\": " << equationPack.preview_escaped_count << ",\n";
    out << "    \"preview_bounded_count\": " << equationPack.preview_bounded_count << ",\n";
    out << "    \"preview_nonfinite_count\": " << equationPack.preview_nonfinite_count << ",\n";
    out << "    \"preview_mean_iterations\": " << std::setprecision(12) << equationPack.preview_mean_iterations << ",\n";
    out << "    \"preview_mean_abs2\": " << std::setprecision(12) << equationPack.preview_mean_abs2 << ",\n";
    out << "    \"preview_result_hash\": ";
    if (equationPack.preview_result_hash.empty()) {
        out << "null";
    } else {
        WriteAutomationReportString(out, equationPack.preview_result_hash);
    }
    out << ",\n";
    out << "    \"preview_image_width\": " << equationPack.preview_image_width << ",\n";
    out << "    \"preview_image_height\": " << equationPack.preview_image_height << ",\n";
    out << "    \"preview_image_hash\": ";
    if (equationPack.preview_image_hash.empty()) {
        out << "null";
    } else {
        WriteAutomationReportString(out, equationPack.preview_image_hash);
    }
    out << "\n";
    out << "  },\n";
    if (colorPipelineWindow.ui_automation_click_control_id.empty()) {
        out << "  \"requested_click_control_id\": null,\n";
    } else {
        out << "  \"requested_click_control_id\": ";
        WriteAutomationReportString(out, colorPipelineWindow.ui_automation_click_control_id);
        out << ",\n";
    }
    out << "  \"click_consumed\": " << (colorPipelineWindow.ui_automation_click_consumed ? "true" : "false") << ",\n";
    if (colorPipelineWindow.ui_automation_set_control_id.empty()) {
        out << "  \"requested_set_control_id\": null,\n";
    } else {
        out << "  \"requested_set_control_id\": ";
        WriteAutomationReportString(out, colorPipelineWindow.ui_automation_set_control_id);
        out << ",\n";
    }
    out << "  \"requested_set_value\": " << std::setprecision(12) << colorPipelineWindow.ui_automation_set_control_value << ",\n";
    out << "  \"set_value_consumed\": " << (colorPipelineWindow.ui_automation_set_consumed ? "true" : "false") << ",\n";
    if (colorPipelineWindow.ui_automation_set_error.empty()) {
        out << "  \"set_value_error\": null,\n";
    } else {
        out << "  \"set_value_error\": ";
        WriteAutomationReportString(out, colorPipelineWindow.ui_automation_set_error);
        out << ",\n";
    }
    out << "  \"rendered_frame_ready\": " << (frameProbe.ready ? "true" : "false") << ",\n";
    out << "  \"rendered_frame_width\": " << frameProbe.width << ",\n";
    out << "  \"rendered_frame_height\": " << frameProbe.height << ",\n";
    out << "  \"rendered_frame_hash\": ";
    if (frameProbe.ready) {
        std::ostringstream hashText;
        hashText << "fnv1a64:" << std::hex << std::setw(16) << std::setfill('0') << frameProbe.hash;
        WriteAutomationReportString(out, hashText.str());
    } else {
        out << "null";
    }
    out << ",\n";
    out << "  \"lane_rows\": [";
    bool firstLaneRow = true;
    for (const ColorPipelineLaneState& lane : colorPipelineWindow.lanes) {
        for (const ColorPipelineRowState& row : lane.rows) {
            if (!firstLaneRow) {
                out << ',';
            }
            firstLaneRow = false;
            out << "\n    ";
            WriteAutomationReportString(out, lane.lane_id + std::string(":") + row.function_id);
        }
    }
    if (!firstLaneRow) {
        out << '\n';
    }
    out << "  ],\n";
    out << "  \"rows\": [";
    bool firstRowState = true;
    for (const ColorPipelineLaneState& lane : colorPipelineWindow.lanes) {
        for (const ColorPipelineRowState& row : lane.rows) {
            if (!firstRowState) {
                out << ",";
            }
            firstRowState = false;
            out << "\n    {\"lane_id\": ";
            WriteAutomationReportString(out, lane.lane_id);
            out << ", \"ui_row_id\": " << row.ui_row_id << ", \"function_id\": ";
            WriteAutomationReportString(out, row.function_id);
            out << ", \"enabled\": " << (row.enabled ? "true" : "false") << "}";
        }
    }
    if (!firstRowState) {
        out << '\n';
    }
    out << "  ],\n";
    out << "  \"validation_messages\": [";
    for (std::size_t index = 0; index < colorPipelineWindow.validation_messages.size(); ++index) {
        if (index > 0) {
            out << ',';
        }
        out << "\n    ";
        WriteAutomationReportString(out, colorPipelineWindow.validation_messages[index]);
    }
    if (!colorPipelineWindow.validation_messages.empty()) {
        out << '\n';
    }
    out << "  ],\n";
    out << "  \"controls\": [";
    bool firstControl = true;
    for (const ViewerUiAutomationRect& rect : viewerUiAutomationRects) {
        if (!firstControl) {
            out << ',';
        }
        firstControl = false;
        out << "\n    {\"control_id\": ";
        WriteAutomationReportString(out, rect.control_id);
        out << ", \"screen_rect\": ["
            << (clientOrigin.x + rect.client_left) << ", "
            << (clientOrigin.y + rect.client_top) << ", "
            << (clientOrigin.x + rect.client_right) << ", "
            << (clientOrigin.y + rect.client_bottom) << "]}";
    }
    for (const ColorPipelineUiAutomationRect& rect : colorPipelineWindow.ui_automation_rects) {
        if (!firstControl) {
            out << ',';
        }
        firstControl = false;
        out << "\n    {\"control_id\": ";
        WriteAutomationReportString(out, rect.control_id);
        out << ", \"screen_rect\": ["
            << (clientOrigin.x + rect.client_left) << ", "
            << (clientOrigin.y + rect.client_top) << ", "
            << (clientOrigin.x + rect.client_right) << ", "
            << (clientOrigin.y + rect.client_bottom) << "]}";
    }
    out << "\n  ]\n}\n";
    out.close();
    if (!out) {
        std::filesystem::remove(tempPath, ignoredError);
        return;
    }

    std::filesystem::remove(finalPath, ignoredError);
    std::filesystem::rename(tempPath, finalPath, ignoredError);
    if (ignoredError) {
        ignoredError.clear();
        std::filesystem::copy_file(tempPath, finalPath, std::filesystem::copy_options::overwrite_existing, ignoredError);
        std::filesystem::remove(tempPath, ignoredError);
    }
}
