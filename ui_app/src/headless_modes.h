#pragma once

#include <string>
#include <vector>

#include "fractal_probe_contract.h"

// --- File I/O utilities ---
std::string ReadTextFile(const char* path);
bool TryReadTextFileExact(const std::string& path, std::string* outText, std::string* outError);
bool ReadStdinText(std::string* outText, std::string* outError);
bool WriteTextFileExact(const std::string& path, const std::string& text, std::string* outError);

// --- Probe helpers ---
FractalProbeResponse BuildProbeErrorResponse(const std::string& requestId,
    const std::string& exePath,
    const FractalProbeOperatorContext& operatorContext,
    const std::string& error);

bool EmitProbeResponse(const std::string& responseJson,
    bool toStdout,
    const std::string& responsePath,
    std::string* outError);

// --- Headless mode dispatch ---

struct SampleModeArgs {
    bool request_stdin = false;
    bool response_stdout = false;
    std::string request_json_path;
    std::string response_json_path;
    bool conflict_validate_ui = false;
    bool conflict_capture_diagnostic = false;
    bool conflict_capture_finding = false;
};

// Returns 0 on success, 1 on error.
int RunSampleMode(const SampleModeArgs& args, const std::string& exePath);

// Returns 0 on success, 1 on error.
int RunDescribeFunctionsMode(bool toStdout, const std::string& jsonPath,
    const std::vector<std::string>& schemaCandidates);
