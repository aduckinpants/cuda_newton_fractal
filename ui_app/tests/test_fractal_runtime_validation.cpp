#include "../src/fractal_runtime_validation.h"

#include <iostream>
#include <string>

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
            error != "nova_alpha must be finite and in (0,5]") {
            std::cerr << "Expected Explaino-Nova to share the Nova alpha validation contract\n";
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

    std::cout << "test_fractal_runtime_validation: all passed\n";
    return 0;
}