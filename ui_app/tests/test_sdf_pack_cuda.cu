// CUDA parity tests for authored SDF pack runtime descriptors.

#include "sdf_pack.h"
#include "sdf_pack_cuda.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <limits>
#include <map>
#include <string>
#include <vector>

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

static bool Nearly(double actual, double expected, double tol = 1.0e-10) {
    const double scale = (std::max)(1.0, (std::max)(std::fabs(actual), std::fabs(expected)));
    return std::fabs(actual - expected) <= tol * scale;
}

static void CheckCudaParity(
    const char* label,
    const char* packJson,
    const std::map<std::string, double>& overrides,
    const std::vector<SdfPackGpuPoint>& points) {
    SdfPackParseResult parsed = ParseSdfPackJson(packJson);
    CHECK(parsed.ok, "SDF pack parses");
    if (!parsed.ok) {
        std::printf("%s parse error: %s\n", label, parsed.error.c_str());
        return;
    }

    SdfPackLowerResult lowered = LowerSdfPackToRuntimeDesc(parsed.pack, overrides);
    CHECK(lowered.ok, "SDF pack lowers to runtime descriptor");
    if (!lowered.ok) {
        std::printf("%s lower error: %s\n", label, lowered.error.c_str());
        return;
    }

    std::vector<SdfPackGpuSample> gpu(points.size());
    const char* error = nullptr;
    const bool cudaOk = SampleSdfPackCuda(
        points.data(),
        static_cast<int>(points.size()),
        lowered.desc,
        gpu.data(),
        &error);
    CHECK(cudaOk, "SampleSdfPackCuda succeeds");
    if (!cudaOk) {
        std::printf("%s cuda error: %s\n", label, error ? error : "<none>");
        return;
    }

    for (size_t i = 0; i < points.size(); ++i) {
        SdfPackSampleResult cpu = SampleSdfPackCpu(parsed.pack, points[i].x, points[i].y, overrides);
        CHECK(cpu.ok, "CPU reference sample succeeds");
        CHECK(gpu[i].ok, "CUDA sample reports ok");
        CHECK(gpu[i].error_code == 0, "CUDA sample has no per-point error");
        CHECK(Nearly(gpu[i].distance, cpu.distance), "CUDA/CPU distance parity");
    }
}

static void TestPrimitiveAndCombinatorParity() {
    const char* packJson = R"json({
      "schema": 1,
      "pack_id": "cuda_primitive_combinator",
      "name": "CUDA Primitive Combinator",
      "kind": "sdf_scene_2d",
      "params": [],
      "ast": {
        "op": "subtract",
        "a": {
          "op": "union",
          "a": { "op": "box", "center": [0.0, 0.0], "half_size": [0.7, 0.25] },
          "b": { "op": "circle", "center": [0.45, 0.0], "radius": 0.35 }
        },
        "b": { "op": "capsule", "a": [-0.9, 0.0], "b": [0.9, 0.0], "radius": 0.08 }
      }
    })json";
    CheckCudaParity(
        "primitive combinator",
        packJson,
        {},
        {{0.0, 0.0}, {0.7, 0.0}, {-1.1, 0.0}, {0.2, 0.55}, {1.2, -0.3}});
}

static void TestTransformAndParameterParity() {
    const char* packJson = R"json({
      "schema": 1,
      "pack_id": "cuda_transform_param",
      "name": "CUDA Transform Param",
      "kind": "sdf_scene_2d",
      "params": [
        { "id": "radius", "type": "float", "default": 0.25, "range": [0.05, 1.0] },
        { "id": "angle", "type": "float", "default": 0.25, "range": [-3.14159, 3.14159] },
        { "id": "scale", "type": "float", "default": 1.25, "range": [0.25, 4.0] }
      ],
      "ast": {
        "op": "scale",
        "factor": { "param": "scale" },
        "child": {
          "op": "rotate",
          "angle": { "op": "param", "name": "angle" },
          "child": {
            "op": "translate",
            "offset": [0.2, -0.1],
            "child": { "op": "circle", "center": [0.0, 0.0], "radius": { "param": "radius" } }
          }
        }
      }
    })json";
    CheckCudaParity(
        "transform param default",
        packJson,
        {},
        {{0.0, 0.0}, {0.35, 0.15}, {-0.7, 0.4}, {0.9, -0.2}});
    CheckCudaParity(
        "transform param override",
        packJson,
        {{"radius", 0.55}, {"angle", -0.6}, {"scale", 2.0}},
        {{0.0, 0.0}, {0.35, 0.15}, {-0.7, 0.4}, {0.9, -0.2}});
}

static void TestSmoothRepeatNestedParity() {
    const char* packJson = R"json({
      "schema": 1,
      "pack_id": "cuda_smooth_repeat_nested",
      "name": "CUDA Smooth Repeat Nested",
      "kind": "sdf_scene_2d",
      "params": [
        { "id": "blend", "type": "float", "default": 0.2, "range": [0.0, 1.0] }
      ],
      "ast": {
        "op": "repeat",
        "period": [1.4, 1.1],
        "child": {
          "op": "smooth_union",
          "k": { "param": "blend" },
          "a": { "op": "circle", "center": [-0.25, 0.0], "radius": 0.35 },
          "b": {
            "op": "intersect",
            "a": { "op": "box", "center": [0.25, 0.0], "half_size": [0.35, 0.18] },
            "b": { "op": "circle", "center": [0.25, 0.0], "radius": 0.42 }
          }
        }
      }
    })json";
    CheckCudaParity(
        "smooth repeat nested",
        packJson,
        {{"blend", 0.35}},
        {{0.0, 0.0}, {1.45, 0.0}, {-1.4, 1.2}, {0.8, -0.9}, {2.2, 1.7}});
}

static void TestBoundedPerformanceWitness() {
    const char* packJson = R"json({
      "schema": 1,
      "pack_id": "cuda_perf_witness",
      "name": "CUDA Perf Witness",
      "kind": "sdf_scene_2d",
      "params": [
        { "id": "blend", "type": "float", "default": 0.18, "range": [0.0, 1.0] }
      ],
      "ast": {
        "op": "repeat",
        "period": [0.75, 0.75],
        "child": {
          "op": "smooth_union",
          "k": { "param": "blend" },
          "a": { "op": "circle", "center": [-0.12, 0.0], "radius": 0.18 },
          "b": { "op": "box", "center": [0.16, 0.0], "half_size": [0.16, 0.1] }
        }
      }
    })json";
    SdfPackParseResult parsed = ParseSdfPackJson(packJson);
    CHECK(parsed.ok, "performance pack parses");
    if (!parsed.ok) return;
    SdfPackLowerResult lowered = LowerSdfPackToRuntimeDesc(parsed.pack, {});
    CHECK(lowered.ok, "performance pack lowers");
    if (!lowered.ok) return;

    constexpr int width = 160;
    constexpr int height = 120;
    std::vector<SdfPackGpuPoint> points;
    points.reserve(width * height);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            points.push_back({
                -2.0 + 4.0 * static_cast<double>(x) / static_cast<double>(width - 1),
                -1.5 + 3.0 * static_cast<double>(y) / static_cast<double>(height - 1),
            });
        }
    }
    std::vector<SdfPackGpuSample> samples(points.size());
    const char* error = nullptr;
    const auto start = std::chrono::steady_clock::now();
    const bool ok = SampleSdfPackCuda(points.data(), static_cast<int>(points.size()), lowered.desc, samples.data(), &error);
    const auto finish = std::chrono::steady_clock::now();
    CHECK(ok, "performance CUDA sample succeeds");
    if (!ok) {
        std::printf("performance cuda error: %s\n", error ? error : "<none>");
        return;
    }
    double checksum = 0.0;
    for (const SdfPackGpuSample& sample : samples) {
        CHECK(sample.ok, "performance sample has ok flag");
        checksum += sample.distance;
    }
    const double elapsedMs = std::chrono::duration<double, std::milli>(finish - start).count();
    CHECK(std::isfinite(checksum), "performance checksum is finite");
    CHECK(elapsedMs < 5000.0, "bounded performance witness completes within a loose native-test budget");
    std::printf("test_sdf_pack_cuda performance: points=%d elapsed_ms=%.3f checksum=%.9f\n",
        width * height,
        elapsedMs,
        checksum);
}

static void TestInvalidDescriptorFailsFast() {
    SdfPackRuntimeDesc desc{};
    desc.node_count = 1;
    desc.root_node = 0;
    desc.nodes[0].op = SdfPackNodeOp::union_op;
    desc.nodes[0].child_a = -1;
    desc.nodes[0].child_b = -1;

    SdfPackGpuPoint point{0.0, 0.0};
    SdfPackGpuSample sample{};
    const char* error = nullptr;
    const bool ok = SampleSdfPackCuda(&point, 1, desc, &sample, &error);
    CHECK(!ok, "invalid runtime descriptor fails before CUDA sampling");
    CHECK(error != nullptr, "invalid runtime descriptor reports an error");

    SdfPackRuntimeDesc nonfinite{};
    nonfinite.node_count = 1;
    nonfinite.root_node = 0;
    nonfinite.nodes[0].op = SdfPackNodeOp::circle;
    nonfinite.nodes[0].radius.kind = SdfPackScalarKind::constant;
    nonfinite.nodes[0].radius.value = std::numeric_limits<double>::quiet_NaN();
    error = nullptr;
    const bool finiteOk = SampleSdfPackCuda(&point, 1, nonfinite, &sample, &error);
    CHECK(!finiteOk, "nonfinite runtime descriptor fails before CUDA sampling");
    CHECK(error != nullptr, "nonfinite runtime descriptor reports an error");
}

int main() {
    TestPrimitiveAndCombinatorParity();
    TestTransformAndParameterParity();
    TestSmoothRepeatNestedParity();
    TestBoundedPerformanceWitness();
    TestInvalidDescriptorFailsFast();
    std::printf("test_sdf_pack_cuda: pass=%d fail=%d\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
