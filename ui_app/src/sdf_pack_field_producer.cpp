#include "sdf_pack_field_producer.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace {

constexpr std::size_t kMaxFieldSamples = 64ull * 1024ull * 1024ull;

SdfPackFieldBackendFn& RegisteredCudaBackend() {
    static SdfPackFieldBackendFn backend = nullptr;
    return backend;
}

void SetError(std::string* outError, SdfPackFieldReport* outReport, const std::string& message) {
    if (outError) {
        *outError = message;
    }
    if (outReport) {
        outReport->error = message;
    }
}

void ResetReport(
    SdfPackFieldReport* outReport,
    SdfPackFieldBackend requested,
    SdfPackFieldBackend used,
    const SdfPack* pack) {
    if (!outReport) return;
    *outReport = {};
    outReport->requested = requested;
    outReport->used = used;
    outReport->pack_id = pack ? pack->pack_id : std::string{};
}

double PixelToWorldX(const SdfPackFieldGeometry& geometry, int x, int width) {
    const double u = ((static_cast<double>(x) + 0.5) / static_cast<double>(width)) * 2.0 - 1.0;
    return geometry.center_x + u * geometry.half_width;
}

double PixelToWorldY(const SdfPackFieldGeometry& geometry, int y, int height) {
    const double v = ((static_cast<double>(y) + 0.5) / static_cast<double>(height) - 0.5) * 2.0;
    return geometry.center_y + v * geometry.half_height;
}

} // namespace

const char* SdfPackFieldBackendId(SdfPackFieldBackend backend) {
    switch (backend) {
    case SdfPackFieldBackend::cpu_reference: return "cpu_reference";
    case SdfPackFieldBackend::cuda_sample: return "cuda_sample";
    case SdfPackFieldBackend::auto_backend: return "auto";
    default: return "cpu_reference";
    }
}

void RegisterSdfPackFieldCudaBackend(SdfPackFieldBackendFn backendFn) {
    RegisteredCudaBackend() = backendFn;
}

bool ResolveSdfPackFieldGeometry(
    const SdfPackFieldRequest& request,
    SdfPackFieldGeometry* outGeometry,
    std::string* outError) {
    if (!request.pack) {
        if (outError) *outError = "SDF pack field request requires a pack";
        return false;
    }
    if (request.width <= 0 || request.height <= 0) {
        if (outError) *outError = "SDF pack field dimensions must be positive";
        return false;
    }
    const std::size_t sampleCount =
        static_cast<std::size_t>(request.width) * static_cast<std::size_t>(request.height);
    if (sampleCount == 0 || sampleCount > kMaxFieldSamples ||
        sampleCount > static_cast<std::size_t>((std::numeric_limits<int>::max)())) {
        if (outError) *outError = "SDF pack field dimensions are too large";
        return false;
    }

    SdfPackFieldRegion region = request.region;
    if (!region.has_region) {
        region.has_region = true;
        if (request.pack->region.has_region) {
            region.center_x = request.pack->region.center_x;
            region.center_y = request.pack->region.center_y;
            region.half_height = request.pack->region.half_height;
        } else {
            region.center_x = 0.0;
            region.center_y = 0.0;
            region.half_height = 1.0;
        }
    }

    if (!std::isfinite(region.center_x) || !std::isfinite(region.center_y) ||
        !std::isfinite(region.half_height) || region.half_height <= 0.0) {
        if (outError) *outError = "SDF pack field region must be finite with positive half_height";
        return false;
    }

    const double aspect = static_cast<double>(request.width) / static_cast<double>(request.height);
    const double halfWidth = region.half_height * aspect;
    const double pixelScale = (2.0 * region.half_height) / static_cast<double>(request.height);
    if (!std::isfinite(halfWidth) || !std::isfinite(pixelScale) || pixelScale <= 0.0) {
        if (outError) *outError = "SDF pack field region produced invalid pixel scale";
        return false;
    }

    if (outGeometry) {
        outGeometry->center_x = region.center_x;
        outGeometry->center_y = region.center_y;
        outGeometry->half_width = halfWidth;
        outGeometry->half_height = region.half_height;
        outGeometry->pixel_scale = pixelScale;
        outGeometry->sample_count = sampleCount;
    }
    return true;
}

bool ComputeSdfPackFieldCpu(
    const SdfPackFieldRequest& request,
    SdfFieldResult& outField,
    SdfPackFieldReport* outReport,
    std::string* outError) {
    ResetReport(outReport, SdfPackFieldBackend::cpu_reference, SdfPackFieldBackend::cpu_reference, request.pack);
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

    outField.width = request.width;
    outField.height = request.height;
    outField.pixel_scale = static_cast<float>(geometry.pixel_scale);
    outField.sign_convention = SdfSignConvention::negative_inside_positive_outside;
    outField.source_kind = SdfFieldSourceKind::authored_sdf_pack;
    outField.signed_distance_px.assign(geometry.sample_count, 0.0f);

    for (int y = 0; y < request.height; ++y) {
        const double worldY = PixelToWorldY(geometry, y, request.height);
        for (int x = 0; x < request.width; ++x) {
            const double worldX = PixelToWorldX(geometry, x, request.width);
            SdfPackSampleResult sample = SampleSdfPackCpu(*request.pack, worldX, worldY, request.overrides);
            if (!sample.ok) {
                outField.Clear();
                SetError(outError, outReport, sample.error.empty() ? "SDF pack CPU sample failed" : sample.error);
                return false;
            }
            if (!std::isfinite(sample.distance)) {
                outField.Clear();
                SetError(outError, outReport, "SDF pack CPU sample returned nonfinite distance");
                return false;
            }
            const double distancePx = sample.distance / geometry.pixel_scale;
            if (!std::isfinite(distancePx)) {
                outField.Clear();
                SetError(outError, outReport, "SDF pack CPU sample produced nonfinite field distance");
                return false;
            }
            outField.signed_distance_px[
                static_cast<std::size_t>(y) * static_cast<std::size_t>(request.width) + static_cast<std::size_t>(x)] =
                static_cast<float>(distancePx);
        }
    }

    return true;
}

bool ComputeSdfPackFieldWithBackend(
    const SdfPackFieldRequest& request,
    SdfPackFieldBackend backend,
    SdfFieldResult& outField,
    SdfPackFieldReport* outReport,
    std::string* outError) {
    if (backend == SdfPackFieldBackend::cpu_reference) {
        const bool ok = ComputeSdfPackFieldCpu(request, outField, outReport, outError);
        if (outReport) {
            outReport->requested = SdfPackFieldBackend::cpu_reference;
            outReport->used = SdfPackFieldBackend::cpu_reference;
            outReport->fallback_used = false;
        }
        return ok;
    }

    ResetReport(outReport, backend, SdfPackFieldBackend::cuda_sample, request.pack);
    if (outError) {
        outError->clear();
    }

    std::string cudaError;
    SdfPackFieldBackendFn cudaBackend = RegisteredCudaBackend();
    if (cudaBackend) {
        const bool cudaOk = cudaBackend(request, outField, outReport, &cudaError);
        if (cudaOk) {
            if (outReport) {
                outReport->requested = backend;
                outReport->used = SdfPackFieldBackend::cuda_sample;
                outReport->fallback_used = false;
                outReport->error.clear();
            }
            return true;
        }
    } else {
        cudaError = "SDF pack CUDA backend is not registered";
    }

    if (backend == SdfPackFieldBackend::cuda_sample) {
        SetError(outError, outReport, cudaError);
        return false;
    }

    SdfPackFieldReport cpuReport;
    std::string cpuError;
    const bool cpuOk = ComputeSdfPackFieldCpu(request, outField, &cpuReport, &cpuError);
    if (!cpuOk) {
        SetError(outError, outReport, "CUDA failed: " + cudaError + "; CPU fallback failed: " + cpuError);
        return false;
    }

    if (outReport) {
        outReport->requested = SdfPackFieldBackend::auto_backend;
        outReport->used = SdfPackFieldBackend::cpu_reference;
        outReport->fallback_used = true;
        outReport->pack_id = cpuReport.pack_id;
        outReport->error.clear();
    }
    if (outError) {
        outError->clear();
    }
    return true;
}
