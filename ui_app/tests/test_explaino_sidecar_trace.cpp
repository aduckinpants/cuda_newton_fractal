#include "../src/explaino_sidecar_trace.h"

#include <cmath>
#include <iostream>
#include <string>

namespace {

bool NearlyEqual(double left, double right, double eps = 1.0e-9) {
    const double delta = left - right;
    return delta < eps && delta > -eps;
}

SidecarParamSurfaceEntry MakeSurfaceEntry(
    const char* path,
    const char* label,
    double minValue,
    double maxValue,
    bool hasCostHint,
    double costHint) {
    SidecarParamSurfaceEntry entry;
    entry.path = path;
    entry.label = label;
    entry.type = "float";
    entry.has_min = true;
    entry.min_value = minValue;
    entry.has_max = true;
    entry.max_value = maxValue;
    entry.has_declared_span = true;
    entry.declared_span = maxValue - minValue;
    entry.has_cost_hint = hasCostHint;
    entry.cost_hint = costHint;
    return entry;
}

SidecarBudgetRow MakeBudgetRow(
    const char* path,
    const char* label,
    double estimatedInformationGain,
    double cumulativeInformationGain,
    double posteriorUncertainty,
    double decodeStability,
    int observationCount) {
    SidecarBudgetRow row;
    row.path = path;
    row.label = label;
    row.type = "float";
    row.estimated_information_gain = estimatedInformationGain;
    row.cumulative_information_gain = cumulativeInformationGain;
    row.posterior_uncertainty = posteriorUncertainty;
    row.decode_stability = decodeStability;
    row.observation_count = observationCount;
    return row;
}

SidecarLensProjectionRow MakeLensRow(
    const char* path,
    const char* label,
    double activeMin,
    double activeMax,
    double activeFraction,
    bool inactive,
    const char* guidance) {
    SidecarLensProjectionRow row;
    row.path = path;
    row.label = label;
    row.type = "float";
    row.active_min = activeMin;
    row.active_max = activeMax;
    row.active_fraction = activeFraction;
    row.inactive = inactive;
    row.guidance = guidance;
    return row;
}

SidecarHypothesisSpace BuildSpace() {
    SidecarHypothesisSpace space;
    space.function_id = "fractal.sample";
    space.applicable_parameters.push_back(MakeSurfaceEntry("fractal.params.ripple_amplitude", "Ripple", 0.0, 0.15, true, 2.55));
    space.applicable_parameters.push_back(MakeSurfaceEntry("fractal.params.vortex_strength", "Vortex", 0.0, 0.3, true, 0.99));
    return space;
}

SidecarBudgetState BuildBudget() {
    SidecarBudgetState budget;
    budget.function_id = "fractal.sample";
    budget.fractal_type_id = "explaino";
    budget.batch_count = 1;
    budget.rows.push_back(MakeBudgetRow("fractal.params.ripple_amplitude", "Ripple", 4.0, 4.0, 0.8, 0.9, 1));
    budget.rows.push_back(MakeBudgetRow("fractal.params.vortex_strength", "Vortex", 2.5, 2.5, 0.8, 0.95, 1));
    return budget;
}

SidecarLensProjection BuildLens() {
    SidecarLensProjection lens;
    lens.function_id = "fractal.sample";
    lens.fractal_type_id = "explaino";
    lens.rows.push_back(MakeLensRow("fractal.params.ripple_amplitude", "Ripple", 0.02, 0.13, 0.73, false, "explore +"));
    lens.rows.push_back(MakeLensRow("fractal.params.vortex_strength", "Vortex", 0.08, 0.22, 0.47, false, "refine +"));
    return lens;
}

SidecarAutoDemoControllerDecision MakeDecision(
    SidecarAutoDemoControllerStatus status,
    const char* path,
    double targetValue,
    bool shouldMutate) {
    SidecarAutoDemoControllerDecision decision;
    decision.status = status;
    decision.summary = shouldMutate ? "apply ready" : "proposal ready";
    decision.reason = shouldMutate ? "eligible auto-demonstration step armed" : "runtime mutation requires explicit opt-in";
    decision.label = path && std::string(path) == "fractal.params.vortex_strength" ? "Vortex" : "Ripple";
    decision.path = path;
    decision.type = "float";
    decision.guidance = "explore +";
    decision.utility = 1.5;
    decision.target_value = targetValue;
    decision.has_target_value = true;
    decision.should_mutate = shouldMutate;
    return decision;
}

} // namespace

int main() {
    SidecarEnergyLandscape energy;
    std::string error;
    if (!BuildSidecarEnergyLandscape(BuildSpace(), BuildBudget(), BuildLens(), &energy, &error)) {
        std::cerr << "Expected energy landscape fixture to build for trace test: " << error << "\n";
        return 1;
    }

    {
        SidecarSlimeTrace trace;
        if (!BuildSidecarSlimeTrace(
                energy,
                MakeDecision(SidecarAutoDemoControllerStatus::proposal_ready, "fractal.params.vortex_strength", 0.22, false),
                nullptr,
                &trace,
                &error)) {
            std::cerr << "Expected proposal-ready slime trace to build: " << error << "\n";
            return 1;
        }
        if (trace.steps.size() != 1 ||
            trace.steps[0].path != "fractal.params.vortex_strength" ||
            trace.steps[0].should_mutate ||
            trace.latest_path != "fractal.params.vortex_strength") {
            std::cerr << "Expected first slime-trace step to record the proposal path explicitly\n";
            return 1;
        }
        if (trace.visit_counts.size() != 1 || trace.visit_counts[0].visit_count != 1) {
            std::cerr << "Expected slime trace to expose an overlay visit count for the chosen energy row\n";
            return 1;
        }

        SidecarSlimeTrace repeated;
        if (!BuildSidecarSlimeTrace(
                energy,
                MakeDecision(SidecarAutoDemoControllerStatus::proposal_ready, "fractal.params.vortex_strength", 0.22, false),
                &trace,
                &repeated,
                &error)) {
            std::cerr << "Expected repeated identical slime-trace decision to build: " << error << "\n";
            return 1;
        }
        if (repeated.steps.size() != 1) {
            std::cerr << "Expected identical idle proposals to avoid duplicating slime-trace steps\n";
            return 1;
        }
    }

    {
        SidecarSlimeTrace first;
        if (!BuildSidecarSlimeTrace(
                energy,
                MakeDecision(SidecarAutoDemoControllerStatus::proposal_ready, "fractal.params.vortex_strength", 0.22, false),
                nullptr,
                &first,
                &error)) {
            std::cerr << "Expected initial slime trace to build before append test: " << error << "\n";
            return 1;
        }

        SidecarSlimeTrace second;
        if (!BuildSidecarSlimeTrace(
                energy,
                MakeDecision(SidecarAutoDemoControllerStatus::apply_ready, "fractal.params.ripple_amplitude", 0.13, true),
                &first,
                &second,
                &error)) {
            std::cerr << "Expected apply-ready slime trace to append: " << error << "\n";
            return 1;
        }
        if (second.steps.size() != 2 ||
            second.steps[1].path != "fractal.params.ripple_amplitude" ||
            !second.steps[1].should_mutate ||
            !NearlyEqual(second.steps[1].target_value, 0.13)) {
            std::cerr << "Expected a distinct apply-ready trace decision to append a second step\n";
            return 1;
        }
        if (second.visit_counts.size() != 2 ||
            second.visit_counts[0].latest_step_index != 2 ||
            second.visit_counts[0].path != "fractal.params.ripple_amplitude") {
            std::cerr << "Expected slime trace overlay rows to sort by latest visit and carry step indices\n";
            return 1;
        }
    }

    {
        SidecarSlimeTrace previous;
        if (!BuildSidecarSlimeTrace(
                energy,
                MakeDecision(SidecarAutoDemoControllerStatus::proposal_ready, "fractal.params.vortex_strength", 0.22, false),
                nullptr,
                &previous,
                &error)) {
            std::cerr << "Expected initial slime trace to build before blocked-state preservation test: " << error << "\n";
            return 1;
        }

        SidecarAutoDemoControllerDecision blocked;
        blocked.status = SidecarAutoDemoControllerStatus::blocked_no_action;
        blocked.summary = "blocked";
        blocked.reason = "no eligible passive recommendation";

        SidecarSlimeTrace preserved;
        if (!BuildSidecarSlimeTrace(energy, blocked, &previous, &preserved, &error)) {
            std::cerr << "Expected blocked controller states to preserve the slime trace without error: " << error << "\n";
            return 1;
        }
        if (preserved.steps.size() != previous.steps.size() || preserved.latest_path != previous.latest_path) {
            std::cerr << "Expected blocked controller states to leave the prior slime trace unchanged\n";
            return 1;
        }
    }

    {
        SidecarSlimeTrace previous;
        if (!BuildSidecarSlimeTrace(
                energy,
                MakeDecision(SidecarAutoDemoControllerStatus::proposal_ready, "fractal.params.vortex_strength", 0.22, false),
                nullptr,
                &previous,
                &error)) {
            std::cerr << "Expected initial slime trace to build before fractal-type reset test: " << error << "\n";
            return 1;
        }

        SidecarEnergyLandscape changedType = energy;
        changedType.fractal_type_id = "mandelbrot";

        SidecarSlimeTrace reset;
        if (!BuildSidecarSlimeTrace(
                changedType,
                MakeDecision(SidecarAutoDemoControllerStatus::proposal_ready, "fractal.params.ripple_amplitude", 0.13, false),
                &previous,
                &reset,
                &error)) {
            std::cerr << "Expected incompatible prior trace identity to reset instead of failing the current trace build: " << error << "\n";
            return 1;
        }
        if (reset.steps.size() != 1 ||
            reset.steps[0].path != "fractal.params.ripple_amplitude" ||
            reset.fractal_type_id != "mandelbrot") {
            std::cerr << "Expected fractal-type changes to drop stale slime-trace history and start a fresh path on the current surface\n";
            return 1;
        }
    }

    {
        SidecarSlimeTrace previous;
        if (!BuildSidecarSlimeTrace(
                energy,
                MakeDecision(SidecarAutoDemoControllerStatus::proposal_ready, "fractal.params.ripple_amplitude", 0.13, false),
                nullptr,
                &previous,
                &error)) {
            std::cerr << "Expected initial slime trace to build before surface-reset test: " << error << "\n";
            return 1;
        }

        SidecarEnergyLandscape reduced = energy;
        const std::string stalePath = previous.latest_path;
        const auto staleRow = std::find_if(
            reduced.rows.begin(),
            reduced.rows.end(),
            [&stalePath](const SidecarEnergyLandscapeRow& row) {
                return row.path == stalePath;
            });
        if (staleRow == reduced.rows.end()) {
            std::cerr << "Expected surface-reset fixture to find the stale slime-trace path on the current energy surface\n";
            return 1;
        }
        reduced.rows.erase(staleRow);

        SidecarSlimeTrace reset;
        if (!BuildSidecarSlimeTrace(
                reduced,
                MakeDecision(SidecarAutoDemoControllerStatus::proposal_ready, "fractal.params.vortex_strength", 0.22, false),
                &previous,
                &reset,
                &error)) {
            std::cerr << "Expected prior trace paths missing from the current energy surface to reset instead of failing: " << error << "\n";
            return 1;
        }
        if (reset.steps.size() != 1 || reset.steps[0].path != "fractal.params.vortex_strength") {
            std::cerr << "Expected stale slime-trace paths to be dropped when the current energy surface no longer exposes them\n";
            return 1;
        }
    }

    {
        SidecarSlimeTrace trace;
        if (BuildSidecarSlimeTrace(
                energy,
                MakeDecision(SidecarAutoDemoControllerStatus::proposal_ready, "fractal.params.not_real", 0.22, false),
                nullptr,
                &trace,
                &error)) {
            std::cerr << "Expected slime trace to fail fast when the controller path is missing from the energy surface\n";
            return 1;
        }
        if (error.find("missing energy row") == std::string::npos) {
            std::cerr << "Expected missing-energy-row failure to mention the missing path\n";
            return 1;
        }
    }

    std::cout << "test_explaino_sidecar_trace: all passed\n";
    return 0;
}