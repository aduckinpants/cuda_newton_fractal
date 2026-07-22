#pragma once

#include "fractal_types.h"

#include <array>
#include <string>

struct FractalViewportComplexPoint {
    double real{0.0};
    double imag{0.0};
};

struct FractalViewportFacts {
    int width{0};
    int height{0};
    double aspect_ratio{0.0};

    FractalType fractal_type{FractalType::explaino};
    double center_hp_x{0.0};
    double center_hp_y{0.0};
    double log2_zoom{0.0};
    double resolved_zoom{0.0};
    double rotation_degrees{0.0};

    double local_half_width{0.0};
    double local_half_height{0.0};
    double local_full_width{0.0};
    double local_full_height{0.0};

    FractalViewportComplexPoint pixel_step_x{};
    FractalViewportComplexPoint pixel_step_y{};
    double complex_units_per_pixel_x{0.0};
    double complex_units_per_pixel_y{0.0};

    std::array<FractalViewportComplexPoint, 4> continuous_edge_corners{};
    std::array<FractalViewportComplexPoint, 4> pixel_center_corners{};
    FractalViewportComplexPoint axis_aligned_min{};
    FractalViewportComplexPoint axis_aligned_max{};
};

bool ComputeFractalViewportFacts(
    const ViewState& view,
    const RenderSettings& render,
    FractalViewportFacts* outFacts,
    std::string* outError);

// Deterministic UTF-8 JSON with a single trailing newline.
bool BuildFractalViewportFactsJson(
    const ViewState& view,
    const RenderSettings& render,
    std::string* outJson,
    std::string* outError);

// Uses the established same-directory .tmp replacement convention.
bool WriteFractalViewportFactsJsonFile(
    const std::string& path,
    const ViewState& view,
    const RenderSettings& render,
    std::string* outError);
