#pragma once

#include <iostream>
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

// --- Session mode (V2-B) ---

// Process a single session line given current state.  Returns the JSON
// response line (without trailing newline).  Sets *sessionDone = true
// when the session should exit.
// stateTokenCounter is the monotonic counter for state_token generation.
// Returns empty string if line was empty/whitespace (caller should skip).
std::string ProcessSessionLine(const std::string& line,
    bool* sessionOpen,
    bool* sessionDone,
    int* stateTokenCounter,
    const std::string& exePath);

// Run session mode reading from `in` and writing to `out`.
// Each request/response is a single JSON line.
// Returns 0 on clean close, 1 on error.
int RunSessionMode(std::istream& in, std::ostream& out,
    const std::string& exePath);
