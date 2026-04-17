#include "runtime_walk_viewer_session.h"

#include "finding_state_actions.h"
#include "fractal_family_rules.h"

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
    session.asset = asset;
    *outSession = session;
    return true;
}

bool ApplyRuntimeWalkViewerPlaybackSnapshot(const RuntimeWalkViewerSession& session,
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
    *ioView = session.asset.base_view;
    *ioParams = session.asset.base_params;
    ApplyRuntimeWalkSnapshot(snapshot, ioView, ioParams);
    if (outSnapshot) *outSnapshot = snapshot;
    return true;
}

bool UpdateRuntimeWalkViewerPlayback(const RuntimeWalkViewerSession& session,
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
