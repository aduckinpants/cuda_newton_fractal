#pragma once

#include "fractal_types.h"
#include "runtime_walk.h"

#include <string>
#include <vector>

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
    bool request_exists = false;
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
