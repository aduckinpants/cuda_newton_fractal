#include "explaino_sidecar_divergence.h"

#include <algorithm>
#include <cmath>

namespace {

bool ValidateOrientation(
    const SidecarOrientationVector& orientation,
    const char* label,
    std::string* outError) {
    if (!SidecarOrientationHasFiniteValues(orientation)) {
        if (outError) *outError = std::string("Sidecar divergence requires non-finite-free ") + label + " orientation values";
        return false;
    }
    return true;
}

double NormalizedDelta(double previousValue, double currentValue) {
    const double scale = std::max(1.0, std::max(std::fabs(previousValue), std::fabs(currentValue)));
    return std::fabs(currentValue - previousValue) / scale;
}

double ComputeScalarDivergence(
    const SidecarOrientationVector& previousOrientation,
    const SidecarOrientationVector& currentOrientation,
    bool importChanged,
    bool projectionChanged) {
    double scalar = 0.0;
    if (importChanged) {
        scalar += 1.0;
    }
    if (projectionChanged) {
        scalar += 1.0;
    }
    scalar += NormalizedDelta(previousOrientation.field_embedding_stats, currentOrientation.field_embedding_stats);
    scalar += NormalizedDelta(previousOrientation.slime_energy_delta, currentOrientation.slime_energy_delta);
    scalar += NormalizedDelta(previousOrientation.busy_beaver_metrics, currentOrientation.busy_beaver_metrics);
    scalar += NormalizedDelta(previousOrientation.decode_stability, currentOrientation.decode_stability);
    scalar += NormalizedDelta(previousOrientation.diff_magnitude, currentOrientation.diff_magnitude);
    return scalar;
}

} // namespace

bool SidecarOrientationHasFiniteValues(const SidecarOrientationVector& orientation) {
    return std::isfinite(orientation.field_embedding_stats) &&
        std::isfinite(orientation.slime_energy_delta) &&
        std::isfinite(orientation.busy_beaver_metrics) &&
        std::isfinite(orientation.decode_stability) &&
        std::isfinite(orientation.diff_magnitude);
}

bool BuildSidecarStateDivergence(
    const SidecarOrientationVector* previousOrientation,
    const SidecarOrientationVector& currentOrientation,
    SidecarStateDivergence* outDivergence,
    std::string* outError) {
    if (!outDivergence) {
        if (outError) *outError = "BuildSidecarStateDivergence requires outDivergence";
        return false;
    }
    if (!ValidateOrientation(currentOrientation, "current", outError)) {
        *outDivergence = {};
        return false;
    }

    SidecarStateDivergence next;
    if (!previousOrientation) {
        next.status = SidecarStateDivergenceStatus::unavailable;
        next.summary = "unavailable";
        next.reason = "divergence requires previous orientation";
        *outDivergence = std::move(next);
        return true;
    }
    if (!ValidateOrientation(*previousOrientation, "previous", outError)) {
        *outDivergence = {};
        return false;
    }

    next.import_changed = previousOrientation->import_signature != currentOrientation.import_signature;
    next.projection_changed = previousOrientation->pack_projection_hash != currentOrientation.pack_projection_hash;

    if (!next.import_changed && !next.projection_changed) {
        next.status = SidecarStateDivergenceStatus::stable;
        next.summary = "stable";
        next.reason = "no structural state transition detected";
        *outDivergence = std::move(next);
        return true;
    }

    next.status = SidecarStateDivergenceStatus::diverged;
    next.summary = "diverged";
    next.reason = next.import_changed
        ? "fractal_type or combined seed changed"
        : "applicable parameter surface changed";
    next.scalar_divergence = ComputeScalarDivergence(
        *previousOrientation,
        currentOrientation,
        next.import_changed,
        next.projection_changed);
    *outDivergence = std::move(next);
    return true;
}