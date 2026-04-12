#include "explaino_sidecar_refresh.h"

bool SidecarAutoDemoControllerPoliciesMatch(
    const SidecarAutoDemoControllerPolicy& left,
    const SidecarAutoDemoControllerPolicy& right) {
    return left.enabled == right.enabled
        && left.allow_runtime_mutation == right.allow_runtime_mutation
        && left.run_paced_loop == right.run_paced_loop
        && left.paced_loop_interval_seconds == right.paced_loop_interval_seconds
        && left.stop_demonstrated_fraction == right.stop_demonstrated_fraction
        && left.stop_uncertain_count == right.stop_uncertain_count;
}

bool ShouldRefreshExplainoSidecarState(
    bool dirty,
    bool sidecarStateValid,
    const SidecarAutoDemoControllerPolicy* cachedPolicy,
    const SidecarAutoDemoControllerPolicy& requestedPolicy) {
    if (dirty || !sidecarStateValid || cachedPolicy == nullptr) {
        return true;
    }
    return !SidecarAutoDemoControllerPoliciesMatch(*cachedPolicy, requestedPolicy);
}