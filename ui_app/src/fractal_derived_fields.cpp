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
    case FractalType::multicorn:
        center = {-0.3f, 0.0f};
        zoom = 1.5f;
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
    case FractalType::explaino_mult:
    case FractalType::explaino_phoenix:
    case FractalType::explaino_transcendental:
    case FractalType::explaino_inertial:
    case FractalType::explaino_julia:
    case FractalType::explaino_rational:
    case FractalType::collatz:
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
        view.fractal_type == FractalType::explaino_dual ||
        view.fractal_type == FractalType::explaino_mult ||
        view.fractal_type == FractalType::explaino_phoenix ||
        view.fractal_type == FractalType::explaino_transcendental ||
        view.fractal_type == FractalType::explaino_inertial ||
        view.fractal_type == FractalType::explaino_julia ||
        view.fractal_type == FractalType::explaino_rational) {
        params.max_iter = (view.fractal_type == FractalType::explaino ||
            view.fractal_type == FractalType::explaino_halley ||
            view.fractal_type == FractalType::explaino_dual ||
            view.fractal_type == FractalType::explaino_mult ||
            view.fractal_type == FractalType::explaino_phoenix ||
            view.fractal_type == FractalType::explaino_transcendental ||
            view.fractal_type == FractalType::explaino_inertial ||
            view.fractal_type == FractalType::explaino_rational) ? 500 :
            (view.fractal_type == FractalType::explaino_nova ? 300 :
            (view.fractal_type == FractalType::explaino_julia ? 1200 : 650));
        params.epsilon = 1e-6f;
        params.nova_alpha = 0.50f;
        params.poly_kind = PolyKind::custom;
        params.coloring_mode = DefaultColoringModeForFractal(view.fractal_type);
        params.exposure = 1.0f;
        params.multibrot_power = 3;
        params.phoenix_p_real = 0.0f;
        params.phoenix_p_imag = 0.0f;
        params.explaino_seed_b = 1.0;
        params.explaino_mix = 0.5f;
        params.explaino_warp_strength = 0.0f;
        params.explaino_cluster_radius = 0.0f;
        params.explaino_root_count = 0;
        params.momentum_beta = 0.0f;
        // Explaino-Phoenix: use a gentle memory term so basins remain visible.
        if (view.fractal_type == FractalType::explaino_phoenix) {
            params.phoenix_p_real = 0.12f;
        }
        // Explaino-Inertial: gentle default momentum so structure is visible.
        if (view.fractal_type == FractalType::explaino_inertial) {
            params.momentum_beta = 0.15f;
        }
        // Explaino-Rational: give a default rational perturbation so the pole is visible.
        if (view.fractal_type == FractalType::explaino_rational) {
            params.explaino_cluster_radius = 0.1f;
        }
        if (ioDirty) *ioDirty = true;
        return;
    }

    if (view.fractal_type == FractalType::multicorn) {
        params.max_iter = 1200;
        params.coloring_mode = DefaultColoringModeForFractal(view.fractal_type);
        params.exposure = 1.5f;
        params.multibrot_power = 2;  // Classic Tricorn
        if (ioDirty) *ioDirty = true;
        return;
    }

    if (view.fractal_type == FractalType::halley) {
        params.max_iter = 500;
        params.epsilon = 1e-6f;
        params.poly_kind = PolyKind::z3_minus_1;
        SetPolyPreset(params);
        params.coloring_mode = DefaultColoringModeForFractal(view.fractal_type);
        params.exposure = 1.0f;
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

    if (view.fractal_type == FractalType::collatz) {
        params.max_iter = 200;
        params.coloring_mode = DefaultColoringModeForFractal(view.fractal_type);
        params.exposure = 1.0f;
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

    float a = sh.a;
    float b = sh.b;
    float c = sh.c;
    float d = sh.d;

    if (view.fractal_type == FractalType::explaino_mult) {
        // Multiplicity mode: cluster roots in pairs around two centers.
        // Each center gets two near-coincident roots offset by cluster_radius.
        // This exposes solver failure geometry at near-degenerate polynomials.
        float cr = ClampF(params.explaino_cluster_radius, 0.0f, 2.0f);
        float half = cr * 0.5f;

        // Center 1: (a, b) — split into two roots offset along the real axis
        params.explaino_roots[0] = {a + half, b};
        params.explaino_roots[1] = {a - half, b};
        // Center 2: (c, d) — split into two roots offset along the imaginary axis
        params.explaino_roots[2] = {c, d + half};
        params.explaino_roots[3] = {c, d - half};
    } else {
        params.explaino_roots[0] = {a, b};
        params.explaino_roots[1] = {a, -b};
        params.explaino_roots[2] = {c, d};
        params.explaino_roots[3] = {c, -d};
    }

    // Compute degree-4 polynomial from the four roots: (z-r0)(z-r1)(z-r2)(z-r3)
    // Use the actual root positions stored in params.explaino_roots[].
    {
        const float r0x = params.explaino_roots[0].x, r0y = params.explaino_roots[0].y;
        const float r1x = params.explaino_roots[1].x, r1y = params.explaino_roots[1].y;
        const float r2x = params.explaino_roots[2].x, r2y = params.explaino_roots[2].y;
        const float r3x = params.explaino_roots[3].x, r3y = params.explaino_roots[3].y;

        // Quadratic factor from roots 0,1: (z - r0)(z - r1) = z^2 - (r0+r1)z + r0*r1
        // Sum s01 = r0 + r1,  product p01 = r0 * r1
        const float s01x = r0x + r1x, s01y = r0y + r1y;
        const float p01x = r0x * r1x - r0y * r1y, p01y = r0x * r1y + r0y * r1x;

        // Quadratic factor from roots 2,3: (z - r2)(z - r3) = z^2 - (r2+r3)z + r2*r3
        const float s23x = r2x + r3x, s23y = r2y + r3y;
        const float p23x = r2x * r3x - r2y * r3y, p23y = r2x * r3y + r2y * r3x;

        // (z^2 - s01*z + p01)(z^2 - s23*z + p23)
        // = z^4 - (s01+s23) z^3 + (p01 + s01*s23 + p23) z^2 - (p01*s23 + s01*p23) z + p01*p23
        // For real coefficients, take only real parts (imaginary parts cancel for conjugate pairs;
        // for explaino_mult near-coincident pairs, small imaginary residuals are dropped).
        params.poly_coeffs[4] = 1.0f;
        params.poly_coeffs[3] = -(s01x + s23x);
        params.poly_coeffs[2] = p01x + (s01x * s23x - s01y * s23y) + p23x;
        params.poly_coeffs[1] = -(p01x * s23x - p01y * s23y + s01x * p23x - s01y * p23y);
        params.poly_coeffs[0] = p01x * p23x - p01y * p23y;
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