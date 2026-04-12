#pragma once

#include "explaino_sidecar_energy.h"

#include <string>

struct SidecarActionRecommendation {
    std::string label;
    std::string path;
    std::string type;
    std::string guidance;
    double current_value{0.0};
    double estimated_information_gain{0.0};
    double effective_information_gain{0.0};
    double cumulative_information_gain{0.0};
    double information_gradient{0.0};
    double information_curvature{0.0};
    double posterior_uncertainty{1.0};
    double decode_stability{1.0};
    double cost_hint{0.0};
    double gamma{1.0};
    double utility{0.0};
    double active_min{0.0};
    double active_max{0.0};
    double active_fraction{0.0};
    int observation_count{0};
};

bool BuildSidecarActionRecommendation(
    const SidecarEnergyLandscape& energyLandscape,
    SidecarActionRecommendation* outRecommendation,
    std::string* outError);

bool BuildSidecarActionRecommendation(
    const SidecarHypothesisSpace& space,
    const SidecarBudgetState& budget,
    const SidecarLensProjection& lens,
    SidecarActionRecommendation* outRecommendation,
    std::string* outError);