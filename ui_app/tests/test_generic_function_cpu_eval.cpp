#include "../src/generic_function_cpu_eval.h"

#include <cmath>
#include <cstdio>

namespace {

int g_passed = 0;
int g_failed = 0;

void Check(bool condition, const char* name) {
    if (condition) {
        std::printf("  PASS: %s\n", name);
        ++g_passed;
    } else {
        std::printf("  FAIL: %s\n", name);
        ++g_failed;
    }
}

bool NearlyEqual(double left, double right, double eps = 1.0e-9) {
    return std::fabs(left - right) <= eps;
}

GenericFunctionDesc MakeEmptyDesc() {
    GenericFunctionDesc desc{};
    desc.node_count = 0;
    desc.param_count = 0;
    desc.root_node = 0;
    desc.max_iterate = 1;
    return desc;
}

GenericFunctionDesc MakeConstantDesc(double value) {
    GenericFunctionDesc desc = MakeEmptyDesc();
    desc.nodes[0] = {GFNodeOp::gf_const_real, -1, -1, 0};
    desc.node_count = 1;
    desc.param_count = 1;
    desc.params[0] = value;
    desc.root_node = 0;
    return desc;
}

GenericFunctionDesc MakeSquareDesc() {
    GenericFunctionDesc desc = MakeEmptyDesc();
    desc.nodes[0] = {GFNodeOp::gf_var_z, -1, -1, -1};
    desc.nodes[1] = {GFNodeOp::gf_mul, 0, 0, -1};
    desc.node_count = 2;
    desc.root_node = 1;
    return desc;
}

void TestComplexArithmeticHelpers() {
    const GFCpuComplex a{3.0, 4.0};
    const GFCpuComplex b{1.5, -2.0};

    const GFCpuComplex sum = gf_cpu_add(a, b);
    Check(NearlyEqual(sum.x, 4.5) && NearlyEqual(sum.y, 2.0),
        "TestComplexArithmeticHelpers_Add");

    const GFCpuComplex product = gf_cpu_mul(a, b);
    Check(NearlyEqual(product.x, 12.5) && NearlyEqual(product.y, 0.0),
        "TestComplexArithmeticHelpers_Mul");

    const GFCpuComplex quotient = gf_cpu_div(product, b);
    Check(NearlyEqual(quotient.x, a.x) && NearlyEqual(quotient.y, a.y),
        "TestComplexArithmeticHelpers_DivRoundTrip");

    const GFCpuComplex invalid = gf_cpu_div(a, {0.0, 0.0});
    Check(invalid.x == 1e300 && invalid.y == 1e300,
        "TestComplexArithmeticHelpers_DivideByZeroSentinel");
}

void TestEvalRecursiveCoreOps() {
    GenericFunctionDesc desc = MakeEmptyDesc();
    desc.nodes[0] = {GFNodeOp::gf_var_z, -1, -1, -1};
    desc.nodes[1] = {GFNodeOp::gf_const_complex, -1, -1, 0};
    desc.nodes[2] = {GFNodeOp::gf_mul, 0, 0, -1};
    desc.nodes[3] = {GFNodeOp::gf_add, 2, 1, -1};
    desc.node_count = 4;
    desc.param_count = 2;
    desc.params[0] = 1.5;
    desc.params[1] = -0.25;
    desc.root_node = 3;

    const GFCpuComplex value = gf_cpu_eval(desc, {2.0, 3.0});
    Check(NearlyEqual(value.x, -3.5) && NearlyEqual(value.y, 11.75),
        "TestEvalRecursiveCoreOps_SquarePlusComplexConstant");

    desc = MakeEmptyDesc();
    desc.nodes[0] = {GFNodeOp::gf_var_z, -1, -1, -1};
    desc.nodes[1] = {GFNodeOp::gf_abs2, 0, -1, -1};
    desc.nodes[2] = {GFNodeOp::gf_conj, 0, -1, -1};
    desc.nodes[3] = {GFNodeOp::gf_pack2, 1, 2, -1};
    desc.node_count = 4;
    desc.root_node = 3;

    const GFCpuComplex packed = gf_cpu_eval(desc, {3.0, 4.0});
    Check(NearlyEqual(packed.x, 25.0) && NearlyEqual(packed.y, 3.0),
        "TestEvalRecursiveCoreOps_Abs2ConjPack2");
}

void TestComposeIterateAndGuards() {
    GenericFunctionDesc desc = MakeEmptyDesc();
    desc.nodes[0] = {GFNodeOp::gf_var_z, -1, -1, -1};
    desc.nodes[1] = {GFNodeOp::gf_const_real, -1, -1, 0};
    desc.nodes[2] = {GFNodeOp::gf_add, 0, 1, -1};
    desc.nodes[3] = {GFNodeOp::gf_mul, 0, 0, -1};
    desc.nodes[4] = {GFNodeOp::gf_compose, 3, 2, -1};
    desc.node_count = 5;
    desc.param_count = 1;
    desc.params[0] = 1.0;
    desc.root_node = 4;

    const GFCpuComplex composed = gf_cpu_eval(desc, {2.0, 0.0});
    Check(NearlyEqual(composed.x, 9.0) && NearlyEqual(composed.y, 0.0),
        "TestComposeIterateAndGuards_ComposeSquareAfterIncrement");

    desc.nodes[4] = {GFNodeOp::gf_iterate, 2, -1, 1};
    desc.param_count = 2;
    desc.params[1] = 3.0;
    desc.root_node = 4;
    const GFCpuComplex iterated = gf_cpu_eval(desc, {2.0, 0.0});
    Check(NearlyEqual(iterated.x, 5.0) && NearlyEqual(iterated.y, 0.0),
        "TestComposeIterateAndGuards_IterateIncrementThreeTimes");

    const GFCpuComplex invalid = gf_cpu_eval_recursive(desc, -1, {2.0, 0.0});
    Check(invalid.x == 0.0 && invalid.y == 0.0,
        "TestComposeIterateAndGuards_InvalidNodeReturnsZero");
}

void TestDerivativeAndSampleResult() {
    const GenericFunctionDesc square = MakeSquareDesc();
    const GFCpuComplex derivative = gf_cpu_derivative(square, {3.0, 0.0});
    Check(NearlyEqual(derivative.x, 6.0, 1.0e-5) && NearlyEqual(derivative.y, 0.0, 1.0e-5),
        "TestDerivativeAndSampleResult_SquareDerivative");

    const GenericSampleResult converged = gf_cpu_sample(MakeConstantDesc(0.0), {5.0, 0.0});
    Check(converged.value_x == 0.0 && converged.value_y == 0.0 && converged.converged && !converged.diverged,
        "TestDerivativeAndSampleResult_ZeroConstantConverges");

    const GenericSampleResult diverged = gf_cpu_sample(MakeConstantDesc(1.0e11), {0.0, 0.0});
    Check(diverged.abs2 > 1.0e20 && !diverged.converged && diverged.diverged,
        "TestDerivativeAndSampleResult_LargeConstantDiverges");
}

} // namespace

int main() {
    TestComplexArithmeticHelpers();
    TestEvalRecursiveCoreOps();
    TestComposeIterateAndGuards();
    TestDerivativeAndSampleResult();

    std::printf("test_generic_function_cpu_eval: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}
