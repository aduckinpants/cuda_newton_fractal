#include "../src/explaino_sidecar_completeness.h"

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
    const char* type) {
    SidecarParamSurfaceEntry entry;
    entry.path = path;
    entry.label = label;
    entry.type = type;
    return entry;
}

SidecarBudgetRow MakeBudgetRow(
    const char* path,
    const char* label,
    const char* type,
    double posteriorUncertainty,
    int observationCount) {
    SidecarBudgetRow row;
    row.path = path;
    row.label = label;
    row.type = type;
    row.posterior_uncertainty = posteriorUncertainty;
    row.observation_count = observationCount;
    return row;
}

SidecarHypothesisSpace BuildSpace() {
    SidecarHypothesisSpace space;
    space.function_id = "fractal.sample";
    space.applicable_parameters.push_back(MakeSurfaceEntry("fractal.params.explaino_mix", "Mix", "float"));
    space.applicable_parameters.push_back(MakeSurfaceEntry("fractal.view.zoom", "Zoom", "float"));
    space.applicable_parameters.push_back(MakeSurfaceEntry("fractal.view.fractal_type", "Fractal Type", "enum"));
    return space;
}

SidecarBudgetState BuildBudget() {
    SidecarBudgetState budget;
    budget.function_id = "fractal.sample";
    budget.fractal_type_id = "explaino";
    budget.rows.push_back(MakeBudgetRow("fractal.params.explaino_mix", "Mix", "float", 0.2, 3));
    budget.rows.push_back(MakeBudgetRow("fractal.view.zoom", "Zoom", "float", 0.75, 1));
    return budget;
}

} // namespace

int main() {
    {
        SidecarExplorationCompleteness completeness;
        std::string error;
        if (!BuildSidecarExplorationCompleteness(BuildSpace(), BuildBudget(), &completeness, &error)) {
            std::cerr << "Expected sidecar exploration completeness to build: " << error << "\n";
            return 1;
        }
        if (completeness.function_id != "fractal.sample" || completeness.fractal_type_id != "explaino") {
            std::cerr << "Expected completeness identity fields to be populated\n";
            return 1;
        }
        if (completeness.rows.size() != 2) {
            std::cerr << "Expected completeness rows only for numeric tracked params\n";
            return 1;
        }
        if (completeness.demonstrated_count != 1 || completeness.uncertain_count != 1) {
            std::cerr << "Expected completeness buckets to classify one demonstrated and one uncertain param\n";
            return 1;
        }
        if (!NearlyEqual(completeness.demonstrated_fraction, 0.5)) {
            std::cerr << "Expected demonstrated fraction to reflect the exact bucket split\n";
            return 1;
        }
        if (!NearlyEqual(completeness.mean_coverage_score, 0.4625)) {
            std::cerr << "Expected mean completeness coverage score to match the deterministic fixture\n";
            return 1;
        }
        if (completeness.rows[0].path != "fractal.view.zoom" || completeness.rows[0].coverage_bucket != "uncertain") {
            std::cerr << "Expected uncertain params to sort ahead of demonstrated params in completeness output\n";
            return 1;
        }
        if (completeness.rows[1].path != "fractal.params.explaino_mix" || completeness.rows[1].coverage_bucket != "demonstrated") {
            std::cerr << "Expected demonstrated params to retain their bucket annotation\n";
            return 1;
        }
        if (!(completeness.rows[0].coverage_score < completeness.rows[1].coverage_score)) {
            std::cerr << "Expected demonstrated params to carry higher completeness coverage than uncertain params\n";
            return 1;
        }
    }

    {
        SidecarBudgetState broken = BuildBudget();
        broken.rows.pop_back();
        SidecarExplorationCompleteness completeness;
        std::string error;
        if (BuildSidecarExplorationCompleteness(BuildSpace(), broken, &completeness, &error)) {
            std::cerr << "Expected missing numeric budget coverage to fail completeness derivation\n";
            return 1;
        }
        if (error.find("missing budget row") == std::string::npos) {
            std::cerr << "Expected missing-budget completeness failure to mention the invariant\n";
            return 1;
        }
    }

    {
        SidecarBudgetState broken = BuildBudget();
        broken.rows.push_back(broken.rows[0]);
        SidecarExplorationCompleteness completeness;
        std::string error;
        if (BuildSidecarExplorationCompleteness(BuildSpace(), broken, &completeness, &error)) {
            std::cerr << "Expected duplicate budget rows to fail completeness derivation\n";
            return 1;
        }
        if (error.find("duplicate budget row") == std::string::npos) {
            std::cerr << "Expected duplicate-budget completeness failure to mention the invariant\n";
            return 1;
        }
    }

    {
        SidecarBudgetState broken = BuildBudget();
        broken.rows[0].type = "bool";
        SidecarExplorationCompleteness completeness;
        std::string error;
        if (BuildSidecarExplorationCompleteness(BuildSpace(), broken, &completeness, &error)) {
            std::cerr << "Expected type drift to fail completeness derivation\n";
            return 1;
        }
        if (error.find("type mismatch") == std::string::npos) {
            std::cerr << "Expected completeness type-drift failure to mention the mismatch\n";
            return 1;
        }
    }

    {
        SidecarBudgetState broken = BuildBudget();
        broken.rows[0].posterior_uncertainty = 1.25;
        SidecarExplorationCompleteness completeness;
        std::string error;
        if (BuildSidecarExplorationCompleteness(BuildSpace(), broken, &completeness, &error)) {
            std::cerr << "Expected non-unit posterior uncertainty to fail completeness derivation\n";
            return 1;
        }
        if (error.find("posterior_uncertainty") == std::string::npos) {
            std::cerr << "Expected completeness uncertainty failure to mention the bad field\n";
            return 1;
        }
    }

    std::cout << "test_explaino_sidecar_completeness: all passed\n";
    return 0;
}