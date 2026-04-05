#include "../src/schema_startup_policy.h"

#include <iostream>

int main() {
    {
        SchemaStartupFailureResult result = ResolveSchemaBindingFailure(
            "D:/salt-fractal/cuda_newton_fractal_clone/runtime/ui/fractal_binding_surface_v1.ui_schema.canonical.json",
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
        if (result.warning.find("fractal_binding_surface_v1.ui_schema.canonical.json") == std::string::npos) {
            std::cerr << "Expected schema path missing from warning\n";
            return 1;
        }
    }

    std::cout << "test_schema_startup_policy: all passed\n";
    return 0;
}