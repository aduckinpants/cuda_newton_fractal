#include "runtime_walk_viewer_imgui.h"

#include "imgui.h"

#include <algorithm>
#include <filesystem>

namespace {

ImVec2 ToScreenPoint(Double2 point, const ImVec2& rectMin, const ImVec2& rectMax) {
    const float width = rectMax.x - rectMin.x;
    const float height = rectMax.y - rectMin.y;
    return ImVec2(
        rectMin.x + static_cast<float>(point.x) * width,
        rectMin.y + static_cast<float>(point.y) * height);
}

void DrawPolyline(const std::vector<Double2>& points,
    ImU32 color,
    float thickness,
    const ImVec2& rectMin,
    const ImVec2& rectMax,
    ImDrawList* drawList) {
    if (!drawList || points.size() < 2u) return;
    for (std::size_t index = 1; index < points.size(); ++index) {
        drawList->AddLine(
            ToScreenPoint(points[index - 1], rectMin, rectMax),
            ToScreenPoint(points[index], rectMin, rectMax),
            color,
            thickness);
    }
}

} // namespace

bool RenderRuntimeWalkViewerPanel(const RuntimeWalkViewerSession& session,
    RuntimeWalkViewerPlaybackState* ioPlayback,
    RuntimeWalkOverlayProviderConfig* ioOverlayConfig,
    RuntimeWalkViewerUiActions* outActions) {
    if (!ioPlayback || !ioOverlayConfig || !outActions) return false;
    *outActions = {};

    bool interactionChanged = false;
    ImGui::Begin("Runtime Walk Playback");
    if (!session.loaded) {
        ImGui::TextUnformatted("No runtime-walk request loaded.");
        if (ImGui::Button("Open Request...")) {
            outActions->open_request_dialog = true;
            interactionChanged = true;
        }
        ImGui::End();
        return interactionChanged;
    }

    RuntimeWalkSnapshot currentSnapshot;
    std::string snapshotError;
    const bool haveSnapshot = EvaluateRuntimeWalkViewerCurrentSnapshot(session.asset, *ioPlayback, &currentSnapshot, &snapshotError);
    const float minT = static_cast<float>(session.asset.tick_snapshots.front().t);
    const float maxT = static_cast<float>(session.asset.tick_snapshots.back().t);
    float currentT = static_cast<float>(ioPlayback->current_t);
    float speed = static_cast<float>(ioPlayback->speed);

    if (ImGui::Button("Open Request...")) {
        outActions->open_request_dialog = true;
        interactionChanged = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Reload")) {
        outActions->reload_current_request = true;
        interactionChanged = true;
    }
    ImGui::SameLine();
    if (ImGui::Button(ioPlayback->playing ? "Pause" : "Play")) {
        ioPlayback->playing = !ioPlayback->playing;
        interactionChanged = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Prev")) {
        bool changed = false;
        StepRuntimeWalkViewerPlayback(session.asset, -1, ioPlayback, &changed);
        interactionChanged = interactionChanged || changed;
    }
    ImGui::SameLine();
    if (ImGui::Button("Next")) {
        bool changed = false;
        StepRuntimeWalkViewerPlayback(session.asset, 1, ioPlayback, &changed);
        interactionChanged = interactionChanged || changed;
    }

    if (ImGui::SliderFloat("t", &currentT, minT, maxT, "%.3f")) {
        bool changed = false;
        SeekRuntimeWalkViewerPlayback(session.asset, static_cast<double>(currentT), ioPlayback, &changed);
        interactionChanged = interactionChanged || changed;
    }
    if (ImGui::SliderFloat("Speed", &speed, 0.01f, 1.25f, "%.2f")) {
        ioPlayback->speed = static_cast<double>(speed);
        interactionChanged = true;
    }
    interactionChanged = ImGui::Checkbox("Loop", &ioPlayback->loop) || interactionChanged;

    ImGui::Separator();
    ImGui::Text("Tick %d / %d", static_cast<int>(ioPlayback->nearest_tick_index + 1u), static_cast<int>(session.asset.tick_snapshots.size()));
    if (haveSnapshot) {
        ImGui::Text("Branch: %s%s",
            currentSnapshot.branch.nearest_marker_label.empty() ? "(none)" : currentSnapshot.branch.nearest_marker_label.c_str(),
            currentSnapshot.branch.sticky ? " [sticky]" : "");
        ImGui::Text("Current t: %.3f", currentSnapshot.t);
    } else {
        ImGui::TextWrapped("Snapshot unavailable: %s", snapshotError.c_str());
    }

    ImGui::Separator();
    interactionChanged = ImGui::Checkbox("Raw Path", &ioPlayback->show_raw_path) || interactionChanged;
    interactionChanged = ImGui::Checkbox("Spline Path", &ioPlayback->show_spline_path) || interactionChanged;
    interactionChanged = ImGui::Checkbox("Closed Loop Fit", &ioPlayback->show_closed_loop) || interactionChanged;
    interactionChanged = ImGui::Checkbox("Branch Markers", &ioPlayback->show_branch_markers) || interactionChanged;
    interactionChanged = ImGui::Checkbox("Gradient Flow", &ioPlayback->show_gradient_overlay) || interactionChanged;

    float threshold = static_cast<float>(ioOverlayConfig->threshold);
    if (ImGui::SliderFloat("Gradient Threshold", &threshold, 0.01f, 1.0f, "%.2f")) {
        ioOverlayConfig->threshold = static_cast<double>(threshold);
        interactionChanged = true;
    }

    ImGui::Separator();
    ImGui::TextWrapped("Request: %s", session.request_json_path.c_str());
    ImGui::TextWrapped("Session Dir: %s", std::filesystem::path(session.request_json_path).parent_path().string().c_str());
    ImGui::TextWrapped("Authority Mode: %s", RuntimeWalkAuthorityModeId(session.authority_mode));
    ImGui::TextWrapped("Base State (authority): %s", session.resolved_state_json_path.c_str());
    if (!session.asset.authority.mapping_profile_json_path.empty()) {
        ImGui::TextWrapped("Mapping Profile: %s", session.asset.authority.mapping_profile_json_path.c_str());
    }
    if (!session.asset.authority.orientation_inputs_json_path.empty()) {
        ImGui::TextWrapped("Orientation Inputs: %s", session.asset.authority.orientation_inputs_json_path.c_str());
    }
    ImGui::TextWrapped("Bundle (transport): %s", session.asset.request.bundle_json_path.c_str());
    ImGui::Text("Transport Samples: %d", static_cast<int>(session.asset.tick_snapshots.size()));
    if (!session.asset.request.transport_generation_mode.empty()) {
        ImGui::TextWrapped("Transport Mode: %s", session.asset.request.transport_generation_mode.c_str());
    }
    ImGui::Text("Motion Intensity: %.2f", session.asset.request.transport_motion_scale);
    ImGui::Text("Warp Motion: %.2f", session.asset.request.transport_warp_scale);
    if (!session.asset.companion.comparison_fits_path.empty()) {
        ImGui::TextWrapped("Companion FITS (evidence): %s", session.asset.companion.comparison_fits_path.c_str());
    }
    if (!session.asset.companion.rtk_manifest_json_path.empty()) {
        ImGui::TextWrapped("RTK Manifest (evidence): %s", session.asset.companion.rtk_manifest_json_path.c_str());
    }
    if (!session.asset.companion.rtk_harvest_summary_json_path.empty()) {
        ImGui::TextWrapped("RTK Harvest (evidence): %s", session.asset.companion.rtk_harvest_summary_json_path.c_str());
    }

    ImGui::End();
    return interactionChanged;
}

bool RenderRuntimeWalkViewerImportPanel(RuntimeWalkViewerImportPanelState& state,
    bool importAllowed,
    const std::string& blockedReason,
    RuntimeWalkViewerImportUiActions* outActions) {
    if (!outActions) return false;
    *outActions = {};
    if (!state.open) return false;

    bool interactionChanged = false;
    ImGui::Begin("Runtime Walk FITS Import");
    bool synthesizeDefault = state.authority_mode == RuntimeWalkAuthorityMode::synthesized_fits_base;
    if (ImGui::Checkbox("Synthesize Default Explaino State", &synthesizeDefault)) {
        outActions->toggle_authority_mode = true;
        interactionChanged = true;
    }
    ImGui::SameLine();
    if (synthesizeDefault) {
        ImGui::TextUnformatted("(FITS + mapping contract authority)");
    } else {
        ImGui::TextUnformatted("(loaded capture state authority)");
    }
    ImGui::TextWrapped(
        synthesizeDefault
            ? "FITS plus the checked-in mapping contract synthesize the authoritative Explaino base state and default runtime-walk transport when no authored request or bundle exists. RTK outputs remain companion evidence only."
            : "Loaded capture state remains authoritative. If no authored request or bundle exists, FITS import synthesizes default runtime-walk transport. FITS and RTK outputs remain companion evidence only.");
    ImGui::Separator();
    if (synthesizeDefault) {
        ImGui::TextUnformatted("Base State: synthesized from FITS");
    } else {
        ImGui::TextWrapped("Base State: %s", state.base_state_json_path.empty() ? "(none)" : state.base_state_json_path.c_str());
    }
    ImGui::TextWrapped("FITS: %s", state.comparison_fits_path.empty() ? "(none)" : state.comparison_fits_path.c_str());
    ImGui::TextWrapped("Mapping File: %s",
        state.resolved_mapping_profile_json_path.empty() ? "(unresolved)" : state.resolved_mapping_profile_json_path.c_str());
    ImGui::TextWrapped("Mapping Profile Id: %s", state.mapping_profile_id.empty() ? "explaino_default" : state.mapping_profile_id.c_str());
    ImGui::TextWrapped("Synthesized Base Fractal: %s",
        state.resolved_mapping_profile_base_fractal_type.empty() ? "(unknown)" : state.resolved_mapping_profile_base_fractal_type.c_str());
    int sampleCount = static_cast<int>(state.transport_options.sample_count);
    if (ImGui::SliderInt("Generated Samples", &sampleCount, 9, 129)) {
        state.transport_options.sample_count = static_cast<std::size_t>(sampleCount);
        interactionChanged = true;
    }
    float motionScale = static_cast<float>(state.transport_options.motion_scale);
    if (ImGui::SliderFloat("Motion Intensity", &motionScale, 0.10f, 1.50f, "%.2f")) {
        state.transport_options.motion_scale = static_cast<double>(motionScale);
        interactionChanged = true;
    }
    float warpScale = static_cast<float>(state.transport_options.warp_scale);
    if (ImGui::SliderFloat("Warp Motion", &warpScale, 0.0f, 0.50f, "%.2f")) {
        state.transport_options.warp_scale = static_cast<double>(warpScale);
        interactionChanged = true;
    }
    if (!state.status_text.empty()) {
        ImGui::TextWrapped("%s", state.status_text.c_str());
    }
    if (!importAllowed && !blockedReason.empty()) {
        ImGui::TextWrapped("%s", blockedReason.c_str());
    }

    if (ImGui::Button("Choose FITS...")) {
        outActions->open_fits_dialog = true;
        interactionChanged = true;
    }
    ImGui::SameLine();
    const bool haveOpenInput = RuntimeWalkViewerImportHasOpenInput(state);
    ImGui::BeginDisabled(!importAllowed);
    ImGui::BeginDisabled(!haveOpenInput);
    if (ImGui::Button("Open FITS")) {
        outActions->build_and_open = true;
        interactionChanged = true;
    }
    ImGui::EndDisabled();
    ImGui::EndDisabled();

    if (ImGui::Button("Open Latest")) {
        outActions->open_latest = true;
        interactionChanged = true;
    }

    if (ImGui::CollapsingHeader("Advanced Overrides")) {
        if (ImGui::Button("Browse Mapping...")) {
            outActions->open_mapping_profile_dialog = true;
            interactionChanged = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Browse Request...")) {
            outActions->open_request_dialog = true;
            interactionChanged = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Browse Bundle...")) {
            outActions->open_bundle_dialog = true;
            interactionChanged = true;
        }
        ImGui::TextWrapped("Request: %s", state.request_json_path.empty() ? "(none)" : state.request_json_path.c_str());
        ImGui::TextWrapped("Bundle: %s", state.bundle_json_path.empty() ? "(none)" : state.bundle_json_path.c_str());
        ImGui::TextWrapped("Mapping Profile Override: %s", state.mapping_profile_json_path.empty() ? "(default checked-in profile)" : state.mapping_profile_json_path.c_str());
    }

    if (ImGui::CollapsingHeader("Mapping Summary", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (state.mapping_binding_summaries.empty()) {
            ImGui::TextUnformatted("No resolved bindings.");
        } else {
            for (const std::string& line : state.mapping_binding_summaries) {
                ImGui::BulletText("%s", line.c_str());
            }
        }
    }

    if (ImGui::CollapsingHeader("Recent Sessions", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (state.recent_sessions.empty()) {
            ImGui::TextUnformatted("No recent FITS import sessions.");
        }
        for (const RuntimeWalkViewerImportSessionRecord& record : state.recent_sessions) {
            ImGui::PushID(record.session_id.c_str());
            const bool canOpen = record.request_exists && record.viewer_load_succeeded;
            ImGui::BeginDisabled(!canOpen);
            if (ImGui::SmallButton("Open")) {
                outActions->open_recent_request_json_path = record.request_json_path;
                interactionChanged = true;
            }
            ImGui::EndDisabled();
            ImGui::SameLine();
            ImGui::TextWrapped("[%s] %s%s",
                RuntimeWalkAuthorityModeId(record.authority_mode),
                record.comparison_fits_path.empty() ? record.request_json_path.c_str() : record.comparison_fits_path.c_str(),
                canOpen ? "" : (record.request_exists ? " [unloaded]" : " [stale]"));
            ImGui::PopID();
        }
    }

    ImGui::End();
    return interactionChanged;
}

void DrawRuntimeWalkViewerViewportOverlay(const RuntimeWalkViewerPlaybackState& playback,
    const RuntimeWalkOverlayPath& path,
    const RuntimeWalkGradientOverlay& overlay,
    const ImVec2& rectMin,
    const ImVec2& rectMax,
    ImDrawList* drawList) {
    if (!drawList) return;

    if (playback.show_closed_loop) {
        DrawPolyline(path.closed_loop_points, IM_COL32(70, 200, 255, 180), 2.0f, rectMin, rectMax, drawList);
    }
    if (playback.show_spline_path) {
        DrawPolyline(path.spline_points, IM_COL32(255, 210, 80, 180), 2.0f, rectMin, rectMax, drawList);
    }
    if (playback.show_raw_path) {
        DrawPolyline(path.raw_points, IM_COL32(255, 255, 255, 120), 1.0f, rectMin, rectMax, drawList);
    }

    if (playback.show_branch_markers) {
        for (const Double2& point : path.branch_marker_points) {
            drawList->AddCircleFilled(ToScreenPoint(point, rectMin, rectMax), 4.0f, IM_COL32(255, 120, 90, 220));
        }
    }

    if (playback.show_gradient_overlay) {
        for (const RuntimeWalkGradientOverlayGuideStroke& stroke : overlay.strokes) {
            if (stroke.points.size() < 2u) continue;
            const int alpha = std::clamp(static_cast<int>(70.0 + 160.0 * stroke.strength), 70, 230);
            for (std::size_t index = 1; index < stroke.points.size(); ++index) {
                drawList->AddLine(
                    ToScreenPoint(stroke.points[index - 1].point, rectMin, rectMax),
                    ToScreenPoint(stroke.points[index].point, rectMin, rectMax),
                    IM_COL32(120, 255, 180, alpha),
                    2.0f);
            }
        }
    }

    drawList->AddCircleFilled(ToScreenPoint(path.current_point, rectMin, rectMax), 6.0f, IM_COL32(255, 70, 70, 255));
    drawList->AddCircle(ToScreenPoint(path.current_point, rectMin, rectMax), 10.0f, IM_COL32(255, 255, 255, 220), 0, 2.0f);
}
