#pragma once

#include "lens_sdf.h"
#include "sdf_pack.h"
#include "sdf_pack_runtime_types.h"

#include <cstddef>
#include <map>
#include <string>

enum class SdfPackFieldBackend {
    cpu_reference,
    cuda_sample,
    auto_backend,
};

struct SdfPackFieldRegion {
    bool has_region{false};
    double center_x{0.0};
    double center_y{0.0};
    double half_height{1.0};
};

struct SdfPackFieldRequest {
    const SdfPack* pack{nullptr};
    std::map<std::string, double> overrides;
    int width{0};
    int height{0};
    SdfPackFieldRegion region;
};

struct SdfPackRuntimeFieldRequest {
    const SdfPackRuntimeDesc* desc{nullptr};
    std::string pack_id;
    int width{0};
    int height{0};
    SdfPackFieldRegion region;
};

struct SdfPackFieldGeometry {
    double center_x{0.0};
    double center_y{0.0};
    double half_width{1.0};
    double half_height{1.0};
    double pixel_scale{1.0};
    std::size_t sample_count{0};
};

struct SdfPackFieldReport {
    SdfPackFieldBackend requested{SdfPackFieldBackend::cpu_reference};
    SdfPackFieldBackend used{SdfPackFieldBackend::cpu_reference};
    bool fallback_used{false};
    bool direct_grid_evaluation{false};
    std::string pack_id;
    std::string error;
};

const char* SdfPackFieldBackendId(SdfPackFieldBackend backend);

using SdfPackFieldBackendFn = bool (*)(
    const SdfPackFieldRequest& request,
    SdfFieldResult& outField,
    SdfPackFieldReport* outReport,
    std::string* outError);

using SdfPackRuntimeFieldBackendFn = bool (*)(
    const SdfPackRuntimeFieldRequest& request,
    SdfFieldResult& outField,
    SdfPackFieldReport* outReport,
    std::string* outError);

void RegisterSdfPackFieldCudaBackend(SdfPackFieldBackendFn backendFn);
void RegisterSdfPackRuntimeFieldCudaBackend(SdfPackRuntimeFieldBackendFn backendFn);

bool ResolveSdfPackFieldGeometry(
    const SdfPackFieldRequest& request,
    SdfPackFieldGeometry* outGeometry,
    std::string* outError = nullptr);

bool ResolveSdfPackRuntimeFieldGeometry(
    const SdfPackRuntimeFieldRequest& request,
    SdfPackFieldGeometry* outGeometry,
    std::string* outError = nullptr);

bool ComputeSdfPackFieldCpu(
    const SdfPackFieldRequest& request,
    SdfFieldResult& outField,
    SdfPackFieldReport* outReport = nullptr,
    std::string* outError = nullptr);

bool ComputeSdfPackRuntimeFieldCpu(
    const SdfPackRuntimeFieldRequest& request,
    SdfFieldResult& outField,
    SdfPackFieldReport* outReport = nullptr,
    std::string* outError = nullptr);

bool ComputeSdfPackFieldWithBackend(
    const SdfPackFieldRequest& request,
    SdfPackFieldBackend backend,
    SdfFieldResult& outField,
    SdfPackFieldReport* outReport = nullptr,
    std::string* outError = nullptr);

bool ComputeSdfPackRuntimeFieldWithBackend(
    const SdfPackRuntimeFieldRequest& request,
    SdfPackFieldBackend backend,
    SdfFieldResult& outField,
    SdfPackFieldReport* outReport = nullptr,
    std::string* outError = nullptr);
