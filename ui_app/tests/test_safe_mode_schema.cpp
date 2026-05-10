#include "../src/safe_mode_schema.h"

#include <cstdio>

static int g_passed = 0;
static int g_failed = 0;

static void Check(bool cond, const char* name) {
    if (cond) {
        std::printf("  PASS: %s\n", name);
        ++g_passed;
    } else {
        std::printf("  FAIL: %s\n", name);
        ++g_failed;
    }
}

static const UISchemaPanel* FindPanelById(const UISchema& schema, const char* id) {
    for (const auto& panel : schema.panels) {
        if (panel.id == id) return &panel;
    }
    return nullptr;
}

static const UISchemaControl* FindControlById(const UISchemaPanel& panel, const char* id) {
    for (const auto& control : panel.controls) {
        if (control.id == id) return &control;
    }
    return nullptr;
}

static bool IsExpectedActionButton(const UISchemaPanel& panel, const char* id, const char* bindingPath) {
    const UISchemaControl* control = FindControlById(panel, id);
    return control &&
        control->type == "button" &&
        control->has_binding &&
        control->binding.kind == "action" &&
        control->binding.path == bindingPath;
}

static void TestSafeModeSchemaExposesExpectedPanelsAndActions() {
    UISchema safeMode = BuildSafeModeSchema();
    const UISchemaPanel* viewPanel = FindPanelById(safeMode, "view");
    const UISchemaPanel* fractalPanel = FindPanelById(safeMode, "fractal");
    const UISchemaPanel* renderPanel = FindPanelById(safeMode, "render");

    Check(viewPanel && viewPanel->label == "View (Safe Mode)" && viewPanel->has_order && viewPanel->order == 10 && viewPanel->controls.size() == 11,
        "TestSafeModeSchemaExposesExpectedPanelsAndActions_ViewPanelShape");
    Check(fractalPanel && fractalPanel->label == "Fractal (Safe Mode)" && fractalPanel->has_order && fractalPanel->order == 20 && fractalPanel->controls.size() == 2,
        "TestSafeModeSchemaExposesExpectedPanelsAndActions_FractalPanelShape");
    Check(renderPanel && renderPanel->label == "Render (Safe Mode)" && renderPanel->has_order && renderPanel->order == 30 && renderPanel->controls.size() == 5,
        "TestSafeModeSchemaExposesExpectedPanelsAndActions_RenderPanelShape");

    if (!viewPanel) return;

    Check(IsExpectedActionButton(*viewPanel, "render_once", "fractal.actions.render_once"),
        "TestSafeModeSchemaExposesExpectedPanelsAndActions_RenderOnceBinding");
    Check(IsExpectedActionButton(*viewPanel, "reset_view", "fractal.actions.reset_view"),
        "TestSafeModeSchemaExposesExpectedPanelsAndActions_ResetViewBinding");
    Check(IsExpectedActionButton(*viewPanel, "reset_all", "fractal.actions.reset_all"),
        "TestSafeModeSchemaExposesExpectedPanelsAndActions_ResetAllBinding");
    Check(IsExpectedActionButton(*viewPanel, "load_state", "fractal.actions.load_state"),
        "TestSafeModeSchemaExposesExpectedPanelsAndActions_LoadStateBinding");
    Check(IsExpectedActionButton(*viewPanel, "capture_finding", "fractal.actions.capture_finding"),
        "TestSafeModeSchemaExposesExpectedPanelsAndActions_CaptureFindingBinding");
}

static void TestSafeModeSchemaKeepsGroupedDefaults() {
    UISchema safeMode = BuildSafeModeSchema();
    bool foundRenderWidthDefault = false;
    bool foundRenderHeightDefault = false;
    bool foundFractalTypeCommonGroup = false;
    bool foundFractalTypeRootFindingGroup = false;
    bool foundFractalTypeEscapeTimeGroup = false;
    bool foundFractalTypeExplainoGroup = false;
    bool foundFractalTypeDefaultExplaino = false;
    bool foundContinuousRenderDefaultFalse = false;

    for (const auto& panel : safeMode.panels) {
        for (const auto& ctrl : panel.controls) {
            if (ctrl.id == "fractal_type") {
                if (ctrl.has_default && ctrl.def.is_string() && ctrl.def.as_string() == "explaino") {
                    foundFractalTypeDefaultExplaino = true;
                }
                for (const auto& option : ctrl.options) {
                    if (option.id == "explaino" && option.group == "Common") foundFractalTypeCommonGroup = true;
                    if (option.id == "newton" && option.group == "Root-Finding") foundFractalTypeRootFindingGroup = true;
                    if (option.id == "multibrot" && option.group == "Escape-Time") foundFractalTypeEscapeTimeGroup = true;
                    if (option.id == "explaino_lambda" && option.group == "Explaino") foundFractalTypeExplainoGroup = true;
                }
            }
            if (ctrl.id == "width" && ctrl.has_default && ctrl.def.is_number() && ctrl.def.as_number() == 2048.0) {
                foundRenderWidthDefault = true;
            }
            if (ctrl.id == "height" && ctrl.has_default && ctrl.def.is_number() && ctrl.def.as_number() == 1536.0) {
                foundRenderHeightDefault = true;
            }
            if (ctrl.id == "auto_refresh" && ctrl.label == "Continuous Render" && ctrl.has_default && ctrl.def.is_bool() && !ctrl.def.as_bool()) {
                foundContinuousRenderDefaultFalse = true;
            }
        }
    }

    Check(foundRenderWidthDefault,
        "TestSafeModeSchemaKeepsGroupedDefaults_RenderWidthDefault");
    Check(foundRenderHeightDefault,
        "TestSafeModeSchemaKeepsGroupedDefaults_RenderHeightDefault");
    Check(foundFractalTypeCommonGroup,
        "TestSafeModeSchemaKeepsGroupedDefaults_CommonFractalGroup");
    Check(foundFractalTypeRootFindingGroup,
        "TestSafeModeSchemaKeepsGroupedDefaults_RootFindingGroup");
    Check(foundFractalTypeEscapeTimeGroup,
        "TestSafeModeSchemaKeepsGroupedDefaults_EscapeTimeGroup");
    Check(foundFractalTypeExplainoGroup,
        "TestSafeModeSchemaKeepsGroupedDefaults_ExplainoGroup");
    Check(foundFractalTypeDefaultExplaino,
        "TestSafeModeSchemaKeepsGroupedDefaults_DefaultExplaino");
    Check(foundContinuousRenderDefaultFalse,
        "TestSafeModeSchemaKeepsGroupedDefaults_ContinuousRenderDisabled");
}

int main() {
    TestSafeModeSchemaExposesExpectedPanelsAndActions();
    TestSafeModeSchemaKeepsGroupedDefaults();

    std::printf("test_safe_mode_schema: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}