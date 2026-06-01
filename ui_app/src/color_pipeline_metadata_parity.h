#pragma once

#include "color_pipeline_metadata_contract.h"

#include <string>
#include <vector>

struct ColorPipelineLaneTaxonomyGroups {
    std::string lane_id;
    std::vector<std::string> groups;
};

struct ColorPipelineMetadataParityReport {
    bool ok = false;
    int schema_version = 0;
    int lane_count = 0;
    int function_count = 0;
    std::string catalog_authority;
    int active_catalog_function_count = 0;
    int compatibility_count = 0;
    std::string compatibility_authority;
    int active_compatibility_count = 0;
    bool typed_compatibility_pilot_enabled = false;
    std::string typed_compatibility_pilot_authority;
    std::string companion_suggestion_authority;
    int active_companion_suggestion_count = 0;
    int recipe_count = 0;
    std::string recipe_expansion_authority;
    int active_recipe_count = 0;
    int unsupported_pair_count = 0;
    int taxonomy_group_count = 0;
    std::vector<ColorPipelineLaneTaxonomyGroups> lane_taxonomy_groups;
    std::vector<std::string> errors;
};

ColorPipelineMetadataParityReport ValidateColorPipelineMetadataParity(
    const MaterializedColorPipelineContract& contract);

std::string SerializeColorPipelineMetadataParityReportJson(
    const ColorPipelineMetadataParityReport& report,
    const std::string& contractPath);
