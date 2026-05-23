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

static bool IsMagnetVisibleControl(const UISchemaControl* control, const char* bindingPath) {
    return control &&
        control->has_binding &&
        control->binding.kind == "param" &&
        control->binding.path == bindingPath &&
        control->has_visible_if &&
        control->visible_if.op == "eq" &&
        control->visible_if.path == "fractal.view.fractal_type" &&
        control->visible_if.value == "magnet";
}

static bool IsMultibrotVisibleControl(const UISchemaControl* control, const char* bindingPath) {
    return control &&
        control->has_binding &&
        control->binding.kind == "param" &&
        control->binding.path == bindingPath &&
        control->has_visible_if &&
        control->visible_if.op == "eq" &&
        control->visible_if.path == "fractal.view.fractal_type" &&
        control->visible_if.value == "multibrot";
}

static bool IsJuliaVisibleControl(const UISchemaControl* control, const char* bindingPath) {
    return control &&
        control->has_binding &&
        control->binding.kind == "param" &&
        control->binding.path == bindingPath &&
        control->has_visible_if &&
        control->visible_if.op == "eq" &&
        control->visible_if.path == "fractal.view.fractal_type" &&
        control->visible_if.value == "julia";
}

static bool IsPhoenixVisibleControl(const UISchemaControl* control, const char* bindingPath) {
    return control &&
        control->has_binding &&
        control->binding.kind == "param" &&
        control->binding.path == bindingPath &&
        control->has_visible_if &&
        control->visible_if.op == "in" &&
        control->visible_if.path == "fractal.view.fractal_type" &&
        ContainsCsvToken(control->visible_if.value, "phoenix") &&
        ContainsCsvToken(control->visible_if.value, "explaino_phoenix") &&
        !ContainsCsvToken(control->visible_if.value, "explaino_all");
}

static bool IsProjectionAndFlowVisibleControl(const UISchemaControl* control, const char* bindingPath) {
    return control &&
        control->has_binding &&
        control->binding.kind == "param" &&
        control->binding.path == bindingPath &&
        control->has_visible_if &&
        control->visible_if.op == "in" &&
        control->visible_if.path == "fractal.view.fractal_type" &&
        ContainsCsvToken(control->visible_if.value, "projection_and_flow") &&
        ContainsCsvToken(control->visible_if.value, "explaino_projection_and_flow");
}

static void TestSafeModeSchemaExposesExpectedPanelsAndActions() {
    UISchema safeMode = BuildSafeModeSchema();
    const UISchemaPanel* viewPanel = FindPanelById(safeMode, "view");
    const UISchemaPanel* fractalPanel = FindPanelById(safeMode, "fractal");
    const UISchemaPanel* renderPanel = FindPanelById(safeMode, "render");

    Check(viewPanel && viewPanel->label == "View (Safe Mode)" && viewPanel->has_order && viewPanel->order == 10 && viewPanel->controls.size() == 11,
        "TestSafeModeSchemaExposesExpectedPanelsAndActions_ViewPanelShape");
    Check(fractalPanel && fractalPanel->label == "Fractal (Safe Mode)" && fractalPanel->has_order && fractalPanel->order == 20 && fractalPanel->controls.size() == 23,
        "TestSafeModeSchemaExposesExpectedPanelsAndActions_FractalPanelShape");
    Check(renderPanel && renderPanel->label == "Render (Safe Mode)" && renderPanel->has_order && renderPanel->order == 30 && renderPanel->controls.size() == 7,
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
    bool foundResolutionAspectPresetDefault = false;
    bool foundResolutionLongEdgeDefault = false;
    bool foundFractalTypeCommonGroup = false;
    bool foundFractalTypeRootFindingGroup = false;
    bool foundCounterfactualPairRootFindingGroup = false;
    bool foundProjectionAndFlowRootFindingGroup = false;
    bool foundFractalTypeEscapeTimeGroup = false;
    bool foundFractalTypeExplainoGroup = false;
    bool foundExplainoProjectionAndFlowGroup = false;
    bool foundFractalTypeDefaultExplainoAll = false;
    bool foundContinuousRenderDefaultFalse = false;
    bool foundMagnetEscapeTimeGroup = false;

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
                    if (option.id == "magnet" && option.group == "Escape-Time") foundMagnetEscapeTimeGroup = true;
                    if (option.id == "explaino_lambda" && option.group == "Explaino") foundFractalTypeExplainoGroup = true;
                    if (option.id == "explaino_projection_and_flow" && option.group == "Explaino") foundExplainoProjectionAndFlowGroup = true;
                }
            }
            if (ctrl.id == "resolution_aspect_preset" && ctrl.type == "combo" && ctrl.has_binding &&
                ctrl.binding.path == "fractal.render.resolution.aspect_preset" &&
                ctrl.has_default && ctrl.def.is_string() && ctrl.def.as_string() == "4:3" &&
                ctrl.options.size() == 6 && ctrl.options[0].id == "custom" && ctrl.options[1].id == "1:1" &&
                ctrl.options[2].id == "4:3" && ctrl.options[3].id == "16:9" &&
                ctrl.options[4].id == "16:10" && ctrl.options[5].id == "21:9") {
                foundResolutionAspectPresetDefault = true;
            }
            if (ctrl.id == "resolution_long_edge" && ctrl.type == "slider_int" && ctrl.has_binding &&
                ctrl.binding.path == "fractal.render.resolution.long_edge" &&
                ctrl.has_min && ctrl.min == 256.0 && ctrl.has_max && ctrl.max == 4096.0 &&
                ctrl.has_step && ctrl.step == 16.0 && ctrl.has_default && ctrl.def.is_number() && ctrl.def.as_number() == 2048.0) {
                foundResolutionLongEdgeDefault = true;
            }
            if (ctrl.id == "width" && ctrl.has_default && ctrl.def.is_number() && ctrl.def.as_number() == 2048.0 &&
                ctrl.has_visible_if && ctrl.visible_if.op == "eq" && ctrl.visible_if.path == "fractal.render.resolution.aspect_preset" && ctrl.visible_if.value == "custom") {
                foundRenderWidthDefault = true;
            }
            if (ctrl.id == "height" && ctrl.has_default && ctrl.def.is_number() && ctrl.def.as_number() == 1536.0 &&
                ctrl.has_visible_if && ctrl.visible_if.op == "eq" && ctrl.visible_if.path == "fractal.render.resolution.aspect_preset" && ctrl.visible_if.value == "custom") {
                foundRenderHeightDefault = true;
            }
            if (ctrl.id == "auto_refresh" && ctrl.label == "Continuous Render" && ctrl.has_default && ctrl.def.is_bool() && !ctrl.def.as_bool()) {
                foundContinuousRenderDefaultFalse = true;
            }
        }
    }

    Check(foundResolutionAspectPresetDefault,
        "TestSafeModeSchemaKeepsGroupedDefaults_ResolutionAspectPresetDefault");
    Check(foundResolutionLongEdgeDefault,
        "TestSafeModeSchemaKeepsGroupedDefaults_ResolutionLongEdgeDefault");
    Check(foundRenderWidthDefault,
        "TestSafeModeSchemaKeepsGroupedDefaults_RenderWidthCustomOnly");
    Check(foundRenderHeightDefault,
        "TestSafeModeSchemaKeepsGroupedDefaults_RenderHeightCustomOnly");
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
    Check(foundMagnetEscapeTimeGroup,
        "TestSafeModeSchemaKeepsGroupedDefaults_MagnetEscapeTimeGroup");
    Check(foundFractalTypeExplainoGroup,
        "TestSafeModeSchemaKeepsGroupedDefaults_ExplainoGroup");
    Check(foundExplainoProjectionAndFlowGroup,
        "TestSafeModeSchemaKeepsGroupedDefaults_ExplainoProjectionAndFlowGroup");
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

    Check(IsCounterfactualPairCarrierVisibleControl(rootFamily, "fractal.params.counterfactual_pair_root_family") &&
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

static void TestSafeModeSchemaExposesMagnetControls() {
    UISchema safeMode = BuildSafeModeSchema();
    const UISchemaPanel* fractalPanel = FindPanelById(safeMode, "fractal");
    Check(fractalPanel != nullptr, "TestSafeModeSchemaExposesMagnetControls_FractalPanelPresent");
    if (!fractalPanel) return;
    const UISchemaControl* seedReal = FindControlById(*fractalPanel, "magnet_seed_real");
    const UISchemaControl* seedImag = FindControlById(*fractalPanel, "magnet_seed_imag");
    const UISchemaControl* relaxation = FindControlById(*fractalPanel, "magnet_relaxation");
    const UISchemaControl* bailout = FindControlById(*fractalPanel, "magnet_bailout");
    Check(IsMagnetVisibleControl(seedReal, "fractal.params.magnet_seed_real"), "TestSafeModeSchemaExposesMagnetControls_SeedReal");
    Check(IsMagnetVisibleControl(seedImag, "fractal.params.magnet_seed_imag"), "TestSafeModeSchemaExposesMagnetControls_SeedImag");
    Check(IsMagnetVisibleControl(relaxation, "fractal.params.magnet_relaxation") && relaxation->has_ui_min && relaxation->ui_min == 0.05 && relaxation->has_ui_max && relaxation->ui_max == 1.5, "TestSafeModeSchemaExposesMagnetControls_Relaxation");
    Check(IsMagnetVisibleControl(bailout, "fractal.params.magnet_bailout") && bailout->has_min && bailout->min == 2.0 && bailout->has_max && bailout->max == 64.0, "TestSafeModeSchemaExposesMagnetControls_Bailout");
}

static void TestSafeModeSchemaExposesMultibrotControls() {
    UISchema safeMode = BuildSafeModeSchema();
    const UISchemaPanel* fractalPanel = FindPanelById(safeMode, "fractal");
    Check(fractalPanel != nullptr, "TestSafeModeSchemaExposesMultibrotControls_FractalPanelPresent");
    if (!fractalPanel) return;

    const UISchemaControl* real = FindControlById(*fractalPanel, "multibrot_power_float");
    const UISchemaControl* imag = FindControlById(*fractalPanel, "multibrot_power_imag");
    Check(IsMultibrotVisibleControl(real, "fractal.params.multibrot_power_float") &&
            real->has_min && real->min == 0.01 && real->has_max && real->max == 32.0 &&
            real->has_ui_min && real->ui_min == 0.01 && real->has_ui_max && real->ui_max == 12.0 &&
            real->logarithmic,
        "TestSafeModeSchemaExposesMultibrotControls_Real");
    Check(IsMultibrotVisibleControl(imag, "fractal.params.multibrot_power_imag") &&
            imag->has_min && imag->min == -4.0 && imag->has_max && imag->max == 4.0 &&
            imag->has_default && imag->def.is_number() && imag->def.as_number() == 0.0,
        "TestSafeModeSchemaExposesMultibrotControls_Imag");
}

static void TestSafeModeSchemaExposesParameterFunctionalityBatch1Controls() {
    UISchema safeMode = BuildSafeModeSchema();
    const UISchemaPanel* fractalPanel = FindPanelById(safeMode, "fractal");
    Check(fractalPanel != nullptr, "TestSafeModeSchemaExposesParameterFunctionalityBatch1Controls_FractalPanelPresent");
    if (!fractalPanel) return;

    const UISchemaControl* spiderFeedback = FindControlById(*fractalPanel, "spider_feedback");
    const UISchemaControl* rationalDenominatorPower = FindControlById(*fractalPanel, "explaino_rational_escape_denominator_power");
    const UISchemaControl* collatzTransitionStrength = FindControlById(*fractalPanel, "collatz_transition_strength");
    Check(spiderFeedback != nullptr &&
            spiderFeedback->has_binding &&
            spiderFeedback->binding.path == "fractal.params.spider_feedback" &&
            spiderFeedback->has_visible_if &&
            spiderFeedback->visible_if.value == "spider" &&
            spiderFeedback->has_min && spiderFeedback->min == -2.0 &&
            spiderFeedback->has_max && spiderFeedback->max == 2.0 &&
            spiderFeedback->has_ui_min && spiderFeedback->ui_min == 0.0 &&
            spiderFeedback->has_ui_max && spiderFeedback->ui_max == 1.0 &&
            spiderFeedback->has_default &&
            spiderFeedback->def.is_number() &&
            spiderFeedback->def.as_number() == 0.5,
        "TestSafeModeSchemaExposesParameterFunctionalityBatch1Controls_SpiderFeedback");
    Check(rationalDenominatorPower != nullptr &&
            rationalDenominatorPower->has_binding &&
            rationalDenominatorPower->binding.path == "fractal.params.explaino_rational_escape_denominator_power" &&
            rationalDenominatorPower->has_visible_if &&
            rationalDenominatorPower->visible_if.value == "explaino_rational_escape" &&
            rationalDenominatorPower->has_min && rationalDenominatorPower->min == 1.0 &&
            rationalDenominatorPower->has_max && rationalDenominatorPower->max == 6.0 &&
            rationalDenominatorPower->has_default &&
            rationalDenominatorPower->def.is_number() &&
            rationalDenominatorPower->def.as_number() == 3.0,
        "TestSafeModeSchemaExposesParameterFunctionalityBatch1Controls_RationalDenominatorPower");
    Check(collatzTransitionStrength != nullptr &&
            collatzTransitionStrength->has_binding &&
            collatzTransitionStrength->binding.path == "fractal.params.collatz_transition_strength" &&
            collatzTransitionStrength->has_visible_if &&
            collatzTransitionStrength->visible_if.value == "collatz" &&
            collatzTransitionStrength->has_min && collatzTransitionStrength->min == 0.0 &&
            collatzTransitionStrength->has_max && collatzTransitionStrength->max == 4.0 &&
            collatzTransitionStrength->has_ui_min && collatzTransitionStrength->ui_min == 0.0 &&
            collatzTransitionStrength->has_ui_max && collatzTransitionStrength->ui_max == 2.0 &&
            collatzTransitionStrength->has_default &&
            collatzTransitionStrength->def.is_number() &&
            collatzTransitionStrength->def.as_number() == 1.0,
        "TestSafeModeSchemaExposesParameterFunctionalityBatch1Controls_CollatzTransitionStrength");
}

static void TestSafeModeSchemaExposesJuliaControls() {
    UISchema safeMode = BuildSafeModeSchema();
    const UISchemaPanel* fractalPanel = FindPanelById(safeMode, "fractal");
    Check(fractalPanel != nullptr, "TestSafeModeSchemaExposesJuliaControls_FractalPanelPresent");
    if (!fractalPanel) return;

    const UISchemaControl* real = FindControlById(*fractalPanel, "julia_c_real");
    const UISchemaControl* imag = FindControlById(*fractalPanel, "julia_c_imag");
    Check(IsJuliaVisibleControl(real, "fractal.params.julia_c_real") &&
            real->has_default && real->def.is_number() && real->def.as_number() == -0.7 &&
            real->has_ui_min && real->ui_min == -2.0 && real->has_ui_max && real->ui_max == 2.0,
        "TestSafeModeSchemaExposesJuliaControls_Real");
    Check(IsJuliaVisibleControl(imag, "fractal.params.julia_c_imag") &&
            imag->has_default && imag->def.is_number() && imag->def.as_number() == 0.27015 &&
            imag->has_ui_min && imag->ui_min == -2.0 && imag->has_ui_max && imag->ui_max == 2.0,
        "TestSafeModeSchemaExposesJuliaControls_Imag");
}

static void TestSafeModeSchemaExposesPhoenixControls() {
    UISchema safeMode = BuildSafeModeSchema();
    const UISchemaPanel* fractalPanel = FindPanelById(safeMode, "fractal");
    Check(fractalPanel != nullptr, "TestSafeModeSchemaExposesPhoenixControls_FractalPanelPresent");
    if (!fractalPanel) return;

    const UISchemaControl* real = FindControlById(*fractalPanel, "phoenix_p_real");
    const UISchemaControl* imag = FindControlById(*fractalPanel, "phoenix_p_imag");
    Check(IsPhoenixVisibleControl(real, "fractal.params.phoenix_p_real") &&
            real->has_min && real->min == -1.0 && real->has_max && real->max == 1.0 &&
            real->has_default && real->def.is_number() && real->def.as_number() == 0.0,
        "TestSafeModeSchemaExposesPhoenixControls_Real");
    Check(IsPhoenixVisibleControl(imag, "fractal.params.phoenix_p_imag") &&
            imag->has_min && imag->min == -1.0 && imag->has_max && imag->max == 1.0 &&
            imag->has_default && imag->def.is_number() && imag->def.as_number() == 0.0,
        "TestSafeModeSchemaExposesPhoenixControls_Imag");
}

static void TestSafeModeSchemaExposesProjectionAndFlowControls() {
    UISchema safeMode = BuildSafeModeSchema();
    const UISchemaPanel* fractalPanel = FindPanelById(safeMode, "fractal");
    Check(fractalPanel != nullptr, "TestSafeModeSchemaExposesProjectionAndFlowControls_FractalPanelPresent");
    if (!fractalPanel) {
        return;
    }

    const UISchemaControl* rootFamily = FindControlById(*fractalPanel, "projection_and_flow_root_family");
    const UISchemaControl* targetRadius = FindControlById(*fractalPanel, "projection_and_flow_target_radius");
    const UISchemaControl* pressureThreshold = FindControlById(*fractalPanel, "projection_and_flow_pressure_threshold");

    Check(IsProjectionAndFlowVisibleControl(rootFamily, "fractal.params.projection_and_flow_root_family") &&
            rootFamily->has_default && rootFamily->def.is_string() && rootFamily->def.as_string() == "cubic_unit_roots" &&
            rootFamily->options.size() == 2,
        "TestSafeModeSchemaExposesProjectionAndFlowControls_RootFamily");
    Check(IsProjectionAndFlowVisibleControl(targetRadius, "fractal.params.projection_and_flow_target_radius") &&
            targetRadius->has_default && targetRadius->def.is_number() && targetRadius->def.as_number() == 1.0,
        "TestSafeModeSchemaExposesProjectionAndFlowControls_TargetRadius");
    Check(IsProjectionAndFlowVisibleControl(pressureThreshold, "fractal.params.projection_and_flow_pressure_threshold") &&
            pressureThreshold->has_default && pressureThreshold->def.is_number() && pressureThreshold->def.as_number() == 1.0 &&
            pressureThreshold->has_help && pressureThreshold->help.find("transient-pressure band") != std::string::npos &&
            pressureThreshold->help.find("Band 3") != std::string::npos &&
            pressureThreshold->help.find("unstable") != std::string::npos,
        "TestSafeModeSchemaExposesProjectionAndFlowControls_PressureThreshold");
}

int main() {
    TestSafeModeSchemaExposesExpectedPanelsAndActions();
    TestSafeModeSchemaKeepsGroupedDefaults();
    TestSafeModeSchemaExposesCounterfactualPairControls();
    TestSafeModeSchemaExposesMagnetControls();
    TestSafeModeSchemaExposesMultibrotControls();
    TestSafeModeSchemaExposesParameterFunctionalityBatch1Controls();
    TestSafeModeSchemaExposesJuliaControls();
    TestSafeModeSchemaExposesPhoenixControls();
    TestSafeModeSchemaExposesProjectionAndFlowControls();

    std::printf("test_safe_mode_schema: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}
