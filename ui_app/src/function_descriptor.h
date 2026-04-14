#pragma once

#include "ui_schema.h"
#include "json_min.h"

#include <string>
#include <vector>

// A measured sensitivity sample for a described parameter.
struct FunctionParamSensitivitySample {
    double t = 0.0;
    double param_value = 0.0;
    double mean_iterations = 0.0;
    double escape_fraction = 0.0;
    double converged_fraction = 0.0;
    double nonfinite_fraction = 0.0;
    double pole_fraction = 0.0;
    double root_entropy_bits = 0.0;
};

// A measured multi-metric sensitivity report for a described parameter.
struct FunctionParamSensitivityReport {
    std::string baseline_case_id;
    std::string zero_case_id;
    std::string default_case_id;
    std::vector<FunctionParamSensitivitySample> samples;
};

// A described parameter for a callable engine function.
struct FunctionParamDescriptor {
    std::string path;
    std::string type; // float, double, int, bool, enum
    std::string label;
    std::string help;
    bool has_min = false;
    double min_value = 0.0;
    bool has_max = false;
    double max_value = 0.0;
    bool has_step = false;
    double step_value = 0.0;
    bool has_default = false;
    json_min::Value default_value;
    bool required = false;
    bool has_cost_hint = false;
    double cost_hint = 0.0;
    bool has_sensitivity_report = false;
    FunctionParamSensitivityReport sensitivity_report;

    std::vector<UISchemaOption> options; // for enum types

    bool has_applicable_when = false;
    UISchemaPredicate applicable_when;
};

// A described output metric for a callable engine function.
struct FunctionOutputDescriptor {
    std::string name;
    std::string type; // int, double, enum, string
    bool nullable = false;
    std::vector<UISchemaOption> options; // for enum output types
};

// A described summary metric name.
struct FunctionSummaryMetric {
    std::string name;
};

// A fully described callable engine function.
struct FunctionDescriptor {
    std::string id;
    std::string name;
    std::string description;
    std::vector<FunctionParamDescriptor> parameters;
    std::vector<FunctionOutputDescriptor> outputs;
    std::vector<FunctionSummaryMetric> summary_metrics;
};

// Top-level engine capability catalog.
struct EngineFunctionCatalog {
    int engine_version = 1;
    std::vector<FunctionDescriptor> functions;
};

enum class EngineFunctionExecutionKind {
    fractal_sampler,
    generic_sampler,
};

using EngineFunctionDescriptorBuilder = FunctionDescriptor (*)(const UISchema& schema);

// A built-in callable registration that acts as the authority for known
// function ids and their execution path.
struct EngineFunctionRegistration {
    const char* id = "";
    EngineFunctionExecutionKind execution_kind = EngineFunctionExecutionKind::fractal_sampler;
    EngineFunctionDescriptorBuilder descriptor_builder = nullptr;
};

// Return whether the given fractal type id is currently sampleable through the
// probe surface.
bool IsProbeSamplingImplementedForFractalTypeId(const std::string& fractalTypeId);

// Build a function descriptor for the built-in fractal sampler from the UI
// schema. The schema is the metadata authority; this function derives the
// descriptor from it rather than inventing a second source of truth.
FunctionDescriptor BuildFractalSamplerDescriptor(const UISchema& schema);

// Build a function descriptor for the generic function sampler.
FunctionDescriptor BuildGenericSamplerDescriptor();

// Resolve a built-in callable registration by function id.
const EngineFunctionRegistration* FindEngineFunctionRegistration(const std::string& functionId);

// Return the deterministic list of built-in callable ids for error messages.
std::string DescribeRegisteredEngineFunctionIds();

// Resolve a described function from a built catalog.
const FunctionDescriptor* FindFunctionDescriptor(const EngineFunctionCatalog& catalog, const std::string& functionId);

// Build the full engine catalog.
EngineFunctionCatalog BuildEngineCatalog(const UISchema& schema);

// Serialize the catalog to JSON text.
std::string SerializeEngineCatalogJson(const EngineFunctionCatalog& catalog);
