#pragma once

#include <cmath>
#include <cstdint>

struct Float2 {
    float x;
    float y;
};

struct Int2 {
    int x;
    int y;
};

struct Double2 {
    double x;
    double y;
};

enum class PolyKind : int {
    z3_minus_1 = 0,
    z4_minus_1 = 1,
    custom = 2,
};

enum class TerminationKind : uint8_t {
    none = 0,
    root_converged = 1,
    escaped_radius = 2,
    far_field_settled = 3,
    max_iterations = 4,
    nonfinite = 5,
};

struct OrbitTerminationConfig {
    bool enable_root_convergence{true};
    bool enable_escape_radius{true};
    bool enable_far_field_settled{false};
    int far_field_min_iter{12};
    float far_field_epsilon{1.0e-4f};
    float far_field_min_r2{16.0f};
    float denominator_floor{1.0e-12f};
};

enum class ColoringMode : int {
    root_basin = 0,
    iteration_count = 1,
    smooth_escape = 2,
    joy_basins = 3,
    phase = 4,
    iteration_bands = 5,
};

enum class ColorSignal : int {
    root_index = 0,
    iteration_count = 1,
    smooth_escape = 2,
    phase_angle = 3,
    iteration_bands = 4,
    escape_magnitude = 5,
    orbit_stripe = 6,
    root_proximity = 7,
};

enum class ColorPalette : int {
    root_classic = 0,
    joy = 1,
    cyclic_escape = 2,
    phase_wheel = 3,
    banded_escape = 4,
    explaino_cmap = 5,
};

enum class ColorGradingPreset : int {
    basin_default = 0,
    escape_default = 1,
    phase_default = 2,
    bands_default = 3,
    neutral_default = 4,
    tone_map_default = 5,
    glow_default = 6,
    balance_void_default = 7,
};

enum class ColorPipelineShape : int {
    identity = 0,
    offset_scale = 1,
    repeat = 2,
    posterize = 3,
    mirror_repeat = 4,
    bias_gain_curve = 5,
    smooth_window = 6,
};

constexpr int kColorPipelineMaxSourceStackCount = 8;
constexpr int kColorPipelineMaxShapeStackCount = 8;
constexpr int kColorPipelineMaxRootBasinPairCount = 8;
constexpr int kColorPipelineMaxPaletteStackCount = 8;
constexpr int kColorPipelineMaxGradingStackCount = 8;

struct ColorPipelineSourceRuntimeParams {
    float scale{1.0f};
    float bias{0.0f};
    float phase_offset{0.0f};
    float wrap_cycles{1.0f};
    int band_count{8};
    float softness{0.35f};
    float magnitude_scale{1.0f};
    float magnitude_bias{0.0f};
    float stripe_frequency{1.0f};
    float stripe_phase{0.0f};
    float proximity_scale{1.0f};
    float proximity_bias{0.0f};
    float blend_weight{1.0f};
};

struct ColorPipelineSourceStackEntry {
    ColorSignal signal{ColorSignal::smooth_escape};
    ColorPipelineSourceRuntimeParams params{};
};

struct ColorPipelineShapeRuntimeParams {
    float offset{0.0f};
    float scale{1.0f};
    float repeat_frequency{8.0f};
    float repeat_phase{0.0f};
    int posterize_steps{6};
    float posterize_mix{1.0f};
    float bias{0.5f};
    float gain{0.5f};
    float window_center{0.5f};
    float window_width{1.0f};
    float window_softness{0.0f};
};

struct ColorPipelineShapeStackEntry {
    ColorPipelineShape shape{ColorPipelineShape::identity};
    ColorPipelineShapeRuntimeParams params{};
};

enum class ColorPaletteBlendMode : int {
    normal = 0,
};

struct ColorPipelinePaletteRuntimeParams {
    float cycle_scale{1.0f};
    float saturation{1.0f};
    float phase_offset{0.0f};
    float band_emphasis{1.0f};
    float seed_scale{1.0f};
    float seed_phase{0.0f};
    float colorfulness{1.0f};
    float blend_weight{1.0f};
    ColorPaletteBlendMode blend_mode{ColorPaletteBlendMode::normal};
};

struct ColorPipelinePaletteStackEntry {
    ColorPalette palette{ColorPalette::cyclic_escape};
    ColorPipelinePaletteRuntimeParams params{};
};

struct ColorPipelineGradingRuntimeParams {
    float exposure{1.0f};
    float saturation{1.0f};
    float contrast{1.0f};
    float glow{0.25f};
    float balance_void{0.0f};
    float chroma_tension{0.0f};
    float accent_bias{0.0f};
};

struct ColorPipelineGradingStackEntry {
    ColorGradingPreset grading{ColorGradingPreset::escape_default};
    ColorPipelineGradingRuntimeParams params{};
};

struct ColorPipelineSelection {
    ColorSignal signal{ColorSignal::root_index};
    ColorPalette palette{ColorPalette::root_classic};
    ColorGradingPreset grading{ColorGradingPreset::basin_default};
};

enum class TranscendentalFunc : int {
    f_sin = 0,
    f_exp_minus_1 = 1,
    f_cosh = 2,
};

enum class McMullenPreset : int {
    z3_z3 = 0,   // z^3 + lambda/z^3, lambda=-0.125
    z2_z2 = 1,   // z^2 + lambda/z^2, lambda=-0.10
    z4_z2 = 2,   // z^4 + lambda/z^2, lambda=-0.05
    z3_z2 = 3,   // z^3 + lambda/z^2, lambda=-0.10
};

enum class FractalType : int {
    newton = 0,
    nova = 1,
    mandelbrot = 2,
    julia = 3,
    burning_ship = 4,
    multibrot = 5,
    phoenix = 6,
    explaino = 7,
    explaino_y = 8,
    explaino_fp = 9,
    explaino_nova = 10,
    explaino_halley = 11,
    explaino_dual = 12,
    explaino_mult = 13,
    explaino_phoenix = 14,
    explaino_transcendental = 15,
    explaino_inertial = 16,
    explaino_julia = 17,
    explaino_rational = 18,
    multicorn = 19,
    halley = 20,
    collatz = 21,
    explaino_collatz = 22,
    mcmullen = 23,
    lambda_map = 24,
    explaino_lambda = 25,
    explaino_rational_escape = 26,
    spider = 27,
    celtic_mandelbrot = 28,
    perpendicular_burning_ship = 29,
    explaino_joy = 30,
    explaino_fold = 31,
    explaino_bell = 32,
    explaino_ripple = 33,
    explaino_splice = 34,
    explaino_vortex = 35,
    explaino_tension = 36,
    explaino_balance_void = 37,
    explaino_all = 38,
    counterfactual_pair = 39,
    explaino_counterfactual_pair = 40,
    projection_and_flow = 41,
    explaino_projection_and_flow = 42,
};

enum class CounterfactualPairRootFamily : int {
    cubic_unit_roots = 0,
    quartic_unit_roots = 1,
};

enum class CounterfactualPairFrame : int {
    world_absolute = 0,
    view_relative = 1,
};

enum class ProjectionAndFlowRootFamily : int {
    cubic_unit_roots = 0,
    quartic_unit_roots = 1,
};

// --- Precision tier model (two-axis: backend x strategy) ---
// Public presets (API / schema surface).
enum class SampleTier : int {
    tier_auto = 0,  // resolve based on zoom depth + family support
    fast = 1,       // Float32 + Direct
    standard = 2,   // Float64 + Direct
    // deep = 3,    // Float64 + Perturbation (Phase B)
    // ultra = 4,   // DoubleDouble + Perturbation (Phase C)
};

// Internal numeric backend (resolved, not user-facing).
enum class NumericBackend : int {
    float32 = 0,
    float64 = 1,
    // double_double = 2,  // Phase C
};

// Internal iteration strategy (resolved, not user-facing).
enum class IterationStrategy : int {
    direct = 0,
    // perturbation = 1,  // Phase B
};

// Resolved evaluation mode passed through to the kernel.
struct ResolvedEvalMode {
    NumericBackend backend{NumericBackend::float32};
    IterationStrategy strategy{IterationStrategy::direct};
};

// Per-family support flags — not every family supports every tier.
enum SampleTierSupport : uint32_t {
    kSupport_Fast     = 1u << 0,
    kSupport_Standard = 1u << 1,
    // kSupport_Deep  = 1u << 2,  // Phase B
    // kSupport_Ultra = 1u << 3,  // Phase C
};

enum class CameraBehavior : int {
    manual = 0,
    complexity = 1,
    orbit = 2,
    entropy = 3,
    off = 4,
};

// param_anim_target is stored as a name string (char[32]) in ViewState.
// Resolution uses BindFloat at runtime — no enum needed.
// To add a new animatable param: add a BindFloat entry + schema dropdown option.

// --- PARAM ANIMATION TARGET ---
// Stored as a plain name string. Resolved at runtime via BindFloat.
//
// DO NOT add an enum, switch/case, or manual dispatch for animation targets.
// The system discovers targets by probing BindFloat("fractal.params.<name>")
// then BindFloat("fractal.view.<name>"). This is the C++ equivalent of
// reflecting over properties — schema JSON is the single UI source of truth.
//
// To make a new float param animatable:
//   1. Ensure it has a BindFloat entry in schema_binding.cpp (needed for slider anyway)
//   2. Add a dropdown option in fractal_binding_surface_v1.ui_schema.json
//   That's it. No C++ enum/switch/resolve wiring.
//
// See: param_anim_dynamics.cpp, test_param_anim_generic.cpp

struct ViewState {
    Float2 center{0.0f, 0.0f};
    float zoom{1.0f};
    float rotation_degrees{0.0f};
    bool auto_refresh{false};

    // High-precision view state used by the renderer and input updates.
    // The float fields above remain as the schema/UI binding surface.
    double center_hp_x{0.0};
    double center_hp_y{0.0};
    double log2_zoom{0.0};

    FractalType fractal_type{FractalType::explaino};

    bool explaino_alive{false};
    bool explaino_seed_tween{true};
    float explaino_phase{0.0f};
    float explaino_seed_drift{0.0f};
    bool auto_increment_seed{false};
    float explaino_seed_rate{0.001f};
    float explaino_phase_strength{1.0f};

    char param_anim_target[32]{"none"};
    float param_anim_rate{0.001f};

    bool auto_max_iter{false};

    CameraBehavior camera_behavior{CameraBehavior::complexity};
    bool auto_dive{false};
    float dive_speed{1.0f};
};

struct KernelParams {
    int max_iter{500};
    float epsilon{1e-6f};
    float nova_alpha{0.50f};
    float phoenix_p_real{0.0f};
    float phoenix_p_imag{0.0f};
    PolyKind poly_kind{PolyKind::z3_minus_1};
    float poly_coeffs[5]{-1.0f, 0.0f, 0.0f, 1.0f, 0.0f}; // z^3 - 1
    CounterfactualPairRootFamily counterfactual_pair_root_family{CounterfactualPairRootFamily::cubic_unit_roots};
    CounterfactualPairFrame counterfactual_pair_frame{CounterfactualPairFrame::world_absolute};
    float counterfactual_pair_offset_x{0.16f};
    float counterfactual_pair_offset_y{0.08f};
    float counterfactual_pair_reconvergence_ratio{0.60f};
    ProjectionAndFlowRootFamily projection_and_flow_root_family{ProjectionAndFlowRootFamily::cubic_unit_roots};
    float projection_and_flow_target_radius{1.0f};
    float projection_and_flow_pressure_threshold{1.0f};
    int multibrot_power{3};
    float multibrot_power_float{3.0f};
    float lambda_real{2.9685855f};
    float lambda_imag{-0.27446103f};
    ColoringMode coloring_mode{ColoringMode::root_basin};
    ColorPipelineSelection color_pipeline{};
    float exposure{1.0f};

    float color_saturation{1.15f};
    float color_contrast{1.10f};
    float color_glow{0.25f};
    float color_balance_void{0.0f};
    float color_chroma_tension{0.0f};
    float color_accent_bias{0.0f};
    float color_tint_r{1.0f};
    float color_tint_g{1.0f};
    float color_tint_b{1.0f};
    float color_phase_signal_offset{0.0f};
    float color_phase_wrap_cycles{1.0f};
    float color_phase_palette_offset{0.0f};
    int color_source_stack_count{0};
    ColorPipelineSourceStackEntry color_source_stack[kColorPipelineMaxSourceStackCount]{};
    ColorPipelineShape color_shape{ColorPipelineShape::identity};
    int color_shape_stack_count{0};
    ColorPipelineShapeStackEntry color_shape_stack[kColorPipelineMaxShapeStackCount]{};
    int color_root_basin_pair_count{0};
    ColorPipelineSelection color_root_basin_pairs[kColorPipelineMaxRootBasinPairCount]{};
    int color_palette_stack_count{0};
    ColorPipelinePaletteStackEntry color_palette_stack[kColorPipelineMaxPaletteStackCount]{};
    int color_grading_stack_count{0};
    ColorPipelineGradingStackEntry color_grading_stack[kColorPipelineMaxGradingStackCount]{};
    float color_shape_offset{0.0f};
    float color_shape_scale{1.0f};
    float color_shape_repeat_frequency{8.0f};
    float color_shape_repeat_phase{0.0f};
    int color_shape_posterize_steps{6};
    float color_shape_posterize_mix{1.0f};
    float color_shape_bias{0.5f};
    float color_shape_gain{0.5f};
    float color_shape_window_center{0.5f};
    float color_shape_window_width{1.0f};
    float color_shape_window_softness{0.0f};
    int color_iteration_band_count{8};
    float color_iteration_band_softness{0.35f};
    float color_iteration_band_emphasis{1.0f};
    float color_iteration_band_palette_offset{0.0f};
    float color_smooth_escape_scale{1.0f};
    float color_smooth_escape_bias{0.0f};
    float color_escape_magnitude_scale{1.0f};
    float color_escape_magnitude_bias{0.0f};
    float color_orbit_stripe_frequency{1.0f};
    float color_orbit_stripe_phase{0.0f};
    float color_root_proximity_scale{1.0f};
    float color_root_proximity_bias{0.0f};
    float color_heatmap_cycle_scale{1.0f};
    float color_heatmap_saturation{1.0f};
    float color_explaino_palette_seed_scale{1.0f};
    float color_explaino_palette_seed_phase{0.0f};
    float color_explaino_palette_colorfulness{1.0f};
    float color_contrast_lift_exposure{1.0f};
    float color_contrast_lift_saturation{1.0f};

    double explaino_seed{0.0};
    double explaino_seed_b{1.0};
    float explaino_mix{0.5f};
    float explaino_warp_strength{0.0f};
    float explaino_root_spread{0.5f};
    float explaino_damping{1.0f};
    float explaino_cluster_radius{0.0f};
    int explaino_root_count{0};
    Float2 explaino_roots[4]{{0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f}};
    TranscendentalFunc transcendental_func{TranscendentalFunc::f_sin};
    float momentum_beta{0.0f};
    float joy_coupling{0.0f};
    float fold_coupling{0.0f};
    float bell_coupling{0.0f};
    float balance_void{0.0f};
    float symmetry_tension{0.0f};
    float field_curvature{0.0f};
    float ripple_amplitude{0.0f};
    float splice_offset{0.0f};
    float poly_coeffs_b[5]{0.0f, 0.0f, 0.0f, 0.0f, 0.0f}; // Second polynomial for splice
    float vortex_strength{0.0f};
    float tension_strength{0.0f};
    McMullenPreset mcmullen_preset{McMullenPreset::z3_z3};
};

struct RenderSettings {
    static constexpr int kDefaultWidth = 2048;
    static constexpr int kDefaultHeight = 1536;
    static constexpr int kDefaultInteractionDebounceMs = 200;
    static constexpr float kDefaultPreviewTargetFps = 30.0f;
    static constexpr float kDefaultPreviewMinScale = 0.50f;

    Int2 resolution{kDefaultWidth, kDefaultHeight};
    int block_size{256};
    int device_id{0};
    bool benchmark{false};
    int interaction_debounce_ms{kDefaultInteractionDebounceMs};
    float preview_target_fps{kDefaultPreviewTargetFps};
    float preview_min_scale{kDefaultPreviewMinScale};
    SampleTier sample_tier{SampleTier::tier_auto};
    ResolvedEvalMode resolved_eval{};  // filled by resolver before dispatch
};

struct LensSettings {
    bool enabled{false};
    int downsample{2};
};

struct RenderStats {
    float last_render_ms{0.0f};
    int last_iters_avg{0};
    int last_device_id{0};
    ResolvedEvalMode resolved_eval{};
};

bool RenderFractalCUDA(
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    uint32_t* outRGBA,
    uint8_t* outMask,
    RenderStats* outStats,
    const char** outError);

// K2: Sample arbitrary complex-plane coordinates without pixel mapping or coloring.
// coords: host array of complex-plane points (Double2: .x = Re, .y = Im).
// outEvidence: host array of FractalSampleEvidence (caller-allocated, length numPoints).
// Returns false on error; *outError will be set.
struct FractalSampleEvidence;
struct FractalSampleResult;
bool SampleFractalEvidencePoints(
    const Double2* coords,
    int numPoints,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    FractalSampleEvidence* outEvidence,
    const char** outError);

// Legacy projection path preserved for existing callers.
bool SampleFractalPoints(
    const Double2* coords,
    int numPoints,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    FractalSampleResult* outResults,
    const char** outError);

void CleanupFractalSampleCore();
void CleanupFractalCUDA();
