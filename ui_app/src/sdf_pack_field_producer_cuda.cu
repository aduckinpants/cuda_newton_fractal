#include "sdf_pack_field_producer.h"

#include "sdf_pack_cuda.h"

#include <cmath>
#include <vector>

namespace {

void SetError(std::string* outError, SdfPackFieldReport* outReport, const std::string& message) {
    if (outError) {
        *outError = message;
    }
    if (outReport) {
        outReport->error = message;
    }
}

double PixelToWorldX(const SdfPackFieldGeometry& geometry, int x, int width) {
    const double u = ((static_cast<double>(x) + 0.5) / static_cast<double>(width)) * 2.0 - 1.0;
    return geometry.center_x + u * geometry.half_width;
}

double PixelToWorldY(const SdfPackFieldGeometry& geometry, int y, int height) {
    const double v = ((static_cast<double>(y) + 0.5) / static_cast<double>(height) - 0.5) * 2.0;
    return geometry.center_y + v * geometry.half_height;
}

bool ComputeCudaOnly(
    const SdfPackFieldRequest& request,
    SdfFieldResult& outField,
    SdfPackFieldReport* outReport,
    std::string* outError) {
    outField.Clear();
    if (outError) {
        outError->clear();
    }

    SdfPackFieldGeometry geometry;
    std::string error;
    if (!ResolveSdfPackFieldGeometry(request, &geometry, &error)) {
        SetError(outError, outReport, error);
        return false;
    }

    SdfPackLowerResult lowered = LowerSdfPackToRuntimeDesc(*request.pack, request.overrides);
    if (!lowered.ok) {
        SetError(outError, outReport, lowered.error.empty() ? "SDF pack CUDA lower failed" : lowered.error);
        return false;
    }

    std::vector<SdfPackGpuPoint> points(geometry.sample_count);
    for (int y = 0; y < request.height; ++y) {
        const double worldY = PixelToWorldY(geometry, y, request.height);
        for (int x = 0; x < request.width; ++x) {
            points[static_cast<std::size_t>(y) * static_cast<std::size_t>(request.width) + static_cast<std::size_t>(x)] =
                SdfPackGpuPoint{PixelToWorldX(geometry, x, request.width), worldY};
        }
    }

    std::vector<SdfPackGpuSample> samples(geometry.sample_count);
    const char* cudaError = nullptr;
    const bool cudaOk = SampleSdfPackCuda(
        points.empty() ? nullptr : points.data(),
        static_cast<int>(points.size()),
        lowered.desc,
        samples.empty() ? nullptr : samples.data(),
        &cudaError);
    if (!cudaOk) {
        SetError(outError, outReport, cudaError ? cudaError : "SampleSdfPackCuda failed");
        return false;
    }

    outField.width = request.width;
    outField.height = request.height;
    outField.pixel_scale = static_cast<float>(geometry.pixel_scale);
    outField.sign_convention = SdfSignConvention::negative_inside_positive_outside;
    outField.source_kind = SdfFieldSourceKind::authored_sdf_pack;
    outField.signed_distance_px.assign(geometry.sample_count, 0.0f);

    for (std::size_t i = 0; i < samples.size(); ++i) {
        const SdfPackGpuSample& sample = samples[i];
        if (!sample.ok || sample.error_code != SDF_PACK_EVAL_OK) {
            outField.Clear();
            SetError(outError, outReport, "SDF pack CUDA sample returned a per-point error");
            return false;
        }
        if (!std::isfinite(sample.distance)) {
            outField.Clear();
            SetError(outError, outReport, "SDF pack CUDA sample returned nonfinite distance");
            return false;
        }
        const double distancePx = sample.distance / geometry.pixel_scale;
        if (!std::isfinite(distancePx)) {
            outField.Clear();
            SetError(outError, outReport, "SDF pack CUDA sample produced nonfinite field distance");
            return false;
        }
        outField.signed_distance_px[i] = static_cast<float>(distancePx);
    }

    return true;
}

} // namespace

struct SdfPackFieldCudaRegistrar {
    SdfPackFieldCudaRegistrar() {
        RegisterSdfPackFieldCudaBackend(ComputeCudaOnly);
    }
};

SdfPackFieldCudaRegistrar gSdfPackFieldCudaRegistrar;
