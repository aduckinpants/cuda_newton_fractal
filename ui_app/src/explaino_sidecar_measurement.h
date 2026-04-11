#pragma once

#include "explaino_sidecar_model.h"
#include "fractal_sample_result.h"

#include <string>
#include <vector>

struct SidecarMeasurementAggregate {
    double mean_iterations{0.0};
    double mean_residual{0.0};
    double converged_fraction{0.0};
    double escaped_fraction{0.0};
};

struct SidecarMeasurementRow {
    std::string label;
    std::string path;
    std::string type;
    double current_value{0.0};
    double step_value{0.0};
    double information_gain_estimate{0.0};
    double decode_stability{1.0};
    SidecarMeasurementAggregate baseline{};
    SidecarMeasurementAggregate minus_variant{};
    SidecarMeasurementAggregate plus_variant{};
};

struct SidecarMeasurementBatch {
    int coordinate_count{0};
    std::vector<SidecarMeasurementRow> rows;
    double total_information_gain_estimate{0.0};
    double mean_information_gain_estimate{0.0};
    double explored_fraction{0.0};
    double mean_decode_stability{1.0};
    double total_diff_magnitude{0.0};
};

class SidecarMeasurementHost {
public:
    virtual ~SidecarMeasurementHost() = default;

    virtual bool Sample(const std::vector<Double2>& coords,
        const ViewState& view,
        const KernelParams& params,
        const RenderSettings& render,
        std::vector<FractalSampleResult>* outResults,
        std::string* outError) const = 0;
};

bool BuildSidecarMeasurementBatch(
    const SidecarHypothesisSpace& space,
    const BindingContext& ctx,
    const SidecarMeasurementHost& host,
    SidecarMeasurementBatch* outBatch,
    std::string* outError);

SidecarOrientationInputs BuildSidecarOrientationInputs(const SidecarMeasurementBatch& batch);