#pragma once

#include "runtime_walk_viewer.h"

#include <string>

struct RuntimeWalkViewerSession {
    bool loaded = false;
    std::string request_json_path;
    std::string resolved_state_json_path;
    RuntimeWalkAuthorityMode authority_mode = RuntimeWalkAuthorityMode::loaded_base_state;
    RuntimeWalkViewerAsset asset{};
    bool has_operator_baseline = false;
    bool has_last_composed_state = false;
    ViewState operator_baseline_view{};
    KernelParams operator_baseline_params{};
    ViewState last_composed_view{};
    KernelParams last_composed_params{};
};

bool LoadRuntimeWalkViewerSession(const std::string& requestJsonPath,
    RuntimeWalkViewerSession* outSession,
    std::string* outError);

bool ApplyRuntimeWalkViewerPlaybackSnapshot(RuntimeWalkViewerSession& session,
    const RuntimeWalkViewerPlaybackState& playback,
    ViewState* ioView,
    KernelParams* ioParams,
    RuntimeWalkSnapshot* outSnapshot,
    std::string* outError);

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
    std::string* outError);

void ResetRuntimeWalkViewerPlaybackForNewSession(const RuntimeWalkViewerPlaybackState& previous,
    RuntimeWalkViewerPlaybackState* outPlayback);
