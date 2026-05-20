#include "../src/fractal_runtime_validation.h"

#include <iostream>
#include <limits>
#include <string>

namespace {
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
        params.multibrot_power_float = 13.0f;
        if (ValidateFractalRuntimeState(view, params, &error) ||
            error != "multibrot_power_float must be finite and in [2,12]") {
            std::cerr << "Expected Multibrot validation to enforce the shared power range contract\n";
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

    std::cout << "test_fractal_runtime_validation: all passed\n";
    return 0;
}
