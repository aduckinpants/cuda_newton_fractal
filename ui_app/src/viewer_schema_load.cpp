#include "viewer_schema_load.h"

#include "headless_modes.h"
#include "json_min.h"
#include "safe_mode_schema.h"
#include "schema_startup_policy.h"

SchemaLoadResult LoadAndValidateViewerSchema(
    const std::vector<std::string>& candidates,
    BindingContext& bind,
    bool validate_ui_only) {

    SchemaLoadResult result;

    for (const auto& cand : candidates) {
        std::string text = ReadTextFile(cand.c_str());
        if (text.empty()) continue;
        auto pr = json_min::Parse(text);
        if (!pr.error.empty()) {
            result.path = cand;
            result.warning = "Schema JSON parse error (entering Safe Mode):\n" + cand + "\n" + pr.error;
            break;
        }
        auto lr = LoadUISchemaFromJson(pr.value);
        if (!lr.error.empty()) {
            result.path = cand;
            result.warning = "Schema decode error (entering Safe Mode):\n" + cand + "\n" + lr.error;
            break;
        }
        result.schema = std::move(lr.schema);
        result.path = cand;
        result.from_file = true;
        break;
    }

    if (!result.from_file) {
        if (result.path.empty() && !candidates.empty()) result.path = candidates[0];
        if (result.warning.empty()) {
            result.warning = "Schema missing/unreadable (entering Safe Mode). Tried:";
            for (const auto& c : candidates) {
                result.warning += "\n  " + c;
            }
        }
        result.schema = BuildSafeModeSchema();
    }

    std::string bindError;
    if (!ValidateSchemaBindings(result.schema, bind, &bindError)) {
        SchemaStartupFailureResult failure = ResolveSchemaBindingFailure(result.path, bindError, validate_ui_only);
        if (!failure.enter_safe_mode) {
            result.fatal_error = true;
            return result;
        }
        result.warning = failure.warning;
        result.schema = BuildSafeModeSchema();
        std::string safeBindError;
        if (!ValidateSchemaBindings(result.schema, bind, &safeBindError)) {
            result.fatal_error = true;
            return result;
        }
    }

    return result;
}
