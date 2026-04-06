#include "function_descriptor.h"
#include "fractal_probe_runner.h"

#include <cctype>
#include <iomanip>
#include <sstream>
#include <vector>

namespace {

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

FunctionDescriptor BuildFractalSamplerDescriptor(const UISchema& schema) {
    FunctionDescriptor desc;
    desc.id = "fractal.sample";
    desc.name = "Fractal Point Sampler";
    desc.description = "Sample any registered fractal family over a parameter domain.";

    // Extract parameters from the UI schema controls. Only include controls
    // that have a param binding (not action bindings). The schema is the
    // metadata authority.
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

            // fractal_type is required; all others are optional
            if (param.path == "fractal.view.fractal_type") {
                param.required = true;
                param.options = FilterProbeSupportedFractalTypeOptions(param.options);
            }

            desc.parameters.push_back(std::move(param));
        }
    }

    // Standard output fields for all fractal samples.
    desc.outputs.push_back({"iterations", "int", false, {}});
    desc.outputs.push_back({"status", "enum", false, {
        {"escaped", "Escaped"},
        {"converged", "Converged"},
        {"bounded", "Bounded"},
        {"pole", "Pole"},
        {"nonfinite", "Non-finite"},
        {"invalid_param", "Invalid Parameter"},
    }});
    desc.outputs.push_back({"final_z_x", "double", false, {}});
    desc.outputs.push_back({"final_z_y", "double", false, {}});
    desc.outputs.push_back({"final_abs2", "double", false, {}});
    desc.outputs.push_back({"residual", "double", true, {}});
    desc.outputs.push_back({"root_index", "int", true, {}});

    // Summary metrics.
    desc.summary_metrics.push_back({"mean_iterations"});
    desc.summary_metrics.push_back({"escape_fraction"});
    desc.summary_metrics.push_back({"converged_fraction"});
    desc.summary_metrics.push_back({"nonfinite_fraction"});
    desc.summary_metrics.push_back({"pole_fraction"});
    desc.summary_metrics.push_back({"best_sequence_index"});

    return desc;
}

EngineFunctionCatalog BuildEngineCatalog(const UISchema& schema) {
    EngineFunctionCatalog catalog;
    catalog.engine_version = 1;
    catalog.functions.push_back(BuildFractalSamplerDescriptor(schema));
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
