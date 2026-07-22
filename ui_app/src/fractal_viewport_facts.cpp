#include "fractal_viewport_facts.h"

#include "fractal_catalog.h"
#include "fractal_viewport_mapping.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <system_error>

namespace {

bool Fail(const std::string& message, std::string* outError) {
    if (outError) *outError = message;
    return false;
}

FractalViewportComplexPoint ToFactPoint(const FractalViewportMappedPoint& point) {
    return {point.real, point.imag};
}

void AppendJsonString(std::ostringstream& out, const std::string& value) {
    out << '"';
    for (const unsigned char ch : value) {
        switch (ch) {
        case '"': out << "\\\""; break;
        case '\\': out << "\\\\"; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default:
            if (ch < 0x20) {
                constexpr char kHex[] = "0123456789abcdef";
                out << "\\u00" << kHex[(ch >> 4) & 0x0f] << kHex[ch & 0x0f];
            } else {
                out << static_cast<char>(ch);
            }
            break;
        }
    }
    out << '"';
}

void AppendPoint(std::ostringstream& out, const FractalViewportComplexPoint& point, int indent) {
    out << std::string(static_cast<std::size_t>(indent), ' ')
        << "{\"real\": " << point.real << ", \"imag\": " << point.imag << '}';
}

bool RemoveOwnedTemp(const std::filesystem::path& temp) {
    std::error_code error;
    std::filesystem::remove(temp, error);
    return !error;
}

} // namespace

bool ComputeFractalViewportFacts(
    const ViewState& view,
    const RenderSettings& render,
    FractalViewportFacts* outFacts,
    std::string* outError) {
    if (outError) outError->clear();
    if (!outFacts) return Fail("viewport facts require an output object", outError);
    if (render.resolution.x <= 0 || render.resolution.y <= 0) {
        return Fail("viewport facts require positive render dimensions", outError);
    }
    if (!std::isfinite(view.center_hp_x) || !std::isfinite(view.center_hp_y) ||
        !std::isfinite(view.log2_zoom) || !std::isfinite(static_cast<double>(view.rotation_degrees))) {
        return Fail("viewport facts require finite camera authority", outError);
    }
    const FractalCatalogEntry* catalogEntry = FindFractalCatalogEntry(view.fractal_type);
    if (!catalogEntry || !catalogEntry->selector_id || catalogEntry->selector_id[0] == '\0') {
        return Fail("viewport facts require a live catalog selector", outError);
    }

    const FractalViewportMappingTransform transform = BuildFractalViewportMappingTransform(
        render.resolution.x,
        render.resolution.y,
        view.log2_zoom,
        static_cast<double>(view.rotation_degrees));
    const double zoom = transform.resolved_zoom;
    if (!std::isfinite(zoom) || zoom <= 0.0) {
        return Fail("viewport facts resolved zoom must be positive and finite", outError);
    }
    const double aspect = transform.aspect_ratio;
    const double base = transform.base_half_height;
    const double halfWidth = base * aspect;
    const double halfHeight = base;
    if (!std::isfinite(aspect) || !std::isfinite(base) ||
        !std::isfinite(halfWidth) || !std::isfinite(halfHeight) ||
        aspect <= 0.0 || base <= 0.0 || halfWidth <= 0.0 || halfHeight <= 0.0) {
        return Fail("viewport facts spans must be positive and finite", outError);
    }

    const double localStepX = (2.0 * halfWidth) / static_cast<double>(render.resolution.x);
    const double localStepY = (2.0 * halfHeight) / static_cast<double>(render.resolution.y);

    FractalViewportFacts facts{};
    facts.width = render.resolution.x;
    facts.height = render.resolution.y;
    facts.aspect_ratio = aspect;
    facts.fractal_type = view.fractal_type;
    facts.center_hp_x = view.center_hp_x;
    facts.center_hp_y = view.center_hp_y;
    facts.log2_zoom = view.log2_zoom;
    facts.resolved_zoom = zoom;
    facts.rotation_degrees = static_cast<double>(view.rotation_degrees);
    facts.local_half_width = halfWidth;
    facts.local_half_height = halfHeight;
    facts.local_full_width = 2.0 * halfWidth;
    facts.local_full_height = 2.0 * halfHeight;
    facts.pixel_step_x = ToFactPoint(MapFractalViewportLocalPoint(transform, 0.0, 0.0, localStepX, 0.0));
    facts.pixel_step_y = ToFactPoint(MapFractalViewportLocalPoint(transform, 0.0, 0.0, 0.0, localStepY));
    facts.complex_units_per_pixel_x = std::hypot(facts.pixel_step_x.real, facts.pixel_step_x.imag);
    facts.complex_units_per_pixel_y = std::hypot(facts.pixel_step_y.real, facts.pixel_step_y.imag);

    const double edgeLocal[4][2] = {
        {-halfWidth, -halfHeight},
        { halfWidth, -halfHeight},
        { halfWidth,  halfHeight},
        {-halfWidth,  halfHeight},
    };
    const double centerHalfWidth = halfWidth - 0.5 * localStepX;
    const double centerHalfHeight = halfHeight - 0.5 * localStepY;
    const double centerLocal[4][2] = {
        {-centerHalfWidth, -centerHalfHeight},
        { centerHalfWidth, -centerHalfHeight},
        { centerHalfWidth,  centerHalfHeight},
        {-centerHalfWidth,  centerHalfHeight},
    };

    for (std::size_t index = 0; index < facts.continuous_edge_corners.size(); ++index) {
        facts.continuous_edge_corners[index] = ToFactPoint(MapFractalViewportLocalPoint(
            transform, view.center_hp_x, view.center_hp_y, edgeLocal[index][0], edgeLocal[index][1]));
        facts.pixel_center_corners[index] = ToFactPoint(MapFractalViewportLocalPoint(
            transform, view.center_hp_x, view.center_hp_y, centerLocal[index][0], centerLocal[index][1]));
    }
    facts.axis_aligned_min = facts.continuous_edge_corners[0];
    facts.axis_aligned_max = facts.continuous_edge_corners[0];
    for (const auto& corner : facts.continuous_edge_corners) {
        facts.axis_aligned_min.real = (std::min)(facts.axis_aligned_min.real, corner.real);
        facts.axis_aligned_min.imag = (std::min)(facts.axis_aligned_min.imag, corner.imag);
        facts.axis_aligned_max.real = (std::max)(facts.axis_aligned_max.real, corner.real);
        facts.axis_aligned_max.imag = (std::max)(facts.axis_aligned_max.imag, corner.imag);
    }
    *outFacts = facts;
    return true;
}

bool BuildFractalViewportFactsJson(
    const ViewState& view,
    const RenderSettings& render,
    std::string* outJson,
    std::string* outError) {
    if (!outJson) return Fail("viewport facts require a JSON output string", outError);
    FractalViewportFacts facts{};
    if (!ComputeFractalViewportFacts(view, render, &facts, outError)) return false;
    const FractalCatalogEntry* catalogEntry = FindFractalCatalogEntry(facts.fractal_type);
    if (!catalogEntry) return Fail("viewport facts selector disappeared after validation", outError);

    std::ostringstream out;
    out << std::setprecision(17);
    out << "{\n  \"schema_version\": 1,\n  \"mapping_id\": \"cuda_fractal_renderer_pixel_center_v1\",\n  \"selected_fractal_type\": ";
    AppendJsonString(out, catalogEntry->selector_id);
    out << ",\n  \"render\": {\n"
        << "    \"width\": " << facts.width << ",\n"
        << "    \"height\": " << facts.height << ",\n"
        << "    \"aspect_ratio\": " << facts.aspect_ratio << "\n  },\n"
        << "  \"camera\": {\n"
        << "    \"center_hp_x\": " << facts.center_hp_x << ",\n"
        << "    \"center_hp_y\": " << facts.center_hp_y << ",\n"
        << "    \"log2_zoom\": " << facts.log2_zoom << ",\n"
        << "    \"resolved_zoom\": " << facts.resolved_zoom << ",\n"
        << "    \"rotation_degrees\": " << facts.rotation_degrees << "\n  },\n"
        << "  \"local_frame\": {\n"
        << "    \"half_width\": " << facts.local_half_width << ",\n"
        << "    \"half_height\": " << facts.local_half_height << ",\n"
        << "    \"full_width\": " << facts.local_full_width << ",\n"
        << "    \"full_height\": " << facts.local_full_height << "\n  },\n"
        << "  \"complex_pixel_basis\": {\n    \"x_step\": ";
    AppendPoint(out, facts.pixel_step_x, 0);
    out << ",\n    \"y_step\": ";
    AppendPoint(out, facts.pixel_step_y, 0);
    out << ",\n    \"units_per_pixel_x\": " << facts.complex_units_per_pixel_x
        << ",\n    \"units_per_pixel_y\": " << facts.complex_units_per_pixel_y << "\n  },\n";

    out << "  \"continuous_edge_corners\": [\n";
    for (std::size_t index = 0; index < facts.continuous_edge_corners.size(); ++index) {
        AppendPoint(out, facts.continuous_edge_corners[index], 4);
        out << (index + 1 < facts.continuous_edge_corners.size() ? ",\n" : "\n");
    }
    out << "  ],\n  \"pixel_center_corners\": [\n";
    for (std::size_t index = 0; index < facts.pixel_center_corners.size(); ++index) {
        AppendPoint(out, facts.pixel_center_corners[index], 4);
        out << (index + 1 < facts.pixel_center_corners.size() ? ",\n" : "\n");
    }
    out << "  ],\n  \"axis_aligned_complex_bounds\": {\n    \"minimum\": ";
    AppendPoint(out, facts.axis_aligned_min, 0);
    out << ",\n    \"maximum\": ";
    AppendPoint(out, facts.axis_aligned_max, 0);
    out << "\n  },\n  \"fit_model\": {\n"
        << "    \"forward_mapping\": \"complex = center + rotate((nx * (2 / zoom) * aspect, ny * (2 / zoom)), rotation_degrees)\",\n"
        << "    \"pixel_normalization\": \"nx = ((px + 0.5 + sample_offset_x) / width - 0.5) * 2; ny = ((py + 0.5 + sample_offset_y) / height - 0.5) * 2\",\n"
        << "    \"inverse_fit\": \"fit_log2_zoom = log2(min(2 * aspect / required_local_half_width, 2 / required_local_half_height))\",\n"
        << "    \"point_preparation\": \"subtract proposed center, rotate by negative rotation_degrees, bound absolute local coordinates, then include declared margin\"\n"
        << "  }\n}\n";
    *outJson = out.str();
    return true;
}

bool WriteFractalViewportFactsJsonFile(
    const std::string& path,
    const ViewState& view,
    const RenderSettings& render,
    std::string* outError) {
    if (path.empty()) return Fail("viewport facts output path is empty", outError);
    const std::filesystem::path target(path);
    const std::filesystem::path parent = target.parent_path();
    std::error_code statusError;
    if (!parent.empty() && (!std::filesystem::exists(parent, statusError) ||
        !std::filesystem::is_directory(parent, statusError))) {
        return Fail("viewport facts parent directory does not exist: " + parent.string(), outError);
    }
    if (statusError) return Fail("viewport facts parent check failed: " + statusError.message(), outError);

    std::string bytes;
    if (!BuildFractalViewportFactsJson(view, render, &bytes, outError)) return false;
    const std::filesystem::path temp(path + ".tmp");
    std::error_code removeError;
    std::filesystem::remove(temp, removeError);
    if (removeError) return Fail("viewport facts stale temporary cleanup failed: " + removeError.message(), outError);
    {
        std::ofstream file(temp, std::ios::binary | std::ios::trunc);
        if (!file) return Fail("viewport facts temporary open failed: " + temp.string(), outError);
        file.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
        file.flush();
        if (!file.good()) {
            file.close();
            RemoveOwnedTemp(temp);
            return Fail("viewport facts temporary write failed: " + temp.string(), outError);
        }
    }
    std::error_code targetError;
    std::filesystem::remove(target, targetError);
    if (targetError) {
        RemoveOwnedTemp(temp);
        return Fail("viewport facts existing target removal failed: " + targetError.message(), outError);
    }
    std::error_code renameError;
    std::filesystem::rename(temp, target, renameError);
    if (!renameError) return true;
    std::error_code copyError;
    std::filesystem::copy_file(temp, target, std::filesystem::copy_options::overwrite_existing, copyError);
    if (copyError) {
        RemoveOwnedTemp(temp);
        return Fail("viewport facts replacement failed: " + renameError.message() + "; " + copyError.message(), outError);
    }
    if (!RemoveOwnedTemp(temp)) return Fail("viewport facts copy fallback cleanup failed", outError);
    return true;
}
