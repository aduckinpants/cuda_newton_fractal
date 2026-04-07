#include <cstdio>
#include <cstdlib>
#include "sample_tier_resolver.h"

static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__); \
        g_fail = 1; \
    } \
} while(0)

static void test_all_families_support_fast() {
    // Every family must support fast (float32 direct).
    for (int i = 0; i <= (int)FractalType::explaino_rational_escape; ++i) {
        auto ft = (FractalType)i;
        uint32_t flags = GetSampleTierSupport(ft);
        CHECK(flags & kSupport_Fast, "every family should support fast");
    }
}

static void test_newton_supports_standard() {
    uint32_t flags = GetSampleTierSupport(FractalType::newton);
    CHECK(flags & kSupport_Standard, "newton should support standard (float64)");
}

static void test_resolve_fast_always_float32() {
    auto r = ResolveSampleEvalMode(FractalType::newton, SampleTier::fast, 50.0);
    CHECK(r.backend == NumericBackend::float32, "fast tier must resolve to float32");
    CHECK(r.strategy == IterationStrategy::direct, "fast tier must resolve to direct");
}

static void test_resolve_standard_gives_float64() {
    auto r = ResolveSampleEvalMode(FractalType::newton, SampleTier::standard, 5.0);
    CHECK(r.backend == NumericBackend::float64, "standard tier must resolve to float64");
    CHECK(r.strategy == IterationStrategy::direct, "standard tier must resolve to direct");
}

static void test_auto_shallow_stays_float32() {
    auto r = ResolveSampleEvalMode(FractalType::newton, SampleTier::tier_auto, 10.0);
    CHECK(r.backend == NumericBackend::float32, "auto at shallow zoom should stay float32");
}

static void test_auto_deep_upgrades_to_float64() {
    auto r = ResolveSampleEvalMode(FractalType::newton, SampleTier::tier_auto, 25.0);
    CHECK(r.backend == NumericBackend::float64, "auto at deep zoom should upgrade to float64");
}

static void test_auto_boundary_exactly_20() {
    // At exactly 20, not yet deep: should stay float32.
    auto r = ResolveSampleEvalMode(FractalType::newton, SampleTier::tier_auto, 20.0);
    CHECK(r.backend == NumericBackend::float32, "auto at zoom=20 boundary should stay float32");
}

static void test_mandelbrot_auto_upgrades_before_20() {
    auto r = ResolveSampleEvalMode(FractalType::mandelbrot, SampleTier::tier_auto, 18.6);
    CHECK(r.backend == NumericBackend::float64, "mandelbrot auto should upgrade before the old global threshold");
}

static void test_julia_auto_upgrades_before_20() {
    auto r = ResolveSampleEvalMode(FractalType::julia, SampleTier::tier_auto, 17.0);
    CHECK(r.backend == NumericBackend::float64, "julia auto should upgrade before the old global threshold");
}

static void test_escape_time_also_gets_standard() {
    uint32_t flags = GetSampleTierSupport(FractalType::mandelbrot);
    CHECK(flags & kSupport_Standard, "mandelbrot should also support standard");
}

int main() {
    test_all_families_support_fast();
    test_newton_supports_standard();
    test_resolve_fast_always_float32();
    test_resolve_standard_gives_float64();
    test_auto_shallow_stays_float32();
    test_auto_deep_upgrades_to_float64();
    test_auto_boundary_exactly_20();
    test_mandelbrot_auto_upgrades_before_20();
    test_julia_auto_upgrades_before_20();
    test_escape_time_also_gets_standard();

    if (g_fail) {
        fprintf(stderr, "sample_tier_resolver: SOME TESTS FAILED\n");
        return 1;
    }
    printf("sample_tier_resolver: all tests passed\n");
    return 0;
}
