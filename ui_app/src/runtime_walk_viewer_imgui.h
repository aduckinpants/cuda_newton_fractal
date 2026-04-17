#pragma once

#include "runtime_walk_viewer.h"
#include "runtime_walk_viewer_import.h"
#include "runtime_walk_viewer_session.h"

struct ImDrawList;
struct ImVec2;

struct RuntimeWalkViewerImportPanelState {
    bool open = false;
    std::string base_state_json_path;
    std::string comparison_fits_path;
    std::string request_json_path;
    std::string bundle_json_path;
    std::string status_text;
    std::vector<RuntimeWalkViewerImportSessionRecord> recent_sessions;
};

struct RuntimeWalkViewerUiActions {
    bool open_request_dialog = false;
    bool reload_current_request = false;
};

struct RuntimeWalkViewerImportUiActions {
    bool open_fits_dialog = false;
    bool open_request_dialog = false;
    bool open_bundle_dialog = false;
    bool build_and_open = false;
    bool open_latest = false;
    std::string open_recent_request_json_path;
};

bool RenderRuntimeWalkViewerPanel(const RuntimeWalkViewerSession& session,
    RuntimeWalkViewerPlaybackState* ioPlayback,
    RuntimeWalkOverlayProviderConfig* ioOverlayConfig,
    RuntimeWalkViewerUiActions* outActions);

bool RenderRuntimeWalkViewerImportPanel(const RuntimeWalkViewerImportPanelState& state,
    bool importAllowed,
    const std::string& blockedReason,
    RuntimeWalkViewerImportUiActions* outActions);

void DrawRuntimeWalkViewerViewportOverlay(const RuntimeWalkViewerPlaybackState& playback,
    const RuntimeWalkOverlayPath& path,
    const RuntimeWalkGradientOverlay& overlay,
    const ImVec2& rectMin,
    const ImVec2& rectMax,
    ImDrawList* drawList);
