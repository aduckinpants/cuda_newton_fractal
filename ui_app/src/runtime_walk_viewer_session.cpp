#include "runtime_walk_viewer_session.h"

#include "finding_state_actions.h"
#include "fractal_family_rules.h"
#include <cstring>

namespace {

bool RuntimeWalkLiveStateMatches(const ViewState& lhsView,
    const KernelParams& lhsParams,
    const ViewState& rhsView,
    const KernelParams& rhsParams) {
    return std::memcmp(&lhsView, &rhsView, sizeof(ViewState)) == 0 &&
        std::memcmp(&lhsParams, &rhsParams, sizeof(KernelParams)) == 0;
}

} // namespace


void ResetRuntimeWalkViewerPlaybackForNewSession(const RuntimeWalkViewerPlaybackState& previous,
    RuntimeWalkViewerPlaybackState* outPlayback) {
    if (!outPlayback) return;
    RuntimeWalkViewerPlaybackState next{};
    next.loop = previous.loop;
    next.show_raw_path = previous.show_raw_path;
    next.show_spline_path = previous.show_spline_path;
    next.show_closed_loop = previous.show_closed_loop;
    next.show_branch_markers = previous.show_branch_markers;
    next.show_gradient_overlay = previous.show_gradient_overlay;
    *outPlayback = next;
}

bool LoadRuntimeWalkViewerSession(const std::string& requestJsonPath,
    RuntimeWalkViewerSession* outSession,
    std::string* outError) {
    if (outError) outError->clear();
    if (!outSession) {
        if (outError) *outError = "Runtime walk viewer session output is required";
        return false;
    }

    RuntimeWalkRequest request;
    if (!LoadRuntimeWalkRequestFile(requestJsonPath, &request, outError)) return false;

    RuntimeWalkBundle bundle;
    if (!LoadRuntimeWalkBundleFile(request.bundle_json_path, &bundle, outError)) return false;

    ViewState baseView{};
    KernelParams baseParams{};
    RenderSettings baseRender{};
    std::string resolvedStatePath;
    if (!LoadFindingSelectionIntoRuntime(
            request.base_state_json_path,
            &baseView,
            &baseParams,
            &baseRender,
            &resolvedStatePath,
            outError)) {
        return false;
    }

    if (!IsExplainoFamily(baseView.fractal_type)) {
        if (outError) *outError = "runtime walk viewer requires an Explaino-family base state";
        return false;
    }

    if (baseRender.resolution.x <= 0 || baseRender.resolution.y <= 0) {
        baseRender.resolution = {1024, 768};
    }
    if (baseRender.block_size <= 0) baseRender.block_size = 256;
    if (baseRender.device_id < 0) baseRender.device_id = 0;

    RuntimeWalkViewerAsset asset;
    if (!BuildRuntimeWalkViewerAsset(request, bundle, baseView, baseParams, baseRender, &asset, outError)) {
        return false;
    }

    RuntimeWalkViewerSession session;
    session.loaded = true;
    session.request_json_path = requestJsonPath;
    session.resolved_state_json_path = resolvedStatePath;
    session.authority_mode = request.authority_mode;
    session.asset = asset;
    session.operator_baseline_view = baseView;
    session.operator_baseline_params = baseParams;
    session.has_operator_baseline = true;
    session.has_last_composed_state = false;
    *outSession = session;
    return true;
}

bool ApplyRuntimeWalkViewerPlaybackSnapshot(RuntimeWalkViewerSession& session,
    const RuntimeWalkViewerPlaybackState& playback,
    ViewState* ioView,
    KernelParams* ioParams,
    RuntimeWalkSnapshot* outSnapshot,
    std::string* outError) {
    if (outError) outError->clear();
    if (!session.loaded) {
        if (outError) *outError = "Runtime walk viewer session is not loaded";
        return false;
    }
    if (!ioView || !ioParams) {
        if (outError) *outError = "Runtime walk viewer snapshot application requires view and params";
        return false;
    }

    RuntimeWalkSnapshot snapshot;
    if (!EvaluateRuntimeWalkViewerCurrentSnapshot(session.asset, playback, &snapshot, outError)) {
        return false;
    }
    if (!session.has_operator_baseline) {
        session.operator_baseline_view = session.asset.base_view;
        session.operator_baseline_params = session.asset.base_params;
        session.has_operator_baseline = true;
    } else if (session.has_last_composed_state &&
        !RuntimeWalkLiveStateMatches(*ioView, *ioParams, session.last_composed_view, session.last_composed_params)) {
        session.operator_baseline_view = *ioView;
        session.operator_baseline_params = *ioParams;
    }

    ComposeRuntimeWalkSnapshotOverLiveBaseline(
        session.asset,
        snapshot,
        session.operator_baseline_view,
        session.operator_baseline_params,
        ioView,
        ioParams);
    session.last_composed_view = *ioView;
    session.last_composed_params = *ioParams;
    session.has_last_composed_state = true;
    if (outSnapshot) *outSnapshot = snapshot;
    return true;
}

bool UpdateRuntimeWalkViewerPlayback(RuntimeWalkViewerSession& session,
    double deltaSeconds,
    bool togglePlayPause,
    bool stepPrevious,
    bool stepNext,
    RuntimeWalkViewerPlaybackState* ioPlayback,
    ViewState* ioView,
    KernelParams* ioParams,
    RuntimeWalkSnapshot* outSnapshot,
    bool* outChanged,
    std::string* outError) {
    if (outChanged) *outChanged = false;
    if (outError) outError->clear();
    if (!session.loaded) {
        if (outError) *outError = "Runtime walk viewer session is not loaded";
        return false;
    }
    if (!ioPlayback) {
        if (outError) *outError = "Runtime walk viewer playback state is required";
        return false;
    }

    bool changed = false;
    if (!ioPlayback->loaded) {
        if (!SeekRuntimeWalkViewerPlayback(session.asset, session.asset.tick_snapshots.front().t, ioPlayback, &changed)) {
            if (outError) *outError = "Failed to initialize runtime walk viewer playback state";
            return false;
        }
    }
    if (togglePlayPause) {
        ioPlayback->playing = !ioPlayback->playing;
    }
    bool localChanged = false;
    if (stepPrevious) {
        if (!StepRuntimeWalkViewerPlayback(session.asset, -1, ioPlayback, &localChanged)) return false;
        changed = changed || localChanged;
    }
    if (stepNext) {
        if (!StepRuntimeWalkViewerPlayback(session.asset, 1, ioPlayback, &localChanged)) return false;
        changed = changed || localChanged;
    }
    if (!(stepPrevious || stepNext)) {
        if (!AdvanceRuntimeWalkViewerPlayback(session.asset, deltaSeconds, ioPlayback, &localChanged)) return false;
        changed = changed || localChanged;
    }
    if (!ApplyRuntimeWalkViewerPlaybackSnapshot(session, *ioPlayback, ioView, ioParams, outSnapshot, outError)) {
        return false;
    }
    if (outChanged) *outChanged = changed;
    return true;
}
