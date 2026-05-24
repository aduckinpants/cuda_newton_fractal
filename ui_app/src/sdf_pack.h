#pragma once

#include "json_min.h"
#include "sdf_pack_runtime_types.h"

#include <map>
#include <string>
#include <string_view>
#include <vector>

struct SdfPackParam {
    std::string id;
    std::string type;
    double default_value{0.0};
    double min_value{0.0};
    double max_value{0.0};
};

struct SdfPackControl {
    std::string param;
    std::string label;
    bool has_ui_min{false};
    bool has_ui_max{false};
    double ui_min{0.0};
    double ui_max{0.0};
};

struct SdfPackRegion {
    bool has_region{false};
    double center_x{0.0};
    double center_y{0.0};
    double half_height{1.0};
};

struct SdfPack {
    int schema{0};
    std::string pack_id;
    std::string name;
    std::string kind;
    std::vector<SdfPackParam> params;
    std::vector<SdfPackControl> controls;
    SdfPackRegion region;
    json_min::Value ast;
};

struct SdfPackParseResult {
    SdfPack pack;
    bool ok{false};
    std::string error;
};

struct SdfPackSampleResult {
    bool ok{false};
    double distance{0.0};
    std::string error;
};

struct SdfPackLowerResult {
    SdfPackRuntimeDesc desc;
    bool ok{false};
    std::string error;
};

SdfPackParseResult ParseSdfPackJson(std::string_view text);
SdfPackParseResult ParseSdfPackFromValue(const json_min::Value& value);

bool BuildSdfPackParamValues(
    const SdfPack& pack,
    const std::map<std::string, double>& overrides,
    std::map<std::string, double>* outValues,
    std::string* outError);

SdfPackSampleResult SampleSdfPackCpu(
    const SdfPack& pack,
    double x,
    double y,
    const std::map<std::string, double>& overrides);

SdfPackLowerResult LowerSdfPackToRuntimeDesc(
    const SdfPack& pack,
    const std::map<std::string, double>& overrides);
