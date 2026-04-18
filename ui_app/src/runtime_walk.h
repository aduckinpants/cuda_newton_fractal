#pragma once

#include "explaino_sidecar_controller.h"
#include "fractal_types.h"

#include <array>
#include <string>
#include <vector>

struct BindingContext;
struct EngineFunctionCatalog;
struct SidecarBudgetState;
struct SidecarOrientationVector;
struct ViewState;
class CudaSidecarMeasurementHost;

struct RuntimeWalkBundleSample {
    std::string id;
    double t = 0.0;
    std::array<double, 13> channels{};
};

struct RuntimeWalkBranchMarker {
    std::string id;
    std::string label;
    std::string parent_id;
    double t = 0.0;
    double sticky_radius = 0.08;
};

struct RuntimeWalkBundle {
    std::string field_name;
    std::vector<RuntimeWalkBundleSample> samples;
    std::vector<RuntimeWalkBranchMarker> branch_markers;
};

struct RuntimeWalkBranchAnnotation {
    std::string nearest_marker_id;
    std::string nearest_marker_label;
    std::string parent_id;
    bool sticky = false;
    double distance = 0.0;
    double proximity = 0.0;
};

struct RuntimeWalkSnapshot {
    double t = 0.0;
    std::array<double, 13> channels{};
    double seed01 = 0.0;
    double combined_seed = 0.0;
    double seed_b = 0.0;
    double mix = 0.0;
    double warp_strength = 0.0;
    double phase = 0.0;
    double center_hp_x = 0.0;
    double center_hp_y = 0.0;
    double log2_zoom = 0.0;
    double dx_world = 0.0;
    double dy_world = 0.0;
    double dlog2_zoom = 0.0;
    RuntimeWalkBranchAnnotation branch;
};

enum class RuntimeWalkAuthorityMode {
    loaded_base_state = 0,
    synthesized_fits_base = 1,
};

struct RuntimeWalkRequest {
    RuntimeWalkAuthorityMode authority_mode = RuntimeWalkAuthorityMode::loaded_base_state;
    std::string base_state_json_path;
    std::string bundle_json_path;
    std::string output_dir;
    std::vector<double> t_values;
    std::string comparison_fits_path;
    std::string rtk_manifest_json_path;
    std::string rtk_harvest_summary_json_path;
    std::string mapping_profile_json_path;
    std::string mapping_profile_id;
    std::string orientation_inputs_json_path;
    bool transport_generated = false;
    std::string transport_generation_mode;
    std::size_t transport_sample_count = 0;
    double transport_motion_scale = 0.75;
};

const char* RuntimeWalkAuthorityModeId(RuntimeWalkAuthorityMode mode);
bool TryParseRuntimeWalkAuthorityModeId(const std::string& text, RuntimeWalkAuthorityMode* outMode);

bool ParseRuntimeWalkBundleJson(const std::string& jsonText,
    RuntimeWalkBundle* outBundle,
    std::string* outError);

bool LoadRuntimeWalkBundleFile(const std::string& path,
    RuntimeWalkBundle* outBundle,
    std::string* outError);

bool ParseRuntimeWalkRequestJson(const std::string& jsonText,
    RuntimeWalkRequest* outRequest,
    std::string* outError);

bool LoadRuntimeWalkRequestFile(const std::string& path,
    RuntimeWalkRequest* outRequest,
    std::string* outError);

bool EvaluateRuntimeWalkSnapshot(const RuntimeWalkBundle& bundle,
    double t,
    const ViewState& baseView,
    const KernelParams& baseParams,
    const RenderSettings& render,
    RuntimeWalkSnapshot* outSnapshot,
    std::string* outError);

void ApplyRuntimeWalkSnapshot(const RuntimeWalkSnapshot& snapshot,
    ViewState* ioView,
    KernelParams* ioParams);

int RunRuntimeWalkRequest(const std::string& exeDir,
    const std::string& requestJsonPath,
    const EngineFunctionCatalog& engineCatalog,
    BindingContext& bind,
    CudaSidecarMeasurementHost& measurementHost,
    const SidecarAutoDemoControllerPolicy& sidecarControllerPolicy,
    LensSettings& lens);
