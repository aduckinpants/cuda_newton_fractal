#include "generic_equation_pack_workbench.h"

#include "generic_sample_core.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <limits>
#include <sstream>

#ifndef GENERIC_EQUATION_PACK_WORKBENCH_NO_IMGUI
#include "imgui.h"
#include "viewer_ui_automation_report.h"
#endif

namespace {

constexpr int kDefaultPreviewGridWidth = 32;
constexpr int kDefaultPreviewGridHeight = 32;
constexpr int kMaxPreviewGridDimension = 48;

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

double Saturate(double value) {
    if (!std::isfinite(value)) {
        return 0.0;
    }
    return (std::max)(0.0, (std::min)(1.0, value));
}

std::uint8_t ToByte(double value) {
    return static_cast<std::uint8_t>(std::lround(Saturate(value) * 255.0));
}

std::uint32_t PackRgba(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255) {
    return static_cast<std::uint32_t>(r) |
        (static_cast<std::uint32_t>(g) << 8) |
        (static_cast<std::uint32_t>(b) << 16) |
        (static_cast<std::uint32_t>(a) << 24);
}

double SafeMagnitudeSignal(const GenericSampleResult& result) {
    if (std::isfinite(result.abs2) && result.abs2 >= 0.0) {
        return std::log1p(std::sqrt(result.abs2));
    }
    if (std::isfinite(result.value_x) && std::isfinite(result.value_y)) {
        return std::log1p(std::hypot(result.value_x, result.value_y));
    }
    return 0.0;
}

std::uint32_t PreviewPixelFromResult(const GenericSampleResult& result) {
    if (!std::isfinite(result.value_x) ||
        !std::isfinite(result.value_y) ||
        !std::isfinite(result.abs2)) {
        return PackRgba(190, 64, 220);
    }

    constexpr double kPi = 3.14159265358979323846;
    const double magnitude = Saturate(SafeMagnitudeSignal(result) / 5.0);
    const double iteration = Saturate(static_cast<double>((std::max)(0, result.iterations)) / 128.0);
    const double phase = Saturate((std::atan2(result.value_y, result.value_x) + kPi) / (2.0 * kPi));

    if (result.converged) {
        return PackRgba(
            ToByte(0.15 + 0.55 * phase),
            ToByte(0.55 + 0.35 * (1.0 - magnitude)),
            ToByte(0.20 + 0.55 * magnitude));
    }
    if (result.diverged) {
        return PackRgba(
            ToByte(0.75 + 0.20 * iteration),
            ToByte(0.28 + 0.45 * (1.0 - magnitude)),
            ToByte(0.10 + 0.25 * phase));
    }
    return PackRgba(
        ToByte(0.12 + 0.35 * magnitude),
        ToByte(0.40 + 0.35 * phase),
        ToByte(0.60 + 0.30 * (1.0 - magnitude)));
}

std::string HashPreviewPixels(const std::vector<std::uint32_t>& pixels, int width, int height) {
    std::uint64_t hash = 1469598103934665603ull;
    hash = Fnv1aAppend(hash, static_cast<std::uint64_t>(static_cast<std::uint32_t>(width)));
    hash = Fnv1aAppend(hash, static_cast<std::uint64_t>(static_cast<std::uint32_t>(height)));
    for (std::uint32_t pixel : pixels) {
        hash = Fnv1aAppend(hash, static_cast<std::uint64_t>(pixel));
    }
    return FormatHash(hash);
}

double ClampControlValue(const GenericEquationPackControl& control, double value) {
    if (control.has_min && value < control.min_value) {
        value = control.min_value;
    }
    if (control.has_max && value > control.max_value) {
        value = control.max_value;
    }
    return value;
}

const GenericEquationPackControl* FindControlByAutomationId(
    const GenericEquationPack& pack,
    const std::string& controlId) {
    for (const GenericEquationPackControl& control : pack.controls) {
        if (GenericEquationPackWorkbenchControlAutomationId(control) == controlId) {
            return &control;
        }
    }
    return nullptr;
}

int EffectiveGridWidth(const GenericEquationPackRegion& region) {
    const int source = region.has_region ? region.grid_width : kDefaultPreviewGridWidth;
    return (std::max)(1, (std::min)(source, kMaxPreviewGridDimension));
}

int EffectiveGridHeight(const GenericEquationPackRegion& region) {
    const int source = region.has_region ? region.grid_height : kDefaultPreviewGridHeight;
    return (std::max)(1, (std::min)(source, kMaxPreviewGridDimension));
}

double EffectiveCenterX(const GenericEquationPackRegion& region) {
    return region.has_region ? region.center_x : 0.0;
}

double EffectiveCenterY(const GenericEquationPackRegion& region) {
    return region.has_region ? region.center_y : 0.0;
}

double EffectiveSpanX(const GenericEquationPackRegion& region) {
    return (region.has_region && region.span_x > 0.0) ? region.span_x : 3.0;
}

double EffectiveSpanY(const GenericEquationPackRegion& region) {
    return (region.has_region && region.span_y > 0.0) ? region.span_y : 3.0;
}

std::vector<GFPoint> BuildPreviewGrid(const GenericEquationPackRegion& region) {
    const int width = EffectiveGridWidth(region);
    const int height = EffectiveGridHeight(region);
    const double centerX = EffectiveCenterX(region);
    const double centerY = EffectiveCenterY(region);
    const double spanX = EffectiveSpanX(region);
    const double spanY = EffectiveSpanY(region);

    std::vector<GFPoint> coords;
    coords.reserve(static_cast<std::size_t>(width) * static_cast<std::size_t>(height));
    for (int y = 0; y < height; ++y) {
        const double fy = (height == 1) ? 0.5 : (static_cast<double>(y) + 0.5) / static_cast<double>(height);
        for (int x = 0; x < width; ++x) {
            const double fx = (width == 1) ? 0.5 : (static_cast<double>(x) + 0.5) / static_cast<double>(width);
            coords.push_back({
                centerX + (fx - 0.5) * spanX,
                centerY + (fy - 0.5) * spanY,
            });
        }
    }
    return coords;
}

GenericEquationPackWorkbenchPreviewSummary SummarizeResults(
    const std::vector<GenericSampleResult>& results,
    int width,
    int height) {
    GenericEquationPackWorkbenchPreviewSummary summary;
    summary.ok = true;
    summary.backend_used = "cuda";
    summary.sample_count = static_cast<int>(results.size());
    summary.image_width = width;
    summary.image_height = height;
    summary.pixels_rgba.reserve(results.size());

    double iterationSum = 0.0;
    double abs2Sum = 0.0;
    int finiteAbs2Count = 0;
    std::uint64_t hash = 1469598103934665603ull;

    for (const GenericSampleResult& result : results) {
        iterationSum += static_cast<double>(result.iterations);
        if (std::isfinite(result.abs2)) {
            abs2Sum += result.abs2;
            ++finiteAbs2Count;
        }
        if (!std::isfinite(result.value_x) || !std::isfinite(result.value_y)) {
            ++summary.nonfinite_count;
        } else if (result.converged) {
            ++summary.converged_count;
        } else if (result.diverged) {
            ++summary.escaped_count;
        } else {
            ++summary.bounded_count;
        }

        hash = Fnv1aAppend(hash, StableDoubleBits(result.value_x));
        hash = Fnv1aAppend(hash, StableDoubleBits(result.value_y));
        hash = Fnv1aAppend(hash, StableDoubleBits(result.abs2));
        hash = Fnv1aAppend(hash, StableDoubleBits(result.derivative_x));
        hash = Fnv1aAppend(hash, StableDoubleBits(result.derivative_y));
        hash = Fnv1aAppend(hash, static_cast<std::uint64_t>(static_cast<std::int64_t>(result.iterations)));
        hash = Fnv1aAppend(hash, result.converged ? 1ull : 0ull);
        hash = Fnv1aAppend(hash, result.diverged ? 1ull : 0ull);
        summary.pixels_rgba.push_back(PreviewPixelFromResult(result));
    }

    if (!results.empty()) {
        summary.mean_iterations = iterationSum / static_cast<double>(results.size());
    }
    if (finiteAbs2Count > 0) {
        summary.mean_abs2 = abs2Sum / static_cast<double>(finiteAbs2Count);
    }
    summary.result_hash = FormatHash(hash);
    summary.image_hash = HashPreviewPixels(summary.pixels_rgba, summary.image_width, summary.image_height);
    return summary;
}

void InitializeStateFromPack(
    GenericEquationPackWorkbenchState* ioState,
    GenericEquationPack pack,
    const std::string& sourcePath) {
    ioState->pack = std::move(pack);
    ioState->params = ioState->pack.params;
    for (const GenericEquationPackControl& control : ioState->pack.controls) {
        if (ioState->params.find(control.param) == ioState->params.end() && control.has_default_value) {
            ioState->params[control.param] = control.default_value;
        }
    }
    ioState->pack.params = ioState->params;
    ioState->pack_path = sourcePath;
    ioState->pack_load_error.clear();
    ioState->have_pack = true;
    ioState->initialized = true;
    ioState->preview_dirty = true;
    ioState->last_preview = {};
}

#ifndef GENERIC_EQUATION_PACK_WORKBENCH_NO_IMGUI
void NoteWorkbenchUiAutomationRect(
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
    GenericEquationPackWorkbenchState* ioState,
    const std::string& controlId,
    GenericEquationPackWorkbenchSetValueAutomation* setValueAutomation) {
    if (!ioState ||
        !setValueAutomation ||
        !setValueAutomation->control_id ||
        setValueAutomation->control_id->empty() ||
        (setValueAutomation->consumed && *setValueAutomation->consumed) ||
        *setValueAutomation->control_id != controlId) {
        return false;
    }

    std::string error;
    const bool applied = SetGenericEquationPackWorkbenchControlValue(
        ioState,
        controlId,
        setValueAutomation->value,
        &error);
    if (setValueAutomation->consumed) {
        *setValueAutomation->consumed = applied;
    }
    if (setValueAutomation->error) {
        *setValueAutomation->error = applied ? std::string() : error;
    }
    return applied;
}

ImU32 ImGuiColorFromRgba(std::uint32_t rgba) {
    return IM_COL32(
        static_cast<int>(rgba & 0xffu),
        static_cast<int>((rgba >> 8) & 0xffu),
        static_cast<int>((rgba >> 16) & 0xffu),
        static_cast<int>((rgba >> 24) & 0xffu));
}

void DrawPreviewCanvas(
    const GenericEquationPackWorkbenchPreviewSummary& preview,
    std::vector<ViewerUiAutomationRect>* automationRects) {
    if (preview.image_width <= 0 ||
        preview.image_height <= 0 ||
        preview.pixels_rgba.size() <
            static_cast<std::size_t>(preview.image_width) * static_cast<std::size_t>(preview.image_height)) {
        return;
    }

    const float availableWidth = ImGui::GetContentRegionAvail().x;
    float canvasWidth = availableWidth > 0.0f ? (std::min)(availableWidth, 320.0f) : 260.0f;
    canvasWidth = (std::max)(32.0f, canvasWidth);
    float canvasHeight = canvasWidth *
        static_cast<float>(preview.image_height) / static_cast<float>(preview.image_width);
    if (canvasHeight > 240.0f) {
        const float scale = 240.0f / canvasHeight;
        canvasWidth *= scale;
        canvasHeight = 240.0f;
    }

    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const ImVec2 size(canvasWidth, canvasHeight);
    ImGui::InvisibleButton("##equation_pack_preview_canvas", size);
    NoteWorkbenchUiAutomationRect(automationRects, "equation_pack.preview_canvas");

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec2 rectMax(origin.x + canvasWidth, origin.y + canvasHeight);
    drawList->AddRectFilled(origin, rectMax, IM_COL32(14, 16, 22, 255));

    for (int y = 0; y < preview.image_height; ++y) {
        const float y0 = origin.y + canvasHeight * static_cast<float>(y) / static_cast<float>(preview.image_height);
        const float y1 = origin.y + canvasHeight * static_cast<float>(y + 1) / static_cast<float>(preview.image_height);
        for (int x = 0; x < preview.image_width; ++x) {
            const float x0 = origin.x + canvasWidth * static_cast<float>(x) / static_cast<float>(preview.image_width);
            const float x1 = origin.x + canvasWidth * static_cast<float>(x + 1) / static_cast<float>(preview.image_width);
            const std::uint32_t pixel = preview.pixels_rgba[
                static_cast<std::size_t>(y) * static_cast<std::size_t>(preview.image_width) +
                static_cast<std::size_t>(x)];
            drawList->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), ImGuiColorFromRgba(pixel));
        }
    }
    drawList->AddRect(origin, rectMax, IM_COL32(220, 226, 236, 180));
}
#endif

} // namespace

std::string GenericEquationPackWorkbenchControlAutomationId(const GenericEquationPackControl& control) {
    return std::string("equation_pack.") + control.id + ".primary";
}

bool GenericEquationPackWorkbenchWantsSetValueControl(const std::string& controlId) {
    return controlId.rfind("equation_pack.", 0) == 0;
}

bool LoadGenericEquationPackWorkbenchJson(
    GenericEquationPackWorkbenchState* ioState,
    const std::string& jsonText,
    const std::string& sourcePath,
    std::string* outError) {
    if (!ioState) {
        if (outError) *outError = "workbench state is required";
        return false;
    }
    GenericEquationPackParseResult parsed = ParseGenericEquationPackJson(jsonText);
    if (!parsed.ok) {
        ioState->pack_load_error = parsed.error;
        ioState->have_pack = false;
        ioState->initialized = true;
        if (outError) *outError = parsed.error;
        return false;
    }
    InitializeStateFromPack(ioState, std::move(parsed.pack), sourcePath);
    if (outError) outError->clear();
    return true;
}

bool LoadGenericEquationPackWorkbenchPack(
    GenericEquationPackWorkbenchState* ioState,
    const std::string& packPath,
    std::string* outError) {
    std::ifstream in(packPath, std::ios::binary);
    if (!in) {
        const std::string error = "unable to open equation pack: " + packPath;
        if (ioState) {
            ioState->pack_load_error = error;
            ioState->initialized = true;
        }
        if (outError) *outError = error;
        return false;
    }
    std::ostringstream buffer;
    buffer << in.rdbuf();
    return LoadGenericEquationPackWorkbenchJson(ioState, buffer.str(), packPath, outError);
}

bool SetGenericEquationPackWorkbenchControlValue(
    GenericEquationPackWorkbenchState* ioState,
    const std::string& controlId,
    double value,
    std::string* outError) {
    if (!ioState || !ioState->have_pack) {
        if (outError) *outError = "no equation pack loaded";
        return false;
    }
    if (!std::isfinite(value)) {
        if (outError) *outError = "equation pack control value must be finite";
        return false;
    }
    const GenericEquationPackControl* control = FindControlByAutomationId(ioState->pack, controlId);
    if (!control) {
        if (outError) *outError = "unknown equation pack control: " + controlId;
        return false;
    }
    if (ioState->params.find(control->param) == ioState->params.end() && !control->has_default_value) {
        if (outError) *outError = "equation pack control param is missing: " + control->param;
        return false;
    }

    ioState->params[control->param] = ClampControlValue(*control, value);
    ioState->pack.params = ioState->params;
    ioState->preview_dirty = true;
    if (outError) outError->clear();
    return true;
}

bool RunGenericEquationPackWorkbenchPreview(
    GenericEquationPackWorkbenchState* ioState,
    std::string* outError) {
    if (!ioState || !ioState->have_pack) {
        if (outError) *outError = "no equation pack loaded";
        return false;
    }
    ioState->pack.params = ioState->params;
    GenericEquationLowerResult lowered = LowerGenericEquationPackToDesc(ioState->pack);
    if (!lowered.ok) {
        ioState->last_preview = {};
        ioState->last_preview.error = "AST lower error: " + lowered.error;
        ioState->preview_dirty = false;
        if (outError) *outError = ioState->last_preview.error;
        return false;
    }

    const int previewWidth = EffectiveGridWidth(ioState->pack.region);
    const int previewHeight = EffectiveGridHeight(ioState->pack.region);
    std::vector<GFPoint> coords = BuildPreviewGrid(ioState->pack.region);
    std::vector<GenericSampleResult> results(coords.size());
    const char* rawError = nullptr;
    if (!SampleGenericFunction(
            coords.data(),
            static_cast<int>(coords.size()),
            lowered.desc,
            ioState->pack.epsilon,
            ioState->pack.escape_radius,
            results.data(),
            &rawError)) {
        ioState->last_preview = {};
        ioState->last_preview.error = rawError ? rawError : "CUDA generic sample execution failed";
        ioState->preview_dirty = false;
        if (outError) *outError = ioState->last_preview.error;
        return false;
    }

    ioState->last_preview = SummarizeResults(results, previewWidth, previewHeight);
    ioState->preview_dirty = false;
    if (outError) outError->clear();
    return true;
}

GenericEquationPackWorkbenchAutomationReport BuildGenericEquationPackWorkbenchAutomationReport(
    const GenericEquationPackWorkbenchState& state) {
    GenericEquationPackWorkbenchAutomationReport report;
    report.window_open = state.open;
    report.initialized = state.initialized;
    report.force_open_for_automation = state.force_open_for_automation;
    report.have_pack = state.have_pack;
    report.pack_path = state.pack_path;
    if (state.have_pack) {
        report.pack_id = state.pack.pack_id;
        report.pack_name = state.pack.name;
    }
    report.preview_ok = state.last_preview.ok;
    report.preview_error = state.last_preview.error;
    report.preview_backend_used = state.last_preview.backend_used;
    report.preview_sample_count = state.last_preview.sample_count;
    report.preview_converged_count = state.last_preview.converged_count;
    report.preview_escaped_count = state.last_preview.escaped_count;
    report.preview_bounded_count = state.last_preview.bounded_count;
    report.preview_nonfinite_count = state.last_preview.nonfinite_count;
    report.preview_mean_iterations = state.last_preview.mean_iterations;
    report.preview_mean_abs2 = state.last_preview.mean_abs2;
    report.preview_result_hash = state.last_preview.result_hash;
    report.preview_image_width = state.last_preview.image_width;
    report.preview_image_height = state.last_preview.image_height;
    report.preview_image_hash = state.last_preview.image_hash;
    return report;
}

#ifndef GENERIC_EQUATION_PACK_WORKBENCH_NO_IMGUI
void RenderGenericEquationPackWorkbench(
    GenericEquationPackWorkbenchState* ioState,
    std::vector<ViewerUiAutomationRect>* automationRects,
    GenericEquationPackWorkbenchSetValueAutomation* setValueAutomation,
    bool* outInteracted) {
    if (!ioState || !ioState->open) {
        return;
    }

    if (!ImGui::Begin("Equation Pack Workbench", &ioState->open)) {
        ImGui::End();
        return;
    }

    if (!ioState->pack_load_error.empty()) {
        ImGui::TextWrapped("%s", ioState->pack_load_error.c_str());
    }

    if (ioState->have_pack) {
        ImGui::TextUnformatted(ioState->pack.name.c_str());
        ImGui::Text("Pack: %s", ioState->pack.pack_id.c_str());

        bool edited = false;
        for (const GenericEquationPackControl& control : ioState->pack.controls) {
            double value = control.has_default_value ? control.default_value : 0.0;
            auto valueIt = ioState->params.find(control.param);
            if (valueIt != ioState->params.end()) {
                value = valueIt->second;
            }

            const std::string controlId = GenericEquationPackWorkbenchControlAutomationId(control);
            const std::string label = control.label.empty() ? control.id : control.label;
            const float minValue = static_cast<float>(control.has_min ? control.min_value : value - 1.0);
            const float maxValue = static_cast<float>(control.has_max ? control.max_value : value + 1.0);
            float uiValue = static_cast<float>(value);
            ImGui::PushID(control.id.c_str());
            bool changed = false;
            if (control.has_min && control.has_max && control.max_value > control.min_value) {
                changed = ImGui::SliderFloat(label.c_str(), &uiValue, minValue, maxValue);
            } else {
                const float speed = static_cast<float>(control.has_step ? control.step_value : 0.01);
                changed = ImGui::DragFloat(label.c_str(), &uiValue, speed);
            }
            NoteWorkbenchUiAutomationRect(automationRects, controlId);
            if (ConsumeSetValueIfRequested(ioState, controlId, setValueAutomation)) {
                edited = true;
            } else if (changed) {
                std::string error;
                if (SetGenericEquationPackWorkbenchControlValue(ioState, controlId, static_cast<double>(uiValue), &error)) {
                    edited = true;
                }
            }
            ImGui::PopID();
        }

        if (edited) {
            if (outInteracted) {
                *outInteracted = true;
            }
        }

        if (ioState->preview_dirty) {
            std::string error;
            RunGenericEquationPackWorkbenchPreview(ioState, &error);
        }

        if (ImGui::Button("Run Preview")) {
            std::string error;
            RunGenericEquationPackWorkbenchPreview(ioState, &error);
            if (outInteracted) {
                *outInteracted = true;
            }
        }
        const GenericEquationPackWorkbenchPreviewSummary& preview = ioState->last_preview;
        if (preview.ok) {
            ImGui::Text("Preview: %s, %d samples, mean iter %.2f",
                preview.backend_used.c_str(),
                preview.sample_count,
                preview.mean_iterations);
            DrawPreviewCanvas(preview, automationRects);
            ImGui::Text("Result Hash: %s", preview.result_hash.c_str());
            ImGui::Text("Image Hash: %s", preview.image_hash.c_str());
        } else if (!preview.error.empty()) {
            ImGui::TextWrapped("%s", preview.error.c_str());
        }
    }

    ImGui::End();
}
#endif
