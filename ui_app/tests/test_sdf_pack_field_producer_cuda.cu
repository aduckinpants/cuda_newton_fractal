// CUDA parity tests for authored SDF pack field production.

#include "sdf_pack_field_producer.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <map>
#include <string>

static int g_pass = 0;
static int g_fail = 0;

static void Check(bool cond, const char* msg, int line) {
    if (cond) {
        ++g_pass;
    } else {
        ++g_fail;
        std::printf("FAIL line %d: %s\n", line, msg);
    }
}

#define CHECK(cond, msg) Check((cond), (msg), __LINE__)

static bool Nearly(double a, double b, double tol = 1.0e-5) {
    const double scale = (std::max)(1.0, (std::max)(std::fabs(a), std::fabs(b)));
    return std::fabs(a - b) <= tol * scale;
}

static float At(const SdfFieldResult& field, int x, int y) {
    return field.signed_distance_px[static_cast<size_t>(y) * static_cast<size_t>(field.width) + static_cast<size_t>(x)];
}

static int RowOfMinimumDistanceAtX(const SdfFieldResult& field, int x) {
    int bestRow = 0;
    float bestValue = At(field, x, 0);
    for (int y = 1; y < field.height; ++y) {
        const float value = At(field, x, y);
        if (value < bestValue) {
            bestValue = value;
            bestRow = y;
        }
    }
    return bestRow;
}

static const char* kCirclePack = R"json({
  "schema": 1,
  "pack_id": "field_circle_cuda",
  "name": "Field Circle CUDA",
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

static const char* kYOffsetCirclePack = R"json({
  "schema": 1,
  "pack_id": "field_y_offset_circle_cuda",
  "name": "Field Y Offset Circle CUDA",
  "kind": "sdf_scene_2d",
  "params": [],
  "region": {
    "center": [0.0, 0.0],
    "half_height": 1.0
  },
  "ast": {
    "op": "circle",
    "center": [0.0, 0.5],
    "radius": 0.2
  }
})json";

static const char* kSmoothRepeatPack = R"json({
  "schema": 1,
  "pack_id": "field_smooth_repeat_cuda",
  "name": "Field Smooth Repeat CUDA",
  "kind": "sdf_scene_2d",
  "params": [
    { "id": "blend", "type": "float", "default": 0.2, "range": [0.0, 1.0] }
  ],
  "region": {
    "center": [0.0, 0.0],
    "half_height": 1.4
  },
  "ast": {
    "op": "repeat",
    "period": [1.2, 1.0],
    "child": {
      "op": "smooth_union",
      "k": { "param": "blend" },
      "a": { "op": "circle", "center": [-0.2, 0.0], "radius": 0.3 },
      "b": { "op": "capsule", "a": [-0.6, 0.0], "b": [0.6, 0.0], "radius": 0.12 }
    }
  }
})json";

static SdfPack ParsePack(const char* json) {
    SdfPackParseResult parsed = ParseSdfPackJson(json);
    CHECK(parsed.ok, "pack parses");
    if (!parsed.ok) {
        std::printf("parse error: %s\n", parsed.error.c_str());
    }
    return parsed.pack;
}

static void CheckFieldParity(const char* label, const char* packJson, const std::map<std::string, double>& overrides) {
    SdfPack pack = ParsePack(packJson);
    SdfPackFieldRequest request{};
    request.pack = &pack;
    request.width = 17;
    request.height = 11;
    request.overrides = overrides;

    SdfFieldResult cpu;
    SdfFieldResult cudaField;
    SdfPackFieldReport report;
    std::string error;
    CHECK(ComputeSdfPackFieldCpu(request, cpu, nullptr, &error), "CPU field succeeds");
    if (cpu.signed_distance_px.empty()) {
        std::printf("%s cpu error: %s\n", label, error.c_str());
        return;
    }

    error.clear();
    const bool ok = ComputeSdfPackFieldWithBackend(
        request,
        SdfPackFieldBackend::cuda_sample,
        cudaField,
        &report,
        &error);
    CHECK(ok, "CUDA field succeeds");
    if (!ok) {
        std::printf("%s cuda error: %s\n", label, error.c_str());
        return;
    }
    CHECK(report.requested == SdfPackFieldBackend::cuda_sample, "report records requested CUDA backend");
    CHECK(report.used == SdfPackFieldBackend::cuda_sample, "report records used CUDA backend");
    CHECK(!report.fallback_used, "direct CUDA does not report fallback");
    CHECK(cudaField.source_kind == SdfFieldSourceKind::authored_sdf_pack, "CUDA field source identifies authored pack");
    CHECK(cudaField.width == cpu.width && cudaField.height == cpu.height, "CUDA field dimensions match CPU");
    CHECK(Nearly(cudaField.pixel_scale, cpu.pixel_scale), "CUDA field pixel scale matches CPU");
    CHECK(cudaField.signed_distance_px.size() == cpu.signed_distance_px.size(), "CUDA sample count matches CPU");
    for (size_t i = 0; i < cpu.signed_distance_px.size(); ++i) {
        CHECK(Nearly(cudaField.signed_distance_px[i], cpu.signed_distance_px[i]), "CUDA field matches CPU field");
    }
}

static void TestCudaYAxisMatchesViewportDragContract() {
    SdfPack pack = ParsePack(kYOffsetCirclePack);
    SdfPackFieldRequest request{};
    request.pack = &pack;
    request.width = 5;
    request.height = 5;
    request.region.has_region = true;
    request.region.center_x = 0.0;
    request.region.center_y = 0.0;
    request.region.half_height = 1.0;

    SdfFieldResult centeredField;
    std::string error;
    CHECK(ComputeSdfPackFieldWithBackend(
            request,
            SdfPackFieldBackend::cuda_sample,
            centeredField,
            nullptr,
            &error),
        "CUDA y-offset field computes");
    if (centeredField.signed_distance_px.empty()) {
        std::printf("orientation cuda error: %s\n", error.c_str());
        return;
    }

    const int centeredRow = RowOfMinimumDistanceAtX(centeredField, 2);
    CHECK(centeredRow > 2, "CUDA positive world Y appears in the lower screen half like the fractal renderer");

    request.region.center_y = -0.4;
    SdfFieldResult draggedDownField;
    error.clear();
    CHECK(ComputeSdfPackFieldWithBackend(
            request,
            SdfPackFieldBackend::cuda_sample,
            draggedDownField,
            nullptr,
            &error),
        "CUDA dragged-down y-offset field computes");
    if (draggedDownField.signed_distance_px.empty()) return;

    const int draggedDownRow = RowOfMinimumDistanceAtX(draggedDownField, 2);
    CHECK(draggedDownRow > centeredRow,
        "CUDA decreasing center_y, the normal drag-down result, moves authored SDF content down on screen");
}

static void TestAutoBackendPrefersCuda() {
    SdfPack pack = ParsePack(kCirclePack);
    SdfPackFieldRequest request{};
    request.pack = &pack;
    request.width = 9;
    request.height = 9;

    SdfFieldResult field;
    SdfPackFieldReport report;
    std::string error;
    const bool ok = ComputeSdfPackFieldWithBackend(
        request,
        SdfPackFieldBackend::auto_backend,
        field,
        &report,
        &error);
    CHECK(ok, "auto backend succeeds");
    if (!ok) {
        std::printf("auto error: %s\n", error.c_str());
        return;
    }
    CHECK(report.requested == SdfPackFieldBackend::auto_backend, "auto report records requested backend");
    CHECK(report.used == SdfPackFieldBackend::cuda_sample, "auto backend prefers CUDA when available");
    CHECK(!report.fallback_used, "auto CUDA success does not report fallback");
}

static void TestCpuBackendThroughDispatcher() {
    SdfPack pack = ParsePack(kCirclePack);
    SdfPackFieldRequest request{};
    request.pack = &pack;
    request.width = 5;
    request.height = 5;

    SdfFieldResult field;
    SdfPackFieldReport report;
    CHECK(ComputeSdfPackFieldWithBackend(request, SdfPackFieldBackend::cpu_reference, field, &report, nullptr), "dispatcher CPU backend succeeds");
    CHECK(report.requested == SdfPackFieldBackend::cpu_reference, "dispatcher CPU report requested backend");
    CHECK(report.used == SdfPackFieldBackend::cpu_reference, "dispatcher CPU report used backend");
}

int main() {
    CheckFieldParity("circle", kCirclePack, {});
    CheckFieldParity("circle override", kCirclePack, {{"radius", 0.7}});
    CheckFieldParity("smooth repeat", kSmoothRepeatPack, {{"blend", 0.35}});
    TestCudaYAxisMatchesViewportDragContract();
    TestAutoBackendPrefersCuda();
    TestCpuBackendThroughDispatcher();
    std::printf("test_sdf_pack_field_producer_cuda: pass=%d fail=%d\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
