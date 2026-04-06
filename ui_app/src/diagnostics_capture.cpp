#include "diagnostics_capture.h"

#include <Windows.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

namespace {

const char* FractalTypeId(FractalType fractalType) {
    switch (fractalType) {
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
    case FractalType::explaino_nova: return "explaino_nova";
    case FractalType::explaino_halley: return "explaino_halley";
    case FractalType::explaino_dual: return "explaino_dual";
    case FractalType::explaino_mult: return "explaino_mult";
    case FractalType::explaino_phoenix: return "explaino_phoenix";
    case FractalType::explaino_transcendental: return "explaino_transcendental";
    case FractalType::explaino_inertial: return "explaino_inertial";
    case FractalType::explaino_julia: return "explaino_julia";
    case FractalType::explaino_rational: return "explaino_rational";
    case FractalType::multicorn: return "multicorn";
    case FractalType::halley: return "halley";
    case FractalType::collatz: return "collatz";
    case FractalType::explaino_collatz: return "explaino_collatz";
    case FractalType::mcmullen: return "mcmullen";
    case FractalType::lambda_map: return "lambda";
    }
    return "unknown";
}

const char* ColoringModeId(ColoringMode coloringMode) {
    switch (coloringMode) {
    case ColoringMode::root_basin: return "root_basin";
    case ColoringMode::iteration_count: return "iteration_count";
    case ColoringMode::smooth_escape: return "smooth_escape";
    case ColoringMode::joy_basins: return "joy_basins";
    }
    return "unknown";
}

const char* TranscendentalFuncId(TranscendentalFunc func) {
    switch (func) {
    case TranscendentalFunc::f_sin: return "f_sin";
    case TranscendentalFunc::f_exp_minus_1: return "f_exp_minus_1";
    case TranscendentalFunc::f_cosh: return "f_cosh";
    }
    return "unknown";
}

const char* McMullenPresetId(McMullenPreset preset) {
    switch (preset) {
    case McMullenPreset::z3_z3: return "z3_z3";
    case McMullenPreset::z2_z2: return "z2_z2";
    case McMullenPreset::z4_z2: return "z4_z2";
    case McMullenPreset::z3_z2: return "z3_z2";
    }
    return "unknown";
}

bool WriteTextFile(const std::filesystem::path& path, const std::string& text, std::string* outError) {
    std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file) {
        if (outError) *outError = "Failed to open text file for write: " + path.string();
        return false;
    }
    file.write(text.data(), static_cast<std::streamsize>(text.size()));
    if (!file.good()) {
        if (outError) *outError = "Failed to write text file: " + path.string();
        return false;
    }
    return true;
}

bool WriteRgbaBmp(const std::filesystem::path& path, const uint32_t* rgba, int width, int height, std::string* outError) {
    if (!rgba || width <= 0 || height <= 0) {
        if (outError) *outError = "WriteRgbaBmp received invalid image buffer";
        return false;
    }

    const int bytesPerPixel = 3;
    const int rowStride = width * bytesPerPixel;
    const int rowPadding = (4 - (rowStride % 4)) % 4;
    const int pixelBytes = (rowStride + rowPadding) * height;

    BITMAPFILEHEADER fileHeader{};
    fileHeader.bfType = 0x4d42;
    fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    fileHeader.bfSize = fileHeader.bfOffBits + pixelBytes;

    BITMAPINFOHEADER infoHeader{};
    infoHeader.biSize = sizeof(BITMAPINFOHEADER);
    infoHeader.biWidth = width;
    infoHeader.biHeight = height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 24;
    infoHeader.biCompression = BI_RGB;
    infoHeader.biSizeImage = pixelBytes;

    std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file) {
        if (outError) *outError = "Failed to open BMP for write: " + path.string();
        return false;
    }

    file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
    file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));

    std::vector<uint8_t> row(static_cast<size_t>(rowStride + rowPadding), 0);
    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            uint32_t pixel = rgba[static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x)];
            row[static_cast<size_t>(x) * 3 + 0] = static_cast<uint8_t>((pixel >> 16) & 0xff);
            row[static_cast<size_t>(x) * 3 + 1] = static_cast<uint8_t>((pixel >> 8) & 0xff);
            row[static_cast<size_t>(x) * 3 + 2] = static_cast<uint8_t>(pixel & 0xff);
        }
        file.write(reinterpret_cast<const char*>(row.data()), static_cast<std::streamsize>(row.size()));
    }

    if (!file.good()) {
        if (outError) *outError = "Failed while writing BMP: " + path.string();
        return false;
    }
    return true;
}

std::string BuildStateJson(const ViewState& view, const KernelParams& params, const RenderSettings& render, const RenderStats& stats) {
    std::ostringstream js;
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
    js << "    \"auto_increment_seed\": " << (view.auto_increment_seed ? "true" : "false") << ",\n";
    js << "    \"explaino_seed_rate\": " << static_cast<double>(view.explaino_seed_rate) << "\n";
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
    js << "    \"explaino_root_count\": " << params.explaino_root_count << ",\n";
    js << "    \"explaino_cluster_radius\": " << static_cast<double>(params.explaino_cluster_radius) << ",\n";
    js << "    \"transcendental_func\": \"" << TranscendentalFuncId(params.transcendental_func) << "\",\n";
    js << "    \"momentum_beta\": " << static_cast<double>(params.momentum_beta) << ",\n";
    js << "    \"mcmullen_preset\": \"" << McMullenPresetId(params.mcmullen_preset) << "\",\n";
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
    js << "    \"block_size\": " << render.block_size << ",\n";
    js << "    \"device_id\": " << render.device_id << "\n";
    js << "  },\n";
    js << "  \"stats\": {\n";
    js << "    \"last_render_ms\": " << static_cast<double>(stats.last_render_ms) << ",\n";
    js << "    \"last_iters_avg\": " << stats.last_iters_avg << ",\n";
    js << "    \"last_device_id\": " << stats.last_device_id << "\n";
    js << "  }\n";
    js << "}\n";
    return js.str();
}

} // namespace

bool CaptureDiagnosticsLastBundle(const std::string& exeDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    DiagnosticsCaptureResult* outResult,
    std::string* outError) {
    if (outError) outError->clear();

    std::filesystem::path bundleDir = std::filesystem::path(exeDir) / "diagnostics" / "last";
    bundleDir = bundleDir.lexically_normal();

    std::error_code ec;
    std::filesystem::create_directories(bundleDir, ec);
    if (ec) {
        if (outError) *outError = "Failed to create diagnostics directory: " + bundleDir.string();
        return false;
    }

    const std::filesystem::path framePath = bundleDir / "frame.bmp";
    const std::filesystem::path statePath = bundleDir / "state.json";

    if (!WriteRgbaBmp(framePath, rgba, render.resolution.x, render.resolution.y, outError)) {
        return false;
    }

    std::string stateJson = BuildStateJson(view, params, render, stats);
    if (!WriteTextFile(statePath, stateJson, outError)) {
        return false;
    }

    if (outResult) {
        outResult->output_dir = bundleDir.string();
        outResult->frame_bmp_path = framePath.string();
        outResult->state_json_path = statePath.string();
    }
    return true;
}