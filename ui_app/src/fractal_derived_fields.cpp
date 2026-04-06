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
    case FractalType::multibrot:
        center = {-0.15f, 0.75f};
        zoom = 4.5f;
        break;
    case FractalType::phoenix:
        center = {0.36f, -0.1f};
        zoom = 2.8f;
        break;
    case FractalType::newton:
    case FractalType::nova:
    case FractalType::explaino:
    case FractalType::explaino_y:
    case FractalType::explaino_fp:
    case FractalType::explaino_nova:
    case FractalType::explaino_dual:
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

void ApplyFractalPresetDefaults(const ViewState& view, KernelParams& params, bool* ioDirty) {
    if (view.fractal_type == FractalType::newton) {
        params.max_iter = 500;
        params.epsilon = 1e-6f;
        params.nova_alpha = 0.50f;
        params.poly_kind = PolyKind::z3_minus_1;
        SetPolyPreset(params);
        params.coloring_mode = DefaultColoringModeForFractal(view.fractal_type);
        params.exposure = 1.0f;
        params.multibrot_power = 3;
        params.phoenix_p_real = -0.50f;
        params.phoenix_p_imag = 0.0f;
        if (ioDirty) *ioDirty = true;
        return;
    }

    if (view.fractal_type == FractalType::nova) {
        params.max_iter = 300;
        params.epsilon = 1e-6f;
        params.nova_alpha = 0.50f;
        params.poly_kind = PolyKind::z3_minus_1;
        SetPolyPreset(params);
        params.coloring_mode = DefaultColoringModeForFractal(view.fractal_type);
        params.exposure = 1.0f;
        params.multibrot_power = 3;
        params.phoenix_p_real = -0.50f;
        params.phoenix_p_imag = 0.0f;
        if (ioDirty) *ioDirty = true;
        return;
    }

    if (view.fractal_type == FractalType::explaino ||
        view.fractal_type == FractalType::explaino_y ||
        view.fractal_type == FractalType::explaino_fp ||
        view.fractal_type == FractalType::explaino_nova ||
        view.fractal_type == FractalType::explaino_halley ||
        view.fractal_type == FractalType::explaino_dual) {
        params.max_iter = (view.fractal_type == FractalType::explaino ||
            view.fractal_type == FractalType::explaino_halley ||
            view.fractal_type == FractalType::explaino_dual) ? 500 :
            (view.fractal_type == FractalType::explaino_nova ? 300 : 650);
        params.epsilon = 1e-6f;
        params.nova_alpha = 0.50f;
        params.poly_kind = PolyKind::custom;
        params.coloring_mode = DefaultColoringModeForFractal(view.fractal_type);
        params.exposure = 1.0f;
        params.multibrot_power = 3;
        params.phoenix_p_real = -0.50f;
        params.phoenix_p_imag = 0.0f;
        params.explaino_seed_b = 1.0;
        params.explaino_mix = 0.5f;
        params.explaino_warp_strength = 0.0f;
        params.explaino_root_count = 0;
        if (ioDirty) *ioDirty = true;
        return;
    }

    if (view.fractal_type == FractalType::phoenix) {
        params.max_iter = 1200;
        params.epsilon = 1e-6f;
        params.nova_alpha = 0.50f;
        params.phoenix_p_real = 0.5667f;
        params.phoenix_p_imag = 0.0f;
        params.coloring_mode = DefaultColoringModeForFractal(view.fractal_type);
        params.exposure = 1.6f;
        params.multibrot_power = 3;
        if (ioDirty) *ioDirty = true;
        return;
    }

    if (view.fractal_type == FractalType::mandelbrot) {
        params.max_iter = 1200;
        params.coloring_mode = DefaultColoringModeForFractal(view.fractal_type);
        params.exposure = 1.5f;
        params.multibrot_power = 3;
        if (ioDirty) *ioDirty = true;
        return;
    }

    if (view.fractal_type == FractalType::burning_ship) {
        params.max_iter = 1200;
        params.coloring_mode = DefaultColoringModeForFractal(view.fractal_type);
        params.exposure = 1.5f;
        params.multibrot_power = 3;
        if (ioDirty) *ioDirty = true;
        return;
    }

    if (view.fractal_type == FractalType::julia) {
        params.max_iter = 1000;
        params.coloring_mode = DefaultColoringModeForFractal(view.fractal_type);
        params.exposure = 1.4f;
        params.multibrot_power = 3;
        if (ioDirty) *ioDirty = true;
        return;
    }

    if (view.fractal_type == FractalType::multibrot) {
        params.max_iter = 1000;
        params.coloring_mode = DefaultColoringModeForFractal(view.fractal_type);
        params.exposure = 1.4f;
        params.multibrot_power = 3;
        if (ioDirty) *ioDirty = true;
        return;
    }

    params.max_iter = 800;
    params.coloring_mode = DefaultColoringModeForFractal(view.fractal_type);
    params.exposure = 1.0f;
    params.multibrot_power = 3;
    if (ioDirty) *ioDirty = true;
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

    ExplainoSeedShape sh = ExplainoShapeForCombinedSeed(
        ExplainoSeedCombined(view, params),
        view.explaino_seed_tween,
        phase,
        spread,
        phaseStrength);
    if (view.fractal_type == FractalType::explaino_dual) {
        float mix = params.explaino_mix;
        if (!std::isfinite(mix)) mix = 0.5f;
        mix = ClampF(mix, 0.0f, 1.0f);
        ExplainoSeedShape shB = ExplainoShapeForCombinedSeed(
            params.explaino_seed_b,
            view.explaino_seed_tween,
            phase,
            spread,
            phaseStrength);
        sh = LerpExplainoShape(sh, shB, mix);
    }

    const float a = sh.a;
    const float b = sh.b;
    const float c = sh.c;
    const float d = sh.d;

    params.explaino_roots[0] = {a, b};
    params.explaino_roots[1] = {a, -b};
    params.explaino_roots[2] = {c, d};
    params.explaino_roots[3] = {c, -d};

    const float q1 = -2.0f * a;
    const float q0 = a * a + b * b;
    const float r1 = -2.0f * c;
    const float r0 = c * c + d * d;

    params.poly_coeffs[4] = 1.0f;
    params.poly_coeffs[3] = q1 + r1;
    params.poly_coeffs[2] = q0 + q1 * r1 + r0;
    params.poly_coeffs[1] = q0 * r1 + q1 * r0;
    params.poly_coeffs[0] = q0 * r0;

    if (ioDirty) *ioDirty = true;
}

void ApplyFractalDerivedFieldsAndSyncHp(ViewState& view, KernelParams& params, bool* ioDirty,
    bool haveExplainoSeedOverride, double explainoSeedOverride) {
    ApplyFractalPresetDefaults(view, params, ioDirty);
    if (haveExplainoSeedOverride) ExplainoSeedSetCombined(view, params, explainoSeedOverride);
    UpdateExplainoPolynomial(view, params, ioDirty);
    SyncViewHpFromUi(view);
}