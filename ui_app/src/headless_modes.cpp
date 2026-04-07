#include "headless_modes.h"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>

#include "fractal_probe_runner.h"
#include "function_descriptor.h"
#include "json_min.h"
#include "ui_schema.h"

// --- File I/O utilities ---

std::string ReadTextFile(const char* path) {
    std::ifstream f(path, std::ios::in | std::ios::binary);
    if (!f) return {};
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

bool TryReadTextFileExact(const std::string& path, std::string* outText, std::string* outError) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        if (outError) *outError = "Failed to open sample request file: " + path;
        return false;
    }
    std::ostringstream text;
    text << file.rdbuf();
    if (!file.good() && !file.eof()) {
        if (outError) *outError = "Failed to read sample request file: " + path;
        return false;
    }
    if (outText) *outText = text.str();
    return true;
}

bool ReadStdinText(std::string* outText, std::string* outError) {
    std::ostringstream text;
    text << std::cin.rdbuf();
    if (!std::cin.good() && !std::cin.eof()) {
        if (outError) *outError = "Failed to read sample request from stdin";
        return false;
    }
    if (outText) *outText = text.str();
    return true;
}

bool WriteTextFileExact(const std::string& path, const std::string& text, std::string* outError) {
    std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file) {
        if (outError) *outError = "Failed to open sample response file for write: " + path;
        return false;
    }
    file.write(text.data(), static_cast<std::streamsize>(text.size()));
    if (!file.good()) {
        if (outError) *outError = "Failed to write sample response file: " + path;
        return false;
    }
    return true;
}

// --- Probe helpers ---

FractalProbeResponse BuildProbeErrorResponse(const std::string& requestId,
    const std::string& exePath,
    const FractalProbeOperatorContext& operatorContext,
    const std::string& error) {
    FractalProbeResponse response;
    response.request_id = requestId;
    response.ok = false;
    response.runtime.exe_path = exePath;
    response.operator_context = operatorContext;
    response.error = error;
    return response;
}

bool EmitProbeResponse(const std::string& responseJson,
    bool toStdout,
    const std::string& responsePath,
    std::string* outError) {
    if (toStdout) {
        if (std::fwrite(responseJson.data(), 1, responseJson.size(), stdout) != responseJson.size()) {
            if (outError) *outError = "Failed to write sample response to stdout";
            return false;
        }
        std::fflush(stdout);
    }
    if (!responsePath.empty()) {
        if (!WriteTextFileExact(responsePath, responseJson, outError)) return false;
    }
    return true;
}

// --- Headless mode dispatch ---

int RunSampleMode(const SampleModeArgs& args, const std::string& exePath) {
    const bool haveRequestJson = !args.request_json_path.empty();
    const bool haveResponseJson = !args.response_json_path.empty();
    const int sourceCount = (args.request_stdin ? 1 : 0) + (haveRequestJson ? 1 : 0);

    std::string error;
    std::string requestText;
    FractalProbeRequest request;
    bool haveParsedRequest = false;

    if (sourceCount != 1) {
        error = "sample mode requires exactly one request source";
    } else if (!args.response_stdout && !haveResponseJson) {
        error = "sample mode requires at least one response sink";
    } else if (args.conflict_validate_ui || args.conflict_capture_diagnostic || args.conflict_capture_finding) {
        error = "sample mode is mutually exclusive with --validate-ui, --capture-diagnostic, and --capture-finding";
    } else if (!(args.request_stdin
            ? ReadStdinText(&requestText, &error)
            : TryReadTextFileExact(args.request_json_path, &requestText, &error))) {
    } else if (!ParseFractalProbeRequestJson(requestText, &request, &error)) {
    } else {
        haveParsedRequest = true;
        FractalProbeResponse response;
        if (!RunFractalProbeRequest(request, exePath, &response, &error)) {
            response = BuildProbeErrorResponse(request.request_id, exePath, request.operator_context, error);
        }
        const std::string responseJson = SerializeFractalProbeResponseJson(response);
        std::string emitError;
        if (!EmitProbeResponse(responseJson,
                args.response_stdout,
                haveResponseJson ? args.response_json_path : std::string(),
                &emitError)) {
            std::fprintf(stderr, "%s\n", emitError.c_str());
            return 1;
        }
        return response.ok ? 0 : 1;
    }

    FractalProbeResponse response = BuildProbeErrorResponse(
        haveParsedRequest ? request.request_id : std::string(),
        exePath,
        haveParsedRequest ? request.operator_context : FractalProbeOperatorContext{},
        error);
    const std::string responseJson = SerializeFractalProbeResponseJson(response);
    std::string emitError;
    if ((args.response_stdout || haveResponseJson) &&
        EmitProbeResponse(responseJson,
            args.response_stdout,
            haveResponseJson ? args.response_json_path : std::string(),
            &emitError)) {
        return 1;
    }
    if (!emitError.empty()) {
        std::fprintf(stderr, "%s\n", emitError.c_str());
    }
    std::fprintf(stderr, "%s\n", error.c_str());
    return 1;
}

int RunDescribeFunctionsMode(bool toStdout, const std::string& jsonPath,
    const std::vector<std::string>& schemaCandidates) {
    UISchema descSchema;
    bool descSchemaLoaded = false;
    for (const auto& cand : schemaCandidates) {
        std::string text = ReadTextFile(cand.c_str());
        if (text.empty()) continue;
        auto pr = json_min::Parse(text);
        if (!pr.error.empty()) continue;
        auto lr = LoadUISchemaFromJson(pr.value);
        if (!lr.error.empty()) continue;
        descSchema = std::move(lr.schema);
        descSchemaLoaded = true;
        break;
    }
    if (!descSchemaLoaded) {
        std::fprintf(stderr, "Failed to load UI schema for --describe-functions\n");
        return 1;
    }

    EngineFunctionCatalog catalog = BuildEngineCatalog(descSchema);
    std::string catalogJson = SerializeEngineCatalogJson(catalog);

    if (toStdout) {
        std::cout << catalogJson;
    }
    if (!jsonPath.empty()) {
        std::string writeError;
        if (!WriteTextFileExact(jsonPath, catalogJson, &writeError)) {
            std::fprintf(stderr, "%s\n", writeError.c_str());
            return 1;
        }
    }
    return 0;
}
