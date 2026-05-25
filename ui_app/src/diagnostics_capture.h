#pragma once

#include "explaino_sidecar_controller.h"
#include "explaino_sidecar_model.h"
#include "fractal_types.h"

#include <cstddef>
#include <string>

struct ColorPipelineWindowState;

struct DiagnosticsCaptureResult {
    std::string output_dir;
    std::string frame_bmp_path;
    std::string state_json_path;
};

bool CaptureDiagnosticsLastBundle(const std::string& exeDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    DiagnosticsCaptureResult* outResult,
    std::string* outError);

bool CaptureDiagnosticsLastBundle(const std::string& exeDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const ColorPipelineWindowState* colorPipelineWindow,
    DiagnosticsCaptureResult* outResult,
    std::string* outError);

bool CaptureDiagnosticsLastBundleWithLens(const std::string& exeDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const SidecarOrientationVector* sidecarOrientation,
    const SidecarAutoDemoControllerPolicy* sidecarControllerPolicy,
    const SidecarAutoDemoMutationHistory* sidecarMutationHistory,
    const ColorPipelineWindowState* colorPipelineWindow,
    const LensSettings* lens,
    DiagnosticsCaptureResult* outResult,
    std::string* outError);

bool CaptureDiagnosticsLastBundle(const std::string& exeDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const SidecarOrientationVector* sidecarOrientation,
    DiagnosticsCaptureResult* outResult,
    std::string* outError);

bool CaptureDiagnosticsLastBundle(const std::string& exeDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const SidecarOrientationVector* sidecarOrientation,
    const ColorPipelineWindowState* colorPipelineWindow,
    DiagnosticsCaptureResult* outResult,
    std::string* outError);

bool CaptureDiagnosticsLastBundle(const std::string& exeDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const SidecarOrientationVector* sidecarOrientation,
    const SidecarAutoDemoControllerPolicy* sidecarControllerPolicy,
    DiagnosticsCaptureResult* outResult,
    std::string* outError);

bool CaptureDiagnosticsLastBundle(const std::string& exeDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const SidecarOrientationVector* sidecarOrientation,
    const SidecarAutoDemoControllerPolicy* sidecarControllerPolicy,
    const SidecarAutoDemoMutationHistory* sidecarMutationHistory,
    DiagnosticsCaptureResult* outResult,
    std::string* outError);

bool CaptureDiagnosticsLastBundle(const std::string& exeDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const SidecarOrientationVector* sidecarOrientation,
    const SidecarAutoDemoControllerPolicy* sidecarControllerPolicy,
    const SidecarAutoDemoMutationHistory* sidecarMutationHistory,
    const ColorPipelineWindowState* colorPipelineWindow,
    DiagnosticsCaptureResult* outResult,
    std::string* outError);

bool CaptureDiagnosticsBundleToDir(const std::string& outputDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    DiagnosticsCaptureResult* outResult,
    std::string* outError);

bool CaptureDiagnosticsBundleToDir(const std::string& outputDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const SidecarOrientationVector* sidecarOrientation,
    DiagnosticsCaptureResult* outResult,
    std::string* outError);

bool CaptureDiagnosticsBundleToDir(const std::string& outputDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const SidecarOrientationVector* sidecarOrientation,
    const SidecarAutoDemoControllerPolicy* sidecarControllerPolicy,
    DiagnosticsCaptureResult* outResult,
    std::string* outError);

bool CaptureDiagnosticsBundleToDir(const std::string& outputDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const SidecarOrientationVector* sidecarOrientation,
    const SidecarAutoDemoControllerPolicy* sidecarControllerPolicy,
    const SidecarAutoDemoMutationHistory* sidecarMutationHistory,
    DiagnosticsCaptureResult* outResult,
    std::string* outError);

bool CaptureDiagnosticsBundleToDir(const std::string& outputDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const SidecarOrientationVector* sidecarOrientation,
    const SidecarAutoDemoControllerPolicy* sidecarControllerPolicy,
    const SidecarAutoDemoMutationHistory* sidecarMutationHistory,
    const ColorPipelineWindowState* colorPipelineWindow,
    DiagnosticsCaptureResult* outResult,
    std::string* outError);

bool CaptureDiagnosticsBundleToDirWithLens(const std::string& outputDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const SidecarOrientationVector* sidecarOrientation,
    const SidecarAutoDemoControllerPolicy* sidecarControllerPolicy,
    const SidecarAutoDemoMutationHistory* sidecarMutationHistory,
    const ColorPipelineWindowState* colorPipelineWindow,
    const LensSettings* lens,
    DiagnosticsCaptureResult* outResult,
    std::string* outError);
