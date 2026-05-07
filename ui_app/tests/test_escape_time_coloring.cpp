#include "../src/escape_time_coloring.h"
#include "../src/fractal_family_rules.h"

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
    params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
    params.color_shape = ColorPipelineShape::identity;
    params.color_shape_offset = 0.0f;
    params.color_shape_scale = 1.0f;
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

    params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::iteration_count);
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

    params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::iteration_count);
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
    params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
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
    params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
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

    {
        params.color_pipeline = {ColorSignal::smooth_escape, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
        params.color_smooth_escape_scale = 1.0f;
        params.color_smooth_escape_bias = 0.0f;
        params.color_heatmap_cycle_scale = 1.0f;
        params.color_heatmap_saturation = 1.0f;
        params.color_contrast_lift_exposure = 1.0f;
        params.color_contrast_lift_saturation = 1.0f;

        const TestColor programmableBase = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::smooth_escape,
            true,
            10,
            100,
            TestComplex{4.0f, 0.0f},
            params);

        params.color_smooth_escape_scale = 2.0f;
        const TestColor scaledSignal = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::smooth_escape,
            true,
            10,
            100,
            TestComplex{4.0f, 0.0f},
            params);
        if (Equals(programmableBase, scaledSignal)) {
            std::cerr << "smooth_escape_ramp should react to its live scale owner field\n";
            return 1;
        }

        params.color_smooth_escape_scale = 1.0f;
        params.color_heatmap_cycle_scale = 2.0f;
        const TestColor fasterHeatmap = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::smooth_escape,
            true,
            10,
            100,
            TestComplex{4.0f, 0.0f},
            params);
        if (Equals(programmableBase, fasterHeatmap)) {
            std::cerr << "heatmap should react to its live cycle-scale owner field\n";
            return 1;
        }

        params.color_heatmap_cycle_scale = 1.0f;
        params.color_shape = ColorPipelineShape::offset_scale;
        params.color_shape_offset = 0.35f;
        params.color_shape_scale = 1.8f;
        const TestColor reshapedSignal = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::smooth_escape,
            true,
            10,
            100,
            TestComplex{4.0f, 0.0f},
            params);
        if (Equals(programmableBase, reshapedSignal)) {
            std::cerr << "offset_scale should react to its live Shape owner fields\n";
            return 1;
        }

        params.color_shape = ColorPipelineShape::repeat;
        params.color_shape_repeat_frequency = 6.0f;
        params.color_shape_repeat_phase = 0.2f;
        const TestColor repeatedSignal = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::smooth_escape,
            true,
            10,
            100,
            TestComplex{4.0f, 0.0f},
            params);
        if (Equals(programmableBase, repeatedSignal)) {
            std::cerr << "repeat should react to its live Shape owner fields\n";
            return 1;
        }

        params.color_shape = ColorPipelineShape::identity;
        params.color_shape_offset = 0.0f;
        params.color_shape_scale = 1.0f;
        params.color_shape_repeat_frequency = 8.0f;
        params.color_shape_repeat_phase = 0.0f;
        const TestColor gradedBase = ApplyFractalColorGrading(programmableBase, params);
        params.color_contrast_lift_exposure = 1.8f;
        params.color_contrast_lift_saturation = 1.5f;
        const TestColor liftedGrade = ApplyFractalColorGrading(programmableBase, params);
        if (Equals(gradedBase, liftedGrade)) {
            std::cerr << "contrast_lift should react to its live exposure and saturation owner fields\n";
            return 1;
        }
    }

    {
        params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::phase);
        const TestComplex phaseCoord{1.0f, 1.0f};
        const TestColor phaseBase = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::phase,
            true,
            12,
            100,
            phaseCoord,
            params);

        params.color_phase_signal_offset = 1.5707963f;
        const TestColor phaseSignalShift = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::phase,
            true,
            12,
            100,
            phaseCoord,
            params);
        if (Equals(phaseBase, phaseSignalShift)) {
            std::cerr << "Phase coloring should react to the live signal phase offset owner field\n";
            return 1;
        }

        params.color_phase_signal_offset = 0.0f;
        params.color_phase_wrap_cycles = 2.0f;
        const TestColor phaseWrapped = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::phase,
            true,
            12,
            100,
            phaseCoord,
            params);
        if (Equals(phaseBase, phaseWrapped)) {
            std::cerr << "Phase coloring should react to the live wrap-cycle owner field\n";
            return 1;
        }

        params.color_phase_wrap_cycles = 1.0f;
        params.color_phase_palette_offset = 1.0471976f;
        const TestColor phasePaletteShift = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::phase,
            true,
            12,
            100,
            phaseCoord,
            params);
        if (Equals(phaseBase, phasePaletteShift)) {
            std::cerr << "Phase coloring should react to the live palette phase offset owner field\n";
            return 1;
        }

        params.color_phase_palette_offset = 0.0f;
        params.color_shape = ColorPipelineShape::offset_scale;
        params.color_shape_offset = 0.2f;
        params.color_shape_scale = 1.4f;
        const TestColor phaseReshaped = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::phase,
            true,
            12,
            100,
            phaseCoord,
            params);
        if (Equals(phaseBase, phaseReshaped)) {
            std::cerr << "Phase coloring should react to the live offset_scale Shape owner fields\n";
            return 1;
        }

        params.color_shape = ColorPipelineShape::repeat;
        params.color_shape_repeat_frequency = 4.0f;
        params.color_shape_repeat_phase = -0.15f;
        const TestColor phaseRepeated = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::phase,
            true,
            12,
            100,
            phaseCoord,
            params);
        if (Equals(phaseBase, phaseRepeated)) {
            std::cerr << "Phase coloring should react to the live repeat Shape owner fields\n";
            return 1;
        }
    }

    {
        params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::iteration_bands);
        params.color_iteration_band_count = 8;
        params.color_iteration_band_softness = 0.0f;
        params.color_iteration_band_emphasis = 1.0f;
        params.color_iteration_band_palette_offset = 0.0f;
        const TestColor bandsBase = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::iteration_bands,
            true,
            31,
            100,
            TestComplex{4.0f, 0.0f},
            params);

        params.color_iteration_band_count = 3;
        const TestColor bandsCountShift = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::iteration_bands,
            true,
            31,
            100,
            TestComplex{4.0f, 0.0f},
            params);
        if (Equals(bandsBase, bandsCountShift)) {
            std::cerr << "Iteration-band coloring should react to the live band-count owner field\n";
            return 1;
        }

        params.color_iteration_band_count = 8;
        params.color_iteration_band_softness = 1.0f;
        const TestColor bandsSoftened = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::iteration_bands,
            true,
            31,
            100,
            TestComplex{4.0f, 0.0f},
            params);
        if (Equals(bandsBase, bandsSoftened)) {
            std::cerr << "Iteration-band coloring should react to the live band-softness owner field\n";
            return 1;
        }

        params.color_iteration_band_softness = 0.0f;
        params.color_iteration_band_emphasis = 1.8f;
        const TestColor bandsEmphasized = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::iteration_bands,
            true,
            31,
            100,
            TestComplex{4.0f, 0.0f},
            params);
        if (Equals(bandsBase, bandsEmphasized)) {
            std::cerr << "Iteration-band coloring should react to the live band-emphasis owner field\n";
            return 1;
        }

        params.color_iteration_band_emphasis = 1.0f;
        params.color_iteration_band_palette_offset = 1.5707963f;
        const TestColor bandsPaletteShift = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::iteration_bands,
            true,
            31,
            100,
            TestComplex{4.0f, 0.0f},
            params);
        if (Equals(bandsBase, bandsPaletteShift)) {
            std::cerr << "Iteration-band coloring should react to the live palette offset owner field\n";
            return 1;
        }
    }

    std::cout << "test_escape_time_coloring: all passed\n";
    return 0;
}