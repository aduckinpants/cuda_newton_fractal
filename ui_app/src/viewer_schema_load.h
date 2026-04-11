#pragma once

#include "ui_schema.h"
#include "schema_binding.h"
#include <string>
#include <vector>

struct SchemaLoadResult {
    UISchema schema;
    std::string path;
    std::string warning;  // non-empty => Safe Mode active
    bool from_file{false};
    bool fatal_error{false};  // caller should exit(1)
};

// Load UI schema from the first valid candidate path.  If no file loads
// or parsing fails, falls back to Safe Mode.  Then validates bindings;
// on binding failure, resolves via schema_startup_policy.  If resolution
// says "do not enter safe mode" (e.g. --validate-ui), sets fatal_error.
SchemaLoadResult LoadAndValidateViewerSchema(
    const std::vector<std::string>& candidates,
    BindingContext& bind,
    bool validate_ui_only);

std::vector<std::string> BuildViewerSchemaCandidates(const std::string& exeDir);
