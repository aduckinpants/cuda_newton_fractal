#pragma once

#include "explaino_sidecar_lens.h"
#include "explaino_sidecar_model.h"

#include <string>
#include <vector>

enum class SidecarEnergyLandscapeRowStatus {
    available,
    inactive,
    missing_cost_hint,
    unsupported_type,
};

struct SidecarEnergyLandscapeRow {
    std::string label;
    std::string path;
    std::string type;
    std::string guidance;
    std::string summary;
    std::string reason;
    double current_value{0.0};
    double estimated_information_gain{0.0};
    double effective_information_gain{0.0};
    double cumulative_information_gain{0.0};
    double information_gradient{0.0};
    double information_curvature{0.0};
    double posterior_uncertainty{1.0};
    double decode_stability{1.0};
    double cost_hint{0.0};
    double gamma{0.0};
    double energy{0.0};
    double active_min{0.0};
    double active_max{0.0};
    double active_fraction{0.0};
    int observation_count{0};
    SidecarEnergyLandscapeRowStatus status{SidecarEnergyLandscapeRowStatus::unsupported_type};
    bool recommendation_eligible{false};
};

struct SidecarEnergyLandscape {
    std::string function_id;
    std::string fractal_type_id;
    int available_row_count{0};
    int recommendation_eligible_count{0};
    double peak_energy{0.0};
    double mean_energy{0.0};
    std::string peak_path;
    std::vector<SidecarEnergyLandscapeRow> rows;
};

bool BuildSidecarEnergyLandscape(
    const SidecarHypothesisSpace& space,
    const SidecarBudgetState& budget,
    const SidecarLensProjection& lens,
    SidecarEnergyLandscape* outLandscape,
    std::string* outError);