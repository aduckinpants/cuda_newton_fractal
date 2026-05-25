#include "finding_state_actions.h"

#define COLOR_PIPELINE_WINDOW_NO_IMGUI
#include "color_pipeline_window.h"
#undef COLOR_PIPELINE_WINDOW_NO_IMGUI
#include "diagnostics_state_io.h"
#include "fractal_derived_fields.h"
#include "fractal_family_rules.h"
#include "view_hp_sync.h"

#include <filesystem>

namespace {

bool LoadResolvedStateJsonIntoRuntime(const std::string& resolvedStatePath,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    SidecarAutoDemoControllerPolicy* outControllerPolicy,
    bool* outHasControllerPolicy,
    SidecarAutoDemoMutationHistory* outMutationHistory,
    bool* outHasMutationHistory,
    LensSettings* ioLens,
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
        if (outError) *outError = "LoadResolvedStateJsonIntoRuntime requires non-null output pointers";
        return false;
    }

    ViewState nextView = *ioView;
    KernelParams nextParams = *ioParams;
    RenderSettings nextRender = *ioRender;
    LensSettings nextLens{};
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
            ioLens ? &nextLens : nullptr,
            &nextColorPipelineWindow,
            outError)) {
        return false;
    }

    bool hasExplicitExplainoRoots = false;
    if (!DiagnosticsStateFileHasExplicitExplainoRoots(resolvedStatePath, &hasExplicitExplainoRoots, outError)) {
        return false;
    }
    if (IsExplainoFamily(nextView.fractal_type) && !hasExplicitExplainoRoots) {
        UpdateExplainoPolynomial(nextView, nextParams, nullptr);
    }
    SyncViewUiFromHp(nextView);

    *ioView = nextView;
    *ioParams = nextParams;
    *ioRender = nextRender;
    if (ioLens) *ioLens = nextLens;
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

} // namespace

bool LoadExplicitStateJsonIntoRuntime(const std::string& stateJsonPath,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    LensSettings* ioLens,
    ColorPipelineWindowState* outColorPipelineWindow,
    std::string* outResolvedStatePath,
    std::string* outError) {
    const std::string resolvedStatePath = std::filesystem::path(stateJsonPath).lexically_normal().string();
    return LoadResolvedStateJsonIntoRuntime(
        resolvedStatePath,
        ioView,
        ioParams,
        ioRender,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        ioLens,
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
    LensSettings* ioLens,
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

    return LoadResolvedStateJsonIntoRuntime(
        resolvedStatePath,
        ioView,
        ioParams,
        ioRender,
        outOrientation,
        outHasOrientation,
        outControllerPolicy,
        outHasControllerPolicy,
        outMutationHistory,
        outHasMutationHistory,
        ioLens,
        outColorPipelineWindow,
        outResolvedStatePath,
        outError);
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
