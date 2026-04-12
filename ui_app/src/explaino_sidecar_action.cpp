#include "explaino_sidecar_action.h"

namespace {

bool RecommendationOrder(const SidecarActionRecommendation& left,
    const SidecarActionRecommendation& right) {
    if (left.utility != right.utility) {
        return left.utility > right.utility;
    }
    if (left.effective_information_gain != right.effective_information_gain) {
        return left.effective_information_gain > right.effective_information_gain;
    }
    if (left.label != right.label) {
        return left.label < right.label;
    }
    return left.path < right.path;
}

} // namespace

bool BuildSidecarActionRecommendation(
    const SidecarEnergyLandscape& energyLandscape,
    SidecarActionRecommendation* outRecommendation,
    std::string* outError) {
    if (!outRecommendation) {
        if (outError) *outError = "BuildSidecarActionRecommendation requires outRecommendation";
        return false;
    }

    bool found = false;
    SidecarActionRecommendation best;
    for (const SidecarEnergyLandscapeRow& energyRow : energyLandscape.rows) {
        if (!energyRow.recommendation_eligible) {
            continue;
        }

        SidecarActionRecommendation candidate;
        candidate.label = energyRow.label;
        candidate.path = energyRow.path;
        candidate.type = energyRow.type;
        candidate.guidance = energyRow.guidance;
        candidate.current_value = energyRow.current_value;
        candidate.estimated_information_gain = energyRow.estimated_information_gain;
        candidate.effective_information_gain = energyRow.effective_information_gain;
        candidate.cumulative_information_gain = energyRow.cumulative_information_gain;
        candidate.information_gradient = energyRow.information_gradient;
        candidate.information_curvature = energyRow.information_curvature;
        candidate.posterior_uncertainty = energyRow.posterior_uncertainty;
        candidate.decode_stability = energyRow.decode_stability;
        candidate.cost_hint = energyRow.cost_hint;
        candidate.gamma = energyRow.gamma;
        candidate.utility = energyRow.energy;
        candidate.active_min = energyRow.active_min;
        candidate.active_max = energyRow.active_max;
        candidate.active_fraction = energyRow.active_fraction;
        candidate.observation_count = energyRow.observation_count;

        if (!found || RecommendationOrder(candidate, best)) {
            best = std::move(candidate);
            found = true;
        }
    }

    if (!found) {
        *outRecommendation = {};
        if (outError) *outError = "Sidecar action recommendation found no eligible cost-annotated numeric param";
        return false;
    }

    *outRecommendation = std::move(best);
    return true;
}

bool BuildSidecarActionRecommendation(
    const SidecarHypothesisSpace& space,
    const SidecarBudgetState& budget,
    const SidecarLensProjection& lens,
    SidecarActionRecommendation* outRecommendation,
    std::string* outError) {
    SidecarEnergyLandscape energyLandscape;
    std::string energyError;
    if (!BuildSidecarEnergyLandscape(space, budget, lens, &energyLandscape, &energyError)) {
        *outRecommendation = {};
        if (outError) {
            *outError = energyError;
        }
        return false;
    }

    return BuildSidecarActionRecommendation(energyLandscape, outRecommendation, outError);
}