#include "../src/color_pipeline_core.h"
#include "../src/escape_time_coloring.h"
#include "../src/fractal_family_rules.h"

#include <cmath>
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

bool NearlyEqual(float left, float right, float eps = 1.0e-6f) {
    const float delta = left - right;
    return delta < eps && delta > -eps;
}

TestColor ApplyGlowHighlight(TestColor color, float glow) {
    const float clampedGlow = EscapeTimeColorClamp(glow, 0.0f, 2.0f);
    const auto applyChannel = [clampedGlow](unsigned char value) {
        const float normalized = static_cast<float>(value) / 255.0f;
        const float highlight = normalized * normalized;
        const float boosted = static_cast<float>(value) + (255.0f - static_cast<float>(value)) * highlight * (clampedGlow * 0.5f);
        return static_cast<unsigned char>(EscapeTimeColorClamp(boosted, 0.0f, 255.0f));
    };
    return {applyChannel(color.x), applyChannel(color.y), applyChannel(color.z), color.w};
}

TestColor ApplyFractalColorGlowFinishPassForTest(
    TestColor color,
    const KernelParams& params,
    float exposure,
    float saturation,
    float contrast,
    float glow) {
    return ApplyGlowHighlight(
        ApplyFractalColorGradingPass(color, params, exposure, saturation, contrast),
        glow);
}

float NormalizeChannel(unsigned char value) {
    return static_cast<float>(value) / 255.0f;
}

unsigned char QuantizeChannel(float value) {
    return static_cast<unsigned char>(EscapeTimeColorClamp(std::round(value * 255.0f), 0.0f, 255.0f));
}

TestColor ApplyBalanceVoidGradeForTest(
    TestColor color,
    float balanceVoid,
    float chromaTension,
    float accentBias) {
    const float clampedBalanceVoid = EscapeTimeColorClamp(balanceVoid, -1.0f, 1.0f);
    const float clampedChromaTension = EscapeTimeColorClamp(chromaTension, -1.0f, 1.0f);
    const float clampedAccentBias = EscapeTimeColorClamp(accentBias, -1.0f, 1.0f);

    const float red = NormalizeChannel(color.x);
    const float green = NormalizeChannel(color.y);
    const float blue = NormalizeChannel(color.z);
    const float luminance = red * 0.299f + green * 0.587f + blue * 0.114f;
    const float accent = (luminance - 0.5f) * 2.0f;
    const float chromaScale = 1.0f + clampedChromaTension * (0.35f + 0.35f * std::fabs(accent));
    const float warmShift = clampedBalanceVoid * (0.12f + 0.08f * (1.0f - std::fabs(accent)));
    const float accentLift = clampedAccentBias * accent * 0.18f;

    const float balancedRed = EscapeTimeColorClamp(
        luminance + (red - luminance) * chromaScale + warmShift + accentLift,
        0.0f,
        1.0f);
    const float balancedGreen = EscapeTimeColorClamp(
        luminance + (green - luminance) * (1.0f - clampedChromaTension * 0.18f) - accentLift * 0.35f,
        0.0f,
        1.0f);
    const float balancedBlue = EscapeTimeColorClamp(
        luminance + (blue - luminance) * chromaScale - warmShift - accentLift,
        0.0f,
        1.0f);

    return {QuantizeChannel(balancedRed), QuantizeChannel(balancedGreen), QuantizeChannel(balancedBlue), color.w};
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
        params.color_pipeline = {ColorSignal::smooth_escape, ColorPalette::explaino_cmap, ColorGradingPreset::escape_default};
        params.color_explaino_palette_seed_scale = 1.0f;
        params.color_explaino_palette_seed_phase = 0.0f;
        params.color_explaino_palette_colorfulness = 1.0f;
        const TestColor explainoBase = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::smooth_escape,
            true,
            10,
            100,
            TestComplex{4.0f, 0.0f},
            params);
        if (Equals(programmableBase, explainoBase)) {
            std::cerr << "explaino_cmap should not silently fall back to the shipped heatmap palette\n";
            return 1;
        }

        params.color_explaino_palette_seed_scale = 1.75f;
        const TestColor scaledExplaino = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::smooth_escape,
            true,
            10,
            100,
            TestComplex{4.0f, 0.0f},
            params);
        if (Equals(explainoBase, scaledExplaino)) {
            std::cerr << "explaino_cmap should react to its dedicated seed-scale owner field\n";
            return 1;
        }

        params.color_explaino_palette_seed_scale = 1.0f;
        params.color_explaino_palette_seed_phase = 0.25f;
        const TestColor shiftedExplaino = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::smooth_escape,
            true,
            10,
            100,
            TestComplex{4.0f, 0.0f},
            params);
        if (Equals(explainoBase, shiftedExplaino)) {
            std::cerr << "explaino_cmap should react to its dedicated seed-phase owner field\n";
            return 1;
        }

        params.color_explaino_palette_seed_phase = 0.0f;
        params.color_explaino_palette_colorfulness = 0.0f;
        const TestColor mutedExplaino = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::smooth_escape,
            true,
            10,
            100,
            TestComplex{4.0f, 0.0f},
            params);
        if (Equals(explainoBase, mutedExplaino)) {
            std::cerr << "explaino_cmap should react to its dedicated colorfulness owner field\n";
            return 1;
        }

        params.color_pipeline = {ColorSignal::smooth_escape, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
        params.color_explaino_palette_seed_scale = 1.0f;
        params.color_explaino_palette_seed_phase = 0.0f;
        params.color_explaino_palette_colorfulness = 1.0f;

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

        params.color_shape = ColorPipelineShape::mirror_repeat;
        params.color_shape_repeat_frequency = 6.0f;
        params.color_shape_repeat_phase = 0.2f;
        const TestColor mirrorRepeatedSignal = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::smooth_escape,
            true,
            10,
            100,
            TestComplex{4.0f, 0.0f},
            params);
        if (Equals(repeatedSignal, mirrorRepeatedSignal)) {
            std::cerr << "mirror_repeat should produce a different mirrored wave than repeat while reusing the same Shape owner fields\n";
            return 1;
        }

        params.color_shape = ColorPipelineShape::bias_gain_curve;
        params.color_shape_bias = 0.25f;
        params.color_shape_gain = 0.75f;
        const TestColor biasGainSignal = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::smooth_escape,
            true,
            10,
            100,
            TestComplex{4.0f, 0.0f},
            params);
        if (Equals(programmableBase, biasGainSignal)) {
            std::cerr << "bias_gain_curve should react to its dedicated live Shape owner fields\n";
            return 1;
        }
        params.color_shape_bias = 0.5f;
        params.color_shape_gain = 0.5f;
        const TestColor neutralBiasGain = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::smooth_escape,
            true,
            10,
            100,
            TestComplex{4.0f, 0.0f},
            params);
        if (!Equals(programmableBase, neutralBiasGain)) {
            std::cerr << "bias_gain_curve bias=gain=0.5 should preserve the incoming signal\n";
            return 1;
        }

        params.color_shape = ColorPipelineShape::smooth_window;
        params.color_shape_window_center = 0.35f;
        params.color_shape_window_width = 0.4f;
        params.color_shape_window_softness = 0.05f;
        const TestColor smoothWindowSignal = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::smooth_escape,
            true,
            10,
            100,
            TestComplex{4.0f, 0.0f},
            params);
        if (Equals(programmableBase, smoothWindowSignal)) {
            std::cerr << "smooth_window should react to its dedicated live Shape owner fields\n";
            return 1;
        }
        params.color_shape_window_center = 0.95f;
        params.color_shape_window_width = 0.2f;
        params.color_shape_window_softness = 0.0f;
        const float wrappedWindowValue = ApplyColorPipelineShapeValue(0.02f, params, 1.0f);
        if (wrappedWindowValue <= 0.0f) {
            std::cerr << "smooth_window should wrap across the shape domain seam when centered near the upper edge\n";
            return 1;
        }

        params.color_shape = ColorPipelineShape::identity;
        params.color_shape_stack_count = 2;
        params.color_shape_stack[0].shape = ColorPipelineShape::offset_scale;
        params.color_shape_stack[0].params.offset = 0.35f;
        params.color_shape_stack[0].params.scale = 1.8f;
        params.color_shape_stack[1].shape = ColorPipelineShape::repeat;
        params.color_shape_stack[1].params.repeat_frequency = 6.0f;
        params.color_shape_stack[1].params.repeat_phase = 0.2f;

        KernelParams offsetOnlyParams = params;
        offsetOnlyParams.color_shape_stack_count = 0;
        offsetOnlyParams.color_shape = ColorPipelineShape::offset_scale;
        offsetOnlyParams.color_shape_offset = 0.35f;
        offsetOnlyParams.color_shape_scale = 1.8f;
        KernelParams repeatOnlyParams = params;
        repeatOnlyParams.color_shape_stack_count = 0;
        repeatOnlyParams.color_shape = ColorPipelineShape::repeat;
        repeatOnlyParams.color_shape_repeat_frequency = 6.0f;
        repeatOnlyParams.color_shape_repeat_phase = 0.2f;
        const float stackedShapeValue = ApplyColorPipelineShapeValue(0.25f, params, 1.0f);
        const float repeatedOnlyValue = ApplyColorPipelineShapeValue(0.25f, repeatOnlyParams, 1.0f);
        const float manualStackedValue = ApplyRepeatShapeValue(
            ApplyColorPipelineShapeValue(0.25f, offsetOnlyParams, 1.0f),
            repeatOnlyParams,
            1.0f);
        if (std::fabs(stackedShapeValue - repeatedOnlyValue) <= 1.0e-6f) {
            std::cerr << "A two-row Shape stack should not collapse to the final Shape row only\n";
            return 1;
        }
        if (std::fabs(stackedShapeValue - manualStackedValue) > 1.0e-6f) {
            std::cerr << "A two-row Shape stack should execute supported Shape rows in order\n";
            return 1;
        }

        params.color_shape = ColorPipelineShape::posterize;
        params.color_shape_stack_count = 0;
        params.color_shape_posterize_steps = 2;
        params.color_shape_posterize_mix = 1.0f;
        const TestColor posterizedSignal = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::smooth_escape,
            true,
            10,
            100,
            TestComplex{4.0f, 0.0f},
            params);
        if (Equals(programmableBase, posterizedSignal)) {
            std::cerr << "posterize should react to its live Shape owner fields\n";
            return 1;
        }
        params.color_shape_posterize_mix = 0.0f;
        const TestColor unblendedPosterize = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::smooth_escape,
            true,
            10,
            100,
            TestComplex{4.0f, 0.0f},
            params);
        if (!Equals(programmableBase, unblendedPosterize)) {
            std::cerr << "posterize mix=0 should preserve the incoming signal\n";
            return 1;
        }

        params.color_shape = ColorPipelineShape::identity;
        params.color_shape_offset = 0.0f;
        params.color_shape_scale = 1.0f;
        params.color_shape_repeat_frequency = 8.0f;
        params.color_shape_repeat_phase = 0.0f;
        params.color_shape_posterize_steps = 6;
        params.color_shape_posterize_mix = 1.0f;
        const TestColor gradedBase = ApplyFractalColorGrading(programmableBase, params);
        params.color_contrast_lift_exposure = 1.8f;
        params.color_contrast_lift_saturation = 1.5f;
        const TestColor liftedGrade = ApplyFractalColorGrading(programmableBase, params);
        if (Equals(gradedBase, liftedGrade)) {
            std::cerr << "contrast_lift should react to its live exposure and saturation owner fields\n";
            return 1;
        }

        params.color_grading_stack_count = 2;
        params.color_grading_stack[0].grading = ColorGradingPreset::escape_default;
        params.color_grading_stack[0].params.exposure = 1.4f;
        params.color_grading_stack[0].params.saturation = 1.2f;
        params.color_grading_stack[0].params.contrast = 1.0f;
        params.color_grading_stack[1].grading = ColorGradingPreset::phase_default;
        params.color_grading_stack[1].params.exposure = 1.0f;
        params.color_grading_stack[1].params.saturation = 0.8f;
        params.color_grading_stack[1].params.contrast = 1.6f;
        const TestColor stackedGrade = ApplyFractalColorGrading(programmableBase, params);
        const TestColor manualStackedGrade = ApplyFractalColorGradingStackRow(
            ApplyFractalColorGradingStackRow(programmableBase, params, params.color_grading_stack[0]),
            params,
            params.color_grading_stack[1]);
        KernelParams finalGradeOnlyParams = params;
        finalGradeOnlyParams.color_grading_stack_count = 1;
        finalGradeOnlyParams.color_grading_stack[0] = params.color_grading_stack[1];
        const TestColor finalGradeOnly = ApplyFractalColorGrading(programmableBase, finalGradeOnlyParams);
        if (Equals(stackedGrade, finalGradeOnly)) {
            std::cerr << "A two-row Grading stack should not collapse to the final Grading row only\n";
            return 1;
        }
        if (!Equals(stackedGrade, manualStackedGrade)) {
            std::cerr << "A two-row Grading stack should execute supported Grading rows in order\n";
            return 1;
        }
        ColorGradingPreset neutralGrading = ColorGradingPreset::escape_default;
        if (!color_pipeline_core::TryParseAdvancedColorGradingFunctionId("neutral_finish", &neutralGrading)) {
            std::cerr << "neutral_finish should parse as a shipped Grading row id before runtime math proof\n";
            return 1;
        }
        params.exposure = 0.75f;
        params.color_saturation = 1.6f;
        params.color_contrast = 0.5f;
        params.color_grading_stack_count = 1;
        params.color_grading_stack[0].grading = neutralGrading;
        params.color_grading_stack[0].params.exposure = 1.25f;
        params.color_grading_stack[0].params.saturation = 0.85f;
        params.color_grading_stack[0].params.contrast = 1.4f;
        const TestColor neutralFinishGrade = ApplyFractalColorGrading(programmableBase, params);
        const TestColor neutralFinishExpected = ApplyFractalColorGradingPass(programmableBase, params, 1.25f, 0.85f, 1.4f);
        const TestColor legacyMirrorOnlyGrade = ApplyFractalColorGradingPass(programmableBase, params, 0.75f, 1.6f, 0.5f);
        if (Equals(neutralFinishGrade, legacyMirrorOnlyGrade)) {
            std::cerr << "neutral_finish should use its stack-entry exposure, saturation, and contrast values instead of the legacy mirror fallback\n";
            return 1;
        }
        if (!Equals(neutralFinishGrade, neutralFinishExpected)) {
            std::cerr << "neutral_finish should execute real runtime grading math through the shared grading pass\n";
            return 1;
        }
        ColorGradingPreset toneMapGrading = ColorGradingPreset::escape_default;
        if (!color_pipeline_core::TryParseAdvancedColorGradingFunctionId("tone_map_finish", &toneMapGrading)) {
            std::cerr << "tone_map_finish should parse as a shipped Grading row id before runtime math proof\n";
            return 1;
        }
        params.exposure = 0.65f;
        params.color_saturation = 1.4f;
        params.color_contrast = 0.55f;
        params.color_grading_stack_count = 1;
        params.color_grading_stack[0].grading = toneMapGrading;
        params.color_grading_stack[0].params.exposure = 1.35f;
        params.color_grading_stack[0].params.saturation = 0.75f;
        params.color_grading_stack[0].params.contrast = 1.6f;
        const TestColor toneMapFinishGrade = ApplyFractalColorGrading(programmableBase, params);
        const TestColor toneMapNeutralOrder = ApplyFractalColorGradingPass(programmableBase, params, 1.35f, 0.75f, 1.6f);
        const TestColor toneMapPreToneMap = ApplyFractalColorContrast(
            ApplyFractalColorSaturation(
                ApplyFractalColorTint(programmableBase, params),
                0.75f),
            1.6f);
        const TestColor toneMapExpected = EscapeTimeColorMake<TestColor>(
            EscapeTimeColorToneMap(toneMapPreToneMap.x, 1.35f),
            EscapeTimeColorToneMap(toneMapPreToneMap.y, 1.35f),
            EscapeTimeColorToneMap(toneMapPreToneMap.z, 1.35f),
            255);
        const TestColor toneMapLegacyMirrorOnly = ApplyFractalColorGradingPass(programmableBase, params, 0.65f, 1.4f, 0.55f);
        if (Equals(toneMapFinishGrade, toneMapLegacyMirrorOnly)) {
            std::cerr << "tone_map_finish should use its stack-entry exposure, saturation, and contrast values instead of the legacy mirror fallback\n";
            return 1;
        }
        if (Equals(toneMapFinishGrade, toneMapNeutralOrder)) {
            std::cerr << "tone_map_finish should not collapse to neutral_finish ordering; it needs its own runtime math branch\n";
            return 1;
        }
        if (!Equals(toneMapFinishGrade, toneMapExpected)) {
            std::cerr << "tone_map_finish should execute real runtime grading math by tone-mapping after tint, saturation, and contrast\n";
            return 1;
        }
        params.color_grading_stack_count = 0;

        ColorGradingPreset gradeGlowGrading = ColorGradingPreset::escape_default;
        if (!color_pipeline_core::TryParseAdvancedColorGradingFunctionId("grade_glow", &gradeGlowGrading)) {
            std::cerr << "grade_glow should parse as a shipped Grading row id before runtime math proof\n";
            return 1;
        }
        params.exposure = 0.7f;
        params.color_saturation = 1.4f;
        params.color_contrast = 0.6f;
        params.color_glow = 0.1f;
        params.color_grading_stack_count = 1;
        params.color_grading_stack[0].grading = gradeGlowGrading;
        params.color_grading_stack[0].params.exposure = 1.2f;
        params.color_grading_stack[0].params.saturation = 0.8f;
        params.color_grading_stack[0].params.contrast = 1.5f;
        params.color_grading_stack[0].params.glow = 0.6f;
        const TestColor gradeGlowFinishGrade = ApplyFractalColorGrading(programmableBase, params);
        const TestColor gradeGlowNeutralOrder = ApplyFractalColorGradingPass(programmableBase, params, 1.2f, 0.8f, 1.5f);
        const TestColor gradeGlowToneMapOrder = ApplyFractalColorToneMapFinishPass(programmableBase, params, 1.2f, 0.8f, 1.5f);
        const TestColor gradeGlowExpected = ApplyFractalColorGlowFinishPassForTest(programmableBase, params, 1.2f, 0.8f, 1.5f, 0.6f);
        const TestColor gradeGlowLegacyMirrorOnly = ApplyFractalColorGlowFinishPassForTest(programmableBase, params, 0.7f, 1.4f, 0.6f, 0.1f);
        if (Equals(gradeGlowFinishGrade, gradeGlowLegacyMirrorOnly)) {
            std::cerr << "grade_glow should use its stack-entry exposure, saturation, contrast, and glow values instead of the legacy mirror fallback\n";
            return 1;
        }
        if (Equals(gradeGlowFinishGrade, gradeGlowNeutralOrder) || Equals(gradeGlowFinishGrade, gradeGlowToneMapOrder)) {
            std::cerr << "grade_glow should not collapse to neutral_finish or tone_map_finish ordering; it needs its own runtime glow branch\n";
            return 1;
        }
        if (!Equals(gradeGlowFinishGrade, gradeGlowExpected)) {
            std::cerr << "grade_glow should execute real runtime grading math by adding controlled highlight bloom after the shared grading pass\n";
            return 1;
        }
        params.color_grading_stack_count = 0;

        ColorGradingPreset balanceVoidGrading = ColorGradingPreset::escape_default;
        if (!color_pipeline_core::TryParseAdvancedColorGradingFunctionId("balance_void_grade", &balanceVoidGrading)) {
            std::cerr << "balance_void_grade should parse as a shipped Grading row before runtime math proof\n";
            return 1;
        }
        params.exposure = 0.9f;
        params.color_saturation = 1.35f;
        params.color_contrast = 0.55f;
        params.color_grading_stack_count = 1;
        params.color_grading_stack[0].grading = balanceVoidGrading;
        params.color_grading_stack[0].params.balance_void = 0.35f;
        params.color_grading_stack[0].params.chroma_tension = 0.6f;
        params.color_grading_stack[0].params.accent_bias = -0.25f;
        const TestColor balanceVoidFinishGrade = ApplyFractalColorGrading(programmableBase, params);
        const TestColor balanceVoidNeutralOrder = ApplyFractalColorGradingPass(programmableBase, params, 0.9f, 1.35f, 0.55f);
        const TestColor balanceVoidExpected = ApplyBalanceVoidGradeForTest(programmableBase, 0.35f, 0.6f, -0.25f);
        if (Equals(balanceVoidFinishGrade, balanceVoidNeutralOrder)) {
            std::cerr << "balance_void_grade should not collapse to neutral_finish ordering; it needs its own runtime balance/void math branch\n";
            return 1;
        }
        if (!Equals(balanceVoidFinishGrade, balanceVoidExpected)) {
            std::cerr << "balance_void_grade should execute dedicated balance/void grading math through its balance_void, chroma_tension, and accent_bias owners\n";
            return 1;
        }
        params.color_grading_stack_count = 0;

        KernelParams smoothSourceParams = params;
        smoothSourceParams.color_source_stack_count = 0;
        smoothSourceParams.color_pipeline.signal = ColorSignal::smooth_escape;
        smoothSourceParams.color_smooth_escape_scale = 0.5f;
        smoothSourceParams.color_smooth_escape_bias = 0.25f;

        KernelParams magnitudeSourceParams = params;
        magnitudeSourceParams.color_source_stack_count = 0;
        magnitudeSourceParams.color_pipeline.signal = ColorSignal::escape_magnitude;
        magnitudeSourceParams.color_escape_magnitude_scale = 1.5f;
        magnitudeSourceParams.color_escape_magnitude_bias = -0.25f;

        KernelParams sourceStackParams = magnitudeSourceParams;
        sourceStackParams.color_source_stack_count = 2;
        sourceStackParams.color_source_stack[0].signal = ColorSignal::smooth_escape;
        sourceStackParams.color_source_stack[0].params.scale = 0.5f;
        sourceStackParams.color_source_stack[0].params.bias = 0.25f;
        sourceStackParams.color_source_stack[0].params.blend_weight = 1.0f;
        sourceStackParams.color_source_stack[1].signal = ColorSignal::escape_magnitude;
        sourceStackParams.color_source_stack[1].params.magnitude_scale = 1.5f;
        sourceStackParams.color_source_stack[1].params.magnitude_bias = -0.25f;
        sourceStackParams.color_source_stack[1].params.blend_weight = 0.25f;

        const TestComplex sourceCoord{4.0f, 1.0f};
        const float smoothSourceValue = ResolveProgrammableEscapeTimeSignal(
            FractalType::mandelbrot,
            10,
            100,
            sourceCoord,
            smoothSourceParams);
        const float magnitudeSourceValue = ResolveProgrammableEscapeTimeSignal(
            FractalType::mandelbrot,
            10,
            100,
            sourceCoord,
            magnitudeSourceParams);
        const float stackedSourceValue = ResolveProgrammableEscapeTimeSignal(
            FractalType::mandelbrot,
            10,
            100,
            sourceCoord,
            sourceStackParams);
        const float manualWeightedBlend = EscapeTimeColorLerp(smoothSourceValue, magnitudeSourceValue, 0.25f);
        if (std::fabs(stackedSourceValue - magnitudeSourceValue) <= 1.0e-6f) {
            std::cerr << "A two-row Source stack should not collapse to the final Source row only\n";
            return 1;
        }
        if (std::fabs(stackedSourceValue - manualWeightedBlend) > 1.0e-6f) {
            std::cerr << "A two-row Source stack should execute weighted blend math before Shape, Palette, and Grading\n";
            return 1;
        }

        KernelParams singleSourceStackParams = smoothSourceParams;
        singleSourceStackParams.color_source_stack_count = 1;
        singleSourceStackParams.color_source_stack[0].signal = ColorSignal::smooth_escape;
        singleSourceStackParams.color_source_stack[0].params.scale = 0.5f;
        singleSourceStackParams.color_source_stack[0].params.bias = 0.25f;
        singleSourceStackParams.color_source_stack[0].params.blend_weight = 1.0f;
        const float singleSourceStackValue = ResolveProgrammableEscapeTimeSignal(
            FractalType::mandelbrot,
            10,
            100,
            sourceCoord,
            singleSourceStackParams);
        if (std::fabs(singleSourceStackValue - smoothSourceValue) > 1.0e-6f) {
            std::cerr << "A one-row Source stack should preserve the existing shipped single-row Source behavior\n";
            return 1;
        }

        params.color_pipeline = {ColorSignal::smooth_escape, ColorPalette::explaino_cmap, ColorGradingPreset::escape_default};
        params.color_palette_stack_count = 2;
        params.color_palette_stack[0].palette = ColorPalette::cyclic_escape;
        params.color_palette_stack[0].params.cycle_scale = 1.0f;
        params.color_palette_stack[0].params.saturation = 1.0f;
        params.color_palette_stack[1].palette = ColorPalette::explaino_cmap;
        params.color_palette_stack[1].params.seed_scale = 1.0f;
        params.color_palette_stack[1].params.seed_phase = 0.25f;
        params.color_palette_stack[1].params.colorfulness = 0.8f;
        params.color_palette_stack[1].params.blend_weight = 0.35f;
        params.color_explaino_palette_seed_scale = 1.0f;
        params.color_explaino_palette_seed_phase = 0.25f;
        params.color_explaino_palette_colorfulness = 0.8f;

        const TestColor stackedPalette = SampleProgrammableEscapeTimePalette<TestColor>(0.37f, true, params);
        KernelParams finalPaletteOnlyParams = params;
        finalPaletteOnlyParams.color_palette_stack_count = 0;
        const TestColor finalPaletteOnly = SampleProgrammableEscapeTimePalette<TestColor>(0.37f, true, finalPaletteOnlyParams);
        const EscapeTimeColorRgb heatmapRgb = SampleEscapeHeatmap(0.37f, 1.0f);
        const EscapeTimeColorRgb explainoRgb = ApplyExplainoColorfulness(
            SampleExplainoSeedChannels(0.37f + 0.25f),
            0.8f);
        const TestColor manualStackedPalette = EscapeTimeColorFromRgb<TestColor>({
            EscapeTimeColorLerp(heatmapRgb.r, explainoRgb.r, 0.35f),
            EscapeTimeColorLerp(heatmapRgb.g, explainoRgb.g, 0.35f),
            EscapeTimeColorLerp(heatmapRgb.b, explainoRgb.b, 0.35f),
        });
        if (Equals(stackedPalette, finalPaletteOnly)) {
            std::cerr << "A two-row Palette stack should not collapse to the final Palette row only\n";
            return 1;
        }
        if (!Equals(stackedPalette, manualStackedPalette)) {
            std::cerr << "A two-row Palette stack should execute supported Palette rows with explicit RGB blend math\n";
            return 1;
        }
        params.color_palette_stack_count = 1;
        params.color_palette_stack[0] = {};
        params.color_palette_stack[0].palette = ColorPalette::phase_wheel;
        params.color_palette_stack[0].params.phase_offset = 0.0f;
        params.color_palette_stack[0].params.saturation = 0.0f;
        const TestColor phaseWheelDesaturated = SampleProgrammableEscapeTimePalette<TestColor>(0.37f, true, params);
        params.color_palette_stack[0].params.saturation = 2.0f;
        const TestColor phaseWheelSaturated = SampleProgrammableEscapeTimePalette<TestColor>(0.37f, true, params);
        if (Equals(phaseWheelDesaturated, phaseWheelSaturated)) {
            std::cerr << "A phase_wheel_palette stack row should render its persisted saturation parameter\n";
            return 1;
        }
        params.color_palette_stack_count = 0;
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

    {
        params.color_pipeline = {ColorSignal::escape_magnitude, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
        params.color_shape = ColorPipelineShape::identity;
        params.color_shape_offset = 0.0f;
        params.color_shape_scale = 1.0f;
        params.color_shape_repeat_frequency = 8.0f;
        params.color_shape_repeat_phase = 0.0f;
        params.color_escape_magnitude_scale = 1.0f;
        params.color_escape_magnitude_bias = 0.0f;
        const TestColor magnitudeBase = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::smooth_escape,
            true,
            10,
            100,
            TestComplex{4.0f, 0.0f},
            params);
        params.color_escape_magnitude_scale = 2.0f;
        const TestColor magnitudeScaled = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::smooth_escape,
            true,
            10,
            100,
            TestComplex{4.0f, 0.0f},
            params);
        if (Equals(magnitudeBase, magnitudeScaled)) {
            std::cerr << "escape_magnitude should react to its live magnitude-scale owner field\n";
            return 1;
        }

        const TestColor explainoResidualTight = MakeProgrammableBasinColor<TestColor>(
            FractalType::explaino,
            true,
            12,
            100,
            TestComplex{1.25f, -0.5f},
            1.0e-6f,
            params);
        const TestColor explainoResidualLoose = MakeProgrammableBasinColor<TestColor>(
            FractalType::explaino,
            true,
            12,
            100,
            TestComplex{1.25f, -0.5f},
            1.0e-3f,
            params);
        if (!Equals(explainoResidualTight, explainoResidualLoose)) {
            std::cerr << "Explaino escape_magnitude should sample orbit magnitude, not residual-sensitive basin exit state\n";
            return 1;
        }
    }

    {
        params.color_pipeline = {ColorSignal::orbit_stripe, ColorPalette::phase_wheel, ColorGradingPreset::phase_default};
        params.color_shape = ColorPipelineShape::identity;
        params.color_shape_offset = 0.0f;
        params.color_shape_scale = 1.0f;
        params.color_shape_repeat_frequency = 8.0f;
        params.color_shape_repeat_phase = 0.0f;
        params.color_orbit_stripe_frequency = 1.0f;
        params.color_orbit_stripe_phase = 0.0f;
        const TestColor stripeBase = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::phase,
            true,
            12,
            100,
            TestComplex{1.0f, 1.0f},
            params);
        params.color_orbit_stripe_frequency = 2.5f;
        const TestColor stripeFrequencyShift = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::phase,
            true,
            12,
            100,
            TestComplex{1.0f, 1.0f},
            params);
        if (Equals(stripeBase, stripeFrequencyShift)) {
            std::cerr << "orbit_stripe should react to its live stripe-frequency owner field\n";
            return 1;
        }
        const TestColor stripeInterior = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::mandelbrot,
            ColoringMode::phase,
            false,
            12,
            100,
            TestComplex{1.0f, 1.0f},
            params);
        if (Equals(stripeInterior, TestColor{0, 0, 0, 255})) {
            std::cerr << "orbit_stripe should preserve the phase-wheel interior color instead of forcing non-escaped samples to black\n";
            return 1;
        }
    }

    {
        params.poly_kind = PolyKind::z3_minus_1;
        params.color_pipeline = {ColorSignal::root_proximity, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
        params.color_shape = ColorPipelineShape::identity;
        params.color_shape_offset = 0.0f;
        params.color_shape_scale = 1.0f;
        params.color_shape_repeat_frequency = 8.0f;
        params.color_shape_repeat_phase = 0.0f;
        params.color_root_proximity_scale = 1.0f;
        params.color_root_proximity_bias = 0.0f;
        const TestColor nearRoot = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::newton,
            ColoringMode::smooth_escape,
            true,
            8,
            100,
            TestComplex{1.0f, 0.0f},
            params);
        const TestColor farFromRoot = MakeEscapeTimeBaseColor<TestColor>(
            FractalType::newton,
            ColoringMode::smooth_escape,
            true,
            8,
            100,
            TestComplex{0.0f, 0.0f},
            params);
        if (Equals(nearRoot, farFromRoot)) {
            std::cerr << "root_proximity should distinguish near-root and far-from-root samples on basin-capable families\n";
            return 1;
        }
    }

    {
        params.poly_kind = PolyKind::custom;
        params.explaino_root_count = 0;
        const float noRootSignal = ResolveRootProximitySignal(TestComplex{0.5f, -0.5f}, params);
        if (!NearlyEqual(noRootSignal, 0.0f, 1.0e-6)) {
            std::cerr << "root_proximity should fail closed to a zero signal when no roots are available\n";
            return 1;
        }
    }

    std::cout << "test_escape_time_coloring: all passed\n";
    return 0;
}