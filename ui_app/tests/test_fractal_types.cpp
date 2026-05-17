#include "../src/fractal_types.h"

#include <cmath>
#include <cstring>
#include <iostream>

namespace {

int g_passed = 0;
int g_failed = 0;

void Check(bool condition, const char* message) {
    if (condition) {
        ++g_passed;
    } else {
        ++g_failed;
        std::cerr << "FAIL: " << message << "\n";
    }
}

bool Near(float actual, float expected, float tolerance = 1.0e-6f) {
    return std::fabs(actual - expected) <= tolerance;
}

bool NearDouble(double actual, double expected, double tolerance = 1.0e-12) {
    return std::fabs(actual - expected) <= tolerance;
}

void TestVectorAggregates() {
    const Float2 floatPoint{1.25f, -2.5f};
    Check(Near(floatPoint.x, 1.25f) && Near(floatPoint.y, -2.5f), "Float2 preserves aggregate coordinates");

    const Int2 intPoint{2048, 1536};
    Check(intPoint.x == 2048 && intPoint.y == 1536, "Int2 preserves aggregate coordinates");

    const Double2 doublePoint{0.125, -0.75};
    Check(NearDouble(doublePoint.x, 0.125) && NearDouble(doublePoint.y, -0.75),
        "Double2 preserves aggregate coordinates");
}

void TestEnumOrdinalContracts() {
    Check(static_cast<int>(PolyKind::z3_minus_1) == 0, "PolyKind z3_minus_1 ordinal is stable");
    Check(static_cast<int>(PolyKind::custom) == 2, "PolyKind custom ordinal is stable");
    Check(static_cast<int>(TerminationKind::none) == 0, "TerminationKind none ordinal is stable");
    Check(static_cast<int>(TerminationKind::nonfinite) == 5, "TerminationKind nonfinite ordinal is stable");
    Check(static_cast<int>(ColoringMode::root_basin) == 0, "ColoringMode root_basin ordinal is stable");
    Check(static_cast<int>(ColoringMode::iteration_bands) == 5, "ColoringMode iteration_bands ordinal is stable");
    Check(static_cast<int>(ColorSignal::root_index) == 0, "ColorSignal root_index ordinal is stable");
    Check(static_cast<int>(ColorSignal::root_proximity) == 7, "ColorSignal root_proximity ordinal is stable");
    Check(static_cast<int>(ColorPalette::root_classic) == 0, "ColorPalette root_classic ordinal is stable");
    Check(static_cast<int>(ColorPalette::explaino_cmap) == 5, "ColorPalette explaino_cmap ordinal is stable");
    Check(static_cast<int>(ColorGradingPreset::basin_default) == 0, "ColorGradingPreset basin_default ordinal is stable");
    Check(static_cast<int>(ColorGradingPreset::bands_default) == 3, "ColorGradingPreset bands_default ordinal is stable");
    Check(static_cast<int>(ColorPipelineShape::identity) == 0, "ColorPipelineShape identity ordinal is stable");
    Check(static_cast<int>(ColorPipelineShape::smooth_window) == 6, "ColorPipelineShape smooth_window ordinal is stable");
    Check(static_cast<int>(TranscendentalFunc::f_sin) == 0, "TranscendentalFunc f_sin ordinal is stable");
    Check(static_cast<int>(TranscendentalFunc::f_cosh) == 2, "TranscendentalFunc f_cosh ordinal is stable");
    Check(static_cast<int>(McMullenPreset::z3_z3) == 0, "McMullenPreset z3_z3 ordinal is stable");
    Check(static_cast<int>(McMullenPreset::z3_z2) == 3, "McMullenPreset z3_z2 ordinal is stable");
    Check(static_cast<int>(FractalType::newton) == 0, "FractalType newton ordinal is stable");
    Check(static_cast<int>(FractalType::explaino) == 7, "FractalType explaino ordinal is stable");
    Check(static_cast<int>(FractalType::lambda_map) == 24, "FractalType lambda_map ordinal is stable");
    Check(static_cast<int>(FractalType::explaino_tension) == 36, "FractalType explaino_tension ordinal is stable");
    Check(static_cast<int>(FractalType::counterfactual_pair) == 39, "FractalType counterfactual_pair ordinal is stable");
    Check(static_cast<int>(FractalType::explaino_counterfactual_pair) == 40,
        "FractalType explaino_counterfactual_pair ordinal is stable");
    Check(static_cast<int>(FractalType::projection_and_flow) == 41,
        "FractalType projection_and_flow ordinal is stable");
    Check(static_cast<int>(FractalType::explaino_projection_and_flow) == 42,
        "FractalType explaino_projection_and_flow ordinal is stable");
    Check(static_cast<int>(CounterfactualPairRootFamily::cubic_unit_roots) == 0, "CounterfactualPairRootFamily cubic ordinal is stable");
    Check(static_cast<int>(CounterfactualPairRootFamily::quartic_unit_roots) == 1, "CounterfactualPairRootFamily quartic ordinal is stable");
    Check(static_cast<int>(CounterfactualPairFrame::world_absolute) == 0, "CounterfactualPairFrame world-absolute ordinal is stable");
    Check(static_cast<int>(CounterfactualPairFrame::view_relative) == 1, "CounterfactualPairFrame view-relative ordinal is stable");
    Check(static_cast<int>(ProjectionAndFlowRootFamily::cubic_unit_roots) == 0, "ProjectionAndFlowRootFamily cubic ordinal is stable");
    Check(static_cast<int>(ProjectionAndFlowRootFamily::quartic_unit_roots) == 1, "ProjectionAndFlowRootFamily quartic ordinal is stable");
    Check(static_cast<int>(SampleTier::tier_auto) == 0, "SampleTier auto ordinal is stable");
    Check(static_cast<int>(SampleTier::standard) == 2, "SampleTier standard ordinal is stable");
    Check(static_cast<int>(NumericBackend::float32) == 0, "NumericBackend float32 ordinal is stable");
    Check(static_cast<int>(NumericBackend::float64) == 1, "NumericBackend float64 ordinal is stable");
    Check(static_cast<int>(IterationStrategy::direct) == 0, "IterationStrategy direct ordinal is stable");
    Check(kSupport_Fast == (1u << 0), "Fast tier support bit is stable");
    Check(kSupport_Standard == (1u << 1), "Standard tier support bit is stable");
    Check((kSupport_Fast | kSupport_Standard) == 3u, "Sample tier support bits compose predictably");
    Check(static_cast<int>(CameraBehavior::manual) == 0, "CameraBehavior manual ordinal is stable");
    Check(static_cast<int>(CameraBehavior::off) == 4, "CameraBehavior off ordinal is stable");
}

void TestColorPipelineDefaults() {
    Check(kColorPipelineMaxShapeStackCount == 8, "Shape stack maximum is stable");
    Check(kColorPipelineMaxRootBasinPairCount == 8, "Root-basin pair maximum is stable");

    const ColorPipelineShapeRuntimeParams params{};
    Check(Near(params.offset, 0.0f) && Near(params.scale, 1.0f), "Shape params default offset and scale");
    Check(Near(params.repeat_frequency, 8.0f) && Near(params.repeat_phase, 0.0f),
        "Shape params default repeat controls");
    Check(params.posterize_steps == 6 && Near(params.posterize_mix, 1.0f),
        "Shape params default posterize controls");
    Check(Near(params.bias, 0.5f) && Near(params.gain, 0.5f), "Shape params default bias/gain controls");
    Check(Near(params.window_center, 0.5f) && Near(params.window_width, 1.0f) && Near(params.window_softness, 0.0f),
        "Shape params default window controls");

    const ColorPipelineShapeStackEntry entry{};
    Check(entry.shape == ColorPipelineShape::identity, "Shape stack entry defaults to identity");
    Check(Near(entry.params.scale, 1.0f), "Shape stack entry carries default params");

    const ColorPipelineSelection selection{};
    Check(selection.signal == ColorSignal::root_index, "Pipeline selection defaults to root-index signal");
    Check(selection.palette == ColorPalette::root_classic, "Pipeline selection defaults to root-classic palette");
    Check(selection.grading == ColorGradingPreset::basin_default, "Pipeline selection defaults to basin grading");
}

void TestViewStateDefaults() {
    const ViewState view{};
    Check(Near(view.center.x, 0.0f) && Near(view.center.y, 0.0f), "ViewState center defaults to origin");
    Check(Near(view.zoom, 1.0f) && Near(view.rotation_degrees, 0.0f), "ViewState zoom and rotation defaults are stable");
    Check(!view.auto_refresh, "ViewState auto_refresh defaults off");
    Check(NearDouble(view.center_hp_x, 0.0) && NearDouble(view.center_hp_y, 0.0) && NearDouble(view.log2_zoom, 0.0),
        "ViewState high precision fields default to origin and unit zoom");
    Check(view.fractal_type == FractalType::explaino, "ViewState defaults to explaino fractal type");
    Check(!view.explaino_alive && view.explaino_seed_tween, "ViewState explaino life and tween defaults are stable");
    Check(Near(view.explaino_phase, 0.0f) && Near(view.explaino_seed_drift, 0.0f),
        "ViewState explaino phase defaults are stable");
    Check(!view.auto_increment_seed && Near(view.explaino_seed_rate, 0.001f),
        "ViewState seed increment defaults are stable");
    Check(Near(view.explaino_phase_strength, 1.0f), "ViewState phase strength defaults to one");
    Check(std::strcmp(view.param_anim_target, "none") == 0, "ViewState param animation target defaults to none");
    Check(Near(view.param_anim_rate, 0.001f), "ViewState param animation rate defaults are stable");
    Check(!view.auto_max_iter, "ViewState auto max-iter defaults off");
    Check(view.camera_behavior == CameraBehavior::complexity, "ViewState camera behavior defaults to complexity");
    Check(!view.auto_dive && Near(view.dive_speed, 1.0f), "ViewState dive controls default off at unit speed");
}

void TestKernelParamsDefaults() {
    const KernelParams params{};
    Check(params.max_iter == 500 && Near(params.epsilon, 1.0e-6f), "KernelParams iteration defaults are stable");
    Check(Near(params.nova_alpha, 0.50f), "KernelParams nova alpha default is stable");
    Check(Near(params.phoenix_p_real, 0.0f) && Near(params.phoenix_p_imag, 0.0f),
        "KernelParams phoenix defaults are stable");
    Check(params.poly_kind == PolyKind::z3_minus_1, "KernelParams polynomial kind defaults to z^3-1");
    Check(Near(params.poly_coeffs[0], -1.0f) && Near(params.poly_coeffs[3], 1.0f) && Near(params.poly_coeffs[4], 0.0f),
        "KernelParams polynomial coefficients default to z^3-1");
    Check(params.multibrot_power == 3 && Near(params.multibrot_power_float, 3.0f),
        "KernelParams multibrot power defaults are stable");
    Check(Near(params.lambda_real, 2.9685855f) && Near(params.lambda_imag, -0.27446103f),
        "KernelParams lambda defaults are stable");
    Check(params.coloring_mode == ColoringMode::root_basin, "KernelParams coloring mode defaults to root basin");
    Check(params.color_pipeline.signal == ColorSignal::root_index && params.color_pipeline.palette == ColorPalette::root_classic,
        "KernelParams color pipeline selection defaults are stable");
    Check(Near(params.exposure, 1.0f), "KernelParams exposure defaults to one");
    Check(Near(params.color_saturation, 1.15f) && Near(params.color_contrast, 1.10f),
        "KernelParams grading defaults are stable");
    Check(Near(params.color_tint_r, 1.0f) && Near(params.color_tint_g, 1.0f) && Near(params.color_tint_b, 1.0f),
        "KernelParams color tint defaults are stable");
    Check(params.color_shape == ColorPipelineShape::identity && params.color_shape_stack_count == 0,
        "KernelParams shape stack defaults to empty identity");
    Check(params.color_shape_stack[0].shape == ColorPipelineShape::identity &&
            params.color_shape_stack[kColorPipelineMaxShapeStackCount - 1].shape == ColorPipelineShape::identity,
        "KernelParams shape stack entries default to identity");
    Check(params.color_root_basin_pair_count == 0 &&
            params.color_root_basin_pairs[0].signal == ColorSignal::root_index,
        "KernelParams root basin pairs default empty with root-index entries");
    Check(Near(params.color_shape_scale, 1.0f) && Near(params.color_shape_repeat_frequency, 8.0f),
        "KernelParams legacy shape controls keep stable defaults");
    Check(params.color_shape_posterize_steps == 6 && Near(params.color_shape_posterize_mix, 1.0f),
        "KernelParams posterize controls keep stable defaults");
    Check(params.color_iteration_band_count == 8 && Near(params.color_iteration_band_softness, 0.35f),
        "KernelParams iteration-band controls keep stable defaults");
    Check(Near(params.color_smooth_escape_scale, 1.0f) && Near(params.color_escape_magnitude_scale, 1.0f),
        "KernelParams color source scale defaults are stable");
    Check(Near(params.color_orbit_stripe_frequency, 1.0f) && Near(params.color_root_proximity_scale, 1.0f),
        "KernelParams advanced color signal defaults are stable");
    Check(NearDouble(params.explaino_seed, 0.0) && NearDouble(params.explaino_seed_b, 1.0),
        "KernelParams explaino seed defaults are stable");
    Check(Near(params.explaino_mix, 0.5f) && Near(params.explaino_warp_strength, 0.0f),
        "KernelParams explaino mix and warp defaults are stable");
    Check(Near(params.explaino_root_spread, 0.5f) && Near(params.explaino_damping, 1.0f),
        "KernelParams explaino root spread and damping defaults are stable");
    Check(params.explaino_root_count == 0 && Near(params.explaino_roots[3].x, 0.0f) && Near(params.explaino_roots[3].y, 0.0f),
        "KernelParams explaino roots default empty and zeroed");
    Check(params.transcendental_func == TranscendentalFunc::f_sin, "KernelParams transcendental function defaults to sine");
    Check(Near(params.momentum_beta, 0.0f) && Near(params.joy_coupling, 0.0f) && Near(params.fold_coupling, 0.0f),
        "KernelParams coupling defaults are stable");
    Check(Near(params.bell_coupling, 0.0f) && Near(params.ripple_amplitude, 0.0f) && Near(params.splice_offset, 0.0f),
        "KernelParams variant defaults are stable");
    Check(Near(params.poly_coeffs_b[0], 0.0f) && Near(params.poly_coeffs_b[4], 0.0f),
        "KernelParams splice polynomial defaults are zeroed");
    Check(Near(params.vortex_strength, 0.0f) && Near(params.tension_strength, 0.0f),
        "KernelParams vortex and tension defaults are stable");
    Check(params.counterfactual_pair_root_family == CounterfactualPairRootFamily::cubic_unit_roots, "KernelParams Counterfactual Pair root family defaults to cubic unit roots");
    Check(params.counterfactual_pair_frame == CounterfactualPairFrame::world_absolute, "KernelParams Counterfactual Pair frame defaults to world-absolute");
    Check(Near(params.counterfactual_pair_offset_x, 0.16f) && Near(params.counterfactual_pair_offset_y, 0.08f),
        "KernelParams Counterfactual Pair offsets default to the shipped baseline gap");
    Check(Near(params.counterfactual_pair_reconvergence_ratio, 0.60f),
        "KernelParams Counterfactual Pair reconvergence ratio default is stable");
    Check(params.projection_and_flow_root_family == ProjectionAndFlowRootFamily::cubic_unit_roots,
        "KernelParams Projection-and-Flow root family defaults to cubic unit roots");
    Check(Near(params.projection_and_flow_target_radius, 1.0f),
        "KernelParams Projection-and-Flow target radius defaults to the unit circle");
    Check(Near(params.projection_and_flow_pressure_threshold, 1.0f),
        "KernelParams Projection-and-Flow pressure threshold default is stable");
    Check(params.mcmullen_preset == McMullenPreset::z3_z3, "KernelParams McMullen preset defaults to z3_z3");
}

void TestRenderAndStatsDefaults() {
    const RenderSettings render{};
    Check(RenderSettings::kDefaultWidth == 2048 && RenderSettings::kDefaultHeight == 1536,
        "RenderSettings static resolution defaults are stable");
    Check(render.resolution.x == 2048 && render.resolution.y == 1536, "RenderSettings resolution defaults are stable");
    Check(render.block_size == 256 && render.device_id == 0, "RenderSettings block and device defaults are stable");
    Check(!render.benchmark, "RenderSettings benchmark defaults off");
    Check(render.interaction_debounce_ms == RenderSettings::kDefaultInteractionDebounceMs,
        "RenderSettings debounce default uses static constant");
    Check(Near(render.preview_target_fps, 30.0f) && Near(render.preview_min_scale, 0.50f),
        "RenderSettings preview defaults are stable");
    Check(render.sample_tier == SampleTier::tier_auto, "RenderSettings sample tier defaults to auto");
    Check(render.resolved_eval.backend == NumericBackend::float32 && render.resolved_eval.strategy == IterationStrategy::direct,
        "RenderSettings resolved eval defaults to float32 direct");

    const LensSettings lens{};
    Check(!lens.enabled && lens.downsample == 2, "LensSettings defaults to disabled two-times downsample");

    const RenderStats stats{};
    Check(Near(stats.last_render_ms, 0.0f) && stats.last_iters_avg == 0 && stats.last_device_id == 0,
        "RenderStats timing and device defaults are stable");
    Check(stats.resolved_eval.backend == NumericBackend::float32 && stats.resolved_eval.strategy == IterationStrategy::direct,
        "RenderStats resolved eval defaults to float32 direct");
}

void TestOrbitTerminationDefaults() {
    const OrbitTerminationConfig config{};
    Check(config.enable_root_convergence, "Orbit termination root convergence defaults on");
    Check(config.enable_escape_radius, "Orbit termination escape radius defaults on");
    Check(!config.enable_far_field_settled, "Orbit termination far-field settled defaults off");
    Check(config.far_field_min_iter == 12, "Orbit termination far-field min iter default is stable");
    Check(Near(config.far_field_epsilon, 1.0e-4f), "Orbit termination far-field epsilon default is stable");
    Check(Near(config.far_field_min_r2, 16.0f), "Orbit termination far-field radius default is stable");
    Check(Near(config.denominator_floor, 1.0e-12f, 1.0e-15f), "Orbit termination denominator floor default is stable");
}

} // namespace

int main() {
    TestVectorAggregates();
    TestEnumOrdinalContracts();
    TestColorPipelineDefaults();
    TestViewStateDefaults();
    TestKernelParamsDefaults();
    TestRenderAndStatsDefaults();
    TestOrbitTerminationDefaults();

    std::cout << "test_fractal_types: passed=" << g_passed << " failed=" << g_failed << "\n";
    return g_failed == 0 ? 0 : 1;
}
