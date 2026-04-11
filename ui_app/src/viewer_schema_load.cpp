#include "viewer_schema_load.h"

#include "headless_modes.h"
#include "json_min.h"
#include "safe_mode_schema.h"
#include "schema_startup_policy.h"

#include <filesystem>

namespace {

constexpr char kViewerSchemaFileName[] = "fractal_binding_surface_v1.ui_schema.json";

void AppendUniqueCandidate(const std::filesystem::path& candidatePath,
    std::vector<std::string>* outCandidates) {
    if (!outCandidates) return;
    const std::string candidate = candidatePath.lexically_normal().generic_string();
    if (candidate.empty()) return;
    for (const auto& existing : *outCandidates) {
        if (existing == candidate) return;
    }
    outCandidates->push_back(candidate);
}

} // namespace

std::vector<std::string> BuildViewerSchemaCandidates(const std::string& exeDir) {
    std::vector<std::string> candidates;
    if (exeDir.empty()) return candidates;

    const std::filesystem::path exePath = std::filesystem::path(exeDir).lexically_normal();
    AppendUniqueCandidate(exePath / "ui" / kViewerSchemaFileName, &candidates);
    AppendUniqueCandidate(exePath / ".." / "ui" / kViewerSchemaFileName, &candidates);
    return candidates;
}

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
