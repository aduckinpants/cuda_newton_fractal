#include "schema_startup_policy.h"

SchemaStartupFailureResult ResolveSchemaBindingFailure(const std::string& schemaPath, const std::string& bindingError, bool validate_ui_mode) {
    SchemaStartupFailureResult result;
    if (validate_ui_mode) {
        result.enter_safe_mode = false;
        result.warning = "Schema binding validation failed:\n" + schemaPath + "\n" + bindingError;
    } else {
        result.enter_safe_mode = true;
        result.warning = "Schema binding error (entering Safe Mode):\n" + schemaPath + "\n" + bindingError;
    }
    return result;
}