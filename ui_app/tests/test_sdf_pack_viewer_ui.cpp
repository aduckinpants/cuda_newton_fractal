// Native tests for authored SDF pack viewer UI state, controls, and preview proof.

#include "sdf_pack_viewer_ui.h"

#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

static int g_passed = 0;
static int g_failed = 0;

static void Check(bool condition, const char* message) {
    if (condition) {
        ++g_passed;
    } else {
        ++g_failed;
        std::printf("  FAIL: %s\n", message);
    }
}

static bool ReadTextFile(const char* path, std::string* outText) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    std::ostringstream buffer;
    buffer << in.rdbuf();
    *outText = buffer.str();
    return true;
}

static bool WriteTextFile(const std::filesystem::path& path, const std::string& text) {
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    if (ec) return false;
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;
    out << text;
    return static_cast<bool>(out);
}

static bool ReadCatalogPackJson(const SdfPackViewerBuiltInPackCatalogEntry& entry, std::string* outText) {
    const std::string path = std::string("../") + entry.relative_path;
    return ReadTextFile(path.c_str(), outText);
}

static bool ReportHasBuiltInPackOption(
    const SdfPackViewerAutomationReport& report,
    const std::string& packId,
    bool selected) {
    for (const SdfPackViewerBuiltInPackReport& option : report.built_in_packs) {
        if (option.pack_id == packId) {
            return option.selected == selected && !option.label.empty();
        }
    }
    return false;
}

static const SdfPackParam* FindPackParamLocal(const SdfPack& pack, const std::string& id) {
    for (const SdfPackParam& param : pack.params) {
        if (param.id == id) {
            return &param;
        }
    }
    return nullptr;
}

static const SdfPackViewerControlReport* FindReportControl(
    const SdfPackViewerAutomationReport& report,
    const std::string& controlId) {
    for (const SdfPackViewerControlReport& control : report.controls) {
        if (control.control_id == controlId) {
            return &control;
        }
    }
    return nullptr;
}

static const char* kCirclePack = R"json({
  "schema": 1,
  "pack_id": "viewer_circle",
  "name": "Viewer Circle",
  "kind": "sdf_scene_2d",
  "params": [
    { "id": "radius", "type": "float", "default": 0.35, "range": [0.1, 1.0] },
    { "id": "offset_x", "type": "float", "default": 0.0, "range": [-1.0, 1.0] }
  ],
  "controls": [
    { "param": "radius", "label": "Radius", "ui_min": 0.1, "ui_max": 0.8 },
    { "param": "offset_x", "label": "Offset X", "ui_min": -0.5, "ui_max": 0.5 }
  ],
  "region": {
    "center": [0.0, 0.0],
    "half_height": 1.0
  },
  "ast": {
    "op": "circle",
    "center": [{ "param": "offset_x" }, 0.0],
    "radius": { "param": "radius" }
  }
})json";

static void TestBuiltInCatalogEntriesLoadAndLower() {
    const std::vector<SdfPackViewerBuiltInPackCatalogEntry>& catalog = SdfPackViewerBuiltInPackCatalog();
    Check(catalog.size() >= 3, "built-in SDF pack catalog exposes at least three curated entries");
    Check(std::string(SdfPackViewerDefaultBuiltInPackId()) == "sdf_smooth_lattice_2d",
        "built-in SDF pack catalog keeps smooth lattice as the default");
    Check(SdfPackViewerBuiltInPackSelectorAutomationId() == "sdf_pack.builtin_pack",
        "built-in SDF pack selector automation path is stable");

    std::set<std::string> ids;
    for (const SdfPackViewerBuiltInPackCatalogEntry& entry : catalog) {
        Check(!entry.pack_id.empty(), "built-in SDF pack catalog entry has an id");
        Check(!entry.label.empty(), "built-in SDF pack catalog entry has a label");
        Check(entry.relative_path.find("docs/examples/sdf_packs/") == 0,
            "built-in SDF pack catalog entry points at docs/examples/sdf_packs");
        Check(ids.insert(entry.pack_id).second, "built-in SDF pack catalog ids are unique");

        std::string json;
        Check(ReadCatalogPackJson(entry, &json), "built-in SDF pack catalog entry file is readable");
        if (json.empty()) continue;
        SdfPackParseResult parsed = ParseSdfPackJson(json);
        Check(parsed.ok, "built-in SDF pack catalog entry parses");
        if (!parsed.ok) continue;
        Check(parsed.pack.pack_id == entry.pack_id, "built-in SDF pack catalog id matches pack JSON");
        Check(parsed.pack.kind == "sdf_scene_2d", "built-in SDF pack catalog entry is an SDF scene");
        Check(!parsed.pack.controls.empty(), "built-in SDF pack catalog entry exposes controls");
        Check(LowerSdfPackToRuntimeDesc(parsed.pack, {}).ok,
            "built-in SDF pack catalog entry lowers using the shipped op surface");
    }
}

static void TestBuiltInSmoothLatticeViewerControls() {
    std::string json;
    Check(ReadTextFile("../docs/examples/sdf_packs/sdf_smooth_lattice_2d.sdf_pack.json", &json),
        "built-in smooth lattice pack file is readable for viewer UI");
    if (json.empty()) return;
    SdfPackViewerState state{};
    std::string error;
    Check(LoadSdfPackViewerJson(&state, json, "builtin://sdf_smooth_lattice_2d", &error),
        "built-in smooth lattice pack loads into viewer UI state");
    if (!state.have_pack) return;
    SdfPackViewerAutomationReport report = BuildSdfPackViewerAutomationReport(state);
    Check(report.pack_id == "sdf_smooth_lattice_2d", "built-in viewer report publishes pack id");
    Check(report.built_in_pack_selector_control_id == "sdf_pack.builtin_pack",
        "built-in viewer report publishes pack selector control id");
    Check(report.selected_built_in_pack_id == "sdf_smooth_lattice_2d",
        "built-in viewer report publishes selected built-in pack id");
    Check(ReportHasBuiltInPackOption(report, "sdf_smooth_lattice_2d", true),
        "built-in viewer report marks default pack option selected");
    Check(ReportHasBuiltInPackOption(report, "sdf_capsule_weave_2d", false),
        "built-in viewer report publishes capsule weave option");
    Check(ReportHasBuiltInPackOption(report, "sdf_ring_cells_2d", false),
        "built-in viewer report publishes ring cells option");
    Check(report.controls.size() == 6, "built-in viewer report exposes six controls");
    std::set<std::string> controlIds;
    for (const SdfPackViewerControlReport& control : report.controls) {
        controlIds.insert(control.control_id);
    }
    for (const char* id : {
            "sdf_pack.period.primary",
            "sdf_pack.radius.primary",
            "sdf_pack.smooth_blend.primary",
            "sdf_pack.rotation.primary",
            "sdf_pack.offset_x.primary",
            "sdf_pack.offset_y.primary",
        }) {
        Check(controlIds.find(id) != controlIds.end(), "built-in viewer report exposes required control id");
    }
    Check(SetSdfPackViewerControlValue(&state, "sdf_pack.period.primary", 1.2, &error),
        "built-in period control is editable through no-mouse set-value path");
    Check(std::fabs(state.params["period"] - 1.2) < 1e-12, "built-in period edit updates param state");
}

static void TestDefaultLoadPolicyDoesNotClobberCurrentPack() {
    SdfPackViewerState empty{};
    Check(SdfPackViewerShouldLoadDefaultBuiltInPack(empty),
        "empty SDF pack viewer state asks for the default built-in pack");

    SdfPackViewerState loaded{};
    std::string json;
    std::string error;
    Check(ReadCatalogPackJson(SdfPackViewerBuiltInPackCatalog()[1], &json),
        "non-default built-in pack file is readable for default policy test");
    Check(LoadSdfPackViewerJson(&loaded, json, "builtin://non_default", &error),
        "non-default built-in pack loads for default policy test");
    Check(!SdfPackViewerShouldLoadDefaultBuiltInPack(loaded),
        "default built-in load policy does not clobber an already loaded pack");

    SdfPackViewerState failed{};
    failed.initialized = true;
    failed.pack_load_error = "intentional failure";
    Check(!SdfPackViewerShouldLoadDefaultBuiltInPack(failed),
        "default built-in load policy does not retry over a failed initialized state");
}

static void TestRuntimeStagedBuiltInPackWinsOverRepoRootMetadata() {
    namespace fs = std::filesystem;
    const SdfPackViewerBuiltInPackCatalogEntry& entry = SdfPackViewerBuiltInPackCatalog().front();
    std::string validJson;
    Check(ReadCatalogPackJson(entry, &validJson), "runtime staging priority test reads catalog JSON");
    if (validJson.empty()) return;

    std::string repoRootJson = validJson;
    const std::string expectedId = "\"pack_id\": \"sdf_smooth_lattice_2d\"";
    const std::string wrongId = "\"pack_id\": \"repo_root_should_not_win\"";
    const std::size_t idPos = repoRootJson.find(expectedId);
    Check(idPos != std::string::npos, "runtime staging priority test finds built-in pack id");
    if (idPos == std::string::npos) return;
    repoRootJson.replace(idPos, expectedId.size(), wrongId);

    const fs::path tempRoot = fs::temp_directory_path() / "cuda_newton_sdf_pack_runtime_staging_tests";
    std::error_code ec;
    fs::remove_all(tempRoot, ec);
    const fs::path runtimeDir = tempRoot / "runtime";
    const fs::path repoRoot = tempRoot / "repo_root";
    const fs::path relativePath(entry.relative_path);
    Check(WriteTextFile(runtimeDir / relativePath, validJson),
        "runtime staging priority test writes staged runtime built-in pack");
    Check(WriteTextFile(repoRoot / relativePath, repoRootJson),
        "runtime staging priority test writes conflicting repo-root built-in pack");
    Check(WriteTextFile(runtimeDir / "fractal_ui_repo_root.txt", repoRoot.string()),
        "runtime staging priority test writes repo-root metadata");

    SdfPackViewerState state{};
    std::string error;
    Check(LoadSdfPackViewerBuiltInPack(&state, runtimeDir.string(), entry.pack_id, &error),
        "runtime staged built-in pack loads even when repo-root metadata points at conflicting pack");
    Check(state.have_pack, "runtime staged built-in pack sets have_pack");
    Check(state.pack.pack_id == entry.pack_id, "runtime staged built-in pack keeps expected pack id");
    Check(state.pack_path.find(runtimeDir.string()) != std::string::npos,
        "runtime staged built-in pack path comes from runtime directory before repo-root fallback");

    fs::remove_all(tempRoot, ec);
}

static void TestAllBuiltInPacksHaveSensitiveControls() {
    for (const SdfPackViewerBuiltInPackCatalogEntry& entry : SdfPackViewerBuiltInPackCatalog()) {
        std::string json;
        Check(ReadCatalogPackJson(entry, &json), "built-in pack sensitivity test reads catalog JSON");
        if (json.empty()) continue;
        SdfPackViewerState state{};
        std::string error;
        Check(LoadSdfPackViewerJson(&state, json, entry.relative_path, &error),
            "built-in pack sensitivity test loads pack into viewer state");
        if (!state.have_pack || state.pack.controls.empty()) continue;
        Check(RunSdfPackViewerPreview(&state, &error), "built-in pack sensitivity baseline preview runs");
        const std::string beforeHash = state.last_preview.field_hash;
        const SdfPackControl& control = state.pack.controls.front();
        const SdfPackParam* param = FindPackParamLocal(state.pack, control.param);
        double nextValue = state.params[control.param] + 0.125;
        if (param && std::fabs(state.params[control.param] - param->max_value) > 1e-9) {
            nextValue = param->max_value;
        } else if (param) {
            nextValue = param->min_value;
        }
        Check(SetSdfPackViewerControlValue(&state, SdfPackViewerControlAutomationId(control), nextValue, &error),
            "built-in pack first visible control accepts no-mouse edit");
        Check(RunSdfPackViewerPreview(&state, &error), "built-in pack sensitivity edited preview runs");
        Check(state.last_preview.field_hash != beforeHash,
            "built-in pack first visible control changes the preview field hash");
    }
}
static void TestPackControlsAreVisibleAndEditable() {
    SdfPackViewerState state{};
    std::string error;
    Check(LoadSdfPackViewerJson(&state, kCirclePack, "memory://viewer_circle", &error),
        "pack JSON loads into viewer SDF pack state");
    Check(state.have_pack, "viewer state records loaded pack");
    Check(state.pack.pack_id == "viewer_circle", "pack id preserved");
    Check(!state.use_as_sdf_field_source, "loaded pack does not replace Lens SDF field source by default");
    Check(state.params["radius"] == 0.35, "initial params copied");
    Check(SdfPackViewerControlAutomationId(state.pack.controls[0]) == "sdf_pack.radius.primary",
        "control automation id is derived from SDF pack param");
    Check(SdfPackViewerUseAsSdfFieldSourceAutomationId() == "sdf_pack.use_as_sdf_field_source.primary",
        "field source automation id is stable");

    Check(SetSdfPackViewerControlValue(&state, "sdf_pack.use_as_sdf_field_source.primary", 1.0, &error),
        "set-value automation accepts authored field source toggle");
    Check(state.use_as_sdf_field_source, "authored field source toggle updates state");
    Check(SetSdfPackViewerControlValue(&state, "sdf_pack.radius.primary", 0.55, &error),
        "set-value automation accepts visible SDF pack control");
    Check(std::fabs(state.params["radius"] - 0.55) < 1e-12, "set-value updates the bound SDF pack param");
    Check(state.preview_dirty, "set-value marks preview dirty");

    Check(!SetSdfPackViewerControlValue(&state, "sdf_pack.missing.primary", 0.25, &error),
        "unknown SDF pack control fails");
    Check(error.find("unknown") != std::string::npos, "unknown control error is descriptive");
}

static void TestAutomationReportAndPreviewHashChanges() {
    SdfPackViewerState state{};
    std::string error;
    Check(LoadSdfPackViewerJson(&state, kCirclePack, "memory://viewer_circle", &error),
        "pack JSON loads before report");
    Check(RunSdfPackViewerPreview(&state, &error), "preview runs through field producer");
    SdfPackViewerAutomationReport report = BuildSdfPackViewerAutomationReport(state);
    Check(report.have_pack, "automation report records loaded pack");
    Check(!report.use_as_sdf_field_source, "automation report records default field source toggle");
    Check(report.pack_id == "viewer_circle", "automation report publishes pack id");
    Check(report.controls.size() == 2, "automation report publishes SDF pack controls");
    Check(report.preview_ok, "automation report publishes successful preview");
    Check(report.preview_backend_used == "cpu_reference", "native preview reports CPU reference backend");
    Check(report.preview_width == 32 && report.preview_height == 32, "preview uses bounded default grid");
    Check(!report.preview_field_hash.empty(), "preview report includes deterministic field hash");

    const SdfPackViewerControlReport* radius = FindReportControl(report, "sdf_pack.radius.primary");
    Check(radius != nullptr, "radius control appears in report");
    if (radius) {
        Check(radius->param == "radius", "radius report keeps bound param");
        Check(radius->label == "Radius", "radius report keeps label");
        Check(radius->has_min && std::fabs(radius->min_value - 0.1) < 1e-12, "radius report publishes UI min");
        Check(radius->has_max && std::fabs(radius->max_value - 0.8) < 1e-12, "radius report publishes UI max");
        Check(radius->has_default_value && std::fabs(radius->default_value - 0.35) < 1e-12,
            "radius report publishes default");
    }

    const std::string firstHash = report.preview_field_hash;
    Check(SetSdfPackViewerControlValue(&state, "sdf_pack.radius.primary", 0.65, &error),
        "control edit succeeds before second preview");
    Check(RunSdfPackViewerPreview(&state, &error), "second preview runs");
    report = BuildSdfPackViewerAutomationReport(state);
    Check(report.preview_field_hash != firstHash, "control edit changes field preview hash");
}

static void TestResetDefaultsAndStateJsonRoundTrip() {
    SdfPackViewerState state{};
    std::string error;
    Check(LoadSdfPackViewerJson(&state, kCirclePack, "memory://viewer_circle", &error),
        "pack JSON loads before reset");
    Check(SetSdfPackViewerControlValue(&state, "sdf_pack.radius.primary", 0.7, &error),
        "control edit succeeds before reset");
    Check(ResetSdfPackViewerControlsToDefaults(&state, &error),
        "reset-to-defaults succeeds");
    Check(std::fabs(state.params["radius"] - 0.35) < 1e-12, "reset restores radius default");

    state.open = true;
    state.backend_preference = SdfPackFieldBackend::cpu_reference;
    state.use_as_sdf_field_source = true;
    std::string stateJson = SerializeSdfPackViewerStateJson(state);
    Check(stateJson.find("\"sdf_pack\"") != std::string::npos, "serialized state includes sdf_pack object");
    Check(stateJson.find("\"pack_id\": \"viewer_circle\"") != std::string::npos,
        "serialized state includes explicit pack id marker");
    Check(stateJson.find("\"use_as_sdf_field_source\": true") != std::string::npos,
        "serialized state includes authored field-source authority");
    Check(stateJson.find("\"radius\"") != std::string::npos, "serialized state includes control params");

    SdfPackViewerState restored{};
    Check(LoadSdfPackViewerStateJson(stateJson, &restored, &error), "serialized SDF pack state reloads");
    Check(restored.open, "restored state keeps panel open flag");
    Check(restored.use_as_sdf_field_source, "restored state keeps authored field-source authority");
    Check(restored.have_pack, "restored state reloads pack content");
    Check(restored.pack.pack_id == "viewer_circle", "restored state keeps pack id");
    Check(std::fabs(restored.params["radius"] - 0.35) < 1e-12, "restored state keeps param value");

    SdfPackViewerState oldState{};
    Check(LoadSdfPackViewerStateJson("{}", &oldState, &error), "old state without sdf_pack loads cleanly");
    Check(!oldState.have_pack && !oldState.open, "old state defaults to no SDF pack loaded");
}

static void TestDiagnosticsStateMergeRoundTrip() {
    SdfPackViewerState state{};
    std::string error;
    Check(LoadSdfPackViewerJson(&state, kCirclePack, "memory://viewer_circle", &error),
        "pack JSON loads before diagnostics merge");
    state.open = true;
    state.use_as_sdf_field_source = true;
    Check(SetSdfPackViewerControlValue(&state, "sdf_pack.radius.primary", 0.61, &error),
        "control edit succeeds before diagnostics merge");

    const std::string baseState = R"json({
  "state_version": 3,
  "fractal_type": "julia",
  "view": { "center_x": 0, "center_y": 0, "zoom": 1 },
  "params": { "max_iter": 64 },
  "render": { "width": 32, "height": 32 },
  "stats": { "last_render_ms": 0 }
})json";
    std::string merged;
    Check(MergeSdfPackViewerStateIntoDiagnosticsStateJson(baseState, state, &merged, &error),
        "SDF pack viewer state merges into diagnostics state JSON");
    Check(merged.find("\"sdf_pack\"") != std::string::npos, "merged diagnostics state includes sdf_pack member");

    SdfPackViewerState restored{};
    Check(LoadSdfPackViewerStateJson(merged, &restored, &error), "merged diagnostics state reloads SDF pack viewer state");
    Check(restored.have_pack, "merged diagnostics state restores loaded pack");
    Check(restored.open, "merged diagnostics state restores panel open flag");
    Check(restored.use_as_sdf_field_source, "merged diagnostics state restores field-source authority");
    Check(std::fabs(restored.params["radius"] - 0.61) < 1e-12,
        "merged diagnostics state restores edited SDF pack param");

    std::string unchanged;
    SdfPackViewerState empty{};
    Check(MergeSdfPackViewerStateIntoDiagnosticsStateJson(baseState, empty, &unchanged, &error),
        "empty SDF pack viewer state is accepted by diagnostics merge");
    Check(unchanged == baseState, "empty SDF pack viewer state leaves diagnostics state unchanged");
}

int main() {
    TestBuiltInCatalogEntriesLoadAndLower();
    TestBuiltInSmoothLatticeViewerControls();
    TestDefaultLoadPolicyDoesNotClobberCurrentPack();
    TestRuntimeStagedBuiltInPackWinsOverRepoRootMetadata();
    TestAllBuiltInPacksHaveSensitiveControls();
    TestPackControlsAreVisibleAndEditable();
    TestAutomationReportAndPreviewHashChanges();
    TestResetDefaultsAndStateJsonRoundTrip();
    TestDiagnosticsStateMergeRoundTrip();
    if (g_failed != 0) {
        std::printf("test_sdf_pack_viewer_ui: %d failure(s)\n", g_failed);
        return 1;
    }
    std::printf("test_sdf_pack_viewer_ui: passed=%d\n", g_passed);
    return 0;
}
