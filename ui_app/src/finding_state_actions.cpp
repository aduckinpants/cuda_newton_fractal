#include "finding_state_actions.h"

#define COLOR_PIPELINE_WINDOW_NO_IMGUI
#include "color_pipeline_window.h"
#undef COLOR_PIPELINE_WINDOW_NO_IMGUI
#include "diagnostics_state_io.h"
#include "fractal_derived_fields.h"
#include "fractal_family_rules.h"
#include "view_hp_sync.h"

bool LoadFindingSelectionIntoRuntime(const std::string& selectedPath,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    std::string* outResolvedStatePath,
    std::string* outError) {
    return LoadFindingSelectionIntoRuntime(
        selectedPath,
        ioView,
        ioParams,
        ioRender,
        outOrientation,
        outHasOrientation,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        outResolvedStatePath,
        outError);
}

bool LoadFindingSelectionIntoRuntime(const std::string& selectedPath,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    ColorPipelineWindowState* outColorPipelineWindow,
    std::string* outResolvedStatePath,
    std::string* outError) {
    return LoadFindingSelectionIntoRuntime(
        selectedPath,
        ioView,
        ioParams,
        ioRender,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        outColorPipelineWindow,
        outResolvedStatePath,
        outError);
}

bool LoadFindingSelectionIntoRuntime(const std::string& selectedPath,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    SidecarAutoDemoControllerPolicy* outControllerPolicy,
    bool* outHasControllerPolicy,
    SidecarAutoDemoMutationHistory* outMutationHistory,
    bool* outHasMutationHistory,
    std::string* outResolvedStatePath,
    std::string* outError) {
    return LoadFindingSelectionIntoRuntime(
        selectedPath,
        ioView,
        ioParams,
        ioRender,
        outOrientation,
        outHasOrientation,
        outControllerPolicy,
        outHasControllerPolicy,
        outMutationHistory,
        outHasMutationHistory,
        nullptr,
        outResolvedStatePath,
        outError);
}

bool LoadFindingSelectionIntoRuntime(const std::string& selectedPath,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    SidecarAutoDemoControllerPolicy* outControllerPolicy,
    bool* outHasControllerPolicy,
    SidecarAutoDemoMutationHistory* outMutationHistory,
    bool* outHasMutationHistory,
    ColorPipelineWindowState* outColorPipelineWindow,
    std::string* outResolvedStatePath,
    std::string* outError) {
    if (outError) outError->clear();
    if (outOrientation) *outOrientation = {};
    if (outHasOrientation) *outHasOrientation = false;
    if (outControllerPolicy) *outControllerPolicy = {};
    if (outHasControllerPolicy) *outHasControllerPolicy = false;
    if (outMutationHistory) outMutationHistory->clear();
    if (outHasMutationHistory) *outHasMutationHistory = false;
    if (outColorPipelineWindow) *outColorPipelineWindow = {};
    if (!ioView || !ioParams || !ioRender) {
        if (outError) *outError = "LoadFindingSelectionIntoRuntime requires non-null output pointers";
        return false;
    }

    std::string resolvedStatePath;
    if (!ResolveFindingStateJsonPath(selectedPath, &resolvedStatePath, outError)) {
        return false;
    }

    ViewState nextView = *ioView;
    KernelParams nextParams = *ioParams;
    RenderSettings nextRender = *ioRender;
    SidecarOrientationVector nextOrientation{};
    bool nextHasOrientation = false;
    SidecarAutoDemoControllerPolicy nextControllerPolicy{};
    bool nextHasControllerPolicy = false;
    SidecarAutoDemoMutationHistory nextMutationHistory;
    bool nextHasMutationHistory = false;
    ColorPipelineWindowState nextColorPipelineWindow;
    if (!LoadDiagnosticsStateFile(
            resolvedStatePath,
            &nextView,
            &nextParams,
            &nextRender,
            &nextOrientation,
            &nextHasOrientation,
            &nextControllerPolicy,
            &nextHasControllerPolicy,
            &nextMutationHistory,
            &nextHasMutationHistory,
            &nextColorPipelineWindow,
            outError)) {
        return false;
    }

    if (IsExplainoFamily(nextView.fractal_type)) {
        UpdateExplainoPolynomial(nextView, nextParams, nullptr);
    }
    SyncViewUiFromHp(nextView);

    *ioView = nextView;
    *ioParams = nextParams;
    *ioRender = nextRender;
    if (outOrientation) *outOrientation = nextOrientation;
    if (outHasOrientation) *outHasOrientation = nextHasOrientation;
    if (outControllerPolicy) *outControllerPolicy = nextControllerPolicy;
    if (outHasControllerPolicy) *outHasControllerPolicy = nextHasControllerPolicy;
    if (outMutationHistory) *outMutationHistory = nextMutationHistory;
    if (outHasMutationHistory) *outHasMutationHistory = nextHasMutationHistory;
    if (outColorPipelineWindow) *outColorPipelineWindow = std::move(nextColorPipelineWindow);
    if (outResolvedStatePath) *outResolvedStatePath = resolvedStatePath;
    return true;
}

bool LoadFindingSelectionIntoRuntime(const std::string& selectedPath,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    std::string* outResolvedStatePath,
    std::string* outError) {
    return LoadFindingSelectionIntoRuntime(
        selectedPath,
        ioView,
        ioParams,
        ioRender,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
    nullptr,
        outResolvedStatePath,
        outError);
}