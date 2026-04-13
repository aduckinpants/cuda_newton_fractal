#pragma once

#include "explaino_sidecar_measurement.h"

#include <string>
#include <vector>

struct BindingContext;
struct EngineFunctionCatalog;
struct ExplainoSidecarWindowState;

struct ExplainoExplorationAdvisorObservation {
    int rank{0};
    std::string label;
    std::string path;
    std::string type;
    std::string guidance;
    std::string summary;
    std::string reason;
    double current_value{0.0};
    double estimated_information_gain{0.0};
    double effective_information_gain{0.0};
    double cumulative_information_gain{0.0};
    double posterior_uncertainty{1.0};
    double cost_hint{0.0};
    double utility{0.0};
    double active_min{0.0};
    double active_max{0.0};
    double active_fraction{0.0};
    int observation_count{0};
};

struct ExplainoExplorationAdvisorReport {
    int report_version{1};
    std::string function_id;
    std::string fractal_type;
    std::string fractal_type_id;
    int coordinate_count{0};
    int available_row_count{0};
    int recommendation_eligible_count{0};
    int demonstrated_count{0};
    int uncertain_count{0};
    double demonstrated_fraction{1.0};
    double estimated_information_gain_total{0.0};
    double cumulative_information_gain_total{0.0};
    double mean_posterior_uncertainty{1.0};
    bool has_recommended_observation{false};
    ExplainoExplorationAdvisorObservation recommended_observation{};
    std::vector<ExplainoExplorationAdvisorObservation> recommendations;
};

bool BuildExplainoExplorationAdvisorReport(
    const ExplainoSidecarWindowState& state,
    ExplainoExplorationAdvisorReport* outReport,
    std::string* outError);

bool BuildExplainoExplorationAdvisorReport(
    const EngineFunctionCatalog& catalog,
    const BindingContext& ctx,
    const SidecarMeasurementHost& measurementHost,
    ExplainoExplorationAdvisorReport* outReport,
    std::string* outError);

std::string SerializeExplainoExplorationAdvisorReportJson(
    const ExplainoExplorationAdvisorReport& report);