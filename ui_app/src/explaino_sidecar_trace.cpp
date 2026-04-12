#include "explaino_sidecar_trace.h"

#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace {

bool SameDecisionAsLastStep(
    const SidecarSlimeTraceStep& step,
    const SidecarAutoDemoControllerDecision& decision) {
    return step.controller_status == decision.status &&
        step.path == decision.path &&
        step.has_target_value == decision.has_target_value &&
        step.should_mutate == decision.should_mutate &&
        (!decision.has_target_value || step.target_value == decision.target_value);
}

bool VisitCountOrder(
    const SidecarSlimeTraceVisitCount& left,
    const SidecarSlimeTraceVisitCount& right) {
    if (left.latest_step_index != right.latest_step_index) {
        return left.latest_step_index > right.latest_step_index;
    }
    if (left.visit_count != right.visit_count) {
        return left.visit_count > right.visit_count;
    }
    if (left.label != right.label) {
        return left.label < right.label;
    }
    return left.path < right.path;
}

bool PreviousTraceCompatibleWithEnergy(
    const SidecarSlimeTrace& trace,
    const SidecarEnergyLandscape& energyLandscape) {
    if (trace.function_id.empty() || trace.fractal_type_id.empty()) {
        return false;
    }
    if (trace.function_id != energyLandscape.function_id) {
        return false;
    }
    if (trace.fractal_type_id != energyLandscape.fractal_type_id) {
        return false;
    }

    std::unordered_map<std::string, bool> rowPathSet;
    rowPathSet.reserve(energyLandscape.rows.size());
    for (const auto& row : energyLandscape.rows) {
        rowPathSet.emplace(row.path, true);
    }
    for (const auto& step : trace.steps) {
        if (rowPathSet.find(step.path) == rowPathSet.end()) {
            return false;
        }
    }
    return true;
}

void RebuildTraceSummaries(SidecarSlimeTrace* ioTrace) {
    ioTrace->proposal_step_count = 0;
    ioTrace->apply_step_count = 0;
    ioTrace->latest_path.clear();
    ioTrace->visit_counts.clear();

    std::unordered_map<std::string, std::size_t> visitIndexByPath;
    visitIndexByPath.reserve(ioTrace->steps.size());
    for (const auto& step : ioTrace->steps) {
        if (step.should_mutate) {
            ++ioTrace->apply_step_count;
        } else {
            ++ioTrace->proposal_step_count;
        }
        ioTrace->latest_path = step.path;

        const auto [it, inserted] = visitIndexByPath.emplace(step.path, ioTrace->visit_counts.size());
        if (inserted) {
            SidecarSlimeTraceVisitCount visit;
            visit.label = step.label;
            visit.path = step.path;
            visit.visit_count = 1;
            visit.latest_step_index = step.step_index;
            visit.latest_should_mutate = step.should_mutate;
            visit.latest_target_value = step.target_value;
            ioTrace->visit_counts.push_back(std::move(visit));
            continue;
        }

        SidecarSlimeTraceVisitCount& visit = ioTrace->visit_counts[it->second];
        ++visit.visit_count;
        visit.latest_step_index = step.step_index;
        visit.latest_should_mutate = step.should_mutate;
        visit.latest_target_value = step.target_value;
    }

    std::stable_sort(ioTrace->visit_counts.begin(), ioTrace->visit_counts.end(), VisitCountOrder);
}

} // namespace

bool BuildSidecarSlimeTrace(
    const SidecarEnergyLandscape& energyLandscape,
    const SidecarAutoDemoControllerDecision& controllerDecision,
    const SidecarSlimeTrace* previousTrace,
    SidecarSlimeTrace* outTrace,
    std::string* outError) {
    if (!outTrace) {
        if (outError) *outError = "BuildSidecarSlimeTrace requires outTrace";
        return false;
    }
    if (energyLandscape.function_id.empty()) {
        *outTrace = {};
        if (outError) *outError = "Sidecar slime trace requires energy landscape function_id";
        return false;
    }

    SidecarSlimeTrace next;
    if (previousTrace && PreviousTraceCompatibleWithEnergy(*previousTrace, energyLandscape)) {
        next = *previousTrace;
    }

    next.function_id = energyLandscape.function_id;
    next.fractal_type_id = energyLandscape.fractal_type_id;
    if (!controllerDecision.has_target_value || controllerDecision.path.empty()) {
        RebuildTraceSummaries(&next);
        *outTrace = std::move(next);
        return true;
    }
    if (!std::isfinite(controllerDecision.utility)) {
        *outTrace = {};
        if (outError) *outError = "Sidecar slime trace requires finite controller utility";
        return false;
    }
    if (!std::isfinite(controllerDecision.target_value)) {
        *outTrace = {};
        if (outError) *outError = "Sidecar slime trace requires finite controller target_value";
        return false;
    }

    const SidecarEnergyLandscapeRow* energyRow = nullptr;
    for (const auto& row : energyLandscape.rows) {
        if (row.path == controllerDecision.path) {
            energyRow = &row;
            break;
        }
    }
    if (!energyRow) {
        *outTrace = {};
        if (outError) *outError = "Sidecar slime trace missing energy row for controller path: " + controllerDecision.path;
        return false;
    }
    if (energyRow->status != SidecarEnergyLandscapeRowStatus::available) {
        *outTrace = {};
        if (outError) *outError = "Sidecar slime trace requires available energy row for controller path: " + controllerDecision.path;
        return false;
    }

    if (!next.steps.empty() && SameDecisionAsLastStep(next.steps.back(), controllerDecision)) {
        RebuildTraceSummaries(&next);
        *outTrace = std::move(next);
        return true;
    }

    SidecarSlimeTraceStep step;
    step.step_index = static_cast<int>(next.steps.size()) + 1;
    step.label = controllerDecision.label;
    step.path = controllerDecision.path;
    step.type = controllerDecision.type;
    step.guidance = controllerDecision.guidance;
    step.summary = controllerDecision.summary;
    step.reason = controllerDecision.reason;
    step.controller_status = controllerDecision.status;
    step.energy = energyRow->energy;
    step.utility = controllerDecision.utility;
    step.target_value = controllerDecision.target_value;
    step.has_target_value = controllerDecision.has_target_value;
    step.should_mutate = controllerDecision.should_mutate;
    next.steps.push_back(std::move(step));

    RebuildTraceSummaries(&next);
    *outTrace = std::move(next);
    return true;
}