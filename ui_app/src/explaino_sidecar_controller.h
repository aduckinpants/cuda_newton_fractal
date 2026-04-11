#pragma once

#include "explaino_sidecar_action.h"
#include "explaino_sidecar_completeness.h"

#include <string>

enum class SidecarAutoDemoControllerStatus {
    disabled,
    blocked_no_action,
    stopped_complete,
    proposal_ready,
    apply_ready,
};

struct SidecarAutoDemoControllerPolicy {
    bool enabled{false};
    bool allow_runtime_mutation{false};
    double stop_demonstrated_fraction{1.0};
    int stop_uncertain_count{0};
};

struct SidecarAutoDemoControllerDecision {
    SidecarAutoDemoControllerStatus status{SidecarAutoDemoControllerStatus::disabled};
    std::string summary;
    std::string reason;
    std::string label;
    std::string path;
    std::string type;
    std::string guidance;
    double utility{0.0};
    double target_value{0.0};
    bool has_target_value{false};
    bool should_mutate{false};
    double demonstrated_fraction{1.0};
    int uncertain_count{0};
};

bool BuildSidecarAutoDemoControllerDecision(
    const SidecarActionRecommendation* recommendation,
    const SidecarExplorationCompleteness& completeness,
    const SidecarAutoDemoControllerPolicy& policy,
    SidecarAutoDemoControllerDecision* outDecision,
    std::string* outError);