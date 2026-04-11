#pragma once

#include "explaino_sidecar_budget.h"
#include "explaino_sidecar_lens.h"
#include "explaino_sidecar_measurement.h"
#include "explaino_sidecar_model.h"

#include <string>
#include <vector>

struct ExplainoSidecarWindowRow {
    std::string label;
    std::string path;
    std::string type;
    std::string range_text;
    std::string default_text;
};

struct ExplainoSidecarWindowState {
    std::string title{"Explaino Sidecar"};
    std::string function_id;
    std::string fractal_type_id;
    std::string error_message;
    std::string measurement_error_message;
    SidecarOrientationVector orientation{};
    SidecarMeasurementBatch measurement{};
    SidecarBudgetState budget{};
    SidecarLensProjection lens{};
    std::vector<ExplainoSidecarWindowRow> rows;
};

bool BuildExplainoSidecarWindowState(
    const EngineFunctionCatalog& catalog,
    const BindingContext& ctx,
    const SidecarMeasurementHost* measurementHost,
    const SidecarBudgetState* previousBudget,
    ExplainoSidecarWindowState* outState,
    std::string* outError);

bool BuildExplainoSidecarWindowState(
    const EngineFunctionCatalog& catalog,
    const BindingContext& ctx,
    const SidecarMeasurementHost* measurementHost,
    ExplainoSidecarWindowState* outState,
    std::string* outError);

bool BuildExplainoSidecarWindowState(
    const EngineFunctionCatalog& catalog,
    const BindingContext& ctx,
    ExplainoSidecarWindowState* outState,
    std::string* outError);

void RenderExplainoSidecarWindow(const ExplainoSidecarWindowState& state);