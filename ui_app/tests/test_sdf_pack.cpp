// Native tests for the authored SDF pack parser and CPU reference evaluator.

#include "sdf_pack.h"

#include <cmath>
#include <iostream>
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

static bool Nearly(double a, double b, double tol = 1.0e-6) {
    const double scale = (std::max)(1.0, (std::max)(std::fabs(a), std::fabs(b)));
    return std::fabs(a - b) <= tol * scale;
}

static const char* kSmoothUnionPack = R"json({
  "schema": 1,
  "pack_id": "sdf_smooth_capsule_union_demo",
  "name": "Smooth Capsule Union Demo",
  "kind": "sdf_scene_2d",
  "params": [
    { "id": "blend", "type": "float", "default": 0.15, "range": [0.0, 2.0] }
  ],
  "controls": [
    { "param": "blend", "label": "Blend", "ui_min": 0.0, "ui_max": 1.0 }
  ],
  "region": {
    "center": [0.0, 0.0],
    "half_height": 1.5
  },
  "ast": {
    "op": "smooth_union",
    "k": { "param": "blend" },
    "a": { "op": "circle", "center": [0.0, 0.0], "radius": 0.5 },
    "b": { "op": "capsule", "a": [-1.0, 0.0], "b": [1.0, 0.0], "radius": 0.2 }
  }
})json";

static void TestParseAndSampleKnownPack() {
    SdfPackParseResult parsed = ParseSdfPackJson(kSmoothUnionPack);
    CHECK(parsed.ok, "valid SDF pack parses");
    if (!parsed.ok) {
        std::cerr << parsed.error << "\n";
        return;
    }
    CHECK(parsed.pack.schema == 1, "schema is captured");
    CHECK(parsed.pack.pack_id == "sdf_smooth_capsule_union_demo", "pack id is captured");
    CHECK(parsed.pack.params.size() == 1, "one param parsed");
    CHECK(parsed.pack.controls.size() == 1, "one control parsed");
    CHECK(parsed.pack.region.has_region, "region parsed");

    SdfPackSampleResult center = SampleSdfPackCpu(parsed.pack, 0.0, 0.0, {});
    CHECK(center.ok, "center sample succeeds");
    CHECK(center.distance < -0.45, "center is inside the union");

    SdfPackSampleResult far = SampleSdfPackCpu(parsed.pack, 2.0, 0.0, {});
    CHECK(far.ok, "far sample succeeds");
    CHECK(far.distance > 0.75, "far sample is outside the union");
}

static void TestPrimitiveDistances() {
    const char* primitivePack = R"json({
      "schema": 1,
      "pack_id": "circle_box_capsule",
      "name": "Circle Box Capsule",
      "kind": "sdf_scene_2d",
      "params": [],
      "ast": {
        "op": "union",
        "a": { "op": "box", "center": [0.0, 0.0], "half_size": [0.5, 0.25] },
        "b": { "op": "circle", "center": [1.0, 0.0], "radius": 0.25 }
      }
    })json";
    SdfPackParseResult parsed = ParseSdfPackJson(primitivePack);
    CHECK(parsed.ok, "primitive pack parses");
    if (!parsed.ok) return;

    SdfPackSampleResult insideBox = SampleSdfPackCpu(parsed.pack, 0.0, 0.0, {});
    SdfPackSampleResult onCircle = SampleSdfPackCpu(parsed.pack, 1.25, 0.0, {});
    SdfPackSampleResult outside = SampleSdfPackCpu(parsed.pack, 1.6, 0.0, {});
    CHECK(insideBox.ok && Nearly(insideBox.distance, -0.25), "box interior distance is exact");
    CHECK(onCircle.ok && Nearly(onCircle.distance, 0.0), "circle boundary distance is exact");
    CHECK(outside.ok && outside.distance > 0.25, "outside distance is positive");
}

static void TestParamOverrideChangesDistance() {
    SdfPackParseResult parsed = ParseSdfPackJson(kSmoothUnionPack);
    CHECK(parsed.ok, "override pack parses");
    if (!parsed.ok) return;

    SdfPackSampleResult tight = SampleSdfPackCpu(parsed.pack, 0.0, 0.42, {{"blend", 0.0}});
    SdfPackSampleResult smooth = SampleSdfPackCpu(parsed.pack, 0.0, 0.42, {{"blend", 0.5}});
    CHECK(tight.ok && smooth.ok, "override samples succeed");
    CHECK(!Nearly(tight.distance, smooth.distance), "blend override changes the field");
    CHECK(smooth.distance < tight.distance, "larger smooth-union blend expands the union near the seam");
}

static void TestRejections() {
    const char* unknownRoot = R"json({"schema":1,"pack_id":"bad","name":"Bad","kind":"sdf_scene_2d","ast":{"op":"circle","radius":1.0},"unexpected":true})json";
    SdfPackParseResult parsed = ParseSdfPackJson(unknownRoot);
    CHECK(!parsed.ok, "unknown root key rejects");
    CHECK(parsed.error.find("Unknown") != std::string::npos, "unknown root key error is descriptive");

    const char* badOp = R"json({"schema":1,"pack_id":"bad_op","name":"Bad Op","kind":"sdf_scene_2d","ast":{"op":"unknown_op"}})json";
    parsed = ParseSdfPackJson(badOp);
    CHECK(!parsed.ok, "unknown AST op rejects during parse");

    const char* nonFinite = R"json({"schema":1,"pack_id":"nonfinite","name":"Nonfinite","kind":"sdf_scene_2d","params":[{"id":"gain","type":"float","default":1e309,"range":[0.0,2.0]}],"ast":{"op":"circle","radius":{"param":"gain"}}})json";
    parsed = ParseSdfPackJson(nonFinite);
    CHECK(!parsed.ok, "nonfinite param default rejects");

    const char* duplicateParams = R"json({"schema":1,"pack_id":"dup","name":"Dup","kind":"sdf_scene_2d","params":[{"id":"r","type":"float","default":0.5,"range":[0.0,1.0]},{"id":"r","type":"float","default":0.25,"range":[0.0,1.0]}],"ast":{"op":"circle","radius":{"param":"r"}}})json";
    parsed = ParseSdfPackJson(duplicateParams);
    CHECK(!parsed.ok, "duplicate params reject");

    const char* badOverride = R"json({"schema":1,"pack_id":"override","name":"Override","kind":"sdf_scene_2d","params":[{"id":"r","type":"float","default":0.5,"range":[0.0,1.0]}],"ast":{"op":"circle","radius":{"param":"r"}}})json";
    parsed = ParseSdfPackJson(badOverride);
    CHECK(parsed.ok, "override bounds pack parses");
    SdfPackSampleResult sample = SampleSdfPackCpu(parsed.pack, 0.0, 0.0, {{"r", 2.0}});
    CHECK(!sample.ok, "out-of-range override rejects");

    std::string tooLargeAst = "{\"schema\":1,\"pack_id\":\"too_large\",\"name\":\"Too Large\",\"kind\":\"sdf_scene_2d\",\"ast\":";
    for (int i = 0; i < SDF_PACK_MAX_AST_NODES + 4; ++i) {
        tooLargeAst += "{\"op\":\"translate\",\"offset\":[0.01,0.0],\"child\":";
    }
    tooLargeAst += "{\"op\":\"circle\",\"radius\":1.0}";
    for (int i = 0; i < SDF_PACK_MAX_AST_NODES + 4; ++i) {
        tooLargeAst += "}";
    }
    tooLargeAst += "}";
    parsed = ParseSdfPackJson(tooLargeAst);
    CHECK(!parsed.ok, "oversized AST rejects");
}

int main() {
    TestParseAndSampleKnownPack();
    TestPrimitiveDistances();
    TestParamOverrideChangesDistance();
    TestRejections();
    std::cout << "test_sdf_pack: pass=" << g_pass << " fail=" << g_fail << "\n";
    return g_fail == 0 ? 0 : 1;
}
