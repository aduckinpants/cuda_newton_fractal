#pragma once

#include <string>

struct SchemaStartupFailureResult {
    bool enter_safe_mode{false};
    std::string warning;
};

SchemaStartupFailureResult ResolveSchemaBindingFailure(const std::string& schemaPath, const std::string& bindingError, bool validate_ui_mode = false);