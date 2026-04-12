#include "../src/explaino_sidecar_controller.h"
#include "../src/schema_binding.h"

#include <cmath>
#include <iostream>
#include <string>

namespace {

bool NearlyEqual(double left, double right, double eps = 1.0e-9) {
    const double delta = left - right;
    return delta < eps && delta > -eps;
}

SidecarAutoDemoControllerDecision MakeArmedDecision(
    const char* path = "fractal.params.ripple_amplitude",
    const char* type = "float",
    double targetValue = 0.12) {
    SidecarAutoDemoControllerDecision decision;
    decision.status = SidecarAutoDemoControllerStatus::apply_ready;
    decision.path = path;
    decision.type = type;
    decision.target_value = targetValue;
    decision.has_target_value = true;
    decision.should_mutate = true;
    return decision;
}

SidecarActionRecommendation MakeRecommendation(
    const char* path,
    const char* guidance,
    double activeMin,
    double activeMax) {
    SidecarActionRecommendation recommendation;
    recommendation.label = "Ripple";
    recommendation.path = path;
    recommendation.type = "float";
    recommendation.guidance = guidance;
    recommendation.utility = 1.5;
    recommendation.active_min = activeMin;
    recommendation.active_max = activeMax;
    recommendation.active_fraction = 0.4;
    return recommendation;
}

SidecarExplorationCompleteness BuildIncompleteCompleteness() {
    SidecarExplorationCompleteness completeness;
    completeness.function_id = "fractal.sample";
    completeness.fractal_type_id = "explaino";
    completeness.demonstrated_count = 1;
    completeness.uncertain_count = 2;
    completeness.demonstrated_fraction = 1.0 / 3.0;
    completeness.mean_coverage_score = 0.45;
    return completeness;
}

SidecarExplorationCompleteness BuildCompleteCompleteness() {
    SidecarExplorationCompleteness completeness;
    completeness.function_id = "fractal.sample";
    completeness.fractal_type_id = "explaino";
    completeness.demonstrated_count = 3;
    completeness.uncertain_count = 0;
    completeness.demonstrated_fraction = 1.0;
    completeness.mean_coverage_score = 1.0;
    return completeness;
}

} // namespace

int main() {
    {
    SidecarActionRecommendation recommendation = MakeRecommendation("fractal.params.ripple_amplitude", "explore +", 0.02, 0.12);
        SidecarAutoDemoControllerDecision decision;
        std::string error;
        if (!BuildSidecarAutoDemoControllerDecision(
        &recommendation,
                BuildIncompleteCompleteness(),
                {},
                &decision,
                &error)) {
            std::cerr << "Expected disabled-by-default controller decision to build: " << error << "\n";
            return 1;
        }
        if (decision.status != SidecarAutoDemoControllerStatus::disabled || decision.should_mutate) {
            std::cerr << "Expected auto-demonstration controller to stay disabled by default\n";
            return 1;
        }
    }

    {
        SidecarAutoDemoControllerPolicy policy;
        policy.enabled = true;
        SidecarActionRecommendation recommendation = MakeRecommendation("fractal.params.ripple_amplitude", "explore +", 0.02, 0.12);
        SidecarAutoDemoControllerDecision decision;
        std::string error;
        if (!BuildSidecarAutoDemoControllerDecision(
            &recommendation,
                BuildIncompleteCompleteness(),
                policy,
                &decision,
                &error)) {
            std::cerr << "Expected enabled controller decision to build without runtime mutation opt-in: " << error << "\n";
            return 1;
        }
        if (decision.status != SidecarAutoDemoControllerStatus::proposal_ready || decision.should_mutate) {
            std::cerr << "Expected enabled controller without mutation opt-in to expose a proposal but not mutate\n";
            return 1;
        }
        if (decision.path != "fractal.params.ripple_amplitude" || !NearlyEqual(decision.target_value, 0.12)) {
            std::cerr << "Expected plus-guided controller decision to target the active max\n";
            return 1;
        }
    }

    {
        SidecarAutoDemoControllerPolicy policy;
        policy.enabled = true;
        policy.allow_runtime_mutation = true;
        SidecarActionRecommendation recommendation = MakeRecommendation("fractal.params.ripple_amplitude", "explore -", 0.02, 0.12);
        SidecarAutoDemoControllerDecision decision;
        std::string error;
        if (!BuildSidecarAutoDemoControllerDecision(
            &recommendation,
                BuildIncompleteCompleteness(),
                policy,
                &decision,
                &error)) {
            std::cerr << "Expected opt-in-enabled controller decision to build: " << error << "\n";
            return 1;
        }
        if (decision.status != SidecarAutoDemoControllerStatus::apply_ready || !decision.should_mutate) {
            std::cerr << "Expected explicit opt-in to arm a mutation-ready controller decision\n";
            return 1;
        }
        if (!NearlyEqual(decision.target_value, 0.02)) {
            std::cerr << "Expected minus-guided controller decision to target the active min\n";
            return 1;
        }
    }

    {
        SidecarAutoDemoControllerPolicy policy;
        policy.enabled = true;
        policy.allow_runtime_mutation = true;
        SidecarAutoDemoControllerDecision decision;
        std::string error;
        if (!BuildSidecarAutoDemoControllerDecision(
                nullptr,
                BuildIncompleteCompleteness(),
                policy,
                &decision,
                &error)) {
            std::cerr << "Expected missing-recommendation controller decision to build as an explicit blocked state: " << error << "\n";
            return 1;
        }
        if (decision.status != SidecarAutoDemoControllerStatus::blocked_no_action || decision.should_mutate) {
            std::cerr << "Expected missing action recommendations to block controller mutation without fallback\n";
            return 1;
        }
    }

    {
        SidecarAutoDemoControllerPolicy policy;
        policy.enabled = true;
        policy.allow_runtime_mutation = true;
        SidecarActionRecommendation recommendation = MakeRecommendation("fractal.params.ripple_amplitude", "explore +", 0.02, 0.12);
        SidecarAutoDemoControllerDecision decision;
        std::string error;
        if (!BuildSidecarAutoDemoControllerDecision(
            &recommendation,
                BuildCompleteCompleteness(),
                policy,
                &decision,
                &error)) {
            std::cerr << "Expected completeness stop-point controller decision to build: " << error << "\n";
            return 1;
        }
        if (decision.status != SidecarAutoDemoControllerStatus::stopped_complete || decision.should_mutate) {
            std::cerr << "Expected controller stop conditions to prevent further mutation once completeness is satisfied\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        BindingContext ctx;
        ctx.view = &view;
        ctx.params = &params;
        ctx.render = &render;
        ctx.lens = &lens;

        SidecarAutoDemoControllerDecision decision;
        decision.path = "fractal.params.ripple_amplitude";
        decision.type = "float";
        decision.target_value = 0.125;
        decision.has_target_value = true;
        decision.should_mutate = true;

        std::string error;
        if (!ApplySidecarAutoDemoControllerDecision(decision, ctx, &error)) {
            std::cerr << "Expected float controller mutation apply to succeed: " << error << "\n";
            return 1;
        }
        if (!NearlyEqual(params.ripple_amplitude, 0.125f, 1.0e-6)) {
            std::cerr << "Expected float controller mutation to update ripple_amplitude\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        BindingContext ctx;
        ctx.view = &view;
        ctx.params = &params;
        ctx.render = &render;
        ctx.lens = &lens;

        SidecarAutoDemoControllerDecision decision;
        decision.path = "fractal.params.explaino_seed";
        decision.type = "double";
        decision.target_value = 7.25;
        decision.has_target_value = true;
        decision.should_mutate = true;

        std::string error;
        if (!ApplySidecarAutoDemoControllerDecision(decision, ctx, &error)) {
            std::cerr << "Expected double controller mutation apply to succeed: " << error << "\n";
            return 1;
        }
        if (!NearlyEqual(params.explaino_seed, 7.25, 1.0e-9)) {
            std::cerr << "Expected double controller mutation to update explaino_seed\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        BindingContext ctx;
        ctx.view = &view;
        ctx.params = &params;
        ctx.render = &render;
        ctx.lens = &lens;

        SidecarAutoDemoControllerDecision decision;
        decision.path = "fractal.params.max_iter";
        decision.type = "int";
        decision.target_value = 513.4;
        decision.has_target_value = true;
        decision.should_mutate = true;

        std::string error;
        if (!ApplySidecarAutoDemoControllerDecision(decision, ctx, &error)) {
            std::cerr << "Expected int controller mutation apply to succeed: " << error << "\n";
            return 1;
        }
        if (params.max_iter != 513) {
            std::cerr << "Expected int controller mutation to round target_value to max_iter\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        BindingContext ctx;
        ctx.view = &view;
        ctx.params = &params;
        ctx.render = &render;
        ctx.lens = &lens;

        SidecarAutoDemoControllerDecision decision;
        decision.path = "fractal.params.unknown";
        decision.type = "float";
        decision.target_value = 1.0;
        decision.has_target_value = true;
        decision.should_mutate = true;

        std::string error;
        if (ApplySidecarAutoDemoControllerDecision(decision, ctx, &error)) {
            std::cerr << "Expected unsupported controller mutation path to fail\n";
            return 1;
        }
        if (error.find("Unsupported sidecar auto-demo mutation type/path pair") == std::string::npos) {
            std::cerr << "Unexpected unsupported-path error text: " << error << "\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        BindingContext ctx;
        ctx.view = &view;
        ctx.params = &params;
        ctx.render = &render;
        ctx.lens = &lens;

        SidecarAutoDemoControllerDecision decision;
        decision.path = "fractal.params.ripple_amplitude";
        decision.type = "float";

        std::string error;
        if (ApplySidecarAutoDemoControllerDecision(decision, ctx, &error)) {
            std::cerr << "Expected targetless controller mutation apply to fail\n";
            return 1;
        }
        if (error.find("requires a target_value") == std::string::npos) {
            std::cerr << "Unexpected targetless error text: " << error << "\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        BindingContext ctx;
        ctx.view = &view;
        ctx.params = &params;
        ctx.render = &render;
        ctx.lens = &lens;

        SidecarAutoDemoControllerDecision decision;
        decision.status = SidecarAutoDemoControllerStatus::proposal_ready;
        decision.path = "fractal.params.ripple_amplitude";
        decision.type = "float";
        decision.target_value = 0.25;
        decision.has_target_value = true;

        std::string error;
        if (ApplySidecarAutoDemoControllerDecision(decision, ctx, &error)) {
            std::cerr << "Expected proposal-ready controller mutation apply to fail without an armed mutation\n";
            return 1;
        }
        if (error.find("requires an armed mutation decision") == std::string::npos) {
            std::cerr << "Unexpected proposal-ready error text: " << error << "\n";
            return 1;
        }
        if (!NearlyEqual(params.ripple_amplitude, 0.0f, 1.0e-6)) {
            std::cerr << "Unarmed controller mutation unexpectedly changed ripple_amplitude\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        BindingContext ctx;
        ctx.view = &view;
        ctx.params = &params;
        ctx.render = &render;
        ctx.lens = &lens;

        SidecarAutoDemoControllerDecision decision;
        decision.status = SidecarAutoDemoControllerStatus::apply_ready;
        decision.path = "fractal.params.ripple_amplitude";
        decision.type = "int";
        decision.target_value = 2.0;
        decision.has_target_value = true;
        decision.should_mutate = true;

        std::string error;
        if (ApplySidecarAutoDemoControllerDecision(decision, ctx, &error)) {
            std::cerr << "Expected controller mutation type/path drift to fail fast\n";
            return 1;
        }
        if (error.find("Unsupported sidecar auto-demo mutation type/path pair") == std::string::npos) {
            std::cerr << "Unexpected type/path mismatch error text: " << error << "\n";
            return 1;
        }
        if (!NearlyEqual(params.ripple_amplitude, 0.0f, 1.0e-6)) {
            std::cerr << "Type/path mismatch unexpectedly changed ripple_amplitude\n";
            return 1;
        }
    }

    {
        SidecarAutoDemoControllerPolicy policy;
        policy.enabled = true;
        policy.allow_runtime_mutation = true;
        policy.run_paced_loop = true;
        policy.paced_loop_interval_seconds = 0.5;
        SidecarAutoDemoLoopState loopState;
        bool shouldApply = false;
        std::string error;

        if (!AdvanceSidecarAutoDemoLoop(MakeArmedDecision(), policy, 0.2, false, &loopState, &shouldApply, &error)) {
            std::cerr << "Expected paced auto-demo loop tick to succeed below interval: " << error << "\n";
            return 1;
        }
        if (shouldApply) {
            std::cerr << "Expected paced auto-demo loop not to fire before the dwell interval\n";
            return 1;
        }
        if (!AdvanceSidecarAutoDemoLoop(MakeArmedDecision(), policy, 0.3, false, &loopState, &shouldApply, &error)) {
            std::cerr << "Expected paced auto-demo loop tick to succeed at interval: " << error << "\n";
            return 1;
        }
        if (!shouldApply) {
            std::cerr << "Expected paced auto-demo loop to fire after accumulating the dwell interval\n";
            return 1;
        }
    }

    {
        SidecarAutoDemoControllerPolicy policy;
        policy.enabled = true;
        policy.allow_runtime_mutation = true;
        policy.run_paced_loop = true;
        policy.paced_loop_interval_seconds = 0.5;
        SidecarAutoDemoLoopState loopState;
        bool shouldApply = false;
        std::string error;

        if (!AdvanceSidecarAutoDemoLoop(MakeArmedDecision(), policy, 0.4, false, &loopState, &shouldApply, &error)) {
            std::cerr << "Expected initial paced loop tick to succeed before interaction reset: " << error << "\n";
            return 1;
        }
        if (!AdvanceSidecarAutoDemoLoop(MakeArmedDecision(), policy, 0.0, true, &loopState, &shouldApply, &error)) {
            std::cerr << "Expected paced loop interaction reset to succeed: " << error << "\n";
            return 1;
        }
        if (shouldApply) {
            std::cerr << "Expected user interaction to reset the paced loop timer instead of firing immediately\n";
            return 1;
        }
        if (!AdvanceSidecarAutoDemoLoop(MakeArmedDecision(), policy, 0.4, false, &loopState, &shouldApply, &error)) {
            std::cerr << "Expected paced loop tick after interaction reset to succeed: " << error << "\n";
            return 1;
        }
        if (shouldApply) {
            std::cerr << "Expected paced loop to require a full fresh interval after user interaction\n";
            return 1;
        }
        if (!AdvanceSidecarAutoDemoLoop(MakeArmedDecision(), policy, 0.1, false, &loopState, &shouldApply, &error)) {
            std::cerr << "Expected paced loop tick to succeed once the post-interaction interval completes: " << error << "\n";
            return 1;
        }
        if (!shouldApply) {
            std::cerr << "Expected paced loop to fire once the post-interaction dwell interval completes\n";
            return 1;
        }
    }

    {
        SidecarAutoDemoControllerPolicy policy;
        policy.enabled = true;
        policy.allow_runtime_mutation = true;
        policy.run_paced_loop = true;
        policy.paced_loop_interval_seconds = 0.5;
        SidecarAutoDemoLoopState loopState;
        bool shouldApply = false;
        std::string error;

        if (!AdvanceSidecarAutoDemoLoop(MakeArmedDecision("fractal.params.ripple_amplitude", "float", 0.12), policy, 0.4, false, &loopState, &shouldApply, &error)) {
            std::cerr << "Expected paced loop tick to succeed before decision-change reset: " << error << "\n";
            return 1;
        }
        if (!AdvanceSidecarAutoDemoLoop(MakeArmedDecision("fractal.params.ripple_amplitude", "float", 0.24), policy, 0.1, false, &loopState, &shouldApply, &error)) {
            std::cerr << "Expected paced loop tick to succeed after the armed decision changes: " << error << "\n";
            return 1;
        }
        if (shouldApply) {
            std::cerr << "Expected a changed armed decision to reset the paced loop timer\n";
            return 1;
        }
        if (!AdvanceSidecarAutoDemoLoop(MakeArmedDecision("fractal.params.ripple_amplitude", "float", 0.24), policy, 0.5, false, &loopState, &shouldApply, &error)) {
            std::cerr << "Expected paced loop tick to succeed after the new decision dwells long enough: " << error << "\n";
            return 1;
        }
        if (!shouldApply) {
            std::cerr << "Expected the new armed decision to fire after its own dwell interval\n";
            return 1;
        }
    }

    {
        SidecarAutoDemoControllerPolicy policy;
        policy.enabled = true;
        policy.allow_runtime_mutation = true;
        policy.run_paced_loop = true;
        policy.paced_loop_interval_seconds = 0.5;
        SidecarAutoDemoLoopState loopState;
        bool shouldApply = false;
        std::string error;

        if (!AdvanceSidecarAutoDemoLoop(MakeArmedDecision(), policy, 0.4, false, &loopState, &shouldApply, &error)) {
            std::cerr << "Expected paced loop tick to succeed before reset test: " << error << "\n";
            return 1;
        }
        ResetSidecarAutoDemoLoopState(&loopState);
        if (!AdvanceSidecarAutoDemoLoop(MakeArmedDecision(), policy, 0.4, false, &loopState, &shouldApply, &error)) {
            std::cerr << "Expected paced loop tick to succeed after explicit reset: " << error << "\n";
            return 1;
        }
        if (shouldApply) {
            std::cerr << "Expected explicit loop-state reset to require a fresh dwell interval\n";
            return 1;
        }
        if (!AdvanceSidecarAutoDemoLoop(MakeArmedDecision(), policy, 0.1, false, &loopState, &shouldApply, &error)) {
            std::cerr << "Expected paced loop tick to succeed once the post-reset dwell interval completes: " << error << "\n";
            return 1;
        }
        if (!shouldApply) {
            std::cerr << "Expected explicit loop-state reset to restart paced auto-demo timing from zero\n";
            return 1;
        }
    }

    {
        SidecarAutoDemoControllerPolicy policy;
        policy.enabled = true;
        policy.allow_runtime_mutation = true;
        policy.run_paced_loop = true;
        policy.paced_loop_interval_seconds = 0.5;
        SidecarAutoDemoLoopState loopState;
        bool shouldApply = true;
        std::string error;

        SidecarAutoDemoControllerDecision proposal = MakeArmedDecision();
        proposal.should_mutate = false;
        proposal.status = SidecarAutoDemoControllerStatus::proposal_ready;
        if (!AdvanceSidecarAutoDemoLoop(proposal, policy, 1.0, false, &loopState, &shouldApply, &error)) {
            std::cerr << "Expected paced loop tick to succeed for a proposal-only decision: " << error << "\n";
            return 1;
        }
        if (shouldApply) {
            std::cerr << "Expected paced loop not to fire when the controller decision is not armed\n";
            return 1;
        }
    }

    std::cout << "test_explaino_sidecar_controller: all passed\n";
    return 0;
}