#include "../src/fractal_runtime_validation.h"

#include <iostream>
#include <limits>
#include <string>

namespace {
void SetCounterfactualPairQuarticPreset(KernelParams& params) {
    params.poly_coeffs[0] = -1.0f;
    params.poly_coeffs[1] = 0.0f;
    params.poly_coeffs[2] = 0.0f;
    params.poly_coeffs[3] = 0.0f;
    params.poly_coeffs[4] = 1.0f;
}

void SetProjectionAndFlowQuarticPreset(KernelParams& params) {
    params.poly_coeffs[0] = -1.0f;
    params.poly_coeffs[1] = 0.0f;
    params.poly_coeffs[2] = 0.0f;
    params.poly_coeffs[3] = 0.0f;
    params.poly_coeffs[4] = 1.0f;
}
}

int main() {
    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        if (!ValidateFractalRuntimeState(view, params, &error)) {
            std::cerr << "Default Explaino runtime state should validate, got: " << error << "\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        params.max_iter = 0;
        if (ValidateFractalRuntimeState(view, params, &error) || error != "max_iter must be > 0") {
            std::cerr << "Expected max_iter validation to fail fast\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::phoenix;
        params.coloring_mode = ColoringMode::joy_basins;
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "selected coloring_mode is not valid for fractal_type") {
            std::cerr << "Expected escape-time fractals to reject basin coloring in shared runtime validation\n";
            return 1;
        }
    }

    {
        struct FoldMixCase {
            FractalType fractalType;
            float KernelParams::*field;
            const char* expectedError;
        };
        const FoldMixCase cases[] = {
            {FractalType::burning_ship, &KernelParams::burning_ship_fold_mix, "burning_ship_fold_mix must be finite and in [0,1]"},
            {FractalType::celtic_mandelbrot, &KernelParams::celtic_abs_mix, "celtic_abs_mix must be finite and in [0,1]"},
            {FractalType::perpendicular_burning_ship, &KernelParams::perpendicular_fold_mix, "perpendicular_fold_mix must be finite and in [0,1]"},
        };
        for (const FoldMixCase& testCase : cases) {
            ViewState view{};
            KernelParams params{};
            std::string error;
            view.fractal_type = testCase.fractalType;
            params.coloring_mode = ColoringMode::smooth_escape;
            params.*(testCase.field) = 0.0f;
            if (!ValidateFractalRuntimeState(view, params, &error)) {
                std::cerr << "Expected fixed-family fold/mix validation to accept lower bound: " << error << "\n";
                return 1;
            }
            params.*(testCase.field) = 1.0f;
            if (!ValidateFractalRuntimeState(view, params, &error)) {
                std::cerr << "Expected fixed-family fold/mix validation to accept default upper bound: " << error << "\n";
                return 1;
            }
            params.*(testCase.field) = -0.01f;
            error.clear();
            if (ValidateFractalRuntimeState(view, params, &error) || error != testCase.expectedError) {
                std::cerr << "Expected fixed-family fold/mix validation to reject below range\n";
                return 1;
            }
            params.*(testCase.field) = 1.01f;
            error.clear();
            if (ValidateFractalRuntimeState(view, params, &error) || error != testCase.expectedError) {
                std::cerr << "Expected fixed-family fold/mix validation to reject above range\n";
                return 1;
            }
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::explaino_nova;
        params.coloring_mode = ColoringMode::smooth_escape;
        params.nova_alpha = 0.0f;
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "nova_alpha must be finite and in (0,2]") {
            std::cerr << "Expected Explaino-Nova to share the Nova alpha validation contract\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::nova;
        params.coloring_mode = ColoringMode::smooth_escape;
        params.nova_alpha = 2.0f;
        if (!ValidateFractalRuntimeState(view, params, &error)) {
            std::cerr << "Expected Nova validation to accept the kernel-valid alpha cap, got: " << error << "\n";
            return 1;
        }
        params.nova_alpha = 2.1f;
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "nova_alpha must be finite and in (0,2]") {
            std::cerr << "Expected Nova validation to reject alpha above the kernel-valid cap\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::lambda_map;
        params.coloring_mode = ColoringMode::smooth_escape;
        params.lambda_real = 5.0f;
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "lambda_real/lambda_imag must be finite and in [-4,4]") {
            std::cerr << "Expected Lambda validation to enforce the shared finite/range contract\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::multibrot;
        params.coloring_mode = ColoringMode::smooth_escape;
        const float acceptedRealPowers[] = {0.01f, 1.5f, 12.0f, 32.0f};
        for (float power : acceptedRealPowers) {
            params.multibrot_power_float = power;
            params.multibrot_power_imag = 0.0f;
            error.clear();
            if (!ValidateFractalRuntimeState(view, params, &error)) {
                std::cerr << "Expected Multibrot validation to accept real power " << power << ", got: " << error << "\n";
                return 1;
            }
        }

        params.multibrot_power_float = 0.0f;
        error.clear();
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "multibrot_power_float must be finite and in [0.01,32]") {
            std::cerr << "Expected Multibrot validation to reject zero real power\n";
            return 1;
        }
        params.multibrot_power_float = -0.1f;
        error.clear();
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "multibrot_power_float must be finite and in [0.01,32]") {
            std::cerr << "Expected Multibrot validation to reject negative real power\n";
            return 1;
        }
        params.multibrot_power_float = 32.1f;
        error.clear();
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "multibrot_power_float must be finite and in [0.01,32]") {
            std::cerr << "Expected Multibrot validation to reject real power above the hard cap\n";
            return 1;
        }

        params.multibrot_power_float = 3.0f;
        const float acceptedImagPowers[] = {-4.0f, 0.0f, 4.0f};
        for (float power : acceptedImagPowers) {
            params.multibrot_power_imag = power;
            error.clear();
            if (!ValidateFractalRuntimeState(view, params, &error)) {
                std::cerr << "Expected Multibrot validation to accept imaginary power " << power << ", got: " << error << "\n";
                return 1;
            }
        }
        params.multibrot_power_imag = 4.1f;
        error.clear();
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "multibrot_power_imag must be finite and in [-4,4]") {
            std::cerr << "Expected Multibrot validation to reject imaginary power above the hard cap\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        params.coloring_mode = ColoringMode::smooth_escape;
        const FractalType collatzLanes[] = {FractalType::collatz, FractalType::explaino_collatz_direct};
        for (FractalType collatzLane : collatzLanes) {
            view.fractal_type = collatzLane;
            const float acceptedStrengths[] = {0.0f, 1.0f, 4.0f};
            for (float strength : acceptedStrengths) {
                params.collatz_transition_strength = strength;
                error.clear();
                if (!ValidateFractalRuntimeState(view, params, &error)) {
                    std::cerr << "Expected Collatz validation to accept transition strength " << strength << ", got: " << error << "\n";
                    return 1;
                }
            }
            params.collatz_transition_strength = -0.01f;
            error.clear();
            if (ValidateFractalRuntimeState(view, params, &error) ||
                error != "collatz_transition_strength must be finite and in [0,4]") {
                std::cerr << "Expected Collatz validation to reject transition strength below the hard floor\n";
                return 1;
            }
            params.collatz_transition_strength = 4.01f;
            error.clear();
            if (ValidateFractalRuntimeState(view, params, &error) ||
                error != "collatz_transition_strength must be finite and in [0,4]") {
                std::cerr << "Expected Collatz validation to reject transition strength above the hard cap\n";
                return 1;
            }
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::spider;
        params.coloring_mode = ColoringMode::smooth_escape;
        const float acceptedFeedback[] = {-2.0f, 0.5f, 2.0f};
        for (float feedback : acceptedFeedback) {
            params.spider_feedback = feedback;
            error.clear();
            if (!ValidateFractalRuntimeState(view, params, &error)) {
                std::cerr << "Expected Spider validation to accept feedback " << feedback << ", got: " << error << "\n";
                return 1;
            }
        }
        params.spider_feedback = 2.1f;
        error.clear();
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "spider_feedback must be finite and in [-2,2]") {
            std::cerr << "Expected Spider validation to reject feedback above the hard cap\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::explaino_rational_escape;
        params.coloring_mode = ColoringMode::smooth_escape;
        const int acceptedPowers[] = {1, 3, 6};
        for (int power : acceptedPowers) {
            params.explaino_rational_escape_denominator_power = power;
            error.clear();
            if (!ValidateFractalRuntimeState(view, params, &error)) {
                std::cerr << "Expected Explaino Rational Escape validation to accept denominator power " << power << ", got: " << error << "\n";
                return 1;
            }
        }
        params.explaino_rational_escape_denominator_power = 0;
        error.clear();
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "explaino_rational_escape_denominator_power must be in [1,6]") {
            std::cerr << "Expected Explaino Rational Escape validation to reject denominator power below the hard floor\n";
            return 1;
        }
        params.explaino_rational_escape_denominator_power = 7;
        error.clear();
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "explaino_rational_escape_denominator_power must be in [1,6]") {
            std::cerr << "Expected Explaino Rational Escape validation to reject denominator power above the hard cap\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::phoenix;
        params.coloring_mode = ColoringMode::smooth_escape;
        params.phoenix_p_real = 1.5f;
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "phoenix_p_real/imag must be in [-1,1]") {
            std::cerr << "Expected Phoenix validation to enforce the shared memory-term domain contract\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::explaino_phoenix;
        params.coloring_mode = ColoringMode::joy_basins;
        params.phoenix_p_real = 1.5f;
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "phoenix_p_real/imag must be in [-1,1]") {
            std::cerr << "Expected Explaino structural carriers to share the Phoenix memory-term domain contract\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::explaino_joy;
        params.coloring_mode = ColoringMode::joy_basins;
        params.phoenix_p_real = 1.5f;
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "phoenix_p_real/imag must be in [-1,1]") {
            std::cerr << "Expected the zero-default Explaino phoenix-step carriers to share the Phoenix memory-term domain contract\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::explaino_mult;
        params.coloring_mode = ColoringMode::joy_basins;
        params.explaino_cluster_radius = -0.1f;
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "explaino_cluster_radius must be finite and in [0,2]") {
            std::cerr << "Expected Explaino-Mult to enforce the explicit cluster-radius split selector domain contract\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::explaino_rational;
        params.coloring_mode = ColoringMode::joy_basins;
        params.explaino_cluster_radius = -0.1f;
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "explaino_cluster_radius must be finite and in [0,2]") {
            std::cerr << "Expected Explaino-Rational to enforce the explicit cluster-radius split selector domain contract\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::explaino;
        params.explaino_mix = 1.5f;
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "explaino_mix must be finite and in [0,1]") {
            std::cerr << "Expected Explaino validation to enforce the shared mix contract\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::explaino;
        params.explaino_warp_strength = 1.5f;
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "explaino_warp_strength must be finite and in [0,1]") {
            std::cerr << "Expected Explaino warp validation to enforce the shared [0,1] contract\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::counterfactual_pair;
        params.coloring_mode = ColoringMode::root_basin;
        params.counterfactual_pair_root_family = CounterfactualPairRootFamily::quartic_unit_roots;
        params.poly_kind = PolyKind::z4_minus_1;
        SetCounterfactualPairQuarticPreset(params);
        if (!ValidateFractalRuntimeState(view, params, &error)) {
            std::cerr << "Expected Counterfactual Pair validation to accept the quartic root-family preset, got: " << error << "\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::counterfactual_pair;
        params.coloring_mode = ColoringMode::root_basin;
        params.counterfactual_pair_root_family = CounterfactualPairRootFamily::quartic_unit_roots;
        params.poly_kind = PolyKind::z3_minus_1;
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "counterfactual_pair root family and poly_kind must agree") {
            std::cerr << "Expected Counterfactual Pair validation to reject root-family/poly-kind drift\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::explaino_counterfactual_pair;
        params.coloring_mode = ColoringMode::root_basin;
        params.counterfactual_pair_root_family = CounterfactualPairRootFamily::quartic_unit_roots;
        params.poly_kind = PolyKind::custom;
        params.explaino_root_count = 4;
        params.poly_coeffs[0] = 1.0f;
        params.poly_coeffs[1] = 0.0f;
        params.poly_coeffs[2] = 0.0f;
        params.poly_coeffs[3] = 1.0f;
        params.poly_coeffs[4] = 1.0f;
        if (!ValidateFractalRuntimeState(view, params, &error)) {
            std::cerr << "Expected Explaino Counterfactual validation to accept a quartic custom carrier polynomial, got: " << error << "\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::explaino_counterfactual_pair;
        params.coloring_mode = ColoringMode::root_basin;
        params.counterfactual_pair_root_family = CounterfactualPairRootFamily::quartic_unit_roots;
        params.poly_kind = PolyKind::custom;
        params.explaino_root_count = 3;
        params.poly_coeffs[3] = 1.0f;
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "explaino_counterfactual_pair root family and explaino_root_count must agree") {
            std::cerr << "Expected Explaino Counterfactual validation to reject stale cubic custom state under quartic root-family authority\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::projection_and_flow;
        params.coloring_mode = ColoringMode::root_basin;
        params.projection_and_flow_root_family = ProjectionAndFlowRootFamily::quartic_unit_roots;
        params.poly_kind = PolyKind::z4_minus_1;
        params.projection_and_flow_target_radius = 1.75f;
        params.projection_and_flow_pressure_threshold = 0.5f;
        SetProjectionAndFlowQuarticPreset(params);
        if (!ValidateFractalRuntimeState(view, params, &error)) {
            std::cerr << "Expected Projection-and-Flow validation to accept the bounded quartic hardening controls, got: " << error << "\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::projection_and_flow;
        params.coloring_mode = ColoringMode::root_basin;
        params.projection_and_flow_root_family = ProjectionAndFlowRootFamily::quartic_unit_roots;
        params.poly_kind = PolyKind::z3_minus_1;
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "projection_and_flow root family and poly_kind must agree") {
            std::cerr << "Expected Projection-and-Flow validation to reject root-family/poly-kind drift\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::projection_and_flow;
        params.coloring_mode = ColoringMode::root_basin;
        params.projection_and_flow_root_family = ProjectionAndFlowRootFamily::quartic_unit_roots;
        params.poly_kind = PolyKind::z4_minus_1;
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "projection_and_flow root family must own the shipped polynomial preset") {
            std::cerr << "Expected Projection-and-Flow validation to reject stale cubic coefficients under quartic controls\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::explaino_projection_and_flow;
        params.coloring_mode = ColoringMode::root_basin;
        params.projection_and_flow_root_family = ProjectionAndFlowRootFamily::quartic_unit_roots;
        params.poly_kind = PolyKind::custom;
        params.projection_and_flow_target_radius = 1.75f;
        params.projection_and_flow_pressure_threshold = 0.5f;
        params.explaino_seed = 3.0;
        params.explaino_warp_strength = 0.25f;
        params.explaino_damping = 0.75f;
        params.explaino_root_count = 4;
        params.poly_coeffs[0] = 1.0f;
        params.poly_coeffs[1] = 0.0f;
        params.poly_coeffs[2] = 0.0f;
        params.poly_coeffs[3] = 1.0f;
        params.poly_coeffs[4] = 1.0f;
        if (!ValidateFractalRuntimeState(view, params, &error)) {
            std::cerr << "Expected Explaino Projection-and-Flow validation to accept a custom Explaino carrier polynomial plus the shared Projection-and-Flow controls, got: " << error << "\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::explaino_projection_and_flow;
        params.coloring_mode = ColoringMode::root_basin;
        params.projection_and_flow_root_family = ProjectionAndFlowRootFamily::quartic_unit_roots;
        params.poly_kind = PolyKind::z4_minus_1;
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "explaino_projection_and_flow must preserve custom Explaino polynomial authority") {
            std::cerr << "Expected Explaino Projection-and-Flow validation to reject falling back to the standalone preset polynomial seam\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::explaino_projection_and_flow;
        params.coloring_mode = ColoringMode::root_basin;
        params.projection_and_flow_root_family = ProjectionAndFlowRootFamily::quartic_unit_roots;
        params.poly_kind = PolyKind::custom;
        params.explaino_root_count = 4;
        params.poly_coeffs[0] = 1.0f;
        params.poly_coeffs[1] = 0.0f;
        params.poly_coeffs[2] = 0.0f;
        params.poly_coeffs[3] = 1.0f;
        params.poly_coeffs[4] = 0.0f;
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "explaino_projection_and_flow root family must match the custom carrier polynomial degree") {
            std::cerr << "Expected Explaino Projection-and-Flow validation to reject a quartic root-family claim backed by a cubic custom polynomial\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::projection_and_flow;
        params.coloring_mode = ColoringMode::root_basin;
        params.projection_and_flow_target_radius = 0.0f;
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "projection_and_flow_target_radius must be finite and in (0,4]") {
            std::cerr << "Expected Projection-and-Flow validation to reject non-positive target radius\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::projection_and_flow;
        params.coloring_mode = ColoringMode::root_basin;
        params.projection_and_flow_pressure_threshold = -0.1f;
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "projection_and_flow_pressure_threshold must be finite and in [0,8]") {
            std::cerr << "Expected Projection-and-Flow validation to reject negative pressure threshold\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::magnet;
        params.coloring_mode = ColoringMode::smooth_escape;
        params.magnet_seed_real = -0.05f;
        params.magnet_seed_imag = 0.02f;
        params.magnet_relaxation = 0.75f;
        params.magnet_bailout = 18.0f;
        if (!ValidateFractalRuntimeState(view, params, &error)) {
            std::cerr << "Expected Magnet validation to accept bounded Type I controls, got: " << error << "\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::magnet;
        params.coloring_mode = ColoringMode::smooth_escape;
        params.magnet_seed_real = std::numeric_limits<float>::infinity();
        if (ValidateFractalRuntimeState(view, params, &error) || error != "magnet_seed_real/imag must be finite") {
            std::cerr << "Expected Magnet validation to reject non-finite seeds\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::magnet;
        params.coloring_mode = ColoringMode::smooth_escape;
        params.magnet_relaxation = 0.0f;
        if (ValidateFractalRuntimeState(view, params, &error) || error != "magnet_relaxation must be finite and in [0.05,1.5]") {
            std::cerr << "Expected Magnet validation to reject out-of-range relaxation\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::magnet;
        params.coloring_mode = ColoringMode::smooth_escape;
        params.magnet_bailout = 1.0f;
        if (ValidateFractalRuntimeState(view, params, &error) || error != "magnet_bailout must be finite and in [2,64]") {
            std::cerr << "Expected Magnet validation to reject out-of-range bailout\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::julia;
        params.coloring_mode = ColoringMode::smooth_escape;
        params.julia_c_real = 0.285f;
        params.julia_c_imag = 0.01f;
        if (!ValidateFractalRuntimeState(view, params, &error)) {
            std::cerr << "Expected Julia validation to accept finite user-owned constants, got: " << error << "\n";
            return 1;
        }
        params.julia_c_imag = std::numeric_limits<float>::quiet_NaN();
        if (ValidateFractalRuntimeState(view, params, &error) || error != "julia_c_real/imag must be finite") {
            std::cerr << "Expected Julia validation to reject non-finite constants\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        std::string error;
        view.fractal_type = FractalType::explaino_julia;
        params.coloring_mode = ColoringMode::smooth_escape;
        params.explaino_julia_constant_mode = ExplainoJuliaConstantMode::custom;
        params.explaino_julia_c_real = 0.285f;
        params.explaino_julia_c_imag = 0.01f;
        if (!ValidateFractalRuntimeState(view, params, &error)) {
            std::cerr << "Expected Explaino Julia validation to accept finite custom constants, got: " << error << "\n";
            return 1;
        }
        params.explaino_julia_c_imag = std::numeric_limits<float>::quiet_NaN();
        if (ValidateFractalRuntimeState(view, params, &error) || error != "explaino_julia_c_real/imag must be finite when custom mode is active") {
            std::cerr << "Expected Explaino Julia validation to reject non-finite custom constants\n";
            return 1;
        }
        params.explaino_julia_c_imag = 0.01f;
        params.explaino_julia_constant_mode = static_cast<ExplainoJuliaConstantMode>(99);
        if (ValidateFractalRuntimeState(view, params, &error) || error != "explaino_julia_constant_mode must be seeded or custom") {
            std::cerr << "Expected Explaino Julia validation to reject invalid authority mode\n";
            return 1;
        }
    }

    std::cout << "test_fractal_runtime_validation: all passed\n";
    return 0;
}
