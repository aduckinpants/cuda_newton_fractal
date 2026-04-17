#include "runtime_walk_bootstrap.h"

#include "explaino_seed.h"
#include "enum_id_utils.h"
#include "fractal_derived_fields.h"
#include "fractal_family_rules.h"
#include "json_min.h"
#include "view_hp_sync.h"

#include <Windows.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <map>
#include <sstream>
#include <string_view>

namespace {

constexpr std::string_view kDefaultMappingProfileRelativePath = "ui/runtime_walk_fits_mapping_profiles_v1.json";
constexpr std::string_view kOrientationExtractorRelativePath = "tools/runtime_walk_extract_fits_orientation.py";

bool ReadTextFile(const std::string& path, std::string* outText, std::string* outError) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        if (outError) *outError = "Failed to open runtime-walk bootstrap file: " + path;
        return false;
    }
    std::ostringstream text;
    text << file.rdbuf();
    if (!file.good() && !file.eof()) {
        if (outError) *outError = "Failed to read runtime-walk bootstrap file: " + path;
        return false;
    }
    if (outText) *outText = text.str();
    return true;
}

bool WriteTextFile(const std::string& path, const std::string& text, std::string* outError) {
    std::filesystem::path outputPath(path);
    std::error_code ec;
    std::filesystem::create_directories(outputPath.parent_path(), ec);
    if (ec) {
        if (outError) *outError = "Failed to create output directory: " + outputPath.parent_path().string();
        return false;
    }
    std::ofstream file(outputPath, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file) {
        if (outError) *outError = "Failed to open output file: " + outputPath.string();
        return false;
    }
    file.write(text.data(), static_cast<std::streamsize>(text.size()));
    if (!file.good()) {
        if (outError) *outError = "Failed to write output file: " + outputPath.string();
        return false;
    }
    return true;
}

bool GetRequiredArray(const json_min::Value& object, const char* key, const json_min::Value** outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_array()) {
        if (outError) *outError = std::string("Missing or invalid array field: ") + key;
        return false;
    }
    if (outValue) *outValue = value;
    return true;
}

bool GetRequiredString(const json_min::Value& object, const char* key, std::string* outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_string()) {
        if (outError) *outError = std::string("Missing or invalid string field: ") + key;
        return false;
    }
    if (outValue) *outValue = value->as_string();
    return true;
}

bool GetOptionalBool(const json_min::Value& object, const char* key, bool* outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value) return true;
    if (!value->is_bool()) {
        if (outError) *outError = std::string("Invalid bool field: ") + key;
        return false;
    }
    if (outValue) *outValue = value->as_bool();
    return true;
}

bool GetOptionalNumber(const json_min::Value& object, const char* key, double* outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value) return true;
    if (!value->is_number()) {
        if (outError) *outError = std::string("Invalid number field: ") + key;
        return false;
    }
    const double number = value->as_number();
    if (!std::isfinite(number)) {
        if (outError) *outError = std::string("Non-finite number field: ") + key;
        return false;
    }
    if (outValue) *outValue = number;
    return true;
}

bool IsSupportedTargetPath(std::string_view path) {
    static constexpr std::array<std::string_view, 9> kPaths = {
        "seed.combined",
        "params.explaino_seed_b",
        "params.explaino_mix",
        "params.explaino_warp_strength",
        "view.explaino_phase",
        "view.explaino_seed_drift",
        "view.center_hp_x",
        "view.center_hp_y",
        "view.log2_zoom",
    };
    return std::find(kPaths.begin(), kPaths.end(), path) != kPaths.end();
}

const RuntimeWalkFitsMappingProfile* FindProfile(const RuntimeWalkFitsMappingCatalog& catalog, std::string_view profileId) {
    for (const RuntimeWalkFitsMappingProfile& profile : catalog.profiles) {
        if (profile.id == profileId) return &profile;
    }
    return nullptr;
}

bool LookupSignal(const RuntimeWalkFitsOrientationInputs& inputs, std::string_view sourceSignal, double* outValue, std::string* outError) {
    const auto it = inputs.signals.find(std::string(sourceSignal));
    if (it == inputs.signals.end()) {
        if (outError) *outError = "Missing FITS orientation signal: " + std::string(sourceSignal);
        return false;
    }
    if (outValue) *outValue = it->second;
    return true;
}

double ClampDouble(double value, double minValue, double maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

double Clamp01(double value) {
    return ClampDouble(value, 0.0, 1.0);
}

double SignalOrDefault(const RuntimeWalkFitsOrientationInputs& inputs, std::string_view signalName, double defaultValue) {
    const auto it = inputs.signals.find(std::string(signalName));
    if (it == inputs.signals.end()) return defaultValue;
    if (!std::isfinite(it->second)) return defaultValue;
    return it->second;
}

double NormalizeUnitSignal(const RuntimeWalkFitsOrientationInputs& inputs, std::string_view signalName, double defaultValue = 0.5) {
    return Clamp01(SignalOrDefault(inputs, signalName, defaultValue));
}

double NormalizeSignedSignal(const RuntimeWalkFitsOrientationInputs& inputs, std::string_view signalName, double defaultValue = 0.0, double maxAbs = 1.0) {
    if (!(maxAbs > 0.0)) return 0.5;
    const double value = SignalOrDefault(inputs, signalName, defaultValue);
    return Clamp01(0.5 + 0.5 * (value / maxAbs));
}

double ResolveBindingValue(const RuntimeWalkFitsMappingBinding& binding, double signalValue) {
    const double span = binding.input_max - binding.input_min;
    double normalized = 0.0;
    if (std::fabs(span) > 1.0e-12) {
        normalized = (signalValue - binding.input_min) / span;
    }
    normalized = ClampDouble(normalized, 0.0, 1.0);
    double mapped = binding.offset + normalized * binding.scale * binding.weight;
    if (binding.has_clamp) {
        mapped = ClampDouble(mapped, binding.clamp_min, binding.clamp_max);
    }
    return mapped;
}

bool ApplyBindingValue(const RuntimeWalkFitsMappingBinding& binding, double value, ViewState* ioView, KernelParams* ioParams, std::string* outError) {
    if (!ioView || !ioParams) {
        if (outError) *outError = "Runtime-walk base-state synthesis requires view and params";
        return false;
    }

    if (binding.target_path == "seed.combined") {
        ExplainoSeedSetCombined(*ioView, *ioParams, value);
        return true;
    }
    if (binding.target_path == "params.explaino_seed_b") {
        ioParams->explaino_seed_b = value;
        return true;
    }
    if (binding.target_path == "params.explaino_mix") {
        ioParams->explaino_mix = static_cast<float>(value);
        return true;
    }
    if (binding.target_path == "params.explaino_warp_strength") {
        ioParams->explaino_warp_strength = static_cast<float>(value);
        return true;
    }
    if (binding.target_path == "view.explaino_phase") {
        ioView->explaino_phase = static_cast<float>(value);
        return true;
    }
    if (binding.target_path == "view.explaino_seed_drift") {
        ioView->explaino_seed_drift = static_cast<float>(value);
        return true;
    }
    if (binding.target_path == "view.center_hp_x") {
        ioView->center_hp_x = value;
        return true;
    }
    if (binding.target_path == "view.center_hp_y") {
        ioView->center_hp_y = value;
        return true;
    }
    if (binding.target_path == "view.log2_zoom") {
        ioView->log2_zoom = value;
        return true;
    }

    if (outError) *outError = "Unsupported FITS mapping target path: " + binding.target_path;
    return false;
}

std::wstring WidenUtf8(const std::string& text) {
    if (text.empty()) return {};
    const int needed = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    if (needed <= 0) return std::wstring(text.begin(), text.end());
    std::wstring wide(static_cast<size_t>(needed) - 1u, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, wide.data(), needed);
    return wide;
}

std::string TrimWhitespace(const std::string& text) {
    size_t start = 0;
    while (start < text.size()) {
        const char ch = text[start];
        if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n') break;
        ++start;
    }
    size_t end = text.size();
    while (end > start) {
        const char ch = text[end - 1];
        if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n') break;
        --end;
    }
    return text.substr(start, end - start);
}

std::filesystem::path FindRepoRootFromRuntimeMetadataLocal(const std::filesystem::path& runtimeDir) {
    if (runtimeDir.empty()) return {};
    const std::filesystem::path metadataPath = runtimeDir.lexically_normal() / "fractal_ui_repo_root.txt";
    std::ifstream file(metadataPath, std::ios::in | std::ios::binary);
    if (!file) return {};
    std::string text;
    std::getline(file, text);
    const std::string trimmed = TrimWhitespace(text);
    if (trimmed.empty()) return {};
    std::error_code ec;
    std::filesystem::path repoRoot(trimmed);
    repoRoot = repoRoot.lexically_normal();
    if (!std::filesystem::exists(repoRoot, ec) || ec) return {};
    return repoRoot;
}

bool NeedsCommandLineQuotes(const std::wstring& text) {
    if (text.empty()) return true;
    for (wchar_t ch : text) {
        if (ch == L' ' || ch == L'\t' || ch == L'\n' || ch == L'\v' || ch == L'"') return true;
    }
    return false;
}

void AppendCommandLineArg(std::wstring* commandLine, const std::wstring& arg) {
    if (!commandLine) return;
    if (!commandLine->empty()) commandLine->push_back(L' ');
    if (!NeedsCommandLineQuotes(arg)) {
        commandLine->append(arg);
        return;
    }
    commandLine->push_back(L'"');
    size_t backslashCount = 0;
    for (wchar_t ch : arg) {
        if (ch == L'\\') {
            ++backslashCount;
            continue;
        }
        if (ch == L'"') {
            commandLine->append(backslashCount * 2 + 1, L'\\');
            commandLine->push_back(L'"');
            backslashCount = 0;
            continue;
        }
        commandLine->append(backslashCount, L'\\');
        backslashCount = 0;
        commandLine->push_back(ch);
    }
    commandLine->append(backslashCount * 2, L'\\');
    commandLine->push_back(L'"');
}

std::string FormatWin32Error(DWORD errorCode) {
    LPSTR message = nullptr;
    const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
    const DWORD length = FormatMessageA(flags, nullptr, errorCode, 0, reinterpret_cast<LPSTR>(&message), 0, nullptr);
    std::string text;
    if (length != 0 && message) {
        text.assign(message, message + length);
        LocalFree(message);
    }
    text = TrimWhitespace(text);
    if (text.empty()) text = "Win32 error " + std::to_string(errorCode);
    return text;
}

bool ResolvePythonLauncher(std::filesystem::path* outPath, std::string* outError) {
    if (!outPath) return false;

    wchar_t windowsDir[MAX_PATH] = {};
    const UINT windowsDirLen = GetWindowsDirectoryW(windowsDir, MAX_PATH);
    if (windowsDirLen > 0 && windowsDirLen < MAX_PATH) {
        const std::filesystem::path defaultPy = std::filesystem::path(windowsDir) / "py.exe";
        std::error_code ec;
        if (std::filesystem::exists(defaultPy, ec) && !ec) {
            *outPath = defaultPy;
            return true;
        }
    }

    const DWORD required = SearchPathW(nullptr, L"py.exe", nullptr, 0, nullptr, nullptr);
    if (required > 0) {
        std::wstring resolved(static_cast<size_t>(required), L'\0');
        const DWORD actual = SearchPathW(nullptr, L"py.exe", nullptr, required, resolved.data(), nullptr);
        if (actual > 0 && actual < required) {
            resolved.resize(actual);
            *outPath = std::filesystem::path(resolved);
            return true;
        }
    }

    if (outError) *outError = "Could not resolve py.exe for runtime-walk FITS extraction.";
    return false;
}

std::filesystem::path ResolveRepoRoot(const std::string& exeDir) {
    const std::filesystem::path runtimeDir = std::filesystem::path(exeDir).lexically_normal();
    const std::filesystem::path metadataRoot = FindRepoRootFromRuntimeMetadataLocal(runtimeDir);
    if (!metadataRoot.empty()) return metadataRoot;
    const auto hasProfileSurface = [](const std::filesystem::path& candidate) {
        if (candidate.empty()) return false;
        std::error_code ec;
        return std::filesystem::exists(candidate / kDefaultMappingProfileRelativePath, ec) && !ec;
    };

    std::filesystem::path sourceRoot = std::filesystem::path(__FILE__).lexically_normal().parent_path().parent_path().parent_path();
    if (hasProfileSurface(sourceRoot)) return sourceRoot;

    std::error_code cwdError;
    const std::filesystem::path cwd = std::filesystem::current_path(cwdError);
    if (!cwdError) {
        if (hasProfileSurface(cwd)) return cwd;
        if (hasProfileSurface(cwd.parent_path())) return cwd.parent_path();
    }

    return sourceRoot;
}

std::string JsonEscape(const std::string& text) {
    std::string escaped;
    escaped.reserve(text.size() + 16);
    for (unsigned char ch : text) {
        switch (ch) {
        case '\\': escaped += "\\\\"; break;
        case '"': escaped += "\\\""; break;
        case '\b': escaped += "\\b"; break;
        case '\f': escaped += "\\f"; break;
        case '\n': escaped += "\\n"; break;
        case '\r': escaped += "\\r"; break;
        case '\t': escaped += "\\t"; break;
        default:
            if (ch < 0x20u) {
                char buffer[7];
                std::snprintf(buffer, sizeof(buffer), "\\u%04x", static_cast<unsigned int>(ch));
                escaped += buffer;
            } else {
                escaped.push_back(static_cast<char>(ch));
            }
            break;
        }
    }
    return escaped;
}

std::string BuildSynthesizedStateJson(const ViewState& view, const KernelParams& params, const RenderSettings& render) {
    std::ostringstream js;
    js.setf(std::ios::fixed);
    js.precision(8);
    js << "{\n";
    js << "  \"state_version\": 3,\n";
    js << "  \"fractal_type\": \"" << FractalTypeId(view.fractal_type) << "\",\n";
    js << "  \"view\": {\n";
    js << "    \"center_x\": " << static_cast<double>(view.center.x) << ",\n";
    js << "    \"center_y\": " << static_cast<double>(view.center.y) << ",\n";
    js << "    \"zoom\": " << static_cast<double>(view.zoom) << ",\n";
    js << "    \"rotation_degrees\": " << static_cast<double>(view.rotation_degrees) << ",\n";
    js << "    \"center_hp_x\": " << view.center_hp_x << ",\n";
    js << "    \"center_hp_y\": " << view.center_hp_y << ",\n";
    js << "    \"log2_zoom\": " << view.log2_zoom << ",\n";
    js << "    \"explaino_phase\": " << static_cast<double>(view.explaino_phase) << ",\n";
    js << "    \"explaino_seed_drift\": " << static_cast<double>(view.explaino_seed_drift) << ",\n";
    js << "    \"explaino_seed_tween\": " << (view.explaino_seed_tween ? "true" : "false") << ",\n";
    js << "    \"auto_max_iter\": " << (view.auto_max_iter ? "true" : "false") << ",\n";
    js << "    \"auto_increment_seed\": " << (view.auto_increment_seed ? "true" : "false") << ",\n";
    js << "    \"explaino_seed_rate\": " << static_cast<double>(view.explaino_seed_rate) << ",\n";
    js << "    \"explaino_phase_strength\": " << static_cast<double>(view.explaino_phase_strength) << "\n";
    js << "  },\n";
    js << "  \"params\": {\n";
    js << "    \"max_iter\": " << params.max_iter << ",\n";
    js << "    \"epsilon\": " << static_cast<double>(params.epsilon) << ",\n";
    js << "    \"exposure\": " << static_cast<double>(params.exposure) << ",\n";
    js << "    \"poly_kind\": " << static_cast<int>(params.poly_kind) << ",\n";
    js << "    \"coloring_mode\": \"" << ColoringModeId(params.coloring_mode) << "\",\n";
    js << "    \"nova_alpha\": " << static_cast<double>(params.nova_alpha) << ",\n";
    js << "    \"phoenix_p_real\": " << static_cast<double>(params.phoenix_p_real) << ",\n";
    js << "    \"phoenix_p_imag\": " << static_cast<double>(params.phoenix_p_imag) << ",\n";
    js << "    \"multibrot_power\": " << params.multibrot_power << ",\n";
    js << "    \"multibrot_power_float\": " << static_cast<double>(params.multibrot_power_float) << ",\n";
    js << "    \"lambda_real\": " << static_cast<double>(params.lambda_real) << ",\n";
    js << "    \"lambda_imag\": " << static_cast<double>(params.lambda_imag) << ",\n";
    js << "    \"explaino_seed\": " << params.explaino_seed << ",\n";
    js << "    \"explaino_seed_b\": " << params.explaino_seed_b << ",\n";
    js << "    \"explaino_mix\": " << static_cast<double>(params.explaino_mix) << ",\n";
    js << "    \"explaino_warp_strength\": " << static_cast<double>(params.explaino_warp_strength) << ",\n";
    js << "    \"explaino_root_spread\": " << static_cast<double>(params.explaino_root_spread) << ",\n";
    js << "    \"explaino_root_count\": " << params.explaino_root_count << ",\n";
    js << "    \"poly_coeffs\": [";
    for (int i = 0; i < 5; ++i) {
        if (i > 0) js << ", ";
        js << static_cast<double>(params.poly_coeffs[i]);
    }
    js << "],\n";
    js << "    \"color_saturation\": " << static_cast<double>(params.color_saturation) << ",\n";
    js << "    \"color_contrast\": " << static_cast<double>(params.color_contrast) << ",\n";
    js << "    \"color_tint_r\": " << static_cast<double>(params.color_tint_r) << ",\n";
    js << "    \"color_tint_g\": " << static_cast<double>(params.color_tint_g) << ",\n";
    js << "    \"color_tint_b\": " << static_cast<double>(params.color_tint_b) << "\n";
    js << "  },\n";
    js << "  \"render\": {\n";
    js << "    \"width\": " << render.resolution.x << ",\n";
    js << "    \"height\": " << render.resolution.y << ",\n";
    js << "    \"interaction_debounce_ms\": " << render.interaction_debounce_ms << ",\n";
    js << "    \"preview_target_fps\": " << static_cast<double>(render.preview_target_fps) << ",\n";
    js << "    \"preview_min_scale\": " << static_cast<double>(render.preview_min_scale) << ",\n";
    js << "    \"block_size\": " << render.block_size << ",\n";
    js << "    \"device_id\": " << render.device_id << "\n";
    js << "  }\n";
    js << "}\n";
    return js.str();
}

std::array<double, 13> BuildTransportSample0() {
    std::array<double, 13> channels{};
    channels.fill(0.5);
    return channels;
}

std::array<double, 13> BuildTransportAnchor(const RuntimeWalkFitsOrientationInputs& inputs) {
    const double mean = NormalizeUnitSignal(inputs, "mean");
    const double stddev = NormalizeUnitSignal(inputs, "stddev");
    const double center = NormalizeSignedSignal(inputs, "center_bias");
    const double energy = NormalizeUnitSignal(inputs, "residual_energy");
    const double edge = NormalizeSignedSignal(inputs, "edge_balance");
    const double delta = NormalizeUnitSignal(inputs, "frame_delta");
    const double xBias = NormalizeSignedSignal(inputs, "x_bias");
    const double yBias = NormalizeSignedSignal(inputs, "y_bias");
    const double focus = NormalizeUnitSignal(inputs, "focus_ratio");
    const double composite = Clamp01(0.40 * mean + 0.35 * focus + 0.25 * stddev);

    return {
        Clamp01(0.15 + 0.75 * xBias),
        Clamp01(0.15 + 0.75 * (1.0 - xBias)),
        Clamp01(0.15 + 0.75 * yBias),
        Clamp01(0.15 + 0.60 * (1.0 - yBias) + 0.10 * energy),
        Clamp01(0.20 + 0.65 * focus),
        Clamp01(0.20 + 0.65 * mean),
        Clamp01(0.15 + 0.70 * composite),
        Clamp01(0.15 + 0.70 * center),
        Clamp01(0.15 + 0.70 * (1.0 - center)),
        Clamp01(0.15 + 0.70 * edge),
        Clamp01(0.15 + 0.70 * (1.0 - edge)),
        Clamp01(0.20 + 0.60 * (1.0 - focus) + 0.10 * (1.0 - delta)),
        Clamp01(0.20 + 0.65 * energy),
    };
}

double HarmonicWave(double theta, double frequency, double phase) {
    return 0.5 + 0.5 * std::sin(theta * frequency + phase);
}

std::array<double, 13> BuildClosedLoopTransportSample(const RuntimeWalkFitsOrientationInputs& inputs,
    const std::array<double, 13>& anchor,
    double t) {
    const double mean = NormalizeUnitSignal(inputs, "mean");
    const double stddev = NormalizeUnitSignal(inputs, "stddev");
    const double center = NormalizeSignedSignal(inputs, "center_bias");
    const double energy = NormalizeUnitSignal(inputs, "residual_energy");
    const double edge = NormalizeSignedSignal(inputs, "edge_balance");
    const double delta = NormalizeUnitSignal(inputs, "frame_delta");
    const double xBias = NormalizeSignedSignal(inputs, "x_bias");
    const double yBias = NormalizeSignedSignal(inputs, "y_bias");
    const double focus = NormalizeUnitSignal(inputs, "focus_ratio");
    const double theta = 6.28318530717958647692 * t;
    const double phase = 6.28318530717958647692 * Clamp01(0.35 * mean + 0.35 * focus + 0.30 * energy);
    const double swirl = 0.18 + 0.16 * energy + 0.08 * stddev;
    const double drift = 0.12 + 0.10 * delta;
    const double loopX = std::cos(theta + phase);
    const double loopY = std::sin(theta + phase);
    const double lobeX = std::cos(2.0 * theta + center * 1.7);
    const double lobeY = std::sin(2.0 * theta - edge * 1.3);
    const double twist = std::sin(3.0 * theta + phase * 0.5);

    std::array<double, 13> sample{};
    sample[0] = Clamp01(anchor[0] + swirl * loopX + 0.08 * xBias + 0.05 * lobeX);
    sample[1] = Clamp01(anchor[1] - swirl * loopX + 0.08 * (1.0 - xBias) - 0.05 * lobeX);
    sample[2] = Clamp01(anchor[2] + swirl * loopY + 0.08 * yBias + 0.05 * lobeY);
    sample[3] = Clamp01(anchor[3] - swirl * loopY + 0.06 * energy + 0.08 * (1.0 - yBias));
    sample[4] = Clamp01(anchor[4] + 0.20 * HarmonicWave(theta, 1.0, phase) + 0.10 * focus + 0.05 * lobeX);
    sample[5] = Clamp01(anchor[5] + 0.18 * HarmonicWave(theta, 2.0, phase * 0.5) + 0.08 * mean);
    sample[6] = Clamp01(anchor[6] + 0.24 * HarmonicWave(theta, 1.0, 1.0471975512 + phase) + 0.08 * energy);
    sample[7] = Clamp01(anchor[7] + 0.20 * HarmonicWave(theta, 2.0, center * 1.2) + 0.08 * xBias + 0.04 * twist);
    sample[8] = Clamp01(anchor[8] + 0.20 * HarmonicWave(theta, 2.0, -center * 1.2) + 0.08 * yBias - 0.04 * twist);
    sample[9] = Clamp01(anchor[9] + drift * HarmonicWave(theta, 3.0, edge) + 0.08 * stddev);
    sample[10] = Clamp01(anchor[10] + drift * HarmonicWave(theta, 3.0, 3.1415926535 - edge) + 0.08 * mean);
    sample[11] = Clamp01(anchor[11] + 0.18 * HarmonicWave(theta, 1.0, 1.5707963267 + phase) + 0.10 * (1.0 - focus));
    sample[12] = Clamp01(anchor[12] + 0.20 * HarmonicWave(theta, 2.0, 0.7853981634 + phase) + 0.08 * delta);
    return sample;
}

std::string SerializeRuntimeWalkBundleJson(const RuntimeWalkBundle& bundle) {
    std::ostringstream out;
    out.setf(std::ios::fixed);
    out.precision(8);
    out << "{\n";
    out << "  \"version\": 1,\n";
    out << "  \"field_name\": \"" << JsonEscape(bundle.field_name) << "\",\n";
    out << "  \"samples\": [\n";
    for (std::size_t sampleIndex = 0; sampleIndex < bundle.samples.size(); ++sampleIndex) {
        const RuntimeWalkBundleSample& sample = bundle.samples[sampleIndex];
        out << "    {\n";
        out << "      \"id\": \"" << JsonEscape(sample.id) << "\",\n";
        out << "      \"t\": " << sample.t << ",\n";
        out << "      \"channels\": [";
        for (std::size_t channelIndex = 0; channelIndex < sample.channels.size(); ++channelIndex) {
            if (channelIndex > 0) out << ", ";
            out << sample.channels[channelIndex];
        }
        out << "]\n";
        out << "    }";
        if (sampleIndex + 1u < bundle.samples.size()) out << ",";
        out << "\n";
    }
    out << "  ],\n";
    out << "  \"branch_markers\": [\n";
    for (std::size_t markerIndex = 0; markerIndex < bundle.branch_markers.size(); ++markerIndex) {
        const RuntimeWalkBranchMarker& marker = bundle.branch_markers[markerIndex];
        out << "    {\n";
        out << "      \"id\": \"" << JsonEscape(marker.id) << "\",\n";
        out << "      \"label\": \"" << JsonEscape(marker.label) << "\",\n";
        out << "      \"parent_id\": \"" << JsonEscape(marker.parent_id) << "\",\n";
        out << "      \"t\": " << marker.t << ",\n";
        out << "      \"sticky_radius\": " << marker.sticky_radius << "\n";
        out << "    }";
        if (markerIndex + 1u < bundle.branch_markers.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

} // namespace

bool ParseRuntimeWalkFitsMappingCatalogJson(const std::string& jsonText,
    RuntimeWalkFitsMappingCatalog* outCatalog,
    std::string* outError) {
    if (outError) outError->clear();
    if (outCatalog) *outCatalog = {};

    json_min::ParseResult parsed = json_min::Parse(jsonText);
    if (!parsed.error.empty() || !parsed.value.is_object()) {
        if (outError) *outError = parsed.error.empty() ? "Runtime-walk FITS mapping catalog JSON must be an object" : parsed.error;
        return false;
    }

    double version = 0.0;
    if (!GetOptionalNumber(parsed.value, "version", &version, outError)) return false;
    if (version != 1.0) {
        if (outError) *outError = "runtime-walk FITS mapping catalog version must be 1";
        return false;
    }

    const json_min::Value* profilesValue = nullptr;
    if (!GetRequiredArray(parsed.value, "profiles", &profilesValue, outError)) return false;

    RuntimeWalkFitsMappingCatalog catalog;
    for (const json_min::Value& profileValue : profilesValue->as_array()) {
        if (!profileValue.is_object()) {
            if (outError) *outError = "runtime-walk FITS mapping profiles must be objects";
            return false;
        }
        RuntimeWalkFitsMappingProfile profile;
        if (!GetRequiredString(profileValue, "id", &profile.id, outError)) return false;
        if (!GetRequiredString(profileValue, "target_selector", &profile.target_selector, outError)) return false;
        std::string fractalTypeId;
        if (!GetRequiredString(profileValue, "base_fractal_type", &fractalTypeId, outError)) return false;
        if (!TryParseFractalTypeId(fractalTypeId, &profile.base_fractal_type)) {
            if (outError) *outError = "Unknown base_fractal_type in runtime-walk FITS mapping profile: " + fractalTypeId;
            return false;
        }

        const json_min::Value* bindingsValue = nullptr;
        if (!GetRequiredArray(profileValue, "bindings", &bindingsValue, outError)) return false;
        for (const json_min::Value& bindingValue : bindingsValue->as_array()) {
            if (!bindingValue.is_object()) {
                if (outError) *outError = "runtime-walk FITS mapping bindings must be objects";
                return false;
            }
            RuntimeWalkFitsMappingBinding binding;
            if (!GetRequiredString(bindingValue, "target_selector", &binding.target_selector, outError)) return false;
            if (!GetRequiredString(bindingValue, "source_signal", &binding.source_signal, outError)) return false;
            if (!GetRequiredString(bindingValue, "target_path", &binding.target_path, outError)) return false;
            if (!IsSupportedTargetPath(binding.target_path)) {
                if (outError) *outError = "Unsupported runtime-walk FITS mapping target path: " + binding.target_path;
                return false;
            }
            if (!GetOptionalNumber(bindingValue, "input_min", &binding.input_min, outError)) return false;
            if (!GetOptionalNumber(bindingValue, "input_max", &binding.input_max, outError)) return false;
            if (!GetOptionalNumber(bindingValue, "scale", &binding.scale, outError)) return false;
            if (!GetOptionalNumber(bindingValue, "offset", &binding.offset, outError)) return false;
            if (!GetOptionalNumber(bindingValue, "weight", &binding.weight, outError)) return false;
            if (!GetOptionalBool(bindingValue, "enabled", &binding.enabled, outError)) return false;
            const json_min::Value* clampMinValue = bindingValue.get("clamp_min");
            const json_min::Value* clampMaxValue = bindingValue.get("clamp_max");
            if ((clampMinValue != nullptr) != (clampMaxValue != nullptr)) {
                if (outError) *outError = "runtime-walk FITS mapping clamps require both clamp_min and clamp_max";
                return false;
            }
            if (clampMinValue && clampMaxValue) {
                binding.has_clamp = true;
                if (!GetOptionalNumber(bindingValue, "clamp_min", &binding.clamp_min, outError)) return false;
                if (!GetOptionalNumber(bindingValue, "clamp_max", &binding.clamp_max, outError)) return false;
                if (!(binding.clamp_min <= binding.clamp_max)) {
                    if (outError) *outError = "runtime-walk FITS mapping clamp_min must be <= clamp_max";
                    return false;
                }
            }
            if (std::fabs(binding.input_max - binding.input_min) <= 1.0e-12) {
                if (outError) *outError = "runtime-walk FITS mapping input range must be non-zero for target path: " + binding.target_path;
                return false;
            }
            profile.bindings.push_back(binding);
        }
        catalog.profiles.push_back(profile);
    }

    if (outCatalog) *outCatalog = catalog;
    return true;
}

bool LoadRuntimeWalkFitsMappingCatalogFile(const std::string& path,
    RuntimeWalkFitsMappingCatalog* outCatalog,
    std::string* outError) {
    std::string jsonText;
    if (!ReadTextFile(path, &jsonText, outError)) return false;
    return ParseRuntimeWalkFitsMappingCatalogJson(jsonText, outCatalog, outError);
}

bool ParseRuntimeWalkFitsOrientationInputsJson(const std::string& jsonText,
    RuntimeWalkFitsOrientationInputs* outInputs,
    std::string* outError) {
    if (outError) outError->clear();
    if (outInputs) *outInputs = {};

    json_min::ParseResult parsed = json_min::Parse(jsonText);
    if (!parsed.error.empty() || !parsed.value.is_object()) {
        if (outError) *outError = parsed.error.empty() ? "Runtime-walk FITS orientation JSON must be an object" : parsed.error;
        return false;
    }

    RuntimeWalkFitsOrientationInputs inputs;
    double version = 0.0;
    if (!GetOptionalNumber(parsed.value, "version", &version, outError)) return false;
    if (version != 1.0) {
        if (outError) *outError = "runtime-walk FITS orientation input version must be 1";
        return false;
    }
    if (!GetRequiredString(parsed.value, "fits_path", &inputs.fits_path, outError)) return false;
    const json_min::Value* signalsValue = parsed.value.get("signals");
    if (!signalsValue || !signalsValue->is_object()) {
        if (outError) *outError = "runtime-walk FITS orientation inputs require object field: signals";
        return false;
    }
    for (const auto& it : signalsValue->as_object()) {
        if (!it.second.is_number()) {
            if (outError) *outError = "runtime-walk FITS orientation signals must be numeric";
            return false;
        }
        const double value = it.second.as_number();
        if (!std::isfinite(value)) {
            if (outError) *outError = "runtime-walk FITS orientation signals must be finite";
            return false;
        }
        inputs.signals[it.first] = value;
    }
    if (inputs.signals.empty()) {
        if (outError) *outError = "runtime-walk FITS orientation inputs require at least one signal";
        return false;
    }

    if (outInputs) *outInputs = inputs;
    return true;
}

bool LoadRuntimeWalkFitsOrientationInputsFile(const std::string& path,
    RuntimeWalkFitsOrientationInputs* outInputs,
    std::string* outError) {
    std::string jsonText;
    if (!ReadTextFile(path, &jsonText, outError)) return false;
    return ParseRuntimeWalkFitsOrientationInputsJson(jsonText, outInputs, outError);
}

bool ResolveDefaultRuntimeWalkFitsMappingProfilePath(const std::string& exeDir,
    std::string* outPath,
    std::string* outError) {
    if (outError) outError->clear();
    const std::filesystem::path repoRoot = ResolveRepoRoot(exeDir);
    if (repoRoot.empty()) {
        if (outError) *outError = "Could not resolve repo root for runtime-walk FITS mapping profile.";
        return false;
    }
    const std::filesystem::path profilePath = repoRoot / kDefaultMappingProfileRelativePath;
    std::error_code ec;
    if (!std::filesystem::exists(profilePath, ec) || ec) {
        if (outError) *outError = "Default runtime-walk FITS mapping profile missing: " + profilePath.string();
        return false;
    }
    if (outPath) *outPath = profilePath.lexically_normal().string();
    return true;
}

bool ExtractRuntimeWalkFitsOrientationInputs(const std::string& exeDir,
    const std::string& fitsPath,
    const std::string& outJsonPath,
    std::string* outError) {
    if (outError) outError->clear();
    if (fitsPath.empty()) {
        if (outError) *outError = "Runtime-walk FITS extraction requires a FITS path";
        return false;
    }
    if (outJsonPath.empty()) {
        if (outError) *outError = "Runtime-walk FITS extraction requires an output JSON path";
        return false;
    }

    const std::filesystem::path repoRoot = ResolveRepoRoot(exeDir);
    if (repoRoot.empty()) {
        if (outError) *outError = "Could not resolve repo root for runtime-walk FITS extraction.";
        return false;
    }
    const std::filesystem::path scriptPath = repoRoot / kOrientationExtractorRelativePath;
    std::error_code ec;
    if (!std::filesystem::exists(scriptPath, ec) || ec) {
        if (outError) *outError = "Runtime-walk FITS orientation extractor missing: " + scriptPath.string();
        return false;
    }

    std::filesystem::path pythonLauncher;
    if (!ResolvePythonLauncher(&pythonLauncher, outError)) return false;

    std::wstring commandLine;
    AppendCommandLineArg(&commandLine, pythonLauncher.wstring());
    AppendCommandLineArg(&commandLine, L"-3.14");
    AppendCommandLineArg(&commandLine, scriptPath.wstring());
    AppendCommandLineArg(&commandLine, L"--fits");
    AppendCommandLineArg(&commandLine, WidenUtf8(fitsPath));
    AppendCommandLineArg(&commandLine, L"--out-json");
    AppendCommandLineArg(&commandLine, WidenUtf8(outJsonPath));

    STARTUPINFOW startup{};
    startup.cb = sizeof(startup);
    PROCESS_INFORMATION process{};
    std::wstring mutableCommandLine = commandLine;
    std::wstring workingDirectory = repoRoot.wstring();
    const BOOL created = CreateProcessW(
        pythonLauncher.wstring().c_str(),
        mutableCommandLine.data(),
        nullptr,
        nullptr,
        FALSE,
        CREATE_NO_WINDOW,
        nullptr,
        workingDirectory.empty() ? nullptr : workingDirectory.c_str(),
        &startup,
        &process);
    if (!created) {
        if (outError) *outError = "CreateProcessW failed for runtime-walk FITS extraction: " + FormatWin32Error(GetLastError());
        return false;
    }

    WaitForSingleObject(process.hProcess, INFINITE);
    DWORD exitCode = 1;
    GetExitCodeProcess(process.hProcess, &exitCode);
    CloseHandle(process.hThread);
    CloseHandle(process.hProcess);
    if (exitCode != 0) {
        if (outError) *outError = "Runtime-walk FITS orientation extractor failed with exit code " + std::to_string(exitCode);
        return false;
    }
    return true;
}

bool SynthesizeRuntimeWalkBaseState(const RuntimeWalkFitsMappingCatalog& catalog,
    const std::string& profileId,
    const RuntimeWalkFitsOrientationInputs& inputs,
    ViewState* outView,
    KernelParams* outParams,
    RenderSettings* outRender,
    std::string* outError) {
    if (outError) outError->clear();
    if (!outView || !outParams || !outRender) {
        if (outError) *outError = "Runtime-walk base-state synthesis requires non-null outputs";
        return false;
    }

    const RuntimeWalkFitsMappingProfile* profile = FindProfile(catalog, profileId);
    if (!profile) {
        if (outError) *outError = "Unknown runtime-walk FITS mapping profile: " + profileId;
        return false;
    }
    if (!IsExplainoFamily(profile->base_fractal_type)) {
        if (outError) *outError = "Synthesized runtime-walk base state currently requires an Explaino-family profile";
        return false;
    }

    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    bool dirty = false;
    view.fractal_type = profile->base_fractal_type;
    ApplyFractalViewPresetDefaults(view, &dirty);
    ApplyFractalPresetDefaults(view, params, &dirty);
    view.camera_behavior = CameraBehavior::manual;
    view.auto_dive = false;
    view.dive_speed = 0.0f;
    view.auto_refresh = false;
    view.auto_increment_seed = false;
    view.explaino_seed_tween = true;
    SyncViewUiFromHp(view);

    for (const RuntimeWalkFitsMappingBinding& binding : profile->bindings) {
        if (!binding.enabled) continue;
        if (!(binding.target_selector == profile->target_selector || binding.target_selector == "all")) continue;
        double signalValue = 0.0;
        if (!LookupSignal(inputs, binding.source_signal, &signalValue, outError)) return false;
        const double mappedValue = ResolveBindingValue(binding, signalValue);
        if (!ApplyBindingValue(binding, mappedValue, &view, &params, outError)) return false;
    }

    UpdateExplainoPolynomial(view, params, &dirty);
    SyncViewUiFromHp(view);

    if (render.resolution.x <= 0 || render.resolution.y <= 0) {
        render.resolution = {1024, 768};
    }
    if (render.block_size <= 0) render.block_size = 256;
    if (render.device_id < 0) render.device_id = 0;

    *outView = view;
    *outParams = params;
    *outRender = render;
    return true;
}

bool SynthesizeRuntimeWalkTransportBundle(const RuntimeWalkFitsMappingCatalog& catalog,
    const std::string& profileId,
    const RuntimeWalkFitsOrientationInputs& inputs,
    RuntimeWalkBundle* outBundle,
    std::string* outError) {
    if (outError) outError->clear();
    if (!outBundle) {
        if (outError) *outError = "Runtime-walk transport synthesis requires a bundle output";
        return false;
    }

    const RuntimeWalkFitsMappingProfile* profile = FindProfile(catalog, profileId);
    if (!profile) {
        if (outError) *outError = "Unknown runtime-walk FITS mapping profile: " + profileId;
        return false;
    }
    if (!IsExplainoFamily(profile->base_fractal_type)) {
        if (outError) *outError = "Synthesized runtime-walk transport currently requires an Explaino-family profile";
        return false;
    }
    if (inputs.fits_path.empty()) {
        if (outError) *outError = "Synthesized runtime-walk transport requires FITS-derived orientation inputs";
        return false;
    }

    RuntimeWalkBundle bundle;
    bundle.field_name = "mr_zipper_branch";
    const std::array<double, 13> anchor = BuildTransportAnchor(inputs);
    constexpr int kClosedLoopSegments = 16;
    bundle.samples.reserve(static_cast<std::size_t>(kClosedLoopSegments + 1));
    for (int index = 0; index <= kClosedLoopSegments; ++index) {
        const double t = static_cast<double>(index) / static_cast<double>(kClosedLoopSegments);
        std::ostringstream id;
        id << "loop_" << std::setw(2) << std::setfill('0') << index;
        bundle.samples.push_back(RuntimeWalkBundleSample{id.str(), t, BuildClosedLoopTransportSample(inputs, anchor, t)});
    }
    bundle.branch_markers = {
        RuntimeWalkBranchMarker{"entry", "entry", "main", 0.125, 0.08},
        RuntimeWalkBranchMarker{"coast", "coast", "main", 0.375, 0.10},
        RuntimeWalkBranchMarker{"branch", "branch", "main", 0.625, 0.10},
        RuntimeWalkBranchMarker{"settle", "settle", "main", 0.875, 0.08},
    };
    *outBundle = bundle;
    return true;
}

bool WriteRuntimeWalkBundleJsonFile(const std::string& path,
    const RuntimeWalkBundle& bundle,
    std::string* outError) {
    return WriteTextFile(path, SerializeRuntimeWalkBundleJson(bundle), outError);
}

bool WriteRuntimeWalkSynthesizedStateJson(const std::string& path,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    std::string* outError) {
    return WriteTextFile(path, BuildSynthesizedStateJson(view, params, render), outError);
}
