#pragma once

#include "explaino_sidecar_controller.h"
#include "explaino_sidecar_energy.h"

#include <string>
#include <vector>

struct SidecarSlimeTraceStep {
    int step_index{0};
    std::string label;
    std::string path;
    std::string type;
    std::string guidance;
    std::string summary;
    std::string reason;
    SidecarAutoDemoControllerStatus controller_status{SidecarAutoDemoControllerStatus::disabled};
    double energy{0.0};
    double utility{0.0};
    double target_value{0.0};
    bool has_target_value{false};
    bool should_mutate{false};
};

struct SidecarSlimeTraceVisitCount {
    std::string label;
    std::string path;
    int visit_count{0};
    int latest_step_index{0};
    bool latest_should_mutate{false};
    double latest_target_value{0.0};
};

struct SidecarSlimeTrace {
    std::string function_id;
    std::string fractal_type_id;
    int proposal_step_count{0};
    int apply_step_count{0};
    std::string latest_path;
    std::vector<SidecarSlimeTraceStep> steps;
    std::vector<SidecarSlimeTraceVisitCount> visit_counts;
};

bool BuildSidecarSlimeTrace(
    const SidecarEnergyLandscape& energyLandscape,
    const SidecarAutoDemoControllerDecision& controllerDecision,
    const SidecarSlimeTrace* previousTrace,
    SidecarSlimeTrace* outTrace,
    std::string* outError);