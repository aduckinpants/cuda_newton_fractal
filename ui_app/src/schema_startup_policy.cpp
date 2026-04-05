#include "schema_startup_policy.h"

SchemaStartupFailureResult ResolveSchemaBindingFailure(const std::string& schemaPath, const std::string& bindingError) {
    SchemaStartupFailureResult result;
    result.enter_safe_mode = true;
    result.warning = "Schema binding error (entering Safe Mode):\n" + schemaPath + "\n" + bindingError;
    return result;
}