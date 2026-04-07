#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <Windows.h>
#include "../src/headless_modes.h"

static int g_passed = 0;
static int g_failed = 0;

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

    std::fprintf(stderr, "test_headless_modes: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}
