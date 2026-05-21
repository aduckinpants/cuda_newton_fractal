#include "../src/fractal_family_rules.h"
#include "../src/generic_equation_pack.h"
#include "../src/generic_equation_pack_live.h"

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

namespace {

constexpr const char* kPackJson = R"json({
  "schema_version": 1,
  "pack_id": "live_quadratic",
  "name": "Live Quadratic",
  "formula": {
    "kind": "iterate_map",
    "iteration_param": "steps",
    "ast": {
      "op": "add",
      "args": [
        {"op": "pow_int", "base": {"op": "var_z"}, "exponent": 2},
        {"op": "complex_param", "name": "c"}
      ]
    }
  },
  "params": {"steps": 18.0, "c_real": -0.75, "c_imag": 0.1},
  "controls": [
    {"id": "steps", "param": "steps", "label": "Steps", "min": 1.0, "max": 80.0, "step": 1.0, "default": 18.0},
    {"id": "c_real", "param": "c_real", "label": "C Real", "min": -2.0, "max": 2.0, "step": 0.01, "default": -0.75}
  ],
  "epsilon": 1e-9,
  "escape_radius": 1000.0,
  "region": {"center_x": 0.0, "center_y": 0.0, "span_x": 2.0, "span_y": 2.0, "grid_width": 8, "grid_height": 6}
})json";

std::uint64_t HashPixels(const std::vector<std::uint32_t>& pixels) {
    std::uint64_t hash = 1469598103934665603ull;
    for (std::uint32_t pixel : pixels) {
        hash ^= static_cast<std::uint64_t>(pixel);
        hash *= 1099511628211ull;
    }
    return hash;
}

GenericEquationPack LoadPack() {
    GenericEquationPackParseResult parsed = ParseGenericEquationPackJson(kPackJson);
    if (!parsed.ok) {
        std::cerr << "pack parse failed: " << parsed.error << std::endl;
        std::exit(2);
    }
    return parsed.pack;
}

bool RenderPack(GenericEquationPack pack, const KernelParams& params, std::uint64_t* outHash, RenderStats* outStats) {
    ViewState view{};
    view.fractal_type = FractalType::generic_equation_pack;
    view.center_hp_x = 0.0;
    view.center_hp_y = 0.0;
    view.log2_zoom = 0.0;

    RenderSettings render{};
    render.resolution = {24, 18};
    render.benchmark = true;

    std::vector<std::uint32_t> pixels(static_cast<std::size_t>(render.resolution.x) * static_cast<std::size_t>(render.resolution.y));
    std::string error;
    if (!RenderGenericEquationPackLiveFrame(pack, view, params, render, pixels.data(), nullptr, outStats, &error)) {
        std::cerr << "generic live render failed: " << error << std::endl;
        return false;
    }
    *outHash = HashPixels(pixels);
    return true;
}

bool TestGenericPackRendersMainViewportFrame() {
    GenericEquationPack pack = LoadPack();
    KernelParams params{};
    params.coloring_mode = ColoringMode::smooth_escape;
    params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
    std::uint64_t hash = 0;
    RenderStats stats{};
    if (!RenderPack(pack, params, &hash, &stats)) return false;
    if (hash == 0) {
        std::cerr << "generic live render produced an empty hash" << std::endl;
        return false;
    }
    if (stats.last_pixel_count != 24 * 18 || stats.last_iters_sum == 0 || stats.last_iters_avg <= 0) {
        std::cerr << "generic live render stats were not populated" << std::endl;
        return false;
    }
    return true;
}

bool TestGenericPackControlChangesMainViewportFrame() {
    GenericEquationPack baseline = LoadPack();
    GenericEquationPack edited = LoadPack();
    edited.params["c_real"] = 0.35;
    KernelParams params{};
    params.coloring_mode = ColoringMode::smooth_escape;
    params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);

    std::uint64_t baselineHash = 0;
    std::uint64_t editedHash = 0;
    RenderStats baselineStats{};
    RenderStats editedStats{};
    if (!RenderPack(baseline, params, &baselineHash, &baselineStats)) return false;
    if (!RenderPack(edited, params, &editedHash, &editedStats)) return false;
    if (baselineHash == editedHash) {
        std::cerr << "generic live render hash did not change after pack control edit" << std::endl;
        return false;
    }
    return true;
}

bool TestGenericPackUsesColorPipelineControls() {
    GenericEquationPack pack = LoadPack();
    KernelParams baselineParams{};
    baselineParams.coloring_mode = ColoringMode::smooth_escape;
    baselineParams.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);

    KernelParams scaledParams = baselineParams;
    scaledParams.color_smooth_escape_scale = 3.0f;

    KernelParams phaseParams{};
    phaseParams.coloring_mode = ColoringMode::phase;
    phaseParams.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::phase);

    std::uint64_t baselineHash = 0;
    std::uint64_t scaledHash = 0;
    std::uint64_t phaseHash = 0;
    RenderStats stats{};
    if (!RenderPack(pack, baselineParams, &baselineHash, &stats)) return false;
    if (!RenderPack(pack, scaledParams, &scaledHash, &stats)) return false;
    if (!RenderPack(pack, phaseParams, &phaseHash, &stats)) return false;

    if (baselineHash == scaledHash) {
        std::cerr << "generic live render ignored Color Pipeline smooth_escape scale" << std::endl;
        return false;
    }
    if (baselineHash == phaseHash) {
        std::cerr << "generic live render ignored Color Pipeline palette/signal selection" << std::endl;
        return false;
    }
    return true;
}

} // namespace

int main() {
    if (!TestGenericPackRendersMainViewportFrame()) return 1;
    if (!TestGenericPackControlChangesMainViewportFrame()) return 1;
    if (!TestGenericPackUsesColorPipelineControls()) return 1;
    std::cout << "test_generic_equation_pack_live: passed" << std::endl;
    return 0;
}
