#pragma once

#include "function_descriptor.h"
#include "imgui.h"
#include "imgui_stack_editor.h"

#include <cmath>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

struct ColorPipelineParamState {
    std::string path;
    std::string type;
    double number_value = 0.0;
    bool bool_value = false;
    std::string enum_value;
};

struct ColorPipelineLaneState {
    std::string lane_id;
    std::string label;
    std::uint64_t ui_row_id = 0;
    std::string function_id;
    std::vector<ColorPipelineParamState> parameter_values;
};

struct ColorPipelineWindowState {
    bool open = false;
    bool initialized = false;
    std::uint64_t next_row_id = 1;
    std::vector<ColorPipelineLaneState> lanes;
    std::vector<std::string> validation_messages;
};

struct ColorPipelineLaneCatalog {
    const char* lane_id = "";
    const char* label = "";
    const char* default_function_id = "";
    std::vector<FunctionDescriptor> functions;
};

inline json_min::Value MakeColorPipelineNumberValue(double value) {
    json_min::Value out;
    out.v = value;
    return out;
}

inline json_min::Value MakeColorPipelineBoolValue(bool value) {
    json_min::Value out;
    out.v = value;
    return out;
}

inline FunctionParamDescriptor MakeColorPipelineFloatParam(
    const char* path,
    const char* label,
    const char* help,
    double minValue,
    double maxValue,
    double stepValue,
    double defaultValue) {
    FunctionParamDescriptor param;
    param.path = path ? path : "";
    param.type = "float";
    param.label = label ? label : "";
    param.help = help ? help : "";
    param.has_min = true;
    param.min_value = minValue;
    param.has_max = true;
    param.max_value = maxValue;
    param.has_step = true;
    param.step_value = stepValue;
    param.has_default = true;
    param.default_value = MakeColorPipelineNumberValue(defaultValue);
    return param;
}

inline FunctionParamDescriptor MakeColorPipelineIntParam(
    const char* path,
    const char* label,
    const char* help,
    int minValue,
    int maxValue,
    int stepValue,
    int defaultValue) {
    FunctionParamDescriptor param;
    param.path = path ? path : "";
    param.type = "int";
    param.label = label ? label : "";
    param.help = help ? help : "";
    param.has_min = true;
    param.min_value = (double)minValue;
    param.has_max = true;
    param.max_value = (double)maxValue;
    param.has_step = true;
    param.step_value = (double)stepValue;
    param.has_default = true;
    param.default_value = MakeColorPipelineNumberValue((double)defaultValue);
    return param;
}

inline FunctionParamDescriptor MakeColorPipelineBoolParam(
    const char* path,
    const char* label,
    const char* help,
    bool defaultValue) {
    FunctionParamDescriptor param;
    param.path = path ? path : "";
    param.type = "bool";
    param.label = label ? label : "";
    param.help = help ? help : "";
    param.has_default = true;
    param.default_value = MakeColorPipelineBoolValue(defaultValue);
    return param;
}

inline FunctionDescriptor MakeColorPipelineFunction(
    const char* id,
    const char* name,
    const char* description,
    std::vector<FunctionParamDescriptor> parameters) {
    FunctionDescriptor descriptor;
    descriptor.id = id ? id : "";
    descriptor.name = name ? name : "";
    descriptor.description = description ? description : "";
    descriptor.parameters = std::move(parameters);
    return descriptor;
}

inline const std::vector<ColorPipelineLaneCatalog>& GetColorPipelineLaneCatalogs() {
    static const std::vector<ColorPipelineLaneCatalog> catalogs = {
        {
            "signal",
            "Signal",
            "smooth_escape_ramp",
            {
                MakeColorPipelineFunction(
                    "smooth_escape_ramp",
                    "Smooth Escape Ramp",
                    "Map escape continuity into a tunable ramp before palette lookup.",
                    {
                        MakeColorPipelineFloatParam("signal.scale", "Scale", "Amplify or compress the escape ramp.", 0.25, 4.0, 0.01, 1.0),
                        MakeColorPipelineFloatParam("signal.bias", "Bias", "Shift the ramp before downstream palette work.", -1.0, 1.0, 0.01, 0.0),
                    }),
                MakeColorPipelineFunction(
                    "phase_orbit",
                    "Phase Orbit",
                    "Use orbit angle as the primary signal with controllable wrap and phase offset.",
                    {
                        MakeColorPipelineFloatParam("signal.phase_offset", "Phase Offset", "Rotate the sampled phase before palette lookup.", -3.141592653589793, 3.141592653589793, 0.01, 0.0),
                        MakeColorPipelineFloatParam("signal.hue_cycles", "Hue Cycles", "Control how many palette cycles appear across one full rotation.", 0.5, 6.0, 0.01, 1.0),
                    }),
                MakeColorPipelineFunction(
                    "iteration_bands",
                    "Iteration Bands",
                    "Quantize the escape signal into banded steps with optional soft edges.",
                    {
                        MakeColorPipelineIntParam("signal.band_count", "Band Count", "Choose how many bands to carve out of the escape signal.", 2, 24, 1, 8),
                        MakeColorPipelineFloatParam("signal.softness", "Softness", "Blend between hard posterization and soft band transitions.", 0.0, 1.0, 0.01, 0.35),
                        MakeColorPipelineFloatParam("signal.gain", "Gain", "Scale the banded result before palette mapping.", 0.25, 4.0, 0.01, 1.2),
                    }),
            },
        },
        {
            "palette",
            "Palette",
            "phase_wheel",
            {
                MakeColorPipelineFunction(
                    "classic_basin",
                    "Classic Basin",
                    "Blend root colors with a small edge mix for basin readability.",
                    {
                        MakeColorPipelineFloatParam("palette.edge_mix", "Edge Mix", "Add a controlled amount of boundary tinting to basin colors.", 0.0, 1.0, 0.01, 0.2),
                    }),
                MakeColorPipelineFunction(
                    "phase_wheel",
                    "Phase Wheel",
                    "Wrap signal values around a hue wheel with adjustable offset and saturation.",
                    {
                        MakeColorPipelineFloatParam("palette.phase_offset", "Phase Offset", "Rotate the wheel before it is applied to the incoming signal.", -3.141592653589793, 3.141592653589793, 0.01, 0.0),
                        MakeColorPipelineFloatParam("palette.saturation", "Saturation", "Push or soften color separation in the wheel.", 0.0, 2.0, 0.01, 1.15),
                    }),
                MakeColorPipelineFunction(
                    "heatmap",
                    "Heatmap",
                    "Map the signal through a warm-to-hot gradient for dense escape structure.",
                    {
                        MakeColorPipelineFloatParam("palette.warmth", "Warmth", "Shift the gradient toward yellow-red highlights.", 0.0, 2.0, 0.01, 1.0),
                        MakeColorPipelineFloatParam("palette.contrast", "Contrast", "Increase separation between neighboring heat bands.", 0.0, 2.0, 0.01, 1.0),
                    }),
            },
        },
        {
            "grade",
            "Grade",
            "contrast_lift",
            {
                MakeColorPipelineFunction(
                    "contrast_lift",
                    "Contrast Lift",
                    "Apply final exposure and contrast shaping to the composed color signal.",
                    {
                        MakeColorPipelineFloatParam("grade.exposure", "Exposure", "Set the overall brightness of the final output.", 0.1, 3.0, 0.01, 1.0),
                        MakeColorPipelineFloatParam("grade.contrast", "Contrast", "Stretch mid-tones after palette mapping.", 0.0, 3.0, 0.01, 1.1),
                    }),
                MakeColorPipelineFunction(
                    "soft_clamp",
                    "Soft Clamp",
                    "Roll highlights and shadows gently toward a clamped display range.",
                    {
                        MakeColorPipelineFloatParam("grade.shoulder", "Shoulder", "Compress highlights before they clip.", 0.0, 1.0, 0.01, 0.35),
                        MakeColorPipelineFloatParam("grade.toe", "Toe", "Preserve shadow detail while deepening blacks.", 0.0, 1.0, 0.01, 0.2),
                    }),
                MakeColorPipelineFunction(
                    "glow_rolloff",
                    "Glow Rolloff",
                    "Add controlled bloom-style emphasis while protecting dark regions.",
                    {
                        MakeColorPipelineFloatParam("grade.glow", "Glow", "Control the amount of highlight glow added to the output.", 0.0, 2.0, 0.01, 0.5),
                        MakeColorPipelineBoolParam("grade.preserve_blacks", "Preserve Blacks", "Keep shadow tones anchored while glow is applied.", true),
                    }),
            },
        },
    };
    return catalogs;
}

inline const ColorPipelineLaneCatalog* FindColorPipelineLaneCatalog(const std::string& laneId) {
    for (const ColorPipelineLaneCatalog& catalog : GetColorPipelineLaneCatalogs()) {
        if (catalog.lane_id == laneId) {
            return &catalog;
        }
    }
    return nullptr;
}

inline const FunctionDescriptor* FindColorPipelineFunctionDescriptor(
    const ColorPipelineLaneCatalog& catalog,
    const std::string& functionId) {
    for (const FunctionDescriptor& descriptor : catalog.functions) {
        if (descriptor.id == functionId) {
            return &descriptor;
        }
    }
    return nullptr;
}

inline double ResolveColorPipelineNumericDefault(const FunctionParamDescriptor& param) {
    if (param.has_default && param.default_value.is_number()) {
        return param.default_value.as_number();
    }
    if (param.has_min && param.has_max) {
        return (param.min_value + param.max_value) * 0.5;
    }
    return 0.0;
}

inline bool ResolveColorPipelineBoolDefault(const FunctionParamDescriptor& param) {
    if (param.has_default && param.default_value.is_bool()) {
        return param.default_value.as_bool();
    }
    return false;
}

inline std::string ResolveColorPipelineEnumDefault(const FunctionParamDescriptor& param) {
    if (param.has_default && param.default_value.is_string()) {
        return param.default_value.as_string();
    }
    if (!param.options.empty()) {
        return param.options.front().id;
    }
    return {};
}

inline void ClearColorPipelineValidationMessages(ColorPipelineWindowState* ioState) {
    if (!ioState) {
        return;
    }
    ioState->validation_messages.clear();
}

inline void PushColorPipelineValidationMessage(ColorPipelineWindowState* ioState, const std::string& message) {
    if (!ioState || message.empty()) {
        return;
    }
    ioState->validation_messages.push_back(message);
}

inline bool SetColorPipelineLaneFunction(
    ColorPipelineWindowState* ioState,
    ColorPipelineLaneState* ioLane,
    const FunctionDescriptor& descriptor) {
    if (!ioState || !ioLane) {
        return false;
    }

    ioLane->function_id = descriptor.id;
    ioLane->parameter_values.clear();
    ioLane->parameter_values.reserve(descriptor.parameters.size());

    for (const FunctionParamDescriptor& param : descriptor.parameters) {
        ColorPipelineParamState value;
        value.path = param.path;
        value.type = param.type;
        if (param.type == "bool") {
            value.bool_value = ResolveColorPipelineBoolDefault(param);
        } else if (param.type == "enum") {
            value.enum_value = ResolveColorPipelineEnumDefault(param);
        } else {
            value.number_value = ResolveColorPipelineNumericDefault(param);
        }
        ioLane->parameter_values.push_back(std::move(value));
    }

    return true;
}

inline bool EnsureColorPipelineWindowInitialized(ColorPipelineWindowState* ioState) {
    if (!ioState) {
        return false;
    }

    if (ioState->initialized) {
        for (ColorPipelineLaneState& lane : ioState->lanes) {
            if (!EnsureImGuiStackEditorRowId(&lane.ui_row_id, &ioState->next_row_id)) {
                return false;
            }
        }
        return true;
    }

    ioState->lanes.clear();
    ClearColorPipelineValidationMessages(ioState);
    for (const ColorPipelineLaneCatalog& catalog : GetColorPipelineLaneCatalogs()) {
        ColorPipelineLaneState lane;
        lane.lane_id = catalog.lane_id;
        lane.label = catalog.label;
        if (!EnsureImGuiStackEditorRowId(&lane.ui_row_id, &ioState->next_row_id)) {
            return false;
        }

        const FunctionDescriptor* descriptor = FindColorPipelineFunctionDescriptor(catalog, catalog.default_function_id);
        if (!descriptor) {
            PushColorPipelineValidationMessage(ioState,
                std::string("Missing default advanced color function for lane: ") + catalog.label);
            return false;
        }
        if (!SetColorPipelineLaneFunction(ioState, &lane, *descriptor)) {
            return false;
        }
        ioState->lanes.push_back(std::move(lane));
    }
    ioState->initialized = true;
    return true;
}

inline bool SelectColorPipelineLaneFunction(
    ColorPipelineWindowState* ioState,
    std::size_t laneIndex,
    const char* functionId) {
    if (!ioState || !functionId || functionId[0] == '\0') {
        return false;
    }
    if (!EnsureColorPipelineWindowInitialized(ioState)) {
        return false;
    }
    if (laneIndex >= ioState->lanes.size()) {
        PushColorPipelineValidationMessage(ioState, "Advanced color pipeline lane index was out of range.");
        return false;
    }

    ColorPipelineLaneState& lane = ioState->lanes[laneIndex];
    const ColorPipelineLaneCatalog* catalog = FindColorPipelineLaneCatalog(lane.lane_id);
    if (!catalog) {
        PushColorPipelineValidationMessage(ioState,
            std::string("Unknown advanced color pipeline lane id: ") + lane.lane_id);
        return false;
    }

    const FunctionDescriptor* descriptor = FindColorPipelineFunctionDescriptor(*catalog, functionId);
    if (!descriptor) {
        PushColorPipelineValidationMessage(ioState,
            std::string("Unknown advanced color function '") + functionId + "' for lane " + lane.label);
        return false;
    }

    return SetColorPipelineLaneFunction(ioState, &lane, *descriptor);
}

inline bool RenderColorPipelineParamControl(
    const FunctionParamDescriptor& param,
    ColorPipelineParamState* ioValue) {
    if (!ioValue) {
        return false;
    }

    bool changed = false;
    ImGui::PushID(param.path.c_str());
    if (param.type == "float" || param.type == "double") {
        float value = static_cast<float>(ioValue->number_value);
        if (param.has_min && param.has_max) {
            changed = ImGui::SliderFloat(param.label.c_str(), &value,
                static_cast<float>(param.min_value),
                static_cast<float>(param.max_value));
        } else {
            const float step = param.has_step ? static_cast<float>(param.step_value) : 0.01f;
            changed = ImGui::DragFloat(param.label.c_str(), &value, step);
        }
        if (changed) {
            ioValue->number_value = value;
        }
    } else if (param.type == "int") {
        int value = static_cast<int>(std::lround(ioValue->number_value));
        if (param.has_min && param.has_max) {
            changed = ImGui::SliderInt(param.label.c_str(), &value,
                static_cast<int>(param.min_value),
                static_cast<int>(param.max_value));
        } else {
            const float step = param.has_step ? static_cast<float>(param.step_value) : 1.0f;
            changed = ImGui::DragInt(param.label.c_str(), &value, step);
        }
        if (changed) {
            ioValue->number_value = static_cast<double>(value);
        }
    } else if (param.type == "bool") {
        bool value = ioValue->bool_value;
        changed = ImGui::Checkbox(param.label.c_str(), &value);
        if (changed) {
            ioValue->bool_value = value;
        }
    } else if (param.type == "enum") {
        const char* preview = ioValue->enum_value.empty() ? "(select)" : ioValue->enum_value.c_str();
        if (ImGui::BeginCombo(param.label.c_str(), preview)) {
            for (const UISchemaOption& option : param.options) {
                const bool isSelected = (option.id == ioValue->enum_value);
                if (ImGui::Selectable(option.label.c_str(), isSelected)) {
                    ioValue->enum_value = option.id;
                    changed = true;
                }
                if (isSelected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
    } else {
        ImGui::TextUnformatted(param.label.c_str());
        ImGui::SameLine();
        ImGui::TextDisabled("Unsupported param type: %s", param.type.c_str());
    }

    if (!param.help.empty()) {
        ImGui::TextDisabled("%s", param.help.c_str());
    }
    ImGui::PopID();
    return changed;
}

inline void OpenColorPipelineWindow(ColorPipelineWindowState* ioState) {
    if (!ioState) {
        return;
    }
    ioState->open = true;
}

inline bool RenderColorPipelineWindow(ColorPipelineWindowState* ioState) {
    if (!ioState || !ioState->open) {
        return false;
    }
    if (!EnsureColorPipelineWindowInitialized(ioState)) {
        return false;
    }

    bool open = ioState->open;
    ImGui::SetNextWindowSize(ImVec2(720.0f, 520.0f), ImGuiCond_FirstUseEver);
    const bool began = ImGui::Begin("Color Pipeline", &open);
    if (began) {
        ImGui::TextWrapped("Draft only: this advanced color-pipeline editor does not yet drive the renderer.");
        ImGui::Separator();
        ImGui::TextWrapped("The public Color panel remains the live authority while this window previews the fixed three-segment editor shape.");
        if (!ioState->validation_messages.empty()) {
            ImGui::Spacing();
            RenderImGuiStackEditorValidationBox("Advanced Color Pipeline", ioState->validation_messages);
            ImGui::Spacing();
        }

        for (std::size_t laneIndex = 0; laneIndex < ioState->lanes.size(); ++laneIndex) {
            ColorPipelineLaneState& lane = ioState->lanes[laneIndex];
            const ColorPipelineLaneCatalog* catalog = FindColorPipelineLaneCatalog(lane.lane_id);
            if (!catalog) {
                PushColorPipelineValidationMessage(ioState,
                    std::string("Unknown advanced color pipeline lane id: ") + lane.lane_id);
                continue;
            }
            const FunctionDescriptor* descriptor = FindColorPipelineFunctionDescriptor(*catalog, lane.function_id);
            if (!descriptor) {
                PushColorPipelineValidationMessage(ioState,
                    std::string("Unknown advanced color function '") + lane.function_id + "' for lane " + lane.label);
                continue;
            }
            const FunctionDescriptor* currentDescriptor = descriptor;

            const std::string headerLabel = lane.label + ": " + currentDescriptor->name;
            ImGuiStackEditorRowChromeSpec rowSpec;
            rowSpec.tree_node_id = lane.lane_id.c_str();
            rowSpec.header_label = headerLabel.c_str();
            rowSpec.stable_row_id = lane.ui_row_id;
            rowSpec.allow_remove = false;
            rowSpec.allow_move_up = false;
            rowSpec.allow_move_down = false;

            RenderImGuiStackEditorRowChrome(rowSpec, [&]() {
                const char* comboPreview = currentDescriptor->name.empty() ? "(select)" : currentDescriptor->name.c_str();
                if (ImGui::BeginCombo("Function", comboPreview)) {
                    for (const FunctionDescriptor& candidate : catalog->functions) {
                        const bool isSelected = (candidate.id == lane.function_id);
                        if (ImGui::Selectable(candidate.name.c_str(), isSelected)) {
                            ClearColorPipelineValidationMessages(ioState);
                            if (SelectColorPipelineLaneFunction(ioState, laneIndex, candidate.id.c_str())) {
                                currentDescriptor = FindColorPipelineFunctionDescriptor(*catalog, lane.function_id);
                            }
                        }
                        if (isSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                    ImGui::EndCombo();
                }
                if (!currentDescriptor) {
                    return;
                }
                if (!currentDescriptor->description.empty()) {
                    ImGui::TextWrapped("%s", currentDescriptor->description.c_str());
                }
                for (std::size_t paramIndex = 0; paramIndex < currentDescriptor->parameters.size() &&
                        paramIndex < lane.parameter_values.size();
                     ++paramIndex) {
                    RenderColorPipelineParamControl(currentDescriptor->parameters[paramIndex], &lane.parameter_values[paramIndex]);
                }
            });
            ImGui::Spacing();
        }
    }
    ImGui::End();

    ioState->open = open;
    return true;
}