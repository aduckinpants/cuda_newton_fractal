#pragma once

#include "fractal_types.h"
#include "runtime_walk.h"

#include <cstddef>
#include <string>
#include <vector>

struct RuntimeWalkViewerCompanionArtifacts {
    std::string comparison_fits_path;
    std::string rtk_manifest_json_path;
    std::string rtk_harvest_summary_json_path;
};

struct RuntimeWalkViewerAuthorityInfo {
    RuntimeWalkAuthorityMode mode = RuntimeWalkAuthorityMode::loaded_base_state;
    std::string resolved_base_state_json_path;
    std::string synthesized_base_state_json_path;
    std::string mapping_profile_json_path;
    std::string mapping_profile_id;
    std::string orientation_inputs_json_path;
};

struct RuntimeWalkViewerAsset {
    RuntimeWalkRequest request{};
    RuntimeWalkBundle bundle{};
    ViewState base_view{};
    KernelParams base_params{};
    RenderSettings base_render{};
    RuntimeWalkViewerAuthorityInfo authority{};
    RuntimeWalkViewerCompanionArtifacts companion{};
    std::vector<RuntimeWalkSnapshot> tick_snapshots;
};

struct RuntimeWalkViewerPlaybackState {
    bool loaded = false;
    bool playing = true;
    bool loop = true;
    double speed = 0.20;
    double current_t = 0.0;
    std::size_t nearest_tick_index = 0;
    bool show_raw_path = true;
    bool show_spline_path = true;
    bool show_closed_loop = false;
    bool show_branch_markers = true;
    bool show_gradient_overlay = true;
};

struct RuntimeWalkOverlayPath {
    std::vector<Double2> raw_points;
    std::vector<Double2> spline_points;
    std::vector<Double2> closed_loop_points;
    std::vector<Double2> branch_marker_points;
    Double2 current_point{0.5, 0.5};
};

struct RuntimeWalkGradientOverlayGuidePoint {
    Double2 point{0.0, 0.0};
    double strength = 0.0;
};

struct RuntimeWalkGradientOverlayGuideStroke {
    std::vector<RuntimeWalkGradientOverlayGuidePoint> points;
    double strength = 0.0;
};

struct RuntimeWalkGradientOverlay {
    std::vector<RuntimeWalkGradientOverlayGuideStroke> strokes;
};

enum class RuntimeWalkOverlayProviderKind {
    none = 0,
    runtime_local_gradient = 1,
    external_rtk_reserved = 2,
};

struct RuntimeWalkOverlayProviderConfig {
    RuntimeWalkOverlayProviderKind kind = RuntimeWalkOverlayProviderKind::runtime_local_gradient;
    double threshold = 0.08;
    int max_strokes = 12;
    int max_steps_per_stroke = 8;
    double branch_bias = 0.25;
};

struct RuntimeWalkOverlayProviderInputs {
    double decode_stability = 1.0;
    double divergence = 0.0;
    double branch_proximity = 0.0;
};

bool BuildRuntimeWalkViewerAsset(const RuntimeWalkRequest& request,
    const RuntimeWalkBundle& bundle,
    const ViewState& baseView,
    const KernelParams& baseParams,
    const RenderSettings& baseRender,
    RuntimeWalkViewerAsset* outAsset,
    std::string* outError);

bool SeekRuntimeWalkViewerPlayback(const RuntimeWalkViewerAsset& asset,
    double t,
    RuntimeWalkViewerPlaybackState* ioState,
    bool* outChanged);

bool AdvanceRuntimeWalkViewerPlayback(const RuntimeWalkViewerAsset& asset,
    double deltaSeconds,
    RuntimeWalkViewerPlaybackState* ioState,
    bool* outChanged);

bool StepRuntimeWalkViewerPlayback(const RuntimeWalkViewerAsset& asset,
    int direction,
    RuntimeWalkViewerPlaybackState* ioState,
    bool* outChanged);

bool EvaluateRuntimeWalkViewerCurrentSnapshot(const RuntimeWalkViewerAsset& asset,
    const RuntimeWalkViewerPlaybackState& playback,
    RuntimeWalkSnapshot* outSnapshot,
    std::string* outError);

void BuildRuntimeWalkOverlayPath(const RuntimeWalkViewerAsset& asset,
    const RuntimeWalkViewerPlaybackState& playback,
    RuntimeWalkOverlayPath* outPath);

bool BuildRuntimeWalkGradientOverlay(const RuntimeWalkViewerAsset& asset,
    const RuntimeWalkViewerPlaybackState& playback,
    const RuntimeWalkOverlayProviderConfig& config,
    const RuntimeWalkOverlayProviderInputs& inputs,
    RuntimeWalkGradientOverlay* outOverlay,
    std::string* outError);
