#include "color_pipeline_metadata_parity.h"

#include "color_pipeline_core.h"
#include "enum_id_utils.h"

#include <algorithm>
#include <set>
#include <sstream>

namespace {

void AddError(ColorPipelineMetadataParityReport* report, const std::string& error) {
    if (report) {
        report->errors.push_back(error);
    }
}

bool Near(double actual, double expected) {
    const double delta = actual - expected;
    return delta >= -1.0e-9 && delta <= 1.0e-9;
}

bool ParamMatches(const FunctionParamDescriptor& expected, const MaterializedColorPipelineParam& actual) {
    if (actual.path != expected.path || actual.type != expected.type || actual.label != expected.label) return false;
    if (actual.has_min != expected.has_min || actual.has_max != expected.has_max || actual.has_step != expected.has_step) return false;
    if (actual.has_min && !Near(actual.min_value, expected.min_value)) return false;
    if (actual.has_max && !Near(actual.max_value, expected.max_value)) return false;
    if (actual.has_step && !Near(actual.step_value, expected.step_value)) return false;
    if (actual.has_default != expected.has_default) return false;
    if (expected.has_default) {
        if (expected.default_value.is_number() && !(actual.default_kind == "number" && Near(actual.number_default, expected.default_value.as_number()))) return false;
        if (expected.default_value.is_bool() && !(actual.default_kind == "bool" && actual.bool_default == expected.default_value.as_bool())) return false;
        if (expected.default_value.is_string() && !(actual.default_kind == "string" && actual.string_default == expected.default_value.as_string())) return false;
    }
    if (actual.enum_options.size() != expected.options.size()) return false;
    for (std::size_t index = 0; index < actual.enum_options.size(); ++index) {
        if (actual.enum_options[index] != expected.options[index].id) return false;
    }
    return true;
}

void ValidateLaneCatalogs(
    const MaterializedColorPipelineContract& contract,
    ColorPipelineMetadataParityReport* report) {
    const std::vector<ColorPipelineLaneCatalog>& catalogs = color_pipeline_core::GetHardcodedColorPipelineLaneCatalogs();
    report->lane_count = static_cast<int>(contract.lanes.size());
    std::set<std::string> uniqueTaxonomyGroups;
    if (contract.lanes.size() != catalogs.size()) {
        AddError(report, "lane count mismatch");
        return;
    }
    for (std::size_t laneIndex = 0; laneIndex < catalogs.size(); ++laneIndex) {
        const ColorPipelineLaneCatalog& expectedLane = catalogs[laneIndex];
        const MaterializedColorPipelineLane* actualLane = FindMaterializedColorPipelineLane(contract, expectedLane.lane_id);
        if (!actualLane) {
            AddError(report, std::string("missing lane: ") + expectedLane.lane_id);
            continue;
        }
        if (contract.lanes[laneIndex].id != expectedLane.lane_id ||
            actualLane->label != expectedLane.label ||
            actualLane->default_function_id != expectedLane.default_function_id) {
            AddError(report, std::string("lane metadata mismatch: ") + expectedLane.lane_id);
        }
        if (actualLane->functions.size() != expectedLane.functions.size()) {
            AddError(report, std::string("function count mismatch for lane: ") + expectedLane.lane_id);
            continue;
        }
        report->function_count += static_cast<int>(actualLane->functions.size());
        ColorPipelineLaneTaxonomyGroups laneGroups;
        laneGroups.lane_id = expectedLane.lane_id;
        for (std::size_t functionIndex = 0; functionIndex < expectedLane.functions.size(); ++functionIndex) {
            const FunctionDescriptor& expectedFunction = expectedLane.functions[functionIndex];
            const MaterializedColorPipelineFunction& actualFunction = actualLane->functions[functionIndex];
            if (actualFunction.id != expectedFunction.id ||
                actualFunction.label != expectedFunction.name ||
                actualFunction.description != expectedFunction.description ||
                actualFunction.taxonomy_group != expectedFunction.taxonomy_group ||
                !actualFunction.runtime_backed) {
                AddError(report, std::string("function metadata mismatch: ") + expectedFunction.id);
            }
            if (expectedFunction.taxonomy_group.empty()) {
                AddError(report, std::string("missing taxonomy group: ") + expectedFunction.id);
            } else {
                if (uniqueTaxonomyGroups.insert(expectedFunction.taxonomy_group).second) {
                    ++report->taxonomy_group_count;
                }
                if (std::find(laneGroups.groups.begin(), laneGroups.groups.end(), expectedFunction.taxonomy_group) ==
                    laneGroups.groups.end()) {
                    laneGroups.groups.push_back(expectedFunction.taxonomy_group);
                }
            }
            if (actualFunction.params.size() != expectedFunction.parameters.size()) {
                AddError(report, std::string("parameter count mismatch: ") + expectedFunction.id);
                continue;
            }
            for (std::size_t paramIndex = 0; paramIndex < expectedFunction.parameters.size(); ++paramIndex) {
                if (!ParamMatches(expectedFunction.parameters[paramIndex], actualFunction.params[paramIndex])) {
                    AddError(report, std::string("parameter metadata mismatch: ") + expectedFunction.id + ":" + expectedFunction.parameters[paramIndex].path);
                }
            }
            if (std::string(expectedLane.lane_id) == "source") {
                const char* expectedKind = color_pipeline_core::ColorPipelineSourceSignalKindId(
                    color_pipeline_core::ColorPipelineSourceSignalKindForFunctionId(expectedFunction.id.c_str()));
                if (actualFunction.signal_kind != expectedKind) {
                    AddError(report, std::string("source signal-kind mismatch: ") + expectedFunction.id);
                }
            } else if (!actualFunction.signal_kind.empty()) {
                AddError(report, std::string("non-source function has signal_kind: ") + expectedFunction.id);
            }
        }
        report->lane_taxonomy_groups.push_back(std::move(laneGroups));
    }
}

void ValidateCompatibility(
    const MaterializedColorPipelineContract& contract,
    ColorPipelineMetadataParityReport* report) {
    report->compatibility_count = static_cast<int>(contract.compatibility.size());
    const std::vector<ColorPipelineLaneCatalog>& catalogs = color_pipeline_core::GetHardcodedColorPipelineLaneCatalogs();
    const ColorPipelineLaneCatalog* sourceLane = nullptr;
    const ColorPipelineLaneCatalog* paletteLane = nullptr;
    for (const ColorPipelineLaneCatalog& catalog : catalogs) {
        if (std::string(catalog.lane_id) == "source") {
            sourceLane = &catalog;
        } else if (std::string(catalog.lane_id) == "palette") {
            paletteLane = &catalog;
        }
    }
    if (!sourceLane || !paletteLane) {
        AddError(report, "missing source or palette lane");
        return;
    }
    for (const FunctionDescriptor& sourceFunction : sourceLane->functions) {
        for (const FunctionDescriptor& paletteFunction : paletteLane->functions) {
            ColorPipelineSelection selection;
            ColoringMode mode = ColoringMode::smooth_escape;
            const bool supported = color_pipeline_core::TryBuildHardcodedColorPipelineSelectionFromLaneIds(
                sourceFunction.id.c_str(),
                paletteFunction.id.c_str(),
                &selection,
                &mode);
            const MaterializedColorPipelineCompatibility* row =
                FindMaterializedColorPipelineCompatibility(contract, sourceFunction.id, paletteFunction.id);
            if (!supported) {
                ++report->unsupported_pair_count;
            }
            if (supported != (row != nullptr)) {
                AddError(report, std::string("compatibility allow/deny mismatch: ") + sourceFunction.id + "+" + paletteFunction.id);
                continue;
            }
            if (supported && row) {
                if (row->signal != color_pipeline_core::AdvancedColorSignalFunctionId(selection.signal) ||
                    row->palette_runtime != color_pipeline_core::AdvancedColorPaletteFunctionId(selection.palette) ||
                    row->grading != color_pipeline_core::AdvancedColorGradingFunctionId(selection.grading) ||
                    row->mode != ColoringModeId(mode)) {
                    AddError(report, std::string("compatibility tuple mismatch: ") + sourceFunction.id + "+" + paletteFunction.id);
                }
            }
        }
    }
}

void ValidateRecipes(
    const MaterializedColorPipelineContract& contract,
    ColorPipelineMetadataParityReport* report) {
    report->recipe_count = static_cast<int>(contract.recipes.size());
    const std::vector<MaterializedColorPipelineRecipe>& hardcoded =
        color_pipeline_core::GetHardcodedColorPipelineRecipes();
    if (contract.recipes.size() != hardcoded.size()) {
        AddError(report, "recipe count mismatch");
        return;
    }
    for (std::size_t recipeIndex = 0; recipeIndex < hardcoded.size(); ++recipeIndex) {
        const MaterializedColorPipelineRecipe& expected = hardcoded[recipeIndex];
        const MaterializedColorPipelineRecipe& actual = contract.recipes[recipeIndex];
        if (actual.id != expected.id ||
            actual.label != expected.label ||
            actual.source != expected.source ||
            actual.shape != expected.shape ||
            actual.palette != expected.palette ||
            actual.grading != expected.grading ||
            actual.fail_closed_reason != expected.fail_closed_reason) {
            AddError(report, std::string("recipe metadata mismatch: ") + expected.id);
        }
    }
}

std::string JsonEscape(const std::string& text) {
    std::ostringstream out;
    for (const char ch : text) {
        switch (ch) {
        case '\\': out << "\\\\"; break;
        case '"': out << "\\\""; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default: out << ch; break;
        }
    }
    return out.str();
}

} // namespace

ColorPipelineMetadataParityReport ValidateColorPipelineMetadataParity(
    const MaterializedColorPipelineContract& contract) {
    ColorPipelineMetadataParityReport report;
    report.schema_version = contract.schema_version;
    ValidateLaneCatalogs(contract, &report);
    ValidateCompatibility(contract, &report);
    ValidateRecipes(contract, &report);
    report.ok = report.errors.empty();
    return report;
}

std::string SerializeColorPipelineMetadataParityReportJson(
    const ColorPipelineMetadataParityReport& report,
    const std::string& contractPath) {
    std::ostringstream out;
    out << "{\n";
    out << "  \"ok\": " << (report.ok ? "true" : "false") << ",\n";
    out << "  \"contract_path\": \"" << JsonEscape(contractPath) << "\",\n";
    out << "  \"schema_version\": " << report.schema_version << ",\n";
    out << "  \"lane_count\": " << report.lane_count << ",\n";
    out << "  \"function_count\": " << report.function_count << ",\n";
    out << "  \"catalog_authority\": \"" << JsonEscape(report.catalog_authority) << "\",\n";
    out << "  \"active_catalog_function_count\": " << report.active_catalog_function_count << ",\n";
    out << "  \"compatibility_count\": " << report.compatibility_count << ",\n";
    out << "  \"compatibility_authority\": \"" << JsonEscape(report.compatibility_authority) << "\",\n";
    out << "  \"active_compatibility_count\": " << report.active_compatibility_count << ",\n";
    out << "  \"companion_suggestion_authority\": \"" << JsonEscape(report.companion_suggestion_authority) << "\",\n";
    out << "  \"active_companion_suggestion_count\": " << report.active_companion_suggestion_count << ",\n";
    out << "  \"recipe_count\": " << report.recipe_count << ",\n";
    out << "  \"recipe_expansion_authority\": \"" << JsonEscape(report.recipe_expansion_authority) << "\",\n";
    out << "  \"active_recipe_count\": " << report.active_recipe_count << ",\n";
    out << "  \"taxonomy_group_count\": " << report.taxonomy_group_count << ",\n";
    out << "  \"lane_taxonomy_groups\": {";
    for (std::size_t laneIndex = 0; laneIndex < report.lane_taxonomy_groups.size(); ++laneIndex) {
        if (laneIndex > 0) {
            out << ',';
        }
        const ColorPipelineLaneTaxonomyGroups& lane = report.lane_taxonomy_groups[laneIndex];
        out << "\n    \"" << JsonEscape(lane.lane_id) << "\": [";
        for (std::size_t groupIndex = 0; groupIndex < lane.groups.size(); ++groupIndex) {
            if (groupIndex > 0) {
                out << ", ";
            }
            out << "\"" << JsonEscape(lane.groups[groupIndex]) << "\"";
        }
        out << "]";
    }
    if (!report.lane_taxonomy_groups.empty()) {
        out << "\n  ";
    }
    out << "},\n";
    out << "  \"unsupported_pair_count\": " << report.unsupported_pair_count << ",\n";
    out << "  \"errors\": [";
    for (std::size_t index = 0; index < report.errors.size(); ++index) {
        if (index > 0) out << ", ";
        out << "\"" << JsonEscape(report.errors[index]) << "\"";
    }
    out << "]\n";
    out << "}\n";
    return out.str();
}
