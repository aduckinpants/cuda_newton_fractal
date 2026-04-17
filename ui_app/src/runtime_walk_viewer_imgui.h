#pragma once

#include "runtime_walk_viewer.h"
#include "runtime_walk_viewer_session.h"

struct ImDrawList;
struct ImVec2;

struct RuntimeWalkViewerUiActions {
    bool open_request_dialog = false;
    bool reload_current_request = false;
};

bool RenderRuntimeWalkViewerPanel(const RuntimeWalkViewerSession& session,
    RuntimeWalkViewerPlaybackState* ioPlayback,
    RuntimeWalkOverlayProviderConfig* ioOverlayConfig,
    RuntimeWalkViewerUiActions* outActions);

void DrawRuntimeWalkViewerViewportOverlay(const RuntimeWalkViewerPlaybackState& playback,
    const RuntimeWalkOverlayPath& path,
    const RuntimeWalkGradientOverlay& overlay,
    const ImVec2& rectMin,
    const ImVec2& rectMax,
    ImDrawList* drawList);
