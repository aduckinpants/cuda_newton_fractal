#include "../src/viewer_schema_load.h"
#include "../src/fractal_types.h"

// Standalone ReadTextFile — avoids linking headless_modes.cpp and its heavy dependency chain.
#include "../src/headless_modes.h"
#include <fstream>
#include <sstream>
std::string ReadTextFile(const char* path) {
    std::ifstream f(path, std::ios::in | std::ios::binary);
    if (!f) return {};
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}
bool TryReadTextFileExact(const std::string&, std::string*, std::string*) { return false; }
bool WriteTextFileExact(const std::string&, const std::string&, std::string*) { return false; }

#include <cstdio>
#include <cstdlib>
#include <cstring>

static int gPass = 0, gFail = 0;
#define CHECK(name, cond) do { if (cond) { ++gPass; printf("  PASS: %s\n", name); } \
    else { ++gFail; printf("  FAIL: %s  (%s:%d)\n", name, __FILE__, __LINE__); } } while(0)

// Write a temp file, returns true on success.
static bool WriteTempFile(const char* path, const char* content) {
    FILE* f = fopen(path, "wb");
    if (!f) return false;
    size_t len = strlen(content);
    size_t written = fwrite(content, 1, len, f);
    fclose(f);
    return written == len;
}
static void RemoveTempFile(const char* path) { remove(path); }

// Minimal valid schema JSON for testing.
static const char* kMinimalSchemaJson =
    "{ \"schema_version\": \"1\", \"namespace\": \"test\", \"panels\": [] }";

// --- Test: valid schema from candidate path ---

static void TestValidSchemaLoads() {
    const char* tmpPath = "_test_schema_valid.json";
    WriteTempFile(tmpPath, kMinimalSchemaJson);

    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    LensSettings lens{};
    BindingContext bind;
    bind.view = &view;
    bind.params = &params;
    bind.render = &render;
    bind.lens = &lens;

    std::vector<std::string> candidates = { tmpPath };
    SchemaLoadResult r = LoadAndValidateViewerSchema(candidates, bind, false);

    CHECK("ValidSchema_NotFatal", !r.fatal_error);
    CHECK("ValidSchema_FromFile", r.from_file);
    CHECK("ValidSchema_Path", r.path == tmpPath);
    CHECK("ValidSchema_NoWarning", r.warning.empty());

    RemoveTempFile(tmpPath);
}

// --- Test: no candidates → SafeMode ---

static void TestNoCandidatesSafeMode() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    LensSettings lens{};
    BindingContext bind;
    bind.view = &view;
    bind.params = &params;
    bind.render = &render;
    bind.lens = &lens;

    std::vector<std::string> candidates;
    SchemaLoadResult r = LoadAndValidateViewerSchema(candidates, bind, false);

    CHECK("NoCandidates_NotFatal", !r.fatal_error);
    CHECK("NoCandidates_NotFromFile", !r.from_file);
    CHECK("NoCandidates_HasWarning", !r.warning.empty());
}

// --- Test: all candidates missing → SafeMode ---

static void TestMissingFilesSafeMode() {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    LensSettings lens{};
    BindingContext bind;
    bind.view = &view;
    bind.params = &params;
    bind.render = &render;
    bind.lens = &lens;

    std::vector<std::string> candidates = { "_nonexistent_a.json", "_nonexistent_b.json" };
    SchemaLoadResult r = LoadAndValidateViewerSchema(candidates, bind, false);

    CHECK("MissingFiles_NotFatal", !r.fatal_error);
    CHECK("MissingFiles_NotFromFile", !r.from_file);
    CHECK("MissingFiles_HasWarning", !r.warning.empty());
    CHECK("MissingFiles_PathIsFirst", r.path == "_nonexistent_a.json");
}

// --- Test: bad JSON → SafeMode with parse error ---

static void TestBadJsonSafeMode() {
    const char* tmpPath = "_test_schema_bad.json";
    WriteTempFile(tmpPath, "{ not valid json !!!");

    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    LensSettings lens{};
    BindingContext bind;
    bind.view = &view;
    bind.params = &params;
    bind.render = &render;
    bind.lens = &lens;

    std::vector<std::string> candidates = { tmpPath };
    SchemaLoadResult r = LoadAndValidateViewerSchema(candidates, bind, false);

    CHECK("BadJson_NotFatal", !r.fatal_error);
    CHECK("BadJson_NotFromFile", !r.from_file);
    CHECK("BadJson_HasWarning", !r.warning.empty());
    CHECK("BadJson_WarningMentionsParseError", r.warning.find("parse error") != std::string::npos);
    CHECK("BadJson_Path", r.path == tmpPath);

    RemoveTempFile(tmpPath);
}

// --- Test: first candidate bad, second good → SafeMode (stops at first error) ---

static void TestFirstBadStopsSearch() {
    const char* badPath = "_test_schema_bad2.json";
    const char* goodPath = "_test_schema_good2.json";
    WriteTempFile(badPath, "{ broken }");
    WriteTempFile(goodPath, kMinimalSchemaJson);

    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    LensSettings lens{};
    BindingContext bind;
    bind.view = &view;
    bind.params = &params;
    bind.render = &render;
    bind.lens = &lens;

    std::vector<std::string> candidates = { badPath, goodPath };
    SchemaLoadResult r = LoadAndValidateViewerSchema(candidates, bind, false);

    // First candidate had an error → SafeMode, not the second candidate.
    CHECK("FirstBad_NotFromFile", !r.from_file);
    CHECK("FirstBad_PathIsBad", r.path == badPath);
    CHECK("FirstBad_HasWarning", !r.warning.empty());

    RemoveTempFile(badPath);
    RemoveTempFile(goodPath);
}

// --- Test: first candidate missing, second good → loads second ---

static void TestSkipsMissingCandidate() {
    const char* goodPath = "_test_schema_skip.json";
    WriteTempFile(goodPath, kMinimalSchemaJson);

    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    LensSettings lens{};
    BindingContext bind;
    bind.view = &view;
    bind.params = &params;
    bind.render = &render;
    bind.lens = &lens;

    std::vector<std::string> candidates = { "_nonexistent_skip.json", goodPath };
    SchemaLoadResult r = LoadAndValidateViewerSchema(candidates, bind, false);

    CHECK("SkipMissing_NotFatal", !r.fatal_error);
    CHECK("SkipMissing_FromFile", r.from_file);
    CHECK("SkipMissing_PathIsGood", r.path == goodPath);
    CHECK("SkipMissing_NoWarning", r.warning.empty());

    RemoveTempFile(goodPath);
}

// --- Test: validate_ui_only with bad binding → fatal ---

static void TestValidateUiOnlyFatal() {
    // Write a schema with a binding that doesn't exist → validation will fail.
    const char* schemaJson =
        "{ \"schema_version\": \"1\", \"namespace\": \"test\", \"panels\": [ { \"id\": \"p1\", \"label\": \"test\", \"controls\": ["
        "{ \"id\": \"bad\", \"type\": \"slider\", \"label\": \"Bad\", \"value_type\": \"float\","
        "  \"binding\": { \"kind\": \"param\", \"path\": \"nonexistent.binding.path\" },"
        "  \"min\": 0, \"max\": 1 }"
        "] } ] }";
    const char* tmpPath = "_test_schema_badbind.json";
    WriteTempFile(tmpPath, schemaJson);

    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    LensSettings lens{};
    BindingContext bind;
    bind.view = &view;
    bind.params = &params;
    bind.render = &render;
    bind.lens = &lens;

    std::vector<std::string> candidates = { tmpPath };
    SchemaLoadResult r = LoadAndValidateViewerSchema(candidates, bind, true);

    CHECK("ValidateUiFatal_IsFatal", r.fatal_error);

    RemoveTempFile(tmpPath);
}

// --- Test: bad binding without validate_ui_only → SafeMode ---

static void TestBadBindingSafeMode() {
    const char* schemaJson =
        "{ \"schema_version\": \"1\", \"namespace\": \"test\", \"panels\": [ { \"id\": \"p1\", \"label\": \"test\", \"controls\": ["
        "{ \"id\": \"bad\", \"type\": \"slider\", \"label\": \"Bad\", \"value_type\": \"float\","
        "  \"binding\": { \"kind\": \"param\", \"path\": \"nonexistent.binding.path\" },"
        "  \"min\": 0, \"max\": 1 }"
        "] } ] }";
    const char* tmpPath = "_test_schema_badbind2.json";
    WriteTempFile(tmpPath, schemaJson);

    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    LensSettings lens{};
    BindingContext bind;
    bind.view = &view;
    bind.params = &params;
    bind.render = &render;
    bind.lens = &lens;

    std::vector<std::string> candidates = { tmpPath };
    SchemaLoadResult r = LoadAndValidateViewerSchema(candidates, bind, false);

    CHECK("BadBindSafe_NotFatal", !r.fatal_error);
    CHECK("BadBindSafe_HasWarning", !r.warning.empty());

    RemoveTempFile(tmpPath);
}

int main() {
    TestValidSchemaLoads();
    TestNoCandidatesSafeMode();
    TestMissingFilesSafeMode();
    TestBadJsonSafeMode();
    TestFirstBadStopsSearch();
    TestSkipsMissingCandidate();
    TestValidateUiOnlyFatal();
    TestBadBindingSafeMode();

    printf("test_viewer_schema_load: %d passed, %d failed\n", gPass, gFail);
    return gFail > 0 ? 1 : 0;
}
