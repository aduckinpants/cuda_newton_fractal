// test_generic_sample_parity.cu — parity rails for the current generic.sample CPU and CUDA paths.

#include "generic_sample_core.h"
#include "generic_function_cpu_eval.h"
#include "generic_function_parser.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

static int g_pass = 0;
static int g_fail = 0;
static constexpr double kDerivativeParityTolerance = 1e-7;

static void check(bool cond, const char* msg, int line) {
    if (cond) {
        g_pass++;
    } else {
        g_fail++;
        std::printf("  FAIL [line %d]: %s\n", line, msg);
    }
}

#define CHECK(cond, msg) check((cond), (msg), __LINE__)

static bool nearly_equal(double actual, double expected, double tol) {
    if (std::isnan(actual) || std::isnan(expected)) {
        return std::isnan(actual) && std::isnan(expected);
    }
    if (!std::isfinite(actual) || !std::isfinite(expected)) {
        return actual == expected;
    }
    double scale = (std::max)(1.0, (std::max)(std::fabs(actual), std::fabs(expected)));
    return std::fabs(actual - expected) <= tol * scale;
}

static GenericSampleResult cpu_public_sample(
    const GenericFunctionDesc& desc,
    GFCpuComplex z,
    double epsilon,
    double escape_radius)
{
    const GFNode& root = desc.nodes[desc.root_node];
    if (root.op != GFNodeOp::gf_iterate) {
        return gf_cpu_sample(desc, z, epsilon, escape_radius);
    }

    int max_iter = (root.param_index >= 0 && root.param_index < desc.param_count)
        ? static_cast<int>(desc.params[root.param_index])
        : desc.max_iterate;

    const int subtree = root.child_left;
    const double eps2 = epsilon * epsilon;
    const double esc2 = escape_radius * escape_radius;

    int iterations = 0;
    bool converged = false;
    bool diverged = false;
    for (; iterations < max_iter; ++iterations) {
        GFCpuComplex next = gf_cpu_eval_recursive(desc, subtree, z);
        double dx = next.x - z.x;
        double dy = next.y - z.y;
        double delta2 = dx * dx + dy * dy;
        z = next;
        if (delta2 < eps2) {
            converged = true;
            iterations++;
            break;
        }
        double abs2 = gf_cpu_abs2(z);
        if (abs2 > esc2) {
            diverged = true;
            iterations++;
            break;
        }
    }

    double mag = std::sqrt(gf_cpu_abs2(z));
    double h = 1e-8 * (std::max)(mag, 1.0);
    GFCpuComplex fz = gf_cpu_eval_recursive(desc, subtree, z);
    GFCpuComplex fzh = gf_cpu_eval_recursive(desc, subtree, {z.x + h, z.y});

    GenericSampleResult result{};
    result.value_x = z.x;
    result.value_y = z.y;
    result.abs2 = gf_cpu_abs2(z);
    result.derivative_x = (fzh.x - fz.x) / h;
    result.derivative_y = (fzh.y - fz.y) / h;
    result.iterations = iterations;
    result.converged = converged;
    result.diverged = diverged;
    return result;
}

static bool run_cuda_samples(
    const GenericFunctionDesc& desc,
    const std::vector<GFPoint>& coords,
    double epsilon,
    double escape_radius,
    std::vector<GenericSampleResult>* out_results,
    std::string* out_error)
{
    out_results->assign(coords.size(), GenericSampleResult{});
    const char* raw_error = nullptr;
    bool ok = SampleGenericFunction(
        coords.data(),
        static_cast<int>(coords.size()),
        desc,
        epsilon,
        escape_radius,
        out_results->data(),
        &raw_error);
    if (!ok && out_error) {
        *out_error = raw_error ? raw_error : "SampleGenericFunction failed";
    }
    return ok;
}

static void compare_sample(const GenericSampleResult& cpu, const GenericSampleResult& gpu, const char* label) {
    char message[256];

    std::snprintf(message, sizeof(message), "%s value_x parity", label);
    CHECK(nearly_equal(cpu.value_x, gpu.value_x, 1e-10), message);

    std::snprintf(message, sizeof(message), "%s value_y parity", label);
    CHECK(nearly_equal(cpu.value_y, gpu.value_y, 1e-10), message);

    std::snprintf(message, sizeof(message), "%s abs2 parity", label);
    CHECK(nearly_equal(cpu.abs2, gpu.abs2, 1e-10), message);

    std::snprintf(message, sizeof(message), "%s derivative_x parity", label);
    if (!nearly_equal(cpu.derivative_x, gpu.derivative_x, kDerivativeParityTolerance)) {
        std::printf(
            "  derivative_x mismatch for %s: cpu=%.17g gpu=%.17g diff=%.17g\n",
            label,
            cpu.derivative_x,
            gpu.derivative_x,
            gpu.derivative_x - cpu.derivative_x);
        CHECK(false, message);
    } else {
        CHECK(true, message);
    }

    std::snprintf(message, sizeof(message), "%s derivative_y parity", label);
    if (!nearly_equal(cpu.derivative_y, gpu.derivative_y, kDerivativeParityTolerance)) {
        std::printf(
            "  derivative_y mismatch for %s: cpu=%.17g gpu=%.17g diff=%.17g\n",
            label,
            cpu.derivative_y,
            gpu.derivative_y,
            gpu.derivative_y - cpu.derivative_y);
        CHECK(false, message);
    } else {
        CHECK(true, message);
    }

    std::snprintf(message, sizeof(message), "%s iterations parity", label);
    CHECK(cpu.iterations == gpu.iterations, message);

    std::snprintf(message, sizeof(message), "%s converged parity", label);
    CHECK(cpu.converged == gpu.converged, message);

    std::snprintf(message, sizeof(message), "%s diverged parity", label);
    CHECK(cpu.diverged == gpu.diverged, message);
}

static void check_expression_parity(
    const char* label,
    const char* expression,
    const std::map<std::string, double>& params,
    const std::vector<GFPoint>& coords,
    double epsilon,
    double escape_radius)
{
    std::printf("--- %s ---\n", label);

    GFParseResult parse = ParseGenericFunctionExpression(expression, params);
    CHECK(parse.ok, "expression parsed successfully");
    if (!parse.ok) {
        std::printf("  parse error: %s\n", parse.error.c_str());
        return;
    }

    std::vector<GenericSampleResult> gpu_results;
    std::string cuda_error;
    bool ok = run_cuda_samples(parse.desc, coords, epsilon, escape_radius, &gpu_results, &cuda_error);
    CHECK(ok, "CUDA sampler succeeded");
    if (!ok) {
        std::printf("  cuda error: %s\n", cuda_error.c_str());
        return;
    }

    for (size_t index = 0; index < coords.size(); ++index) {
        GFCpuComplex z{coords[index].x, coords[index].y};
        GenericSampleResult cpu = cpu_public_sample(parse.desc, z, epsilon, escape_radius);

        char label_with_index[256];
        std::snprintf(label_with_index, sizeof(label_with_index), "%s sample %zu", label, index);
        compare_sample(cpu, gpu_results[index], label_with_index);
    }
}

static void test_direct_eval_parity() {
    std::vector<GFPoint> coords = {
        {1.0, 0.0},
        {2.0, -0.5},
        {-0.75, 0.25},
        {0.25, 1.0},
    };

    check_expression_parity(
        "direct z^3-1",
        "z ^ 3 - 1",
        {},
        coords,
        1e-10,
        1e6);

    check_expression_parity(
        "direct log",
        "log(z)",
        {},
        coords,
        1e-10,
        1e6);

    check_expression_parity(
        "compose square-plus-one",
        "compose(z ^ 2, z + 1)",
        {},
        coords,
        1e-10,
        1e6);
}

static void test_iterate_parity() {
    std::vector<GFPoint> coords = {
        {1.0, 0.0},
        {1.5, 0.2},
        {-0.6, 0.8},
        {0.75, -0.4},
    };

    check_expression_parity(
        "newton sine ladder",
        "iterate(z - sin(z) / cos(z), 40)",
        {},
        coords,
        1e-9,
        1000.0);

    std::map<std::string, double> params;
    params["c_real"] = -0.75;
    params["c_imag"] = 0.1;
    check_expression_parity(
        "quadratic iterate with c",
        "iterate(z ^ 2 + c, 25)",
        params,
        coords,
        1e-9,
        1000.0);

    params["steps"] = 25.0;
    check_expression_parity(
        "quadratic iterate with parameterized count",
        "iterate(z ^ 2 + c, steps)",
        params,
        coords,
        1e-9,
        1000.0);
}

int main() {
    std::printf("=== generic.sample CPU/CUDA parity tests ===\n\n");

    test_direct_eval_parity();
    test_iterate_parity();

    std::printf("\nPass: %d\nFail: %d\n", g_pass, g_fail);
    if (g_fail != 0) {
        std::printf("PARITY TESTS FAILED\n");
        return 1;
    }

    std::printf("All parity tests passed.\n");
    return 0;
}