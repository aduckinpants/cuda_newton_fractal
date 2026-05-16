#include "../src/safe_mode_schema.h"

#include <cstdio>
#include <string>
#include <string_view>

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

static bool ContainsCsvToken(std::string_view csv, std::string_view token) {
    std::size_t start = 0;
    while (start <= csv.size()) {
        const std::size_t comma = csv.find(',', start);
        const std::size_t end = (comma == std::string_view::npos) ? csv.size() : comma;
        if (csv.substr(start, end - start) == token) {
            return true;
        }
        if (comma == std::string_view::npos) {
            break;
        }
        start = comma + 1;
    }
    return false;
}

static bool IsCounterfactualPairVisibleControl(const UISchemaControl* control, const char* bindingPath) {
    return control &&
        control->has_binding &&
        control->binding.kind == "param" &&
        control->binding.path == bindingPath &&
        control->has_visible_if &&
        control->visible_if.op == "eq" &&
        control->visible_if.path == "fractal.view.fractal_type" &&
        control->visible_if.value == "counterfactual_pair";
}

static bool IsCounterfactualPairCarrierVisibleControl(const UISchemaControl* control, const char* bindingPath) {
    return control &&
        control->has_binding &&
        control->binding.kind == "param" &&
        control->binding.path == bindingPath &&
        control->has_visible_if &&
        control->visible_if.op == "in" &&
        control->visible_if.path == "fractal.view.fractal_type" &&
        ContainsCsvToken(control->visible_if.value, "counterfactual_pair") &&
        ContainsCsvToken(control->visible_if.value, "explaino_counterfactual_pair");
}

static void TestSafeModeSchemaExposesExpectedPanelsAndActions() {
    UISchema safeMode = BuildSafeModeSchema();
    const UISchemaPanel* viewPanel = FindPanelById(safeMode, "view");
    const UISchemaPanel* fractalPanel = FindPanelById(safeMode, "fractal");
    const UISchemaPanel* renderPanel = FindPanelById(safeMode, "render");

    Check(viewPanel && viewPanel->label == "View (Safe Mode)" && viewPanel->has_order && viewPanel->order == 10 && viewPanel->controls.size() == 11,
        "TestSafeModeSchemaExposesExpectedPanelsAndActions_ViewPanelShape");
    Check(fractalPanel && fractalPanel->label == "Fractal (Safe Mode)" && fractalPanel->has_order && fractalPanel->order == 20 && fractalPanel->controls.size() == 7,
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
    bool foundCounterfactualPairRootFindingGroup = false;
    bool foundProjectionAndFlowRootFindingGroup = false;
    bool foundFractalTypeEscapeTimeGroup = false;
    bool foundFractalTypeExplainoGroup = false;
    bool foundFractalTypeDefaultExplainoAll = false;
    bool foundContinuousRenderDefaultFalse = false;

    for (const auto& panel : safeMode.panels) {
        for (const auto& ctrl : panel.controls) {
            if (ctrl.id == "fractal_type") {
                if (ctrl.has_default && ctrl.def.is_string() && ctrl.def.as_string() == "explaino_all") {
                    foundFractalTypeDefaultExplainoAll = true;
                }
                for (const auto& option : ctrl.options) {
                    if (option.id == "explaino" && option.group == "Common") foundFractalTypeCommonGroup = true;
                    if (option.id == "newton" && option.group == "Root-Finding") foundFractalTypeRootFindingGroup = true;
                    if (option.id == "counterfactual_pair" && option.group == "Root-Finding") foundCounterfactualPairRootFindingGroup = true;
                    if (option.id == "projection_and_flow" && option.group == "Root-Finding") foundProjectionAndFlowRootFindingGroup = true;
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
    Check(foundCounterfactualPairRootFindingGroup,
        "TestSafeModeSchemaKeepsGroupedDefaults_CounterfactualPairRootFindingGroup");
    Check(foundProjectionAndFlowRootFindingGroup,
        "TestSafeModeSchemaKeepsGroupedDefaults_ProjectionAndFlowRootFindingGroup");
    Check(foundFractalTypeEscapeTimeGroup,
        "TestSafeModeSchemaKeepsGroupedDefaults_EscapeTimeGroup");
    Check(foundFractalTypeExplainoGroup,
        "TestSafeModeSchemaKeepsGroupedDefaults_ExplainoGroup");
    Check(foundFractalTypeDefaultExplainoAll,
        "TestSafeModeSchemaKeepsGroupedDefaults_DefaultExplainoAll");
    Check(foundContinuousRenderDefaultFalse,
        "TestSafeModeSchemaKeepsGroupedDefaults_ContinuousRenderDisabled");
}

static void TestSafeModeSchemaExposesCounterfactualPairControls() {
    UISchema safeMode = BuildSafeModeSchema();
    const UISchemaPanel* fractalPanel = FindPanelById(safeMode, "fractal");
    Check(fractalPanel != nullptr, "TestSafeModeSchemaExposesCounterfactualPairControls_FractalPanelPresent");
    if (!fractalPanel) {
        return;
    }

    const UISchemaControl* rootFamily = FindControlById(*fractalPanel, "counterfactual_pair_root_family");
    const UISchemaControl* frame = FindControlById(*fractalPanel, "counterfactual_pair_frame");
    const UISchemaControl* offsetX = FindControlById(*fractalPanel, "counterfactual_pair_offset_x");
    const UISchemaControl* offsetY = FindControlById(*fractalPanel, "counterfactual_pair_offset_y");
    const UISchemaControl* reconvergenceRatio = FindControlById(*fractalPanel, "counterfactual_pair_reconvergence_ratio");

    Check(IsCounterfactualPairVisibleControl(rootFamily, "fractal.params.counterfactual_pair_root_family") &&
            rootFamily->has_default && rootFamily->def.is_string() && rootFamily->def.as_string() == "cubic_unit_roots" &&
            rootFamily->options.size() == 2,
        "TestSafeModeSchemaExposesCounterfactualPairControls_RootFamily");
    Check(IsCounterfactualPairCarrierVisibleControl(frame, "fractal.params.counterfactual_pair_frame") &&
            frame->has_default && frame->def.is_string() && frame->def.as_string() == "world_absolute" &&
            frame->options.size() == 2,
        "TestSafeModeSchemaExposesCounterfactualPairControls_Frame");
    Check(IsCounterfactualPairCarrierVisibleControl(offsetX, "fractal.params.counterfactual_pair_offset_x") &&
            offsetX->has_default && offsetX->def.is_number() && offsetX->def.as_number() == 0.16,
        "TestSafeModeSchemaExposesCounterfactualPairControls_OffsetX");
    Check(IsCounterfactualPairCarrierVisibleControl(offsetY, "fractal.params.counterfactual_pair_offset_y") &&
            offsetY->has_default && offsetY->def.is_number() && offsetY->def.as_number() == 0.08,
        "TestSafeModeSchemaExposesCounterfactualPairControls_OffsetY");
    Check(IsCounterfactualPairCarrierVisibleControl(reconvergenceRatio, "fractal.params.counterfactual_pair_reconvergence_ratio") &&
            reconvergenceRatio->has_default && reconvergenceRatio->def.is_number() && reconvergenceRatio->def.as_number() == 0.60 &&
            reconvergenceRatio->has_help && reconvergenceRatio->help.find("Class 2") != std::string::npos,
        "TestSafeModeSchemaExposesCounterfactualPairControls_ReconvergenceRatio");
}

int main() {
    TestSafeModeSchemaExposesExpectedPanelsAndActions();
    TestSafeModeSchemaKeepsGroupedDefaults();
    TestSafeModeSchemaExposesCounterfactualPairControls();

    std::printf("test_safe_mode_schema: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}
