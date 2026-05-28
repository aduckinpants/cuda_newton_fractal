// Native tests for converting authored SDF packs into the shared SDF field surface.

#include "sdf_pack_field_producer.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <map>
#include <string>

static int g_pass = 0;
static int g_fail = 0;

static void Check(bool cond, const char* msg, int line) {
    if (cond) {
        ++g_pass;
    } else {
        ++g_fail;
        std::cerr << "FAIL line " << line << ": " << msg << "\n";
    }
}

#define CHECK(cond, msg) Check((cond), (msg), __LINE__)

static bool Nearly(double a, double b, double tol = 1.0e-5) {
    const double scale = (std::max)(1.0, (std::max)(std::fabs(a), std::fabs(b)));
    return std::fabs(a - b) <= tol * scale;
}

static const char* kCirclePack = R"json({
  "schema": 1,
  "pack_id": "field_circle",
  "name": "Field Circle",
  "kind": "sdf_scene_2d",
  "params": [
    { "id": "radius", "type": "float", "default": 0.4, "range": [0.1, 1.0] }
  ],
  "region": {
    "center": [0.0, 0.0],
    "half_height": 1.0
  },
  "ast": {
    "op": "circle",
    "center": [0.0, 0.0],
    "radius": { "param": "radius" }
  }
})json";

static SdfPack ParseCirclePack() {
    SdfPackParseResult parsed = ParseSdfPackJson(kCirclePack);
    CHECK(parsed.ok, "circle pack parses");
    if (!parsed.ok) {
        std::cerr << parsed.error << "\n";
    }
    return parsed.pack;
}

static float At(const SdfFieldResult& field, int x, int y) {
    return field.signed_distance_px[static_cast<size_t>(y) * static_cast<size_t>(field.width) + static_cast<size_t>(x)];
}

static void TestCpuFieldUsesPackRegionAndFieldPixelUnits() {
    SdfPack pack = ParseCirclePack();
    SdfPackFieldRequest request{};
    request.pack = &pack;
    request.width = 5;
    request.height = 5;

    SdfFieldResult field;
    SdfPackFieldReport report;
    std::string error;
    const bool ok = ComputeSdfPackFieldCpu(request, field, &report, &error);
    CHECK(ok, "CPU field producer succeeds");
    if (!ok) {
        std::cerr << error << "\n";
        return;
    }

    CHECK(field.width == 5 && field.height == 5, "field dimensions are preserved");
    CHECK(field.source_kind == SdfFieldSourceKind::authored_sdf_pack, "field source identifies authored SDF pack");
    CHECK(field.sign_convention == SdfSignConvention::negative_inside_positive_outside, "field sign convention is preserved");
    CHECK(field.signed_distance_px.size() == 25, "field has one sample per pixel");
    CHECK(Nearly(field.pixel_scale, 0.4), "pixel scale is world units per field pixel");
    CHECK(Nearly(At(field, 2, 2), -1.0), "center world distance is converted to field pixels");
    CHECK(Nearly(At(field, 3, 2), 0.0), "boundary sample lands at zero distance");
    CHECK(Nearly(At(field, 2, 0), 1.0), "outside sample is positive in field pixels");
    CHECK(report.requested == SdfPackFieldBackend::cpu_reference, "CPU report records requested backend");
    CHECK(report.used == SdfPackFieldBackend::cpu_reference, "CPU report records used backend");
    CHECK(report.pack_id == "field_circle", "report records pack id");
}

static void TestExplicitRegionOverridesPackRegion() {
    SdfPack pack = ParseCirclePack();
    SdfPackFieldRequest request{};
    request.pack = &pack;
    request.width = 5;
    request.height = 5;
    request.region.has_region = true;
    request.region.center_x = 1.0;
    request.region.center_y = 0.0;
    request.region.half_height = 1.0;

    SdfFieldResult field;
    const bool ok = ComputeSdfPackFieldCpu(request, field, nullptr, nullptr);
    CHECK(ok, "explicit region field succeeds");
    if (!ok) return;
    CHECK(At(field, 2, 2) > 1.0f, "explicit region moves center sample away from the circle");
}

static void TestParamOverrideChangesProducedField() {
    SdfPack pack = ParseCirclePack();
    SdfPackFieldRequest request{};
    request.pack = &pack;
    request.width = 5;
    request.height = 5;

    SdfFieldResult defaultField;
    CHECK(ComputeSdfPackFieldCpu(request, defaultField, nullptr, nullptr), "default field succeeds");
    request.overrides["radius"] = 0.8;
    SdfFieldResult overrideField;
    CHECK(ComputeSdfPackFieldCpu(request, overrideField, nullptr, nullptr), "override field succeeds");
    CHECK(Nearly(At(defaultField, 2, 2), -1.0), "default center distance is expected");
    CHECK(Nearly(At(overrideField, 2, 2), -2.0), "override center distance changes in field pixels");
}

static void TestDispatcherWorksWithoutCudaRegistration() {
    SdfPack pack = ParseCirclePack();
    SdfPackFieldRequest request{};
    request.pack = &pack;
    request.width = 5;
    request.height = 5;

    SdfFieldResult field;
    SdfPackFieldReport report;
    std::string error;
    CHECK(ComputeSdfPackFieldWithBackend(request, SdfPackFieldBackend::cpu_reference, field, &report, &error), "CPU dispatcher succeeds without CUDA object");
    CHECK(report.requested == SdfPackFieldBackend::cpu_reference, "CPU dispatcher records requested backend");
    CHECK(report.used == SdfPackFieldBackend::cpu_reference, "CPU dispatcher records used backend");
    CHECK(!report.fallback_used, "CPU dispatcher does not report fallback");

    field.Clear();
    error.clear();
    CHECK(ComputeSdfPackFieldWithBackend(request, SdfPackFieldBackend::auto_backend, field, &report, &error), "auto dispatcher falls back without CUDA object");
    CHECK(report.requested == SdfPackFieldBackend::auto_backend, "auto dispatcher records requested backend");
    CHECK(report.used == SdfPackFieldBackend::cpu_reference, "auto dispatcher reports CPU fallback when CUDA object is absent");
    CHECK(report.fallback_used, "auto dispatcher reports fallback when CUDA object is absent");

    field.Clear();
    error.clear();
    CHECK(!ComputeSdfPackFieldWithBackend(request, SdfPackFieldBackend::cuda_sample, field, &report, &error), "explicit CUDA fails closed without CUDA object");
    CHECK(report.requested == SdfPackFieldBackend::cuda_sample, "explicit CUDA failure records requested backend");
    CHECK(error.find("not registered") != std::string::npos, "explicit CUDA failure reports missing backend registration");
}

static void TestInvalidRequestsFailClosed() {
    SdfPack pack = ParseCirclePack();

    SdfPackFieldRequest request{};
    request.pack = nullptr;
    request.width = 5;
    request.height = 5;
    SdfFieldResult field;
    std::string error;
    CHECK(!ComputeSdfPackFieldCpu(request, field, nullptr, &error), "null pack rejects");
    CHECK(error.find("pack") != std::string::npos, "null pack reports pack error");

    request.pack = &pack;
    request.width = 0;
    request.height = 5;
    error.clear();
    CHECK(!ComputeSdfPackFieldCpu(request, field, nullptr, &error), "zero width rejects");
    CHECK(error.find("dimensions") != std::string::npos, "zero width reports dimension error");

    request.width = 5;
    request.region.has_region = true;
    request.region.half_height = std::numeric_limits<double>::quiet_NaN();
    error.clear();
    CHECK(!ComputeSdfPackFieldCpu(request, field, nullptr, &error), "nonfinite region rejects");
    CHECK(error.find("region") != std::string::npos, "nonfinite region reports region error");

    request.region = {};
    request.overrides["radius"] = 2.0;
    error.clear();
    CHECK(!ComputeSdfPackFieldCpu(request, field, nullptr, &error), "out-of-range override rejects");
    CHECK(error.find("range") != std::string::npos, "out-of-range override reports range error");
}

int main() {
    TestCpuFieldUsesPackRegionAndFieldPixelUnits();
    TestExplicitRegionOverridesPackRegion();
    TestParamOverrideChangesProducedField();
    TestDispatcherWorksWithoutCudaRegistration();
    TestInvalidRequestsFailClosed();
    std::cout << "test_sdf_pack_field_producer: pass=" << g_pass << " fail=" << g_fail << "\n";
    return g_fail == 0 ? 0 : 1;
}
