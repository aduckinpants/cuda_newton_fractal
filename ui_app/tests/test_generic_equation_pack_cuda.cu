// CUDA parity tests for AST-lowered equation packs.

#include "generic_equation_pack.h"
#include "generic_function_cpu_eval.h"
#include "generic_sample_core.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
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

static bool Nearly(double actual, double expected, double tol) {
    const double scale = (std::max)(1.0, (std::max)(std::fabs(actual), std::fabs(expected)));
    return std::fabs(actual - expected) <= tol * scale;
}

static GenericSampleResult CpuPublicSample(
    const GenericFunctionDesc& desc,
    GFCpuComplex z,
    double epsilon,
    double escapeRadius)
{
    const GFNode& root = desc.nodes[desc.root_node];
    if (root.op != GFNodeOp::gf_iterate) {
        return gf_cpu_sample(desc, z, epsilon, escapeRadius);
    }

    const int maxIter = static_cast<int>(desc.params[root.param_index]);
    const int subtree = root.child_left;
    const double eps2 = epsilon * epsilon;
    const double esc2 = escapeRadius * escapeRadius;
    int iter = 0;
    bool converged = false;
    bool diverged = false;
    for (; iter < maxIter; ++iter) {
        GFCpuComplex next = gf_cpu_eval_recursive(desc, subtree, z);
        const double dx = next.x - z.x;
        const double dy = next.y - z.y;
        z = next;
        if (dx * dx + dy * dy < eps2) {
            converged = true;
            ++iter;
            break;
        }
        if (gf_cpu_abs2(z) > esc2) {
            diverged = true;
            ++iter;
            break;
        }
    }

    GenericSampleResult result{};
    result.value_x = z.x;
    result.value_y = z.y;
    result.abs2 = gf_cpu_abs2(z);
    result.iterations = iter;
    result.converged = converged;
    result.diverged = diverged;
    return result;
}

static void CheckPackCudaParity(const char* label, const char* packJson) {
    GenericEquationPackParseResult parsed = ParseGenericEquationPackJson(packJson);
    CHECK(parsed.ok, "pack parses");
    if (!parsed.ok) {
        std::printf("%s parse error: %s\n", label, parsed.error.c_str());
        return;
    }
    GenericEquationLowerResult lowered = LowerGenericEquationPackToDesc(parsed.pack);
    CHECK(lowered.ok, "pack lowers");
    if (!lowered.ok) {
        std::printf("%s lower error: %s\n", label, lowered.error.c_str());
        return;
    }

    std::vector<GFPoint> coords = {
        {1.0, 0.0},
        {1.1, 0.05},
        {-0.4, 0.9},
        {0.25, -0.7},
    };
    std::vector<GenericSampleResult> gpu(coords.size());
    const char* error = nullptr;
    const bool ok = SampleGenericFunction(
        coords.data(),
        static_cast<int>(coords.size()),
        lowered.desc,
        parsed.pack.epsilon,
        parsed.pack.escape_radius,
        gpu.data(),
        &error);
    CHECK(ok, "SampleGenericFunction succeeds");
    if (!ok) {
        std::printf("%s cuda error: %s\n", label, error ? error : "<none>");
        return;
    }

    for (size_t i = 0; i < coords.size(); ++i) {
        GenericSampleResult cpu = CpuPublicSample(
            lowered.desc,
            {coords[i].x, coords[i].y},
            parsed.pack.epsilon,
            parsed.pack.escape_radius);
        CHECK(Nearly(gpu[i].value_x, cpu.value_x, 1e-10), "CUDA/CPU value_x parity");
        CHECK(Nearly(gpu[i].value_y, cpu.value_y, 1e-10), "CUDA/CPU value_y parity");
        CHECK(Nearly(gpu[i].abs2, cpu.abs2, 1e-10), "CUDA/CPU abs2 parity");
        CHECK(gpu[i].iterations == cpu.iterations, "CUDA/CPU iterations parity");
        CHECK(gpu[i].converged == cpu.converged, "CUDA/CPU convergence parity");
        CHECK(gpu[i].diverged == cpu.diverged, "CUDA/CPU divergence parity");
    }
}

int main() {
    const char* directPack = R"json({
      "schema_version": 1,
      "pack_id": "direct_compose",
      "name": "Direct Compose",
      "formula": {
        "kind": "direct",
        "ast": {
          "op": "compose",
          "args": [
            {"op": "pow_int", "base": {"op": "var_z"}, "exponent": 2},
            {"op": "add", "args": [{"op": "var_z"}, {"op": "const", "value": 1.0}]}
          ]
        }
      },
      "params": {},
      "epsilon": 1e-10,
      "escape_radius": 1000.0
    })json";
    CheckPackCudaParity("direct compose", directPack);

    const char* iteratePack = R"json({
      "schema_version": 1,
      "pack_id": "quadratic_iterate",
      "name": "Quadratic Iterate",
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
      "params": {"steps": 30.0, "c_real": -0.75, "c_imag": 0.1},
      "epsilon": 1e-9,
      "escape_radius": 1000.0
    })json";
    CheckPackCudaParity("quadratic iterate", iteratePack);

    std::printf("test_generic_equation_pack_cuda: pass=%d fail=%d\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
