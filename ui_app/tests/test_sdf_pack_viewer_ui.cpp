// Native tests for authored SDF pack viewer UI state, controls, and preview proof.

#include "sdf_pack_viewer_ui.h"

#include <cmath>
#include <cstdio>
#include <string>

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

static void TestPackControlsAreVisibleAndEditable() {
    SdfPackViewerState state{};
    std::string error;
    Check(LoadSdfPackViewerJson(&state, kCirclePack, "memory://viewer_circle", &error),
        "pack JSON loads into viewer SDF pack state");
    Check(state.have_pack, "viewer state records loaded pack");
    Check(state.pack.pack_id == "viewer_circle", "pack id preserved");
    Check(state.params["radius"] == 0.35, "initial params copied");
    Check(SdfPackViewerControlAutomationId(state.pack.controls[0]) == "sdf_pack.radius.primary",
        "control automation id is derived from SDF pack param");

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
    std::string stateJson = SerializeSdfPackViewerStateJson(state);
    Check(stateJson.find("\"sdf_pack\"") != std::string::npos, "serialized state includes sdf_pack object");
    Check(stateJson.find("\"radius\"") != std::string::npos, "serialized state includes control params");

    SdfPackViewerState restored{};
    Check(LoadSdfPackViewerStateJson(stateJson, &restored, &error), "serialized SDF pack state reloads");
    Check(restored.open, "restored state keeps panel open flag");
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
    Check(std::fabs(restored.params["radius"] - 0.61) < 1e-12,
        "merged diagnostics state restores edited SDF pack param");

    std::string unchanged;
    SdfPackViewerState empty{};
    Check(MergeSdfPackViewerStateIntoDiagnosticsStateJson(baseState, empty, &unchanged, &error),
        "empty SDF pack viewer state is accepted by diagnostics merge");
    Check(unchanged == baseState, "empty SDF pack viewer state leaves diagnostics state unchanged");
}

int main() {
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
