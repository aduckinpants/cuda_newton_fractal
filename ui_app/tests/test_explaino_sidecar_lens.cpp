#include "../src/explaino_sidecar_lens.h"

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
    double maxValue) {
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
    return entry;
}

SidecarMeasurementAggregate MakeAggregate(
    double meanIterations,
    double meanResidual,
    double convergedFraction,
    double escapedFraction) {
    SidecarMeasurementAggregate aggregate;
    aggregate.mean_iterations = meanIterations;
    aggregate.mean_residual = meanResidual;
    aggregate.converged_fraction = convergedFraction;
    aggregate.escaped_fraction = escapedFraction;
    return aggregate;
}

SidecarMeasurementRow MakeMeasurementRow(
    const char* path,
    const char* label,
    double currentValue,
    double stepValue,
    double estimatedInformationGain,
    double decodeStability,
    const SidecarMeasurementAggregate& baseline,
    const SidecarMeasurementAggregate& minusVariant,
    const SidecarMeasurementAggregate& plusVariant) {
    SidecarMeasurementRow row;
    row.path = path;
    row.label = label;
    row.type = "float";
    row.current_value = currentValue;
    row.step_value = stepValue;
    row.information_gain_estimate = estimatedInformationGain;
    row.decode_stability = decodeStability;
    row.baseline = baseline;
    row.minus_variant = minusVariant;
    row.plus_variant = plusVariant;
    return row;
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

SidecarHypothesisSpace BuildSpace() {
    SidecarHypothesisSpace space;
    space.function_id = "fractal.sample";
    space.applicable_parameters.push_back(MakeSurfaceEntry("fractal.params.explaino_mix", "Mix", 0.0, 1.0));
    space.applicable_parameters.push_back(MakeSurfaceEntry("fractal.view.zoom", "Zoom", 1.0, 100.0));
    return space;
}

SidecarHypothesisSpace BuildDuplicateSurfaceSpace() {
    SidecarHypothesisSpace space = BuildSpace();
    space.applicable_parameters.push_back(MakeSurfaceEntry("fractal.params.explaino_mix", "Mix Duplicate", 0.0, 1.0));
    return space;
}

SidecarMeasurementBatch BuildBatch() {
    SidecarMeasurementBatch batch;
    batch.coordinate_count = 5;
    batch.mean_information_gain_estimate = 3.5;
    batch.total_information_gain_estimate = 7.0;
    batch.mean_decode_stability = 0.8;

    const SidecarMeasurementAggregate baseline = MakeAggregate(10.0, 1.0, 0.6, 0.2);
    batch.rows.push_back(MakeMeasurementRow(
        "fractal.params.explaino_mix",
        "Mix",
        0.5,
        0.1,
        5.0,
        0.9,
        baseline,
        MakeAggregate(11.0, 1.1, 0.65, 0.15),
        MakeAggregate(16.0, 1.5, 0.8, 0.05)));
    batch.rows.push_back(MakeMeasurementRow(
        "fractal.view.zoom",
        "Zoom",
        10.0,
        1.0,
        2.0,
        0.75,
        baseline,
        MakeAggregate(15.0, 1.4, 0.75, 0.1),
        MakeAggregate(11.0, 1.1, 0.62, 0.18)));
    return batch;
}

SidecarBudgetState BuildBudget(double posteriorUncertainty) {
    SidecarBudgetState budget;
    budget.function_id = "fractal.sample";
    budget.fractal_type_id = "explaino";
    budget.batch_count = 1;
    budget.estimated_information_gain_total = 7.0;
    budget.cumulative_information_gain_total = 7.0;
    budget.mean_posterior_uncertainty = posteriorUncertainty;
    budget.mean_decode_stability = 0.825;
    budget.rows.push_back(MakeBudgetRow("fractal.params.explaino_mix", "Mix", 5.0, 5.0, posteriorUncertainty, 0.9, 1));
    budget.rows.push_back(MakeBudgetRow("fractal.view.zoom", "Zoom", 2.0, 2.0, posteriorUncertainty, 0.75, 1));
    return budget;
}

} // namespace

int main() {
    {
        SidecarLensProjection projection;
        std::string error;
        if (!BuildSidecarLensProjection(BuildSpace(), BuildBatch(), BuildBudget(0.8), &projection, &error)) {
            std::cerr << "Expected sidecar lens projection to build: " << error << "\n";
            return 1;
        }
        if (projection.function_id != "fractal.sample" || projection.fractal_type_id != "explaino") {
            std::cerr << "Expected sidecar lens projection identity fields to be populated\n";
            return 1;
        }
        if (projection.rows.size() != 2) {
            std::cerr << "Expected lens rows for both measured params\n";
            return 1;
        }
        if (projection.rows[0].path != "fractal.params.explaino_mix") {
            std::cerr << "Expected lens rows to follow ranked budget ordering\n";
            return 1;
        }

        const SidecarLensProjectionRow& mix = projection.rows[0];
        if (!(mix.active_min < mix.current_value && mix.active_max > mix.current_value)) {
            std::cerr << "Expected active zone to straddle the current value for informative params\n";
            return 1;
        }
        if (!(mix.active_max - mix.current_value > mix.current_value - mix.active_min)) {
            std::cerr << "Expected active zone to bias toward the stronger plus-direction measurement delta\n";
            return 1;
        }
        if (mix.guidance.find('+') == std::string::npos) {
            std::cerr << "Expected plus-biased lens guidance for explaino_mix\n";
            return 1;
        }
        if (!(mix.active_fraction > 0.0 && mix.active_fraction <= 1.0)) {
            std::cerr << "Expected bounded active-zone fraction for declared ranges\n";
            return 1;
        }

        const SidecarLensProjectionRow& zoom = projection.rows[1];
        if (!(zoom.current_value - zoom.active_min > zoom.active_max - zoom.current_value)) {
            std::cerr << "Expected zoom active zone to bias toward the stronger minus-direction measurement delta\n";
            return 1;
        }
        if (zoom.guidance.find('-') == std::string::npos) {
            std::cerr << "Expected minus-biased lens guidance for zoom\n";
            return 1;
        }
    }

    {
        SidecarLensProjection wide;
        SidecarLensProjection narrow;
        std::string error;
        if (!BuildSidecarLensProjection(BuildSpace(), BuildBatch(), BuildBudget(0.8), &wide, &error)) {
            std::cerr << "Expected wide-uncertainty lens projection to build: " << error << "\n";
            return 1;
        }
        if (!BuildSidecarLensProjection(BuildSpace(), BuildBatch(), BuildBudget(0.2), &narrow, &error)) {
            std::cerr << "Expected narrow-uncertainty lens projection to build: " << error << "\n";
            return 1;
        }

        const double wideSpan = wide.rows[0].active_max - wide.rows[0].active_min;
        const double narrowSpan = narrow.rows[0].active_max - narrow.rows[0].active_min;
        if (!(wideSpan > narrowSpan)) {
            std::cerr << "Expected higher posterior uncertainty to widen the active zone\n";
            return 1;
        }
    }

    {
        SidecarMeasurementBatch brokenBatch = BuildBatch();
        brokenBatch.rows.push_back(brokenBatch.rows[0]);
        SidecarLensProjection projection;
        std::string error;
        if (BuildSidecarLensProjection(BuildSpace(), brokenBatch, BuildBudget(0.8), &projection, &error)) {
            std::cerr << "Expected duplicate measurement paths to fail lens projection\n";
            return 1;
        }
        if (error.find("duplicate") == std::string::npos) {
            std::cerr << "Expected duplicate measurement error to mention the invariant\n";
            return 1;
        }
    }

    {
        SidecarBudgetState brokenBudget = BuildBudget(0.8);
        brokenBudget.rows.pop_back();
        SidecarLensProjection projection;
        std::string error;
        if (BuildSidecarLensProjection(BuildSpace(), BuildBatch(), brokenBudget, &projection, &error)) {
            std::cerr << "Expected missing budget rows to fail lens projection\n";
            return 1;
        }
        if (error.find("missing budget row") == std::string::npos) {
            std::cerr << "Expected missing-budget error to mention the missing row\n";
            return 1;
        }
    }

    {
        SidecarLensProjection projection;
        std::string error;
        if (BuildSidecarLensProjection(BuildDuplicateSurfaceSpace(), BuildBatch(), BuildBudget(0.8), &projection, &error)) {
            std::cerr << "Expected duplicate surface paths to fail lens projection\n";
            return 1;
        }
        if (error.find("duplicate surface") == std::string::npos) {
            std::cerr << "Expected duplicate-surface error to mention the invariant\n";
            return 1;
        }
    }

    std::cout << "test_explaino_sidecar_lens: all passed\n";
    return 0;
}