#pragma once

#include "explaino_sidecar_measurement.h"
#include "fractal_types.h"

#include <cstdint>
#include <string>
#include <vector>

struct RuntimeWalkFieldSlimeConfig {
    int min_marbles = 16;
    int max_marbles = 96;
    int max_steps = 3;
    int grid_resolution = 32;
    double gradient_sensitivity = 1.0;
    double hysteresis = 0.35;
    int export_cadence = 4;
    double finite_difference_step = 0.0025;
    double step_scale = 0.015;
};

struct RuntimeWalkFieldSlimeMarble {
    int marble_id = 0;
    Double2 previous_world{0.0, 0.0};
    Double2 world{0.0, 0.0};
    Double2 tangent{0.0, 0.0};
    double tangent_angle = 0.0;
    double residual = 0.0;
    int iterations = 0;
    double score = 0.0;
    int cell_id = -1;
    int traveler_cluster_id = -1;
    bool active = true;
    std::string stop_reason;
};

struct RuntimeWalkFieldSlimeCell {
    int cell_id = -1;
    int marble_count = 0;
    Double2 centroid_world{0.0, 0.0};
    double score = 0.0;
};

struct RuntimeWalkFieldSlimeTraveler {
    int cluster_id = -1;
    int marble_count = 0;
    Double2 centroid_world{0.0, 0.0};
    double confidence = 0.0;
};

struct RuntimeWalkFieldSlimeState {
    std::uint32_t seed = 0;
    double t = 0.0;
    int actual_sample_count = 0;
    std::vector<RuntimeWalkFieldSlimeMarble> marbles;
    std::vector<RuntimeWalkFieldSlimeCell> cells;
    RuntimeWalkFieldSlimeTraveler traveler;
    int export_sequence = 0;
    bool export_due = false;
};

int ComputeRuntimeWalkFieldSlimeAdaptiveMarbleCount(const RuntimeWalkFieldSlimeConfig& config,
    double gradientMagnitude,
    double tangentCurvature,
    double bindingVelocity,
    double clusterSpread);

bool RuntimeWalkFieldSlimeNeedsResetForSeed(const RuntimeWalkFieldSlimeState& state,
    std::uint32_t seed);

bool InitializeRuntimeWalkFieldSlime(const RuntimeWalkFieldSlimeConfig& config,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    std::uint32_t seed,
    RuntimeWalkFieldSlimeState* outState,
    std::string* outError);

bool StepRuntimeWalkFieldSlime(const SidecarMeasurementHost& measurementHost,
    const RuntimeWalkFieldSlimeConfig& config,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    double t,
    RuntimeWalkFieldSlimeState* ioState,
    std::string* outError);

bool WriteRuntimeWalkFieldSlimeCsv(const RuntimeWalkFieldSlimeState& state,
    const std::string& flowLinesCsvPath,
    const std::string& fieldCellsCsvPath,
    std::string* outError);

