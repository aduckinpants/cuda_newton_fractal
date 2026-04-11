#include "../src/explaino_sidecar_action.h"

#include <cmath>
#include <iostream>
#include <string>

namespace {

bool NearlyEqual(double left, double right, double eps = 1.0e-9) {
    const double delta = left - right;
    return delta < eps && delta > -eps;
}

SidecarParamSurfaceEntry MakeSurfaceEntry(
    const char* path,
    const char* label,
    double minValue,
    double maxValue,
    bool hasCostHint,
    double costHint) {
    SidecarParamSurfaceEntry entry;
    entry.path = path;
    entry.label = label;
    entry.type = "float";
    entry.has_min = true;
    entry.min_value = minValue;
    entry.has_max = true;
    entry.max_value = maxValue;
    entry.has_declared_span = true;
    entry.declared_span = maxValue - minValue;
    entry.has_cost_hint = hasCostHint;
    entry.cost_hint = costHint;
    return entry;
}

SidecarBudgetRow MakeBudgetRow(
    const char* path,
    const char* label,
    double estimatedInformationGain,
    double cumulativeInformationGain,
    double posteriorUncertainty,
    double decodeStability,
    int observationCount) {
    SidecarBudgetRow row;
    row.path = path;
    row.label = label;
    row.type = "float";
    row.estimated_information_gain = estimatedInformationGain;
    row.cumulative_information_gain = cumulativeInformationGain;
    row.posterior_uncertainty = posteriorUncertainty;
    row.decode_stability = decodeStability;
    row.observation_count = observationCount;
    return row;
}

SidecarLensProjectionRow MakeLensRow(
    const char* path,
    const char* label,
    double activeMin,
    double activeMax,
    double activeFraction,
    bool inactive,
    const char* guidance) {
    SidecarLensProjectionRow row;
    row.path = path;
    row.label = label;
    row.type = "float";
    row.active_min = activeMin;
    row.active_max = activeMax;
    row.active_fraction = activeFraction;
    row.inactive = inactive;
    row.guidance = guidance;
    return row;
}

SidecarHypothesisSpace BuildSpace() {
    SidecarHypothesisSpace space;
    space.function_id = "fractal.sample";
    space.applicable_parameters.push_back(MakeSurfaceEntry("fractal.params.ripple_amplitude", "Ripple", 0.0, 0.15, true, 2.55));
    space.applicable_parameters.push_back(MakeSurfaceEntry("fractal.params.vortex_strength", "Vortex", 0.0, 0.3, true, 0.99));
    space.applicable_parameters.push_back(MakeSurfaceEntry("fractal.view.zoom", "Zoom", 1.0, 100.0, false, 0.0));
    return space;
}

SidecarBudgetState BuildBudget() {
    SidecarBudgetState budget;
    budget.function_id = "fractal.sample";
    budget.fractal_type_id = "explaino";
    budget.batch_count = 1;
    budget.rows.push_back(MakeBudgetRow("fractal.view.zoom", "Zoom", 6.0, 6.0, 0.8, 0.9, 1));
    budget.rows.push_back(MakeBudgetRow("fractal.params.ripple_amplitude", "Ripple", 4.0, 4.0, 0.8, 0.9, 1));
    budget.rows.push_back(MakeBudgetRow("fractal.params.vortex_strength", "Vortex", 2.5, 2.5, 0.8, 0.95, 1));
    return budget;
}

SidecarLensProjection BuildLens() {
    SidecarLensProjection lens;
    lens.function_id = "fractal.sample";
    lens.fractal_type_id = "explaino";
    lens.rows.push_back(MakeLensRow("fractal.view.zoom", "Zoom", 8.0, 11.0, 0.03, false, "explore -"));
    lens.rows.push_back(MakeLensRow("fractal.params.ripple_amplitude", "Ripple", 0.02, 0.13, 0.73, false, "explore +"));
    lens.rows.push_back(MakeLensRow("fractal.params.vortex_strength", "Vortex", 0.08, 0.22, 0.47, false, "refine +"));
    return lens;
}

} // namespace

int main() {
    {
        SidecarActionRecommendation recommendation;
        std::string error;
        if (!BuildSidecarActionRecommendation(BuildSpace(), BuildBudget(), BuildLens(), &recommendation, &error)) {
            std::cerr << "Expected sidecar action recommendation to build: " << error << "\n";
            return 1;
        }
        if (recommendation.path != "fractal.params.vortex_strength") {
            std::cerr << "Expected lower-cost vortex action to outrank ripple after the cost penalty\n";
            return 1;
        }
        if (recommendation.guidance != "refine +") {
            std::cerr << "Expected recommendation to carry the selected lens guidance\n";
            return 1;
        }
        if (!NearlyEqual(recommendation.cost_hint, 0.99)) {
            std::cerr << "Expected recommendation to surface the selected cost hint\n";
            return 1;
        }
        if (!(recommendation.utility > 0.0)) {
            std::cerr << "Expected selected recommendation to expose positive utility in this fixture\n";
            return 1;
        }
        if (!NearlyEqual(recommendation.active_min, 0.08) || !NearlyEqual(recommendation.active_max, 0.22)) {
            std::cerr << "Expected recommendation to carry the selected active zone\n";
            return 1;
        }
    }

    {
        SidecarLensProjection lens = BuildLens();
        lens.rows[2].inactive = true;
        SidecarActionRecommendation recommendation;
        std::string error;
        if (!BuildSidecarActionRecommendation(BuildSpace(), BuildBudget(), lens, &recommendation, &error)) {
            std::cerr << "Expected action recommendation to fall back to the next eligible row: " << error << "\n";
            return 1;
        }
        if (recommendation.path != "fractal.params.ripple_amplitude") {
            std::cerr << "Expected inactive lens rows to be skipped during action recommendation\n";
            return 1;
        }
    }

    {
        SidecarHypothesisSpace noCostSpace;
        noCostSpace.function_id = "fractal.sample";
        noCostSpace.applicable_parameters.push_back(MakeSurfaceEntry("fractal.view.zoom", "Zoom", 1.0, 100.0, false, 0.0));

        SidecarBudgetState budget;
        budget.function_id = "fractal.sample";
        budget.fractal_type_id = "explaino";
        budget.rows.push_back(MakeBudgetRow("fractal.view.zoom", "Zoom", 4.0, 4.0, 1.0, 1.0, 1));

        SidecarLensProjection lens;
        lens.function_id = "fractal.sample";
        lens.fractal_type_id = "explaino";
        lens.rows.push_back(MakeLensRow("fractal.view.zoom", "Zoom", 8.0, 12.0, 0.04, false, "explore -"));

        SidecarActionRecommendation recommendation;
        std::string error;
        if (BuildSidecarActionRecommendation(noCostSpace, budget, lens, &recommendation, &error)) {
            std::cerr << "Expected missing cost hints to make passive action recommendation unavailable\n";
            return 1;
        }
        if (error.find("no eligible cost-annotated numeric param") == std::string::npos) {
            std::cerr << "Expected missing-cost recommendation failure to mention the ineligible surface\n";
            return 1;
        }
    }

    {
        SidecarLensProjection lens = BuildLens();
        lens.rows.push_back(lens.rows[1]);
        SidecarActionRecommendation recommendation;
        std::string error;
        if (BuildSidecarActionRecommendation(BuildSpace(), BuildBudget(), lens, &recommendation, &error)) {
            std::cerr << "Expected duplicate lens rows to fail action recommendation\n";
            return 1;
        }
        if (error.find("duplicate lens row") == std::string::npos) {
            std::cerr << "Expected duplicate-lens invariant failure to mention the duplicate row\n";
            return 1;
        }
    }

    {
        SidecarBudgetState budget = BuildBudget();
        budget.rows.push_back(budget.rows[1]);
        SidecarActionRecommendation recommendation;
        std::string error;
        if (BuildSidecarActionRecommendation(BuildSpace(), budget, BuildLens(), &recommendation, &error)) {
            std::cerr << "Expected duplicate budget rows to fail action recommendation\n";
            return 1;
        }
        if (error.find("duplicate budget row") == std::string::npos) {
            std::cerr << "Expected duplicate-budget invariant failure to mention the duplicate row\n";
            return 1;
        }
    }

    std::cout << "test_explaino_sidecar_action: all passed\n";
    return 0;
}