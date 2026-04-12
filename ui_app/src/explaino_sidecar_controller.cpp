#include "explaino_sidecar_controller.h"

#include "schema_binding.h"
#include "view_hp_sync.h"

#include <cmath>
#include <limits>

namespace {

std::string UnsupportedMutationTypePathPair(
    const SidecarAutoDemoControllerDecision& decision) {
    return "Unsupported sidecar auto-demo mutation type/path pair: " +
        decision.type + " -> " + decision.path;
}

bool ValidateControllerPolicy(
    const SidecarAutoDemoControllerPolicy& policy,
    std::string* outError) {
    if (!std::isfinite(policy.paced_loop_interval_seconds) ||
        policy.paced_loop_interval_seconds <= 0.0) {
        if (outError) *outError = "Invalid sidecar auto-demo paced_loop_interval_seconds";
        return false;
    }
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

bool PolicyEnablesPacedLoop(const SidecarAutoDemoControllerPolicy& policy) {
    return policy.enabled && policy.allow_runtime_mutation && policy.run_paced_loop;
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

bool MutationRequiresViewHpSync(const std::string& path) {
    return path == "fractal.view.center.x" ||
        path == "fractal.view.center.y" ||
        path == "fractal.view.zoom" ||
        path == "fractal.view.rotation";
}

bool DecisionMatchesLoopState(
    const SidecarAutoDemoControllerDecision& decision,
    const SidecarAutoDemoLoopState& state) {
    return decision.path == state.armed_path &&
        decision.type == state.armed_type &&
        decision.has_target_value == state.armed_has_target_value &&
        (!decision.has_target_value || decision.target_value == state.armed_target_value);
}

void SeedLoopState(
    const SidecarAutoDemoControllerDecision& decision,
    SidecarAutoDemoLoopState* ioState) {
    if (!ioState) {
        return;
    }
    ioState->armed_idle_seconds = 0.0;
    ioState->armed_path = decision.path;
    ioState->armed_type = decision.type;
    ioState->armed_target_value = decision.target_value;
    ioState->armed_has_target_value = decision.has_target_value;
}

void ResetLoopState(SidecarAutoDemoLoopState* ioState) {
    if (!ioState) {
        return;
    }
    *ioState = {};
}

bool ApplyFloatMutation(const SidecarAutoDemoControllerDecision& decision,
    BindingContext& ctx,
    std::string* outError) {
    float* value = nullptr;
    if (!ctx.BindFloat(decision.path, &value) || !value) {
        if (outError) *outError = UnsupportedMutationTypePathPair(decision);
        return false;
    }
    *value = static_cast<float>(decision.target_value);
    if (ctx.view && MutationRequiresViewHpSync(decision.path)) {
        SyncViewHpFromUi(*ctx.view);
    }
    return true;
}

bool ApplyDoubleMutation(const SidecarAutoDemoControllerDecision& decision,
    BindingContext& ctx,
    std::string* outError) {
    double* value = nullptr;
    if (!ctx.BindDouble(decision.path, &value) || !value) {
        if (outError) *outError = UnsupportedMutationTypePathPair(decision);
        return false;
    }
    *value = decision.target_value;
    return true;
}

bool ApplyIntMutation(const SidecarAutoDemoControllerDecision& decision,
    BindingContext& ctx,
    std::string* outError) {
    if (!std::isfinite(decision.target_value) ||
        decision.target_value < static_cast<double>(INT_MIN) ||
        decision.target_value > static_cast<double>(INT_MAX)) {
        if (outError) *outError = "Sidecar auto-demo target_value is out of int range";
        return false;
    }

    int* value = nullptr;
    if (!ctx.BindInt(decision.path, &value) || !value) {
        if (outError) *outError = UnsupportedMutationTypePathPair(decision);
        return false;
    }
    *value = static_cast<int>(std::lround(decision.target_value));
    return true;
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

bool ApplySidecarAutoDemoControllerDecision(
    const SidecarAutoDemoControllerDecision& decision,
    BindingContext& ctx,
    std::string* outError) {
    if (outError) outError->clear();

    if (decision.path.empty()) {
        if (outError) *outError = "Sidecar auto-demo mutation requires a bound path";
        return false;
    }
    if (!decision.has_target_value || !std::isfinite(decision.target_value)) {
        if (outError) *outError = "Sidecar auto-demo mutation requires a target_value";
        return false;
    }
    if (!decision.should_mutate) {
        if (outError) *outError = "Sidecar auto-demo mutation requires an armed mutation decision";
        return false;
    }

    if (decision.type == "int") {
        return ApplyIntMutation(decision, ctx, outError);
    }
    if (decision.type == "double") {
        return ApplyDoubleMutation(decision, ctx, outError);
    }
    if (decision.type == "float") {
        return ApplyFloatMutation(decision, ctx, outError);
    }

    if (outError) *outError = "Unsupported sidecar auto-demo mutation type: " + decision.type;
    return false;
}

void ResetSidecarAutoDemoLoopState(
    SidecarAutoDemoLoopState* ioState) {
    ResetLoopState(ioState);
}

bool AdvanceSidecarAutoDemoLoop(
    const SidecarAutoDemoControllerDecision& decision,
    const SidecarAutoDemoControllerPolicy& policy,
    double deltaSeconds,
    bool interactionChanged,
    SidecarAutoDemoLoopState* ioState,
    bool* outShouldApply,
    std::string* outError) {
    if (outError) outError->clear();
    if (!ioState || !outShouldApply) {
        if (outError) *outError = "AdvanceSidecarAutoDemoLoop requires non-null state and output pointers";
        return false;
    }

    *outShouldApply = false;
    if (!ValidateControllerPolicy(policy, outError)) {
        ResetLoopState(ioState);
        return false;
    }
    if (!std::isfinite(deltaSeconds) || deltaSeconds < 0.0) {
        ResetLoopState(ioState);
        if (outError) *outError = "AdvanceSidecarAutoDemoLoop requires a finite non-negative deltaSeconds";
        return false;
    }
    if (!PolicyEnablesPacedLoop(policy) ||
        !decision.should_mutate ||
        decision.path.empty() ||
        !decision.has_target_value ||
        !std::isfinite(decision.target_value)) {
        ResetLoopState(ioState);
        return true;
    }
    if (interactionChanged || !DecisionMatchesLoopState(decision, *ioState)) {
        SeedLoopState(decision, ioState);
        ioState->armed_idle_seconds = deltaSeconds;
        return true;
    }

    ioState->armed_idle_seconds += deltaSeconds;
    if (ioState->armed_idle_seconds + 1.0e-12 < policy.paced_loop_interval_seconds) {
        return true;
    }

    ioState->armed_idle_seconds = 0.0;
    *outShouldApply = true;
    return true;
}