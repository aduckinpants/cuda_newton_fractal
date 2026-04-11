#pragma once

#include "explaino_sidecar_budget.h"

#include <string>
#include <vector>

struct SidecarExplorationCompletenessRow {
    std::string label;
    std::string path;
    std::string type;
    double posterior_uncertainty{1.0};
    int observation_count{0};
    double coverage_score{0.0};
    bool demonstrated{false};
    std::string coverage_bucket;
};

struct SidecarExplorationCompleteness {
    std::string function_id;
    std::string fractal_type_id;
    int demonstrated_count{0};
    int uncertain_count{0};
    double demonstrated_fraction{1.0};
    double mean_coverage_score{1.0};
    std::vector<SidecarExplorationCompletenessRow> rows;
};

bool BuildSidecarExplorationCompleteness(
    const SidecarHypothesisSpace& space,
    const SidecarBudgetState& budget,
    SidecarExplorationCompleteness* outCompleteness,
    std::string* outError);