#include "fractal_types.h"

#include <Windows.h>
#include <commdlg.h>
#include <d3d11.h>
#include <shellapi.h>
#include <tchar.h>

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"

#include "cli_args.h"
#include "viewer_cli.h"
#include "viewer_schema_load.h"
#include "viewer_state_init.h"
#include "diagnostics_capture.h"
#include "finding_archive_actions.h"
#include "finding_capture_state.h"
#include "finding_state_actions.h"
#include "explaino_seed.h"
#include "explaino_sidecar_cuda_sample_host.h"
#include "explaino_sidecar_window.h"
#include "explaino_seed_dynamics.h"
#include "param_anim_dynamics.h"
#include "fractal_derived_fields.h"
#include "fractal_family_rules.h"
#include "fractal_probe_contract.h"
#include "fractal_probe_runner.h"
#include "function_descriptor.h"
#include "headless_modes.h"
#include "json_min.h"
#include "lens_sdf.h"
#include "render_capture_guard.h"
#include "runtime_reset.h"
#include "safe_mode_schema.h"
#include "schema_binding.h"
#include "schema_startup_policy.h"
#include "sweep_player.h"
#include "ui_schema.h"
#include "viewer_render_pacing.h"
#include "viewer_shutdown.h"
#include "viewer_sweep.h"
#include "viewport_interaction.h"
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

// Tracks the viewport's available pixel dimensions for adaptive-resolution settle.
static Int2 g_viewportPixels{0, 0};

static void CleanupRenderTarget() {
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

static std::string GetExePath() {
    char buf[MAX_PATH] = {};
    DWORD n = GetModuleFileNameA(nullptr, buf, (DWORD)sizeof(buf));
    if (n == 0 || n >= sizeof(buf)) return std::string();
    return std::string(buf);
}

static std::string GetExeDir() {
    std::string path = GetExePath();
    if (path.empty()) return std::string();
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

// --- Headless capture helpers ---

static int RunHeadlessDiagnosticCapture(
    const std::string& exeDir, const ViewState& view,
    const KernelParams& params, const RenderSettings& render) {
    std::vector<uint32_t> headlessRgba;
    headlessRgba.resize((size_t)render.resolution.x * (size_t)render.resolution.y);
    const char* err = nullptr;
    RenderStats headlessStats{};
    if (!RenderFractalCUDA(view, params, render, headlessRgba.data(), nullptr, &headlessStats, &err)) {
        return 1;
    }
    std::string captureError;
    DiagnosticsCaptureResult captureResult;
    if (!CaptureDiagnosticsLastBundle(exeDir, view, params, render, headlessStats,
            headlessRgba.data(), headlessRgba.size(), &captureResult, &captureError)) {
        return 1;
    }
    return 0;
}

static int RunHeadlessFindingCapture(
    const std::string& exeDir, const ViewerCliArgs& cli,
    const ViewState& view, const KernelParams& params, const RenderSettings& render) {
    ClearHeadlessErrorFile(exeDir, "capture_finding_error.txt");
    RenderSettings findingRender = BuildFindingArchiveCaptureRender(render);
    std::vector<uint32_t> headlessRgba;
    headlessRgba.resize((size_t)findingRender.resolution.x * (size_t)findingRender.resolution.y);
    const char* err = nullptr;
    RenderStats headlessStats{};
    if (!RenderFractalCUDA(view, params, findingRender, headlessRgba.data(), nullptr, &headlessStats, &err)) {
        WriteHeadlessErrorFile(exeDir, "capture_finding_error.txt",
            err ? err : "RenderFractalCUDA failed during headless finding capture.");
        return 1;
    }
    std::string findingDir;
    std::string findingError;
    const std::string findingGroup = cli.have_finding_group ? cli.finding_group : "manual_capture";
    const std::string findingWhy = cli.have_finding_why ? cli.finding_why : "Headless finding capture.";
    if (!CaptureAndArchiveFindingBundle(exeDir, view, params, findingRender, headlessStats,
            headlessRgba.data(), headlessRgba.size(), findingGroup, findingWhy, &findingDir, &findingError)) {
        WriteHeadlessErrorFile(exeDir, "capture_finding_error.txt",
            findingError.empty() ? "CaptureAndArchiveFindingBundle failed during headless finding capture." : findingError);
        return 1;
    }
    return 0;
}

// --- In-loop capture helpers ---

static void RunInLoopDiagnosticCapture(
    const std::string& exeDir, const ViewState& view, const KernelParams& params,
    const RenderSettings& render, const RenderStats& stats,
    const std::vector<uint32_t>& rgba, const RenderedFrameState& renderedFrame,
    std::string& findingStatus) {
    std::string captureError;
    RenderSettings captureRender = render;
    captureRender.resolution = {renderedFrame.width, renderedFrame.height};
    if (!CanCaptureRenderedFrame(captureRender, rgba.size(), renderedFrame, &captureError)) {
        findingStatus = "Capture diagnostic failed: " + captureError;
    } else {
        DiagnosticsCaptureResult captureResult;
        if (!CaptureDiagnosticsLastBundle(exeDir, view, params, captureRender, stats,
                rgba.data(), rgba.size(), &captureResult, &captureError)) {
            findingStatus = "Capture diagnostic failed: " + captureError;
        } else {
            findingStatus = "Diagnostic captured.";
        }
    }
}

static bool RunInLoopFindingCapture(
    const std::string& exeDir, ViewState& view, KernelParams& params,
    const RenderSettings& render, std::string& findingStatus, std::string& lastFindingPath) {
    std::string findingDir;
    std::string captureError;
    const bool invalidateCaches = PrepareFindingCaptureRuntimeState(view, params);
    RenderSettings findingRender = BuildFindingArchiveCaptureRender(render);
    std::vector<uint32_t> findingRgba;
    findingRgba.resize((size_t)findingRender.resolution.x * (size_t)findingRender.resolution.y);
    const char* err = nullptr;
    RenderStats findingStats{};
    if (!RenderFractalCUDA(view, params, findingRender, findingRgba.data(), nullptr, &findingStats, &err)) {
        findingStatus = std::string("Capture finding failed: ") + (err ? err : "unknown error");
        return invalidateCaches;
    } else if (!CaptureAndArchiveFindingBundle(exeDir, view, params, findingRender, findingStats,
            findingRgba.data(), findingRgba.size(), "manual_capture", "Manual viewer capture.",
            &findingDir, &captureError)) {
        findingStatus = "Capture finding failed: " + captureError;
    } else {
        findingStatus = "Captured finding: " + findingDir;
        lastFindingPath = findingDir;
    }
    return invalidateCaches;
}

// --- Image window helpers ---

static void RenderFractalViewport(
    const ImGuiIO& io, const RenderSettings& render,
    ViewState& view, bool& dirty, bool& interactionChanged) {
    ImGui::Begin("Fractal");
    if (g_fractalSRV) {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        // Track viewport pixel dimensions for adaptive-resolution settle.
        if (avail.x >= 64.0f && avail.y >= 64.0f) {
            g_viewportPixels = {(int)avail.x, (int)avail.y};
        }
        float scale = 1.0f;
        if (render.resolution.x > 0 && render.resolution.y > 0) {
            float sx = avail.x / (float)render.resolution.x;
            float sy = avail.y / (float)render.resolution.y;
            scale = (sx < sy) ? sx : sy;
            if (scale <= 0.0f) scale = 1.0f;
        }
        ImVec2 size((float)render.resolution.x * scale, (float)render.resolution.y * scale);

        ImGui::InvisibleButton("##viewport", size, ImGuiButtonFlags_MouseButtonLeft);
        ImVec2 rectMin = ImGui::GetItemRectMin();
        ImVec2 rectMax = ImGui::GetItemRectMax();
        ImGui::GetWindowDrawList()->AddImage((ImTextureID)g_fractalSRV, rectMin, rectMax);

        bool hovered = ImGui::IsItemHovered();
        bool active = ImGui::IsItemActive();

        if (hovered) {
            if (io.MouseWheel != 0.0f && size.x > 0.0f && size.y > 0.0f) {
                ImVec2 mp = ImGui::GetMousePos();
                float u = ClampF((mp.x - rectMin.x) / size.x, 0.0f, 1.0f);
                float v = ClampF((mp.y - rectMin.y) / size.y, 0.0f, 1.0f);
                double aspect = (render.resolution.y > 0) ? (double)render.resolution.x / (double)render.resolution.y : 1.0;

                auto zr = ComputeZoomAroundCursor(
                    view.center_hp_x, view.center_hp_y, view.log2_zoom,
                    u, v, io.MouseWheel, aspect);
                view.center_hp_x = zr.new_center_hp_x;
                view.center_hp_y = zr.new_center_hp_y;
                view.log2_zoom = zr.new_log2_zoom;
                SyncViewUiFromHp(view);
                dirty = true;
                interactionChanged = true;
            }

            if (active && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && render.resolution.x > 0 && render.resolution.y > 0) {
                ImVec2 dpx = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
                ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);

                if (ApplyDragPanStep(view, dpx.x, dpx.y, render.resolution.x, render.resolution.y)) {
                    dirty = true;
                    interactionChanged = true;
                }
            }
        }
    } else {
        ImGui::TextUnformatted("No texture yet. Enable Continuous Render or click Render Once.");
    }
    ImGui::End();
}

static void RenderAuxImageWindow(const char* title, ID3D11ShaderResourceView* srv, const RenderedFrameState& frame) {
    if (!srv || !frame.ready) return;
    int texW = frame.width;
    int texH = frame.height;
    ImGui::Begin(title);
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float scale = 1.0f;
    if (texW > 0 && texH > 0) {
        float sx = avail.x / (float)texW;
        float sy = avail.y / (float)texH;
        scale = (sx < sy) ? sx : sy;
        if (scale <= 0.0f) scale = 1.0f;
    }
    ImVec2 size((float)texW * scale, (float)texH * scale);
    ImGui::Image((ImTextureID)srv, size);
    ImGui::End();
}

// --- Render dispatch helper ---

static void DispatchRenderFrame(
    ViewState& view, KernelParams& params, const RenderSettings& render,
    const LensSettings& lens, const ViewerRenderPacingDecision& renderPacing,
    bool forceFullQuality, bool autoRefresh, bool dirty,
    bool renderOnce, bool captureDiag, bool captureFinding,
    std::vector<uint32_t>& rgba, std::vector<uint8_t>& maskBuffer,
    std::vector<uint32_t>& lensSdfRgba,
    RenderedFrameState& renderedFrame, RenderStats& stats, bool& dirtyOut) {

    if (!ShouldDispatchRender(autoRefresh, dirty, renderOnce, captureDiag, captureFinding, renderPacing.full_quality_due))
        return;

    InvalidateRenderedFrame(&renderedFrame);
    if (IsExplainoFamily(view.fractal_type)) {
        UpdateExplainoPolynomial(view, params, nullptr);
    }
    if (view.auto_max_iter) {
        params.max_iter = ComputeAutoMaxIter(view.log2_zoom, view.fractal_type);
    }
    RenderSettings dispatchRender = render;
    if (renderPacing.preview_active && !forceFullQuality) {
        dispatchRender.resolution = renderPacing.render_resolution;
    } else if (forceFullQuality && g_viewportPixels.x >= 64 && g_viewportPixels.y >= 64) {
        // Settle to viewport pixel dimensions for full-quality render.
        dispatchRender.resolution = g_viewportPixels;
    }
    rgba.resize((size_t)dispatchRender.resolution.x * (size_t)dispatchRender.resolution.y);
    EnsureFractalTexture(dispatchRender.resolution.x, dispatchRender.resolution.y);

    uint8_t* maskPtr = nullptr;
    if (lens.enabled) {
        maskBuffer.resize((size_t)dispatchRender.resolution.x * (size_t)dispatchRender.resolution.y);
        maskPtr = maskBuffer.data();
    }

    const char* err = nullptr;
    RenderStats newStats{};
    if (!RenderFractalCUDA(view, params, dispatchRender, rgba.data(), maskPtr, &newStats, &err)) {
        ImGui::Begin("CUDA Error");
        ImGui::TextWrapped("Render failed: %s", err ? err : "unknown error");
        ImGui::End();
    } else {
        stats = newStats;
        MarkRenderedFrameReady(dispatchRender, &renderedFrame);
        UploadFractalRGBA(rgba.data(), dispatchRender.resolution.x, dispatchRender.resolution.y);
        if (lens.enabled && maskPtr) {
            EnsureMaskTexture(dispatchRender.resolution.x, dispatchRender.resolution.y);
            UploadMaskAsRGBA(maskPtr, dispatchRender.resolution.x, dispatchRender.resolution.y);
            EnsureLensSdfTexture(dispatchRender.resolution.x, dispatchRender.resolution.y);
            ComputeSignedDistanceSdfChamfer(maskPtr, dispatchRender.resolution.x, dispatchRender.resolution.y, 48.0f, lensSdfRgba);
            UploadLensSdfRGBA(lensSdfRgba.data(), dispatchRender.resolution.x, dispatchRender.resolution.y);
        }
    }
    dirtyOut = !renderedFrame.ready;
}

static void ApplyFractalTypeAndPolyCoherence(ViewState& view, KernelParams& params, bool& dirty,
                                              FractalType& lastFractalType, PolyKind& lastPolyKind) {
    if (view.fractal_type != lastFractalType) {
        lastFractalType = view.fractal_type;
        ApplyFractalViewPresetDefaults(view, &dirty);
        ApplyFractalPresetDefaults(view, params, &dirty);
        if (IsExplainoFamily(view.fractal_type)) {
            UpdateExplainoPolynomial(view, params, nullptr);
        }
        SyncViewHpFromUi(view);
    }
    if (params.poly_kind != lastPolyKind) {
        lastPolyKind = params.poly_kind;
        if (params.poly_kind != PolyKind::custom) {
            SetPolyPreset(params);
            dirty = true;
        }
    }
}

static bool InitializeSweepIfEnabled(const SweepPlayerConfig& config, SweepPlayerState& sweepState,
                                     ViewState& view, KernelParams& params, bool& dirty) {
    if (!config.enabled) return true;
    std::string sweepError;
    if (!InitializeSweepPlayer(config, &sweepState, &sweepError)) return false;
    double initialSweepSeed = 0.0;
    if (!SweepPlayerCurrentSeed(sweepState, &initialSweepSeed)) return false;
    ExplainoSeedSetCombined(view, params, initialSweepSeed);
    UpdateExplainoPolynomial(view, params, nullptr);
    dirty = true;
    return true;
}

struct UiActionFlags {
    bool renderOnce = false;
    bool resetView = false;
    bool resetAll = false;
    bool loadState = false;
    bool captureFinding = false;
    bool captureDiagnostic = false;
    bool nextSeed = false;
    bool prevSeed = false;
    bool interactionChanged = false;
};

static UiActionFlags RenderSchemaPanels(const UISchema& schema, BindingContext& bind, bool& dirty) {
    UiActionFlags a;
    for (const auto& panel : schema.panels) {
        if (ImGui::CollapsingHeader(panel.label.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            bool prevWasSeedButton = false;
            for (const auto& ctrl : panel.controls) {
                if (ctrl.type == "button" && ctrl.has_binding && ctrl.binding.kind == "action") {
                    if (ctrl.has_visible_if && !bind.EvalVisibleIf(ctrl.visible_if)) {
                        prevWasSeedButton = false;
                        continue;
                    }
                    bool isSeedButton = (ctrl.binding.path == "fractal.actions.prev_seed" ||
                                         ctrl.binding.path == "fractal.actions.next_seed");
                    if (isSeedButton && prevWasSeedButton) {
                        ImGui::SameLine();
                    }
                    ImGui::PushID(ctrl.id.c_str());
                    bool pressed = ImGui::Button(ctrl.label.c_str());
                    ImGui::PopID();
                    if (pressed) {
                        if (ctrl.binding.path == "fractal.actions.render_once") a.renderOnce = true;
                        if (ctrl.binding.path == "fractal.actions.reset_view") a.resetView = true;
                        if (ctrl.binding.path == "fractal.actions.reset_all") a.resetAll = true;
                        if (ctrl.binding.path == "fractal.actions.load_state") a.loadState = true;
                        if (ctrl.binding.path == "fractal.actions.capture_finding") a.captureFinding = true;
                        if (ctrl.binding.path == "fractal.actions.capture_diagnostic") a.captureDiagnostic = true;
                        if (ctrl.binding.path == "fractal.actions.next_seed") a.nextSeed = true;
                        if (ctrl.binding.path == "fractal.actions.prev_seed") a.prevSeed = true;
                    }
                    prevWasSeedButton = isSeedButton;
                } else {
                    prevWasSeedButton = false;
                    bool controlInteracted = false;
                    RenderControlFromSchema(ctrl, bind, &dirty, &a.renderOnce, &controlInteracted);
                    if (controlInteracted) {
                        a.interactionChanged = true;
                    }
                }
            }
        }
    }
    return a;
}

// Forward declarations for functions called by RenderControlsWindow
static void RenderStatusPanel(const RenderStats& stats, const RenderSettings& render,
                              const RenderedFrameState& renderedFrame, const ViewState& view,
                              const std::string& findingStatus, const std::string& lastFindingPath);
static void RenderSweepControls(const SweepPlayerConfig& config, SweepPlayerState& sweepState,
                                bool& sweepPaused, bool& sweepSingleStep,
                                ViewState& view, KernelParams& params, bool& dirty);

static UiActionFlags RenderControlsWindow(
        const UISchema& schema, const std::string& schemaWarning, const std::string& schemaPath,
        ViewState& view, KernelParams& params, RenderSettings& render, LensSettings& lens,
        const RenderStats& stats, const RenderedFrameState& renderedFrame,
        const std::string& findingStatus, const std::string& lastFindingPath,
        const SweepPlayerConfig& sweepConfig, SweepPlayerState& sweepState,
        bool& sweepPaused, bool& sweepSingleStep,
        FractalType& lastFractalType, PolyKind& lastPolyKind, bool& dirty) {
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
    Float2 uiCenterBefore = view.center;
    float uiZoomBefore = view.zoom;
    UiActionFlags actions = RenderSchemaPanels(schema, bind, dirty);
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
    ApplyFractalTypeAndPolyCoherence(view, params, dirty, lastFractalType, lastPolyKind);
    RenderStatusPanel(stats, render, renderedFrame, view, findingStatus, lastFindingPath);
    RenderSweepControls(sweepConfig, sweepState, sweepPaused, sweepSingleStep, view, params, dirty);
    ImGui::End();
    return actions;
}

static void RenderStatusPanel(const RenderStats& stats, const RenderSettings& render,
                              const RenderedFrameState& renderedFrame, const ViewState& view,
                              const std::string& findingStatus, const std::string& lastFindingPath) {
    ImGui::Separator();
    ImGui::Text("Last render: %.3f ms (benchmark), avg iters ~ %d, device %d", stats.last_render_ms, stats.last_iters_avg, stats.last_device_id);
    ImGui::Text("Target render: %d x %d", render.resolution.x, render.resolution.y);
    {
        int liveWidth = renderedFrame.ready ? renderedFrame.width : render.resolution.x;
        int liveHeight = renderedFrame.ready ? renderedFrame.height : render.resolution.y;
        const int liveWidthMax = (render.resolution.x > liveWidth) ? render.resolution.x : liveWidth;
        const int liveHeightMax = (render.resolution.y > liveHeight) ? render.resolution.y : liveHeight;
        ImGui::BeginDisabled();
        ImGui::SliderInt("Live Width", &liveWidth, 64, liveWidthMax);
        ImGui::SliderInt("Live Height", &liveHeight, 64, liveHeightMax);
        ImGui::EndDisabled();
    }
    if (renderedFrame.ready && (renderedFrame.width != render.resolution.x || renderedFrame.height != render.resolution.y)) {
        ImGui::Text("Interactive preview: %d x %d -> settle to %d x %d",
            renderedFrame.width, renderedFrame.height, render.resolution.x, render.resolution.y);
    }
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
    const double zhp = SafeZoomFromLog2(view.log2_zoom);
    ImGui::Text("HP zoom: 2^(%.3f) ~= %.3e", view.log2_zoom, zhp);
    const char* backendStr = (stats.resolved_eval.backend == NumericBackend::float64) ? "float64" : "float32";
    const char* tierLabel = (render.sample_tier == SampleTier::tier_auto) ? "auto" :
                            (render.sample_tier == SampleTier::fast) ? "fast" : "standard";
    ImGui::Text("Precision: %s -> %s", tierLabel, backendStr);
}

static void RenderSweepControls(const SweepPlayerConfig& config, SweepPlayerState& sweepState,
                                bool& sweepPaused, bool& sweepSingleStep,
                                ViewState& view, KernelParams& params, bool& dirty) {
    if (!config.enabled) return;
    double currentSweepSeed = 0.0;
    if (!SweepPlayerCurrentSeed(sweepState, &currentSweepSeed)) return;
    ImGui::Text("Sweep: %d/%d  seed %.6f%s",
        sweepState.current_index + 1,
        (int)sweepState.seeds.size(),
        currentSweepSeed,
        sweepState.finished ? "  [done]" : (sweepPaused ? "  [paused]" : (config.loop ? "  [loop]" : "")));
    ImGui::Text("Sweep dwell: %.0f ms", config.dwell_seconds * 1000.0);
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
        if (InitializeSweepPlayer(config, &sweepState, &sweepError) && SweepPlayerCurrentSeed(sweepState, &currentSweepSeed)) {
            ExplainoSeedSetCombined(view, params, currentSweepSeed);
            UpdateExplainoPolynomial(view, params, nullptr);
            dirty = true;
        }
    }
    ImGui::TextUnformatted("Space toggles sweep pause.");
}

static void DispatchUiActions(HWND hwnd,
                              bool resetViewAction, bool resetAllAction, bool loadStateAction,
                              bool nextSeedAction, bool prevSeedAction,
                              ViewState& view, KernelParams& params, RenderSettings& render, LensSettings& lens,
                              bool& dirty, bool& interactionChanged,
                              PolyKind& lastPolyKind, FractalType& lastFractalType,
                              std::string& findingStatus, std::string& lastFindingPath) {
    if (resetViewAction) {
        ApplyFractalViewPresetDefaults(view, &dirty);
        SyncViewHpFromUi(view);
        dirty = true;
        interactionChanged = true;
    }
    if (resetAllAction) {
        ResetRuntimeStateForCurrentFractal(view, params, render, lens, &dirty);
        lastPolyKind = params.poly_kind;
        lastFractalType = view.fractal_type;
        interactionChanged = true;
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
                interactionChanged = true;
            }
        }
    }
    if (nextSeedAction && IsExplainoFamily(view.fractal_type)) {
        ExplainoSeedSetCombined(view, params, ExplainoSeedCombined(view, params) + 1.0);
        UpdateExplainoPolynomial(view, params, nullptr);
        dirty = true;
        interactionChanged = true;
    }
    if (prevSeedAction && IsExplainoFamily(view.fractal_type)) {
        ExplainoSeedSetCombined(view, params, ExplainoSeedCombined(view, params) - 1.0);
        UpdateExplainoPolynomial(view, params, nullptr);
        dirty = true;
        interactionChanged = true;
    }
}

static void ApplyArrowKeySeedScrub(const ImGuiIO& io, ViewState& view, KernelParams& params,
                                    float& seedScrubAccel, bool& dirty, bool& interactionChanged) {
    if (!IsExplainoFamily(view.fractal_type)) return;
    bool left = ImGui::IsKeyDown(ImGuiKey_LeftArrow);
    bool right = ImGui::IsKeyDown(ImGuiKey_RightArrow);
    if (left || right) {
        seedScrubAccel = fminf(seedScrubAccel + io.DeltaTime * 2.0f, 1.0f);
        float t = seedScrubAccel * seedScrubAccel * seedScrubAccel;
        bool shift = ImGui::IsKeyDown(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_RightShift);
        float rate = shift ? 1.0f : fmaxf(0.0001f, view.explaino_seed_rate);
        double delta = (double)(t * rate) * (double)io.DeltaTime;
        if (left) delta = -delta;
        ExplainoSeedSetCombined(view, params, ExplainoSeedCombined(view, params) + delta);
        UpdateExplainoPolynomial(view, params, nullptr);
        dirty = true;
        interactionChanged = true;
    } else {
        seedScrubAccel = 0.0f;
    }
}

static void ApplySweepPlaybackPerFrame(const SweepPlayerConfig& config, float deltaTime,
                                       bool& sweepPaused, bool& sweepSingleStep,
                                       SweepPlayerState& sweepState,
                                       ViewState& view, KernelParams& params, bool& dirty) {
    if (!config.enabled) return;
    if (ImGui::IsKeyPressed(ImGuiKey_Space, false)) {
        sweepPaused = !sweepPaused;
    }
    bool sweepDirty = false;
    if (ApplySweepPlayback(config, sweepPaused, sweepSingleStep, (double)deltaTime, &sweepState, &view, &params, &sweepDirty)) {
        if (sweepDirty) dirty = true;
    }
    sweepSingleStep = false;
}

static bool ValidateCliConflicts(const ViewerCliArgs& cli) {
    if (cli.capture_diagnostic_only && cli.capture_finding_only) return false;
    if (cli.validate_ui_only && (cli.capture_diagnostic_only || cli.capture_finding_only)) return false;
    return true;
}

static int TryDispatchHeadlessMode(const ViewerCliArgs& cli, const std::string& exeDir,
                                    ViewState& view, KernelParams& params, RenderSettings& render, bool& dirty) {
    if (cli.capture_diagnostic_only || cli.capture_finding_only) {
        if (IsExplainoFamily(view.fractal_type)) {
            UpdateExplainoPolynomial(view, params, &dirty);
        }
        if (view.auto_max_iter) {
            params.max_iter = ComputeAutoMaxIter(view.log2_zoom, view.fractal_type);
        }
    }
    if (cli.validate_ui_only) return 0;
    if (cli.capture_diagnostic_only) return RunHeadlessDiagnosticCapture(exeDir, view, params, render);
    if (cli.capture_finding_only) return RunHeadlessFindingCapture(exeDir, cli, view, params, render);
    return -1;
}

static void ShutdownViewer(HWND hwnd, const WNDCLASSEX& wc) {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupFractalCUDA();
    CleanupDeviceD3D();
    if (IsWindow(hwnd)) {
        DestroyWindow(hwnd);
    }
    UnregisterClass(wc.lpszClassName, wc.hInstance);
}

static void PresentFrame() {
    ImGui::Render();
    const float clear_color_with_alpha[4] = {0.08f, 0.08f, 0.10f, 1.00f};
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    g_pSwapChain->Present(1, 0);
}

static SampleModeArgs BuildSampleModeArgs(const ViewerCliArgs& cli) {
    SampleModeArgs sma;
    sma.request_stdin = cli.sample_request_stdin;
    sma.response_stdout = cli.sample_response_stdout;
    sma.request_json_path = cli.have_sample_request_json ? cli.sample_request_json_path : std::string();
    sma.response_json_path = cli.have_sample_response_json ? cli.sample_response_json_path : std::string();
    sma.conflict_validate_ui = cli.validate_ui_only;
    sma.conflict_capture_diagnostic = cli.capture_diagnostic_only;
    sma.conflict_capture_finding = cli.capture_finding_only;
    return sma;
}

static int RunSampleSessionMode(const ViewerCliArgs& cli, const std::string& exePath) {
    if (cli.have_sample_session_pipe) {
        return RunNamedPipeSessionMode(cli.sample_session_pipe_name, exePath);
    }
    return RunSessionMode(std::cin, std::cout, exePath);
}

static int TryDispatchCommandLineModes(const ViewerCliArgs& cli, const std::string& exePath,
                                       const std::string& exeDir) {
    if (cli.sample_session) {
        if (cli.any_sample_mode_arg || cli.describe_functions || cli.have_describe_functions_json ||
            cli.validate_ui_only || cli.capture_diagnostic_only || cli.capture_finding_only) {
            std::fprintf(stderr, "--sample-session is mutually exclusive with other headless verbs\n");
            return 1;
        }
        return RunSampleSessionMode(cli, exePath);
    }

    if (cli.any_sample_mode_arg) {
        return RunSampleMode(BuildSampleModeArgs(cli), exePath);
    }

    if (cli.describe_functions || cli.have_describe_functions_json) {
        if (cli.validate_ui_only || cli.capture_diagnostic_only || cli.capture_finding_only || cli.any_sample_mode_arg) {
            std::fprintf(stderr, "--describe-functions is mutually exclusive with other headless verbs\n");
            return 1;
        }
        return RunDescribeFunctionsMode(cli.describe_functions,
            cli.have_describe_functions_json ? cli.describe_functions_json_path : std::string(),
            BuildViewerSchemaCandidates(exeDir));
    }

    return -1;
}

static int InitializeViewerSchemaAndDefaults(const ViewerCliArgs& cli,
                                             const std::vector<std::string>& schemaCandidates,
                                             ViewState& view, KernelParams& params,
                                             RenderSettings& render, LensSettings& lens,
                                             bool& dirty, UISchema& uiSchema,
                                             std::string& schemaPath,
                                             std::string& schemaWarning,
                                             EngineFunctionCatalog& engineCatalog) {
    BindingContext initBind;
    initBind.view = &view;
    initBind.params = &params;
    initBind.render = &render;
    initBind.lens = &lens;

    SchemaLoadResult schemaResult = LoadAndValidateViewerSchema(schemaCandidates, initBind, cli.validate_ui_only);
    if (schemaResult.fatal_error) return 1;

    uiSchema = std::move(schemaResult.schema);
    schemaPath = std::move(schemaResult.path);
    schemaWarning = std::move(schemaResult.warning);
    engineCatalog = BuildEngineCatalog(uiSchema);

    ApplySchemaDefaults(uiSchema, initBind, &dirty);
    if (params.poly_kind != PolyKind::custom) {
        SetPolyPreset(params);
    }
    return ApplyCliOverrides(cli, view, params, render, &dirty);
}

static bool InitializeViewerWindowAndImGui(HINSTANCE hInstance, const std::string& exeDir,
                                           HWND* outHwnd, WNDCLASSEX* outWindowClass) {
    WNDCLASSEX wc = {sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, hInstance, nullptr, nullptr, nullptr, nullptr, _T("FractalUI"), nullptr};
    RegisterClassEx(&wc);

    HWND hwnd = CreateWindow(wc.lpszClassName, _T("CUDA Newton Fractal Explorer"),
        WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd)) {
        CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return false;
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

    if (outHwnd) *outHwnd = hwnd;
    if (outWindowClass) *outWindowClass = wc;
    return true;
}

static void RefreshSidecarStateIfNeeded(bool dirty, ViewState& view, KernelParams& params,
                                        const EngineFunctionCatalog& engineCatalog,
                                        const BindingContext& bind,
                                        CudaSidecarMeasurementHost& sidecarMeasurementHost,
                                        ExplainoSidecarWindowState& sidecarState,
                                        bool& sidecarStateValid,
                                        SidecarBudgetState& sidecarBudgetState,
                                        bool& sidecarBudgetStateValid) {
    if (!dirty && sidecarStateValid) return;

    if (IsExplainoFamily(view.fractal_type)) {
        UpdateExplainoPolynomial(view, params, nullptr);
    }
    if (view.auto_max_iter) {
        params.max_iter = ComputeAutoMaxIter(view.log2_zoom, view.fractal_type);
    }

    BuildExplainoSidecarWindowState(engineCatalog,
        bind,
        &sidecarMeasurementHost,
        sidecarBudgetStateValid ? &sidecarBudgetState : nullptr,
        &sidecarState,
        nullptr);
    if (!sidecarState.budget.function_id.empty()) {
        sidecarBudgetState = sidecarState.budget;
        sidecarBudgetStateValid = true;
    } else {
        sidecarBudgetState = {};
        sidecarBudgetStateValid = false;
    }
    sidecarStateValid = true;
}

static void RunPendingInLoopCaptures(const std::string& exeDir, const UiActionFlags& actions,
                                     ViewState& view, KernelParams& params,
                                     const RenderSettings& render, const RenderStats& stats,
                                     const std::vector<uint32_t>& rgba,
                                     const RenderedFrameState& renderedFrame,
                                     std::string& findingStatus,
                                     std::string& lastFindingPath,
                                     bool& sidecarStateValid,
                                     bool& sidecarBudgetStateValid) {
    if (actions.captureDiagnostic) {
        RunInLoopDiagnosticCapture(exeDir, view, params, render, stats, rgba, renderedFrame, findingStatus);
    }

    if (actions.captureFinding) {
        if (RunInLoopFindingCapture(exeDir, view, params, render, findingStatus, lastFindingPath)) {
            sidecarStateValid = false;
            sidecarBudgetStateValid = false;
        }
    }
}

static void ApplyAutoDivePerFrame(ViewState& view, bool* ioDirty) {
    if (ApplyAutoDiveStep(view)) {
        if (ioDirty) *ioDirty = true;
    }
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
    std::vector<std::string> args = GetCommandLineArgsUtf8();
    ViewerCliArgs cli{};
    { int rc = ParseViewerCli(args, &cli); if (rc != 0) return rc; }
    const std::string exePath = GetExePath();

    std::string exeDir = GetExeDir();
    { int headlessRc = TryDispatchCommandLineModes(cli, exePath, exeDir); if (headlessRc >= 0) return headlessRc; }

    if (!ValidateCliConflicts(cli)) return 1;

    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    RenderStats stats{};
    LensSettings lens{};
    bool dirty = true;

    std::vector<std::string> schemaCandidates = BuildViewerSchemaCandidates(exeDir);
    UISchema uiSchema;
    std::string schemaPath;
    std::string schemaWarning;
    EngineFunctionCatalog engineCatalog;
    CudaSidecarMeasurementHost sidecarMeasurementHost;
    ExplainoSidecarWindowState sidecarState;
    bool sidecarStateValid = false;
    SidecarBudgetState sidecarBudgetState;
    bool sidecarBudgetStateValid = false;

    { int initRc = InitializeViewerSchemaAndDefaults(cli, schemaCandidates, view, params, render, lens,
          dirty, uiSchema, schemaPath, schemaWarning, engineCatalog);
      if (initRc != 0) return initRc; }

    { int headless = TryDispatchHeadlessMode(cli, exeDir, view, params, render, dirty);
      if (headless >= 0) return headless; }

    HWND hwnd = nullptr;
    WNDCLASSEX wc{};
    if (!InitializeViewerWindowAndImGui(hInstance, exeDir, &hwnd, &wc)) {
        return 1;
    }

    ImGuiIO& io = ImGui::GetIO();
    BindingContext bind;
    bind.view = &view;
    bind.params = &params;
    bind.render = &render;
    bind.lens = &lens;

    std::vector<uint32_t> rgba;
    std::vector<uint8_t> maskBuffer;
    std::vector<uint32_t> lensSdfRgba;
    RenderedFrameState renderedFrame{};
    ViewerRenderPacingState renderPacingState{};
    rgba.resize((size_t)render.resolution.x * (size_t)render.resolution.y);
    PolyKind lastPolyKind = params.poly_kind;
    FractalType lastFractalType = view.fractal_type;
    std::string findingStatus;
    std::string lastFindingPath; // last captured/loaded path for copy/open buttons
    SweepPlayerState sweepState{};
    bool sweepPaused = false;
    bool sweepSingleStep = false;
    float seedScrubAccel = 0.0f; // acceleration state for arrow-key seed scrubbing
    if (!InitializeSweepIfEnabled(cli.sweep_config, sweepState, view, params, dirty)) {
        CleanupDeviceD3D();
        DestroyWindow(hwnd);
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
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

        ApplySweepPlaybackPerFrame(cli.sweep_config, io.DeltaTime, sweepPaused, sweepSingleStep, sweepState, view, params, dirty);

        UiActionFlags actions = RenderControlsWindow(uiSchema, schemaWarning, schemaPath,
            view, params, render, lens, stats, renderedFrame,
            findingStatus, lastFindingPath,
            cli.sweep_config, sweepState, sweepPaused, sweepSingleStep,
            lastFractalType, lastPolyKind, dirty);

        DispatchUiActions(hwnd, actions.resetView, actions.resetAll, actions.loadState,
            actions.nextSeed, actions.prevSeed, view, params, render, lens,
            dirty, actions.interactionChanged, lastPolyKind, lastFractalType,
            findingStatus, lastFindingPath);

        ApplyArrowKeySeedScrub(io, view, params, seedScrubAccel, dirty, actions.interactionChanged);

        if (ApplyExplainoSeedDynamics(stats, io.DeltaTime, view, params)) {
            dirty = true;
        }
        if (ApplyParamAnimDynamics(io.DeltaTime, view, params)) {
            dirty = true;
        }

        if (actions.interactionChanged) {
            NoteViewerInteraction(&renderPacingState);
        }
        const ViewerRenderPacingConfig renderPacingConfig = BuildViewerRenderPacingConfig(render);
        const ViewerRenderPacingDecision renderPacing = AdvanceViewerRenderPacing(
            render,
            stats,
            (double)io.DeltaTime,
            renderPacingConfig,
            &renderPacingState);

        const bool forceFullQualityRender = actions.renderOnce || actions.captureDiagnostic || actions.captureFinding || renderPacing.full_quality_due;
        RefreshSidecarStateIfNeeded(dirty || !sidecarStateValid, view, params, engineCatalog,
            bind, sidecarMeasurementHost, sidecarState, sidecarStateValid,
            sidecarBudgetState, sidecarBudgetStateValid);
        DispatchRenderFrame(view, params, render, lens, renderPacing,
            forceFullQualityRender, view.auto_refresh, dirty,
            actions.renderOnce, actions.captureDiagnostic, actions.captureFinding,
            rgba, maskBuffer, lensSdfRgba, renderedFrame, stats, dirty);

        RunPendingInLoopCaptures(exeDir, actions, view, params, render, stats, rgba,
            renderedFrame, findingStatus, lastFindingPath,
            sidecarStateValid, sidecarBudgetStateValid);

        RenderExplainoSidecarWindow(sidecarState);

        RenderFractalViewport(io, render, view, dirty, actions.interactionChanged);

        if (lens.enabled) {
            RenderAuxImageWindow("Mask", g_maskSRV, renderedFrame);
            RenderAuxImageWindow("Lens SDF", g_lensSdfSRV, renderedFrame);
        }

        // Camera behavior loop (per-frame deltas scaled only by dive_speed; no dt semantics).
        // This runs after UI, so changing behavior updates immediately.
        ApplyAutoDivePerFrame(view, &dirty);

        PresentFrame();
    }

    ShutdownViewer(hwnd, wc);
    return 0;
}
