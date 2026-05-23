#include "runtime_walk_bootstrap.h"

#include "explaino_seed.h"
#include "explaino_seed_curve.h"
#include "enum_id_utils.h"
#include "fractal_derived_fields.h"
#include "fractal_family_rules.h"
#include "json_min.h"
#include "schema_binding.h"
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

bool GetOptionalString(const json_min::Value& object, const char* key, std::string* outValue) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_string()) return false;
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

std::string CanonicalTargetPathInternal(std::string_view path) {
    if (path == "seed.combined") return "fractal.params.explaino_seed";
    if (path == "params.explaino_seed_b") return "fractal.params.explaino_seed_b";
    if (path == "params.explaino_mix") return "fractal.params.explaino_mix";
    if (path == "params.explaino_damping") return "fractal.params.explaino_damping";
    if (path == "view.explaino_phase") return "fractal.view.explaino_phase";
    if (path == "view.explaino_phase_strength") return "fractal.view.explaino_phase_strength";
    if (path == "view.explaino_seed_drift") return "fractal.view.explaino_seed_drift";
    if (path == "view.center_hp_x") return "fractal.view.center.x";
    if (path == "view.center_hp_y") return "fractal.view.center.y";
    if (path == "view.log2_zoom") return "fractal.view.zoom";
    return std::string(path);
}

bool IsWarpTargetPath(std::string_view canonicalPath) {
    return canonicalPath == "fractal.params.explaino_warp_strength" ||
        canonicalPath == "fractal.params.warp_strength";
}

const std::vector<std::string>& CandidateTargetPaths() {
    static const std::vector<std::string> kPaths = [] {
        std::vector<std::string> paths = {
            "fractal.params.explaino_seed",
            "fractal.params.explaino_seed_b",
            "fractal.params.explaino_mix",
            "fractal.params.explaino_damping",
            "fractal.params.explaino_root_spread",
            "fractal.params.explaino_cluster_radius",
            "fractal.params.momentum_beta",
            "fractal.params.joy_coupling",
            "fractal.params.fold_coupling",
            "fractal.params.bell_coupling",
        };
        paths.reserve(paths.size() + (sizeof(kExplainoAxisRegistry) / sizeof(kExplainoAxisRegistry[0])) + 6u);
        for (const auto& axis : kExplainoAxisRegistry) {
            paths.push_back(axis.binding_path);
        }
        paths.push_back("fractal.view.explaino_phase");
        paths.push_back("fractal.view.explaino_phase_strength");
        paths.push_back("fractal.view.explaino_seed_drift");
        paths.push_back("fractal.view.center.x");
        paths.push_back("fractal.view.center.y");
        paths.push_back("fractal.view.zoom");
        return paths;
    }();
    return kPaths;
}

bool IsSupportedTargetPath(std::string_view path) {
    const std::string canonical = CanonicalTargetPathInternal(path);
    if (IsWarpTargetPath(canonical)) return false;
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    BindingContext ctx{&view, &params, &render, nullptr};
    float* floatPtr = nullptr;
    double* doublePtr = nullptr;
    return ctx.BindFloat(canonical, &floatPtr) || ctx.BindDouble(canonical, &doublePtr);
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

bool LookupSignalMapValue(const std::map<std::string, double>& signals, std::string_view sourceSignal, double* outValue) {
    const auto it = signals.find(std::string(sourceSignal));
    if (it == signals.end() || !std::isfinite(it->second)) return false;
    if (outValue) *outValue = it->second;
    return true;
}

std::string EffectiveSourceSignal(const RuntimeWalkFitsMappingBinding& binding) {
    if (!binding.source_signal.empty()) return binding.source_signal;
    constexpr std::string_view prefix = "fits.frame.";
    if (binding.source_path.rfind(std::string(prefix), 0) == 0) {
        return binding.source_path.substr(prefix.size());
    }
    return binding.source_path;
}

bool BindingUsesFieldSource(const RuntimeWalkFitsMappingBinding& binding) {
    return binding.source_kind == "field" ||
        binding.source_signal.rfind("field.", 0) == 0 ||
        binding.source_path.rfind("field.", 0) == 0;
}

bool ResolveFieldSignalValue(const RuntimeWalkFitsMappingBinding& binding,
    const RuntimeWalkFitsFieldSignals& fieldSignals,
    double* outValue,
    std::string* outError) {
    const std::string key = binding.source_path.empty() ? binding.source_signal : binding.source_path;
    if (key == "field.traveler.score" || key == "traveler_score") {
        if (outValue) *outValue = fieldSignals.traveler_score;
        return true;
    }
    if (key == "field.traveler.confidence" || key == "traveler_confidence") {
        if (outValue) *outValue = fieldSignals.traveler_confidence;
        return true;
    }
    if (key == "field.tangent.angle" || key == "field.tangent_angle" || key == "tangent_angle") {
        if (outValue) *outValue = fieldSignals.tangent_angle;
        return true;
    }
    if (key == "field.tangent.x" || key == "tangent_x") {
        if (outValue) *outValue = fieldSignals.tangent_x;
        return true;
    }
    if (key == "field.tangent.y" || key == "tangent_y") {
        if (outValue) *outValue = fieldSignals.tangent_y;
        return true;
    }
    if (key == "field.residual.pressure" || key == "residual_pressure") {
        if (outValue) *outValue = fieldSignals.residual_pressure;
        return true;
    }
    if (key == "field.cluster.spread" || key == "cluster_spread") {
        if (outValue) *outValue = fieldSignals.cluster_spread;
        return true;
    }
    if (outError) *outError = "Unsupported runtime-walk FITS field source: " + key;
    return false;
}

double ClampDouble(double value, double minValue, double maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

double ClampRuntimeWalkTargetDomainValue(const std::string& targetPath, double value, bool* outClamped = nullptr) {
    double clamped = value;
    if (targetPath == "fractal.params.explaino_mix" ||
        targetPath == "fractal.view.explaino_phase_strength") {
        clamped = ClampDouble(value, 0.0, 1.0);
    } else if (targetPath == "fractal.view.explaino_seed_drift") {
        clamped = ClampDouble(value, 0.0, 1.0);
    }
    if (outClamped) *outClamped = std::fabs(clamped - value) > 1.0e-12;
    return clamped;
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
    if (binding.polarity < 0) {
        normalized = 1.0 - normalized;
    }

    const double smoothing = ClampDouble(binding.smoothing, 0.0, 1.0);
    const double smoothStep = normalized * normalized * (3.0 - 2.0 * normalized);
    double shaped = normalized + (smoothStep - normalized) * smoothing;
    double mapped = 0.0;
    if (binding.curve == "wedge_logistic_seed") {
        shaped = ExplainoWedgeTween(shaped);
        mapped = binding.offset + shaped * binding.scale * binding.weight;
    } else if (binding.curve == "smoothstep") {
        mapped = binding.offset + smoothStep * binding.scale * binding.weight;
    } else if (binding.curve == "signed_tanh") {
        mapped = binding.offset + std::tanh((normalized * 2.0 - 1.0) * 1.75) * binding.scale * binding.weight;
    } else if (binding.curve == "linear") {
        mapped = binding.offset + normalized * binding.scale * binding.weight;
    } else {
        mapped = binding.offset + shaped * binding.scale * binding.weight;
    }
    return mapped;
}

bool ReadRuntimeWalkTargetValue(const std::string& targetPath,
    const ViewState& view,
    const KernelParams& params,
    double* outValue) {
    if (!outValue) return false;
    if (targetPath == "fractal.params.explaino_seed") {
        *outValue = ExplainoSeedCombined(view, params);
        return true;
    }
    if (targetPath == "fractal.view.center.x") {
        *outValue = view.center_hp_x;
        return true;
    }
    if (targetPath == "fractal.view.center.y") {
        *outValue = view.center_hp_y;
        return true;
    }
    if (targetPath == "fractal.view.zoom") {
        *outValue = view.log2_zoom;
        return true;
    }
    ViewState viewCopy = view;
    KernelParams paramsCopy = params;
    BindingContext ctx{&viewCopy, &paramsCopy, nullptr, nullptr};
    double* doublePtr = nullptr;
    if (ctx.BindDouble(targetPath, &doublePtr) && doublePtr) {
        *outValue = *doublePtr;
        return true;
    }
    float* floatPtr = nullptr;
    if (ctx.BindFloat(targetPath, &floatPtr) && floatPtr) {
        *outValue = static_cast<double>(*floatPtr);
        return true;
    }
    return false;
}

bool ApplyRuntimeWalkTargetValue(const std::string& targetPath,
    double value,
    ViewState* ioView,
    KernelParams* ioParams,
    std::string* outError) {
    value = ClampRuntimeWalkTargetDomainValue(targetPath, value);
    if (targetPath == "fractal.params.explaino_seed") {
        ExplainoSeedSetCombined(*ioView, *ioParams, value);
        return true;
    }
    if (targetPath == "fractal.view.center.x") {
        ioView->center_hp_x = value;
        ioView->center.x = static_cast<float>(value);
        return true;
    }
    if (targetPath == "fractal.view.center.y") {
        ioView->center_hp_y = value;
        ioView->center.y = static_cast<float>(value);
        return true;
    }
    if (targetPath == "fractal.view.zoom") {
        ioView->log2_zoom = ClampDouble(value, Log2D(kMinZoom), kMaxLog2Zoom);
        SyncViewUiFromHp(*ioView);
        return true;
    }
    BindingContext ctx{ioView, ioParams, nullptr, nullptr};
    double* doublePtr = nullptr;
    if (ctx.BindDouble(targetPath, &doublePtr) && doublePtr) {
        *doublePtr = value;
        return true;
    }
    float* floatPtr = nullptr;
    if (ctx.BindFloat(targetPath, &floatPtr) && floatPtr) {
        *floatPtr = static_cast<float>(value);
        return true;
    }
    if (outError) *outError = "Unsupported FITS mapping target path: " + targetPath;
    return false;
}

bool ApplyBindingValue(const RuntimeWalkFitsMappingBinding& binding, double value, ViewState* ioView, KernelParams* ioParams, std::string* outError) {
    if (!ioView || !ioParams) {
        if (outError) *outError = "Runtime-walk base-state synthesis requires view and params";
        return false;
    }

    const std::string targetPath = CanonicalTargetPathInternal(binding.target_path);
    value = ClampRuntimeWalkTargetDomainValue(targetPath, value);
    if (IsWarpTargetPath(targetPath)) {
        if (outError) *outError = "Warp is not a supported default FITS binding target: " + targetPath;
        return false;
    }
    if (targetPath == "fractal.params.explaino_seed") {
        ExplainoSeedSetCombined(*ioView, *ioParams, value);
        return true;
    }
    if (targetPath == "fractal.view.center.x") {
        ioView->center_hp_x = value;
        ioView->center.x = static_cast<float>(value);
        return true;
    }
    if (targetPath == "fractal.view.center.y") {
        ioView->center_hp_y = value;
        ioView->center.y = static_cast<float>(value);
        return true;
    }
    if (targetPath == "fractal.view.zoom") {
        const double zoom = (std::max)(1.0e-12, value);
        ioView->zoom = static_cast<float>(zoom);
        ioView->log2_zoom = std::log2(zoom);
        return true;
    }

    BindingContext ctx{ioView, ioParams, nullptr, nullptr};
    double* doublePtr = nullptr;
    if (ctx.BindDouble(targetPath, &doublePtr) && doublePtr) {
        *doublePtr = value;
        return true;
    }
    float* floatPtr = nullptr;
    if (ctx.BindFloat(targetPath, &floatPtr) && floatPtr) {
        *floatPtr = static_cast<float>(value);
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
    js << "    \"julia_c_real\": " << static_cast<double>(params.julia_c_real) << ",\n";
    js << "    \"julia_c_imag\": " << static_cast<double>(params.julia_c_imag) << ",\n";
    js << "    \"explaino_julia_constant_mode\": \"" << ExplainoJuliaConstantModeId(params.explaino_julia_constant_mode) << "\",\n";
    js << "    \"explaino_julia_c_real\": " << static_cast<double>(params.explaino_julia_c_real) << ",\n";
    js << "    \"explaino_julia_c_imag\": " << static_cast<double>(params.explaino_julia_c_imag) << ",\n";
    js << "    \"multibrot_power\": " << params.multibrot_power << ",\n";
    js << "    \"multibrot_power_float\": " << static_cast<double>(params.multibrot_power_float) << ",\n";
    js << "    \"multibrot_power_imag\": " << static_cast<double>(params.multibrot_power_imag) << ",\n";
    js << "    \"burning_ship_fold_mix\": " << static_cast<double>(params.burning_ship_fold_mix) << ",\n";
    js << "    \"celtic_abs_mix\": " << static_cast<double>(params.celtic_abs_mix) << ",\n";
    js << "    \"perpendicular_fold_mix\": " << static_cast<double>(params.perpendicular_fold_mix) << ",\n";
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
    js << "    \"color_tint_b\": " << static_cast<double>(params.color_tint_b) << ",\n";
    js << "    \"color_phase_signal_offset\": " << static_cast<double>(params.color_phase_signal_offset) << ",\n";
    js << "    \"color_phase_wrap_cycles\": " << static_cast<double>(params.color_phase_wrap_cycles) << ",\n";
    js << "    \"color_phase_palette_offset\": " << static_cast<double>(params.color_phase_palette_offset) << ",\n";
    js << "    \"color_iteration_band_count\": " << params.color_iteration_band_count << ",\n";
    js << "    \"color_iteration_band_softness\": " << static_cast<double>(params.color_iteration_band_softness) << ",\n";
    js << "    \"color_iteration_band_emphasis\": " << static_cast<double>(params.color_iteration_band_emphasis) << ",\n";
    js << "    \"color_iteration_band_palette_offset\": " << static_cast<double>(params.color_iteration_band_palette_offset) << ",\n";
    js << "    \"color_smooth_escape_scale\": " << static_cast<double>(params.color_smooth_escape_scale) << ",\n";
    js << "    \"color_smooth_escape_bias\": " << static_cast<double>(params.color_smooth_escape_bias) << ",\n";
    js << "    \"color_heatmap_cycle_scale\": " << static_cast<double>(params.color_heatmap_cycle_scale) << ",\n";
    js << "    \"color_heatmap_saturation\": " << static_cast<double>(params.color_heatmap_saturation) << ",\n";
    js << "    \"color_contrast_lift_exposure\": " << static_cast<double>(params.color_contrast_lift_exposure) << ",\n";
    js << "    \"color_contrast_lift_saturation\": " << static_cast<double>(params.color_contrast_lift_saturation) << "\n";
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
    const RuntimeWalkTransportSynthesisOptions& options,
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
    const double motionScale = ClampDouble(options.motion_scale, 0.0, 2.0);
    const double swirl = (0.18 + 0.16 * energy + 0.08 * stddev) * motionScale;
    const double drift = (0.12 + 0.10 * delta) * motionScale;
    const double loopX = std::cos(theta + phase);
    const double loopY = std::sin(theta + phase);
    const double lobeX = std::cos(2.0 * theta + center * 1.7);
    const double lobeY = std::sin(2.0 * theta - edge * 1.3);
    const double twist = std::sin(3.0 * theta + phase * 0.5);

    std::array<double, 13> sample{};
    sample[0] = Clamp01(anchor[0] + swirl * loopX + 0.08 * xBias + 0.05 * lobeX);
    sample[1] = Clamp01(anchor[1] - swirl * loopX + 0.08 * (1.0 - xBias) - 0.05 * lobeX);
    sample[2] = Clamp01(anchor[2] + swirl * loopY + 0.08 * yBias + 0.05 * lobeY);
    sample[3] = Clamp01(anchor[3] - swirl * loopY * 0.65 + 0.03 * energy + 0.04 * (1.0 - yBias));
    sample[4] = Clamp01(anchor[4] + motionScale * (0.20 * HarmonicWave(theta, 1.0, phase) + 0.05 * lobeX) + 0.08 * focus);
    sample[5] = Clamp01(anchor[5] + motionScale * 0.18 * HarmonicWave(theta, 2.0, phase * 0.5) + 0.08 * mean);
    sample[6] = Clamp01(anchor[6] + motionScale * 0.18 * HarmonicWave(theta, 1.0, 1.0471975512 + phase) + 0.05 * energy);
    sample[7] = Clamp01(anchor[7] + motionScale * (0.16 * HarmonicWave(theta, 2.0, center * 1.2) + 0.03 * twist) + 0.06 * xBias);
    sample[8] = Clamp01(anchor[8] + motionScale * (0.16 * HarmonicWave(theta, 2.0, -center * 1.2) - 0.03 * twist) + 0.06 * yBias);
    sample[9] = Clamp01(anchor[9] + drift * HarmonicWave(theta, 3.0, edge) + 0.05 * stddev);
    sample[10] = Clamp01(anchor[10] + drift * HarmonicWave(theta, 3.0, 3.1415926535 - edge) + 0.05 * mean);
    sample[11] = Clamp01(anchor[11] + motionScale * 0.16 * HarmonicWave(theta, 1.0, 1.5707963267 + phase) + 0.08 * (1.0 - focus));
    sample[12] = Clamp01(anchor[12] + motionScale * (0.06 * HarmonicWave(theta, 2.0, 0.7853981634 + phase) + 0.02 * delta));
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

std::string CanonicalRuntimeWalkFitsMappingTargetPath(const std::string& targetPath) {
    return CanonicalTargetPathInternal(targetPath);
}

bool IsSupportedRuntimeWalkFitsMappingTargetPath(const std::string& targetPath) {
    return IsSupportedTargetPath(targetPath);
}

std::vector<std::string> RuntimeWalkFitsSupportedMappingTargetPaths() {
    std::vector<std::string> paths;
    for (const std::string& path : CandidateTargetPaths()) {
        if (IsSupportedTargetPath(path)) paths.push_back(path);
    }
    return paths;
}

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
            GetOptionalString(bindingValue, "source_kind", &binding.source_kind);
            GetOptionalString(bindingValue, "source_path", &binding.source_path);
            GetOptionalString(bindingValue, "curve", &binding.curve);
            if (binding.source_kind.empty()) binding.source_kind = BindingUsesFieldSource(binding) ? "field" : "fits_frame";
            if (binding.source_path.empty()) binding.source_path = BindingUsesFieldSource(binding) ? binding.source_signal : std::string("fits.frame.") + binding.source_signal;
            if (binding.curve.empty()) binding.curve = "smoothstep";
            if (!GetRequiredString(bindingValue, "target_path", &binding.target_path, outError)) return false;
            binding.target_path = CanonicalTargetPathInternal(binding.target_path);
            if (!IsSupportedTargetPath(binding.target_path)) {
                if (outError) *outError = "Unsupported runtime-walk FITS mapping target path: " + binding.target_path;
                return false;
            }
            if (!GetOptionalNumber(bindingValue, "input_min", &binding.input_min, outError)) return false;
            if (!GetOptionalNumber(bindingValue, "input_max", &binding.input_max, outError)) return false;
            if (!GetOptionalNumber(bindingValue, "scale", &binding.scale, outError)) return false;
            if (!GetOptionalNumber(bindingValue, "offset", &binding.offset, outError)) return false;
            if (!GetOptionalNumber(bindingValue, "weight", &binding.weight, outError)) return false;
            if (!GetOptionalNumber(bindingValue, "smoothing", &binding.smoothing, outError)) return false;
            double polarityNumber = static_cast<double>(binding.polarity);
            if (!GetOptionalNumber(bindingValue, "polarity", &polarityNumber, outError)) return false;
            binding.polarity = polarityNumber < 0.0 ? -1 : 1;
            GetOptionalString(bindingValue, "safety_class", &binding.safety_class);
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

    const json_min::Value* metadataValue = parsed.value.get("metadata");
    if (metadataValue) {
        if (!metadataValue->is_object()) {
            if (outError) *outError = "runtime-walk FITS orientation metadata must be an object";
            return false;
        }
        for (const auto& it : metadataValue->as_object()) {
            if (it.second.is_string()) {
                inputs.metadata[it.first] = it.second.as_string();
            } else if (it.second.is_number()) {
                std::ostringstream number;
                number << it.second.as_number();
                inputs.metadata[it.first] = number.str();
            } else if (it.second.is_bool()) {
                inputs.metadata[it.first] = it.second.as_bool() ? "true" : "false";
            }
        }
    }

    const auto parseSignals = [&](const json_min::Value& signalsObject,
        std::map<std::string, double>* outSignals) -> bool {
        if (!signalsObject.is_object()) {
            if (outError) *outError = "runtime-walk FITS orientation signals must be objects";
            return false;
        }
        for (const auto& it : signalsObject.as_object()) {
            if (!it.second.is_number()) {
                if (outError) *outError = "runtime-walk FITS orientation signals must be numeric";
                return false;
            }
            const double value = it.second.as_number();
            if (!std::isfinite(value)) {
                if (outError) *outError = "runtime-walk FITS orientation signals must be finite";
                return false;
            }
            (*outSignals)[it.first] = value;
        }
        return true;
    };

    const json_min::Value* signalsValue = parsed.value.get("signals");
    if (!signalsValue || !parseSignals(*signalsValue, &inputs.signals)) {
        if (outError && outError->empty()) *outError = "runtime-walk FITS orientation inputs require object field: signals";
        return false;
    }
    if (inputs.signals.empty()) {
        if (outError) *outError = "runtime-walk FITS orientation inputs require at least one signal";
        return false;
    }

    const json_min::Value* framesValue = parsed.value.get("frames");
    if (framesValue) {
        if (!framesValue->is_array()) {
            if (outError) *outError = "runtime-walk FITS orientation frames must be an array";
            return false;
        }
        for (const json_min::Value& frameValue : framesValue->as_array()) {
            if (!frameValue.is_object()) {
                if (outError) *outError = "runtime-walk FITS orientation frame entries must be objects";
                return false;
            }
            RuntimeWalkFitsSignalFrame frame;
            double frameIndex = 0.0;
            double frameT = 0.0;
            if (!GetOptionalNumber(frameValue, "frame_index", &frameIndex, outError)) return false;
            if (!GetOptionalNumber(frameValue, "t", &frameT, outError)) return false;
            if (!std::isfinite(frameT)) {
                if (outError) *outError = "runtime-walk FITS orientation frame t must be finite";
                return false;
            }
            frame.frame_index = static_cast<int>(std::lround(frameIndex));
            frame.t = frameT;
            const json_min::Value* frameSignals = frameValue.get("signals");
            if (!frameSignals || !parseSignals(*frameSignals, &frame.signals)) return false;
            if (frame.signals.empty()) {
                if (outError) *outError = "runtime-walk FITS orientation frame signals cannot be empty";
                return false;
            }
            inputs.frames.push_back(frame);
        }
        std::sort(inputs.frames.begin(), inputs.frames.end(), [](const RuntimeWalkFitsSignalFrame& lhs, const RuntimeWalkFitsSignalFrame& rhs) {
            return lhs.t < rhs.t;
        });
    }
    if (inputs.frames.empty()) {
        inputs.frames.push_back(RuntimeWalkFitsSignalFrame{0, 0.0, inputs.signals});
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

bool EvaluateRuntimeWalkFitsSignalAtT(const RuntimeWalkFitsOrientationInputs& inputs,
    const std::string& signalName,
    double t,
    double* outValue,
    int* outFrameIndex,
    std::string* outError) {
    if (outError) outError->clear();
    if (outFrameIndex) *outFrameIndex = -1;
    if (inputs.frames.empty()) {
        if (!LookupSignalMapValue(inputs.signals, signalName, outValue)) {
            if (outError) *outError = "Missing FITS frame signal: " + signalName;
            return false;
        }
        if (outFrameIndex) *outFrameIndex = 0;
        return true;
    }
    if (inputs.frames.size() == 1u || t <= inputs.frames.front().t) {
        if (!LookupSignalMapValue(inputs.frames.front().signals, signalName, outValue)) {
            if (outError) *outError = "Missing FITS frame signal: " + signalName;
            return false;
        }
        if (outFrameIndex) *outFrameIndex = inputs.frames.front().frame_index;
        return true;
    }
    if (t >= inputs.frames.back().t) {
        if (!LookupSignalMapValue(inputs.frames.back().signals, signalName, outValue)) {
            if (outError) *outError = "Missing FITS frame signal: " + signalName;
            return false;
        }
        if (outFrameIndex) *outFrameIndex = inputs.frames.back().frame_index;
        return true;
    }
    for (std::size_t index = 1; index < inputs.frames.size(); ++index) {
        const RuntimeWalkFitsSignalFrame& next = inputs.frames[index];
        if (t > next.t) continue;
        const RuntimeWalkFitsSignalFrame& prev = inputs.frames[index - 1u];
        double lhs = 0.0;
        double rhs = 0.0;
        if (!LookupSignalMapValue(prev.signals, signalName, &lhs) || !LookupSignalMapValue(next.signals, signalName, &rhs)) {
            if (outError) *outError = "Missing FITS frame signal: " + signalName;
            return false;
        }
        const double span = (std::max)(1.0e-12, next.t - prev.t);
        const double local = ClampDouble((t - prev.t) / span, 0.0, 1.0);
        if (outValue) *outValue = lhs + (rhs - lhs) * local;
        if (outFrameIndex) *outFrameIndex = prev.frame_index;
        return true;
    }
    if (outError) *outError = "Failed to evaluate FITS frame signal: " + signalName;
    return false;
}

bool ComposeRuntimeWalkFitsBindingsOverLiveBaseline(const std::vector<RuntimeWalkFitsMappingBinding>& bindings,
    const RuntimeWalkFitsOrientationInputs& inputs,
    const RuntimeWalkFitsFieldSignals& fieldSignals,
    double t,
    const ViewState& baselineView,
    const KernelParams& baselineParams,
    ViewState* outView,
    KernelParams* outParams,
    std::vector<RuntimeWalkFitsLiveBindingResult>* outResults,
    std::string* outError) {
    if (outError) outError->clear();
    if (!outView || !outParams) {
        if (outError) *outError = "Runtime-walk FITS live binding composition requires outputs";
        return false;
    }
    ViewState view = baselineView;
    KernelParams params = baselineParams;
    std::vector<RuntimeWalkFitsLiveBindingResult> results;
    for (const RuntimeWalkFitsMappingBinding& binding : bindings) {
        if (!binding.enabled) continue;
        RuntimeWalkFitsLiveBindingResult result;
        result.source_path = binding.source_path.empty() ? binding.source_signal : binding.source_path;
        result.target_path = CanonicalTargetPathInternal(binding.target_path);
        if (IsWarpTargetPath(result.target_path)) {
            result.error = "warp binding is forbidden in FITS live playback";
            results.push_back(result);
            continue;
        }
        double sourceValue = 0.0;
        int frameIndex = -1;
        bool haveSource = false;
        if (BindingUsesFieldSource(binding)) {
            haveSource = ResolveFieldSignalValue(binding, fieldSignals, &sourceValue, outError);
        } else {
            haveSource = EvaluateRuntimeWalkFitsSignalAtT(inputs, EffectiveSourceSignal(binding), t, &sourceValue, &frameIndex, outError);
        }
        if (!haveSource) {
            result.error = outError ? *outError : "source unavailable";
            results.push_back(result);
            continue;
        }
        double baselineValue = 0.0;
        if (!ReadRuntimeWalkTargetValue(result.target_path, view, params, &baselineValue)) {
            result.error = "target unavailable";
            results.push_back(result);
            continue;
        }
        double offsetValue = ResolveBindingValue(binding, sourceValue);
        double composedValue = baselineValue + offsetValue;
        bool clamped = false;
        if (binding.has_clamp) {
            const double unclamped = composedValue;
            composedValue = ClampDouble(composedValue, binding.clamp_min, binding.clamp_max);
            clamped = std::fabs(unclamped - composedValue) > 1.0e-12;
        }
        bool targetDomainClamped = false;
        composedValue = ClampRuntimeWalkTargetDomainValue(result.target_path, composedValue, &targetDomainClamped);
        clamped = clamped || targetDomainClamped;
        if (!ApplyRuntimeWalkTargetValue(result.target_path, composedValue, &view, &params, outError)) {
            result.error = outError ? *outError : "target apply failed";
            results.push_back(result);
            continue;
        }
        result.frame_index = frameIndex;
        result.t = t;
        result.source_value = sourceValue;
        result.baseline_value = baselineValue;
        result.offset_value = offsetValue;
        result.composed_value = composedValue;
        result.clamped = clamped;
        result.ok = true;
        results.push_back(result);
    }
    params.explaino_warp_strength = baselineParams.explaino_warp_strength;
    UpdateExplainoPolynomial(view, params, nullptr);
    SyncViewUiFromHp(view);
    *outView = view;
    *outParams = params;
    if (outResults) *outResults = std::move(results);
    return true;
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
        if (BindingUsesFieldSource(binding)) continue;
        if (!(binding.target_selector == profile->target_selector || binding.target_selector == "all")) continue;
        double signalValue = 0.0;
        if (!LookupSignal(inputs, EffectiveSourceSignal(binding), &signalValue, outError)) return false;
        double mappedValue = ResolveBindingValue(binding, signalValue);
        if (binding.has_clamp) {
            mappedValue = ClampDouble(mappedValue, binding.clamp_min, binding.clamp_max);
        }
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
    const RuntimeWalkTransportSynthesisOptions& options,
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
    const std::size_t requestedSegments = std::max<std::size_t>(8u, options.sample_count > 1u ? options.sample_count - 1u : 8u);
    bundle.samples.reserve(requestedSegments + 1u);
    for (std::size_t index = 0; index <= requestedSegments; ++index) {
        const double t = static_cast<double>(index) / static_cast<double>(requestedSegments);
        std::ostringstream id;
        id << "loop_" << std::setw(2) << std::setfill('0') << index;
        bundle.samples.push_back(RuntimeWalkBundleSample{id.str(), t, BuildClosedLoopTransportSample(inputs, anchor, options, t)});
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
