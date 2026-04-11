#include "../src/explaino_sidecar_budget.h"

#include <cmath>
#include <iostream>
#include <string>

namespace {

bool NearlyEqual(double left, double right, double eps = 1.0e-9) {
    const double delta = left - right;
    return delta < eps && delta > -eps;
}

SidecarParamSurfaceEntry MakeSurfaceEntry(const char* path, const char* label) {
    SidecarParamSurfaceEntry entry;
    entry.path = path;
    entry.label = label;
    entry.type = "float";
    return entry;
}

SidecarMeasurementRow MakeMeasurementRow(
    const char* path,
    const char* label,
    double estimatedInformationGain,
    double decodeStability) {
    SidecarMeasurementRow row;
    row.path = path;
    row.label = label;
    row.type = "float";
    row.information_gain_estimate = estimatedInformationGain;
    row.decode_stability = decodeStability;
    return row;
}

SidecarHypothesisSpace BuildSpace() {
    SidecarHypothesisSpace space;
    space.function_id = "fractal.sample";
    space.applicable_parameters.push_back(MakeSurfaceEntry("fractal.params.explaino_mix", "Mix"));
    space.applicable_parameters.push_back(MakeSurfaceEntry("fractal.view.zoom", "Zoom"));
    return space;
}

SidecarMeasurementBatch BuildBatch(double mixEig, double zoomEig) {
    SidecarMeasurementBatch batch;
    batch.coordinate_count = 5;
    batch.rows.push_back(MakeMeasurementRow("fractal.params.explaino_mix", "Mix", mixEig, 0.9));
    batch.rows.push_back(MakeMeasurementRow("fractal.view.zoom", "Zoom", zoomEig, 0.75));
    batch.total_information_gain_estimate = mixEig + zoomEig;
    batch.mean_information_gain_estimate = (mixEig + zoomEig) / 2.0;
    batch.mean_decode_stability = 0.825;
    return batch;
}

SidecarHypothesisSpace BuildEmptySpace() {
    SidecarHypothesisSpace space;
    space.function_id = "fractal.sample";
    return space;
}

SidecarMeasurementBatch BuildEmptyBatch() {
    SidecarMeasurementBatch batch;
    batch.coordinate_count = 5;
    batch.mean_decode_stability = 1.0;
    return batch;
}

} // namespace

int main() {
    {
        SidecarBudgetState state;
        std::string error;
        if (!UpdateSidecarBudgetState(BuildSpace(), "explaino", BuildBatch(5.0, 2.0), nullptr, &state, &error)) {
            std::cerr << "Expected first budget update to succeed: " << error << "\n";
            return 1;
        }
        if (state.function_id != "fractal.sample" || state.fractal_type_id != "explaino") {
            std::cerr << "Expected budget state identity fields to be populated\n";
            return 1;
        }
        if (state.batch_count != 1) {
            std::cerr << "Expected first budget update to start batch_count at 1\n";
            return 1;
        }
        if (state.rows.size() != 2) {
            std::cerr << "Expected budget rows for both measurement params\n";
            return 1;
        }
        if (state.rows[0].path != "fractal.params.explaino_mix") {
            std::cerr << "Expected budget rows to rank by current estimated information gain\n";
            return 1;
        }
        if (state.rows[0].estimated_information_gain != 5.0 || state.rows[0].cumulative_information_gain != 5.0) {
            std::cerr << "Expected first budget update to seed cumulative information gain from the current estimate\n";
            return 1;
        }
        if (state.rows[0].observation_count != 1) {
            std::cerr << "Expected first budget update to start observation counts at 1\n";
            return 1;
        }
        if (!(state.rows[0].posterior_uncertainty > 0.0 && state.rows[0].posterior_uncertainty < 1.0)) {
            std::cerr << "Expected first budget update to reduce posterior uncertainty below 1\n";
            return 1;
        }
        if (state.estimated_information_gain_total != 7.0 || state.cumulative_information_gain_total != 7.0) {
            std::cerr << "Expected budget totals to match the first batch\n";
            return 1;
        }
        if (!NearlyEqual(state.mean_posterior_uncertainty, 0.25)) {
            std::cerr << "Expected first budget update to report the exact mean posterior uncertainty\n";
            return 1;
        }
        if (!NearlyEqual(state.mean_decode_stability, 0.825)) {
            std::cerr << "Expected first budget update to report the exact mean decode stability\n";
            return 1;
        }
    }

    {
        SidecarBudgetState first;
        SidecarBudgetState second;
        std::string error;
        if (!UpdateSidecarBudgetState(BuildSpace(), "explaino", BuildBatch(5.0, 2.0), nullptr, &first, &error)) {
            std::cerr << "Expected first budget update to succeed before accumulation test: " << error << "\n";
            return 1;
        }
        if (!UpdateSidecarBudgetState(BuildSpace(), "explaino", BuildBatch(1.0, 4.0), &first, &second, &error)) {
            std::cerr << "Expected second budget update to succeed: " << error << "\n";
            return 1;
        }
        if (second.batch_count != 2) {
            std::cerr << "Expected budget updates on the same surface to accumulate batch_count\n";
            return 1;
        }
        if (second.rows[0].path != "fractal.view.zoom") {
            std::cerr << "Expected current estimated information gain to control budget row ordering\n";
            return 1;
        }
        if (second.rows[0].cumulative_information_gain != 6.0) {
            std::cerr << "Expected zoom cumulative information gain to add the second batch estimate\n";
            return 1;
        }
        if (second.rows[1].cumulative_information_gain != 6.0) {
            std::cerr << "Expected explaino_mix cumulative information gain to add the second batch estimate\n";
            return 1;
        }
        if (second.rows[0].observation_count != 2 || second.rows[1].observation_count != 2) {
            std::cerr << "Expected observation counts to accumulate on repeated surfaces\n";
            return 1;
        }
        if (!(second.rows[0].posterior_uncertainty < first.rows[1].posterior_uncertainty)) {
            std::cerr << "Expected repeated observations to reduce posterior uncertainty\n";
            return 1;
        }
        if (second.cumulative_information_gain_total != 12.0) {
            std::cerr << "Expected cumulative information budget total to accumulate across batches\n";
            return 1;
        }
        if (!NearlyEqual(second.mean_posterior_uncertainty, 1.0 / 7.0)) {
            std::cerr << "Expected repeated budget updates to report the exact accumulated mean posterior uncertainty\n";
            return 1;
        }
    }

    {
        SidecarBudgetState first;
        SidecarBudgetState reset;
        std::string error;
        if (!UpdateSidecarBudgetState(BuildSpace(), "explaino", BuildBatch(5.0, 2.0), nullptr, &first, &error)) {
            std::cerr << "Expected first budget update to succeed before reset test: " << error << "\n";
            return 1;
        }
        if (!UpdateSidecarBudgetState(BuildSpace(), "mandelbrot", BuildBatch(3.0, 1.0), &first, &reset, &error)) {
            std::cerr << "Expected fractal-type reset update to succeed: " << error << "\n";
            return 1;
        }
        if (reset.batch_count != 1) {
            std::cerr << "Expected fractal-type changes to reset batch_count\n";
            return 1;
        }
        if (reset.cumulative_information_gain_total != 4.0) {
            std::cerr << "Expected fractal-type changes to reset cumulative information gain to the current batch\n";
            return 1;
        }
        if (reset.rows[0].observation_count != 1 || reset.rows[1].observation_count != 1) {
            std::cerr << "Expected fractal-type changes to reset per-param observation counts\n";
            return 1;
        }
    }

    {
        SidecarBudgetState first;
        SidecarBudgetState second;
        std::string error;
        if (!UpdateSidecarBudgetState(BuildEmptySpace(), "explaino", BuildEmptyBatch(), nullptr, &first, &error)) {
            std::cerr << "Expected first empty-surface budget update to succeed: " << error << "\n";
            return 1;
        }
        if (!UpdateSidecarBudgetState(BuildEmptySpace(), "explaino", BuildEmptyBatch(), &first, &second, &error)) {
            std::cerr << "Expected repeated empty-surface budget update to succeed: " << error << "\n";
            return 1;
        }
        if (second.batch_count != 2) {
            std::cerr << "Expected repeated empty-surface budget updates to accumulate batch_count\n";
            return 1;
        }
        if (!second.rows.empty()) {
            std::cerr << "Expected empty-surface budget updates to keep zero budget rows\n";
            return 1;
        }
        if (!NearlyEqual(second.mean_posterior_uncertainty, 1.0) || !NearlyEqual(second.mean_decode_stability, 1.0)) {
            std::cerr << "Expected empty-surface budget updates to keep default mean values\n";
            return 1;
        }
    }

    {
        SidecarBudgetState state;
        std::string error;
        if (!UpdateSidecarBudgetState(BuildSpace(), "explaino", BuildBatch(0.0, 0.0), nullptr, &state, &error)) {
            std::cerr << "Expected zero-information budget update to succeed: " << error << "\n";
            return 1;
        }
        if (state.rows.size() != 2 || state.rows[0].observation_count != 1 || state.rows[1].observation_count != 1) {
            std::cerr << "Expected zero-information measurements to still count as observations\n";
            return 1;
        }
        if (!NearlyEqual(state.mean_posterior_uncertainty, 1.0)) {
            std::cerr << "Expected zero-information measurements to preserve full posterior uncertainty\n";
            return 1;
        }
    }

    {
        SidecarMeasurementBatch broken = BuildBatch(5.0, 2.0);
        broken.rows.push_back(MakeMeasurementRow("fractal.params.explaino_mix", "Mix Duplicate", 1.0, 0.5));
        SidecarBudgetState state;
        std::string error;
        if (UpdateSidecarBudgetState(BuildSpace(), "explaino", broken, nullptr, &state, &error)) {
            std::cerr << "Expected duplicate measurement paths to fail fast\n";
            return 1;
        }
        if (error.find("duplicate") == std::string::npos) {
            std::cerr << "Expected duplicate-path budget error to mention the invariant\n";
            return 1;
        }
    }

    std::cout << "test_explaino_sidecar_budget: all passed\n";
    return 0;
}