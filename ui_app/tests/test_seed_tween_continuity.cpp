#include "../src/explaino_seed.h"
#include "../src/explaino_seed_curve.h"
#include "../src/fractal_derived_fields.h"

#include <cmath>
#include <cstdio>

static float MaxRootDelta(const KernelParams& a, const KernelParams& b) {
    float maxD = 0.0f;
    for (int i = 0; i < 4; ++i) {
        float dx = std::fabs(a.explaino_roots[i].x - b.explaino_roots[i].x);
        float dy = std::fabs(a.explaino_roots[i].y - b.explaino_roots[i].y);
        if (dx > maxD) maxD = dx;
        if (dy > maxD) maxD = dy;
    }
    return maxD;
}

static float MaxCoeffDelta(const KernelParams& a, const KernelParams& b) {
    float maxD = 0.0f;
    for (int i = 0; i < 5; ++i) {
        float d = std::fabs(a.poly_coeffs[i] - b.poly_coeffs[i]);
        if (d > maxD) maxD = d;
    }
    return maxD;
}

static void DumpRoots(const char* label, double seed, const KernelParams& p) {
    printf("  %s (seed=%.6f): roots=[(%.6f,%.6f),(%.6f,%.6f),(%.6f,%.6f),(%.6f,%.6f)]\n",
        label, seed,
        p.explaino_roots[0].x, p.explaino_roots[0].y,
        p.explaino_roots[1].x, p.explaino_roots[1].y,
        p.explaino_roots[2].x, p.explaino_roots[2].y,
        p.explaino_roots[3].x, p.explaino_roots[3].y);
    printf("         coeffs=[%.6f, %.6f, %.6f, %.6f, %.6f]\n",
        p.poly_coeffs[0], p.poly_coeffs[1], p.poly_coeffs[2],
        p.poly_coeffs[3], p.poly_coeffs[4]);
}

int main() {
    // Simulate auto-increment: seed moves by tiny delta each frame.
    // At ~60fps with rate=1.0, that's ~0.016 per frame.
    // Use even smaller steps to be strict.
    const double seedStart = 5.0;
    const double steps[] = {0.001, 0.01, 0.05, 0.10, 0.25, 0.50, 0.75, 0.99};
    const int numSteps = sizeof(steps) / sizeof(steps[0]);

    printf("=== Seed tween continuity test ===\n\n");

    // First: show H(t) values to verify the curve shape
    printf("Wedge tween H(t) values:\n");
    for (int i = 0; i <= 10; ++i) {
        double t = i * 0.1;
        printf("  H(%.1f) = %.9f\n", t, ExplainoWedgeTween(t));
    }
    printf("\n");

    // Base frame at integer seed
    ViewState viewBase{};
    KernelParams paramsBase{};
    viewBase.fractal_type = FractalType::explaino;
    viewBase.explaino_seed_tween = true;
    ExplainoSeedSetCombined(viewBase, paramsBase, seedStart);
    UpdateExplainoPolynomial(viewBase, paramsBase, nullptr);
    DumpRoots("BASE", seedStart, paramsBase);

    printf("\n--- Tiny increments from seed %.1f ---\n", seedStart);

    int failures = 0;
    KernelParams paramsPrev = paramsBase;
    double prevSeed = seedStart;

    for (int i = 0; i < numSteps; ++i) {
        double seed = seedStart + steps[i];

        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::explaino;
        view.explaino_seed_tween = true;
        ExplainoSeedSetCombined(view, params, seed);
        UpdateExplainoPolynomial(view, params, nullptr);

        float rootDelta = MaxRootDelta(paramsBase, params);
        float coeffDelta = MaxCoeffDelta(paramsBase, params);
        float stepRootDelta = MaxRootDelta(paramsPrev, params);

        DumpRoots("STEP", seed, params);
        printf("    vs BASE: max_root_delta=%.9f  max_coeff_delta=%.9f\n", rootDelta, coeffDelta);
        printf("    vs PREV: max_root_delta=%.9f  (step=%.4f)\n", stepRootDelta, steps[i] - (i > 0 ? steps[i-1] : 0.0));

        // KEY CHECK: A 0.001 seed increment should NOT move roots by more than
        // a small fraction. If roots move by >0.1 for a 0.001 step, something
        // is catastrophically wrong.
        if (steps[i] <= 0.01 && rootDelta > 0.05f) {
            printf("    *** FAIL: tiny seed change (%.4f) caused huge root motion (%.9f) ***\n", steps[i], rootDelta);
            failures++;
        }

        paramsPrev = params;
        prevSeed = seed;
    }

    // Also test crossing an integer boundary: 5.999 -> 6.001
    printf("\n--- Integer boundary crossing: 5.999 -> 6.001 ---\n");
    {
        ViewState viewA{}, viewB{};
        KernelParams paramsA{}, paramsB{};
        viewA.fractal_type = FractalType::explaino;
        viewB.fractal_type = FractalType::explaino;
        viewA.explaino_seed_tween = true;
        viewB.explaino_seed_tween = true;
        ExplainoSeedSetCombined(viewA, paramsA, 5.999);
        ExplainoSeedSetCombined(viewB, paramsB, 6.001);
        UpdateExplainoPolynomial(viewA, paramsA, nullptr);
        UpdateExplainoPolynomial(viewB, paramsB, nullptr);

        DumpRoots("5.999", 5.999, paramsA);
        DumpRoots("6.001", 6.001, paramsB);

        float rootDelta = MaxRootDelta(paramsA, paramsB);
        float coeffDelta = MaxCoeffDelta(paramsA, paramsB);
        printf("    boundary delta: max_root=%.9f  max_coeff=%.9f\n", rootDelta, coeffDelta);

        if (rootDelta > 0.05f) {
            printf("    *** FAIL: integer boundary crossing caused discontinuity (%.9f) ***\n", rootDelta);
            failures++;
        }
    }

    printf("\n=== Result: %d failures ===\n", failures);
    return failures > 0 ? 1 : 0;
}
