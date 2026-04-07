#include "../src/fractal_types.h"

#include <cstdint>
#include <iostream>

namespace {

struct ExpectedColor {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

bool RenderNewtonPixelAt(double x, double y, uint32_t* outPixel) {
    if (!outPixel) return false;

    ViewState view{};
    view.fractal_type = FractalType::newton;
    view.center_hp_x = x;
    view.center_hp_y = y;
    view.center.x = static_cast<float>(x);
    view.center.y = static_cast<float>(y);
    view.log2_zoom = 0.0;
    view.zoom = 1.0f;

    KernelParams params{};
    params.max_iter = 32;
    params.coloring_mode = ColoringMode::root_basin;
    params.exposure = 1.0f;
    params.color_tint_r = 1.0f;
    params.color_tint_g = 1.0f;
    params.color_tint_b = 1.0f;
    params.color_saturation = 1.0f;
    params.color_contrast = 1.0f;

    RenderSettings render{};
    render.resolution = {1, 1};
    render.block_size = 256;
    render.device_id = 0;
    render.sample_tier = SampleTier::fast;

    const char* error = nullptr;
    RenderStats stats{};
    if (!RenderFractalCUDA(view, params, render, outPixel, nullptr, &stats, &error)) {
        std::cerr << "RenderFractalCUDA failed for Newton basin regression: " << (error ? error : "unknown") << "\n";
        return false;
    }
    return true;
}

ExpectedColor Decode(uint32_t rgba) {
    return {
        static_cast<unsigned char>(rgba & 0xffu),
        static_cast<unsigned char>((rgba >> 8) & 0xffu),
        static_cast<unsigned char>((rgba >> 16) & 0xffu),
        static_cast<unsigned char>((rgba >> 24) & 0xffu),
    };
}

bool Equals(ExpectedColor left, ExpectedColor right) {
    return left.r == right.r && left.g == right.g && left.b == right.b && left.a == right.a;
}

} // namespace

int main() {
    const struct {
        double x;
        double y;
        ExpectedColor expected;
    } cases[] = {
        {1.0, 0.0, {56, 100, 161, 255}},
        {-0.5, 0.8660254037844386, {161, 56, 56, 255}},
        {-0.5, -0.8660254037844386, {56, 161, 56, 255}},
    };

    for (const auto& testCase : cases) {
        uint32_t pixel = 0u;
        if (!RenderNewtonPixelAt(testCase.x, testCase.y, &pixel)) {
            CleanupFractalCUDA();
            return 1;
        }
        const ExpectedColor decoded = Decode(pixel);
        if (!Equals(decoded, testCase.expected)) {
            std::cerr << "Newton basin regression mismatch at (" << testCase.x << ", " << testCase.y << ")"
                      << " expected rgba=(" << (int)testCase.expected.r << ',' << (int)testCase.expected.g << ','
                      << (int)testCase.expected.b << ',' << (int)testCase.expected.a << ")"
                      << " got rgba=(" << (int)decoded.r << ',' << (int)decoded.g << ','
                      << (int)decoded.b << ',' << (int)decoded.a << ")\n";
            CleanupFractalCUDA();
            return 1;
        }
    }

    CleanupFractalCUDA();
    std::cout << "test_newton_basin_regression: all passed\n";
    return 0;
}