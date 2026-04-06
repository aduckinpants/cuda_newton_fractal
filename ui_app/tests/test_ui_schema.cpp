#include "../src/ui_schema.h"
#include "../src/json_min.h"
#include "../src/schema_binding.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace {

std::filesystem::path FindRepoRoot() {
    std::filesystem::path cur = std::filesystem::current_path();
    for (int index = 0; index < 8; ++index) {
        if (std::filesystem::exists(cur / "ui" / "fractal_binding_surface_v1.ui_schema.json") &&
            std::filesystem::exists(cur / "ui_app" / "src" / "ui_schema.cpp")) {
            return cur;
        }
        if (!cur.has_parent_path() || cur.parent_path() == cur) break;
        cur = cur.parent_path();
    }
    return {};
}

bool ReadTextFile(const std::filesystem::path& path, std::string* outText) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) return false;
    std::ostringstream ss;
    ss << file.rdbuf();
    *outText = ss.str();
    return true;
}

bool LoadAndValidateSchemaFile(const std::filesystem::path& path) {
    std::string text;
    if (!ReadTextFile(path, &text)) {
        std::cerr << "Failed to read schema file: " << path.string() << "\n";
        return false;
    }

    json_min::ParseResult pr = json_min::Parse(text);
    if (!pr.error.empty()) {
        std::cerr << "Schema JSON parse failed for " << path.string() << ": " << pr.error << "\n";
        return false;
    }

    UISchemaLoadResult result = LoadUISchemaFromJson(pr.value);
    if (!result.error.empty()) {
        std::cerr << "Schema decode failed for " << path.string() << ": " << result.error << "\n";
        return false;
    }

    bool foundAutoMaxIter = false;
    bool foundMcMullenPreset = false;
    for (const auto& panel : result.schema.panels) {
        for (const auto& ctrl : panel.controls) {
            if (ctrl.id == "auto_max_iter" && ctrl.has_binding && ctrl.binding.path == "fractal.view.auto_max_iter") {
                foundAutoMaxIter = true;
            }
            if (ctrl.id == "mcmullen_preset" && ctrl.has_binding && ctrl.binding.path == "fractal.params.mcmullen_preset") {
                foundMcMullenPreset = true;
            }
        }
    }
    if (!foundAutoMaxIter) {
        std::cerr << "Schema missing auto_max_iter binding in " << path.string() << "\n";
        return false;
    }
    if (!foundMcMullenPreset) {
        std::cerr << "Schema missing mcmullen_preset binding in " << path.string() << "\n";
        return false;
    }

    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    LensSettings lens{};
    BindingContext ctx;
    ctx.view = &view;
    ctx.params = &params;
    ctx.render = &render;
    ctx.lens = &lens;

    std::string bindError;
    if (!ValidateSchemaBindings(result.schema, ctx, &bindError)) {
        std::cerr << "Schema binding validation failed for " << path.string() << ": " << bindError << "\n";
        return false;
    }
    return true;
}

} // namespace

int main() {
    // Test logarithmic flag parsing
    {
        const char* json = R"({
            "schema_version": "1",
            "namespace": "test",
            "panels": [
                {
                    "id": "p1",
                    "label": "Panel",
                    "controls": [
                        {
                            "id": "log_slider",
                            "type": "slider_float",
                            "label": "Log Slider",
                            "value_type": "float",
                            "min": 0.001,
                            "max": 10.0,
                            "logarithmic": true,
                            "binding": { "kind": "param", "path": "test.log_param" }
                        },
                        {
                            "id": "linear_slider",
                            "type": "slider_float",
                            "label": "Linear Slider",
                            "value_type": "float",
                            "min": 0.0,
                            "max": 1.0,
                            "binding": { "kind": "param", "path": "test.lin_param" }
                        }
                    ]
                }
            ]
        })";

        json_min::ParseResult pr = json_min::Parse(json);
        if (!pr.error.empty()) {
            std::cerr << "JSON parse failed: " << pr.error << "\n";
            return 1;
        }

        UISchemaLoadResult result = LoadUISchemaFromJson(pr.value);
        if (!result.error.empty()) {
            std::cerr << "Schema load failed: " << result.error << "\n";
            return 1;
        }

        if (result.schema.panels.size() != 1 || result.schema.panels[0].controls.size() != 2) {
            std::cerr << "Expected 1 panel with 2 controls\n";
            return 1;
        }

        const auto& logCtrl = result.schema.panels[0].controls[0];
        const auto& linCtrl = result.schema.panels[0].controls[1];

        if (!logCtrl.logarithmic) {
            std::cerr << "Log slider should have logarithmic=true\n";
            return 1;
        }
        if (linCtrl.logarithmic) {
            std::cerr << "Linear slider should have logarithmic=false (default)\n";
            return 1;
        }
    }

    // Test that the checked-in schema file the app depends on parses,
    // decodes, and validates against the live binding surface.
    {
        const std::filesystem::path repoRoot = FindRepoRoot();
        if (repoRoot.empty()) {
            std::cerr << "Could not locate repo root for schema validation test\n";
            return 1;
        }

        const std::filesystem::path schemaPath = repoRoot / "ui" / "fractal_binding_surface_v1.ui_schema.json";

        bool foundLogRate = false;
        bool foundPhaseSlider = false;
        bool foundDualSeedB = false;
        bool foundDualSeedMix = false;
        bool foundDualSeedColoring = false;
        bool foundMultibrotPowerFloat = false;
        bool foundLambdaReal = false;
        bool foundLambdaImag = false;

        if (!LoadAndValidateSchemaFile(schemaPath)) {
            return 1;
        }

        std::string text;
        if (!ReadTextFile(schemaPath, &text)) {
            std::cerr << "Failed to re-read schema file: " << schemaPath.string() << "\n";
            return 1;
        }
        json_min::ParseResult pr2 = json_min::Parse(text);
        UISchemaLoadResult result = LoadUISchemaFromJson(pr2.value);

        for (const auto& panel : result.schema.panels) {
            for (const auto& ctrl : panel.controls) {
                if (ctrl.id == "explaino_seed_rate" && ctrl.logarithmic) foundLogRate = true;
                if (ctrl.id == "explaino_phase" && ctrl.type == "slider_float") foundPhaseSlider = true;
                if (ctrl.id == "explaino_seed_b") foundDualSeedB = true;
                if (ctrl.id == "explaino_mix") foundDualSeedMix = true;
                if (ctrl.id == "multibrot_power_float" && ctrl.type == "slider_float" && ctrl.has_binding &&
                    ctrl.binding.path == "fractal.params.multibrot_power_float") {
                    foundMultibrotPowerFloat = true;
                }
                if (ctrl.id == "lambda_real" && ctrl.has_binding && ctrl.binding.path == "fractal.params.lambda_real") {
                    foundLambdaReal = true;
                }
                if (ctrl.id == "lambda_imag" && ctrl.has_binding && ctrl.binding.path == "fractal.params.lambda_imag") {
                    foundLambdaImag = true;
                }
                if (ctrl.id == "coloring_mode_newton" && ctrl.has_visible_if &&
                    ctrl.visible_if.value.find("explaino_dual") != std::string::npos) {
                    foundDualSeedColoring = true;
                }
            }
        }

        if (!foundLogRate) {
            std::cerr << "Did not find explaino_seed_rate control with logarithmic=true in schema\n";
            return 1;
        }
        if (!foundPhaseSlider) {
            std::cerr << "Did not find explaino_phase slider_float control in schema\n";
            return 1;
        }
        if (!foundMultibrotPowerFloat) {
            std::cerr << "Did not find non-integer Multibrot power float control in schema\n";
            return 1;
        }
        if (!foundLambdaReal || !foundLambdaImag) {
            std::cerr << "Did not find Lambda real/imag controls in schema\n";
            return 1;
        }
        if (!foundDualSeedB || !foundDualSeedMix || !foundDualSeedColoring) {
            std::cerr << "Did not find Explaino-DualSeed controls and coloring visibility in schema\n";
            return 1;
        }
    }

    std::cout << "test_ui_schema: all passed\n";
    return 0;
}
