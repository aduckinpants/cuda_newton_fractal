#include "fractal_types.h"

#include <Windows.h>
#include <commdlg.h>
#include <d3d11.h>
#include <shellapi.h>
#include <tchar.h>

#include <cstdlib>
#include <cstdio>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <cmath>


#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"

#include "diagnostics_capture.h"
#include "finding_archive_actions.h"
#include "finding_state_actions.h"
#include "explaino_seed.h"
#include "explaino_seed_dynamics.h"
#include "fractal_derived_fields.h"
#include "fractal_family_rules.h"
#include "json_min.h"
#include "lens_sdf.h"
#include "runtime_reset.h"
#include "safe_mode_schema.h"
#include "schema_binding.h"
#include "schema_startup_policy.h"
#include "sweep_player.h"
#include "ui_schema.h"
#include "viewer_shutdown.h"
#include "viewer_sweep.h"
#include "view_hp_sync.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "Comdlg32.lib")

// Forward declare the CUDA renderer (declared in fractal_types.h, linked from fractal_renderer.cu).

static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

static ID3D11Texture2D* g_fractalTexture = nullptr;
static ID3D11ShaderResourceView* g_fractalSRV = nullptr;

static ID3D11Texture2D* g_maskTexture = nullptr;
static ID3D11ShaderResourceView* g_maskSRV = nullptr;

static ID3D11Texture2D* g_lensSdfTexture = nullptr;
static ID3D11ShaderResourceView* g_lensSdfSRV = nullptr;

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

static std::string HeadlessDiagnosticsErrorPath(const std::string& exeDir, const char* fileName) {
    return JoinPath(JoinPath(exeDir, "diagnostics\\last"), fileName);
}

static void ClearHeadlessErrorFile(const std::string& exeDir, const char* fileName) {
    const std::string errorPath = HeadlessDiagnosticsErrorPath(exeDir, fileName);
    DeleteFileA(errorPath.c_str());
}

static void WriteHeadlessErrorFile(const std::string& exeDir, const char* fileName, const std::string& message) {
    const std::string diagnosticsDir = JoinPath(exeDir, "diagnostics");
    const std::string diagnosticsLastDir = JoinPath(diagnosticsDir, "last");
    CreateDirectoryA(diagnosticsDir.c_str(), nullptr);
    CreateDirectoryA(diagnosticsLastDir.c_str(), nullptr);

    const std::string errorPath = HeadlessDiagnosticsErrorPath(exeDir, fileName);
    std::ofstream file(errorPath, std::ios::out | std::ios::binary | std::ios::trunc);
    if (file) file << message << "\n";

    std::fprintf(stderr, "%s\n", message.c_str());
}

static std::string Utf8FromWide(const wchar_t* wide) {
    if (!wide) return {};
    int needed = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
    if (needed <= 0) return {};
    std::string utf8((size_t)needed, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide, -1, utf8.data(), needed, nullptr, nullptr);
    utf8.pop_back();
    return utf8;
}

static std::vector<std::string> GetCommandLineArgsUtf8() {
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    std::vector<std::string> args;
    if (!argv) return args;
    args.reserve((size_t)argc);
    for (int i = 0; i < argc; ++i) args.push_back(Utf8FromWide(argv[i]));
    LocalFree(argv);
    return args;
}

static bool PromptOpenFindingStatePath(HWND owner, std::string* outPath) {
    char buffer[MAX_PATH] = {};
    OPENFILENAMEA dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = owner;
    dialog.lpstrFile = buffer;
    dialog.nMaxFile = static_cast<DWORD>(sizeof(buffer));
    dialog.lpstrFilter = "Finding or State JSON\0finding.json;state.json;*.json\0JSON Files\0*.json\0\0";
    dialog.nFilterIndex = 1;
    dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
    dialog.lpstrDefExt = "json";
    if (!GetOpenFileNameA(&dialog)) return false;
    if (outPath) *outPath = buffer;
    return true;
}

static bool HasArg(const std::vector<std::string>& args, const char* flag) {
    for (const auto& arg : args) {
        if (arg == flag) return true;
    }
    return false;
}

static bool TryGetArgValue(const std::vector<std::string>& args, const char* flag, std::string* outValue) {
    for (size_t i = 0; i + 1 < args.size(); ++i) {
        if (args[i] == flag) {
            if (outValue) *outValue = args[i + 1];
            return true;
        }
    }
    return false;
}

static bool TryParseDoubleArg(const std::vector<std::string>& args, const char* flag, double* outValue) {
    std::string text;
    if (!TryGetArgValue(args, flag, &text)) return false;
    char* end = nullptr;
    double value = std::strtod(text.c_str(), &end);
    if (!end || *end != '\0') return false;
    if (outValue) *outValue = value;
    return true;
}

static bool TryParseIntArg(const std::vector<std::string>& args, const char* flag, int* outValue) {
    std::string text;
    if (!TryGetArgValue(args, flag, &text)) return false;
    char* end = nullptr;
    long value = std::strtol(text.c_str(), &end, 10);
    if (!end || *end != '\0') return false;
    if (outValue) *outValue = (int)value;
    return true;
}

static bool TryParseFractalTypeArg(const std::vector<std::string>& args, FractalType* outType) {
    std::string text;
    if (!TryGetArgValue(args, "--fractal-type", &text)) return false;

    if (text == "newton") { if (outType) *outType = FractalType::newton; return true; }
    if (text == "nova") { if (outType) *outType = FractalType::nova; return true; }
    if (text == "mandelbrot") { if (outType) *outType = FractalType::mandelbrot; return true; }
    if (text == "julia") { if (outType) *outType = FractalType::julia; return true; }
    if (text == "burning_ship") { if (outType) *outType = FractalType::burning_ship; return true; }
    if (text == "multibrot") { if (outType) *outType = FractalType::multibrot; return true; }
    if (text == "phoenix") { if (outType) *outType = FractalType::phoenix; return true; }
    if (text == "explaino") { if (outType) *outType = FractalType::explaino; return true; }
    if (text == "explaino_y") { if (outType) *outType = FractalType::explaino_y; return true; }
    if (text == "explaino_fp") { if (outType) *outType = FractalType::explaino_fp; return true; }
    if (text == "explaino_nova") { if (outType) *outType = FractalType::explaino_nova; return true; }
    if (text == "explaino_halley") { if (outType) *outType = FractalType::explaino_halley; return true; }
    if (text == "explaino_dual") { if (outType) *outType = FractalType::explaino_dual; return true; }
    if (text == "explaino_mult") { if (outType) *outType = FractalType::explaino_mult; return true; }
    if (text == "explaino_phoenix") { if (outType) *outType = FractalType::explaino_phoenix; return true; }
    if (text == "explaino_transcendental") { if (outType) *outType = FractalType::explaino_transcendental; return true; }
    if (text == "explaino_inertial") { if (outType) *outType = FractalType::explaino_inertial; return true; }
    if (text == "explaino_julia") { if (outType) *outType = FractalType::explaino_julia; return true; }
    if (text == "explaino_rational") { if (outType) *outType = FractalType::explaino_rational; return true; }
    if (text == "multicorn") { if (outType) *outType = FractalType::multicorn; return true; }
    if (text == "halley") { if (outType) *outType = FractalType::halley; return true; }
    if (text == "collatz") { if (outType) *outType = FractalType::collatz; return true; }
    return false;
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
    if (g_lensSdfSRV) { g_lensSdfSRV->Release(); g_lensSdfSRV = nullptr; }
    if (g_lensSdfTexture) { g_lensSdfTexture->Release(); g_lensSdfTexture = nullptr; }
    if (g_maskSRV) { g_maskSRV->Release(); g_maskSRV = nullptr; }
    if (g_maskTexture) { g_maskTexture->Release(); g_maskTexture = nullptr; }
    if (g_fractalSRV) { g_fractalSRV->Release(); g_fractalSRV = nullptr; }
    if (g_fractalTexture) { g_fractalTexture->Release(); g_fractalTexture = nullptr; }

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

static void UploadMaskAsRGBA(const uint8_t* mask, int width, int height) {
    if (!mask || !g_maskTexture) return;

    D3D11_MAPPED_SUBRESOURCE mapped{};
    if (SUCCEEDED(g_pd3dDeviceContext->Map(g_maskTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        uint8_t* dst = reinterpret_cast<uint8_t*>(mapped.pData);
        for (int y = 0; y < height; y++) {
            uint8_t* row = dst + (size_t)mapped.RowPitch * (size_t)y;
            const uint8_t* srcRow = mask + (size_t)width * (size_t)y;
            for (int x = 0; x < width; x++) {
                uint8_t v = srcRow[x];
                row[x * 4 + 0] = v;
                row[x * 4 + 1] = v;
                row[x * 4 + 2] = v;
                row[x * 4 + 3] = 255;
            }
        }
        g_pd3dDeviceContext->Unmap(g_maskTexture, 0);
    }
}

static void EnsureLensSdfTexture(int width, int height) {
    if (width <= 0 || height <= 0) return;

    D3D11_TEXTURE2D_DESC desc{};
    if (g_lensSdfTexture) {
        g_lensSdfTexture->GetDesc(&desc);
        if ((int)desc.Width == width && (int)desc.Height == height) return;

        g_lensSdfSRV->Release();
        g_lensSdfSRV = nullptr;
        g_lensSdfTexture->Release();
        g_lensSdfTexture = nullptr;
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

    g_pd3dDevice->CreateTexture2D(&desc, nullptr, &g_lensSdfTexture);
    g_pd3dDevice->CreateShaderResourceView(g_lensSdfTexture, nullptr, &g_lensSdfSRV);
}

static void UploadLensSdfRGBA(const uint32_t* rgba, int width, int height) {
    if (!rgba || !g_lensSdfTexture) return;

    D3D11_MAPPED_SUBRESOURCE mapped{};
    if (SUCCEEDED(g_pd3dDeviceContext->Map(g_lensSdfTexture, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        const uint8_t* src = reinterpret_cast<const uint8_t*>(rgba);
        uint8_t* dst = reinterpret_cast<uint8_t*>(mapped.pData);
        size_t rowBytes = (size_t)width * 4;
        for (int y = 0; y < height; ++y) {
            memcpy(dst + (size_t)mapped.RowPitch * (size_t)y, src + rowBytes * (size_t)y, rowBytes);
        }
        g_pd3dDeviceContext->Unmap(g_lensSdfTexture, 0);
    }
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
        return true;
    }

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

static std::string ReadTextFile(const char* path) {
    std::ifstream f(path, std::ios::in | std::ios::binary);
    if (!f) return {};
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static inline float ClampF(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
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
    view.log2_zoom = ClampInteractionLog2Zoom(view.log2_zoom + dlog2);
    SyncViewUiFromHp(view);
    if (ioDirty) *ioDirty = true;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    std::vector<std::string> args = GetCommandLineArgsUtf8();
    const bool validateUiOnly = HasArg(args, "--validate-ui");
    const bool captureDiagnosticOnly = HasArg(args, "--capture-diagnostic");
    const bool captureFindingOnly = HasArg(args, "--capture-finding");
    if (captureDiagnosticOnly && captureFindingOnly) {
        return 1;
    }
    if (validateUiOnly && (captureDiagnosticOnly || captureFindingOnly)) {
        return 1;
    }

    FractalType cliFractalType = FractalType::newton;
    const bool haveCliFractalType = TryParseFractalTypeArg(args, &cliFractalType);
    if (HasArg(args, "--fractal-type") && !haveCliFractalType) {
        return 1;
    }

    double explainoSeedOverride = 0.0;
    const bool haveExplainoSeedOverride = TryParseDoubleArg(args, "--explaino-seed", &explainoSeedOverride);
    if (HasArg(args, "--explaino-seed") && !haveExplainoSeedOverride) {
        return 1;
    }

    double explainoSeedBOverride = 0.0;
    const bool haveExplainoSeedBOverride = TryParseDoubleArg(args, "--explaino-seed-b", &explainoSeedBOverride);
    if (HasArg(args, "--explaino-seed-b") && !haveExplainoSeedBOverride) {
        return 1;
    }

    double explainoMixOverride = 0.0;
    const bool haveExplainoMixOverride = TryParseDoubleArg(args, "--explaino-mix", &explainoMixOverride);
    if (HasArg(args, "--explaino-mix") && !haveExplainoMixOverride) {
        return 1;
    }

    double explainoPhaseOverride = 0.0;
    const bool haveExplainoPhaseOverride = TryParseDoubleArg(args, "--explaino-phase", &explainoPhaseOverride);
    if (HasArg(args, "--explaino-phase") && !haveExplainoPhaseOverride) {
        return 1;
    }

    double explainoWarpOverride = 0.0;
    const bool haveExplainoWarpOverride = TryParseDoubleArg(args, "--explaino-warp-strength", &explainoWarpOverride);
    if (HasArg(args, "--explaino-warp-strength") && !haveExplainoWarpOverride) {
        return 1;
    }

    double explainoSeedDriftOverride = 0.0;
    const bool haveExplainoSeedDriftOverride = TryParseDoubleArg(args, "--explaino-seed-drift", &explainoSeedDriftOverride);
    if (HasArg(args, "--explaino-seed-drift") && !haveExplainoSeedDriftOverride) {
        return 1;
    }

    int widthOverride = 0;
    const bool haveWidthOverride = TryParseIntArg(args, "--width", &widthOverride);
    if (HasArg(args, "--width") && !haveWidthOverride) {
        return 1;
    }

    int heightOverride = 0;
    const bool haveHeightOverride = TryParseIntArg(args, "--height", &heightOverride);
    if (HasArg(args, "--height") && !haveHeightOverride) {
        return 1;
    }

    std::string loadStateSelection;
    const bool haveLoadStateSelection = TryGetArgValue(args, "--load-state-json", &loadStateSelection);
    if (HasArg(args, "--load-state-json") && !haveLoadStateSelection) {
        return 1;
    }

    std::string findingGroupArg;
    const bool haveFindingGroupArg = TryGetArgValue(args, "--finding-group", &findingGroupArg);
    if (HasArg(args, "--finding-group") && !haveFindingGroupArg) {
        return 1;
    }

    std::string findingWhyArg;
    const bool haveFindingWhyArg = TryGetArgValue(args, "--finding-why", &findingWhyArg);
    if (HasArg(args, "--finding-why") && !haveFindingWhyArg) {
        return 1;
    }

    double sweepSeedStart = 0.0;
    const bool haveSweepSeedStart = TryParseDoubleArg(args, "--sweep-seed-start", &sweepSeedStart);
    if (HasArg(args, "--sweep-seed-start") && !haveSweepSeedStart) {
        return 1;
    }

    double sweepSeedStop = 0.0;
    const bool haveSweepSeedStop = TryParseDoubleArg(args, "--sweep-seed-stop", &sweepSeedStop);
    if (HasArg(args, "--sweep-seed-stop") && !haveSweepSeedStop) {
        return 1;
    }

    double sweepSeedStep = 0.0;
    const bool haveSweepSeedStep = TryParseDoubleArg(args, "--sweep-seed-step", &sweepSeedStep);
    if (HasArg(args, "--sweep-seed-step") && !haveSweepSeedStep) {
        return 1;
    }

    int sweepDwellMs = 450;
    const bool haveSweepDwellMs = TryParseIntArg(args, "--sweep-dwell-ms", &sweepDwellMs);
    if (HasArg(args, "--sweep-dwell-ms") && !haveSweepDwellMs) {
        return 1;
    }

    const bool sweepLoop = HasArg(args, "--sweep-loop");
    const bool haveAnySweepArg = haveSweepSeedStart || haveSweepSeedStop || haveSweepSeedStep || haveSweepDwellMs || sweepLoop;
    SweepPlayerConfig sweepConfig{};
    if (haveAnySweepArg) {
        if (!(haveSweepSeedStart && haveSweepSeedStop && haveSweepSeedStep)) {
            return 1;
        }
        sweepConfig.enabled = true;
        sweepConfig.seed_start = sweepSeedStart;
        sweepConfig.seed_stop = sweepSeedStop;
        sweepConfig.seed_step = sweepSeedStep;
        sweepConfig.dwell_seconds = (double)sweepDwellMs / 1000.0;
        sweepConfig.loop = sweepLoop;
    }

    if ((haveWidthOverride && widthOverride <= 0) || (haveHeightOverride && heightOverride <= 0)) {
        return 1;
    }

    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    RenderStats stats{};
    LensSettings lens{};
    bool dirty = true;

    // Schema load policy (short-term unblock):
    // - Prefer exe-relative canonical schema (treat canonical JSON as an artifact)
    // - If missing/invalid while editing, keep the app running with a built-in Safe Mode UI
    std::string schemaPath;
    std::string schemaWarning; // non-empty => Safe Mode UI active
    UISchema uiSchema;
    bool schemaFromFile = false;

    std::string exeDir = GetExeDir();
    std::vector<std::string> schemaCandidates;
    schemaCandidates.push_back(JoinPath(exeDir, "..\\ui\\fractal_binding_surface_v1.ui_schema.canonical.json"));
    schemaCandidates.push_back(JoinPath(exeDir, "ui\\fractal_binding_surface_v1.ui_schema.canonical.json"));
    schemaCandidates.push_back("..\\ui\\fractal_binding_surface_v1.ui_schema.canonical.json");

    for (const auto& cand : schemaCandidates) {
        std::string text = ReadTextFile(cand.c_str());
        if (text.empty()) continue;
        auto pr = json_min::Parse(text);
        if (!pr.error.empty()) {
            schemaPath = cand;
            schemaWarning = "Schema JSON parse error (entering Safe Mode):\n" + cand + "\n" + pr.error;
            break;
        }
        auto lr = LoadUISchemaFromJson(pr.value);
        if (!lr.error.empty()) {
            schemaPath = cand;
            schemaWarning = "Schema decode error (entering Safe Mode):\n" + cand + "\n" + lr.error;
            break;
        }
        uiSchema = std::move(lr.schema);
        schemaPath = cand;
        schemaFromFile = true;
        break;
    }

    if (!schemaFromFile) {
        if (schemaPath.empty()) schemaPath = schemaCandidates[0];
        if (schemaWarning.empty()) {
            schemaWarning = std::string("Schema missing/unreadable (entering Safe Mode). Tried:\n")
                + "  " + schemaCandidates[0] + "\n"
                + "  " + schemaCandidates[1] + "\n"
                + "  " + schemaCandidates[2];
        }
        uiSchema = BuildSafeModeSchema();
    }

    BindingContext initBind;
    initBind.view = &view;
    initBind.params = &params;
    initBind.render = &render;
    initBind.lens = &lens;
    {
        std::string bindError;
        if (!ValidateSchemaBindings(uiSchema, initBind, &bindError)) {
            SchemaStartupFailureResult failure = ResolveSchemaBindingFailure(schemaPath, bindError, validateUiOnly);
            if (!failure.enter_safe_mode) {
                return 1;
            }
            schemaWarning = failure.warning;
            uiSchema = BuildSafeModeSchema();
            std::string safeBindError;
            if (!ValidateSchemaBindings(uiSchema, initBind, &safeBindError)) {
                return 1;
            }
        }
    }

    {
        bool loadedState = false;
        ApplySchemaDefaults(uiSchema, initBind, &dirty);

        // Ensure polynomial coefficients remain coherent if schema sets poly_kind.
        if (params.poly_kind != PolyKind::custom) {
            SetPolyPreset(params);
        }

        if (haveLoadStateSelection) {
            std::string loadError;
            std::string loadedStatePath;
            if (!LoadFindingSelectionIntoRuntime(loadStateSelection, &view, &params, &render, &loadedStatePath, &loadError)) {
                return 1;
            }
            loadedState = true;
            dirty = true;
        }

        if (haveCliFractalType) {
            view.fractal_type = cliFractalType;
            ApplyFractalViewPresetDefaults(view, &dirty);
            dirty = true;
        } else if (haveExplainoSeedOverride) {
            view.fractal_type = FractalType::explaino;
            ApplyFractalViewPresetDefaults(view, &dirty);
            dirty = true;
        } else if (sweepConfig.enabled) {
            view.fractal_type = FractalType::explaino;
            ApplyFractalViewPresetDefaults(view, &dirty);
            dirty = true;
        }

        if (sweepConfig.enabled && !IsExplainoFamily(view.fractal_type)) {
            return 1;
        }

        if (haveWidthOverride) render.resolution.x = widthOverride;
        if (haveHeightOverride) render.resolution.y = heightOverride;
        if (sweepConfig.enabled) view.auto_refresh = true;

        const bool needPresetDerivedFields = !loadedState || haveCliFractalType || haveExplainoSeedOverride || sweepConfig.enabled;
        if (needPresetDerivedFields) {
            ApplyFractalDerivedFieldsAndSyncHp(view, params, &dirty, haveExplainoSeedOverride, explainoSeedOverride);
        } else {
            if (IsExplainoFamily(view.fractal_type)) {
                UpdateExplainoPolynomial(view, params, &dirty);
            }
            SyncViewUiFromHp(view);
        }

        if (haveExplainoSeedOverride) ExplainoSeedSetCombined(view, params, explainoSeedOverride);
        if (haveExplainoPhaseOverride) view.explaino_phase = (float)explainoPhaseOverride;
        if (haveExplainoSeedDriftOverride) view.explaino_seed_drift = (float)explainoSeedDriftOverride;
        if (haveExplainoSeedBOverride) params.explaino_seed_b = explainoSeedBOverride;
        if (haveExplainoMixOverride) params.explaino_mix = (float)explainoMixOverride;
        if (haveExplainoWarpOverride) params.explaino_warp_strength = (float)explainoWarpOverride;
        if (IsExplainoFamily(view.fractal_type)) {
            UpdateExplainoPolynomial(view, params, &dirty);
        }
    }

    if (validateUiOnly) {
        return 0;
    }

    if (captureDiagnosticOnly) {
        std::vector<uint32_t> headlessRgba;
        headlessRgba.resize((size_t)render.resolution.x * (size_t)render.resolution.y);

        const char* err = nullptr;
        RenderStats headlessStats{};
        if (!RenderFractalCUDA(view, params, render, headlessRgba.data(), nullptr, &headlessStats, &err)) {
            return 1;
        }

        std::string captureError;
        DiagnosticsCaptureResult captureResult;
        if (!CaptureDiagnosticsLastBundle(exeDir, view, params, render, headlessStats, headlessRgba.data(), &captureResult, &captureError)) {
            return 1;
        }
        return 0;
    }

    if (captureFindingOnly) {
        ClearHeadlessErrorFile(exeDir, "capture_finding_error.txt");
        std::vector<uint32_t> headlessRgba;
        headlessRgba.resize((size_t)render.resolution.x * (size_t)render.resolution.y);

        const char* err = nullptr;
        RenderStats headlessStats{};
        if (!RenderFractalCUDA(view, params, render, headlessRgba.data(), nullptr, &headlessStats, &err)) {
            WriteHeadlessErrorFile(exeDir, "capture_finding_error.txt", err ? err : "RenderFractalCUDA failed during headless finding capture.");
            return 1;
        }

        std::string findingDir;
        std::string findingError;
        const std::string findingGroup = haveFindingGroupArg ? findingGroupArg : "manual_capture";
        const std::string findingWhy = haveFindingWhyArg ? findingWhyArg : "Headless finding capture.";
        if (!CaptureAndArchiveFindingBundle(exeDir, view, params, render, headlessStats, headlessRgba.data(), findingGroup, findingWhy, &findingDir, &findingError)) {
            WriteHeadlessErrorFile(exeDir, "capture_finding_error.txt", findingError.empty() ? "CaptureAndArchiveFindingBundle failed during headless finding capture." : findingError);
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
    static std::string imguiIniPath;
    imguiIniPath = JoinPath(exeDir, "imgui.ini");
    io.IniFilename = imguiIniPath.c_str();
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    std::vector<uint32_t> rgba;
    std::vector<uint8_t> maskBuffer;
    std::vector<uint32_t> lensSdfRgba;
    rgba.resize((size_t)render.resolution.x * (size_t)render.resolution.y);
    PolyKind lastPolyKind = params.poly_kind;
    FractalType lastFractalType = view.fractal_type;
    std::string findingStatus;
    std::string lastFindingPath; // last captured/loaded path for copy/open buttons
    SweepPlayerState sweepState{};
    bool sweepPaused = false;
    bool sweepSingleStep = false;
    float seedScrubAccel = 0.0f; // acceleration state for arrow-key seed scrubbing
    if (sweepConfig.enabled) {
        std::string sweepError;
        if (!InitializeSweepPlayer(sweepConfig, &sweepState, &sweepError)) {
            CleanupDeviceD3D();
            DestroyWindow(hwnd);
            UnregisterClass(wc.lpszClassName, wc.hInstance);
            return 1;
        }

        double initialSweepSeed = 0.0;
        if (!SweepPlayerCurrentSeed(sweepState, &initialSweepSeed)) {
            CleanupDeviceD3D();
            DestroyWindow(hwnd);
            UnregisterClass(wc.lpszClassName, wc.hInstance);
            return 1;
        }
        ExplainoSeedSetCombined(view, params, initialSweepSeed);
        UpdateExplainoPolynomial(view, params, nullptr);
        dirty = true;
    }

    MSG msg;
    ZeroMemory(&msg, sizeof(msg));

    while (msg.message != WM_QUIT) {
        bool quitRequested = false;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            if (NoteQuitMessage(msg, &quitRequested)) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (ShouldExitUiLoop(quitRequested)) {
            break;
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (sweepConfig.enabled) {
            if (ImGui::IsKeyPressed(ImGuiKey_Space, false)) {
                sweepPaused = !sweepPaused;
            }
            bool sweepDirty = false;
            if (ApplySweepPlayback(sweepConfig, sweepPaused, sweepSingleStep, (double)io.DeltaTime, &sweepState, &view, &params, &sweepDirty)) {
                if (sweepDirty) dirty = true;
            }
            sweepSingleStep = false;
        }

        // Controls window
        ImGui::Begin("Controls");

        if (!schemaWarning.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.2f, 1.0f), "UI Safe Mode (schema issue)");
            ImGui::TextWrapped("%s", schemaWarning.c_str());
            ImGui::TextWrapped("Schema path: %s", schemaPath.c_str());
            ImGui::Separator();
        }

        BindingContext bind;
        bind.view = &view;
        bind.params = &params;
        bind.render = &render;
        bind.lens = &lens;

        bool renderOnceAction = false;
        bool resetViewAction = false;
        bool resetAllAction = false;
        bool loadStateAction = false;
        bool captureFindingAction = false;
        bool captureDiagnosticAction = false;
        bool nextSeedAction = false;
        bool prevSeedAction = false;

        // If schema/UI edits the float surface (center/zoom), keep high-precision state in sync.
        Float2 uiCenterBefore = view.center;
        float uiZoomBefore = view.zoom;

        for (const auto& panel : uiSchema.panels) {
            if (ImGui::CollapsingHeader(panel.label.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                bool prevWasSeedButton = false;
                for (const auto& ctrl : panel.controls) {
                    // Hook extra actions without expanding the schema model yet.
                    if (ctrl.type == "button" && ctrl.has_binding && ctrl.binding.kind == "action") {
                        // Evaluate visibility predicates for action buttons too.
                        if (ctrl.has_visible_if && !bind.EvalVisibleIf(ctrl.visible_if)) {
                            prevWasSeedButton = false;
                            continue;
                        }

                        // Place prev/next seed buttons on the same line.
                        bool isSeedButton = (ctrl.binding.path == "fractal.actions.prev_seed" ||
                                             ctrl.binding.path == "fractal.actions.next_seed");
                        if (isSeedButton && prevWasSeedButton) {
                            ImGui::SameLine();
                        }

                        ImGui::PushID(ctrl.id.c_str());
                        bool pressed = ImGui::Button(ctrl.label.c_str());
                        ImGui::PopID();
                        if (pressed) {
                            if (ctrl.binding.path == "fractal.actions.render_once") renderOnceAction = true;
                            if (ctrl.binding.path == "fractal.actions.reset_view") resetViewAction = true;
                            if (ctrl.binding.path == "fractal.actions.reset_all") resetAllAction = true;
                            if (ctrl.binding.path == "fractal.actions.load_state") loadStateAction = true;
                            if (ctrl.binding.path == "fractal.actions.capture_finding") captureFindingAction = true;
                            if (ctrl.binding.path == "fractal.actions.capture_diagnostic") captureDiagnosticAction = true;
                            if (ctrl.binding.path == "fractal.actions.next_seed") nextSeedAction = true;
                            if (ctrl.binding.path == "fractal.actions.prev_seed") prevSeedAction = true;
                        }
                        prevWasSeedButton = isSeedButton;
                    } else {
                        prevWasSeedButton = false;
                        RenderControlFromSchema(ctrl, bind, &dirty, &renderOnceAction);
                    }
                }
            }
        }

        if (view.center.x != uiCenterBefore.x || view.center.y != uiCenterBefore.y || view.zoom != uiZoomBefore) {
            SyncViewHpFromUi(view);
        }

        if (IsEscapeTimeFamily(view.fractal_type)) {
            ImGui::Spacing();
            ImGui::TextWrapped("Note: escape-time fractals use iteration-based coloring.");
            if (!IsColoringModeAllowedForFractal(view.fractal_type, params.coloring_mode)) {
                ImGui::TextWrapped("Root-basin coloring is for Newton and the Explaino family. Choose 'iteration_count' or 'smooth_escape' for escape-time modes.");
            }
        }

        // Apply per-fractal presets immediately when the user changes fractal type.
        if (view.fractal_type != lastFractalType) {
            lastFractalType = view.fractal_type;
            ApplyFractalViewPresetDefaults(view, &dirty);
            ApplyFractalPresetDefaults(view, params, &dirty);
            if (IsExplainoFamily(view.fractal_type)) {
                UpdateExplainoPolynomial(view, params, nullptr);
            }
            SyncViewHpFromUi(view);
        }

        // Keep presets coherent when schema-driven poly changes happen.
        if (params.poly_kind != lastPolyKind) {
            lastPolyKind = params.poly_kind;
            if (params.poly_kind != PolyKind::custom) {
                SetPolyPreset(params);
                dirty = true;
            }
        }

        ImGui::Separator();
        ImGui::Text("Last render: %.3f ms (benchmark), avg iters ~ %d, device %d", stats.last_render_ms, stats.last_iters_avg, stats.last_device_id);
        if (!findingStatus.empty()) {
            ImGui::TextWrapped("%s", findingStatus.c_str());
            if (!lastFindingPath.empty()) {
                if (ImGui::SmallButton("Copy Path")) {
                    ImGui::SetClipboardText(lastFindingPath.c_str());
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("Open Folder")) {
                    ShellExecuteA(nullptr, "explore", lastFindingPath.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
                }
            }
        }

        if (sweepConfig.enabled) {
            double currentSweepSeed = 0.0;
            if (SweepPlayerCurrentSeed(sweepState, &currentSweepSeed)) {
                ImGui::Text("Sweep: %d/%d  seed %.6f%s",
                    sweepState.current_index + 1,
                    (int)sweepState.seeds.size(),
                    currentSweepSeed,
                    sweepState.finished ? "  [done]" : (sweepPaused ? "  [paused]" : (sweepConfig.loop ? "  [loop]" : "")));
                ImGui::Text("Sweep dwell: %.0f ms", sweepConfig.dwell_seconds * 1000.0);
                ImGui::Text("Combined seed: %.6f", ExplainoSeedCombined(view, params));
                if (ImGui::Button(sweepPaused ? "Resume Sweep" : "Pause Sweep")) {
                    sweepPaused = !sweepPaused;
                }
                ImGui::SameLine();
                if (ImGui::Button("Step Sweep")) {
                    sweepPaused = true;
                    sweepSingleStep = true;
                }
                ImGui::SameLine();
                if (ImGui::Button("Restart Sweep")) {
                    std::string sweepError;
                    if (InitializeSweepPlayer(sweepConfig, &sweepState, &sweepError) && SweepPlayerCurrentSeed(sweepState, &currentSweepSeed)) {
                        ExplainoSeedSetCombined(view, params, currentSweepSeed);
                        UpdateExplainoPolynomial(view, params, nullptr);
                        dirty = true;
                    }
                }
                ImGui::TextUnformatted("Space toggles sweep pause.");
            }
        }

        {
            const double zhp = SafeZoomFromLog2(view.log2_zoom);
            ImGui::Text("HP zoom: 2^(%.3f) ~= %.3e", view.log2_zoom, zhp);
        }

        ImGui::End();

        if (resetViewAction) {
            ApplyFractalViewPresetDefaults(view, &dirty);
            SyncViewHpFromUi(view);
            dirty = true;
        }

        if (resetAllAction) {
            ResetRuntimeStateForCurrentFractal(view, params, render, lens, &dirty);
            lastPolyKind = params.poly_kind;
            lastFractalType = view.fractal_type;
        }

        if (loadStateAction) {
            std::string selectedPath;
            if (PromptOpenFindingStatePath(hwnd, &selectedPath)) {
                std::string resolvedStatePath;
                std::string loadError;
                if (!LoadFindingSelectionIntoRuntime(selectedPath, &view, &params, &render, &resolvedStatePath, &loadError)) {
                    findingStatus = "Load state failed: " + loadError;
                } else {
                    lastPolyKind = params.poly_kind;
                    lastFractalType = view.fractal_type;
                    findingStatus = "Loaded finding state: " + resolvedStatePath;
                    lastFindingPath = resolvedStatePath;
                    dirty = true;
                }
            }
        }

        if (nextSeedAction && IsExplainoFamily(view.fractal_type)) {
            ExplainoSeedSetCombined(view, params, ExplainoSeedCombined(view, params) + 1.0);
            UpdateExplainoPolynomial(view, params, nullptr);
            dirty = true;
        }
        if (prevSeedAction && IsExplainoFamily(view.fractal_type)) {
            ExplainoSeedSetCombined(view, params, ExplainoSeedCombined(view, params) - 1.0);
            UpdateExplainoPolynomial(view, params, nullptr);
            dirty = true;
        }

        // Arrow-key seed scrubber with short acceleration ramp.
        // Holding Left/Right smoothly advances the seed; acceleration builds
        // over ~0.5s from a slow crawl to the configured seed rate.
        if (IsExplainoFamily(view.fractal_type)) {
            bool left = ImGui::IsKeyDown(ImGuiKey_LeftArrow);
            bool right = ImGui::IsKeyDown(ImGuiKey_RightArrow);
            if (left || right) {
                // Ramp from 0 to 1 over ~0.5s (acceleration window)
                seedScrubAccel = fminf(seedScrubAccel + io.DeltaTime * 2.0f, 1.0f);
                // Nonlinear curve: slow start, fast finish (cubic ease-in)
                float t = seedScrubAccel * seedScrubAccel * seedScrubAccel;
                float rate = fmaxf(0.001f, view.explaino_seed_rate);
                double delta = (double)(t * rate) * (double)io.DeltaTime;
                if (left) delta = -delta;
                ExplainoSeedSetCombined(view, params, ExplainoSeedCombined(view, params) + delta);
                UpdateExplainoPolynomial(view, params, nullptr);
                dirty = true;
            } else {
                seedScrubAccel = 0.0f;
            }
        }

        if (captureDiagnosticAction) {
            rgba.resize((size_t)render.resolution.x * (size_t)render.resolution.y);
            std::string captureError;
            DiagnosticsCaptureResult captureResult;
            if (!CaptureDiagnosticsLastBundle(exeDir, view, params, render, stats, rgba.data(), &captureResult, &captureError)) {
                findingStatus = "Capture diagnostic failed: " + captureError;
            } else {
                findingStatus = "Diagnostic captured.";
            }
        }

        if (ApplyExplainoSeedDynamics(stats, io.DeltaTime, view, params)) {
            dirty = true;
        }

        // Render dispatch (fail-fast, delta-independent):
        // - If auto_refresh is ON, we render every frame.
        // - Otherwise, render on explicit request or any state change (dirty).
        if (view.auto_refresh || dirty || renderOnceAction || captureFindingAction) {
            if (IsExplainoFamily(view.fractal_type)) {
                UpdateExplainoPolynomial(view, params, nullptr);
            }
            rgba.resize((size_t)render.resolution.x * (size_t)render.resolution.y);
            EnsureFractalTexture(render.resolution.x, render.resolution.y);

            uint8_t* maskPtr = nullptr;
            if (lens.enabled) {
                maskBuffer.resize((size_t)render.resolution.x * (size_t)render.resolution.y);
                maskPtr = maskBuffer.data();
            }

            const char* err = nullptr;
            RenderStats newStats{};
            if (!RenderFractalCUDA(view, params, render, rgba.data(), maskPtr, &newStats, &err)) {
                // Show error pane
                ImGui::Begin("CUDA Error");
                ImGui::TextWrapped("Render failed: %s", err ? err : "unknown error");
                ImGui::End();
            } else {
                stats = newStats;
                UploadFractalRGBA(rgba.data(), render.resolution.x, render.resolution.y);
                if (lens.enabled && maskPtr) {
                    EnsureMaskTexture(render.resolution.x, render.resolution.y);
                    UploadMaskAsRGBA(maskPtr, render.resolution.x, render.resolution.y);
                    EnsureLensSdfTexture(render.resolution.x, render.resolution.y);
                    ComputeSignedDistanceSdfChamfer(maskPtr, render.resolution.x, render.resolution.y, 48.0f, lensSdfRgba);
                    UploadLensSdfRGBA(lensSdfRgba.data(), render.resolution.x, render.resolution.y);
                }
            }
            dirty = false;
        }

        if (captureFindingAction) {
            std::string findingDir;
            std::string captureError;
            if (!CaptureAndArchiveFindingBundle(exeDir, view, params, render, stats, rgba.data(), "manual_capture", "Manual viewer capture.", &findingDir, &captureError)) {
                findingStatus = "Capture finding failed: " + captureError;
            } else {
                findingStatus = "Captured finding: " + findingDir;
                lastFindingPath = findingDir;
            }
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
                    view.log2_zoom = ClampInteractionLog2Zoom(view.log2_zoom + Log2D(factor));
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

        // Mask window (lens pipeline)
        if (lens.enabled && g_maskSRV) {
            ImGui::Begin("Mask");
            ImVec2 avail = ImGui::GetContentRegionAvail();
            float scale = 1.0f;
            if (render.resolution.x > 0 && render.resolution.y > 0) {
                float sx = avail.x / (float)render.resolution.x;
                float sy = avail.y / (float)render.resolution.y;
                scale = (sx < sy) ? sx : sy;
                if (scale <= 0.0f) scale = 1.0f;
            }
            ImVec2 maskSize((float)render.resolution.x * scale, (float)render.resolution.y * scale);
            ImGui::Image((ImTextureID)g_maskSRV, maskSize);
            ImGui::End();
        }

        if (lens.enabled && g_lensSdfSRV) {
            ImGui::Begin("Lens SDF");
            ImVec2 avail = ImGui::GetContentRegionAvail();
            float scale = 1.0f;
            if (render.resolution.x > 0 && render.resolution.y > 0) {
                float sx = avail.x / (float)render.resolution.x;
                float sy = avail.y / (float)render.resolution.y;
                scale = (sx < sy) ? sx : sy;
                if (scale <= 0.0f) scale = 1.0f;
            }
            ImVec2 sdfSize((float)render.resolution.x * scale, (float)render.resolution.y * scale);
            ImGui::Image((ImTextureID)g_lensSdfSRV, sdfSize);
            ImGui::End();
        }

        // Camera behavior loop (per-frame deltas scaled only by dive_speed; no dt semantics).
        // This runs after UI, so changing behavior updates immediately.
        ApplyAutoDivePerFrame(view, &dirty);

        // Auto-refresh must be enabled for auto-dive to be observable.
        if (view.auto_dive && !view.auto_refresh) {
            view.auto_refresh = true;
            dirty = true;
        }

        // Render frame
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

    CleanupFractalCUDA();
    CleanupDeviceD3D();
    if (IsWindow(hwnd)) {
        DestroyWindow(hwnd);
    }
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}
