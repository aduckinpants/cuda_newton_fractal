#pragma once

#include "color_pipeline_metadata_contract.h"

#include <string>
#include <vector>

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
    int unsupported_pair_count = 0;
    std::vector<std::string> errors;
};

ColorPipelineMetadataParityReport ValidateColorPipelineMetadataParity(
    const MaterializedColorPipelineContract& contract);

std::string SerializeColorPipelineMetadataParityReportJson(
    const ColorPipelineMetadataParityReport& report,
    const std::string& contractPath);
