#include "../src/explaino_sidecar_energy.h"

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
        SidecarEnergyLandscape energy;
        std::string error;
        if (!BuildSidecarEnergyLandscape(BuildSpace(), BuildBudget(), BuildLens(), &energy, &error)) {
            std::cerr << "Expected sidecar energy landscape to build: " << error << "\n";
            return 1;
        }
        if (energy.rows.size() != 3) {
            std::cerr << "Expected energy landscape to retain one row per measured budget surface\n";
            return 1;
        }
        if (energy.available_row_count != 2 || energy.recommendation_eligible_count != 2) {
            std::cerr << "Expected two available cost-annotated numeric rows in the energy landscape fixture\n";
            return 1;
        }
        if (energy.peak_path != "fractal.params.vortex_strength") {
            std::cerr << "Expected lower-cost vortex row to define the peak energy entry\n";
            return 1;
        }
        if (energy.rows[0].status != SidecarEnergyLandscapeRowStatus::available ||
            energy.rows[0].path != "fractal.params.vortex_strength") {
            std::cerr << "Expected available energy rows to sort in descending energy order\n";
            return 1;
        }
        if (energy.rows[2].status != SidecarEnergyLandscapeRowStatus::missing_cost_hint ||
            energy.rows[2].path != "fractal.view.zoom") {
            std::cerr << "Expected missing-cost rows to remain explicit in the energy landscape\n";
            return 1;
        }
        if (std::isfinite(energy.rows[2].energy) || std::isfinite(energy.rows[2].cost_hint)) {
            std::cerr << "Expected missing-cost rows to avoid silently exposing fake zero energy/cost values\n";
            return 1;
        }
        if (!NearlyEqual(energy.rows[0].cost_hint, 0.99) || !(energy.rows[0].energy > energy.rows[1].energy)) {
            std::cerr << "Expected energy rows to expose cost and combined energy values\n";
            return 1;
        }
    }

    {
        SidecarLensProjection lens = BuildLens();
        lens.rows[2].inactive = true;
        SidecarEnergyLandscape energy;
        std::string error;
        if (!BuildSidecarEnergyLandscape(BuildSpace(), BuildBudget(), lens, &energy, &error)) {
            std::cerr << "Expected inactive lens rows to remain representable in the energy landscape: " << error << "\n";
            return 1;
        }
        if (energy.available_row_count != 1 || energy.peak_path != "fractal.params.ripple_amplitude") {
            std::cerr << "Expected inactive energy rows to be excluded from the available peak summary\n";
            return 1;
        }
        bool foundInactive = false;
        for (const auto& row : energy.rows) {
            if (row.path == "fractal.params.vortex_strength") {
                foundInactive = true;
                if (row.status != SidecarEnergyLandscapeRowStatus::inactive) {
                    std::cerr << "Expected inactive lens rows to stay explicit in the energy profile\n";
                    return 1;
                }
                if (std::isfinite(row.energy)) {
                    std::cerr << "Expected inactive rows to avoid silently exposing a current energy value\n";
                    return 1;
                }
            }
        }
        if (!foundInactive) {
            std::cerr << "Expected inactive vortex row to remain present in the energy profile\n";
            return 1;
        }
    }

    {
        SidecarHypothesisSpace space = BuildSpace();
        space.applicable_parameters[1].cost_hint = -0.25;
        SidecarEnergyLandscape energy;
        std::string error;
        if (BuildSidecarEnergyLandscape(space, BuildBudget(), BuildLens(), &energy, &error)) {
            std::cerr << "Expected invalid sidecar energy cost metadata to fail fast\n";
            return 1;
        }
        if (error.find("cost_hint") == std::string::npos ||
            error.find("fractal.params.vortex_strength") == std::string::npos) {
            std::cerr << "Expected invalid-cost energy failure to mention the broken path\n";
            return 1;
        }
    }

    std::cout << "test_explaino_sidecar_energy: all passed\n";
    return 0;
}
