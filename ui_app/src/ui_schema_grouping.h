#pragma once

#include "ui_schema.h"

#include <algorithm>
#include <string>
#include <vector>

inline bool HasGroupedOptions(const UISchemaControl& control) {
    for (const auto& option : control.options) {
        if (!option.group.empty()) return true;
    }
    return false;
}

inline std::string NormalizedOptionGroup(const UISchemaOption& option, bool hasExplicitGroups) {
    if (!hasExplicitGroups) return "";
    return option.group.empty() ? std::string("Other") : option.group;
}

inline std::vector<std::string> CollectOptionGroups(const UISchemaControl& control) {
    std::vector<std::string> groups;
    const bool hasExplicitGroups = HasGroupedOptions(control);
    for (const auto& option : control.options) {
        const std::string group = NormalizedOptionGroup(option, hasExplicitGroups);
        if (group.empty()) continue;
        if (std::find(groups.begin(), groups.end(), group) == groups.end()) {
            groups.push_back(group);
        }
    }
    return groups;
}

inline std::string OptionGroupForId(const UISchemaControl& control, const std::string& optionId) {
    const bool hasExplicitGroups = HasGroupedOptions(control);
    for (const auto& option : control.options) {
        if (option.id == optionId) {
            return NormalizedOptionGroup(option, hasExplicitGroups);
        }
    }
    const std::vector<std::string> groups = CollectOptionGroups(control);
    return groups.empty() ? std::string() : groups.front();
}

inline std::vector<const UISchemaOption*> OptionsForGroup(const UISchemaControl& control, const std::string& group) {
    std::vector<const UISchemaOption*> options;
    const bool hasExplicitGroups = HasGroupedOptions(control);
    for (const auto& option : control.options) {
        if (!hasExplicitGroups || NormalizedOptionGroup(option, hasExplicitGroups) == group) {
            options.push_back(&option);
        }
    }
    return options;
}