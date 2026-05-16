#include "explaino_sidecar_lens.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace {

constexpr double kGainEpsilon = 1.0e-9;
constexpr double kMinStabilityScale = 0.35;

double ClampUnit(double value) {
    return std::max(0.0, std::min(1.0, value));
}

double ClampToSurface(const SidecarParamSurfaceEntry& entry, double value) {
    if (entry.has_min) value = std::max(value, entry.min_value);
    if (entry.has_max) value = std::min(value, entry.max_value);
    if (entry.type == "int") value = std::round(value);
    return value;
}

double ComputeAggregateDelta(
    const SidecarMeasurementAggregate& baseline,
    const SidecarMeasurementAggregate& variant) {
    double score = 0.0;
    score += std::fabs(variant.mean_iterations - baseline.mean_iterations);
    score += std::log1p(std::fabs(variant.mean_residual - baseline.mean_residual));
    score += 25.0 * std::fabs(variant.converged_fraction - baseline.converged_fraction);
    score += 25.0 * std::fabs(variant.escaped_fraction - baseline.escaped_fraction);
    return score;
}

double ComputeCounterfactualFlowScore(const SidecarCounterfactualWitness& witness) {
    if (witness.coordinate_count <= 0) {
        return 0.0;
    }
    return witness.mean_sample_coord_distance;
}

double ComputeProjectionFlowBias(const SidecarMeasurementRow& row, double* outConfidence) {
    const double minusScore = ComputeCounterfactualFlowScore(row.minus_counterfactual);
    const double plusScore = ComputeCounterfactualFlowScore(row.plus_counterfactual);
    const double totalScore = minusScore + plusScore;
    if (outConfidence) {
        *outConfidence = totalScore <= kGainEpsilon ? 0.0 : ClampUnit(totalScore / (1.0 + totalScore));
    }
    if (!(totalScore > kGainEpsilon)) {
        return 0.0;
    }
    return std::max(-1.0, std::min(1.0, (plusScore - minusScore) / totalScore));
}

bool IsLensNumericType(const std::string& type) {
    return type == "float" || type == "double" || type == "int";
}

std::string DirectionTag(double minusDelta, double plusDelta, double projectionFlowBias) {
    if (plusDelta > minusDelta + kGainEpsilon) return "+";
    if (minusDelta > plusDelta + kGainEpsilon) return "-";
    if (projectionFlowBias > kGainEpsilon) return "+";
    if (projectionFlowBias < -kGainEpsilon) return "-";
    return "=";
}

std::string BuildGuidance(double totalDelta,
    double posteriorUncertainty,
    double decodeStability,
    double minusDelta,
    double plusDelta,
    double projectionFlowBias) {
    if (!(totalDelta > kGainEpsilon)) return "dead zone";

    const std::string direction = DirectionTag(minusDelta, plusDelta, projectionFlowBias);
    if (decodeStability < 0.5) return "unstable " + direction;
    if (posteriorUncertainty >= 0.5) return "explore " + direction;
    return "refine " + direction;
}

} // namespace

bool BuildSidecarLensProjection(
    const SidecarHypothesisSpace& space,
    const SidecarMeasurementBatch& measurement,
    const SidecarBudgetState& budget,
    SidecarLensProjection* outProjection,
    std::string* outError) {
    if (!outProjection) {
        if (outError) *outError = "BuildSidecarLensProjection requires outProjection";
        return false;
    }
    if (budget.function_id != space.function_id) {
        *outProjection = {};
        if (outError) {
            *outError = "Sidecar lens projection budget function_id mismatch: expected " +
                space.function_id + ", got " + budget.function_id;
        }
        return false;
    }

    std::unordered_map<std::string, const SidecarParamSurfaceEntry*> surfaceByPath;
    surfaceByPath.reserve(space.applicable_parameters.size());
    for (const SidecarParamSurfaceEntry& entry : space.applicable_parameters) {
        const auto [_, inserted] = surfaceByPath.emplace(entry.path, &entry);
        if (!inserted) {
            *outProjection = {};
            if (outError) *outError = "Sidecar lens projection saw duplicate surface path: " + entry.path;
            return false;
        }
    }

    std::unordered_map<std::string, const SidecarMeasurementRow*> measurementByPath;
    measurementByPath.reserve(measurement.rows.size());
    for (const SidecarMeasurementRow& row : measurement.rows) {
        const auto [_, inserted] = measurementByPath.emplace(row.path, &row);
        if (!inserted) {
            *outProjection = {};
            if (outError) *outError = "Sidecar lens projection saw duplicate measurement row path: " + row.path;
            return false;
        }
    }

    std::unordered_map<std::string, const SidecarBudgetRow*> budgetByPath;
    budgetByPath.reserve(budget.rows.size());
    for (const SidecarBudgetRow& row : budget.rows) {
        const auto [_, inserted] = budgetByPath.emplace(row.path, &row);
        if (!inserted) {
            *outProjection = {};
            if (outError) *outError = "Sidecar lens projection saw duplicate budget row path: " + row.path;
            return false;
        }
    }

    for (const SidecarMeasurementRow& row : measurement.rows) {
        if (budgetByPath.find(row.path) == budgetByPath.end()) {
            *outProjection = {};
            if (outError) *outError = "Sidecar lens projection missing budget row for path: " + row.path;
            return false;
        }
    }

    SidecarLensProjection next;
    next.function_id = space.function_id;
    next.fractal_type_id = budget.fractal_type_id;

    const double referenceGain = std::max(measurement.mean_information_gain_estimate, kGainEpsilon);
    for (const SidecarBudgetRow& budgetRow : budget.rows) {
        const auto measurementIt = measurementByPath.find(budgetRow.path);
        if (measurementIt == measurementByPath.end()) {
            *outProjection = {};
            if (outError) *outError = "Sidecar lens projection missing budget row measurement path: " + budgetRow.path;
            return false;
        }

        const auto surfaceIt = surfaceByPath.find(budgetRow.path);
        if (surfaceIt == surfaceByPath.end()) {
            *outProjection = {};
            if (outError) *outError = "Sidecar lens projection missing surface entry for path: " + budgetRow.path;
            return false;
        }

        const SidecarMeasurementRow& measurementRow = *measurementIt->second;
        const SidecarParamSurfaceEntry& surface = *surfaceIt->second;
        if (!IsLensNumericType(measurementRow.type) || !IsLensNumericType(surface.type)) {
            *outProjection = {};
            if (outError) *outError = "Unsupported sidecar lens param type: " + budgetRow.path + " (" + measurementRow.type + ")";
            return false;
        }
        if (!(measurementRow.step_value > 0.0) || !std::isfinite(measurementRow.step_value)) {
            *outProjection = {};
            if (outError) *outError = "Invalid sidecar lens step value for path: " + budgetRow.path;
            return false;
        }

        const double minusDelta = ComputeAggregateDelta(measurementRow.baseline, measurementRow.minus_variant);
        const double plusDelta = ComputeAggregateDelta(measurementRow.baseline, measurementRow.plus_variant);
        const double totalDelta = minusDelta + plusDelta;

        SidecarLensProjectionRow row;
        row.label = budgetRow.label;
        row.path = budgetRow.path;
        row.type = budgetRow.type;
        row.current_value = measurementRow.current_value;
        row.information_gradient = measurementRow.information_gradient;
        row.information_curvature = measurementRow.information_curvature;
        row.projection_flow_bias = ComputeProjectionFlowBias(measurementRow, &row.projection_flow_confidence);
        row.posterior_uncertainty = ClampUnit(budgetRow.posterior_uncertainty);
        row.decode_stability = ClampUnit(measurementRow.decode_stability);
        row.guidance = BuildGuidance(
            totalDelta,
            row.posterior_uncertainty,
            row.decode_stability,
            minusDelta,
            plusDelta,
            row.projection_flow_bias);

        if (!(totalDelta > kGainEpsilon)) {
            row.inactive = true;
            row.active_min = measurementRow.current_value;
            row.active_max = measurementRow.current_value;
        } else {
            const double normalizedGain = std::min(2.0, std::max(0.0, measurementRow.information_gain_estimate / referenceGain));
            const double stabilityScale = std::max(kMinStabilityScale, row.decode_stability);
            const double uncertaintyScale = 0.5 + 0.5 * row.posterior_uncertainty;
            const double gainScale = 0.75 + 0.25 * normalizedGain;
            const double baseSpan = measurementRow.step_value * gainScale * uncertaintyScale * stabilityScale;
            const double flowAdjust = 0.25 * row.projection_flow_bias * row.projection_flow_confidence;
            const double minusWeight = std::max(0.25, 0.5 + (minusDelta / totalDelta) - flowAdjust);
            const double plusWeight = std::max(0.25, 0.5 + (plusDelta / totalDelta) + flowAdjust);

            row.active_min = ClampToSurface(surface, measurementRow.current_value - baseSpan * minusWeight);
            row.active_max = ClampToSurface(surface, measurementRow.current_value + baseSpan * plusWeight);
            if (row.active_min > row.active_max) {
                std::swap(row.active_min, row.active_max);
            }
        }

        if (surface.has_declared_span && std::isfinite(surface.declared_span) && surface.declared_span > 0.0) {
            row.active_fraction = ClampUnit((row.active_max - row.active_min) / surface.declared_span);
        }

        next.rows.push_back(std::move(row));
    }

    *outProjection = std::move(next);
    return true;
}
