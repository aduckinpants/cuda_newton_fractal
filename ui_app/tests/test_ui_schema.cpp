#include "../src/ui_schema.h"
#include "../src/json_min.h"

#include <fstream>
#include <iostream>
#include <sstream>

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

    // Test that the live schema file parses without error
    // (reads the actual schema file used by the app)
    {
        const char* schemaPath = "../../ui/fractal_binding_surface_v1.ui_schema.json";
        std::ifstream f(schemaPath, std::ios::in | std::ios::binary);
        if (!f) {
            // Try from build output directory
            schemaPath = "../../../ui/fractal_binding_surface_v1.ui_schema.json";
            f.open(schemaPath, std::ios::in | std::ios::binary);
        }
        if (f) {
            std::ostringstream ss;
            ss << f.rdbuf();
            std::string text = ss.str();

            json_min::ParseResult pr2 = json_min::Parse(text);
            if (!pr2.error.empty()) {
                std::cerr << "Live schema JSON parse failed: " << pr2.error << "\n";
                return 1;
            }

            UISchemaLoadResult result = LoadUISchemaFromJson(pr2.value);
            if (!result.error.empty()) {
                std::cerr << "Live schema load failed: " << result.error << "\n";
                return 1;
            }

            // Verify the seed rate control has logarithmic=true
            bool foundLogRate = false;
            bool foundDualSeedB = false;
            bool foundDualSeedMix = false;
            bool foundDualSeedColoring = false;
            for (const auto& panel : result.schema.panels) {
                for (const auto& ctrl : panel.controls) {
                    if (ctrl.id == "explaino_seed_rate") {
                        if (!ctrl.logarithmic) {
                            std::cerr << "explaino_seed_rate should be logarithmic\n";
                            return 1;
                        }
                        foundLogRate = true;
                    }
                    // Verify explaino_phase is slider_float
                    if (ctrl.id == "explaino_phase") {
                        if (ctrl.type != "slider_float") {
                            std::cerr << "explaino_phase should be slider_float, got: " << ctrl.type << "\n";
                            return 1;
                        }
                    }
                    if (ctrl.id == "explaino_seed_b") {
                        foundDualSeedB = true;
                    }
                    if (ctrl.id == "explaino_mix") {
                        foundDualSeedMix = true;
                    }
                    if (ctrl.id == "coloring_mode_newton" && ctrl.has_visible_if) {
                        if (ctrl.visible_if.value.find("explaino_dual") != std::string::npos) {
                            foundDualSeedColoring = true;
                        }
                    }
                }
            }
            if (!foundLogRate) {
                std::cerr << "Did not find explaino_seed_rate control in schema\n";
                return 1;
            }
            if (!foundDualSeedB || !foundDualSeedMix || !foundDualSeedColoring) {
                std::cerr << "Did not find Explaino-DualSeed controls and coloring visibility in schema\n";
                return 1;
            }
        }
        // Not an error if file not found (test may run from odd CWD)
    }

    std::cout << "test_ui_schema: all passed\n";
    return 0;
}
