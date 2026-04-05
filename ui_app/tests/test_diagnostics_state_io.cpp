#include "../src/diagnostics_state_io.h"

#include <filesystem>
#include <fstream>
#include <iostream>

static bool NearlyEqual(double a, double b, double eps = 1.0e-9) {
    return std::fabs(a - b) <= eps;
}

int main() {
    namespace fs = std::filesystem;

    const fs::path tempRoot = fs::temp_directory_path() / "cuda_newton_fractal_clone_state_io_tests";
    fs::create_directories(tempRoot);

    {
        const fs::path statePath = tempRoot / "loaded_state.json";
        std::ofstream file(statePath, std::ios::out | std::ios::binary | std::ios::trunc);
        file << R"({
        "state_version": 2,
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
    "explaino_seed_tween": false
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
    "explaino_root_count": 0,
    "poly_coeffs": [-1, 0, 0, 1, 0]
  },
  "render": {
    "width": 1440,
    "height": 900,
    "block_size": 512,
    "device_id": 1
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
        if (view.explaino_seed_tween != false) {
            std::cerr << "view explaino_seed_tween mismatch\n";
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
        if (!NearlyEqual(view.explaino_seed_rate, 0.35f, 1.0e-6)) {
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
        if (render.resolution.x != 1440 || render.resolution.y != 900 || render.block_size != 512 || render.device_id != 1) {
            std::cerr << "render mismatch\n";
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
        // Should get struct defaults when fields are missing
        if (!NearlyEqual(p.color_saturation, 1.15, 0.01)) { std::cerr << "v2 color_saturation should be default 1.15\n"; return 1; }
        if (!NearlyEqual(p.color_contrast, 1.10, 0.01)) { std::cerr << "v2 color_contrast should be default 1.10\n"; return 1; }
        if (!NearlyEqual(p.color_tint_r, 1.0, 0.01)) { std::cerr << "v2 color_tint_r should be default 1.0\n"; return 1; }
    }

    std::cout << "test_diagnostics_state_io: all passed\n";
    return 0;
}