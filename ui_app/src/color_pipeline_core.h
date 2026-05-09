#pragma once

#include "fractal_types.h"
#include "function_descriptor.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>
#include <utility>
#include <vector>

struct ColorPipelineLaneCatalog {
    const char* lane_id = "";
    const char* label = "";
    const char* default_function_id = "";
    std::vector<FunctionDescriptor> functions;
};

struct ColorPipelineParamState {
    std::string path;
    std::string type;
    double number_value = 0.0;
    bool bool_value = false;
    std::string enum_value;
};

struct ColorPipelineRowState {
    std::uint64_t ui_row_id = 0;
    bool enabled = true;
    std::string function_id;
    std::vector<ColorPipelineParamState> parameter_values;
};

struct ColorPipelineLaneState {
    std::string lane_id;
    std::string label;
    std::vector<ColorPipelineRowState> rows;
};

struct ColorPipelineLiveSnapshot {
    bool valid = false;
    bool draft_import_supported = false;
    FractalType fractal_type = FractalType::explaino;
    ColoringMode coloring_mode = ColoringMode::root_basin;
    ColorPipelineSelection pipeline{};
    std::vector<ColorPipelineLaneState> lanes;
};

namespace color_pipeline_core {

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
    param.min_value = static_cast<double>(minValue);
    param.has_max = true;
    param.max_value = static_cast<double>(maxValue);
    param.has_step = true;
    param.step_value = static_cast<double>(stepValue);
    param.has_default = true;
    param.default_value = MakeColorPipelineNumberValue(static_cast<double>(defaultValue));
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

inline const char* AdvancedColorSignalFunctionId(ColorSignal value) {
    switch (value) {
    case ColorSignal::smooth_escape:
        return "smooth_escape_ramp";
    case ColorSignal::phase_angle:
        return "phase_orbit";
    case ColorSignal::iteration_bands:
        return "banded_signal";
    case ColorSignal::escape_magnitude:
        return "escape_magnitude";
    case ColorSignal::orbit_stripe:
        return "orbit_stripe";
    case ColorSignal::root_proximity:
        return "root_proximity";
    case ColorSignal::root_index:
        return "root_index";
    }
    return nullptr;
}

inline bool TryParseAdvancedColorSignalFunctionId(const std::string& functionId, ColorSignal* outValue) {
    if (functionId == "smooth_escape_ramp") {
        if (outValue) *outValue = ColorSignal::smooth_escape;
        return true;
    }
    if (functionId == "phase_orbit") {
        if (outValue) *outValue = ColorSignal::phase_angle;
        return true;
    }
    if (functionId == "banded_signal") {
        if (outValue) *outValue = ColorSignal::iteration_bands;
        return true;
    }
    if (functionId == "escape_magnitude") {
        if (outValue) *outValue = ColorSignal::escape_magnitude;
        return true;
    }
    if (functionId == "orbit_stripe") {
        if (outValue) *outValue = ColorSignal::orbit_stripe;
        return true;
    }
    if (functionId == "root_proximity") {
        if (outValue) *outValue = ColorSignal::root_proximity;
        return true;
    }
    if (functionId == "root_index") {
        if (outValue) *outValue = ColorSignal::root_index;
        return true;
    }
    return false;
}

inline const char* AdvancedColorPaletteFunctionId(ColorPalette value) {
    switch (value) {
    case ColorPalette::root_classic:
        return "root_classic_palette";
    case ColorPalette::joy:
        return "joy_root_palette";
    case ColorPalette::cyclic_escape:
        return "heatmap";
    case ColorPalette::phase_wheel:
        return "phase_wheel_palette";
    case ColorPalette::banded_escape:
        return "banded_heatmap";
    case ColorPalette::explaino_cmap:
        return "explaino_cmap";
    }
    return nullptr;
}

inline bool TryParseAdvancedColorPaletteFunctionId(const std::string& functionId, ColorPalette* outValue) {
    if (functionId == "root_classic_palette") {
        if (outValue) *outValue = ColorPalette::root_classic;
        return true;
    }
    if (functionId == "joy_root_palette") {
        if (outValue) *outValue = ColorPalette::joy;
        return true;
    }
    if (functionId == "heatmap") {
        if (outValue) *outValue = ColorPalette::cyclic_escape;
        return true;
    }
    if (functionId == "phase_wheel_palette") {
        if (outValue) *outValue = ColorPalette::phase_wheel;
        return true;
    }
    if (functionId == "banded_heatmap") {
        if (outValue) *outValue = ColorPalette::banded_escape;
        return true;
    }
    if (functionId == "explaino_cmap") {
        if (outValue) *outValue = ColorPalette::explaino_cmap;
        return true;
    }
    return false;
}

inline const char* AdvancedColorGradingFunctionId(ColorGradingPreset value) {
    switch (value) {
    case ColorGradingPreset::escape_default:
        return "contrast_lift";
    case ColorGradingPreset::phase_default:
        return "phase_finish";
    case ColorGradingPreset::bands_default:
        return "band_finish";
    }
    return nullptr;
}

inline bool TryParseAdvancedColorGradingFunctionId(const std::string& functionId, ColorGradingPreset* outValue) {
    if (functionId == "contrast_lift") {
        if (outValue) *outValue = ColorGradingPreset::escape_default;
        return true;
    }
    if (functionId == "phase_finish") {
        if (outValue) *outValue = ColorGradingPreset::phase_default;
        return true;
    }
    if (functionId == "band_finish") {
        if (outValue) *outValue = ColorGradingPreset::bands_default;
        return true;
    }
    return false;
}

inline const char* AdvancedColorShapeFunctionId(ColorPipelineShape value) {
    switch (value) {
    case ColorPipelineShape::identity:
        return "identity";
    case ColorPipelineShape::offset_scale:
        return "offset_scale";
    case ColorPipelineShape::repeat:
        return "repeat";
    case ColorPipelineShape::posterize:
        return "posterize";
    case ColorPipelineShape::mirror_repeat:
        return "mirror_repeat";
    case ColorPipelineShape::bias_gain_curve:
        return "bias_gain_curve";
    case ColorPipelineShape::smooth_window:
        return "smooth_window";
    }
    return nullptr;
}

inline std::vector<FunctionDescriptor> BuildColorPipelineSignalFunctions() {
    return {
        MakeColorPipelineFunction(
            "smooth_escape_ramp",
            "Smooth Escape Ramp",
            "Use the continuous escape estimate as the upstream color signal.",
            {
                MakeColorPipelineFloatParam("signal.scale", "Scale", "Expand or compress the smooth escape ramp.", 0.25, 4.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("signal.bias", "Bias", "Shift the smooth escape ramp before palette lookup.", -1.0, 1.0, 0.01, 0.0),
            }),
        MakeColorPipelineFunction(
            "phase_orbit",
            "Phase Orbit",
            "Use orbit phase as the upstream signal.",
            {
                MakeColorPipelineFloatParam("signal.phase_offset", "Phase Offset", "Rotate the sampled phase before downstream palette work.", -3.141592653589793, 3.141592653589793, 0.01, 0.0),
                MakeColorPipelineFloatParam("signal.wrap_cycles", "Wrap Cycles", "Control how many hue cycles appear across one full rotation.", 0.5, 6.0, 0.01, 1.0),
            }),
        MakeColorPipelineFunction(
            "banded_signal",
            "Iteration Bands",
            "Quantize the escape signal into stepped iteration bands.",
            {
                MakeColorPipelineIntParam("signal.band_count", "Band Count", "Choose how many bands to carve out of the escape signal.", 2, 24, 1, 8),
                MakeColorPipelineFloatParam("signal.softness", "Softness", "Blend between hard posterization and soft band transitions.", 0.0, 1.0, 0.01, 0.35),
            }),
        MakeColorPipelineFunction(
            "escape_magnitude",
            "Escape Magnitude",
            "Use the orbit magnitude as the upstream escape-time source.",
            {
                MakeColorPipelineFloatParam("signal.magnitude_scale", "Magnitude Scale", "Expand or compress the escape-magnitude source before palette lookup.", 0.25, 4.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("signal.magnitude_bias", "Magnitude Bias", "Shift the escape-magnitude source before palette lookup.", -1.0, 1.0, 0.01, 0.0),
            }),
        MakeColorPipelineFunction(
            "orbit_stripe",
            "Orbit Stripe",
            "Fold orbit phase into a controllable stripe source before palette lookup.",
            {
                MakeColorPipelineFloatParam("signal.stripe_frequency", "Stripe Frequency", "Control how often orbit stripes repeat around the phase wheel.", 0.25, 12.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("signal.phase_offset", "Phase Offset", "Offset the stripe wave before palette lookup.", -3.141592653589793, 3.141592653589793, 0.01, 0.0),
            }),
        MakeColorPipelineFunction(
            "root_proximity",
            "Root Proximity",
            "Use nearest-root proximity as the upstream source on basin-capable fractal families.",
            {
                MakeColorPipelineFloatParam("signal.proximity_scale", "Proximity Scale", "Control how quickly root proximity falls off away from a root.", 0.25, 8.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("signal.proximity_bias", "Proximity Bias", "Shift the root-proximity source before palette lookup.", -1.0, 1.0, 0.01, 0.0),
            }),
        MakeColorPipelineFunction(
            "root_index",
            "Root Index",
            "Use the resolved nearest-root classification index as the upstream basin source.",
            {}),
    };
}

inline std::vector<FunctionDescriptor> BuildColorPipelinePaletteFunctions() {
    return {
        MakeColorPipelineFunction(
            "heatmap",
            "Heatmap",
            "Wrap escape signals through the cyclic escape palette.",
            {
                MakeColorPipelineFloatParam("palette.cycle_scale", "Cycle Scale", "Control how quickly the palette repeats across the signal.", 0.25, 4.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("palette.saturation", "Saturation", "Push or soften color separation in the cyclic palette.", 0.0, 2.0, 0.01, 1.0),
            }),
        MakeColorPipelineFunction(
            "phase_wheel_palette",
            "Phase Wheel",
            "Wrap signal values around a phase wheel palette.",
            {
                MakeColorPipelineFloatParam("palette.phase_offset", "Phase Offset", "Rotate the wheel before it is applied to the incoming signal.", -3.141592653589793, 3.141592653589793, 0.01, 0.0),
                MakeColorPipelineFloatParam("palette.saturation", "Saturation", "Push or soften color separation in the wheel.", 0.0, 2.0, 0.01, 1.15),
            }),
        MakeColorPipelineFunction(
            "banded_heatmap",
            "Banded Heatmap",
            "Map banded escape signals through the runtime band palette.",
            {
                MakeColorPipelineFloatParam("palette.band_emphasis", "Band Emphasis", "Increase or relax the contrast between neighboring bands.", 0.0, 2.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("palette.phase_offset", "Phase Offset", "Offset the band palette without changing the source signal.", -3.141592653589793, 3.141592653589793, 0.01, 0.0),
            }),
        MakeColorPipelineFunction(
            "explaino_cmap",
            "Explaino CMap",
            "Map scalar signals through the legacy ExplainO seed colormap lineage.",
            {
                MakeColorPipelineFloatParam("palette.seed_scale", "Seed Scale", "Control how quickly the ExplainO seed palette cycles across the incoming signal.", 0.25, 4.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("palette.seed_phase", "Seed Phase", "Rotate the ExplainO seed palette without changing the upstream signal.", -1.0, 1.0, 0.01, 0.0),
                MakeColorPipelineFloatParam("palette.colorfulness", "Colorfulness", "Blend between the raw ExplainO seed channels and the full nonlinear legacy color transform.", 0.0, 1.0, 0.01, 1.0),
            }),
        MakeColorPipelineFunction(
            "root_classic_palette",
            "Root Classic Palette",
            "Materialize basin root classification through the existing root-classic palette lineage.",
            {}),
        MakeColorPipelineFunction(
            "joy_root_palette",
            "Joy Root Palette",
            "Materialize basin root classification through the existing joy-basins palette lineage.",
            {}),
    };
}

inline std::vector<FunctionDescriptor> BuildColorPipelineShapeFunctions() {
    return {
        MakeColorPipelineFunction(
            "identity",
            "Identity",
            "Keep the incoming source signal unchanged before palette materialization.",
            {}),
        MakeColorPipelineFunction(
            "offset_scale",
            "Offset + Scale",
            "Shift and scale the incoming signal before palette materialization.",
            {
                MakeColorPipelineFloatParam("shape.offset", "Offset", "Shift the incoming signal before downstream palette work.", -2.0, 2.0, 0.01, 0.0),
                MakeColorPipelineFloatParam("shape.scale", "Scale", "Expand or compress the incoming signal.", 0.1, 8.0, 0.01, 1.0),
            }),
        MakeColorPipelineFunction(
            "repeat",
            "Repeat",
            "Tile the incoming signal into repeating bands.",
            {
                MakeColorPipelineFloatParam("shape.frequency", "Frequency", "Control how often the signal repeats.", 0.25, 24.0, 0.01, 8.0),
                MakeColorPipelineFloatParam("shape.phase", "Phase", "Offset the repeated pattern without changing the source.", -1.0, 1.0, 0.01, 0.0),
            }),
        MakeColorPipelineFunction(
            "posterize",
            "Posterize",
            "Quantize the incoming signal into stepped bands.",
            {
                MakeColorPipelineIntParam("shape.steps", "Steps", "Choose how many discrete levels to keep.", 2, 24, 1, 6),
                MakeColorPipelineFloatParam("shape.mix", "Mix", "Blend between the original signal and the stepped version.", 0.0, 1.0, 0.01, 1.0),
            }),
        MakeColorPipelineFunction(
            "mirror_repeat",
            "Mirror Repeat",
            "Tile the incoming signal into a mirrored triangle-wave pattern.",
            {
                MakeColorPipelineFloatParam("shape.frequency", "Frequency", "Control how often the mirrored pattern repeats.", 0.25, 24.0, 0.01, 8.0),
                MakeColorPipelineFloatParam("shape.phase", "Phase", "Offset the mirrored pattern without changing the source.", -1.0, 1.0, 0.01, 0.0),
            }),
        MakeColorPipelineFunction(
            "bias_gain_curve",
            "Bias + Gain Curve",
            "Remap the incoming signal through a bias/gain curve while preserving neutral defaults.",
            {
                MakeColorPipelineFloatParam("shape.bias", "Bias", "Push the incoming signal toward the low or high end before gain is applied.", 0.0, 1.0, 0.01, 0.5),
                MakeColorPipelineFloatParam("shape.gain", "Gain", "Adjust midtone contrast while preserving a neutral center at 0.5.", 0.0, 1.0, 0.01, 0.5),
            }),
        MakeColorPipelineFunction(
            "smooth_window",
            "Smooth Window",
            "Gate the incoming signal through a smoothstep window over the current wrap domain.",
            {
                MakeColorPipelineFloatParam("shape.center", "Center", "Choose where the smooth window is centered across the current wrap domain.", 0.0, 1.0, 0.01, 0.5),
                MakeColorPipelineFloatParam("shape.width", "Width", "Control how much of the current wrap domain stays inside the window.", 0.0, 1.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("shape.softness", "Softness", "Feather the window edges with a smoothstep blend instead of a hard cutoff.", 0.0, 1.0, 0.01, 0.0),
            }),
    };
}

inline std::vector<FunctionDescriptor> BuildColorPipelineGradeFunctions() {
    return {
        MakeColorPipelineFunction(
            "contrast_lift",
            "Contrast Lift",
            "Apply the default escape-time grading profile.",
            {
                MakeColorPipelineFloatParam("grade.exposure", "Exposure", "Set the overall escape brightness.", 0.1, 3.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("grade.saturation", "Saturation", "Push or soften the escape palette intensity.", 0.0, 2.0, 0.01, 1.0),
            }),
        MakeColorPipelineFunction(
            "phase_finish",
            "Phase Finish",
            "Apply the default phase grading profile.",
            {
                MakeColorPipelineFloatParam("grade.saturation", "Saturation", "Push or soften phase-wheel intensity.", 0.0, 2.0, 0.01, 1.15),
                MakeColorPipelineFloatParam("grade.contrast", "Contrast", "Stretch the phase palette mid-tones.", 0.0, 3.0, 0.01, 1.0),
            }),
        MakeColorPipelineFunction(
            "band_finish",
            "Band Finish",
            "Apply the default banded grading profile.",
            {
                MakeColorPipelineFloatParam("grade.band_emphasis", "Band Emphasis", "Increase or relax final band contrast.", 0.0, 2.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("grade.glow", "Glow", "Add controlled highlight bloom to the final output.", 0.0, 2.0, 0.01, 0.25),
            }),
    };
}

inline bool IsColorPipelineFunctionRuntimeBacked(const char* laneId, const std::string& functionId) {
    if (!laneId || laneId[0] == '\0') {
        return false;
    }

    if (std::string(laneId) == "source") {
        return functionId == "smooth_escape_ramp" ||
            functionId == "phase_orbit" ||
            functionId == "banded_signal" ||
            functionId == "escape_magnitude" ||
            functionId == "orbit_stripe" ||
            functionId == "root_proximity" ||
            functionId == "root_index";
    }
    if (std::string(laneId) == "shape") {
        return functionId == "identity" ||
            functionId == "offset_scale" ||
            functionId == "repeat" ||
            functionId == "posterize" ||
            functionId == "mirror_repeat" ||
            functionId == "bias_gain_curve" ||
            functionId == "smooth_window";
    }
    if (std::string(laneId) == "palette") {
        return functionId == "heatmap" ||
            functionId == "phase_wheel_palette" ||
            functionId == "banded_heatmap" ||
            functionId == "explaino_cmap" ||
            functionId == "root_classic_palette" ||
            functionId == "joy_root_palette";
    }
    return false;
}

inline std::vector<FunctionDescriptor> FilterRuntimeBackedColorPipelineFunctions(
    const char* laneId,
    std::vector<FunctionDescriptor> functions) {
    std::vector<FunctionDescriptor> filtered;
    filtered.reserve(functions.size());
    for (FunctionDescriptor& descriptor : functions) {
        if (IsColorPipelineFunctionRuntimeBacked(laneId, descriptor.id)) {
            filtered.push_back(std::move(descriptor));
        }
    }
    return filtered;
}

inline const std::vector<ColorPipelineLaneCatalog>& GetColorPipelineLaneCatalogs() {
    static const std::vector<ColorPipelineLaneCatalog> catalogs = {
        {"source", "Source", "smooth_escape_ramp", FilterRuntimeBackedColorPipelineFunctions("source", BuildColorPipelineSignalFunctions())},
        {"shape", "Shape", "identity", FilterRuntimeBackedColorPipelineFunctions("shape", BuildColorPipelineShapeFunctions())},
        {"palette", "Palette", "heatmap", FilterRuntimeBackedColorPipelineFunctions("palette", BuildColorPipelinePaletteFunctions())},
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

inline bool SetColorPipelineRowFunction(
    ColorPipelineRowState* ioRow,
    const FunctionDescriptor& descriptor) {
    if (!ioRow) {
        return false;
    }

    ioRow->function_id = descriptor.id;
    ioRow->parameter_values.clear();
    ioRow->parameter_values.reserve(descriptor.parameters.size());
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
        ioRow->parameter_values.push_back(std::move(value));
    }
    return true;
}

inline bool BuildColorPipelineRowFromFunctionId(
    const ColorPipelineLaneCatalog& catalog,
    const char* functionId,
    std::uint64_t stableRowId,
    ColorPipelineRowState* outRow,
    std::string* outError = nullptr) {
    if (!outRow || !functionId || functionId[0] == '\0') {
        if (outError) *outError = "Color pipeline row builder requires a non-empty function id";
        return false;
    }

    const FunctionDescriptor* descriptor = FindColorPipelineFunctionDescriptor(catalog, functionId);
    if (!descriptor) {
        if (outError) {
            *outError = std::string("Unknown advanced color function '") + functionId + "' for lane " + catalog.label;
        }
        return false;
    }

    ColorPipelineRowState row;
    row.ui_row_id = stableRowId;
    if (!SetColorPipelineRowFunction(&row, *descriptor)) {
        if (outError) *outError = std::string("Failed to initialize advanced color function '") + functionId + "'";
        return false;
    }
    *outRow = std::move(row);
    return true;
}

inline bool BuildColorPipelineLaneWithSingleRow(
    const ColorPipelineLaneCatalog& catalog,
    const char* functionId,
    std::uint64_t stableRowId,
    ColorPipelineLaneState* outLane,
    std::string* outError = nullptr) {
    if (!outLane) {
        if (outError) *outError = "Color pipeline lane builder requires an output lane";
        return false;
    }

    ColorPipelineLaneState lane;
    lane.lane_id = catalog.lane_id;
    lane.label = catalog.label;

    ColorPipelineRowState row;
    if (!BuildColorPipelineRowFromFunctionId(catalog, functionId, stableRowId, &row, outError)) {
        return false;
    }
    lane.rows.push_back(std::move(row));
    *outLane = std::move(lane);
    return true;
}

inline bool TryGetColorPipelineParamNumber(
    const ColorPipelineRowState& row,
    const char* path,
    double* outValue,
    std::string* outError = nullptr) {
    if (!path || !outValue) {
        if (outError) *outError = "Advanced color parameter lookup requires a path and output storage";
        return false;
    }
    for (const ColorPipelineParamState& param : row.parameter_values) {
        if (param.path == path) {
            *outValue = param.number_value;
            return true;
        }
    }
    if (outError) *outError = std::string("Missing advanced color parameter path '") + path + "' for function '" + row.function_id + "'";
    return false;
}

inline bool SetColorPipelineParamNumber(
    ColorPipelineRowState* ioRow,
    const char* path,
    double value,
    std::string* outError = nullptr) {
    if (!ioRow || !path) {
        if (outError) *outError = "Advanced color parameter import requires a row and parameter path";
        return false;
    }
    for (ColorPipelineParamState& param : ioRow->parameter_values) {
        if (param.path == path) {
            param.number_value = value;
            return true;
        }
    }
    if (outError) *outError = std::string("Missing advanced color parameter path '") + path + "' for function '" + ioRow->function_id + "'";
    return false;
}

inline bool ImportSupportedColorPipelineParamsFromLive(
    ColorPipelineRowState* ioRow,
    const KernelParams& liveParams,
    std::string* outError = nullptr) {
    if (!ioRow) {
        if (outError) *outError = "Advanced color parameter import requires a row";
        return false;
    }
    if (ioRow->function_id == "smooth_escape_ramp") {
        return SetColorPipelineParamNumber(ioRow, "signal.scale", liveParams.color_smooth_escape_scale, outError) &&
            SetColorPipelineParamNumber(ioRow, "signal.bias", liveParams.color_smooth_escape_bias, outError);
    }
    if (ioRow->function_id == "heatmap") {
        return SetColorPipelineParamNumber(ioRow, "palette.cycle_scale", liveParams.color_heatmap_cycle_scale, outError) &&
            SetColorPipelineParamNumber(ioRow, "palette.saturation", liveParams.color_heatmap_saturation, outError);
    }
    if (ioRow->function_id == "contrast_lift") {
        return SetColorPipelineParamNumber(ioRow, "grade.exposure", liveParams.color_contrast_lift_exposure, outError) &&
            SetColorPipelineParamNumber(ioRow, "grade.saturation", liveParams.color_contrast_lift_saturation, outError);
    }
    if (ioRow->function_id == "phase_orbit") {
        return SetColorPipelineParamNumber(ioRow, "signal.phase_offset", liveParams.color_phase_signal_offset, outError) &&
            SetColorPipelineParamNumber(ioRow, "signal.wrap_cycles", liveParams.color_phase_wrap_cycles, outError);
    }
    if (ioRow->function_id == "escape_magnitude") {
        return SetColorPipelineParamNumber(ioRow, "signal.magnitude_scale", liveParams.color_escape_magnitude_scale, outError) &&
            SetColorPipelineParamNumber(ioRow, "signal.magnitude_bias", liveParams.color_escape_magnitude_bias, outError);
    }
    if (ioRow->function_id == "orbit_stripe") {
        return SetColorPipelineParamNumber(ioRow, "signal.stripe_frequency", liveParams.color_orbit_stripe_frequency, outError) &&
            SetColorPipelineParamNumber(ioRow, "signal.phase_offset", liveParams.color_orbit_stripe_phase, outError);
    }
    if (ioRow->function_id == "root_proximity") {
        return SetColorPipelineParamNumber(ioRow, "signal.proximity_scale", liveParams.color_root_proximity_scale, outError) &&
            SetColorPipelineParamNumber(ioRow, "signal.proximity_bias", liveParams.color_root_proximity_bias, outError);
    }
    if (ioRow->function_id == "phase_wheel_palette") {
        return SetColorPipelineParamNumber(ioRow, "palette.phase_offset", liveParams.color_phase_palette_offset, outError);
    }
    if (ioRow->function_id == "explaino_cmap") {
        return SetColorPipelineParamNumber(ioRow, "palette.seed_scale", liveParams.color_explaino_palette_seed_scale, outError) &&
            SetColorPipelineParamNumber(ioRow, "palette.seed_phase", liveParams.color_explaino_palette_seed_phase, outError) &&
            SetColorPipelineParamNumber(ioRow, "palette.colorfulness", liveParams.color_explaino_palette_colorfulness, outError);
    }
    if (ioRow->function_id == "banded_signal") {
        return SetColorPipelineParamNumber(ioRow, "signal.band_count", static_cast<double>(liveParams.color_iteration_band_count), outError) &&
            SetColorPipelineParamNumber(ioRow, "signal.softness", liveParams.color_iteration_band_softness, outError);
    }
    if (ioRow->function_id == "banded_heatmap") {
        return SetColorPipelineParamNumber(ioRow, "palette.band_emphasis", liveParams.color_iteration_band_emphasis, outError) &&
            SetColorPipelineParamNumber(ioRow, "palette.phase_offset", liveParams.color_iteration_band_palette_offset, outError);
    }
    return true;
}

inline bool ValidateColorPipelineParamRange(
    const char* path,
    double value,
    double minValue,
    double maxValue,
    std::string* outError = nullptr) {
    if (value < minValue || value > maxValue) {
        if (outError) {
            *outError = std::string("Advanced color parameter '") + (path ? path : "") +
                "' is outside the supported range [" + std::to_string(minValue) + ", " + std::to_string(maxValue) + "]";
        }
        return false;
    }
    return true;
}

inline bool TryBuildColorPipelineSelectionFromLaneIds(
    const char* sourceFunctionId,
    const char* paletteFunctionId,
    ColorPipelineSelection* outPipeline,
    ColoringMode* outMode) {
    if (!sourceFunctionId || sourceFunctionId[0] == '\0' ||
        !paletteFunctionId || paletteFunctionId[0] == '\0' ||
        !outPipeline || !outMode) {
        return false;
    }

    if (std::strcmp(sourceFunctionId, "smooth_escape_ramp") == 0 && std::strcmp(paletteFunctionId, "heatmap") == 0) {
        *outPipeline = {ColorSignal::smooth_escape, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
        *outMode = ColoringMode::smooth_escape;
        return true;
    }
    if (std::strcmp(sourceFunctionId, "smooth_escape_ramp") == 0 && std::strcmp(paletteFunctionId, "explaino_cmap") == 0) {
        *outPipeline = {ColorSignal::smooth_escape, ColorPalette::explaino_cmap, ColorGradingPreset::escape_default};
        *outMode = ColoringMode::smooth_escape;
        return true;
    }
    if (std::strcmp(sourceFunctionId, "phase_orbit") == 0 && std::strcmp(paletteFunctionId, "phase_wheel_palette") == 0) {
        *outPipeline = {ColorSignal::phase_angle, ColorPalette::phase_wheel, ColorGradingPreset::phase_default};
        *outMode = ColoringMode::phase;
        return true;
    }
    if (std::strcmp(sourceFunctionId, "banded_signal") == 0 && std::strcmp(paletteFunctionId, "banded_heatmap") == 0) {
        *outPipeline = {ColorSignal::iteration_bands, ColorPalette::banded_escape, ColorGradingPreset::bands_default};
        *outMode = ColoringMode::iteration_bands;
        return true;
    }
    if (std::strcmp(sourceFunctionId, "escape_magnitude") == 0 && std::strcmp(paletteFunctionId, "heatmap") == 0) {
        *outPipeline = {ColorSignal::escape_magnitude, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
        *outMode = ColoringMode::smooth_escape;
        return true;
    }
    if (std::strcmp(sourceFunctionId, "escape_magnitude") == 0 && std::strcmp(paletteFunctionId, "explaino_cmap") == 0) {
        *outPipeline = {ColorSignal::escape_magnitude, ColorPalette::explaino_cmap, ColorGradingPreset::escape_default};
        *outMode = ColoringMode::smooth_escape;
        return true;
    }
    if (std::strcmp(sourceFunctionId, "orbit_stripe") == 0 && std::strcmp(paletteFunctionId, "phase_wheel_palette") == 0) {
        *outPipeline = {ColorSignal::orbit_stripe, ColorPalette::phase_wheel, ColorGradingPreset::phase_default};
        *outMode = ColoringMode::phase;
        return true;
    }
    if (std::strcmp(sourceFunctionId, "root_proximity") == 0 && std::strcmp(paletteFunctionId, "heatmap") == 0) {
        *outPipeline = {ColorSignal::root_proximity, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
        *outMode = ColoringMode::smooth_escape;
        return true;
    }
    if (std::strcmp(sourceFunctionId, "root_proximity") == 0 && std::strcmp(paletteFunctionId, "explaino_cmap") == 0) {
        *outPipeline = {ColorSignal::root_proximity, ColorPalette::explaino_cmap, ColorGradingPreset::escape_default};
        *outMode = ColoringMode::smooth_escape;
        return true;
    }
    if (std::strcmp(sourceFunctionId, "root_index") == 0 && std::strcmp(paletteFunctionId, "root_classic_palette") == 0) {
        *outPipeline = {ColorSignal::root_index, ColorPalette::root_classic, ColorGradingPreset::basin_default};
        *outMode = ColoringMode::root_basin;
        return true;
    }
    if (std::strcmp(sourceFunctionId, "root_index") == 0 && std::strcmp(paletteFunctionId, "joy_root_palette") == 0) {
        *outPipeline = {ColorSignal::root_index, ColorPalette::joy, ColorGradingPreset::basin_default};
        *outMode = ColoringMode::joy_basins;
        return true;
    }
    return false;
}

inline bool ApplySupportedColorPipelineRowParamsToLive(
    const ColorPipelineRowState& row,
    KernelParams* ioParams,
    bool* outChanged = nullptr,
    std::string* outError = nullptr) {
    if (outChanged) {
        *outChanged = false;
    }
    if (!ioParams) {
        if (outError) *outError = "Advanced color parameter apply requires live KernelParams";
        return false;
    }

    bool changed = false;
    const auto assignInt = [&](int* target, int value) {
        if (*target != value) {
            *target = value;
            changed = true;
        }
    };
    const auto assignFloat = [&](float* target, float value) {
        if (std::fabs(*target - value) > 1.0e-6f) {
            *target = value;
            changed = true;
        }
    };
    const auto resetPaletteHeatmap = [&]() {
        assignFloat(&ioParams->color_heatmap_cycle_scale, 1.0f);
        assignFloat(&ioParams->color_heatmap_saturation, 1.0f);
    };
    const auto resetPalettePhaseWheel = [&]() {
        assignFloat(&ioParams->color_phase_palette_offset, 0.0f);
    };
    const auto resetPaletteBandedHeatmap = [&]() {
        assignFloat(&ioParams->color_iteration_band_emphasis, 1.0f);
        assignFloat(&ioParams->color_iteration_band_palette_offset, 0.0f);
    };
    const auto resetPaletteExplaino = [&]() {
        assignFloat(&ioParams->color_explaino_palette_seed_scale, 1.0f);
        assignFloat(&ioParams->color_explaino_palette_seed_phase, 0.0f);
        assignFloat(&ioParams->color_explaino_palette_colorfulness, 1.0f);
    };
    const auto tryReadInteger = [&](const char* path, int* outValue) {
        double value = 0.0;
        if (!TryGetColorPipelineParamNumber(row, path, &value, outError)) {
            return false;
        }
        const double rounded = std::round(value);
        if (std::fabs(value - rounded) > 1.0e-6) {
            if (outError) *outError = std::string("Advanced color parameter '") + path + "' must be an integer";
            return false;
        }
        *outValue = static_cast<int>(rounded);
        return true;
    };

    if (row.function_id == "smooth_escape_ramp") {
        double scale = 0.0;
        double bias = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "signal.scale", &scale, outError) ||
            !TryGetColorPipelineParamNumber(row, "signal.bias", &bias, outError) ||
            !ValidateColorPipelineParamRange("signal.scale", scale, 0.25, 4.0, outError) ||
            !ValidateColorPipelineParamRange("signal.bias", bias, -1.0, 1.0, outError)) {
            return false;
        }
        assignFloat(&ioParams->color_smooth_escape_scale, static_cast<float>(scale));
        assignFloat(&ioParams->color_smooth_escape_bias, static_cast<float>(bias));
    } else if (row.function_id == "heatmap") {
        double cycleScale = 0.0;
        double saturation = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "palette.cycle_scale", &cycleScale, outError) ||
            !TryGetColorPipelineParamNumber(row, "palette.saturation", &saturation, outError) ||
            !ValidateColorPipelineParamRange("palette.cycle_scale", cycleScale, 0.25, 4.0, outError) ||
            !ValidateColorPipelineParamRange("palette.saturation", saturation, 0.0, 2.0, outError)) {
            return false;
        }
        assignFloat(&ioParams->color_heatmap_cycle_scale, static_cast<float>(cycleScale));
        assignFloat(&ioParams->color_heatmap_saturation, static_cast<float>(saturation));
        resetPalettePhaseWheel();
        resetPaletteBandedHeatmap();
        resetPaletteExplaino();
    } else if (row.function_id == "contrast_lift") {
        double exposure = 0.0;
        double saturation = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "grade.exposure", &exposure, outError) ||
            !TryGetColorPipelineParamNumber(row, "grade.saturation", &saturation, outError) ||
            !ValidateColorPipelineParamRange("grade.exposure", exposure, 0.1, 3.0, outError) ||
            !ValidateColorPipelineParamRange("grade.saturation", saturation, 0.0, 2.0, outError)) {
            return false;
        }
        assignFloat(&ioParams->color_contrast_lift_exposure, static_cast<float>(exposure));
        assignFloat(&ioParams->color_contrast_lift_saturation, static_cast<float>(saturation));
    } else if (row.function_id == "phase_orbit") {
        double phaseOffset = 0.0;
        double wrapCycles = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "signal.phase_offset", &phaseOffset, outError) ||
            !TryGetColorPipelineParamNumber(row, "signal.wrap_cycles", &wrapCycles, outError) ||
            !ValidateColorPipelineParamRange("signal.phase_offset", phaseOffset, -3.141592653589793, 3.141592653589793, outError) ||
            !ValidateColorPipelineParamRange("signal.wrap_cycles", wrapCycles, 0.5, 6.0, outError)) {
            return false;
        }
        assignFloat(&ioParams->color_phase_signal_offset, static_cast<float>(phaseOffset));
        assignFloat(&ioParams->color_phase_wrap_cycles, static_cast<float>(wrapCycles));
    } else if (row.function_id == "escape_magnitude") {
        double magnitudeScale = 0.0;
        double magnitudeBias = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "signal.magnitude_scale", &magnitudeScale, outError) ||
            !TryGetColorPipelineParamNumber(row, "signal.magnitude_bias", &magnitudeBias, outError) ||
            !ValidateColorPipelineParamRange("signal.magnitude_scale", magnitudeScale, 0.25, 4.0, outError) ||
            !ValidateColorPipelineParamRange("signal.magnitude_bias", magnitudeBias, -1.0, 1.0, outError)) {
            return false;
        }
        assignFloat(&ioParams->color_escape_magnitude_scale, static_cast<float>(magnitudeScale));
        assignFloat(&ioParams->color_escape_magnitude_bias, static_cast<float>(magnitudeBias));
    } else if (row.function_id == "orbit_stripe") {
        double stripeFrequency = 0.0;
        double stripePhase = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "signal.stripe_frequency", &stripeFrequency, outError) ||
            !TryGetColorPipelineParamNumber(row, "signal.phase_offset", &stripePhase, outError) ||
            !ValidateColorPipelineParamRange("signal.stripe_frequency", stripeFrequency, 0.25, 12.0, outError) ||
            !ValidateColorPipelineParamRange("signal.phase_offset", stripePhase, -3.141592653589793, 3.141592653589793, outError)) {
            return false;
        }
        assignFloat(&ioParams->color_orbit_stripe_frequency, static_cast<float>(stripeFrequency));
        assignFloat(&ioParams->color_orbit_stripe_phase, static_cast<float>(stripePhase));
    } else if (row.function_id == "root_proximity") {
        double proximityScale = 0.0;
        double proximityBias = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "signal.proximity_scale", &proximityScale, outError) ||
            !TryGetColorPipelineParamNumber(row, "signal.proximity_bias", &proximityBias, outError) ||
            !ValidateColorPipelineParamRange("signal.proximity_scale", proximityScale, 0.25, 8.0, outError) ||
            !ValidateColorPipelineParamRange("signal.proximity_bias", proximityBias, -1.0, 1.0, outError)) {
            return false;
        }
        assignFloat(&ioParams->color_root_proximity_scale, static_cast<float>(proximityScale));
        assignFloat(&ioParams->color_root_proximity_bias, static_cast<float>(proximityBias));
    } else if (row.function_id == "phase_wheel_palette") {
        double paletteOffset = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "palette.phase_offset", &paletteOffset, outError) ||
            !ValidateColorPipelineParamRange("palette.phase_offset", paletteOffset, -3.141592653589793, 3.141592653589793, outError)) {
            return false;
        }
        assignFloat(&ioParams->color_phase_palette_offset, static_cast<float>(paletteOffset));
        resetPaletteHeatmap();
        resetPaletteBandedHeatmap();
        resetPaletteExplaino();
    } else if (row.function_id == "explaino_cmap") {
        double seedScale = 0.0;
        double seedPhase = 0.0;
        double colorfulness = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "palette.seed_scale", &seedScale, outError) ||
            !TryGetColorPipelineParamNumber(row, "palette.seed_phase", &seedPhase, outError) ||
            !TryGetColorPipelineParamNumber(row, "palette.colorfulness", &colorfulness, outError) ||
            !ValidateColorPipelineParamRange("palette.seed_scale", seedScale, 0.25, 4.0, outError) ||
            !ValidateColorPipelineParamRange("palette.seed_phase", seedPhase, -1.0, 1.0, outError) ||
            !ValidateColorPipelineParamRange("palette.colorfulness", colorfulness, 0.0, 1.0, outError)) {
            return false;
        }
        assignFloat(&ioParams->color_explaino_palette_seed_scale, static_cast<float>(seedScale));
        assignFloat(&ioParams->color_explaino_palette_seed_phase, static_cast<float>(seedPhase));
        assignFloat(&ioParams->color_explaino_palette_colorfulness, static_cast<float>(colorfulness));
        resetPaletteHeatmap();
        resetPalettePhaseWheel();
        resetPaletteBandedHeatmap();
    } else if (row.function_id == "banded_signal") {
        int bandCount = 0;
        double softness = 0.0;
        if (!tryReadInteger("signal.band_count", &bandCount) ||
            !TryGetColorPipelineParamNumber(row, "signal.softness", &softness, outError) ||
            !ValidateColorPipelineParamRange("signal.band_count", static_cast<double>(bandCount), 2.0, 24.0, outError) ||
            !ValidateColorPipelineParamRange("signal.softness", softness, 0.0, 1.0, outError)) {
            return false;
        }
        assignInt(&ioParams->color_iteration_band_count, bandCount);
        assignFloat(&ioParams->color_iteration_band_softness, static_cast<float>(softness));
    } else if (row.function_id == "banded_heatmap") {
        double emphasis = 0.0;
        double paletteOffset = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "palette.band_emphasis", &emphasis, outError) ||
            !TryGetColorPipelineParamNumber(row, "palette.phase_offset", &paletteOffset, outError) ||
            !ValidateColorPipelineParamRange("palette.band_emphasis", emphasis, 0.0, 2.0, outError) ||
            !ValidateColorPipelineParamRange("palette.phase_offset", paletteOffset, -3.141592653589793, 3.141592653589793, outError)) {
            return false;
        }
        assignFloat(&ioParams->color_iteration_band_emphasis, static_cast<float>(emphasis));
        assignFloat(&ioParams->color_iteration_band_palette_offset, static_cast<float>(paletteOffset));
        resetPaletteHeatmap();
        resetPalettePhaseWheel();
        resetPaletteExplaino();
    }

    if (outChanged) {
        *outChanged = changed;
    }
    return true;
}

inline bool TryBuildColorPipelineScheduleBridgeIds(
    const ColorPipelineSelection& pipeline,
    const char** outSourceFunctionId,
    const char** outPaletteFunctionId) {
    if (outSourceFunctionId) {
        *outSourceFunctionId = nullptr;
    }
    if (outPaletteFunctionId) {
        *outPaletteFunctionId = nullptr;
    }

    if (pipeline.signal == ColorSignal::smooth_escape &&
        pipeline.palette == ColorPalette::cyclic_escape &&
        pipeline.grading == ColorGradingPreset::escape_default) {
        if (outSourceFunctionId) *outSourceFunctionId = "smooth_escape_ramp";
        if (outPaletteFunctionId) *outPaletteFunctionId = "heatmap";
        return true;
    }
    if (pipeline.signal == ColorSignal::phase_angle &&
        pipeline.palette == ColorPalette::phase_wheel &&
        pipeline.grading == ColorGradingPreset::phase_default) {
        if (outSourceFunctionId) *outSourceFunctionId = "phase_orbit";
        if (outPaletteFunctionId) *outPaletteFunctionId = "phase_wheel_palette";
        return true;
    }
    if (pipeline.signal == ColorSignal::iteration_bands &&
        pipeline.palette == ColorPalette::banded_escape &&
        pipeline.grading == ColorGradingPreset::bands_default) {
        if (outSourceFunctionId) *outSourceFunctionId = "banded_signal";
        if (outPaletteFunctionId) *outPaletteFunctionId = "banded_heatmap";
        return true;
    }
    if (pipeline.signal == ColorSignal::escape_magnitude &&
        pipeline.palette == ColorPalette::cyclic_escape &&
        pipeline.grading == ColorGradingPreset::escape_default) {
        if (outSourceFunctionId) *outSourceFunctionId = "escape_magnitude";
        if (outPaletteFunctionId) *outPaletteFunctionId = "heatmap";
        return true;
    }
    if (pipeline.signal == ColorSignal::orbit_stripe &&
        pipeline.palette == ColorPalette::phase_wheel &&
        pipeline.grading == ColorGradingPreset::phase_default) {
        if (outSourceFunctionId) *outSourceFunctionId = "orbit_stripe";
        if (outPaletteFunctionId) *outPaletteFunctionId = "phase_wheel_palette";
        return true;
    }
    if (pipeline.signal == ColorSignal::root_proximity &&
        pipeline.palette == ColorPalette::cyclic_escape &&
        pipeline.grading == ColorGradingPreset::escape_default) {
        if (outSourceFunctionId) *outSourceFunctionId = "root_proximity";
        if (outPaletteFunctionId) *outPaletteFunctionId = "heatmap";
        return true;
    }
    if (pipeline.signal == ColorSignal::smooth_escape &&
        pipeline.palette == ColorPalette::explaino_cmap &&
        pipeline.grading == ColorGradingPreset::escape_default) {
        if (outSourceFunctionId) *outSourceFunctionId = "smooth_escape_ramp";
        if (outPaletteFunctionId) *outPaletteFunctionId = "explaino_cmap";
        return true;
    }
    if (pipeline.signal == ColorSignal::escape_magnitude &&
        pipeline.palette == ColorPalette::explaino_cmap &&
        pipeline.grading == ColorGradingPreset::escape_default) {
        if (outSourceFunctionId) *outSourceFunctionId = "escape_magnitude";
        if (outPaletteFunctionId) *outPaletteFunctionId = "explaino_cmap";
        return true;
    }
    if (pipeline.signal == ColorSignal::root_proximity &&
        pipeline.palette == ColorPalette::explaino_cmap &&
        pipeline.grading == ColorGradingPreset::escape_default) {
        if (outSourceFunctionId) *outSourceFunctionId = "root_proximity";
        if (outPaletteFunctionId) *outPaletteFunctionId = "explaino_cmap";
        return true;
    }
    if (pipeline.signal == ColorSignal::root_index &&
        pipeline.palette == ColorPalette::root_classic &&
        pipeline.grading == ColorGradingPreset::basin_default) {
        if (outSourceFunctionId) *outSourceFunctionId = "root_index";
        if (outPaletteFunctionId) *outPaletteFunctionId = "root_classic_palette";
        return true;
    }
    if (pipeline.signal == ColorSignal::root_index &&
        pipeline.palette == ColorPalette::joy &&
        pipeline.grading == ColorGradingPreset::basin_default) {
        if (outSourceFunctionId) *outSourceFunctionId = "root_index";
        if (outPaletteFunctionId) *outPaletteFunctionId = "joy_root_palette";
        return true;
    }
    return false;
}

} // namespace color_pipeline_core
