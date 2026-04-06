#include "../src/schema_startup_policy.h"

#include <iostream>

int main() {
    {
        SchemaStartupFailureResult result = ResolveSchemaBindingFailure(
            "D:/salt-fractal/cuda_newton_fractal_clone/runtime/ui/fractal_binding_surface_v1.ui_schema.json",
            "Unknown action binding path: fractal.actions.bad_path (control: bad_button)");

        if (!result.enter_safe_mode) {
            std::cerr << "Schema binding failures should enter safe mode instead of blocking startup\n";
            return 1;
        }
        if (result.warning.find("Schema binding error (entering Safe Mode):") == std::string::npos) {
            std::cerr << "Expected warning header missing\n";
            return 1;
        }
        if (result.warning.find("fractal.actions.bad_path") == std::string::npos) {
            std::cerr << "Expected binding error details missing\n";
            return 1;
        }
        if (result.warning.find("fractal_binding_surface_v1.ui_schema.json") == std::string::npos) {
            std::cerr << "Expected schema path missing from warning\n";
            return 1;
        }
    }

    // validate-ui mode: binding failures must NOT enter safe mode (must fail hard)
    {
        SchemaStartupFailureResult result = ResolveSchemaBindingFailure(
            "schema.json",
            "Bind failed for float path: fractal.params.color_saturation (control: color_saturation)",
            true);

        if (result.enter_safe_mode) {
            std::cerr << "validate-ui mode should reject (not enter safe mode) on binding failure\n";
            return 1;
        }
        if (result.warning.find("validation failed") == std::string::npos) {
            std::cerr << "validate-ui warning should mention validation failed\n";
            return 1;
        }
    }

    // normal mode: same error should enter safe mode
    {
        SchemaStartupFailureResult result = ResolveSchemaBindingFailure(
            "schema.json",
            "Bind failed for float path: fractal.params.color_saturation (control: color_saturation)",
            false);

        if (!result.enter_safe_mode) {
            std::cerr << "normal mode should enter safe mode on binding failure\n";
            return 1;
        }
    }

    std::cout << "test_schema_startup_policy: all passed\n";
    return 0;
}