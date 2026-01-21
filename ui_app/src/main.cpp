#include "fractal_types.h"

#include <Windows.h>
#include <d3d11.h>
#include <tchar.h>

#include <vector>
#include <array>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>


#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"

#include "json_min.h"
#include "ui_schema.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

// Forward declare the CUDA renderer.
bool RenderFractalCUDA(
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    uint32_t* outRGBA,
    uint8_t* outMask,
    RenderStats* outStats,
    const char** outError);

static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

static ID3D11Texture2D* g_fractalTexture = nullptr;
static ID3D11ShaderResourceView* g_fractalSRV = nullptr;

static ID3D11Texture2D* g_lensTexture = nullptr;
static ID3D11ShaderResourceView* g_lensSRV = nullptr;

static ID3D11Texture2D* g_maskTexture = nullptr;
static ID3D11ShaderResourceView* g_maskSRV = nullptr;

static float SaturateF(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

static float FractF(float v) {
    return v - floorf(v);
}

static ImVec2 CatmullRom(ImVec2 p0, ImVec2 p1, ImVec2 p2, ImVec2 p3, float t) {
    // Standard Catmull–Rom spline (uniform, tension=0.5) that passes through p1 and p2.
    float t2 = t * t;
    float t3 = t2 * t;
    ImVec2 out;
    out.x = 0.5f * ((2.0f * p1.x) + (-p0.x + p2.x) * t + (2.0f * p0.x - 5.0f * p1.x + 4.0f * p2.x - p3.x) * t2 + (-p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x) * t3);
    out.y = 0.5f * ((2.0f * p1.y) + (-p0.y + p2.y) * t + (2.0f * p0.y - 5.0f * p1.y + 4.0f * p2.y - p3.y) * t2 + (-p0.y + 3.0f * p1.y - 3.0f * p2.y + p3.y) * t3);
    return out;
}

static void CleanupRenderTarget() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}
static std::string GetExeDir() {
    char buf[MAX_PATH] = {};
    DWORD n = GetModuleFileNameA(nullptr, buf, (DWORD)sizeof(buf));
    if (n == 0 || n >= sizeof(buf)) return std::string();
    std::string path(buf);
    size_t pos = path.find_last_of("\\/");
    if (pos == std::string::npos) return std::string();
    path.resize(pos);
    return path;
}
static std::string JoinPath(const std::string& a, const char* b) {
    if (a.empty()) return std::string(b);
    if (!b || !*b) return a;
    char last = a.back();
    if (last == '\\' || last == '/') return a + b;
    return a + "\\" + b;
}

static bool WriteTextFile(const std::string& path, const std::string& text) {
    std::ofstream f(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!f) return false;
    f.write(text.data(), (std::streamsize)text.size());
    return (bool)f;
}

static bool WriteTextFileAtomicReplace(const std::string& path, const std::string& text) {
    // Write to a sibling temp file then atomically replace the target.
    // This avoids partial reads when another process polls the file.
    const std::string tmp = path + ".tmp";
    if (!WriteTextFile(tmp, text)) {
        return false;
    }
    // MOVEFILE_WRITE_THROUGH reduces the chance of consumer seeing stale metadata.
    if (!MoveFileExA(tmp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        DeleteFileA(tmp.c_str());
        return false;
    }
    return true;
}

static bool ReadTextFile(const std::string& path, std::string& outText) {
    std::ifstream f(path, std::ios::in | std::ios::binary);
    if (!f) return false;
    std::ostringstream ss;
    ss << f.rdbuf();
    outText = ss.str();
    return true;
}

static std::string NormalizePath(const std::string& path) {
    if (path.empty()) return path;
    char buf[4096] = {};
    DWORD n = GetFullPathNameA(path.c_str(), (DWORD)sizeof(buf), buf, nullptr);
    if (n == 0 || n >= sizeof(buf)) return path;
    return std::string(buf);
}

static bool EnsureDirectoryRecursive(const std::string& path) {
    if (path.empty()) return false;

    auto isSep = [](char ch) { return ch == '\\' || ch == '/'; };

    std::string cur;
    cur.reserve(path.size());
    size_t i = 0;

    // Handle "C:\\" prefix.
    if (path.size() >= 3 && ((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z')) && path[1] == ':' && isSep(path[2])) {
        cur.assign(path.begin(), path.begin() + 3);
        i = 3;
    }

    for (; i < path.size(); ++i) {
        char ch = path[i];
        cur.push_back(ch);
        if (isSep(ch) || i + 1 == path.size()) {
            // Trim trailing separator.
            std::string dir = cur;
            while (!dir.empty() && isSep(dir.back())) dir.pop_back();
            if (dir.empty()) continue;

            DWORD attr = GetFileAttributesA(dir.c_str());
            if (attr != INVALID_FILE_ATTRIBUTES) {
                if (attr & FILE_ATTRIBUTE_DIRECTORY) continue;
                return false;
            }

            if (!CreateDirectoryA(dir.c_str(), nullptr)) {
                DWORD err = GetLastError();
                if (err != ERROR_ALREADY_EXISTS) return false;
            }
        }
    }
    return true;
}

static bool WriteBmp32BGRA(const std::string& path, const uint32_t* rgba, int width, int height) {
    if (!rgba || width <= 0 || height <= 0) return false;

#pragma pack(push, 1)
    struct BmpFileHeader {
        uint16_t bfType;
        uint32_t bfSize;
        uint16_t bfReserved1;
        uint16_t bfReserved2;
        uint32_t bfOffBits;
    };
    struct BmpInfoHeader {
        uint32_t biSize;
        int32_t biWidth;
        int32_t biHeight;
        uint16_t biPlanes;
        uint16_t biBitCount;
        uint32_t biCompression;
        uint32_t biSizeImage;
        int32_t biXPelsPerMeter;
        int32_t biYPelsPerMeter;
        uint32_t biClrUsed;
        uint32_t biClrImportant;
    };
#pragma pack(pop)

    const uint32_t pixelBytes = 4;
    const uint32_t rowBytes = (uint32_t)width * pixelBytes;
    const uint32_t dataBytes = rowBytes * (uint32_t)height;

    const uint32_t fileHeaderBytes = 14;
    const uint32_t infoHeaderBytes = 40;

    BmpFileHeader fh{};
    fh.bfType = 0x4D42; // 'BM'
    fh.bfOffBits = fileHeaderBytes + infoHeaderBytes;
    fh.bfSize = fh.bfOffBits + dataBytes;

    BmpInfoHeader ih{};
    ih.biSize = infoHeaderBytes;
    ih.biWidth = (int32_t)width;
    ih.biHeight = -(int32_t)height; // top-down
    ih.biPlanes = 1;
    ih.biBitCount = 32;
    ih.biCompression = 0; // BI_RGB
    ih.biSizeImage = dataBytes;
    ih.biXPelsPerMeter = 2835;
    ih.biYPelsPerMeter = 2835;

    std::ofstream f(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!f) return false;
    f.write(reinterpret_cast<const char*>(&fh), sizeof(fh));
    f.write(reinterpret_cast<const char*>(&ih), sizeof(ih));

    std::vector<uint8_t> row;
    row.resize(rowBytes);
    for (int y = 0; y < height; ++y) {
        const uint32_t* src = rgba + (size_t)y * (size_t)width;
        for (int x = 0; x < width; ++x) {
            uint32_t p = src[x];
            uint8_t r = (uint8_t)(p & 0xFF);
            uint8_t g = (uint8_t)((p >> 8) & 0xFF);
            uint8_t b = (uint8_t)((p >> 16) & 0xFF);
            uint8_t a = (uint8_t)((p >> 24) & 0xFF);
            size_t o = (size_t)x * 4;
            row[o + 0] = b;
            row[o + 1] = g;
            row[o + 2] = r;
            row[o + 3] = a;
        }
        f.write(reinterpret_cast<const char*>(row.data()), (std::streamsize)row.size());
    }
    return (bool)f;
}

static std::string JsonEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 16);
    for (unsigned char ch : s) {
        switch (ch) {
        case '\\': out += "\\\\"; break;
        case '"': out += "\\\""; break;
        case '\b': out += "\\b"; break;
        case '\f': out += "\\f"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if (ch < 0x20) {
                char buf[7];
                sprintf_s(buf, "\\u%04x", (unsigned int)ch);
                out += buf;
            } else {
                out.push_back((char)ch);
            }
            break;
        }
    }
    return out;
}

static bool g_uiErrorReported = false;
static bool g_uiSoftErrors = false;
static bool g_uiSchemaDisabled = false;
static bool g_uiNoMessageBox = false;
static std::string g_uiLastErrorStage;
static std::string g_uiLastErrorPath;
static std::string g_uiLastErrorDetail;
static void DrawUiErrorOverlayIfAny() {
    if (!g_uiSoftErrors) return;
    if (g_uiLastErrorStage.empty() && g_uiLastErrorDetail.empty()) return;

    const float pad = 10.0f;
    const ImVec2 pos = ImVec2(pad, pad);
    const ImVec2 pivot = ImVec2(0.0f, 0.0f);
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, pivot);
    ImGui::SetNextWindowBgAlpha(0.85f);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                             ImGuiWindowFlags_NoNav;
    if (ImGui::Begin("UI Error", nullptr, flags)) {
        ImGui::TextUnformatted("UI schema error (soft mode)");
        if (!g_uiLastErrorStage.empty()) ImGui::Text("stage: %s", g_uiLastErrorStage.c_str());
        if (!g_uiLastErrorPath.empty()) ImGui::Text("path:  %s", g_uiLastErrorPath.c_str());
        if (!g_uiLastErrorDetail.empty()) ImGui::TextWrapped("%s", g_uiLastErrorDetail.c_str());
        ImGui::Separator();
        ImGui::TextUnformatted("See ..\\ui\\last_ui_error.json for details.");
        ImGui::TextUnformatted("Run with --validate-ui for fast checks.");
    }
    ImGui::End();
}

static void ReportUiErrorOnce(const char* stage, const std::string& schemaPath, const std::string& failingPath, const std::string& detail) {
    if (g_uiErrorReported) return;
    g_uiErrorReported = true;

    auto canonicalize_path = [](const std::string& p) -> std::string {
        if (p.empty()) return p;
        HANDLE h = CreateFileA(
            p.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
            nullptr);
        if (h == INVALID_HANDLE_VALUE) return p;

        char buf[4096];
        DWORD n = GetFinalPathNameByHandleA(h, buf, (DWORD)sizeof(buf), FILE_NAME_NORMALIZED);
        CloseHandle(h);
        if (n == 0 || n >= sizeof(buf)) return p;

        std::string out(buf, (size_t)n);
        const char* prefix = "\\\\?\\";
        if (out.rfind(prefix, 0) == 0) {
            out.erase(0, 4);
        }
        const char* uncPrefix = "UNC\\";
        if (out.rfind(uncPrefix, 0) == 0) {
            out = std::string("\\\\") + out.substr(4);
        }
        return out;
    };

    const std::string schemaPathCanonical = canonicalize_path(schemaPath);

    const std::string exeDir = GetExeDir();
    const std::string outPrimary = JoinPath(exeDir, "..\\ui\\last_ui_error.json");
    const std::string outSecondary = JoinPath(exeDir, "ui\\last_ui_error.json");

    const std::string json = std::string("{\n")
        + "  \"stage\": \"" + JsonEscape(stage ? stage : "ui") + "\",\n"
        + "  \"schema_path\": \"" + JsonEscape(schemaPathCanonical) + "\",\n"
        + "  \"failing_path\": \"" + JsonEscape(failingPath) + "\",\n"
        + "  \"detail\": \"" + JsonEscape(detail) + "\"\n"
        + "}\n";

    if (!WriteTextFile(outPrimary, json)) {
        WriteTextFile(outSecondary, json);
    }
}

static void FailFastUiError(const char* stage, const std::string& schemaPath, const std::string& failingPath, const std::string& detail) {
    ReportUiErrorOnce(stage, schemaPath, failingPath, detail);
    if (!g_uiNoMessageBox) {
        MessageBoxA(nullptr, detail.c_str(), "UI error", MB_OK | MB_ICONERROR);
    }
    ExitProcess(1);
}

static bool UiErrorOrFail(const char* stage, const std::string& schemaPath, const std::string& failingPath, const std::string& detail) {
    if (!g_uiSoftErrors) {
        FailFastUiError(stage, schemaPath, failingPath, detail);
        return false;
    }
    ReportUiErrorOnce(stage, schemaPath, failingPath, detail);
    g_uiSchemaDisabled = true;
    g_uiLastErrorStage = stage ? stage : "ui";
    g_uiLastErrorPath = failingPath;
    g_uiLastErrorDetail = detail;
    return false;
}

static std::vector<std::string> GetCommandLineArgsUtf8() {
    std::vector<std::string> out;
    int argc = 0;
    LPWSTR* argvW = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argvW) return out;
    out.reserve((size_t)argc);
    for (int i = 0; i < argc; ++i) {
        const wchar_t* w = argvW[i];
        int needed = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
        if (needed <= 0) {
            out.emplace_back();
            continue;
        }
        std::string s;
        s.resize((size_t)needed - 1);
        WideCharToMultiByte(CP_UTF8, 0, w, -1, s.data(), needed, nullptr, nullptr);
        out.push_back(std::move(s));
    }
    LocalFree(argvW);
    return out;
}

static bool HasArg(const std::vector<std::string>& args, const char* needle) {
    if (!needle) return false;
    for (const auto& a : args) {
        if (_stricmp(a.c_str(), needle) == 0) return true;
    }
    return false;
}

static bool TryGetArgValue(const std::vector<std::string>& args, const char* key, std::string& outValue) {
    outValue.clear();
    if (!key) return false;
    const std::string k = key;

    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& a = args[i];

        // --key value
        if (_stricmp(a.c_str(), k.c_str()) == 0) {
            if (i + 1 < args.size()) {
                outValue = args[i + 1];
                return !outValue.empty();
            }
            return false;
        }

        // --key=value
        if (a.size() > k.size() + 1) {
            bool match = true;
            for (size_t j = 0; j < k.size(); ++j) {
                const char ca = a[j];
                const char cb = k[j];
                if (tolower((unsigned char)ca) != tolower((unsigned char)cb)) {
                    match = false;
                    break;
                }
            }
            if (match && a[k.size()] == '=') {
                outValue = a.substr(k.size() + 1);
                return !outValue.empty();
            }
        }
    }

    return false;
}

// Forward declarations (used by schema loader + state import/export helpers declared early).
static std::string ReadTextFile(const char* path);
static void SyncViewUiFromHp(ViewState& view);

static bool TryLoadSchemaFromCandidates(const std::vector<std::string>& candidates, UISchema& outSchema, std::string& outSchemaPath, std::string& outStage, std::string& outDetail) {
    bool sawFailure = false;
    std::string firstFailPath;
    std::string firstFailStage;
    std::string firstFailDetail;

    for (const auto& cand : candidates) {
        std::string text = ReadTextFile(cand.c_str());
        if (text.empty()) continue;

        auto pr = json_min::Parse(text);
        if (!pr.error.empty()) {
            if (!sawFailure) {
                sawFailure = true;
                firstFailPath = cand;
                firstFailStage = "schema.json_parse";
                firstFailDetail = pr.error;
            }
            continue;
        }

        auto lr = LoadUISchemaFromJson(pr.value);
        if (!lr.error.empty()) {
            if (!sawFailure) {
                sawFailure = true;
                firstFailPath = cand;
                firstFailStage = "schema.decode";
                firstFailDetail = lr.error;
            }
            continue;
        }

        outSchema = std::move(lr.schema);
        outSchemaPath = cand;
        outStage.clear();
        outDetail.clear();
        return true;
    }

    if (sawFailure) {
        outSchemaPath = firstFailPath;
        outStage = firstFailStage;
        outDetail = firstFailDetail;
        return false;
    }

    outSchemaPath = candidates.empty() ? std::string() : candidates[0];
    outStage = "schema.missing";
    std::ostringstream oss;
    oss << "Schema missing/unreadable. Tried:\n";
    for (const auto& c : candidates) oss << "  " << c << "\n";
    outDetail = oss.str();
    return false;
}

static bool SchemaHasBinding(const UISchema& schema, const char* kind, const char* path) {
    if (!kind || !path) return false;
    for (const auto& p : schema.panels) {
        for (const auto& c : p.controls) {
            if (!c.has_binding) continue;
            if (_stricmp(c.binding.kind.c_str(), kind) != 0) continue;
            if (_stricmp(c.binding.path.c_str(), path) != 0) continue;
            return true;
        }
    }
    return false;
}

static const char* FractalTypeId(FractalType t) {
    switch (t) {
    case FractalType::newton: return "newton";
    case FractalType::nova: return "nova";
    case FractalType::mandelbrot: return "mandelbrot";
    case FractalType::julia: return "julia";
    case FractalType::burning_ship: return "burning_ship";
    case FractalType::multibrot: return "multibrot";
    case FractalType::phoenix: return "phoenix";
    case FractalType::explaino: return "explaino";
    case FractalType::explaino_y: return "explaino_y";
    case FractalType::explaino_fp: return "explaino_fp";
    }
    return "";
}

static bool FractalTypeFromId(const std::string& id, FractalType& out) {
    if (id == "newton") { out = FractalType::newton; return true; }
    if (id == "nova") { out = FractalType::nova; return true; }
    if (id == "mandelbrot") { out = FractalType::mandelbrot; return true; }
    if (id == "julia") { out = FractalType::julia; return true; }
    if (id == "burning_ship") { out = FractalType::burning_ship; return true; }
    if (id == "multibrot") { out = FractalType::multibrot; return true; }
    if (id == "phoenix") { out = FractalType::phoenix; return true; }
    if (id == "explaino") { out = FractalType::explaino; return true; }
    if (id == "explaino_y") { out = FractalType::explaino_y; return true; }
    if (id == "explaino_fp") { out = FractalType::explaino_fp; return true; }
    return false;
}

static const char* CameraBehaviorId(CameraBehavior b) {
    switch (b) {
    case CameraBehavior::manual: return "manual";
    case CameraBehavior::complexity: return "complexity";
    case CameraBehavior::orbit: return "orbit";
    case CameraBehavior::entropy: return "entropy";
    case CameraBehavior::off: return "off";
    }
    return "";
}

static bool CameraBehaviorFromId(const std::string& id, CameraBehavior& out) {
    if (id == "manual") { out = CameraBehavior::manual; return true; }
    if (id == "complexity") { out = CameraBehavior::complexity; return true; }
    if (id == "orbit") { out = CameraBehavior::orbit; return true; }
    if (id == "entropy") { out = CameraBehavior::entropy; return true; }
    if (id == "off") { out = CameraBehavior::off; return true; }
    return false;
}

static const char* PolyKindId(PolyKind k) {
    switch (k) {
    case PolyKind::z3_minus_1: return "z3_minus_1";
    case PolyKind::z4_minus_1: return "z4_minus_1";
    case PolyKind::custom: return "custom";
    }
    return "";
}

static bool PolyKindFromId(const std::string& id, PolyKind& out) {
    if (id == "z3_minus_1") { out = PolyKind::z3_minus_1; return true; }
    if (id == "z4_minus_1") { out = PolyKind::z4_minus_1; return true; }
    if (id == "custom") { out = PolyKind::custom; return true; }
    return false;
}

static const char* ColoringModeId(ColoringMode m) {
    switch (m) {
    case ColoringMode::root_basin: return "root_basin";
    case ColoringMode::iteration_count: return "iteration_count";
    case ColoringMode::smooth_escape: return "smooth_escape";
    case ColoringMode::joy_basins: return "joy_basins";
    }
    return "";
}

static bool ColoringModeFromId(const std::string& id, ColoringMode& out) {
    if (id == "root_basin") { out = ColoringMode::root_basin; return true; }
    if (id == "iteration_count") { out = ColoringMode::iteration_count; return true; }
    if (id == "smooth_escape") { out = ColoringMode::smooth_escape; return true; }
    if (id == "joy_basins") { out = ColoringMode::joy_basins; return true; }
    return false;
}

static std::string GetStatePrimaryPath() {
    const std::string exeDir = GetExeDir();
    return JoinPath(exeDir, "..\\ui\\last_state.json");
}

static std::string GetStateSecondaryPath() {
    const std::string exeDir = GetExeDir();
    return JoinPath(exeDir, "ui\\last_state.json");
}

static std::string GetDiagnosticsPrimaryDir() {
    const std::string exeDir = GetExeDir();
    return JoinPath(exeDir, "..\\ui\\diagnostics\\last");
}

static std::string GetDiagnosticsSecondaryDir() {
    const std::string exeDir = GetExeDir();
    return JoinPath(exeDir, "ui\\diagnostics\\last");
}

static void ComputeSignedDistanceSdfChamfer(const uint8_t* mask, int w, int h, float maxAbsPx, std::vector<uint32_t>& outRgba);

static bool WriteLensSdfBmpLastBundle(const std::string& schemaPath,
                                     const uint8_t* mask, int width, int height,
                                     float maxAbsPx,
                                     const char* outName) {
    if (!mask || width <= 0 || height <= 0) return false;

    std::string baseDir = NormalizePath(GetDiagnosticsPrimaryDir());
    if (!EnsureDirectoryRecursive(baseDir)) {
        baseDir = NormalizePath(GetDiagnosticsSecondaryDir());
        if (!EnsureDirectoryRecursive(baseDir)) {
            return UiErrorOrFail("diag.mkdir_failed", schemaPath, baseDir, "Failed to create diagnostics folder");
        }
    }

    std::vector<uint32_t> sdf;
    sdf.resize((size_t)width * (size_t)height);
    ComputeSignedDistanceSdfChamfer(mask, width, height, maxAbsPx, sdf);

    const std::string outPath = JoinPath(baseDir, outName ? outName : "lens_sdf.bmp");
    if (!WriteBmp32BGRA(outPath, sdf.data(), width, height)) {
        return UiErrorOrFail("diag.sdf_write_failed", schemaPath, outPath, "Failed to write diagnostics lens SDF BMP");
    }
    return true;
}

static std::string GetDefaultSeedTextPath() {
    const std::string exeDir = GetExeDir();
    // repo_root/docs/seeds/explaino_infinity_prompt.md relative to ui_app
    // (Avoid using docs/last_response.md as a default; it's an output surface and can be stale/misleading.)
    return NormalizePath(JoinPath(exeDir, "..\\..\\docs\\seeds\\explaino_infinity_prompt.md"));
}

static std::string BuildStateJson(const ViewState& view, const KernelParams& params, const RenderSettings& render, const LensSettings& lens) {
    std::ostringstream ss;
    ss.setf(std::ios::fixed);
    ss << std::setprecision(17);

    ss << "{\n";
    ss << "  \"version\": 1,\n";
    ss << "  \"camera\": {\n";
    ss << "    \"center_hp_x\": " << view.center_hp_x << ",\n";
    ss << "    \"center_hp_y\": " << view.center_hp_y << ",\n";
    ss << "    \"log2_zoom\": " << view.log2_zoom << ",\n";
    ss << "    \"rotation_degrees\": " << (double)view.rotation_degrees << ",\n";
    ss << "    \"camera_behavior\": \"" << CameraBehaviorId(view.camera_behavior) << "\",\n";
    ss << "    \"auto_dive\": " << (view.auto_dive ? "true" : "false") << ",\n";
    ss << "    \"dive_speed\": " << (double)view.dive_speed << "\n";
    ss << "  },\n";
    ss << "  \"fractal\": {\n";
    ss << "    \"fractal_type\": \"" << FractalTypeId(view.fractal_type) << "\",\n";
    ss << "    \"explaino_alive\": " << (view.explaino_alive ? "true" : "false") << ",\n";
    ss << "    \"explaino_seed_tween\": " << (view.explaino_seed_tween ? "true" : "false") << ",\n";
    ss << "    \"explaino_alive_speed\": " << (double)view.explaino_alive_speed << ",\n";
    ss << "    \"explaino_phase\": " << (double)view.explaino_phase << ",\n";
    ss << "    \"explaino_seed_drift\": " << (double)view.explaino_seed_drift << "\n";
    ss << "  },\n";
    ss << "  \"lens\": {\n";
    ss << "    \"enabled\": " << (lens.enabled ? "true" : "false") << ",\n";
    ss << "    \"downsample\": " << (double)lens.downsample << "\n";
    ss << "  },\n";
    ss << "  \"params\": {\n";
    ss << "    \"max_iter\": " << (double)params.max_iter << ",\n";
    ss << "    \"epsilon\": " << (double)params.epsilon << ",\n";
    ss << "    \"nova_alpha\": " << (double)params.nova_alpha << ",\n";
    ss << "    \"phoenix_p_real\": " << (double)params.phoenix_p_real << ",\n";
    ss << "    \"phoenix_p_imag\": " << (double)params.phoenix_p_imag << ",\n";
    ss << "    \"poly_kind\": \"" << PolyKindId(params.poly_kind) << "\",\n";
    ss << "    \"poly_coeffs\": [";
    for (int i = 0; i < 5; i++) {
        if (i) ss << ", ";
        ss << (double)params.poly_coeffs[i];
    }
    ss << "],\n";
    ss << "    \"multibrot_power\": " << (double)params.multibrot_power << ",\n";
    ss << "    \"coloring_mode\": \"" << ColoringModeId(params.coloring_mode) << "\",\n";
    ss << "    \"exposure\": " << (double)params.exposure << ",\n";
    ss << "    \"color_saturation\": " << (double)params.color_saturation << ",\n";
    ss << "    \"color_contrast\": " << (double)params.color_contrast << ",\n";
    ss << "    \"color_tint_r\": " << (double)params.color_tint_r << ",\n";
    ss << "    \"color_tint_g\": " << (double)params.color_tint_g << ",\n";
    ss << "    \"color_tint_b\": " << (double)params.color_tint_b << ",\n";
    ss << "    \"explaino_seed\": " << (double)params.explaino_seed << ",\n";
    ss << "    \"explaino_warp_strength\": " << (double)params.explaino_warp_strength << "\n";
    ss << "  },\n";
    ss << "  \"engine\": {\n";
    ss << "    \"resolution\": [" << (double)render.resolution.x << ", " << (double)render.resolution.y << "],\n";
    ss << "    \"block_size\": " << (double)render.block_size << ",\n";
    ss << "    \"device_id\": " << (double)render.device_id << ",\n";
    ss << "    \"benchmark\": " << (render.benchmark ? "true" : "false") << "\n";
    ss << "  }\n";
    ss << "}\n";
    return ss.str();
}

static bool CaptureDiagnosticsLastBundle(const std::string& schemaPath, const ViewState& view, const KernelParams& params, const RenderSettings& render, const LensSettings& lens,
                                        const uint32_t* rgba, int width, int height) {
    std::string json = BuildStateJson(view, params, render, lens);
    if (!schemaPath.empty() && !json.empty() && json[0] == '{') {
        json = std::string("{\n  \"schema_path\": \"") + JsonEscape(schemaPath) + "\"," + json.substr(1);
    }

    std::string baseDir = NormalizePath(GetDiagnosticsPrimaryDir());
    if (!EnsureDirectoryRecursive(baseDir)) {
        baseDir = NormalizePath(GetDiagnosticsSecondaryDir());
        if (!EnsureDirectoryRecursive(baseDir)) {
            return UiErrorOrFail("diag.mkdir_failed", schemaPath, baseDir, "Failed to create diagnostics folder");
        }
    }

    const std::string statePath = JoinPath(baseDir, "state.json");
    if (!WriteTextFile(statePath, json)) {
        return UiErrorOrFail("diag.state_write_failed", schemaPath, statePath, "Failed to write diagnostics state.json");
    }

    const std::string framePath = JoinPath(baseDir, "frame.bmp");
    if (!WriteBmp32BGRA(framePath, rgba, width, height)) {
        return UiErrorOrFail("diag.frame_write_failed", schemaPath, framePath, "Failed to write diagnostics frame.bmp");
    }

    // Optional: copy the last UI error artifact alongside, if present.
    {
        const std::string exeDir = GetExeDir();
        const std::string src1 = JoinPath(exeDir, "..\\ui\\last_ui_error.json");
        const std::string src2 = JoinPath(exeDir, "ui\\last_ui_error.json");
        const std::string dst = JoinPath(baseDir, "ui_error.json");
        if (GetFileAttributesA(src1.c_str()) != INVALID_FILE_ATTRIBUTES) {
            CopyFileA(src1.c_str(), dst.c_str(), FALSE);
        } else if (GetFileAttributesA(src2.c_str()) != INVALID_FILE_ATTRIBUTES) {
            CopyFileA(src2.c_str(), dst.c_str(), FALSE);
        }
    }

    return true;
}

static void DownsampleMask2x(const uint8_t* inMask, int inW, int inH, std::vector<uint8_t>& outMask, int& outW, int& outH) {
    outW = (inW + 1) / 2;
    outH = (inH + 1) / 2;
    outMask.assign((size_t)outW * (size_t)outH, 0);
    for (int y = 0; y < outH; ++y) {
        int sy = y * 2;
        if (sy >= inH) sy = inH - 1;
        for (int x = 0; x < outW; ++x) {
            int sx = x * 2;
            if (sx >= inW) sx = inW - 1;
            outMask[(size_t)y * (size_t)outW + (size_t)x] = inMask[(size_t)sy * (size_t)inW + (size_t)sx];
        }
    }
}

static int NormalizeLensDownsamplePow2(int v) {
    if (v <= 1) return 1;
    if (v <= 2) return 2;
    if (v <= 4) return 4;
    return 8;
}

static void DownsampleMaskPow2(const uint8_t* inMask, int inW, int inH, int downsample, std::vector<uint8_t>& outMask, int& outW, int& outH) {
    const int ds = NormalizeLensDownsamplePow2(downsample);
    if (ds <= 1) {
        outW = inW;
        outH = inH;
        outMask.assign(inMask, inMask + (size_t)inW * (size_t)inH);
        return;
    }

    // Iterative 2x reduction.
    std::vector<uint8_t> tmpA;
    std::vector<uint8_t> tmpB;
    int w = inW;
    int h = inH;
    const uint8_t* cur = inMask;

    int steps = 0;
    for (int x = ds; x > 1; x >>= 1) steps++;
    for (int i = 0; i < steps; i++) {
        std::vector<uint8_t>& dst = (i % 2 == 0) ? tmpA : tmpB;
        int nw = 0, nh = 0;
        DownsampleMask2x(cur, w, h, dst, nw, nh);
        cur = dst.data();
        w = nw;
        h = nh;
    }

    outW = w;
    outH = h;
    const size_t n = (size_t)outW * (size_t)outH;
    outMask.assign(cur, cur + n);
}

static void ComputeSignedDistanceSdfChamfer(const uint8_t* mask, int w, int h, float maxAbsPx, std::vector<uint32_t>& outRgba) {
    if (!mask || w <= 0 || h <= 0) {
        outRgba.clear();
        return;
    }

    const float INF = 1.0e9f;
    const float W1 = 1.0f;
    const float W2 = 1.41421356f;

    const size_t n = (size_t)w * (size_t)h;
    std::vector<float> dToInside(n, INF);
    std::vector<float> dToOutside(n, INF);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            size_t i = (size_t)y * (size_t)w + (size_t)x;
            bool inside = mask[i] > 127;
            if (inside) dToInside[i] = 0.0f;
            else dToOutside[i] = 0.0f;
        }
    }

    auto relaxForward = [&](std::vector<float>& d) {
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                size_t i = (size_t)y * (size_t)w + (size_t)x;
                float best = d[i];
                if (x > 0) best = fminf(best, d[i - 1] + W1);
                if (y > 0) best = fminf(best, d[i - (size_t)w] + W1);
                if (x > 0 && y > 0) best = fminf(best, d[i - (size_t)w - 1] + W2);
                if (x + 1 < w && y > 0) best = fminf(best, d[i - (size_t)w + 1] + W2);
                d[i] = best;
            }
        }
    };

    auto relaxBackward = [&](std::vector<float>& d) {
        for (int y = h - 1; y >= 0; --y) {
            for (int x = w - 1; x >= 0; --x) {
                size_t i = (size_t)y * (size_t)w + (size_t)x;
                float best = d[i];
                if (x + 1 < w) best = fminf(best, d[i + 1] + W1);
                if (y + 1 < h) best = fminf(best, d[i + (size_t)w] + W1);
                if (x + 1 < w && y + 1 < h) best = fminf(best, d[i + (size_t)w + 1] + W2);
                if (x > 0 && y + 1 < h) best = fminf(best, d[i + (size_t)w - 1] + W2);
                d[i] = best;
            }
        }
    };

    relaxForward(dToInside);
    relaxBackward(dToInside);
    relaxForward(dToOutside);
    relaxBackward(dToOutside);

    outRgba.resize(n);
    float denom = fmaxf(1.0f, maxAbsPx);
    for (size_t i = 0; i < n; ++i) {
        float signedPx = dToInside[i] - dToOutside[i]; // outside positive, inside negative
        float v = 0.5f + (signedPx / (2.0f * denom));
        if (v < 0.0f) v = 0.0f;
        if (v > 1.0f) v = 1.0f;
        uint8_t c = (uint8_t)(v * 255.0f + 0.5f);

        // Subtle contour at the boundary.
        if (fabsf(signedPx) < 0.75f) {
            outRgba[i] = (uint32_t)(255) | ((uint32_t)64 << 8) | ((uint32_t)64 << 16) | ((uint32_t)255 << 24);
        } else {
            outRgba[i] = (uint32_t)c | ((uint32_t)c << 8) | ((uint32_t)c << 16) | ((uint32_t)255 << 24);
        }
    }
}

static bool SampleSignedDistanceSdfChamfer(const uint8_t* mask, int w, int h, int x, int y, float& outSignedPx, bool& outInside) {
    if (!mask || w <= 0 || h <= 0) {
        return false;
    }
    if (x < 0 || y < 0 || x >= w || y >= h) {
        return false;
    }

    const float INF = 1.0e9f;
    const float W1 = 1.0f;
    const float W2 = 1.41421356f;

    const size_t n = (size_t)w * (size_t)h;
    std::vector<float> dToInside(n, INF);
    std::vector<float> dToOutside(n, INF);

    for (int yy = 0; yy < h; ++yy) {
        for (int xx = 0; xx < w; ++xx) {
            size_t i = (size_t)yy * (size_t)w + (size_t)xx;
            bool inside = mask[i] > 127;
            if (inside) dToInside[i] = 0.0f;
            else dToOutside[i] = 0.0f;
        }
    }

    auto relaxForward = [&](std::vector<float>& d) {
        for (int yy = 0; yy < h; ++yy) {
            for (int xx = 0; xx < w; ++xx) {
                size_t i = (size_t)yy * (size_t)w + (size_t)xx;
                float best = d[i];
                if (xx > 0) best = fminf(best, d[i - 1] + W1);
                if (yy > 0) best = fminf(best, d[i - (size_t)w] + W1);
                if (xx > 0 && yy > 0) best = fminf(best, d[i - (size_t)w - 1] + W2);
                if (xx + 1 < w && yy > 0) best = fminf(best, d[i - (size_t)w + 1] + W2);
                d[i] = best;
            }
        }
    };

    auto relaxBackward = [&](std::vector<float>& d) {
        for (int yy = h - 1; yy >= 0; --yy) {
            for (int xx = w - 1; xx >= 0; --xx) {
                size_t i = (size_t)yy * (size_t)w + (size_t)xx;
                float best = d[i];
                if (xx + 1 < w) best = fminf(best, d[i + 1] + W1);
                if (yy + 1 < h) best = fminf(best, d[i + (size_t)w] + W1);
                if (xx + 1 < w && yy + 1 < h) best = fminf(best, d[i + (size_t)w + 1] + W2);
                if (xx > 0 && yy + 1 < h) best = fminf(best, d[i + (size_t)w - 1] + W2);
                d[i] = best;
            }
        }
    };

    relaxForward(dToInside);
    relaxBackward(dToInside);
    relaxForward(dToOutside);
    relaxBackward(dToOutside);

    const size_t idx = (size_t)y * (size_t)w + (size_t)x;
    outSignedPx = dToInside[idx] - dToOutside[idx]; // outside positive, inside negative
    outInside = mask[idx] > 127;
    return true;
}

static uint32_t Fnv1a32(const void* data, size_t sizeBytes) {
    const uint8_t* p = (const uint8_t*)data;
    uint32_t h = 2166136261u;
    for (size_t i = 0; i < sizeBytes; i++) {
        h ^= (uint32_t)p[i];
        h *= 16777619u;
    }
    return h;
}

static void ComputeConversationSpectrum8(const std::string& text, uint32_t outSpectrum[8]) {
    // Not a true FFT; this is an intentionally cheap, deterministic "spectral" sketch.
    // We hash 8 interleaved byte streams to get 8 stable components.
    uint32_t acc[8] = { 2166136261u,2166136261u,2166136261u,2166136261u,2166136261u,2166136261u,2166136261u,2166136261u };
    for (size_t i = 0; i < text.size(); i++) {
        uint32_t& h = acc[i & 7u];
        h ^= (uint8_t)text[i];
        h *= 16777619u;
    }
    for (int i = 0; i < 8; i++) outSpectrum[i] = acc[i];
}

static bool WriteFlashlightProbeLastBundle(const std::string& schemaPath, const std::string& jsonText) {
    std::string baseDir = NormalizePath(GetDiagnosticsPrimaryDir());
    if (!EnsureDirectoryRecursive(baseDir)) {
        baseDir = NormalizePath(GetDiagnosticsSecondaryDir());
        if (!EnsureDirectoryRecursive(baseDir)) {
            return UiErrorOrFail("diag.mkdir_failed", schemaPath, baseDir, "Failed to create diagnostics folder");
        }
    }

    const std::string probePath = JoinPath(baseDir, "flashlight_probe.json");
    if (!WriteTextFile(probePath, jsonText)) {
        return UiErrorOrFail("diag.probe_write_failed", schemaPath, probePath, "Failed to write diagnostics flashlight_probe.json");
    }
    return true;
}

static bool WriteFlashlightBridgeRequestLastBundle(const std::string& schemaPath, const std::string& jsonText) {
    std::string baseDir = NormalizePath(GetDiagnosticsPrimaryDir());
    if (!EnsureDirectoryRecursive(baseDir)) {
        baseDir = NormalizePath(GetDiagnosticsSecondaryDir());
        if (!EnsureDirectoryRecursive(baseDir)) {
            return UiErrorOrFail("diag.mkdir_failed", schemaPath, baseDir, "Failed to create diagnostics folder");
        }
    }

    const std::string reqPath = JoinPath(baseDir, "flashlight_bridge_request.json");
    if (!WriteTextFileAtomicReplace(reqPath, jsonText)) {
        return UiErrorOrFail("diag.bridge_request_write_failed", schemaPath, reqPath, "Failed to write diagnostics flashlight_bridge_request.json");
    }
    return true;
}

static uint64_t GetFileLastWriteTime64(const std::string& path) {
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &fad)) return 0;
    ULARGE_INTEGER ui;
    ui.LowPart = fad.ftLastWriteTime.dwLowDateTime;
    ui.HighPart = fad.ftLastWriteTime.dwHighDateTime;
    return (uint64_t)ui.QuadPart;
}

static bool WriteProbeSdfLastBundle(const std::string& schemaPath,
                                   const ViewState& view, const KernelParams& params, const RenderSettings& render, const LensSettings& lens,
                                   int x, int y, int lensW, int lensH, int ds, float maxAbsPxLow, float signedPx, bool inside) {
    std::string baseDir = NormalizePath(GetDiagnosticsPrimaryDir());
    if (!EnsureDirectoryRecursive(baseDir)) {
        baseDir = NormalizePath(GetDiagnosticsSecondaryDir());
        if (!EnsureDirectoryRecursive(baseDir)) {
            return UiErrorOrFail("diag.mkdir_failed", schemaPath, baseDir, "Failed to create diagnostics folder");
        }
    }

    const std::string stateJson = BuildStateJson(view, params, render, lens);

    std::ostringstream ss;
    ss.setf(std::ios::fixed);
    ss << std::setprecision(9);
    ss << "{\n";
    ss << "  \"schema_path\": \"" << JsonEscape(schemaPath) << "\",\n";
    ss << "  \"probe\": {\n";
    ss << "    \"type\": \"lens_sdf_chamfer\",\n";
    ss << "    \"coord_space\": \"lens_low\",\n";
    ss << "    \"x\": " << (double)x << ",\n";
    ss << "    \"y\": " << (double)y << ",\n";
    ss << "    \"lens_size\": [" << (double)lensW << ", " << (double)lensH << "],\n";
    ss << "    \"downsample\": " << (double)ds << ",\n";
    ss << "    \"max_abs_px\": " << (double)maxAbsPxLow << ",\n";
    ss << "    \"mask_inside\": " << (inside ? "true" : "false") << ",\n";
    ss << "    \"signed_px\": " << (double)signedPx << ",\n";
    ss << "    \"normalized\": " << (double)(signedPx / fmaxf(1.0f, maxAbsPxLow)) << "\n";
    ss << "  },\n";
    ss << "  \"state\": " << stateJson;
    ss << "}\n";

    const std::string probePath = JoinPath(baseDir, "probe_sdf.json");
    if (!WriteTextFile(probePath, ss.str())) {
        return UiErrorOrFail("diag.probe_write_failed", schemaPath, probePath, "Failed to write diagnostics probe_sdf.json");
    }
    return true;
}

static void ExportStateToFile(const std::string& schemaPath, const ViewState& view, const KernelParams& params, const RenderSettings& render, const LensSettings& lens) {
    const std::string json = BuildStateJson(view, params, render, lens);

    const std::string outPrimary = GetStatePrimaryPath();
    const std::string outSecondary = GetStateSecondaryPath();
    if (!WriteTextFile(outPrimary, json)) {
        if (!WriteTextFile(outSecondary, json)) {
            FailFastUiError("state.export.write_failed", schemaPath, outPrimary, "Failed to write state file: " + outPrimary);
        }
    }
}

static const json_min::Value* RequireObjField(const std::string& schemaPath, const json_min::Value& obj, const std::string& path, const char* key) {
    if (!obj.is_object()) {
        FailFastUiError("state.import.type", schemaPath, path, "Expected object at: " + path);
        return nullptr;
    }
    const json_min::Value* v = obj.get(key);
    if (!v) {
        FailFastUiError("state.import.missing", schemaPath, path + "." + key, "Missing required key: " + path + "." + key);
        return nullptr;
    }
    return v;
}

static double RequireNumber(const std::string& schemaPath, const json_min::Value& v, const std::string& path) {
    if (!v.is_number()) {
        FailFastUiError("state.import.type", schemaPath, path, "Expected number at: " + path);
    }
    return v.as_number();
}

static bool RequireBool(const std::string& schemaPath, const json_min::Value& v, const std::string& path) {
    if (!v.is_bool()) {
        FailFastUiError("state.import.type", schemaPath, path, "Expected bool at: " + path);
    }
    return v.as_bool();
}

static std::string RequireString(const std::string& schemaPath, const json_min::Value& v, const std::string& path) {
    if (!v.is_string()) {
        FailFastUiError("state.import.type", schemaPath, path, "Expected string at: " + path);
    }
    return v.as_string();
}

static int RequireIntRange(const std::string& schemaPath, const json_min::Value& v, const std::string& path, int lo, int hi) {
    double n = RequireNumber(schemaPath, v, path);
    double r = std::round(n);
    if (std::fabs(n - r) > 1e-9) {
        FailFastUiError("state.import.type", schemaPath, path, "Expected integer (number) at: " + path);
    }
    if (r < (double)lo || r > (double)hi) {
        FailFastUiError("state.import.range", schemaPath, path, "Integer out of range at: " + path);
    }
    return (int)r;
}

static void ImportStateFromFile(const std::string& schemaPath, ViewState& view, KernelParams& params, RenderSettings& render, LensSettings& lens, bool* ioDirty) {
    const std::string inPrimary = GetStatePrimaryPath();
    std::string text = ReadTextFile(inPrimary.c_str());
    if (text.empty()) {
        const std::string inSecondary = GetStateSecondaryPath();
        text = ReadTextFile(inSecondary.c_str());
        if (text.empty()) {
            FailFastUiError("state.import.missing", schemaPath, inPrimary, "State file missing/unreadable: " + inPrimary);
        }
    }

    auto pr = json_min::Parse(text);
    if (!pr.error.empty()) {
        FailFastUiError("state.import.parse", schemaPath, inPrimary, pr.error);
    }
    if (!pr.value.is_object()) {
        FailFastUiError("state.import.type", schemaPath, "root", "Top-level state must be an object");
    }

    const json_min::Value* vVersion = pr.value.get("version");
    if (!vVersion || !vVersion->is_number() || (int)std::round(vVersion->as_number()) != 1) {
        FailFastUiError("state.import.version", schemaPath, "version", "Unsupported or missing state version (expected 1)");
    }

    const json_min::Value* vCamera = RequireObjField(schemaPath, pr.value, "root", "camera");
    const json_min::Value* vFractal = RequireObjField(schemaPath, pr.value, "root", "fractal");
    const json_min::Value* vParams = RequireObjField(schemaPath, pr.value, "root", "params");
    const json_min::Value* vEngine = RequireObjField(schemaPath, pr.value, "root", "engine");

    // Lens (optional)
    if (const json_min::Value* vLens = pr.value.get("lens")) {
        if (vLens->is_object()) {
            if (const json_min::Value* vEnabled = vLens->get("enabled")) {
                if (vEnabled->is_bool()) lens.enabled = vEnabled->as_bool();
            }
            if (const json_min::Value* vDown = vLens->get("downsample")) {
                if (vDown->is_number()) {
                    int d = (int)std::lround(vDown->as_number());
                    lens.downsample = NormalizeLensDownsamplePow2(d);
                }
            }
        }
    }

    // Camera
    view.center_hp_x = RequireNumber(schemaPath, *RequireObjField(schemaPath, *vCamera, "camera", "center_hp_x"), "camera.center_hp_x");
    view.center_hp_y = RequireNumber(schemaPath, *RequireObjField(schemaPath, *vCamera, "camera", "center_hp_y"), "camera.center_hp_y");
    view.log2_zoom = RequireNumber(schemaPath, *RequireObjField(schemaPath, *vCamera, "camera", "log2_zoom"), "camera.log2_zoom");
    view.rotation_degrees = (float)RequireNumber(schemaPath, *RequireObjField(schemaPath, *vCamera, "camera", "rotation_degrees"), "camera.rotation_degrees");
    {
        std::string cb = RequireString(schemaPath, *RequireObjField(schemaPath, *vCamera, "camera", "camera_behavior"), "camera.camera_behavior");
        CameraBehavior b;
        if (!CameraBehaviorFromId(cb, b)) {
            FailFastUiError("state.import.enum", schemaPath, "camera.camera_behavior", "Unknown camera_behavior: " + cb);
        }
        view.camera_behavior = b;
    }
    view.auto_dive = RequireBool(schemaPath, *RequireObjField(schemaPath, *vCamera, "camera", "auto_dive"), "camera.auto_dive");
    view.dive_speed = (float)RequireNumber(schemaPath, *RequireObjField(schemaPath, *vCamera, "camera", "dive_speed"), "camera.dive_speed");
    SyncViewUiFromHp(view);

    // Fractal
    {
        std::string ft = RequireString(schemaPath, *RequireObjField(schemaPath, *vFractal, "fractal", "fractal_type"), "fractal.fractal_type");
        FractalType t;
        if (!FractalTypeFromId(ft, t)) {
            FailFastUiError("state.import.enum", schemaPath, "fractal.fractal_type", "Unknown fractal_type: " + ft);
        }
        view.fractal_type = t;
    }
    view.explaino_alive = RequireBool(schemaPath, *RequireObjField(schemaPath, *vFractal, "fractal", "explaino_alive"), "fractal.explaino_alive");
    {
        // Optional: seed tween (default true). Older state files may omit it.
        if (const json_min::Value* v = vFractal->get("explaino_seed_tween")) {
            if (v->is_bool()) view.explaino_seed_tween = v->as_bool();
        }
    }
    view.explaino_alive_speed = (float)RequireNumber(schemaPath, *RequireObjField(schemaPath, *vFractal, "fractal", "explaino_alive_speed"), "fractal.explaino_alive_speed");
    view.explaino_phase = (float)RequireNumber(schemaPath, *RequireObjField(schemaPath, *vFractal, "fractal", "explaino_phase"), "fractal.explaino_phase");
    view.explaino_seed_drift = (float)RequireNumber(schemaPath, *RequireObjField(schemaPath, *vFractal, "fractal", "explaino_seed_drift"), "fractal.explaino_seed_drift");

    // Params
    params.max_iter = RequireIntRange(schemaPath, *RequireObjField(schemaPath, *vParams, "params", "max_iter"), "params.max_iter", 1, 5000000);
    params.epsilon = (float)RequireNumber(schemaPath, *RequireObjField(schemaPath, *vParams, "params", "epsilon"), "params.epsilon");
    params.nova_alpha = (float)RequireNumber(schemaPath, *RequireObjField(schemaPath, *vParams, "params", "nova_alpha"), "params.nova_alpha");
    params.phoenix_p_real = (float)RequireNumber(schemaPath, *RequireObjField(schemaPath, *vParams, "params", "phoenix_p_real"), "params.phoenix_p_real");
    params.phoenix_p_imag = (float)RequireNumber(schemaPath, *RequireObjField(schemaPath, *vParams, "params", "phoenix_p_imag"), "params.phoenix_p_imag");
    {
        std::string pk = RequireString(schemaPath, *RequireObjField(schemaPath, *vParams, "params", "poly_kind"), "params.poly_kind");
        PolyKind k;
        if (!PolyKindFromId(pk, k)) {
            FailFastUiError("state.import.enum", schemaPath, "params.poly_kind", "Unknown poly_kind: " + pk);
        }
        params.poly_kind = k;
    }
    {
        const json_min::Value* arrV = RequireObjField(schemaPath, *vParams, "params", "poly_coeffs");
        if (!arrV->is_array() || arrV->as_array().size() != 5) {
            FailFastUiError("state.import.type", schemaPath, "params.poly_coeffs", "poly_coeffs must be an array of 5 numbers");
        }
        for (int i = 0; i < 5; i++) {
            const auto& it = arrV->as_array()[i];
            if (!it.is_number()) {
                FailFastUiError("state.import.type", schemaPath, "params.poly_coeffs", "poly_coeffs must be numbers");
            }
            params.poly_coeffs[i] = (float)it.as_number();
        }
    }
    params.multibrot_power = RequireIntRange(schemaPath, *RequireObjField(schemaPath, *vParams, "params", "multibrot_power"), "params.multibrot_power", 2, 64);
    {
        std::string cm = RequireString(schemaPath, *RequireObjField(schemaPath, *vParams, "params", "coloring_mode"), "params.coloring_mode");
        ColoringMode m;
        if (!ColoringModeFromId(cm, m)) {
            FailFastUiError("state.import.enum", schemaPath, "params.coloring_mode", "Unknown coloring_mode: " + cm);
        }
        params.coloring_mode = m;
    }
    params.exposure = (float)RequireNumber(schemaPath, *RequireObjField(schemaPath, *vParams, "params", "exposure"), "params.exposure");
    params.color_saturation = (float)RequireNumber(schemaPath, *RequireObjField(schemaPath, *vParams, "params", "color_saturation"), "params.color_saturation");
    params.color_contrast = (float)RequireNumber(schemaPath, *RequireObjField(schemaPath, *vParams, "params", "color_contrast"), "params.color_contrast");
    params.color_tint_r = (float)RequireNumber(schemaPath, *RequireObjField(schemaPath, *vParams, "params", "color_tint_r"), "params.color_tint_r");
    params.color_tint_g = (float)RequireNumber(schemaPath, *RequireObjField(schemaPath, *vParams, "params", "color_tint_g"), "params.color_tint_g");
    params.color_tint_b = (float)RequireNumber(schemaPath, *RequireObjField(schemaPath, *vParams, "params", "color_tint_b"), "params.color_tint_b");
    params.explaino_seed = RequireIntRange(schemaPath, *RequireObjField(schemaPath, *vParams, "params", "explaino_seed"), "params.explaino_seed", -2147483647, 2147483647);
    params.explaino_warp_strength = (float)RequireNumber(schemaPath, *RequireObjField(schemaPath, *vParams, "params", "explaino_warp_strength"), "params.explaino_warp_strength");

    // Engine
    {
        const json_min::Value* resV = RequireObjField(schemaPath, *vEngine, "engine", "resolution");
        if (!resV->is_array() || resV->as_array().size() != 2 || !resV->as_array()[0].is_number() || !resV->as_array()[1].is_number()) {
            FailFastUiError("state.import.type", schemaPath, "engine.resolution", "resolution must be [x,y] numbers");
        }
        render.resolution.x = (int)std::round(resV->as_array()[0].as_number());
        render.resolution.y = (int)std::round(resV->as_array()[1].as_number());
        if (render.resolution.x <= 0 || render.resolution.y <= 0) {
            FailFastUiError("state.import.range", schemaPath, "engine.resolution", "resolution must be positive");
        }
    }
    render.block_size = RequireIntRange(schemaPath, *RequireObjField(schemaPath, *vEngine, "engine", "block_size"), "engine.block_size", 1, 4096);
    render.device_id = RequireIntRange(schemaPath, *RequireObjField(schemaPath, *vEngine, "engine", "device_id"), "engine.device_id", 0, 64);
    render.benchmark = RequireBool(schemaPath, *RequireObjField(schemaPath, *vEngine, "engine", "benchmark"), "engine.benchmark");

    if (ioDirty) *ioDirty = true;
}

static UISchema BuildSafeModeSchema() {
    UISchema s;
    s.schema_version = "1";
    s.name_space = "fractal";
    UISchemaPanel view;
    view.id = "view";
    view.label = "View (Safe Mode)";
    view.order = 10;
    view.has_order = true;
    {
        UISchemaControl c;
        c.id = "fractal_type";
        c.type = "combo";
        c.label = "Fractal Type";
        c.value_type = "enum";
        c.has_binding = true;
        c.binding.kind = "param";
        c.binding.path = "fractal.view.fractal_type";
        c.has_default = true;
        c.def.v = std::string("newton");
        c.options = {
            {"newton", "Newton"},
            {"nova", "Nova"},
            {"explaino", "Explaino"},
            {"mandelbrot", "Mandelbrot"},
            {"julia", "Julia"},
            {"burning_ship", "Burning Ship"},
            {"multibrot", "Multibrot"},
            {"phoenix", "Phoenix"},
        };
        view.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "center_x";
        c.type = "drag_float";
        c.label = "Center X";
        c.value_type = "float";
        c.min = -2.0; c.max = 2.0; c.step = 0.001;
        c.has_min = c.has_max = c.has_step = true;
        c.has_binding = true;
        c.binding.kind = "param";
        c.binding.path = "fractal.view.center.x";
        c.has_default = true;
        c.def.v = 0.0;
        view.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "center_y";
        c.type = "drag_float";
        c.label = "Center Y";
        c.value_type = "float";
        c.min = -2.0; c.max = 2.0; c.step = 0.001;
        c.has_min = c.has_max = c.has_step = true;
        c.has_binding = true;
        c.binding.kind = "param";
        c.binding.path = "fractal.view.center.y";
        c.has_default = true;
        c.def.v = 0.0;
        view.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "zoom";
        c.type = "drag_float";
        c.label = "Zoom";
        c.value_type = "float";
        c.min = 0.1; c.max = 1000.0; c.step = 0.01;
        c.has_min = c.has_max = c.has_step = true;
        c.has_binding = true;
        c.binding.kind = "param";
        c.binding.path = "fractal.view.zoom";
        c.has_default = true;
        c.def.v = 1.0;
        view.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "rotation_deg";
        c.type = "drag_float";
        c.label = "Rotation (deg)";
        c.value_type = "float";
        c.min = -180.0; c.max = 180.0; c.step = 0.1;
        c.has_min = c.has_max = c.has_step = true;
        c.has_binding = true;
        c.binding.kind = "param";
        c.binding.path = "fractal.view.rotation";
        c.has_default = true;
        c.def.v = 0.0;
        view.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "auto_refresh";
        c.type = "checkbox";
        c.label = "Auto-refresh";
        c.value_type = "bool";
        c.has_binding = true;
        c.binding.kind = "param";
        c.binding.path = "fractal.view.auto_refresh";
        c.has_default = true;
        c.def.v = true;
        view.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "render_once";
        c.type = "button";
        c.label = "Render Once";
        c.has_binding = true;
        c.binding.kind = "action";
        c.binding.path = "fractal.actions.render_once";
        view.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "reset_view";
        c.type = "button";
        c.label = "Reset View";
        c.has_binding = true;
        c.binding.kind = "action";
        c.binding.path = "fractal.actions.reset_view";
        view.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "reset_all";
        c.type = "button";
        c.label = "Reset All";
        c.has_binding = true;
        c.binding.kind = "action";
        c.binding.path = "fractal.actions.reset_all";
        view.controls.push_back(std::move(c));
    }
    UISchemaPanel fractal;
    fractal.id = "fractal";
    fractal.label = "Fractal (Safe Mode)";
    fractal.order = 20;
    fractal.has_order = true;
    {
        UISchemaControl c;
        c.id = "max_iter";
        c.type = "slider_int";
        c.label = "Max Iterations";
        c.value_type = "int";
        c.min = 1; c.max = 5000; c.step = 1;
        c.has_min = c.has_max = c.has_step = true;
        c.has_binding = true;
        c.binding.kind = "param";
        c.binding.path = "fractal.params.max_iter";
        c.has_default = true;
        c.def.v = 500.0;
        fractal.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "exposure";
        c.type = "slider_float";
        c.label = "Exposure";
        c.value_type = "float";
        c.min = 0.1; c.max = 5.0; c.step = 0.01;
        c.has_min = c.has_max = c.has_step = true;
        c.has_binding = true;
        c.binding.kind = "param";
        c.binding.path = "fractal.params.exposure";
        c.has_default = true;
        c.def.v = 1.0;
        fractal.controls.push_back(std::move(c));
    }
    UISchemaPanel render;
    render.id = "render";
    render.label = "Render (Safe Mode)";
    render.order = 30;
    render.has_order = true;
    {
        UISchemaControl c;
        c.id = "width";
        c.type = "slider_int";
        c.label = "Width";
        c.value_type = "int";
        c.min = 64; c.max = 4096; c.step = 1;
        c.has_min = c.has_max = c.has_step = true;
        c.has_binding = true;
        c.binding.kind = "param";
        c.binding.path = "fractal.render.resolution.x";
        c.has_default = true;
        c.def.v = 1024.0;
        render.controls.push_back(std::move(c));
    }
    {
        UISchemaControl c;
        c.id = "height";
        c.type = "slider_int";
        c.label = "Height";
        c.value_type = "int";
        c.min = 64; c.max = 4096; c.step = 1;
        c.has_min = c.has_max = c.has_step = true;
        c.has_binding = true;
        c.binding.kind = "param";
        c.binding.path = "fractal.render.resolution.y";
        c.has_default = true;
        c.def.v = 768.0;
        render.controls.push_back(std::move(c));
    }
    s.panels.push_back(std::move(view));
    s.panels.push_back(std::move(fractal));
    s.panels.push_back(std::move(render));
    return s;
}

static void CreateRenderTarget() {
    ID3D11Texture2D* pBackBuffer = nullptr;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    if (pBackBuffer) {
        g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
        pBackBuffer->Release();
    }
}

static void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_fractalSRV) { g_fractalSRV->Release(); g_fractalSRV = nullptr; }
    if (g_fractalTexture) { g_fractalTexture->Release(); g_fractalTexture = nullptr; }

    if (g_lensSRV) { g_lensSRV->Release(); g_lensSRV = nullptr; }
    if (g_lensTexture) { g_lensTexture->Release(); g_lensTexture = nullptr; }

    if (g_maskSRV) { g_maskSRV->Release(); g_maskSRV = nullptr; }
    if (g_maskTexture) { g_maskTexture->Release(); g_maskTexture = nullptr; }

    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

static bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = {D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0};

    HRESULT res = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createDeviceFlags,
        featureLevelArray,
        2,
        D3D11_SDK_VERSION,
        &sd,
        &g_pSwapChain,
        &g_pd3dDevice,
        &featureLevel,
        &g_pd3dDeviceContext);

    if (res != S_OK) return false;

    CreateRenderTarget();
    return true;
}

static void EnsureFractalTexture(int width, int height) {
    if (width <= 0 || height <= 0) return;

    D3D11_TEXTURE2D_DESC desc{};
    if (g_fractalTexture) {
        g_fractalTexture->GetDesc(&desc);
        if ((int)desc.Width == width && (int)desc.Height == height) return;

        g_fractalSRV->Release();
        g_fractalSRV = nullptr;
        g_fractalTexture->Release();
        g_fractalTexture = nullptr;
    }

    desc.Width = (UINT)width;
    desc.Height = (UINT)height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    g_pd3dDevice->CreateTexture2D(&desc, nullptr, &g_fractalTexture);
    g_pd3dDevice->CreateShaderResourceView(g_fractalTexture, nullptr, &g_fractalSRV);
}

static void EnsureLensTexture(int width, int height) {
    if (width <= 0 || height <= 0) return;

    D3D11_TEXTURE2D_DESC desc{};
    if (g_lensTexture) {
        g_lensTexture->GetDesc(&desc);
        if ((int)desc.Width == width && (int)desc.Height == height) return;

        g_lensSRV->Release();
        g_lensSRV = nullptr;
        g_lensTexture->Release();
        g_lensTexture = nullptr;
    }

    desc.Width = (UINT)width;
    desc.Height = (UINT)height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    g_pd3dDevice->CreateTexture2D(&desc, nullptr, &g_lensTexture);
    g_pd3dDevice->CreateShaderResourceView(g_lensTexture, nullptr, &g_lensSRV);
}

static void EnsureMaskTexture(int width, int height) {
    if (width <= 0 || height <= 0) return;

    D3D11_TEXTURE2D_DESC desc{};
    if (g_maskTexture) {
        g_maskTexture->GetDesc(&desc);
        if ((int)desc.Width == width && (int)desc.Height == height) return;

        g_maskSRV->Release();
        g_maskSRV = nullptr;
        g_maskTexture->Release();
        g_maskTexture = nullptr;
    }

    desc.Width = (UINT)width;
    desc.Height = (UINT)height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    g_pd3dDevice->CreateTexture2D(&desc, nullptr, &g_maskTexture);
    g_pd3dDevice->CreateShaderResourceView(g_maskTexture, nullptr, &g_maskSRV);
}

static void UploadFractalRGBA(const uint32_t* rgba, int width, int height) {
    if (!rgba || !g_fractalTexture) return;

    D3D11_MAPPED_SUBRESOURCE mapped{};
    if (SUCCEEDED(g_pd3dDeviceContext->Map(g_fractalTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        const uint8_t* src = reinterpret_cast<const uint8_t*>(rgba);
        uint8_t* dst = reinterpret_cast<uint8_t*>(mapped.pData);
        size_t rowBytes = (size_t)width * 4;
        for (int y = 0; y < height; y++) {
            memcpy(dst + (size_t)mapped.RowPitch * (size_t)y, src + rowBytes * (size_t)y, rowBytes);
        }
        g_pd3dDeviceContext->Unmap(g_fractalTexture, 0);
    }
}

static void UploadLensRGBA(const uint32_t* rgba, int width, int height) {
    if (!rgba || !g_lensTexture) return;

    D3D11_MAPPED_SUBRESOURCE mapped{};
    if (SUCCEEDED(g_pd3dDeviceContext->Map(g_lensTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        const uint8_t* src = reinterpret_cast<const uint8_t*>(rgba);
        uint8_t* dst = reinterpret_cast<uint8_t*>(mapped.pData);
        size_t rowBytes = (size_t)width * 4;
        for (int y = 0; y < height; y++) {
            memcpy(dst + (size_t)mapped.RowPitch * (size_t)y, src + rowBytes * (size_t)y, rowBytes);
        }
        g_pd3dDeviceContext->Unmap(g_lensTexture, 0);
    }
}

static void UploadMaskRGBA(const uint32_t* rgba, int width, int height) {
    if (!rgba || !g_maskTexture) return;

    D3D11_MAPPED_SUBRESOURCE mapped{};
    if (SUCCEEDED(g_pd3dDeviceContext->Map(g_maskTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        const uint8_t* src = reinterpret_cast<const uint8_t*>(rgba);
        uint8_t* dst = reinterpret_cast<uint8_t*>(mapped.pData);
        size_t rowBytes = (size_t)width * 4;
        for (int y = 0; y < height; y++) {
            memcpy(dst + (size_t)mapped.RowPitch * (size_t)y, src + rowBytes * (size_t)y, rowBytes);
        }
        g_pd3dDeviceContext->Unmap(g_maskTexture, 0);
    }
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;

    switch (msg) {
    case WM_SIZE:
        if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED) {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

static void SetPolyPreset(KernelParams& params) {
    if (params.poly_kind == PolyKind::z3_minus_1) {
        params.poly_coeffs[0] = -1.0f;
        params.poly_coeffs[1] = 0.0f;
        params.poly_coeffs[2] = 0.0f;
        params.poly_coeffs[3] = 1.0f;
        params.poly_coeffs[4] = 0.0f;
    } else if (params.poly_kind == PolyKind::z4_minus_1) {
        params.poly_coeffs[0] = -1.0f;
        params.poly_coeffs[1] = 0.0f;
        params.poly_coeffs[2] = 0.0f;
        params.poly_coeffs[3] = 0.0f;
        params.poly_coeffs[4] = 1.0f;
    }
}

static std::string ReadTextFile(const char* path) {
    std::ifstream f(path, std::ios::in | std::ios::binary);
    if (!f) return {};
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

struct BindingContext {
    std::string schema_path;
    ViewState* view = nullptr;
    KernelParams* params = nullptr;
    RenderSettings* render = nullptr;
    LensSettings* lens = nullptr;

    // Return enum ids for predicates / combos
    std::string GetEnumId(const std::string& path) const {
        if (params && path == "fractal.params.poly_kind") {
            switch (params->poly_kind) {
            case PolyKind::z3_minus_1: return "z3_minus_1";
            case PolyKind::z4_minus_1: return "z4_minus_1";
            case PolyKind::custom: return "custom";
            }
        }
        if (params && path == "fractal.params.coloring_mode") {
            switch (params->coloring_mode) {
            case ColoringMode::root_basin: return "root_basin";
            case ColoringMode::iteration_count: return "iteration_count";
            case ColoringMode::smooth_escape: return "smooth_escape";
            case ColoringMode::joy_basins: return "joy_basins";
            }
        }
        if (view && path == "fractal.view.fractal_type") {
            switch (view->fractal_type) {
            case FractalType::newton: return "newton";
            case FractalType::nova: return "nova";
            case FractalType::explaino: return "explaino";
            case FractalType::explaino_y: return "explaino_y";
            case FractalType::explaino_fp: return "explaino_fp";
            case FractalType::mandelbrot: return "mandelbrot";
            case FractalType::julia: return "julia";
            case FractalType::burning_ship: return "burning_ship";
            case FractalType::multibrot: return "multibrot";
            case FractalType::phoenix: return "phoenix";
            }
        }
        if (view && path == "fractal.view.camera_behavior") {
            switch (view->camera_behavior) {
            case CameraBehavior::manual: return "manual";
            case CameraBehavior::complexity: return "complexity";
            case CameraBehavior::orbit: return "orbit";
            case CameraBehavior::entropy: return "entropy";
            case CameraBehavior::off: return "off";
            }
        }
        return {};
    }

    bool SetEnumId(const std::string& path, const std::string& id) {
        if (params && path == "fractal.params.poly_kind") {
            if (id == "z3_minus_1") params->poly_kind = PolyKind::z3_minus_1;
            else if (id == "z4_minus_1") params->poly_kind = PolyKind::z4_minus_1;
            else if (id == "custom") params->poly_kind = PolyKind::custom;
            else return false;
            return true;
        }
        if (params && path == "fractal.params.coloring_mode") {
            if (id == "root_basin") params->coloring_mode = ColoringMode::root_basin;
            else if (id == "iteration_count") params->coloring_mode = ColoringMode::iteration_count;
            else if (id == "smooth_escape") params->coloring_mode = ColoringMode::smooth_escape;
            else if (id == "joy_basins") params->coloring_mode = ColoringMode::joy_basins;
            else return false;
            return true;
        }
        if (view && path == "fractal.view.fractal_type") {
            if (id == "newton") view->fractal_type = FractalType::newton;
            else if (id == "nova") view->fractal_type = FractalType::nova;
            else if (id == "explaino") view->fractal_type = FractalType::explaino;
            else if (id == "explaino_y") view->fractal_type = FractalType::explaino_y;
            else if (id == "explaino_fp") view->fractal_type = FractalType::explaino_fp;
            else if (id == "mandelbrot") view->fractal_type = FractalType::mandelbrot;
            else if (id == "julia") view->fractal_type = FractalType::julia;
            else if (id == "burning_ship") view->fractal_type = FractalType::burning_ship;
            else if (id == "multibrot") view->fractal_type = FractalType::multibrot;
            else if (id == "phoenix") view->fractal_type = FractalType::phoenix;
            else return false;
            return true;
        }
        if (view && path == "fractal.view.camera_behavior") {
            if (id == "manual") view->camera_behavior = CameraBehavior::manual;
            else if (id == "complexity") view->camera_behavior = CameraBehavior::complexity;
            else if (id == "orbit") view->camera_behavior = CameraBehavior::orbit;
            else if (id == "entropy") view->camera_behavior = CameraBehavior::entropy;
            else if (id == "off") view->camera_behavior = CameraBehavior::off;
            else return false;
            return true;
        }
        return false;
    }

    bool GetBoolValue(const std::string& path, bool& out) const {
        // Capability predicates (virtual paths): keep schema free of fractal-type allowlists.
        // These are derived from the authoritative engine state (view->fractal_type) in one place.
        if (path.rfind("fractal.cap.", 0) == 0) {
            if (!view) return false;
            FractalType ft = view->fractal_type;

            const bool isExplainoFamily = (ft == FractalType::explaino || ft == FractalType::explaino_y || ft == FractalType::explaino_fp);
            const bool isRootFamily = (ft == FractalType::newton || ft == FractalType::nova || isExplainoFamily);

            if (path == "fractal.cap.supports_multibrot_power") { out = (ft == FractalType::multibrot); return true; }
            if (path == "fractal.cap.supports_phoenix_feedback") { out = (ft == FractalType::phoenix); return true; }
            if (path == "fractal.cap.supports_nova_alpha") { out = (ft == FractalType::nova); return true; }
            if (path == "fractal.cap.supports_explaino_controls") { out = isExplainoFamily; return true; }

            if (path == "fractal.cap.supports_epsilon") { out = isRootFamily; return true; }
            if (path == "fractal.cap.supports_poly_kind") { out = (ft == FractalType::newton || ft == FractalType::nova); return true; }
            if (path == "fractal.cap.supports_poly_coeffs") { out = (ft == FractalType::newton || ft == FractalType::nova); return true; }
            if (path == "fractal.cap.supports_poly_coeff_c4") { out = (ft == FractalType::newton); return true; }

            // Unknown capability key.
            UiErrorOrFail("predicate.capability_unknown", schema_path, path, "Unknown capability predicate: " + path);
            return false;
        }

        bool* ptr = nullptr;
        BindingContext* self = const_cast<BindingContext*>(this);
        if (!self->BindBool(path, &ptr) || !ptr) return false;
        out = *ptr;
        return true;
    }

    bool GetIntValue(const std::string& path, int& out) const {
        int* ptr = nullptr;
        BindingContext* self = const_cast<BindingContext*>(this);
        if (!self->BindInt(path, &ptr) || !ptr) return false;
        out = *ptr;
        return true;
    }

    bool GetFloatValue(const std::string& path, float& out) const {
        float* ptr = nullptr;
        BindingContext* self = const_cast<BindingContext*>(this);
        if (!self->BindFloat(path, &ptr) || !ptr) return false;
        out = *ptr;
        return true;
    }

    bool EvalVisibleIf(const UISchemaPredicate& pred) const {
        if (pred.op.empty() || pred.path.empty()) {
            UiErrorOrFail("predicate.invalid", schema_path, pred.path, "visible_if missing op/path");
            return true;
        }

        // Enum predicates
        std::string curEnum = GetEnumId(pred.path);
        if (!curEnum.empty()) {
            if (pred.op == "eq") return curEnum == pred.value;
            if (pred.op == "neq") return curEnum != pred.value;
            if (pred.op == "in") {
                // Comma-separated list: "a,b,c" (whitespace tolerated).
                std::string v = pred.value;
                size_t i = 0;
                while (i < v.size()) {
                    while (i < v.size() && (v[i] == ' ' || v[i] == '\t' || v[i] == '\n' || v[i] == '\r' || v[i] == ',')) ++i;
                    size_t j = i;
                    while (j < v.size() && v[j] != ',') ++j;
                    std::string tok = v.substr(i, j - i);
                    // Trim trailing whitespace
                    while (!tok.empty() && (tok.back() == ' ' || tok.back() == '\t' || tok.back() == '\n' || tok.back() == '\r')) tok.pop_back();
                    if (!tok.empty() && tok == curEnum) return true;
                    i = j + 1;
                }
                return false;
            }
            UiErrorOrFail("predicate.enum_op_unknown", schema_path, pred.path, "Unknown enum predicate op: " + pred.op);
            return true;
        }

        // Bool predicates
        bool curB = false;
        if (GetBoolValue(pred.path, curB)) {
            bool rhs = (pred.value == "true" || pred.value == "1");
            if (pred.op == "eq") return curB == rhs;
            if (pred.op == "neq") return curB != rhs;
            UiErrorOrFail("predicate.bool_op_unknown", schema_path, pred.path, "Unknown bool predicate op: " + pred.op);
            return true;
        }

        // Numeric predicates (int/float)
        double curN = 0.0;
        {
            int curI = 0;
            float curF = 0.0f;
            if (GetIntValue(pred.path, curI)) curN = (double)curI;
            else if (GetFloatValue(pred.path, curF)) curN = (double)curF;
            else {
                UiErrorOrFail("predicate.path_unbound", schema_path, pred.path, "Predicate path did not bind to enum/bool/int/float: " + pred.path);
                return true;
            }
        }

        double rhsN = 0.0;
        try {
            rhsN = std::stod(pred.value);
        } catch (...) {
            UiErrorOrFail("predicate.numeric_rhs_invalid", schema_path, pred.path, "Predicate RHS is not numeric: '" + pred.value + "'");
            return true;
        }

        if (pred.op == "eq") return curN == rhsN;
        if (pred.op == "neq") return curN != rhsN;
        if (pred.op == "lt") return curN < rhsN;
        if (pred.op == "lte") return curN <= rhsN;
        if (pred.op == "gt") return curN > rhsN;
        if (pred.op == "gte") return curN >= rhsN;
        UiErrorOrFail("predicate.numeric_op_unknown", schema_path, pred.path, "Unknown numeric predicate op: " + pred.op);
        return true;
    }

    bool BindFloat(const std::string& path, float** outPtr) {
        if (!view || !params) return false;
        if (path == "fractal.view.center.x") { *outPtr = &view->center.x; return true; }
        if (path == "fractal.view.center.y") { *outPtr = &view->center.y; return true; }
        if (path == "fractal.view.zoom") { *outPtr = &view->zoom; return true; }
        if (path == "fractal.view.rotation") { *outPtr = &view->rotation_degrees; return true; }
        if (path == "fractal.view.dive_speed") { *outPtr = &view->dive_speed; return true; }
        if (path == "fractal.view.explaino_alive_speed") { *outPtr = &view->explaino_alive_speed; return true; }
        if (path == "fractal.params.epsilon") { *outPtr = &params->epsilon; return true; }
        if (path == "fractal.params.nova_alpha") { *outPtr = &params->nova_alpha; return true; }
        if (path == "fractal.params.phoenix_p_real") { *outPtr = &params->phoenix_p_real; return true; }
        if (path == "fractal.params.phoenix_p_imag") { *outPtr = &params->phoenix_p_imag; return true; }
        if (path == "fractal.params.exposure") { *outPtr = &params->exposure; return true; }
        if (path == "fractal.params.color_saturation") { *outPtr = &params->color_saturation; return true; }
        if (path == "fractal.params.color_contrast") { *outPtr = &params->color_contrast; return true; }
        if (path == "fractal.params.color_tint_r") { *outPtr = &params->color_tint_r; return true; }
        if (path == "fractal.params.color_tint_g") { *outPtr = &params->color_tint_g; return true; }
        if (path == "fractal.params.color_tint_b") { *outPtr = &params->color_tint_b; return true; }
        if (path == "fractal.params.explaino_warp_strength") { *outPtr = &params->explaino_warp_strength; return true; }
        if (path == "fractal.params.poly_coeffs.0") { *outPtr = &params->poly_coeffs[0]; return true; }
        if (path == "fractal.params.poly_coeffs.1") { *outPtr = &params->poly_coeffs[1]; return true; }
        if (path == "fractal.params.poly_coeffs.2") { *outPtr = &params->poly_coeffs[2]; return true; }
        if (path == "fractal.params.poly_coeffs.3") { *outPtr = &params->poly_coeffs[3]; return true; }
        if (path == "fractal.params.poly_coeffs.4") { *outPtr = &params->poly_coeffs[4]; return true; }
        return false;
    }

    bool BindInt(const std::string& path, int** outPtr) {
        if (!params || !render) return false;
        if (path == "fractal.params.max_iter") { *outPtr = &params->max_iter; return true; }
        if (path == "fractal.params.multibrot_power") { *outPtr = &params->multibrot_power; return true; }
        if (path == "fractal.params.explaino_seed") { *outPtr = &params->explaino_seed; return true; }
        if (path == "fractal.render.resolution.x") { *outPtr = &render->resolution.x; return true; }
        if (path == "fractal.render.resolution.y") { *outPtr = &render->resolution.y; return true; }
        if (path == "fractal.render.block_size") { *outPtr = &render->block_size; return true; }
        if (path == "fractal.render.device_id") { *outPtr = &render->device_id; return true; }
        if (path == "fractal.lens.downsample") { if (!lens) return false; *outPtr = &lens->downsample; return true; }
        return false;
    }

    bool BindBool(const std::string& path, bool** outPtr) {
        if (!view || !render) return false;
        if (path == "fractal.view.auto_refresh") { *outPtr = &view->auto_refresh; return true; }
        if (path == "fractal.view.auto_dive") { *outPtr = &view->auto_dive; return true; }
        if (path == "fractal.view.explaino_alive") { *outPtr = &view->explaino_alive; return true; }
        if (path == "fractal.view.explaino_seed_tween") { *outPtr = &view->explaino_seed_tween; return true; }
        if (path == "fractal.render.benchmark") { *outPtr = &render->benchmark; return true; }
        if (path == "fractal.lens.enabled") { if (!lens) return false; *outPtr = &lens->enabled; return true; }
        return false;
    }
};

static bool ApplySchemaDefaultForControl(const UISchemaControl& c, BindingContext& ctx, bool* ioDirty) {
    if (!c.has_binding || c.binding.kind != "param" || !c.has_default) return false;

    const std::string& path = c.binding.path;

    if (c.value_type == "bool") {
        bool* ptr = nullptr;
        if (!ctx.BindBool(path, &ptr) || !ptr) return false;
        bool newV = *ptr;
        if (c.def.is_bool()) newV = c.def.as_bool();
        else if (c.def.is_number()) newV = (c.def.as_number() != 0.0);
        else if (c.def.is_string()) newV = (c.def.as_string() == "true" || c.def.as_string() == "1");
        else return false;
        if (*ptr != newV) {
            *ptr = newV;
            if (ioDirty) *ioDirty = true;
            return true;
        }
        return false;
    }

    if (c.value_type == "int") {
        int* ptr = nullptr;
        if (!ctx.BindInt(path, &ptr) || !ptr) return false;
        int newV = *ptr;
        if (c.def.is_number()) newV = (int)c.def.as_number();
        else if (c.def.is_string()) {
            try { newV = std::stoi(c.def.as_string()); } catch (...) { return false; }
        } else return false;
        if (*ptr != newV) {
            *ptr = newV;
            if (ioDirty) *ioDirty = true;
            return true;
        }
        return false;
    }

    if (c.value_type == "float") {
        float* ptr = nullptr;
        if (!ctx.BindFloat(path, &ptr) || !ptr) return false;
        float newV = *ptr;
        if (c.def.is_number()) newV = (float)c.def.as_number();
        else if (c.def.is_string()) {
            try { newV = (float)std::stod(c.def.as_string()); } catch (...) { return false; }
        } else return false;
        if (*ptr != newV) {
            *ptr = newV;
            if (ioDirty) *ioDirty = true;
            return true;
        }
        return false;
    }

    if (c.value_type == "enum") {
        if (!c.def.is_string()) return false;
        std::string cur = ctx.GetEnumId(path);
        const std::string& want = c.def.as_string();
        if (cur != want) {
            bool ok = ctx.SetEnumId(path, want);
            if (ok && ioDirty) *ioDirty = true;
            return ok;
        }
        return false;
    }

    // vec2/vec3/vec4 defaults not implemented in this demo.
    return false;
}

static void ApplySchemaDefaults(const UISchema& schema, BindingContext& ctx, bool* ioDirty) {
    for (const auto& panel : schema.panels) {
        for (const auto& ctrl : panel.controls) {
            ApplySchemaDefaultForControl(ctrl, ctx, ioDirty);
        }
    }
}

static inline float ClampF(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static inline double ClampD(double v, double lo, double hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static inline double Log2D(double v) {
    return log(v) / log(2.0);
}

static inline double Exp2D(double v) {
    return exp(v * log(2.0));
}

static constexpr double kMinZoom = 1.0e-6;
static constexpr double kMaxLog2Zoom = 1020.0; // exp2(1020) ~ 1e307 (finite in double)

static inline double SafeZoomFromLog2(double log2Zoom) {
    return Exp2D(ClampD(log2Zoom, Log2D(1.0e-30), kMaxLog2Zoom));
}

static void SyncViewHpFromUi(ViewState& view) {
    view.center_hp_x = (double)view.center.x;
    view.center_hp_y = (double)view.center.y;
    view.log2_zoom = Log2D(fmax(1.0e-30, (double)view.zoom));
}

static void SyncViewUiFromHp(ViewState& view) {
    // Keep UI-facing floats finite.
    double z = SafeZoomFromLog2(view.log2_zoom);
    z = ClampD(z, 1.0e-30, 1.0e30);
    view.zoom = (float)z;
    view.center.x = (float)view.center_hp_x;
    view.center.y = (float)view.center_hp_y;
}

static void ApplyAutoDivePerFrame(ViewState& view, bool* ioDirty) {
    if (!view.auto_dive) return;
    if (view.camera_behavior == CameraBehavior::manual || view.camera_behavior == CameraBehavior::off) return;

    // Quick-stabilization pass (per informal spec feedback):
    // "Default behavior should be a simple center-zoom".
    // Per-frame deltas only; scaled only by dive_speed.
    float speed = fmaxf(0.0f, view.dive_speed);
    if (speed <= 0.0f) return;

    // Small, observable per-frame zoom-in toward the current view center.
    // Keep this intentionally simple and always non-zero.
    // A too-low max-zoom clamp looks like a "stall" once reached.
    double zoomFactor = 1.0 + 0.002 * (double)speed;
    double dlog2 = Log2D(zoomFactor);
    // No arbitrary max zoom; clamp only to keep the exponent finite.
    view.log2_zoom = ClampD(view.log2_zoom + dlog2, Log2D(kMinZoom), kMaxLog2Zoom);
    SyncViewUiFromHp(view);
    if (ioDirty) *ioDirty = true;
}

static void ApplyFractalPresetDefaults(const ViewState& view, KernelParams& params, bool* ioDirty) {
    // Explicit preset selection based on explicit fractal type.
    // This is NOT a fallback mechanism: we only apply this on (a) Reset All, or (b) when the user changes fractal_type.
    // View mapping controls (center/zoom/rotation) remain untouched.

    if (view.fractal_type == FractalType::newton) {
        params.max_iter = 500;
        params.epsilon = 1e-6f;
        params.nova_alpha = 0.50f;
        params.poly_kind = PolyKind::z3_minus_1;
        SetPolyPreset(params);
        params.coloring_mode = ColoringMode::joy_basins;
        params.exposure = 1.35f;
        params.color_saturation = 1.20f;
        params.color_contrast = 1.15f;
        params.color_tint_r = 1.00f;
        params.color_tint_g = 1.00f;
        params.color_tint_b = 1.00f;
        params.multibrot_power = 3;
        params.phoenix_p_real = -0.50f;
        params.phoenix_p_imag = 0.0f;
        if (ioDirty) *ioDirty = true;
        return;
    }

    if (view.fractal_type == FractalType::nova) {
        params.max_iter = 300;
        params.epsilon = 1e-6f;
        params.nova_alpha = 0.50f;
        params.poly_kind = PolyKind::z3_minus_1;
        SetPolyPreset(params);
        params.coloring_mode = ColoringMode::joy_basins;
        params.exposure = 1.35f;
        params.color_saturation = 1.20f;
        params.color_contrast = 1.15f;
        params.color_tint_r = 1.00f;
        params.color_tint_g = 1.00f;
        params.color_tint_b = 1.00f;
        params.multibrot_power = 3;
        params.phoenix_p_real = -0.50f;
        params.phoenix_p_imag = 0.0f;
        if (ioDirty) *ioDirty = true;
        return;
    }

    if (view.fractal_type == FractalType::explaino) {
        // Explaino defaults: joy-forward, stable, basin-based.
        params.max_iter = 500;
        params.epsilon = 1e-6f;
        params.nova_alpha = 0.50f;
        params.poly_kind = PolyKind::custom;
        params.coloring_mode = ColoringMode::joy_basins;
        params.exposure = 1.35f;
        params.color_saturation = 1.20f;
        params.color_contrast = 1.15f;
        params.color_tint_r = 1.00f;
        params.color_tint_g = 1.00f;
        params.color_tint_b = 1.00f;
        params.multibrot_power = 3;
        params.phoenix_p_real = -0.50f;
        params.phoenix_p_imag = 0.0f;
        params.explaino_seed = 1337;
        params.explaino_warp_strength = 0.0f;
        params.explaino_root_count = 0;
        if (ioDirty) *ioDirty = true;
        return;
    }

    if (view.fractal_type == FractalType::explaino_y || view.fractal_type == FractalType::explaino_fp) {
        // Explaino-family (Y-unfold / fixed-point flow): always bounded; joy-forward.
        params.max_iter = 650;
        params.epsilon = 1e-6f;
        params.nova_alpha = 0.50f;
        params.poly_kind = PolyKind::custom;
        params.coloring_mode = ColoringMode::joy_basins;
        params.exposure = 1.35f;
        params.color_saturation = 1.20f;
        params.color_contrast = 1.15f;
        params.color_tint_r = 1.00f;
        params.color_tint_g = 1.00f;
        params.color_tint_b = 1.00f;
        params.multibrot_power = 3;
        params.phoenix_p_real = -0.50f;
        params.phoenix_p_imag = 0.0f;
        params.explaino_seed = 1337;
        params.explaino_warp_strength = 0.0f;
        params.explaino_root_count = 0;
        if (ioDirty) *ioDirty = true;
        return;
    }

    if (view.fractal_type == FractalType::phoenix) {
        params.max_iter = 800;
        params.epsilon = 1e-6f;
        params.nova_alpha = 0.50f;
        params.phoenix_p_real = -0.50f;
        params.phoenix_p_imag = 0.0f;
        params.coloring_mode = ColoringMode::smooth_escape;
        params.exposure = 1.15f;
        params.color_saturation = 1.10f;
        params.color_contrast = 1.10f;
        params.color_tint_r = 1.00f;
        params.color_tint_g = 1.00f;
        params.color_tint_b = 1.00f;
        params.multibrot_power = 3;
        if (ioDirty) *ioDirty = true;
        return;
    }

    // Escape-time family.
    params.max_iter = 800;
    params.coloring_mode = ColoringMode::smooth_escape;
    params.exposure = 1.15f;
    params.color_saturation = 1.10f;
    params.color_contrast = 1.10f;
    params.color_tint_r = 1.00f;
    params.color_tint_g = 1.00f;
    params.color_tint_b = 1.00f;
    params.multibrot_power = 3;
    if (ioDirty) *ioDirty = true;
}

static inline uint32_t HashU32(uint32_t x) {
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

static inline float Hash01(uint32_t x) {
    // 24-bit fraction
    return (float)(HashU32(x) & 0x00ffffffU) / (float)0x01000000U;
}

// Flashlight manifold sampler (shared invariant): the LIVE trace and the headless probe
// MUST use the same (seed32, walkT) -> (rx, ry, rz) banded schedule.
struct FlashlightManifoldStep {
    int band = 0;
    float bandScale = 1.0f;
    float rx = 0.0f;
    float ry = 0.0f;
    float rz = 0.0f;
    double log2ZoomTick = 0.0;
    double dxWorld = 0.0;
    double dyWorld = 0.0;
};

static inline FlashlightManifoldStep FlashlightManifoldAt(
    uint32_t seed32,
    int walkT,
    int bands,
    double baseLog2,
    float radius,
    float zoomRadius,
    double aspect)
{
    FlashlightManifoldStep s;
    if (bands < 1) bands = 1;
    s.band = walkT % bands;
    s.bandScale = powf(0.5f, (float)s.band);

    const uint32_t h0 = HashU32(seed32 ^ (uint32_t)(walkT * 0x9e3779b9u));
    const uint32_t h1 = HashU32(seed32 ^ (uint32_t)(walkT * 0x85ebca6bu) ^ 0x13579bdu);
    const uint32_t h2 = HashU32(seed32 ^ (uint32_t)(walkT * 0xc2b2ae35u) ^ 0x2468aceu);
    s.rx = (Hash01(h0) - 0.5f) * 2.0f;
    s.ry = (Hash01(h1) - 0.5f) * 2.0f;
    s.rz = (Hash01(h2) - 0.5f) * 2.0f;

    s.log2ZoomTick = ClampD(baseLog2 + (double)(zoomRadius * s.bandScale * s.rz), Log2D(kMinZoom), kMaxLog2Zoom);
    const double zoomTick = SafeZoomFromLog2(s.log2ZoomTick);
    const double baseTick = 2.0 / fmax(1e-30, zoomTick);
    s.dxWorld = (double)(radius * s.bandScale * s.rx) * baseTick * aspect;
    s.dyWorld = (double)(radius * s.bandScale * s.ry) * baseTick;
    return s;
}

static inline float LerpF(float a, float b, float t) {
    return a + (b - a) * t;
}

struct ExplainoSeedShape {
    float a;
    float b;
    float c;
    float d;
};

static ExplainoSeedShape ExplainoShapeForSeed(uint32_t s, float phase, float spread) {
    float r0 = Hash01(s ^ 0x13579bdu);
    float r1 = Hash01(s ^ 0x2468aceu);
    float r2 = Hash01(s ^ 0xdeadbeefu);
    float r3 = Hash01(s ^ 0x9e3779b9u);

    float baseR = 0.85f + 0.95f * spread;

    float aAng = (r0 * 6.2831853f) + 0.35f * sinf(phase * (0.15f + 0.20f * r2));
    float cAng = (r1 * 6.2831853f) + 0.35f * cosf(phase * (0.13f + 0.23f * r3));

    ExplainoSeedShape out;
    out.a = baseR * cosf(aAng);
    out.b = (0.25f + 0.95f * fabsf(sinf(aAng + 0.7f))) * (0.65f + 0.45f * r2);
    out.c = baseR * cosf(cAng);
    out.d = (0.25f + 0.95f * fabsf(sinf(cAng - 0.4f))) * (0.65f + 0.45f * r3);
    return out;
}

static void UpdateExplainoPolynomial(const ViewState& view, KernelParams& params, bool* ioDirty) {
    if (view.fractal_type != FractalType::explaino && view.fractal_type != FractalType::explaino_y && view.fractal_type != FractalType::explaino_fp) {
        params.explaino_root_count = 0;
        return;
    }

    // Explaino is always a custom quartic with real coefficients.
    // We build it as (z^2 - 2a z + (a^2+b^2)) * (z^2 - 2c z + (c^2+d^2)).
    params.poly_kind = PolyKind::custom;
    params.explaino_root_count = 4;

    float drift = view.explaino_seed_drift;
    float driftFloor = floorf(drift);
    float driftFrac = drift - driftFloor; // [0,1)
    if (!isfinite(driftFrac)) driftFrac = 0.0f;
    driftFrac = ClampF(driftFrac, 0.0f, 1.0f);

    int seedBase = params.explaino_seed + (int)driftFloor;
    uint32_t s0 = (uint32_t)seedBase;
    uint32_t s1 = (uint32_t)(seedBase + 1);
    float phase = view.explaino_phase;

    // Spread controls "how far roots are" which changes basin geometry.
    float spread = fmaxf(0.0f, fminf(1.0f, params.explaino_warp_strength));

    ExplainoSeedShape sh0 = ExplainoShapeForSeed(s0, phase, spread);
    ExplainoSeedShape sh = sh0;
    if (view.explaino_seed_tween && driftFrac > 0.0f) {
        ExplainoSeedShape sh1 = ExplainoShapeForSeed(s1, phase, spread);
        sh.a = LerpF(sh0.a, sh1.a, driftFrac);
        sh.b = LerpF(sh0.b, sh1.b, driftFrac);
        sh.c = LerpF(sh0.c, sh1.c, driftFrac);
        sh.d = LerpF(sh0.d, sh1.d, driftFrac);
    }

    float a = sh.a;
    float b = sh.b;
    float c = sh.c;
    float d = sh.d;

    // Store roots (two conjugate pairs). Only the +imag members are unique, but we store all 4.
    params.explaino_roots[0] = {a, b};
    params.explaino_roots[1] = {a, -b};
    params.explaino_roots[2] = {c, d};
    params.explaino_roots[3] = {c, -d};

    // Quadratic factors: z^2 + q1 z + q0
    // Here q1 = -2a, q0 = a^2+b^2; and similarly for (c,d).
    float q1 = -2.0f * a;
    float q0 = a * a + b * b;
    float r1c = -2.0f * c;
    float r0c = c * c + d * d;

    // Multiply (z^2 + q1 z + q0) * (z^2 + r1c z + r0c)
    // => z^4 + (q1+r1c) z^3 + (q0 + q1*r1c + r0c) z^2 + (q0*r1c + q1*r0c) z + (q0*r0c)
    float c4 = 1.0f;
    float c3 = q1 + r1c;
    float c2 = q0 + q1 * r1c + r0c;
    float c1 = q0 * r1c + q1 * r0c;
    float c0 = q0 * r0c;

    params.poly_coeffs[0] = c0;
    params.poly_coeffs[1] = c1;
    params.poly_coeffs[2] = c2;
    params.poly_coeffs[3] = c3;
    params.poly_coeffs[4] = c4;

    if (ioDirty) *ioDirty = true;
}

static bool ValidateSchemaBindings(const UISchema& schema, const std::string& schemaPath, BindingContext& ctx, std::string* outError) {
    for (const auto& panel : schema.panels) {
        for (const auto& c : panel.controls) {
            if (!c.has_binding) continue;

            const auto& b = c.binding;
            if (b.path.empty() || b.kind.empty()) {
                if (outError) *outError = "Schema binding missing kind/path for control: " + c.id;
                UiErrorOrFail("schema.binding_missing", schemaPath, c.id, outError ? *outError : "Schema binding missing kind/path");
                return false;
            }

            if (b.kind == "action") {
                if (!(b.path == "fractal.actions.render_once" || b.path == "fractal.actions.reset_view" || b.path == "fractal.actions.reset_all" ||
                      b.path == "fractal.actions.export_state" || b.path == "fractal.actions.import_state" ||
                      b.path == "fractal.actions.capture_diagnostic" ||
                      b.path == "fractal.actions.next_seed" || b.path == "fractal.actions.prev_seed")) {
                    if (outError) *outError = "Unknown action binding path: " + b.path + " (control: " + c.id + ")";
                    UiErrorOrFail("schema.action_unknown", schemaPath, b.path, outError ? *outError : "Unknown action binding path");
                    return false;
                }
                continue;
            }

            if (b.kind != "param") {
                if (outError) *outError = "Unknown binding kind: " + b.kind + " (control: " + c.id + ")";
                UiErrorOrFail("schema.binding_kind_unknown", schemaPath, b.kind, outError ? *outError : "Unknown binding kind");
                return false;
            }

            if (c.value_type == "bool") {
                bool* ptr = nullptr;
                if (!ctx.BindBool(b.path, &ptr) || !ptr) {
                    if (outError) *outError = "Bind failed for bool path: " + b.path + " (control: " + c.id + ")";
                    UiErrorOrFail("schema.bind_failed", schemaPath, b.path, outError ? *outError : "Bind failed for bool path");
                    return false;
                }
            } else if (c.value_type == "int") {
                int* ptr = nullptr;
                if (!ctx.BindInt(b.path, &ptr) || !ptr) {
                    if (outError) *outError = "Bind failed for int path: " + b.path + " (control: " + c.id + ")";
                    UiErrorOrFail("schema.bind_failed", schemaPath, b.path, outError ? *outError : "Bind failed for int path");
                    return false;
                }
            } else if (c.value_type == "float") {
                float* ptr = nullptr;
                if (!ctx.BindFloat(b.path, &ptr) || !ptr) {
                    if (outError) *outError = "Bind failed for float path: " + b.path + " (control: " + c.id + ")";
                    UiErrorOrFail("schema.bind_failed", schemaPath, b.path, outError ? *outError : "Bind failed for float path");
                    return false;
                }
            } else if (c.value_type == "enum") {
                // Validate enum paths explicitly.
                if (!(b.path == "fractal.view.fractal_type" || b.path == "fractal.view.camera_behavior" || b.path == "fractal.params.poly_kind" ||
                      b.path == "fractal.params.coloring_mode")) {
                    if (outError) *outError = "Unknown enum binding path: " + b.path + " (control: " + c.id + ")";
                    UiErrorOrFail("schema.enum_path_unknown", schemaPath, b.path, outError ? *outError : "Unknown enum binding path");
                    return false;
                }
            }
        }
    }
    return true;
}

static bool RenderControlFromSchema(const UISchemaControl& c, BindingContext& ctx, bool* ioDirty, bool* ioRenderOnce) {
    if (c.has_visible_if) {
        if (!ctx.EvalVisibleIf(c.visible_if)) return false;
    }

    ImGui::PushID(c.id.c_str());

    if (c.has_help) {
        ImGui::TextDisabled("?");
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::TextWrapped("%s", c.help.c_str());
            ImGui::EndTooltip();
        }
        ImGui::SameLine();
    }

    if (!c.has_binding) {
        FailFastUiError("ui.control_unbound", ctx.schema_path, c.id, "Control has no binding: " + c.id);
        ImGui::TextDisabled("%s (UNBOUND)", c.label.c_str());
        ImGui::PopID();
        return false;
    }

    const auto& b = c.binding;
    if (c.type == "button") {
        if (b.kind != "action") {
            FailFastUiError("ui.bad_action_binding", ctx.schema_path, b.path, "Button control has non-action binding kind: " + b.kind + " (control: " + c.id + ")");
            ImGui::TextDisabled("%s (bad action binding)", c.label.c_str());
            ImGui::PopID();
            return false;
        }
        if (ImGui::Button(c.label.c_str())) {
            if (b.path == "fractal.actions.render_once") {
                if (ioRenderOnce) *ioRenderOnce = true;
            }
        }
        ImGui::PopID();
        return true;
    }

    if (b.kind != "param") {
        FailFastUiError("ui.bad_param_binding", ctx.schema_path, b.path, "Non-button control has non-param binding kind: " + b.kind + " (control: " + c.id + ")");
        ImGui::TextDisabled("%s (bad param binding)", c.label.c_str());
        ImGui::PopID();
        return false;
    }

    if (c.type == "checkbox") {
        bool* ptr = nullptr;
        if (!ctx.BindBool(b.path, &ptr) || !ptr) {
            FailFastUiError("ui.bind_failed", ctx.schema_path, b.path, "BindBool failed for path: " + b.path + " (control: " + c.id + ")");
            ImGui::TextDisabled("%s (bind failed)", c.label.c_str());
            ImGui::PopID();
            return false;
        }
        bool changed = ImGui::Checkbox(c.label.c_str(), ptr);
        if (changed && ioDirty) *ioDirty = true;
        ImGui::PopID();
        return changed;
    }

    if (c.type == "slider_int" || c.type == "drag_int") {
        int* ptr = nullptr;
        if (!ctx.BindInt(b.path, &ptr) || !ptr) {
            FailFastUiError("ui.bind_failed", ctx.schema_path, b.path, "BindInt failed for path: " + b.path + " (control: " + c.id + ")");
            ImGui::TextDisabled("%s (bind failed)", c.label.c_str());
            ImGui::PopID();
            return false;
        }

        int minV = c.has_min ? (int)c.min : 0;
        int maxV = c.has_max ? (int)c.max : 100;

        bool changed = false;
        if (c.type == "slider_int") {
            changed = ImGui::SliderInt(c.label.c_str(), ptr, minV, maxV, c.has_format ? c.format.c_str() : "%d");
        } else {
            float speed = c.has_step ? (float)c.step : 1.0f;
            changed = ImGui::DragInt(c.label.c_str(), ptr, speed, minV, maxV, c.has_format ? c.format.c_str() : "%d");
        }
        if (changed && ioDirty) *ioDirty = true;
        ImGui::PopID();
        return changed;
    }

    if (c.type == "slider_float" || c.type == "drag_float") {
        float* ptr = nullptr;
        if (!ctx.BindFloat(b.path, &ptr) || !ptr) {
            FailFastUiError("ui.bind_failed", ctx.schema_path, b.path, "BindFloat failed for path: " + b.path + " (control: " + c.id + ")");
            ImGui::TextDisabled("%s (bind failed)", c.label.c_str());
            ImGui::PopID();
            return false;
        }

        float minV = c.has_min ? (float)c.min : 0.0f;
        float maxV = c.has_max ? (float)c.max : 1.0f;
        float speed = c.has_step ? (float)c.step : 0.01f;

        bool changed = false;
        if (c.type == "slider_float") {
            changed = ImGui::SliderFloat(c.label.c_str(), ptr, minV, maxV, c.has_format ? c.format.c_str() : "%.3f");
        } else {
            changed = ImGui::DragFloat(c.label.c_str(), ptr, speed, minV, maxV, c.has_format ? c.format.c_str() : "%.3f");
        }
        if (changed && ioDirty) *ioDirty = true;
        ImGui::PopID();
        return changed;
    }

    if (c.type == "combo") {
        if (c.value_type == "enum") {
            std::string cur = ctx.GetEnumId(b.path);
            int curIndex = 0;
            for (int i = 0; i < (int)c.options.size(); i++) {
                if (c.options[i].id == cur) { curIndex = i; break; }
            }

            std::vector<const char*> labels;
            labels.reserve(c.options.size());
            for (const auto& o : c.options) labels.push_back(o.label.c_str());

            bool changed = false;
            if (!labels.empty() && ImGui::Combo(c.label.c_str(), &curIndex, labels.data(), (int)labels.size())) {
                if (curIndex >= 0 && curIndex < (int)c.options.size()) {
                    changed = ctx.SetEnumId(b.path, c.options[curIndex].id);
                    if (!changed) {
                        UiErrorOrFail("ui.enum_set_failed", ctx.schema_path, b.path, "SetEnumId failed for path: " + b.path + " (value: " + c.options[curIndex].id + ", control: " + c.id + ")");
                    }
                    if (changed && ioDirty) *ioDirty = true;
                }
            }
            ImGui::PopID();
            return changed;
        }

        if (c.value_type == "int") {
            int* ptr = nullptr;
            if (!ctx.BindInt(b.path, &ptr) || !ptr) {
                FailFastUiError("ui.bind_failed", ctx.schema_path, b.path, "BindInt failed for path: " + b.path + " (control: " + c.id + ")");
                ImGui::TextDisabled("%s (bind failed)", c.label.c_str());
                ImGui::PopID();
                return false;
            }

            int curIndex = 0;
            for (int i = 0; i < (int)c.options.size(); i++) {
                try {
                    int optV = std::stoi(c.options[i].id);
                    if (optV == *ptr) { curIndex = i; break; }
                } catch (...) {
                    // ignore malformed option ids
                }
            }

            std::vector<const char*> labels;
            labels.reserve(c.options.size());
            for (const auto& o : c.options) labels.push_back(o.label.c_str());

            bool changed = false;
            if (!labels.empty() && ImGui::Combo(c.label.c_str(), &curIndex, labels.data(), (int)labels.size())) {
                if (curIndex >= 0 && curIndex < (int)c.options.size()) {
                    int newV = *ptr;
                    try { newV = std::stoi(c.options[curIndex].id); } catch (...) { newV = *ptr; }
                    newV = NormalizeLensDownsamplePow2(newV);
                    if (*ptr != newV) {
                        *ptr = newV;
                        changed = true;
                        if (ioDirty) *ioDirty = true;
                    }
                }
            }
            ImGui::PopID();
            return changed;
        }

        UiErrorOrFail("ui.combo_value_type_unsupported", ctx.schema_path, c.id, "Combo control unsupported value_type: " + c.value_type + " (control: " + c.id + ")");
        ImGui::TextDisabled("%s (unsupported combo value_type: %s)", c.label.c_str(), c.value_type.c_str());
        ImGui::PopID();
        return false;
    }

    UiErrorOrFail("ui.control_type_unsupported", ctx.schema_path, c.id, "Unsupported control type: " + c.type + " (control: " + c.id + ")");
    ImGui::TextDisabled("%s (unsupported control type: %s)", c.label.c_str(), c.type.c_str());
    ImGui::PopID();
    return false;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    // Command-line switches:
    //  - --validate-ui : load schema + validate bindings + apply defaults, then exit(0) or exit(1)
    //  - --ui-soft     : do not terminate on UI/schema errors; fall back to safe-mode UI and show error overlay
    //  - --capture-diagnostic : headless render + write ..\\ui\\diagnostics\\last\\{frame.bmp,state.json,ui_error.json?}
    //  - --probe-sdf <x> <y> : headless render + compute lens chamfer SDF + write ..\\ui\\diagnostics\\last\\probe_sdf.json
    //  - --flashlight-probe <path> : headless multi-tick probe seeded from a text file; writes ..\\ui\\diagnostics\\last\\flashlight_probe.json
    //      Optional: --flashlight-ticks <N> (default 8), --flashlight-radius <r> (default 0.75), --flashlight-zoom-radius <r> (default 0.25)
    //  - --flashlight-live <path> : same probe, but runs with the interactive window and draws live debug lenses + vector trace
    //  - --ui-schema <path> : explicit schema path (disables implicit search)
    //  - --no-messagebox : disable MessageBox popups (useful for CI / scripts)
    const std::vector<std::string> args = GetCommandLineArgsUtf8();
    g_uiSoftErrors = HasArg(args, "--ui-soft");
    const bool validateUiOnly = HasArg(args, "--validate-ui");
    const bool captureDiagnosticOnly = HasArg(args, "--capture-diagnostic");

    bool probeSdfOnly = false;
    int probeSdfX = 0;
    int probeSdfY = 0;
    if (HasArg(args, "--probe-sdf")) {
        probeSdfOnly = true;
        g_uiSoftErrors = true;
        g_uiNoMessageBox = true;

        // Parse: --probe-sdf <x> <y>
        bool ok = false;
        for (size_t i = 0; i < args.size(); i++) {
            if (args[i] == "--probe-sdf") {
                if (i + 2 < args.size()) {
                    try {
                        probeSdfX = std::stoi(args[i + 1]);
                        probeSdfY = std::stoi(args[i + 2]);
                        ok = true;
                    } catch (...) {
                        ok = false;
                    }
                }
                break;
            }
        }
        if (!ok) {
            ReportUiErrorOnce("args.missing", "", "--probe-sdf", "Usage: --probe-sdf <x> <y>");
            return 1;
        }
    }

    bool flashlightProbeOnly = false;
    std::string flashlightPath;
    const int kFlashlightTicksMax = 4096;
    int flashlightTicks = 8;
    float flashlightRadius = 0.75f;
    float flashlightZoomRadius = 0.25f;
    float flashlightWarp = 0.0f;
    bool flashlightClosureLast = false;
    int flashlightClosureRefT = 0;
    std::string flashlightFractalType;

    // Shared flashlight tuning flags (used by both --flashlight-probe and --flashlight-live).
    // Note: historically these were only parsed for --flashlight-probe, which caused LIVE mode
    // to silently stick to defaults (e.g., ticks=8). Keep them unified here.
    {
        std::string tmp;
        if (TryGetArgValue(args, "--flashlight-ticks", tmp) && !tmp.empty()) {
            try { flashlightTicks = std::stoi(tmp); } catch (...) { flashlightTicks = 8; }
        }
        if (TryGetArgValue(args, "--flashlight-radius", tmp) && !tmp.empty()) {
            try { flashlightRadius = (float)std::stod(tmp); } catch (...) { flashlightRadius = 0.75f; }
        }
        if (TryGetArgValue(args, "--flashlight-zoom-radius", tmp) && !tmp.empty()) {
            try { flashlightZoomRadius = (float)std::stod(tmp); } catch (...) { flashlightZoomRadius = 0.25f; }
        }
        if (TryGetArgValue(args, "--flashlight-warp", tmp) && !tmp.empty()) {
            try { flashlightWarp = (float)std::stod(tmp); } catch (...) { flashlightWarp = 0.0f; }
        }

        // Optional: choose the fractal family implementation for the probe.
        // Default is explaino_fp (flow) for stability and demo-forward motion.
        TryGetArgValue(args, "--flashlight-fractal-type", flashlightFractalType);

        // Closure schedule: make the final tick revisit the base state (camera + explaino drift/phase).
        flashlightClosureLast = HasArg(args, "--flashlight-closure") || HasArg(args, "--flashlight-closure-last");
        flashlightClosureRefT = 0;

        if (flashlightTicks < 1) flashlightTicks = 1;
        if (flashlightTicks > kFlashlightTicksMax) flashlightTicks = kFlashlightTicksMax; // keep bounded and predictable
        flashlightRadius = ClampF(flashlightRadius, 0.0f, 10.0f);
        flashlightZoomRadius = ClampF(flashlightZoomRadius, 0.0f, 10.0f);
        flashlightWarp = ClampF(flashlightWarp, 0.0f, 1.0f);
        if (flashlightClosureRefT < 0) flashlightClosureRefT = 0;
    }
    if (HasArg(args, "--flashlight-probe")) {
        flashlightProbeOnly = true;
        g_uiSoftErrors = true;
        g_uiNoMessageBox = true;

        if (!TryGetArgValue(args, "--flashlight-probe", flashlightPath) || flashlightPath.empty()) {
            ReportUiErrorOnce("args.missing", "", "--flashlight-probe", "Usage: --flashlight-probe <path-to-text>");
            return 1;
        }

    }

    bool flashlightLive = false;
    std::string flashlightLivePath;
    int flashlightLiveTicks = flashlightTicks;
    float flashlightLiveRadius = flashlightRadius;
    float flashlightLiveZoomRadius = flashlightZoomRadius;
    float flashlightLiveWarp = flashlightWarp;
    bool flashlightLiveClosureLast = flashlightClosureLast;
    int flashlightLiveClosureRefT = flashlightClosureRefT;
    uint32_t flashlightLiveSeed32 = 0;
    uint32_t flashlightLiveSpectrum8[8] = {};
    std::string flashlightLiveFractalType;
    if (HasArg(args, "--flashlight-live")) {
        flashlightLive = true;
        g_uiSoftErrors = true;

        if (!TryGetArgValue(args, "--flashlight-live", flashlightLivePath) || flashlightLivePath.empty()) {
            ReportUiErrorOnce("args.missing", "", "--flashlight-live", "Usage: --flashlight-live <path-to-text>");
            return 1;
        }

        // Share the same tuning flags as the headless probe (if present).
        flashlightLiveTicks = flashlightTicks;
        flashlightLiveRadius = flashlightRadius;
        flashlightLiveZoomRadius = flashlightZoomRadius;
        flashlightLiveWarp = flashlightWarp;
        flashlightLiveClosureLast = flashlightClosureLast;
        flashlightLiveClosureRefT = flashlightClosureRefT;
        flashlightLiveFractalType = flashlightFractalType;

        // Fail-fast seed read: we want the UI session to be deterministic.
        std::string text;
        if (!ReadTextFile(flashlightLivePath, text)) {
            ReportUiErrorOnce("probe.read_failed", "", flashlightLivePath, "Failed to read flashlight seed text file");
            return 1;
        }
        flashlightLiveSeed32 = Fnv1a32(text.data(), text.size());
        ComputeConversationSpectrum8(text, flashlightLiveSpectrum8);
    }

    if (HasArg(args, "--no-messagebox")) {
        g_uiNoMessageBox = true;
    }

    // Validate-only should never hard-terminate (we want a fast 0/1 result).
    if (validateUiOnly) {
        g_uiSoftErrors = true;
        g_uiNoMessageBox = true;
    }

    // Capture-only should never hard-terminate (we want a fast 0/1 result).
    if (captureDiagnosticOnly) {
        g_uiSoftErrors = true;
        g_uiNoMessageBox = true;
    }

    if (probeSdfOnly && (validateUiOnly || captureDiagnosticOnly || flashlightProbeOnly)) {
        ReportUiErrorOnce("args.conflict", "", "--probe-sdf", "--probe-sdf cannot be combined with --validate-ui or --capture-diagnostic");
        return 1;
    }

    if (flashlightProbeOnly && (validateUiOnly || captureDiagnosticOnly)) {
        ReportUiErrorOnce("args.conflict", "", "--flashlight-probe", "--flashlight-probe cannot be combined with --validate-ui or --capture-diagnostic");
        return 1;
    }

    if (flashlightLive && (validateUiOnly || captureDiagnosticOnly || probeSdfOnly || flashlightProbeOnly)) {
        ReportUiErrorOnce("args.conflict", "", "--flashlight-live", "--flashlight-live cannot be combined with headless modes (--validate-ui/--capture-diagnostic/--probe-sdf/--flashlight-probe)");
        return 1;
    }

    std::string schemaOverride;
    TryGetArgValue(args, "--ui-schema", schemaOverride);

    std::string exeDirEarly = GetExeDir();
    std::vector<std::string> schemaCandidatesEarly;
    if (!schemaOverride.empty()) {
        // Deterministic override (preferred for scripts/CI).
        schemaCandidatesEarly.push_back(schemaOverride);
    } else {
        // Fail-fast by default: only try the primary editable schema next to the repo artifacts.
        // (No canonical fallback; no CWD-relative probing.)
        schemaCandidatesEarly.push_back(JoinPath(exeDirEarly, "..\\ui\\fractal_binding_surface_v1.ui_schema.json"));
    }

    if (validateUiOnly) {
        // Validation mode: no window, no D3D. This is for fast post-edit checks.
        UISchema schema;
        std::string schemaPath;
        std::string stage;
        std::string detail;
        if (!TryLoadSchemaFromCandidates(schemaCandidatesEarly, schema, schemaPath, stage, detail)) {
            ReportUiErrorOnce(stage.c_str(), schemaPath, schemaPath, detail);
            return 1;
        }

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        bool dirty = false;

        BindingContext ctx;
        ctx.schema_path = schemaPath;
        ctx.view = &view;
        ctx.params = &params;
        ctx.render = &render;
        ctx.lens = &lens;

        std::string bindError;
        if (!ValidateSchemaBindings(schema, schemaPath, ctx, &bindError)) {
            // ValidateSchemaBindings already reported in strict mode; mirror here for completeness.
            ReportUiErrorOnce("schema.validate", schemaPath, schemaPath, bindError.empty() ? "Schema binding validation failed" : bindError);
            return 1;
        }

        ApplySchemaDefaults(schema, ctx, &dirty);

        // Invariants for fast bug-fix workflows: these must exist in the loaded schema.
        if (!SchemaHasBinding(schema, "param", "fractal.lens.enabled")) {
            ReportUiErrorOnce("schema.invariant_missing", schemaPath, "fractal.lens.enabled", "Schema missing required binding (param): fractal.lens.enabled");
            return 1;
        }
        if (!SchemaHasBinding(schema, "action", "fractal.actions.capture_diagnostic")) {
            ReportUiErrorOnce("schema.invariant_missing", schemaPath, "fractal.actions.capture_diagnostic", "Schema missing required binding (action): fractal.actions.capture_diagnostic");
            return 1;
        }
        return 0;
    }

    if (captureDiagnosticOnly) {
        // Headless diagnostic capture: render once and write a single overwrite-last bundle.
        UISchema schema;
        std::string schemaPath;
        std::string stage;
        std::string detail;
        if (!TryLoadSchemaFromCandidates(schemaCandidatesEarly, schema, schemaPath, stage, detail)) {
            ReportUiErrorOnce(stage.c_str(), schemaPath, schemaPath, detail);
            return 1;
        }

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        bool dirty = false;

        BindingContext ctx;
        ctx.schema_path = schemaPath;
        ctx.view = &view;
        ctx.params = &params;
        ctx.render = &render;
        ctx.lens = &lens;

        std::string bindError;
        if (!ValidateSchemaBindings(schema, schemaPath, ctx, &bindError)) {
            ReportUiErrorOnce("schema.validate", schemaPath, schemaPath, bindError.empty() ? "Schema binding validation failed" : bindError);
            return 1;
        }

        ApplySchemaDefaults(schema, ctx, &dirty);

        // Same invariants as validate-only: ensures headless captures always include lens toggle + diag capture action in the schema.
        if (!SchemaHasBinding(schema, "param", "fractal.lens.enabled")) {
            ReportUiErrorOnce("schema.invariant_missing", schemaPath, "fractal.lens.enabled", "Schema missing required binding (param): fractal.lens.enabled");
            return 1;
        }
        if (!SchemaHasBinding(schema, "action", "fractal.actions.capture_diagnostic")) {
            ReportUiErrorOnce("schema.invariant_missing", schemaPath, "fractal.actions.capture_diagnostic", "Schema missing required binding (action): fractal.actions.capture_diagnostic");
            return 1;
        }

        // Ensure derived fields are coherent with the chosen fractal.
        ApplyFractalPresetDefaults(view, params, &dirty);
        UpdateExplainoPolynomial(view, params, &dirty);
        SyncViewHpFromUi(view);

        // Sensible headless render defaults.
        if (render.resolution.x <= 0 || render.resolution.y <= 0) {
            render.resolution = { 1024, 768 };
        }
        if (render.block_size <= 0) render.block_size = 256;
        if (render.device_id < 0) render.device_id = 0;
        render.benchmark = false;

        std::vector<uint32_t> rgba;
        rgba.resize((size_t)render.resolution.x * (size_t)render.resolution.y);

        std::vector<uint8_t> mask;
        uint8_t* outMask = nullptr;
        if (lens.enabled) {
            mask.resize((size_t)render.resolution.x * (size_t)render.resolution.y);
            outMask = mask.data();
        }

        const char* err = nullptr;
        RenderStats stats{};
        if (!RenderFractalCUDA(view, params, render, rgba.data(), outMask, &stats, &err)) {
            ReportUiErrorOnce("render.cuda", schemaPath, "RenderFractalCUDA", err ? err : "Render failed");
            return 1;
        }

        if (!CaptureDiagnosticsLastBundle(schemaPath, view, params, render, lens, rgba.data(), render.resolution.x, render.resolution.y)) {
            // CaptureDiagnosticsLastBundle already reported an error artifact via UiErrorOrFail.
            return 1;
        }

        return 0;
    }

    if (probeSdfOnly) {
        // Headless SDF probe: render once, compute lens chamfer SDF, and emit a probe artifact.
        UISchema schema;
        std::string schemaPath;
        std::string stage;
        std::string detail;
        if (!TryLoadSchemaFromCandidates(schemaCandidatesEarly, schema, schemaPath, stage, detail)) {
            ReportUiErrorOnce(stage.c_str(), schemaPath, schemaPath, detail);
            return 1;
        }

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        bool dirty = false;

        BindingContext ctx;
        ctx.schema_path = schemaPath;
        ctx.view = &view;
        ctx.params = &params;
        ctx.render = &render;
        ctx.lens = &lens;

        std::string bindError;
        if (!ValidateSchemaBindings(schema, schemaPath, ctx, &bindError)) {
            ReportUiErrorOnce("schema.validate", schemaPath, schemaPath, bindError.empty() ? "Schema binding validation failed" : bindError);
            return 1;
        }

        ApplySchemaDefaults(schema, ctx, &dirty);

        // Force lens on for a probe (we need a mask to compute SDF).
        lens.enabled = true;

        ApplyFractalPresetDefaults(view, params, &dirty);
        UpdateExplainoPolynomial(view, params, &dirty);
        SyncViewHpFromUi(view);

        if (render.resolution.x <= 0 || render.resolution.y <= 0) {
            render.resolution = { 1024, 768 };
        }
        if (render.block_size <= 0) render.block_size = 256;
        if (render.device_id < 0) render.device_id = 0;
        render.benchmark = false;

        std::vector<uint32_t> rgba;
        rgba.resize((size_t)render.resolution.x * (size_t)render.resolution.y);

        std::vector<uint8_t> mask;
        mask.resize((size_t)render.resolution.x * (size_t)render.resolution.y);

        const char* err = nullptr;
        RenderStats stats{};
        if (!RenderFractalCUDA(view, params, render, rgba.data(), mask.data(), &stats, &err)) {
            ReportUiErrorOnce("render.cuda", schemaPath, "RenderFractalCUDA", err ? err : "Render failed");
            return 1;
        }

        // Always capture the usual proof bundle as well.
        if (!CaptureDiagnosticsLastBundle(schemaPath, view, params, render, lens, rgba.data(), render.resolution.x, render.resolution.y)) {
            return 1;
        }

        const int ds = NormalizeLensDownsamplePow2(lens.downsample);
        std::vector<uint8_t> lensMaskLow;
        int lensW = 0;
        int lensH = 0;
        DownsampleMaskPow2(mask.data(), render.resolution.x, render.resolution.y, ds, lensMaskLow, lensW, lensH);
        const float maxAbsPxLow = 48.0f / (float)ds;

        float signedPx = 0.0f;
        bool inside = false;
        if (!SampleSignedDistanceSdfChamfer(lensMaskLow.data(), lensW, lensH, probeSdfX, probeSdfY, signedPx, inside)) {
            ReportUiErrorOnce("probe.bounds", schemaPath, "--probe-sdf", "Probe coordinate out of bounds for lens_low buffer");
            return 1;
        }

        if (!WriteProbeSdfLastBundle(schemaPath, view, params, render, lens, probeSdfX, probeSdfY, lensW, lensH, ds, maxAbsPxLow, signedPx, inside)) {
            return 1;
        }

        return 0;
    }

    if (flashlightProbeOnly) {
        // CUDA runtime flashlight probe: derive a deterministic seed from a conversation/history text file,
        // then explore multi-scale local perturbations (FFT-like bands) across a bounded number of ticks.
        std::string text;
        if (!ReadTextFile(flashlightPath, text)) {
            ReportUiErrorOnce("probe.read_failed", "", flashlightPath, "Failed to read flashlight seed text file");
            return 1;
        }
        const uint32_t seed32 = Fnv1a32(text.data(), text.size());
        uint32_t spectrum8[8] = {};
        ComputeConversationSpectrum8(text, spectrum8);

        UISchema schema;
        std::string schemaPath;
        std::string stage;
        std::string detail;
        if (!TryLoadSchemaFromCandidates(schemaCandidatesEarly, schema, schemaPath, stage, detail)) {
            ReportUiErrorOnce(stage.c_str(), schemaPath, schemaPath, detail);
            return 1;
        }

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        bool dirty = false;

        BindingContext ctx;
        ctx.schema_path = schemaPath;
        ctx.view = &view;
        ctx.params = &params;
        ctx.render = &render;
        ctx.lens = &lens;

        std::string bindError;
        if (!ValidateSchemaBindings(schema, schemaPath, ctx, &bindError)) {
            ReportUiErrorOnce("schema.validate", schemaPath, schemaPath, bindError.empty() ? "Schema binding validation failed" : bindError);
            return 1;
        }
        // Apply UI schema defaults so the headless probe matches what the UI would do.
        // (Probe safety still overrides camera automation below.)
        ApplySchemaDefaults(schema, ctx, &dirty);

        // Force an Explain-o family target so the probe is sampling the seed-driven runtime.
        // Default: explaino_fp (flow).
        if (!flashlightFractalType.empty()) {
            FractalType ft = view.fractal_type;
            if (FractalTypeFromId(flashlightFractalType, ft)) {
                view.fractal_type = ft;
            } else {
                ReportUiErrorOnce("args.enum", schemaPath, "--flashlight-fractal-type", "Unknown fractal_type: " + flashlightFractalType);
                return 1;
            }
        } else {
            view.fractal_type = FractalType::explaino_fp;
        }
        lens.enabled = true;

        // Use preset defaults for this mode (keeps the headless probe aligned with the normal UI presets).
        ApplyFractalPresetDefaults(view, params, &dirty);

        // PROBE SAFETY: disable all camera automation.
        view.camera_behavior = CameraBehavior::manual;
        view.auto_dive = false;
        view.dive_speed = 0.0f;
        view.auto_refresh = false;

        // Explicit probe camera start.
        view.center = {0.0f, 0.0f};
        view.zoom = 1.0f;

        // Deterministic parameterization from the seed.
        params.explaino_seed = (int)seed32;
        // PROBE SAFETY: warp must be explicitly provided (external parameter). Default 0.
        params.explaino_warp_strength = flashlightWarp;
        view.explaino_seed_tween = true;
        view.explaino_alive = false;
        view.explaino_phase = 0.0f;
        view.explaino_seed_drift = 0.0f;

        UpdateExplainoPolynomial(view, params, &dirty);
        SyncViewHpFromUi(view);

        if (render.resolution.x <= 0 || render.resolution.y <= 0) {
            render.resolution = { 1024, 768 };
        }
        if (render.block_size <= 0) render.block_size = 256;
        if (render.device_id < 0) render.device_id = 0;
        render.benchmark = false;

        // Base camera in HP space (locality is relative to this root).
        const double baseCx = view.center_hp_x;
        const double baseCy = view.center_hp_y;
        const double baseLog2 = view.log2_zoom;
        const double aspect = (render.resolution.y > 0) ? (double)render.resolution.x / (double)render.resolution.y : 1.0;

        // Fixed set of probe points in normalized viewport space (local samples).
        // We sample the SDF on the downsampled lens buffer.
        struct ProbeUV { float u; float v; };
        const ProbeUV uvPts[5] = {
            {0.5f, 0.5f},
            {0.25f, 0.25f},
            {0.75f, 0.25f},
            {0.25f, 0.75f},
            {0.75f, 0.75f},
        };

        std::ostringstream js;
        js.setf(std::ios::fixed);
        js << std::setprecision(8);
        js << "{\n";
        js << "  \"version\": 1,\n";
        js << "  \"schema_path\": \"" << JsonEscape(schemaPath) << "\",\n";
        js << "  \"seed_path\": \"" << JsonEscape(flashlightPath) << "\",\n";
        js << "  \"conversation_seed32\": " << (uint32_t)seed32 << ",\n";
        js << "  \"spectrum8_u32\": [";
        for (int i = 0; i < 8; i++) {
            if (i) js << ", ";
            js << (uint32_t)spectrum8[i];
        }
        js << "],\n";
        js << "  \"ticks\": " << flashlightTicks << ",\n";
        js << "  \"radius\": " << (double)flashlightRadius << ",\n";
        js << "  \"zoom_radius\": " << (double)flashlightZoomRadius << ",\n";
        js << "  \"fractal_type\": \"" << FractalTypeId(view.fractal_type) << "\",\n";
        js << "  \"probe_params\": {\n";
        js << "    \"explaino_warp_strength\": " << (double)params.explaino_warp_strength << "\n";
        js << "  },\n";
        js << "  \"probe_safety\": {\n";
        js << "    \"applied_schema_defaults\": true,\n";
        js << "    \"camera_behavior\": \"manual\",\n";
        js << "    \"auto_dive\": false,\n";
        js << "    \"dive_speed\": 0.0\n";
        js << "  },\n";
        js << "  \"base_camera\": {\n";
        js << "    \"center_hp_x\": " << baseCx << ",\n";
        js << "    \"center_hp_y\": " << baseCy << ",\n";
        js << "    \"log2_zoom\": " << baseLog2 << "\n";
        js << "  },\n";
        js << "  \"schedule\": {\n";
        js << "    \"bands\": 4,\n";
        js << "    \"closure_mode\": \"repeat_ref_tick\",\n";
        js << "    \"closure_last\": " << (flashlightClosureLast ? "true" : "false") << ",\n";
        js << "    \"closure_ref_t\": " << flashlightClosureRefT << "\n";
        js << "  },\n";
        js << "  \"trace\": [\n";

        int prevIters = 0;
        float prevMs = 0.0f;
        float prevSigned0 = 0.0f;
        int baseIters0 = 0;
        int peakIters = -1;
        int peakT = 0;
        float bestSaddleMinAbs = 1e30f;
        int bestSaddleT = 0;
        int bestSaddleNear = 0;

        int closureRefIters = 0;
        float closureRefSigned0 = 0.0f;
        bool closureRefSampled0 = false;
        int closureIters = 0;
        float closureSigned0 = 0.0f;
        bool closureSampled0 = false;

        for (int t = 0; t < flashlightTicks; t++) {
            const bool isClosureTick = flashlightClosureLast && (t == flashlightTicks - 1);
            const int walkT = isClosureTick ? flashlightClosureRefT : t;

            // Multi-scale locality: treat t as a walk across "bands".
            const int bands = 4;
            const FlashlightManifoldStep step = FlashlightManifoldAt(seed32, walkT, bands, baseLog2, flashlightRadius, flashlightZoomRadius, aspect);

            // Apply a local camera perturbation in world units using the same mapping as the UI viewport.
            view.log2_zoom = step.log2ZoomTick;
            const double dxWorld = step.dxWorld;
            const double dyWorld = step.dyWorld;

            view.center_hp_x = baseCx + dxWorld;
            view.center_hp_y = baseCy + dyWorld;
            SyncViewUiFromHp(view);

            // Treat time as tick index: drift the seed in a deterministic, bounded way.
            view.explaino_seed_drift = (float)walkT;
            view.explaino_phase = (float)walkT * 0.15f;
            UpdateExplainoPolynomial(view, params, &dirty);

            std::vector<uint32_t> rgba;
            rgba.resize((size_t)render.resolution.x * (size_t)render.resolution.y);
            std::vector<uint8_t> mask;
            mask.resize((size_t)render.resolution.x * (size_t)render.resolution.y);

            const char* err = nullptr;
            RenderStats stats{};
            if (!RenderFractalCUDA(view, params, render, rgba.data(), mask.data(), &stats, &err)) {
                ReportUiErrorOnce("render.cuda", schemaPath, "RenderFractalCUDA", err ? err : "Render failed");
                return 1;
            }

            const int ds = NormalizeLensDownsamplePow2(lens.downsample);
            std::vector<uint8_t> lensMaskLow;
            int lensW = 0;
            int lensH = 0;
            DownsampleMaskPow2(mask.data(), render.resolution.x, render.resolution.y, ds, lensMaskLow, lensW, lensH);
            const float maxAbsPxLow = 48.0f / (float)ds;

            // Sample SDF at a small fixed set of points.
            float signed0 = 0.0f;
            bool inside0 = false;
            bool sampled0 = false;

            float minAbsSigned = 1e30f;
            int nearCount = 0;
            int insideCount = 0;
            const float nearEpsPx = 2.0f;

            js << "    {\n";
            js << "      \"t\": " << t << ",\n";
            js << "      \"band\": " << step.band << ",\n";
            js << "      \"closure\": " << (isClosureTick ? "true" : "false") << ",\n";
            if (isClosureTick) {
                js << "      \"closure_ref_t\": " << walkT << ",\n";
            }
            js << "      \"camera\": {\n";
            js << "        \"center_hp_x\": " << view.center_hp_x << ",\n";
            js << "        \"center_hp_y\": " << view.center_hp_y << ",\n";
            js << "        \"log2_zoom\": " << view.log2_zoom << "\n";
            js << "      },\n";
            js << "      \"render\": {\n";
            js << "        \"last_render_ms\": " << (double)stats.last_render_ms << ",\n";
            js << "        \"last_iters_avg\": " << stats.last_iters_avg << ",\n";
            js << "        \"device_id\": " << stats.last_device_id << "\n";
            js << "      },\n";
            js << "      \"lens\": {\n";
            js << "        \"downsample\": " << ds << ",\n";
            js << "        \"lens_low_size\": [" << lensW << ", " << lensH << "],\n";
            js << "        \"sdf_max_abs_px_low\": " << (double)maxAbsPxLow << "\n";
            js << "      },\n";
            js << "      \"samples\": [\n";

            for (int pi = 0; pi < 5; pi++) {
                const float u = uvPts[pi].u;
                const float v = uvPts[pi].v;
                int x = (int)floorf(u * (float)lensW);
                int y = (int)floorf(v * (float)lensH);
                if (x < 0) x = 0;
                if (y < 0) y = 0;
                if (x >= lensW) x = lensW - 1;
                if (y >= lensH) y = lensH - 1;

                float signedPx = 0.0f;
                bool inside = false;
                bool ok = SampleSignedDistanceSdfChamfer(lensMaskLow.data(), lensW, lensH, x, y, signedPx, inside);
                if (pi == 0) {
                    signed0 = signedPx;
                    inside0 = inside;
                    sampled0 = ok;
                }
                if (ok) {
                    float a = fabsf(signedPx);
                    if (a < minAbsSigned) minAbsSigned = a;
                    if (a <= nearEpsPx) nearCount++;
                }
                if (inside) insideCount++;
                js << "        {\"u\": " << (double)u << ", \"v\": " << (double)v
                   << ", \"x\": " << x << ", \"y\": " << y
                   << ", \"signed_px\": " << (double)signedPx
                   << ", \"inside\": " << (inside ? "true" : "false")
                   << ", \"ok\": " << (ok ? "true" : "false") << "}";
                if (pi + 1 < 5) js << ",";
                js << "\n";
            }

            js << "      ],\n";

            // Saddle-ish diagnostics: boundary proximity and mixed inside/outside around the center.
            const bool mixedInside = (insideCount > 0 && insideCount < 5);
            js << "      \"saddle\": {\n";
            js << "        \"min_abs_signed_px\": " << (double)minAbsSigned << ",\n";
            js << "        \"near_eps_px\": " << (double)nearEpsPx << ",\n";
            js << "        \"near_count\": " << nearCount << ",\n";
            js << "        \"mixed_inside\": " << (mixedInside ? "true" : "false") << "\n";
            js << "      },\n";

            // Loss proxies / alignment metrics (cheap, deterministic): deltas in render cost and local SDF.
            const int dIters = (t == 0) ? 0 : (stats.last_iters_avg - prevIters);
            const float dMs = (t == 0) ? 0.0f : (stats.last_render_ms - prevMs);
            const float dSdf0 = (t == 0) ? 0.0f : (signed0 - prevSigned0);
            js << "      \"loss_proxy\": {\n";
            js << "        \"iters_avg\": " << stats.last_iters_avg << ",\n";
            js << "        \"render_ms\": " << (double)stats.last_render_ms << ",\n";
            js << "        \"sdf_center_signed_px\": " << (double)signed0 << ",\n";
            js << "        \"sdf_center_inside\": " << (inside0 ? "true" : "false") << "\n";
            js << "      },\n";
            js << "      \"delta\": {\n";
            js << "        \"d_iters_avg\": " << dIters << ",\n";
            js << "        \"d_render_ms\": " << (double)dMs << ",\n";
            js << "        \"d_sdf_center_signed_px\": " << (double)dSdf0 << "\n";
            js << "      }\n";
            js << "    }";
            if (t + 1 < flashlightTicks) js << ",";
            js << "\n";

            prevIters = stats.last_iters_avg;
            prevMs = stats.last_render_ms;
            if (sampled0) prevSigned0 = signed0;

            if (t == 0) baseIters0 = stats.last_iters_avg;
            if (stats.last_iters_avg > peakIters) { peakIters = stats.last_iters_avg; peakT = t; }
            if (minAbsSigned < bestSaddleMinAbs) { bestSaddleMinAbs = minAbsSigned; bestSaddleT = t; bestSaddleNear = nearCount; }

            if (!isClosureTick && t == flashlightClosureRefT) {
                closureRefIters = stats.last_iters_avg;
                closureRefSigned0 = signed0;
                closureRefSampled0 = sampled0;
            }
            if (isClosureTick) {
                closureIters = stats.last_iters_avg;
                closureSigned0 = signed0;
                closureSampled0 = sampled0;
            }

            // Capture the standard diagnostics bundle for the final tick as a proof surface (frame.bmp + state.json).
            if (t == flashlightTicks - 1) {
                if (!CaptureDiagnosticsLastBundle(schemaPath, view, params, render, lens, rgba.data(), render.resolution.x, render.resolution.y)) {
                    return 1;
                }

                // Also emit a full-resolution lens SDF visualization in screen space.
                // This is a direct way to inspect boundary pressure and alignment.
                // (Uses the per-pixel lens mask from the renderer.)
                if (!mask.empty()) {
                    if (!WriteLensSdfBmpLastBundle(schemaPath, mask.data(), render.resolution.x, render.resolution.y, 48.0f, "lens_sdf.bmp")) {
                        return 1;
                    }
                }
            }
        }

        // Summaries: overshoot/peak and best saddle candidate.
        const int itersThreshold = (baseIters0 > 0) ? (int)fmax(50.0, (double)baseIters0 + 20.0) : 50;
        // Keep it simple: report threshold and peak amplitude.
        const int peakAmplitude = (baseIters0 > 0) ? (peakIters - baseIters0) : peakIters;

        js << "  ],\n";
        const int closureDIters = closureIters - closureRefIters;
        const float closureDSigned0 = closureSigned0 - closureRefSigned0;
        const bool closureSignedComparable = closureRefSampled0 && closureSampled0;
        const bool closureOk = !flashlightClosureLast
            ? true
            : (closureDIters == 0 && (!closureSignedComparable || fabsf(closureDSigned0) <= 1e-6f));

        js << "  \"summary\": {\n";
        js << "    \"base_iters_avg_t0\": " << baseIters0 << ",\n";
        js << "    \"peak_iters_avg\": " << peakIters << ",\n";
        js << "    \"peak_t\": " << peakT << ",\n";
        js << "    \"peak_amplitude_over_t0\": " << peakAmplitude << ",\n";
        js << "    \"iters_threshold\": " << itersThreshold << ",\n";
        js << "    \"best_saddle_t\": " << bestSaddleT << ",\n";
        js << "    \"best_saddle_min_abs_signed_px\": " << (double)bestSaddleMinAbs << ",\n";
        js << "    \"best_saddle_near_count\": " << bestSaddleNear << ",\n";
        js << "    \"closure\": {\n";
        js << "      \"enabled\": " << (flashlightClosureLast ? "true" : "false") << ",\n";
        js << "      \"ref_t\": " << flashlightClosureRefT << ",\n";
        js << "      \"closure_t\": " << (flashlightTicks - 1) << ",\n";
        js << "      \"ok\": " << (closureOk ? "true" : "false") << ",\n";
        js << "      \"d_iters_avg\": " << closureDIters << ",\n";
        js << "      \"signed_comparable\": " << (closureSignedComparable ? "true" : "false") << ",\n";
        js << "      \"d_sdf_center_signed_px\": " << (double)closureDSigned0 << "\n";
        js << "    }\n";
        js << "  }\n";
        js << "}\n";

        if (!WriteFlashlightProbeLastBundle(schemaPath, js.str())) {
            return 1;
        }
        return 0;
    }

    WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, hInstance, nullptr, nullptr, nullptr, nullptr, _T("FractalUI"), nullptr};
    RegisterClassEx(&wc);

    HWND hwnd = CreateWindow(wc.lpszClassName, _T("CUDA Newton Fractal Explorer"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    LensSettings lens{};
    RenderStats stats{};
    bool dirty = true;

    // Schema load policy:
    // - Prefer exe-relative canonical schema (treat canonical JSON as an artifact)
    // - Default: strict fail-fast and write deterministic error artifact
    // - Optional: --ui-soft => fall back to safe-mode schema and keep app running
    std::string schemaPath;
    UISchema uiSchema;
    {
        std::string stage;
        std::string detail;
        if (!TryLoadSchemaFromCandidates(schemaCandidatesEarly, uiSchema, schemaPath, stage, detail)) {
            if (g_uiSoftErrors) {
                UiErrorOrFail(stage.c_str(), schemaPath, schemaPath, detail);
                uiSchema = BuildSafeModeSchema();
                schemaPath = "(safe-mode)";
            } else {
                FailFastUiError(stage.c_str(), schemaPath, schemaPath, detail);
                return 1;
            }
        }
    }

    BindingContext initBind;
    initBind.schema_path = schemaPath;
    initBind.view = &view;
    initBind.params = &params;
    initBind.render = &render;
    initBind.lens = &lens;
    {
        std::string bindError;
        if (!ValidateSchemaBindings(uiSchema, schemaPath, initBind, &bindError)) {
            if (g_uiSoftErrors) {
                UiErrorOrFail("schema.validate", schemaPath, schemaPath, bindError.empty() ? "Schema binding validation failed" : bindError);
                uiSchema = BuildSafeModeSchema();
                schemaPath = "(safe-mode)";
                initBind.schema_path = schemaPath;
                g_uiSchemaDisabled = true;
            }
        }
    }

    {
        ApplySchemaDefaults(uiSchema, initBind, &dirty);

        // Ensure polynomial coefficients remain coherent if schema sets poly_kind.
        if (params.poly_kind != PolyKind::custom) {
            SetPolyPreset(params);
        }

        // Ensure non-Newton starts with non-Newton-friendly params.
        ApplyFractalPresetDefaults(view, params, &dirty);

        // Explaino derives its polynomial from seed (+ optional phase).
        UpdateExplainoPolynomial(view, params, &dirty);

        // Initialize high-precision view from the finalized initial float surface.
        SyncViewHpFromUi(view);
    }

    std::vector<uint32_t> rgba;
    rgba.resize((size_t)render.resolution.x * (size_t)render.resolution.y);
    std::vector<uint8_t> lensMask;
    std::vector<uint8_t> lensMaskLow;
    std::vector<uint32_t> lensRgba;
    std::vector<uint32_t> maskRgba;
    int lensW = 0;
    int lensH = 0;

    struct FlashlightLiveState {
        bool requested = false;
        bool active = false;
        bool pending = false;
        bool wroteArtifacts = false;
        int t = 0;
        int ticks = 0;
        float radius = 0.75f;
        float zoomRadius = 0.25f;
        uint32_t seed32 = 0;
        uint32_t spectrum8[8] = {};
        std::string seedPath;
        double baseCx = 0.0;
        double baseCy = 0.0;
        double baseLog2 = 0.0;
        double aspect = 1.0;

        // Motion pacing.
        // Default: fixed overall orbit time (fast line is OK; resolution comes from tick count).
        bool paceFixedTickHz = false;
        float orbitSeconds = 40.0f; // used when paceFixedTickHz=false
        float tickRateHz = 2.0f;    // used when paceFixedTickHz=true

        // Visual trail.
        float trailFraction = 0.80f; // fraction of orbit visible

        // "Nth heads": marching ridge pattern (intrinsic; driven by SDF nearness).
        int ridgeN = 6;
        float ridgeSigmaTicks = 0.90f;
        float ridgeStrength = 0.90f;

        // Mold tendrils: tiny SDF-derived filaments toward / along manifold.
        bool moldEnabled = false;
        float moldStrength = 0.75f;
        int moldEvery = 6;
        int moldSteps = 10;
        float moldStepPxLow = 0.90f;
        float moldTangentMix = 0.55f;
        float moldPressureMin = 0.18f;

        // Playback time.
        bool liveTimeInit = false;
        double liveStartTime = 0.0;
        double pauseElapsed = 0.0;

        double nextTickTime = 0.0;
        int pendingT = 0;
        int prevIters = 0;
        float prevMs = 0.0f;
        float prevSigned0 = 0.0f;
        bool closureLast = false;
        int closureRefT = 0;

        int baseIters0 = 0;
        int peakIters = -1;
        int peakT = 0;
        float bestSaddleMinAbs = 1e30f;
        int bestSaddleT = 0;
        int bestSaddleNear = 0;

        int closureRefIters = 0;
        float closureRefSigned0 = 0.0f;
        bool closureRefSampled0 = false;
        int closureIters = 0;
        float closureSigned0 = 0.0f;
        bool closureSampled0 = false;
        bool orbitPrecomputed = false;
        int orbitTicks = 0;
        std::vector<std::string> tickJson;
        std::vector<ImVec2> path;
        std::vector<ImVec2> lensLowXY;
        std::vector<std::array<float, 5>> samplesSignedPx;
        std::vector<std::array<uint8_t, 5>> samplesInside;
        std::vector<int> walkT;
        bool showHud = true;
        bool showTrace = true;
        bool showComposite = true;
        // Trace visuals: math-driven only (no time-based reveal, glow, beads, or garnish).
        bool traceSpline = false;
        bool traceSplinters = false;
        float traceWidthMin = 2.25f;
        float traceWidthMax = 5.25f;
        float tracePressureDecayPx = 18.0f;
        float traceBoundsPullback = 1.15f;
        bool traceBoundsInit = false;
        float traceMinX = 0.0f;
        float traceMaxX = 0.0f;
        float traceMinY = 0.0f;
        float traceMaxY = 0.0f;

        // Smoothed trace window camera (keeps the line centered, reduces rescale jumps).
        bool traceViewInit = false;
        float traceViewCx = 0.0f;
        float traceViewCy = 0.0f;
        float traceViewHx = 1.0f;
        float traceViewHy = 1.0f;

        int traceSplineSteps = 12;
        float splinterScale = 1.75f;
        float splinterCurvature = 0.22f;
        float splinterMaxPx = 22.0f;

        // Composite view controls
        bool compositeCamPanZoom = true;
        float compositeCamLog2Zoom = 0.0f;
        ImVec2 compositeCamPanPx = ImVec2(0.0f, 0.0f);

        // Fade/dim the fractal underlay so motion reads.
        float compositeFrameAlpha = 0.92f;
        float compositeSdfAlpha = 0.10f;
        float compositeUnderDarken = 0.00f;
        bool compositeTrace = true;
        bool compositeVectors = false;
        float compositeVectorStrength = 1.0f;
        float compositeVectorLenFrac = 0.66f;

        // Composite meaning:
        // - Reach-only: don't globally emphasize structure where the manifold never goes.
        // - Electric: add an energetic sheath around the reachable manifold, driven by connectedness/mix.
        bool compositeReachOnly = true;
        bool compositeElectric = true;
        float compositeElectricStrength = 1.35f;

        // Camera framing for the orbit (steady view).
        bool orbitCamInit = false;
        double orbitCamCx = 0.0;
        double orbitCamCy = 0.0;
        double orbitCamLog2 = 0.0;
    } fl;

    struct FlashlightBridgeUiState {
        bool initialized = false;
        char seedPath[1024] = {};
        int ticks = 7;
        float warp = 0.0f;
        bool closureLast = true;
        bool noExport = false;
        uint32_t requestCounter = 0;
        uint64_t lastPrefillWriteTime = 0;
        uint32_t lastPrefillIdAutoEmitted = 0;
        bool pendingAutoEmit = false;
        std::string lastPrefillNote;
        std::string lastStatus;
    } bridge;
    PolyKind lastPolyKind = params.poly_kind;
    FractalType lastFractalType = view.fractal_type;
    int lastExplainoSeed = params.explaino_seed;
    float lastExplainoWarp = params.explaino_warp_strength;
    bool lastExplainoAlive = view.explaino_alive;

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));

    while (msg.message != WM_QUIT) {
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Controls window
        ImGui::Begin("Controls");

        if (!schemaPath.empty())
        {
            ImGui::TextDisabled("Schema: %s", schemaPath.c_str());
        }

        BindingContext bind;
        bind.schema_path = schemaPath;
        bind.view = &view;
        bind.params = &params;
        bind.render = &render;
        bind.lens = &lens;

        bool renderOnceAction = false;
        bool resetViewAction = false;
        bool resetAllAction = false;
        bool exportStateAction = false;
        bool importStateAction = false;
        bool captureDiagnosticAction = false;
        bool captureDiagnosticPending = false;
        bool nextSeedAction = false;
        bool prevSeedAction = false;

        // Live flashlight probe (interactive)
        if (flashlightLive) {
            fl.requested = true;

            if (!fl.active && fl.tickJson.empty()) {
                fl.active = true;
                fl.pending = false;
                fl.wroteArtifacts = false;
                fl.t = 0;
                fl.ticks = flashlightLiveTicks;
                fl.radius = flashlightLiveRadius;
                fl.zoomRadius = flashlightLiveZoomRadius;
                fl.seed32 = flashlightLiveSeed32;
                for (int i = 0; i < 8; i++) fl.spectrum8[i] = flashlightLiveSpectrum8[i];
                fl.seedPath = flashlightLivePath;
                fl.closureLast = flashlightLiveClosureLast;
                fl.closureRefT = flashlightLiveClosureRefT;
                fl.baseIters0 = 0;
                fl.peakIters = -1;
                fl.peakT = 0;
                fl.bestSaddleMinAbs = 1e30f;
                fl.bestSaddleT = 0;
                fl.bestSaddleNear = 0;
                fl.closureRefIters = 0;
                fl.closureRefSigned0 = 0.0f;
                fl.closureRefSampled0 = false;
                fl.closureIters = 0;
                fl.closureSigned0 = 0.0f;
                fl.closureSampled0 = false;
                fl.tickJson.clear();
                fl.path.clear();
                fl.lensLowXY.clear();
                fl.samplesSignedPx.clear();
                fl.samplesInside.clear();
                fl.walkT.clear();
                fl.prevIters = 0;
                fl.prevMs = 0.0f;
                fl.prevSigned0 = 0.0f;
                fl.nextTickTime = ImGui::GetTime();
                fl.liveTimeInit = false;
                fl.pauseElapsed = 0.0;
                fl.orbitCamInit = false;
                fl.orbitPrecomputed = false;
                fl.orbitTicks = 0;
                fl.traceViewInit = false;

                // Force Explain-o family + lens on.
                view.fractal_type = FractalType::explaino_fp;
                lens.enabled = true;

                // PROBE SAFETY: disable all camera automation.
                view.camera_behavior = CameraBehavior::manual;
                view.auto_dive = false;
                view.dive_speed = 0.0f;
                // Keep the background stable (render-once). Motion is in overlays.
                view.auto_refresh = false;

                // Ensure derived fields match the target.
                ApplyFractalPresetDefaults(view, params, &dirty);
                params.explaino_seed = (int)fl.seed32;
                // PROBE SAFETY: warp must be explicitly provided (external parameter). Default 0.
                params.explaino_warp_strength = flashlightLiveWarp;
                view.explaino_seed_tween = false;
                view.explaino_alive = false;
                view.explaino_phase = 0.0f;
                view.explaino_seed_drift = 0.0f;
                UpdateExplainoPolynomial(view, params, &dirty);
                SyncViewHpFromUi(view);

                // Base camera in HP space.
                fl.baseCx = view.center_hp_x;
                fl.baseCy = view.center_hp_y;
                fl.baseLog2 = view.log2_zoom;
                fl.aspect = (render.resolution.y > 0) ? (double)render.resolution.x / (double)render.resolution.y : 1.0;

                fl.orbitCamInit = false;

                dirty = true;
            }

            ImGui::Separator();
            ImGui::TextUnformatted("Flashlight Live (CUDA runtime probe)");
            ImGui::TextDisabled("seed: %s", fl.seedPath.empty() ? "(none)" : fl.seedPath.c_str());
            ImGui::Text("ticks: %d/%d", fl.t, fl.ticks);
            ImGui::Checkbox("pace_fixed_tick_hz", &fl.paceFixedTickHz);
            if (fl.paceFixedTickHz) {
                ImGui::SliderFloat("tick_rate_hz", &fl.tickRateHz, 0.05f, 10.0f, "%.2f");
                float orbitSec = (fl.tickRateHz > 0.0f) ? ((float)fmax(0, fl.ticks - (fl.closureLast ? 1 : 0)) / fl.tickRateHz) : 0.0f;
                ImGui::TextDisabled("orbit_seconds: %.1f", orbitSec);
            } else {
                ImGui::SliderFloat("orbit_seconds", &fl.orbitSeconds, 3.0f, 240.0f, "%.1f");
                if (fl.orbitSeconds < 1.0f) fl.orbitSeconds = 1.0f;
                float hz = (fl.orbitSeconds > 0.0f && fl.ticks > 0) ? ((float)fmax(0, fl.ticks - (fl.closureLast ? 1 : 0)) / fl.orbitSeconds) : 0.0f;
                ImGui::TextDisabled("hz: %.2f", hz);
            }
            ImGui::Checkbox("show_hud", &fl.showHud);
            ImGui::Checkbox("show_trace", &fl.showTrace);
            ImGui::Checkbox("show_composite", &fl.showComposite);
            ImGui::SeparatorText("Trace (math-driven)");
            ImGui::SliderFloat("trail_fraction", &fl.trailFraction, 0.05f, 0.95f, "%.2f");
            ImGui::Checkbox("trace_spline", &fl.traceSpline);
            ImGui::SeparatorText("Intrinsic head (marching ridges)");
            ImGui::SliderInt("ridge_n", &fl.ridgeN, 1, 16);
            ImGui::SliderFloat("ridge_sigma_ticks", &fl.ridgeSigmaTicks, 0.20f, 2.50f, "%.2f");
            ImGui::SliderFloat("ridge_strength", &fl.ridgeStrength, 0.0f, 3.0f, "%.2f");
            ImGui::SeparatorText("Mold tendrils (SDF-driven)");
            ImGui::Checkbox("mold_tendrils", &fl.moldEnabled);
            ImGui::SliderFloat("mold_strength", &fl.moldStrength, 0.0f, 3.0f, "%.2f");
            ImGui::SliderInt("mold_every", &fl.moldEvery, 1, 12);
            ImGui::SliderInt("mold_steps", &fl.moldSteps, 1, 40);
            ImGui::SliderFloat("mold_step_px_low", &fl.moldStepPxLow, 0.10f, 3.00f, "%.2f");
            ImGui::SliderFloat("mold_tangent_mix", &fl.moldTangentMix, 0.0f, 1.0f, "%.2f");
            ImGui::SliderFloat("mold_pressure_min", &fl.moldPressureMin, 0.0f, 1.0f, "%.2f");
            ImGui::SeparatorText("Style");
            ImGui::SliderFloat("width_min", &fl.traceWidthMin, 0.25f, 6.0f, "%.2f");
            ImGui::SliderFloat("width_max", &fl.traceWidthMax, 0.25f, 10.0f, "%.2f");
            ImGui::SliderFloat("pressure_decay_px", &fl.tracePressureDecayPx, 1.0f, 80.0f, "%.1f");
            ImGui::SliderFloat("bounds_pullback", &fl.traceBoundsPullback, 1.0f, 1.6f, "%.2f");
            ImGui::SliderInt("spline_steps", &fl.traceSplineSteps, 1, 32);
            ImGui::SliderFloat("splinter_scale", &fl.splinterScale, 0.05f, 6.0f, "%.2f");
            ImGui::SliderFloat("splinter_curve", &fl.splinterCurvature, 0.0f, 0.8f, "%.2f");
            ImGui::SliderFloat("splinter_max_px", &fl.splinterMaxPx, 4.0f, 120.0f, "%.1f");
            ImGui::SeparatorText("Composite (frame + SDF + vectors)");
            ImGui::Checkbox("cam_panzoom", &fl.compositeCamPanZoom);
            ImGui::SameLine();
            if (ImGui::Button("cam_reset")) {
                fl.compositeCamLog2Zoom = 0.0f;
                fl.compositeCamPanPx = ImVec2(0.0f, 0.0f);
            }
            ImGui::SliderFloat("frame_alpha", &fl.compositeFrameAlpha, 0.0f, 1.0f, "%.2f");
            ImGui::SliderFloat("sdf_alpha", &fl.compositeSdfAlpha, 0.0f, 1.0f, "%.2f");
            ImGui::SliderFloat("under_darken", &fl.compositeUnderDarken, 0.0f, 0.8f, "%.2f");
            ImGui::Checkbox("composite_trace", &fl.compositeTrace);
            ImGui::Checkbox("composite_vectors", &fl.compositeVectors);
            ImGui::SliderFloat("vector_strength", &fl.compositeVectorStrength, 0.0f, 3.0f, "%.2f");
            ImGui::SliderFloat("vector_len", &fl.compositeVectorLenFrac, 0.0f, 1.0f, "%.2f");
            ImGui::SeparatorText("Composite (meaning)");
            ImGui::Checkbox("reach_only", &fl.compositeReachOnly);
            ImGui::Checkbox("electric", &fl.compositeElectric);
            ImGui::SliderFloat("electric_strength", &fl.compositeElectricStrength, 0.0f, 3.0f, "%.2f");
            // head flare removed (visual garnish)
            if (ImGui::Button(fl.active ? "Stop" : "Start")) {
                const double now = ImGui::GetTime();
                if (fl.active) {
                    fl.pauseElapsed = fl.liveTimeInit ? (now - fl.liveStartTime) : 0.0;
                    fl.active = false;
                } else {
                    if (!fl.liveTimeInit) {
                        fl.liveStartTime = now;
                        fl.pauseElapsed = 0.0;
                        fl.liveTimeInit = true;
                    } else {
                        fl.liveStartTime = now - fl.pauseElapsed;
                    }
                    fl.active = true;
                }
                fl.pending = false;
                fl.nextTickTime = now;
            }
            ImGui::SameLine();
            if (ImGui::Button("Reset")) {
                fl.active = true;
                fl.pending = false;
                fl.wroteArtifacts = false;
                fl.t = 0;
                fl.tickJson.clear();
                fl.path.clear();
                fl.lensLowXY.clear();
                fl.samplesSignedPx.clear();
                fl.samplesInside.clear();
                fl.walkT.clear();
                fl.traceBoundsInit = false;
                fl.traceViewInit = false;
                fl.prevIters = 0;
                fl.prevMs = 0.0f;
                fl.prevSigned0 = 0.0f;
                fl.orbitPrecomputed = false;
                fl.orbitTicks = 0;
                fl.nextTickTime = ImGui::GetTime();
                fl.liveTimeInit = false;
                fl.pauseElapsed = 0.0;
                fl.orbitCamInit = false;

                view.center_hp_x = fl.baseCx;
                view.center_hp_y = fl.baseCy;
                view.log2_zoom = fl.baseLog2;
                SyncViewUiFromHp(view);
                dirty = true;
            }
        }

        // Workspace bridge (UI -> watcher)
        {
            if (!bridge.initialized) {
                const std::string def = GetDefaultSeedTextPath();
                snprintf(bridge.seedPath, sizeof(bridge.seedPath), "%s", def.c_str());
                bridge.initialized = true;
            }

            // Optional prefill hook for demo tooling.
            // If ui/diagnostics/last/flashlight_bridge_prefill.json exists, load it and apply defaults.
            // Supports: seed_path, ticks, warp, closure_last, no_export, prefill_id, auto_emit.
            {
                std::string baseDir = NormalizePath(GetDiagnosticsPrimaryDir());
                if (GetFileAttributesA(baseDir.c_str()) == INVALID_FILE_ATTRIBUTES) {
                    baseDir = NormalizePath(GetDiagnosticsSecondaryDir());
                }
                const std::string prefillPath = JoinPath(baseDir, "flashlight_bridge_prefill.json");
                const uint64_t wt = GetFileLastWriteTime64(prefillPath);
                if (wt != 0 && wt != bridge.lastPrefillWriteTime) {
                    bridge.lastPrefillWriteTime = wt;

                    std::string text;
                    if (ReadTextFile(prefillPath, text)) {
                        auto pr = json_min::Parse(text);
                        if (pr.error.empty() && pr.value.is_object()) {
                            if (const json_min::Value* v = pr.value.get("seed_path")) {
                                if (v->is_string()) {
                                    const std::string sp = v->as_string();
                                    if (!sp.empty()) {
                                        snprintf(bridge.seedPath, sizeof(bridge.seedPath), "%s", sp.c_str());
                                    }
                                }
                            }
                            if (const json_min::Value* v = pr.value.get("ticks")) {
                                if (v->is_number()) {
                                    int t = (int)std::round(v->as_number());
                                    if (t < 1) t = 1;
                                    if (t > kFlashlightTicksMax) t = kFlashlightTicksMax;
                                    bridge.ticks = t;
                                }
                            }
                            if (const json_min::Value* v = pr.value.get("warp")) {
                                if (v->is_number()) {
                                    float w = (float)v->as_number();
                                    if (w < 0.0f) w = 0.0f;
                                    if (w > 1.0f) w = 1.0f;
                                    bridge.warp = w;
                                }
                            }
                            if (const json_min::Value* v = pr.value.get("closure_last")) {
                                if (v->is_bool()) bridge.closureLast = v->as_bool();
                            }
                            if (const json_min::Value* v = pr.value.get("no_export")) {
                                if (v->is_bool()) bridge.noExport = v->as_bool();
                            }

                            uint32_t prefillId = 0;
                            if (const json_min::Value* v = pr.value.get("prefill_id")) {
                                if (v->is_number()) {
                                    double n = v->as_number();
                                    if (n > 0) prefillId = (uint32_t)std::llround(n);
                                }
                            }
                            bool autoEmit = false;
                            if (const json_min::Value* v = pr.value.get("auto_emit")) {
                                if (v->is_bool()) autoEmit = v->as_bool();
                            }
                            if (const json_min::Value* v = pr.value.get("note")) {
                                if (v->is_string()) bridge.lastPrefillNote = v->as_string();
                            } else {
                                bridge.lastPrefillNote.clear();
                            }

                            if (autoEmit && prefillId != 0 && prefillId != bridge.lastPrefillIdAutoEmitted) {
                                bridge.pendingAutoEmit = true;
                                bridge.lastPrefillIdAutoEmitted = prefillId;
                            }

                            // One-shot behavior: if the workspace is using this, treat it as consumed.
                            DeleteFileA(prefillPath.c_str());
                        } else {
                            bridge.lastPrefillNote = std::string("prefill_parse_error: ") + pr.error;
                        }
                    } else {
                        bridge.lastPrefillNote = "prefill_read_failed";
                    }
                }
            }

            ImGui::Separator();
            ImGui::TextUnformatted("Bridge: UI -> Workspace watcher");
            ImGui::TextDisabled("Writes ui/diagnostics/last/flashlight_bridge_request.json");
            ImGui::TextDisabled("Optional: reads ui/diagnostics/last/flashlight_bridge_prefill.json");
            ImGui::InputText("seed_path", bridge.seedPath, sizeof(bridge.seedPath));
            ImGui::SliderInt("ticks", &bridge.ticks, 1, kFlashlightTicksMax);
            ImGui::SliderFloat("warp", &bridge.warp, 0.0f, 1.0f, "%.3f");
            ImGui::Checkbox("closure_last", &bridge.closureLast);
            ImGui::Checkbox("no_export", &bridge.noExport);

            if (!bridge.lastPrefillNote.empty()) {
                ImGui::TextDisabled("prefill: %s", bridge.lastPrefillNote.c_str());
            }

            auto emitBridgeRequest = [&]() {
                bridge.requestCounter++;
                uint64_t nowMs = (uint64_t)GetTickCount64();
                std::ostringstream js;
                js.setf(std::ios::fixed);
                js << std::setprecision(8);
                js << "{\n";
                js << "  \"version\": 1,\n";
                js << "  \"kind\": \"flashlight_probe_headless\",\n";
                js << "  \"request_id\": " << (uint32_t)bridge.requestCounter << ",\n";
                js << "  \"ts_ms\": " << (uint64_t)nowMs << ",\n";
                js << "  \"client_pid\": " << (uint32_t)GetCurrentProcessId() << ",\n";
                js << "  \"seed_path\": \"" << JsonEscape(NormalizePath(std::string(bridge.seedPath))) << "\",\n";
                js << "  \"ticks\": " << (int)bridge.ticks << ",\n";
                js << "  \"warp\": " << (double)bridge.warp << ",\n";
                js << "  \"closure_last\": " << (bridge.closureLast ? "true" : "false") << ",\n";
                js << "  \"no_export\": " << (bridge.noExport ? "true" : "false") << "\n";
                js << "}\n";
                WriteFlashlightBridgeRequestLastBundle(schemaPath, js.str());
            };

            if (ImGui::Button("Emit request (headless probe)")) {
                emitBridgeRequest();
            }

            if (bridge.pendingAutoEmit) {
                // One-shot auto-run requested by a prefill file.
                bridge.pendingAutoEmit = false;
                emitBridgeRequest();
            }

            ImGui::SameLine();
            if (ImGui::Button("Refresh status")) {
                std::string baseDir = NormalizePath(GetDiagnosticsPrimaryDir());
                if (GetFileAttributesA(baseDir.c_str()) == INVALID_FILE_ATTRIBUTES) {
                    baseDir = NormalizePath(GetDiagnosticsSecondaryDir());
                }
                const std::string statusPath = JoinPath(baseDir, "flashlight_bridge_status.json");
                std::string text;
                if (ReadTextFile(statusPath, text)) {
                    bridge.lastStatus = text;
                } else {
                    bridge.lastStatus = std::string("(no status yet) expected at: ") + statusPath;
                }
            }

            if (!bridge.lastStatus.empty()) {
                ImGui::TextUnformatted("status:");
                ImGui::BeginChild("bridge_status", ImVec2(0, 90), true);
                ImGui::TextUnformatted(bridge.lastStatus.c_str());
                ImGui::EndChild();
            }
        }

        // If schema/UI edits the float surface (center/zoom), keep high-precision state in sync.
        Float2 uiCenterBefore = view.center;
        float uiZoomBefore = view.zoom;

        for (const auto& panel : uiSchema.panels) {
            if (ImGui::CollapsingHeader(panel.label.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                for (const auto& ctrl : panel.controls) {
                    // Hook extra actions without expanding the schema model yet.
                    if (ctrl.type == "button" && ctrl.has_binding && ctrl.binding.kind == "action") {
                        ImGui::PushID(ctrl.id.c_str());
                        bool pressed = ImGui::Button(ctrl.label.c_str());
                        ImGui::PopID();
                        if (pressed) {
                            if (ctrl.binding.path == "fractal.actions.render_once") renderOnceAction = true;
                            if (ctrl.binding.path == "fractal.actions.reset_view") resetViewAction = true;
                            if (ctrl.binding.path == "fractal.actions.reset_all") resetAllAction = true;
                            if (ctrl.binding.path == "fractal.actions.export_state") exportStateAction = true;
                            if (ctrl.binding.path == "fractal.actions.import_state") importStateAction = true;
                            if (ctrl.binding.path == "fractal.actions.capture_diagnostic") captureDiagnosticAction = true;
                            if (ctrl.binding.path == "fractal.actions.next_seed") nextSeedAction = true;
                            if (ctrl.binding.path == "fractal.actions.prev_seed") prevSeedAction = true;
                        }
                    } else {
                        RenderControlFromSchema(ctrl, bind, &dirty, &renderOnceAction);
                    }
                }
            }

            // Hotkey: F12 triggers a diagnostics capture.
            if (ImGui::IsKeyPressed(ImGuiKey_F12, false)) {
                captureDiagnosticAction = true;
            }
        }

        if (view.center.x != uiCenterBefore.x || view.center.y != uiCenterBefore.y || view.zoom != uiZoomBefore) {
            SyncViewHpFromUi(view);
        }

        auto isExplainoFamily = [](FractalType t) {
            return t == FractalType::explaino || t == FractalType::explaino_y || t == FractalType::explaino_fp;
        };

        if (view.fractal_type != FractalType::newton && view.fractal_type != FractalType::nova && !isExplainoFamily(view.fractal_type)) {
            ImGui::Spacing();
            ImGui::TextWrapped("Note: escape-time fractals use iteration-based coloring.");
            if (params.coloring_mode == ColoringMode::root_basin) {
                ImGui::TextWrapped("Coloring Mode 'root_basin' is Newton/Nova-only. Choose 'iteration_count' or 'smooth_escape'.");
            }
        }

        // Apply per-fractal presets immediately when the user changes fractal type.
        if (view.fractal_type != lastFractalType) {
            lastFractalType = view.fractal_type;
            ApplyFractalPresetDefaults(view, params, &dirty);
            UpdateExplainoPolynomial(view, params, &dirty);
            SyncViewHpFromUi(view);
        }

        // If Explaino seed/strength changes via UI, regenerate polynomial.
        if (isExplainoFamily(view.fractal_type)) {
            if (params.explaino_seed != lastExplainoSeed || params.explaino_warp_strength != lastExplainoWarp) {
                lastExplainoSeed = params.explaino_seed;
                lastExplainoWarp = params.explaino_warp_strength;
                UpdateExplainoPolynomial(view, params, &dirty);
            }
        }

        if (isExplainoFamily(view.fractal_type) && (nextSeedAction || prevSeedAction)) {
            int delta = nextSeedAction ? 1 : -1;
            params.explaino_seed += delta;
            dirty = true;
        }

        // Keep presets coherent when schema-driven poly changes happen.
        if (params.poly_kind != lastPolyKind) {
            lastPolyKind = params.poly_kind;
            if (isExplainoFamily(view.fractal_type)) {
                // Explaino owns its polynomial; keep it custom.
                UpdateExplainoPolynomial(view, params, &dirty);
                lastPolyKind = params.poly_kind;
            } else if (params.poly_kind != PolyKind::custom) {
                SetPolyPreset(params);
                dirty = true;
            }
        }

        ImGui::Separator();
        ImGui::Text("Last render: %.3f ms (benchmark), avg iters ~ %d, device %d", stats.last_render_ms, stats.last_iters_avg, stats.last_device_id);

        {
            const double zhp = SafeZoomFromLog2(view.log2_zoom);
            ImGui::Text("HP zoom: 2^(%.3f) ~= %.3e", view.log2_zoom, zhp);
        }

        ImGui::End();

        if (resetViewAction) {
            view.center = {0.0f, 0.0f};
            view.zoom = 1.0f;
            view.rotation_degrees = 0.0f;
            SyncViewHpFromUi(view);
            dirty = true;
        }

        if (resetAllAction) {
            // View defaults
            view.center = {0.0f, 0.0f};
            view.zoom = 1.0f;
            view.rotation_degrees = 0.0f;
            view.auto_refresh = true;
            view.camera_behavior = CameraBehavior::manual;
            view.auto_dive = false;
            view.dive_speed = 0.0f;
            view.explaino_alive = false;
            view.explaino_seed_tween = true;
            view.explaino_alive_speed = 1.0f;
            view.explaino_phase = 0.0f;
            view.explaino_seed_drift = 0.0f;

            // Kernel defaults (per current fractal type)
            ApplyFractalPresetDefaults(view, params, &dirty);
            UpdateExplainoPolynomial(view, params, &dirty);
            SyncViewHpFromUi(view);
            lastPolyKind = params.poly_kind;
            lastFractalType = view.fractal_type;
            lastExplainoSeed = params.explaino_seed;
            lastExplainoWarp = params.explaino_warp_strength;
            lastExplainoAlive = view.explaino_alive;

            // Render defaults
            render.resolution = {1024, 768};
            render.block_size = 256;
            render.device_id = 0;
            render.benchmark = false;

            dirty = true;
        }

        if (exportStateAction) {
            ExportStateToFile(schemaPath, view, params, render, lens);
        }

        if (importStateAction) {
            ImportStateFromFile(schemaPath, view, params, render, lens, &dirty);
            // Prevent auto preset application from clobbering imported values.
            lastFractalType = view.fractal_type;
            lastPolyKind = params.poly_kind;
            lastExplainoSeed = params.explaino_seed;
            lastExplainoWarp = params.explaino_warp_strength;
            lastExplainoAlive = view.explaino_alive;
            // Ensure Explaino polynomial is coherent after import.
            UpdateExplainoPolynomial(view, params, &dirty);
        }

        if (captureDiagnosticAction) {
            // Capture a matching (frame + state) bundle. Prefer capturing after a render.
            captureDiagnosticPending = true;
            if (!view.auto_refresh && !dirty && !renderOnceAction) {
                renderOnceAction = true;
            }
        }

        // Alive toggle edges: avoid snap-back by committing drift when disabling.
        if (isExplainoFamily(view.fractal_type) && (view.explaino_alive != lastExplainoAlive)) {
            if (view.explaino_alive) {
                // Start fresh drift at the current seed.
                view.explaino_seed_drift = 0.0f;
            } else {
                // Commit drift into the visible seed so the image doesn't jump.
                int off = (int)floorf(view.explaino_seed_drift);
                if (off != 0) {
                    params.explaino_seed += off;
                    lastExplainoSeed = params.explaino_seed;
                }
                view.explaino_seed_drift = 0.0f;
                UpdateExplainoPolynomial(view, params, &dirty);
                dirty = true;
            }
            lastExplainoAlive = view.explaino_alive;
        }

        // Render dispatch (fail-fast, delta-independent):
        // - If auto_refresh is ON, we render every frame.
        // - Otherwise, render on explicit request or any state change (dirty).
        // Flashlight live: steady camera framing + moving point over a static SDF field.
        // Camera is fitted once so the entire orbit bounds remain in-frame.
        // We precompute all orbit samples once, then render-time tweening drives the motion.
        if (flashlightLive && fl.requested && fl.ticks > 0) {
            const double now = ImGui::GetTime();
            if (!fl.liveTimeInit) {
                fl.liveTimeInit = true;
                fl.liveStartTime = now;
                fl.pauseElapsed = 0.0;
            }

            // Require a lens buffer before precomputing (needs one render to produce lensMaskLow).
            if (!lens.enabled || lensMaskLow.empty() || lensW <= 0 || lensH <= 0 || render.resolution.x <= 0 || render.resolution.y <= 0) {
                if (!dirty && !renderOnceAction) {
                    dirty = true;
                }
            } else {
                const int orbitTicks = fl.closureLast ? ((fl.ticks > 1) ? (fl.ticks - 1) : 1) : fl.ticks;

                // Orbit in world space around the base camera center.
                const double zoom0 = SafeZoomFromLog2(fl.baseLog2);
                const double base0 = 2.0 / fmax(1e-30, zoom0);
                const double radiusWorldX = (double)fl.radius * base0 * fl.aspect;
                const double radiusWorldY = (double)fl.radius * base0;

                // Fit a steady camera to the full orbit bounds (once). After changing the view,
                // we need one render to refresh lensMaskLow for that camera.
                if (!fl.orbitCamInit) {
                    const double pad = 1.18;
                    const double hx = radiusWorldX * pad;
                    const double hy = radiusWorldY * pad;
                    const double baseNeed = fmax(hy, hx / fmax(1e-12, fl.aspect));
                    const double zoomNeed = 2.0 / fmax(1e-30, baseNeed);
                    const double log2Need = ClampD(log2(zoomNeed) / log(2.0), Log2D(kMinZoom), kMaxLog2Zoom);
                    fl.orbitCamInit = true;
                    fl.orbitCamCx = fl.baseCx;
                    fl.orbitCamCy = fl.baseCy;
                    fl.orbitCamLog2 = log2Need;

                    view.center_hp_x = fl.orbitCamCx;
                    view.center_hp_y = fl.orbitCamCy;
                    view.log2_zoom = fl.orbitCamLog2;
                    SyncViewUiFromHp(view);
                    dirty = true;
                } else {
                    const bool needRebuild = !fl.orbitPrecomputed || fl.orbitTicks != orbitTicks;
                    if (needRebuild) {
                        fl.orbitPrecomputed = false;
                        fl.orbitTicks = orbitTicks;
                        fl.t = 0;
                        fl.tickJson.clear();
                        fl.path.clear();
                        fl.lensLowXY.clear();
                        fl.samplesSignedPx.clear();
                        fl.samplesInside.clear();
                        fl.walkT.clear();
                    }

                    if (!fl.orbitPrecomputed && orbitTicks > 0) {
                        const int ds = NormalizeLensDownsamplePow2(lens.downsample);
                        const int sampleOff = (ds > 0) ? (int)fmax(1.0, round(8.0 / (double)ds)) : 8;
                        const float nearEpsPx = 2.0f;

                        auto worldToLensLow = [&](ImVec2 w, int& outX, int& outY) {
                            double aspect = (render.resolution.y > 0) ? (double)render.resolution.x / (double)render.resolution.y : 1.0;
                            double zoomNow = SafeZoomFromLog2(view.log2_zoom);
                            double base = 2.0 / fmax(1e-30, zoomNow);
                            double nx = ((double)w.x - view.center_hp_x) / (base * aspect);
                            double ny = ((double)w.y - view.center_hp_y) / (base);
                            double px = (nx / 2.0 + 0.5) * (double)render.resolution.x;
                            double py = (ny / 2.0 + 0.5) * (double)render.resolution.y;
                            int x = (ds > 0) ? (int)floor(px / (double)ds) : (int)floor(px);
                            int y = (ds > 0) ? (int)floor(py / (double)ds) : (int)floor(py);
                            if (x < 0) x = 0;
                            if (y < 0) y = 0;
                            if (x >= lensW) x = lensW - 1;
                            if (y >= lensH) y = lensH - 1;
                            outX = x;
                            outY = y;
                        };


                        fl.tickJson.reserve((size_t)orbitTicks);
                        fl.path.reserve((size_t)orbitTicks);
                        fl.lensLowXY.reserve((size_t)orbitTicks);
                        fl.samplesSignedPx.reserve((size_t)orbitTicks);
                        fl.samplesInside.reserve((size_t)orbitTicks);
                        fl.walkT.reserve((size_t)orbitTicks);

                        // Flashlight manifold sampling (LIVE invariant): derive the trace from the Explaino/flashlight
                        // schedule itself (seed32 + tick -> banded rx/ry/rz). This MUST match the probe sampler.
                        // The camera stays steady for viewing; we move the sample point through world space.
                        const int bands = 4;
                        const double aspect = (double)fl.aspect;
                        const double baseLog2 = fl.baseLog2;

                        for (int t = 0; t < orbitTicks; t++) {
                            const int walkT = t;
                            const FlashlightManifoldStep step = FlashlightManifoldAt(fl.seed32, walkT, bands, baseLog2, fl.radius, fl.zoomRadius, aspect);
                            const ImVec2 worldP = ImVec2((float)(fl.baseCx + step.dxWorld), (float)(fl.baseCy + step.dyWorld));
                            int cx = 0, cy = 0;
                            worldToLensLow(worldP, cx, cy);

                            struct P { int dx; int dy; } offs[5] = {
                                {0, 0},
                                {-sampleOff, -sampleOff},
                                {+sampleOff, -sampleOff},
                                {-sampleOff, +sampleOff},
                                {+sampleOff, +sampleOff},
                            };

                            std::array<float, 5> signedPxArr{};
                            std::array<uint8_t, 5> insideArr{};
                            float minAbsSigned = 1e30f;
                            int nearCount = 0;
                            int insideCount = 0;

                            for (int pi = 0; pi < 5; pi++) {
                                int x = cx + offs[pi].dx;
                                int y = cy + offs[pi].dy;
                                if (x < 0) x = 0;
                                if (y < 0) y = 0;
                                if (x >= lensW) x = lensW - 1;
                                if (y >= lensH) y = lensH - 1;

                                float signedPx = 0.0f;
                                bool inside = false;
                                bool ok = SampleSignedDistanceSdfChamfer(lensMaskLow.data(), lensW, lensH, x, y, signedPx, inside);
                                signedPxArr[(size_t)pi] = signedPx;
                                insideArr[(size_t)pi] = inside ? 1 : 0;
                                if (ok) {
                                    float a = fabsf(signedPx);
                                    if (a < minAbsSigned) minAbsSigned = a;
                                    if (a <= nearEpsPx) nearCount++;
                                }
                                if (inside) insideCount++;
                            }

                            std::ostringstream tick;
                            tick.setf(std::ios::fixed);
                            tick << std::setprecision(8);
                            tick << "    {\n";
                            tick << "      \"t\": " << t << ",\n";
                            tick << "      \"world\": {\"x\": " << (double)worldP.x << ", \"y\": " << (double)worldP.y << "},\n";
                            tick << "      \"lens\": {\"low_xy\": [" << cx << ", " << cy << "], \"downsample\": " << ds << "},\n";
                            tick << "      \"saddle\": {\"min_abs_signed_px\": " << (double)minAbsSigned << ", \"near_count\": " << nearCount << ", \"mixed_inside\": " << ((insideCount > 0 && insideCount < 5) ? "true" : "false") << "}\n";
                            tick << "    }";

                            fl.tickJson.push_back(tick.str());
                            fl.path.push_back(worldP);
                            fl.lensLowXY.push_back(ImVec2((float)cx, (float)cy));
                            fl.samplesSignedPx.push_back(signedPxArr);
                            fl.samplesInside.push_back(insideArr);
                            fl.walkT.push_back(walkT);
                        }

                        fl.t = fl.ticks;
                        fl.orbitPrecomputed = true;
                    }
                }
            }
        }

        if (view.auto_refresh || dirty || renderOnceAction) {
            rgba.resize((size_t)render.resolution.x * (size_t)render.resolution.y);
            EnsureFractalTexture(render.resolution.x, render.resolution.y);

            uint8_t* outMask = nullptr;
            if (lens.enabled) {
                lensMask.resize((size_t)render.resolution.x * (size_t)render.resolution.y);
                outMask = lensMask.data();
            }

            // Derived encoding: for Alive seed drift, keep UI seed stable while the renderer sees an effective seed.
            KernelParams paramsForRender = params;
            if (isExplainoFamily(view.fractal_type) && view.explaino_alive) {
                int off = (int)floorf(view.explaino_seed_drift);
                paramsForRender.explaino_seed = params.explaino_seed + off;
            }

            const char* err = nullptr;
            RenderStats newStats{};
            if (!RenderFractalCUDA(view, paramsForRender, render, rgba.data(), outMask, &newStats, &err)) {
                // Show error pane
                ImGui::Begin("CUDA Error");
                ImGui::TextWrapped("Render failed: %s", err ? err : "unknown error");
                ImGui::End();
            } else {
                stats = newStats;
                UploadFractalRGBA(rgba.data(), render.resolution.x, render.resolution.y);

                if (lens.enabled && !lensMask.empty()) {
                    const int ds = NormalizeLensDownsamplePow2(lens.downsample);
                    DownsampleMaskPow2(lensMask.data(), render.resolution.x, render.resolution.y, ds, lensMaskLow, lensW, lensH);
                    // Keep the SDF visual radius roughly stable in *source* pixels.
                    const float maxAbsPxLow = 48.0f / (float)ds;
                    ComputeSignedDistanceSdfChamfer(lensMaskLow.data(), lensW, lensH, maxAbsPxLow, lensRgba);
                    EnsureLensTexture(lensW, lensH);
                    UploadLensRGBA(lensRgba.data(), lensW, lensH);

                    // Mask lens: raw interior mask at the same downsampled resolution.
                    maskRgba.resize((size_t)lensW * (size_t)lensH);
                    for (int i = 0; i < lensW * lensH; i++) {
                        uint8_t v = lensMaskLow[(size_t)i];
                        maskRgba[(size_t)i] = 0xff000000u | ((uint32_t)v << 16) | ((uint32_t)v << 8) | (uint32_t)v;
                    }
                    EnsureMaskTexture(lensW, lensH);
                    UploadMaskRGBA(maskRgba.data(), lensW, lensH);
                }

                // Flashlight live record (after a successful render).
                if (flashlightLive && fl.requested && fl.pending) {
                    const int t = fl.pendingT;
                    const int bands = 4;
                    const bool isClosureTick = fl.closureLast && (t == fl.ticks - 1);
                    const int walkT = isClosureTick ? fl.closureRefT : t;
                    const int band = walkT % bands;

                    // Sample SDF at fixed UV points on the downsampled lens buffer.
                    struct ProbeUV { float u; float v; };
                    const ProbeUV uvPts[5] = {
                        {0.5f, 0.5f},
                        {0.25f, 0.25f},
                        {0.75f, 0.25f},
                        {0.25f, 0.75f},
                        {0.75f, 0.75f},
                    };

                    float signed0 = 0.0f;
                    bool inside0 = false;
                    bool sampled0 = false;

                    std::array<float, 5> signedPxArr{};
                    std::array<uint8_t, 5> insideArr{};

                    float minAbsSigned = 1e30f;
                    int nearCount = 0;
                    int insideCount = 0;
                    const float nearEpsPx = 2.0f;

                    std::ostringstream tick;
                    tick.setf(std::ios::fixed);
                    tick << std::setprecision(8);
                    tick << "    {\n";
                    tick << "      \"t\": " << t << ",\n";
                    tick << "      \"band\": " << band << ",\n";
                    tick << "      \"closure\": " << (isClosureTick ? "true" : "false") << ",\n";
                    if (isClosureTick) {
                        tick << "      \"closure_ref_t\": " << walkT << ",\n";
                    }
                    tick << "      \"camera\": {\n";
                    tick << "        \"center_hp_x\": " << view.center_hp_x << ",\n";
                    tick << "        \"center_hp_y\": " << view.center_hp_y << ",\n";
                    tick << "        \"log2_zoom\": " << view.log2_zoom << "\n";
                    tick << "      },\n";
                    tick << "      \"render\": {\n";
                    tick << "        \"last_render_ms\": " << (double)stats.last_render_ms << ",\n";
                    tick << "        \"last_iters_avg\": " << stats.last_iters_avg << ",\n";
                    tick << "        \"device_id\": " << stats.last_device_id << "\n";
                    tick << "      },\n";

                    const int ds = NormalizeLensDownsamplePow2(lens.downsample);
                    const float maxAbsPxLow = (ds > 0) ? (48.0f / (float)ds) : 48.0f;
                    tick << "      \"lens\": {\n";
                    tick << "        \"downsample\": " << ds << ",\n";
                    tick << "        \"lens_low_size\": [" << lensW << ", " << lensH << "],\n";
                    tick << "        \"sdf_max_abs_px_low\": " << (double)maxAbsPxLow << "\n";
                    tick << "      },\n";

                    tick << "      \"samples\": [\n";
                    for (int pi = 0; pi < 5; pi++) {
                        const float u = uvPts[pi].u;
                        const float v = uvPts[pi].v;
                        int x = (int)floorf(u * (float)lensW);
                        int y = (int)floorf(v * (float)lensH);
                        if (x < 0) x = 0;
                        if (y < 0) y = 0;
                        if (x >= lensW) x = lensW - 1;
                        if (y >= lensH) y = lensH - 1;

                        float signedPx = 0.0f;
                        bool inside = false;
                        bool ok = false;
                        if (!lensMaskLow.empty() && lensW > 0 && lensH > 0) {
                            ok = SampleSignedDistanceSdfChamfer(lensMaskLow.data(), lensW, lensH, x, y, signedPx, inside);
                        }
                        if (pi == 0) {
                            signed0 = signedPx;
                            inside0 = inside;
                            sampled0 = ok;
                        }
                        signedPxArr[(size_t)pi] = signedPx;
                        insideArr[(size_t)pi] = inside ? 1 : 0;
                        if (ok) {
                            float a = fabsf(signedPx);
                            if (a < minAbsSigned) minAbsSigned = a;
                            if (a <= nearEpsPx) nearCount++;
                        }
                        if (inside) insideCount++;

                        tick << "        {\"u\": " << (double)u << ", \"v\": " << (double)v
                             << ", \"x\": " << x << ", \"y\": " << y
                             << ", \"signed_px\": " << (double)signedPx
                             << ", \"inside\": " << (inside ? "true" : "false")
                             << ", \"ok\": " << (ok ? "true" : "false") << "}";
                        if (pi + 1 < 5) tick << ",";
                        tick << "\n";
                    }
                    tick << "      ],\n";

                    const bool mixedInside = (insideCount > 0 && insideCount < 5);
                    tick << "      \"saddle\": {\n";
                    tick << "        \"min_abs_signed_px\": " << (double)minAbsSigned << ",\n";
                    tick << "        \"near_eps_px\": " << (double)nearEpsPx << ",\n";
                    tick << "        \"near_count\": " << nearCount << ",\n";
                    tick << "        \"mixed_inside\": " << (mixedInside ? "true" : "false") << "\n";
                    tick << "      },\n";

                    const int dIters = (t == 0) ? 0 : (stats.last_iters_avg - fl.prevIters);
                    const float dMs = (t == 0) ? 0.0f : (stats.last_render_ms - fl.prevMs);
                    const float dSdf0 = (t == 0) ? 0.0f : (signed0 - fl.prevSigned0);
                    tick << "      \"loss_proxy\": {\n";
                    tick << "        \"iters_avg\": " << stats.last_iters_avg << ",\n";
                    tick << "        \"render_ms\": " << (double)stats.last_render_ms << ",\n";
                    tick << "        \"sdf_center_signed_px\": " << (double)signed0 << ",\n";
                    tick << "        \"sdf_center_inside\": " << (inside0 ? "true" : "false") << "\n";
                    tick << "      },\n";
                    tick << "      \"delta\": {\n";
                    tick << "        \"d_iters_avg\": " << dIters << ",\n";
                    tick << "        \"d_render_ms\": " << (double)dMs << ",\n";
                    tick << "        \"d_sdf_center_signed_px\": " << (double)dSdf0 << "\n";
                    tick << "      }\n";
                    tick << "    }";

                    fl.tickJson.push_back(tick.str());
                    fl.path.push_back(ImVec2((float)view.center_hp_x, (float)view.center_hp_y));
                    fl.samplesSignedPx.push_back(signedPxArr);
                    fl.samplesInside.push_back(insideArr);
                    fl.walkT.push_back(walkT);



                    fl.prevIters = stats.last_iters_avg;
                    fl.prevMs = stats.last_render_ms;
                    if (sampled0) fl.prevSigned0 = signed0;

                    if (t == 0) fl.baseIters0 = stats.last_iters_avg;
                    if (stats.last_iters_avg > fl.peakIters) { fl.peakIters = stats.last_iters_avg; fl.peakT = t; }
                    if (minAbsSigned < fl.bestSaddleMinAbs) { fl.bestSaddleMinAbs = minAbsSigned; fl.bestSaddleT = t; fl.bestSaddleNear = nearCount; }

                    if (!isClosureTick && t == fl.closureRefT) {
                        fl.closureRefIters = stats.last_iters_avg;
                        fl.closureRefSigned0 = signed0;
                        fl.closureRefSampled0 = sampled0;
                    }
                    if (isClosureTick) {
                        fl.closureIters = stats.last_iters_avg;
                        fl.closureSigned0 = signed0;
                        fl.closureSampled0 = sampled0;
                    }

                    // Final tick: capture proof bundle and write the JSON artifact.
                    if (t == fl.ticks - 1 && !fl.wroteArtifacts) {
                        CaptureDiagnosticsLastBundle(schemaPath, view, params, render, lens, rgba.data(), render.resolution.x, render.resolution.y);

                        std::ostringstream js;
                        js.setf(std::ios::fixed);
                        js << std::setprecision(8);
                        js << "{\n";
                        js << "  \"version\": 1,\n";
                        js << "  \"schema_path\": \"" << JsonEscape(schemaPath) << "\",\n";
                        js << "  \"seed_path\": \"" << JsonEscape(fl.seedPath) << "\",\n";
                        js << "  \"conversation_seed32\": " << (uint32_t)fl.seed32 << ",\n";
                        js << "  \"spectrum8_u32\": [";
                        for (int i = 0; i < 8; i++) { if (i) js << ", "; js << (uint32_t)fl.spectrum8[i]; }
                        js << "],\n";
                        js << "  \"ticks\": " << fl.ticks << ",\n";
                        js << "  \"radius\": " << (double)fl.radius << ",\n";
                        js << "  \"zoom_radius\": " << (double)fl.zoomRadius << ",\n";
                        js << "  \"probe_params\": {\n";
                        js << "    \"explaino_warp_strength\": " << (double)params.explaino_warp_strength << "\n";
                        js << "  },\n";
                        js << "  \"probe_safety\": {\n";
                        js << "    \"applied_schema_defaults\": false,\n";
                        js << "    \"camera_behavior\": \"manual\",\n";
                        js << "    \"auto_dive\": false,\n";
                        js << "    \"dive_speed\": 0.0\n";
                        js << "  },\n";
                        js << "  \"base_camera\": {\n";
                        js << "    \"center_hp_x\": " << fl.baseCx << ",\n";
                        js << "    \"center_hp_y\": " << fl.baseCy << ",\n";
                        js << "    \"log2_zoom\": " << fl.baseLog2 << "\n";
                        js << "  },\n";
                        js << "  \"schedule\": {\n";
                        js << "    \"bands\": 4,\n";
                        js << "    \"closure_mode\": \"repeat_ref_tick\",\n";
                        js << "    \"closure_last\": " << (fl.closureLast ? "true" : "false") << ",\n";
                        js << "    \"closure_ref_t\": " << fl.closureRefT << "\n";
                        js << "  },\n";
                        js << "  \"trace\": [\n";
                        for (size_t i = 0; i < fl.tickJson.size(); i++) {
                            js << fl.tickJson[i];
                            if (i + 1 < fl.tickJson.size()) js << ",";
                            js << "\n";
                        }
                        js << "  ],\n";

                        const int itersThreshold = (fl.baseIters0 > 0) ? (int)fmax(50.0, (double)fl.baseIters0 + 20.0) : 50;
                        const int peakAmplitude = (fl.baseIters0 > 0) ? (fl.peakIters - fl.baseIters0) : fl.peakIters;

                        const int closureDIters = fl.closureIters - fl.closureRefIters;
                        const float closureDSigned0 = fl.closureSigned0 - fl.closureRefSigned0;
                        const bool closureSignedComparable = fl.closureRefSampled0 && fl.closureSampled0;
                        const bool closureOk = !fl.closureLast
                            ? true
                            : (closureDIters == 0 && (!closureSignedComparable || fabsf(closureDSigned0) <= 1e-6f));

                        js << "  \"summary\": {\n";
                        js << "    \"base_iters_avg_t0\": " << fl.baseIters0 << ",\n";
                        js << "    \"peak_iters_avg\": " << fl.peakIters << ",\n";
                        js << "    \"peak_t\": " << fl.peakT << ",\n";
                        js << "    \"peak_amplitude_over_t0\": " << peakAmplitude << ",\n";
                        js << "    \"iters_threshold\": " << itersThreshold << ",\n";
                        js << "    \"best_saddle_t\": " << fl.bestSaddleT << ",\n";
                        js << "    \"best_saddle_min_abs_signed_px\": " << (double)fl.bestSaddleMinAbs << ",\n";
                        js << "    \"best_saddle_near_count\": " << fl.bestSaddleNear << ",\n";
                        js << "    \"closure\": {\n";
                        js << "      \"enabled\": " << (fl.closureLast ? "true" : "false") << ",\n";
                        js << "      \"ref_t\": " << fl.closureRefT << ",\n";
                        js << "      \"closure_t\": " << (fl.ticks - 1) << ",\n";
                        js << "      \"ok\": " << (closureOk ? "true" : "false") << ",\n";
                        js << "      \"d_iters_avg\": " << closureDIters << ",\n";
                        js << "      \"signed_comparable\": " << (closureSignedComparable ? "true" : "false") << ",\n";
                        js << "      \"d_sdf_center_signed_px\": " << (double)closureDSigned0 << "\n";
                        js << "    }\n";
                        js << "  }\n";
                        js << "}\n";
                        WriteFlashlightProbeLastBundle(schemaPath, js.str());
                        fl.wroteArtifacts = true;
                    }

                    fl.pending = false;
                    fl.t = t + 1;
                }

                if (captureDiagnosticPending) {
                    CaptureDiagnosticsLastBundle(schemaPath, view, params, render, lens, rgba.data(), render.resolution.x, render.resolution.y);
                    captureDiagnosticPending = false;
                }
            }
            dirty = false;
        }

        // If a capture was requested but no render happened (or render failed), capture the last available frame.
        if (captureDiagnosticPending) {
            if (!rgba.empty()) {
                CaptureDiagnosticsLastBundle(schemaPath, view, params, render, lens, rgba.data(), render.resolution.x, render.resolution.y);
            }
            captureDiagnosticPending = false;
        }

        // Image window
        ImGui::Begin("Fractal");
        if (g_fractalSRV) {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            float scale = 1.0f;
            if (render.resolution.x > 0 && render.resolution.y > 0) {
                float sx = avail.x / (float)render.resolution.x;
                float sy = avail.y / (float)render.resolution.y;
                scale = (sx < sy) ? sx : sy;
                if (scale <= 0.0f) scale = 1.0f;
            }
            ImVec2 size((float)render.resolution.x * scale, (float)render.resolution.y * scale);

            // Use an InvisibleButton for reliable mouse capture (prevents window dragging while panning).
            ImGui::InvisibleButton("##viewport", size, ImGuiButtonFlags_MouseButtonLeft);
            ImVec2 rectMin = ImGui::GetItemRectMin();
            ImVec2 rectMax = ImGui::GetItemRectMax();
            ImGui::GetWindowDrawList()->AddImage((ImTextureID)g_fractalSRV, rectMin, rectMax);

            // Flashlight HUD + sample point overlay.
            if (flashlightLive && fl.requested && fl.showHud) {
                ImDrawList* dl = ImGui::GetWindowDrawList();
                ImVec2 p = ImVec2(rectMin.x + 6.0f, rectMin.y + 6.0f);
                dl->AddRectFilled(p, ImVec2(p.x + 340.0f, p.y + 60.0f), IM_COL32(0, 0, 0, 160), 4.0f);
                char buf[256] = {};
                snprintf(buf, sizeof(buf), "flashlight t=%d/%d  iters=%d  ms=%.3f", fl.t, fl.ticks, stats.last_iters_avg, stats.last_render_ms);
                dl->AddText(ImVec2(p.x + 6.0f, p.y + 6.0f), IM_COL32(255, 255, 255, 255), buf);
                snprintf(buf, sizeof(buf), "seed32=%u  warp=%.3f", (uint32_t)fl.seed32, params.explaino_warp_strength);
                dl->AddText(ImVec2(p.x + 6.0f, p.y + 26.0f), IM_COL32(220, 220, 220, 255), buf);
                snprintf(buf, sizeof(buf), "center=(%.6f, %.6f)  log2_zoom=%.4f", view.center_hp_x, view.center_hp_y, view.log2_zoom);
                dl->AddText(ImVec2(p.x + 6.0f, p.y + 44.0f), IM_COL32(220, 220, 220, 255), buf);

                // UV sample points.
                const ImVec2 sp[5] = {
                    ImVec2(0.5f, 0.5f),
                    ImVec2(0.25f, 0.25f),
                    ImVec2(0.75f, 0.25f),
                    ImVec2(0.25f, 0.75f),
                    ImVec2(0.75f, 0.75f),
                };
                for (int i = 0; i < 5; i++) {
                    ImVec2 q(rectMin.x + sp[i].x * size.x, rectMin.y + sp[i].y * size.y);
                    dl->AddCircle(q, 4.0f, IM_COL32(255, 240, 0, 200), 12, 1.5f);
                }
            }

            bool hovered = ImGui::IsItemHovered();
            bool active = ImGui::IsItemActive();

            if (hovered) {
                // Mouse wheel zoom around cursor.
                if (io.MouseWheel != 0.0f && size.x > 0.0f && size.y > 0.0f) {
                    ImVec2 mp = ImGui::GetMousePos();
                    float u = (mp.x - rectMin.x) / size.x;
                    float v = (mp.y - rectMin.y) / size.y;
                    u = ClampF(u, 0.0f, 1.0f);
                    v = ClampF(v, 0.0f, 1.0f);

                    double aspect = (render.resolution.y > 0) ? (double)render.resolution.x / (double)render.resolution.y : 1.0;
                    double zoomOld = SafeZoomFromLog2(view.log2_zoom);
                    double baseOld = 2.0 / fmax(1e-30, zoomOld);
                    double nx = ((double)u - 0.5) * 2.0;
                    double ny = ((double)v - 0.5) * 2.0;

                    double worldX = view.center_hp_x + nx * baseOld * aspect;
                    double worldY = view.center_hp_y + ny * baseOld;

                    double factor = pow(1.10, (double)io.MouseWheel);
                    view.log2_zoom = ClampD(view.log2_zoom + Log2D(factor), Log2D(kMinZoom), kMaxLog2Zoom);
                    double zoomNew = SafeZoomFromLog2(view.log2_zoom);
                    double baseNew = 2.0 / fmax(1e-30, zoomNew);
                    view.center_hp_x = worldX - nx * baseNew * aspect;
                    view.center_hp_y = worldY - ny * baseNew;
                    SyncViewUiFromHp(view);
                    dirty = true;
                }

                // Drag pan.
                if (active && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && render.resolution.x > 0 && render.resolution.y > 0) {
                    ImVec2 dpx = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
                    ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);

                    double aspect = (render.resolution.y > 0) ? (double)render.resolution.x / (double)render.resolution.y : 1.0;
                    double zoomNow = SafeZoomFromLog2(view.log2_zoom);
                    double base = 2.0 / fmax(1e-30, zoomNow);

                    double dxWorld = ((double)dpx.x / (double)render.resolution.x) * 2.0 * base * aspect;
                    double dyWorld = ((double)dpx.y / (double)render.resolution.y) * 2.0 * base;

                    // "Grab" semantics: dragging right moves content right.
                    view.center_hp_x -= dxWorld;
                    view.center_hp_y -= dyWorld;
                    SyncViewUiFromHp(view);
                    dirty = true;
                }
            }
        } else {
            ImGui::TextUnformatted("No texture yet. Toggle auto-refresh or click Render Once.");
        }
        ImGui::End();

        // Lens window (SDF preview)
        ImGui::Begin("Lens");
        if (!lens.enabled) {
            ImGui::TextUnformatted("Lens disabled (enable in Controls -> Lens, or bind it in schema). ");
        } else if (g_lensSRV) {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            float scale = 1.0f;
            if (lensW > 0 && lensH > 0) {
                float sx = avail.x / (float)lensW;
                float sy = avail.y / (float)lensH;
                scale = (sx < sy) ? sx : sy;
                if (scale <= 0.0f) scale = 1.0f;
            }
            ImVec2 size((float)lensW * scale, (float)lensH * scale);
            ImGui::Image((ImTextureID)g_lensSRV, size);
            ImGui::Text("SDF (low-res %dx%d)", lensW, lensH);
        } else {
            ImGui::TextUnformatted("No lens texture yet.");
        }
        ImGui::End();

        // Mask window (raw interior mask)
        ImGui::Begin("Mask");
        if (!lens.enabled) {
            ImGui::TextUnformatted("Lens disabled (mask not produced).");
        } else if (g_maskSRV) {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            float scale = 1.0f;
            if (lensW > 0 && lensH > 0) {
                float sx = avail.x / (float)lensW;
                float sy = avail.y / (float)lensH;
                scale = (sx < sy) ? sx : sy;
                if (scale <= 0.0f) scale = 1.0f;
            }
            ImVec2 size((float)lensW * scale, (float)lensH * scale);
            ImGui::Image((ImTextureID)g_maskSRV, size);
            ImGui::Text("mask (low-res %dx%d)", lensW, lensH);
        } else {
            ImGui::TextUnformatted("No mask texture yet.");
        }
        ImGui::End();

        // Flashlight trace window (vector path)
        if (flashlightLive && fl.requested && fl.showTrace) {
            {
                ImGuiViewport* vp = ImGui::GetMainViewport();
                if (vp) {
                    const float w = fmaxf(360.0f, vp->WorkSize.x * 0.26f);
                    const float h = fmaxf(220.0f, vp->WorkSize.y * 0.30f);
                    ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_FirstUseEver);
                    ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x + vp->WorkSize.x - w, vp->WorkPos.y + 8.0f), ImGuiCond_FirstUseEver);
                }
            }
            ImGui::Begin("Flashlight Trace");
            ImVec2 avail = ImGui::GetContentRegionAvail();
            if (avail.x < 10.0f) avail.x = 10.0f;
            if (avail.y < 10.0f) avail.y = 10.0f;

            ImVec2 p0 = ImGui::GetCursorScreenPos();
            ImVec2 p1(p0.x + avail.x, p0.y + avail.y);
            ImDrawList* dl = ImGui::GetWindowDrawList();
            dl->AddRectFilled(p0, p1, IM_COL32(10, 10, 10, 255));
            dl->AddRect(p0, p1, IM_COL32(80, 80, 80, 255));

            if (fl.path.size() >= 2 && !fl.samplesSignedPx.empty() && fl.samplesSignedPx.size() == fl.path.size()) {
                const int orbitTicks = (int)fl.path.size();

                const double now = ImGui::GetTime();
                const double elapsed = fl.active ? (now - fl.liveStartTime) : fl.pauseElapsed;
                const double orbitSeconds = fl.paceFixedTickHz
                    ? ((double)orbitTicks / (double)fmaxf(0.01f, fl.tickRateHz))
                    : (double)fmaxf(1.0f, fl.orbitSeconds);
                double headPhase = (orbitSeconds > 0.0) ? (elapsed / orbitSeconds) * (double)orbitTicks : 0.0;
                headPhase = fmod(headPhase, (double)orbitTicks);
                if (headPhase < 0.0) headPhase += (double)orbitTicks;
                const int headIdx = (int)floor(headPhase);
                const double headF = headPhase - (double)headIdx;

                int trailTicks = (int)round(fmaxf(0.05f, fminf(0.95f, fl.trailFraction)) * (float)orbitTicks);
                if (trailTicks < 2) trailTicks = 2;
                if (trailTicks > orbitTicks - 1) trailTicks = orbitTicks - 1;

                auto minAbsSignedAt = [&](int idx) -> float {
                    const auto& s = fl.samplesSignedPx[(size_t)idx];
                    float minAbs = fabsf(s[0]);
                    for (int ci = 1; ci <= 4; ci++) {
                        float a = fabsf(s[(size_t)ci]);
                        if (a < minAbs) minAbs = a;
                    }
                    return minAbs;
                };

                auto pressureAt = [&](int idx) -> float {
                    const float decay = fmaxf(0.25f, fl.tracePressureDecayPx);
                    const float minAbs = minAbsSignedAt(idx);
                    return expf(-minAbs / decay);
                };

                auto wrapAge = [&](double phaseAt) -> double {
                    double a = headPhase - phaseAt;
                    while (a < 0.0) a += (double)orbitTicks;
                    while (a >= (double)orbitTicks) a -= (double)orbitTicks;
                    return a;
                };

                // Fixed bounds: entire orbit stays in-frame.
                float minx = fl.path[0].x;
                float maxx = fl.path[0].x;
                float miny = fl.path[0].y;
                float maxy = fl.path[0].y;
                for (int i = 1; i < orbitTicks; i++) {
                    const ImVec2 v = fl.path[(size_t)i];
                    if (v.x < minx) minx = v.x;
                    if (v.x > maxx) maxx = v.x;
                    if (v.y < miny) miny = v.y;
                    if (v.y > maxy) maxy = v.y;
                }

                float pull = fmaxf(1.0f, fl.traceBoundsPullback);
                float cx = 0.5f * (minx + maxx);
                float cy = 0.5f * (miny + maxy);
                float hx = 0.5f * (maxx - minx) * pull;
                float hy = 0.5f * (maxy - miny) * pull;
                if (hx < 1e-12f) hx = 1e-12f;
                if (hy < 1e-12f) hy = 1e-12f;
                minx = cx - hx;
                maxx = cx + hx;
                miny = cy - hy;
                maxy = cy + hy;

                float dx = maxx - minx;
                float dy = maxy - miny;
                if (dx < 1e-6f) dx = 1e-6f;
                if (dy < 1e-6f) dy = 1e-6f;
                const float pad = 8.0f;

                auto mapToPlot = [&](ImVec2 w) {
                    float u = (w.x - minx) / dx;
                    float vv = (w.y - miny) / dy;
                    ImVec2 q;
                    q.x = p0.x + pad + u * (avail.x - 2.0f * pad);
                    q.y = p1.y - pad - vv * (avail.y - 2.0f * pad);
                    return q;
                };

                // Build a trailing segment (fixed tick samples) + a tweened head point.
                // IMPORTANT: spline smoothing must NOT include the moving head endpoint,
                // otherwise the whole curve will "wobble" as the endpoint moves.
                std::vector<ImVec2> rawPtsTrail;
                std::vector<float> rawPressureTrail;
                std::vector<double> rawPhaseTrail;
                rawPtsTrail.reserve((size_t)trailTicks + 1u);
                rawPressureTrail.reserve((size_t)trailTicks + 1u);
                rawPhaseTrail.reserve((size_t)trailTicks + 1u);

                for (int k = trailTicks; k >= 0; k--) {
                    int idx = headIdx - k;
                    while (idx < 0) idx += orbitTicks;
                    while (idx >= orbitTicks) idx -= orbitTicks;
                    rawPtsTrail.push_back(mapToPlot(fl.path[(size_t)idx]));
                    rawPressureTrail.push_back(pressureAt(idx));
                    rawPhaseTrail.push_back((double)idx);
                }

                const int idx0 = headIdx;
                const int idx1 = (headIdx + 1) % orbitTicks;
                const ImVec2 w0 = fl.path[(size_t)idx0];
                const ImVec2 w1 = fl.path[(size_t)idx1];
                const ImVec2 headWorld = ImVec2(w0.x + (w1.x - w0.x) * (float)headF, w0.y + (w1.y - w0.y) * (float)headF);
                const float p0 = pressureAt(idx0);
                const float p1 = pressureAt(idx1);
                const float pHead = p0 + (p1 - p0) * (float)headF;

                const ImVec2 headPlot = mapToPlot(headWorld);

                // Optional spline smoothing.
                std::vector<ImVec2> pts;
                std::vector<float> pres;
                std::vector<double> phase;
                if (fl.traceSpline && rawPtsTrail.size() >= 2) {
                    const int steps = (fl.traceSplineSteps < 1) ? 1 : ((fl.traceSplineSteps > 64) ? 64 : fl.traceSplineSteps);
                    pts.reserve(rawPtsTrail.size() * (size_t)steps + 2);
                    pres.reserve(rawPtsTrail.size() * (size_t)steps + 2);
                    phase.reserve(rawPtsTrail.size() * (size_t)steps + 2);
                    for (int i = 0; i + 1 < (int)rawPtsTrail.size(); i++) {
                        ImVec2 p0s = rawPtsTrail[(i - 1 >= 0) ? i - 1 : i];
                        ImVec2 p1s = rawPtsTrail[i];
                        ImVec2 p2s = rawPtsTrail[i + 1];
                        ImVec2 p3s = rawPtsTrail[(i + 2 < (int)rawPtsTrail.size()) ? i + 2 : (int)rawPtsTrail.size() - 1];
                        for (int j = 0; j < steps; j++) {
                            float tt = (float)j / (float)steps;
                            pts.push_back(CatmullRom(p0s, p1s, p2s, p3s, tt));
                            pres.push_back(rawPressureTrail[(size_t)i] * (1.0f - tt) + rawPressureTrail[(size_t)i + 1] * tt);
                            phase.push_back(rawPhaseTrail[(size_t)i] * (1.0 - (double)tt) + rawPhaseTrail[(size_t)i + 1] * (double)tt);
                        }
                    }
                    pts.push_back(rawPtsTrail.back());
                    pres.push_back(rawPressureTrail.back());
                    phase.push_back(rawPhaseTrail.back());
                } else {
                    pts = rawPtsTrail;
                    pres = rawPressureTrail;
                    phase = rawPhaseTrail;
                }

                // Append the tweened head as a final segment without affecting the spline.
                pts.push_back(headPlot);
                pres.push_back(pHead);
                phase.push_back(headPhase);

                // Draw: lightweight rainbow phase line with mask connectedness modulation.
                if (pts.size() >= 2) {
                    const float w0 = fmaxf(0.25f, fl.traceWidthMin);
                    const float w1 = fmaxf(w0, fl.traceWidthMax);
                    const int ridgeN = (fl.ridgeN < 1) ? 1 : ((fl.ridgeN > 64) ? 64 : fl.ridgeN);
                    const double period = (double)orbitTicks / (double)ridgeN;
                    const double sigma = fmax(0.10, (double)fl.ridgeSigmaTicks);
                    const float ridgeStrength = fmaxf(0.0f, fl.ridgeStrength);

                    // Connectedness/mixing: derived from the 5-point inside/outside samples.
                    // If the neighborhood is uniform (all inside or all outside), connectedness is high.
                    // If it is mixed, connectedness is low (boundary pressure / divergence cue).
                    auto mixAt = [&](int idx) -> float {
                        if (idx < 0) idx = 0;
                        if (idx >= orbitTicks) idx = orbitTicks - 1;
                        if (fl.samplesInside.empty() || (int)fl.samplesInside.size() != orbitTicks) return 0.0f;
                        const auto& in = fl.samplesInside[(size_t)idx];
                        int insideCount = 0;
                        for (int k = 0; k < 5; k++) insideCount += (in[(size_t)k] ? 1 : 0);
                        const int outsideCount = 5 - insideCount;
                        const int minority = (insideCount < outsideCount) ? insideCount : outsideCount; // 0..2
                        // Normalize minority count to [0..1]. For 5 samples, max minority is 2.
                        return ClampF((float)minority / 2.0f, 0.0f, 1.0f);
                    };

                    const float hue0 = (float)Hash01(HashU32(fl.seed32 ^ 0xB8F1C2D3u));

                    auto hsvCol = [&](float h, float s, float v, float a) -> ImU32 {
                        float r = 1.0f, g = 1.0f, b = 1.0f;
                        ImGui::ColorConvertHSVtoRGB(h - floorf(h), s, v, r, g, b);
                        return ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, SaturateF(a)));
                    };

                    // Cheap "phase tendrils": short, random-ish filaments aligned to local tangent.
                    if (fl.traceSplinters) {
                        const int every = 5;
                        for (int i = 1; i + 1 < (int)pts.size(); i += every) {
                            const float pHere = pres[(size_t)i];
                            if (pHere < 0.12f) continue;

                            ImVec2 a = pts[(size_t)i];
                            ImVec2 b = pts[(size_t)i + 1];
                            ImVec2 t(b.x - a.x, b.y - a.y);
                            float tn = sqrtf(t.x * t.x + t.y * t.y);
                            if (!isfinite(tn) || tn < 1e-4f) continue;
                            t.x /= tn; t.y /= tn;
                            ImVec2 n(-t.y, t.x);

                            const uint32_t hh = HashU32((uint32_t)i ^ (uint32_t)orbitTicks ^ fl.seed32 ^ 0x13C0FFEEu);
                            const float j0 = ((float)Hash01(hh) - 0.5f);
                            const float j1 = ((float)Hash01(HashU32(hh ^ 0x9E3779B9u)) - 0.5f);
                            const float twist = 0.85f + 0.60f * j0;
                            const float sgn = (j1 < 0.0f) ? -1.0f : 1.0f;

                            ImVec2 dir(t.x * (1.0f - 0.35f) + n.x * (0.35f * twist * sgn),
                                      t.y * (1.0f - 0.35f) + n.y * (0.35f * twist * sgn));
                            float dn = sqrtf(dir.x * dir.x + dir.y * dir.y);
                            if (!isfinite(dn) || dn < 1e-4f) continue;
                            dir.x /= dn; dir.y /= dn;

                            const float len = (6.0f + 18.0f * pHere) * (0.65f + 0.70f * fabsf(j0));
                            const float h = hue0 + (float)fmod(phase[(size_t)i] / (double)orbitTicks, 1.0);
                            const float a0 = 0.06f + 0.16f * pHere;
                            dl->AddLine(a, ImVec2(a.x + dir.x * len, a.y + dir.y * len), hsvCol(h, 0.85f, 1.00f, a0), 1.0f);
                        }
                    }

                    for (int i = 0; i + 1 < (int)pts.size(); i++) {
                        const ImVec2 a = pts[(size_t)i];
                        const ImVec2 b = pts[(size_t)i + 1];
                        const float pMid = 0.5f * (pres[(size_t)i] + pres[(size_t)i + 1]);
                        const double phMid = 0.5 * (phase[(size_t)i] + phase[(size_t)i + 1]);
                        const double age = wrapAge(phMid);
                        int idxG = (int)floor(phMid + 0.5);
                        while (idxG < 0) idxG += orbitTicks;
                        while (idxG >= orbitTicks) idxG -= orbitTicks;
                        const float mix = mixAt(idxG);

                        // Gaussian ridges repeating every (orbitTicks / ridgeN).
                        double m = fmod(age, period);
                        if (m < 0.0) m += period;
                        const double d = fmin(m, period - m);
                        const double g = exp(-(d * d) / (2.0 * sigma * sigma));

                        const float fade = powf(ClampF(1.0f - (float)(age / (double)trailTicks), 0.0f, 1.0f), 1.10f);
                        const float ridge = ClampF(pMid * (ridgeStrength * (float)g), 0.0f, 1.0f);
                        const float intensity = ClampF(0.25f + 0.75f * pMid, 0.0f, 1.0f);
                        // Wider/stronger where the local mask is *less* connected (more mixed).
                        const float wide = ClampF(0.55f + 0.45f * mix, 0.0f, 1.0f);
                        const float width = w0 + (w1 - w0) * ClampF(0.45f * intensity + 0.35f * wide + 0.20f * ridge, 0.0f, 1.0f);

                        const float h = hue0 + (float)fmod(phMid / (double)orbitTicks, 1.0);
                        const float aCore = (0.10f + 0.52f * fade) * (0.35f + 0.65f * fmaxf(intensity, wide));
                        const float aOut = aCore * 0.85f;

                        // Thin outline + core (2 passes) keeps it readable without killing FPS.
                        dl->AddLine(a, b, IM_COL32(0, 0, 0, (int)(255.0f * SaturateF(aOut))), width + 1.75f);
                        dl->AddLine(a, b, hsvCol(h, 0.90f, 1.00f, aCore), width);
                    }
                }
            } else {
                dl->AddText(ImVec2(p0.x + 8.0f, p0.y + 8.0f), IM_COL32(200, 200, 200, 255), "(no trace yet)");
            }

            ImGui::Dummy(avail);
            ImGui::End();
        }

        // Flashlight composite window (frame + SDF + overlays in camera projection)
        if (flashlightLive && fl.requested && fl.showComposite) {
            {
                ImGuiViewport* vp = ImGui::GetMainViewport();
                if (vp) {
                    const float w = fmaxf(640.0f, vp->WorkSize.x * 0.72f);
                    const float h = fmaxf(420.0f, vp->WorkSize.y * 0.90f);
                    ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_FirstUseEver);
                    ImGui::SetNextWindowPos(ImVec2(vp->WorkPos.x + 8.0f, vp->WorkPos.y + 8.0f), ImGuiCond_FirstUseEver);
                }
            }
            ImGui::Begin("Flashlight Composite");

            ImVec2 avail = ImGui::GetContentRegionAvail();
            if (avail.x < 10.0f) avail.x = 10.0f;
            if (avail.y < 10.0f) avail.y = 10.0f;

            const float rw = (float)render.resolution.x;
            const float rh = (float)render.resolution.y;

            float scale = 1.0f;
            if (rw > 0.0f && rh > 0.0f) {
                float sx = avail.x / rw;
                float sy = avail.y / rh;
                scale = (sx < sy) ? sx : sy;
                if (scale <= 0.0f) scale = 1.0f;
            }

            ImVec2 rectMin = ImGui::GetCursorScreenPos();
            ImGui::InvisibleButton("##flashlight_composite_canvas", ImVec2(rw * scale, rh * scale));
            ImVec2 rectMax = ImGui::GetItemRectMax();
            ImDrawList* dl = ImGui::GetWindowDrawList();

            const bool hovered = ImGui::IsItemHovered();
            const bool active = ImGui::IsItemActive();

            // Composite pan/zoom operates in image pixel space (not world space) so it affects
            // the underlay + SDF + all overlays equally.
            if (fl.compositeCamPanZoom && hovered && rw > 0.0f && rh > 0.0f && scale > 0.0f) {
                if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                    fl.compositeCamLog2Zoom = 0.0f;
                    fl.compositeCamPanPx = ImVec2(0.0f, 0.0f);
                }

                // Mouse wheel zoom around cursor.
                if (io.MouseWheel != 0.0f) {
                    ImVec2 mp = ImGui::GetMousePos();
                    ImVec2 pDisp((mp.x - rectMin.x) / scale, (mp.y - rectMin.y) / scale);

                    ImVec2 centerPx(rw * 0.5f, rh * 0.5f);
                    float zoomOld = exp2f(fl.compositeCamLog2Zoom);
                    if (!isfinite(zoomOld) || zoomOld <= 1e-6f) zoomOld = 1.0f;

                    // Inverse mapping: pSrc = center + (pDisp-center)/zoom - pan
                    ImVec2 pSrc(
                        centerPx.x + (pDisp.x - centerPx.x) / zoomOld - fl.compositeCamPanPx.x,
                        centerPx.y + (pDisp.y - centerPx.y) / zoomOld - fl.compositeCamPanPx.y);

                    float nextLog2 = fl.compositeCamLog2Zoom + io.MouseWheel * 0.18f;
                    nextLog2 = ClampF(nextLog2, -6.0f, 6.0f);
                    float zoomNew = exp2f(nextLog2);
                    if (!isfinite(zoomNew) || zoomNew <= 1e-6f) zoomNew = 1.0f;

                    ImVec2 newPan(
                        centerPx.x + (pDisp.x - centerPx.x) / zoomNew - pSrc.x,
                        centerPx.y + (pDisp.y - centerPx.y) / zoomNew - pSrc.y);

                    fl.compositeCamLog2Zoom = nextLog2;
                    fl.compositeCamPanPx = newPan;
                }

                // Drag pan.
                if (active && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
                    ImVec2 dpx = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
                    ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
                    float zoomNow = exp2f(fl.compositeCamLog2Zoom);
                    if (!isfinite(zoomNow) || zoomNow <= 1e-6f) zoomNow = 1.0f;
                    // Drag right -> content right (grab semantics).
                    fl.compositeCamPanPx.x += (dpx.x / scale) / zoomNow;
                    fl.compositeCamPanPx.y += (dpx.y / scale) / zoomNow;
                }
            }

            ImVec2 uvMin(0.0f, 0.0f);
            ImVec2 uvMax(1.0f, 1.0f);
            {
                // Convert composite camera (pixel-space) into an affine UV crop.
                ImVec2 centerPx(rw * 0.5f, rh * 0.5f);
                float zoomNow = fl.compositeCamPanZoom ? exp2f(fl.compositeCamLog2Zoom) : 1.0f;
                if (!isfinite(zoomNow) || zoomNow <= 1e-6f) zoomNow = 1.0f;

                // pSrc = center + (pDisp-center)/zoom - pan
                ImVec2 pSrc0(
                    centerPx.x + (0.0f - centerPx.x) / zoomNow - fl.compositeCamPanPx.x,
                    centerPx.y + (0.0f - centerPx.y) / zoomNow - fl.compositeCamPanPx.y);
                ImVec2 pSrc1(
                    centerPx.x + (rw - centerPx.x) / zoomNow - fl.compositeCamPanPx.x,
                    centerPx.y + (rh - centerPx.y) / zoomNow - fl.compositeCamPanPx.y);

                uvMin = ImVec2(pSrc0.x / rw, pSrc0.y / rh);
                uvMax = ImVec2(pSrc1.x / rw, pSrc1.y / rh);
            }

            auto pixelToScreen = [&](ImVec2 pSrc) -> ImVec2 {
                // Forward mapping: pDisp = center + zoom*(pSrc-center + pan)
                ImVec2 centerPx(rw * 0.5f, rh * 0.5f);
                float zoomNow = fl.compositeCamPanZoom ? exp2f(fl.compositeCamLog2Zoom) : 1.0f;
                if (!isfinite(zoomNow) || zoomNow <= 1e-6f) zoomNow = 1.0f;
                ImVec2 pDisp(
                    centerPx.x + zoomNow * ((pSrc.x - centerPx.x) + fl.compositeCamPanPx.x),
                    centerPx.y + zoomNow * ((pSrc.y - centerPx.y) + fl.compositeCamPanPx.y));
                return ImVec2(rectMin.x + pDisp.x * scale, rectMin.y + pDisp.y * scale);
            };

            // Background/SDF are drawn after computing revealT (so we can optionally "inflate" them from the trace).

            auto worldToPixel = [&](ImVec2 w) -> ImVec2 {
                // Project world -> pixel using the current view camera.
                double aspect = (render.resolution.y > 0) ? (double)render.resolution.x / (double)render.resolution.y : 1.0;
                double zoomNow = SafeZoomFromLog2(view.log2_zoom);
                double base = 2.0 / fmax(1e-30, zoomNow);
                double nx = ((double)w.x - view.center_hp_x) / (base * aspect);
                double ny = ((double)w.y - view.center_hp_y) / (base);
                double px = (nx / 2.0 + 0.5) * (double)render.resolution.x;
                double py = (ny / 2.0 + 0.5) * (double)render.resolution.y;
                return ImVec2((float)px, (float)py);
            };

            ImU32 frameTint = IM_COL32(220, 235, 255, (int)(255.0f * ClampF(fl.compositeFrameAlpha, 0.0f, 1.0f)));

            auto sampleFrameColor = [&](ImVec2 pSrc) -> ImU32 {
                if (rgba.empty() || render.resolution.x <= 0 || render.resolution.y <= 0) {
                    return IM_COL32(255, 255, 255, 255);
                }
                int ix = (int)floorf(pSrc.x);
                int iy = (int)floorf(pSrc.y);
                if (ix < 0) ix = 0;
                if (iy < 0) iy = 0;
                if (ix >= render.resolution.x) ix = render.resolution.x - 1;
                if (iy >= render.resolution.y) iy = render.resolution.y - 1;
                size_t idx = (size_t)iy * (size_t)render.resolution.x + (size_t)ix;
                if (idx >= rgba.size()) return IM_COL32(255, 255, 255, 255);
                uint32_t v = rgba[idx]; // BGRA in memory
                uint8_t b = (uint8_t)(v & 0xFF);
                uint8_t g = (uint8_t)((v >> 8) & 0xFF);
                uint8_t r = (uint8_t)((v >> 16) & 0xFF);
                return IM_COL32((int)r, (int)g, (int)b, 255);
            };

            if (fl.path.size() >= 2 && rw > 0.0f && rh > 0.0f && !fl.samplesSignedPx.empty() && fl.samplesSignedPx.size() == fl.path.size()) {
                const int orbitTicks = (int)fl.path.size();

                const double now = ImGui::GetTime();
                const double elapsed = fl.active ? (now - fl.liveStartTime) : fl.pauseElapsed;
                const double orbitSeconds = fl.paceFixedTickHz
                    ? ((double)orbitTicks / (double)fmaxf(0.01f, fl.tickRateHz))
                    : (double)fmaxf(1.0f, fl.orbitSeconds);
                double headPhase = (orbitSeconds > 0.0) ? (elapsed / orbitSeconds) * (double)orbitTicks : 0.0;
                headPhase = fmod(headPhase, (double)orbitTicks);
                if (headPhase < 0.0) headPhase += (double)orbitTicks;
                const int headIdx = (int)floor(headPhase);
                const double headF = headPhase - (double)headIdx;

                int trailTicks = (int)round(fmaxf(0.05f, fminf(0.95f, fl.trailFraction)) * (float)orbitTicks);
                if (trailTicks < 2) trailTicks = 2;
                if (trailTicks > orbitTicks - 1) trailTicks = orbitTicks - 1;

                auto minAbsSignedAt = [&](int idx) -> float {
                    const auto& s = fl.samplesSignedPx[(size_t)idx];
                    float minAbs = fabsf(s[0]);
                    for (int ci = 1; ci <= 4; ci++) {
                        float a = fabsf(s[(size_t)ci]);
                        if (a < minAbs) minAbs = a;
                    }
                    return minAbs;
                };

                auto pressureAt = [&](int idx) -> float {
                    const float decay = fmaxf(0.25f, fl.tracePressureDecayPx);
                    const float minAbs = minAbsSignedAt(idx);
                    return expf(-minAbs / decay);
                };

                auto wrapAge = [&](double phaseAt) -> double {
                    double a = headPhase - phaseAt;
                    while (a < 0.0) a += (double)orbitTicks;
                    while (a >= (double)orbitTicks) a -= (double)orbitTicks;
                    return a;
                };

                // Composite layering preference (meaning-first):
                // - Preserve the fractal structure (frame is primary).
                // - Use SDF as a secondary field; when reach-only is enabled, keep it subtle globally
                //   and add local energy only along the reachable manifold.
                dl->AddRectFilled(rectMin, rectMax, IM_COL32(0, 0, 0, 255));

                if (g_fractalSRV) {
                    float baseA = ClampF(fl.compositeFrameAlpha, 0.0f, 1.0f);
                    ImU32 underTint = IM_COL32(255, 255, 255, (int)(255.0f * baseA));
                    dl->AddImage((ImTextureID)g_fractalSRV, rectMin, rectMax, uvMin, uvMax, underTint);
                }

                if (g_lensSRV && fl.compositeSdfAlpha > 0.0f) {
                    float a = ClampF(fl.compositeSdfAlpha, 0.0f, 1.0f);
                    // Reach-only: keep global SDF faint; structure emphasis will be added locally.
                    if (fl.compositeReachOnly) a *= 0.20f;
                    ImU32 tint = IM_COL32(255, 255, 255, (int)(255.0f * a));
                    dl->AddImage((ImTextureID)g_lensSRV, rectMin, rectMax, uvMin, uvMax, tint);
                }

                // Optional global darken to stop bright regions (yellow) from washing out the line.
                {
                    float a = ClampF(fl.compositeUnderDarken, 0.0f, 0.8f);
                    if (a > 0.0f) {
                        dl->AddRectFilled(rectMin, rectMax, IM_COL32(0, 0, 0, (int)(255.0f * a)));
                    }
                }

                // Connectedness/mix (reach-only driver): derived from 5-point inside/outside sampling.
                // 0 => uniform neighborhood (high connectedness), 1 => mixed neighborhood (boundary pressure).
                auto mixAt = [&](int idx) -> float {
                    if (idx < 0) idx = 0;
                    if (idx >= orbitTicks) idx = orbitTicks - 1;
                    if (fl.samplesInside.empty() || (int)fl.samplesInside.size() != orbitTicks) return 0.0f;
                    const auto& in = fl.samplesInside[(size_t)idx];
                    int insideCount = 0;
                    for (int k = 0; k < 5; k++) insideCount += (in[(size_t)k] ? 1 : 0);
                    const int outsideCount = 5 - insideCount;
                    const int minority = (insideCount < outsideCount) ? insideCount : outsideCount;
                    return ClampF((float)minority / 2.0f, 0.0f, 1.0f);
                };

                // Build a trailing segment (fixed tick samples) + a tweened head point.
                // IMPORTANT: spline smoothing must NOT include the moving head endpoint,
                // otherwise the whole curve will "wobble" as the endpoint moves.
                std::vector<ImVec2> rawPtsTrail;
                std::vector<float> rawPressureTrail;
                std::vector<double> rawPhaseTrail;
                rawPtsTrail.reserve((size_t)trailTicks + 1u);
                rawPressureTrail.reserve((size_t)trailTicks + 1u);
                rawPhaseTrail.reserve((size_t)trailTicks + 1u);

                for (int k = trailTicks; k >= 0; k--) {
                    int idx = headIdx - k;
                    while (idx < 0) idx += orbitTicks;
                    while (idx >= orbitTicks) idx -= orbitTicks;
                    rawPtsTrail.push_back(pixelToScreen(worldToPixel(fl.path[(size_t)idx])));
                    rawPressureTrail.push_back(pressureAt(idx));
                    rawPhaseTrail.push_back((double)idx);
                }

                const int idx0 = headIdx;
                const int idx1 = (headIdx + 1) % orbitTicks;
                const ImVec2 w0 = fl.path[(size_t)idx0];
                const ImVec2 w1 = fl.path[(size_t)idx1];
                const ImVec2 headWorld = ImVec2(w0.x + (w1.x - w0.x) * (float)headF, w0.y + (w1.y - w0.y) * (float)headF);
                const float p0 = pressureAt(idx0);
                const float p1 = pressureAt(idx1);
                const float pHead = p0 + (p1 - p0) * (float)headF;
                const ImVec2 headScreen = pixelToScreen(worldToPixel(headWorld));

                // Reach-only meaning layer: add localized energy only along the manifold.
                // This keeps the rest of the frame readable and avoids “global SDF fog”.
                if (fl.compositeReachOnly && fl.compositeElectric && fl.compositeElectricStrength > 0.0f) {
                    const float strength = fmaxf(0.0f, fl.compositeElectricStrength);
                    const float baseGlow = 0.12f * strength;
                    const float baseSpark = 0.16f * strength;

                    auto addGlowDot = [&](ImVec2 p, float intensity, bool inside) {
                        intensity = ClampF(intensity, 0.0f, 1.0f);
                        if (intensity <= 1e-4f) return;
                        const int a0 = (int)(255.0f * SaturateF(baseGlow * (0.25f + 0.75f * intensity)));
                        const int a1 = (int)(255.0f * SaturateF(baseGlow * 0.55f * (0.20f + 0.80f * intensity)));
                        const int a2 = (int)(255.0f * SaturateF(baseGlow * 0.30f * (0.20f + 0.80f * intensity)));

                        // Electric palette: inside=cyan, outside=magenta, with a white-hot core.
                        ImU32 colOuter = inside ? IM_COL32(40, 235, 255, a2) : IM_COL32(255, 60, 245, a2);
                        ImU32 colMid = inside ? IM_COL32(120, 250, 255, a1) : IM_COL32(255, 140, 250, a1);
                        ImU32 colCore = IM_COL32(255, 255, 255, a0);

                        dl->AddCircle(p, 10.0f + 10.0f * intensity, colOuter, 20, 2.0f);
                        dl->AddCircle(p, 6.0f + 7.0f * intensity, colMid, 18, 2.0f);
                        dl->AddCircleFilled(p, 2.0f + 2.5f * intensity, colCore, 14);
                    };

                    // Use the fixed tick trail points (not the tweened spline) so the meaning layer is stable.
                    for (int k = trailTicks; k >= 0; k--) {
                        int idx = headIdx - k;
                        while (idx < 0) idx += orbitTicks;
                        while (idx >= orbitTicks) idx -= orbitTicks;
                        const float p = pressureAt(idx);
                        const float mix = mixAt(idx);
                        const float intensity = ClampF(mix * (0.20f + 0.80f * p), 0.0f, 1.0f);
                        const bool inside = (fl.samplesInside[(size_t)idx][0] != 0);
                        const ImVec2 pos = pixelToScreen(worldToPixel(fl.path[(size_t)idx]));
                        addGlowDot(pos, intensity, inside);

                        // Sparse sparks aligned to local tangent.
                        if (intensity > 0.18f) {
                            const uint32_t hh = HashU32((uint32_t)idx ^ fl.seed32 ^ 0xA17C9E31u);
                            const float r0 = (float)Hash01(hh);
                            if (r0 < 0.24f) {
                                int idxN = (idx + 1) % orbitTicks;
                                ImVec2 pA = pos;
                                ImVec2 pB = pixelToScreen(worldToPixel(fl.path[(size_t)idxN]));
                                ImVec2 t(pB.x - pA.x, pB.y - pA.y);
                                float tn = sqrtf(t.x * t.x + t.y * t.y);
                                if (isfinite(tn) && tn > 1e-3f) {
                                    t.x /= tn; t.y /= tn;
                                    ImVec2 n(-t.y, t.x);
                                    const float j0 = ((float)Hash01(HashU32(hh ^ 0x9E3779B9u)) - 0.5f);
                                    const float j1 = ((float)Hash01(HashU32(hh ^ 0x7F4A7C15u)) - 0.5f);
                                    const float len = (10.0f + 34.0f * intensity) * (0.70f + 0.80f * fabsf(j0));
                                    const float bend = 0.25f + 0.55f * fabsf(j1);
                                    ImVec2 dir(t.x * (1.0f - 0.30f) + n.x * (0.30f * bend * (j0 < 0.0f ? -1.0f : 1.0f)),
                                              t.y * (1.0f - 0.30f) + n.y * (0.30f * bend * (j0 < 0.0f ? -1.0f : 1.0f)));
                                    float dn = sqrtf(dir.x * dir.x + dir.y * dir.y);
                                    if (isfinite(dn) && dn > 1e-3f) {
                                        dir.x /= dn; dir.y /= dn;
                                        const int aS = (int)(255.0f * SaturateF(baseSpark * (0.25f + 0.75f * intensity)));
                                        ImU32 col = inside ? IM_COL32(80, 245, 255, aS) : IM_COL32(255, 90, 250, aS);
                                        dl->AddLine(pA, ImVec2(pA.x + dir.x * len, pA.y + dir.y * len), col, 1.6f);
                                        dl->AddLine(ImVec2(pA.x + dir.x * len, pA.y + dir.y * len),
                                                    ImVec2(pA.x + dir.x * len + n.x * (0.28f * len) * (j1 < 0.0f ? -1.0f : 1.0f),
                                                          pA.y + dir.y * len + n.y * (0.28f * len) * (j1 < 0.0f ? -1.0f : 1.0f)),
                                                    IM_COL32(255, 255, 255, (int)(0.55f * (float)aS)), 1.2f);
                                    }
                                }
                            }
                        }
                    }
                }

                // Optional spline smoothing.
                std::vector<ImVec2> pts;
                std::vector<float> pres;
                std::vector<double> phase;
                if (fl.traceSpline && rawPtsTrail.size() >= 2) {
                    const int steps = (fl.traceSplineSteps < 1) ? 1 : ((fl.traceSplineSteps > 64) ? 64 : fl.traceSplineSteps);
                    pts.reserve(rawPtsTrail.size() * (size_t)steps + 2);
                    pres.reserve(rawPtsTrail.size() * (size_t)steps + 2);
                    phase.reserve(rawPtsTrail.size() * (size_t)steps + 2);
                    for (int i = 0; i + 1 < (int)rawPtsTrail.size(); i++) {
                        ImVec2 p0s = rawPtsTrail[(i - 1 >= 0) ? i - 1 : i];
                        ImVec2 p1s = rawPtsTrail[i];
                        ImVec2 p2s = rawPtsTrail[i + 1];
                        ImVec2 p3s = rawPtsTrail[(i + 2 < (int)rawPtsTrail.size()) ? i + 2 : (int)rawPtsTrail.size() - 1];
                        for (int j = 0; j < steps; j++) {
                            float tt = (float)j / (float)steps;
                            pts.push_back(CatmullRom(p0s, p1s, p2s, p3s, tt));
                            pres.push_back(rawPressureTrail[(size_t)i] * (1.0f - tt) + rawPressureTrail[(size_t)i + 1] * tt);
                            phase.push_back(rawPhaseTrail[(size_t)i] * (1.0 - (double)tt) + rawPhaseTrail[(size_t)i + 1] * (double)tt);
                        }
                    }
                    pts.push_back(rawPtsTrail.back());
                    pres.push_back(rawPressureTrail.back());
                    phase.push_back(rawPhaseTrail.back());
                } else {
                    pts = rawPtsTrail;
                    pres = rawPressureTrail;
                    phase = rawPhaseTrail;
                }

                // Append the tweened head as a final segment without affecting the spline.
                pts.push_back(headScreen);
                pres.push_back(pHead);
                phase.push_back(headPhase);

                // Vectors (tick-to-tick displacement arrows).
                if (fl.compositeVectors && pts.size() >= 2) {
                    float strength = fmaxf(0.0f, fl.compositeVectorStrength);
                    float lenFrac = ClampF(fl.compositeVectorLenFrac, 0.0f, 1.0f);
                    for (int i = 0; i + 1 < (int)pts.size(); i++) {
                        ImVec2 a = pts[(size_t)i];
                        ImVec2 b = pts[(size_t)i + 1];
                        ImVec2 v(b.x - a.x, b.y - a.y);
                        b = ImVec2(a.x + v.x * strength * lenFrac, a.y + v.y * strength * lenFrac);

                        float len = sqrtf((b.x - a.x) * (b.x - a.x) + (b.y - a.y) * (b.y - a.y));
                        if (len < 1e-3f) continue;
                        ImVec2 d((b.x - a.x) / len, (b.y - a.y) / len);
                        ImVec2 p(-d.y, d.x);
                        float head = 8.0f;
                        ImVec2 h0(b.x - d.x * head + p.x * (0.6f * head), b.y - d.y * head + p.y * (0.6f * head));
                        ImVec2 h1(b.x - d.x * head - p.x * (0.6f * head), b.y - d.y * head - p.y * (0.6f * head));

                        ImU32 col = IM_COL32(120, 235, 255, 120);
                        dl->AddLine(a, b, col, 1.4f);
                        dl->AddTriangleFilled(b, h0, h1, IM_COL32(120, 235, 255, 95));
                    }
                }

                // Trace (lightweight rainbow phase line with mask connectedness modulation).
                if (fl.compositeTrace && pts.size() >= 2) {
                    const float w0 = fmaxf(0.25f, fl.traceWidthMin);
                    const float w1 = fmaxf(w0, fl.traceWidthMax);
                    const int ridgeN = (fl.ridgeN < 1) ? 1 : ((fl.ridgeN > 64) ? 64 : fl.ridgeN);
                    const double period = (double)orbitTicks / (double)ridgeN;
                    const double sigma = fmax(0.10, (double)fl.ridgeSigmaTicks);
                    const float ridgeStrength = fmaxf(0.0f, fl.ridgeStrength);

                    auto mixAt = [&](int idx) -> float {
                        if (idx < 0) idx = 0;
                        if (idx >= orbitTicks) idx = orbitTicks - 1;
                        if (fl.samplesInside.empty() || (int)fl.samplesInside.size() != orbitTicks) return 0.0f;
                        const auto& in = fl.samplesInside[(size_t)idx];
                        int insideCount = 0;
                        for (int k = 0; k < 5; k++) insideCount += (in[(size_t)k] ? 1 : 0);
                        const int outsideCount = 5 - insideCount;
                        const int minority = (insideCount < outsideCount) ? insideCount : outsideCount;
                        return ClampF((float)minority / 2.0f, 0.0f, 1.0f);
                    };

                    const float hue0 = (float)Hash01(HashU32(fl.seed32 ^ 0xB8F1C2D3u));
                    auto hsvCol = [&](float h, float s, float v, float a) -> ImU32 {
                        float r = 1.0f, g = 1.0f, b = 1.0f;
                        ImGui::ColorConvertHSVtoRGB(h - floorf(h), s, v, r, g, b);
                        return ImGui::ColorConvertFloat4ToU32(ImVec4(r, g, b, SaturateF(a)));
                    };

                    // Cheap "phase tendrils" on the trail.
                    if (fl.traceSplinters) {
                        const int every = 6;
                        for (int i = 1; i + 1 < (int)pts.size(); i += every) {
                            const float pHere = pres[(size_t)i];
                            if (pHere < 0.12f) continue;
                            ImVec2 a = pts[(size_t)i];
                            ImVec2 b = pts[(size_t)i + 1];
                            ImVec2 t(b.x - a.x, b.y - a.y);
                            float tn = sqrtf(t.x * t.x + t.y * t.y);
                            if (!isfinite(tn) || tn < 1e-4f) continue;
                            t.x /= tn; t.y /= tn;
                            ImVec2 n(-t.y, t.x);

                            const uint32_t hh = HashU32((uint32_t)i ^ (uint32_t)orbitTicks ^ fl.seed32 ^ 0x13C0FFEEu);
                            const float j0 = ((float)Hash01(hh) - 0.5f);
                            const float j1 = ((float)Hash01(HashU32(hh ^ 0x9E3779B9u)) - 0.5f);
                            const float twist = 0.85f + 0.60f * j0;
                            const float sgn = (j1 < 0.0f) ? -1.0f : 1.0f;

                            ImVec2 dir(t.x * (1.0f - 0.35f) + n.x * (0.35f * twist * sgn),
                                      t.y * (1.0f - 0.35f) + n.y * (0.35f * twist * sgn));
                            float dn = sqrtf(dir.x * dir.x + dir.y * dir.y);
                            if (!isfinite(dn) || dn < 1e-4f) continue;
                            dir.x /= dn; dir.y /= dn;

                            const float len = (6.0f + 22.0f * pHere) * (0.65f + 0.70f * fabsf(j0));
                            const float h = hue0 + (float)fmod(phase[(size_t)i] / (double)orbitTicks, 1.0);
                            const float a0 = 0.06f + 0.16f * pHere;
                            dl->AddLine(a, ImVec2(a.x + dir.x * len, a.y + dir.y * len), hsvCol(h, 0.85f, 1.00f, a0), 1.0f);
                        }
                    }

                    for (int i = 0; i + 1 < (int)pts.size(); i++) {
                        const ImVec2 a = pts[(size_t)i];
                        const ImVec2 b = pts[(size_t)i + 1];
                        const float pMid = 0.5f * (pres[(size_t)i] + pres[(size_t)i + 1]);
                        const double phMid = 0.5 * (phase[(size_t)i] + phase[(size_t)i + 1]);
                        const double age = wrapAge(phMid);
                        int idxG = (int)floor(phMid + 0.5);
                        while (idxG < 0) idxG += orbitTicks;
                        while (idxG >= orbitTicks) idxG -= orbitTicks;
                        const float mix = mixAt(idxG);

                        double m = fmod(age, period);
                        if (m < 0.0) m += period;
                        const double d = fmin(m, period - m);
                        const double g = exp(-(d * d) / (2.0 * sigma * sigma));

                        const float fade = powf(ClampF(1.0f - (float)(age / (double)trailTicks), 0.0f, 1.0f), 1.10f);
                        const float ridge = ClampF(pMid * (ridgeStrength * (float)g), 0.0f, 1.0f);
                        const float intensity = ClampF(0.25f + 0.75f * pMid, 0.0f, 1.0f);
                        const float wide = ClampF(0.55f + 0.45f * mix, 0.0f, 1.0f);
                        const float width = w0 + (w1 - w0) * ClampF(0.45f * intensity + 0.35f * wide + 0.20f * ridge, 0.0f, 1.0f);

                        const float h = hue0 + (float)fmod(phMid / (double)orbitTicks, 1.0);
                        const float aCore = (0.10f + 0.52f * fade) * (0.35f + 0.65f * fmaxf(intensity, wide));
                        const float aOut = aCore * 0.85f;

                        dl->AddLine(a, b, IM_COL32(0, 0, 0, (int)(255.0f * SaturateF(aOut))), width + 1.75f);
                        dl->AddLine(a, b, hsvCol(h, 0.90f, 1.00f, aCore), width);
                    }
                }

                // Mold tendrils (SDF-gradient filaments in lens-low space, mapped to screen).
                if (fl.moldEnabled && !lensMaskLow.empty() && lensW > 1 && lensH > 1) {
                    const int ds = NormalizeLensDownsamplePow2(lens.downsample);
                    const float stepPx = fmaxf(0.05f, fl.moldStepPxLow);
                    const int steps = (fl.moldSteps < 1) ? 1 : ((fl.moldSteps > 64) ? 64 : fl.moldSteps);
                    const int every = (fl.moldEvery < 1) ? 1 : ((fl.moldEvery > 128) ? 128 : fl.moldEvery);
                    const float tangentMix = ClampF(fl.moldTangentMix, 0.0f, 1.0f);
                    const float strength = fmaxf(0.0f, fl.moldStrength);
                    const float pMin = ClampF(fl.moldPressureMin, 0.0f, 1.0f);

                    auto clampXY = [&](int& x, int& y) {
                        if (x < 0) x = 0;
                        if (y < 0) y = 0;
                        if (x >= lensW) x = lensW - 1;
                        if (y >= lensH) y = lensH - 1;
                    };

                    auto sampleSigned = [&](int x, int y, float& outSigned, bool& outInside) -> bool {
                        clampXY(x, y);
                        outSigned = 0.0f;
                        outInside = false;
                        return SampleSignedDistanceSdfChamfer(lensMaskLow.data(), lensW, lensH, x, y, outSigned, outInside);
                    };

                    auto lowToScreen = [&](ImVec2 low) -> ImVec2 {
                        const float px = (float)ds * (low.x + 0.5f);
                        const float py = (float)ds * (low.y + 0.5f);
                        return pixelToScreen(ImVec2(px, py));
                    };

                    for (int k = trailTicks; k >= 0; k--) {
                        int ti = headIdx - k;
                        while (ti < 0) ti += orbitTicks;
                        while (ti >= orbitTicks) ti -= orbitTicks;

                        if ((k % every) != 0) continue;
                        const float pHere = pressureAt(ti);
                        if (pHere < pMin) continue;

                        ImVec2 low = fl.lensLowXY.empty() ? ImVec2(0, 0) : fl.lensLowXY[(size_t)ti];

                        // Add a little phase-dependent jitter so filaments de-correlate.
                        const float jx = ((float)Hash01(HashU32((uint32_t)ti ^ fl.seed32 ^ 0xA511E9B3u)) - 0.5f) * 0.35f;
                        const float jy = ((float)Hash01(HashU32((uint32_t)ti ^ fl.seed32 ^ 0x63D83595u)) - 0.5f) * 0.35f;
                        low.x += jx;
                        low.y += jy;

                        ImVec2 prev = low;
                        for (int si = 0; si < steps; si++) {
                            int ix = (int)floorf(prev.x);
                            int iy = (int)floorf(prev.y);
                            float sC = 0.0f;
                            bool inC = false;
                            if (!sampleSigned(ix, iy, sC, inC)) break;

                            float sx1 = 0.0f, sx0 = 0.0f, sy1 = 0.0f, sy0 = 0.0f;
                            bool tmp = false;
                            sampleSigned(ix + 1, iy, sx1, tmp);
                            sampleSigned(ix - 1, iy, sx0, tmp);
                            sampleSigned(ix, iy + 1, sy1, tmp);
                            sampleSigned(ix, iy - 1, sy0, tmp);
                            ImVec2 grad((sx1 - sx0) * 0.5f, (sy1 - sy0) * 0.5f);
                            float gn = sqrtf(grad.x * grad.x + grad.y * grad.y);
                            if (!isfinite(gn) || gn < 1e-5f) break;
                            grad.x /= gn;
                            grad.y /= gn;

                            // Step toward the manifold (zero level set), then mix in tangent to "crawl".
                            float sgn = (sC >= 0.0f) ? 1.0f : -1.0f;
                            ImVec2 toward(-grad.x * sgn, -grad.y * sgn);
                            ImVec2 tan(-toward.y, toward.x);
                            const float turn = (Hash01(HashU32((uint32_t)ti * 0x9E3779B9u ^ fl.seed32)) < 0.5f) ? -1.0f : 1.0f;
                            tan.x *= turn;
                            tan.y *= turn;

                            ImVec2 dir(
                                toward.x * (1.0f - tangentMix) + tan.x * tangentMix,
                                toward.y * (1.0f - tangentMix) + tan.y * tangentMix);
                            float dn = sqrtf(dir.x * dir.x + dir.y * dir.y);
                            if (!isfinite(dn) || dn < 1e-5f) break;
                            dir.x /= dn;
                            dir.y /= dn;

                            ImVec2 next(prev.x + dir.x * stepPx, prev.y + dir.y * stepPx);

                            float a = (0.02f + 0.28f * pHere) * strength * (1.0f - (float)si / (float)steps);
                            a = ClampF(a, 0.0f, 0.50f);
                            ImVec4 c = inC ? ImVec4(1.00f, 0.62f, 0.20f, a) : ImVec4(0.20f, 0.95f, 1.00f, a);
                            dl->AddLine(lowToScreen(prev), lowToScreen(next), ImGui::ColorConvertFloat4ToU32(c), 1.00f);
                            prev = next;
                        }
                    }
                }
            } else {
                // No trace yet: show full background + SDF so the window is still meaningful.
                dl->AddRectFilled(rectMin, rectMax, IM_COL32(0, 0, 0, 255));
                if (g_lensSRV && fl.compositeSdfAlpha > 0.0f) {
                    float a = ClampF(fl.compositeSdfAlpha, 0.0f, 1.0f);
                    dl->AddImage((ImTextureID)g_lensSRV, rectMin, rectMax, uvMin, uvMax, IM_COL32(255, 255, 255, (int)(255.0f * a)));
                }
                if (g_fractalSRV) {
                    dl->AddImage((ImTextureID)g_fractalSRV, rectMin, rectMax, uvMin, uvMax, frameTint);
                }
                {
                    float a = ClampF(fl.compositeUnderDarken, 0.0f, 0.8f);
                    if (a > 0.0f) {
                        dl->AddRectFilled(rectMin, rectMax, IM_COL32(0, 0, 0, (int)(255.0f * a)));
                    }
                }
            }

            ImGui::End();
        }

        // Explaino "alive" evolution: advance phase only when enabled.
        // Phase is time-integrated, but its rate is driven by the fractal itself via render stats
        // (avg iterations). This keeps the motion coupled to fractal complexity rather than acting
        // like an arbitrary noise warp.
        if (isExplainoFamily(view.fractal_type) && view.explaino_alive) {
            float dt = io.DeltaTime;
            if (!isfinite(dt) || dt < 0.0f) dt = 0.0f;
            float speed = fmaxf(0.0f, view.explaino_alive_speed);

            float drive = 0.0f;
            if (params.max_iter > 0) {
                drive = (float)stats.last_iters_avg / (float)params.max_iter;
            }
            drive = ClampF(drive, 0.0f, 1.0f);

            // Near boundaries/slow convergence, avg iters rises -> drive rises -> more evolution.
            float dphase = dt * speed * (0.20f + 1.80f * drive);
            if (dphase != 0.0f) {
                view.explaino_phase += dphase;
                // keep phase bounded to avoid float blow-up over long sessions
                if (view.explaino_phase > 1.0e6f) view.explaino_phase = fmodf(view.explaino_phase, 6.2831853f);
            }

            // Seed drift: slow walk across seeds. This changes both the polynomial and the warp (via paramsForRender).
            float dseed = dt * speed * (0.06f + 0.44f * drive);
            if (dseed != 0.0f) {
                view.explaino_seed_drift += dseed;
                if (view.explaino_seed_drift > 1.0e6f) view.explaino_seed_drift = fmodf(view.explaino_seed_drift, 1024.0f);
            }

            if (dphase != 0.0f || dseed != 0.0f) {
                UpdateExplainoPolynomial(view, params, &dirty);
                dirty = true;
            }
        }

        // Camera behavior loop (per-frame deltas scaled only by dive_speed; no dt semantics).
        // This runs after UI, so changing behavior updates immediately.
        if (!flashlightLive) {
            ApplyAutoDivePerFrame(view, &dirty);
        }

        // Auto-refresh must be enabled for auto-dive to be observable.
        if (!flashlightLive && view.auto_dive && !view.auto_refresh) {
            view.auto_refresh = true;
            dirty = true;
        }

        // Render frame
        DrawUiErrorOverlayIfAny();
        ImGui::Render();

        const float clear_color_with_alpha[4] = {0.08f, 0.08f, 0.10f, 1.00f};
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}
