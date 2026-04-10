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

bool UsesGridRows(FractalProbeMode mode) {
    return mode == FractalProbeMode::grid;
}

bool EmitSampleModeResponse(const SampleModeArgs& args,
    const std::string& responseText,
    std::string* outError) {
    return EmitProbeResponse(responseText,
        args.response_stdout,
        args.response_json_path,
        outError);
}

std::string BuildNdjsonProbeResponse(const FractalProbeRequest& request,
    const FractalProbeResponse& response,
    const std::string& stateToken) {
    std::ostringstream out;
    const bool groupByRow = UsesGridRows(request.mode);
    size_t batchStart = 0;
    bool wroteLine = false;

    while (batchStart < response.samples.size()) {
        const int batchSequenceIndex = response.samples[batchStart].sequence_index;
        const int batchRowIndex = groupByRow ? response.samples[batchStart].grid_y : -1;
        size_t batchEnd = batchStart + 1;
        while (batchEnd < response.samples.size()) {
            const FractalProbeSample& sample = response.samples[batchEnd];
            if (sample.sequence_index != batchSequenceIndex) break;
            if (groupByRow && sample.grid_y != batchRowIndex) break;
            ++batchEnd;
        }

        if (wroteLine) out << "\n";
        out << SerializeFractalProbeNdjsonSampleBatchJson(
            response.request_id,
            response.function_id,
            batchSequenceIndex,
            batchRowIndex,
            std::vector<FractalProbeSample>(response.samples.begin() + batchStart,
                response.samples.begin() + batchEnd),
            response.metric_selection);
        wroteLine = true;
        batchStart = batchEnd;
    }

    if (wroteLine) out << "\n";
    out << SerializeFractalProbeNdjsonSummaryJson(response, stateToken);
    return out.str();
}

int RunSingleSampleRequest(const json_min::Value& requestValue,
    const SampleModeArgs& args,
    const std::string& exePath) {
    std::string error;
    FractalProbeRequest request;
    if (!ParseFractalProbeRequestFromValue(requestValue, &request, &error)) {
        FractalProbeResponse response = BuildProbeErrorResponse(
            std::string(), exePath, FractalProbeOperatorContext{}, error);
        std::string emitError;
        if (!EmitSampleModeResponse(args, SerializeFractalProbeResponseJson(response), &emitError)) {
            std::fprintf(stderr, "%s\n", emitError.c_str());
        }
        return 1;
    }

    FractalProbeResponse response;
    if (!RunFractalProbeRequest(request, exePath, &response, &error)) {
        response = BuildProbeErrorResponse(request.request_id, exePath, request.operator_context, error);
    }

    const std::string responseText =
        request.output_mode == FractalProbeOutputMode::ndjson && response.ok
        ? BuildNdjsonProbeResponse(request, response, std::string())
        : SerializeFractalProbeResponseJson(response);

    std::string emitError;
    if (!EmitSampleModeResponse(args, responseText, &emitError)) {
        std::fprintf(stderr, "%s\n", emitError.c_str());
        return 1;
    }
    return response.ok ? 0 : 1;
}

int RunBatchSampleRequests(const json_min::Array& requestArray,
    const SampleModeArgs& args,
    const std::string& exePath) {
    bool anyFailed = false;
    std::string batchJson = "[";
    for (size_t i = 0; i < requestArray.size(); ++i) {
        FractalProbeRequest request;
        std::string reqError;
        FractalProbeResponse response;
        if (!ParseFractalProbeRequestFromValue(requestArray[i], &request, &reqError)) {
            std::string failedRequestId;
            if (requestArray[i].is_object()) {
                auto idIt = requestArray[i].as_object().find("request_id");
                if (idIt != requestArray[i].as_object().end() && idIt->second.is_string()) {
                    failedRequestId = idIt->second.as_string();
                }
            }
            response = BuildProbeErrorResponse(failedRequestId, exePath, FractalProbeOperatorContext{}, reqError);
            anyFailed = true;
        } else if (request.output_mode == FractalProbeOutputMode::ndjson) {
            response = BuildProbeErrorResponse(
                request.request_id,
                exePath,
                request.operator_context,
                "output_mode ndjson is not allowed inside batch request arrays");
            anyFailed = true;
        } else {
            std::string runError;
            if (!RunFractalProbeRequest(request, exePath, &response, &runError)) {
                response = BuildProbeErrorResponse(request.request_id, exePath, request.operator_context, runError);
                anyFailed = true;
            }
        }
        if (i > 0) batchJson += ",";
        batchJson += SerializeFractalProbeResponseJson(response);
    }
    batchJson += "]";

    std::string emitError;
    if (!EmitSampleModeResponse(args, batchJson, &emitError)) {
        std::fprintf(stderr, "%s\n", emitError.c_str());
        return 1;
    }
    return anyFailed ? 1 : 0;
}

// --- Headless mode dispatch ---

int RunSampleMode(const SampleModeArgs& args, const std::string& exePath) {
    const bool haveRequestJson = !args.request_json_path.empty();
    const bool haveResponseJson = !args.response_json_path.empty();
    const int sourceCount = (args.request_stdin ? 1 : 0) + (haveRequestJson ? 1 : 0);

    std::string error;
    std::string requestText;

    // --- Validate args ---
    if (sourceCount != 1) {
        error = "sample mode requires exactly one request source";
    } else if (!args.response_stdout && !haveResponseJson) {
        error = "sample mode requires at least one response sink";
    } else if (args.conflict_validate_ui || args.conflict_capture_diagnostic || args.conflict_capture_finding) {
        error = "sample mode is mutually exclusive with --validate-ui, --capture-diagnostic, and --capture-finding";
    } else if (!(args.request_stdin
            ? ReadStdinText(&requestText, &error)
            : TryReadTextFileExact(args.request_json_path, &requestText, &error))) {
        // error already set by the read function
    } else {
        // --- Parse root JSON to detect object vs array ---
        json_min::ParseResult parsed = json_min::Parse(requestText);
        if (!parsed.error.empty()) {
            error = parsed.error;
        } else if (parsed.value.is_object()) {
            return RunSingleSampleRequest(parsed.value, args, exePath);
        } else if (parsed.value.is_array()) {
            return RunBatchSampleRequests(parsed.value.as_array(), args, exePath);
        } else {
            error = "Probe request root must be an object or array";
        }
    }

    // --- Error fallback (pre-parse or structural errors) ---
    FractalProbeResponse response = BuildProbeErrorResponse(
        std::string(), exePath, FractalProbeOperatorContext{}, error);
    const std::string responseJson = SerializeFractalProbeResponseJson(response);
    std::string emitError;
    if ((args.response_stdout || haveResponseJson) &&
        !EmitSampleModeResponse(args, responseJson, &emitError)) {
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

// --- Session mode (V2-B / V2-C) ---

// --- V2-C: Override merge ---

std::vector<FractalProbeOverride> MergeOverrides(
    const std::vector<FractalProbeOverride>& base,
    const std::vector<FractalProbeOverride>& diff) {
    if (diff.empty()) return base;
    if (base.empty()) return diff;

    // Copy base, then apply diff: replace matching paths, append new ones.
    std::vector<FractalProbeOverride> merged = base;
    for (const auto& d : diff) {
        bool replaced = false;
        for (auto& m : merged) {
            if (m.path == d.path) {
                m.value = d.value;
                replaced = true;
                break;
            }
        }
        if (!replaced) {
            merged.push_back(d);
        }
    }
    return merged;
}

static std::string MakeStateToken(int counter) {
    return "s" + std::to_string(counter);
}

// Escape a string for JSON embedding (minimal: quotes, backslash, control chars).
static std::string JsonEscapeString(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (static_cast<unsigned char>(c) < 0x20) {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", (unsigned char)c);
                    out += buf;
                } else {
                    out += c;
                }
        }
    }
    return out;
}

static std::string BuildSessionError(const std::string& error) {
    return "{\"ok\":false,\"error\":\"" + JsonEscapeString(error) + "\"}";
}

// Compact pretty-printed JSON to a single line (for session NDJSON output).
static std::string CompactJson(const std::string& json) {
    std::string result;
    result.reserve(json.size());
    bool inString = false;
    bool prevBackslash = false;
    for (size_t i = 0; i < json.size(); ++i) {
        char c = json[i];
        if (inString) {
            result += c;
            if (c == '\\' && !prevBackslash) { prevBackslash = true; continue; }
            if (c == '"' && !prevBackslash) inString = false;
            prevBackslash = false;
        } else {
            if (c == '"') { inString = true; result += c; }
            else if (c == '\n' || c == '\r' || c == ' ' || c == '\t') { /* skip whitespace outside strings */ }
            else { result += c; }
        }
    }
    return result;
}

std::string ProcessSessionLine(const std::string& line,
    bool* sessionOpen,
    bool* sessionDone,
    int* stateTokenCounter,
    SessionOverrideMap* accumulatedOverrides,
    const std::string& exePath) {

    // Skip empty/whitespace lines
    bool allWhitespace = true;
    for (char c : line) {
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n') { allWhitespace = false; break; }
    }
    if (line.empty() || allWhitespace) return {};

    // Parse JSON
    json_min::ParseResult parsed = json_min::Parse(line);
    if (!parsed.error.empty()) {
        if (!*sessionOpen) {
            *sessionDone = true;
            return BuildSessionError("session not open and line is not valid JSON: " + parsed.error);
        }
        return BuildSessionError("JSON parse error: " + parsed.error);
    }

    if (!parsed.value.is_object()) {
        if (!*sessionOpen) {
            *sessionDone = true;
            return BuildSessionError("session not open and line is not a JSON object");
        }
        return BuildSessionError("session line must be a JSON object");
    }

    const auto& obj = parsed.value.as_object();

    // Check for session control message
    auto sessIt = obj.find("session");
    if (sessIt != obj.end() && sessIt->second.is_string()) {
        const std::string& verb = sessIt->second.as_string();

        if (verb == "open") {
            if (*sessionOpen) {
                return BuildSessionError("session already open");
            }
            *sessionOpen = true;
            std::string token = MakeStateToken(*stateTokenCounter);
            return "{\"session\":\"ready\",\"state_token\":\"" + token + "\",\"engine_version\":2}";
        }

        if (verb == "close") {
            *sessionDone = true;
            return "{\"session\":\"closed\"}";
        }

        // Unknown session verb
        return BuildSessionError("unknown session verb: " + verb);
    }

    // Not a session control message — treat as sample request
    if (!*sessionOpen) {
        *sessionDone = true;
        return BuildSessionError("session not open");
    }

    // Parse and execute the probe request
    FractalProbeRequest request;
    std::string reqError;
    FractalProbeResponse response;

    if (!ParseFractalProbeRequestFromValue(parsed.value, &request, &reqError)) {
        std::string failedRequestId;
        auto idIt = obj.find("request_id");
        if (idIt != obj.end() && idIt->second.is_string()) {
            failedRequestId = idIt->second.as_string();
        }
        response = BuildProbeErrorResponse(failedRequestId, exePath, FractalProbeOperatorContext{}, reqError);
    } else {
        // V2-C: If state_token is present, merge accumulated overrides
        if (!request.state_token.empty() && accumulatedOverrides) {
            auto it = accumulatedOverrides->find(request.state_token);
            if (it == accumulatedOverrides->end()) {
                response = BuildProbeErrorResponse(request.request_id, exePath,
                    request.operator_context,
                    "unknown state_token: " + request.state_token);
                goto emit_response;
            }
            request.overrides = MergeOverrides(it->second, request.overrides);
        }

        std::string runError;
        if (!RunFractalProbeRequest(request, exePath, &response, &runError)) {
            response = BuildProbeErrorResponse(request.request_id, exePath, request.operator_context, runError);
        }
    }

emit_response:
    // Stamp response_version=2 and state_token
    response.response_version = 2;
    (*stateTokenCounter)++;
    std::string token = MakeStateToken(*stateTokenCounter);
    std::string responseJson;
    if (request.output_mode == FractalProbeOutputMode::ndjson && response.ok) {
        responseJson = BuildNdjsonProbeResponse(request, response, token);
    } else {
        responseJson = CompactJson(SerializeFractalProbeResponseJson(response));
        size_t lastBrace = responseJson.rfind('}');
        if (lastBrace != std::string::npos) {
            responseJson.insert(lastBrace, ",\"state_token\":\"" + token + "\"");
        }
    }

    // V2-C: Store accumulated overrides for this token
    if (accumulatedOverrides && response.ok) {
        (*accumulatedOverrides)[token] = request.overrides;
    }

    return responseJson;
}

int RunSessionMode(std::istream& in, std::ostream& out, const std::string& exePath) {
    bool sessionOpen = false;
    bool sessionDone = false;
    int stateTokenCounter = 0;
    SessionOverrideMap accumulatedOverrides;

    std::string line;
    while (std::getline(in, line)) {
        // Strip trailing \r from CRLF
        if (!line.empty() && line.back() == '\r') line.pop_back();

        std::string response = ProcessSessionLine(line, &sessionOpen, &sessionDone,
            &stateTokenCounter, &accumulatedOverrides, exePath);

        if (!response.empty()) {
            out << response << "\n";
            out.flush();
        }

        if (sessionDone) {
            // If the session was never opened, this is an error exit
            return sessionOpen ? 0 : 1;
        }
    }

    // EOF without close
    if (sessionOpen) {
        return 1;
    }
    return 1;
}
