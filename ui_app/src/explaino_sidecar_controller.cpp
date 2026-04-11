#include "explaino_sidecar_controller.h"

#include <cmath>

namespace {

bool ValidateControllerPolicy(
    const SidecarAutoDemoControllerPolicy& policy,
    std::string* outError) {
    if (!std::isfinite(policy.stop_demonstrated_fraction) ||
        policy.stop_demonstrated_fraction < 0.0 ||
        policy.stop_demonstrated_fraction > 1.0) {
        if (outError) *outError = "Invalid sidecar auto-demo stop_demonstrated_fraction";
        return false;
    }
    if (policy.stop_uncertain_count < 0) {
        if (outError) *outError = "Invalid sidecar auto-demo stop_uncertain_count";
        return false;
    }
    return true;
}

bool ValidateCompleteness(
    const SidecarExplorationCompleteness& completeness,
    std::string* outError) {
    if (completeness.function_id.empty()) {
        if (outError) *outError = "Sidecar auto-demo requires completeness function_id";
        return false;
    }
    if (!std::isfinite(completeness.demonstrated_fraction) ||
        completeness.demonstrated_fraction < 0.0 ||
        completeness.demonstrated_fraction > 1.0) {
        if (outError) *outError = "Sidecar auto-demo requires demonstrated_fraction in [0,1]";
        return false;
    }
    if (completeness.demonstrated_count < 0 || completeness.uncertain_count < 0) {
        if (outError) *outError = "Sidecar auto-demo requires non-negative completeness counts";
        return false;
    }
    return true;
}

bool HasCompletenessStopPoint(
    const SidecarExplorationCompleteness& completeness,
    const SidecarAutoDemoControllerPolicy& policy) {
    return completeness.demonstrated_fraction >= policy.stop_demonstrated_fraction ||
        completeness.uncertain_count <= policy.stop_uncertain_count;
}

double ResolveTargetValue(const SidecarActionRecommendation& recommendation) {
    const bool plus = recommendation.guidance.find('+') != std::string::npos;
    const bool minus = recommendation.guidance.find('-') != std::string::npos;
    if (plus && !minus) {
        return recommendation.active_max;
    }
    if (minus && !plus) {
        return recommendation.active_min;
    }
    return 0.5 * (recommendation.active_min + recommendation.active_max);
}

} // namespace

bool BuildSidecarAutoDemoControllerDecision(
    const SidecarActionRecommendation* recommendation,
    const SidecarExplorationCompleteness& completeness,
    const SidecarAutoDemoControllerPolicy& policy,
    SidecarAutoDemoControllerDecision* outDecision,
    std::string* outError) {
    if (!outDecision) {
        if (outError) *outError = "BuildSidecarAutoDemoControllerDecision requires outDecision";
        return false;
    }
    if (!ValidateControllerPolicy(policy, outError) || !ValidateCompleteness(completeness, outError)) {
        *outDecision = {};
        return false;
    }

    SidecarAutoDemoControllerDecision next;
    next.demonstrated_fraction = completeness.demonstrated_fraction;
    next.uncertain_count = completeness.uncertain_count;

    if (!policy.enabled) {
        next.status = SidecarAutoDemoControllerStatus::disabled;
        next.summary = "disabled";
        next.reason = "auto-demonstration requires explicit enablement";
        *outDecision = std::move(next);
        return true;
    }

    if (HasCompletenessStopPoint(completeness, policy)) {
        next.status = SidecarAutoDemoControllerStatus::stopped_complete;
        next.summary = "stopped";
        next.reason = "completeness stop condition reached";
        *outDecision = std::move(next);
        return true;
    }

    if (!recommendation || recommendation->path.empty()) {
        next.status = SidecarAutoDemoControllerStatus::blocked_no_action;
        next.summary = "blocked";
        next.reason = "no eligible passive recommendation";
        *outDecision = std::move(next);
        return true;
    }
    if (!std::isfinite(recommendation->utility)) {
        *outDecision = {};
        if (outError) *outError = "Sidecar auto-demo recommendation utility must be finite";
        return false;
    }
    if (!(recommendation->utility > 0.0)) {
        next.status = SidecarAutoDemoControllerStatus::blocked_no_action;
        next.summary = "blocked";
        next.reason = "recommendation utility is not positive";
        *outDecision = std::move(next);
        return true;
    }
    if (!std::isfinite(recommendation->active_min) || !std::isfinite(recommendation->active_max) ||
        recommendation->active_min > recommendation->active_max) {
        *outDecision = {};
        if (outError) *outError = "Sidecar auto-demo recommendation active zone is invalid";
        return false;
    }

    next.label = recommendation->label;
    next.path = recommendation->path;
    next.type = recommendation->type;
    next.guidance = recommendation->guidance;
    next.utility = recommendation->utility;
    next.target_value = ResolveTargetValue(*recommendation);
    next.has_target_value = true;

    if (!policy.allow_runtime_mutation) {
        next.status = SidecarAutoDemoControllerStatus::proposal_ready;
        next.summary = "proposal ready";
        next.reason = "runtime mutation requires explicit opt-in";
        *outDecision = std::move(next);
        return true;
    }

    next.status = SidecarAutoDemoControllerStatus::apply_ready;
    next.summary = "apply ready";
    next.reason = "eligible auto-demonstration step armed";
    next.should_mutate = true;
    *outDecision = std::move(next);
    return true;
}