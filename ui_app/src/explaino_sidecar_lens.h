#pragma once

#include "explaino_sidecar_budget.h"

#include <string>
#include <vector>

struct SidecarLensProjectionRow {
    std::string label;
    std::string path;
    std::string type;
    double current_value{0.0};
    double active_min{0.0};
    double active_max{0.0};
    double active_fraction{0.0};
    double posterior_uncertainty{1.0};
    double decode_stability{1.0};
    bool inactive{false};
    std::string guidance;
};

struct SidecarLensProjection {
    std::string function_id;
    std::string fractal_type_id;
    std::vector<SidecarLensProjectionRow> rows;
};

bool BuildSidecarLensProjection(
    const SidecarHypothesisSpace& space,
    const SidecarMeasurementBatch& measurement,
    const SidecarBudgetState& budget,
    SidecarLensProjection* outProjection,
    std::string* outError);