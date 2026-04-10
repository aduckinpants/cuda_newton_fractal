#pragma once

#include "ui_schema.h"
#include "json_min.h"

#include <string>
#include <vector>

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

// Build a function descriptor for the built-in fractal sampler from the UI
// schema. The schema is the metadata authority; this function derives the
// descriptor from it rather than inventing a second source of truth.
FunctionDescriptor BuildFractalSamplerDescriptor(const UISchema& schema);

// Build a function descriptor for the generic function sampler.
FunctionDescriptor BuildGenericSamplerDescriptor();

// Build the full engine catalog.
EngineFunctionCatalog BuildEngineCatalog(const UISchema& schema);

// Serialize the catalog to JSON text.
std::string SerializeEngineCatalogJson(const EngineFunctionCatalog& catalog);
