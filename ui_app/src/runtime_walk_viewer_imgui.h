#pragma once

#include "runtime_walk_viewer.h"
#include "runtime_walk_viewer_import.h"
#include "runtime_walk_viewer_session.h"

struct ImDrawList;
struct ImVec2;

struct RuntimeWalkViewerUiActions {
    bool open_request_dialog = false;
    bool reload_current_request = false;
};

struct RuntimeWalkViewerImportUiActions {
    bool toggle_authority_mode = false;
    bool open_fits_dialog = false;
    bool open_mapping_profile_dialog = false;
    bool open_request_dialog = false;
    bool open_bundle_dialog = false;
    bool build_and_open = false;
    bool open_latest = false;
    std::string open_recent_request_json_path;
};

bool RenderRuntimeWalkViewerPanel(RuntimeWalkViewerSession& session,
    RuntimeWalkViewerPlaybackState* ioPlayback,
    RuntimeWalkOverlayProviderConfig* ioOverlayConfig,
    RuntimeWalkViewerUiActions* outActions);

bool RenderRuntimeWalkViewerImportPanel(RuntimeWalkViewerImportPanelState& state,
    bool importAllowed,
    const std::string& blockedReason,
    RuntimeWalkViewerImportUiActions* outActions);

void DrawRuntimeWalkViewerViewportOverlay(const RuntimeWalkViewerPlaybackState& playback,
    const RuntimeWalkOverlayPath& path,
    const RuntimeWalkGradientOverlay& overlay,
    const ImVec2& rectMin,
    const ImVec2& rectMax,
    ImDrawList* drawList);
