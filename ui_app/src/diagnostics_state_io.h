#pragma once

#include "explaino_sidecar_controller.h"
#include "explaino_sidecar_model.h"
#include "fractal_types.h"

#include <string>

struct ColorPipelineWindowState;

bool LoadDiagnosticsStateJson(const std::string& text,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    std::string* outError);

bool LoadDiagnosticsStateJson(const std::string& text,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    ColorPipelineWindowState* outColorPipelineWindow,
    std::string* outError);

bool LoadDiagnosticsStateJson(const std::string& text,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    std::string* outError);

bool LoadDiagnosticsStateJson(const std::string& text,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    SidecarAutoDemoControllerPolicy* outControllerPolicy,
    bool* outHasControllerPolicy,
    std::string* outError);

bool LoadDiagnosticsStateJson(const std::string& text,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    SidecarAutoDemoControllerPolicy* outControllerPolicy,
    bool* outHasControllerPolicy,
    SidecarAutoDemoMutationHistory* outMutationHistory,
    bool* outHasMutationHistory,
    std::string* outError);

bool LoadDiagnosticsStateJson(const std::string& text,
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
    std::string* outError);

bool LoadDiagnosticsStateFile(const std::string& path,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    std::string* outError);

bool LoadDiagnosticsStateFile(const std::string& path,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    ColorPipelineWindowState* outColorPipelineWindow,
    std::string* outError);

bool LoadDiagnosticsStateFile(const std::string& path,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    std::string* outError);

bool LoadDiagnosticsStateFile(const std::string& path,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    SidecarAutoDemoControllerPolicy* outControllerPolicy,
    bool* outHasControllerPolicy,
    std::string* outError);

bool LoadDiagnosticsStateFile(const std::string& path,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    SidecarAutoDemoControllerPolicy* outControllerPolicy,
    bool* outHasControllerPolicy,
    SidecarAutoDemoMutationHistory* outMutationHistory,
    bool* outHasMutationHistory,
    std::string* outError);

bool LoadDiagnosticsStateFile(const std::string& path,
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
    std::string* outError);

bool DiagnosticsStateJsonHasExplicitExplainoRoots(const std::string& text,
    bool* outHasExplicitExplainoRoots,
    std::string* outError);

bool DiagnosticsStateFileHasExplicitExplainoRoots(const std::string& path,
    bool* outHasExplicitExplainoRoots,
    std::string* outError);

bool ResolveFindingStateJsonPath(const std::string& selectedPath,
    std::string* outStateJsonPath,
    std::string* outError);