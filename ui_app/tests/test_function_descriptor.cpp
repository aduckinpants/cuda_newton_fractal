#include "../src/function_descriptor.h"
#include "../src/ui_schema.h"
#include "../src/json_min.h"
#include "../src/fractal_probe_contract.h"
#include "../src/fractal_probe_runner.h"

#include <iostream>
#include <string>

namespace {

bool NearlyEqual(double left, double right, double eps = 1.0e-6) {
    const double delta = left - right;
    return delta < eps && delta > -eps;
}

} // namespace

int main() {
    // --- Test 1: BuildFractalSamplerDescriptor from a minimal schema ---
    {
        // Build a minimal schema with two controls: fractal_type enum and epsilon float.
        UISchema schema;
        schema.schema_version = "1";
        schema.name_space = "fractal";

        UISchemaPanel viewPanel;
        viewPanel.id = "view";
        viewPanel.label = "View";

        UISchemaControl fractalTypeControl;
        fractalTypeControl.id = "fractal_type";
        fractalTypeControl.type = "combo";
        fractalTypeControl.label = "Fractal Type";
        fractalTypeControl.value_type = "enum";
        fractalTypeControl.has_default = true;
        fractalTypeControl.def.v = std::string("newton");
        fractalTypeControl.has_binding = true;
        fractalTypeControl.binding.kind = "param";
        fractalTypeControl.binding.path = "fractal.view.fractal_type";
        fractalTypeControl.options.push_back({"newton", "Newton"});
        fractalTypeControl.options.push_back({"mandelbrot", "Mandelbrot"});
        fractalTypeControl.options.push_back({"explaino", "Explaino"});
        fractalTypeControl.options.push_back({"burning_ship", "Burning Ship"});
        fractalTypeControl.options.push_back({"multibrot", "Multibrot"});
        fractalTypeControl.options.push_back({"multicorn", "Multicorn"});
        fractalTypeControl.options.push_back({"phoenix", "Phoenix"});
        fractalTypeControl.options.push_back({"future_fractal", "Future Fractal"});
        fractalTypeControl.has_help = true;
        fractalTypeControl.help = "Pick a fractal";
        viewPanel.controls.push_back(fractalTypeControl);

        // An action binding (should be excluded from parameters)
        UISchemaControl renderOnce;
        renderOnce.id = "render_once";
        renderOnce.type = "button";
        renderOnce.label = "Render Once";
        renderOnce.has_binding = true;
        renderOnce.binding.kind = "action";
        renderOnce.binding.path = "fractal.actions.render_once";
        viewPanel.controls.push_back(renderOnce);

        schema.panels.push_back(viewPanel);

        UISchemaPanel paramsPanel;
        paramsPanel.id = "params";
        paramsPanel.label = "Parameters";

        UISchemaControl epsilonControl;
        epsilonControl.id = "epsilon";
        epsilonControl.type = "drag_float";
        epsilonControl.label = "Epsilon";
        epsilonControl.value_type = "float";
        epsilonControl.has_min = true;
        epsilonControl.min = 1e-12;
        epsilonControl.has_max = true;
        epsilonControl.max = 0.01;
        epsilonControl.has_step = true;
        epsilonControl.step = 1e-6;
        epsilonControl.has_default = true;
        epsilonControl.def.v = 1e-6;
        epsilonControl.has_binding = true;
        epsilonControl.binding.kind = "param";
        epsilonControl.binding.path = "fractal.params.epsilon";
        epsilonControl.has_visible_if = true;
        epsilonControl.visible_if.op = "in";
        epsilonControl.visible_if.path = "fractal.view.fractal_type";
        epsilonControl.visible_if.value = "newton,explaino";
        paramsPanel.controls.push_back(epsilonControl);

        UISchemaControl rippleControl;
        rippleControl.id = "ripple_amplitude";
        rippleControl.type = "drag_float";
        rippleControl.label = "Ripple Amplitude";
        rippleControl.value_type = "float";
        rippleControl.has_min = true;
        rippleControl.min = 0.0;
        rippleControl.has_max = true;
        rippleControl.max = 0.15;
        rippleControl.has_step = true;
        rippleControl.step = 0.01;
        rippleControl.has_default = true;
        rippleControl.def.v = 0.15;
        rippleControl.has_binding = true;
        rippleControl.binding.kind = "param";
        rippleControl.binding.path = "fractal.params.ripple_amplitude";
        rippleControl.has_visible_if = true;
        rippleControl.visible_if.op = "eq";
        rippleControl.visible_if.path = "fractal.view.fractal_type";
        rippleControl.visible_if.value = "explaino_ripple";
        paramsPanel.controls.push_back(rippleControl);

        schema.panels.push_back(paramsPanel);

        FunctionDescriptor desc = BuildFractalSamplerDescriptor(schema);

        if (desc.id != "fractal.sample") {
            std::cerr << "Expected function id fractal.sample, got: " << desc.id << "\n";
            return 1;
        }
        if (desc.parameters.size() != 3) {
            std::cerr << "Expected 3 params (action excluded), got: " << desc.parameters.size() << "\n";
            return 1;
        }

        // fractal_type param
        const auto& p0 = desc.parameters[0];
        if (p0.path != "fractal.view.fractal_type" || p0.type != "enum") {
            std::cerr << "First param should be fractal_type enum\n";
            return 1;
        }
        if (!p0.required) {
            std::cerr << "fractal_type should be required\n";
            return 1;
        }
        if (p0.options.size() != 7) {
            std::cerr << "Expected unsupported fractal types to be filtered from the enum options, got: " << p0.options.size() << "\n";
            return 1;
        }
        bool foundBurningShip = false;
        bool foundMultibrot = false;
        bool foundMulticorn = false;
        bool foundPhoenix = false;
        for (const auto& option : p0.options) {
            if (option.id == "burning_ship") foundBurningShip = true;
            if (option.id == "multibrot") foundMultibrot = true;
            if (option.id == "multicorn") foundMulticorn = true;
            if (option.id == "phoenix") foundPhoenix = true;
            if (option.id == "future_fractal") {
                std::cerr << "unsupported enum ids should be filtered from the descriptor surface\n";
                return 1;
            }
        }
        if (!foundBurningShip || !foundMultibrot || !foundMulticorn || !foundPhoenix) {
            std::cerr << "Expected supported fractal types to be advertised as sampleable\n";
            return 1;
        }
        if (!p0.has_default || !p0.default_value.is_string() || p0.default_value.as_string() != "newton") {
            std::cerr << "fractal_type default should be 'newton'\n";
            return 1;
        }
        if (p0.help != "Pick a fractal") {
            std::cerr << "fractal_type help mismatch\n";
            return 1;
        }

        // epsilon param
        const auto& p1 = desc.parameters[1];
        if (p1.path != "fractal.params.epsilon" || p1.type != "float") {
            std::cerr << "Second param should be epsilon float\n";
            return 1;
        }
        if (p1.required) {
            std::cerr << "epsilon should not be required\n";
            return 1;
        }
        if (!p1.has_min || !NearlyEqual(p1.min_value, 1e-12, 1e-14)) {
            std::cerr << "epsilon min mismatch\n";
            return 1;
        }
        if (!p1.has_applicable_when || p1.applicable_when.op != "in") {
            std::cerr << "epsilon should have applicable_when with op=in\n";
            return 1;
        }

        const auto& p2 = desc.parameters[2];
        if (p2.path != "fractal.params.ripple_amplitude" || p2.type != "float") {
            std::cerr << "Third param should be ripple_amplitude float\n";
            return 1;
        }
        if (!p2.has_applicable_when || p2.applicable_when.op != "eq" || p2.applicable_when.value != "explaino_ripple") {
            std::cerr << "ripple_amplitude should keep the explaino_ripple applicable_when predicate\n";
            return 1;
        }
        if (!p2.has_cost_hint || !NearlyEqual(p2.cost_hint, 2.55, 1.0e-6)) {
            std::cerr << "ripple_amplitude should expose the measured relative cost hint\n";
            return 1;
        }
        if (!p2.has_sensitivity_report) {
            std::cerr << "ripple_amplitude should expose a sensitivity report\n";
            return 1;
        }
        if (p2.sensitivity_report.zero_case_id != "explaino_ripple_zero" ||
            p2.sensitivity_report.default_case_id != "explaino_ripple_default") {
            std::cerr << "ripple_amplitude sensitivity report should carry the phase-2 case ids\n";
            return 1;
        }
        if (p2.sensitivity_report.samples.size() != 5) {
            std::cerr << "ripple_amplitude sensitivity report should contain 5 sweep samples\n";
            return 1;
        }
        if (!NearlyEqual(p2.sensitivity_report.samples.front().param_value, 0.0) ||
            !NearlyEqual(p2.sensitivity_report.samples.back().param_value, 0.15)) {
            std::cerr << "ripple_amplitude sensitivity sweep should span zero to shipped default\n";
            return 1;
        }

        // outputs
        if (desc.outputs.size() != 7) {
            std::cerr << "Expected 7 outputs, got: " << desc.outputs.size() << "\n";
            return 1;
        }
        if (desc.outputs[0].name != "iterations" || desc.outputs[0].type != "int") {
            std::cerr << "First output should be iterations\n";
            return 1;
        }
        if (desc.outputs[1].name != "status" || desc.outputs[1].type != "enum") {
            std::cerr << "Second output should be status enum\n";
            return 1;
        }
        if (desc.outputs[5].name != "residual" || !desc.outputs[5].nullable) {
            std::cerr << "residual output should be nullable\n";
            return 1;
        }

        // summary_metrics
        if (desc.summary_metrics.size() != 6) {
            std::cerr << "Expected 6 summary metrics, got: " << desc.summary_metrics.size() << "\n";
            return 1;
        }
        if (desc.summary_metrics[5].name != "best_sequence_index") {
            std::cerr << "Expected best_sequence_index as the final summary metric\n";
            return 1;
        }
    }

    // --- Test 2: BuildEngineCatalog and serialize to JSON ---
    {
        UISchema schema;
        schema.schema_version = "1";
        schema.name_space = "fractal";
        UISchemaPanel panel;
        panel.id = "view";
        UISchemaControl c;
        c.id = "fractal_type";
        c.type = "combo";
        c.label = "Fractal Type";
        c.value_type = "enum";
        c.has_binding = true;
        c.binding.kind = "param";
        c.binding.path = "fractal.view.fractal_type";
        c.options.push_back({"newton", "Newton"});
        c.has_default = true;
        c.def.v = std::string("newton");
        panel.controls.push_back(c);
        schema.panels.push_back(panel);

        UISchemaPanel paramsPanel;
        paramsPanel.id = "params";
        UISchemaControl ripple;
        ripple.id = "ripple_amplitude";
        ripple.type = "drag_float";
        ripple.label = "Ripple Amplitude";
        ripple.value_type = "float";
        ripple.has_binding = true;
        ripple.binding.kind = "param";
        ripple.binding.path = "fractal.params.ripple_amplitude";
        ripple.has_default = true;
        ripple.def.v = 0.15;
        paramsPanel.controls.push_back(ripple);
        schema.panels.push_back(paramsPanel);

        EngineFunctionCatalog catalog = BuildEngineCatalog(schema);
        if (catalog.engine_version != 1 || catalog.functions.size() != 2) {
            std::cerr << "Catalog should have version 1 and 2 functions, got " << catalog.functions.size() << "\n";
            return 1;
        }

        std::string json = SerializeEngineCatalogJson(catalog);
        auto parsed = json_min::Parse(json);
        if (!parsed.error.empty()) {
            std::cerr << "Catalog JSON should parse: " << parsed.error << "\n";
            return 1;
        }
        const auto* functions = parsed.value.get("functions");
        if (!functions || !functions->is_array() || functions->as_array().size() != 2) {
            std::cerr << "Expected two functions in catalog JSON\n";
            return 1;
        }
        const auto& func = functions->as_array()[0];
        const auto* fid = func.get("id");
        if (!fid || !fid->is_string() || fid->as_string() != "fractal.sample") {
            std::cerr << "Expected function id fractal.sample in JSON\n";
            return 1;
        }
        const auto* params = func.get("parameters");
        if (!params || !params->is_array() || params->as_array().size() != 2) {
            std::cerr << "Expected 2 parameters in catalog JSON\n";
            return 1;
        }
        const auto& rippleParam = params->as_array()[1];
        const auto* costHint = rippleParam.get("cost_hint");
        if (!costHint || !costHint->is_number() || !NearlyEqual(costHint->as_number(), 2.55, 1.0e-6)) {
            std::cerr << "Expected ripple cost_hint in catalog JSON\n";
            return 1;
        }
        const auto* sensitivity = rippleParam.get("sensitivity");
        if (!sensitivity || !sensitivity->is_object()) {
            std::cerr << "Expected ripple sensitivity object in catalog JSON\n";
            return 1;
        }
        const auto* points = sensitivity->get("points");
        if (!points || !points->is_array() || points->as_array().size() != 5) {
            std::cerr << "Expected 5 ripple sensitivity points in catalog JSON\n";
            return 1;
        }
        const auto* zeroCaseId = sensitivity->get("zero_case_id");
        const auto* defaultCaseId = sensitivity->get("default_case_id");
        if (!zeroCaseId || !zeroCaseId->is_string() || zeroCaseId->as_string() != "explaino_ripple_zero" ||
            !defaultCaseId || !defaultCaseId->is_string() || defaultCaseId->as_string() != "explaino_ripple_default") {
            std::cerr << "Expected ripple phase-2 case ids in catalog JSON\n";
            return 1;
        }
        const auto* outputs = func.get("outputs");
        if (!outputs || !outputs->is_array() || outputs->as_array().size() != 7) {
            std::cerr << "Expected 7 outputs in catalog JSON\n";
            return 1;
        }
        const auto* summaryMetrics = func.get("summary_metrics");
        if (!summaryMetrics || !summaryMetrics->is_array() || summaryMetrics->as_array().size() != 6) {
            std::cerr << "Expected 6 summary_metrics in catalog JSON\n";
            return 1;
        }
    }

    // --- Test 3: built-in callable registration is the function-id authority ---
    {
        const EngineFunctionRegistration* fractalRegistration = FindEngineFunctionRegistration("fractal.sample");
        if (!fractalRegistration) {
            std::cerr << "Expected fractal.sample to be present in the built-in callable registry\n";
            return 1;
        }
        if (fractalRegistration->execution_kind != EngineFunctionExecutionKind::fractal_sampler) {
            std::cerr << "Expected fractal.sample to dispatch through the fractal sampler kind\n";
            return 1;
        }
        if (!fractalRegistration->descriptor_builder) {
            std::cerr << "Expected fractal.sample to provide a descriptor builder callback\n";
            return 1;
        }

        const EngineFunctionRegistration* genericRegistration = FindEngineFunctionRegistration("generic.sample");
        if (!genericRegistration) {
            std::cerr << "Expected generic.sample to be present in the built-in callable registry\n";
            return 1;
        }
        if (genericRegistration->execution_kind != EngineFunctionExecutionKind::generic_sampler) {
            std::cerr << "Expected generic.sample to dispatch through the generic sampler kind\n";
            return 1;
        }
        if (!genericRegistration->descriptor_builder) {
            std::cerr << "Expected generic.sample to provide a descriptor builder callback\n";
            return 1;
        }

        if (FindEngineFunctionRegistration("nonexistent.function") != nullptr) {
            std::cerr << "Unknown function ids must not resolve through the built-in callable registry\n";
            return 1;
        }

        if (DescribeRegisteredEngineFunctionIds() != "fractal.sample, generic.sample") {
            std::cerr << "Expected a deterministic built-in callable id listing\n";
            return 1;
        }

        UISchema schema;
        schema.schema_version = "1";
        schema.name_space = "fractal";
        UISchemaPanel panel;
        panel.id = "view";
        UISchemaControl control;
        control.id = "fractal_type";
        control.type = "combo";
        control.label = "Fractal Type";
        control.value_type = "enum";
        control.has_binding = true;
        control.binding.kind = "param";
        control.binding.path = "fractal.view.fractal_type";
        control.options.push_back({"newton", "Newton"});
        panel.controls.push_back(control);
        schema.panels.push_back(panel);

        EngineFunctionCatalog catalog = BuildEngineCatalog(schema);
        if (catalog.functions.size() != 2) {
            std::cerr << "Expected BuildEngineCatalog to emit every registered callable function\n";
            return 1;
        }

        FunctionDescriptor fractalDescriptor = fractalRegistration->descriptor_builder(schema);
        if (fractalDescriptor.id != "fractal.sample") {
            std::cerr << "Expected fractal registry builder to emit the fractal sampler descriptor\n";
            return 1;
        }

        FunctionDescriptor genericDescriptor = genericRegistration->descriptor_builder(schema);
        if (genericDescriptor.id != "generic.sample") {
            std::cerr << "Expected generic registry builder to emit the generic sampler descriptor\n";
            return 1;
        }

        for (const auto& function : catalog.functions) {
            if (!FindEngineFunctionRegistration(function.id)) {
                std::cerr << "Catalog emitted function without a matching built-in registration: " << function.id << "\n";
                return 1;
            }
        }
    }

    // --- Test 4: function_id in probe request parsing ---
    {
        // With explicit function_id
        const std::string withFid = R"({
  "request_version": 1,
  "request_id": "fid-test",
  "function_id": "fractal.sample",
  "mode": "point_set",
  "points": [{ "x": 0.0, "y": 0.0 }]
})";
        FractalProbeRequest req1{};
        std::string error;
        if (!ParseFractalProbeRequestJson(withFid, &req1, &error)) {
            std::cerr << "Expected request with function_id to parse: " << error << "\n";
            return 1;
        }
        if (req1.function_id != "fractal.sample") {
            std::cerr << "Expected function_id=fractal.sample, got: " << req1.function_id << "\n";
            return 1;
        }

        // Without function_id (should default to fractal.sample)
        const std::string withoutFid = R"({
  "request_version": 1,
  "request_id": "fid-default-test",
  "mode": "point_set",
  "points": [{ "x": 0.0, "y": 0.0 }]
})";
        FractalProbeRequest req2{};
        if (!ParseFractalProbeRequestJson(withoutFid, &req2, &error)) {
            std::cerr << "Expected request without function_id to parse: " << error << "\n";
            return 1;
        }
        if (req2.function_id != "fractal.sample") {
            std::cerr << "Expected default function_id=fractal.sample, got: " << req2.function_id << "\n";
            return 1;
        }
    }

    // --- Test 5: unknown function_id fails fast ---
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "bad-fid";
        request.function_id = "nonexistent.function";
        request.mode = FractalProbeMode::point_set;
        request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String("newton")});
        request.points.push_back({0.0, 0.0});

        FractalProbeResponse response{};
        std::string error;
        if (RunFractalProbeRequest(request, "test.exe", &response, &error)) {
            std::cerr << "Expected unknown function_id to fail\n";
            return 1;
        }
        if (error.find("nonexistent.function") == std::string::npos) {
            std::cerr << "Expected error to mention the unknown function_id\n";
            return 1;
        }
        if (error.find("fractal.sample, generic.sample") == std::string::npos) {
            std::cerr << "Expected error to advertise the registered callable ids\n";
            return 1;
        }
    }

    // --- Test 6: function_id echoed in response ---
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "echo-fid";
        request.function_id = "fractal.sample";
        request.mode = FractalProbeMode::point_set;
        request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String("mandelbrot")});
        request.points.push_back({0.0, 0.0});

        FractalProbeResponse response{};
        std::string error;
        if (!RunFractalProbeRequest(request, "test.exe", &response, &error)) {
            std::cerr << "Expected fractal.sample probe to run: " << error << "\n";
            return 1;
        }
        if (response.function_id != "fractal.sample") {
            std::cerr << "Expected function_id=fractal.sample in response, got: " << response.function_id << "\n";
            return 1;
        }

        // Check serialized response contains function_id
        std::string json = SerializeFractalProbeResponseJson(response);
        auto parsed = json_min::Parse(json);
        if (!parsed.error.empty()) {
            std::cerr << "Response JSON parse error: " << parsed.error << "\n";
            return 1;
        }
        const auto* fidVal = parsed.value.get("function_id");
        if (!fidVal || !fidVal->is_string() || fidVal->as_string() != "fractal.sample") {
            std::cerr << "Serialized response should contain function_id=fractal.sample\n";
            return 1;
        }
    }

    // --- Test 7: summary-only metrics omit sample payload and unrequested summary fields ---
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "summary-only-metrics";
        request.function_id = "fractal.sample";
        request.mode = FractalProbeMode::point_set;
        request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String("mandelbrot")});
        request.points.push_back({-0.75, 0.0});
        request.points.push_back({0.25, 0.0});
        request.metrics = {"summary_mean_iterations", "summary_escape_fraction"};

        FractalProbeResponse response{};
        std::string error;
        if (!RunFractalProbeRequest(request, "test.exe", &response, &error)) {
            std::cerr << "Expected summary-only metrics probe to run: " << error << "\n";
            return 1;
        }
        if (!response.samples.empty()) {
            std::cerr << "Summary-only metrics should not retain per-sample payloads\n";
            return 1;
        }

        const std::string json = SerializeFractalProbeResponseJson(response);
        auto parsed = json_min::Parse(json);
        if (!parsed.error.empty()) {
            std::cerr << "Summary-only response JSON parse error: " << parsed.error << "\n";
            return 1;
        }
        const auto* summary = parsed.value.get("summary");
        if (!summary || !summary->is_object()) {
            std::cerr << "Serialized response should contain a summary object\n";
            return 1;
        }
        if (!summary->get("sample_count") || !summary->get("mean_iterations") || !summary->get("escape_fraction")) {
            std::cerr << "Summary should contain sample_count plus requested summary metrics\n";
            return 1;
        }
        if (summary->get("converged_fraction") || summary->get("nonfinite_fraction") ||
            summary->get("pole_fraction") || summary->get("best_sequence_index")) {
            std::cerr << "Summary should omit unrequested summary metrics\n";
            return 1;
        }
        const auto* samples = parsed.value.get("samples");
        if (!samples || !samples->is_array() || !samples->as_array().empty()) {
            std::cerr << "Summary-only metrics should serialize an empty samples array\n";
            return 1;
        }
        const auto* sequenceResults = parsed.value.get("sequence_results");
        if (!sequenceResults || !sequenceResults->is_array() || sequenceResults->as_array().size() != 1) {
            std::cerr << "Expected one implicit sequence result for summary-only metrics\n";
            return 1;
        }
        const auto* sequenceSummary = sequenceResults->as_array()[0].get("summary");
        if (!sequenceSummary || !sequenceSummary->is_object()) {
            std::cerr << "Sequence result should contain a summary object\n";
            return 1;
        }
        if (!sequenceSummary->get("mean_iterations") || !sequenceSummary->get("escape_fraction")) {
            std::cerr << "Sequence summary should contain requested summary metrics\n";
            return 1;
        }
        if (sequenceSummary->get("converged_fraction") || sequenceSummary->get("pole_fraction")) {
            std::cerr << "Sequence summary should omit unrequested metrics\n";
            return 1;
        }
    }

    // --- Test 8: sample metric subsets omit unrequested sample fields ---
    {
        FractalProbeRequest request{};
        request.request_version = 1;
        request.request_id = "sample-metric-subset";
        request.function_id = "fractal.sample";
        request.mode = FractalProbeMode::point_set;
        request.overrides.push_back({"fractal.view.fractal_type", FractalProbeScalar::String("mandelbrot")});
        request.points.push_back({0.0, 0.0});
        request.metrics = {"iterations", "status", "summary_mean_iterations"};

        FractalProbeResponse response{};
        std::string error;
        if (!RunFractalProbeRequest(request, "test.exe", &response, &error)) {
            std::cerr << "Expected sample-metric subset probe to run: " << error << "\n";
            return 1;
        }
        if (response.samples.size() != 1) {
            std::cerr << "Expected sample-metric subset probe to keep one sample\n";
            return 1;
        }

        const std::string json = SerializeFractalProbeResponseJson(response);
        auto parsed = json_min::Parse(json);
        if (!parsed.error.empty()) {
            std::cerr << "Sample-metric subset response JSON parse error: " << parsed.error << "\n";
            return 1;
        }
        const auto* samples = parsed.value.get("samples");
        if (!samples || !samples->is_array() || samples->as_array().size() != 1) {
            std::cerr << "Expected one serialized sample for sample-metric subset probe\n";
            return 1;
        }
        const json_min::Value& sample = samples->as_array()[0];
        if (!sample.get("coord_x") || !sample.get("coord_y") || !sample.get("iterations") || !sample.get("status")) {
            std::cerr << "Serialized sample should include structural coordinates plus requested metrics\n";
            return 1;
        }
        if (sample.get("final_z_x") || sample.get("final_z_y") || sample.get("final_abs2") ||
            sample.get("residual") || sample.get("root_index")) {
            std::cerr << "Serialized sample should omit unrequested metrics\n";
            return 1;
        }
    }

    std::cout << "test_function_descriptor: all passed\n";
    return 0;
}
