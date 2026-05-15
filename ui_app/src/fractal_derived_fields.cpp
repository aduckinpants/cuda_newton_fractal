#include "fractal_derived_fields.h"

#include "explaino_seed.h"
#include "explaino_seed_curve.h"
#include "fractal_family_rules.h"
#include "view_hp_sync.h"

#include <cmath>

static inline float ClampF(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

uint32_t HashU32(uint32_t x) {
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

float Hash01(uint32_t x) {
    return static_cast<float>(HashU32(x) & 0x00ffffffU) / static_cast<float>(0x01000000U);
}

static inline float LerpF(float a, float b, float t) {
    return a + (b - a) * t;
}

void ApplyFractalViewPresetDefaults(ViewState& view, bool* ioDirty) {
    Float2 center = {0.0f, 0.0f};
    float zoom = 1.0f;

    view.auto_max_iter = DefaultAutoMaxIterForFractal(view.fractal_type);

    switch (view.fractal_type) {
    case FractalType::mandelbrot:
        center = {-0.745f, 0.186f};
        zoom = 38.0f;
        break;
    case FractalType::julia:
        center = {0.0f, 0.0f};
        zoom = 1.5f;
        break;
    case FractalType::burning_ship:
        center = {-1.762f, -0.028f};
        zoom = 25.0f;
        break;
    case FractalType::spider:
        center = {-0.12f, 0.75f};
        zoom = 4.0f;
        break;
    case FractalType::celtic_mandelbrot:
        center = {-0.45f, 0.42f};
        zoom = 3.2f;
        break;
    case FractalType::perpendicular_burning_ship:
        center = {-1.785f, -0.012f};
        zoom = 18.0f;
        break;
    case FractalType::multibrot:
        center = {-0.15f, 0.75f};
        zoom = 4.5f;
        break;
    case FractalType::multicorn:
        center = {-0.3f, 0.0f};
        zoom = 1.5f;
        break;
    case FractalType::lambda_map:
    case FractalType::explaino_lambda:
        center = {0.5f, 0.0f};
        zoom = 4.5f;
        break;
    case FractalType::explaino_rational_escape:
        center = {0.0f, 0.0f};
        zoom = 1.8f;
        break;
    case FractalType::phoenix:
        center = {0.36f, -0.1f};
        zoom = 2.8f;
        break;
    case FractalType::newton:
    case FractalType::nova:
    case FractalType::explaino_all:
    case FractalType::explaino:
    case FractalType::explaino_y:
    case FractalType::explaino_fp:
    case FractalType::explaino_nova:
    case FractalType::explaino_dual:
    case FractalType::explaino_mult:
    case FractalType::explaino_phoenix:
    case FractalType::explaino_transcendental:
    case FractalType::explaino_inertial:
    case FractalType::explaino_julia:
    case FractalType::explaino_rational:
    case FractalType::explaino_collatz:
    case FractalType::explaino_joy:
    case FractalType::explaino_fold:
    case FractalType::explaino_bell:
    case FractalType::explaino_ripple:
    case FractalType::explaino_splice:
    case FractalType::explaino_vortex:
    case FractalType::explaino_tension:
    case FractalType::explaino_balance_void:
    case FractalType::collatz:
    case FractalType::mcmullen:
    default:
        break;
    }

    view.center = center;
    view.zoom = zoom;
    view.rotation_degrees = 0.0f;
    if (ioDirty) *ioDirty = true;
}

void SetPolyPreset(KernelParams& params) {
    if (params.poly_kind == PolyKind::z3_minus_1) {
        params.poly_coeffs[0] = -1.0f;
        params.poly_coeffs[1] = 0.0f;
        params.poly_coeffs[2] = 0.0f;
        params.poly_coeffs[3] = 1.0f;
        params.poly_coeffs[4] = 0.0f;
    } else if (params.poly_kind == PolyKind::z4_minus_1) {
        params.poly_coeffs[0] = -1.0f;
        params.poly_coeffs[1] = 0.0f;
        params.poly_coeffs[2] = 0.0f;
        params.poly_coeffs[3] = 0.0f;
        params.poly_coeffs[4] = 1.0f;
    }
}

static void MarkDirty(bool* ioDirty) {
    if (ioDirty) *ioDirty = true;
}

static void ApplyCommonPresetDefaults(KernelParams& params) {
    params.multibrot_power = 3;
    params.multibrot_power_float = 3.0f;
    params.lambda_real = 2.9685855f;
    params.lambda_imag = -0.27446103f;
    params.color_saturation = 1.15f;
    params.color_contrast = 1.10f;
    params.color_tint_r = 1.0f;
    params.color_tint_g = 1.0f;
    params.color_tint_b = 1.0f;
    params.color_phase_signal_offset = 0.0f;
    params.color_phase_wrap_cycles = 1.0f;
    params.color_phase_palette_offset = 0.0f;
    params.color_shape = ColorPipelineShape::identity;
    params.color_shape_stack_count = 0;
    for (ColorPipelineShapeStackEntry& shapeEntry : params.color_shape_stack) {
        shapeEntry = {};
    }
    params.color_palette_stack_count = 0;
    for (ColorPipelinePaletteStackEntry& paletteEntry : params.color_palette_stack) {
        paletteEntry = {};
    }
    params.color_grading_stack_count = 0;
    for (ColorPipelineGradingStackEntry& gradingEntry : params.color_grading_stack) {
        gradingEntry = {};
    }
    params.color_shape_offset = 0.0f;
    params.color_shape_scale = 1.0f;
    params.color_shape_repeat_frequency = 8.0f;
    params.color_shape_repeat_phase = 0.0f;
    params.color_shape_posterize_steps = 6;
    params.color_shape_posterize_mix = 1.0f;
    params.color_shape_bias = 0.5f;
    params.color_shape_gain = 0.5f;
    params.color_shape_window_center = 0.5f;
    params.color_shape_window_width = 1.0f;
    params.color_shape_window_softness = 0.0f;
    params.color_iteration_band_count = 8;
    params.color_iteration_band_softness = 0.35f;
    params.color_iteration_band_emphasis = 1.0f;
    params.color_iteration_band_palette_offset = 0.0f;
    params.color_smooth_escape_scale = 1.0f;
    params.color_smooth_escape_bias = 0.0f;
    params.color_heatmap_cycle_scale = 1.0f;
    params.color_heatmap_saturation = 1.0f;
    params.color_explaino_palette_seed_scale = 1.0f;
    params.color_explaino_palette_seed_phase = 0.0f;
    params.color_explaino_palette_colorfulness = 1.0f;
    params.color_contrast_lift_exposure = 1.0f;
    params.color_contrast_lift_saturation = 1.0f;
}

static void ApplyDefaultColoringSelection(FractalType fractalType, KernelParams& params) {
    params.coloring_mode = DefaultColoringModeForFractal(fractalType);
    params.color_pipeline = DefaultColorPipelineForFractal(fractalType);
}

static void ApplyNewtonLikePresetDefaults(FractalType fractalType, KernelParams& params) {
    params.max_iter = fractalType == FractalType::nova ? 300 : 500;
    params.epsilon = 1e-6f;
    params.nova_alpha = 0.50f;
    params.poly_kind = PolyKind::z3_minus_1;
    SetPolyPreset(params);
    ApplyDefaultColoringSelection(fractalType, params);
    params.exposure = 1.0f;
    params.phoenix_p_real = -0.50f;
    params.phoenix_p_imag = 0.0f;
}

static bool IsExplainoPresetFractal(FractalType fractalType) {
    switch (fractalType) {
    case FractalType::explaino_all:
    case FractalType::explaino:
    case FractalType::explaino_y:
    case FractalType::explaino_fp:
    case FractalType::explaino_nova:
    case FractalType::explaino_halley:
    case FractalType::explaino_dual:
    case FractalType::explaino_mult:
    case FractalType::explaino_phoenix:
    case FractalType::explaino_transcendental:
    case FractalType::explaino_inertial:
    case FractalType::explaino_julia:
    case FractalType::explaino_rational:
    case FractalType::explaino_collatz:
    case FractalType::explaino_lambda:
    case FractalType::explaino_rational_escape:
    case FractalType::explaino_joy:
    case FractalType::explaino_fold:
    case FractalType::explaino_bell:
    case FractalType::explaino_ripple:
    case FractalType::explaino_splice:
    case FractalType::explaino_vortex:
    case FractalType::explaino_tension:
    case FractalType::explaino_balance_void:
        return true;
    default:
        return false;
    }
}

static int DefaultExplainoMaxIter(FractalType fractalType) {
    switch (fractalType) {
    case FractalType::explaino_all:
    case FractalType::explaino:
    case FractalType::explaino_halley:
    case FractalType::explaino_dual:
    case FractalType::explaino_mult:
    case FractalType::explaino_phoenix:
    case FractalType::explaino_transcendental:
    case FractalType::explaino_inertial:
    case FractalType::explaino_rational:
    case FractalType::explaino_collatz:
    case FractalType::explaino_joy:
    case FractalType::explaino_fold:
    case FractalType::explaino_bell:
    case FractalType::explaino_ripple:
    case FractalType::explaino_splice:
    case FractalType::explaino_vortex:
    case FractalType::explaino_tension:
    case FractalType::explaino_balance_void:
        return 500;
    case FractalType::explaino_nova:
        return 300;
    case FractalType::explaino_julia:
    case FractalType::explaino_lambda:
    case FractalType::explaino_rational_escape:
        return 1200;
    default:
        return 650;
    }
}

static void ApplyExplainoPresetDefaults(FractalType fractalType, KernelParams& params) {
    params.max_iter = DefaultExplainoMaxIter(fractalType);
    params.epsilon = 1e-6f;
    params.nova_alpha = 0.50f;
    params.poly_kind = PolyKind::custom;
    ApplyDefaultColoringSelection(fractalType, params);
    params.exposure = 1.0f;
    params.phoenix_p_real = 0.0f;
    params.phoenix_p_imag = 0.0f;
    params.explaino_seed_b = 1.0;
    params.explaino_mix = 0.5f;
    params.explaino_warp_strength = 0.0f;
    params.explaino_cluster_radius = 0.0f;
    params.explaino_root_count = 0;
    ApplyExplainoCouplingRegistryDefaults(fractalType, params);
    ApplyExplainoAxisRegistryDefaults(fractalType, params);
    ApplyPhoenixStepCarrierDefaults(fractalType, params);
    ApplyExplainoStructuralRegistryDefaults(fractalType, params);
    if (fractalType == FractalType::explaino_lambda) {
        params.exposure = 1.4f;
    }
    if (fractalType == FractalType::explaino_rational_escape) {
        params.exposure = 1.2f;
    }
}

static void ApplyEscapeTimePresetDefaults(FractalType fractalType, KernelParams& params, int maxIter, float exposure) {
    params.max_iter = maxIter;
    ApplyDefaultColoringSelection(fractalType, params);
    params.exposure = exposure;
}

static void ApplyHalleyPresetDefaults(FractalType fractalType, KernelParams& params) {
    params.max_iter = 500;
    params.epsilon = 1e-6f;
    params.poly_kind = PolyKind::z3_minus_1;
    SetPolyPreset(params);
    ApplyDefaultColoringSelection(fractalType, params);
    params.exposure = 1.0f;
}

static void ApplyPhoenixPresetDefaults(FractalType fractalType, KernelParams& params) {
    params.max_iter = 1200;
    params.epsilon = 1e-6f;
    params.nova_alpha = 0.50f;
    params.phoenix_p_real = 0.0f;
    params.explaino_cluster_radius = 0.0f;
    ApplyPhoenixStepCarrierDefaults(fractalType, params);
    ApplyExplainoStructuralRegistryDefaults(fractalType, params);
    params.phoenix_p_imag = 0.0f;
    ApplyDefaultColoringSelection(fractalType, params);
    params.exposure = 1.6f;
}

static void ApplyLambdaPresetDefaults(FractalType fractalType, KernelParams& params) {
    ApplyEscapeTimePresetDefaults(fractalType, params, 1200, 1.4f);
    params.lambda_real = 2.9685855f;
    params.lambda_imag = -0.27446103f;
}

static void ApplyCollatzPresetDefaults(FractalType fractalType, KernelParams& params) {
    params.max_iter = 200;
    ApplyDefaultColoringSelection(fractalType, params);
    params.exposure = 1.0f;
}

static void ApplyMcMullenPresetDefaults(FractalType fractalType, KernelParams& params) {
    params.max_iter = 500;
    ApplyDefaultColoringSelection(fractalType, params);
    params.exposure = 1.2f;
    params.mcmullen_preset = McMullenPreset::z3_z3;
}

void ApplyFractalPresetDefaults(const ViewState& view, KernelParams& params, bool* ioDirty) {
    ApplyCommonPresetDefaults(params);

    switch (view.fractal_type) {
    case FractalType::newton:
    case FractalType::nova:
        ApplyNewtonLikePresetDefaults(view.fractal_type, params);
        break;
    case FractalType::multicorn:
        ApplyEscapeTimePresetDefaults(view.fractal_type, params, 1200, 1.5f);
        params.multibrot_power = 2;
        params.multibrot_power_float = 2.0f;
        break;
    case FractalType::halley:
        ApplyHalleyPresetDefaults(view.fractal_type, params);
        break;
    case FractalType::phoenix:
        ApplyPhoenixPresetDefaults(view.fractal_type, params);
        break;
    case FractalType::mandelbrot:
    case FractalType::spider:
    case FractalType::celtic_mandelbrot:
    case FractalType::perpendicular_burning_ship:
    case FractalType::burning_ship:
        ApplyEscapeTimePresetDefaults(view.fractal_type, params, 1200, 1.5f);
        break;
    case FractalType::julia:
    case FractalType::multibrot:
        ApplyEscapeTimePresetDefaults(view.fractal_type, params, 1000, 1.4f);
        break;
    case FractalType::lambda_map:
        ApplyLambdaPresetDefaults(view.fractal_type, params);
        break;
    case FractalType::collatz:
        ApplyCollatzPresetDefaults(view.fractal_type, params);
        break;
    case FractalType::mcmullen:
        ApplyMcMullenPresetDefaults(view.fractal_type, params);
        break;
    default:
        if (IsExplainoPresetFractal(view.fractal_type)) {
            ApplyExplainoPresetDefaults(view.fractal_type, params);
            break;
        }
        params.max_iter = 800;
        ApplyDefaultColoringSelection(view.fractal_type, params);
        params.exposure = 1.0f;
        break;
    }

    MarkDirty(ioDirty);
}

struct ExplainoSeedShape {
    float a;
    float b;
    float c;
    float d;
};

static ExplainoSeedShape LerpExplainoShape(const ExplainoSeedShape& left, const ExplainoSeedShape& right, float mix) {
    ExplainoSeedShape out{};
    out.a = LerpF(left.a, right.a, mix);
    out.b = LerpF(left.b, right.b, mix);
    out.c = LerpF(left.c, right.c, mix);
    out.d = LerpF(left.d, right.d, mix);
    return out;
}

static ExplainoSeedShape ExplainoShapeForSeed(uint32_t seed, float phase, float spread, float phaseStrength) {
    const float r0 = Hash01(seed ^ 0x13579bdu);
    const float r1 = Hash01(seed ^ 0x2468aceu);
    const float r2 = Hash01(seed ^ 0xdeadbeefu);
    const float r3 = Hash01(seed ^ 0x9e3779b9u);

    const float baseR = 0.85f + 0.95f * spread;
    const float ps = 0.35f * phaseStrength;
    const float aAngle = (r0 * 6.2831853f) + ps * std::sin(phase * (0.15f + 0.20f * r2));
    const float cAngle = (r1 * 6.2831853f) + ps * std::cos(phase * (0.13f + 0.23f * r3));

    ExplainoSeedShape out{};
    out.a = baseR * std::cos(aAngle);
    out.b = (0.25f + 0.95f * std::fabs(std::sin(aAngle + 0.7f))) * (0.65f + 0.45f * r2);
    out.c = baseR * std::cos(cAngle);
    out.d = (0.25f + 0.95f * std::fabs(std::sin(cAngle - 0.4f))) * (0.65f + 0.45f * r3);
    return out;
}

static ExplainoSeedShape ExplainoShapeForCombinedSeed(double combinedSeed, bool seedTween, float phase, float spread, float phaseStrength) {
    if (!std::isfinite(combinedSeed)) combinedSeed = 0.0;

    const double seedFloor = std::floor(combinedSeed);
    float driftFrac = static_cast<float>(combinedSeed - seedFloor);
    if (!std::isfinite(driftFrac)) driftFrac = 0.0f;
    driftFrac = ClampF(driftFrac, 0.0f, 1.0f);

    const int seedBase = static_cast<int>(seedFloor);
    const uint32_t s0 = static_cast<uint32_t>(seedBase);
    const uint32_t s1 = static_cast<uint32_t>(seedBase + 1);

    ExplainoSeedShape shape0 = ExplainoShapeForSeed(s0, phase, spread, phaseStrength);
    if (!seedTween || driftFrac <= 0.0f) {
        return shape0;
    }

    ExplainoSeedShape shape1 = ExplainoShapeForSeed(s1, phase, spread, phaseStrength);
    float tweenFrac = static_cast<float>(ExplainoWedgeTween(static_cast<double>(driftFrac)));
    return LerpExplainoShape(shape0, shape1, tweenFrac);
}

static ExplainoSeedShape ResolveExplainoSeedShape(const ViewState& view, const KernelParams& params,
                                                  float phase, float spread, float phaseStrength) {
    ExplainoSeedShape shape = ExplainoShapeForCombinedSeed(
        ExplainoSeedCombined(view, params),
        view.explaino_seed_tween,
        phase,
        spread,
        phaseStrength);
    if (view.fractal_type != FractalType::explaino_dual) {
        return shape;
    }

    float mix = params.explaino_mix;
    if (!std::isfinite(mix)) mix = 0.5f;
    mix = ClampF(mix, 0.0f, 1.0f);
    ExplainoSeedShape shapeB = ExplainoShapeForCombinedSeed(
        params.explaino_seed_b,
        view.explaino_seed_tween,
        phase,
        spread,
        phaseStrength);
    return LerpExplainoShape(shape, shapeB, mix);
}

static void SetExplainoRootsForShape(FractalType fractalType, float clusterRadius,
                                     const ExplainoSeedShape& shape, KernelParams& params) {
    if (fractalType == FractalType::explaino_mult) {
        const float halfClusterRadius = ClampF(clusterRadius, 0.0f, 2.0f) * 0.5f;
        params.explaino_roots[0] = {shape.a + halfClusterRadius, shape.b};
        params.explaino_roots[1] = {shape.a - halfClusterRadius, shape.b};
        params.explaino_roots[2] = {shape.c, shape.d + halfClusterRadius};
        params.explaino_roots[3] = {shape.c, shape.d - halfClusterRadius};
        return;
    }

    params.explaino_roots[0] = {shape.a, shape.b};
    params.explaino_roots[1] = {shape.a, -shape.b};
    params.explaino_roots[2] = {shape.c, shape.d};
    params.explaino_roots[3] = {shape.c, -shape.d};
}

static void SetDegree4PolynomialCoefficientsFromRoots(const Float2 roots[4], float coeffs[5]) {
    const float r0x = roots[0].x, r0y = roots[0].y;
    const float r1x = roots[1].x, r1y = roots[1].y;
    const float r2x = roots[2].x, r2y = roots[2].y;
    const float r3x = roots[3].x, r3y = roots[3].y;

    const float s01x = r0x + r1x, s01y = r0y + r1y;
    const float p01x = r0x * r1x - r0y * r1y, p01y = r0x * r1y + r0y * r1x;
    const float s23x = r2x + r3x, s23y = r2y + r3y;
    const float p23x = r2x * r3x - r2y * r3y, p23y = r2x * r3y + r2y * r3x;

    coeffs[4] = 1.0f;
    coeffs[3] = -(s01x + s23x);
    coeffs[2] = p01x + (s01x * s23x - s01y * s23y) + p23x;
    coeffs[1] = -(p01x * s23x - p01y * s23y + s01x * p23x - s01y * p23y);
    coeffs[0] = p01x * p23x - p01y * p23y;
}

static void UpdateExplainoSplicePolynomial(const ViewState& view, const KernelParams& params,
                                           float phase, float spread, float phaseStrength,
                                           float coeffs[5]) {
    const double offsetSeed = ExplainoSeedCombined(view, params) + (double)params.splice_offset;
    const ExplainoSeedShape shape = ExplainoShapeForCombinedSeed(
        offsetSeed,
        view.explaino_seed_tween,
        phase,
        spread,
        phaseStrength);
    const Float2 roots[4] = {
        {shape.a, shape.b},
        {shape.a, -shape.b},
        {shape.c, shape.d},
        {shape.c, -shape.d},
    };
    SetDegree4PolynomialCoefficientsFromRoots(roots, coeffs);
}

static void ClearPolynomialCoefficients(float coeffs[5]) {
    for (int i = 0; i < 5; ++i) {
        coeffs[i] = 0.0f;
    }
}

static bool IsExplainoComposedVariantType(FractalType fractalType) {
    switch (fractalType) {
    case FractalType::explaino_all:
    case FractalType::explaino_ripple:
    case FractalType::explaino_splice:
    case FractalType::explaino_vortex:
    case FractalType::explaino_tension:
        return true;
    default:
        return false;
    }
}

void UpdateExplainoPolynomial(const ViewState& view, KernelParams& params, bool* ioDirty) {
    if (!IsExplainoFamily(view.fractal_type)) {
        params.explaino_root_count = 0;
        return;
    }

    params.poly_kind = PolyKind::custom;
    params.explaino_root_count = 4;

    const float phase = view.explaino_phase;
    const float spread = std::fmax(0.0f, std::fmin(3.0f, params.explaino_root_spread));
    const float phaseStrength = view.explaino_phase_strength;

    const ExplainoSeedShape shape = ResolveExplainoSeedShape(view, params, phase, spread, phaseStrength);
    SetExplainoRootsForShape(view.fractal_type, params.explaino_cluster_radius, shape, params);
    SetDegree4PolynomialCoefficientsFromRoots(params.explaino_roots, params.poly_coeffs);

    const bool needsSplicePolynomial =
        view.fractal_type == FractalType::explaino_splice ||
        (IsExplainoComposedVariantType(view.fractal_type) && params.splice_offset != 0.0f);
    if (needsSplicePolynomial) {
        UpdateExplainoSplicePolynomial(view, params, phase, spread, phaseStrength, params.poly_coeffs_b);
    } else {
        ClearPolynomialCoefficients(params.poly_coeffs_b);
    }

    if (ioDirty) *ioDirty = true;
}

void ApplyFractalDerivedFieldsAndSyncHp(ViewState& view, KernelParams& params, bool* ioDirty,
    bool haveExplainoSeedOverride, double explainoSeedOverride) {
    ApplyFractalPresetDefaults(view, params, ioDirty);
    if (haveExplainoSeedOverride) ExplainoSeedSetCombined(view, params, explainoSeedOverride);
    UpdateExplainoPolynomial(view, params, ioDirty);
    SyncViewHpFromUi(view);
}
