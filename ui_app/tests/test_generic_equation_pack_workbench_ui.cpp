#include "generic_equation_pack_workbench.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

static int g_passed = 0;
static int g_failed = 0;

static void Check(bool condition, const char* message) {
    if (condition) {
        ++g_passed;
    } else {
        ++g_failed;
        std::printf("  FAIL: %s\n", message);
    }
}

static const char* kInteractivePack = R"json({
  "schema_version": 1,
  "pack_id": "interactive_quadratic",
  "name": "Interactive Quadratic",
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
  "params": {"steps": 12.0, "c_real": -0.75, "c_imag": 0.1},
  "controls": [
    {"id": "steps", "param": "steps", "label": "Steps", "min": 1.0, "max": 80.0, "step": 1.0, "default": 12.0},
    {"id": "c_real", "param": "c_real", "label": "C Real", "min": -2.0, "max": 2.0, "step": 0.01, "default": -0.75}
  ],
  "epsilon": 1e-9,
  "escape_radius": 1000.0,
  "region": {"center_x": 0.0, "center_y": 0.0, "span_x": 2.0, "span_y": 2.0, "grid_width": 4, "grid_height": 3}
})json";

bool SampleGenericFunction(
    const GFPoint* coords,
    int numPoints,
    const GenericFunctionDesc& func,
    double,
    double,
    GenericSampleResult* outResults,
    const char** outError)
{
    if (outError) {
        *outError = nullptr;
    }
    if (!coords || !outResults || numPoints < 0) {
        if (outError) {
            *outError = "bad test sampler input";
        }
        return false;
    }

    double paramSum = 0.0;
    for (int index = 0; index < func.param_count; ++index) {
        paramSum += func.params[index];
    }
    for (int index = 0; index < numPoints; ++index) {
        GenericSampleResult result{};
        result.value_x = coords[index].x + paramSum;
        result.value_y = coords[index].y - paramSum;
        result.abs2 = result.value_x * result.value_x + result.value_y * result.value_y;
        result.iterations = static_cast<int>(std::fabs(paramSum)) + index + 1;
        result.converged = (index % 2) == 0;
        result.diverged = (index % 2) != 0;
        outResults[index] = result;
    }
    return true;
}

static void TestPackControlsAreSchemaDrivenAndEditable() {
    GenericEquationPackWorkbenchState state{};
    std::string error;
    Check(LoadGenericEquationPackWorkbenchJson(&state, kInteractivePack, "memory://interactive", &error),
        "pack JSON loads into workbench state");
    Check(state.have_pack, "state records loaded pack");
    Check(state.pack.pack_id == "interactive_quadratic", "pack id preserved");
    Check(state.params["steps"] == 12.0, "initial params copied");
    Check(GenericEquationPackWorkbenchControlAutomationId(state.pack.controls[1]) == "equation_pack.c_real.primary",
        "control automation id is derived from pack control id");

    Check(SetGenericEquationPackWorkbenchControlValue(&state, "equation_pack.c_real.primary", 0.25, &error),
        "set-value automation accepts visible pack control");
    Check(std::fabs(state.params["c_real"] - 0.25) < 1e-12, "set-value updates the bound pack param");
    Check(state.preview_dirty, "set-value marks preview dirty");

    Check(!SetGenericEquationPackWorkbenchControlValue(&state, "equation_pack.missing.primary", 1.0, &error),
        "unknown equation pack control fails");
    Check(error.find("unknown") != std::string::npos, "unknown control error is descriptive");
}

static void TestPreviewRunsThroughSamplerAndReportChanges() {
    GenericEquationPackWorkbenchState state{};
    std::string error;
    Check(LoadGenericEquationPackWorkbenchJson(&state, kInteractivePack, "memory://interactive", &error),
        "pack JSON loads before preview");
    Check(RunGenericEquationPackWorkbenchPreview(&state, &error), "preview runs through SampleGenericFunction");
    GenericEquationPackWorkbenchAutomationReport firstReport = BuildGenericEquationPackWorkbenchAutomationReport(state);
    Check(firstReport.preview_ok, "preview report records success");
    Check(firstReport.preview_backend_used == "cuda", "preview report identifies CUDA sampler path");
    Check(firstReport.preview_sample_count == 12, "preview grid respects pack region dimensions");
    Check(!firstReport.preview_result_hash.empty(), "preview report includes deterministic result hash");

    const std::string firstHash = firstReport.preview_result_hash;
    Check(SetGenericEquationPackWorkbenchControlValue(&state, "equation_pack.c_real.primary", 0.75, &error),
        "second control edit succeeds");
    Check(RunGenericEquationPackWorkbenchPreview(&state, &error), "second preview runs");
    GenericEquationPackWorkbenchAutomationReport secondReport = BuildGenericEquationPackWorkbenchAutomationReport(state);
    Check(secondReport.preview_result_hash != firstHash, "control edit changes preview result hash");
}

int main() {
    TestPackControlsAreSchemaDrivenAndEditable();
    TestPreviewRunsThroughSamplerAndReportChanges();
    if (g_failed != 0) {
        std::printf("test_generic_equation_pack_workbench_ui: %d failure(s)\n", g_failed);
        return 1;
    }
    std::printf("test_generic_equation_pack_workbench_ui: passed=%d\n", g_passed);
    return 0;
}
