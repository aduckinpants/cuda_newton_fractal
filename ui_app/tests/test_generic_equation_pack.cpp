// Native tests for the AST-backed generic equation pack contract.

#include "generic_equation_pack.h"
#include "generic_function_cpu_eval.h"
#include "generic_function_parser.h"

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

static bool Nearly(double a, double b, double tol = 1e-8) {
    const double scale = (std::max)(1.0, (std::max)(std::fabs(a), std::fabs(b)));
    return std::fabs(a - b) <= tol * scale;
}

static const char* kNewtonPack = R"json({
  "schema_version": 1,
  "pack_id": "newton_z3_minus_1",
  "name": "Newton z^3 - 1",
  "formula": {
    "kind": "iterate_map",
    "iteration_param": "steps",
    "ast": {
      "op": "sub",
      "args": [
        {"op": "var_z"},
        {
          "op": "div",
          "args": [
            {"op": "sub", "args": [
              {"op": "pow_int", "base": {"op": "var_z"}, "exponent": 3},
              {"op": "const", "value": 1.0}
            ]},
            {"op": "mul", "args": [
              {"op": "const", "value": 3.0},
              {"op": "pow_int", "base": {"op": "var_z"}, "exponent": 2}
            ]}
          ]
        }
      ]
    }
  },
  "params": {"steps": 24.0},
  "epsilon": 1e-10,
  "escape_radius": 1000.0,
  "controls": [
    {"id": "steps", "param": "steps", "label": "Steps", "min": 1.0, "max": 200.0, "step": 1.0, "default": 24.0}
  ],
  "region": {"center_x": 0.0, "center_y": 0.0, "span_x": 3.0, "span_y": 3.0, "grid_width": 32, "grid_height": 32}
})json";

static const char* kQuadraticPack = R"json({
  "schema_version": 1,
  "pack_id": "quadratic_c",
  "name": "Quadratic c",
  "formula": {
    "kind": "iterate_map",
    "iteration_param": "steps",
    "ast": {
      "op": "add",
      "args": [
        {"op": "pow_int", "base": {"op": "var_z"}, "exponent": 2},
        {"op": "complex_param", "name": "c"}
      ]
    }
  },
  "params": {"steps": 20.0, "c_real": -0.75, "c_imag": 0.1},
  "epsilon": 1e-9,
  "escape_radius": 1000.0
})json";

static void CompareAgainstExpression(
    const char* label,
    const char* packJson,
    const std::string& expression,
    const std::map<std::string, double>& params,
    GFCpuComplex z)
{
    GenericEquationPackParseResult parsed = ParseGenericEquationPackJson(packJson);
    CHECK(parsed.ok, "pack parses");
    if (!parsed.ok) {
        std::cerr << parsed.error << "\n";
        return;
    }

    GenericEquationLowerResult lowered = LowerGenericEquationPackToDesc(parsed.pack);
    CHECK(lowered.ok, "pack lowers to GenericFunctionDesc");
    if (!lowered.ok) {
        std::cerr << lowered.error << "\n";
        return;
    }

    GFParseResult expr = ParseGenericFunctionExpression(expression, params);
    CHECK(expr.ok, "reference expression parses");
    if (!expr.ok) return;

    const GFCpuComplex packValue = gf_cpu_eval(lowered.desc, z);
    const GFCpuComplex exprValue = gf_cpu_eval(expr.desc, z);
    if (!Nearly(packValue.x, exprValue.x, 1e-8) || !Nearly(packValue.y, exprValue.y, 1e-8)) {
        std::cerr << label << " pack=(" << packValue.x << "," << packValue.y
                  << ") expr=(" << exprValue.x << "," << exprValue.y << ")\n";
    }
    CHECK(Nearly(packValue.x, exprValue.x, 1e-8), "pack/expression real values match");
    CHECK(Nearly(packValue.y, exprValue.y, 1e-8), "pack/expression imaginary values match");
}

static void TestKnownFormulaEquivalence() {
    CompareAgainstExpression(
        "newton_z3_minus_1",
        kNewtonPack,
        "iterate(z - (z^3 - 1) / (3 * z^2), steps)",
        {{"steps", 24.0}},
        {1.1, 0.05});

    CompareAgainstExpression(
        "quadratic_c",
        kQuadraticPack,
        "iterate(z^2 + c, steps)",
        {{"steps", 20.0}, {"c_real", -0.75}, {"c_imag", 0.1}},
        {0.0, 0.0});
}

static void TestRejections() {
    const char* unknownRoot = R"json({"schema_version":1,"pack_id":"bad","name":"Bad","unexpected":true,"formula":{"kind":"direct","ast":{"op":"var_z"}},"params":{}})json";
    GenericEquationPackParseResult parsed = ParseGenericEquationPackJson(unknownRoot);
    CHECK(!parsed.ok, "unknown root field rejects");
    CHECK(parsed.error.find("Unknown") != std::string::npos, "unknown root field error is descriptive");

    const char* badOp = R"json({"schema_version":1,"pack_id":"bad_op","name":"Bad Op","formula":{"kind":"direct","ast":{"op":"unknown_op"}},"params":{}})json";
    parsed = ParseGenericEquationPackJson(badOp);
    CHECK(parsed.ok, "bad-op pack parses before lowering");
    GenericEquationLowerResult lowered = LowerGenericEquationPackToDesc(parsed.pack);
    CHECK(!lowered.ok, "invalid AST op rejects during lowering");

    const char* nonFinite = R"json({"schema_version":1,"pack_id":"nonfinite","name":"Nonfinite","formula":{"kind":"direct","ast":{"op":"param","name":"gain"}},"params":{"gain":1e309}})json";
    parsed = ParseGenericEquationPackJson(nonFinite);
    CHECK(!parsed.ok, "nonfinite param rejects");

    const char* duplicateControls = R"json({"schema_version":1,"pack_id":"dup","name":"Dup","formula":{"kind":"direct","ast":{"op":"var_z"}},"params":{"gain":1.0},"controls":[{"id":"gain_a","param":"gain"},{"id":"gain_b","param":"gain"}]})json";
    parsed = ParseGenericEquationPackJson(duplicateControls);
    CHECK(!parsed.ok, "duplicate control param rejects");

    const char* badSteps = R"json({"schema_version":1,"pack_id":"bad_steps","name":"Bad Steps","formula":{"kind":"iterate_map","iteration_param":"steps","ast":{"op":"var_z"}},"params":{"steps":0.0}})json";
    parsed = ParseGenericEquationPackJson(badSteps);
    CHECK(parsed.ok, "bad-step pack parses before lowering");
    lowered = LowerGenericEquationPackToDesc(parsed.pack);
    CHECK(!lowered.ok, "bad iteration count rejects during lowering");

    std::string tooLargeAst = "{\"schema_version\":1,\"pack_id\":\"too_large\",\"name\":\"Too Large\",\"formula\":{\"kind\":\"direct\",\"ast\":";
    for (int i = 0; i < MAX_GF_NODES + 4; ++i) {
        tooLargeAst += "{\"op\":\"add\",\"args\":[";
    }
    tooLargeAst += "{\"op\":\"var_z\"}";
    for (int i = 0; i < MAX_GF_NODES + 4; ++i) {
        tooLargeAst += ",{\"op\":\"const\",\"value\":1.0}]}";
    }
    tooLargeAst += "},\"params\":{}}";
    parsed = ParseGenericEquationPackJson(tooLargeAst);
    CHECK(parsed.ok, "oversized AST pack parses before lowering");
    lowered = LowerGenericEquationPackToDesc(parsed.pack);
    CHECK(!lowered.ok, "oversized AST rejects during lowering");
}

int main() {
    TestKnownFormulaEquivalence();
    TestRejections();
    std::cout << "test_generic_equation_pack: pass=" << g_pass << " fail=" << g_fail << "\n";
    return g_fail == 0 ? 0 : 1;
}
