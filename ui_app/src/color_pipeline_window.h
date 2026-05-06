#pragma once

#include "imgui.h"

struct ColorPipelineWindowState {
    bool open = false;
};

inline void OpenColorPipelineWindow(ColorPipelineWindowState* ioState) {
    if (!ioState) {
        return;
    }
    ioState->open = true;
}

inline bool RenderColorPipelineWindow(ColorPipelineWindowState* ioState) {
    if (!ioState || !ioState->open) {
        return false;
    }

    bool open = ioState->open;
    ImGui::SetNextWindowSize(ImVec2(460.0f, 180.0f), ImGuiCond_FirstUseEver);
    const bool began = ImGui::Begin("Color Pipeline", &open);
    if (began) {
        ImGui::TextWrapped("Advanced Color Pipeline editing is staged behind this window.");
        ImGui::Separator();
        ImGui::TextWrapped("Slice 2 keeps Coloring Mode and Grading as the public operator-facing controls while the descriptor-driven editor lands in follow-up slices.");
    }
    ImGui::End();

    ioState->open = open;
    return true;
}