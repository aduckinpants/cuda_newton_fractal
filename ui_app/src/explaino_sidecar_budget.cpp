#include "explaino_sidecar_budget.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>
#include <utility>

namespace {

constexpr double kMaxPosteriorUncertainty = 1.0;

struct BudgetSeedRow {
    std::string label;
    std::string path;
    std::string type;
    double estimated_information_gain{0.0};
    double decode_stability{1.0};
};

bool BudgetRowOrder(const SidecarBudgetRow& left, const SidecarBudgetRow& right) {
    if (left.estimated_information_gain != right.estimated_information_gain) {
        return left.estimated_information_gain > right.estimated_information_gain;
    }
    if (left.label != right.label) return left.label < right.label;
    return left.path < right.path;
}

bool SeedRowOrder(const BudgetSeedRow& left, const BudgetSeedRow& right) {
    if (left.path != right.path) return left.path < right.path;
    return left.type < right.type;
}

std::string BudgetRowKey(const std::string& path, const std::string& type) {
    return path + "\n" + type;
}

double ComputePosteriorUncertainty(double cumulativeInformationGain) {
    if (!std::isfinite(cumulativeInformationGain) || cumulativeInformationGain <= 0.0) {
        return kMaxPosteriorUncertainty;
    }
    return 1.0 / (1.0 + cumulativeInformationGain);
}

bool CollectSeedRows(const SidecarMeasurementBatch& batch,
    std::vector<BudgetSeedRow>* outRows,
    std::string* outError) {
    if (!outRows) {
        if (outError) *outError = "CollectSeedRows requires outRows";
        return false;
    }

    std::unordered_map<std::string, size_t> seenPaths;
    std::vector<BudgetSeedRow> rows;
    rows.reserve(batch.rows.size());
    for (const SidecarMeasurementRow& measurementRow : batch.rows) {
        const auto [_, inserted] = seenPaths.emplace(measurementRow.path, rows.size());
        if (!inserted) {
            if (outError) *outError = "Sidecar budget update saw duplicate measurement row path: " + measurementRow.path;
            return false;
        }

        BudgetSeedRow row;
        row.label = measurementRow.label;
        row.path = measurementRow.path;
        row.type = measurementRow.type;
        row.estimated_information_gain = measurementRow.information_gain_estimate;
        row.decode_stability = measurementRow.decode_stability;
        rows.push_back(std::move(row));
    }

    std::sort(rows.begin(), rows.end(), SeedRowOrder);
    *outRows = std::move(rows);
    return true;
}

bool SameBudgetSurface(const SidecarBudgetState& previousState,
    const std::string& functionId,
    const std::string& fractalTypeId,
    const std::vector<BudgetSeedRow>& currentRows) {
    if (previousState.function_id != functionId || previousState.fractal_type_id != fractalTypeId) {
        return false;
    }
    if (previousState.rows.size() != currentRows.size()) {
        return false;
    }

    std::vector<SidecarBudgetRow> previousRows = previousState.rows;
    std::sort(previousRows.begin(), previousRows.end(), [](const SidecarBudgetRow& left, const SidecarBudgetRow& right) {
        if (left.path != right.path) return left.path < right.path;
        return left.type < right.type;
    });

    for (size_t index = 0; index < currentRows.size(); ++index) {
        if (previousRows[index].path != currentRows[index].path) return false;
        if (previousRows[index].type != currentRows[index].type) return false;
    }
    return true;
}

} // namespace

bool UpdateSidecarBudgetState(
    const SidecarHypothesisSpace& space,
    const std::string& fractalTypeId,
    const SidecarMeasurementBatch& batch,
    const SidecarBudgetState* previousState,
    SidecarBudgetState* outState,
    std::string* outError) {
    if (!outState) {
        if (outError) *outError = "UpdateSidecarBudgetState requires outState";
        return false;
    }

    std::vector<BudgetSeedRow> currentRows;
    if (!CollectSeedRows(batch, &currentRows, outError)) {
        *outState = {};
        return false;
    }

    const bool accumulate = previousState && SameBudgetSurface(*previousState, space.function_id, fractalTypeId, currentRows);

    std::unordered_map<std::string, SidecarBudgetRow> previousByKey;
    if (accumulate) {
        for (const SidecarBudgetRow& row : previousState->rows) {
            previousByKey.emplace(BudgetRowKey(row.path, row.type), row);
        }
    }

    SidecarBudgetState next;
    next.function_id = space.function_id;
    next.fractal_type_id = fractalTypeId;
    next.batch_count = accumulate ? previousState->batch_count + 1 : 1;
    next.estimated_information_gain_total = batch.total_information_gain_estimate;
    next.mean_posterior_uncertainty = 0.0;
    next.mean_decode_stability = 0.0;

    for (const BudgetSeedRow& currentRow : currentRows) {
        SidecarBudgetRow row;
        row.label = currentRow.label;
        row.path = currentRow.path;
        row.type = currentRow.type;
        row.estimated_information_gain = currentRow.estimated_information_gain;
        row.decode_stability = currentRow.decode_stability;

        const auto previousIt = previousByKey.find(BudgetRowKey(currentRow.path, currentRow.type));
        if (previousIt != previousByKey.end()) {
            row.cumulative_information_gain = previousIt->second.cumulative_information_gain + currentRow.estimated_information_gain;
            row.observation_count = previousIt->second.observation_count + 1;
        } else {
            row.cumulative_information_gain = currentRow.estimated_information_gain;
            row.observation_count = 1;
        }
        row.posterior_uncertainty = ComputePosteriorUncertainty(row.cumulative_information_gain);

        next.cumulative_information_gain_total += row.cumulative_information_gain;
        next.mean_posterior_uncertainty += row.posterior_uncertainty;
        next.mean_decode_stability += row.decode_stability;
        next.rows.push_back(std::move(row));
    }

    if (!next.rows.empty()) {
        const double denom = static_cast<double>(next.rows.size());
        next.mean_posterior_uncertainty /= denom;
        next.mean_decode_stability /= denom;
        std::sort(next.rows.begin(), next.rows.end(), BudgetRowOrder);
    } else {
        next.mean_posterior_uncertainty = kMaxPosteriorUncertainty;
        next.mean_decode_stability = 1.0;
    }

    *outState = std::move(next);
    return true;
}