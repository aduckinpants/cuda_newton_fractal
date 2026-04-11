#include "explaino_sidecar_completeness.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace {

constexpr int kDemonstratedObservationCount = 2;
constexpr double kDemonstratedUncertaintyThreshold = 0.5;

bool IsCompletenessNumericType(const std::string& type) {
    return type == "float" || type == "double" || type == "int";
}

double ClampUnit(double value) {
    return std::max(0.0, std::min(1.0, value));
}

double ComputeCoverageScore(double posteriorUncertainty, int observationCount) {
    const double uncertaintyScore = 1.0 - posteriorUncertainty;
    const double observationScore = std::min(1.0, static_cast<double>(observationCount) /
        static_cast<double>(kDemonstratedObservationCount));
    return ClampUnit(uncertaintyScore * observationScore);
}

bool IsDemonstrated(double posteriorUncertainty, int observationCount) {
    return observationCount >= kDemonstratedObservationCount &&
        posteriorUncertainty < kDemonstratedUncertaintyThreshold;
}

bool CompletenessRowOrder(const SidecarExplorationCompletenessRow& left,
    const SidecarExplorationCompletenessRow& right) {
    if (left.demonstrated != right.demonstrated) {
        return !left.demonstrated && right.demonstrated;
    }
    if (left.posterior_uncertainty != right.posterior_uncertainty) {
        return left.posterior_uncertainty > right.posterior_uncertainty;
    }
    if (left.observation_count != right.observation_count) {
        return left.observation_count < right.observation_count;
    }
    if (left.label != right.label) {
        return left.label < right.label;
    }
    return left.path < right.path;
}

} // namespace

bool BuildSidecarExplorationCompleteness(
    const SidecarHypothesisSpace& space,
    const SidecarBudgetState& budget,
    SidecarExplorationCompleteness* outCompleteness,
    std::string* outError) {
    if (!outCompleteness) {
        if (outError) *outError = "BuildSidecarExplorationCompleteness requires outCompleteness";
        return false;
    }
    if (budget.function_id != space.function_id) {
        *outCompleteness = {};
        if (outError) {
            *outError = "Sidecar exploration completeness budget function_id mismatch: expected " +
                space.function_id + ", got " + budget.function_id;
        }
        return false;
    }

    std::unordered_map<std::string, const SidecarParamSurfaceEntry*> surfaceByPath;
    surfaceByPath.reserve(space.applicable_parameters.size());
    for (const SidecarParamSurfaceEntry& entry : space.applicable_parameters) {
        if (!IsCompletenessNumericType(entry.type)) {
            continue;
        }
        const auto [_, inserted] = surfaceByPath.emplace(entry.path, &entry);
        if (!inserted) {
            *outCompleteness = {};
            if (outError) *outError = "Sidecar exploration completeness saw duplicate surface path: " + entry.path;
            return false;
        }
    }

    std::unordered_map<std::string, const SidecarBudgetRow*> budgetByPath;
    budgetByPath.reserve(budget.rows.size());
    for (const SidecarBudgetRow& row : budget.rows) {
        const auto [_, inserted] = budgetByPath.emplace(row.path, &row);
        if (!inserted) {
            *outCompleteness = {};
            if (outError) *outError = "Sidecar exploration completeness saw duplicate budget row path: " + row.path;
            return false;
        }
    }

    for (const auto& [path, surface] : surfaceByPath) {
        const auto budgetIt = budgetByPath.find(path);
        if (budgetIt == budgetByPath.end()) {
            *outCompleteness = {};
            if (outError) *outError = "Sidecar exploration completeness missing budget row for path: " + path;
            return false;
        }
        if (budgetIt->second->type != surface->type) {
            *outCompleteness = {};
            if (outError) {
                *outError = "Sidecar exploration completeness type mismatch for path: " + path +
                    " (surface=" + surface->type + ", budget=" + budgetIt->second->type + ")";
            }
            return false;
        }
    }

    SidecarExplorationCompleteness next;
    next.function_id = space.function_id;
    next.fractal_type_id = budget.fractal_type_id;
    next.demonstrated_fraction = 1.0;
    next.mean_coverage_score = 1.0;

    double totalCoverageScore = 0.0;
    for (const SidecarBudgetRow& budgetRow : budget.rows) {
        const auto surfaceIt = surfaceByPath.find(budgetRow.path);
        if (surfaceIt == surfaceByPath.end()) {
            *outCompleteness = {};
            if (outError) *outError = "Sidecar exploration completeness missing surface entry for path: " + budgetRow.path;
            return false;
        }
        if (!std::isfinite(budgetRow.posterior_uncertainty) ||
            budgetRow.posterior_uncertainty < 0.0 || budgetRow.posterior_uncertainty > 1.0) {
            *outCompleteness = {};
            if (outError) {
                *outError = "Invalid sidecar exploration completeness posterior_uncertainty for path: " + budgetRow.path;
            }
            return false;
        }
        if (budgetRow.observation_count < 0) {
            *outCompleteness = {};
            if (outError) {
                *outError = "Invalid sidecar exploration completeness observation_count for path: " + budgetRow.path;
            }
            return false;
        }

        SidecarExplorationCompletenessRow row;
        row.label = budgetRow.label;
        row.path = budgetRow.path;
        row.type = budgetRow.type;
        row.posterior_uncertainty = budgetRow.posterior_uncertainty;
        row.observation_count = budgetRow.observation_count;
        row.coverage_score = ComputeCoverageScore(row.posterior_uncertainty, row.observation_count);
        row.demonstrated = IsDemonstrated(row.posterior_uncertainty, row.observation_count);
        row.coverage_bucket = row.demonstrated ? "demonstrated" : "uncertain";

        if (row.demonstrated) {
            ++next.demonstrated_count;
        } else {
            ++next.uncertain_count;
        }
        totalCoverageScore += row.coverage_score;
        next.rows.push_back(std::move(row));
    }

    if (!next.rows.empty()) {
        const double denom = static_cast<double>(next.rows.size());
        next.demonstrated_fraction = static_cast<double>(next.demonstrated_count) / denom;
        next.mean_coverage_score = totalCoverageScore / denom;
        std::sort(next.rows.begin(), next.rows.end(), CompletenessRowOrder);
    }

    *outCompleteness = std::move(next);
    return true;
}