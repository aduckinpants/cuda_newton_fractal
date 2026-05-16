#include "../src/ui_schema.h"
#include "../src/fractal_family_rules.h"
#include "../src/enum_id_utils.h"
#include "../src/json_min.h"
#include "../src/safe_mode_schema.h"
#include "../src/schema_binding.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>
#include <vector>

namespace {

std::filesystem::path FindRepoRoot() {
    std::filesystem::path cur = std::filesystem::current_path();
    for (int index = 0; index < 8; ++index) {
        if (std::filesystem::exists(cur / "ui" / "fractal_binding_surface_v1.ui_schema.json") &&
            std::filesystem::exists(cur / "ui_app" / "src" / "ui_schema.cpp")) {
            return cur;
        }
        if (!cur.has_parent_path() || cur.parent_path() == cur) break;
        cur = cur.parent_path();
    }
    return {};
}

bool ReadTextFile(const std::filesystem::path& path, std::string* outText) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) return false;
    std::ostringstream ss;
    ss << file.rdbuf();
    *outText = ss.str();
    return true;
}

bool LoadAndValidateSchemaFile(const std::filesystem::path& path) {
    std::string text;
    if (!ReadTextFile(path, &text)) {
        std::cerr << "Failed to read schema file: " << path.string() << "\n";
        return false;
    }

    json_min::ParseResult pr = json_min::Parse(text);
    if (!pr.error.empty()) {
        std::cerr << "Schema JSON parse failed for " << path.string() << ": " << pr.error << "\n";
        return false;
    }

    UISchemaLoadResult result = LoadUISchemaFromJson(pr.value);
    if (!result.error.empty()) {
        std::cerr << "Schema decode failed for " << path.string() << ": " << result.error << "\n";
        return false;
    }

    bool foundAutoMaxIter = false;
    bool foundMcMullenPreset = false;
    for (const auto& panel : result.schema.panels) {
        for (const auto& ctrl : panel.controls) {
            if (ctrl.id == "auto_max_iter" && ctrl.has_binding && ctrl.binding.path == "fractal.view.auto_max_iter") {
                foundAutoMaxIter = true;
            }
            if (ctrl.id == "mcmullen_preset" && ctrl.has_binding && ctrl.binding.path == "fractal.params.mcmullen_preset") {
                foundMcMullenPreset = true;
            }
        }
    }
    if (!foundAutoMaxIter) {
        std::cerr << "Schema missing auto_max_iter binding in " << path.string() << "\n";
        return false;
    }
    if (!foundMcMullenPreset) {
        std::cerr << "Schema missing mcmullen_preset binding in " << path.string() << "\n";
        return false;
    }

    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    LensSettings lens{};
    BindingContext ctx;
    ctx.view = &view;
    ctx.params = &params;
    ctx.render = &render;
    ctx.lens = &lens;

    std::string bindError;
    if (!ValidateSchemaBindings(result.schema, ctx, &bindError)) {
        std::cerr << "Schema binding validation failed for " << path.string() << ": " << bindError << "\n";
        return false;
    }
    return true;
}

const UISchemaPanel* FindPanelById(const UISchema& schema, const char* id) {
    for (const auto& panel : schema.panels) {
        if (panel.id == id) {
            return &panel;
        }
    }
    return nullptr;
}

const UISchemaControl* FindControlById(const UISchemaPanel& panel, const char* id) {
    for (const auto& control : panel.controls) {
        if (control.id == id) {
            return &control;
        }
    }
    return nullptr;
}

bool ContainsCsvToken(std::string_view csv, std::string_view token) {
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

bool VisibleIfIncludesFractalType(const UISchemaControl& control, std::string_view fractalTypeId) {
    return control.has_visible_if &&
        control.visible_if.path == "fractal.view.fractal_type" &&
        (control.visible_if.op == "eq" || control.visible_if.op == "in") &&
        ContainsCsvToken(control.visible_if.value, fractalTypeId);
}

std::size_t CsvTokenCount(std::string_view csv) {
    if (csv.empty()) {
        return 0;
    }
    std::size_t count = 0;
    std::size_t start = 0;
    while (start <= csv.size()) {
        const std::size_t comma = csv.find(',', start);
        ++count;
        if (comma == std::string_view::npos) {
            break;
        }
        start = comma + 1;
    }
    return count;
}

inline constexpr const char* kKnownExplainoAllVisibleBindingPaths[] = {
    "fractal.view.auto_increment_seed",
    "fractal.params.epsilon",
    "fractal.params.explaino_seed",
    "fractal.actions.prev_seed",
    "fractal.actions.next_seed",
    "fractal.params.explaino_warp_strength",
    "fractal.params.explaino_root_spread",
    "fractal.view.explaino_phase_strength",
    "fractal.params.explaino_damping",
    "fractal.view.explaino_phase",
    "fractal.view.explaino_seed_drift",
    "fractal.view.explaino_seed_tween",
};

inline constexpr std::size_t kKnownExplainoAllVisibleBindingPathCount =
    sizeof(kKnownExplainoAllVisibleBindingPaths) / sizeof(kKnownExplainoAllVisibleBindingPaths[0]);

inline constexpr std::size_t kExpectedExplainoGroupedSelectorCount =
    (sizeof(kExplainoSelectorRegistry) / sizeof(kExplainoSelectorRegistry[0])) - 1u;

bool IsKnownExplainoAllVisibleBindingPath(std::string_view bindingPath) {
    for (const char* knownPath : kKnownExplainoAllVisibleBindingPaths) {
        if (bindingPath == knownPath) {
            return true;
        }
    }
    return false;
}

bool ValidateCounterfactualPairControlSurface(const UISchema& schema) {
    const UISchemaPanel* fractalPanel = FindPanelById(schema, "fractal");
    if (!fractalPanel) {
        std::cerr << "Main schema missing fractal panel for Counterfactual Pair control validation\n";
        return false;
    }

    const UISchemaControl* rootFamily = FindControlById(*fractalPanel, "counterfactual_pair_root_family");
    const UISchemaControl* frame = FindControlById(*fractalPanel, "counterfactual_pair_frame");
    const UISchemaControl* offsetX = FindControlById(*fractalPanel, "counterfactual_pair_offset_x");
    const UISchemaControl* offsetY = FindControlById(*fractalPanel, "counterfactual_pair_offset_y");
    const UISchemaControl* reconvergenceRatio = FindControlById(*fractalPanel, "counterfactual_pair_reconvergence_ratio");
    if (!rootFamily || !frame || !offsetX || !offsetY || !reconvergenceRatio) {
        std::cerr << "Main schema is missing one or more Counterfactual Pair controls\n";
        return false;
    }

    const bool rootFamilyOk =
        rootFamily->has_binding &&
        rootFamily->binding.path == "fractal.params.counterfactual_pair_root_family" &&
        rootFamily->has_default &&
        rootFamily->def.is_string() &&
        rootFamily->def.as_string() == "cubic_unit_roots" &&
        rootFamily->options.size() == 2 &&
        rootFamily->has_visible_if &&
        rootFamily->visible_if.op == "eq" &&
        rootFamily->visible_if.path == "fractal.view.fractal_type" &&
        rootFamily->visible_if.value == "counterfactual_pair";
    if (!rootFamilyOk) {
        std::cerr << "Main schema Counterfactual Pair root-family control drifted from the bounded owner seam\n";
        return false;
    }

    const bool frameOk =
        frame->has_binding &&
        frame->binding.path == "fractal.params.counterfactual_pair_frame" &&
        frame->has_default &&
        frame->def.is_string() &&
        frame->def.as_string() == "world_absolute" &&
        frame->options.size() == 2 &&
        frame->has_help &&
        frame->help.find("World Absolute") != std::string::npos &&
        frame->help.find("View Relative") != std::string::npos &&
        frame->has_visible_if &&
        frame->visible_if.op == "eq" &&
        frame->visible_if.path == "fractal.view.fractal_type" &&
        frame->visible_if.value == "counterfactual_pair";
    if (!frameOk) {
        std::cerr << "Main schema Counterfactual Pair frame control does not make the perturbation frame explicit\n";
        return false;
    }

    const bool offsetXOk =
        offsetX->has_binding &&
        offsetX->binding.path == "fractal.params.counterfactual_pair_offset_x" &&
        offsetX->has_default &&
        offsetX->def.is_number() &&
        offsetX->def.as_number() == 0.16 &&
        offsetX->has_ui_min &&
        offsetX->ui_min == -1.0 &&
        offsetX->has_ui_max &&
        offsetX->ui_max == 1.0 &&
        offsetX->has_visible_if &&
        offsetX->visible_if.value == "counterfactual_pair";
    const bool offsetYOk =
        offsetY->has_binding &&
        offsetY->binding.path == "fractal.params.counterfactual_pair_offset_y" &&
        offsetY->has_default &&
        offsetY->def.is_number() &&
        offsetY->def.as_number() == 0.08 &&
        offsetY->has_ui_min &&
        offsetY->ui_min == -1.0 &&
        offsetY->has_ui_max &&
        offsetY->ui_max == 1.0 &&
        offsetY->has_visible_if &&
        offsetY->visible_if.value == "counterfactual_pair";
    if (!offsetXOk || !offsetYOk) {
        std::cerr << "Main schema Counterfactual Pair offset controls drifted from the bounded partner-gap owner seam\n";
        return false;
    }

    const bool reconvergenceOk =
        reconvergenceRatio->has_binding &&
        reconvergenceRatio->binding.path == "fractal.params.counterfactual_pair_reconvergence_ratio" &&
        reconvergenceRatio->has_default &&
        reconvergenceRatio->def.is_number() &&
        reconvergenceRatio->def.as_number() == 0.6 &&
        reconvergenceRatio->has_min &&
        reconvergenceRatio->min == 0.0 &&
        reconvergenceRatio->has_help &&
        reconvergenceRatio->help.find("Class 0") != std::string::npos &&
        reconvergenceRatio->help.find("Class 1") != std::string::npos &&
        reconvergenceRatio->help.find("Class 2") != std::string::npos &&
        reconvergenceRatio->help.find("Class 3") != std::string::npos &&
        reconvergenceRatio->has_visible_if &&
        reconvergenceRatio->visible_if.value == "counterfactual_pair";
    if (!reconvergenceOk) {
        std::cerr << "Main schema Counterfactual Pair reconvergence control does not expose the public class model truthfully\n";
        return false;
    }

    return true;
}

} // namespace

int main() {
    // Test logarithmic flag parsing
    {
        const char* json = R"({
            "schema_version": "1",
            "namespace": "test",
            "panels": [
                {
                    "id": "p1",
                    "label": "Panel",
                    "controls": [
                        {
                            "id": "log_slider",
                            "type": "slider_float",
                            "label": "Log Slider",
                            "value_type": "float",
                            "min": 0.001,
                            "max": 10.0,
                            "logarithmic": true,
                            "binding": { "kind": "param", "path": "test.log_param" }
                        },
                        {
                            "id": "linear_slider",
                            "type": "slider_float",
                            "label": "Linear Slider",
                            "value_type": "float",
                            "min": 0.0,
                            "max": 1.0,
                            "binding": { "kind": "param", "path": "test.lin_param" }
                        }
                    ]
                }
            ]
        })";

        json_min::ParseResult pr = json_min::Parse(json);
        if (!pr.error.empty()) {
            std::cerr << "JSON parse failed: " << pr.error << "\n";
            return 1;
        }

        UISchemaLoadResult result = LoadUISchemaFromJson(pr.value);
        if (!result.error.empty()) {
            std::cerr << "Schema load failed: " << result.error << "\n";
            return 1;
        }

        if (result.schema.panels.size() != 1 || result.schema.panels[0].controls.size() != 2) {
            std::cerr << "Expected 1 panel with 2 controls\n";
            return 1;
        }

        const auto& logCtrl = result.schema.panels[0].controls[0];
        const auto& linCtrl = result.schema.panels[0].controls[1];

        if (!logCtrl.logarithmic) {
            std::cerr << "Log slider should have logarithmic=true\n";
            return 1;
        }
        if (linCtrl.logarithmic) {
            std::cerr << "Linear slider should have logarithmic=false (default)\n";
            return 1;
        }
    }

    // Test explicit UI-range metadata parsing for drag controls.
    {
        const char* json = R"({
            "schema_version": "1",
            "namespace": "test",
            "panels": [
                {
                    "id": "p1",
                    "label": "Panel",
                    "controls": [
                        {
                            "id": "center_x",
                            "type": "drag_float",
                            "label": "Center X",
                            "value_type": "float",
                            "ui_min": -2.0,
                            "ui_max": 2.0,
                            "step": 0.001,
                            "binding": { "kind": "param", "path": "fractal.view.center.x" }
                        }
                    ]
                }
            ]
        })";

        json_min::ParseResult pr = json_min::Parse(json);
        if (!pr.error.empty()) {
            std::cerr << "JSON parse failed for ui-range test: " << pr.error << "\n";
            return 1;
        }

        UISchemaLoadResult result = LoadUISchemaFromJson(pr.value);
        if (!result.error.empty()) {
            std::cerr << "Schema load failed for ui-range test: " << result.error << "\n";
            return 1;
        }

        const auto& control = result.schema.panels[0].controls[0];
        if (!control.has_ui_min || !control.has_ui_max || control.ui_min != -2.0 || control.ui_max != 2.0) {
            std::cerr << "Drag control should parse explicit ui_min/ui_max metadata\n";
            return 1;
        }
        if (control.has_min || control.has_max) {
            std::cerr << "ui_min/ui_max should stay separate from hard min/max metadata\n";
            return 1;
        }
    }

    // Test grouped enum option parsing for organized selectors.
    {
        const char* json = R"({
            "schema_version": "1",
            "namespace": "test",
            "panels": [
                {
                    "id": "p1",
                    "label": "Panel",
                    "controls": [
                        {
                            "id": "fractal_type",
                            "type": "combo",
                            "label": "Fractal Type",
                            "value_type": "enum",
                            "options": [
                                { "id": "explaino", "label": "Explaino", "group": "Common" },
                                { "id": "newton", "label": "Newton", "group": "Root-Finding" },
                                { "id": "mandelbrot", "label": "Mandelbrot", "group": "Common" }
                            ],
                            "binding": { "kind": "param", "path": "fractal.view.fractal_type" }
                        }
                    ]
                }
            ]
        })";

        json_min::ParseResult pr = json_min::Parse(json);
        if (!pr.error.empty()) {
            std::cerr << "JSON parse failed for grouped selector test: " << pr.error << "\n";
            return 1;
        }

        UISchemaLoadResult result = LoadUISchemaFromJson(pr.value);
        if (!result.error.empty()) {
            std::cerr << "Schema load failed for grouped selector test: " << result.error << "\n";
            return 1;
        }

        if (result.schema.panels.size() != 1 || result.schema.panels[0].controls.size() != 1) {
            std::cerr << "Expected 1 panel with 1 grouped selector control\n";
            return 1;
        }

        const auto& fractalCtrl = result.schema.panels[0].controls[0];
        if (fractalCtrl.options.size() != 3) {
            std::cerr << "Expected 3 grouped selector options\n";
            return 1;
        }
        if (fractalCtrl.options[0].group != "Common" ||
            fractalCtrl.options[1].group != "Root-Finding" ||
            fractalCtrl.options[2].group != "Common") {
            std::cerr << "Expected grouped selector metadata to round-trip through schema parsing\n";
            return 1;
        }
    }

    // Test that the checked-in schema file the app depends on parses,
    // decodes, and validates against the live binding surface.
    {
        const std::filesystem::path repoRoot = FindRepoRoot();
        if (repoRoot.empty()) {
            std::cerr << "Could not locate repo root for schema validation test\n";
            return 1;
        }

        const std::filesystem::path schemaPath = repoRoot / "ui" / "fractal_binding_surface_v1.ui_schema.json";

        bool foundLogRate = false;
        bool foundPhaseSlider = false;
        bool foundDualSeedB = false;
        bool foundDualSeedMix = false;
        bool foundColoringModeControl = false;
        bool foundColorSignalControl = false;
        bool foundColorPaletteControl = false;
        bool foundColorGradingControl = false;
        int coloringModeControlCount = 0;
        int colorSignalControlCount = 0;
        int colorPaletteControlCount = 0;
        int colorGradingControlCount = 0;
        bool foundMultibrotPowerFloat = false;
        bool foundLambdaReal = false;
        bool foundLambdaImag = false;
        bool foundFractalTypeCommonGroup = false;
        bool foundFractalTypeRootFindingGroup = false;
        bool foundCounterfactualPairRootFindingGroup = false;
        bool foundFractalTypeEscapeTimeGroup = false;
        bool foundFractalTypeExplainoGroup = false;
        bool foundFractalTypeExplainoAllGroup = false;
        bool foundFractalTypeExplainoAllFirst = false;
        bool foundSpiderEscapeTimeGroup = false;
        bool foundCelticEscapeTimeGroup = false;
        bool foundPerpendicularShipEscapeTimeGroup = false;
        bool foundFractalTypeDefaultExplainoAll = false;
        bool foundEpsilonVisibleForExplainoAll = false;
        bool foundEpsilonVisibleForCounterfactualPair = false;
        bool foundExplainoSeedVisibleForExplainoAll = false;
        bool foundRippleAmplitudeVisibleForExplainoAll = false;
        bool foundSpliceOffsetVisibleForExplainoAll = false;
        bool foundVortexStrengthVisibleForExplainoAll = false;
        bool foundTensionStrengthVisibleForExplainoAll = false;
        bool foundBalanceVoidVisibleForExplainoAll = false;
        bool foundSymmetryTensionVisibleForExplainoAll = false;
        bool foundFieldCurvatureVisibleForExplainoAll = false;
        std::size_t explainoGroupOptionCount = 0;
        std::size_t explainoAllVisibleControlCount = 0;
        std::size_t explainoAllVisibleAxisCount = 0;
        std::size_t explainoAllVisibleNonAxisCount = 0;
        bool foundRenderWidthDefault = false;
        bool foundRenderHeightDefault = false;
        bool foundInteractionDebounceDefault = false;
        bool foundPreviewTargetFpsDefault = false;
        bool foundPreviewMinScaleDefault = false;
        bool foundContinuousRenderDefaultFalse = false;
        bool foundNegativeExplainoSeedRange = false;
        bool foundNegativeExplainoSeedBRange = false;
        bool foundNegativePhaseStrengthRange = false;
        bool foundRootSpreadClampAligned = false;
        bool foundClusterRadiusClampAligned = false;
        bool foundClusterRadiusVisibilityFixed = false;
        bool foundPositiveNovaAlphaMin = false;
        bool foundCenterXUiRange = false;
        bool foundCenterYUiRange = false;
        bool foundZoomNoUiCap = false;
        bool foundRotationUiRange = false;
        bool foundMaxIterUiCap = false;
        bool foundEpsilonSoftMax = false;
        bool foundSeedRateSoftMin = false;
        bool foundParamAnimRateSoftMin = false;
        bool foundDiveSpeedUiCap = false;
        bool foundPolyCoeffUiRange = false;
        bool foundExplainoPhaseUiRange = false;
        bool foundExplainoPhaseStrengthUiRange = false;
        bool foundExplainoPhaseVisibleForDual = false;
        bool foundExplainoWarpClampAligned = false;
        bool foundExplainoDampingUiRange = false;
        bool foundMomentumBetaUiRange = false;
        bool foundJoyCouplingUiRange = false;
        bool foundFoldCouplingUiRange = false;
        bool foundBellCouplingUiRange = false;
        bool foundDeferredCouplingSchemaMatches[sizeof(kExplainoCouplingRegistry) / sizeof(kExplainoCouplingRegistry[0])] = {};
        bool foundDualSeedSchemaMatches[sizeof(kExplainoDualSeedRegistry) / sizeof(kExplainoDualSeedRegistry[0])] = {};
        bool foundPhoenixCarrierSchemaMatches[sizeof(kPhoenixStepCarrierRegistry) / sizeof(kPhoenixStepCarrierRegistry[0])] = {};
        bool foundClusterRadiusSchemaMatches[sizeof(kExplainoClusterRadiusRegistry) / sizeof(kExplainoClusterRadiusRegistry[0])] = {};
        bool foundRippleAmplitudeUiRange = false;
        bool foundSpliceOffsetUiRange = false;
        bool foundVortexStrengthUiRange = false;
        bool foundTensionStrengthUiRange = false;

        if (!LoadAndValidateSchemaFile(schemaPath)) {
            return 1;
        }

        std::string text;
        if (!ReadTextFile(schemaPath, &text)) {
            std::cerr << "Failed to re-read schema file: " << schemaPath.string() << "\n";
            return 1;
        }
        json_min::ParseResult pr2 = json_min::Parse(text);
        UISchemaLoadResult result = LoadUISchemaFromJson(pr2.value);
        if (!ValidateCounterfactualPairControlSurface(result.schema)) {
            return 1;
        }

        for (const auto& panel : result.schema.panels) {
            for (const auto& ctrl : panel.controls) {
                if (ctrl.id == "explaino_seed_rate" && ctrl.logarithmic) foundLogRate = true;
                if (ctrl.id == "explaino_phase" && ctrl.type == "slider_float") foundPhaseSlider = true;
                if (ctrl.id == "explaino_seed_b") foundDualSeedB = true;
                if (ctrl.id == "explaino_mix") foundDualSeedMix = true;
                if (ctrl.id == "multibrot_power_float" && ctrl.type == "slider_float" && ctrl.has_binding &&
                    ctrl.binding.path == "fractal.params.multibrot_power_float") {
                    foundMultibrotPowerFloat = true;
                }
                if (ctrl.id == "lambda_real" && ctrl.has_binding && ctrl.binding.path == "fractal.params.lambda_real") {
                    foundLambdaReal = true;
                }
                if (ctrl.id == "lambda_imag" && ctrl.has_binding && ctrl.binding.path == "fractal.params.lambda_imag") {
                    foundLambdaImag = true;
                }
                if (ctrl.id == "fractal_type") {
                    if (ctrl.has_default && ctrl.def.is_string() && ctrl.def.as_string() == "explaino_all") {
                        foundFractalTypeDefaultExplainoAll = true;
                    }
                    for (std::size_t optionIndex = 0; optionIndex < ctrl.options.size(); ++optionIndex) {
                        const auto& option = ctrl.options[optionIndex];
                        if (option.id == "explaino" && option.group == "Common") {
                            foundFractalTypeCommonGroup = true;
                            const ExplainoSelectorDescriptor* descriptor = FindExplainoSelectorDescriptor(option.id);
                            if (!descriptor || descriptor->role != ExplainoSelectorRole::baseline) {
                                std::cerr << "Main schema did not classify baseline explaino through the canonical Explaino selector registry\n";
                                return 1;
                            }
                        }
                        if (option.id == "newton" && option.group == "Root-Finding") foundFractalTypeRootFindingGroup = true;
                        if (option.id == "counterfactual_pair" && option.group == "Root-Finding") foundCounterfactualPairRootFindingGroup = true;
                        if (option.id == "multibrot" && option.group == "Escape-Time") foundFractalTypeEscapeTimeGroup = true;
                        if (option.group == "Explaino") {
                            ++explainoGroupOptionCount;
                            const ExplainoSelectorDescriptor* descriptor = FindExplainoSelectorDescriptor(option.id);
                            if (!descriptor || descriptor->role == ExplainoSelectorRole::baseline) {
                                std::cerr << "Main schema Explaino selector options drifted outside the canonical Explaino selector registry\n";
                                return 1;
                            }
                        }
                        if (option.id == "explaino_lambda" && option.group == "Explaino") foundFractalTypeExplainoGroup = true;
                        if (option.id == "explaino_all" && option.group == "Explaino") foundFractalTypeExplainoAllGroup = true;
                        if (!foundFractalTypeExplainoAllFirst && option.group == "Explaino") {
                            foundFractalTypeExplainoAllFirst = option.id == "explaino_all";
                        }
                        if (option.id == "spider" && option.group == "Escape-Time") foundSpiderEscapeTimeGroup = true;
                        if (option.id == "celtic_mandelbrot" && option.group == "Escape-Time") foundCelticEscapeTimeGroup = true;
                        if (option.id == "perpendicular_burning_ship" && option.group == "Escape-Time") foundPerpendicularShipEscapeTimeGroup = true;
                    }
                }
                if (ctrl.id == "height" && ctrl.has_default && ctrl.def.is_number() && ctrl.def.as_number() == 1536.0) {
                    foundRenderHeightDefault = true;
                }
                if (ctrl.id == "width" && ctrl.has_default && ctrl.def.is_number() && ctrl.def.as_number() == 2048.0) {
                    foundRenderWidthDefault = true;
                }
                if (ctrl.id == "interaction_debounce_ms" && ctrl.has_default && ctrl.def.is_number() && ctrl.def.as_number() == 200.0) {
                    foundInteractionDebounceDefault = true;
                }
                if (ctrl.id == "preview_target_fps" && ctrl.has_default && ctrl.def.is_number() && ctrl.def.as_number() == 30.0) {
                    foundPreviewTargetFpsDefault = true;
                }
                if (ctrl.id == "preview_min_scale" && ctrl.has_default && ctrl.def.is_number() && ctrl.def.as_number() == 0.5) {
                    foundPreviewMinScaleDefault = true;
                }
                if (ctrl.id == "auto_refresh" && ctrl.label == "Continuous Render" && ctrl.has_default && ctrl.def.is_bool() && !ctrl.def.as_bool()) {
                    foundContinuousRenderDefaultFalse = true;
                }
                if (ctrl.id == "explaino_seed" && ctrl.has_ui_min && ctrl.has_ui_max &&
                    ctrl.ui_min == -10.0 && ctrl.ui_max == 10.0 && !ctrl.has_min && !ctrl.has_max) {
                    foundNegativeExplainoSeedRange = true;
                }
                if (ctrl.id == "explaino_seed_b" && ctrl.has_ui_min && ctrl.has_ui_max &&
                    ctrl.ui_min == -10.0 && ctrl.ui_max == 10.0 && !ctrl.has_min && !ctrl.has_max) {
                    foundNegativeExplainoSeedBRange = true;
                }
                if (ctrl.id == "explaino_phase_strength" && ctrl.has_ui_min && ctrl.has_ui_max &&
                    ctrl.ui_min == -20.0 && ctrl.ui_max == 20.0 && !ctrl.has_min && !ctrl.has_max) {
                    foundNegativePhaseStrengthRange = true;
                }
                if (ctrl.id == "explaino_root_spread" && ctrl.has_max && ctrl.max == 3.0) {
                    foundRootSpreadClampAligned = true;
                }
                if (ctrl.id == "explaino_cluster_radius" && ctrl.has_max && ctrl.max == 2.0) {
                    foundClusterRadiusClampAligned = true;
                }
                if (ctrl.id == "explaino_cluster_radius" && ctrl.has_visible_if && ctrl.visible_if.op == "in") {
                    foundClusterRadiusVisibilityFixed = true;
                }
                if (ctrl.id == "nova_alpha" && ctrl.has_min && ctrl.min > 0.0) {
                    foundPositiveNovaAlphaMin = true;
                }
                if (ctrl.id == "center_x" && ctrl.has_ui_min && ctrl.has_ui_max && ctrl.ui_min == -2.0 && ctrl.ui_max == 2.0 && !ctrl.has_min && !ctrl.has_max) {
                    foundCenterXUiRange = true;
                }
                if (ctrl.id == "center_y" && ctrl.has_ui_min && ctrl.has_ui_max && ctrl.ui_min == -2.0 && ctrl.ui_max == 2.0 && !ctrl.has_min && !ctrl.has_max) {
                    foundCenterYUiRange = true;
                }
                if (ctrl.id == "zoom" && ctrl.has_min && ctrl.min == 1e-12 &&
                    !ctrl.has_ui_min && !ctrl.has_ui_max &&
                    !ctrl.has_max && ctrl.logarithmic) {
                    foundZoomNoUiCap = true;
                }
                if (ctrl.id == "rotation_deg" && ctrl.has_ui_min && ctrl.ui_min == -180.0 &&
                    ctrl.has_ui_max && ctrl.ui_max == 180.0 && !ctrl.has_min && !ctrl.has_max) {
                    foundRotationUiRange = true;
                }
                if (ctrl.id == "max_iter" && ctrl.has_min && ctrl.min == 1.0 &&
                    ctrl.has_ui_min && ctrl.ui_min == 1.0 && ctrl.has_ui_max && ctrl.ui_max == 5000.0 &&
                    !ctrl.has_max) {
                    foundMaxIterUiCap = true;
                }
                if (ctrl.id == "epsilon" && ctrl.has_min && ctrl.min == 1e-12 &&
                    ctrl.has_ui_min && ctrl.ui_min == 1e-12 && ctrl.has_ui_max && ctrl.ui_max == 0.01 &&
                    !ctrl.has_max && ctrl.logarithmic) {
                    foundEpsilonSoftMax = true;
                }
                if (ctrl.id == "explaino_seed_rate" && ctrl.has_min && ctrl.min == 0.0 &&
                    ctrl.has_ui_min && ctrl.ui_min == 0.0001 && ctrl.has_ui_max && ctrl.ui_max == 5.0 &&
                    !ctrl.has_max && ctrl.logarithmic) {
                    foundSeedRateSoftMin = true;
                }
                if (ctrl.id == "param_anim_rate" && ctrl.has_min && ctrl.min == 0.0 &&
                    ctrl.has_ui_min && ctrl.ui_min == 0.0001 && ctrl.has_ui_max && ctrl.ui_max == 5.0 &&
                    !ctrl.has_max && ctrl.logarithmic) {
                    foundParamAnimRateSoftMin = true;
                }
                if (ctrl.id == "dive_speed" && ctrl.has_min && ctrl.min == 0.0 &&
                    ctrl.has_ui_min && ctrl.ui_min == 0.0 && ctrl.has_ui_max && ctrl.ui_max == 5.0 && !ctrl.has_max) {
                    foundDiveSpeedUiCap = true;
                }
                if ((ctrl.id == "poly_c0" || ctrl.id == "poly_c1" || ctrl.id == "poly_c2" ||
                     ctrl.id == "poly_c3" || ctrl.id == "poly_c4") &&
                    ctrl.has_ui_min && ctrl.ui_min == -10.0 && ctrl.has_ui_max && ctrl.ui_max == 10.0 &&
                    !ctrl.has_min && !ctrl.has_max) {
                    foundPolyCoeffUiRange = true;
                }
                if (ctrl.id == "explaino_phase" && ctrl.has_ui_min && ctrl.ui_min == -6.283185307179586 &&
                    ctrl.has_ui_max && ctrl.ui_max == 6.283185307179586 && !ctrl.has_min && !ctrl.has_max) {
                    foundExplainoPhaseUiRange = true;
                }
                if (ctrl.id == "explaino_phase" && ctrl.has_visible_if &&
                    ctrl.visible_if.value.find("explaino_dual") != std::string::npos) {
                    foundExplainoPhaseVisibleForDual = true;
                }
                if (ctrl.id == "explaino_phase_strength" && ctrl.has_ui_min && ctrl.ui_min == -20.0 &&
                    ctrl.has_ui_max && ctrl.ui_max == 20.0 && !ctrl.has_min && !ctrl.has_max) {
                    foundExplainoPhaseStrengthUiRange = true;
                }
                if (ctrl.id == "explaino_warp_strength" && ctrl.has_min && ctrl.min == 0.0 &&
                    ctrl.has_max && ctrl.max == 1.0 && !ctrl.has_ui_min && !ctrl.has_ui_max) {
                    foundExplainoWarpClampAligned = true;
                }
                if (ctrl.id == "explaino_damping" && ctrl.has_ui_min && ctrl.ui_min == 0.01 &&
                    ctrl.has_ui_max && ctrl.ui_max == 10.0 && !ctrl.has_min && !ctrl.has_max) {
                    foundExplainoDampingUiRange = true;
                }
                if (ctrl.id == "momentum_beta" && ctrl.has_ui_min && ctrl.ui_min == -1.0 &&
                    ctrl.has_ui_max && ctrl.ui_max == 1.0 && !ctrl.has_min && !ctrl.has_max) {
                    foundMomentumBetaUiRange = true;
                }
                if (ctrl.id == "joy_coupling" && ctrl.has_ui_min && ctrl.ui_min == 0.0 &&
                    ctrl.has_ui_max && ctrl.ui_max == 1.0 && !ctrl.has_min && !ctrl.has_max) {
                    foundJoyCouplingUiRange = true;
                }
                if (ctrl.id == "fold_coupling" && ctrl.has_ui_min && ctrl.ui_min == 0.0 &&
                    ctrl.has_ui_max && ctrl.ui_max == 1.0 && !ctrl.has_min && !ctrl.has_max) {
                    foundFoldCouplingUiRange = true;
                }
                if (ctrl.id == "bell_coupling" && ctrl.has_ui_min && ctrl.ui_min == 0.0 &&
                    ctrl.has_ui_max && ctrl.ui_max == 1.0 && !ctrl.has_min && !ctrl.has_max) {
                    foundBellCouplingUiRange = true;
                }
                if (ctrl.has_binding) {
                    const ExplainoDualSeedDescriptor* dualSeed = FindExplainoDualSeedDescriptorByBindingPath(ctrl.binding.path);
                    if (dualSeed) {
                        const std::size_t dualIndex = static_cast<std::size_t>(dualSeed - kExplainoDualSeedRegistry);
                        const char* carrierId = FractalTypeId(dualSeed->carrier_fractal_type);
                        if (dualIndex >= (sizeof(foundDualSeedSchemaMatches) / sizeof(foundDualSeedSchemaMatches[0])) ||
                            foundDualSeedSchemaMatches[dualIndex]) {
                            std::cerr << "Main schema duplicated a deferred Explaino dual-seed control instead of deriving one owner per parameter\n";
                            return 1;
                        }
                        if (ctrl.id != dualSeed->param_id ||
                            !ctrl.has_visible_if || ctrl.visible_if.op != "eq" ||
                            ctrl.visible_if.path != "fractal.view.fractal_type" ||
                            !carrierId || ctrl.visible_if.value != carrierId ||
                            VisibleIfIncludesFractalType(ctrl, "explaino_all")) {
                            std::cerr << "Main schema deferred Explaino dual-seed controls should stay fenced to explaino_dual until canonical proof lands\n";
                            return 1;
                        }
                        foundDualSeedSchemaMatches[dualIndex] = true;
                    }
                    const ExplainoCouplingDescriptor* coupling = FindExplainoCouplingDescriptorByBindingPath(ctrl.binding.path);
                    if (coupling) {
                        const std::size_t slotIndex = static_cast<std::size_t>(coupling->slot);
                        const char* carrierId = FractalTypeId(coupling->carrier_fractal_type);
                        if (slotIndex >= (sizeof(foundDeferredCouplingSchemaMatches) / sizeof(foundDeferredCouplingSchemaMatches[0])) ||
                            foundDeferredCouplingSchemaMatches[slotIndex]) {
                            std::cerr << "Main schema duplicated a deferred Explaino coupling control instead of deriving one owner per coupling\n";
                            return 1;
                        }
                        if (ctrl.id != coupling->param_id ||
                            !ctrl.has_visible_if || ctrl.visible_if.op != "eq" ||
                            ctrl.visible_if.path != "fractal.view.fractal_type" ||
                            !carrierId || ctrl.visible_if.value != carrierId ||
                            VisibleIfIncludesFractalType(ctrl, "explaino_all")) {
                            std::cerr << "Main schema deferred Explaino couplings should stay fenced to their owning legacy selector only\n";
                            return 1;
                        }
                        foundDeferredCouplingSchemaMatches[slotIndex] = true;
                    }
                    const PhoenixStepCarrierDescriptor* phoenixCarrier = FindPhoenixStepCarrierDescriptorByBindingPath(ctrl.binding.path);
                    if (phoenixCarrier) {
                        const std::size_t slotIndex = static_cast<std::size_t>(phoenixCarrier->slot);
                        if (slotIndex >= (sizeof(foundPhoenixCarrierSchemaMatches) / sizeof(foundPhoenixCarrierSchemaMatches[0])) ||
                            foundPhoenixCarrierSchemaMatches[slotIndex]) {
                            std::cerr << "Main schema duplicated a shared phoenix-step carrier control instead of deriving one owner per parameter\n";
                            return 1;
                        }
                        std::size_t expectedCarrierCount = 0;
                        for (const auto& carrier : kPhoenixStepCarrierSelectorRegistry) {
                            if (carrier.slot == phoenixCarrier->slot) {
                                const char* carrierId = FractalTypeId(carrier.carrier_fractal_type);
                                if (!carrierId || !VisibleIfIncludesFractalType(ctrl, carrierId)) {
                                    std::cerr << "Main schema phoenix-step carrier controls should stay fenced to their explicit shared carrier selectors only\n";
                                    return 1;
                                }
                                ++expectedCarrierCount;
                            }
                        }
                        if (ctrl.id != phoenixCarrier->param_id ||
                            !ctrl.has_visible_if || ctrl.visible_if.op != "in" ||
                            ctrl.visible_if.path != "fractal.view.fractal_type" ||
                            VisibleIfIncludesFractalType(ctrl, "explaino_all") ||
                            CsvTokenCount(ctrl.visible_if.value) != expectedCarrierCount) {
                            std::cerr << "Main schema phoenix-step carrier control should decode exactly to its explicit shared carrier map\n";
                            return 1;
                        }
                        foundPhoenixCarrierSchemaMatches[slotIndex] = true;
                    }
                    const ExplainoClusterRadiusDescriptor* clusterRadius = FindExplainoClusterRadiusDescriptorByBindingPath(ctrl.binding.path);
                    if (clusterRadius) {
                        const std::size_t slotIndex = static_cast<std::size_t>(clusterRadius->slot);
                        if (slotIndex >= (sizeof(foundClusterRadiusSchemaMatches) / sizeof(foundClusterRadiusSchemaMatches[0])) ||
                            foundClusterRadiusSchemaMatches[slotIndex]) {
                            std::cerr << "Main schema duplicated the deferred explaino_cluster_radius control instead of deriving one owner per parameter\n";
                            return 1;
                        }
                        std::size_t expectedCarrierCount = 0;
                        for (const auto& carrier : kExplainoClusterRadiusSelectorRegistry) {
                            if (carrier.slot == clusterRadius->slot) {
                                const char* carrierId = FractalTypeId(carrier.carrier_fractal_type);
                                if (!carrierId || !VisibleIfIncludesFractalType(ctrl, carrierId)) {
                                    std::cerr << "Main schema explaino_cluster_radius control should stay fenced to its explicit split selectors only\n";
                                    return 1;
                                }
                                ++expectedCarrierCount;
                            }
                        }
                        if (ctrl.id != clusterRadius->param_id ||
                            !ctrl.has_visible_if || ctrl.visible_if.op != "in" ||
                            ctrl.visible_if.path != "fractal.view.fractal_type" ||
                            VisibleIfIncludesFractalType(ctrl, "explaino_all") ||
                            CsvTokenCount(ctrl.visible_if.value) != expectedCarrierCount) {
                            std::cerr << "Main schema explaino_cluster_radius control should decode exactly to its explicit split selector map\n";
                            return 1;
                        }
                        foundClusterRadiusSchemaMatches[slotIndex] = true;
                    }
                }
                if (ctrl.id == "ripple_amplitude" && ctrl.has_ui_min && ctrl.ui_min == 0.0 &&
                    ctrl.has_ui_max && ctrl.ui_max == 0.5 && !ctrl.has_min && !ctrl.has_max) {
                    foundRippleAmplitudeUiRange = true;
                }
                if (ctrl.id == "epsilon" && VisibleIfIncludesFractalType(ctrl, "explaino_all")) {
                    foundEpsilonVisibleForExplainoAll = true;
                }
                if (ctrl.id == "epsilon" && VisibleIfIncludesFractalType(ctrl, "counterfactual_pair")) {
                    foundEpsilonVisibleForCounterfactualPair = true;
                }
                if (ctrl.id == "explaino_seed" && VisibleIfIncludesFractalType(ctrl, "explaino_all")) {
                    foundExplainoSeedVisibleForExplainoAll = true;
                }
                if (ctrl.id == "ripple_amplitude" && VisibleIfIncludesFractalType(ctrl, "explaino_all")) {
                    foundRippleAmplitudeVisibleForExplainoAll = true;
                }
                if (ctrl.id == "splice_offset" && ctrl.has_ui_min && ctrl.ui_min == 0.0 &&
                    ctrl.has_ui_max && ctrl.ui_max == 2.0 && !ctrl.has_min && !ctrl.has_max) {
                    foundSpliceOffsetUiRange = true;
                }
                if (ctrl.id == "splice_offset" && VisibleIfIncludesFractalType(ctrl, "explaino_all")) {
                    foundSpliceOffsetVisibleForExplainoAll = true;
                }
                if (ctrl.id == "vortex_strength" && ctrl.has_ui_min && ctrl.ui_min == 0.0 &&
                    ctrl.has_ui_max && ctrl.ui_max == 1.0 && !ctrl.has_min && !ctrl.has_max) {
                    foundVortexStrengthUiRange = true;
                }
                if (ctrl.id == "vortex_strength" && VisibleIfIncludesFractalType(ctrl, "explaino_all")) {
                    foundVortexStrengthVisibleForExplainoAll = true;
                }
                if (ctrl.id == "tension_strength" && ctrl.has_ui_min && ctrl.ui_min == 0.0 &&
                    ctrl.has_ui_max && ctrl.ui_max == 0.1 && !ctrl.has_min && !ctrl.has_max) {
                    foundTensionStrengthUiRange = true;
                }
                if (ctrl.id == "tension_strength" && VisibleIfIncludesFractalType(ctrl, "explaino_all")) {
                    foundTensionStrengthVisibleForExplainoAll = true;
                }
                if (ctrl.id == "balance_void" && VisibleIfIncludesFractalType(ctrl, "explaino_all")) {
                    foundBalanceVoidVisibleForExplainoAll = true;
                }
                if (ctrl.id == "symmetry_tension" && VisibleIfIncludesFractalType(ctrl, "explaino_all")) {
                    foundSymmetryTensionVisibleForExplainoAll = true;
                }
                if (ctrl.id == "field_curvature" && VisibleIfIncludesFractalType(ctrl, "explaino_all")) {
                    foundFieldCurvatureVisibleForExplainoAll = true;
                }
                if (VisibleIfIncludesFractalType(ctrl, "explaino_all")) {
                    ++explainoAllVisibleControlCount;
                    const ExplainoAxisDescriptor* axis = FindExplainoAxisDescriptor(ctrl.id);
                    if (axis) {
                        ++explainoAllVisibleAxisCount;
                        if (!ctrl.has_binding || ctrl.binding.path != axis->binding_path) {
                            std::cerr << "Main schema Explaino-all axis controls drifted away from the canonical Explaino axis registry\n";
                            return 1;
                        }
                        const char* carrierId = FractalTypeId(axis->carrier_fractal_type);
                        if (!ctrl.has_visible_if || ctrl.visible_if.op != "in" ||
                            ctrl.visible_if.path != "fractal.view.fractal_type" ||
                            !VisibleIfIncludesFractalType(ctrl, "explaino_all") ||
                            !carrierId || !VisibleIfIncludesFractalType(ctrl, carrierId) ||
                            CsvTokenCount(ctrl.visible_if.value) != 2u) {
                            std::cerr << "Main schema Explaino-all axis controls should decode to explaino_all plus their owning legacy carrier only\n";
                            return 1;
                        }
                    } else {
                        ++explainoAllVisibleNonAxisCount;
                        if (!ctrl.has_binding || !IsKnownExplainoAllVisibleBindingPath(ctrl.binding.path)) {
                            std::cerr << "Main schema exposed a new explaino_all-visible non-axis control without explicit canonical classification\n";
                            return 1;
                        }
                    }
                }
                if (ctrl.has_binding && ctrl.binding.path == "fractal.params.coloring_mode") {
                    ++coloringModeControlCount;
                    if (ctrl.id == "coloring_mode" && ctrl.options.size() == 6) {
                        bool foundRootBasin = false;
                        bool foundSmoothEscape = false;
                        bool foundPhase = false;
                        for (const auto& option : ctrl.options) {
                            if (option.id == "root_basin") foundRootBasin = true;
                            if (option.id == "smooth_escape") foundSmoothEscape = true;
                            if (option.id == "phase") foundPhase = true;
                        }
                        foundColoringModeControl = foundRootBasin && foundSmoothEscape && foundPhase;
                    }
                }
                if (ctrl.has_binding && ctrl.binding.path == "fractal.params.color_signal") {
                    ++colorSignalControlCount;
                    if (ctrl.id == "color_signal" && ctrl.options.size() == 5) {
                        bool foundRootIndex = false;
                        bool foundSmoothEscape = false;
                        bool foundPhaseAngle = false;
                        for (const auto& option : ctrl.options) {
                            if (option.id == "root_index") foundRootIndex = true;
                            if (option.id == "smooth_escape") foundSmoothEscape = true;
                            if (option.id == "phase_angle") foundPhaseAngle = true;
                        }
                        foundColorSignalControl = foundRootIndex && foundSmoothEscape && foundPhaseAngle;
                    }
                }
                if (ctrl.has_binding && ctrl.binding.path == "fractal.params.color_palette") {
                    ++colorPaletteControlCount;
                    if (ctrl.id == "color_palette" && ctrl.options.size() == 5) {
                        bool foundRootClassic = false;
                        bool foundJoy = false;
                        bool foundBandedEscape = false;
                        for (const auto& option : ctrl.options) {
                            if (option.id == "root_classic") foundRootClassic = true;
                            if (option.id == "joy") foundJoy = true;
                            if (option.id == "banded_escape") foundBandedEscape = true;
                        }
                        foundColorPaletteControl = foundRootClassic && foundJoy && foundBandedEscape;
                    }
                }
                if (ctrl.has_binding && ctrl.binding.path == "fractal.params.color_grading") {
                    ++colorGradingControlCount;
                    if (ctrl.id == "color_grading" && ctrl.options.size() == 4) {
                        bool foundBasinDefault = false;
                        bool foundPhaseDefault = false;
                        bool foundBandsDefault = false;
                        for (const auto& option : ctrl.options) {
                            if (option.id == "basin_default") foundBasinDefault = true;
                            if (option.id == "phase_default") foundPhaseDefault = true;
                            if (option.id == "bands_default") foundBandsDefault = true;
                        }
                        foundColorGradingControl = foundBasinDefault && foundPhaseDefault && foundBandsDefault;
                    }
                }
            }
        }

        if (!foundLogRate) {
            std::cerr << "Did not find explaino_seed_rate control with logarithmic=true in schema\n";
            return 1;
        }
        if (!foundPhaseSlider) {
            std::cerr << "Did not find explaino_phase slider_float control in schema\n";
            return 1;
        }
        if (!foundMultibrotPowerFloat) {
            std::cerr << "Did not find non-integer Multibrot power float control in schema\n";
            return 1;
        }
        if (!foundRenderWidthDefault || !foundRenderHeightDefault) {
            std::cerr << "Did not find exploration-first render resolution defaults in schema\n";
            return 1;
        }
        if (!foundInteractionDebounceDefault || !foundPreviewTargetFpsDefault || !foundPreviewMinScaleDefault) {
            std::cerr << "Did not find adaptive preview pacing controls with the expected defaults in schema\n";
            return 1;
        }
        if (!foundContinuousRenderDefaultFalse) {
            std::cerr << "Did not find continuous-render control with a disabled-by-default schema value\n";
            return 1;
        }
        if (!foundNegativeExplainoSeedRange || !foundNegativeExplainoSeedBRange || !foundNegativePhaseStrengthRange) {
            std::cerr << "Did not find negative-capable Explaino controls in schema\n";
            return 1;
        }
        if (!foundRootSpreadClampAligned || !foundClusterRadiusClampAligned || !foundClusterRadiusVisibilityFixed || !foundPositiveNovaAlphaMin) {
            std::cerr << "Did not find engine-aligned Explaino/Nova control limits in schema\n";
            return 1;
        }
        if (!foundCenterXUiRange || !foundCenterYUiRange) {
            std::cerr << "Did not find explicit UI ranges for unbounded center drag controls in schema\n";
            return 1;
        }
        if (!foundZoomNoUiCap || !foundRotationUiRange || !foundMaxIterUiCap || !foundEpsilonSoftMax ||
            !foundSeedRateSoftMin || !foundParamAnimRateSoftMin || !foundDiveSpeedUiCap ||
            !foundPolyCoeffUiRange || !foundExplainoPhaseUiRange || !foundExplainoPhaseStrengthUiRange ||
            !foundExplainoPhaseVisibleForDual || !foundExplainoWarpClampAligned || !foundExplainoDampingUiRange || !foundMomentumBetaUiRange ||
            !foundJoyCouplingUiRange || !foundFoldCouplingUiRange || !foundBellCouplingUiRange ||
            !foundRippleAmplitudeUiRange || !foundSpliceOffsetUiRange || !foundVortexStrengthUiRange ||
            !foundTensionStrengthUiRange) {
            std::cerr << "Did not find the expected soft-range MVP control updates in schema\n";
            return 1;
        }
        if (!foundLambdaReal || !foundLambdaImag) {
            std::cerr << "Did not find Lambda real/imag controls in schema\n";
            return 1;
        }
        if (!foundFractalTypeCommonGroup || !foundFractalTypeRootFindingGroup || !foundCounterfactualPairRootFindingGroup || !foundFractalTypeEscapeTimeGroup ||
            !foundFractalTypeExplainoGroup || !foundFractalTypeExplainoAllGroup) {
            std::cerr << "Did not find grouped fractal selector categories including the canonical Explaino-all entry in schema\n";
            return 1;
        }
        if (!foundFractalTypeExplainoAllFirst) {
            std::cerr << "Did not find Explaino-all as the first Explaino selector entry in schema\n";
            return 1;
        }
        if (explainoGroupOptionCount != kExpectedExplainoGroupedSelectorCount) {
            std::cerr << "Schema Explaino selector group count drifted away from the canonical Explaino selector registry\n";
            return 1;
        }
        if (!foundFractalTypeDefaultExplainoAll) {
            std::cerr << "Did not find Explaino-all as the canonical startup fractal default in schema\n";
            return 1;
        }
        if (!foundEpsilonVisibleForExplainoAll || !foundEpsilonVisibleForCounterfactualPair || !foundExplainoSeedVisibleForExplainoAll) {
            std::cerr << "Did not preserve the existing Explaino family control surface for the canonical Explaino-all identity\n";
            return 1;
        }
        if (!foundRippleAmplitudeVisibleForExplainoAll || !foundSpliceOffsetVisibleForExplainoAll ||
            !foundVortexStrengthVisibleForExplainoAll || !foundTensionStrengthVisibleForExplainoAll ||
            !foundBalanceVoidVisibleForExplainoAll || !foundSymmetryTensionVisibleForExplainoAll ||
            !foundFieldCurvatureVisibleForExplainoAll) {
            std::cerr << "Did not find the canonical Explaino-all axis control surface in schema\n";
            return 1;
        }
        if (explainoAllVisibleAxisCount != (sizeof(kExplainoAxisRegistry) / sizeof(kExplainoAxisRegistry[0])) ||
            explainoAllVisibleNonAxisCount != kKnownExplainoAllVisibleBindingPathCount ||
            explainoAllVisibleControlCount != explainoAllVisibleAxisCount + explainoAllVisibleNonAxisCount) {
            std::cerr << "Main schema Explaino-all visible control surface drifted outside the canonical axis registry and explicit non-axis allowlist\n";
            return 1;
        }
        for (bool foundDeferredCouplingSchema : foundDeferredCouplingSchemaMatches) {
            if (!foundDeferredCouplingSchema) {
                std::cerr << "Main schema must expose every deferred Explaino coupling through one explicit legacy-only control fence\n";
                return 1;
            }
        }
        for (bool foundDeferredDualSeedSchema : foundDualSeedSchemaMatches) {
            if (!foundDeferredDualSeedSchema) {
                std::cerr << "Main schema must expose every deferred Explaino dual-seed control through one explicit explaino_dual fence\n";
                return 1;
            }
        }
        for (bool foundClusterRadiusSchema : foundClusterRadiusSchemaMatches) {
            if (!foundClusterRadiusSchema) {
                std::cerr << "Main schema must expose explaino_cluster_radius through one explicit split-selector fence\n";
                return 1;
            }
        }
        if (!foundSpiderEscapeTimeGroup || !foundCelticEscapeTimeGroup || !foundPerpendicularShipEscapeTimeGroup) {
            std::cerr << "Did not find the new escape-time catalog wave options in schema\n";
            return 1;
        }
        if (!foundDualSeedB || !foundDualSeedMix || !foundColoringModeControl || !foundColorGradingControl ||
            coloringModeControlCount != 1 ||
            foundColorSignalControl || foundColorPaletteControl ||
            colorSignalControlCount != 0 || colorPaletteControlCount != 0 || colorGradingControlCount != 1) {
            std::cerr << "Did not find the restored legacy public coloring_mode control surface in schema\n";
            return 1;
        }
    }

    {
        UISchema safeMode = BuildSafeModeSchema();
        const UISchemaPanel* viewPanel = FindPanelById(safeMode, "view");
        const UISchemaPanel* fractalPanel = FindPanelById(safeMode, "fractal");
        const UISchemaPanel* renderPanel = FindPanelById(safeMode, "render");
        bool foundRenderWidthDefault = false;
        bool foundRenderHeightDefault = false;
        bool foundFractalTypeCommonGroup = false;
        bool foundFractalTypeRootFindingGroup = false;
        bool foundCounterfactualPairRootFindingGroup = false;
        bool foundFractalTypeEscapeTimeGroup = false;
        bool foundFractalTypeExplainoGroup = false;
        bool foundFractalTypeExplainoAllGroup = false;
        bool foundFractalTypeExplainoAllFirst = false;
        bool foundSpiderEscapeTimeGroup = false;
        bool foundCelticEscapeTimeGroup = false;
        bool foundPerpendicularShipEscapeTimeGroup = false;
        bool foundFractalTypeDefaultExplainoAll = false;
        bool foundInteractionDebounceDefault = false;
        bool foundPreviewTargetFpsDefault = false;
        bool foundPreviewMinScaleDefault = false;
        bool foundContinuousRenderDefaultFalse = false;
        bool foundSafeModeCenterXUiRange = false;
        bool foundSafeModeCenterYUiRange = false;
        bool foundSafeModeZoomNoUiCap = false;
        bool foundSafeModeRotationUiRange = false;
        bool foundSafeModeMaxIterUiCap = false;
        std::size_t safeModeExplainoGroupOptionCount = 0;

        if (!viewPanel || viewPanel->label != "View (Safe Mode)" || !viewPanel->has_order || viewPanel->order != 10 ||
            viewPanel->controls.size() != 11) {
            std::cerr << "Safe-mode schema did not expose the expected view panel shape\n";
            return 1;
        }
        if (!fractalPanel || fractalPanel->label != "Fractal (Safe Mode)" || !fractalPanel->has_order || fractalPanel->order != 20 ||
            fractalPanel->controls.size() != 7) {
            std::cerr << "Safe-mode schema did not expose the expected fractal panel shape\n";
            return 1;
        }
        if (!renderPanel || renderPanel->label != "Render (Safe Mode)" || !renderPanel->has_order || renderPanel->order != 30 ||
            renderPanel->controls.size() != 5) {
            std::cerr << "Safe-mode schema did not expose the expected render panel shape\n";
            return 1;
        }

        const UISchemaControl* renderOnce = FindControlById(*viewPanel, "render_once");
        const UISchemaControl* resetView = FindControlById(*viewPanel, "reset_view");
        const UISchemaControl* resetAll = FindControlById(*viewPanel, "reset_all");
        const UISchemaControl* loadState = FindControlById(*viewPanel, "load_state");
        const UISchemaControl* captureFinding = FindControlById(*viewPanel, "capture_finding");
        if (!renderOnce || renderOnce->type != "button" || !renderOnce->has_binding || renderOnce->binding.kind != "action" ||
            renderOnce->binding.path != "fractal.actions.render_once") {
            std::cerr << "Safe-mode schema did not bind render_once to the expected action path\n";
            return 1;
        }
        if (!resetView || resetView->type != "button" || !resetView->has_binding || resetView->binding.kind != "action" ||
            resetView->binding.path != "fractal.actions.reset_view") {
            std::cerr << "Safe-mode schema did not bind reset_view to the expected action path\n";
            return 1;
        }
        if (!resetAll || resetAll->type != "button" || !resetAll->has_binding || resetAll->binding.kind != "action" ||
            resetAll->binding.path != "fractal.actions.reset_all") {
            std::cerr << "Safe-mode schema did not bind reset_all to the expected action path\n";
            return 1;
        }
        if (!loadState || loadState->type != "button" || !loadState->has_binding || loadState->binding.kind != "action" ||
            loadState->binding.path != "fractal.actions.load_state") {
            std::cerr << "Safe-mode schema did not bind load_state to the expected action path\n";
            return 1;
        }
        if (!captureFinding || captureFinding->type != "button" || !captureFinding->has_binding ||
            captureFinding->binding.kind != "action" || captureFinding->binding.path != "fractal.actions.capture_finding") {
            std::cerr << "Safe-mode schema did not bind capture_finding to the expected action path\n";
            return 1;
        }
        const UISchemaControl* pairRootFamily = FindControlById(*fractalPanel, "counterfactual_pair_root_family");
        const UISchemaControl* pairFrame = FindControlById(*fractalPanel, "counterfactual_pair_frame");
        const UISchemaControl* pairOffsetX = FindControlById(*fractalPanel, "counterfactual_pair_offset_x");
        const UISchemaControl* pairOffsetY = FindControlById(*fractalPanel, "counterfactual_pair_offset_y");
        const UISchemaControl* pairReconvergenceRatio = FindControlById(*fractalPanel, "counterfactual_pair_reconvergence_ratio");
        if (!pairRootFamily || !pairFrame || !pairOffsetX || !pairOffsetY || !pairReconvergenceRatio) {
            std::cerr << "Safe-mode schema did not expose the Counterfactual Pair hardening controls\n";
            return 1;
        }
        if (!pairRootFamily->has_binding || pairRootFamily->binding.path != "fractal.params.counterfactual_pair_root_family" ||
            !pairRootFamily->has_default || !pairRootFamily->def.is_string() || pairRootFamily->def.as_string() != "cubic_unit_roots" ||
            !pairRootFamily->has_visible_if || pairRootFamily->visible_if.value != "counterfactual_pair") {
            std::cerr << "Safe-mode schema Counterfactual Pair root-family control drifted from the bounded owner seam\n";
            return 1;
        }
        if (!pairFrame->has_binding || pairFrame->binding.path != "fractal.params.counterfactual_pair_frame" ||
            !pairFrame->has_default || !pairFrame->def.is_string() || pairFrame->def.as_string() != "world_absolute" ||
            !pairFrame->has_visible_if || pairFrame->visible_if.value != "counterfactual_pair") {
            std::cerr << "Safe-mode schema Counterfactual Pair frame control drifted from the bounded owner seam\n";
            return 1;
        }
        if (!pairOffsetX->has_binding || pairOffsetX->binding.path != "fractal.params.counterfactual_pair_offset_x" ||
            !pairOffsetX->has_default || !pairOffsetX->def.is_number() || pairOffsetX->def.as_number() != 0.16 ||
            !pairOffsetY->has_binding || pairOffsetY->binding.path != "fractal.params.counterfactual_pair_offset_y" ||
            !pairOffsetY->has_default || !pairOffsetY->def.is_number() || pairOffsetY->def.as_number() != 0.08) {
            std::cerr << "Safe-mode schema Counterfactual Pair offset controls drifted from the bounded owner seam\n";
            return 1;
        }
        if (!pairReconvergenceRatio->has_binding ||
            pairReconvergenceRatio->binding.path != "fractal.params.counterfactual_pair_reconvergence_ratio" ||
            !pairReconvergenceRatio->has_default || !pairReconvergenceRatio->def.is_number() ||
            pairReconvergenceRatio->def.as_number() != 0.6 ||
            !pairReconvergenceRatio->has_help ||
            pairReconvergenceRatio->help.find("Class 3") == std::string::npos) {
            std::cerr << "Safe-mode schema Counterfactual Pair reconvergence control did not expose the public class model\n";
            return 1;
        }

        for (const auto& panel : safeMode.panels) {
            for (const auto& ctrl : panel.controls) {
                if (ctrl.id == "fractal_type") {
                    if (ctrl.has_default && ctrl.def.is_string() && ctrl.def.as_string() == "explaino_all") {
                        foundFractalTypeDefaultExplainoAll = true;
                    }
                    for (std::size_t optionIndex = 0; optionIndex < ctrl.options.size(); ++optionIndex) {
                        const auto& option = ctrl.options[optionIndex];
                        if (option.id == "explaino" && option.group == "Common") {
                            foundFractalTypeCommonGroup = true;
                            const ExplainoSelectorDescriptor* descriptor = FindExplainoSelectorDescriptor(option.id);
                            if (!descriptor || descriptor->role != ExplainoSelectorRole::baseline) {
                                std::cerr << "Safe-mode schema did not classify baseline explaino through the canonical Explaino selector registry\n";
                                return 1;
                            }
                        }
                        if (option.id == "newton" && option.group == "Root-Finding") foundFractalTypeRootFindingGroup = true;
                        if (option.id == "counterfactual_pair" && option.group == "Root-Finding") foundCounterfactualPairRootFindingGroup = true;
                        if (option.id == "multibrot" && option.group == "Escape-Time") foundFractalTypeEscapeTimeGroup = true;
                        if (option.group == "Explaino") {
                            ++safeModeExplainoGroupOptionCount;
                            const ExplainoSelectorDescriptor* descriptor = FindExplainoSelectorDescriptor(option.id);
                            if (!descriptor || descriptor->role == ExplainoSelectorRole::baseline) {
                                std::cerr << "Safe-mode Explaino selector options drifted outside the canonical Explaino selector registry\n";
                                return 1;
                            }
                        }
                        if (option.id == "explaino_lambda" && option.group == "Explaino") foundFractalTypeExplainoGroup = true;
                        if (option.id == "explaino_all" && option.group == "Explaino") foundFractalTypeExplainoAllGroup = true;
                        if (!foundFractalTypeExplainoAllFirst && option.group == "Explaino") {
                            foundFractalTypeExplainoAllFirst = option.id == "explaino_all";
                        }
                        if (option.id == "spider" && option.group == "Escape-Time") foundSpiderEscapeTimeGroup = true;
                        if (option.id == "celtic_mandelbrot" && option.group == "Escape-Time") foundCelticEscapeTimeGroup = true;
                        if (option.id == "perpendicular_burning_ship" && option.group == "Escape-Time") foundPerpendicularShipEscapeTimeGroup = true;
                    }
                }
                if (ctrl.id == "width" && ctrl.has_default && ctrl.def.is_number() && ctrl.def.as_number() == 2048.0) {
                    foundRenderWidthDefault = true;
                }
                if (ctrl.id == "height" && ctrl.has_default && ctrl.def.is_number() && ctrl.def.as_number() == 1536.0) {
                    foundRenderHeightDefault = true;
                }
                if (ctrl.id == "interaction_debounce_ms" && ctrl.has_default && ctrl.def.is_number() && ctrl.def.as_number() == 200.0) {
                    foundInteractionDebounceDefault = true;
                }
                if (ctrl.id == "preview_target_fps" && ctrl.has_default && ctrl.def.is_number() && ctrl.def.as_number() == 30.0) {
                    foundPreviewTargetFpsDefault = true;
                }
                if (ctrl.id == "preview_min_scale" && ctrl.has_default && ctrl.def.is_number() && ctrl.def.as_number() == 0.5) {
                    foundPreviewMinScaleDefault = true;
                }
                if (ctrl.id == "auto_refresh" && ctrl.label == "Continuous Render" && ctrl.has_default && ctrl.def.is_bool() && !ctrl.def.as_bool()) {
                    foundContinuousRenderDefaultFalse = true;
                }
                if (ctrl.id == "center_x" && ctrl.has_ui_min && ctrl.ui_min == -2.0 && ctrl.has_ui_max && ctrl.ui_max == 2.0 && !ctrl.has_min && !ctrl.has_max) {
                    foundSafeModeCenterXUiRange = true;
                }
                if (ctrl.id == "center_y" && ctrl.has_ui_min && ctrl.ui_min == -2.0 && ctrl.has_ui_max && ctrl.ui_max == 2.0 && !ctrl.has_min && !ctrl.has_max) {
                    foundSafeModeCenterYUiRange = true;
                }
                if (ctrl.id == "zoom" && ctrl.has_min && ctrl.min == 1e-12 && !ctrl.has_ui_min &&
                    !ctrl.has_ui_max && !ctrl.has_max && ctrl.logarithmic) {
                    foundSafeModeZoomNoUiCap = true;
                }
                if (ctrl.id == "rotation_deg" && ctrl.has_ui_min && ctrl.ui_min == -180.0 && ctrl.has_ui_max && ctrl.ui_max == 180.0 &&
                    !ctrl.has_min && !ctrl.has_max) {
                    foundSafeModeRotationUiRange = true;
                }
                if (ctrl.id == "max_iter" && ctrl.has_min && ctrl.min == 1.0 && ctrl.has_ui_min && ctrl.ui_min == 1.0 &&
                    ctrl.has_ui_max && ctrl.ui_max == 5000.0 && !ctrl.has_max) {
                    foundSafeModeMaxIterUiCap = true;
                }
            }
        }
        if (!foundRenderWidthDefault || !foundRenderHeightDefault) {
            std::cerr << "Safe-mode schema did not inherit exploration-first render resolution defaults\n";
            return 1;
        }
        if (!foundInteractionDebounceDefault || !foundPreviewTargetFpsDefault || !foundPreviewMinScaleDefault) {
            std::cerr << "Safe-mode schema did not expose the adaptive preview pacing controls with the expected defaults\n";
            return 1;
        }
        if (!foundFractalTypeCommonGroup || !foundFractalTypeRootFindingGroup || !foundCounterfactualPairRootFindingGroup || !foundFractalTypeEscapeTimeGroup ||
            !foundFractalTypeExplainoGroup || !foundFractalTypeExplainoAllGroup) {
            std::cerr << "Safe-mode schema did not expose grouped fractal selector categories including the canonical Explaino-all entry\n";
            return 1;
        }
        if (!foundFractalTypeExplainoAllFirst) {
            std::cerr << "Safe-mode schema did not place Explaino-all first in the Explaino selector group\n";
            return 1;
        }
        if (safeModeExplainoGroupOptionCount != kExpectedExplainoGroupedSelectorCount) {
            std::cerr << "Safe-mode Explaino selector group count drifted away from the canonical Explaino selector registry\n";
            return 1;
        }
        if (!foundFractalTypeDefaultExplainoAll) {
            std::cerr << "Safe-mode schema did not default startup fractal selection to Explaino-all\n";
            return 1;
        }
        if (!foundSpiderEscapeTimeGroup || !foundCelticEscapeTimeGroup || !foundPerpendicularShipEscapeTimeGroup) {
            std::cerr << "Safe-mode schema did not expose the new escape-time catalog wave options\n";
            return 1;
        }
        if (!foundContinuousRenderDefaultFalse) {
            std::cerr << "Safe-mode schema did not expose the disabled-by-default continuous-render control\n";
            return 1;
        }
        if (!foundSafeModeCenterXUiRange || !foundSafeModeCenterYUiRange || !foundSafeModeZoomNoUiCap ||
            !foundSafeModeRotationUiRange || !foundSafeModeMaxIterUiCap) {
            std::cerr << "Safe-mode schema did not inherit the MVP soft-range control updates\n";
            return 1;
        }
    }

    std::cout << "test_ui_schema: all passed\n";
    return 0;
}
