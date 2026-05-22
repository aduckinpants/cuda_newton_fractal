#pragma once

#include "diagnostics_capture.h"
#include "explaino_sidecar_controller.h"
#include "explaino_sidecar_model.h"
#include "fractal_types.h"

#include <cstddef>
#include <filesystem>
#include <string>

struct FindingArchiveIdentity {
    std::filesystem::path out_root;
    std::string finding_id;
    std::filesystem::path output_dir;
};

FindingArchiveIdentity BuildUniqueFindingIdentity(
    const std::filesystem::path& findingsRoot,
    const std::string& group,
    const std::string& dateLabel,
    const std::string& timeLabel,
    FractalType fractalType);

std::filesystem::path FindRepoRootContainingArchiveScript(const std::filesystem::path& startPath);

std::filesystem::path FindRepoRootFromRuntimeMetadata(const std::filesystem::path& runtimeDir);

RenderSettings BuildFindingArchiveCaptureRender(const RenderSettings& render);

RenderSettings BuildFindingArchiveCaptureRenderForSource(
    const RenderSettings& render,
    const Int2& sourceResolution);

std::wstring BuildArchiveScriptCommandLine(
    const std::filesystem::path& pythonLauncher,
    const std::filesystem::path& scriptPath,
    const std::filesystem::path& repoRoot,
    const std::filesystem::path& diagnosticsDir,
    const std::filesystem::path& outRoot,
    const std::string& findingId,
    const std::string& why,
    const std::string& reproCommand);

bool CaptureAndArchiveFindingBundle(
    const std::string& exeDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const ColorPipelineWindowState* colorPipelineWindow,
    const std::string& group,
    const std::string& why,
    std::string* outFindingDir,
    std::string* outError);

bool CaptureAndArchiveFindingBundle(
    const std::string& exeDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const SidecarOrientationVector* sidecarOrientation,
    const SidecarAutoDemoControllerPolicy* sidecarControllerPolicy,
    const std::string& group,
    const std::string& why,
    std::string* outFindingDir,
    std::string* outError);

bool CaptureAndArchiveFindingBundle(
    const std::string& exeDir,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const RenderStats& stats,
    const uint32_t* rgba,
    std::size_t rgbaPixelCount,
    const SidecarOrientationVector* sidecarOrientation,
    const SidecarAutoDemoControllerPolicy* sidecarControllerPolicy,
    const SidecarAutoDemoMutationHistory* sidecarMutationHistory,
    const std::string& group,
    const std::string& why,
    std::string* outFindingDir,
    std::string* outError);

bool CaptureAndArchiveFindingBundle(
    const std::string& exeDir,
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
    const std::string& group,
    const std::string& why,
    std::string* outFindingDir,
    std::string* outError);
