#pragma once

#include "explaino_sidecar_measurement.h"

#include <string>
#include <vector>

struct SidecarBudgetRow {
    std::string label;
    std::string path;
    std::string type;
    double estimated_information_gain{0.0};
    double cumulative_information_gain{0.0};
    double posterior_uncertainty{1.0};
    double decode_stability{1.0};
    int observation_count{0};
};

struct SidecarBudgetState {
    std::string function_id;
    std::string fractal_type_id;
    int batch_count{0};
    double estimated_information_gain_total{0.0};
    double cumulative_information_gain_total{0.0};
    double mean_posterior_uncertainty{1.0};
    double mean_decode_stability{1.0};
    std::vector<SidecarBudgetRow> rows;
};

bool UpdateSidecarBudgetState(
    const SidecarHypothesisSpace& space,
    const std::string& fractalTypeId,
    const SidecarMeasurementBatch& batch,
    const SidecarBudgetState* previousState,
    SidecarBudgetState* outState,
    std::string* outError);