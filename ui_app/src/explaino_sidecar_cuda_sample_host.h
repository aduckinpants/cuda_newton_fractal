#pragma once

#include "explaino_sidecar_measurement.h"

class CudaSidecarMeasurementHost final : public SidecarMeasurementHost {
public:
    bool Sample(const std::vector<Double2>& coords,
        const ViewState& view,
        const KernelParams& params,
        const RenderSettings& render,
        std::vector<FractalSampleResult>* outResults,
        std::string* outError) const override;
};