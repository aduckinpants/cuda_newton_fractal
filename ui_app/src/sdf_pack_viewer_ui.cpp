#include "sdf_pack_viewer_ui.h"

#include "json_min.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>

#ifndef SDF_PACK_VIEWER_UI_NO_IMGUI
#include "imgui.h"
#include "viewer_ui_automation_report.h"
#endif

namespace {

constexpr int kPreviewWidth = 32;
constexpr int kPreviewHeight = 32;
constexpr const char* kResetDefaultsControlId = "sdf_pack.reset_defaults";
constexpr const char* kUseAsSdfFieldSourceControlId = "sdf_pack.use_as_sdf_field_source.primary";

std::uint64_t Fnv1aAppend(std::uint64_t hash, std::uint64_t value) {
    for (int byteIndex = 0; byteIndex < 8; ++byteIndex) {
        const auto byte = static_cast<unsigned char>((value >> (byteIndex * 8)) & 0xffu);
        hash ^= static_cast<std::uint64_t>(byte);
        hash *= 1099511628211ull;
    }
    return hash;
}

std::uint64_t StableDoubleBits(double value) {
    if (value == 0.0) {
        value = 0.0;
    }
    if (std::isnan(value)) {
        return 0x7ff8000000000000ull;
    }
    std::uint64_t bits = 0;
    static_assert(sizeof(bits) == sizeof(value), "double bit hash layout mismatch");
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}

std::string FormatHash(std::uint64_t hash) {
    std::ostringstream out;
    out << "fnv1a64:" << std::hex << std::setw(16) << std::setfill('0') << hash;
    return out.str();
}

void WriteJsonEscapedString(std::ostringstream& out, const std::string& text) {
    out << '"';
    for (char ch : text) {
        switch (ch) {
        case '\\': out << "\\\\"; break;
        case '"': out << "\\\""; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default: out << ch; break;
        }
    }
    out << '"';
}

const SdfPackParam* FindParam(const SdfPack& pack, const std::string& id) {
    for (const SdfPackParam& param : pack.params) {
        if (param.id == id) {
            return &param;
        }
    }
    return nullptr;
}

const SdfPackControl* FindControlByAutomationId(const SdfPack& pack, const std::string& controlId) {
    for (const SdfPackControl& control : pack.controls) {
        if (SdfPackViewerControlAutomationId(control) == controlId) {
            return &control;
        }
    }
    return nullptr;
}

double CurrentControlValue(const SdfPackControl& control, const SdfPackViewerState& state) {
    const auto found = state.params.find(control.param);
    if (found != state.params.end()) {
        return found->second;
    }
    const SdfPackParam* param = FindParam(state.pack, control.param);
    return param ? param->default_value : 0.0;
}

bool InitializeStateFromPack(
    SdfPackViewerState* ioState,
    SdfPack pack,
    const std::string& sourcePath,
    const std::string& jsonText,
    std::string* outError) {
    std::map<std::string, double> values;
    if (!BuildSdfPackParamValues(pack, {}, &values, outError)) {
        return false;
    }
    ioState->pack = std::move(pack);
    ioState->params = std::move(values);
    ioState->pack_path = sourcePath;
    ioState->pack_json = jsonText;
    ioState->pack_load_error.clear();
    ioState->have_pack = true;
    ioState->initialized = true;
    ioState->preview_dirty = true;
    ioState->last_preview = {};
    return true;
}

std::string HashField(const SdfFieldResult& field) {
    std::uint64_t hash = 1469598103934665603ull;
    hash = Fnv1aAppend(hash, static_cast<std::uint64_t>(static_cast<std::uint32_t>(field.width)));
    hash = Fnv1aAppend(hash, static_cast<std::uint64_t>(static_cast<std::uint32_t>(field.height)));
    hash = Fnv1aAppend(hash, StableDoubleBits(field.pixel_scale));
    for (float value : field.signed_distance_px) {
        hash = Fnv1aAppend(hash, StableDoubleBits(static_cast<double>(value)));
    }
    return FormatHash(hash);
}

SdfPackViewerPreviewSummary SummarizeField(
    const SdfFieldResult& field,
    const SdfPackFieldReport& report) {
    SdfPackViewerPreviewSummary summary;
    summary.ok = true;
    summary.backend_used = SdfPackFieldBackendId(report.used);
    summary.backend_fallback_used = report.fallback_used;
    summary.width = field.width;
    summary.height = field.height;
    summary.pixel_scale = field.pixel_scale;
    summary.sample_count = field.signed_distance_px.size();
    if (!field.signed_distance_px.empty()) {
        double minValue = field.signed_distance_px.front();
        double maxValue = field.signed_distance_px.front();
        double sum = 0.0;
        for (float value : field.signed_distance_px) {
            minValue = (std::min)(minValue, static_cast<double>(value));
            maxValue = (std::max)(maxValue, static_cast<double>(value));
            sum += static_cast<double>(value);
        }
        summary.min_signed_distance_px = minValue;
        summary.max_signed_distance_px = maxValue;
        summary.mean_signed_distance_px = sum / static_cast<double>(field.signed_distance_px.size());
    }
    summary.field_hash = HashField(field);
    return summary;
}

bool ApplyParamValues(
    SdfPackViewerState* ioState,
    const std::map<std::string, double>& values,
    std::string* outError) {
    std::map<std::string, double> normalized;
    if (!BuildSdfPackParamValues(ioState->pack, values, &normalized, outError)) {
        return false;
    }
    ioState->params = std::move(normalized);
    ioState->preview_dirty = true;
    return true;
}

bool TryGetObject(const json_min::Value& object, const char* key, const json_min::Value** outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value) {
        *outValue = nullptr;
        return true;
    }
    if (!value->is_object()) {
        if (outError) *outError = std::string(key) + " must be an object";
        return false;
    }
    *outValue = value;
    return true;
}

bool TryGetString(const json_min::Value& object, const char* key, std::string* out, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value) {
        return true;
    }
    if (!value->is_string()) {
        if (outError) *outError = std::string(key) + " must be a string";
        return false;
    }
    *out = value->as_string();
    return true;
}

bool TryGetBool(const json_min::Value& object, const char* key, bool* out, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value) {
        return true;
    }
    if (!value->is_bool()) {
        if (outError) *outError = std::string(key) + " must be a bool";
        return false;
    }
    *out = value->as_bool();
    return true;
}

bool ApplyStateJsonParams(
    const json_min::Value* paramsObject,
    SdfPackViewerState* outState,
    std::string* outError) {
    if (!paramsObject || !outState->have_pack) {
        return true;
    }
    std::map<std::string, double> next = outState->params;
    for (const auto& entry : paramsObject->as_object()) {
        if (!entry.second.is_number()) {
            if (outError) *outError = "sdf_pack.params values must be numbers";
            return false;
        }
        next[entry.first] = entry.second.as_number();
    }
    return ApplyParamValues(outState, next, outError);
}

#ifndef SDF_PACK_VIEWER_UI_NO_IMGUI
void NoteSdfPackUiAutomationRect(
    std::vector<ViewerUiAutomationRect>* automationRects,
    const std::string& controlId) {
    if (!automationRects || controlId.empty()) {
        return;
    }
    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    if (max.x <= min.x || max.y <= min.y) {
        return;
    }
    ViewerUiAutomationRect rect;
    rect.control_id = controlId;
    rect.client_left = static_cast<int>(std::lround(min.x));
    rect.client_top = static_cast<int>(std::lround(min.y));
    rect.client_right = static_cast<int>(std::lround(max.x));
    rect.client_bottom = static_cast<int>(std::lround(max.y));
    automationRects->push_back(std::move(rect));
}

bool ConsumeSetValueIfRequested(
    SdfPackViewerState* ioState,
    const std::string& controlId,
    SdfPackViewerSetValueAutomation* setValueAutomation) {
    if (!ioState ||
        !setValueAutomation ||
        !setValueAutomation->control_id ||
        setValueAutomation->control_id->empty() ||
        (setValueAutomation->consumed && *setValueAutomation->consumed) ||
        *setValueAutomation->control_id != controlId) {
        return false;
    }
    std::string error;
    const bool applied = SetSdfPackViewerControlValue(ioState, controlId, setValueAutomation->value, &error);
    if (setValueAutomation->consumed) {
        *setValueAutomation->consumed = applied;
    }
    if (setValueAutomation->error) {
        *setValueAutomation->error = applied ? std::string() : error;
    }
    return applied;
}

bool ConsumeResetClickIfRequested(
    SdfPackViewerState* ioState,
    const std::string& controlId,
    SdfPackViewerClickAutomation* clickAutomation) {
    if (!ioState ||
        !clickAutomation ||
        !clickAutomation->control_id ||
        clickAutomation->control_id->empty() ||
        (clickAutomation->consumed && *clickAutomation->consumed) ||
        *clickAutomation->control_id != controlId) {
        return false;
    }
    std::string error;
    const bool applied = ResetSdfPackViewerControlsToDefaults(ioState, &error);
    if (clickAutomation->consumed) {
        *clickAutomation->consumed = applied;
    }
    return applied;
}
#endif

} // namespace

std::string SdfPackViewerControlAutomationId(const SdfPackControl& control) {
    return std::string("sdf_pack.") + control.param + ".primary";
}

std::string SdfPackViewerResetDefaultsAutomationId() {
    return kResetDefaultsControlId;
}

std::string SdfPackViewerUseAsSdfFieldSourceAutomationId() {
    return kUseAsSdfFieldSourceControlId;
}

bool SdfPackViewerWantsSetValueControl(const std::string& controlId) {
    return controlId.rfind("sdf_pack.", 0) == 0 && controlId != kResetDefaultsControlId;
}

bool SdfPackViewerWantsClickControl(const std::string& controlId) {
    return controlId == kResetDefaultsControlId;
}

const char* SdfPackViewerBackendPreferenceId(SdfPackFieldBackend backend) {
    return SdfPackFieldBackendId(backend);
}

bool TryParseSdfPackViewerBackendPreferenceId(const std::string& id, SdfPackFieldBackend* outBackend) {
    if (id == "cpu_reference") {
        if (outBackend) *outBackend = SdfPackFieldBackend::cpu_reference;
        return true;
    }
    if (id == "cuda_sample") {
        if (outBackend) *outBackend = SdfPackFieldBackend::cuda_sample;
        return true;
    }
    if (id == "auto") {
        if (outBackend) *outBackend = SdfPackFieldBackend::auto_backend;
        return true;
    }
    return false;
}

bool LoadSdfPackViewerJson(
    SdfPackViewerState* ioState,
    const std::string& jsonText,
    const std::string& sourcePath,
    std::string* outError) {
    if (!ioState) {
        if (outError) *outError = "SDF pack viewer state is required";
        return false;
    }
    SdfPackParseResult parsed = ParseSdfPackJson(jsonText);
    if (!parsed.ok) {
        ioState->pack_load_error = parsed.error;
        ioState->have_pack = false;
        ioState->initialized = true;
        if (outError) *outError = parsed.error;
        return false;
    }
    if (!InitializeStateFromPack(ioState, std::move(parsed.pack), sourcePath, jsonText, outError)) {
        ioState->pack_load_error = outError ? *outError : "failed to initialize SDF pack viewer state";
        ioState->have_pack = false;
        ioState->initialized = true;
        return false;
    }
    if (outError) outError->clear();
    return true;
}

bool LoadSdfPackViewerPack(
    SdfPackViewerState* ioState,
    const std::string& packPath,
    std::string* outError) {
    std::ifstream in(packPath, std::ios::binary);
    if (!in) {
        const std::string error = "unable to open SDF pack: " + packPath;
        if (ioState) {
            ioState->pack_load_error = error;
            ioState->initialized = true;
        }
        if (outError) *outError = error;
        return false;
    }
    std::ostringstream buffer;
    buffer << in.rdbuf();
    return LoadSdfPackViewerJson(ioState, buffer.str(), packPath, outError);
}

bool SetSdfPackViewerControlValue(
    SdfPackViewerState* ioState,
    const std::string& controlId,
    double value,
    std::string* outError) {
    if (!ioState || !ioState->have_pack) {
        if (outError) *outError = "no SDF pack loaded";
        return false;
    }
    if (!std::isfinite(value)) {
        if (outError) *outError = "SDF pack control value must be finite";
        return false;
    }
    if (controlId == kUseAsSdfFieldSourceControlId) {
        ioState->use_as_sdf_field_source = value >= 0.5;
        if (outError) outError->clear();
        return true;
    }
    const SdfPackControl* control = FindControlByAutomationId(ioState->pack, controlId);
    if (!control) {
        if (outError) *outError = "unknown SDF pack control: " + controlId;
        return false;
    }
    std::map<std::string, double> next = ioState->params;
    next[control->param] = value;
    if (!ApplyParamValues(ioState, next, outError)) {
        return false;
    }
    if (outError) outError->clear();
    return true;
}

bool ResetSdfPackViewerControlsToDefaults(SdfPackViewerState* ioState, std::string* outError) {
    if (!ioState || !ioState->have_pack) {
        if (outError) *outError = "no SDF pack loaded";
        return false;
    }
    std::map<std::string, double> next = ioState->params;
    for (const SdfPackControl& control : ioState->pack.controls) {
        const SdfPackParam* param = FindParam(ioState->pack, control.param);
        if (param) {
            next[control.param] = param->default_value;
        }
    }
    if (!ApplyParamValues(ioState, next, outError)) {
        return false;
    }
    if (outError) outError->clear();
    return true;
}

bool RunSdfPackViewerPreview(SdfPackViewerState* ioState, std::string* outError) {
    if (!ioState || !ioState->have_pack) {
        if (outError) *outError = "no SDF pack loaded";
        return false;
    }
    SdfPackFieldRequest request;
    request.pack = &ioState->pack;
    request.overrides = ioState->params;
    request.width = kPreviewWidth;
    request.height = kPreviewHeight;

    SdfFieldResult field;
    SdfPackFieldReport report;
    std::string error;
    if (!ComputeSdfPackFieldWithBackend(request, ioState->backend_preference, field, &report, &error)) {
        ioState->last_preview = {};
        ioState->last_preview.error = error;
        ioState->preview_dirty = false;
        if (outError) *outError = error;
        return false;
    }
    ioState->last_preview = SummarizeField(field, report);
    ioState->preview_dirty = false;
    if (outError) outError->clear();
    return true;
}

SdfPackViewerAutomationReport BuildSdfPackViewerAutomationReport(const SdfPackViewerState& state) {
    SdfPackViewerAutomationReport report;
    report.panel_open = state.open;
    report.initialized = state.initialized;
    report.force_open_for_automation = state.force_open_for_automation;
    report.have_pack = state.have_pack;
    report.use_as_sdf_field_source = state.use_as_sdf_field_source;
    report.pack_path = state.pack_path;
    report.pack_load_error = state.pack_load_error;
    report.backend_preference = SdfPackViewerBackendPreferenceId(state.backend_preference);
    if (state.have_pack) {
        report.pack_id = state.pack.pack_id;
        report.pack_name = state.pack.name;
        for (const SdfPackControl& control : state.pack.controls) {
            const SdfPackParam* param = FindParam(state.pack, control.param);
            SdfPackViewerControlReport controlReport;
            controlReport.control_id = SdfPackViewerControlAutomationId(control);
            controlReport.param = control.param;
            controlReport.label = control.label.empty() ? control.param : control.label;
            controlReport.value = CurrentControlValue(control, state);
            controlReport.has_min = control.has_ui_min || param != nullptr;
            controlReport.has_max = control.has_ui_max || param != nullptr;
            controlReport.has_default_value = param != nullptr;
            controlReport.min_value = control.has_ui_min ? control.ui_min : (param ? param->min_value : 0.0);
            controlReport.max_value = control.has_ui_max ? control.ui_max : (param ? param->max_value : 0.0);
            controlReport.default_value = param ? param->default_value : 0.0;
            report.controls.push_back(std::move(controlReport));
        }
    }
    report.preview_ok = state.last_preview.ok;
    report.preview_error = state.last_preview.error;
    report.preview_backend_used = state.last_preview.backend_used;
    report.preview_backend_fallback_used = state.last_preview.backend_fallback_used;
    report.preview_width = state.last_preview.width;
    report.preview_height = state.last_preview.height;
    report.preview_pixel_scale = state.last_preview.pixel_scale;
    report.preview_sample_count = state.last_preview.sample_count;
    report.preview_min_signed_distance_px = state.last_preview.min_signed_distance_px;
    report.preview_max_signed_distance_px = state.last_preview.max_signed_distance_px;
    report.preview_mean_signed_distance_px = state.last_preview.mean_signed_distance_px;
    report.preview_field_hash = state.last_preview.field_hash;
    return report;
}

std::string SerializeSdfPackViewerStateJson(const SdfPackViewerState& state) {
    std::ostringstream js;
    js << std::setprecision(std::numeric_limits<double>::max_digits10);
    js << "{\n";
    js << "  \"sdf_pack\": {\n";
    js << "    \"open\": " << (state.open ? "true" : "false") << ",\n";
    js << "    \"use_as_sdf_field_source\": " << (state.use_as_sdf_field_source ? "true" : "false") << ",\n";
    js << "    \"pack_id\": ";
    WriteJsonEscapedString(js, state.have_pack ? state.pack.pack_id : std::string());
    js << ",\n";
    js << "    \"pack_path\": ";
    WriteJsonEscapedString(js, state.pack_path);
    js << ",\n";
    js << "    \"pack_json\": ";
    WriteJsonEscapedString(js, state.pack_json);
    js << ",\n";
    js << "    \"backend_preference\": ";
    WriteJsonEscapedString(js, SdfPackViewerBackendPreferenceId(state.backend_preference));
    js << ",\n";
    js << "    \"params\": {\n";
    std::size_t index = 0;
    for (const auto& entry : state.params) {
        js << "      ";
        WriteJsonEscapedString(js, entry.first);
        js << ": " << entry.second;
        if (++index < state.params.size()) {
            js << ",";
        }
        js << "\n";
    }
    js << "    }\n";
    js << "  }\n";
    js << "}\n";
    return js.str();
}

bool MergeSdfPackViewerStateIntoDiagnosticsStateJson(
    const std::string& diagnosticsStateJson,
    const SdfPackViewerState& state,
    std::string* outMergedJson,
    std::string* outError) {
    if (!outMergedJson) {
        if (outError) *outError = "merged diagnostics state output is required";
        return false;
    }
    if (!state.initialized && !state.have_pack && !state.open) {
        *outMergedJson = diagnosticsStateJson;
        if (outError) outError->clear();
        return true;
    }
    json_min::ParseResult parsed = json_min::Parse(diagnosticsStateJson);
    if (!parsed.error.empty() || !parsed.value.is_object()) {
        if (outError) *outError = parsed.error.empty() ? "diagnostics state JSON must be an object" : parsed.error;
        return false;
    }
    if (parsed.value.get("sdf_pack")) {
        if (outError) *outError = "diagnostics state JSON already contains sdf_pack";
        return false;
    }

    const std::string sdfPackRoot = SerializeSdfPackViewerStateJson(state);
    const std::size_t memberStart = sdfPackRoot.find("\"sdf_pack\"");
    const std::size_t memberEnd = sdfPackRoot.find_last_of('}');
    if (memberStart == std::string::npos || memberEnd == std::string::npos || memberEnd <= memberStart) {
        if (outError) *outError = "failed to build sdf_pack state member";
        return false;
    }
    std::string member = sdfPackRoot.substr(memberStart, memberEnd - memberStart);
    while (!member.empty() && (member.back() == '\r' || member.back() == '\n' || member.back() == ' ' || member.back() == '\t')) {
        member.pop_back();
    }

    std::size_t insertAt = diagnosticsStateJson.find_last_of('}');
    if (insertAt == std::string::npos) {
        if (outError) *outError = "diagnostics state JSON is missing closing object brace";
        return false;
    }
    std::string merged = diagnosticsStateJson;
    merged.insert(insertAt, std::string(",\n  ") + member + "\n");
    *outMergedJson = std::move(merged);
    if (outError) outError->clear();
    return true;
}

bool LoadSdfPackViewerStateJson(const std::string& stateJson, SdfPackViewerState* outState, std::string* outError) {
    if (!outState) {
        if (outError) *outError = "SDF pack viewer state output is required";
        return false;
    }
    *outState = {};
    json_min::ParseResult parsed = json_min::Parse(stateJson);
    if (!parsed.error.empty()) {
        if (outError) *outError = parsed.error;
        return false;
    }
    if (!parsed.value.is_object()) {
        if (outError) *outError = "state JSON must be an object";
        return false;
    }
    const json_min::Value* sdfPackObject = parsed.value.get("sdf_pack");
    if (!sdfPackObject) {
        if (outError) outError->clear();
        return true;
    }
    if (!sdfPackObject->is_object()) {
        if (outError) *outError = "sdf_pack must be an object";
        return false;
    }

    bool open = false;
    bool useAsSdfFieldSource = false;
    std::string packPath;
    std::string packJson;
    std::string backendId = "auto";
    if (!TryGetBool(*sdfPackObject, "open", &open, outError) ||
        !TryGetBool(*sdfPackObject, "use_as_sdf_field_source", &useAsSdfFieldSource, outError) ||
        !TryGetString(*sdfPackObject, "pack_path", &packPath, outError) ||
        !TryGetString(*sdfPackObject, "pack_json", &packJson, outError) ||
        !TryGetString(*sdfPackObject, "backend_preference", &backendId, outError)) {
        return false;
    }
    SdfPackFieldBackend backend = SdfPackFieldBackend::auto_backend;
    if (!TryParseSdfPackViewerBackendPreferenceId(backendId, &backend)) {
        if (outError) *outError = "Unknown sdf_pack.backend_preference: " + backendId;
        return false;
    }

    bool loaded = false;
    if (!packJson.empty()) {
        loaded = LoadSdfPackViewerJson(outState, packJson, packPath.empty() ? "state_json" : packPath, outError);
    } else if (!packPath.empty()) {
        loaded = LoadSdfPackViewerPack(outState, packPath, outError);
    }
    if (!loaded) {
        if (packJson.empty() && packPath.empty()) {
            outState->initialized = true;
        } else {
            return false;
        }
    }

    outState->open = open;
    outState->use_as_sdf_field_source = useAsSdfFieldSource;
    outState->backend_preference = backend;

    const json_min::Value* paramsObject = nullptr;
    if (!TryGetObject(*sdfPackObject, "params", &paramsObject, outError)) {
        return false;
    }
    if (!ApplyStateJsonParams(paramsObject, outState, outError)) {
        return false;
    }

    if (outError) outError->clear();
    return true;
}

#ifndef SDF_PACK_VIEWER_UI_NO_IMGUI
void RenderSdfPackViewerInlinePanel(
    SdfPackViewerState* ioState,
    std::vector<ViewerUiAutomationRect>* automationRects,
    SdfPackViewerSetValueAutomation* setValueAutomation,
    SdfPackViewerClickAutomation* clickAutomation,
    bool* outInteracted) {
    if (!ioState || (!ioState->open && !ioState->force_open_for_automation && !ioState->have_pack)) {
        return;
    }

    ImGui::Separator();
    ImGui::TextUnformatted("SDF Pack");
    if (!ioState->pack_load_error.empty()) {
        ImGui::TextWrapped("%s", ioState->pack_load_error.c_str());
    }
    if (!ioState->have_pack) {
        ImGui::TextUnformatted("No SDF pack loaded.");
        return;
    }

    ImGui::TextUnformatted(ioState->pack.name.c_str());
    ImGui::Text("Pack: %s", ioState->pack.pack_id.c_str());
    bool edited = false;

    bool useAsSource = ioState->use_as_sdf_field_source;
    if (ImGui::Checkbox("Use as SDF Field Source", &useAsSource)) {
        ioState->use_as_sdf_field_source = useAsSource;
        edited = true;
    }
    NoteSdfPackUiAutomationRect(automationRects, kUseAsSdfFieldSourceControlId);
    if (ConsumeSetValueIfRequested(ioState, kUseAsSdfFieldSourceControlId, setValueAutomation)) {
        edited = true;
    }

    for (const SdfPackControl& control : ioState->pack.controls) {
        const double value = CurrentControlValue(control, *ioState);
        const SdfPackParam* param = FindParam(ioState->pack, control.param);
        const float minValue = static_cast<float>(control.has_ui_min ? control.ui_min : (param ? param->min_value : value - 1.0));
        const float maxValue = static_cast<float>(control.has_ui_max ? control.ui_max : (param ? param->max_value : value + 1.0));
        float uiValue = static_cast<float>(value);
        const std::string controlId = SdfPackViewerControlAutomationId(control);
        const std::string label = control.label.empty() ? control.param : control.label;
        ImGui::PushID(control.param.c_str());
        bool changed = false;
        if (maxValue > minValue) {
            changed = ImGui::SliderFloat(label.c_str(), &uiValue, minValue, maxValue);
        } else {
            changed = ImGui::DragFloat(label.c_str(), &uiValue, 0.01f);
        }
        NoteSdfPackUiAutomationRect(automationRects, controlId);
        if (ConsumeSetValueIfRequested(ioState, controlId, setValueAutomation)) {
            edited = true;
        } else if (changed) {
            std::string error;
            if (SetSdfPackViewerControlValue(ioState, controlId, static_cast<double>(uiValue), &error)) {
                edited = true;
            }
        }
        ImGui::PopID();
    }

    if (ImGui::Button("Reset SDF Pack Defaults")) {
        std::string error;
        if (ResetSdfPackViewerControlsToDefaults(ioState, &error)) {
            edited = true;
        }
    }
    NoteSdfPackUiAutomationRect(automationRects, kResetDefaultsControlId);
    if (ConsumeResetClickIfRequested(ioState, kResetDefaultsControlId, clickAutomation)) {
        edited = true;
    }

    if (ioState->preview_dirty) {
        std::string error;
        RunSdfPackViewerPreview(ioState, &error);
    }
    if (ioState->last_preview.ok) {
        ImGui::Text("Preview: %s, %d x %d",
            ioState->last_preview.backend_used.c_str(),
            ioState->last_preview.width,
            ioState->last_preview.height);
        ImGui::Text("Field Hash: %s", ioState->last_preview.field_hash.c_str());
    } else if (!ioState->last_preview.error.empty()) {
        ImGui::TextWrapped("%s", ioState->last_preview.error.c_str());
    }

    if (edited && outInteracted) {
        *outInteracted = true;
    }
}
#endif
