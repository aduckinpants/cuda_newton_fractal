#include "explaino_sidecar_action.h"

#include <cmath>
#include <unordered_map>

namespace {

constexpr double kUtilityEpsilon = 1.0e-9;
constexpr double kActionCostGamma = 1.0;

double ClampUnit(double value) {
    return std::max(0.0, std::min(1.0, value));
}

bool IsActionNumericType(const std::string& type) {
    return type == "float" || type == "double" || type == "int";
}

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
    const SidecarHypothesisSpace& space,
    const SidecarBudgetState& budget,
    const SidecarLensProjection& lens,
    SidecarActionRecommendation* outRecommendation,
    std::string* outError) {
    if (!outRecommendation) {
        if (outError) *outError = "BuildSidecarActionRecommendation requires outRecommendation";
        return false;
    }
    if (budget.function_id != space.function_id) {
        *outRecommendation = {};
        if (outError) {
            *outError = "Sidecar action recommendation budget function_id mismatch: expected " +
                space.function_id + ", got " + budget.function_id;
        }
        return false;
    }
    if (lens.function_id != space.function_id) {
        *outRecommendation = {};
        if (outError) {
            *outError = "Sidecar action recommendation lens function_id mismatch: expected " +
                space.function_id + ", got " + lens.function_id;
        }
        return false;
    }
    if (lens.fractal_type_id != budget.fractal_type_id) {
        *outRecommendation = {};
        if (outError) {
            *outError = "Sidecar action recommendation fractal_type mismatch: budget=" +
                budget.fractal_type_id + ", lens=" + lens.fractal_type_id;
        }
        return false;
    }

    std::unordered_map<std::string, const SidecarParamSurfaceEntry*> surfaceByPath;
    surfaceByPath.reserve(space.applicable_parameters.size());
    for (const SidecarParamSurfaceEntry& entry : space.applicable_parameters) {
        const auto [_, inserted] = surfaceByPath.emplace(entry.path, &entry);
        if (!inserted) {
            *outRecommendation = {};
            if (outError) *outError = "Sidecar action recommendation saw duplicate surface path: " + entry.path;
            return false;
        }
    }

    std::unordered_map<std::string, const SidecarLensProjectionRow*> lensByPath;
    lensByPath.reserve(lens.rows.size());
    for (const SidecarLensProjectionRow& row : lens.rows) {
        const auto [_, inserted] = lensByPath.emplace(row.path, &row);
        if (!inserted) {
            *outRecommendation = {};
            if (outError) *outError = "Sidecar action recommendation saw duplicate lens row path: " + row.path;
            return false;
        }
    }

    std::unordered_map<std::string, const SidecarBudgetRow*> budgetByPath;
    budgetByPath.reserve(budget.rows.size());
    for (const SidecarBudgetRow& row : budget.rows) {
        const auto [_, inserted] = budgetByPath.emplace(row.path, &row);
        if (!inserted) {
            *outRecommendation = {};
            if (outError) *outError = "Sidecar action recommendation saw duplicate budget row path: " + row.path;
            return false;
        }
    }

    bool found = false;
    SidecarActionRecommendation best;
    for (const SidecarBudgetRow& budgetRow : budget.rows) {
        const auto surfaceIt = surfaceByPath.find(budgetRow.path);
        if (surfaceIt == surfaceByPath.end()) {
            *outRecommendation = {};
            if (outError) *outError = "Sidecar action recommendation missing surface entry for path: " + budgetRow.path;
            return false;
        }

        const auto lensIt = lensByPath.find(budgetRow.path);
        if (lensIt == lensByPath.end()) {
            *outRecommendation = {};
            if (outError) *outError = "Sidecar action recommendation missing lens row for path: " + budgetRow.path;
            return false;
        }

        const SidecarParamSurfaceEntry& surface = *surfaceIt->second;
        const SidecarLensProjectionRow& lensRow = *lensIt->second;
        if (budgetRow.type != surface.type) {
            *outRecommendation = {};
            if (outError) {
                *outError = "Sidecar action recommendation budget type mismatch for path: " + budgetRow.path +
                    " (surface=" + surface.type + ", budget=" + budgetRow.type + ")";
            }
            return false;
        }
        if (lensRow.type != surface.type) {
            *outRecommendation = {};
            if (outError) {
                *outError = "Sidecar action recommendation lens type mismatch for path: " + budgetRow.path +
                    " (surface=" + surface.type + ", lens=" + lensRow.type + ")";
            }
            return false;
        }
        if (!IsActionNumericType(surface.type)) {
            continue;
        }
        if (lensRow.inactive || !(budgetRow.estimated_information_gain > kUtilityEpsilon)) {
            continue;
        }
        if (!surface.has_cost_hint) {
            continue;
        }
        if (!std::isfinite(surface.cost_hint) || surface.cost_hint < 0.0) {
            *outRecommendation = {};
            if (outError) *outError = "Invalid sidecar action recommendation cost_hint for path: " + budgetRow.path;
            return false;
        }

        SidecarActionRecommendation candidate;
        candidate.label = budgetRow.label;
        candidate.path = budgetRow.path;
        candidate.type = budgetRow.type;
        candidate.guidance = lensRow.guidance;
        candidate.estimated_information_gain = budgetRow.estimated_information_gain;
        candidate.cumulative_information_gain = budgetRow.cumulative_information_gain;
        candidate.posterior_uncertainty = ClampUnit(budgetRow.posterior_uncertainty);
        candidate.decode_stability = ClampUnit(budgetRow.decode_stability);
        candidate.cost_hint = surface.cost_hint;
        candidate.gamma = kActionCostGamma;
        candidate.effective_information_gain =
            candidate.estimated_information_gain * candidate.posterior_uncertainty * candidate.decode_stability;
        candidate.utility = candidate.effective_information_gain - candidate.gamma * candidate.cost_hint;
        candidate.active_min = lensRow.active_min;
        candidate.active_max = lensRow.active_max;
        candidate.active_fraction = lensRow.active_fraction;
        candidate.observation_count = budgetRow.observation_count;

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