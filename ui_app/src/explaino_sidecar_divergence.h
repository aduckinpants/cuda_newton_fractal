#pragma once

#include "explaino_sidecar_model.h"

#include <string>

enum class SidecarStateDivergenceStatus {
    unavailable,
    stable,
    diverged,
};

struct SidecarStateDivergence {
    SidecarStateDivergenceStatus status{SidecarStateDivergenceStatus::unavailable};
    std::string summary;
    std::string reason;
    double scalar_divergence{0.0};
    bool import_changed{false};
    bool projection_changed{false};
};

bool SidecarOrientationHasFiniteValues(const SidecarOrientationVector& orientation);

bool BuildSidecarStateDivergence(
    const SidecarOrientationVector* previousOrientation,
    const SidecarOrientationVector& currentOrientation,
    SidecarStateDivergence* outDivergence,
    std::string* outError);