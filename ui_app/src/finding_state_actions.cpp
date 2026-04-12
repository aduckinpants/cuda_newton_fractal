#include "finding_state_actions.h"

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
    if (outError) outError->clear();
    if (outOrientation) *outOrientation = {};
    if (outHasOrientation) *outHasOrientation = false;
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
    if (!LoadDiagnosticsStateFile(
            resolvedStatePath,
            &nextView,
            &nextParams,
            &nextRender,
            &nextOrientation,
            &nextHasOrientation,
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
        outResolvedStatePath,
        outError);
}