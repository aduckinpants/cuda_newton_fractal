#include "fractal_preset_core.h"

#include "color_pipeline_core.h"
#include "enum_id_utils.h"
#include "explaino_root_field.h"
#include "fractal_derived_fields.h"
#include "fractal_family_rules.h"
#include "json_min.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace {

void JsonString(std::ostringstream& out, const char* value) {
    out << '"';
    for (const char* cursor = value ? value : ""; *cursor; ++cursor) {
        if (*cursor == '"' || *cursor == '\\') out << '\\';
        out << *cursor;
    }
    out << '"';
}

void JsonNumber(std::ostringstream& out, double value) {
    out << std::setprecision(17) << value;
}

bool GetObject(const json_min::Value& value, const char* field, const json_min::Object** outObject, std::string* outError) {
    const json_min::Value* child = value.get(field);
    if (!child || !child->is_object()) {
        if (outError) *outError = std::string("preset_core missing object field: ") + field;
        return false;
    }
    *outObject = &child->as_object();
    return true;
}

bool GetString(const json_min::Object& object, const char* field, std::string* outValue, std::string* outError) {
    auto it = object.find(field);
    if (it == object.end() || !it->second.is_string()) {
        if (outError) *outError = std::string("preset_core missing string field: ") + field;
        return false;
    }
    *outValue = it->second.as_string();
    return true;
}

bool GetNumber(const json_min::Object& object, const char* field, double* outValue, std::string* outError) {
    auto it = object.find(field);
    if (it == object.end() || !it->second.is_number() || !std::isfinite(it->second.as_number())) {
        if (outError) *outError = std::string("preset_core missing finite numeric field: ") + field;
        return false;
    }
    *outValue = it->second.as_number();
    return true;
}

bool GetOptionalNumber(const json_min::Object& object, const char* field, double* outValue) {
    auto it = object.find(field);
    if (it == object.end() || !it->second.is_number() || !std::isfinite(it->second.as_number())) return false;
    *outValue = it->second.as_number();
    return true;
}

int ClampInt(double value, int lo, int hi) {
    return (std::max)(lo, (std::min)(hi, static_cast<int>(std::lround(value))));
}

void WriteRootField(std::ostringstream& out, const ViewState& view, const KernelParams& params, std::string* outError) {
    ExplainoRootFieldDescriptor desc{};
    std::string error;
    if (!ResolveExplainoRootFieldDescriptor(view, params, &desc, &error)) {
        if (outError) *outError = error;
        out << "\"root_field\":{\"layout\":\"none\",\"source\":\"none\",\"active_count\":0,\"base_roots\":[]}";
        return;
    }

    out << "\"root_field\":{";
    out << "\"layout\":";
    JsonString(out, ExplainoRootFieldLayoutKindId(desc.layout_kind));
    out << ",\"source\":";
    JsonString(out, ExplainoRootFieldSourceKindId(desc.source_kind));
    out << ",\"active_count\":" << desc.active_count;
    out << ",\"base_root_hash\":" << desc.base_root_hash;
    out << ",\"base_roots\":[";
    for (int index = 0; index < desc.active_count; ++index) {
        if (index > 0) out << ',';
        out << '[';
        JsonNumber(out, desc.base_roots[index].x);
        out << ',';
        JsonNumber(out, desc.base_roots[index].y);
        out << ']';
    }
    out << "]}";
}

void WriteSourceStack(std::ostringstream& out, const KernelParams& params) {
    out << "\"source_stack\":[";
    const int count = (std::max)(0, (std::min)(params.color_source_stack_count, kColorPipelineMaxSourceStackCount));
    for (int index = 0; index < count; ++index) {
        const ColorPipelineSourceStackEntry& entry = params.color_source_stack[index];
        if (index > 0) out << ',';
        out << "{\"signal\":";
        JsonString(out, ColorSignalId(entry.signal));
        out << ",\"scale\":";
        JsonNumber(out, entry.params.scale);
        out << ",\"bias\":";
        JsonNumber(out, entry.params.bias);
        out << ",\"blend_weight\":";
        JsonNumber(out, entry.params.blend_weight);
        out << ",\"sdf_gate\":";
        JsonString(out, color_pipeline_core::ColorPipelineSdfGateModeId(entry.params.sdf_gate));
        out << ",\"sdf_gate_width_px\":";
        JsonNumber(out, entry.params.sdf_gate_width_px);
        out << ",\"sdf_boundary_width_px\":";
        JsonNumber(out, entry.params.sdf_boundary_width_px);
        out << ",\"sdf_sample_step\":" << entry.params.sdf_sample_step;
        out << ",\"sdf_field_downsample\":" << entry.params.sdf_field_downsample;
        out << '}';
    }
    out << ']';
}

bool ParseRootField(const json_min::Object& object, KernelParams* outParams, std::string* outError) {
    std::string sourceId;
    double activeCountRaw = 0.0;
    if (!GetString(object, "source", &sourceId, outError) ||
        !GetNumber(object, "active_count", &activeCountRaw, outError)) {
        return false;
    }
    ExplainoRootFieldSourceKind source{};
    if (!TryParseExplainoRootFieldSourceKindId(sourceId, &source)) {
        if (outError) *outError = "preset_core root_field source is unknown";
        return false;
    }
    outParams->explaino_root_authority = source == ExplainoRootFieldSourceKind::custom
        ? ExplainoRootAuthority::custom
        : ExplainoRootAuthority::generated;
    const int activeCount = ClampInt(activeCountRaw, 0, 4);
    outParams->explaino_root_count = activeCount;

    auto rootsIt = object.find("base_roots");
    if (rootsIt == object.end() || !rootsIt->second.is_array()) {
        if (outError) *outError = "preset_core root_field missing base_roots";
        return false;
    }
    const json_min::Array& roots = rootsIt->second.as_array();
    if (roots.size() != static_cast<std::size_t>(activeCount)) {
        if (outError) *outError = "preset_core root_field base_roots count mismatch";
        return false;
    }
    for (int index = 0; index < activeCount; ++index) {
        if (!roots[index].is_array() || roots[index].as_array().size() != 2 ||
            !roots[index].as_array()[0].is_number() || !roots[index].as_array()[1].is_number()) {
            if (outError) *outError = "preset_core root_field base_roots entries must be [x,y]";
            return false;
        }
        outParams->explaino_roots[index] = {
            static_cast<float>(roots[index].as_array()[0].as_number()),
            static_cast<float>(roots[index].as_array()[1].as_number())};
    }
    return true;
}

bool ParseSourceStack(const json_min::Object& object, KernelParams* outParams, std::string* outError) {
    auto it = object.find("source_stack");
    if (it == object.end()) return true;
    if (!it->second.is_array()) {
        if (outError) *outError = "preset_core color_pipeline.source_stack must be an array";
        return false;
    }
    const json_min::Array& rows = it->second.as_array();
    if (rows.size() > static_cast<std::size_t>(kColorPipelineMaxSourceStackCount)) {
        if (outError) *outError = "preset_core color_pipeline.source_stack exceeds max rows";
        return false;
    }
    outParams->color_source_stack_count = static_cast<int>(rows.size());
    for (int index = 0; index < outParams->color_source_stack_count; ++index) {
        if (!rows[index].is_object()) {
            if (outError) *outError = "preset_core source_stack rows must be objects";
            return false;
        }
        const json_min::Object& row = rows[index].as_object();
        ColorPipelineSourceStackEntry entry{};
        std::string signalId;
        if (!GetString(row, "signal", &signalId, outError) ||
            !TryParseColorSignalId(signalId, &entry.signal)) {
            if (outError && outError->empty()) *outError = "preset_core source_stack signal is unknown";
            return false;
        }
        double value = 0.0;
        if (GetOptionalNumber(row, "scale", &value)) entry.params.scale = static_cast<float>(value);
        if (GetOptionalNumber(row, "bias", &value)) entry.params.bias = static_cast<float>(value);
        if (GetOptionalNumber(row, "blend_weight", &value)) entry.params.blend_weight = static_cast<float>(value);
        if (GetOptionalNumber(row, "sdf_gate_width_px", &value)) entry.params.sdf_gate_width_px = static_cast<float>(value);
        if (GetOptionalNumber(row, "sdf_boundary_width_px", &value)) entry.params.sdf_boundary_width_px = static_cast<float>(value);
        if (GetOptionalNumber(row, "sdf_sample_step", &value)) entry.params.sdf_sample_step = ClampInt(value, 1, 16);
        if (GetOptionalNumber(row, "sdf_field_downsample", &value)) entry.params.sdf_field_downsample = ClampInt(value, 1, 16);
        std::string gateId;
        if (GetString(row, "sdf_gate", &gateId, outError) &&
            !color_pipeline_core::TryParseColorPipelineSdfGateModeId(gateId, &entry.params.sdf_gate)) {
            if (outError) *outError = "preset_core source_stack sdf_gate is unknown";
            return false;
        }
        outParams->color_source_stack[index] = entry;
    }
    return true;
}

bool ParseAa(const json_min::Object& root, RenderSettings* outRender, std::string* outError) {
    if (!outRender) return true;
    auto it = root.find("aa");
    if (it == root.end()) return true;
    if (!it->second.is_object()) {
        if (outError) *outError = "preset_core aa must be an object";
        return false;
    }
    const json_min::Object& aa = it->second.as_object();
    std::string modeId;
    if (!GetString(aa, "mode", &modeId, outError)) return false;
    RenderAntiAliasingMode mode{};
    if (!TryParseRenderAntiAliasingModeId(modeId, &mode)) {
        if (outError) *outError = "preset_core aa mode is unknown";
        return false;
    }
    outRender->aa_mode = mode;
    return true;
}

} // namespace

std::string BuildFractalPresetCoreJson(
    const ViewState& view,
    const KernelParams& params,
    const LensSettings& lens,
    std::string* outError) {
    RenderSettings render{};
    return BuildFractalPresetCoreJson(view, params, lens, render, outError);
}

std::string BuildFractalPresetCoreJson(
    const ViewState& view,
    const KernelParams& params,
    const LensSettings& lens,
    const RenderSettings& render,
    std::string* outError) {
    if (outError) outError->clear();
    std::ostringstream out;
    out << "{\"schema_id\":\"viewer.preset_core.v1\",\"version\":1";
    out << ",\"fractal_type\":";
    JsonString(out, FractalTypeId(view.fractal_type));
    out << ",\"explaino_seed\":";
    JsonNumber(out, params.explaino_seed);
    out << ",\"explaino_phase\":";
    JsonNumber(out, view.explaino_phase);
    out << ',';
    WriteRootField(out, view, params, outError);
    out << ",\"root_sdf\":{";
    out << "\"radius\":";
    JsonNumber(out, params.explaino_root_sdf_radius);
    out << ",\"bridge_width\":";
    JsonNumber(out, params.explaino_root_sdf_bridge_width);
    out << ",\"smooth_blend\":";
    JsonNumber(out, params.explaino_root_sdf_smooth_blend);
    out << ",\"h_source\":";
    JsonString(out, ExplainoRootSdfHSourceId(params.explaino_root_sdf_h_source));
    out << ",\"h_amplitude\":";
    JsonNumber(out, params.explaino_root_sdf_h_amplitude);
    out << ",\"h_frequency\":";
    JsonNumber(out, params.explaino_root_sdf_h_frequency);
    out << '}';
    out << ",\"sdf_field\":{\"lens_downsample\":" << lens.downsample << '}';
    out << ",\"color_pipeline\":{";
    out << "\"signal\":";
    JsonString(out, ColorSignalId(params.color_pipeline.signal));
    out << ",\"shape\":";
    JsonString(out, ColorPipelineShapeId(params.color_shape));
    out << ",\"palette\":";
    JsonString(out, ColorPaletteId(params.color_pipeline.palette));
    out << ",\"grading\":";
    JsonString(out, ColorGradingPresetId(params.color_pipeline.grading));
    out << ',';
    WriteSourceStack(out, params);
    out << '}';
    out << ",\"aa\":{\"mode\":";
    JsonString(out, RenderAntiAliasingModeId(render.aa_mode));
    out << '}';
    out << '}';
    return out.str();
}

bool ApplyFractalPresetCoreJson(
    std::string_view json,
    ViewState* outView,
    KernelParams* outParams,
    LensSettings* outLens,
    std::string* outError) {
    return ApplyFractalPresetCoreJson(json, outView, outParams, outLens, nullptr, outError);
}

bool ApplyFractalPresetCoreJson(
    std::string_view json,
    ViewState* outView,
    KernelParams* outParams,
    LensSettings* outLens,
    RenderSettings* outRender,
    std::string* outError) {
    if (!outView || !outParams || !outLens) {
        if (outError) *outError = "preset_core apply requires view, params, and lens outputs";
        return false;
    }
    json_min::ParseResult parsed = json_min::Parse(json);
    if (!parsed.error.empty() || !parsed.value.is_object()) {
        if (outError) *outError = parsed.error.empty() ? "preset_core root must be an object" : parsed.error;
        return false;
    }
    const json_min::Object& root = parsed.value.as_object();
    std::string schemaId;
    std::string fractalTypeId;
    if (!GetString(root, "schema_id", &schemaId, outError) ||
        schemaId != "viewer.preset_core.v1" ||
        !GetString(root, "fractal_type", &fractalTypeId, outError) ||
        !TryParseFractalTypeId(fractalTypeId, &outView->fractal_type)) {
        if (outError && outError->empty()) *outError = "preset_core header is invalid";
        return false;
    }
    double value = 0.0;
    if (GetOptionalNumber(root, "explaino_phase", &value)) outView->explaino_phase = static_cast<float>(value);
    if (GetOptionalNumber(root, "explaino_seed", &value)) outParams->explaino_seed = value;

    const json_min::Object* rootField = nullptr;
    if (!GetObject(parsed.value, "root_field", &rootField, outError) ||
        !ParseRootField(*rootField, outParams, outError)) {
        return false;
    }

    const json_min::Object* rootSdf = nullptr;
    if (GetObject(parsed.value, "root_sdf", &rootSdf, outError)) {
        if (GetOptionalNumber(*rootSdf, "radius", &value)) outParams->explaino_root_sdf_radius = static_cast<float>(value);
        if (GetOptionalNumber(*rootSdf, "bridge_width", &value)) outParams->explaino_root_sdf_bridge_width = static_cast<float>(value);
        if (GetOptionalNumber(*rootSdf, "smooth_blend", &value)) outParams->explaino_root_sdf_smooth_blend = static_cast<float>(value);
        if (GetOptionalNumber(*rootSdf, "h_amplitude", &value)) outParams->explaino_root_sdf_h_amplitude = static_cast<float>(value);
        if (GetOptionalNumber(*rootSdf, "h_frequency", &value)) outParams->explaino_root_sdf_h_frequency = static_cast<float>(value);
        std::string hSourceId;
        if (GetString(*rootSdf, "h_source", &hSourceId, outError) &&
            !TryParseExplainoRootSdfHSourceId(hSourceId, &outParams->explaino_root_sdf_h_source)) {
            if (outError) *outError = "preset_core root_sdf h_source is unknown";
            return false;
        }
    } else {
        return false;
    }

    const json_min::Object* sdfField = nullptr;
    if (!GetObject(parsed.value, "sdf_field", &sdfField, outError)) return false;
    if (GetOptionalNumber(*sdfField, "lens_downsample", &value)) outLens->downsample = ClampInt(value, 1, 16);

    const json_min::Object* color = nullptr;
    if (!GetObject(parsed.value, "color_pipeline", &color, outError)) return false;
    std::string id;
    if (!GetString(*color, "signal", &id, outError) || !TryParseColorSignalId(id, &outParams->color_pipeline.signal)) return false;
    if (!GetString(*color, "shape", &id, outError) || !TryParseColorPipelineShapeId(id, &outParams->color_shape)) return false;
    if (!GetString(*color, "palette", &id, outError) || !TryParseColorPaletteId(id, &outParams->color_pipeline.palette)) return false;
    if (!GetString(*color, "grading", &id, outError) || !TryParseColorGradingPresetId(id, &outParams->color_pipeline.grading)) return false;
    if (!ParseSourceStack(*color, outParams, outError)) return false;
    if (!ParseAa(root, outRender, outError)) return false;

    if (UsesExplainoRootLayoutAuthority(outView->fractal_type)) {
        UpdateExplainoPolynomial(*outView, *outParams, nullptr);
    }
    return true;
}
