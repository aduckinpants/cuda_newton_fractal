#include "../src/escape_time_coloring.h"

#include <iostream>

namespace {

struct TestColor {
    unsigned char x;
    unsigned char y;
    unsigned char z;
    unsigned char w;
};

struct TestComplex {
    float x;
    float y;
};

bool Equals(TestColor left, TestColor right) {
    return left.x == right.x && left.y == right.y && left.z == right.z && left.w == right.w;
}

} // namespace

int main() {
    KernelParams params{};
    params.exposure = 1.0f;
    params.color_tint_r = 1.0f;
    params.color_tint_g = 1.0f;
    params.color_tint_b = 1.0f;
    params.color_saturation = 1.0f;
    params.color_contrast = 1.0f;
    params.multibrot_power_float = 4.0f;

    if (!Equals(MakeEscapeTimeBaseColor<TestColor>(FractalType::mandelbrot,
            ColoringMode::root_basin,
            true,
            10,
            100,
            TestComplex{4.0f, 0.0f},
            params),
            TestColor{255, 0, 255, 255})) {
        std::cerr << "Escape-time basin modes should remain explicit error colors\n";
        return 1;
    }

    if (!Equals(MakeEscapeTimeBaseColor<TestColor>(FractalType::mandelbrot,
            ColoringMode::iteration_count,
            false,
            10,
            100,
            TestComplex{4.0f, 0.0f},
            params),
            TestColor{0, 0, 0, 255})) {
        std::cerr << "Unescaped escape-time pixels should stay black\n";
        return 1;
    }

    if (!Equals(MakeEscapeTimeBaseColor<TestColor>(FractalType::mandelbrot,
            ColoringMode::iteration_count,
            true,
            50,
            100,
            TestComplex{4.0f, 0.0f},
            params),
            TestColor{64, 127, 128, 255})) {
        std::cerr << "Iteration-count coloring should preserve the current escape-time ramp\n";
        return 1;
    }

    params.multibrot_power_float = 3.0f;
    if (!Equals(MakeEscapeTimeBaseColor<TestColor>(FractalType::mandelbrot,
            ColoringMode::smooth_escape,
            true,
            10,
            100,
            TestComplex{4.0f, 0.0f},
            params),
            TestColor{15, 114, 178, 255})) {
        std::cerr << "Smooth-escape coloring should preserve the Mandelbrot palette sample\n";
        return 1;
    }

    params.multibrot_power_float = 4.0f;
    if (!Equals(MakeEscapeTimeBaseColor<TestColor>(FractalType::multibrot,
            ColoringMode::smooth_escape,
            true,
            9,
            100,
            TestComplex{2.0f, 0.0f},
            params),
            TestColor{16, 121, 181, 255})) {
        std::cerr << "Smooth-escape coloring should respect the multibrot power denominator\n";
        return 1;
    }

    if (!Equals(ApplyFractalColorGrading(TestColor{100, 150, 200, 255}, params), TestColor{82, 113, 138, 255})) {
        std::cerr << "Fractal color grading should preserve the current exposure/tint/saturation/contrast pipeline\n";
        return 1;
    }

    std::cout << "test_escape_time_coloring: all passed\n";
    return 0;
}