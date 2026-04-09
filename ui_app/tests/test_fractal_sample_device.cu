// test_fractal_sample_device.cu
//
// K1 validation: after extracting fractal_sample_device() from kernel_render,
// the renderer must produce identical output for all fractal types.
// This test renders a 1x1 pixel for every FractalType and asserts:
//   (a) RenderFractalCUDA succeeds (no crash, no error)
//   (b) the pixel is not fully transparent (alpha != 0)
//   (c) FractalSampleResult has the expected fields and sizes

#include "../src/fractal_types.h"
#include "../src/fractal_sample_result.h"
#include "../src/fractal_family_rules.h"

#include <cstdint>
#include <cstring>
#include <iostream>

static int gPass = 0;
static int gFail = 0;

#define CHECK(name, cond) do { \
    if (cond) { \
        ++gPass; \
    } else { \
        ++gFail; \
        std::cerr << "  FAIL: " << (name) << "  (" << __FILE__ << ":" << __LINE__ << ")\n"; \
    } \
} while(0)

namespace {

// Verify FractalSampleResult struct is well-formed.
void TestSampleResultLayout() {
    FractalSampleResult r{};
    r.iterations = 42;
    r.final_z_x = 1.0f;
    r.final_z_y = -1.0f;
    r.residual = 0.001f;
    r.converged = true;
    r.escaped = false;

    CHECK("iterations field", r.iterations == 42);
    CHECK("final_z_x field", r.final_z_x == 1.0f);
    CHECK("final_z_y field", r.final_z_y == -1.0f);
    CHECK("residual field", r.residual == 0.001f);
    CHECK("converged field", r.converged == true);
    CHECK("escaped field", r.escaped == false);

    // Struct should be small enough to pass by value in a kernel.
    CHECK("sizeof FractalSampleResult <= 24", sizeof(FractalSampleResult) <= 24);
}

// Render one pixel of the given fractal type through the full pipeline.
// Returns true if RenderFractalCUDA succeeds and produces a non-transparent pixel.
bool RenderSinglePixel(FractalType ft, const char* name) {
    ViewState view{};
    view.fractal_type = ft;
    view.center_hp_x = 0.0;
    view.center_hp_y = 0.0;
    view.log2_zoom = 0.0;
    view.zoom = 1.0f;

    KernelParams params{};
    params.max_iter = 32;
    params.epsilon = 1e-6f;
    params.coloring_mode = DefaultColoringModeForFractal(ft);
    params.exposure = 1.0f;
    params.color_tint_r = 1.0f;
    params.color_tint_g = 1.0f;
    params.color_tint_b = 1.0f;
    params.color_saturation = 1.0f;
    params.color_contrast = 1.0f;

    // Apply family-specific defaults.
    if (IsExplainoFamily(ft)) {
        params.explaino_damping = 1.0f;
        params.explaino_warp_strength = 0.1f;
    }

    RenderSettings render{};
    render.resolution = {1, 1};
    render.block_size = 256;
    render.device_id = 0;
    render.sample_tier = SampleTier::fast;

    uint32_t pixel = 0u;
    const char* error = nullptr;
    RenderStats stats{};
    if (!RenderFractalCUDA(view, params, render, &pixel, nullptr, &stats, &error)) {
        std::cerr << "  RenderFractalCUDA FAILED for " << name
                  << ": " << (error ? error : "unknown") << "\n";
        return false;
    }

    unsigned char alpha = static_cast<unsigned char>((pixel >> 24) & 0xffu);
    if (alpha == 0) {
        std::cerr << "  Pixel fully transparent for " << name << "\n";
        return false;
    }
    return true;
}

} // namespace

int main() {
    TestSampleResultLayout();

    // Render every fractal type through the full pipeline.
    // After the K1 extraction, kernel_render must still produce valid output.
    const struct {
        FractalType type;
        const char* name;
    } types[] = {
        {FractalType::newton, "newton"},
        {FractalType::nova, "nova"},
        {FractalType::mandelbrot, "mandelbrot"},
        {FractalType::julia, "julia"},
        {FractalType::burning_ship, "burning_ship"},
        {FractalType::multibrot, "multibrot"},
        {FractalType::phoenix, "phoenix"},
        {FractalType::explaino, "explaino"},
        {FractalType::explaino_y, "explaino_y"},
        {FractalType::explaino_fp, "explaino_fp"},
        {FractalType::explaino_nova, "explaino_nova"},
        {FractalType::explaino_halley, "explaino_halley"},
        {FractalType::explaino_dual, "explaino_dual"},
        {FractalType::explaino_mult, "explaino_mult"},
        {FractalType::explaino_phoenix, "explaino_phoenix"},
        {FractalType::explaino_transcendental, "explaino_transcendental"},
        {FractalType::explaino_inertial, "explaino_inertial"},
        {FractalType::explaino_julia, "explaino_julia"},
        {FractalType::explaino_rational, "explaino_rational"},
        {FractalType::multicorn, "multicorn"},
        {FractalType::halley, "halley"},
        {FractalType::collatz, "collatz"},
        {FractalType::explaino_collatz, "explaino_collatz"},
        {FractalType::mcmullen, "mcmullen"},
        {FractalType::lambda_map, "lambda_map"},
        {FractalType::explaino_lambda, "explaino_lambda"},
        {FractalType::explaino_rational_escape, "explaino_rational_escape"},
        {FractalType::spider, "spider"},
        {FractalType::celtic_mandelbrot, "celtic_mandelbrot"},
        {FractalType::perpendicular_burning_ship, "perpendicular_burning_ship"},
        {FractalType::explaino_joy, "explaino_joy"},
        {FractalType::explaino_fold, "explaino_fold"},
        {FractalType::explaino_bell, "explaino_bell"},
        {FractalType::explaino_ripple, "explaino_ripple"},
        {FractalType::explaino_splice, "explaino_splice"},
        {FractalType::explaino_vortex, "explaino_vortex"},
        {FractalType::explaino_tension, "explaino_tension"},
    };

    for (const auto& tc : types) {
        bool ok = RenderSinglePixel(tc.type, tc.name);
        CHECK(tc.name, ok);
    }

    CleanupFractalCUDA();
    std::cout << "test_fractal_sample_device: passed=" << gPass
              << " failed=" << gFail << "\n";
    return (gFail == 0) ? 0 : 1;
}
