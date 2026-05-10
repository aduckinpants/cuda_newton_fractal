#include "../src/generic_function_types.h"
#include "../src/fractal_types.h"

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

GenericFunctionDesc MakeEmptyDesc() {
    GenericFunctionDesc desc{};
    desc.node_count = 0;
    desc.param_count = 0;
    desc.root_node = 0;
    desc.max_iterate = 1;
    return desc;
}

void TestValidateGenericFunctionDesc() {
    GenericFunctionDesc desc = MakeEmptyDesc();
    GFValidationResult validation = ValidateGenericFunctionDesc(desc);
    Check(!validation.valid, "TestValidateGenericFunctionDesc_RejectsEmpty");

    desc = MakeEmptyDesc();
    desc.nodes[0] = {GFNodeOp::gf_var_z, -1, -1, -1};
    desc.node_count = 1;
    validation = ValidateGenericFunctionDesc(desc);
    Check(validation.valid, "TestValidateGenericFunctionDesc_AcceptsSingleVarZ");

    desc = MakeEmptyDesc();
    desc.nodes[0] = {GFNodeOp::gf_add, 0, 0, -1};
    desc.node_count = 1;
    validation = ValidateGenericFunctionDesc(desc);
    Check(!validation.valid, "TestValidateGenericFunctionDesc_RejectsSelfReference");

    desc = MakeEmptyDesc();
    desc.nodes[0] = {GFNodeOp::gf_add, 5, -1, -1};
    desc.node_count = 1;
    validation = ValidateGenericFunctionDesc(desc);
    Check(!validation.valid, "TestValidateGenericFunctionDesc_RejectsOutOfRangeChild");

    desc = MakeEmptyDesc();
    desc.nodes[0] = {GFNodeOp::gf_const_complex, -1, -1, 0};
    desc.node_count = 1;
    desc.param_count = 1;
    desc.params[0] = 2.0;
    validation = ValidateGenericFunctionDesc(desc);
    Check(!validation.valid, "TestValidateGenericFunctionDesc_RejectsShortComplexConst");

    desc = MakeEmptyDesc();
    desc.nodes[0] = {GFNodeOp::gf_iterate, -1, -1, 0};
    desc.node_count = 1;
    desc.param_count = 1;
    desc.params[0] = 3.5;
    validation = ValidateGenericFunctionDesc(desc);
    Check(!validation.valid, "TestValidateGenericFunctionDesc_RejectsFractionalIterateCount");

    desc.params[0] = 0.0;
    validation = ValidateGenericFunctionDesc(desc);
    Check(!validation.valid, "TestValidateGenericFunctionDesc_RejectsZeroIterateCount");

    desc.params[0] = 4.0;
    validation = ValidateGenericFunctionDesc(desc);
    Check(validation.valid, "TestValidateGenericFunctionDesc_AcceptsIntegralIterateCount");
}

void TestGenericSampleResultContract() {
    GenericSampleResult result{};
    Check(result.value_x == 0.0, "TestGenericSampleResultContract_DefaultValueX");
    Check(result.value_y == 0.0, "TestGenericSampleResultContract_DefaultValueY");
    Check(result.abs2 == 0.0, "TestGenericSampleResultContract_DefaultAbs2");
    Check(result.iterations == 0, "TestGenericSampleResultContract_DefaultIterations");
    Check(!result.converged, "TestGenericSampleResultContract_DefaultConverged");
    Check(!result.diverged, "TestGenericSampleResultContract_DefaultDiverged");
    Check(result.termination_kind == TerminationKind::none,
        "TestGenericSampleResultContract_DefaultTerminationKind");
    Check(!result.has_far_field_delta, "TestGenericSampleResultContract_DefaultHasFarFieldDelta");
    Check(result.far_field_delta == 0.0, "TestGenericSampleResultContract_DefaultFarFieldDelta");

    result.value_x = 1.5;
    result.value_y = -0.5;
    result.abs2 = 2.5;
    result.derivative_x = 3.0;
    result.derivative_y = -4.0;
    result.iterations = 7;
    result.converged = true;
    result.diverged = false;
    result.termination_kind = TerminationKind::root_converged;
    result.has_far_field_delta = true;
    result.far_field_delta = 0.75;

    Check(result.value_x == 1.5 && result.value_y == -0.5,
        "TestGenericSampleResultContract_RoundTripValues");
    Check(result.derivative_x == 3.0 && result.derivative_y == -4.0,
        "TestGenericSampleResultContract_RoundTripDerivatives");
    Check(result.iterations == 7 && result.converged && !result.diverged,
        "TestGenericSampleResultContract_RoundTripFlags");
    Check(result.termination_kind == TerminationKind::root_converged &&
            result.has_far_field_delta && result.far_field_delta == 0.75,
        "TestGenericSampleResultContract_RoundTripTermination");
}

void TestPODLayoutExpectations() {
    Check(sizeof(GFNode) == 4 * sizeof(int), "TestPODLayoutExpectations_GFNodeSize");
    Check(sizeof(GenericFunctionDesc) > sizeof(GFNode) * MAX_GF_NODES,
        "TestPODLayoutExpectations_GenericFunctionDescSize");
}

} // namespace

int main() {
    TestValidateGenericFunctionDesc();
    TestGenericSampleResultContract();
    TestPODLayoutExpectations();

    std::printf("test_generic_function_types: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}