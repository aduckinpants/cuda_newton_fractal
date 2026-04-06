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

enum class PolyKind : int {
    z3_minus_1 = 0,
    z4_minus_1 = 1,
    custom = 2,
};

enum class ColoringMode : int {
    root_basin = 0,
    iteration_count = 1,
    smooth_escape = 2,
    joy_basins = 3,
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
};

enum class CameraBehavior : int {
    manual = 0,
    complexity = 1,
    orbit = 2,
    entropy = 3,
    off = 4,
};

struct ViewState {
    Float2 center{0.0f, 0.0f};
    float zoom{1.0f};
    float rotation_degrees{0.0f};
    bool auto_refresh{true};

    // High-precision view state used by the renderer and input updates.
    // The float fields above remain as the schema/UI binding surface.
    double center_hp_x{0.0};
    double center_hp_y{0.0};
    double log2_zoom{0.0};

    FractalType fractal_type{FractalType::newton};

    bool explaino_alive{false};
    bool explaino_seed_tween{true};
    float explaino_phase{0.0f};
    float explaino_seed_drift{0.0f};
    bool auto_increment_seed{false};
    float explaino_seed_rate{0.05f};
    float explaino_phase_strength{1.0f};

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
    int multibrot_power{3};
    ColoringMode coloring_mode{ColoringMode::root_basin};
    float exposure{1.0f};

    float color_saturation{1.15f};
    float color_contrast{1.10f};
    float color_tint_r{1.0f};
    float color_tint_g{1.0f};
    float color_tint_b{1.0f};

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
    McMullenPreset mcmullen_preset{McMullenPreset::z3_z3};
};

struct RenderSettings {
    Int2 resolution{1024, 768};
    int block_size{256};
    int device_id{0};
    bool benchmark{false};
};

struct LensSettings {
    bool enabled{false};
    int downsample{2};
};

struct RenderStats {
    float last_render_ms{0.0f};
    int last_iters_avg{0};
    int last_device_id{0};
};

bool RenderFractalCUDA(
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    uint32_t* outRGBA,
    uint8_t* outMask,
    RenderStats* outStats,
    const char** outError);

void CleanupFractalCUDA();
