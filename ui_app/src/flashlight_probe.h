#pragma once

#include "fractal_types.h"
#include "sdf_field_signal.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <string>

struct FlashlightProbeConfig {
    std::string seed_path;
    int ticks = 8;
    double radius = 0.75;
    double zoom_radius = 0.25;
    double warp = 0.0;
    bool closure_last = false;
    int closure_ref_t = 0;
    bool have_fractal_type = false;
    FractalType fractal_type = FractalType::explaino_fp;
};

struct FlashlightManifoldStep {
    int band = 0;
    double band_scale = 1.0;
    double rx = 0.0;
    double ry = 0.0;
    double rz = 0.0;
    double log2_zoom_tick = 0.0;
    double dx_world = 0.0;
    double dy_world = 0.0;
};

FlashlightProbeConfig NormalizeFlashlightProbeConfig(const FlashlightProbeConfig& config);
uint32_t FlashlightFnv1a32(const void* data, std::size_t sizeBytes);
std::array<uint32_t, 8> ComputeFlashlightConversationSpectrum8(const std::string& text);
int NormalizeFlashlightLensDownsamplePow2(int value);
FlashlightManifoldStep FlashlightManifoldAt(
    uint32_t seed32,
    int walkT,
    int bands,
    double baseLog2,
    double radius,
    double zoomRadius,
    double aspect);
void AppendFlashlightSdfSignalJsonFields(std::ostream& json, const SdfFieldSignalSample& signals);

int RunFlashlightProbe(const std::string& exeDir,
    const FlashlightProbeConfig& config,
    ViewState& view,
    KernelParams& params,
    RenderSettings& render,
    LensSettings& lens);
