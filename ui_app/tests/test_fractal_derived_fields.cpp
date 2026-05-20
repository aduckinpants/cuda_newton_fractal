#include "../src/explaino_seed.h"
#include "../src/explaino_seed_curve.h"
#include "../src/fractal_family_rules.h"
#include "../src/fractal_derived_fields.h"

#include <cmath>
#include <iostream>

static bool NearlyEqual(float a, float b, float eps = 1.0e-5f) {
    return std::fabs(a - b) <= eps;
}

int main() {
    {
        ViewState view{};
        if (view.auto_dive) {
            std::cerr << "ViewState should start with auto_dive disabled\n";
            return 1;
        }
    }

    {
        ViewState view{};
        view.fractal_type = FractalType::mandelbrot;
        bool dirty = false;
        ApplyFractalViewPresetDefaults(view, &dirty);
        if (!dirty) {
            std::cerr << "Mandelbrot view preset should mark dirty\n";
            return 1;
        }
        if (!NearlyEqual(view.center.x, -0.745f) || !NearlyEqual(view.center.y, 0.186f) || !NearlyEqual(view.zoom, 38.0f)) {
            std::cerr << "Mandelbrot view preset should land on Seahorse Valley\n";
            return 1;
        }
    }

    {
        ViewState view{};
        view.fractal_type = FractalType::burning_ship;
        ApplyFractalViewPresetDefaults(view, nullptr);
        if (!NearlyEqual(view.center.x, -1.762f) || !NearlyEqual(view.center.y, -0.028f) || !NearlyEqual(view.zoom, 25.0f)) {
            std::cerr << "Burning Ship view preset should land on the antenna spiral\n";
            return 1;
        }
    }

    {
        ViewState view{};
        view.fractal_type = FractalType::spider;
        ApplyFractalViewPresetDefaults(view, nullptr);
        if (!NearlyEqual(view.center.x, -0.12f) || !NearlyEqual(view.center.y, 0.75f) || !NearlyEqual(view.zoom, 4.0f)) {
            std::cerr << "Spider view preset should land on a tuned web region\n";
            return 1;
        }
    }

    {
        ViewState view{};
        view.fractal_type = FractalType::celtic_mandelbrot;
        ApplyFractalViewPresetDefaults(view, nullptr);
        if (!NearlyEqual(view.center.x, -0.45f) || !NearlyEqual(view.center.y, 0.42f) || !NearlyEqual(view.zoom, 3.2f)) {
            std::cerr << "Celtic Mandelbrot view preset should land on a tuned Celtic lobe\n";
            return 1;
        }
    }

    {
        ViewState view{};
        view.fractal_type = FractalType::perpendicular_burning_ship;
        ApplyFractalViewPresetDefaults(view, nullptr);
        if (!NearlyEqual(view.center.x, -1.785f) || !NearlyEqual(view.center.y, -0.012f) || !NearlyEqual(view.zoom, 18.0f)) {
            std::cerr << "Perpendicular Burning Ship view preset should land on a tuned ship variant region\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::nova;
        ApplyFractalViewPresetDefaults(view, nullptr);
        if (!view.auto_max_iter) {
            std::cerr << "Nova should default auto_max_iter on for zoomed escape-time views\n";
            return 1;
        }
        ApplyFractalPresetDefaults(view, params, nullptr);
        if (params.coloring_mode != ColoringMode::smooth_escape) {
            std::cerr << "Nova should default to smooth_escape, not basin coloring\n";
            return 1;
        }
        if (params.color_pipeline.signal != ColorSignal::smooth_escape ||
            params.color_pipeline.palette != ColorPalette::cyclic_escape ||
            params.color_pipeline.grading != ColorGradingPreset::escape_default) {
            std::cerr << "Nova defaults should synthesize the escape-time color pipeline\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::newton;
        bool dirty = false;
        ApplyFractalPresetDefaults(view, params, &dirty);
        if (!dirty) {
            std::cerr << "Newton defaults should mark dirty\n";
            return 1;
        }
        if (params.max_iter != 500 || !NearlyEqual(params.epsilon, 1.0e-6f) || !NearlyEqual(params.nova_alpha, 0.50f)) {
            std::cerr << "Newton should use root-finding iteration defaults\n";
            return 1;
        }
        if (params.poly_kind != PolyKind::z3_minus_1) {
            std::cerr << "Newton should default to the z^3-1 preset polynomial\n";
            return 1;
        }
        if (!NearlyEqual(params.poly_coeffs[0], -1.0f) || !NearlyEqual(params.poly_coeffs[3], 1.0f) ||
            !NearlyEqual(params.poly_coeffs[4], 0.0f)) {
            std::cerr << "Newton should apply the z^3-1 coefficient preset\n";
            return 1;
        }
        if (params.coloring_mode != DefaultColoringModeForFractal(view.fractal_type) || !NearlyEqual(params.exposure, 1.0f) ||
            !NearlyEqual(params.phoenix_p_real, -0.50f) || !NearlyEqual(params.phoenix_p_imag, 0.0f)) {
            std::cerr << "Newton should use the expected root-finding coloring and Phoenix defaults\n";
            return 1;
        }
        if (params.color_pipeline.signal != ColorSignal::root_index ||
            params.color_pipeline.palette != ColorPalette::joy ||
            params.color_pipeline.grading != ColorGradingPreset::basin_default) {
            std::cerr << "Newton defaults should synthesize the basin color pipeline\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::mandelbrot;
        bool dirty = false;
        ApplyFractalPresetDefaults(view, params, &dirty);
        if (params.max_iter != 1200 || !NearlyEqual(params.exposure, 1.5f)) {
            std::cerr << "Mandelbrot param preset should have tuned max_iter and exposure\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::spider;
        bool dirty = false;
        ApplyFractalPresetDefaults(view, params, &dirty);
        if (!dirty) {
            std::cerr << "Spider defaults should mark dirty\n";
            return 1;
        }
        if (params.max_iter != 1200 || params.coloring_mode != ColoringMode::smooth_escape || !NearlyEqual(params.exposure, 1.5f)) {
            std::cerr << "Spider param preset should use escape-time defaults\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::celtic_mandelbrot;
        bool dirty = false;
        ApplyFractalPresetDefaults(view, params, &dirty);
        if (!dirty) {
            std::cerr << "Celtic Mandelbrot defaults should mark dirty\n";
            return 1;
        }
        if (params.max_iter != 1200 || params.coloring_mode != ColoringMode::smooth_escape || !NearlyEqual(params.exposure, 1.5f)) {
            std::cerr << "Celtic Mandelbrot param preset should use escape-time defaults\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::perpendicular_burning_ship;
        bool dirty = false;
        ApplyFractalPresetDefaults(view, params, &dirty);
        if (!dirty) {
            std::cerr << "Perpendicular Burning Ship defaults should mark dirty\n";
            return 1;
        }
        if (params.max_iter != 1200 || params.coloring_mode != ColoringMode::smooth_escape || !NearlyEqual(params.exposure, 1.5f)) {
            std::cerr << "Perpendicular Burning Ship param preset should use escape-time defaults\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::magnet;
        bool dirty = false;
        ApplyFractalViewPresetDefaults(view, &dirty);
        if (!dirty) {
            std::cerr << "Magnet view defaults should mark dirty\n";
            return 1;
        }
        if (!NearlyEqual(view.center.x, -0.08f) || !NearlyEqual(view.center.y, 0.0f) ||
            !NearlyEqual(view.zoom, 2.2f) || view.auto_max_iter) {
            std::cerr << "Magnet view defaults should land on the bounded Type I inspection region without default auto max-iter\n";
            return 1;
        }
        params.magnet_seed_real = 0.4f;
        params.magnet_seed_imag = -0.2f;
        params.magnet_relaxation = 0.25f;
        params.magnet_bailout = 40.0f;
        dirty = false;
        ApplyFractalPresetDefaults(view, params, &dirty);
        if (!dirty || params.max_iter != 900 || params.coloring_mode != ColoringMode::smooth_escape ||
            params.color_pipeline.signal != ColorSignal::smooth_escape ||
            params.color_pipeline.palette != ColorPalette::cyclic_escape ||
            params.color_pipeline.grading != ColorGradingPreset::escape_default ||
            !NearlyEqual(params.exposure, 1.35f) ||
            !NearlyEqual(params.magnet_seed_real, 0.0f) || !NearlyEqual(params.magnet_seed_imag, 0.0f) ||
            !NearlyEqual(params.magnet_relaxation, 1.0f) || !NearlyEqual(params.magnet_bailout, 12.0f)) {
            std::cerr << "Magnet selector defaults should restore the bounded Type I smooth-escape preset\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::lambda_map;
        bool dirty = false;
        ApplyFractalPresetDefaults(view, params, &dirty);
        if (!dirty) {
            std::cerr << "Lambda defaults should mark dirty\n";
            return 1;
        }
        if (params.max_iter != 1200 || params.coloring_mode != ColoringMode::smooth_escape || !NearlyEqual(params.exposure, 1.4f)) {
            std::cerr << "Lambda param preset should use escape-time defaults\n";
            return 1;
        }
        if (!NearlyEqual(params.lambda_real, 2.9685855f) || !NearlyEqual(params.lambda_imag, -0.27446103f)) {
            std::cerr << "Lambda should default to deterministic lambda_real/lambda_imag\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::projection_and_flow;
        params.projection_and_flow_root_family = ProjectionAndFlowRootFamily::quartic_unit_roots;
        params.projection_and_flow_target_radius = 1.75f;
        params.projection_and_flow_pressure_threshold = 0.5f;
        params.poly_kind = PolyKind::z4_minus_1;
        params.poly_coeffs[0] = -1.0f;
        params.poly_coeffs[1] = 0.0f;
        params.poly_coeffs[2] = 0.0f;
        params.poly_coeffs[3] = 0.0f;
        params.poly_coeffs[4] = 1.0f;
        bool dirty = false;
        ApplyFractalPresetDefaults(view, params, &dirty);
        if (!dirty) {
            std::cerr << "Projection-and-Flow defaults should mark dirty\n";
            return 1;
        }
        if (params.projection_and_flow_root_family != ProjectionAndFlowRootFamily::cubic_unit_roots ||
            !NearlyEqual(params.projection_and_flow_target_radius, 1.0f) ||
            !NearlyEqual(params.projection_and_flow_pressure_threshold, 1.0f) ||
            params.poly_kind != PolyKind::z3_minus_1 ||
            !NearlyEqual(params.poly_coeffs[0], -1.0f) ||
            !NearlyEqual(params.poly_coeffs[3], 1.0f) ||
            !NearlyEqual(params.poly_coeffs[4], 0.0f)) {
            std::cerr << "Projection-and-Flow selector defaults should restore the shipped cubic/radius/pressure baseline\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::explaino_projection_and_flow;
        params.projection_and_flow_root_family = ProjectionAndFlowRootFamily::quartic_unit_roots;
        params.projection_and_flow_target_radius = 1.75f;
        params.projection_and_flow_pressure_threshold = 0.5f;
        params.explaino_warp_strength = 0.0f;
        params.explaino_damping = 1.0f;
        params.poly_kind = PolyKind::custom;
        params.poly_coeffs[0] = 1.0f;
        params.poly_coeffs[1] = 0.0f;
        params.poly_coeffs[2] = 0.0f;
        params.poly_coeffs[3] = 1.0f;
        params.poly_coeffs[4] = 1.0f;
        bool dirty = false;
        ApplyFractalPresetDefaults(view, params, &dirty);
        UpdateExplainoPolynomial(view, params, &dirty);
        if (!dirty) {
            std::cerr << "Explaino Projection-and-Flow defaults should mark dirty\n";
            return 1;
        }
        if (params.projection_and_flow_root_family != ProjectionAndFlowRootFamily::cubic_unit_roots ||
            !NearlyEqual(params.projection_and_flow_target_radius, 1.0f) ||
            !NearlyEqual(params.projection_and_flow_pressure_threshold, 1.0f) ||
            params.poly_kind != PolyKind::custom ||
            !NearlyEqual(params.explaino_warp_strength, 0.25f) ||
            !NearlyEqual(params.explaino_damping, 0.75f) ||
            params.explaino_root_count != 3 ||
            !NearlyEqual(params.poly_coeffs[3], 1.0f) ||
            !NearlyEqual(params.poly_coeffs[4], 0.0f)) {
            std::cerr << "Explaino Projection-and-Flow defaults should keep the shared Projection-and-Flow controls while deriving a cubic Explaino carrier polynomial\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams narrowSpread{};
        KernelParams wideSpread{};
        view.fractal_type = FractalType::explaino_projection_and_flow;
        view.explaino_phase = 1.0f;
        view.explaino_phase_strength = 1.0f;
        ExplainoSeedSetCombined(view, narrowSpread, 3.25);
        ExplainoSeedSetCombined(view, wideSpread, 3.25);
        narrowSpread.projection_and_flow_root_family = ProjectionAndFlowRootFamily::quartic_unit_roots;
        wideSpread.projection_and_flow_root_family = ProjectionAndFlowRootFamily::quartic_unit_roots;
        narrowSpread.explaino_root_spread = 0.15f;
        wideSpread.explaino_root_spread = 1.35f;

        UpdateExplainoPolynomial(view, narrowSpread, nullptr);
        UpdateExplainoPolynomial(view, wideSpread, nullptr);

        if (narrowSpread.poly_kind != PolyKind::custom ||
            wideSpread.poly_kind != PolyKind::custom ||
            narrowSpread.explaino_root_count != 4 ||
            wideSpread.explaino_root_count != 4) {
            std::cerr << "Explaino Projection-and-Flow root-spread authority should keep the quartic Explaino carrier live\n";
            return 1;
        }

        bool sawRootSpreadDifference = false;
        for (int coeffIndex = 0; coeffIndex < 5; ++coeffIndex) {
            if (!NearlyEqual(narrowSpread.poly_coeffs[coeffIndex], wideSpread.poly_coeffs[coeffIndex], 1.0e-6f)) {
                sawRootSpreadDifference = true;
                break;
            }
        }
        if (!sawRootSpreadDifference) {
            std::cerr << "Explaino Projection-and-Flow root spread should rewrite the derived carrier polynomial instead of persisting as a dead slider\n";
            return 1;
        }
    }

    {
        ViewState neutralView{};
        ViewState activeView{};
        KernelParams neutralPhaseStrength{};
        KernelParams activePhaseStrength{};
        neutralView.fractal_type = FractalType::explaino_projection_and_flow;
        activeView.fractal_type = FractalType::explaino_projection_and_flow;
        neutralView.explaino_phase = 1.0f;
        activeView.explaino_phase = 1.0f;
        neutralView.explaino_phase_strength = 0.0f;
        activeView.explaino_phase_strength = 1.6f;
        ExplainoSeedSetCombined(neutralView, neutralPhaseStrength, 4.5);
        ExplainoSeedSetCombined(activeView, activePhaseStrength, 4.5);
        neutralPhaseStrength.projection_and_flow_root_family = ProjectionAndFlowRootFamily::quartic_unit_roots;
        activePhaseStrength.projection_and_flow_root_family = ProjectionAndFlowRootFamily::quartic_unit_roots;
        neutralPhaseStrength.explaino_root_spread = 0.5f;
        activePhaseStrength.explaino_root_spread = 0.5f;

        UpdateExplainoPolynomial(neutralView, neutralPhaseStrength, nullptr);
        UpdateExplainoPolynomial(activeView, activePhaseStrength, nullptr);

        if (neutralPhaseStrength.poly_kind != PolyKind::custom ||
            activePhaseStrength.poly_kind != PolyKind::custom ||
            neutralPhaseStrength.explaino_root_count != 4 ||
            activePhaseStrength.explaino_root_count != 4) {
            std::cerr << "Explaino Projection-and-Flow phase-strength authority should keep the quartic Explaino carrier live\n";
            return 1;
        }

        bool sawPhaseStrengthDifference = false;
        for (int coeffIndex = 0; coeffIndex < 5; ++coeffIndex) {
            if (!NearlyEqual(neutralPhaseStrength.poly_coeffs[coeffIndex], activePhaseStrength.poly_coeffs[coeffIndex], 1.0e-6f)) {
                sawPhaseStrengthDifference = true;
                break;
            }
        }
        if (!sawPhaseStrengthDifference) {
            std::cerr << "Explaino Projection-and-Flow phase strength should rewrite the derived carrier polynomial instead of staying inert\n";
            return 1;
        }
    }

    // Explaino-Transcendental defaults
    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::explaino_transcendental;
        params.explaino_warp_strength = 0.99f;
        params.coloring_mode = ColoringMode::smooth_escape;

        bool dirty = false;
        ApplyFractalPresetDefaults(view, params, &dirty);
        UpdateExplainoPolynomial(view, params, &dirty);

        if (!dirty) {
            std::cerr << "Explaino-Transcendental defaults should mark dirty\n";
            return 1;
        }
        if (params.coloring_mode != ColoringMode::joy_basins) {
            std::cerr << "Explaino-Transcendental should default to joy_basins\n";
            return 1;
        }
        if (!NearlyEqual(params.explaino_warp_strength, 0.25f)) {
            std::cerr << "Explaino-Transcendental should keep seed-driven warp active by default so the visible seed slider is not inert\n";
            return 1;
        }
    }

    // Explaino-Lambda defaults
    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::explaino_lambda;
        params.explaino_seed = 7.0;
        params.explaino_warp_strength = 0.99f;
        params.coloring_mode = ColoringMode::joy_basins;  // Will be overridden

        bool dirty = false;
        ApplyFractalPresetDefaults(view, params, &dirty);
        UpdateExplainoPolynomial(view, params, &dirty);

        if (!dirty) {
            std::cerr << "Explaino-Lambda defaults should mark dirty\n";
            return 1;
        }
        if (params.max_iter != 1200) {
            std::cerr << "Explaino-Lambda should use Lambda-tuned max_iter (1200)\n";
            return 1;
        }
        if (params.coloring_mode != ColoringMode::smooth_escape) {
            std::cerr << "Explaino-Lambda should default to smooth_escape\n";
            return 1;
        }
        if (!NearlyEqual(params.exposure, 1.4f)) {
            std::cerr << "Explaino-Lambda should use Lambda-tuned exposure (1.4)\n";
            return 1;
        }
        if (!NearlyEqual(params.explaino_warp_strength, 0.25f)) {
            std::cerr << "Explaino-Lambda should keep seed-driven warp active by default so the visible seed slider is not inert\n";
            return 1;
        }
        if (params.poly_kind != PolyKind::custom) {
            std::cerr << "Explaino-Lambda should force custom polynomial\n";
            return 1;
        }
        if (!NearlyEqual(params.lambda_real, 2.9685855f) || !NearlyEqual(params.lambda_imag, -0.27446103f)) {
            std::cerr << "Explaino-Lambda should default to deterministic lambda_real/lambda_imag\n";
            return 1;
        }
    }

    // Explaino-Rational-Escape defaults
    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::explaino_rational_escape;
        params.explaino_seed = 7.0;
        params.explaino_warp_strength = 0.99f;
        params.coloring_mode = ColoringMode::joy_basins;  // Will be overridden

        bool dirty = false;
        ApplyFractalPresetDefaults(view, params, &dirty);
        UpdateExplainoPolynomial(view, params, &dirty);

        if (!dirty) {
            std::cerr << "Explaino-Rational-Escape defaults should mark dirty\n";
            return 1;
        }
        if (params.max_iter != 1200) {
            std::cerr << "Explaino-Rational-Escape should use tuned max_iter (1200), got " << params.max_iter << "\n";
            return 1;
        }
        if (params.coloring_mode != ColoringMode::smooth_escape) {
            std::cerr << "Explaino-Rational-Escape should default to smooth_escape\n";
            return 1;
        }
        if (!NearlyEqual(params.exposure, 1.2f)) {
            std::cerr << "Explaino-Rational-Escape should use tuned exposure (1.2)\n";
            return 1;
        }
        if (params.poly_kind != PolyKind::custom) {
            std::cerr << "Explaino-Rational-Escape should force custom polynomial\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::phoenix;
        bool dirty = false;
        ApplyFractalPresetDefaults(view, params, &dirty);
        if (params.max_iter != 1200 || !NearlyEqual(params.phoenix_p_real, 0.5667f) || !NearlyEqual(params.exposure, 1.6f)) {
            std::cerr << "Phoenix param preset should have tuned p_real, max_iter, and exposure\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::mcmullen;
        bool dirty = false;
        ApplyFractalPresetDefaults(view, params, &dirty);
        if (!dirty) {
            std::cerr << "McMullen defaults should mark dirty\n";
            return 1;
        }
        if (params.max_iter != 500 || !NearlyEqual(params.exposure, 1.2f) ||
            params.coloring_mode != DefaultColoringModeForFractal(view.fractal_type)) {
            std::cerr << "McMullen should use the tuned iteration, exposure, and coloring defaults\n";
            return 1;
        }
        if (params.mcmullen_preset != McMullenPreset::z3_z3) {
            std::cerr << "McMullen should default to the z3_z3 preset\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::explaino;
        params.explaino_seed = 7.0;
        params.explaino_warp_strength = 0.99f;
        view.explaino_phase = 0.0f;
        view.explaino_seed_drift = 0.0f;

        bool dirty = false;
        ApplyFractalPresetDefaults(view, params, &dirty);
        UpdateExplainoPolynomial(view, params, &dirty);

        if (!dirty) {
            std::cerr << "Explaino defaults should mark dirty\n";
            return 1;
        }
        if (params.poly_kind != PolyKind::custom) {
            std::cerr << "Explaino should force custom polynomial\n";
            return 1;
        }
        if (!NearlyEqual(params.explaino_warp_strength, 0.0f)) {
            std::cerr << "Explaino should start with warp disabled\n";
            return 1;
        }
        if (params.explaino_root_count != 4) {
            std::cerr << "Explaino should derive four roots\n";
            return 1;
        }
        if (!NearlyEqual(params.poly_coeffs[4], 1.0f)) {
            std::cerr << "Explaino quartic should be monic\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::explaino_nova;
        ApplyFractalViewPresetDefaults(view, nullptr);
        if (!view.auto_max_iter) {
            std::cerr << "Explaino-Nova should default auto_max_iter on for zoomed escape-time views\n";
            return 1;
        }
        params.explaino_seed = 7.0;
        params.explaino_warp_strength = 0.99f;
        params.coloring_mode = ColoringMode::joy_basins;
        params.nova_alpha = 1.5f;

        bool dirty = false;
        ApplyFractalPresetDefaults(view, params, &dirty);
        UpdateExplainoPolynomial(view, params, &dirty);

        if (!dirty) {
            std::cerr << "Explaino-Nova defaults should mark dirty\n";
            return 1;
        }
        if (params.max_iter != 300 || !NearlyEqual(params.epsilon, 1.0e-6f) || !NearlyEqual(params.nova_alpha, 0.50f)) {
            std::cerr << "Explaino-Nova should use Nova-tuned iteration defaults\n";
            return 1;
        }
        if (params.poly_kind != PolyKind::custom) {
            std::cerr << "Explaino-Nova should force custom polynomial\n";
            return 1;
        }
        if (params.coloring_mode != ColoringMode::smooth_escape) {
            std::cerr << "Explaino-Nova should default to smooth_escape\n";
            return 1;
        }
        if (!NearlyEqual(params.explaino_warp_strength, 0.0f)) {
            std::cerr << "Explaino-Nova should start with warp disabled\n";
            return 1;
        }
        if (params.explaino_root_count != 4 || !NearlyEqual(params.poly_coeffs[4], 1.0f)) {
            std::cerr << "Explaino-Nova should still derive the Explaino quartic surface\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::explaino_dual;
        params.explaino_seed = 2.0;
        params.explaino_seed_b = 9.0;
        params.explaino_mix = 0.25f;
        params.explaino_warp_strength = 0.99f;

        bool dirty = false;
        ApplyFractalPresetDefaults(view, params, &dirty);
        UpdateExplainoPolynomial(view, params, &dirty);

        if (!dirty) {
            std::cerr << "Explaino-DualSeed defaults should mark dirty\n";
            return 1;
        }
        if (params.max_iter != 500 || params.poly_kind != PolyKind::custom) {
            std::cerr << "Explaino-DualSeed should use custom polynomial root-finding defaults\n";
            return 1;
        }
        if (params.coloring_mode != ColoringMode::joy_basins) {
            std::cerr << "Explaino-DualSeed should default to joy_basins\n";
            return 1;
        }
        if (!NearlyEqual(params.explaino_warp_strength, 0.0f)) {
            std::cerr << "Explaino-DualSeed should start with warp disabled\n";
            return 1;
        }
        if (!NearlyEqual((float)params.explaino_seed_b, 1.0f) || !NearlyEqual(params.explaino_mix, 0.5f)) {
            std::cerr << "Explaino-DualSeed should reset to deterministic seed_b/mix defaults\n";
            return 1;
        }
        if (params.explaino_root_count != 4 || !NearlyEqual(params.poly_coeffs[4], 1.0f)) {
            std::cerr << "Explaino-DualSeed should derive the Explaino quartic surface\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::julia;
        params.explaino_root_count = 99;
        UpdateExplainoPolynomial(view, params, nullptr);
        if (params.explaino_root_count != 0) {
            std::cerr << "Non-Explaino modes must clear Explaino roots\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::explaino;
        bool dirty = false;
        ApplyFractalDerivedFieldsAndSyncHp(view, params, &dirty, true, 0.75);
        if (!NearlyEqual((float)params.explaino_seed, 0.0f) || !NearlyEqual(view.explaino_seed_drift, 0.75f)) {
            std::cerr << "Explaino combined seed override must split into base + drift\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::explaino_splice;
        params.splice_offset = 1.0f;
        UpdateExplainoPolynomial(view, params, nullptr);
        if (!NearlyEqual(params.poly_coeffs_b[4], 1.0f)) {
            std::cerr << "Explaino-Splice should populate the secondary polynomial coefficients\n";
            return 1;
        }

        view.fractal_type = FractalType::explaino_joy;
        UpdateExplainoPolynomial(view, params, nullptr);
        for (float coeff : params.poly_coeffs_b) {
            if (!NearlyEqual(coeff, 0.0f)) {
                std::cerr << "Non-splice Explaino modes must clear stale secondary polynomial coefficients\n";
                return 1;
            }
        }
    }

    {
        struct ComposedVariantExpectation {
            FractalType fractal_type;
            float ripple_amplitude;
            float splice_offset;
            float vortex_strength;
            float tension_strength;
            float balance_void;
            float symmetry_tension;
            float field_curvature;
        };

        const ComposedVariantExpectation expectations[] = {
            {FractalType::explaino_all, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
            {FractalType::explaino_ripple, 0.15f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
            {FractalType::explaino_splice, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
            {FractalType::explaino_vortex, 0.0f, 0.0f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f},
            {FractalType::explaino_tension, 0.0f, 0.0f, 0.0f, 0.02f, 0.0f, 0.0f, 0.0f},
            {FractalType::explaino_balance_void, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},
        };

        for (const ComposedVariantExpectation& expectation : expectations) {
            ViewState view{};
            KernelParams params{};
            view.fractal_type = expectation.fractal_type;

            // Simulate schema-default carryover from hidden variant controls.
            params.ripple_amplitude = 0.15f;
            params.splice_offset = 0.5f;
            params.vortex_strength = 0.3f;
            params.tension_strength = 0.02f;
            params.balance_void = 0.35f;
            params.symmetry_tension = -0.2f;
            params.field_curvature = 0.25f;

            ApplyFractalDerivedFieldsAndSyncHp(view, params, nullptr, false, 0.0);

            if (!NearlyEqual(params.ripple_amplitude, expectation.ripple_amplitude) ||
                !NearlyEqual(params.splice_offset, expectation.splice_offset) ||
                !NearlyEqual(params.vortex_strength, expectation.vortex_strength) ||
                !NearlyEqual(params.tension_strength, expectation.tension_strength) ||
                !NearlyEqual(params.balance_void, expectation.balance_void) ||
                !NearlyEqual(params.symmetry_tension, expectation.symmetry_tension) ||
                !NearlyEqual(params.field_curvature, expectation.field_curvature)) {
                std::cerr << "Explaino canonical and carrier presets must derive the seven shared axis defaults from one registry\n";
                return 1;
            }
        }
    }

    {
        ViewState view{};
        KernelParams neutralParams{};
        KernelParams activeParams{};
        view.fractal_type = FractalType::explaino_phoenix;
        view.explaino_phase = 0.0f;
        view.explaino_phase_strength = 1.0f;
        ExplainoSeedSetCombined(view, neutralParams, 4.25);
        ExplainoSeedSetCombined(view, activeParams, 4.25);
        neutralParams.explaino_root_spread = 0.5f;
        activeParams.explaino_root_spread = 0.5f;
        neutralParams.phoenix_p_real = 0.0f;
        activeParams.phoenix_p_real = 0.35f;

        UpdateExplainoPolynomial(view, neutralParams, nullptr);
        UpdateExplainoPolynomial(view, activeParams, nullptr);

        for (int rootIndex = 0; rootIndex < 4; ++rootIndex) {
            if (!NearlyEqual(neutralParams.explaino_roots[rootIndex].x, activeParams.explaino_roots[rootIndex].x, 1.0e-6f) ||
                !NearlyEqual(neutralParams.explaino_roots[rootIndex].y, activeParams.explaino_roots[rootIndex].y, 1.0e-6f)) {
                std::cerr << "phoenix_p_real should stay out of Explaino root-shape authority and act as a runtime memory-term carrier instead\n";
                return 1;
            }
        }
        for (int coeffIndex = 0; coeffIndex < 5; ++coeffIndex) {
            if (!NearlyEqual(neutralParams.poly_coeffs[coeffIndex], activeParams.poly_coeffs[coeffIndex], 1.0e-6f)) {
                std::cerr << "phoenix_p_real should not rewrite the canonical Explaino quartic coefficients in the host-derived-fields seam\n";
                return 1;
            }
        }
    }

    {
        ViewState multView{};
        KernelParams multNeutral{};
        KernelParams multActive{};
        multView.fractal_type = FractalType::explaino_mult;
        multView.explaino_phase = 0.0f;
        multView.explaino_phase_strength = 1.0f;
        ExplainoSeedSetCombined(multView, multNeutral, 5.25);
        ExplainoSeedSetCombined(multView, multActive, 5.25);
        multNeutral.explaino_root_spread = 0.5f;
        multActive.explaino_root_spread = 0.5f;
        multNeutral.explaino_cluster_radius = 0.0f;
        multActive.explaino_cluster_radius = 0.4f;

        UpdateExplainoPolynomial(multView, multNeutral, nullptr);
        UpdateExplainoPolynomial(multView, multActive, nullptr);

        if (NearlyEqual(multNeutral.explaino_roots[0].x, multActive.explaino_roots[0].x, 1.0e-6f) ||
            NearlyEqual(multNeutral.explaino_roots[2].y, multActive.explaino_roots[2].y, 1.0e-6f)) {
            std::cerr << "explaino_cluster_radius should remain root-shape authority for explaino_mult instead of collapsing into a dead slider\n";
            return 1;
        }

        ViewState rationalView{};
        KernelParams rationalNeutral{};
        KernelParams rationalActive{};
        rationalView.fractal_type = FractalType::explaino_rational;
        rationalView.explaino_phase = 0.0f;
        rationalView.explaino_phase_strength = 1.0f;
        ExplainoSeedSetCombined(rationalView, rationalNeutral, 5.25);
        ExplainoSeedSetCombined(rationalView, rationalActive, 5.25);
        rationalNeutral.explaino_root_spread = 0.5f;
        rationalActive.explaino_root_spread = 0.5f;
        rationalNeutral.explaino_cluster_radius = 0.0f;
        rationalActive.explaino_cluster_radius = 0.4f;

        UpdateExplainoPolynomial(rationalView, rationalNeutral, nullptr);
        UpdateExplainoPolynomial(rationalView, rationalActive, nullptr);

        for (int rootIndex = 0; rootIndex < 4; ++rootIndex) {
            if (!NearlyEqual(rationalNeutral.explaino_roots[rootIndex].x, rationalActive.explaino_roots[rootIndex].x, 1.0e-6f) ||
                !NearlyEqual(rationalNeutral.explaino_roots[rootIndex].y, rationalActive.explaino_roots[rootIndex].y, 1.0e-6f)) {
                std::cerr << "explaino_cluster_radius should stay out of root-shape authority for explaino_rational and remain a separate runtime carrier there\n";
                return 1;
            }
        }
        for (int coeffIndex = 0; coeffIndex < 5; ++coeffIndex) {
            if (!NearlyEqual(rationalNeutral.poly_coeffs[coeffIndex], rationalActive.poly_coeffs[coeffIndex], 1.0e-6f)) {
                std::cerr << "explaino_cluster_radius should not rewrite the rational carrier quartic coefficients in host-derived fields\n";
                return 1;
            }
        }
    }

    {
        const FractalType couplingPresetCases[] = {
            FractalType::explaino_all,
            FractalType::explaino_inertial,
            FractalType::explaino_joy,
            FractalType::explaino_fold,
            FractalType::explaino_bell,
        };

        for (FractalType fractalType : couplingPresetCases) {
            ViewState view{};
            KernelParams params{};
            view.fractal_type = fractalType;

            params.momentum_beta = 0.8f;
            params.joy_coupling = 0.9f;
            params.fold_coupling = 0.7f;
            params.bell_coupling = 0.6f;
            params.phoenix_p_real = 0.5667f;
            params.explaino_seed_b = 9.0;
            params.explaino_mix = 0.9f;
            params.explaino_cluster_radius = 0.8f;

            ApplyFractalDerivedFieldsAndSyncHp(view, params, nullptr, false, 0.0);

            for (const auto& coupling : kExplainoCouplingRegistry) {
                const float* value = ResolveExplainoCouplingValue(static_cast<const KernelParams&>(params), coupling.slot);
                const float expected = coupling.carrier_fractal_type == fractalType
                    ? coupling.default_value
                    : coupling.neutral_value;
                if (!value || !NearlyEqual(*value, expected)) {
                    std::cerr << "Deferred Explaino coupling presets should derive from one ownership registry instead of per-selector folklore\n";
                    return 1;
                }
            }
            if (!NearlyEqual(params.phoenix_p_real, 0.0f) ||
                !NearlyEqual(static_cast<float>(params.explaino_seed_b), 1.0f) ||
                !NearlyEqual(params.explaino_mix, 0.5f) ||
                !NearlyEqual(params.explaino_cluster_radius, 0.0f)) {
                    std::cerr << "Deferred Explaino coupling presets must not auto-promote the other deferred parameter classes\n";
                    return 1;
            }
        }
    }

    {
        const FractalType phoenixCarrierPresetCases[] = {
            FractalType::phoenix,
            FractalType::explaino_phoenix,
            FractalType::explaino_joy,
            FractalType::explaino_fold,
            FractalType::explaino_bell,
            FractalType::explaino_ripple,
            FractalType::explaino_splice,
            FractalType::explaino_vortex,
            FractalType::explaino_tension,
        };

        for (FractalType fractalType : phoenixCarrierPresetCases) {
            ViewState view{};
            KernelParams params{};
            view.fractal_type = fractalType;

            params.phoenix_p_real = 0.5667f;
            params.momentum_beta = 0.8f;
            params.joy_coupling = 0.9f;
            params.fold_coupling = 0.7f;
            params.bell_coupling = 0.6f;
            params.explaino_seed_b = 9.0;
            params.explaino_mix = 0.9f;
            params.explaino_cluster_radius = 0.8f;

            ApplyFractalDerivedFieldsAndSyncHp(view, params, nullptr, false, 0.0);

            const PhoenixStepCarrierSelectorDescriptor* phoenixCarrier = FindPhoenixStepCarrierSelectorDescriptor(fractalType);
            const float expectedPhoenix = phoenixCarrier
                ? static_cast<float>(phoenixCarrier->default_value)
                : 0.0f;
            const ExplainoCouplingDescriptor* coupling = FindExplainoCouplingDescriptor(fractalType);
            const float expectedMomentum = coupling && coupling->slot == ExplainoCouplingParamSlot::momentum_beta
                ? coupling->default_value
                : (IsExplainoFamily(fractalType) ? 0.0f : 0.8f);
            const float expectedJoy = coupling && coupling->slot == ExplainoCouplingParamSlot::joy_coupling
                ? coupling->default_value
                : (IsExplainoFamily(fractalType) ? 0.0f : 0.9f);
            const float expectedFold = coupling && coupling->slot == ExplainoCouplingParamSlot::fold_coupling
                ? coupling->default_value
                : (IsExplainoFamily(fractalType) ? 0.0f : 0.7f);
            const float expectedBell = coupling && coupling->slot == ExplainoCouplingParamSlot::bell_coupling
                ? coupling->default_value
                : (IsExplainoFamily(fractalType) ? 0.0f : 0.6f);
            const float expectedSeedB = IsExplainoFamily(fractalType) ? 1.0f : 9.0f;
            const float expectedMix = IsExplainoFamily(fractalType) ? 0.5f : 0.9f;
            if (!NearlyEqual(params.phoenix_p_real, expectedPhoenix) ||
                !NearlyEqual(params.explaino_cluster_radius, 0.0f)) {
                std::cerr << "phoenix_p_real follow-up presets should derive from one shared phoenix-step carrier map instead of the mixed structural registry\n";
                return 1;
            }
            if (!NearlyEqual(params.momentum_beta, expectedMomentum) ||
                !NearlyEqual(params.joy_coupling, expectedJoy) ||
                !NearlyEqual(params.fold_coupling, expectedFold) ||
                !NearlyEqual(params.bell_coupling, expectedBell) ||
                !NearlyEqual(static_cast<float>(params.explaino_seed_b), expectedSeedB) ||
                !NearlyEqual(params.explaino_mix, expectedMix)) {
                std::cerr << "phoenix_p_real follow-up presets must preserve the closed coupling defaults and must not auto-promote dual-seed params\n";
                return 1;
            }
        }

        const FractalType structuralPresetCases[] = {
            FractalType::explaino_all,
            FractalType::explaino_mult,
            FractalType::explaino_rational,
        };

        for (FractalType fractalType : structuralPresetCases) {
            ViewState view{};
            KernelParams params{};
            view.fractal_type = fractalType;

            params.phoenix_p_real = 0.5667f;
            params.momentum_beta = 0.8f;
            params.joy_coupling = 0.9f;
            params.fold_coupling = 0.7f;
            params.bell_coupling = 0.6f;
            params.explaino_seed_b = 9.0;
            params.explaino_mix = 0.9f;
            params.explaino_cluster_radius = 0.8f;

            ApplyFractalDerivedFieldsAndSyncHp(view, params, nullptr, false, 0.0);

            const ExplainoClusterRadiusSelectorDescriptor* structuralCarrier = FindExplainoClusterRadiusSelectorDescriptor(fractalType);
            const float expectedCluster = structuralCarrier
                ? static_cast<float>(structuralCarrier->default_value)
                : 0.0f;
            if (!NearlyEqual(params.phoenix_p_real, 0.0f) ||
                !NearlyEqual(params.explaino_cluster_radius, expectedCluster)) {
                std::cerr << "The cluster-radius split selector map should own the cluster defaults without reviving phoenix_p_real authority\n";
                return 1;
            }
            if (!NearlyEqual(params.momentum_beta, 0.0f) ||
                !NearlyEqual(params.joy_coupling, 0.0f) ||
                !NearlyEqual(params.fold_coupling, 0.0f) ||
                !NearlyEqual(params.bell_coupling, 0.0f) ||
                !NearlyEqual(static_cast<float>(params.explaino_seed_b), 1.0f) ||
                !NearlyEqual(params.explaino_mix, 0.5f)) {
                std::cerr << "The phoenix_p_real follow-up must leave resolved couplings and dual-seed params untouched\n";
                return 1;
            }
        }
    }

    // Normalized-seed regression: the host polynomial update should depend on
    // the split combined seed surface (base + drift), not on raw fractional
    // writes into params.explaino_seed.
    {
        ViewState viewA{};
        ViewState viewB{};
        KernelParams paramsA{};
        KernelParams paramsB{};
        viewA.fractal_type = FractalType::explaino;
        viewB.fractal_type = FractalType::explaino;
        viewA.explaino_phase = 0.0f;
        viewB.explaino_phase = 0.0f;
        viewA.explaino_phase_strength = 1.0f;
        viewB.explaino_phase_strength = 1.0f;

        paramsA.explaino_seed = 5.0;
        viewA.explaino_seed_drift = 0.4f;
        paramsA.explaino_root_spread = 0.5f;
        UpdateExplainoPolynomial(viewA, paramsA, nullptr);
        float rootA0x = paramsA.explaino_roots[0].x;
        float coeffA0 = paramsA.poly_coeffs[0];

        ExplainoSeedSetCombined(viewB, paramsB, 5.4);
        paramsB.explaino_root_spread = 0.5f;
        UpdateExplainoPolynomial(viewB, paramsB, nullptr);
        float rootB0x = paramsB.explaino_roots[0].x;
        float coeffB0 = paramsB.poly_coeffs[0];

        if (!NearlyEqual(rootA0x, rootB0x, 1e-6f)) {
            std::cerr << "Equivalent combined seed splits must produce identical roots\n";
            return 1;
        }
        if (!NearlyEqual(coeffA0, coeffB0, 1e-6f)) {
            std::cerr << "Equivalent combined seed splits must produce identical coefficients\n";
            return 1;
        }
        if (!NearlyEqual((float)paramsB.explaino_seed, 5.0f) || !NearlyEqual(viewB.explaino_seed_drift, 0.4f, 1e-6f)) {
            std::cerr << "ExplainoSeedSetCombined should normalize 5.4 into base=5 drift=0.4\n";
            return 1;
        }
    }

    {
        ViewState viewA{};
        ViewState viewB{};
        ViewState viewTween{};
        KernelParams paramsA{};
        KernelParams paramsB{};
        KernelParams paramsTween{};

        viewA.fractal_type = FractalType::explaino;
        viewB.fractal_type = FractalType::explaino;
        viewTween.fractal_type = FractalType::explaino;

        ExplainoSeedSetCombined(viewA, paramsA, 5.0);
        ExplainoSeedSetCombined(viewB, paramsB, 6.0);
        ExplainoSeedSetCombined(viewTween, paramsTween, 5.25);

        UpdateExplainoPolynomial(viewA, paramsA, nullptr);
        UpdateExplainoPolynomial(viewB, paramsB, nullptr);
        UpdateExplainoPolynomial(viewTween, paramsTween, nullptr);

        // Tween at seed 5.25 should use the wedge-area CDF H(t) as the
        // interpolation curve — smooth, monotone, nonlinear.
        // H(0.25) is the normalized pocket area at t=0.25.
        const float ht = static_cast<float>(ExplainoWedgeTween(0.25));
        const float expectedRoot0X = paramsA.explaino_roots[0].x +
            (paramsB.explaino_roots[0].x - paramsA.explaino_roots[0].x) * ht;
        const float rawLinearRoot0X = paramsA.explaino_roots[0].x +
            (paramsB.explaino_roots[0].x - paramsA.explaino_roots[0].x) * 0.25f;

        if (!NearlyEqual(paramsTween.explaino_roots[0].x, expectedRoot0X, 1e-5f)) {
            std::cerr << "Explaino seed tween should follow wedge-area CDF H(t)\n";
            return 1;
        }
        // Confirm it's genuinely nonlinear (H(0.25) != 0.25)
        if (NearlyEqual(paramsTween.explaino_roots[0].x, rawLinearRoot0X, 1e-5f)) {
            std::cerr << "Explaino seed tween must NOT be raw linear drift\n";
            return 1;
        }
    }

    {
        ViewState viewSeedA{};
        ViewState viewSeedB{};
        ViewState viewDual{};
        KernelParams paramsSeedA{};
        KernelParams paramsSeedB{};
        KernelParams paramsDual{};

        viewSeedA.fractal_type = FractalType::explaino;
        viewSeedB.fractal_type = FractalType::explaino;
        viewDual.fractal_type = FractalType::explaino_dual;

        ExplainoSeedSetCombined(viewSeedA, paramsSeedA, 2.25);
        ExplainoSeedSetCombined(viewSeedB, paramsSeedB, 7.75);
        ExplainoSeedSetCombined(viewDual, paramsDual, 2.25);
        paramsDual.explaino_seed_b = 7.75;

        UpdateExplainoPolynomial(viewSeedA, paramsSeedA, nullptr);
        UpdateExplainoPolynomial(viewSeedB, paramsSeedB, nullptr);

        paramsDual.explaino_mix = 0.0f;
        UpdateExplainoPolynomial(viewDual, paramsDual, nullptr);
        if (!NearlyEqual(paramsDual.explaino_roots[0].x, paramsSeedA.explaino_roots[0].x, 1e-6f) ||
            !NearlyEqual(paramsDual.explaino_roots[2].y, paramsSeedA.explaino_roots[2].y, 1e-6f)) {
            std::cerr << "Explaino-DualSeed mix=0 should match the primary Explaino seed surface\n";
            return 1;
        }

        paramsDual.explaino_mix = 1.0f;
        UpdateExplainoPolynomial(viewDual, paramsDual, nullptr);
        if (!NearlyEqual(paramsDual.explaino_roots[0].x, paramsSeedB.explaino_roots[0].x, 1e-6f) ||
            !NearlyEqual(paramsDual.explaino_roots[2].y, paramsSeedB.explaino_roots[2].y, 1e-6f)) {
            std::cerr << "Explaino-DualSeed mix=1 should match the secondary Explaino seed surface\n";
            return 1;
        }

        paramsDual.explaino_mix = 0.5f;
        UpdateExplainoPolynomial(viewDual, paramsDual, nullptr);
        if (NearlyEqual(paramsDual.explaino_roots[0].x, paramsSeedA.explaino_roots[0].x, 1e-6f) ||
            NearlyEqual(paramsDual.explaino_roots[0].x, paramsSeedB.explaino_roots[0].x, 1e-6f)) {
            std::cerr << "Explaino-DualSeed mix=0.5 should blend away from either endpoint seed surface\n";
            return 1;
        }
    }

    {
        ViewState viewExplaino{};
        ViewState viewCanonical{};
        KernelParams paramsExplaino{};
        KernelParams paramsCanonical{};

        viewExplaino.fractal_type = FractalType::explaino;
        viewCanonical.fractal_type = FractalType::explaino_all;

        ExplainoSeedSetCombined(viewExplaino, paramsExplaino, 2.25);
        ExplainoSeedSetCombined(viewCanonical, paramsCanonical, 2.25);
        paramsCanonical.explaino_seed_b = 9.0;
        paramsCanonical.explaino_mix = 1.0f;

        UpdateExplainoPolynomial(viewExplaino, paramsExplaino, nullptr);
        UpdateExplainoPolynomial(viewCanonical, paramsCanonical, nullptr);

        for (int rootIndex = 0; rootIndex < 4; ++rootIndex) {
            if (!NearlyEqual(paramsCanonical.explaino_roots[rootIndex].x, paramsExplaino.explaino_roots[rootIndex].x, 1e-6f) ||
                !NearlyEqual(paramsCanonical.explaino_roots[rootIndex].y, paramsExplaino.explaino_roots[rootIndex].y, 1e-6f)) {
                std::cerr << "Explaino-all should ignore deferred dual-seed params until a separate ownership model says otherwise\n";
                return 1;
            }
        }
        for (int coeffIndex = 0; coeffIndex < 5; ++coeffIndex) {
            if (!NearlyEqual(paramsCanonical.poly_coeffs[coeffIndex], paramsExplaino.poly_coeffs[coeffIndex], 1e-6f)) {
                std::cerr << "Explaino-all should preserve the canonical quartic when deferred dual-seed params are changed off-carrier\n";
                return 1;
            }
        }
    }

    // Explaino-Halley must get preset defaults (custom poly, basin coloring)
    {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::explaino_halley;
        params.explaino_seed = 3.0;
        bool dirty = false;
        ApplyFractalPresetDefaults(view, params, &dirty);
        UpdateExplainoPolynomial(view, params, &dirty);

        if (!dirty) {
            std::cerr << "Explaino-Halley defaults should mark dirty\n";
            return 1;
        }
        if (params.poly_kind != PolyKind::custom) {
            std::cerr << "Explaino-Halley should force custom polynomial\n";
            return 1;
        }
        if (params.max_iter != 500) {
            std::cerr << "Explaino-Halley should use 500 max_iter (got " << params.max_iter << ")\n";
            return 1;
        }
        if (params.coloring_mode != ColoringMode::joy_basins) {
            std::cerr << "Explaino-Halley should default to joy_basins\n";
            return 1;
        }
        if (params.explaino_root_count != 4 || !NearlyEqual(params.poly_coeffs[4], 1.0f)) {
            std::cerr << "Explaino-Halley should derive the Explaino quartic\n";
            return 1;
        }
    }

    return 0;
}
