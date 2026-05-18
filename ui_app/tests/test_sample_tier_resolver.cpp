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

static KernelParams SmoothEscapeParams() {
    KernelParams params{};
    params.coloring_mode = ColoringMode::smooth_escape;
    params.color_pipeline = {ColorSignal::smooth_escape, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
    return params;
}

static KernelParams RootProximityParams() {
    KernelParams params{};
    params.coloring_mode = ColoringMode::smooth_escape;
    params.color_pipeline = {ColorSignal::root_proximity, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
    return params;
}

static void test_render_auto_promotes_basin_smooth_escape() {
    KernelParams params = SmoothEscapeParams();
    auto r = ResolveSampleEvalModeForRender(FractalType::explaino, params, SampleTier::tier_auto, 10.0);
    CHECK(r.backend == NumericBackend::float64, "render auto should promote basin smooth_escape to float64");
    CHECK(r.strategy == IterationStrategy::direct, "render auto smooth_escape promotion should stay direct");
}

static void test_render_auto_promotes_basin_root_proximity() {
    KernelParams params = RootProximityParams();
    auto result = ResolveSampleEvalModeForRender(FractalType::explaino, params, SampleTier::tier_auto, 10.0);
    CHECK(result.backend == NumericBackend::float64, "render auto should promote basin root_proximity to float64");
    CHECK(result.strategy == IterationStrategy::direct, "render auto root_proximity promotion should stay direct");
}

static void test_render_auto_does_not_promote_basin_root_index() {
    KernelParams params{};
    params.coloring_mode = ColoringMode::smooth_escape;
    params.color_pipeline = {ColorSignal::root_index, ColorPalette::root_classic, ColorGradingPreset::basin_default};
    auto r = ResolveSampleEvalModeForRender(FractalType::explaino, params, SampleTier::tier_auto, 10.0);
    CHECK(r.backend == NumericBackend::float32, "render auto should not promote unrelated basin root_index coloring");
}

static void test_render_context_preserves_explicit_fast() {
    KernelParams params = RootProximityParams();
    auto r = ResolveSampleEvalModeForRender(FractalType::explaino, params, SampleTier::fast, 10.0);
    CHECK(r.backend == NumericBackend::float32, "render context must preserve explicit fast tier");
}

static void test_render_auto_keeps_explaino_ripple_smooth_escape_fast_when_owner_axis_is_active() {
    KernelParams params = SmoothEscapeParams();
    params.ripple_amplitude = 0.15f;
    auto r = ResolveSampleEvalModeForRender(FractalType::explaino_ripple, params, SampleTier::tier_auto, 10.0);
    CHECK(r.backend == NumericBackend::float32, "render auto should keep owner-active explaino_ripple smooth_escape on float32");
}

static void test_render_auto_keeps_explaino_balance_void_smooth_escape_fast_when_owner_axes_are_active() {
    KernelParams params = SmoothEscapeParams();
    params.balance_void = 0.35f;
    params.symmetry_tension = -0.2f;
    params.field_curvature = 0.25f;
    auto r = ResolveSampleEvalModeForRender(FractalType::explaino_balance_void, params, SampleTier::tier_auto, 10.0);
    CHECK(r.backend == NumericBackend::float32, "render auto should keep owner-active explaino_balance_void smooth_escape on float32");
}

static void test_render_auto_still_promotes_neutral_explaino_ripple_without_owner_axis() {
    KernelParams params = SmoothEscapeParams();
    auto r = ResolveSampleEvalModeForRender(FractalType::explaino_ripple, params, SampleTier::tier_auto, 10.0);
    CHECK(r.backend == NumericBackend::float64, "neutral explaino_ripple should inherit baseline explaino smooth_escape float64 promotion");
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
    test_render_auto_promotes_basin_smooth_escape();
    test_render_auto_promotes_basin_root_proximity();
    test_render_auto_does_not_promote_basin_root_index();
    test_render_context_preserves_explicit_fast();
    test_render_auto_keeps_explaino_ripple_smooth_escape_fast_when_owner_axis_is_active();
    test_render_auto_keeps_explaino_balance_void_smooth_escape_fast_when_owner_axes_are_active();
    test_render_auto_still_promotes_neutral_explaino_ripple_without_owner_axis();
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
