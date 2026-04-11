#include "../src/diagnostics_state_io.h"

#include <filesystem>
#include <fstream>
#include <iostream>

static bool NearlyEqual(double a, double b, double eps = 1.0e-9) {
    return std::fabs(a - b) <= eps;
}

static void WriteMinimalStateWithExtraParams(const std::filesystem::path& statePath, const std::string& extraParamsJson) {
  std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
  file << "{\n"
      "  \"state_version\": 3,\n"
      "  \"fractal_type\": \"newton\",\n"
      "  \"view\": {\n"
      "    \"center_x\": 0,\n"
      "    \"center_y\": 0,\n"
      "    \"zoom\": 1,\n"
      "    \"rotation_degrees\": 0,\n"
      "    \"center_hp_x\": 0,\n"
      "    \"center_hp_y\": 0,\n"
      "    \"log2_zoom\": 0,\n"
      "    \"explaino_phase\": 0,\n"
      "    \"explaino_seed_drift\": 0,\n"
      "    \"explaino_seed_tween\": true\n"
      "  },\n"
      "  \"params\": {\n"
      "    \"max_iter\": 500,\n"
      "    \"epsilon\": 0.000001,\n"
      "    \"exposure\": 1.0,\n"
      "    \"poly_kind\": 0,\n"
      "    \"coloring_mode\": \"root_basin\",\n"
      "    \"nova_alpha\": 0.5,\n"
      "    \"phoenix_p_real\": 0.0,\n"
      "    \"phoenix_p_imag\": 0.0,\n"
      "    \"multibrot_power\": 3,\n"
      "    \"multibrot_power_float\": 3.0,\n"
      "    \"lambda_real\": 0.0,\n"
      "    \"lambda_imag\": 0.0,\n"
      "    \"explaino_seed\": 0,\n"
      "    \"explaino_warp_strength\": 0,\n"
      "    \"explaino_root_count\": 0,\n"
      "    \"poly_coeffs\": [-1, 0, 0, 1, 0]";
  if (!extraParamsJson.empty()) {
    file << ",\n" << extraParamsJson;
  }
  file << "\n  },\n"
      "  \"render\": {\n"
      "    \"width\": 1024,\n"
      "    \"height\": 768,\n"
      "    \"block_size\": 256,\n"
      "    \"device_id\": 0\n"
      "  }\n"
      "}";
}

static bool ExpectLoadDiagnosticsStateFailure(const std::filesystem::path& statePath,
  const std::string& expectedErrorSubstring,
  std::string* outObservedError) {
  ViewState view{};
  KernelParams params{};
  RenderSettings render{};
  std::string error;
  if (LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
    if (outObservedError) *outObservedError = "load unexpectedly succeeded";
    return false;
  }
  if (error.find(expectedErrorSubstring) == std::string::npos) {
    if (outObservedError) *outObservedError = error;
    return false;
  }
  return true;
}

int main() {
    namespace fs = std::filesystem;

    const fs::path tempRoot = fs::temp_directory_path() / "cuda_newton_fractal_clone_state_io_tests";
    fs::create_directories(tempRoot);

    {
        const fs::path statePath = tempRoot / "loaded_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
        "state_version": 3,
  "fractal_type": "phoenix",
  "view": {
    "center_x": 0.125,
    "center_y": -0.375,
    "zoom": 2.5,
    "rotation_degrees": 12.0,
    "center_hp_x": 0.125,
    "center_hp_y": -0.375,
    "log2_zoom": 1.3219280948873624,
    "explaino_phase": 0.75,
    "explaino_seed_drift": 0.125,
    "explaino_seed_tween": false,
    "auto_max_iter": true,
    "explaino_phase_strength": -2.5
  },
  "params": {
    "max_iter": 1200,
    "epsilon": 0.000001,
    "exposure": 1.6,
    "poly_kind": 2,
    "coloring_mode": "smooth_escape",
    "nova_alpha": 0.55,
    "phoenix_p_real": 0.5667,
    "phoenix_p_imag": -0.125,
    "multibrot_power": 5,
    "explaino_seed": 4,
    "explaino_warp_strength": 0.3,
    "explaino_root_spread": 1.75,
    "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1440,
    "height": 900,
    "block_size": 512,
    "device_id": 1,
    "interaction_debounce_ms": 420,
    "preview_target_fps": 24.0,
    "preview_min_scale": 0.4
  },
  "stats": {
    "last_render_ms": 0,
    "last_iters_avg": 0,
    "last_device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "LoadDiagnosticsStateFile failed: " << error << "\n";
            return 1;
        }

        if (view.fractal_type != FractalType::phoenix) {
            std::cerr << "fractal_type mismatch\n";
            return 1;
        }
        if (!NearlyEqual(view.center.x, 0.125f, 1.0e-6) || !NearlyEqual(view.center.y, -0.375f, 1.0e-6)) {
            std::cerr << "view center mismatch\n";
            return 1;
        }
        if (!NearlyEqual(view.center_hp_x, 0.125, 1.0e-12) || !NearlyEqual(view.center_hp_y, -0.375, 1.0e-12)) {
            std::cerr << "view HP center mismatch\n";
            return 1;
        }
        if (!NearlyEqual(view.log2_zoom, 1.3219280948873624, 1.0e-12)) {
            std::cerr << "view log2_zoom mismatch\n";
            return 1;
        }
        if (!NearlyEqual(view.explaino_phase_strength, -2.5f, 1.0e-6)) {
          std::cerr << "view explaino_phase_strength mismatch\n";
          return 1;
        }
        if (view.explaino_seed_tween != false) {
            std::cerr << "view explaino_seed_tween mismatch\n";
            return 1;
        }
        if (!view.auto_max_iter) {
          std::cerr << "view auto_max_iter should load from saved state when present\n";
          return 1;
        }
        if (view.explaino_alive) {
          std::cerr << "view explaino_alive should default to false when missing from saved state\n";
          return 1;
        }
        if (view.auto_increment_seed) {
          std::cerr << "view auto_increment_seed should default to false when missing from saved state\n";
          return 1;
        }
        if (!NearlyEqual(view.explaino_seed_rate, 0.001f, 1.0e-6)) {
          std::cerr << "new Explaino seed rate default should survive older saved states\n";
          return 1;
        }
        if (params.max_iter != 1200 || !NearlyEqual(params.exposure, 1.6f, 1.0e-6)) {
            std::cerr << "params mismatch\n";
            return 1;
        }
        if (params.poly_kind != PolyKind::custom) {
            std::cerr << "params poly_kind mismatch\n";
            return 1;
        }
        if (params.coloring_mode != ColoringMode::smooth_escape) {
          std::cerr << "params coloring_mode mismatch\n";
          return 1;
        }
        if (!NearlyEqual(params.nova_alpha, 0.55f, 1.0e-6) || !NearlyEqual(params.phoenix_p_real, 0.5667f, 1.0e-6) || !NearlyEqual(params.phoenix_p_imag, -0.125f, 1.0e-6)) {
          std::cerr << "params family-specific values mismatch\n";
          return 1;
        }
        if (params.multibrot_power != 5) {
          std::cerr << "params multibrot_power mismatch\n";
          return 1;
        }
        if (!NearlyEqual(params.explaino_root_spread, 1.75f, 1.0e-6)) {
          std::cerr << "params explaino_root_spread mismatch\n";
          return 1;
        }
        if (!NearlyEqual(params.multibrot_power_float, 5.0f, 1.0e-6)) {
          std::cerr << "params multibrot_power_float should fall back to legacy int value\n";
          return 1;
        }
        if (render.resolution.x != 1440 || render.resolution.y != 900 || render.block_size != 512 || render.device_id != 1) {
            std::cerr << "render mismatch\n";
            return 1;
        }
        if (render.interaction_debounce_ms != 420 || !NearlyEqual(render.preview_target_fps, 24.0f, 1.0e-6) || !NearlyEqual(render.preview_min_scale, 0.4f, 1.0e-6)) {
          std::cerr << "render adaptive preview pacing mismatch\n";
          return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "negative_explaino_seed_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "explaino_dual",
  "view": {
    "center_x": 0.0,
    "center_y": 0.0,
    "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0,
    "center_hp_y": 0.0,
    "log2_zoom": 0.0,
    "explaino_phase": 0.5,
    "explaino_seed_drift": 0.25,
    "explaino_seed_tween": true,
    "explaino_phase_strength": -1.25
  },
  "params": {
    "max_iter": 650,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 2,
    "coloring_mode": "joy_basins",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "multibrot_power_float": 3.0,
    "lambda_real": 0.0,
    "lambda_imag": 0.0,
    "explaino_seed": -3.0,
    "explaino_seed_b": -7.5,
    "explaino_mix": 0.35,
    "explaino_warp_strength": 0.2,
    "explaino_root_spread": 2.25,
    "explaino_root_count": 4,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  },
  "stats": {
    "last_render_ms": 0,
    "last_iters_avg": 0,
    "last_device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected negative Explaino seeds to load: " << error << "\n";
            return 1;
        }
        if (!NearlyEqual(params.explaino_seed, -3.0, 1.0e-9) || !NearlyEqual(params.explaino_seed_b, -7.5, 1.0e-9)) {
            std::cerr << "Expected Explaino seed controls to preserve negative values\n";
            return 1;
        }
        if (!NearlyEqual(view.explaino_phase_strength, -1.25f, 1.0e-6) || !NearlyEqual(params.explaino_root_spread, 2.25f, 1.0e-6)) {
            std::cerr << "Expected new Explaino state fields to round-trip from state JSON\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "legacy_explaino_nova_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "explaino_nova",
  "view": {
    "center_x": 0.0,
    "center_y": 0.0,
    "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0,
    "center_hp_y": 0.0,
    "log2_zoom": 0.0,
    "explaino_phase": 0.0,
    "explaino_seed_drift": 0.0,
    "explaino_seed_tween": true,
    "explaino_phase_strength": 1.0
  },
  "params": {
    "max_iter": 300,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 2,
    "coloring_mode": "smooth_escape",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "multibrot_power_float": 3.0,
    "lambda_real": 0.0,
    "lambda_imag": 0.0,
    "explaino_seed": 0.0,
    "explaino_seed_b": 1.0,
    "explaino_mix": 0.5,
    "explaino_warp_strength": 0.0,
    "explaino_root_spread": 0.5,
    "explaino_root_count": 4,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  },
  "stats": {
    "last_render_ms": 0,
    "last_iters_avg": 0,
    "last_device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected legacy Explaino-Nova state to load: " << error << "\n";
            return 1;
        }
        if (!view.auto_max_iter) {
            std::cerr << "Legacy Explaino-Nova states should default auto_max_iter on when the field is missing\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "multibrot_float_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 2,
  "fractal_type": "multibrot",
  "view": {
    "center_x": 0,
    "center_y": 0,
    "zoom": 1,
    "rotation_degrees": 0,
    "center_hp_x": 0,
    "center_hp_y": 0,
    "log2_zoom": 0,
    "explaino_phase": 0,
    "explaino_seed_drift": 0,
    "explaino_seed_tween": false
  },
  "params": {
    "max_iter": 1000,
    "epsilon": 0.000001,
    "exposure": 1.4,
    "poly_kind": 0,
    "coloring_mode": "smooth_escape",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "multibrot_power_float": 2.5,
    "explaino_seed": 0,
    "explaino_warp_strength": 0.0,
    "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  },
  "stats": {
    "last_render_ms": 0,
    "last_iters_avg": 0,
    "last_device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected non-integer multibrot state to load: " << error << "\n";
            return 1;
        }
        if (view.fractal_type != FractalType::multibrot) {
            std::cerr << "Expected multibrot fractal type to load\n";
            return 1;
        }
        if (!NearlyEqual(params.multibrot_power_float, 2.5f, 1.0e-6)) {
            std::cerr << "Expected multibrot_power_float to round-trip from saved state\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "lambda_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 2,
  "fractal_type": "lambda",
  "view": {
    "center_x": 0.5,
    "center_y": 0.0,
    "zoom": 2.0,
    "rotation_degrees": 0,
    "center_hp_x": 0.5,
    "center_hp_y": 0.0,
    "log2_zoom": 1.0,
    "explaino_phase": 0,
    "explaino_seed_drift": 0,
    "explaino_seed_tween": false
  },
  "params": {
    "max_iter": 1200,
    "epsilon": 0.000001,
    "exposure": 1.4,
    "poly_kind": 0,
    "coloring_mode": "smooth_escape",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "multibrot_power_float": 3.0,
    "lambda_real": 1.0,
    "lambda_imag": 0.25,
    "explaino_seed": 0,
    "explaino_warp_strength": 0.0,
    "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  },
  "stats": {
    "last_render_ms": 0,
    "last_iters_avg": 0,
    "last_device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected lambda state to load: " << error << "\n";
            return 1;
        }
        if (view.fractal_type != FractalType::lambda_map) {
            std::cerr << "Expected lambda fractal type to round-trip\n";
            return 1;
        }
        if (!NearlyEqual(params.lambda_real, 1.0f, 1.0e-6) || !NearlyEqual(params.lambda_imag, 0.25f, 1.0e-6)) {
            std::cerr << "Expected lambda_real/lambda_imag to round-trip from saved state\n";
            return 1;
        }
    }

    // Explaino-Lambda state round-trip
    {
        const fs::path statePath = tempRoot / "explaino_lambda_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 2,
  "fractal_type": "explaino_lambda",
  "view": {
    "center_x": 0.5,
    "center_y": 0.0,
    "zoom": 4.5,
    "rotation_degrees": 0,
    "center_hp_x": 0.5,
    "center_hp_y": 0.0,
    "log2_zoom": 2.17,
    "explaino_phase": 0,
    "explaino_seed_drift": 0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 1200,
    "epsilon": 0.000001,
    "exposure": 1.4,
    "poly_kind": 2,
    "coloring_mode": "smooth_escape",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "lambda_real": 2.0,
    "lambda_imag": -0.5,
    "explaino_seed": 3.5,
    "explaino_warp_strength": 0.2,
    "explaino_root_count": 4,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  },
  "stats": {
    "last_render_ms": 0,
    "last_iters_avg": 0,
    "last_device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected explaino_lambda state to load: " << error << "\n";
            return 1;
        }
        if (view.fractal_type != FractalType::explaino_lambda) {
            std::cerr << "Expected explaino_lambda fractal type to round-trip\n";
            return 1;
        }
        if (!NearlyEqual(params.lambda_real, 2.0f, 1.0e-6) || !NearlyEqual(params.lambda_imag, -0.5f, 1.0e-6)) {
            std::cerr << "Expected lambda_real/lambda_imag to round-trip from explaino_lambda state\n";
            return 1;
        }
        if (params.coloring_mode != ColoringMode::smooth_escape) {
            std::cerr << "Expected explaino_lambda to use smooth_escape coloring\n";
            return 1;
        }
    }

    // Explaino-Rational-Escape state round-trip
    {
        const fs::path statePath = tempRoot / "explaino_rational_escape_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 2,
  "fractal_type": "explaino_rational_escape",
  "view": {
    "center_x": 0.0,
    "center_y": 0.0,
    "zoom": 1.8,
    "rotation_degrees": 0,
    "center_hp_x": 0.0,
    "center_hp_y": 0.0,
    "log2_zoom": 0.85,
    "explaino_phase": 0,
    "explaino_seed_drift": 0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 1200,
    "epsilon": 0.000001,
    "exposure": 1.2,
    "poly_kind": 2,
    "coloring_mode": "smooth_escape",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 4.2,
    "explaino_warp_strength": 0.3,
    "explaino_root_count": 4,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  },
  "stats": {
    "last_render_ms": 0,
    "last_iters_avg": 0,
    "last_device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected explaino_rational_escape state to load: " << error << "\n";
            return 1;
        }
        if (view.fractal_type != FractalType::explaino_rational_escape) {
            std::cerr << "Expected explaino_rational_escape fractal type to round-trip\n";
            return 1;
        }
        if (!NearlyEqual(params.explaino_seed, 4.0, 1.0e-6) || !NearlyEqual(view.explaino_seed_drift, 0.2f, 1.0e-4)) {
            std::cerr << "Expected explaino_seed/drift to round-trip from explaino_rational_escape state\n";
            return 1;
        }
        if (params.coloring_mode != ColoringMode::smooth_escape) {
            std::cerr << "Expected explaino_rational_escape to use smooth_escape coloring\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "seed_motion_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 2,
  "fractal_type": "explaino_fp",
  "view": {
    "center_x": 0,
    "center_y": 0,
    "zoom": 1,
    "rotation_degrees": 0,
    "center_hp_x": 0,
    "center_hp_y": 0,
    "log2_zoom": 0,
    "explaino_phase": 0.125,
    "explaino_seed_drift": 0.625,
    "explaino_seed_tween": true,
    "auto_increment_seed": true,
    "explaino_seed_rate": 0.6
  },
  "params": {
    "max_iter": 650,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 2,
    "coloring_mode": "joy_basins",
    "nova_alpha": 0.5,
    "phoenix_p_real": -0.5,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 4,
    "explaino_warp_strength": 0.0,
    "explaino_root_count": 4,
    "poly_coeffs": [-1, 0, 0, 1, 1]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  },
  "stats": {
    "last_render_ms": 0,
    "last_iters_avg": 0,
    "last_device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected seed-motion state to load: " << error << "\n";
            return 1;
        }
        if (view.explaino_alive || !view.auto_increment_seed) {
          std::cerr << "Expected auto-increment seed toggle to round-trip while explaino_alive stays at its default\n";
            return 1;
        }
        if (!NearlyEqual(view.explaino_seed_rate, 0.6f, 1.0e-6)) {
          std::cerr << "Expected seed increment rate to round-trip from saved state\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "dual_seed_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 2,
  "fractal_type": "explaino_dual",
  "view": {
    "center_x": 0,
    "center_y": 0,
    "zoom": 1,
    "rotation_degrees": 0,
    "center_hp_x": 0,
    "center_hp_y": 0,
    "log2_zoom": 0,
    "explaino_phase": 0.25,
    "explaino_seed_drift": 0.4,
    "explaino_seed_tween": true,
    "auto_increment_seed": false,
    "explaino_seed_rate": 0.05
  },
  "params": {
    "max_iter": 500,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 2,
    "coloring_mode": "joy_basins",
    "nova_alpha": 0.5,
    "phoenix_p_real": -0.5,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 4,
    "explaino_seed_b": 9.5,
    "explaino_mix": 0.35,
    "explaino_warp_strength": 0.0,
    "explaino_root_count": 4,
    "poly_coeffs": [-1, 0, 0, 1, 1]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  },
  "stats": {
    "last_render_ms": 0,
    "last_iters_avg": 0,
    "last_device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected dual-seed state to load: " << error << "\n";
            return 1;
        }
        if (view.fractal_type != FractalType::explaino_dual) {
            std::cerr << "Expected explaino_dual fractal type to round-trip\n";
            return 1;
        }
        if (!NearlyEqual(params.explaino_seed, 4.0, 1.0e-9) || !NearlyEqual(params.explaino_seed_b, 9.5, 1.0e-9)) {
            std::cerr << "Expected dual-seed endpoints to round-trip from saved state\n";
            return 1;
        }
        if (!NearlyEqual(params.explaino_mix, 0.35f, 1.0e-6)) {
            std::cerr << "Expected dual-seed mix to round-trip from saved state\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "bad_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 1,
  "fractal_type": "mystery_fractal",
  "view": {
    "center_x": 0,
    "center_y": 0,
    "zoom": 1,
    "rotation_degrees": 0,
    "center_hp_x": 0,
    "center_hp_y": 0,
    "log2_zoom": 0,
    "explaino_phase": 0,
    "explaino_seed_drift": 0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500,
    "epsilon": 0.000001,
    "exposure": 1,
    "poly_kind": 0,
    "explaino_seed": 0,
    "explaino_warp_strength": 0,
    "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected unknown fractal type to fail\n";
            return 1;
        }
        if (error.find("Unknown fractal_type") == std::string::npos) {
            std::cerr << "Unexpected error text: " << error << "\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "bad_zero_max_iter_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "newton",
  "view": {
    "center_x": 0,
    "center_y": 0,
    "zoom": 1,
    "rotation_degrees": 0,
    "center_hp_x": 0,
    "center_hp_y": 0,
    "log2_zoom": 0,
    "explaino_phase": 0,
    "explaino_seed_drift": 0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 0,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "root_basin",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "multibrot_power_float": 3.0,
    "lambda_real": 0.0,
    "lambda_imag": 0.0,
    "explaino_seed": 0,
    "explaino_warp_strength": 0,
    "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected zero max_iter to fail\n";
            return 1;
        }
        if (error.find("max_iter") == std::string::npos) {
            std::cerr << "Unexpected zero-max_iter error text: " << error << "\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "bad_zero_width_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "newton",
  "view": {
    "center_x": 0,
    "center_y": 0,
    "zoom": 1,
    "rotation_degrees": 0,
    "center_hp_x": 0,
    "center_hp_y": 0,
    "log2_zoom": 0,
    "explaino_phase": 0,
    "explaino_seed_drift": 0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "root_basin",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "multibrot_power_float": 3.0,
    "lambda_real": 0.0,
    "lambda_imag": 0.0,
    "explaino_seed": 0,
    "explaino_warp_strength": 0,
    "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 0,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected zero width to fail\n";
            return 1;
        }
        if (error.find("width") == std::string::npos) {
            std::cerr << "Unexpected zero-width error text: " << error << "\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "bad_negative_height_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "newton",
  "view": {
    "center_x": 0,
    "center_y": 0,
    "zoom": 1,
    "rotation_degrees": 0,
    "center_hp_x": 0,
    "center_hp_y": 0,
    "log2_zoom": 0,
    "explaino_phase": 0,
    "explaino_seed_drift": 0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "root_basin",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "multibrot_power_float": 3.0,
    "lambda_real": 0.0,
    "lambda_imag": 0.0,
    "explaino_seed": 0,
    "explaino_warp_strength": 0,
    "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": -1,
    "block_size": 256,
    "device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected negative height to fail\n";
            return 1;
        }
        if (error.find("height") == std::string::npos) {
            std::cerr << "Unexpected negative-height error text: " << error << "\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "bad_coloring_mode_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 2,
  "fractal_type": "phoenix",
  "view": {
    "center_x": 0,
    "center_y": 0,
    "zoom": 1,
    "rotation_degrees": 0,
    "center_hp_x": 0,
    "center_hp_y": 0,
    "log2_zoom": 0,
    "explaino_phase": 0,
    "explaino_seed_drift": 0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 1200,
    "epsilon": 0.000001,
    "exposure": 1.6,
    "poly_kind": 2,
    "coloring_mode": "joy_basins",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.5667,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 0,
    "explaino_warp_strength": 0,
    "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected incompatible coloring_mode for phoenix to fail\n";
            return 1;
        }
        if (error.find("not allowed for fractal_type phoenix") == std::string::npos) {
            std::cerr << "Unexpected invalid-coloring error text: " << error << "\n";
            return 1;
        }
    }

    {
        const fs::path statePath = tempRoot / "bad_explaino_poly_kind_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 2,
  "fractal_type": "explaino",
  "view": {
    "center_x": 0,
    "center_y": 0,
    "zoom": 1,
    "rotation_degrees": 0,
    "center_hp_x": 0,
    "center_hp_y": 0,
    "log2_zoom": 0,
    "explaino_phase": 0,
    "explaino_seed_drift": 0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500,
    "epsilon": 0.000001,
    "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "joy_basins",
    "nova_alpha": 0.5,
    "phoenix_p_real": -0.5,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 1,
    "explaino_warp_strength": 0,
    "explaino_root_count": 4,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        std::string error;
        if (LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected explaino state with non-custom poly_kind to fail\n";
            return 1;
        }
        if (error.find("poly_kind must be custom for fractal_type explaino") == std::string::npos) {
            std::cerr << "Unexpected explaino poly_kind error text: " << error << "\n";
            return 1;
        }
    }

      {
        const fs::path statePath = tempRoot / "bad_transcendental_func_type_state.json";
        WriteMinimalStateWithExtraParams(statePath, "    \"transcendental_func\": 123");

        std::string error;
        if (!ExpectLoadDiagnosticsStateFailure(statePath, "transcendental_func", &error)) {
          std::cerr << "Expected invalid transcendental_func type to fail: " << error << "\n";
          return 1;
        }
      }

      {
        const fs::path statePath = tempRoot / "bad_transcendental_func_value_state.json";
        WriteMinimalStateWithExtraParams(statePath, "    \"transcendental_func\": \"f_unknown\"");

        std::string error;
        if (!ExpectLoadDiagnosticsStateFailure(statePath, "transcendental_func", &error)) {
          std::cerr << "Expected unknown transcendental_func to fail: " << error << "\n";
          return 1;
        }
      }

      {
        const fs::path statePath = tempRoot / "bad_momentum_beta_type_state.json";
        WriteMinimalStateWithExtraParams(statePath, "    \"momentum_beta\": \"fast\"");

        std::string error;
        if (!ExpectLoadDiagnosticsStateFailure(statePath, "momentum_beta", &error)) {
          std::cerr << "Expected invalid momentum_beta type to fail: " << error << "\n";
          return 1;
        }
      }

      {
        const fs::path statePath = tempRoot / "bad_mcmullen_preset_type_state.json";
        WriteMinimalStateWithExtraParams(statePath, "    \"mcmullen_preset\": 7");

        std::string error;
        if (!ExpectLoadDiagnosticsStateFailure(statePath, "mcmullen_preset", &error)) {
          std::cerr << "Expected invalid mcmullen_preset type to fail: " << error << "\n";
          return 1;
        }
      }

      {
        const fs::path statePath = tempRoot / "bad_mcmullen_preset_value_state.json";
        WriteMinimalStateWithExtraParams(statePath, "    \"mcmullen_preset\": \"z9_z9\"");

        std::string error;
        if (!ExpectLoadDiagnosticsStateFailure(statePath, "mcmullen_preset", &error)) {
          std::cerr << "Expected unknown mcmullen_preset to fail: " << error << "\n";
          return 1;
        }
      }

    {
        const fs::path statePath = tempRoot / "legacy_state_version1_phoenix.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 1,
  "fractal_type": "phoenix",
  "view": {
    "center_x": 0,
    "center_y": 0,
    "zoom": 1,
    "rotation_degrees": 0,
    "center_hp_x": 0,
    "center_hp_y": 0,
    "log2_zoom": 0,
    "explaino_phase": 0,
    "explaino_seed_drift": 0,
    "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 1200,
    "epsilon": 0.000001,
    "exposure": 1.6,
    "poly_kind": 2,
    "explaino_seed": 0,
    "explaino_warp_strength": 0,
    "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1024,
    "height": 768,
    "block_size": 256,
    "device_id": 0
  }
})";
        file.close();

        ViewState view{};
        KernelParams params{};
        params.coloring_mode = ColoringMode::root_basin;
        RenderSettings render{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &view, &params, &render, &error)) {
            std::cerr << "Expected legacy phoenix state to load: " << error << "\n";
            return 1;
        }
        if (params.coloring_mode != ColoringMode::smooth_escape) {
            std::cerr << "Expected legacy phoenix state to default to smooth_escape\n";
            return 1;
        }
    }

      {
        const fs::path findingDir = tempRoot / "resolve_finding_json";
        fs::create_directories(findingDir);

        const fs::path statePath = findingDir / "state.json";
        std::ofstream stateFile(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        stateFile << "{}";
        stateFile.close();

        const fs::path findingPath = findingDir / "finding.json";
        std::ofstream findingFile(findingPath, std::ios::out | std::ios::binary | std::ios::trunc);
        findingFile << R"({
      "finding_id": "resolve_me",
      "state_file": "state.json"
    })";
        findingFile.close();

        std::string resolvedStatePath;
        std::string error;
        if (!ResolveFindingStateJsonPath(findingPath.string(), &resolvedStatePath, &error)) {
          std::cerr << "ResolveFindingStateJsonPath failed: " << error << "\n";
          return 1;
        }
        if (resolvedStatePath != statePath.string()) {
          std::cerr << "Resolved state path mismatch\n";
          return 1;
        }
      }

      {
        const fs::path findingDir = tempRoot / "missing_finding_state";
        fs::create_directories(findingDir);

        const fs::path findingPath = findingDir / "finding.json";
        std::ofstream findingFile(findingPath, std::ios::out | std::ios::binary | std::ios::trunc);
        findingFile << R"({
      "finding_id": "missing_state",
      "state_file": "missing.json"
    })";
        findingFile.close();

        std::string resolvedStatePath;
        std::string error;
        if (ResolveFindingStateJsonPath(findingPath.string(), &resolvedStatePath, &error)) {
          std::cerr << "Expected missing state file reference to fail\n";
          return 1;
        }
        if (error.find("Finding metadata points to missing state file") == std::string::npos) {
          std::cerr << "Unexpected missing-state error text: " << error << "\n";
          return 1;
        }
      }

      {
        const fs::path safeRoot = tempRoot / "finding_state_traversal";
        const fs::path findingDir = safeRoot / "finding_bundle";
        const fs::path outsideDir = safeRoot / "outside";
        fs::create_directories(findingDir);
        fs::create_directories(outsideDir);

        const fs::path outsideStatePath = outsideDir / "state.json";
        std::ofstream stateFile(outsideStatePath, std::ios::out | std::ios::binary | std::ios::trunc);
        stateFile << "{}";
        stateFile.close();

        const fs::path findingPath = findingDir / "finding.json";
        std::ofstream findingFile(findingPath, std::ios::out | std::ios::binary | std::ios::trunc);
        findingFile << R"({
      "finding_id": "escape_attempt",
      "state_file": "../outside/state.json"
    })";
        findingFile.close();

        std::string resolvedStatePath;
        std::string error;
        if (ResolveFindingStateJsonPath(findingPath.string(), &resolvedStatePath, &error)) {
          std::cerr << "Expected finding state path traversal to fail\n";
          return 1;
        }
        if (error.find("state_file") == std::string::npos) {
          std::cerr << "Unexpected traversal error text: " << error << "\n";
          return 1;
        }
      }

      {
        const fs::path findingDir = tempRoot / "finding_state_directory_ref";
        fs::create_directories(findingDir);

        const fs::path findingPath = findingDir / "finding.json";
        std::ofstream findingFile(findingPath, std::ios::out | std::ios::binary | std::ios::trunc);
        findingFile << R"({
      "finding_id": "directory_ref",
      "state_file": "."
    })";
        findingFile.close();

        std::string resolvedStatePath;
        std::string error;
        if (ResolveFindingStateJsonPath(findingPath.string(), &resolvedStatePath, &error)) {
          std::cerr << "Expected directory state_file reference to fail\n";
          return 1;
        }
        if (error.find("state_file") == std::string::npos) {
          std::cerr << "Unexpected directory-ref error text: " << error << "\n";
          return 1;
        }
      }

    // V3 round-trip: color grading fields
    {
        const fs::path statePath = tempRoot / "v3_grading.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 3,
  "fractal_type": "newton",
  "view": {
    "center_x": 0.0, "center_y": 0.0, "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0, "center_hp_y": 0.0, "log2_zoom": 0.0,
    "explaino_phase": 0.0, "explaino_seed_drift": 0.0, "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500, "epsilon": 1e-06, "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "root_basin",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0, "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 0.0, "explaino_warp_strength": 0.0, "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0],
    "color_saturation": 1.35,
    "color_contrast": 1.22,
    "color_tint_r": 0.9,
    "color_tint_g": 1.1,
    "color_tint_b": 0.85
  },
  "render": { "width": 512, "height": 384, "block_size": 256, "device_id": 0 }
})";
        file.close();

        ViewState v{};
        KernelParams p{};
        RenderSettings r{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &v, &p, &r, &error)) {
            std::cerr << "V3 grading round-trip load failed: " << error << "\n";
            return 1;
        }
        if (!NearlyEqual(p.color_saturation, 1.35, 0.001)) { std::cerr << "color_saturation mismatch\n"; return 1; }
        if (!NearlyEqual(p.color_contrast, 1.22, 0.001)) { std::cerr << "color_contrast mismatch\n"; return 1; }
        if (!NearlyEqual(p.color_tint_r, 0.9, 0.001)) { std::cerr << "color_tint_r mismatch\n"; return 1; }
        if (!NearlyEqual(p.color_tint_g, 1.1, 0.001)) { std::cerr << "color_tint_g mismatch\n"; return 1; }
        if (!NearlyEqual(p.color_tint_b, 0.85, 0.001)) { std::cerr << "color_tint_b mismatch\n"; return 1; }
    }

    // V2 backward compat: grading fields should get defaults
    {
        const fs::path statePath = tempRoot / "v2_grading_defaults.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
  "state_version": 2,
  "fractal_type": "newton",
  "view": {
    "center_x": 0.0, "center_y": 0.0, "zoom": 1.0,
    "rotation_degrees": 0.0,
    "center_hp_x": 0.0, "center_hp_y": 0.0, "log2_zoom": 0.0,
    "explaino_phase": 0.0, "explaino_seed_drift": 0.0, "explaino_seed_tween": true
  },
  "params": {
    "max_iter": 500, "epsilon": 1e-06, "exposure": 1.0,
    "poly_kind": 0,
    "coloring_mode": "root_basin",
    "nova_alpha": 0.5,
    "phoenix_p_real": 0.0, "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 0.0, "explaino_warp_strength": 0.0, "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": { "width": 512, "height": 384, "block_size": 256, "device_id": 0 }
})";
        file.close();

        ViewState v{};
        KernelParams p{};
        RenderSettings r{};
        std::string error;
        if (!LoadDiagnosticsStateFile(statePath.string(), &v, &p, &r, &error)) {
            std::cerr << "V2 backward compat grading load failed: " << error << "\n";
            return 1;
        }
        if (r.interaction_debounce_ms != 200 || !NearlyEqual(r.preview_target_fps, 30.0f, 0.01) || !NearlyEqual(r.preview_min_scale, 0.5f, 0.01)) {
          std::cerr << "v2 render pacing controls should fall back to defaults when missing\n";
          return 1;
        }
        // Should get struct defaults when fields are missing
        if (!NearlyEqual(p.color_saturation, 1.15, 0.01)) { std::cerr << "v2 color_saturation should be default 1.15\n"; return 1; }
        if (!NearlyEqual(p.color_contrast, 1.10, 0.01)) { std::cerr << "v2 color_contrast should be default 1.10\n"; return 1; }
        if (!NearlyEqual(p.color_tint_r, 1.0, 0.01)) { std::cerr << "v2 color_tint_r should be default 1.0\n"; return 1; }
    }

    std::cout << "test_diagnostics_state_io: all passed\n";
    return 0;
}