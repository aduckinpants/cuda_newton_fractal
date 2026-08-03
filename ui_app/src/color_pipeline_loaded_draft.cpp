#include "color_pipeline_loaded_draft.h"

#define COLOR_PIPELINE_WINDOW_NO_IMGUI
#include "color_pipeline_window.h"
#undef COLOR_PIPELINE_WINDOW_NO_IMGUI

#include <sstream>
#include <utility>

namespace {

std::string DescribeLoadedDraftApplyError(
    const ColorPipelineWindowState& state,
    const char* fallback) {
    if (state.validation_messages.empty()) {
        return fallback ? std::string(fallback) : std::string();
    }

    std::ostringstream message;
    for (std::size_t index = 0; index < state.validation_messages.size(); ++index) {
        if (index > 0) {
            message << "; ";
        }
        message << state.validation_messages[index];
    }
    return message.str();
}

} // namespace

bool ApplyLoadedColorPipelineDraftToRuntime(
    ColorPipelineWindowState* ioDraft,
    FractalType liveFractalType,
    KernelParams* ioParams,
    ColorPipelineLoadedDraftApplyResult* outResult,
    std::string* outError) {
    if (outResult) {
        *outResult = {};
    }
    if (outError) {
        outError->clear();
    }
    if (!ioDraft) {
        if (outError) {
            *outError = "Loaded Color Pipeline draft application requires a draft state.";
        }
        return false;
    }
    if (!ioParams) {
        if (outError) {
            *outError = "Loaded Color Pipeline draft application requires live runtime parameters.";
        }
        return false;
    }

    ColorPipelineWindowState candidateDraft = *ioDraft;
    KernelParams candidateParams = *ioParams;
    ClearColorPipelineValidationMessages(&candidateDraft);
    bool changed = false;
    if (!ApplyColorPipelineDraftToLiveState(
            &candidateDraft, liveFractalType, &candidateParams, &changed)) {
        if (outError) {
            *outError = DescribeLoadedDraftApplyError(
                candidateDraft,
                "Failed to apply the loaded Color Pipeline draft to live runtime state.");
        }
        ioDraft->validation_messages = std::move(candidateDraft.validation_messages);
        return false;
    }

    if (!candidateDraft.live_snapshot.draft_import_supported) {
        PushColorPipelineValidationMessage(
            &candidateDraft,
            "Loaded Color Pipeline draft applied in isolation, but the live runtime cannot provide authoritative draft readback.");
        if (outError) {
            *outError = DescribeLoadedDraftApplyError(
                candidateDraft,
                "Loaded Color Pipeline draft has no authoritative runtime readback.");
        }
        ioDraft->validation_messages = std::move(candidateDraft.validation_messages);
        return false;
    }

    *ioDraft = std::move(candidateDraft);
    *ioParams = std::move(candidateParams);
    if (outResult) {
        outResult->changed = changed;
    }
    return true;
}
