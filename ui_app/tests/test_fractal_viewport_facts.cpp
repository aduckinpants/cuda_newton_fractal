#include "fractal_viewport_facts.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <string>

namespace {

bool NearlyEqual(double actual, double expected, double tolerance = 1.0e-12) {
    return std::fabs(actual - expected) <= tolerance;
}

bool Require(bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << "\n";
        return false;
    }
    return true;
}

bool TestUnrotatedRectangularMapping() {
    ViewState view{};
    view.fractal_type = FractalType::mcmullen;
    view.center_hp_x = 1.0;
    view.center_hp_y = -2.0;
    view.log2_zoom = 1.0;
    view.rotation_degrees = 0.0f;
    RenderSettings render{};
    render.resolution = {400, 200};

    FractalViewportFacts facts{};
    std::string error;
    if (!ComputeFractalViewportFacts(view, render, &facts, &error)) {
        std::cerr << error << "\n";
        return false;
    }
    return
        Require(facts.width == 400 && facts.height == 200, "render dimensions mismatch") &&
        Require(NearlyEqual(facts.aspect_ratio, 2.0), "aspect mismatch") &&
        Require(NearlyEqual(facts.resolved_zoom, 2.0), "resolved zoom mismatch") &&
        Require(NearlyEqual(facts.local_half_width, 2.0), "local half width mismatch") &&
        Require(NearlyEqual(facts.local_half_height, 1.0), "local half height mismatch") &&
        Require(NearlyEqual(facts.pixel_step_x.real, 0.01), "pixel step x real mismatch") &&
        Require(NearlyEqual(facts.pixel_step_x.imag, 0.0), "pixel step x imag mismatch") &&
        Require(NearlyEqual(facts.pixel_step_y.real, 0.0), "pixel step y real mismatch") &&
        Require(NearlyEqual(facts.pixel_step_y.imag, 0.01), "pixel step y imag mismatch") &&
        Require(NearlyEqual(facts.continuous_edge_corners[0].real, -1.0), "top-left edge real mismatch") &&
        Require(NearlyEqual(facts.continuous_edge_corners[0].imag, -3.0), "top-left edge imag mismatch") &&
        Require(NearlyEqual(facts.continuous_edge_corners[2].real, 3.0), "bottom-right edge real mismatch") &&
        Require(NearlyEqual(facts.continuous_edge_corners[2].imag, -1.0), "bottom-right edge imag mismatch") &&
        Require(NearlyEqual(facts.pixel_center_corners[0].real, -0.995), "top-left pixel-center real mismatch") &&
        Require(NearlyEqual(facts.pixel_center_corners[0].imag, -2.995), "top-left pixel-center imag mismatch") &&
        Require(NearlyEqual(facts.axis_aligned_min.real, -1.0), "AABB minimum real mismatch") &&
        Require(NearlyEqual(facts.axis_aligned_max.imag, -1.0), "AABB maximum imag mismatch");
}

bool TestRotationUsesRendererOrientation() {
    ViewState view{};
    view.fractal_type = FractalType::julia;
    view.center_hp_x = 0.0;
    view.center_hp_y = 0.0;
    view.log2_zoom = 0.0;
    view.rotation_degrees = 90.0f;
    RenderSettings render{};
    render.resolution = {200, 100};

    FractalViewportFacts facts{};
    std::string error;
    if (!ComputeFractalViewportFacts(view, render, &facts, &error)) {
        std::cerr << error << "\n";
        return false;
    }
    return
        Require(NearlyEqual(facts.pixel_step_x.real, 0.0, 1.0e-9), "rotated x-step real mismatch") &&
        Require(NearlyEqual(facts.pixel_step_x.imag, 0.04, 1.0e-9), "rotated x-step imag mismatch") &&
        Require(NearlyEqual(facts.pixel_step_y.real, -0.04, 1.0e-9), "rotated y-step real mismatch") &&
        Require(NearlyEqual(facts.pixel_step_y.imag, 0.0, 1.0e-9), "rotated y-step imag mismatch") &&
        // Preserve the CUDA renderer's historical float CUDART_PI_F degree conversion.
        Require(NearlyEqual(facts.axis_aligned_min.real, -2.0000000486718577, 1.0e-12), "rotated AABB minimum real mismatch") &&
        Require(NearlyEqual(facts.axis_aligned_max.imag, 4.000000024335929, 1.0e-12), "rotated AABB maximum imag mismatch");
}

bool TestHighZoomAndDeterministicJson() {
    ViewState view{};
    view.fractal_type = FractalType::explaino_all;
    view.center_hp_x = 0.667589;
    view.center_hp_y = -0.042984;
    view.log2_zoom = 97.0;
    view.rotation_degrees = 13.0f;
    RenderSettings render{};
    render.resolution = {4096, 2560};

    FractalViewportFacts facts{};
    std::string error;
    if (!ComputeFractalViewportFacts(view, render, &facts, &error)) return false;
    if (!Require(std::isfinite(facts.local_half_width) && facts.local_half_width > 0.0, "high-zoom width must be positive and finite")) return false;

    std::string first;
    std::string second;
    if (!BuildFractalViewportFactsJson(view, render, &first, &error)) return false;
    if (!BuildFractalViewportFactsJson(view, render, &second, &error)) return false;
    return
        Require(first == second, "viewport JSON is not deterministic") &&
        Require(!first.empty() && first.back() == '\n', "viewport JSON must have one trailing newline") &&
        Require(first.find("\"schema_version\": 1") != std::string::npos, "viewport JSON schema missing") &&
        Require(first.find("\"mapping_id\": \"cuda_fractal_renderer_pixel_center_v1\"") != std::string::npos, "mapping id missing") &&
        Require(first.find("\"selected_fractal_type\": \"explaino_all\"") != std::string::npos, "selector id missing") &&
        Require(first.find("fit_log2_zoom = log2(min(2 * aspect / required_local_half_width, 2 / required_local_half_height))") != std::string::npos, "inverse fit equation missing");
}

bool TestInvalidInputsFailClosed() {
    ViewState view{};
    RenderSettings render{};
    FractalViewportFacts facts{};
    std::string error;

    render.resolution = {0, 100};
    if (!Require(!ComputeFractalViewportFacts(view, render, &facts, &error), "zero width must fail")) return false;
    render.resolution = {100, 100};
    view.log2_zoom = std::numeric_limits<double>::quiet_NaN();
    if (!Require(!ComputeFractalViewportFacts(view, render, &facts, &error), "nonfinite zoom must fail")) return false;
    view.log2_zoom = 0.0;
    view.center_hp_x = std::numeric_limits<double>::infinity();
    return Require(!ComputeFractalViewportFacts(view, render, &facts, &error), "nonfinite center must fail");
}

} // namespace

int main() {
    if (!TestUnrotatedRectangularMapping()) return 1;
    if (!TestRotationUsesRendererOrientation()) return 1;
    if (!TestHighZoomAndDeterministicJson()) return 1;
    if (!TestInvalidInputsFailClosed()) return 1;
    std::cout << "fractal_viewport_facts tests passed\n";
    return 0;
}
