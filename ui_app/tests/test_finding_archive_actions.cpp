#define COLOR_PIPELINE_WINDOW_NO_IMGUI
#include "../src/color_pipeline_window.h"
#undef COLOR_PIPELINE_WINDOW_NO_IMGUI
#include "../src/finding_archive_actions.h"

#include <Windows.h>

#include <shellapi.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#pragma comment(lib, "shell32.lib")

namespace {

std::wstring WideAscii(const char* text) {
    std::wstring wide;
    if (!text) return wide;
    while (*text) {
        wide.push_back(static_cast<wchar_t>(*text));
        ++text;
    }
    return wide;
}

std::vector<std::wstring> ParseWindowsCommandLine(const std::wstring& commandLine) {
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(commandLine.c_str(), &argc);
    if (!argv) return {};

    std::vector<std::wstring> args;
    args.reserve(static_cast<size_t>(argc));
    for (int i = 0; i < argc; ++i) {
        args.push_back(argv[i]);
    }
    LocalFree(argv);
    return args;
}

bool ReadTextFile(const std::filesystem::path& path, std::string* outText) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) return false;
    std::ostringstream text;
    text << file.rdbuf();
    *outText = text.str();
    return true;
}

ColorPipelineWindowState MakeTestColorPipelineDraft() {
    ColorPipelineWindowState state;
    state.initialized = true;
    state.next_row_id = 4;

    ColorPipelineLaneState sourceLane;
    sourceLane.lane_id = "source";
    sourceLane.label = "Source";
    sourceLane.rows.push_back({1, true, "phase_orbit", {
        {"signal.phase_offset", "float", 1.25, false, ""},
        {"signal.wrap_cycles", "float", 2.5, false, ""},
    }});

    ColorPipelineLaneState shapeLane;
    shapeLane.lane_id = "shape";
    shapeLane.label = "Shape";
    shapeLane.rows.push_back({2, true, "repeat", {
        {"shape.frequency", "float", 6.0, false, ""},
        {"shape.phase", "float", 0.2, false, ""},
    }});

    ColorPipelineLaneState paletteLane;
    paletteLane.lane_id = "palette";
    paletteLane.label = "Palette";
    paletteLane.rows.push_back({3, true, "phase_wheel_palette", {
        {"palette.phase_offset", "float", -0.75, false, ""},
        {"palette.saturation", "float", 1.15, false, ""},
    }});

    state.lanes.push_back(std::move(sourceLane));
    state.lanes.push_back(std::move(shapeLane));
    state.lanes.push_back(std::move(paletteLane));
    return state;
}

} // namespace

int main() {
    namespace fs = std::filesystem;

    const fs::path tempRoot = fs::temp_directory_path() / "cuda_newton_fractal_clone_finding_archive_tests";
    fs::remove_all(tempRoot);
    fs::create_directories(tempRoot);

    {
        FindingArchiveIdentity identity = BuildUniqueFindingIdentity(
            tempRoot,
            "manual_capture",
            "2026-04-05",
            "112233_456",
            FractalType::newton);

        if (identity.out_root != tempRoot / "manual_capture" / "2026-04-05") {
            std::cerr << "Expected dated out_root under manual_capture\n";
            return 1;
        }
        if (identity.finding_id != "112233_456__newton") {
            std::cerr << "Unexpected base finding id\n";
            return 1;
        }
        if (identity.output_dir != identity.out_root / identity.finding_id) {
            std::cerr << "Expected output_dir to match out_root / finding_id\n";
            return 1;
        }
    }

    {
        fs::create_directories(tempRoot / "manual_capture" / "2026-04-05" / "112233_456__newton");
        FindingArchiveIdentity identity = BuildUniqueFindingIdentity(
            tempRoot,
            "manual_capture",
            "2026-04-05",
            "112233_456",
            FractalType::newton);

        if (identity.finding_id != "112233_456__newton__02") {
            std::cerr << "Expected collision suffix on second finding id\n";
            return 1;
        }
    }

    {
        FindingArchiveIdentity identity = BuildUniqueFindingIdentity(
            tempRoot,
            "manual sweep!",
            "2026-04-05",
            "235959_999",
            FractalType::explaino_fp);

        if (identity.out_root != tempRoot / "manual_sweep" / "2026-04-05") {
            std::cerr << "Expected sanitized group folder\n";
            return 1;
        }
        if (identity.finding_id != "235959_999__explaino_fp") {
            std::cerr << "Expected fractal type suffix in finding id\n";
            return 1;
        }
    }

    {
        FindingArchiveIdentity identity = BuildUniqueFindingIdentity(
            tempRoot,
            "manual sweep!",
            "2026-04-05",
            "235959_999",
            FractalType::explaino_nova);

        if (identity.finding_id != "235959_999__explaino_nova") {
            std::cerr << "Expected explaino_nova fractal type suffix in finding id\n";
            return 1;
        }
    }

    {
        const fs::path runtimeDir = tempRoot / "capture_bundle_runtime";
        fs::create_directories(runtimeDir);

        ViewState view{};
        view.fractal_type = FractalType::explaino_dual;
        view.explaino_phase_strength = -2.5f;
        view.auto_max_iter = true;
        KernelParams params{};
        params.coloring_mode = ColoringMode::joy_basins;
        params.color_pipeline = {ColorSignal::root_index, ColorPalette::joy, ColorGradingPreset::basin_default};
        params.explaino_seed = -3.0;
        params.explaino_seed_b = -7.5;
        params.explaino_root_spread = 1.75f;
        params.color_phase_signal_offset = 1.25f;
        params.color_phase_wrap_cycles = 2.5f;
        params.color_phase_palette_offset = -0.75f;
        params.color_shape = ColorPipelineShape::posterize;
        params.color_shape_stack_count = 2;
        params.color_shape_stack[0].shape = ColorPipelineShape::offset_scale;
        params.color_shape_stack[0].params.offset = 0.25f;
        params.color_shape_stack[0].params.scale = 1.5f;
        params.color_shape_stack[1].shape = ColorPipelineShape::posterize;
        params.color_shape_stack[1].params.posterize_steps = 5;
        params.color_shape_stack[1].params.posterize_mix = 0.65f;
        params.color_root_basin_pair_count = 2;
        params.color_root_basin_pairs[0] = {ColorSignal::root_index, ColorPalette::root_classic, ColorGradingPreset::basin_default};
        params.color_root_basin_pairs[1] = {ColorSignal::root_index, ColorPalette::joy, ColorGradingPreset::basin_default};
        params.color_grading_stack_count = 2;
        params.color_grading_stack[0].grading = ColorGradingPreset::escape_default;
        params.color_grading_stack[0].params.exposure = 1.4f;
        params.color_grading_stack[0].params.saturation = 1.2f;
        params.color_grading_stack[1].grading = ColorGradingPreset::phase_default;
        params.color_grading_stack[1].params.saturation = 0.8f;
        params.color_grading_stack[1].params.contrast = 1.6f;
        params.color_shape_posterize_steps = 5;
        params.color_shape_posterize_mix = 0.65f;
        params.color_iteration_band_count = 5;
        params.color_iteration_band_softness = 0.8f;
        params.color_iteration_band_emphasis = 1.6f;
        params.color_iteration_band_palette_offset = 0.4f;
        params.color_escape_magnitude_scale = 1.8f;
        params.color_escape_magnitude_bias = -0.25f;
        params.color_orbit_stripe_frequency = 3.5f;
        params.color_orbit_stripe_phase = 0.4f;
        params.color_root_proximity_scale = 2.25f;
        params.color_root_proximity_bias = -0.1f;
        RenderSettings render{};
        render.resolution = {64, 48};
        RenderStats stats{};
        std::vector<uint32_t> rgba(static_cast<size_t>(render.resolution.x) * static_cast<size_t>(render.resolution.y), 0xff336699u);
        SidecarOrientationVector orientation{};
        orientation.import_signature = 9007199254740993ull;
        orientation.pack_projection_hash = 18446744073709551614ull;
        orientation.field_embedding_stats = 5.5;
        orientation.slime_energy_delta = 2.25;
        orientation.busy_beaver_metrics = 0.75;
        orientation.decode_stability = 0.5;
        orientation.diff_magnitude = 1.5;
        const ColorPipelineWindowState colorPipelineWindow = MakeTestColorPipelineDraft();

        DiagnosticsCaptureResult capture;
        std::string error;
        if (!CaptureDiagnosticsLastBundle(runtimeDir.string(), view, params, render, stats, rgba.data(), rgba.size(), &orientation, &colorPipelineWindow, &capture, &error)) {
            std::cerr << "Expected diagnostics capture bundle to succeed: " << error << "\n";
            return 1;
        }

        std::string stateJson;
        if (!ReadTextFile(capture.state_json_path, &stateJson)) {
            std::cerr << "Expected diagnostics capture to write state.json\n";
            return 1;
        }
        if (stateJson.find("\"explaino_phase_strength\": -2.5") == std::string::npos ||
            stateJson.find("\"explaino_root_spread\": 1.75") == std::string::npos ||
            stateJson.find("\"color_phase_signal_offset\": 1.25") == std::string::npos ||
            stateJson.find("\"color_iteration_band_count\": 5") == std::string::npos ||
            stateJson.find("\"color_escape_magnitude_scale\": 1.7999999523162842") == std::string::npos ||
            stateJson.find("\"color_escape_magnitude_bias\": -0.25") == std::string::npos ||
            stateJson.find("\"color_orbit_stripe_frequency\": 3.5") == std::string::npos ||
            stateJson.find("\"color_orbit_stripe_phase\": 0.40000000596046448") == std::string::npos ||
            stateJson.find("\"color_root_proximity_scale\": 2.25") == std::string::npos ||
            stateJson.find("\"color_root_proximity_bias\": -0.10000000149011612") == std::string::npos ||
            stateJson.find("\"color_shape_posterize_steps\": 5") == std::string::npos ||
            stateJson.find("\"color_shape_posterize_mix\": 0.64999997615814209") == std::string::npos ||
            stateJson.find("\"color_shape_stack\"") == std::string::npos ||
            stateJson.find("\"color_grading_stack\"") == std::string::npos ||
            stateJson.find("\"grading\": \"escape_default\"") == std::string::npos ||
            stateJson.find("\"grading\": \"phase_default\"") == std::string::npos ||
            stateJson.find("\"color_root_basin_pairs\"") == std::string::npos ||
            stateJson.find("\"palette\": \"root_classic\"") == std::string::npos ||
            stateJson.find("\"palette\": \"joy\"") == std::string::npos ||
            stateJson.find("\"shape\": \"offset_scale\"") == std::string::npos ||
            stateJson.find("\"shape\": \"posterize\"") == std::string::npos ||
            stateJson.find("\"auto_max_iter\": true") == std::string::npos ||
            stateJson.find("\"interaction_debounce_ms\": 200") == std::string::npos ||
            stateJson.find("\"preview_target_fps\": 30") == std::string::npos ||
            stateJson.find("\"preview_min_scale\": 0.5") == std::string::npos ||
            stateJson.find("\"sidecar_orientation\"") == std::string::npos ||
            stateJson.find("\"import_signature\": \"9007199254740993\"") == std::string::npos ||
            stateJson.find("\"pack_projection_hash\": \"18446744073709551614\"") == std::string::npos ||
            stateJson.find("\"field_embedding_stats\": 5.5") == std::string::npos) {
            std::cerr << "Expected diagnostics capture to persist Explaino fields, Shape stacks, widened source owner fields, adaptive preview pacing, and optional sidecar orientation state\n";
            return 1;
        }
        if (stateJson.find("\"color_signal\": \"root_index\"") == std::string::npos ||
            stateJson.find("\"color_shape\": \"posterize\"") == std::string::npos ||
            stateJson.find("\"color_palette\": \"joy\"") == std::string::npos ||
            stateJson.find("\"color_grading\": \"basin_default\"") == std::string::npos) {
            std::cerr << "Expected diagnostics capture to persist the widened split-color state during Phase 2\n";
            return 1;
        }
        if (stateJson.find("\"coloring_mode\": \"joy_basins\"") == std::string::npos) {
            std::cerr << "Expected diagnostics capture to derive the widened mirrored coloring_mode from the split-color pipeline\n";
            return 1;
        }
        if (stateJson.find("\"coloring_mode\": \"iteration_count\"") != std::string::npos) {
            std::cerr << "Diagnostics capture must not trust a stale legacy coloring_mode when split-color state disagrees\n";
            return 1;
        }
        if (stateJson.find("\"color_pipeline\"") != std::string::npos) {
            std::cerr << "Diagnostics capture must serialize split-color fields explicitly, not the raw internal struct name\n";
            return 1;
        }
        if (stateJson.find("\"color_pipeline_draft\"") == std::string::npos ||
            stateJson.find("\"function_id\": \"repeat\"") == std::string::npos ||
            stateJson.find("\"shape.frequency\"") == std::string::npos) {
            std::cerr << "Expected diagnostics capture to persist the advanced color draft rows used by Capture Finding\n";
            return 1;
        }

        if (CaptureDiagnosticsLastBundle(runtimeDir.string(), view, params, render, stats, rgba.data(), rgba.size() - 1, &colorPipelineWindow, &capture, &error)) {
            std::cerr << "Expected diagnostics capture to reject mismatched pixel counts\n";
            return 1;
        }
        if (error.find("pixel count") == std::string::npos) {
            std::cerr << "Expected pixel-count validation error for mismatched capture buffer\n";
            return 1;
        }
    }

    {
        const fs::path runtimeDir = tempRoot / "capture_bundle_source_stack_runtime";
        fs::create_directories(runtimeDir);

        ViewState view{};
        view.fractal_type = FractalType::mandelbrot;
        KernelParams params{};
        params.coloring_mode = ColoringMode::smooth_escape;
        params.color_pipeline = {ColorSignal::escape_magnitude, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
        params.color_source_stack_count = 2;
        params.color_source_stack[0].signal = ColorSignal::smooth_escape;
        params.color_source_stack[0].params.scale = 0.5f;
        params.color_source_stack[0].params.bias = 0.25f;
        params.color_source_stack[0].params.blend_weight = 1.0f;
        params.color_source_stack[1].signal = ColorSignal::escape_magnitude;
        params.color_source_stack[1].params.magnitude_scale = 1.5f;
        params.color_source_stack[1].params.magnitude_bias = -0.25f;
        params.color_source_stack[1].params.blend_weight = 0.25f;
        params.color_escape_magnitude_scale = 1.5f;
        params.color_escape_magnitude_bias = -0.25f;
        RenderSettings render{};
        render.resolution = {64, 48};
        RenderStats stats{};
        std::vector<uint32_t> rgba(static_cast<size_t>(render.resolution.x) * static_cast<size_t>(render.resolution.y), 0xff336699u);

        DiagnosticsCaptureResult capture;
        std::string error;
        if (!CaptureDiagnosticsLastBundle(runtimeDir.string(), view, params, render, stats, rgba.data(), rgba.size(), &capture, &error)) {
            std::cerr << "Expected Source-stack diagnostics capture bundle to succeed: " << error << "\n";
            return 1;
        }

        std::string stateJson;
        if (!ReadTextFile(capture.state_json_path, &stateJson)) {
            std::cerr << "Expected Source-stack diagnostics capture to write state.json\n";
            return 1;
        }
        if (stateJson.find("\"color_source_stack\"") == std::string::npos ||
            stateJson.find("\"signal\": \"smooth_escape\"") == std::string::npos ||
            stateJson.find("\"signal\": \"escape_magnitude\"") == std::string::npos ||
            stateJson.find("\"magnitude_scale\": 1.5") == std::string::npos ||
            stateJson.find("\"magnitude_bias\": -0.25") == std::string::npos ||
            stateJson.find("\"blend_weight\": 0.25") == std::string::npos ||
            stateJson.find("\"color_signal\": \"escape_magnitude\"") == std::string::npos) {
            std::cerr << "Expected diagnostics capture to persist the generic Source stack while mirroring the final Source row into the flat split-color tuple\n";
            return 1;
        }
    }

    {
        const fs::path runtimeDir = tempRoot / "capture_bundle_neutral_finish_runtime";
        fs::create_directories(runtimeDir);

        ViewState view{};
        view.fractal_type = FractalType::mandelbrot;
        KernelParams params{};
        params.coloring_mode = ColoringMode::smooth_escape;
        params.color_pipeline = {ColorSignal::smooth_escape, ColorPalette::cyclic_escape, ColorGradingPreset::neutral_default};
        params.color_grading_stack_count = 1;
        params.color_grading_stack[0].grading = ColorGradingPreset::neutral_default;
        params.color_grading_stack[0].params.exposure = 1.25f;
        params.color_grading_stack[0].params.saturation = 0.75f;
        params.color_grading_stack[0].params.contrast = 1.5f;
        params.exposure = 1.25f;
        params.color_saturation = 0.75f;
        params.color_contrast = 1.5f;
        RenderSettings render{};
        render.resolution = {64, 48};
        RenderStats stats{};
        std::vector<uint32_t> rgba(static_cast<size_t>(render.resolution.x) * static_cast<size_t>(render.resolution.y), 0xff336699u);

        DiagnosticsCaptureResult capture;
        std::string error;
        if (!CaptureDiagnosticsLastBundle(runtimeDir.string(), view, params, render, stats, rgba.data(), rgba.size(), &capture, &error)) {
            std::cerr << "Expected neutral-finish diagnostics capture bundle to succeed: " << error << "\n";
            return 1;
        }

        std::string stateJson;
        if (!ReadTextFile(capture.state_json_path, &stateJson)) {
            std::cerr << "Expected neutral-finish diagnostics capture to write state.json\n";
            return 1;
        }
        if (stateJson.find("\"coloring_mode\": \"smooth_escape\"") == std::string::npos ||
            stateJson.find("\"color_grading\": \"neutral_default\"") == std::string::npos ||
            stateJson.find("\"color_grading_stack\"") == std::string::npos ||
            stateJson.find("\"grading\": \"neutral_default\"") == std::string::npos ||
            stateJson.find("\"exposure\": 1.25") == std::string::npos ||
            stateJson.find("\"saturation\": 0.75") == std::string::npos ||
            stateJson.find("\"contrast\": 1.5") == std::string::npos) {
            std::cerr << "Expected diagnostics capture to persist neutral_finish grading ids, stack rows, and mirrored owner values in the finding archive bundle\n";
            return 1;
        }
    }

    {
        RenderSettings render{};
        render.resolution = {1536, 1024};
        render.block_size = 128;
        render.device_id = 2;
        render.preview_target_fps = 48.0f;

        const RenderSettings captureRender = BuildFindingArchiveCaptureRender(render);
        if (captureRender.resolution.x != 4096 || captureRender.resolution.y != 4096) {
            std::cerr << "Expected finding archive capture to force a 4k square frame\n";
            return 1;
        }
        if (captureRender.block_size != render.block_size || captureRender.device_id != render.device_id ||
            captureRender.preview_target_fps != render.preview_target_fps) {
            std::cerr << "Expected 4k finding archive render to preserve the non-resolution render settings\n";
            return 1;
        }
    }

    {
        const fs::path runtimeDir = tempRoot / "capture_bundle_mirror_repeat";
        fs::create_directories(runtimeDir);

        ViewState view{};
        view.fractal_type = FractalType::mandelbrot;
        KernelParams params{};
        params.coloring_mode = ColoringMode::smooth_escape;
        params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
        params.color_shape = ColorPipelineShape::mirror_repeat;
        params.color_shape_repeat_frequency = 4.0f;
        params.color_shape_repeat_phase = 0.15f;
        RenderSettings render{};
        render.resolution = {16, 16};
        RenderStats stats{};
        std::vector<uint32_t> rgba(static_cast<size_t>(render.resolution.x) * static_cast<size_t>(render.resolution.y), 0xff112233u);

        DiagnosticsCaptureResult capture;
        std::string error;
        if (!CaptureDiagnosticsLastBundle(
            runtimeDir.string(),
            view,
            params,
            render,
            stats,
            rgba.data(),
            rgba.size(),
            static_cast<const SidecarOrientationVector*>(nullptr),
            static_cast<const ColorPipelineWindowState*>(nullptr),
            &capture,
            &error)) {
            std::cerr << "Expected mirror_repeat diagnostics capture bundle to succeed: " << error << "\n";
            return 1;
        }

        std::string stateJson;
        if (!ReadTextFile(capture.state_json_path, &stateJson)) {
            std::cerr << "Expected mirror_repeat diagnostics capture to write state.json\n";
            return 1;
        }
        if (stateJson.find("\"color_shape\": \"mirror_repeat\"") == std::string::npos) {
            std::cerr << "Expected diagnostics capture to persist the mirror_repeat Shape id once runtime-backed\n";
            return 1;
        }
    }

    {
        const fs::path runtimeDir = tempRoot / "capture_bundle_bias_gain_curve";
        fs::create_directories(runtimeDir);

        ViewState view{};
        view.fractal_type = FractalType::mandelbrot;
        KernelParams params{};
        params.coloring_mode = ColoringMode::smooth_escape;
        params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
        params.color_shape = ColorPipelineShape::bias_gain_curve;
        params.color_shape_bias = 0.25f;
        params.color_shape_gain = 0.75f;
        RenderSettings render{};
        render.resolution = {16, 16};
        RenderStats stats{};
        std::vector<uint32_t> rgba(static_cast<size_t>(render.resolution.x) * static_cast<size_t>(render.resolution.y), 0xff112233u);

        DiagnosticsCaptureResult capture;
        std::string error;
        if (!CaptureDiagnosticsLastBundle(
            runtimeDir.string(),
            view,
            params,
            render,
            stats,
            rgba.data(),
            rgba.size(),
            static_cast<const SidecarOrientationVector*>(nullptr),
            static_cast<const ColorPipelineWindowState*>(nullptr),
            &capture,
            &error)) {
            std::cerr << "Expected bias_gain_curve diagnostics capture bundle to succeed: " << error << "\n";
            return 1;
        }

        std::string stateJson;
        if (!ReadTextFile(capture.state_json_path, &stateJson)) {
            std::cerr << "Expected bias_gain_curve diagnostics capture to write state.json\n";
            return 1;
        }
        if (stateJson.find("\"color_shape\": \"bias_gain_curve\"") == std::string::npos ||
            stateJson.find("\"color_shape_bias\"") == std::string::npos ||
            stateJson.find("\"color_shape_gain\"") == std::string::npos) {
            std::cerr << "Expected diagnostics capture to persist the bias_gain_curve Shape id and dedicated owner fields once runtime-backed\n";
            return 1;
        }
    }

    {
        const fs::path runtimeDir = tempRoot / "capture_bundle_explaino_cmap";
        fs::create_directories(runtimeDir);

        ViewState view{};
        view.fractal_type = FractalType::mandelbrot;
        KernelParams params{};
        params.coloring_mode = ColoringMode::smooth_escape;
        params.color_pipeline = {ColorSignal::smooth_escape, ColorPalette::explaino_cmap, ColorGradingPreset::escape_default};
        params.color_explaino_palette_seed_scale = 1.5f;
        params.color_explaino_palette_seed_phase = 0.25f;
        params.color_explaino_palette_colorfulness = 0.8f;
        RenderSettings render{};
        render.resolution = {16, 16};
        RenderStats stats{};
        std::vector<uint32_t> rgba(static_cast<size_t>(render.resolution.x) * static_cast<size_t>(render.resolution.y), 0xff112233u);

        DiagnosticsCaptureResult capture;
        std::string error;
        if (!CaptureDiagnosticsLastBundle(
            runtimeDir.string(),
            view,
            params,
            render,
            stats,
            rgba.data(),
            rgba.size(),
            static_cast<const SidecarOrientationVector*>(nullptr),
            static_cast<const ColorPipelineWindowState*>(nullptr),
            &capture,
            &error)) {
            std::cerr << "Expected explaino_cmap diagnostics capture bundle to succeed: " << error << "\n";
            return 1;
        }

        std::string stateJson;
        if (!ReadTextFile(capture.state_json_path, &stateJson)) {
            std::cerr << "Expected explaino_cmap diagnostics capture to write state.json\n";
            return 1;
        }
        if (stateJson.find("\"color_palette\": \"explaino_cmap\"") == std::string::npos ||
            stateJson.find("\"color_explaino_palette_seed_scale\"") == std::string::npos ||
            stateJson.find("\"color_explaino_palette_seed_phase\"") == std::string::npos ||
            stateJson.find("\"color_explaino_palette_colorfulness\"") == std::string::npos) {
            std::cerr << "Expected diagnostics capture to persist the explaino_cmap palette id and dedicated owner fields once runtime-backed\n";
            return 1;
        }
    }

    {
        const fs::path runtimeDir = tempRoot / "capture_bundle_smooth_window";
        fs::create_directories(runtimeDir);

        ViewState view{};
        view.fractal_type = FractalType::mandelbrot;
        KernelParams params{};
        params.coloring_mode = ColoringMode::smooth_escape;
        params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
        params.color_shape = ColorPipelineShape::smooth_window;
        params.color_shape_window_center = 0.35f;
        params.color_shape_window_width = 0.4f;
        params.color_shape_window_softness = 0.05f;
        RenderSettings render{};
        render.resolution = {16, 16};
        RenderStats stats{};
        std::vector<uint32_t> rgba(static_cast<size_t>(render.resolution.x) * static_cast<size_t>(render.resolution.y), 0xff112233u);

        DiagnosticsCaptureResult capture;
        std::string error;
        if (!CaptureDiagnosticsLastBundle(
            runtimeDir.string(),
            view,
            params,
            render,
            stats,
            rgba.data(),
            rgba.size(),
            static_cast<const SidecarOrientationVector*>(nullptr),
            static_cast<const ColorPipelineWindowState*>(nullptr),
            &capture,
            &error)) {
            std::cerr << "Expected smooth_window diagnostics capture bundle to succeed: " << error << "\n";
            return 1;
        }

        std::string stateJson;
        if (!ReadTextFile(capture.state_json_path, &stateJson)) {
            std::cerr << "Expected smooth_window diagnostics capture to write state.json\n";
            return 1;
        }
        if (stateJson.find("\"color_shape\": \"smooth_window\"") == std::string::npos ||
            stateJson.find("\"color_shape_window_center\"") == std::string::npos ||
            stateJson.find("\"color_shape_window_width\"") == std::string::npos ||
            stateJson.find("\"color_shape_window_softness\"") == std::string::npos) {
            std::cerr << "Expected diagnostics capture to persist the smooth_window Shape id and dedicated owner fields once runtime-backed\n";
            return 1;
        }
    }

    {
        const fs::path repoRoot = tempRoot / "repo_root_probe";
        const fs::path scriptPath = repoRoot / "tools" / "reality_toolkit" / "scripts" / "run_fractal_explorer_archive_finding.py";
        fs::create_directories(scriptPath.parent_path());
        std::ofstream scriptFile(scriptPath, std::ios::out | std::ios::binary | std::ios::trunc);
        scriptFile << "# archive stub\n";
        scriptFile.close();

        const fs::path nestedStart = repoRoot / "nested" / "deeper" / "still_deeper";
        fs::create_directories(nestedStart);

        const fs::path resolved = FindRepoRootContainingArchiveScript(nestedStart);
        if (resolved != repoRoot) {
            std::cerr << "Expected repo-root discovery to walk upward until archive script is found\n";
            return 1;
        }
    }

    {
        const fs::path repoRoot = tempRoot / "repo_root_metadata_probe";
        const fs::path scriptPath = repoRoot / "tools" / "reality_toolkit" / "scripts" / "run_fractal_explorer_archive_finding.py";
        fs::create_directories(scriptPath.parent_path());
        std::ofstream scriptFile(scriptPath, std::ios::out | std::ios::binary | std::ios::trunc);
        scriptFile << "# archive stub\n";
        scriptFile.close();

        const fs::path runtimeDir = tempRoot / "runtime_root_probe" / "runtime";
        fs::create_directories(runtimeDir);
        std::ofstream metadataFile(runtimeDir / "fractal_ui_repo_root.txt", std::ios::out | std::ios::binary | std::ios::trunc);
        metadataFile << repoRoot.string() << "\n";
        metadataFile.close();

        const fs::path resolved = FindRepoRootFromRuntimeMetadata(runtimeDir);
        if (resolved != repoRoot) {
            std::cerr << "Expected runtime metadata file to resolve the absolute repo root\n";
            return 1;
        }
    }

    {
        const fs::path pythonLauncher = R"(C:\Windows\py.exe)";
        const fs::path scriptPath = R"(C:\code\cuda newton fractal clone\tools\reality_toolkit\scripts\run_fractal_explorer_archive_finding.py)";
        const fs::path repoRoot = R"(C:\code\cuda newton fractal clone)";
        const fs::path diagnosticsDir = R"(D:\salt fractal\cuda_newton_fractal_clone\runtime\diagnostics\last)";
        const fs::path outRoot = R"(D:\salt fractal\cuda_newton_fractal_clone\findings\manual capture\2026-04-05)";
        const std::string findingId = "235959_999__explaino_fp";
        const std::string why = "Slice \"smoke\" capture.";
        const std::string reproCommand =
            R"(D:\salt fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --load-state-json D:\salt fractal\cuda_newton_fractal_clone\findings\manual capture\2026-04-05\235959_999__explaino_fp\state.json --capture-diagnostic)";

        const std::wstring commandLine = BuildArchiveScriptCommandLine(
            pythonLauncher,
            scriptPath,
            repoRoot,
            diagnosticsDir,
            outRoot,
            findingId,
            why,
            reproCommand);
        const std::vector<std::wstring> argv = ParseWindowsCommandLine(commandLine);

        const std::vector<std::wstring> expected = {
            pythonLauncher.wstring(),
            L"-3.14",
            scriptPath.wstring(),
            L"--repo-root",
            repoRoot.wstring(),
            L"--diagnostics-dir",
            diagnosticsDir.wstring(),
            L"--out-root",
            outRoot.wstring(),
            L"--finding-id",
            WideAscii(findingId.c_str()),
            L"--why",
            WideAscii(why.c_str()),
            L"--repro-command",
            WideAscii(reproCommand.c_str()),
        };

        if (argv != expected) {
            std::cerr << "Expected archive script command line to round-trip through CommandLineToArgvW\n";
            return 1;
        }
    }

    {
        const fs::path pythonLauncher = R"(C:\Windows\py.exe)";
        const fs::path scriptPath = R"(C:\code\cuda newton fractal clone\tools\reality_toolkit\scripts\run_fractal_explorer_archive_finding.py)";
        const fs::path repoRoot = R"(C:\code\cuda newton fractal clone\)";
        const fs::path diagnosticsDir = R"(D:\salt fractal\cuda_newton_fractal_clone\runtime\diagnostics\last\)";
        const fs::path outRoot = R"(D:\salt fractal\cuda_newton_fractal_clone\findings\manual capture\2026-04-05\)";
        const std::string findingId = "235959_999__explaino_fp";

        const std::wstring commandLine = BuildArchiveScriptCommandLine(
            pythonLauncher,
            scriptPath,
            repoRoot,
            diagnosticsDir,
            outRoot,
            findingId,
            "Trailing slash capture",
            "fractal_ui.cmd --capture-diagnostic");
        const std::vector<std::wstring> argv = ParseWindowsCommandLine(commandLine);

        if (argv.size() < 9) {
            std::cerr << "Expected trailing-backslash command line to parse into argv entries\n";
            return 1;
        }
        if (argv[4] != repoRoot.wstring() || argv[6] != diagnosticsDir.wstring() || argv[8] != outRoot.wstring()) {
            std::cerr << "Expected trailing-backslash paths to round-trip through CommandLineToArgvW\n";
            return 1;
        }
    }

    std::cout << "test_finding_archive_actions: all passed\n";
    return 0;
}