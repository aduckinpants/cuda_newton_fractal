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


struct MaterializedColorPipelinePort {
    std::string direction;
    std::string id;
    std::string type;
    bool canonical = false;
    std::string generic_group;
};

struct MaterializedColorPipelineAdapter {
    std::string id;
    std::string source;
    std::string target;
    std::string policy;
    bool lossy = false;
    bool reversible = false;
    int cost = 0;
    std::string fail_closed_reason;
};

struct MaterializedColorPipelineEdgePolicy {
    std::string id;
    int max_adapter_hops = 0;
    bool allow_lossy = false;
    bool allow_visible_default = false;
    bool allow_explicit = false;
    bool allow_diagnostic = false;
    bool fail_closed_default = false;
};

struct MaterializedColorPipelineEdgeLink {
    std::string id;
    std::string from_lane;
    std::string to_lane;
    std::string from_port;
    std::string to_port;
    std::string fail_closed_reason;
};

struct MaterializedColorPipelineRouteEdge {
    std::string edge_id;
    std::string from_function;
    std::string to_function;
    std::string from_type;
    std::string to_type;
    std::string output_type;
    std::string status;
    std::vector<std::string> adapters;
    int adapter_hops = 0;
    int adapter_cost = 0;
};

struct MaterializedColorPipelineResolutionCase {
    std::string id;
    std::string source;
    std::string shape;
    std::string palette;
    std::string grading;
    std::string expected_status;
    std::string status;
    bool allow_lossy = false;
    bool allow_visible_default = false;
    bool explicit_adapter_consent = false;
    bool diagnostic_adapter_consent = false;
    std::vector<std::string> chosen_adapters;
    int adapter_hops = 0;
    int adapter_cost = 0;
    std::string tie_break_rule;
    std::vector<std::string> policy_blockers;
    std::vector<MaterializedColorPipelineRouteEdge> route_edges;
    std::string fail_closed_reason;
};

struct MaterializedSignalType {
    std::string id;
    std::string kind;
    std::string domain;
    std::string topology;
    int arity = 0;
    std::string default_adapter_policy;
    std::string units;
    bool has_period = false;
    double period = 0.0;
    std::string color_space;
    std::string coordinate_space;
};

struct MaterializedColorPipelineFunction {
    std::string id;
    std::string label;
    std::string description;
    std::string taxonomy_group;
    bool runtime_backed = false;
    std::string input_kind;
    std::string output_kind;
    std::string signal_kind;
    std::string typed_signal;
    std::vector<MaterializedColorPipelineParam> params;
    std::vector<MaterializedColorPipelinePort> ports;
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

struct MaterializedColorPipelineRowApplicator {
    std::string id;
    std::string label;
    std::string target_lane;
    std::string required_signal_kind;
    bool requires_sdf_field = false;
    std::string storage_param;
    std::string width_param;
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
    std::vector<MaterializedSignalType> signal_types;
    std::vector<MaterializedColorPipelineAdapter> adapters;
    MaterializedColorPipelineEdgePolicy edge_policy;
    std::vector<MaterializedColorPipelineEdgeLink> edge_links;
    std::vector<MaterializedColorPipelineResolutionCase> resolution_cases;
    std::vector<MaterializedColorPipelineLane> lanes;
    std::vector<MaterializedColorPipelineCompatibility> compatibility;
    std::vector<MaterializedColorPipelineRecipe> recipes;
    std::vector<MaterializedColorPipelineRowApplicator> row_applicators;
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
