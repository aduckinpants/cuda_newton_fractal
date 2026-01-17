#include "fractal_types.h"

#include <Windows.h>
#include <d3d11.h>
#include <tchar.h>

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
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
    RenderStats* outStats,
    const char** outError);

static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

static ID3D11Texture2D* g_fractalTexture = nullptr;
static ID3D11ShaderResourceView* g_fractalSRV = nullptr;

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
    ViewState* view = nullptr;
    KernelParams* params = nullptr;
    RenderSettings* render = nullptr;

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
        // Fail-open: if we cannot evaluate a predicate, keep the control visible.
        if (pred.op.empty() || pred.path.empty()) return true;

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
            return true;
        }

        // Bool predicates
        bool curB = false;
        if (GetBoolValue(pred.path, curB)) {
            bool rhs = (pred.value == "true" || pred.value == "1");
            if (pred.op == "eq") return curB == rhs;
            if (pred.op == "neq") return curB != rhs;
            return true;
        }

        // Numeric predicates (int/float)
        double curN = 0.0;
        {
            int curI = 0;
            float curF = 0.0f;
            if (GetIntValue(pred.path, curI)) curN = (double)curI;
            else if (GetFloatValue(pred.path, curF)) curN = (double)curF;
            else return true;
        }

        double rhsN = 0.0;
        try {
            rhsN = std::stod(pred.value);
        } catch (...) {
            return true;
        }

        if (pred.op == "eq") return curN == rhsN;
        if (pred.op == "neq") return curN != rhsN;
        if (pred.op == "lt") return curN < rhsN;
        if (pred.op == "lte") return curN <= rhsN;
        if (pred.op == "gt") return curN > rhsN;
        if (pred.op == "gte") return curN >= rhsN;
        return true;
    }

    bool BindFloat(const std::string& path, float** outPtr) {
        if (!view || !params) return false;
        if (path == "fractal.view.center.x") { *outPtr = &view->center.x; return true; }
        if (path == "fractal.view.center.y") { *outPtr = &view->center.y; return true; }
        if (path == "fractal.view.zoom") { *outPtr = &view->zoom; return true; }
        if (path == "fractal.view.rotation") { *outPtr = &view->rotation_degrees; return true; }
        if (path == "fractal.view.dive_speed") { *outPtr = &view->dive_speed; return true; }
        if (path == "fractal.params.epsilon") { *outPtr = &params->epsilon; return true; }
        if (path == "fractal.params.nova_alpha") { *outPtr = &params->nova_alpha; return true; }
        if (path == "fractal.params.phoenix_p_real") { *outPtr = &params->phoenix_p_real; return true; }
        if (path == "fractal.params.phoenix_p_imag") { *outPtr = &params->phoenix_p_imag; return true; }
        if (path == "fractal.params.exposure") { *outPtr = &params->exposure; return true; }
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
        if (path == "fractal.render.resolution.x") { *outPtr = &render->resolution.x; return true; }
        if (path == "fractal.render.resolution.y") { *outPtr = &render->resolution.y; return true; }
        if (path == "fractal.render.block_size") { *outPtr = &render->block_size; return true; }
        if (path == "fractal.render.device_id") { *outPtr = &render->device_id; return true; }
        return false;
    }

    bool BindBool(const std::string& path, bool** outPtr) {
        if (!view || !render) return false;
        if (path == "fractal.view.auto_refresh") { *outPtr = &view->auto_refresh; return true; }
        if (path == "fractal.view.auto_dive") { *outPtr = &view->auto_dive; return true; }
        if (path == "fractal.render.benchmark") { *outPtr = &render->benchmark; return true; }
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

static constexpr double kMinZoom = 0.05;
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
        params.exposure = 1.0f;
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
        params.exposure = 1.0f;
        params.multibrot_power = 3;
        params.phoenix_p_real = -0.50f;
        params.phoenix_p_imag = 0.0f;
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
        params.exposure = 1.0f;
        params.multibrot_power = 3;
        if (ioDirty) *ioDirty = true;
        return;
    }

    // Escape-time family.
    params.max_iter = 800;
    params.coloring_mode = ColoringMode::smooth_escape;
    params.exposure = 1.0f;
    params.multibrot_power = 3;
    if (ioDirty) *ioDirty = true;
}

static bool ValidateSchemaBindings(const UISchema& schema, BindingContext& ctx, std::string* outError) {
    for (const auto& panel : schema.panels) {
        for (const auto& c : panel.controls) {
            if (!c.has_binding) continue;

            const auto& b = c.binding;
            if (b.path.empty() || b.kind.empty()) {
                if (outError) *outError = "Schema binding missing kind/path for control: " + c.id;
                return false;
            }

            if (b.kind == "action") {
                if (!(b.path == "fractal.actions.render_once" || b.path == "fractal.actions.reset_view" || b.path == "fractal.actions.reset_all")) {
                    if (outError) *outError = "Unknown action binding path: " + b.path + " (control: " + c.id + ")";
                    return false;
                }
                continue;
            }

            if (b.kind != "param") {
                if (outError) *outError = "Unknown binding kind: " + b.kind + " (control: " + c.id + ")";
                return false;
            }

            if (c.value_type == "bool") {
                bool* ptr = nullptr;
                if (!ctx.BindBool(b.path, &ptr) || !ptr) {
                    if (outError) *outError = "Bind failed for bool path: " + b.path + " (control: " + c.id + ")";
                    return false;
                }
            } else if (c.value_type == "int") {
                int* ptr = nullptr;
                if (!ctx.BindInt(b.path, &ptr) || !ptr) {
                    if (outError) *outError = "Bind failed for int path: " + b.path + " (control: " + c.id + ")";
                    return false;
                }
            } else if (c.value_type == "float") {
                float* ptr = nullptr;
                if (!ctx.BindFloat(b.path, &ptr) || !ptr) {
                    if (outError) *outError = "Bind failed for float path: " + b.path + " (control: " + c.id + ")";
                    return false;
                }
            } else if (c.value_type == "enum") {
                // Validate enum paths explicitly.
                if (!(b.path == "fractal.view.fractal_type" || b.path == "fractal.view.camera_behavior" || b.path == "fractal.params.poly_kind" ||
                      b.path == "fractal.params.coloring_mode")) {
                    if (outError) *outError = "Unknown enum binding path: " + b.path + " (control: " + c.id + ")";
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
        ImGui::TextDisabled("%s (UNBOUND)", c.label.c_str());
        ImGui::PopID();
        return false;
    }

    const auto& b = c.binding;
    if (c.type == "button") {
        if (b.kind != "action") {
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
        ImGui::TextDisabled("%s (bad param binding)", c.label.c_str());
        ImGui::PopID();
        return false;
    }

    if (c.type == "checkbox") {
        bool* ptr = nullptr;
        if (!ctx.BindBool(b.path, &ptr) || !ptr) {
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
            ImGui::TextDisabled("%s (bind failed)", c.label.c_str());
            ImGui::PopID();
            return false;
        }

        int minV = c.has_min ? (int)c.min : 0;
        int maxV = c.has_max ? (int)c.max : 100;

        bool changed = false;
        if (c.type == "slider_int") {
            changed = ImGui::SliderInt(c.label.c_str(), ptr, minV, maxV);
        } else {
            float speed = c.has_step ? (float)c.step : 1.0f;
            changed = ImGui::DragInt(c.label.c_str(), ptr, speed, minV, maxV);
        }
        if (changed && ioDirty) *ioDirty = true;
        ImGui::PopID();
        return changed;
    }

    if (c.type == "slider_float" || c.type == "drag_float") {
        float* ptr = nullptr;
        if (!ctx.BindFloat(b.path, &ptr) || !ptr) {
            ImGui::TextDisabled("%s (bind failed)", c.label.c_str());
            ImGui::PopID();
            return false;
        }

        float minV = c.has_min ? (float)c.min : 0.0f;
        float maxV = c.has_max ? (float)c.max : 1.0f;
        float speed = c.has_step ? (float)c.step : 0.01f;

        bool changed = false;
        if (c.type == "slider_float") {
            changed = ImGui::SliderFloat(c.label.c_str(), ptr, minV, maxV);
        } else {
            changed = ImGui::DragFloat(c.label.c_str(), ptr, speed, minV, maxV);
        }
        if (changed && ioDirty) *ioDirty = true;
        ImGui::PopID();
        return changed;
    }

    if (c.type == "combo") {
        // Only enum combos supported in this demo
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
                if (changed && ioDirty) *ioDirty = true;
            }
        }
        ImGui::PopID();
        return changed;
    }

    ImGui::TextDisabled("%s (unsupported control type: %s)", c.label.c_str(), c.type.c_str());
    ImGui::PopID();
    return false;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {
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
    RenderStats stats{};
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
    {
        std::string bindError;
        if (!ValidateSchemaBindings(uiSchema, initBind, &bindError)) {
            MessageBoxA(nullptr, bindError.c_str(), "Schema binding error", MB_OK | MB_ICONERROR);
            return 1;
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

        // Initialize high-precision view from the finalized initial float surface.
        SyncViewHpFromUi(view);
    }

    std::vector<uint32_t> rgba;
    rgba.resize((size_t)render.resolution.x * (size_t)render.resolution.y);
    PolyKind lastPolyKind = params.poly_kind;
    FractalType lastFractalType = view.fractal_type;

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

        bool renderOnceAction = false;
        bool resetViewAction = false;
        bool resetAllAction = false;

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
                        }
                    } else {
                        RenderControlFromSchema(ctrl, bind, &dirty, &renderOnceAction);
                    }
                }
            }
        }

        if (view.center.x != uiCenterBefore.x || view.center.y != uiCenterBefore.y || view.zoom != uiZoomBefore) {
            SyncViewHpFromUi(view);
        }

        if (view.fractal_type != FractalType::newton && view.fractal_type != FractalType::nova) {
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
            view.camera_behavior = CameraBehavior::complexity;
            view.auto_dive = true;
            view.dive_speed = 1.0f;

            // Kernel defaults (per current fractal type)
            ApplyFractalPresetDefaults(view, params, &dirty);
            SyncViewHpFromUi(view);
            lastPolyKind = params.poly_kind;
            lastFractalType = view.fractal_type;

            // Render defaults
            render.resolution = {1024, 768};
            render.block_size = 256;
            render.device_id = 0;
            render.benchmark = false;

            dirty = true;
        }

        // Render dispatch (fail-fast, delta-independent):
        // - If auto_refresh is ON, we render every frame.
        // - Otherwise, render on explicit request or any state change (dirty).
        if (view.auto_refresh || dirty || renderOnceAction) {
            rgba.resize((size_t)render.resolution.x * (size_t)render.resolution.y);
            EnsureFractalTexture(render.resolution.x, render.resolution.y);

            const char* err = nullptr;
            RenderStats newStats{};
            if (!RenderFractalCUDA(view, params, render, rgba.data(), &newStats, &err)) {
                // Show error pane
                ImGui::Begin("CUDA Error");
                ImGui::TextWrapped("Render failed: %s", err ? err : "unknown error");
                ImGui::End();
            } else {
                stats = newStats;
                UploadFractalRGBA(rgba.data(), render.resolution.x, render.resolution.y);
            }
            dirty = false;
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

    CleanupDeviceD3D();
    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}
