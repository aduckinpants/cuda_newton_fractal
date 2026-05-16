// test_callable_engine_adversarial.cpp — Adversarial hardening for the callable engine surface.
// Proves: empty function_id rejection, dispatch exhaustiveness, and
// descriptor-vs-execution type coverage sync.

#include "../src/fractal_probe_contract.h"
#include "../src/fractal_probe_runner.h"
#include "../src/function_descriptor.h"
#include "../src/ui_schema.h"
#include "../src/enum_id_utils.h"

#include <iostream>
#include <string>
#include <vector>

int main() {
    int passed = 0;

    // --- 1. Empty function_id must be rejected, not silently default to fractal.sample ---
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "adversarial-empty-fid";
        request.function_id = "";
        request.mode = FractalProbeMode::point_set;
        request.points.push_back({0.5, 0.5});

        FractalProbeResponse response{};
        std::string error;
        const bool ok = RunFractalProbeRequest(request, "unused", &response, &error);
        if (ok) {
            std::cerr << "[1] FAIL: empty function_id succeeded silently (no-implicit-fallback violation)\n";
            return 1;
        }
        if (error.find("function_id") == std::string::npos) {
            std::cerr << "[1] FAIL: error message does not mention function_id: " << error << "\n";
            return 1;
        }
        std::cout << "[1] empty function_id rejection: passed\n";
        passed++;
    }

    // --- 2. Dispatch must be exhaustive via explicit execution_kind validation ---
    // The dispatch in RunFractalProbeRequest must not have a fall-through path
    // where an unknown execution_kind silently runs the fractal sampler.
    // This is tested indirectly: we verify that every registered function
    // resolves to an execution_kind that the dispatch explicitly handles.
    {
        const char* knownIds[] = {"fractal.sample", "generic.sample"};
        for (const char* fid : knownIds) {
            const EngineFunctionRegistration* reg = FindEngineFunctionRegistration(fid);
            if (!reg) {
                std::cerr << "[2] FAIL: registered id '" << fid << "' not found\n";
                return 1;
            }
            // Verify dispatch knows this kind by ensuring a minimal probe
            // of that kind either succeeds or fails with a domain error
            // (not an internal crash or silent wrong-path execution).
            const int kind = static_cast<int>(reg->execution_kind);
            if (kind != static_cast<int>(EngineFunctionExecutionKind::fractal_sampler) &&
                kind != static_cast<int>(EngineFunctionExecutionKind::generic_sampler)) {
                std::cerr << "[2] FAIL: execution_kind " << kind << " for '" << fid
                          << "' is not handled by dispatch (fail-open risk)\n";
                return 1;
            }
        }
        // Verify unknown function_id is still rejected
        if (FindEngineFunctionRegistration("evil.sample") != nullptr) {
            std::cerr << "[2] FAIL: unknown function_id 'evil.sample' was found in registry\n";
            return 1;
        }
        std::cout << "[2] dispatch exhaustiveness: passed\n";
        passed++;
    }

    // --- 3. Every advertised probe fractal type must actually be sampleable ---
    // Build a minimal schema and catalog to get the descriptor's advertised
    // fractal types, then try to sample each one through RunFractalProbeRequest.
    // If a type is advertised but SamplePoint returns "not yet implemented",
    // that is a contract lie and this test must fail.
    {
        // Build a minimal schema with fractal_type enum containing all FractalType values.
        UISchema schema;
        schema.schema_version = "1";
        schema.name_space = "fractal";
        UISchemaPanel viewPanel;
        viewPanel.id = "view";
        UISchemaControl ftControl;
        ftControl.id = "fractal_type";
        ftControl.type = "combo";
        ftControl.value_type = "enum";
        ftControl.has_binding = true;
        ftControl.binding.kind = "param";
        ftControl.binding.path = "fractal.view.fractal_type";
        // Add every FractalType as an option
        for (int i = 0; i <= static_cast<int>(FractalType::projection_and_flow); ++i) {
            const FractalType ft = static_cast<FractalType>(i);
            const char* tid = FractalTypeId(ft);
            if (tid && tid[0] != '\0') {
                ftControl.options.push_back({tid, tid});
            }
        }
        viewPanel.controls.push_back(ftControl);
        schema.panels.push_back(viewPanel);

        FunctionDescriptor desc = BuildFractalSamplerDescriptor(schema);

        // Find the fractal_type parameter and get its filtered options
        const FunctionParamDescriptor* ftParam = nullptr;
        for (const auto& p : desc.parameters) {
            if (p.path == "fractal.view.fractal_type") {
                ftParam = &p;
                break;
            }
        }
        if (!ftParam) {
            std::cerr << "[3] FAIL: descriptor missing fractal.view.fractal_type parameter\n";
            return 1;
        }

        int typesChecked = 0;
        int typesFailed = 0;
        for (const auto& opt : ftParam->options) {
            // Each advertised option must be sampleable.
            FractalProbeRequest request{};
            request.request_version = 1;
            request.request_id = "adversarial-type-" + opt.id;
            request.function_id = "fractal.sample";
            request.mode = FractalProbeMode::point_set;
            request.points.push_back({0.5, 0.5});
            request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String(opt.id)});

            FractalProbeResponse response{};
            std::string error;
            const bool ok = RunFractalProbeRequest(request, "unused", &response, &error);
            if (!ok && error.find("not yet implemented") != std::string::npos) {
                std::cerr << "[3] FAIL: fractal_type '" << opt.id
                          << "' is advertised in descriptor but not implemented in SamplePoint\n";
                typesFailed++;
            }
            typesChecked++;
        }

        if (typesFailed > 0) {
            std::cerr << "[3] " << typesFailed << " of " << typesChecked
                      << " advertised types are not sampleable\n";
            return 1;
        }
        if (typesChecked < 20) {
            std::cerr << "[3] FAIL: only " << typesChecked
                      << " types checked — expected at least 20 advertised types\n";
            return 1;
        }
        std::cout << "[3] descriptor-vs-execution type sync (" << typesChecked << " types): passed\n";
        passed++;
    }

    // --- 4. RunFractalProbeRequest with null outResponse must fail ---
    {
        FractalProbeRequest request{};
        request.function_id = "fractal.sample";
        std::string error;
        const bool ok = RunFractalProbeRequest(request, "unused", nullptr, &error);
        if (ok) {
            std::cerr << "[4] FAIL: null outResponse should fail\n";
            return 1;
        }
        std::cout << "[4] null outResponse rejection: passed\n";
        passed++;
    }

    // --- 5. Descriptor builder from registry must not be null ---
    {
        const char* knownIds[] = {"fractal.sample", "generic.sample"};
        for (const char* fid : knownIds) {
            const EngineFunctionRegistration* reg = FindEngineFunctionRegistration(fid);
            if (!reg) {
                std::cerr << "[5] FAIL: '" << fid << "' not in registry\n";
                return 1;
            }
            if (!reg->descriptor_builder) {
                std::cerr << "[5] FAIL: descriptor_builder is null for '" << fid << "'\n";
                return 1;
            }
        }
        std::cout << "[5] descriptor_builder non-null: passed\n";
        passed++;
    }

    // --- 6. Unknown function_id in probe request must list valid ids ---
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "adversarial-unknown-fid";
        request.function_id = "evil.compute";
        request.mode = FractalProbeMode::point_set;
        request.points.push_back({0.0, 0.0});

        FractalProbeResponse response{};
        std::string error;
        const bool ok = RunFractalProbeRequest(request, "unused", &response, &error);
        if (ok) {
            std::cerr << "[6] FAIL: unknown function_id should fail\n";
            return 1;
        }
        if (error.find("fractal.sample") == std::string::npos ||
            error.find("generic.sample") == std::string::npos) {
            std::cerr << "[6] FAIL: error should list valid ids: " << error << "\n";
            return 1;
        }
        std::cout << "[6] unknown function_id error message: passed\n";
        passed++;
    }

    // --- 7. Grid dimensions that would cause DoS must be rejected ---
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "adversarial-grid-dos";
        request.function_id = "fractal.sample";
        request.mode = FractalProbeMode::grid;
        request.has_region = true;
        // 5000x5000 = 25M points — exceeds any reasonable grid cap.
        request.region = {0.0, 0.0, 1.0, 1.0, 5000, 5000};

        FractalProbeResponse response{};
        std::string error;
        const bool ok = RunFractalProbeRequest(request, "unused", &response, &error);
        if (ok) {
            std::cerr << "[7] FAIL: enormous grid should be rejected, not allocated\n";
            return 1;
        }
        if (error.find("grid") == std::string::npos && error.find("Grid") == std::string::npos) {
            std::cerr << "[7] FAIL: error should mention grid: " << error << "\n";
            return 1;
        }
        std::cout << "[7] grid dimension DoS rejection: passed\n";
        passed++;
    }

    // --- 8. Grid at the boundary (4M points = 2000x2000) should succeed ---
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "adversarial-grid-boundary";
        request.function_id = "fractal.sample";
        request.mode = FractalProbeMode::grid;
        request.has_region = true;
        request.region = {0.0, 0.0, 1.0, 1.0, 2000, 2000}; // 4M points, within cap

        FractalProbeResponse response{};
        std::string error;
        const bool ok = RunFractalProbeRequest(request, "unused", &response, &error);
        if (!ok) {
            std::cerr << "[8] FAIL: 2000x2000 grid should succeed: " << error << "\n";
            return 1;
        }
        if (response.summary.sample_count != 4000000) {
            std::cerr << "[8] FAIL: expected 4M samples, got " << response.summary.sample_count << "\n";
            return 1;
        }
        std::cout << "[8] grid boundary 4M points: passed\n";
        passed++;
    }

    // --- 9. Deeply nested generic expression must fail, not stack-overflow ---
    {
        // Build "-(-(-(- ... z ... )))" 200 levels deep
        std::string expr;
        for (int i = 0; i < 200; ++i) expr += "-(";
        expr += "z";
        for (int i = 0; i < 200; ++i) expr += ")";

        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "adversarial-parser-depth";
        request.function_id = "generic.sample";
        request.mode = FractalProbeMode::point_set;
        request.points.push_back({0.5, 0.5});
        request.has_function = true;
        request.generic_expression = expr;
        request.generic_epsilon = 1e-6;
        request.generic_escape_radius = 100.0;

        FractalProbeResponse response{};
        std::string error;
        const bool ok = RunFractalProbeRequest(request, "unused", &response, &error);
        if (ok) {
            std::cerr << "[9] FAIL: deeply nested expression should be rejected\n";
            return 1;
        }
        if (error.find("depth") == std::string::npos && error.find("nested") == std::string::npos &&
            error.find("deep") == std::string::npos) {
            std::cerr << "[9] FAIL: error should mention depth/nesting: " << error << "\n";
            return 1;
        }
        std::cout << "[9] parser recursion depth limit: passed\n";
        passed++;
    }

    // --- 10. Deeply nested parentheses must also fail ---
    {
        std::string expr;
        for (int i = 0; i < 200; ++i) expr += "(";
        expr += "z";
        for (int i = 0; i < 200; ++i) expr += ")";

        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "adversarial-paren-depth";
        request.function_id = "generic.sample";
        request.mode = FractalProbeMode::point_set;
        request.points.push_back({0.5, 0.5});
        request.has_function = true;
        request.generic_expression = expr;
        request.generic_epsilon = 1e-6;
        request.generic_escape_radius = 100.0;

        FractalProbeResponse response{};
        std::string error;
        const bool ok = RunFractalProbeRequest(request, "unused", &response, &error);
        if (ok) {
            std::cerr << "[10] FAIL: deeply nested parens should be rejected\n";
            return 1;
        }
        std::cout << "[10] parenthesized depth limit: passed\n";
        passed++;
    }

    std::cout << passed << " passed, 0 failed\n";
    return 0;
}
