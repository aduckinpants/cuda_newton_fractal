#pragma once

#include <string>
#include <vector>

struct MaterializedColorPipelineParam {
    std::string path;
    std::string type;
    std::string label;
    bool has_min = false;
    double min_value = 0.0;
    bool has_max = false;
    double max_value = 0.0;
    bool has_step = false;
    double step_value = 0.0;
    bool has_default = false;
    std::string default_kind;
    double number_default = 0.0;
    bool bool_default = false;
    std::string string_default;
    std::vector<std::string> enum_options;
};

struct MaterializedColorPipelineFunction {
    std::string id;
    std::string label;
    std::string description;
    bool runtime_backed = false;
    std::string input_kind;
    std::string output_kind;
    std::string signal_kind;
    std::vector<MaterializedColorPipelineParam> params;
};

struct MaterializedColorPipelineLane {
    std::string id;
    std::string label;
    std::string default_function_id;
    std::vector<MaterializedColorPipelineFunction> functions;
};

struct MaterializedColorPipelineCompatibility {
    std::string source;
    std::string palette;
    std::string signal;
    std::string palette_runtime;
    std::string grading;
    std::string mode;
    std::string reason;
};

struct MaterializedColorPipelineRecipe {
    std::string id;
    std::string label;
    std::string source;
    std::string shape;
    std::string palette;
    std::string grading;
    std::string fail_closed_reason;
};

struct MaterializedExplainoContractEntry {
    std::string id;
    std::string hypothesis_space;
    std::string authority;
    std::string lens;
    std::string invariant;
    std::string proof;
    std::string fallback;
    bool product_facing = false;
    bool diagnostic = false;
};

struct MaterializedColorPipelineContract {
    int schema_version = 0;
    std::string source_path;
    std::vector<MaterializedColorPipelineLane> lanes;
    std::vector<MaterializedColorPipelineCompatibility> compatibility;
    std::vector<MaterializedColorPipelineRecipe> recipes;
    std::vector<MaterializedExplainoContractEntry> explaino_entries;
};

bool LoadColorPipelineMaterializedContractJson(
    const std::string& path,
    MaterializedColorPipelineContract* outContract,
    std::string* outError = nullptr);

const MaterializedColorPipelineLane* FindMaterializedColorPipelineLane(
    const MaterializedColorPipelineContract& contract,
    const std::string& laneId);

const MaterializedColorPipelineFunction* FindMaterializedColorPipelineFunction(
    const MaterializedColorPipelineLane& lane,
    const std::string& functionId);

const MaterializedColorPipelineCompatibility* FindMaterializedColorPipelineCompatibility(
    const MaterializedColorPipelineContract& contract,
    const std::string& sourceFunctionId,
    const std::string& paletteFunctionId);
