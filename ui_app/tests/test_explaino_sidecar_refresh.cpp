#include "../src/explaino_sidecar_refresh.h"

#include <cstdio>

namespace {

int g_passed = 0;
int g_failed = 0;

void Check(bool condition, const char* name) {
    if (condition) {
        std::printf("  PASS: %s\n", name);
        ++g_passed;
    } else {
        std::printf("  FAIL: %s\n", name);
        ++g_failed;
    }
}

SidecarAutoDemoControllerPolicy BuildPolicy() {
    SidecarAutoDemoControllerPolicy policy;
    policy.enabled = true;
    policy.allow_runtime_mutation = true;
    policy.run_paced_loop = true;
    policy.paced_loop_interval_seconds = 0.5;
    policy.stop_demonstrated_fraction = 0.75;
    policy.stop_uncertain_count = 2;
    return policy;
}

void TestPoliciesMatchRequiresAllFields() {
    const SidecarAutoDemoControllerPolicy baseline = BuildPolicy();

    Check(SidecarAutoDemoControllerPoliciesMatch(baseline, baseline),
        "TestPoliciesMatchRequiresAllFields_IdenticalPolicies");

    SidecarAutoDemoControllerPolicy changed = baseline;
    changed.enabled = false;
    Check(!SidecarAutoDemoControllerPoliciesMatch(baseline, changed),
        "TestPoliciesMatchRequiresAllFields_Enabled");

    changed = baseline;
    changed.allow_runtime_mutation = false;
    Check(!SidecarAutoDemoControllerPoliciesMatch(baseline, changed),
        "TestPoliciesMatchRequiresAllFields_AllowRuntimeMutation");

    changed = baseline;
    changed.run_paced_loop = false;
    Check(!SidecarAutoDemoControllerPoliciesMatch(baseline, changed),
        "TestPoliciesMatchRequiresAllFields_RunPacedLoop");

    changed = baseline;
    changed.paced_loop_interval_seconds = 1.25;
    Check(!SidecarAutoDemoControllerPoliciesMatch(baseline, changed),
        "TestPoliciesMatchRequiresAllFields_PacedLoopInterval");

    changed = baseline;
    changed.stop_demonstrated_fraction = 0.9;
    Check(!SidecarAutoDemoControllerPoliciesMatch(baseline, changed),
        "TestPoliciesMatchRequiresAllFields_StopDemonstratedFraction");

    changed = baseline;
    changed.stop_uncertain_count = 1;
    Check(!SidecarAutoDemoControllerPoliciesMatch(baseline, changed),
        "TestPoliciesMatchRequiresAllFields_StopUncertainCount");
}

void TestShouldRefreshExplainoSidecarState() {
    const SidecarAutoDemoControllerPolicy baseline = BuildPolicy();

    Check(!ShouldRefreshExplainoSidecarState(false, true, &baseline, baseline),
        "TestShouldRefreshExplainoSidecarState_UnchangedPolicy");
    Check(ShouldRefreshExplainoSidecarState(true, true, &baseline, baseline),
        "TestShouldRefreshExplainoSidecarState_Dirty");
    Check(ShouldRefreshExplainoSidecarState(false, false, &baseline, baseline),
        "TestShouldRefreshExplainoSidecarState_InvalidState");
    Check(ShouldRefreshExplainoSidecarState(false, true, nullptr, baseline),
        "TestShouldRefreshExplainoSidecarState_NoCachedPolicy");

    SidecarAutoDemoControllerPolicy changed = baseline;
    changed.stop_uncertain_count = 4;
    Check(ShouldRefreshExplainoSidecarState(false, true, &baseline, changed),
        "TestShouldRefreshExplainoSidecarState_ChangedPolicy");
}

} // namespace

int main() {
    TestPoliciesMatchRequiresAllFields();
    TestShouldRefreshExplainoSidecarState();

    std::printf("test_explaino_sidecar_refresh: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}