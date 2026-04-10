#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>
#include <Windows.h>
#include "../src/headless_modes.h"
#include "../src/json_min.h"

static int g_passed = 0;
static int g_failed = 0;

static json_min::ParseResult ParseJsonLine(const std::string& line);
static std::vector<std::string> SplitLines(const std::string& text);

#define ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::fprintf(stderr, "%s:%d: FAIL: %s\n", __FILE__, __LINE__, (msg)); \
            g_failed++; \
            return false; \
        } \
        g_passed++; \
    } while (0)

static const char* kExePath = "D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe";

static std::string TempPath(const char* name) {
    const char* tmp = std::getenv("TEMP");
    if (!tmp) tmp = ".";
    return std::string(tmp) + "\\test_headless_" + name;
}

// --- File I/O round-trip ---

bool TestWriteAndReadTextFile() {
    std::string path = TempPath("round_trip.txt");
    std::string content = "hello\nworld\n";
    std::string err;
    ASSERT(WriteTextFileExact(path, content, &err), "write should succeed");
    ASSERT(err.empty(), "no error on write");
    std::string readBack = ReadTextFile(path.c_str());
    ASSERT(readBack == content, "read should match written content");
    DeleteFileA(path.c_str());
    return true;
}

bool TestTryReadTextFileExact() {
    std::string path = TempPath("exact_read.txt");
    std::string content = "exact content";
    std::string err;
    ASSERT(WriteTextFileExact(path, content, &err), "write should succeed");
    std::string readBack;
    ASSERT(TryReadTextFileExact(path, &readBack, &err), "read should succeed");
    ASSERT(readBack == content, "content should match");
    DeleteFileA(path.c_str());
    return true;
}

bool TestTryReadNonexistentFile() {
    std::string err;
    std::string text;
    ASSERT(!TryReadTextFileExact("nonexistent_file_12345.txt", &text, &err), "should fail for missing file");
    ASSERT(!err.empty(), "should have error message");
    return true;
}

bool TestWriteToInvalidPath() {
    std::string err;
    ASSERT(!WriteTextFileExact("Z:\\nonexistent\\dir\\file.txt", "data", &err), "should fail for invalid path");
    ASSERT(!err.empty(), "should have error message");
    return true;
}

// --- BuildProbeErrorResponse ---

bool TestBuildProbeErrorResponseFields() {
    FractalProbeOperatorContext ctx;
    ctx.source = "test-source";
    ctx.operator_name = "test-op";
    FractalProbeResponse resp = BuildProbeErrorResponse("req-123", kExePath, ctx, "something broke");
    ASSERT(resp.request_id == "req-123", "request_id should match");
    ASSERT(!resp.ok, "ok should be false");
    ASSERT(resp.runtime.exe_path == kExePath, "exe_path should match");
    ASSERT(resp.operator_context.source == "test-source", "operator_context.source should carry through");
    ASSERT(resp.operator_context.operator_name == "test-op", "operator_context.operator_name should carry through");
    ASSERT(resp.error == "something broke", "error should match");
    return true;
}

// --- EmitProbeResponse ---

bool TestEmitProbeResponseToFile() {
    std::string path = TempPath("emit_response.json");
    std::string json = "{\"ok\":true}";
    std::string err;
    ASSERT(EmitProbeResponse(json, false, path, &err), "emit to file should succeed");
    std::string readBack = ReadTextFile(path.c_str());
    ASSERT(readBack == json, "file content should match emitted JSON");
    DeleteFileA(path.c_str());
    return true;
}

bool TestEmitProbeResponseEmptyPath() {
    std::string err;
    ASSERT(EmitProbeResponse("{}", false, "", &err), "empty path + no stdout should succeed (no-op)");
    return true;
}

// --- RunSampleMode validation ---

bool TestSampleModeNoSource() {
    SampleModeArgs args;
    args.response_stdout = true;
    // No request source set
    int rc = RunSampleMode(args, kExePath);
    ASSERT(rc == 1, "no source should return error");
    return true;
}

bool TestSampleModeNoSink() {
    SampleModeArgs args;
    args.request_json_path = TempPath("nosink_req.json");
    // Write a minimal valid request file
    std::string reqJson = "{\"request_version\":1,\"request_id\":\"test\",\"mode\":\"point_set\","
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"newton\"}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3}]}";
    std::string err;
    WriteTextFileExact(args.request_json_path, reqJson, &err);
    // No response sink set
    int rc = RunSampleMode(args, kExePath);
    ASSERT(rc == 1, "no sink should return error");
    DeleteFileA(args.request_json_path.c_str());
    return true;
}

bool TestSampleModeDualSource() {
    SampleModeArgs args;
    args.request_stdin = true;
    args.request_json_path = "some_file.json";
    args.response_stdout = true;
    int rc = RunSampleMode(args, kExePath);
    ASSERT(rc == 1, "dual source should return error");
    return true;
}

bool TestSampleModeConflictWithValidateUi() {
    SampleModeArgs args;
    args.request_json_path = "req.json";
    args.response_stdout = true;
    args.conflict_validate_ui = true;
    int rc = RunSampleMode(args, kExePath);
    ASSERT(rc == 1, "conflict with validate-ui should return error");
    return true;
}

bool TestSampleModeBadRequestJson() {
    SampleModeArgs args;
    args.request_json_path = TempPath("bad_req.json");
    args.response_json_path = TempPath("bad_resp.json");
    std::string err;
    WriteTextFileExact(args.request_json_path, "not valid json {{{", &err);
    int rc = RunSampleMode(args, kExePath);
    ASSERT(rc == 1, "bad JSON should return error");
    // Response file should exist with error details
    std::string respText = ReadTextFile(args.response_json_path.c_str());
    ASSERT(!respText.empty(), "error response should be written");
    ASSERT(respText.find("\"ok\":false") != std::string::npos ||
           respText.find("\"ok\": false") != std::string::npos,
        "response should indicate failure");
    DeleteFileA(args.request_json_path.c_str());
    DeleteFileA(args.response_json_path.c_str());
    return true;
}

bool TestSampleModeValidFileRoundTrip() {
    SampleModeArgs args;
    args.request_json_path = TempPath("valid_req.json");
    args.response_json_path = TempPath("valid_resp.json");

    std::string reqJson = "{\"request_version\":1,\"request_id\":\"headless-test\",\"mode\":\"point_set\","
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"newton\"}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3}]}";
    std::string err;
    WriteTextFileExact(args.request_json_path, reqJson, &err);

    int rc = RunSampleMode(args, kExePath);
    ASSERT(rc == 0, "valid request should succeed");

    std::string respText = ReadTextFile(args.response_json_path.c_str());
    ASSERT(!respText.empty(), "response file should exist");
    ASSERT(respText.find("\"ok\":true") != std::string::npos ||
           respText.find("\"ok\": true") != std::string::npos,
        "response should indicate success");
    ASSERT(respText.find("\"request_id\":\"headless-test\"") != std::string::npos ||
           respText.find("\"request_id\": \"headless-test\"") != std::string::npos,
        "response should echo request_id");

    DeleteFileA(args.request_json_path.c_str());
    DeleteFileA(args.response_json_path.c_str());
    return true;
}

// --- V2-A: Batch request array ---

bool TestSampleModeBatchTwoValidRequests() {
    SampleModeArgs args;
    args.request_json_path = TempPath("batch_2_req.json");
    args.response_json_path = TempPath("batch_2_resp.json");

    std::string reqJson = "["
        "{\"request_version\":1,\"request_id\":\"batch-1\",\"mode\":\"point_set\","
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"newton\"}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3}]},"
        "{\"request_version\":1,\"request_id\":\"batch-2\",\"mode\":\"point_set\","
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"mandelbrot\"}],"
        "\"points\":[{\"x\":-0.5,\"y\":0.0}]}"
        "]";
    std::string err;
    WriteTextFileExact(args.request_json_path, reqJson, &err);

    int rc = RunSampleMode(args, kExePath);
    ASSERT(rc == 0, "batch of 2 valid requests should succeed");

    std::string respText = ReadTextFile(args.response_json_path.c_str());
    ASSERT(!respText.empty(), "response file should exist");
    // Response must be a JSON array
    ASSERT(respText.front() == '[', "batch response should be a JSON array");
    // Both request_ids should appear
    ASSERT(respText.find("\"batch-1\"") != std::string::npos, "should contain batch-1 response");
    ASSERT(respText.find("\"batch-2\"") != std::string::npos, "should contain batch-2 response");

    DeleteFileA(args.request_json_path.c_str());
    DeleteFileA(args.response_json_path.c_str());
    return true;
}

bool TestSampleModeBatchErrorIsolation() {
    SampleModeArgs args;
    args.request_json_path = TempPath("batch_err_req.json");
    args.response_json_path = TempPath("batch_err_resp.json");

    // First request is valid; second has an unknown field (should fail parse)
    std::string reqJson = "["
        "{\"request_version\":1,\"request_id\":\"ok-req\",\"mode\":\"point_set\","
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"newton\"}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3}]},"
        "{\"request_version\":1,\"request_id\":\"bad-req\",\"mode\":\"point_set\","
        "\"points\":[{\"x\":0.0,\"y\":0.0}],\"mystery_field\":123}"
        "]";
    std::string err;
    WriteTextFileExact(args.request_json_path, reqJson, &err);

    int rc = RunSampleMode(args, kExePath);
    // rc=1 because at least one request failed
    ASSERT(rc == 1, "batch with a bad request should return error exit code");

    std::string respText = ReadTextFile(args.response_json_path.c_str());
    ASSERT(!respText.empty(), "response file should exist");
    ASSERT(respText.front() == '[', "batch response should be a JSON array");
    // First request should succeed
    ASSERT(respText.find("\"ok-req\"") != std::string::npos, "should contain ok-req response");
    // Second request should fail but still appear
    ASSERT(respText.find("\"bad-req\"") != std::string::npos, "should contain bad-req error response");
    ASSERT(respText.find("mystery_field") != std::string::npos, "error should mention the unknown field");

    DeleteFileA(args.request_json_path.c_str());
    DeleteFileA(args.response_json_path.c_str());
    return true;
}

bool TestSampleModeBatchEmptyArray() {
    SampleModeArgs args;
    args.request_json_path = TempPath("batch_empty_req.json");
    args.response_json_path = TempPath("batch_empty_resp.json");

    std::string err;
    WriteTextFileExact(args.request_json_path, "[]", &err);

    int rc = RunSampleMode(args, kExePath);
    ASSERT(rc == 0, "empty batch should succeed");

    std::string respText = ReadTextFile(args.response_json_path.c_str());
    ASSERT(!respText.empty(), "response file should exist");
    ASSERT(respText == "[]", "empty batch should return empty JSON array");

    DeleteFileA(args.request_json_path.c_str());
    DeleteFileA(args.response_json_path.c_str());
    return true;
}

bool TestSampleModeRootNotObjectOrArray() {
    SampleModeArgs args;
    args.request_json_path = TempPath("batch_bad_root_req.json");
    args.response_json_path = TempPath("batch_bad_root_resp.json");

    std::string err;
    WriteTextFileExact(args.request_json_path, "\"just a string\"", &err);

    int rc = RunSampleMode(args, kExePath);
    ASSERT(rc == 1, "non-object non-array root should fail");

    DeleteFileA(args.request_json_path.c_str());
    DeleteFileA(args.response_json_path.c_str());
    return true;
}

bool TestSampleModeSingleObjectStillReturnsSingleObject() {
    // V1 backward compat: single object input -> single object output (not wrapped in array)
    SampleModeArgs args;
    args.request_json_path = TempPath("v1_compat_req.json");
    args.response_json_path = TempPath("v1_compat_resp.json");

    std::string reqJson = "{\"request_version\":1,\"request_id\":\"v1-compat\",\"mode\":\"point_set\","
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"newton\"}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3}]}";
    std::string err;
    WriteTextFileExact(args.request_json_path, reqJson, &err);

    int rc = RunSampleMode(args, kExePath);
    ASSERT(rc == 0, "single object request should succeed");

    std::string respText = ReadTextFile(args.response_json_path.c_str());
    ASSERT(!respText.empty(), "response file should exist");
    // Must be a JSON object, not array
    ASSERT(respText.front() == '{', "V1 single-object response must be an object, not array");

    DeleteFileA(args.request_json_path.c_str());
    DeleteFileA(args.response_json_path.c_str());
    return true;
}

bool TestSampleModeNdjsonGridResponse() {
    SampleModeArgs args;
    args.request_json_path = TempPath("ndjson_grid_req.json");
    args.response_json_path = TempPath("ndjson_grid_resp.ndjson");

    std::string reqJson = "{\"request_version\":1,\"request_id\":\"ndjson-grid\",\"mode\":\"grid\","
        "\"output_mode\":\"ndjson\"," 
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"mandelbrot\"}],"
        "\"region\":{\"center_x\":-0.75,\"center_y\":0.0,\"span_x\":0.5,\"span_y\":0.5,\"grid_width\":2,\"grid_height\":2},"
        "\"metrics\":[\"iterations\",\"status\",\"summary_mean_iterations\"]}";
    std::string err;
    WriteTextFileExact(args.request_json_path, reqJson, &err);

    int rc = RunSampleMode(args, kExePath);
    ASSERT(rc == 0, "ndjson grid request should succeed");

    std::string respText = ReadTextFile(args.response_json_path.c_str());
    auto lines = SplitLines(respText);
    ASSERT(lines.size() == 3, "2 grid rows + 1 summary line expected");

    auto batch0 = ParseJsonLine(lines[0]);
    auto batch1 = ParseJsonLine(lines[1]);
    auto summary = ParseJsonLine(lines[2]);
    ASSERT(batch0.error.empty() && batch0.value.is_object(), "batch line 0 should be valid JSON object");
    ASSERT(batch1.error.empty() && batch1.value.is_object(), "batch line 1 should be valid JSON object");
    ASSERT(summary.error.empty() && summary.value.is_object(), "summary line should be valid JSON object");

    ASSERT(batch0.value.as_object().find("type") != batch0.value.as_object().end() &&
            batch0.value.as_object().find("type")->second.as_string() == "sample_batch",
        "first line should be sample_batch");
    ASSERT(batch1.value.as_object().find("type") != batch1.value.as_object().end() &&
            batch1.value.as_object().find("type")->second.as_string() == "sample_batch",
        "second line should be sample_batch");
    ASSERT(summary.value.as_object().find("type") != summary.value.as_object().end() &&
            summary.value.as_object().find("type")->second.as_string() == "summary",
        "last line should be summary");
    ASSERT(summary.value.as_object().find("cost") != summary.value.as_object().end(),
        "summary line should include cost metadata");
    ASSERT(summary.value.as_object().find("summary") != summary.value.as_object().end(),
        "summary line should include summary object");

    DeleteFileA(args.request_json_path.c_str());
    DeleteFileA(args.response_json_path.c_str());
    return true;
}

bool TestSampleModeNdjsonPointSetSingleBatchAndMetricFiltering() {
    SampleModeArgs args;
    args.request_json_path = TempPath("ndjson_point_set_req.json");
    args.response_json_path = TempPath("ndjson_point_set_resp.ndjson");

    std::string reqJson = "{\"request_version\":1,\"request_id\":\"ndjson-point-set\",\"mode\":\"point_set\"," 
        "\"output_mode\":\"ndjson\"," 
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"newton\"}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3},{\"x\":-0.5,\"y\":0.0}],"
        "\"metrics\":[\"iterations\",\"status\",\"summary_mean_iterations\"]}";
    std::string err;
    WriteTextFileExact(args.request_json_path, reqJson, &err);

    int rc = RunSampleMode(args, kExePath);
    ASSERT(rc == 0, "ndjson point_set request should succeed");

    std::string respText = ReadTextFile(args.response_json_path.c_str());
    auto lines = SplitLines(respText);
    ASSERT(lines.size() == 2, "point_set ndjson should emit one sample batch plus summary");

    auto batch = ParseJsonLine(lines[0]);
    auto summary = ParseJsonLine(lines[1]);
    ASSERT(batch.error.empty() && batch.value.is_object(), "point_set batch should be valid JSON object");
    ASSERT(summary.error.empty() && summary.value.is_object(), "point_set summary should be valid JSON object");

    const auto& batchObj = batch.value.as_object();
    ASSERT(batchObj.find("type") != batchObj.end() && batchObj.find("type")->second.as_string() == "sample_batch",
        "point_set first line should be sample_batch");
    ASSERT(batchObj.find("request_id") != batchObj.end() && batchObj.find("request_id")->second.as_string() == "ndjson-point-set",
        "point_set batch should carry request_id");
    ASSERT(batchObj.find("function_id") != batchObj.end() && batchObj.find("function_id")->second.as_string() == "fractal.sample",
        "point_set batch should carry function_id");
    ASSERT(batchObj.find("row_index") == batchObj.end(), "point_set batch should not include row_index");
    ASSERT(batchObj.find("samples") != batchObj.end() && batchObj.find("samples")->second.is_array(),
        "point_set batch should include a samples array");
    ASSERT(batchObj.find("samples")->second.as_array().size() == 2, "point_set batch should contain both requested points");

    const auto& sampleObj = batchObj.find("samples")->second.as_array()[0].as_object();
    ASSERT(sampleObj.find("iterations") != sampleObj.end(), "iterations should be present when requested");
    ASSERT(sampleObj.find("status") != sampleObj.end(), "status should be present when requested");
    ASSERT(sampleObj.find("final_z_x") == sampleObj.end(), "final_z_x should be omitted when not requested");
    ASSERT(sampleObj.find("final_z_y") == sampleObj.end(), "final_z_y should be omitted when not requested");
    ASSERT(sampleObj.find("final_abs2") == sampleObj.end(), "final_abs2 should be omitted when not requested");
    ASSERT(sampleObj.find("residual") == sampleObj.end(), "residual should be omitted when not requested");
    ASSERT(sampleObj.find("root_index") == sampleObj.end(), "root_index should be omitted when not requested");
    ASSERT(summary.value.as_object().find("type") != summary.value.as_object().end() &&
            summary.value.as_object().find("type")->second.as_string() == "summary",
        "point_set last line should be summary");

    DeleteFileA(args.request_json_path.c_str());
    DeleteFileA(args.response_json_path.c_str());
    return true;
}

bool TestSampleModeNdjsonSummaryOnlyEmitsSummaryLine() {
    SampleModeArgs args;
    args.request_json_path = TempPath("ndjson_summary_only_req.json");
    args.response_json_path = TempPath("ndjson_summary_only_resp.ndjson");

    std::string reqJson = "{\"request_version\":1,\"request_id\":\"ndjson-summary-only\",\"mode\":\"point_set\"," 
        "\"output_mode\":\"ndjson\"," 
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"mandelbrot\"}],"
        "\"points\":[{\"x\":-0.75,\"y\":0.0},{\"x\":0.25,\"y\":0.0}],"
        "\"metrics\":[\"summary_mean_iterations\",\"summary_escape_fraction\"]}";
    std::string err;
    WriteTextFileExact(args.request_json_path, reqJson, &err);

    int rc = RunSampleMode(args, kExePath);
    ASSERT(rc == 0, "ndjson summary-only request should succeed");

    std::string respText = ReadTextFile(args.response_json_path.c_str());
    auto lines = SplitLines(respText);
    ASSERT(lines.size() == 1, "summary-only ndjson should emit only the summary line");

    auto summary = ParseJsonLine(lines[0]);
    ASSERT(summary.error.empty() && summary.value.is_object(), "summary-only line should be valid JSON object");
    ASSERT(summary.value.as_object().find("type") != summary.value.as_object().end() &&
            summary.value.as_object().find("type")->second.as_string() == "summary",
        "summary-only ndjson should emit a summary object");
    ASSERT(summary.value.as_object().find("summary") != summary.value.as_object().end(),
        "summary-only ndjson should still carry summary metrics");
    ASSERT(summary.value.as_object().find("cost") != summary.value.as_object().end(),
        "summary-only ndjson should still carry cost metadata");

    DeleteFileA(args.request_json_path.c_str());
    DeleteFileA(args.response_json_path.c_str());
    return true;
}

bool TestSampleModeGenericSampleNdjsonIncludesFunctionId() {
    SampleModeArgs args;
    args.request_json_path = TempPath("ndjson_generic_req.json");
    args.response_json_path = TempPath("ndjson_generic_resp.ndjson");

    std::string reqJson = "{\"request_version\":1,\"request_id\":\"ndjson-generic\",\"function_id\":\"generic.sample\",\"mode\":\"point_set\"," 
        "\"output_mode\":\"ndjson\"," 
        "\"function\":{\"expression\":\"z^2 + z + 1\"},"
        "\"points\":[{\"x\":1.0,\"y\":0.0}],"
        "\"metrics\":[\"value\",\"abs2\"]}";
    std::string err;
    WriteTextFileExact(args.request_json_path, reqJson, &err);

    int rc = RunSampleMode(args, kExePath);
    ASSERT(rc == 0, "generic.sample ndjson request should succeed");

    std::string respText = ReadTextFile(args.response_json_path.c_str());
    auto lines = SplitLines(respText);
    ASSERT(lines.size() == 2, "generic.sample ndjson should emit one batch plus summary");

    auto batch = ParseJsonLine(lines[0]);
    auto summary = ParseJsonLine(lines[1]);
    ASSERT(batch.error.empty() && batch.value.is_object(), "generic.sample batch should be valid JSON object");
    ASSERT(summary.error.empty() && summary.value.is_object(), "generic.sample summary should be valid JSON object");
    ASSERT(batch.value.as_object().find("function_id") != batch.value.as_object().end() &&
            batch.value.as_object().find("function_id")->second.as_string() == "generic.sample",
        "generic.sample batch should advertise function_id");
    ASSERT(summary.value.as_object().find("function_id") != summary.value.as_object().end() &&
            summary.value.as_object().find("function_id")->second.as_string() == "generic.sample",
        "generic.sample summary should advertise function_id");

    DeleteFileA(args.request_json_path.c_str());
    DeleteFileA(args.response_json_path.c_str());
    return true;
}

bool TestSampleModeNdjsonSequenceGridBatchesPerSequenceStep() {
    SampleModeArgs args;
    args.request_json_path = TempPath("ndjson_sequence_grid_req.json");
    args.response_json_path = TempPath("ndjson_sequence_grid_resp.ndjson");

    std::string reqJson = "{\"request_version\":1,\"request_id\":\"ndjson-sequence-grid\",\"mode\":\"sequence_grid\"," 
        "\"output_mode\":\"ndjson\"," 
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"mandelbrot\"}],"
        "\"region\":{\"center_x\":-0.75,\"center_y\":0.0,\"span_x\":0.5,\"span_y\":0.5,\"grid_width\":2,\"grid_height\":2},"
        "\"sequence\":{\"zip_paths\":true,\"vary\":[{\"path\":\"fractal.view.zoom\",\"values\":[1.0,2.0]}]},"
        "\"metrics\":[\"iterations\",\"status\",\"summary_mean_iterations\"]}";
    std::string err;
    WriteTextFileExact(args.request_json_path, reqJson, &err);

    int rc = RunSampleMode(args, kExePath);
    ASSERT(rc == 0, "ndjson sequence_grid request should succeed");

    std::string respText = ReadTextFile(args.response_json_path.c_str());
    auto lines = SplitLines(respText);
    ASSERT(lines.size() == 3, "2 sequence steps + 1 summary line expected");

    auto batch0 = ParseJsonLine(lines[0]);
    auto batch1 = ParseJsonLine(lines[1]);
    auto summary = ParseJsonLine(lines[2]);
    ASSERT(batch0.error.empty() && batch0.value.is_object(), "sequence batch 0 should be valid JSON object");
    ASSERT(batch1.error.empty() && batch1.value.is_object(), "sequence batch 1 should be valid JSON object");
    ASSERT(summary.error.empty() && summary.value.is_object(), "summary line should be valid JSON object");

    ASSERT(batch0.value.as_object().find("type") != batch0.value.as_object().end() &&
            batch0.value.as_object().find("type")->second.as_string() == "sample_batch",
        "first sequence line should be sample_batch");
    ASSERT(batch1.value.as_object().find("type") != batch1.value.as_object().end() &&
            batch1.value.as_object().find("type")->second.as_string() == "sample_batch",
        "second sequence line should be sample_batch");
    ASSERT(batch0.value.as_object().find("row_index") == batch0.value.as_object().end(),
        "sequence-grid batches should not be split by row_index");
    ASSERT(batch1.value.as_object().find("row_index") == batch1.value.as_object().end(),
        "sequence-grid batches should not be split by row_index");
    ASSERT(batch0.value.as_object().find("sequence_index") != batch0.value.as_object().end() &&
            batch0.value.as_object().find("sequence_index")->second.as_number() == 0.0,
        "first sequence batch should report sequence_index 0");
    ASSERT(batch1.value.as_object().find("sequence_index") != batch1.value.as_object().end() &&
            batch1.value.as_object().find("sequence_index")->second.as_number() == 1.0,
        "second sequence batch should report sequence_index 1");
    ASSERT(batch0.value.as_object().find("samples") != batch0.value.as_object().end() &&
            batch0.value.as_object().find("samples")->second.is_array() &&
            batch0.value.as_object().find("samples")->second.as_array().size() == 4,
        "each sequence-grid batch should contain the full 2x2 grid");
    ASSERT(batch1.value.as_object().find("samples") != batch1.value.as_object().end() &&
            batch1.value.as_object().find("samples")->second.is_array() &&
            batch1.value.as_object().find("samples")->second.as_array().size() == 4,
        "each sequence-grid batch should contain the full 2x2 grid");
    ASSERT(summary.value.as_object().find("type") != summary.value.as_object().end() &&
            summary.value.as_object().find("type")->second.as_string() == "summary",
        "last line should be summary");

    DeleteFileA(args.request_json_path.c_str());
    DeleteFileA(args.response_json_path.c_str());
    return true;
}

bool TestSampleModeNdjsonSequencePointSetBatchesPerSequenceStep() {
    SampleModeArgs args;
    args.request_json_path = TempPath("ndjson_sequence_point_set_req.json");
    args.response_json_path = TempPath("ndjson_sequence_point_set_resp.ndjson");

    std::string reqJson = "{\"request_version\":1,\"request_id\":\"ndjson-sequence-point-set\",\"mode\":\"sequence_point_set\"," 
        "\"output_mode\":\"ndjson\"," 
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"newton\"}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3},{\"x\":-0.5,\"y\":0.0}],"
        "\"sequence\":{\"zip_paths\":true,\"vary\":[{\"path\":\"fractal.view.zoom\",\"values\":[1.0,2.0]}]},"
        "\"metrics\":[\"iterations\",\"status\",\"summary_mean_iterations\"]}";
    std::string err;
    WriteTextFileExact(args.request_json_path, reqJson, &err);

    int rc = RunSampleMode(args, kExePath);
    ASSERT(rc == 0, "ndjson sequence_point_set request should succeed");

    std::string respText = ReadTextFile(args.response_json_path.c_str());
    auto lines = SplitLines(respText);
    ASSERT(lines.size() == 3, "sequence_point_set should emit one batch per sequence step plus summary");

    auto batch0 = ParseJsonLine(lines[0]);
    auto batch1 = ParseJsonLine(lines[1]);
    ASSERT(batch0.error.empty() && batch0.value.is_object(), "sequence point batch 0 should be valid JSON");
    ASSERT(batch1.error.empty() && batch1.value.is_object(), "sequence point batch 1 should be valid JSON");
    ASSERT(batch0.value.as_object().find("row_index") == batch0.value.as_object().end(),
        "sequence_point_set batches should not include row_index");
    ASSERT(batch1.value.as_object().find("row_index") == batch1.value.as_object().end(),
        "sequence_point_set batches should not include row_index");
    ASSERT(batch0.value.as_object().find("sequence_index") != batch0.value.as_object().end() &&
            batch0.value.as_object().find("sequence_index")->second.as_number() == 0.0,
        "first sequence_point_set batch should report sequence_index 0");
    ASSERT(batch1.value.as_object().find("sequence_index") != batch1.value.as_object().end() &&
            batch1.value.as_object().find("sequence_index")->second.as_number() == 1.0,
        "second sequence_point_set batch should report sequence_index 1");
    ASSERT(batch0.value.as_object().find("samples") != batch0.value.as_object().end() &&
            batch0.value.as_object().find("samples")->second.is_array() &&
            batch0.value.as_object().find("samples")->second.as_array().size() == 2,
        "each sequence_point_set batch should contain all requested points");
    ASSERT(batch1.value.as_object().find("samples") != batch1.value.as_object().end() &&
            batch1.value.as_object().find("samples")->second.is_array() &&
            batch1.value.as_object().find("samples")->second.as_array().size() == 2,
        "each sequence_point_set batch should contain all requested points");

    DeleteFileA(args.request_json_path.c_str());
    DeleteFileA(args.response_json_path.c_str());
    return true;
}

bool TestSampleModeBatchRejectsNdjsonOutputMode() {
    SampleModeArgs args;
    args.request_json_path = TempPath("ndjson_batch_req.json");
    args.response_json_path = TempPath("ndjson_batch_resp.json");

    std::string reqJson = "["
        "{\"request_version\":1,\"request_id\":\"ndjson-bad\",\"mode\":\"point_set\",\"output_mode\":\"ndjson\","
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"newton\"}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3}]}"
        "]";
    std::string err;
    WriteTextFileExact(args.request_json_path, reqJson, &err);

    int rc = RunSampleMode(args, kExePath);
    ASSERT(rc == 1, "batch array containing ndjson request should fail");

    std::string respText = ReadTextFile(args.response_json_path.c_str());
    auto parsed = json_min::Parse(respText);
    ASSERT(parsed.error.empty() && parsed.value.is_array(), "batch response should still be a JSON array");
    ASSERT(parsed.value.as_array().size() == 1, "batch response should contain one error entry");
    const auto& obj = parsed.value.as_array()[0].as_object();
    ASSERT(obj.find("ok") != obj.end() && !obj.find("ok")->second.as_bool(), "ndjson-in-batch should return ok=false");
    ASSERT(obj.find("error") != obj.end() && obj.find("error")->second.as_string().find("ndjson") != std::string::npos,
        "error should mention ndjson batch incompatibility");

    DeleteFileA(args.request_json_path.c_str());
    DeleteFileA(args.response_json_path.c_str());
    return true;
}

// --- V2-B: Session mode ---

// Helper: parse JSON line and return the parsed object.
static json_min::ParseResult ParseJsonLine(const std::string& line) {
    return json_min::Parse(line);
}

// Helper: run a full session exchange via RunSessionMode with stringstreams.
static int RunSessionWithStrings(const std::string& input, std::string* output) {
    std::istringstream in(input);
    std::ostringstream out;
    int rc = RunSessionMode(in, out, kExePath);
    *output = out.str();
    return rc;
}

// Helper: split output into lines (skipping empty trailing line from final newline).
static std::vector<std::string> SplitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty()) lines.push_back(line);
    }
    return lines;
}

bool TestSessionOpenReady() {
    // Client sends open, gets ready response.
    std::string input = "{\"session\":\"open\",\"request_id\":\"init\"}\n"
                        "{\"session\":\"close\"}\n";
    std::string output;
    int rc = RunSessionWithStrings(input, &output);
    ASSERT(rc == 0, "clean open+close should return 0");

    auto lines = SplitLines(output);
    ASSERT(lines.size() == 2, "should have ready + close-ack lines");

    auto p0 = ParseJsonLine(lines[0]);
    ASSERT(p0.error.empty(), "ready response should be valid JSON");
    ASSERT(p0.value.is_object(), "ready response should be an object");
    auto& obj = p0.value.as_object();
    auto sessIt = obj.find("session");
    ASSERT(sessIt != obj.end() && sessIt->second.is_string() && sessIt->second.as_string() == "ready",
        "session field should be 'ready'");
    auto tokIt = obj.find("state_token");
    ASSERT(tokIt != obj.end() && tokIt->second.is_string(),
        "ready response must include state_token");
    auto verIt = obj.find("engine_version");
    ASSERT(verIt != obj.end() && verIt->second.is_number(),
        "ready response must include engine_version");
    ASSERT(verIt->second.as_number() == 2.0, "engine_version should be 2");
    return true;
}

bool TestSessionCloseAck() {
    // After close, engine should emit an ack line.
    std::string input = "{\"session\":\"open\",\"request_id\":\"init\"}\n"
                        "{\"session\":\"close\"}\n";
    std::string output;
    int rc = RunSessionWithStrings(input, &output);
    ASSERT(rc == 0, "clean session should return 0");

    auto lines = SplitLines(output);
    ASSERT(lines.size() >= 2, "need at least ready + close-ack");
    auto pLast = ParseJsonLine(lines.back());
    ASSERT(pLast.error.empty(), "close-ack should be valid JSON");
    auto& obj = pLast.value.as_object();
    auto sessIt = obj.find("session");
    ASSERT(sessIt != obj.end() && sessIt->second.is_string() && sessIt->second.as_string() == "closed",
        "close ack session field should be 'closed'");
    return true;
}

bool TestSessionSingleRequest() {
    // Open, one sample request, close.
    std::string reqLine = "{\"request_version\":1,\"request_id\":\"r1\",\"mode\":\"point_set\","
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"newton\"}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3}]}";

    std::string input = "{\"session\":\"open\",\"request_id\":\"init\"}\n"
                        + reqLine + "\n"
                        + "{\"session\":\"close\"}\n";
    std::string output;
    int rc = RunSessionWithStrings(input, &output);
    ASSERT(rc == 0, "session with valid request should return 0");

    auto lines = SplitLines(output);
    ASSERT(lines.size() == 3, "should have ready + response + close-ack");

    // Parse the sample response (line 1)
    auto p1 = ParseJsonLine(lines[1]);
    ASSERT(p1.error.empty(), "sample response should be valid JSON");
    ASSERT(p1.value.is_object(), "sample response should be an object");
    auto& resp = p1.value.as_object();
    auto okIt = resp.find("ok");
    ASSERT(okIt != resp.end() && okIt->second.is_bool() && okIt->second.as_bool(),
        "sample response ok should be true");
    auto idIt = resp.find("request_id");
    ASSERT(idIt != resp.end() && idIt->second.is_string() && idIt->second.as_string() == "r1",
        "request_id should echo back r1");
    auto tokIt = resp.find("state_token");
    ASSERT(tokIt != resp.end() && tokIt->second.is_string(),
        "sample response must include state_token");
    auto costIt = resp.find("cost");
    ASSERT(costIt != resp.end() && costIt->second.is_object(),
        "sample response must include cost metadata");
    auto costSampleCountIt = costIt->second.as_object().find("sample_count");
    ASSERT(costSampleCountIt != costIt->second.as_object().end() &&
            costSampleCountIt->second.is_number() &&
            costSampleCountIt->second.as_number() == 1.0,
        "session sample response cost.sample_count should be 1");
    auto gpuMsIt = costIt->second.as_object().find("gpu_ms");
    ASSERT(gpuMsIt != costIt->second.as_object().end() &&
            gpuMsIt->second.is_number() &&
            gpuMsIt->second.as_number() >= 0.0,
        "session sample response cost.gpu_ms should be a non-negative number");
    return true;
}

bool TestSessionStateTokenIncrements() {
    // Two requests — state_token should change.
    std::string req = "{\"request_version\":1,\"request_id\":\"rX\",\"mode\":\"point_set\","
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"newton\"}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3}]}";

    std::string input = "{\"session\":\"open\",\"request_id\":\"init\"}\n"
                        + req + "\n" + req + "\n"
                        + "{\"session\":\"close\"}\n";
    std::string output;
    int rc = RunSessionWithStrings(input, &output);
    ASSERT(rc == 0, "two-request session should return 0");

    auto lines = SplitLines(output);
    ASSERT(lines.size() == 4, "ready + 2 responses + close-ack");

    // Extract state_tokens from ready, resp1, resp2
    auto getToken = [](const std::string& line) -> std::string {
        auto p = json_min::Parse(line);
        if (!p.error.empty() || !p.value.is_object()) return "";
        auto it = p.value.as_object().find("state_token");
        if (it == p.value.as_object().end() || !it->second.is_string()) return "";
        return it->second.as_string();
    };

    std::string t0 = getToken(lines[0]);
    std::string t1 = getToken(lines[1]);
    std::string t2 = getToken(lines[2]);
    ASSERT(!t0.empty() && !t1.empty() && !t2.empty(), "all responses must have state_token");
    ASSERT(t0 != t1, "token should change after first request");
    ASSERT(t1 != t2, "token should change after second request");
    return true;
}

bool TestSessionRequestWithoutOpen() {
    // Sending a sample request without opening first should error.
    std::string input = "{\"request_version\":1,\"request_id\":\"r1\",\"mode\":\"point_set\","
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"newton\"}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3}]}\n";
    std::string output;
    int rc = RunSessionWithStrings(input, &output);
    ASSERT(rc == 1, "request without open should return error");

    auto lines = SplitLines(output);
    ASSERT(lines.size() >= 1, "should have at least one error line");
    auto p = ParseJsonLine(lines[0]);
    ASSERT(p.error.empty() && p.value.is_object(), "error line should be valid JSON object");
    auto& obj = p.value.as_object();
    auto okIt = obj.find("ok");
    ASSERT(okIt != obj.end() && okIt->second.is_bool() && !okIt->second.as_bool(),
        "ok should be false for session-not-open error");
    auto errIt = obj.find("error");
    ASSERT(errIt != obj.end() && errIt->second.is_string() && !errIt->second.as_string().empty(),
        "should have error message");
    return true;
}

bool TestSessionMalformedJsonLine() {
    // Malformed JSON in session should produce error response, not crash.
    std::string input = "{\"session\":\"open\",\"request_id\":\"init\"}\n"
                        "not valid json {{{{\n"
                        "{\"session\":\"close\"}\n";
    std::string output;
    int rc = RunSessionWithStrings(input, &output);
    ASSERT(rc == 0, "malformed request followed by close should still close cleanly");

    auto lines = SplitLines(output);
    ASSERT(lines.size() == 3, "ready + error_response + close-ack");

    // The error response (line 1)
    auto p1 = ParseJsonLine(lines[1]);
    ASSERT(p1.error.empty() && p1.value.is_object(), "error response should be valid JSON");
    auto& obj = p1.value.as_object();
    auto okIt = obj.find("ok");
    ASSERT(okIt != obj.end() && okIt->second.is_bool() && !okIt->second.as_bool(),
        "ok should be false for parse error");
    return true;
}

bool TestSessionEmptyLines() {
    // Empty and whitespace-only lines should be silently skipped.
    std::string input = "{\"session\":\"open\",\"request_id\":\"init\"}\n"
                        "\n"
                        "   \n"
                        "{\"session\":\"close\"}\n";
    std::string output;
    int rc = RunSessionWithStrings(input, &output);
    ASSERT(rc == 0, "empty lines should be skipped cleanly");

    auto lines = SplitLines(output);
    ASSERT(lines.size() == 2, "ready + close-ack only (no output for blank lines)");
    return true;
}

bool TestSessionEofWithoutClose() {
    // If stdin hits EOF without close, treat as unclean exit.
    std::string input = "{\"session\":\"open\",\"request_id\":\"init\"}\n";
    std::string output;
    int rc = RunSessionWithStrings(input, &output);
    ASSERT(rc == 1, "EOF without close should return error");
    return true;
}

bool TestSessionDoubleOpen() {
    // Sending open twice should error on the second open.
    std::string input = "{\"session\":\"open\",\"request_id\":\"init\"}\n"
                        "{\"session\":\"open\",\"request_id\":\"init2\"}\n"
                        "{\"session\":\"close\"}\n";
    std::string output;
    int rc = RunSessionWithStrings(input, &output);
    // Session should still close cleanly but the second open produced an error response
    ASSERT(rc == 0, "double open should still close cleanly");

    auto lines = SplitLines(output);
    ASSERT(lines.size() == 3, "ready + error_from_double_open + close-ack");

    auto p1 = ParseJsonLine(lines[1]);
    ASSERT(p1.error.empty() && p1.value.is_object(), "double-open error should be valid JSON");
    auto okIt = p1.value.as_object().find("ok");
    ASSERT(okIt != p1.value.as_object().end() && okIt->second.is_bool() && !okIt->second.as_bool(),
        "ok should be false for double-open");
    return true;
}

bool TestSessionBadRequestPreservesSession() {
    // A bad request should produce an error response but the session stays open for next request.
    std::string goodReq = "{\"request_version\":1,\"request_id\":\"good\",\"mode\":\"point_set\","
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"newton\"}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3}]}";
    // Bad: missing required fields
    std::string badReq = "{\"request_version\":1,\"request_id\":\"bad\",\"mode\":\"point_set\"}";

    std::string input = "{\"session\":\"open\",\"request_id\":\"init\"}\n"
                        + badReq + "\n"
                        + goodReq + "\n"
                        + "{\"session\":\"close\"}\n";
    std::string output;
    int rc = RunSessionWithStrings(input, &output);
    ASSERT(rc == 0, "bad request shouldn't kill session");

    auto lines = SplitLines(output);
    ASSERT(lines.size() == 4, "ready + bad_error + good_response + close-ack");

    // bad request should fail
    auto p1 = ParseJsonLine(lines[1]);
    ASSERT(p1.value.is_object(), "bad response should be JSON object");
    auto ok1 = p1.value.as_object().find("ok");
    ASSERT(ok1 != p1.value.as_object().end() && !ok1->second.as_bool(), "bad request ok=false");

    // good request should succeed
    auto p2 = ParseJsonLine(lines[2]);
    ASSERT(p2.value.is_object(), "good response should be JSON object");
    auto ok2 = p2.value.as_object().find("ok");
    ASSERT(ok2 != p2.value.as_object().end() && ok2->second.as_bool(), "good request ok=true");
    return true;
}

bool TestSessionBadRequestDoesNotMintStateToken() {
    std::string goodReq = "{\"request_version\":1,\"request_id\":\"good\",\"mode\":\"point_set\"," 
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"newton\"}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3}]}";
    std::string badReq = "{\"request_version\":1,\"request_id\":\"bad\",\"mode\":\"point_set\"}";

    std::string input = "{\"session\":\"open\",\"request_id\":\"init\"}\n"
                        + badReq + "\n"
                        + goodReq + "\n"
                        + "{\"session\":\"close\"}\n";
    std::string output;
    int rc = RunSessionWithStrings(input, &output);
    ASSERT(rc == 0, "bad request should not kill session");

    auto lines = SplitLines(output);
    ASSERT(lines.size() == 4, "ready + bad_error + good_response + close expected");

    auto bad = ParseJsonLine(lines[1]);
    auto good = ParseJsonLine(lines[2]);
    ASSERT(bad.error.empty() && bad.value.is_object(), "bad response should be valid JSON");
    ASSERT(good.error.empty() && good.value.is_object(), "good response should be valid JSON");
    ASSERT(bad.value.as_object().find("ok") != bad.value.as_object().end() &&
            !bad.value.as_object().find("ok")->second.as_bool(),
        "bad response should be ok=false");
    ASSERT(bad.value.as_object().find("state_token") == bad.value.as_object().end(),
        "bad response should not mint a state_token");
    ASSERT(good.value.as_object().find("state_token") != good.value.as_object().end() &&
            good.value.as_object().find("state_token")->second.as_string() == "s1",
        "first successful response after an error should still mint s1");
    return true;
}

bool TestSessionUnknownSessionVerb() {
    // Unknown session verb (not open/close) with session field should error.
    std::string input = "{\"session\":\"open\",\"request_id\":\"init\"}\n"
                        "{\"session\":\"reset\"}\n"
                        "{\"session\":\"close\"}\n";
    std::string output;
    int rc = RunSessionWithStrings(input, &output);
    ASSERT(rc == 0, "unknown session verb shouldn't crash");

    auto lines = SplitLines(output);
    ASSERT(lines.size() == 3, "ready + error + close-ack");

    auto p1 = ParseJsonLine(lines[1]);
    ASSERT(p1.value.is_object(), "error response should be JSON object");
    auto okIt = p1.value.as_object().find("ok");
    ASSERT(okIt != p1.value.as_object().end() && !okIt->second.as_bool(),
        "unknown session verb should produce ok=false");
    return true;
}

bool TestProcessSessionLineOpen() {
    bool sessionOpen = false;
    bool sessionDone = false;
    int counter = 0;
    SessionOverrideMap accumulated;
    std::string result = ProcessSessionLine(
        "{\"session\":\"open\",\"request_id\":\"init\"}",
        &sessionOpen, &sessionDone, &counter, &accumulated, kExePath);
    ASSERT(!result.empty(), "open should produce a response");
    ASSERT(sessionOpen, "sessionOpen should be set true");
    ASSERT(!sessionDone, "sessionDone should be false after open");

    auto p = json_min::Parse(result);
    ASSERT(p.error.empty() && p.value.is_object(), "open response should be valid JSON");
    auto& obj = p.value.as_object();
    {
        auto it = obj.find("session");
        ASSERT(it != obj.end() && it->second.as_string() == "ready",
            "should be session=ready");
    }
    ASSERT(obj.find("state_token") != obj.end(), "must have state_token");
    return true;
}

bool TestProcessSessionLineClose() {
    bool sessionOpen = true;
    bool sessionDone = false;
    int counter = 1;
    SessionOverrideMap accumulated;
    std::string result = ProcessSessionLine(
        "{\"session\":\"close\"}",
        &sessionOpen, &sessionDone, &counter, &accumulated, kExePath);
    ASSERT(!result.empty(), "close should produce a response");
    ASSERT(sessionDone, "sessionDone should be true after close");

    auto p = json_min::Parse(result);
    ASSERT(p.error.empty() && p.value.is_object(), "close response should be valid JSON");
    auto& obj = p.value.as_object();
    {
        auto it = obj.find("session");
        ASSERT(it != obj.end() && it->second.as_string() == "closed",
            "should be session=closed");
    }
    return true;
}

bool TestProcessSessionLineEmptyReturnsEmpty() {
    bool sessionOpen = true;
    bool sessionDone = false;
    int counter = 0;
    SessionOverrideMap accumulated;
    std::string result = ProcessSessionLine("", &sessionOpen, &sessionDone, &counter, &accumulated, kExePath);
    ASSERT(result.empty(), "empty line should return empty string");
    ASSERT(!sessionDone, "empty line should not end session");
    result = ProcessSessionLine("   ", &sessionOpen, &sessionDone, &counter, &accumulated, kExePath);
    ASSERT(result.empty(), "whitespace-only line should return empty string");
    return true;
}

bool TestProcessSessionLineRequestNotOpen() {
    bool sessionOpen = false;
    bool sessionDone = false;
    int counter = 0;
    SessionOverrideMap accumulated;
    std::string result = ProcessSessionLine(
        "{\"request_version\":1,\"request_id\":\"r1\",\"mode\":\"point_set\","
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"newton\"}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3}]}",
        &sessionOpen, &sessionDone, &counter, &accumulated, kExePath);
    ASSERT(!result.empty(), "should produce error response");
    ASSERT(sessionDone, "session should end on not-open request");

    auto p = json_min::Parse(result);
    ASSERT(p.error.empty() && p.value.is_object(), "error should be valid JSON");
    auto okIt = p.value.as_object().find("ok");
    ASSERT(okIt != p.value.as_object().end() && !okIt->second.as_bool(), "ok should be false");
    return true;
}

bool TestProcessSessionLineValidRequest() {
    bool sessionOpen = true;
    bool sessionDone = false;
    int counter = 1;
    SessionOverrideMap accumulated;
    std::string result = ProcessSessionLine(
        "{\"request_version\":1,\"request_id\":\"r1\",\"mode\":\"point_set\","
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"newton\"}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3}]}",
        &sessionOpen, &sessionDone, &counter, &accumulated, kExePath);
    ASSERT(!result.empty(), "valid request should produce a response");
    ASSERT(!sessionDone, "session should remain open");
    ASSERT(counter == 2, "state token counter should increment");

    auto p = json_min::Parse(result);
    ASSERT(p.error.empty() && p.value.is_object(), "response should be valid JSON");
    auto& obj = p.value.as_object();
    auto okIt = obj.find("ok");
    ASSERT(okIt != obj.end() && okIt->second.as_bool(), "ok should be true");
    auto tokIt = obj.find("state_token");
    ASSERT(tokIt != obj.end() && tokIt->second.is_string(), "must have state_token");
    ASSERT(tokIt->second.as_string() == "s2", "token should be s2 (counter was 1, now 2)");
    return true;
}

bool TestProcessSessionLineMalformedJson() {
    bool sessionOpen = true;
    bool sessionDone = false;
    int counter = 1;
    SessionOverrideMap accumulated;
    std::string result = ProcessSessionLine(
        "not valid json {{{{",
        &sessionOpen, &sessionDone, &counter, &accumulated, kExePath);
    ASSERT(!result.empty(), "malformed JSON should produce error response");
    ASSERT(!sessionDone, "session should stay open after parse error");

    auto p = json_min::Parse(result);
    ASSERT(p.error.empty() && p.value.is_object(), "error response should be valid JSON");
    auto okIt = p.value.as_object().find("ok");
    ASSERT(okIt != p.value.as_object().end() && !okIt->second.as_bool(), "ok should be false");
    return true;
}

bool TestSessionResponseVersionField() {
    // All sample responses in session mode should have response_version: 2.
    std::string req = "{\"request_version\":1,\"request_id\":\"r1\",\"mode\":\"point_set\","
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"newton\"}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3}]}";

    std::string input = "{\"session\":\"open\",\"request_id\":\"init\"}\n"
                        + req + "\n"
                        + "{\"session\":\"close\"}\n";
    std::string output;
    int rc = RunSessionWithStrings(input, &output);
    ASSERT(rc == 0, "session should succeed");

    auto lines = SplitLines(output);
    ASSERT(lines.size() == 3, "ready + response + close-ack");

    auto p = ParseJsonLine(lines[1]);
    ASSERT(p.value.is_object(), "response should be JSON object");
    auto verIt = p.value.as_object().find("response_version");
    ASSERT(verIt != p.value.as_object().end() && verIt->second.is_number(),
        "response should have response_version");
    ASSERT(verIt->second.as_number() == 2.0, "response_version should be 2");
    return true;
}

bool TestSessionNdjsonGridRequest() {
    std::string req = "{\"request_version\":1,\"request_id\":\"ndjson-session\",\"mode\":\"grid\","
        "\"output_mode\":\"ndjson\"," 
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"mandelbrot\"}],"
        "\"region\":{\"center_x\":-0.75,\"center_y\":0.0,\"span_x\":0.5,\"span_y\":0.5,\"grid_width\":2,\"grid_height\":2},"
        "\"metrics\":[\"iterations\",\"status\",\"summary_mean_iterations\"]}";

    std::string input = "{\"session\":\"open\",\"request_id\":\"init\"}\n"
                        + req + "\n"
                        + "{\"session\":\"close\"}\n";
    std::string output;
    int rc = RunSessionWithStrings(input, &output);
    ASSERT(rc == 0, "session ndjson request should succeed");

    auto lines = SplitLines(output);
    ASSERT(lines.size() == 5, "ready + 2 batch rows + summary + close expected");

    auto ready = ParseJsonLine(lines[0]);
    auto batch0 = ParseJsonLine(lines[1]);
    auto batch1 = ParseJsonLine(lines[2]);
    auto summary = ParseJsonLine(lines[3]);
    auto close = ParseJsonLine(lines[4]);
    ASSERT(ready.error.empty() && ready.value.is_object(), "ready should be valid JSON");
    ASSERT(batch0.error.empty() && batch0.value.is_object(), "batch 0 should be valid JSON");
    ASSERT(batch1.error.empty() && batch1.value.is_object(), "batch 1 should be valid JSON");
    ASSERT(summary.error.empty() && summary.value.is_object(), "summary should be valid JSON");
    ASSERT(close.error.empty() && close.value.is_object(), "close should be valid JSON");

    ASSERT(batch0.value.as_object().find("type") != batch0.value.as_object().end() &&
            batch0.value.as_object().find("type")->second.as_string() == "sample_batch",
        "batch line should have type=sample_batch");
    ASSERT(batch1.value.as_object().find("type") != batch1.value.as_object().end() &&
            batch1.value.as_object().find("type")->second.as_string() == "sample_batch",
        "batch line should have type=sample_batch");
    ASSERT(summary.value.as_object().find("type") != summary.value.as_object().end() &&
            summary.value.as_object().find("type")->second.as_string() == "summary",
        "summary line should have type=summary");
    ASSERT(summary.value.as_object().find("state_token") != summary.value.as_object().end(),
        "summary line should carry state_token in session mode");
    ASSERT(summary.value.as_object().find("cost") != summary.value.as_object().end(),
        "summary line should include cost metadata");
    ASSERT(close.value.as_object().find("session") != close.value.as_object().end() &&
            close.value.as_object().find("session")->second.as_string() == "closed",
        "close ack should still be emitted after ndjson request");
    return true;
}

bool TestSessionNdjsonSummaryOnlyRequest() {
    std::string req = "{\"request_version\":1,\"request_id\":\"ndjson-summary-session\",\"mode\":\"point_set\"," 
        "\"output_mode\":\"ndjson\"," 
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"mandelbrot\"}],"
        "\"points\":[{\"x\":-0.75,\"y\":0.0},{\"x\":0.25,\"y\":0.0}],"
        "\"metrics\":[\"summary_mean_iterations\",\"summary_escape_fraction\"]}";

    std::string input = "{\"session\":\"open\",\"request_id\":\"init\"}\n"
                        + req + "\n"
                        + "{\"session\":\"close\"}\n";
    std::string output;
    int rc = RunSessionWithStrings(input, &output);
    ASSERT(rc == 0, "session summary-only ndjson request should succeed");

    auto lines = SplitLines(output);
    ASSERT(lines.size() == 3, "ready + summary + close expected for summary-only ndjson session request");

    auto ready = ParseJsonLine(lines[0]);
    auto summary = ParseJsonLine(lines[1]);
    auto close = ParseJsonLine(lines[2]);
    ASSERT(ready.error.empty() && ready.value.is_object(), "ready should be valid JSON");
    ASSERT(summary.error.empty() && summary.value.is_object(), "summary should be valid JSON");
    ASSERT(close.error.empty() && close.value.is_object(), "close should be valid JSON");
    ASSERT(summary.value.as_object().find("type") != summary.value.as_object().end() &&
            summary.value.as_object().find("type")->second.as_string() == "summary",
        "summary-only session request should emit only a summary line");
    ASSERT(summary.value.as_object().find("state_token") != summary.value.as_object().end(),
        "summary-only session ndjson should carry state_token");
    ASSERT(summary.value.as_object().find("cost") != summary.value.as_object().end(),
        "summary-only session ndjson should include cost metadata");
    ASSERT(close.value.as_object().find("session") != close.value.as_object().end() &&
            close.value.as_object().find("session")->second.as_string() == "closed",
        "close ack should still be emitted after summary-only ndjson request");
    return true;
}

bool TestSessionNdjsonSequencePointSetRequest() {
    std::string req = "{\"request_version\":1,\"request_id\":\"ndjson-sequence-point-session\",\"mode\":\"sequence_point_set\"," 
        "\"output_mode\":\"ndjson\"," 
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"newton\"}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3},{\"x\":-0.5,\"y\":0.0}],"
        "\"sequence\":{\"zip_paths\":true,\"vary\":[{\"path\":\"fractal.view.zoom\",\"values\":[1.0,2.0]}]},"
        "\"metrics\":[\"iterations\",\"status\",\"summary_mean_iterations\"]}";

    std::string input = "{\"session\":\"open\",\"request_id\":\"init\"}\n"
                        + req + "\n"
                        + "{\"session\":\"close\"}\n";
    std::string output;
    int rc = RunSessionWithStrings(input, &output);
    ASSERT(rc == 0, "session sequence_point_set ndjson request should succeed");

    auto lines = SplitLines(output);
    ASSERT(lines.size() == 5, "ready + 2 sequence batches + summary + close expected");

    auto batch0 = ParseJsonLine(lines[1]);
    auto batch1 = ParseJsonLine(lines[2]);
    auto summary = ParseJsonLine(lines[3]);
    ASSERT(batch0.error.empty() && batch0.value.is_object(), "sequence point batch 0 should be valid JSON");
    ASSERT(batch1.error.empty() && batch1.value.is_object(), "sequence point batch 1 should be valid JSON");
    ASSERT(summary.error.empty() && summary.value.is_object(), "summary should be valid JSON");
    ASSERT(batch0.value.as_object().find("type") != batch0.value.as_object().end() &&
            batch0.value.as_object().find("type")->second.as_string() == "sample_batch",
        "first sequence_point_set session line should be sample_batch");
    ASSERT(batch1.value.as_object().find("type") != batch1.value.as_object().end() &&
            batch1.value.as_object().find("type")->second.as_string() == "sample_batch",
        "second sequence_point_set session line should be sample_batch");
    ASSERT(batch0.value.as_object().find("sequence_index") != batch0.value.as_object().end() &&
            batch0.value.as_object().find("sequence_index")->second.as_number() == 0.0,
        "first sequence_point_set session batch should report sequence_index 0");
    ASSERT(batch1.value.as_object().find("sequence_index") != batch1.value.as_object().end() &&
            batch1.value.as_object().find("sequence_index")->second.as_number() == 1.0,
        "second sequence_point_set session batch should report sequence_index 1");
    ASSERT(batch0.value.as_object().find("row_index") == batch0.value.as_object().end(),
        "sequence_point_set session batches should not include row_index");
    ASSERT(batch1.value.as_object().find("row_index") == batch1.value.as_object().end(),
        "sequence_point_set session batches should not include row_index");
    ASSERT(batch0.value.as_object().find("samples") != batch0.value.as_object().end() &&
            batch0.value.as_object().find("samples")->second.is_array() &&
            batch0.value.as_object().find("samples")->second.as_array().size() == 2,
        "each sequence_point_set session batch should contain all requested points");
    ASSERT(batch1.value.as_object().find("samples") != batch1.value.as_object().end() &&
            batch1.value.as_object().find("samples")->second.is_array() &&
            batch1.value.as_object().find("samples")->second.as_array().size() == 2,
        "each sequence_point_set session batch should contain all requested points");
    ASSERT(summary.value.as_object().find("state_token") != summary.value.as_object().end(),
        "sequence_point_set session ndjson summary should carry state_token");
    return true;
}

bool TestSessionNdjsonBadRequestDoesNotMintStateToken() {
    std::string badReq = "{\"request_version\":1,\"request_id\":\"bad-ndjson\",\"mode\":\"point_set\"," 
        "\"output_mode\":\"ndjson\"}";
    std::string goodReq = "{\"request_version\":1,\"request_id\":\"good-ndjson\",\"mode\":\"point_set\"," 
        "\"output_mode\":\"ndjson\"," 
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"newton\"}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3}],"
        "\"metrics\":[\"iterations\",\"status\",\"summary_mean_iterations\"]}";

    std::string input = "{\"session\":\"open\",\"request_id\":\"init\"}\n"
                        + badReq + "\n"
                        + goodReq + "\n"
                        + "{\"session\":\"close\"}\n";
    std::string output;
    int rc = RunSessionWithStrings(input, &output);
    ASSERT(rc == 0, "bad ndjson request should not kill session");

    auto lines = SplitLines(output);
    ASSERT(lines.size() == 5, "ready + error + batch + summary + close expected");

    auto bad = ParseJsonLine(lines[1]);
    auto batch = ParseJsonLine(lines[2]);
    auto summary = ParseJsonLine(lines[3]);
    ASSERT(bad.error.empty() && bad.value.is_object(), "bad ndjson fallback should be valid JSON");
    ASSERT(batch.error.empty() && batch.value.is_object(), "good ndjson batch should be valid JSON");
    ASSERT(summary.error.empty() && summary.value.is_object(), "good ndjson summary should be valid JSON");
    ASSERT(bad.value.as_object().find("ok") != bad.value.as_object().end() &&
            !bad.value.as_object().find("ok")->second.as_bool(),
        "bad ndjson fallback should be ok=false");
    ASSERT(bad.value.as_object().find("state_token") == bad.value.as_object().end(),
        "bad ndjson fallback should not mint a state_token");
    ASSERT(summary.value.as_object().find("state_token") != summary.value.as_object().end() &&
            summary.value.as_object().find("state_token")->second.as_string() == "s1",
        "first successful ndjson summary after an error should still mint s1");
    return true;
}

bool TestSessionNdjsonSequenceGridBatchesPerSequenceStep() {
    std::string req = "{\"request_version\":1,\"request_id\":\"ndjson-sequence-session\",\"mode\":\"sequence_grid\"," 
        "\"output_mode\":\"ndjson\"," 
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"mandelbrot\"}],"
        "\"region\":{\"center_x\":-0.75,\"center_y\":0.0,\"span_x\":0.5,\"span_y\":0.5,\"grid_width\":2,\"grid_height\":2},"
        "\"sequence\":{\"zip_paths\":true,\"vary\":[{\"path\":\"fractal.view.zoom\",\"values\":[1.0,2.0]}]},"
        "\"metrics\":[\"iterations\",\"status\",\"summary_mean_iterations\"]}";

    std::string input = "{\"session\":\"open\",\"request_id\":\"init\"}\n"
                        + req + "\n"
                        + "{\"session\":\"close\"}\n";
    std::string output;
    int rc = RunSessionWithStrings(input, &output);
    ASSERT(rc == 0, "session sequence_grid ndjson request should succeed");

    auto lines = SplitLines(output);
    ASSERT(lines.size() == 5, "ready + 2 sequence batches + summary + close expected");

    auto batch0 = ParseJsonLine(lines[1]);
    auto batch1 = ParseJsonLine(lines[2]);
    auto summary = ParseJsonLine(lines[3]);
    ASSERT(batch0.error.empty() && batch0.value.is_object(), "sequence batch 0 should be valid JSON");
    ASSERT(batch1.error.empty() && batch1.value.is_object(), "sequence batch 1 should be valid JSON");
    ASSERT(summary.error.empty() && summary.value.is_object(), "summary should be valid JSON");
    ASSERT(batch0.value.as_object().find("row_index") == batch0.value.as_object().end(),
        "session sequence-grid batches should not include row_index");
    ASSERT(batch1.value.as_object().find("row_index") == batch1.value.as_object().end(),
        "session sequence-grid batches should not include row_index");
    ASSERT(batch0.value.as_object().find("samples") != batch0.value.as_object().end() &&
            batch0.value.as_object().find("samples")->second.is_array() &&
            batch0.value.as_object().find("samples")->second.as_array().size() == 4,
        "session sequence batch 0 should contain full grid");
    ASSERT(batch1.value.as_object().find("samples") != batch1.value.as_object().end() &&
            batch1.value.as_object().find("samples")->second.is_array() &&
            batch1.value.as_object().find("samples")->second.as_array().size() == 4,
        "session sequence batch 1 should contain full grid");
    ASSERT(summary.value.as_object().find("state_token") != summary.value.as_object().end(),
        "session ndjson summary should still carry state_token");
    return true;
}

// --- V2-C: MergeOverrides unit tests ---

static FractalProbeOverride MakeOverride(const std::string& path, double value) {
    FractalProbeOverride o;
    o.path = path;
    o.value = FractalProbeScalar::Number(value);
    return o;
}

static FractalProbeOverride MakeStringOverride(const std::string& path, const std::string& value) {
    FractalProbeOverride o;
    o.path = path;
    o.value = FractalProbeScalar::String(value);
    return o;
}

bool TestMergeOverridesBothEmpty() {
    std::vector<FractalProbeOverride> base;
    std::vector<FractalProbeOverride> diff;
    auto merged = MergeOverrides(base, diff);
    ASSERT(merged.empty(), "merging two empty vectors should produce empty");
    return true;
}

bool TestMergeOverridesEmptyBase() {
    std::vector<FractalProbeOverride> base;
    std::vector<FractalProbeOverride> diff = { MakeOverride("a.b", 1.0) };
    auto merged = MergeOverrides(base, diff);
    ASSERT(merged.size() == 1, "empty base + 1 diff = 1 override");
    ASSERT(merged[0].path == "a.b", "path should be a.b");
    ASSERT(merged[0].value.number_value == 1.0, "value should be 1.0");
    return true;
}

bool TestMergeOverridesEmptyDiff() {
    std::vector<FractalProbeOverride> base = { MakeOverride("x.y", 2.0) };
    std::vector<FractalProbeOverride> diff;
    auto merged = MergeOverrides(base, diff);
    ASSERT(merged.size() == 1, "1 base + empty diff = 1 override");
    ASSERT(merged[0].path == "x.y", "path preserved from base");
    ASSERT(merged[0].value.number_value == 2.0, "value preserved from base");
    return true;
}

bool TestMergeOverridesDisjointPaths() {
    std::vector<FractalProbeOverride> base = { MakeOverride("a", 1.0) };
    std::vector<FractalProbeOverride> diff = { MakeOverride("b", 2.0) };
    auto merged = MergeOverrides(base, diff);
    ASSERT(merged.size() == 2, "disjoint paths should produce union");
    // base entry first, then appended diff entry
    ASSERT(merged[0].path == "a" && merged[0].value.number_value == 1.0, "base entry first");
    ASSERT(merged[1].path == "b" && merged[1].value.number_value == 2.0, "diff entry second");
    return true;
}

bool TestMergeOverridesReplacesMatchingPath() {
    std::vector<FractalProbeOverride> base = { MakeOverride("a", 1.0) };
    std::vector<FractalProbeOverride> diff = { MakeOverride("a", 99.0) };
    auto merged = MergeOverrides(base, diff);
    ASSERT(merged.size() == 1, "matching path should replace, not duplicate");
    ASSERT(merged[0].path == "a", "path should be a");
    ASSERT(merged[0].value.number_value == 99.0, "value should be replaced by diff");
    return true;
}

bool TestMergeOverridesMixed() {
    // Base: [a=1, b=2, c=3]  Diff: [b=20, d=4]
    // Expected: [a=1, b=20, c=3, d=4]
    std::vector<FractalProbeOverride> base = {
        MakeOverride("a", 1.0), MakeOverride("b", 2.0), MakeOverride("c", 3.0)
    };
    std::vector<FractalProbeOverride> diff = {
        MakeOverride("b", 20.0), MakeOverride("d", 4.0)
    };
    auto merged = MergeOverrides(base, diff);
    ASSERT(merged.size() == 4, "should have 4 entries after merge");
    ASSERT(merged[0].path == "a" && merged[0].value.number_value == 1.0, "a=1 from base");
    ASSERT(merged[1].path == "b" && merged[1].value.number_value == 20.0, "b=20 replaced by diff");
    ASSERT(merged[2].path == "c" && merged[2].value.number_value == 3.0, "c=3 from base");
    ASSERT(merged[3].path == "d" && merged[3].value.number_value == 4.0, "d=4 appended from diff");
    return true;
}

bool TestMergeOverridesKindChange() {
    // Diff can change the value kind (number -> string) for a matching path.
    std::vector<FractalProbeOverride> base = { MakeOverride("a", 1.0) };
    std::vector<FractalProbeOverride> diff = { MakeStringOverride("a", "hello") };
    auto merged = MergeOverrides(base, diff);
    ASSERT(merged.size() == 1, "should have 1 entry");
    ASSERT(merged[0].value.kind == FractalProbeScalar::Kind::string, "kind should change to string");
    ASSERT(merged[0].value.string_value == "hello", "string value should be hello");
    return true;
}

// --- V2-C: Session diff integration tests ---

// Helper: extract state_token from a JSON line.
static std::string GetToken(const std::string& line) {
    auto p = json_min::Parse(line);
    if (!p.error.empty() || !p.value.is_object()) return "";
    auto it = p.value.as_object().find("state_token");
    if (it == p.value.as_object().end() || !it->second.is_string()) return "";
    return it->second.as_string();
}

// Helper: extract runtime.fractal_type from a JSON response line.
static std::string GetRuntimeFractalType(const std::string& line) {
    auto p = json_min::Parse(line);
    if (!p.error.empty() || !p.value.is_object()) return "";
    auto rtIt = p.value.as_object().find("runtime");
    if (rtIt == p.value.as_object().end() || !rtIt->second.is_object()) return "";
    auto ftIt = rtIt->second.as_object().find("fractal_type");
    if (ftIt == rtIt->second.as_object().end() || !ftIt->second.is_string()) return "";
    return ftIt->second.as_string();
}

bool TestSessionDiffValidTokenMergesOverrides() {
    // r1: set fractal_type=mandelbrot + point.  r2 (diff from s1): only override
    // max_iter, no fractal_type.  Without merge, the default is newton.
    // With merge, fractal_type=mandelbrot should carry forward.
    std::string r1 = "{\"request_version\":1,\"request_id\":\"r1\",\"mode\":\"point_set\","
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"mandelbrot\"}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3}]}";

    std::string input = "{\"session\":\"open\"}\n"
                        + r1 + "\n";

    // We need to capture s1 token, then build r2 dynamically.
    // Use RunSessionMode directly with istringstream/ostringstream to read
    // the token, but we can't inject the 2nd request after reading the first.
    // Instead, construct a 2-request session where r2 references "s1"
    // (the first response token).
    // MakeStateToken(1) = "s1", MakeStateToken(2) = "s2" etc.
    // After open (counter=0, token="s0"), r1 response increments to counter=1
    // → token "s1".  So r2 should reference "s1".

    std::string r2 = "{\"request_version\":1,\"request_id\":\"r2\","
        "\"state_token\":\"s1\","
        "\"mode\":\"point_set\","
        "\"overrides\":[{\"path\":\"fractal.view.zoom\",\"value\":2.0}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3}]}";

    std::string fullInput = "{\"session\":\"open\"}\n"
                            + r1 + "\n"
                            + r2 + "\n"
                            + "{\"session\":\"close\"}\n";
    std::string output;
    int rc = RunSessionWithStrings(fullInput, &output);
    ASSERT(rc == 0, "diff session should succeed");

    auto lines = SplitLines(output);
    ASSERT(lines.size() == 4, "ready + r1_resp + r2_resp + close-ack");

    // r1 response should have fractal_type=mandelbrot
    std::string ft1 = GetRuntimeFractalType(lines[1]);
    ASSERT(ft1 == "mandelbrot", "r1 fractal_type should be mandelbrot");

    // r2 response should also have fractal_type=mandelbrot (carried from merge)
    std::string ft2 = GetRuntimeFractalType(lines[2]);
    ASSERT(ft2 == "mandelbrot", "r2 should inherit fractal_type=mandelbrot via merge");

    // Both should have state_token
    std::string t1 = GetToken(lines[1]);
    std::string t2 = GetToken(lines[2]);
    ASSERT(!t1.empty() && !t2.empty(), "both responses must have state_token");
    ASSERT(t1 != t2, "tokens should be different");
    return true;
}

bool TestSessionReadyTokenIsUsableAsBaselineState() {
    std::string request = "{\"request_version\":1,\"request_id\":\"use-s0\"," 
        "\"state_token\":\"s0\"," 
        "\"mode\":\"point_set\"," 
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"mandelbrot\"}],"
        "\"points\":[{\"x\":-0.5,\"y\":0.0}]}";

    std::string input = "{\"session\":\"open\"}\n"
                        + request + "\n"
                        + "{\"session\":\"close\"}\n";
    std::string output;
    int rc = RunSessionWithStrings(input, &output);
    ASSERT(rc == 0, "session should close cleanly after s0 baseline request");

    auto lines = SplitLines(output);
    ASSERT(lines.size() == 3, "ready + response + close-ack");

    auto ready = ParseJsonLine(lines[0]);
    ASSERT(ready.value.is_object(), "ready response should be JSON object");
    auto readyTokenIt = ready.value.as_object().find("state_token");
    ASSERT(readyTokenIt != ready.value.as_object().end() && readyTokenIt->second.is_string(),
        "ready response should include state_token");
    ASSERT(readyTokenIt->second.as_string() == "s0",
        "ready response should advertise the baseline token s0");

    auto response = ParseJsonLine(lines[1]);
    ASSERT(response.value.is_object(), "sample response should be JSON object");
    auto okIt = response.value.as_object().find("ok");
    ASSERT(okIt != response.value.as_object().end() && okIt->second.as_bool(),
        "first request should be able to reference the ready token s0");
    ASSERT(GetRuntimeFractalType(lines[1]) == "mandelbrot",
        "request should still honor explicit overrides when diffing from s0");
    ASSERT(GetToken(lines[1]) == "s1",
        "first successful request should mint s1 after the ready token s0");
    return true;
}

bool TestSessionDiffInvalidTokenErrors() {
    // Referencing a non-existent state_token should produce an error.
    std::string r1 = "{\"request_version\":1,\"request_id\":\"r1\","
        "\"state_token\":\"s999\","
        "\"mode\":\"point_set\","
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"newton\"}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3}]}";

    std::string input = "{\"session\":\"open\"}\n"
                        + r1 + "\n"
                        + "{\"session\":\"close\"}\n";
    std::string output;
    int rc = RunSessionWithStrings(input, &output);
    ASSERT(rc == 0, "session should still close cleanly after error");

    auto lines = SplitLines(output);
    ASSERT(lines.size() == 3, "ready + error + close-ack");

    auto p1 = ParseJsonLine(lines[1]);
    ASSERT(p1.value.is_object(), "error response should be JSON object");
    auto okIt = p1.value.as_object().find("ok");
    ASSERT(okIt != p1.value.as_object().end() && !okIt->second.as_bool(),
        "invalid state_token should produce ok=false");
    auto errIt = p1.value.as_object().find("error");
    ASSERT(errIt != p1.value.as_object().end() && errIt->second.is_string(),
        "should have error message");
    // Error should mention the token
    ASSERT(errIt->second.as_string().find("s999") != std::string::npos,
        "error should mention the invalid token");
    return true;
}

bool TestSessionDiffWithoutTokenWorksNormally() {
    // A request without state_token should work exactly like V2-B (no merge).
    std::string req = "{\"request_version\":1,\"request_id\":\"r1\",\"mode\":\"point_set\","
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"newton\"}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3}]}";

    std::string input = "{\"session\":\"open\"}\n"
                        + req + "\n"
                        + "{\"session\":\"close\"}\n";
    std::string output;
    int rc = RunSessionWithStrings(input, &output);
    ASSERT(rc == 0, "session without state_token should succeed");

    auto lines = SplitLines(output);
    ASSERT(lines.size() == 3, "ready + response + close-ack");

    auto p1 = ParseJsonLine(lines[1]);
    ASSERT(p1.value.is_object(), "response should be JSON object");
    auto okIt = p1.value.as_object().find("ok");
    ASSERT(okIt != p1.value.as_object().end() && okIt->second.as_bool(),
        "no-token request should still succeed");
    auto tokIt = p1.value.as_object().find("state_token");
    ASSERT(tokIt != p1.value.as_object().end() && tokIt->second.is_string(),
        "response should still have state_token");
    return true;
}

bool TestSessionDiffChainedDiffs() {
    // r1: fractal_type=mandelbrot (token s0→s1)
    // r2: diff from s1, add max_iterations=200 (token s1→s2)
    // r3: diff from s2, change only zoom (token s2→s3)
    // r3 should still reflect fractal_type=mandelbrot (from r1) carried through r2→r3.
    std::string r1 = "{\"request_version\":1,\"request_id\":\"r1\",\"mode\":\"point_set\","
        "\"overrides\":[{\"path\":\"fractal.view.fractal_type\",\"value\":\"mandelbrot\"}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3}]}";
    std::string r2 = "{\"request_version\":1,\"request_id\":\"r2\","
        "\"state_token\":\"s1\","
        "\"mode\":\"point_set\","
        "\"overrides\":[{\"path\":\"fractal.view.zoom\",\"value\":2.0}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3}]}";
    std::string r3 = "{\"request_version\":1,\"request_id\":\"r3\","
        "\"state_token\":\"s2\","
        "\"mode\":\"point_set\","
        "\"overrides\":[{\"path\":\"fractal.params.max_iter\",\"value\":100}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3}]}";

    std::string input = "{\"session\":\"open\"}\n"
                        + r1 + "\n" + r2 + "\n" + r3 + "\n"
                        + "{\"session\":\"close\"}\n";
    std::string output;
    int rc = RunSessionWithStrings(input, &output);
    ASSERT(rc == 0, "chained diff session should succeed");

    auto lines = SplitLines(output);
    ASSERT(lines.size() == 5, "ready + 3 responses + close-ack");

    // All three responses should show fractal_type=mandelbrot
    for (int i = 1; i <= 3; ++i) {
        std::string ft = GetRuntimeFractalType(lines[i]);
        ASSERT(ft == "mandelbrot",
            ("response " + std::to_string(i) + " should have fractal_type=mandelbrot").c_str());
    }

    // Tokens should all be different
    std::string t1 = GetToken(lines[1]);
    std::string t2 = GetToken(lines[2]);
    std::string t3 = GetToken(lines[3]);
    ASSERT(t1 != t2 && t2 != t3 && t1 != t3, "all tokens should be distinct");
    return true;
}

bool TestProcessSessionLineDiffRequest() {
    // Unit test: ProcessSessionLine with a diff request that references
    // accumulated overrides.
    bool sessionOpen = true;
    bool sessionDone = false;
    int counter = 1;
    SessionOverrideMap accumulated;

    // Seed the accumulated map with s1 → [fractal_type=mandelbrot]
    accumulated["s1"] = { MakeStringOverride("fractal.view.fractal_type", "mandelbrot") };

    // Send a diff request referencing s1 with only zoom override
    std::string result = ProcessSessionLine(
        "{\"request_version\":1,\"request_id\":\"rd\","
        "\"state_token\":\"s1\","
        "\"mode\":\"point_set\","
        "\"overrides\":[{\"path\":\"fractal.view.zoom\",\"value\":2.0}],"
        "\"points\":[{\"x\":0.5,\"y\":0.3}]}",
        &sessionOpen, &sessionDone, &counter, &accumulated, kExePath);

    ASSERT(!result.empty(), "diff request should produce a response");
    ASSERT(!sessionDone, "session should remain open");

    auto p = json_min::Parse(result);
    ASSERT(p.error.empty() && p.value.is_object(), "response should be valid JSON");
    auto& obj = p.value.as_object();
    auto okIt = obj.find("ok");
    ASSERT(okIt != obj.end() && okIt->second.as_bool(), "diff request should succeed");

    // Check runtime shows mandelbrot (from merge, not default newton)
    std::string ft = GetRuntimeFractalType(result);
    ASSERT(ft == "mandelbrot", "merged overrides should include fractal_type=mandelbrot");

    // The new token should be in the accumulated map
    std::string newToken = GetToken(result);
    ASSERT(!newToken.empty(), "response must have state_token");
    ASSERT(accumulated.count(newToken) == 1,
        "accumulated map should contain entry for new token");
    return true;
}

#define RUN(fn) do { \
    if (fn()) { std::fprintf(stderr, "  PASS: %s\n", #fn); } \
    else { std::fprintf(stderr, "  FAIL: %s\n", #fn); } \
} while (0)

int main() {
    RUN(TestWriteAndReadTextFile);
    RUN(TestTryReadTextFileExact);
    RUN(TestTryReadNonexistentFile);
    RUN(TestWriteToInvalidPath);
    RUN(TestBuildProbeErrorResponseFields);
    RUN(TestEmitProbeResponseToFile);
    RUN(TestEmitProbeResponseEmptyPath);
    RUN(TestSampleModeNoSource);
    RUN(TestSampleModeNoSink);
    RUN(TestSampleModeDualSource);
    RUN(TestSampleModeConflictWithValidateUi);
    RUN(TestSampleModeBadRequestJson);
    RUN(TestSampleModeValidFileRoundTrip);

    // V2-A: Batch request array
    RUN(TestSampleModeBatchTwoValidRequests);
    RUN(TestSampleModeBatchErrorIsolation);
    RUN(TestSampleModeBatchEmptyArray);
    RUN(TestSampleModeRootNotObjectOrArray);
    RUN(TestSampleModeSingleObjectStillReturnsSingleObject);
    RUN(TestSampleModeNdjsonGridResponse);
    RUN(TestSampleModeNdjsonPointSetSingleBatchAndMetricFiltering);
    RUN(TestSampleModeNdjsonSummaryOnlyEmitsSummaryLine);
    RUN(TestSampleModeGenericSampleNdjsonIncludesFunctionId);
    RUN(TestSampleModeNdjsonSequenceGridBatchesPerSequenceStep);
    RUN(TestSampleModeNdjsonSequencePointSetBatchesPerSequenceStep);
    RUN(TestSampleModeBatchRejectsNdjsonOutputMode);

    // V2-B: Session mode — ProcessSessionLine unit tests
    RUN(TestProcessSessionLineOpen);
    RUN(TestProcessSessionLineClose);
    RUN(TestProcessSessionLineEmptyReturnsEmpty);
    RUN(TestProcessSessionLineRequestNotOpen);
    RUN(TestProcessSessionLineValidRequest);
    RUN(TestProcessSessionLineMalformedJson);

    // V2-B: Session mode — RunSessionMode integration tests
    RUN(TestSessionOpenReady);
    RUN(TestSessionCloseAck);
    RUN(TestSessionSingleRequest);
    RUN(TestSessionStateTokenIncrements);
    RUN(TestSessionRequestWithoutOpen);
    RUN(TestSessionMalformedJsonLine);
    RUN(TestSessionEmptyLines);
    RUN(TestSessionEofWithoutClose);
    RUN(TestSessionDoubleOpen);
    RUN(TestSessionBadRequestPreservesSession);
    RUN(TestSessionBadRequestDoesNotMintStateToken);
    RUN(TestSessionUnknownSessionVerb);
    RUN(TestSessionResponseVersionField);
    RUN(TestSessionNdjsonGridRequest);
    RUN(TestSessionNdjsonSummaryOnlyRequest);
    RUN(TestSessionNdjsonSequencePointSetRequest);
    RUN(TestSessionNdjsonSequenceGridBatchesPerSequenceStep);
    RUN(TestSessionNdjsonBadRequestDoesNotMintStateToken);

    // V2-C: MergeOverrides unit tests
    RUN(TestMergeOverridesBothEmpty);
    RUN(TestMergeOverridesEmptyBase);
    RUN(TestMergeOverridesEmptyDiff);
    RUN(TestMergeOverridesDisjointPaths);
    RUN(TestMergeOverridesReplacesMatchingPath);
    RUN(TestMergeOverridesMixed);
    RUN(TestMergeOverridesKindChange);

    // V2-C: Session diff — ProcessSessionLine unit test
    RUN(TestProcessSessionLineDiffRequest);

    // V2-C: Session diff — RunSessionMode integration tests
    RUN(TestSessionDiffValidTokenMergesOverrides);
    RUN(TestSessionReadyTokenIsUsableAsBaselineState);
    RUN(TestSessionDiffInvalidTokenErrors);
    RUN(TestSessionDiffWithoutTokenWorksNormally);
    RUN(TestSessionDiffChainedDiffs);

    std::fprintf(stderr, "test_headless_modes: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}
