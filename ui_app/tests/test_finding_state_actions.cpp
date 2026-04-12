#include "../src/finding_state_actions.h"

#include "../src/diagnostics_state_io.h"
#include "../src/fractal_derived_fields.h"
#include "../src/view_hp_sync.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {

bool NearlyEqual(double a, double b, double eps = 1.0e-9) {
    return std::fabs(a - b) <= eps;
}

void WriteTextFile(const std::filesystem::path& path, const char* text) {
    std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
    file << text;
}

bool PolyCoeffsMatch(const KernelParams& lhs, const KernelParams& rhs, double eps = 1.0e-6) {
    for (int index = 0; index < 5; ++index) {
        if (!NearlyEqual(lhs.poly_coeffs[index], rhs.poly_coeffs[index], eps)) return false;
    }
    return true;
}

} // namespace

int main() {
    namespace fs = std::filesystem;

    const fs::path tempRoot = fs::temp_directory_path() / "cuda_newton_fractal_clone_finding_state_action_tests";
    fs::remove_all(tempRoot);
    fs::create_directories(tempRoot);

    {
        const fs::path findingDir = tempRoot / "explaino_finding";
        fs::create_directories(findingDir);

        const fs::path statePath = findingDir / "state.json";
        WriteTextFile(statePath, R"({
      "state_version": 3,
  "fractal_type": "explaino_fp",
  "view": {
    "center_x": 999.0,
    "center_y": -999.0,
    "zoom": 777.0,
    "rotation_degrees": 15.0,
    "center_hp_x": 0.125,
    "center_hp_y": -0.375,
    "log2_zoom": 3.5,
    "explaino_phase": 0.75,
    "explaino_seed_drift": 0.25,
        "explaino_seed_tween": true,
        "explaino_phase_strength": -1.5
  },
  "params": {
    "max_iter": 650,
    "epsilon": 0.000001,
    "exposure": 1.25,
    "poly_kind": 2,
    "coloring_mode": "joy_basins",
    "nova_alpha": 0.50,
    "phoenix_p_real": -0.50,
    "phoenix_p_imag": 0.0,
    "multibrot_power": 3,
    "explaino_seed": 4,
    "explaino_warp_strength": 0.40,
        "explaino_root_spread": 2.25,
    "explaino_root_count": 99,
    "poly_coeffs": [11, 22, 33, 44, 55]
  },
  "render": {
    "width": 1600,
    "height": 900,
    "block_size": 512,
    "device_id": 1
  }
})");

        const fs::path findingPath = findingDir / "finding.json";
        WriteTextFile(findingPath, R"({
  "finding_id": "explaino_focus",
  "state_file": "state.json"
})");

        ViewState view{};
        view.center = {42.0f, -42.0f};
        view.zoom = 99.0f;
        view.center_hp_x = 42.0;
        view.center_hp_y = -42.0;
        view.log2_zoom = 0.0;

        KernelParams params{};
        params.max_iter = 77;
        params.poly_coeffs[0] = -7.0f;
        params.poly_coeffs[1] = -6.0f;
        params.poly_coeffs[2] = -5.0f;
        params.poly_coeffs[3] = -4.0f;
        params.poly_coeffs[4] = -3.0f;

        RenderSettings render{};
        render.resolution = {320, 200};
        render.block_size = 128;
        render.device_id = 7;

        std::string resolvedStatePath;
        std::string error;
        if (!LoadFindingSelectionIntoRuntime(findingPath.string(), &view, &params, &render, &resolvedStatePath, &error)) {
            std::cerr << "LoadFindingSelectionIntoRuntime failed: " << error << "\n";
            return 1;
        }
        if (resolvedStatePath != statePath.string()) {
            std::cerr << "Resolved state path mismatch\n";
            return 1;
        }

        ViewState expectedView{};
        KernelParams expectedParams{};
        RenderSettings expectedRender{};
        std::string expectedError;
        if (!LoadDiagnosticsStateFile(statePath.string(), &expectedView, &expectedParams, &expectedRender, &expectedError)) {
            std::cerr << "LoadDiagnosticsStateFile failed while building expectation: " << expectedError << "\n";
            return 1;
        }
        UpdateExplainoPolynomial(expectedView, expectedParams, nullptr);
        SyncViewUiFromHp(expectedView);

        if (view.fractal_type != FractalType::explaino_fp) {
            std::cerr << "Expected explaino_fp fractal type after load\n";
            return 1;
        }
        if (!NearlyEqual(view.center.x, expectedView.center.x, 1.0e-6) ||
            !NearlyEqual(view.center.y, expectedView.center.y, 1.0e-6)) {
            std::cerr << "Expected UI center to resync from high-precision state\n";
            return 1;
        }
        if (!NearlyEqual(view.zoom, expectedView.zoom, 1.0e-6)) {
            std::cerr << "Expected UI zoom to resync from high-precision log2_zoom\n";
            return 1;
        }
        if (params.explaino_root_count != 4) {
            std::cerr << "Expected explaino load to recompute four derived roots\n";
            return 1;
        }
        if (!NearlyEqual(params.explaino_root_spread, expectedParams.explaino_root_spread, 1.0e-6) ||
            !NearlyEqual(view.explaino_phase_strength, expectedView.explaino_phase_strength, 1.0e-6)) {
            std::cerr << "Expected explaino root spread and phase strength to survive finding state load\n";
            return 1;
        }
        if (!PolyCoeffsMatch(params, expectedParams)) {
            std::cerr << "Expected explaino polynomial coefficients to be recomputed from seed state\n";
            return 1;
        }
        if (render.resolution.x != 1600 || render.resolution.y != 900 || render.block_size != 512 || render.device_id != 1) {
            std::cerr << "Expected render settings from saved state\n";
            return 1;
        }
    }

    {
        const fs::path findingDir = tempRoot / "explaino_finding_with_orientation";
        fs::create_directories(findingDir);

        const fs::path statePath = findingDir / "state.json";
        WriteTextFile(statePath, R"({
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
        "explaino_phase": 0.0,
        "explaino_seed_drift": 0.0,
        "explaino_seed_tween": true
    },
    "params": {
        "max_iter": 500,
        "epsilon": 0.000001,
        "exposure": 1.0,
        "poly_kind": 2,
        "coloring_mode": "joy_basins",
        "nova_alpha": 0.5,
        "phoenix_p_real": 0.0,
        "phoenix_p_imag": 0.0,
        "multibrot_power": 3,
        "explaino_seed": 6.0,
        "explaino_seed_b": 2.0,
        "explaino_warp_strength": 0.0,
        "explaino_root_count": 4,
        "poly_coeffs": [1, 0, 0, 1, 1]
    },
    "render": {
        "width": 1024,
        "height": 768,
        "block_size": 256,
        "device_id": 0
    },
    "sidecar_orientation": {
        "import_signature": "9007199254740993",
        "pack_projection_hash": "18446744073709551614",
        "field_embedding_stats": 3.5,
        "slime_energy_delta": 1.25,
        "busy_beaver_metrics": 0.75,
        "decode_stability": 0.5,
        "diff_magnitude": 2.0
    }
})");

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        SidecarOrientationVector orientation{};
        bool hasOrientation = false;
        std::string resolvedStatePath;
        std::string error;
        if (!LoadFindingSelectionIntoRuntime(
            statePath.string(),
            &view,
            &params,
            &render,
            &orientation,
            &hasOrientation,
            &resolvedStatePath,
            &error)) {
            std::cerr << "Expected state load with persisted sidecar orientation to succeed: " << error << "\n";
            return 1;
        }
        if (!hasOrientation) {
            std::cerr << "Expected state load to report persisted sidecar orientation when present\n";
            return 1;
        }
        if (resolvedStatePath != statePath.string()) {
            std::cerr << "Resolved state path mismatch for direct state load\n";
            return 1;
        }
        if (orientation.import_signature != 9007199254740993ull ||
            orientation.pack_projection_hash != 18446744073709551614ull ||
            !NearlyEqual(orientation.field_embedding_stats, 3.5) ||
            !NearlyEqual(orientation.slime_energy_delta, 1.25) ||
            !NearlyEqual(orientation.busy_beaver_metrics, 0.75) ||
            !NearlyEqual(orientation.decode_stability, 0.5) ||
            !NearlyEqual(orientation.diff_magnitude, 2.0)) {
            std::cerr << "Expected persisted sidecar orientation to round-trip through finding-state load\n";
            return 1;
        }
        }

    {
        ViewState view{};
        view.center = {3.0f, -4.0f};
        view.zoom = 2.0f;
        view.center_hp_x = 3.0;
        view.center_hp_y = -4.0;
        view.log2_zoom = 1.0;

        KernelParams params{};
        params.max_iter = 777;
        params.poly_coeffs[0] = 1.0f;
        params.poly_coeffs[1] = 2.0f;
        params.poly_coeffs[2] = 3.0f;
        params.poly_coeffs[3] = 4.0f;
        params.poly_coeffs[4] = 5.0f;

        RenderSettings render{};
        render.resolution = {640, 480};
        render.block_size = 128;
        render.device_id = 6;

        std::string resolvedStatePath = "sentinel";
        std::string error;
        if (LoadFindingSelectionIntoRuntime((tempRoot / "missing_selection.json").string(), &view, &params, &render, &resolvedStatePath, &error)) {
            std::cerr << "Expected missing selection path to fail\n";
            return 1;
        }
        if (error.find("Selected path does not exist") == std::string::npos) {
            std::cerr << "Unexpected missing-selection error text: " << error << "\n";
            return 1;
        }
        if (!NearlyEqual(view.center.x, 3.0f, 1.0e-6) || !NearlyEqual(view.center.y, -4.0f, 1.0e-6) || !NearlyEqual(view.zoom, 2.0f, 1.0e-6)) {
            std::cerr << "View state mutated on failed load\n";
            return 1;
        }
        if (params.max_iter != 777 || params.poly_coeffs[0] != 1.0f || params.poly_coeffs[4] != 5.0f) {
            std::cerr << "Kernel params mutated on failed load\n";
            return 1;
        }
        if (render.resolution.x != 640 || render.resolution.y != 480 || render.block_size != 128 || render.device_id != 6) {
            std::cerr << "Render settings mutated on failed load\n";
            return 1;
        }
    }

    {
        const fs::path findingDir = tempRoot / "bad_finding_metadata";
        fs::create_directories(findingDir);

        const fs::path findingPath = findingDir / "finding.json";
        WriteTextFile(findingPath, R"({
  "finding_id": "missing_state_file"
})");

        ViewState view{};
        view.center = {8.0f, 9.0f};
        view.zoom = 10.0f;
        KernelParams params{};
        params.max_iter = 111;
        RenderSettings render{};
        render.resolution = {800, 600};

        std::string resolvedStatePath = "sentinel";
        std::string error;
        if (LoadFindingSelectionIntoRuntime(findingPath.string(), &view, &params, &render, &resolvedStatePath, &error)) {
            std::cerr << "Expected bad finding metadata to fail\n";
            return 1;
        }
        if (error.find("Missing or invalid string field: state_file") == std::string::npos) {
            std::cerr << "Unexpected bad-metadata error text: " << error << "\n";
            return 1;
        }
        if (!NearlyEqual(view.center.x, 8.0f, 1.0e-6) || !NearlyEqual(view.center.y, 9.0f, 1.0e-6) || !NearlyEqual(view.zoom, 10.0f, 1.0e-6)) {
            std::cerr << "View state mutated on bad finding metadata\n";
            return 1;
        }
        if (params.max_iter != 111 || render.resolution.x != 800 || render.resolution.y != 600) {
            std::cerr << "Runtime state mutated on bad finding metadata\n";
            return 1;
        }
    }

    {
        const fs::path findingDir = tempRoot / "invalid_finding_state_payload";
        fs::create_directories(findingDir);

        const fs::path statePath = findingDir / "state.json";
        WriteTextFile(statePath, R"({
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
})");

        const fs::path findingPath = findingDir / "finding.json";
        WriteTextFile(findingPath, R"({
  "finding_id": "bad_state_payload",
  "state_file": "state.json"
})");

        ViewState view{};
        view.center = {12.0f, -13.0f};
        view.zoom = 14.0f;
        KernelParams params{};
        params.max_iter = 222;
        RenderSettings render{};
        render.resolution = {900, 700};

        std::string resolvedStatePath = "sentinel";
        std::string error;
        if (LoadFindingSelectionIntoRuntime(findingPath.string(), &view, &params, &render, &resolvedStatePath, &error)) {
            std::cerr << "Expected invalid finding state payload to fail\n";
            return 1;
        }
        if (error.find("poly_kind must be custom for fractal_type explaino") == std::string::npos) {
            std::cerr << "Unexpected invalid-state propagation text: " << error << "\n";
            return 1;
        }
        if (!NearlyEqual(view.center.x, 12.0f, 1.0e-6) || !NearlyEqual(view.center.y, -13.0f, 1.0e-6) || !NearlyEqual(view.zoom, 14.0f, 1.0e-6)) {
            std::cerr << "View state mutated on invalid finding state payload\n";
            return 1;
        }
        if (params.max_iter != 222 || render.resolution.x != 900 || render.resolution.y != 700) {
            std::cerr << "Runtime state mutated on invalid finding state payload\n";
            return 1;
        }
    }

    {
        const fs::path findingDir = tempRoot / "invalid_sidecar_orientation_payload";
        fs::create_directories(findingDir);

        const fs::path statePath = findingDir / "state.json";
        WriteTextFile(statePath, R"({
    "state_version": 3,
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
        "poly_kind": 2,
        "coloring_mode": "joy_basins",
        "nova_alpha": 0.5,
        "phoenix_p_real": 0.0,
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
    },
    "sidecar_orientation": {
        "import_signature": "not-a-u64",
        "pack_projection_hash": "17",
        "field_embedding_stats": 3.5,
        "slime_energy_delta": 1.25,
        "busy_beaver_metrics": 0.75,
        "decode_stability": 0.5,
        "diff_magnitude": 2.0
    }
})");

        ViewState view{};
        view.center = {21.0f, -22.0f};
        view.zoom = 23.0f;
        KernelParams params{};
        params.max_iter = 333;
        RenderSettings render{};
        render.resolution = {901, 701};

        std::string resolvedStatePath = "sentinel";
        std::string error;
        if (LoadFindingSelectionIntoRuntime(statePath.string(), &view, &params, &render, &resolvedStatePath, &error)) {
            std::cerr << "Expected invalid persisted sidecar orientation payload to fail atomically\n";
            return 1;
        }
        if (error.find("Invalid sidecar_orientation field: import_signature") == std::string::npos) {
            std::cerr << "Unexpected invalid-sidecar error text: " << error << "\n";
            return 1;
        }
        if (!NearlyEqual(view.center.x, 21.0f, 1.0e-6) || !NearlyEqual(view.center.y, -22.0f, 1.0e-6) || !NearlyEqual(view.zoom, 23.0f, 1.0e-6)) {
            std::cerr << "View state mutated on invalid persisted sidecar orientation payload\n";
            return 1;
        }
        if (params.max_iter != 333 || render.resolution.x != 901 || render.resolution.y != 701) {
            std::cerr << "Runtime state mutated on invalid persisted sidecar orientation payload\n";
            return 1;
        }
    }

    std::cout << "test_finding_state_actions: all passed\n";
    return 0;
}