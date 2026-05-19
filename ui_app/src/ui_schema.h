#pragma once

#include <string>
#include <vector>

#include "json_min.h"

struct UISchemaPredicate {
    std::string op;   // eq, neq, lt, lte, gt, gte
    std::string path; // binding path
    std::string value; // stored as string; interpreted by binding layer
};

struct UISchemaOption {
    std::string id;
    std::string label;
    std::string group;

    bool has_visible_if = false;
    UISchemaPredicate visible_if;
};

struct UISchemaBinding {
    std::string kind; // param | action
    std::string path;
};

struct UISchemaControl {
    std::string id;
    std::string type;
    std::string label;

    std::string value_type; // float | double | int | bool | enum | vec2 | vec3 | vec4
    double ui_min = 0.0;
    double ui_max = 0.0;
    bool has_ui_min = false;
    bool has_ui_max = false;
    double min = 0.0;
    double max = 0.0;
    double step = 0.0;
    bool has_min = false;
    bool has_max = false;
    bool has_step = false;

    // default: store as string/number/bool using JSON value
    json_min::Value def;
    bool has_default = false;

    std::string help;
    bool has_help = false;

    std::vector<UISchemaOption> options;

    bool has_binding = false;
    UISchemaBinding binding;

    bool has_visible_if = false;
    UISchemaPredicate visible_if;

    bool logarithmic = false;

    int order = 0;
    bool has_order = false;
};

struct UISchemaPanel {
    std::string id;
    std::string label;
    int order = 0;
    bool has_order = false;
    std::vector<UISchemaControl> controls;
};

struct UISchema {
    std::string schema_version;
    std::string name_space;
    bool incomplete = false;
    std::vector<std::string> notes;
    std::vector<UISchemaPanel> panels;
};

struct UISchemaLoadResult {
    UISchema schema;
    std::string error;
};

UISchemaLoadResult LoadUISchemaFromJson(const json_min::Value& root);
