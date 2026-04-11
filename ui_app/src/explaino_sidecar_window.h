#pragma once

#include "explaino_sidecar_action.h"
#include "explaino_sidecar_budget.h"
#include "explaino_sidecar_completeness.h"
#include "explaino_sidecar_controller.h"
#include "explaino_sidecar_divergence.h"
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
    std::string action_error_message;
    std::string completeness_error_message;
    std::string controller_error_message;
    std::string divergence_error_message;
    bool has_orientation{false};
    SidecarOrientationVector orientation{};
    SidecarAutoDemoControllerPolicy controller_policy{};
    SidecarMeasurementBatch measurement{};
    SidecarBudgetState budget{};
    SidecarLensProjection lens{};
    SidecarExplorationCompleteness completeness{};
    bool has_action_recommendation{false};
    SidecarActionRecommendation action_recommendation{};
    SidecarAutoDemoControllerDecision controller_decision{};
    SidecarStateDivergence divergence{};
    std::vector<ExplainoSidecarWindowRow> rows;
};

bool BuildExplainoSidecarWindowState(
    const EngineFunctionCatalog& catalog,
    const BindingContext& ctx,
    const SidecarMeasurementHost* measurementHost,
    const SidecarBudgetState* previousBudget,
    const SidecarExplorationCompleteness* previousCompleteness,
    const SidecarAutoDemoControllerPolicy* controllerPolicy,
    ExplainoSidecarWindowState* outState,
    std::string* outError);

bool BuildExplainoSidecarWindowState(
    const EngineFunctionCatalog& catalog,
    const BindingContext& ctx,
    const SidecarMeasurementHost* measurementHost,
    const SidecarBudgetState* previousBudget,
    const SidecarExplorationCompleteness* previousCompleteness,
    const SidecarOrientationVector* previousOrientation,
    const SidecarAutoDemoControllerPolicy* controllerPolicy,
    ExplainoSidecarWindowState* outState,
    std::string* outError);

bool BuildExplainoSidecarWindowState(
    const EngineFunctionCatalog& catalog,
    const BindingContext& ctx,
    const SidecarMeasurementHost* measurementHost,
    const SidecarBudgetState* previousBudget,
    const SidecarExplorationCompleteness* previousCompleteness,
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