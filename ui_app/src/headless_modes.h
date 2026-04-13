#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "explaino_sidecar_controller.h"
#include "fractal_probe_contract.h"

struct BindingContext;
struct EngineFunctionCatalog;
struct ExplainoSidecarWindowState;
struct KernelParams;
struct SidecarBudgetState;
struct SidecarOrientationVector;
struct ViewState;
class SidecarMeasurementHost;

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

struct SidecarHeadlessProofConfig {
    int apply_armed_step_count = 0;
    double pump_paced_loop_seconds = 0.0;
};

bool HasSidecarHeadlessProofActions(const SidecarHeadlessProofConfig& config);

bool ApplyHeadlessSidecarProofActions(
    const SidecarHeadlessProofConfig& config,
    ViewState& view,
    KernelParams& params,
    const EngineFunctionCatalog& engineCatalog,
    BindingContext& bind,
    SidecarMeasurementHost& measurementHost,
    const SidecarAutoDemoControllerPolicy& sidecarControllerPolicy,
    SidecarOrientationVector& loadedOrientationBaseline,
    bool& loadedOrientationBaselineValid,
    SidecarAutoDemoMutationHistory& sidecarMutationHistory,
    bool& sidecarMutationHistoryValid,
    ExplainoSidecarWindowState& sidecarState,
    bool& sidecarStateValid,
    SidecarBudgetState& sidecarBudgetState,
    bool& sidecarBudgetStateValid,
    std::string* outError);

// --- V2-C: Override merge ---

// Merge two override vectors.  For each override in `diff`, if an override
// with the same path exists in `base`, replace its value; otherwise append.
// Order: base overrides first (with replacements applied), then new paths
// from diff appended in order.
std::vector<FractalProbeOverride> MergeOverrides(
    const std::vector<FractalProbeOverride>& base,
    const std::vector<FractalProbeOverride>& diff);

// --- Session mode (V2-B / V2-C) ---

// Accumulated override state for a session.  Maps state_token string to
// the full override vector that produced that token.
using SessionOverrideMap = std::map<std::string, std::vector<FractalProbeOverride>>;

// Process a single session line given current state.  Returns the JSON
// response line (without trailing newline).  Sets *sessionDone = true
// when the session should exit.
// stateTokenCounter is the monotonic counter for state_token generation.
// accumulatedOverrides stores the override vector for each state_token.
// Returns empty string if line was empty/whitespace (caller should skip).
std::string ProcessSessionLine(const std::string& line,
    bool* sessionOpen,
    bool* sessionDone,
    int* stateTokenCounter,
    SessionOverrideMap* accumulatedOverrides,
    const std::string& exePath);

// Run session mode reading from `in` and writing to `out`.
// Each request/response is a single JSON line.
// Returns 0 on clean close, 1 on error.
int RunSessionMode(std::istream& in, std::ostream& out,
    const std::string& exePath);

// Build the full Windows named-pipe path for a bounded session pipe name.
// Pipe names must be non-empty and contain only ASCII letters, numbers,
// underscore, dash, or dot.
bool TryBuildSessionPipePath(const std::string& pipeName,
    std::string* outPipePath,
    std::string* outError);

// Run session mode over a Windows named pipe. The pipe transport reuses the
// same one-line JSON session protocol as stdin/stdout session mode.
int RunNamedPipeSessionMode(const std::string& pipeName,
    const std::string& exePath);
