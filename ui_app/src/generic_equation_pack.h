#pragma once

#include "generic_function_types.h"
#include "json_min.h"

#include <map>
#include <string>
#include <string_view>
#include <vector>

struct GenericEquationPackControl {
    std::string id;
    std::string param;
    std::string label;
    bool has_min{false};
    bool has_max{false};
    bool has_step{false};
    bool has_default_value{false};
    double min_value{0.0};
    double max_value{0.0};
    double step_value{0.0};
    double default_value{0.0};
};

struct GenericEquationPackRegion {
    bool has_region{false};
    double center_x{0.0};
    double center_y{0.0};
    double span_x{0.0};
    double span_y{0.0};
    int grid_width{0};
    int grid_height{0};
};

struct GenericEquationPack {
    int schema_version{0};
    std::string pack_id;
    std::string name;
    std::string formula_kind;
    std::string iteration_param;
    json_min::Value ast;
    std::map<std::string, double> params;
    std::vector<GenericEquationPackControl> controls;
    double epsilon{1.0e-6};
    double escape_radius{1000.0};
    GenericEquationPackRegion region;
};

struct GenericEquationPackParseResult {
    GenericEquationPack pack;
    bool ok{false};
    std::string error;
};

struct GenericEquationLowerResult {
    GenericFunctionDesc desc{};
    bool ok{false};
    std::string error;
};

GenericEquationPackParseResult ParseGenericEquationPackJson(std::string_view text);
GenericEquationPackParseResult ParseGenericEquationPackFromValue(const json_min::Value& value);

GenericEquationLowerResult LowerGenericEquationPackToDesc(const GenericEquationPack& pack);
GenericEquationLowerResult LowerGenericEquationAstToDesc(
    const json_min::Value& ast,
    const std::map<std::string, double>& params);
GenericEquationLowerResult LowerGenericEquationAstToDesc(
    const json_min::Value& ast,
    const std::map<std::string, double>& params,
    const std::string& iterationParam);
