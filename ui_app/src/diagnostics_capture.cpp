#include "diagnostics_capture.h"

#define COLOR_PIPELINE_WINDOW_NO_IMGUI
#include "color_pipeline_window.h"
#undef COLOR_PIPELINE_WINDOW_NO_IMGUI
#include "fractal_family_rules.h"
#include "render_capture_guard.h"

#include <Windows.h>

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

namespace {

const char* CaptureFractalTypeId(FractalType fractalType) {
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
    case FractalType::explaino_joy: return "explaino_joy";
    case FractalType::explaino_fold: return "explaino_fold";
    case FractalType::explaino_bell: return "explaino_bell";
    case FractalType::explaino_ripple: return "explaino_ripple";
    case FractalType::explaino_splice: return "explaino_splice";
    case FractalType::explaino_vortex: return "explaino_vortex";
    case FractalType::explaino_tension: return "explaino_tension";
    case FractalType::multicorn: return "multicorn";
    case FractalType::halley: return "halley";
    case FractalType::collatz: return "collatz";
    case FractalType::explaino_collatz: return "explaino_collatz";
    case FractalType::mcmullen: return "mcmullen";
    case FractalType::lambda_map: return "lambda";
    case FractalType::explaino_lambda: return "explaino_lambda";
    case FractalType::explaino_rational_escape: return "explaino_rational_escape";
    case FractalType::spider: return "spider";
    case FractalType::celtic_mandelbrot: return "celtic_mandelbrot";
    case FractalType::perpendicular_burning_ship: return "perpendicular_burning_ship";
    }
    return "unknown";
}

const char* CaptureColoringModeId(ColoringMode coloringMode) {
    switch (coloringMode) {
    case ColoringMode::root_basin: return "root_basin";
    case ColoringMode::iteration_count: return "iteration_count";
    case ColoringMode::smooth_escape: return "smooth_escape";
    case ColoringMode::joy_basins: return "joy_basins";
    case ColoringMode::phase: return "phase";
    case ColoringMode::iteration_bands: return "iteration_bands";
    }
    return "unknown";
}

const char* CaptureColorSignalId(ColorSignal signal) {
    switch (signal) {
    case ColorSignal::root_index: return "root_index";
    case ColorSignal::iteration_count: return "iteration_count";
    case ColorSignal::smooth_escape: return "smooth_escape";
    case ColorSignal::phase_angle: return "phase_angle";
    case ColorSignal::iteration_bands: return "iteration_bands";
    }
    return "unknown";
}

const char* CaptureColorPaletteId(ColorPalette palette) {
    switch (palette) {
    case ColorPalette::root_classic: return "root_classic";
    case ColorPalette::joy: return "joy";
    case ColorPalette::cyclic_escape: return "cyclic_escape";
    case ColorPalette::phase_wheel: return "phase_wheel";
    case ColorPalette::banded_escape: return "banded_escape";
    }
    return "unknown";
}

const char* CaptureColorGradingPresetId(ColorGradingPreset grading) {
    switch (grading) {
    case ColorGradingPreset::basin_default: return "basin_default";
    case ColorGradingPreset::escape_default: return "escape_default";
    case ColorGradingPreset::phase_default: return "phase_default";
    case ColorGradingPreset::bands_default: return "bands_default";
    }
    return "unknown";
}

const char* CaptureColorPipelineShapeId(ColorPipelineShape shape) {
    switch (shape) {
    case ColorPipelineShape::identity: return "identity";
    case ColorPipelineShape::offset_scale: return "offset_scale";
    case ColorPipelineShape::repeat: return "repeat";
    }
    return "unknown";
}

const char* CaptureTranscendentalFuncId(TranscendentalFunc func) {
    switch (func) {
    case TranscendentalFunc::f_sin: return "f_sin";
    case TranscendentalFunc::f_exp_minus_1: return "f_exp_minus_1";
    case TranscendentalFunc::f_cosh: return "f_cosh";
    }
    return "unknown";
}

const char* CaptureMcMullenPresetId(McMullenPreset preset) {
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

void WriteExplainoVariantParamsJson(std::ostringstream& js, const KernelParams& params) {
    js << "    \"joy_coupling\": " << static_cast<double>(params.joy_coupling) << ",\n";
    js << "    \"fold_coupling\": " << static_cast<double>(params.fold_coupling) << ",\n";
    js << "    \"bell_coupling\": " << static_cast<double>(params.bell_coupling) << ",\n";
    js << "    \"ripple_amplitude\": " << static_cast<double>(params.ripple_amplitude) << ",\n";
    js << "    \"splice_offset\": " << static_cast<double>(params.splice_offset) << ",\n";
    js << "    \"vortex_strength\": " << static_cast<double>(params.vortex_strength) << ",\n";
    js << "    \"tension_strength\": " << static_cast<double>(params.tension_strength) << ",\n";
}

void WriteColorParamsJson(std::ostringstream& js, const KernelParams& params) {
    js << "    \"color_saturation\": " << static_cast<double>(params.color_saturation) << ",\n";
    js << "    \"color_contrast\": " << static_cast<double>(params.color_contrast) << ",\n";
    js << "    \"color_tint_r\": " << static_cast<double>(params.color_tint_r) << ",\n";
    js << "    \"color_tint_g\": " << static_cast<double>(params.color_tint_g) << ",\n";
    js << "    \"color_tint_b\": " << static_cast<double>(params.color_tint_b) << ",\n";
    js << "    \"color_phase_signal_offset\": " << static_cast<double>(params.color_phase_signal_offset) << ",\n";
    js << "    \"color_phase_wrap_cycles\": " << static_cast<double>(params.color_phase_wrap_cycles) << ",\n";
    js << "    \"color_phase_palette_offset\": " << static_cast<double>(params.color_phase_palette_offset) << ",\n";
    js << "    \"color_shape_offset\": " << static_cast<double>(params.color_shape_offset) << ",\n";
    js << "    \"color_shape_scale\": " << static_cast<double>(params.color_shape_scale) << ",\n";
    js << "    \"color_shape_repeat_frequency\": " << static_cast<double>(params.color_shape_repeat_frequency) << ",\n";
    js << "    \"color_shape_repeat_phase\": " << static_cast<double>(params.color_shape_repeat_phase) << ",\n";
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
}

void WriteSidecarOrientationJson(std::ostringstream& js, const SidecarOrientationVector& orientation) {
    js << "  \"sidecar_orientation\": {\n";
    js << "    \"import_signature\": \"" << static_cast<unsigned long long>(orientation.import_signature) << "\",\n";
    js << "    \"pack_projection_hash\": \"" << static_cast<unsigned long long>(orientation.pack_projection_hash) << "\",\n";
    js << "    \"field_embedding_stats\": " << orientation.field_embedding_stats << ",\n";
    js << "    \"slime_energy_delta\": " << orientation.slime_energy_delta << ",\n";
    js << "    \"busy_beaver_metrics\": " << orientation.busy_beaver_metrics << ",\n";
    js << "    \"decode_stability\": " << orientation.decode_stability << ",\n";
    js << "    \"diff_magnitude\": " << orientation.diff_magnitude << "\n";
    js << "  }";
}

void WriteSidecarAutoDemoPolicyJson(std::ostringstream& js, const SidecarAutoDemoControllerPolicy& policy) {
    js << "  \"sidecar_auto_demo_policy\": {\n";
    js << "    \"enabled\": " << (policy.enabled ? "true" : "false") << ",\n";
    js << "    \"allow_runtime_mutation\": " << (policy.allow_runtime_mutation ? "true" : "false") << ",\n";
    js << "    \"run_paced_loop\": " << (policy.run_paced_loop ? "true" : "false") << ",\n";
    js << "    \"paced_loop_interval_seconds\": " << policy.paced_loop_interval_seconds << ",\n";
    js << "    \"stop_demonstrated_fraction\": " << policy.stop_demonstrated_fraction << ",\n";
    js << "    \"stop_uncertain_count\": " << policy.stop_uncertain_count << "\n";
    js << "  }";
}

void WriteSidecarMutationHistoryJson(std::ostringstream& js, const SidecarAutoDemoMutationHistory& history) {
    js << "  \"sidecar_mutation_history\": [\n";
    for (size_t index = 0; index < history.size(); ++index) {
        const SidecarAutoDemoMutationRecord& record = history[index];
        js << "    {\n";
        js << "      \"label\": \"" << record.label << "\",\n";
        js << "      \"path\": \"" << record.path << "\",\n";
        js << "      \"type\": \"" << record.type << "\",\n";
        js << "      \"target_value\": " << record.target_value << ",\n";
        js << "      \"utility\": " << record.utility << "\n";
        js << "    }";
        if (index + 1 < history.size()) {
            js << ",";
        }
        js << "\n";
    }
    js << "  ]";
}

void WriteJsonEscapedString(std::ostringstream& js, const std::string& text) {
    js << '"';
    for (char ch : text) {
        switch (ch) {
        case '\\': js << "\\\\"; break;
        case '"': js << "\\\""; break;
        case '\n': js << "\\n"; break;
        case '\r': js << "\\r"; break;
        case '\t': js << "\\t"; break;
        default: js << ch; break;
        }
    }
    js << '"';
}

bool HasSerializableColorPipelineDraft(const ColorPipelineWindowState* state) {
    return state && state->initialized && !state->lanes.empty();
}

void WriteColorPipelineParamStateJson(std::ostringstream& js, const ColorPipelineParamState& param) {
    js << "          {\n";
    js << "            \"path\": ";
    WriteJsonEscapedString(js, param.path);
    js << ",\n";
    js << "            \"type\": ";
    WriteJsonEscapedString(js, param.type);
    js << ",\n";
    if (param.type == "bool") {
        js << "            \"bool_value\": " << (param.bool_value ? "true" : "false") << "\n";
    } else if (param.type == "enum") {
        js << "            \"enum_value\": ";
        WriteJsonEscapedString(js, param.enum_value);
        js << "\n";
    } else {
        js << "            \"number_value\": " << param.number_value << "\n";
    }
    js << "          }";
}

void WriteColorPipelineRowStateJson(std::ostringstream& js, const ColorPipelineRowState& row) {
    js << "        {\n";
    js << "          \"ui_row_id\": " << row.ui_row_id << ",\n";
    js << "          \"enabled\": " << (row.enabled ? "true" : "false") << ",\n";
    js << "          \"function_id\": ";
    WriteJsonEscapedString(js, row.function_id);
    js << ",\n";
    js << "          \"parameter_values\": [\n";
    for (std::size_t index = 0; index < row.parameter_values.size(); ++index) {
        WriteColorPipelineParamStateJson(js, row.parameter_values[index]);
        if (index + 1 < row.parameter_values.size()) {
            js << ",";
        }
        js << "\n";
    }
    js << "          ]\n";
    js << "        }";
}

void WriteColorPipelineLaneStateJson(std::ostringstream& js, const ColorPipelineLaneState& lane) {
    js << "      {\n";
    js << "        \"lane_id\": ";
    WriteJsonEscapedString(js, lane.lane_id);
    js << ",\n";
    js << "        \"label\": ";
    WriteJsonEscapedString(js, lane.label);
    js << ",\n";
    js << "        \"rows\": [\n";
    for (std::size_t index = 0; index < lane.rows.size(); ++index) {
        WriteColorPipelineRowStateJson(js, lane.rows[index]);
        if (index + 1 < lane.rows.size()) {
            js << ",";
        }
        js << "\n";
    }
    js << "        ]\n";
    js << "      }";
}

void WriteColorPipelineDraftJson(std::ostringstream& js, const ColorPipelineWindowState& state) {
    js << "  \"color_pipeline_draft\": {\n";
    js << "    \"next_row_id\": " << state.next_row_id << ",\n";
    js << "    \"lanes\": [\n";
    for (std::size_t index = 0; index < state.lanes.size(); ++index) {
        WriteColorPipelineLaneStateJson(js, state.lanes[index]);
        if (index + 1 < state.lanes.size()) {
            js << ",";
        }
        js << "\n";
    }
    js << "    ]\n";
    js << "  }";
}

std::string BuildStateJson(
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const SidecarOrientationVector* sidecarOrientation,
    const SidecarAutoDemoControllerPolicy* sidecarControllerPolicy,
    const SidecarAutoDemoMutationHistory* sidecarMutationHistory,
    const ColorPipelineWindowState* colorPipelineWindow) {
    ColoringMode mirroredColoringMode = ColoringMode::root_basin;
    const bool hasLegacyColoringMirror = TryLegacyColoringModeForPipeline(params.color_pipeline, &mirroredColoringMode);

    std::ostringstream js;
    js << "{\n";
    js << "  \"state_version\": 3,\n";
    js << "  \"fractal_type\": \"" << CaptureFractalTypeId(view.fractal_type) << "\",\n";
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
    if (hasLegacyColoringMirror) {
        js << "    \"coloring_mode\": \"" << CaptureColoringModeId(mirroredColoringMode) << "\",\n";
    }
    js << "    \"color_signal\": \"" << CaptureColorSignalId(params.color_pipeline.signal) << "\",\n";
    js << "    \"color_shape\": \"" << CaptureColorPipelineShapeId(params.color_shape) << "\",\n";
    js << "    \"color_palette\": \"" << CaptureColorPaletteId(params.color_pipeline.palette) << "\",\n";
    js << "    \"color_grading\": \"" << CaptureColorGradingPresetId(params.color_pipeline.grading) << "\",\n";
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
    js << "    \"explaino_cluster_radius\": " << static_cast<double>(params.explaino_cluster_radius) << ",\n";
    WriteExplainoVariantParamsJson(js, params);
    js << "    \"transcendental_func\": \"" << CaptureTranscendentalFuncId(params.transcendental_func) << "\",\n";
    js << "    \"momentum_beta\": " << static_cast<double>(params.momentum_beta) << ",\n";
    js << "    \"mcmullen_preset\": \"" << CaptureMcMullenPresetId(params.mcmullen_preset) << "\",\n";
    js << "    \"poly_coeffs\": [";
    for (int i = 0; i < 5; ++i) {
        if (i > 0) js << ", ";
        js << static_cast<double>(params.poly_coeffs[i]);
    }
    js << "],\n";
    WriteColorParamsJson(js, params);
    js << "  },\n";
    js << "  \"render\": {\n";
    js << "    \"width\": " << render.resolution.x << ",\n";
    js << "    \"height\": " << render.resolution.y << ",\n";
    js << "    \"interaction_debounce_ms\": " << render.interaction_debounce_ms << ",\n";
    js << "    \"preview_target_fps\": " << static_cast<double>(render.preview_target_fps) << ",\n";
    js << "    \"preview_min_scale\": " << static_cast<double>(render.preview_min_scale) << ",\n";
    js << "    \"block_size\": " << render.block_size << ",\n";
    js << "    \"device_id\": " << render.device_id << "\n";
    js << "  },\n";
    js << "  \"stats\": {\n";
    js << "    \"last_render_ms\": " << static_cast<double>(stats.last_render_ms) << ",\n";
    js << "    \"last_iters_avg\": " << stats.last_iters_avg << ",\n";
    js << "    \"last_device_id\": " << stats.last_device_id << "\n";
    js << "  }";
    if (sidecarOrientation) {
        js << ",\n";
        WriteSidecarOrientationJson(js, *sidecarOrientation);
    }
    if (sidecarControllerPolicy) {
        js << ",\n";
        WriteSidecarAutoDemoPolicyJson(js, *sidecarControllerPolicy);
    }
    if (sidecarMutationHistory) {
        js << ",\n";
        WriteSidecarMutationHistoryJson(js, *sidecarMutationHistory);
    }
    if (HasSerializableColorPipelineDraft(colorPipelineWindow)) {
        js << ",\n";
        WriteColorPipelineDraftJson(js, *colorPipelineWindow);
    }
    js << "}\n";
    return js.str();
}

bool CaptureDiagnosticsBundleToDirWithDraft(const std::string& outputDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const SidecarOrientationVector* sidecarOrientation,
    const SidecarAutoDemoControllerPolicy* sidecarControllerPolicy,
    const SidecarAutoDemoMutationHistory* sidecarMutationHistory,
    const ColorPipelineWindowState* colorPipelineWindow,
    DiagnosticsCaptureResult* outResult,
    std::string* outError) {
    if (outError) outError->clear();
    if (!HasExactRenderPixelCount(render, rgbaPixelCount, outError)) {
        return false;
    }

    std::filesystem::path bundleDir = std::filesystem::path(outputDir).lexically_normal();

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

    std::string stateJson = BuildStateJson(
        view,
        params,
        render,
        stats,
        sidecarOrientation,
        sidecarControllerPolicy,
        sidecarMutationHistory,
        colorPipelineWindow);
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

} // namespace

bool CaptureDiagnosticsLastBundle(const std::string& exeDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    DiagnosticsCaptureResult* outResult,
    std::string* outError) {
    return CaptureDiagnosticsLastBundle(
        exeDir,
        view,
        params,
        render,
        stats,
        rgba,
        rgbaPixelCount,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        outResult,
        outError);
}

bool CaptureDiagnosticsLastBundle(const std::string& exeDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const ColorPipelineWindowState* colorPipelineWindow,
    DiagnosticsCaptureResult* outResult,
    std::string* outError) {
    return CaptureDiagnosticsLastBundle(
        exeDir,
        view,
        params,
        render,
        stats,
        rgba,
        rgbaPixelCount,
        nullptr,
        nullptr,
        nullptr,
        colorPipelineWindow,
        outResult,
        outError);
}

bool CaptureDiagnosticsLastBundle(const std::string& exeDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const SidecarOrientationVector* sidecarOrientation,
    DiagnosticsCaptureResult* outResult,
    std::string* outError) {
    return CaptureDiagnosticsLastBundle(
        exeDir,
        view,
        params,
        render,
        stats,
        rgba,
        rgbaPixelCount,
        sidecarOrientation,
        nullptr,
        nullptr,
        nullptr,
        outResult,
        outError);
}

bool CaptureDiagnosticsLastBundle(const std::string& exeDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const SidecarOrientationVector* sidecarOrientation,
    const ColorPipelineWindowState* colorPipelineWindow,
    DiagnosticsCaptureResult* outResult,
    std::string* outError) {
    return CaptureDiagnosticsLastBundle(
        exeDir,
        view,
        params,
        render,
        stats,
        rgba,
        rgbaPixelCount,
        sidecarOrientation,
        nullptr,
        nullptr,
        colorPipelineWindow,
        outResult,
        outError);
}

bool CaptureDiagnosticsLastBundle(const std::string& exeDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const SidecarOrientationVector* sidecarOrientation,
    const SidecarAutoDemoControllerPolicy* sidecarControllerPolicy,
    DiagnosticsCaptureResult* outResult,
    std::string* outError) {
    return CaptureDiagnosticsLastBundle(
        exeDir,
        view,
        params,
        render,
        stats,
        rgba,
        rgbaPixelCount,
        sidecarOrientation,
        sidecarControllerPolicy,
        nullptr,
        nullptr,
        outResult,
        outError);
}

bool CaptureDiagnosticsLastBundle(const std::string& exeDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const SidecarOrientationVector* sidecarOrientation,
    const SidecarAutoDemoControllerPolicy* sidecarControllerPolicy,
    const SidecarAutoDemoMutationHistory* sidecarMutationHistory,
    DiagnosticsCaptureResult* outResult,
    std::string* outError) {
    return CaptureDiagnosticsLastBundle(
        exeDir,
        view,
        params,
        render,
        stats,
        rgba,
        rgbaPixelCount,
        sidecarOrientation,
        sidecarControllerPolicy,
        sidecarMutationHistory,
        nullptr,
        outResult,
        outError);
}

bool CaptureDiagnosticsLastBundle(const std::string& exeDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const SidecarOrientationVector* sidecarOrientation,
    const SidecarAutoDemoControllerPolicy* sidecarControllerPolicy,
    const SidecarAutoDemoMutationHistory* sidecarMutationHistory,
    const ColorPipelineWindowState* colorPipelineWindow,
    DiagnosticsCaptureResult* outResult,
    std::string* outError) {
    const std::filesystem::path bundleDir = (std::filesystem::path(exeDir) / "diagnostics" / "last").lexically_normal();
    return CaptureDiagnosticsBundleToDirWithDraft(
        bundleDir.string(),
        view,
        params,
        render,
        stats,
        rgba,
        rgbaPixelCount,
        sidecarOrientation,
        sidecarControllerPolicy,
        sidecarMutationHistory,
        colorPipelineWindow,
        outResult,
        outError);
}

bool CaptureDiagnosticsBundleToDir(const std::string& outputDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    DiagnosticsCaptureResult* outResult,
    std::string* outError) {
    return CaptureDiagnosticsBundleToDir(
        outputDir,
        view,
        params,
        render,
        stats,
        rgba,
        rgbaPixelCount,
        nullptr,
        nullptr,
        nullptr,
        outResult,
        outError);
}

bool CaptureDiagnosticsBundleToDir(const std::string& outputDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const SidecarOrientationVector* sidecarOrientation,
    DiagnosticsCaptureResult* outResult,
    std::string* outError) {
    return CaptureDiagnosticsBundleToDir(
        outputDir,
        view,
        params,
        render,
        stats,
        rgba,
        rgbaPixelCount,
        sidecarOrientation,
        nullptr,
        nullptr,
        outResult,
        outError);
}

bool CaptureDiagnosticsBundleToDir(const std::string& outputDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const SidecarOrientationVector* sidecarOrientation,
    const SidecarAutoDemoControllerPolicy* sidecarControllerPolicy,
    DiagnosticsCaptureResult* outResult,
    std::string* outError) {
    return CaptureDiagnosticsBundleToDir(
        outputDir,
        view,
        params,
        render,
        stats,
        rgba,
        rgbaPixelCount,
        sidecarOrientation,
        sidecarControllerPolicy,
        nullptr,
        outResult,
        outError);
}

bool CaptureDiagnosticsBundleToDir(const std::string& outputDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const SidecarOrientationVector* sidecarOrientation,
    const SidecarAutoDemoControllerPolicy* sidecarControllerPolicy,
    const SidecarAutoDemoMutationHistory* sidecarMutationHistory,
    DiagnosticsCaptureResult* outResult,
    std::string* outError) {
    return CaptureDiagnosticsBundleToDirWithDraft(
        outputDir,
        view,
        params,
        render,
        stats,
        rgba,
        rgbaPixelCount,
        sidecarOrientation,
        sidecarControllerPolicy,
        sidecarMutationHistory,
        nullptr,
        outResult,
        outError);
}
