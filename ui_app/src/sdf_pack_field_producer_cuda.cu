#include "sdf_pack_field_producer.h"

#include "sdf_pack_cuda.h"

namespace {

void SetError(std::string* outError, SdfPackFieldReport* outReport, const std::string& message) {
    if (outError) {
        *outError = message;
    }
    if (outReport) {
        outReport->error = message;
    }
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

    outField.width = request.width;
    outField.height = request.height;
    outField.pixel_scale = static_cast<float>(geometry.pixel_scale);
    outField.sign_convention = SdfSignConvention::negative_inside_positive_outside;
    outField.source_kind = SdfFieldSourceKind::authored_sdf_pack;
    outField.signed_distance_px.assign(geometry.sample_count, 0.0f);

    const char* cudaError = nullptr;
    const bool cudaOk = SampleSdfPackGridCuda(
        request.width,
        request.height,
        geometry.center_x,
        geometry.center_y,
        geometry.half_width,
        geometry.half_height,
        geometry.pixel_scale,
        lowered.desc,
        outField.signed_distance_px.data(),
        &cudaError);
    if (!cudaOk) {
        outField.Clear();
        SetError(outError, outReport, cudaError ? cudaError : "SampleSdfPackGridCuda failed");
        return false;
    }
    if (outReport) {
        outReport->direct_grid_evaluation = true;
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
