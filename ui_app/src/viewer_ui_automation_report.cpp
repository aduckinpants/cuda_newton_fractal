#include "viewer_ui_automation_report.h"

#include "enum_id_utils.h"

#include <cmath>
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


void WriteRenderPacingAndFrameReportFields(
    std::ostream& out,
    const ViewerUiAutomationRenderPacingProbe& pacingProbe,
    const ViewerUiAutomationFrameProbe& frameProbe) {
    out << "  \"target_render_width\": " << pacingProbe.target_width << ",\n";
    out << "  \"target_render_height\": " << pacingProbe.target_height << ",\n";
    out << "  \"last_render_ms\": " << std::setprecision(12) << pacingProbe.last_render_ms << ",\n";
    out << "  \"last_render_fps\": ";
    if (pacingProbe.has_last_render_fps) {
        out << std::setprecision(12) << pacingProbe.last_render_fps;
    } else {
        out << "null";
    }
    out << ",\n";
    out << "  \"render_pacing_preview_active\": " << (pacingProbe.pacing_preview_active ? "true" : "false") << ",\n";
    out << "  \"render_pacing_preview_scale\": " << std::setprecision(12) << pacingProbe.pacing_preview_scale << ",\n";
    out << "  \"render_pacing_full_quality_due\": " << (pacingProbe.pacing_full_quality_due ? "true" : "false") << ",\n";
    out << "  \"render_pacing_render_width\": " << pacingProbe.pacing_render_width << ",\n";
    out << "  \"render_pacing_render_height\": " << pacingProbe.pacing_render_height << ",\n";
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
}

void WriteLensSdfReportFields(
    std::ostream& out,
    const ViewerUiAutomationLensSdfProbe& lensSdfProbe) {
    out << "  \"lens_sdf_enabled\": " << (lensSdfProbe.enabled ? "true" : "false") << ",\n";
    out << "  \"lens_sdf_valid\": " << (lensSdfProbe.valid ? "true" : "false") << ",\n";
    out << "  \"lens_sdf_color_pipeline_active\": " << (lensSdfProbe.color_pipeline_active ? "true" : "false") << ",\n";
    out << "  \"lens_sdf_field_source\": ";
    WriteAutomationReportString(out, lensSdfProbe.field_source);
    out << ",\n";
    out << "  \"lens_sdf_field_source_pack_id\": ";
    if (lensSdfProbe.field_source_pack_id.empty()) {
        out << "null";
    } else {
        WriteAutomationReportString(out, lensSdfProbe.field_source_pack_id);
    }
    out << ",\n";
    out << "  \"lens_sdf_field_source_error\": ";
    if (lensSdfProbe.field_source_error.empty()) {
        out << "null";
    } else {
        WriteAutomationReportString(out, lensSdfProbe.field_source_error);
    }
    out << ",\n";
    out << "  \"lens_sdf_backend_used\": ";
    WriteAutomationReportString(out, lensSdfProbe.backend_used);
    out << ",\n";
    out << "  \"lens_sdf_pack_backend_used\": ";
    WriteAutomationReportString(out, lensSdfProbe.pack_backend_used);
    out << ",\n";
    out << "  \"lens_sdf_pack_backend_fallback_used\": " <<
        (lensSdfProbe.pack_backend_fallback_used ? "true" : "false") << ",\n";
    out << "  \"lens_sdf_fallback_used\": " << (lensSdfProbe.fallback_used ? "true" : "false") << ",\n";
    out << "  \"lens_sdf_width\": " << lensSdfProbe.width << ",\n";
    out << "  \"lens_sdf_height\": " << lensSdfProbe.height << ",\n";
    out << "  \"lens_sdf_pixel_scale\": " << std::setprecision(12) << lensSdfProbe.pixel_scale << ",\n";
    out << "  \"lens_sdf_requested_downsample\": " << lensSdfProbe.requested_downsample << ",\n";
    out << "  \"lens_sdf_effective_downsample\": " << lensSdfProbe.effective_downsample << ",\n";
    out << "  \"lens_sdf_quality_mode\": ";
    WriteAutomationReportString(out, lensSdfProbe.quality_mode);
    out << ",\n";
    out << "  \"lens_sdf_field_cache_status\": ";
    WriteAutomationReportString(out, lensSdfProbe.field_cache_status);
    out << ",\n";
    out << "  \"lens_sdf_field_cache_hit\": " << (lensSdfProbe.field_cache_hit ? "true" : "false") << ",\n";
    out << "  \"lens_sdf_field_cache_mask_bytes\": " << lensSdfProbe.field_cache_mask_bytes << ",\n";
    out << "  \"lens_sdf_field_group_count\": " << lensSdfProbe.field_group_count << ",\n";
    out << "  \"lens_sdf_field_groups\": [";
    for (std::size_t index = 0; index < lensSdfProbe.field_groups.size(); ++index) {
        const ViewerUiAutomationLensSdfFieldGroupProbe& group = lensSdfProbe.field_groups[index];
        if (index > 0) {
            out << ", ";
        }
        out << "{";
        out << "\"group_index\": " << group.group_index << ", ";
        out << "\"requested_downsample\": " << group.requested_downsample << ", ";
        out << "\"effective_downsample\": " << group.effective_downsample << ", ";
        out << "\"row_count\": " << group.row_count << ", ";
        out << "\"has_inherited_row\": " << (group.has_inherited_row ? "true" : "false") << ", ";
        out << "\"has_explicit_row\": " << (group.has_explicit_row ? "true" : "false") << ", ";
        out << "\"cache_status\": ";
        WriteAutomationReportString(out, group.cache_status);
        out << ", ";
        out << "\"cache_hit\": " << (group.cache_hit ? "true" : "false") << ", ";
        out << "\"width\": " << group.width << ", ";
        out << "\"height\": " << group.height << ", ";
        out << "\"pixel_scale\": " << std::setprecision(12) << group.pixel_scale << ", ";
        out << "\"field_ms\": " << std::setprecision(12) << group.field_ms << ", ";
        out << "\"mask_downsample_ms\": " << std::setprecision(12) << group.mask_downsample_ms << ", ";
        out << "\"backend_ms\": " << std::setprecision(12) << group.backend_ms << ", ";
        out << "\"cache_lookup_ms\": " << std::setprecision(12) << group.cache_lookup_ms << ", ";
        out << "\"cache_store_ms\": " << std::setprecision(12) << group.cache_store_ms;
        out << "}";
    }
    out << "],\n";
    out << "  \"base_render_ms\": " << std::setprecision(12) << lensSdfProbe.base_render_ms << ",\n";
    out << "  \"lens_sdf_field_ms\": " << std::setprecision(12) << lensSdfProbe.field_ms << ",\n";
    out << "  \"lens_sdf_requested_equivalent_field_ms\": " << std::setprecision(12) << lensSdfProbe.requested_equivalent_field_ms << ",\n";
    out << "  \"lens_sdf_field_cache_lookup_ms\": " << std::setprecision(12) << lensSdfProbe.field_cache_lookup_ms << ",\n";
    out << "  \"lens_sdf_field_mask_downsample_ms\": " << std::setprecision(12) << lensSdfProbe.field_mask_downsample_ms << ",\n";
    out << "  \"lens_sdf_field_backend_ms\": " << std::setprecision(12) << lensSdfProbe.field_backend_ms << ",\n";
    out << "  \"lens_sdf_field_cache_store_ms\": " << std::setprecision(12) << lensSdfProbe.field_cache_store_ms << ",\n";
    out << "  \"lens_sdf_postprocess_ms\": " << std::setprecision(12) << lensSdfProbe.postprocess_ms << ",\n";
    out << "  \"lens_sdf_total_ms\": " << std::setprecision(12) << lensSdfProbe.total_ms << ",\n";
    out << "  \"lens_sdf_postprocess_pixel_step\": " << lensSdfProbe.postprocess_pixel_step << ",\n";
    out << "  \"lens_sdf_postprocess_worker_count\": " << lensSdfProbe.postprocess_worker_count << ",\n";
    out << "  \"lens_sdf_postprocess_backend_used\": ";
    WriteAutomationReportString(out, lensSdfProbe.postprocess_backend_used);
    out << ",\n";
    out << "  \"lens_sdf_postprocess_backend_fallback_used\": " <<
        (lensSdfProbe.postprocess_backend_fallback_used ? "true" : "false") << ",\n";
    out << "  \"lens_sdf_postprocess_backend_buffer_reused\": " <<
        (lensSdfProbe.postprocess_backend_buffer_reused ? "true" : "false") << ",\n";
    out << "  \"lens_sdf_postprocess_backend_buffer_grew\": " <<
        (lensSdfProbe.postprocess_backend_buffer_grew ? "true" : "false") << ",\n";
    out << "  \"lens_sdf_postprocess_direct_sample_count\": " << lensSdfProbe.postprocess_direct_sample_count << ",\n";
    out << "  \"lens_sdf_postprocess_neighborhood_sample_count\": " << lensSdfProbe.postprocess_neighborhood_sample_count << ",\n";
    out << "  \"lens_sdf_postprocess_source_direct_sample_count\": " << lensSdfProbe.postprocess_source_direct_sample_count << ",\n";
    out << "  \"lens_sdf_postprocess_source_neighborhood_sample_count\": " << lensSdfProbe.postprocess_source_neighborhood_sample_count << ",\n";
    out << "  \"lens_sdf_postprocess_filled_pixel_count\": " << lensSdfProbe.postprocess_filled_pixel_count << ",\n";
    out << "  \"lens_sdf_overlay_mode\": ";
    WriteAutomationReportString(out, lensSdfProbe.overlay_mode);
    out << ",\n";
    out << "  \"lens_sdf_overlay_active\": " << (lensSdfProbe.overlay_active ? "true" : "false") << ",\n";
    out << "  \"lens_sdf_overlay_opacity\": " << std::setprecision(12) << lensSdfProbe.overlay_opacity << ",\n";
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

void WriteAutomationReportOptionalDouble(std::ostream& out, bool hasValue, double value) {
    if (!hasValue) {
        out << "null";
        return;
    }
    out << std::setprecision(12) << value;
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

ViewerUiAutomationRenderPacingProbe BuildViewerUiAutomationRenderPacingProbe(
    const RenderSettings& render,
    const RenderStats& stats,
    const ViewerRenderPacingDecision& renderPacing) {
    ViewerUiAutomationRenderPacingProbe probe{};
    probe.target_width = render.resolution.x;
    probe.target_height = render.resolution.y;
    probe.last_render_ms = stats.last_render_ms;
    probe.has_last_render_fps = std::isfinite(stats.last_render_ms) && stats.last_render_ms > 0.0f;
    probe.last_render_fps = probe.has_last_render_fps ? 1000.0 / static_cast<double>(stats.last_render_ms) : 0.0;
    probe.pacing_preview_active = renderPacing.preview_active;
    probe.pacing_preview_scale = renderPacing.preview_scale;
    probe.pacing_full_quality_due = renderPacing.full_quality_due;
    probe.pacing_render_width = renderPacing.render_resolution.x;
    probe.pacing_render_height = renderPacing.render_resolution.y;
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
    const SdfPackViewerAutomationReport* sdfPackViewer,
    const ViewState& view,
    const RenderSettings& render,
    const RenderStats& stats,
    const ViewerRenderPacingDecision& renderPacing,
    const ViewerUiAutomationFrameProbe& frameProbe,
    const ViewerUiAutomationLensSdfProbe& lensSdfProbe,
    const ViewerUiAutomationEnumCommandReport& enumCommandReport,
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
    out << "  \"view_center_hp_x\": " << std::setprecision(17) << view.center_hp_x << ",\n";
    out << "  \"view_center_hp_y\": " << std::setprecision(17) << view.center_hp_y << ",\n";
    out << "  \"view_log2_zoom\": " << std::setprecision(17) << view.log2_zoom << ",\n";
    out << "  \"requested_enum_path\": ";
    if (enumCommandReport.requested_enum_path.empty()) {
        out << "null";
    } else {
        WriteAutomationReportString(out, enumCommandReport.requested_enum_path);
    }
    out << ",\n";
    out << "  \"requested_enum_id\": ";
    if (enumCommandReport.requested_enum_id.empty()) {
        out << "null";
    } else {
        WriteAutomationReportString(out, enumCommandReport.requested_enum_id);
    }
    out << ",\n";
    out << "  \"enum_consumed\": " << (enumCommandReport.enum_consumed ? "true" : "false") << ",\n";
    out << "  \"enum_error\": ";
    if (enumCommandReport.enum_error.empty()) {
        out << "null";
    } else {
        WriteAutomationReportString(out, enumCommandReport.enum_error);
    }
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
    out << "    \"pack_load_error\": ";
    if (equationPack.pack_load_error.empty()) {
        out << "null";
    } else {
        WriteAutomationReportString(out, equationPack.pack_load_error);
    }
    out << ",\n";
    out << "    \"controls\": [";
    for (std::size_t index = 0; index < equationPack.controls.size(); ++index) {
        const GenericEquationPackWorkbenchControlReport& control = equationPack.controls[index];
        if (index > 0) {
            out << ',';
        }
        out << "\n      {\n";
        out << "        \"control_id\": ";
        WriteAutomationReportString(out, control.control_id);
        out << ",\n";
        out << "        \"id\": ";
        WriteAutomationReportString(out, control.id);
        out << ",\n";
        out << "        \"param\": ";
        WriteAutomationReportString(out, control.param);
        out << ",\n";
        out << "        \"label\": ";
        WriteAutomationReportString(out, control.label);
        out << ",\n";
        out << "        \"value\": " << std::setprecision(12) << control.value << ",\n";
        out << "        \"integer_value\": " << (control.integer_value ? "true" : "false") << ",\n";
        out << "        \"min\": ";
        WriteAutomationReportOptionalDouble(out, control.has_min, control.min_value);
        out << ",\n";
        out << "        \"max\": ";
        WriteAutomationReportOptionalDouble(out, control.has_max, control.max_value);
        out << ",\n";
        out << "        \"step\": ";
        WriteAutomationReportOptionalDouble(out, control.has_step, control.step_value);
        out << ",\n";
        out << "        \"default\": ";
        WriteAutomationReportOptionalDouble(out, control.has_default_value, control.default_value);
        out << "\n      }";
    }
    if (!equationPack.controls.empty()) {
        out << "\n    ";
    }
    out << "],\n";
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
    out << "  \"sdf_pack_viewer\": {\n";
    const SdfPackViewerAutomationReport emptySdfPackReport;
    const SdfPackViewerAutomationReport& sdfPack =
        sdfPackViewer ? *sdfPackViewer : emptySdfPackReport;
    out << "    \"panel_open\": " << (sdfPack.panel_open ? "true" : "false") << ",\n";
    out << "    \"initialized\": " << (sdfPack.initialized ? "true" : "false") << ",\n";
    out << "    \"force_open_for_automation\": " << (sdfPack.force_open_for_automation ? "true" : "false") << ",\n";
    out << "    \"have_pack\": " << (sdfPack.have_pack ? "true" : "false") << ",\n";
    out << "    \"use_as_sdf_field_source\": " << (sdfPack.use_as_sdf_field_source ? "true" : "false") << ",\n";
    out << "    \"pack_path\": ";
    WriteAutomationReportString(out, sdfPack.pack_path);
    out << ",\n";
    out << "    \"pack_id\": ";
    WriteAutomationReportString(out, sdfPack.pack_id);
    out << ",\n";
    out << "    \"pack_name\": ";
    WriteAutomationReportString(out, sdfPack.pack_name);
    out << ",\n";
    out << "    \"pack_load_error\": ";
    if (sdfPack.pack_load_error.empty()) {
        out << "null";
    } else {
        WriteAutomationReportString(out, sdfPack.pack_load_error);
    }
    out << ",\n";
    out << "    \"backend_preference\": ";
    WriteAutomationReportString(out, sdfPack.backend_preference);
    out << ",\n";
    out << "    \"built_in_pack_selector_control_id\": ";
    WriteAutomationReportString(out, sdfPack.built_in_pack_selector_control_id);
    out << ",\n";
    out << "    \"selected_built_in_pack_id\": ";
    if (sdfPack.selected_built_in_pack_id.empty()) {
        out << "null";
    } else {
        WriteAutomationReportString(out, sdfPack.selected_built_in_pack_id);
    }
    out << ",\n";
    out << "    \"built_in_packs\": [";
    for (std::size_t index = 0; index < sdfPack.built_in_packs.size(); ++index) {
        const SdfPackViewerBuiltInPackReport& option = sdfPack.built_in_packs[index];
        if (index > 0) {
            out << ',';
        }
        out << "\n      {\n";
        out << "        \"pack_id\": ";
        WriteAutomationReportString(out, option.pack_id);
        out << ",\n";
        out << "        \"label\": ";
        WriteAutomationReportString(out, option.label);
        out << ",\n";
        out << "        \"selected\": " << (option.selected ? "true" : "false") << "\n";
        out << "      }";
    }
    if (!sdfPack.built_in_packs.empty()) {
        out << "\n    ";
    }
    out << "],\n";
    out << "    \"controls\": [";
    for (std::size_t index = 0; index < sdfPack.controls.size(); ++index) {
        const SdfPackViewerControlReport& control = sdfPack.controls[index];
        if (index > 0) {
            out << ',';
        }
        out << "\n      {\n";
        out << "        \"control_id\": ";
        WriteAutomationReportString(out, control.control_id);
        out << ",\n";
        out << "        \"param\": ";
        WriteAutomationReportString(out, control.param);
        out << ",\n";
        out << "        \"label\": ";
        WriteAutomationReportString(out, control.label);
        out << ",\n";
        out << "        \"value\": " << std::setprecision(12) << control.value << ",\n";
        out << "        \"min\": ";
        WriteAutomationReportOptionalDouble(out, control.has_min, control.min_value);
        out << ",\n";
        out << "        \"max\": ";
        WriteAutomationReportOptionalDouble(out, control.has_max, control.max_value);
        out << ",\n";
        out << "        \"default\": ";
        WriteAutomationReportOptionalDouble(out, control.has_default_value, control.default_value);
        out << "\n      }";
    }
    if (!sdfPack.controls.empty()) {
        out << "\n    ";
    }
    out << "],\n";
    out << "    \"preview_ok\": " << (sdfPack.preview_ok ? "true" : "false") << ",\n";
    out << "    \"preview_error\": ";
    if (sdfPack.preview_error.empty()) {
        out << "null";
    } else {
        WriteAutomationReportString(out, sdfPack.preview_error);
    }
    out << ",\n";
    out << "    \"preview_backend_used\": ";
    WriteAutomationReportString(out, sdfPack.preview_backend_used);
    out << ",\n";
    out << "    \"preview_backend_fallback_used\": " << (sdfPack.preview_backend_fallback_used ? "true" : "false") << ",\n";
    out << "    \"preview_width\": " << sdfPack.preview_width << ",\n";
    out << "    \"preview_height\": " << sdfPack.preview_height << ",\n";
    out << "    \"preview_pixel_scale\": " << std::setprecision(12) << sdfPack.preview_pixel_scale << ",\n";
    out << "    \"preview_sample_count\": " << sdfPack.preview_sample_count << ",\n";
    out << "    \"preview_min_signed_distance_px\": " << std::setprecision(12) << sdfPack.preview_min_signed_distance_px << ",\n";
    out << "    \"preview_max_signed_distance_px\": " << std::setprecision(12) << sdfPack.preview_max_signed_distance_px << ",\n";
    out << "    \"preview_mean_signed_distance_px\": " << std::setprecision(12) << sdfPack.preview_mean_signed_distance_px << ",\n";
    out << "    \"preview_field_hash\": ";
    if (sdfPack.preview_field_hash.empty()) {
        out << "null";
    } else {
        WriteAutomationReportString(out, sdfPack.preview_field_hash);
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
    const ViewerUiAutomationRenderPacingProbe pacingProbe = BuildViewerUiAutomationRenderPacingProbe(render, stats, renderPacing);
    WriteRenderPacingAndFrameReportFields(out, pacingProbe, frameProbe);
    WriteLensSdfReportFields(out, lensSdfProbe);
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
