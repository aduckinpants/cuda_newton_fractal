#include "function_descriptor.h"
#include "enum_id_utils.h"
#include "fractal_catalog.h"
#include "fractal_family_rules.h"

#include <cctype>
#include <iomanip>
#include <sstream>
#include <vector>

namespace {

FunctionDescriptor BuildGenericSamplerDescriptorFromRegistry(const UISchema&) {
    return BuildGenericSamplerDescriptor();
}

constexpr EngineFunctionRegistration kRegisteredEngineFunctions[] = {
    {"fractal.sample", EngineFunctionExecutionKind::fractal_sampler, &BuildFractalSamplerDescriptor},
    {"generic.sample", EngineFunctionExecutionKind::generic_sampler, &BuildGenericSamplerDescriptorFromRegistry},
};

bool IsSupportedProbeFractalType(FractalType fractalType) {
    const FractalCatalogEntry* entry = FindFractalCatalogEntry(fractalType);
    return entry && HasFractalCatalogCapabilityFlag(*entry, FractalCatalogCapabilityFlag::sample_probe);
}

std::string TrimAscii(const std::string& value) {
    size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])) != 0) {
        ++start;
    }
    size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
        --end;
    }
    return value.substr(start, end - start);
}

std::vector<std::string> SplitCsvValues(const std::string& text) {
    std::vector<std::string> values;
    size_t start = 0;
    while (start <= text.size()) {
        const size_t comma = text.find(',', start);
        const size_t end = (comma == std::string::npos) ? text.size() : comma;
        const std::string item = TrimAscii(text.substr(start, end - start));
        if (!item.empty()) values.push_back(item);
        if (comma == std::string::npos) break;
        start = comma + 1;
    }
    return values;
}

std::string JoinCsvValues(const std::vector<std::string>& values) {
    std::ostringstream out;
    for (size_t index = 0; index < values.size(); ++index) {
        if (index > 0) out << ",";
        out << values[index];
    }
    return out.str();
}

struct FunctionParamSensitivitySampleData {
    double t;
    double param_value;
    double mean_iterations;
    double escape_fraction;
    double converged_fraction;
    double nonfinite_fraction;
    double pole_fraction;
    double root_entropy_bits;
};

struct FunctionParamSensitivityReportData {
    const char* path;
    double cost_hint;
    const char* baseline_case_id;
    const char* zero_case_id;
    const char* default_case_id;
    const FunctionParamSensitivitySampleData* samples;
    size_t sample_count;
};

struct FunctionParamCostHintData {
    const char* path;
    double cost_hint;
};

constexpr FunctionParamSensitivitySampleData kRippleSensitivity[] = {
    {0.0, 0.0, 9.84375, 0.0, 1.0, 0.0, 0.0, 1.6793929348105143},
    {0.25, 0.0375, 206.9921875, 0.0, 0.0, 0.0, 0.0, 0.0},
    {0.5, 0.075, 157.58203125, 0.0, 0.0, 0.0, 0.0, 0.0},
    {0.75, 0.1125, 241.0, 0.0, 0.0, 0.0, 0.0, 0.0},
    {1.0, 0.15, 250.29296875, 0.0, 0.0, 0.0, 0.0, 0.0},
};

constexpr FunctionParamSensitivitySampleData kSpliceSensitivity[] = {
    {0.0, 0.0, 9.84375, 0.0, 1.0, 0.0, 0.0, 1.6793929348105143},
    {0.25, 0.125, 9.1640625, 0.0, 0.0, 0.0, 0.0, 0.0},
    {0.5, 0.25, 7.0625, 0.0, 0.0, 0.0, 0.0, 0.0},
    {0.75, 0.375, 5.1015625, 0.0, 0.0, 0.0, 0.0, 0.0},
    {1.0, 0.5, 7.1796875, 0.0, 0.0, 0.0, 0.0, 0.0},
};

constexpr FunctionParamSensitivitySampleData kVortexSensitivity[] = {
    {0.0, 0.0, 9.84375, 0.0, 1.0, 0.0, 0.0, 1.6793929348105143},
    {0.25, 0.075, 10.4296875, 0.0, 1.0, 0.0, 0.0, 1.4227623437791515},
    {0.5, 0.15, 10.6796875, 0.0, 1.0, 0.0, 0.0, 1.2006223243127148},
    {0.75, 0.225, 11.2890625, 0.0, 1.0, 0.0, 0.0, 1.0659144123432416},
    {1.0, 0.3, 11.953125, 0.0, 1.0, 0.0, 0.0, 1.0},
};

constexpr FunctionParamSensitivitySampleData kTensionSensitivity[] = {
    {0.0, 0.0, 9.84375, 0.0, 1.0, 0.0, 0.0, 1.6793929348105143},
    {0.25, 0.005, 10.75, 0.0, 0.0, 0.0, 0.0, 0.0},
    {0.5, 0.01, 11.6796875, 0.0, 0.0, 0.0, 0.0, 0.0},
    {0.75, 0.015, 12.46875, 0.0, 0.0, 0.0, 0.0, 0.0},
    {1.0, 0.02, 14.015625, 0.0, 0.0, 0.0, 0.0, 0.0},
};

constexpr FunctionParamSensitivityReportData kOptimizationStagingSensitivity[] = {
    {"fractal.params.ripple_amplitude", 2.55, "explaino_baseline", "explaino_ripple_zero", "explaino_ripple_default", kRippleSensitivity, sizeof(kRippleSensitivity) / sizeof(kRippleSensitivity[0])},
    {"fractal.params.splice_offset", 2.01, "explaino_baseline", "explaino_splice_zero", "explaino_splice_default", kSpliceSensitivity, sizeof(kSpliceSensitivity) / sizeof(kSpliceSensitivity[0])},
    {"fractal.params.vortex_strength", 0.99, "explaino_baseline", "explaino_vortex_zero", "explaino_vortex_default", kVortexSensitivity, sizeof(kVortexSensitivity) / sizeof(kVortexSensitivity[0])},
    {"fractal.params.tension_strength", 2.42, "explaino_baseline", "explaino_tension_zero", "explaino_tension_default", kTensionSensitivity, sizeof(kTensionSensitivity) / sizeof(kTensionSensitivity[0])},
};

constexpr FunctionParamCostHintData kExplainoSidecarCostHints[] = {
    {"fractal.params.explaino_seed", 0.0},
    {"fractal.params.explaino_seed_b", 0.0},
    {"fractal.params.explaino_mix", 0.0},
    {"fractal.params.explaino_warp_strength", 0.0},
    {"fractal.params.explaino_root_spread", 0.0},
    {"fractal.view.explaino_phase_strength", 0.0},
    {"fractal.params.explaino_damping", 0.0},
    {"fractal.view.explaino_phase", 0.0},
    {"fractal.view.explaino_seed_drift", 0.0},
    {"fractal.params.explaino_cluster_radius", 0.0},
    {"fractal.params.momentum_beta", 0.0},
    {"fractal.params.joy_coupling", 0.0},
    {"fractal.params.fold_coupling", 0.0},
    {"fractal.params.bell_coupling", 0.0},
    {"fractal.params.balance_void", 0.0},
    {"fractal.params.symmetry_tension", 0.0},
    {"fractal.params.field_curvature", 0.0},
};

const FunctionParamSensitivityReportData* FindOptimizationStagingSensitivity(const std::string& path) {
    for (const auto& report : kOptimizationStagingSensitivity) {
        if (path == report.path) return &report;
    }
    return nullptr;
}

const FunctionParamCostHintData* FindExplainoSidecarCostHint(const std::string& path) {
    for (const auto& costHint : kExplainoSidecarCostHints) {
        if (path == costHint.path) return &costHint;
    }
    return nullptr;
}

void AttachOptimizationStagingMetadata(FunctionParamDescriptor* ioParam) {
    if (!ioParam) return;
    const FunctionParamSensitivityReportData* report = FindOptimizationStagingSensitivity(ioParam->path);
    if (!report) {
        const FunctionParamCostHintData* genericCostHint = FindExplainoSidecarCostHint(ioParam->path);
        if (!genericCostHint) {
            return;
        }
        ioParam->has_cost_hint = true;
        ioParam->cost_hint = genericCostHint->cost_hint;
        return;
    }

    ioParam->has_cost_hint = true;
    ioParam->cost_hint = report->cost_hint;
    ioParam->has_sensitivity_report = true;
    ioParam->sensitivity_report.baseline_case_id = report->baseline_case_id;
    ioParam->sensitivity_report.zero_case_id = report->zero_case_id;
    ioParam->sensitivity_report.default_case_id = report->default_case_id;
    ioParam->sensitivity_report.samples.clear();
    ioParam->sensitivity_report.samples.reserve(report->sample_count);
    for (size_t index = 0; index < report->sample_count; ++index) {
        const FunctionParamSensitivitySampleData& source = report->samples[index];
        FunctionParamSensitivitySample sample;
        sample.t = source.t;
        sample.param_value = source.param_value;
        sample.mean_iterations = source.mean_iterations;
        sample.escape_fraction = source.escape_fraction;
        sample.converged_fraction = source.converged_fraction;
        sample.nonfinite_fraction = source.nonfinite_fraction;
        sample.pole_fraction = source.pole_fraction;
        sample.root_entropy_bits = source.root_entropy_bits;
        ioParam->sensitivity_report.samples.push_back(sample);
    }
}

std::vector<UISchemaOption> FilterProbeSupportedFractalTypeOptions(const std::vector<UISchemaOption>& options) {
    std::vector<UISchemaOption> filtered;
    for (const auto& option : options) {
        if (!IsProbeSamplingImplementedForFractalTypeId(option.id)) continue;
        filtered.push_back(option);
    }
    return filtered;
}

bool FilterFractalTypePredicate(UISchemaPredicate* ioPredicate) {
    if (!ioPredicate) return true;
    if (ioPredicate->path != "fractal.view.fractal_type") return true;

    std::vector<std::string> values = SplitCsvValues(ioPredicate->value);
    if (values.empty()) return true;

    std::vector<std::string> filtered;
    for (const std::string& value : values) {
        if (IsProbeSamplingImplementedForFractalTypeId(value)) {
            filtered.push_back(value);
        }
    }
    if (filtered.empty()) return false;

    if (ioPredicate->op == "eq" && filtered.size() > 1) {
        ioPredicate->op = "in";
    }
    ioPredicate->value = JoinCsvValues(filtered);
    return true;
}

void EmitJsonString(std::ostringstream& out, const std::string& value) {
    out << '"';
    for (char c : value) {
        if (c == '"') out << "\\\"";
        else if (c == '\\') out << "\\\\";
        else if (c == '\n') out << "\\n";
        else out << c;
    }
    out << '"';
}

void EmitPredicate(std::ostringstream& out, const UISchemaPredicate& pred) {
    out << "{";
    out << "\"op\":";
    EmitJsonString(out, pred.op);
    out << ",\"path\":";
    EmitJsonString(out, pred.path);
    out << ",\"value\":";
    EmitJsonString(out, pred.value);
    out << "}";
}

void EmitSensitivitySample(std::ostringstream& out, const FunctionParamSensitivitySample& sample) {
    out << "{";
    out << "\"t\":" << std::setprecision(17) << sample.t;
    out << ",\"param_value\":" << std::setprecision(17) << sample.param_value;
    out << ",\"mean_iterations\":" << std::setprecision(17) << sample.mean_iterations;
    out << ",\"escape_fraction\":" << std::setprecision(17) << sample.escape_fraction;
    out << ",\"converged_fraction\":" << std::setprecision(17) << sample.converged_fraction;
    out << ",\"nonfinite_fraction\":" << std::setprecision(17) << sample.nonfinite_fraction;
    out << ",\"pole_fraction\":" << std::setprecision(17) << sample.pole_fraction;
    out << ",\"root_entropy_bits\":" << std::setprecision(17) << sample.root_entropy_bits;
    out << "}";
}

void EmitSensitivityReport(std::ostringstream& out, const FunctionParamSensitivityReport& report) {
    out << "{";
    out << "\"baseline_case_id\":";
    EmitJsonString(out, report.baseline_case_id);
    out << ",\"zero_case_id\":";
    EmitJsonString(out, report.zero_case_id);
    out << ",\"default_case_id\":";
    EmitJsonString(out, report.default_case_id);
    out << ",\"points\":[";
    for (size_t index = 0; index < report.samples.size(); ++index) {
        if (index > 0) out << ",";
        EmitSensitivitySample(out, report.samples[index]);
    }
    out << "]}";
}

void EmitParam(std::ostringstream& out, const FunctionParamDescriptor& param) {
    out << "{";
    out << "\"path\":";
    EmitJsonString(out, param.path);
    out << ",\"type\":";
    EmitJsonString(out, param.type);
    if (!param.label.empty()) {
        out << ",\"label\":";
        EmitJsonString(out, param.label);
    }
    if (!param.help.empty()) {
        out << ",\"help\":";
        EmitJsonString(out, param.help);
    }
    if (param.has_min) {
        out << ",\"min\":" << std::setprecision(17) << param.min_value;
    }
    if (param.has_max) {
        out << ",\"max\":" << std::setprecision(17) << param.max_value;
    }
    if (param.has_step) {
        out << ",\"step\":" << std::setprecision(17) << param.step_value;
    }
    if (param.has_default) {
        out << ",\"default\":";
        if (param.default_value.is_string()) {
            EmitJsonString(out, param.default_value.as_string());
        } else if (param.default_value.is_number()) {
            out << std::setprecision(17) << param.default_value.as_number();
        } else if (param.default_value.is_bool()) {
            out << (param.default_value.as_bool() ? "true" : "false");
        } else {
            out << "null";
        }
    }
    if (param.required) {
        out << ",\"required\":true";
    }
    if (param.has_cost_hint) {
        out << ",\"cost_hint\":" << std::setprecision(17) << param.cost_hint;
    }
    if (!param.options.empty()) {
        out << ",\"options\":[";
        for (size_t i = 0; i < param.options.size(); ++i) {
            if (i > 0) out << ",";
            out << "{\"id\":";
            EmitJsonString(out, param.options[i].id);
            out << ",\"label\":";
            EmitJsonString(out, param.options[i].label);
            out << "}";
        }
        out << "]";
    }
    if (param.has_applicable_when) {
        out << ",\"applicable_when\":";
        EmitPredicate(out, param.applicable_when);
    }
    if (param.has_sensitivity_report) {
        out << ",\"sensitivity\":";
        EmitSensitivityReport(out, param.sensitivity_report);
    }
    out << "}";
}

void EmitOutput(std::ostringstream& out, const FunctionOutputDescriptor& output) {
    out << "{";
    out << "\"name\":";
    EmitJsonString(out, output.name);
    out << ",\"type\":";
    EmitJsonString(out, output.type);
    if (output.nullable) {
        out << ",\"nullable\":true";
    }
    if (!output.options.empty()) {
        out << ",\"options\":[";
        for (size_t i = 0; i < output.options.size(); ++i) {
            if (i > 0) out << ",";
            out << "{\"id\":";
            EmitJsonString(out, output.options[i].id);
            out << ",\"label\":";
            EmitJsonString(out, output.options[i].label);
            out << "}";
        }
        out << "]";
    }
    out << "}";
}

} // namespace

bool IsProbeSamplingImplementedForFractalTypeId(const std::string& fractalTypeId) {
    FractalType fractalType = FractalType::newton;
    return TryParseFractalTypeId(fractalTypeId, &fractalType) && IsSupportedProbeFractalType(fractalType);
}

FunctionDescriptor BuildFractalSamplerDescriptor(const UISchema& schema) {
    FunctionDescriptor desc;
    desc.id = "fractal.sample";
    desc.name = "Fractal Point Sampler";
    desc.description = "Sample any registered fractal family over a parameter domain.";
    for (const auto& panel : schema.panels) {
        for (const auto& control : panel.controls) {
            if (!control.has_binding) continue;
            if (control.binding.kind != "param") continue;

            FunctionParamDescriptor param;
            param.path = control.binding.path;
            param.type = control.value_type;
            param.label = control.label;
            if (control.has_help) param.help = control.help;
            if (control.has_min) { param.has_min = true; param.min_value = control.min; }
            if (control.has_max) { param.has_max = true; param.max_value = control.max; }
            if (control.has_step) { param.has_step = true; param.step_value = control.step; }
            if (control.has_default) { param.has_default = true; param.default_value = control.def; }
            param.options = control.options;
            if (control.has_visible_if) {
                UISchemaPredicate filteredPredicate = control.visible_if;
                if (!FilterFractalTypePredicate(&filteredPredicate)) {
                    continue;
                }
                param.has_applicable_when = true;
                param.applicable_when = filteredPredicate;
            }
            if (param.path == "fractal.view.fractal_type") {
                param.required = true;
                param.options = FilterProbeSupportedFractalTypeOptions(param.options);
            }

            AttachOptimizationStagingMetadata(&param);

            desc.parameters.push_back(std::move(param));
        }
    }
    desc.outputs.push_back({"iterations", "int", false, {}});
    desc.outputs.push_back({"status", "enum", false, {
        {"escaped", "Escaped"},
        {"converged", "Converged"},
        {"bounded", "Bounded"},
        {"pole", "Pole"},
        {"nonfinite", "Non-finite"},
        {"invalid_param", "Invalid Parameter"},
    }});
    desc.outputs.push_back({"final_z_x", "double", true, {}});
    desc.outputs.push_back({"final_z_y", "double", true, {}});
    desc.outputs.push_back({"final_abs2", "double", true, {}});
    desc.outputs.push_back({"residual", "double", true, {}});
    desc.outputs.push_back({"root_index", "int", true, {}});
    desc.summary_metrics.push_back({"mean_iterations"});
    desc.summary_metrics.push_back({"escape_fraction"});
    desc.summary_metrics.push_back({"converged_fraction"});
    desc.summary_metrics.push_back({"nonfinite_fraction"});
    desc.summary_metrics.push_back({"pole_fraction"});
    desc.summary_metrics.push_back({"best_sequence_index"});

    return desc;
}

FunctionDescriptor BuildGenericSamplerDescriptor() {
    FunctionDescriptor desc;
    desc.id = "generic.sample";
    desc.name = "Generic Function Sampler";
    desc.description = "Evaluate a user-defined complex function at sample points.";

    // Outputs.
    desc.outputs.push_back({"iterations", "int", false, {}});
    desc.outputs.push_back({"status", "enum", false, {{"escaped"}, {"converged"}, {"bounded"}, {"nonfinite"}}});
    desc.outputs.push_back({"value_x", "double", true, {}});
    desc.outputs.push_back({"value_y", "double", true, {}});
    desc.outputs.push_back({"abs2", "double", true, {}});
    desc.outputs.push_back({"derivative_x", "double", true, {}});
    desc.outputs.push_back({"derivative_y", "double", true, {}});

    // Summary metrics.
    desc.summary_metrics.push_back({"mean_iterations"});
    desc.summary_metrics.push_back({"converged_fraction"});
    desc.summary_metrics.push_back({"escape_fraction"});
    desc.summary_metrics.push_back({"nonfinite_fraction"});
    desc.summary_metrics.push_back({"mean_abs2"});
    desc.summary_metrics.push_back({"diverged_fraction"});

    return desc;
}

const EngineFunctionRegistration* FindEngineFunctionRegistration(const std::string& functionId) {
    for (const auto& registration : kRegisteredEngineFunctions) {
        if (functionId == registration.id) return &registration;
    }
    return nullptr;
}

std::string DescribeRegisteredEngineFunctionIds() {
    std::ostringstream out;
    for (size_t index = 0; index < sizeof(kRegisteredEngineFunctions) / sizeof(kRegisteredEngineFunctions[0]); ++index) {
        if (index > 0) out << ", ";
        out << kRegisteredEngineFunctions[index].id;
    }
    return out.str();
}

const FunctionDescriptor* FindFunctionDescriptor(const EngineFunctionCatalog& catalog, const std::string& functionId) {
    for (const auto& function : catalog.functions) {
        if (function.id == functionId) return &function;
    }
    return nullptr;
}

EngineFunctionCatalog BuildEngineCatalog(const UISchema& schema) {
    EngineFunctionCatalog catalog;
    catalog.engine_version = 1;
    for (const auto& registration : kRegisteredEngineFunctions) {
        if (!registration.descriptor_builder) continue;
        catalog.functions.push_back(registration.descriptor_builder(schema));
    }
    return catalog;
}

std::string SerializeEngineCatalogJson(const EngineFunctionCatalog& catalog) {
    std::ostringstream out;
    out << std::setprecision(17);
    out << "{\n";
    out << "  \"engine_version\": " << catalog.engine_version << ",\n";
    out << "  \"functions\": [\n";
    for (size_t fi = 0; fi < catalog.functions.size(); ++fi) {
        const auto& func = catalog.functions[fi];
        if (fi > 0) out << ",\n";
        out << "    {\n";
        out << "      \"id\": ";
        EmitJsonString(out, func.id);
        out << ",\n";
        out << "      \"name\": ";
        EmitJsonString(out, func.name);
        out << ",\n";
        out << "      \"description\": ";
        EmitJsonString(out, func.description);
        out << ",\n";

        // Parameters
        out << "      \"parameters\": [\n";
        for (size_t pi = 0; pi < func.parameters.size(); ++pi) {
            if (pi > 0) out << ",\n";
            out << "        ";
            EmitParam(out, func.parameters[pi]);
        }
        out << "\n      ],\n";

        // Outputs
        out << "      \"outputs\": [\n";
        for (size_t oi = 0; oi < func.outputs.size(); ++oi) {
            if (oi > 0) out << ",\n";
            out << "        ";
            EmitOutput(out, func.outputs[oi]);
        }
        out << "\n      ],\n";

        // Summary metrics
        out << "      \"summary_metrics\": [";
        for (size_t mi = 0; mi < func.summary_metrics.size(); ++mi) {
            if (mi > 0) out << ", ";
            EmitJsonString(out, func.summary_metrics[mi].name);
        }
        out << "]\n";

        out << "    }";
    }
    out << "\n  ]\n";
    out << "}\n";
    return out.str();
}
