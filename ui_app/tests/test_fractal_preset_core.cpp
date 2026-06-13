#include "../src/fractal_preset_core.h"

#include "../src/fractal_derived_fields.h"
#include "../src/fractal_family_rules.h"

#include <cmath>
#include <iostream>
#include <string>

namespace {

int gPass = 0;
int gFail = 0;

#define CHECK(name, cond) do { \
    if (cond) { \
        ++gPass; \
    } else { \
        ++gFail; \
        std::cerr << "  FAIL: " << (name) << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
    } \
} while (0)

bool Near(float a, float b, float tol = 1.0e-5f) {
    return std::fabs(a - b) <= tol * (std::max)(1.0f, (std::max)(std::fabs(a), std::fabs(b)));
}

bool RoundTrip(const ViewState& view, const KernelParams& params, const LensSettings& lens,
    ViewState* outView, KernelParams* outParams, LensSettings* outLens) {
    std::string error;
    const std::string json = BuildFractalPresetCoreJson(view, params, lens, &error);
    CHECK("preset_core export produced json", !json.empty());
    CHECK("preset_core export reports no error", error.empty());
    const bool ok = ApplyFractalPresetCoreJson(json, outView, outParams, outLens, &error);
    if (!ok) std::cerr << "    error: " << error << "\n";
    return ok;
}

void TestLegacyFlatColorTupleRoundTrips() {
    ViewState view{};
    KernelParams params{};
    LensSettings lens{};
    view.fractal_type = FractalType::julia;
    params.color_pipeline.signal = ColorSignal::smooth_escape;
    params.color_shape = ColorPipelineShape::repeat;
    params.color_pipeline.palette = ColorPalette::cyclic_escape;
    params.color_pipeline.grading = ColorGradingPreset::escape_default;

    ViewState loadedView{};
    KernelParams loadedParams{};
    LensSettings loadedLens{};
    CHECK("legacy preset_core applies", RoundTrip(view, params, lens, &loadedView, &loadedParams, &loadedLens));
    CHECK("legacy fractal selector round-trips", loadedView.fractal_type == FractalType::julia);
    CHECK("legacy signal round-trips", loadedParams.color_pipeline.signal == ColorSignal::smooth_escape);
    CHECK("legacy shape round-trips", loadedParams.color_shape == ColorPipelineShape::repeat);
    CHECK("legacy palette round-trips", loadedParams.color_pipeline.palette == ColorPalette::cyclic_escape);
}

void TestGeneratedExplainoAuthorityRoundTrips() {
    ViewState view{};
    KernelParams params{};
    LensSettings lens{};
    view.fractal_type = FractalType::explaino;
    params.explaino_seed = 0.42;
    UpdateExplainoPolynomial(view, params, nullptr);

    ViewState loadedView{};
    KernelParams loadedParams{};
    LensSettings loadedLens{};
    CHECK("generated ExplainO preset_core applies", RoundTrip(view, params, lens, &loadedView, &loadedParams, &loadedLens));
    CHECK("generated ExplainO selector round-trips", loadedView.fractal_type == FractalType::explaino);
    CHECK("generated ExplainO authority round-trips", loadedParams.explaino_root_authority == ExplainoRootAuthority::generated);
    CHECK("generated ExplainO root count resolves", loadedParams.explaino_root_count == 4);
    CHECK("generated ExplainO seed round-trips", std::fabs(loadedParams.explaino_seed - params.explaino_seed) < 1.0e-12);
}

void TestRootSdfCustomAuthorityAndControlsRoundTrip() {
    ViewState view{};
    KernelParams params{};
    LensSettings lens{};
    view.fractal_type = FractalType::explaino_root_sdf;
    view.explaino_phase = 0.125f;
    lens.downsample = 4;
    params.explaino_root_authority = ExplainoRootAuthority::custom;
    params.explaino_root_count = 3;
    params.explaino_roots[0] = {-0.25f, 0.0f};
    params.explaino_roots[1] = {0.5f, 0.25f};
    params.explaino_roots[2] = {1.0f, -0.75f};
    params.explaino_root_sdf_radius = 0.22f;
    params.explaino_root_sdf_bridge_width = 0.03f;
    params.explaino_root_sdf_smooth_blend = 0.18f;
    params.explaino_root_sdf_h_source = ExplainoRootSdfHSource::phase_sine;
    params.explaino_root_sdf_h_amplitude = 0.12f;
    params.explaino_root_sdf_h_frequency = 2.0f;
    UpdateExplainoPolynomial(view, params, nullptr);

    ViewState loadedView{};
    KernelParams loadedParams{};
    LensSettings loadedLens{};
    CHECK("root-SDF custom preset_core applies", RoundTrip(view, params, lens, &loadedView, &loadedParams, &loadedLens));
    CHECK("root-SDF selector round-trips", loadedView.fractal_type == FractalType::explaino_root_sdf);
    CHECK("root-SDF stays field-primary not ExplainO family", !IsExplainoFamily(loadedView.fractal_type));
    CHECK("root-SDF custom authority round-trips", loadedParams.explaino_root_authority == ExplainoRootAuthority::custom);
    CHECK("root-SDF custom root count round-trips", loadedParams.explaino_root_count == 3);
    CHECK("root-SDF custom root order round-trips", Near(loadedParams.explaino_roots[1].x, 0.5f) && Near(loadedParams.explaino_roots[2].y, -0.75f));
    CHECK("root-SDF radius round-trips", Near(loadedParams.explaino_root_sdf_radius, 0.22f));
    CHECK("root-SDF h source round-trips", loadedParams.explaino_root_sdf_h_source == ExplainoRootSdfHSource::phase_sine);
    CHECK("root-SDF field downsample round-trips", loadedLens.downsample == 4);
}

void TestSdfSourceStackAuthorityRoundTrips() {
    ViewState view{};
    KernelParams params{};
    LensSettings lens{};
    view.fractal_type = FractalType::sdf_pack_scene;
    params.color_pipeline.signal = ColorSignal::sdf_signed_distance;
    params.color_shape = ColorPipelineShape::identity;
    params.color_pipeline.palette = ColorPalette::cyclic_escape;
    params.color_pipeline.grading = ColorGradingPreset::escape_default;
    params.color_source_stack_count = 2;
    params.color_source_stack[0].signal = ColorSignal::sdf_signed_distance;
    params.color_source_stack[0].params.scale = 0.5f;
    params.color_source_stack[0].params.bias = -0.25f;
    params.color_source_stack[0].params.sdf_gate = ColorPipelineSdfGateMode::boundary_band;
    params.color_source_stack[0].params.sdf_gate_width_px = 3.5f;
    params.color_source_stack[0].params.sdf_field_downsample = 2;
    params.color_source_stack[1].signal = ColorSignal::sdf_normal_angle;
    params.color_source_stack[1].params.blend_weight = 0.35f;
    params.color_source_stack[1].params.sdf_boundary_width_px = 6.0f;
    params.color_source_stack[1].params.sdf_sample_step = 3;

    ViewState loadedView{};
    KernelParams loadedParams{};
    LensSettings loadedLens{};
    CHECK("SDF source-stack preset_core applies", RoundTrip(view, params, lens, &loadedView, &loadedParams, &loadedLens));
    CHECK("SDF field-primary selector round-trips", loadedView.fractal_type == FractalType::sdf_pack_scene);
    CHECK("SDF source stack count round-trips", loadedParams.color_source_stack_count == 2);
    CHECK("SDF source stack row 0 signal round-trips", loadedParams.color_source_stack[0].signal == ColorSignal::sdf_signed_distance);
    CHECK("SDF source stack row 0 gate round-trips", loadedParams.color_source_stack[0].params.sdf_gate == ColorPipelineSdfGateMode::boundary_band);
    CHECK("SDF source stack row 0 downsample round-trips", loadedParams.color_source_stack[0].params.sdf_field_downsample == 2);
    CHECK("SDF source stack row 1 signal round-trips", loadedParams.color_source_stack[1].signal == ColorSignal::sdf_normal_angle);
    CHECK("SDF source stack row 1 sample step round-trips", loadedParams.color_source_stack[1].params.sdf_sample_step == 3);
    CHECK("SDF source stack row 1 blend round-trips", Near(loadedParams.color_source_stack[1].params.blend_weight, 0.35f));
}

} // namespace

int main() {
    TestLegacyFlatColorTupleRoundTrips();
    TestGeneratedExplainoAuthorityRoundTrips();
    TestRootSdfCustomAuthorityAndControlsRoundTrip();
    TestSdfSourceStackAuthorityRoundTrips();
    std::cout << "test_fractal_preset_core: " << gPass << " passed, " << gFail << " failed\n";
    return gFail == 0 ? 0 : 1;
}
