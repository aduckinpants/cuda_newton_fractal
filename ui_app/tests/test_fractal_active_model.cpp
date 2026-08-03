#include "fractal_active_model.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

namespace {

int g_failures = 0;

void Check(bool condition, const char* name) {
    if (!condition) {
        std::cerr << "FAIL: " << name << "\n";
        ++g_failures;
    }
}

ActiveFractalModelReceiptContext Context() {
    ActiveFractalModelReceiptContext context;
    context.state_json_sha256 = std::string(64, 'a');
    context.runtime_executable_sha256 = std::string(64, 'b');
    return context;
}

} // namespace

int main() {
    Check(ActiveFractalModelProviderCount() == 1, "one initial provider slot");
    Check(std::string(ActiveFractalModelProviderIdAt(0)) == "polynomial_over_power_escape.v1",
          "provider id exact");
    Check(ActiveFractalModelProviderIdAt(1) == nullptr, "provider index bounds");

    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    view.fractal_type = FractalType::explaino_rational_escape;
    view.log2_zoom = 24.0;
    params.poly_coeffs[0] = -1.0f;
    params.poly_coeffs[1] = 0.25f;
    params.poly_coeffs[2] = 0.0f;
    params.poly_coeffs[3] = 1.0f;
    params.poly_coeffs[4] = 0.5f;
    params.explaino_rational_escape_denominator_power = 9;
    params.explaino_warp_strength = 0.0f;
    params.max_iter = 777;
    render.sample_tier = SampleTier::tier_auto;

    std::string json;
    std::string error;
    Check(BuildActiveFractalModelReceiptJson(view, params, render, Context(), &json, &error),
          "available receipt builds");
    const std::string repeated = json;
    Check(BuildActiveFractalModelReceiptJson(view, params, render, Context(), &json, &error),
          "available receipt repeats");
    Check(json == repeated, "receipt deterministic");
    Check(json.find("\"status\": \"available\"") != std::string::npos, "available status");
    Check(json.find("\"provider_id\": \"polynomial_over_power_escape.v1\"") != std::string::npos,
          "provider serialized");
    Check(json.find("\"denominator_power\": 6") != std::string::npos, "denominator clamp serialized");
    Check(json.find("\"max_iterations\": 777") != std::string::npos, "iteration cap serialized");
    Check(json.find("\"resolved_backend\": \"float64\"") != std::string::npos,
          "resolved numeric backend serialized");
    Check(json.find("\"evaluation_surface\": \"fractal.sample\"") != std::string::npos,
          "canonical evaluation surface serialized");
    Check(json.find(std::string(64, 'a')) != std::string::npos, "state hash serialized");
    Check(json.find(std::string(64, 'b')) != std::string::npos, "runtime hash serialized");
    Check(json.find("epsilon") == std::string::npos, "unused epsilon excluded");
    Check(json.find("branch") == std::string::npos && json.find("timestamp") == std::string::npos,
          "volatile provenance absent");

    params.explaino_warp_strength = 0.01f;
    Check(BuildActiveFractalModelReceiptJson(view, params, render, Context(), &json, &error),
          "nonzero warp receipt builds");
    Check(json.find("\"status\": \"unavailable\"") != std::string::npos,
          "nonzero warp unavailable");
    Check(json.find("nonzero_warp_unsupported") != std::string::npos,
          "nonzero warp reason exact");
    Check(json.find("\"model\": null") != std::string::npos, "unavailable model null");

    view.fractal_type = FractalType::mandelbrot;
    params.explaino_warp_strength = 0.0f;
    Check(BuildActiveFractalModelReceiptJson(view, params, render, Context(), &json, &error),
          "unsupported family receipt builds");
    Check(json.find("unsupported_fractal_type") != std::string::npos,
          "unsupported family explicit");

    ActiveFractalModelReceiptContext invalid = Context();
    invalid.state_json_sha256 = "short";
    Check(!BuildActiveFractalModelReceiptJson(view, params, render, invalid, &json, &error),
          "invalid binding rejected");

    view.fractal_type = FractalType::explaino_rational_escape;
    Check(BuildActiveFractalModelReceiptJson(view, params, render, Context(), &json, &error),
          "file fixture receipt builds");
    const fs::path root = fs::temp_directory_path() / "cuda_fractal_active_model_e2";
    std::error_code ignored;
    fs::remove_all(root, ignored);
    fs::create_directories(root);
    const fs::path target = root / "receipt.json";
    Check(WriteActiveFractalModelReceiptJsonFile(target.string(), json, &error), "file write succeeds");
    std::ifstream in(target, std::ios::binary);
    const std::string bytes((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    Check(bytes == json, "file bytes equal serialized bytes");
    Check(!fs::exists(target.string() + ".tmp"), "owned temp cleaned after success");
    Check(!WriteActiveFractalModelReceiptJsonFile((root / "missing" / "receipt.json").string(), json, &error),
          "missing parent rejected");
    Check(!fs::exists(root / "missing"), "writer does not create parent");
    fs::remove_all(root, ignored);

    if (g_failures == 0) std::cout << "test_fractal_active_model: PASS\n";
    return g_failures == 0 ? 0 : 1;
}
