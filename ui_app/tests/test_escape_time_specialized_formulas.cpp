#include "../src/escape_time_specialized_formulas.h"

#include <iostream>

namespace {

struct Cx {
    float x;
    float y;
};

bool NearlyEqual(float left, float right, float eps = 1.0e-6f) {
    const float delta = left - right;
    return delta < eps && delta > -eps;
}

bool NearlyEqualCx(Cx left, Cx right, float eps = 1.0e-6f) {
    return NearlyEqual(left.x, right.x, eps) && NearlyEqual(left.y, right.y, eps);
}

} // namespace

int main() {
    if (!UsesSpecializedEscapeTimeFormula(FractalType::mcmullen) ||
        !UsesSpecializedEscapeTimeFormula(FractalType::collatz)) {
        std::cerr << "Expected specialized escape-time helper to cover McMullen and Collatz\n";
        return 1;
    }
    if (UsesSpecializedEscapeTimeFormula(FractalType::mandelbrot) ||
        UsesSpecializedEscapeTimeFormula(FractalType::phoenix) ||
        UsesSpecializedEscapeTimeFormula(FractalType::newton)) {
        std::cerr << "Specialized escape-time helper should stay limited to McMullen and Collatz in this slice\n";
        return 1;
    }

    {
        const McMullenPresetConfig config = ResolveMcMullenPresetConfig(McMullenPreset::z2_z2);
        if (config.m != 2 || config.n != 2 || !NearlyEqual(config.lambda, -0.10f)) {
            std::cerr << "Expected z2/z2 McMullen preset to resolve deterministically\n";
            return 1;
        }
    }

    {
        const McMullenPresetConfig config = ResolveMcMullenPresetConfig(McMullenPreset::z3_z3);
        if (config.m != 3 || config.n != 3 || !NearlyEqual(config.lambda, -0.125f)) {
            std::cerr << "Expected default McMullen preset to resolve deterministically\n";
            return 1;
        }
    }

    {
        KernelParams params{};
        params.mcmullen_preset = McMullenPreset::z4_z2;
        params.mcmullen_m = 6;
        params.mcmullen_n = 7;
        params.mcmullen_lambda = 0.75f;
        const McMullenPresetConfig config = ResolveMcMullenRuntimeConfig(params);
        if (config.m != 4 || config.n != 2 || !NearlyEqual(config.lambda, -0.05f)) {
            std::cerr << "Expected non-custom McMullen runtime config to keep preset authority\n";
            return 1;
        }
    }

    {
        KernelParams params{};
        params.mcmullen_preset = McMullenPreset::custom;
        params.mcmullen_m = 5;
        params.mcmullen_n = 1;
        params.mcmullen_lambda = 0.25f;
        const McMullenPresetConfig config = ResolveMcMullenRuntimeConfig(params);
        if (config.m != 5 || config.n != 1 || !NearlyEqual(config.lambda, 0.25f)) {
            std::cerr << "Expected custom McMullen runtime config to use direct controls\n";
            return 1;
        }
    }

    {
        KernelParams params{};
        params.mcmullen_preset = McMullenPreset::custom;
        params.mcmullen_m = -4;
        params.mcmullen_n = 40;
        params.mcmullen_lambda = 3.0f;
        const McMullenPresetConfig config = ResolveMcMullenRuntimeConfig(params);
        if (config.m != 2 || config.n != 8 || !NearlyEqual(config.lambda, 1.0f)) {
            std::cerr << "Expected custom McMullen runtime config to clamp unsafe direct ranges\n";
            return 1;
        }
    }

    {
        const McMullenPresetConfig config = ResolveMcMullenPresetConfig(McMullenPreset::z3_z3);
        Cx z{1.0f, 0.0f};
        const SpecializedEscapeStepResult result = StepMcMullenEscapeState(config, &z);
        if (result != SpecializedEscapeStepResult::advanced || !NearlyEqualCx(z, {0.875f, 0.0f})) {
            std::cerr << "McMullen step should compute z^m + lambda / z^n for the resolved preset\n";
            return 1;
        }
    }

    {
        const McMullenPresetConfig config = ResolveMcMullenPresetConfig(McMullenPreset::z4_z2);
        Cx z{0.0f, 0.0f};
        const SpecializedEscapeStepResult result = StepMcMullenEscapeState(config, &z);
        if (result != SpecializedEscapeStepResult::pole) {
            std::cerr << "McMullen step should surface a pole result when the orbit is too close to zero\n";
            return 1;
        }
    }

    {
        Cx z{1.0f, 0.0f};
        StepCollatzEscapeState(&z);
        if (!NearlyEqualCx(z, {4.0f, 0.0f})) {
            std::cerr << "Collatz step should compute the smooth complex Collatz map deterministically\n";
            return 1;
        }
    }

    {
        Cx canonical{1.0f, 0.0f};
        Cx explicitDefault{1.0f, 0.0f};
        Cx zeroTransition{1.0f, 0.0f};
        StepCollatzEscapeState(&canonical);
        StepCollatzEscapeState(&explicitDefault, 1.0f);
        StepCollatzEscapeState(&zeroTransition, 0.0f);
        if (!NearlyEqualCx(canonical, explicitDefault)) {
            std::cerr << "Collatz transition strength default should preserve canonical map semantics\n";
            return 1;
        }
        if (NearlyEqualCx(canonical, zeroTransition)) {
            std::cerr << "Collatz transition strength should alter the cosine transition term\n";
            return 1;
        }
    }

    if (!NearlyEqual(SpecializedEscapeRadiusSquared(), 10000.0f)) {
        std::cerr << "Specialized escape-time helper should use the 10000 escape-radius contract\n";
        return 1;
    }

    std::cout << "test_escape_time_specialized_formulas: all passed\n";
    return 0;
}
