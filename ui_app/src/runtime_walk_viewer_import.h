#pragma once

#include "fractal_types.h"
#include "runtime_walk.h"
#include "runtime_walk_bootstrap.h"

#include <string>
#include <vector>

struct RuntimeWalkViewerSession;

struct RuntimeWalkViewerImportRequest {
    std::string exe_dir;
    std::string base_state_json_path;
    FractalType base_fractal_type = FractalType::newton;
    RuntimeWalkAuthorityMode authority_mode = RuntimeWalkAuthorityMode::loaded_base_state;
    std::string comparison_fits_path;
    std::string request_json_path;
    std::string bundle_json_path;
    std::string rtk_manifest_json_path;
    std::string rtk_harvest_summary_json_path;
    std::string mapping_profile_json_path;
    std::string mapping_profile_id;
    std::string orientation_inputs_json_path;
    RuntimeWalkTransportSynthesisOptions transport_options;
};

struct RuntimeWalkViewerImportSessionRecord {
    std::string session_id;
    std::string session_dir;
    RuntimeWalkAuthorityMode authority_mode = RuntimeWalkAuthorityMode::loaded_base_state;
    std::string request_json_path;
    std::string base_state_json_path;
    std::string synthesized_base_state_json_path;
    std::string comparison_fits_path;
    std::string bundle_json_path;
    std::string rtk_manifest_json_path;
    std::string rtk_harvest_summary_json_path;
    std::string mapping_profile_json_path;
    std::string mapping_profile_id;
    std::string orientation_inputs_json_path;
    std::string source_request_json_path;
    std::string source_bundle_json_path;
    std::string discovery_source;
    std::string transport_generation_mode;
    std::size_t transport_sample_count = 0;
    double transport_motion_scale = 0.75;
    double transport_warp_scale = 0.0;
    bool transport_generated = false;
    bool request_exists = false;
    bool viewer_load_succeeded = false;
};

struct RuntimeWalkViewerImportPanelState {
    bool open = false;
    std::string base_state_json_path;
    RuntimeWalkAuthorityMode authority_mode = RuntimeWalkAuthorityMode::loaded_base_state;
    std::string comparison_fits_path;
    std::string request_json_path;
    std::string bundle_json_path;
    std::string mapping_profile_json_path;
    std::string mapping_profile_id;
    RuntimeWalkTransportSynthesisOptions transport_options;
    std::string resolved_mapping_profile_json_path;
    std::string resolved_mapping_profile_base_fractal_type;
    std::vector<std::string> mapping_binding_summaries;
    std::string status_text;
    std::vector<RuntimeWalkViewerImportSessionRecord> recent_sessions;
};

bool ValidateRuntimeWalkViewerImportBaseState(RuntimeWalkAuthorityMode authorityMode,
    const std::string& baseStateJsonPath,
    FractalType baseStateFractalType,
    std::string* outError);

bool BuildRuntimeWalkViewerImportSession(const RuntimeWalkViewerImportRequest& request,
    RuntimeWalkViewerImportSessionRecord* outRecord,
    std::string* outError);

bool LoadLatestRuntimeWalkViewerImportSession(const std::string& exeDir,
    RuntimeWalkViewerImportSessionRecord* outRecord,
    std::string* outError);

bool LoadRecentRuntimeWalkViewerImportSessions(const std::string& exeDir,
    std::vector<RuntimeWalkViewerImportSessionRecord>* outRecords,
    std::string* outError);

void PrimeRuntimeWalkViewerImportPanel(const std::string& exeDir,
    const std::string& currentLoadedStatePath,
    const RuntimeWalkViewerSession& session,
    RuntimeWalkViewerImportPanelState* ioPanel);

bool RefreshMappingProfileDisplayState(const std::string& exeDir,
    RuntimeWalkViewerImportPanelState* ioPanel,
    std::string* outError);

bool NoteRuntimeWalkViewerImportSessionLoadSucceeded(const std::string& requestJsonPath,
    std::string* outError);

bool RuntimeWalkViewerImportHasOpenInput(const RuntimeWalkViewerImportPanelState& state);
