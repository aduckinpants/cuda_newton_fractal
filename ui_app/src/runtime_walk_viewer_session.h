#pragma once

#include "runtime_walk_viewer.h"

#include <string>

struct RuntimeWalkViewerSession {
    bool loaded = false;
    std::string request_json_path;
    std::string resolved_state_json_path;
    RuntimeWalkAuthorityMode authority_mode = RuntimeWalkAuthorityMode::loaded_base_state;
    RuntimeWalkViewerAsset asset{};
};

bool LoadRuntimeWalkViewerSession(const std::string& requestJsonPath,
    RuntimeWalkViewerSession* outSession,
    std::string* outError);

bool ApplyRuntimeWalkViewerPlaybackSnapshot(const RuntimeWalkViewerSession& session,
    const RuntimeWalkViewerPlaybackState& playback,
    ViewState* ioView,
    KernelParams* ioParams,
    RuntimeWalkSnapshot* outSnapshot,
    std::string* outError);

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
    std::string* outError);

void ResetRuntimeWalkViewerPlaybackForNewSession(const RuntimeWalkViewerPlaybackState& previous,
    RuntimeWalkViewerPlaybackState* outPlayback);
