#include "explaino_sidecar_cuda_sample_host.h"

#include "fractal_types.h"

#include <limits>
#include <utility>

bool CudaSidecarMeasurementHost::SupportsWidenedEvidence() const {
    return true;
}

bool CudaSidecarMeasurementHost::Sample(const std::vector<Double2>& coords,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    std::vector<FractalSampleResult>* outResults,
    std::string* outError) const {
    if (!outResults) {
        if (outError) *outError = "CudaSidecarMeasurementHost requires outResults";
        return false;
    }

    if (coords.empty()) {
        outResults->clear();
        return true;
    }

    if (coords.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
        if (outError) *outError = "Sidecar measurement coord count exceeds SampleFractalPoints limit";
        return false;
    }

    std::vector<FractalSampleResult> results(coords.size());
    const char* sampleError = nullptr;
    if (!SampleFractalPoints(coords.data(),
            static_cast<int>(coords.size()),
            view,
            params,
            render,
            results.data(),
            &sampleError)) {
        if (outError) {
            *outError = sampleError ? sampleError : "SampleFractalPoints failed for sidecar measurement";
        }
        return false;
    }

    *outResults = std::move(results);
    return true;
}

bool CudaSidecarMeasurementHost::SampleEvidence(const std::vector<Double2>& coords,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    std::vector<FractalSampleEvidence>* outEvidence,
    std::string* outError) const {
    if (!outEvidence) {
        if (outError) *outError = "CudaSidecarMeasurementHost requires outEvidence";
        return false;
    }

    if (coords.empty()) {
        outEvidence->clear();
        return true;
    }

    if (coords.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
        if (outError) *outError = "Sidecar measurement coord count exceeds SampleFractalEvidencePoints limit";
        return false;
    }

    std::vector<FractalSampleEvidence> evidence(coords.size());
    const char* sampleError = nullptr;
    if (!SampleFractalEvidencePoints(coords.data(),
            static_cast<int>(coords.size()),
            view,
            params,
            render,
            evidence.data(),
            &sampleError)) {
        if (outError) {
            *outError = sampleError ? sampleError : "SampleFractalEvidencePoints failed for sidecar measurement";
        }
        return false;
    }

    *outEvidence = std::move(evidence);
    return true;
}
