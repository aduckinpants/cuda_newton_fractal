#pragma once

#include "fractal_types.h"
#include "function_descriptor.h"

#include <string>
#include <utility>
#include <vector>

struct ColorPipelineLaneCatalog {
    const char* lane_id = "";
    const char* label = "";
    const char* default_function_id = "";
    std::vector<FunctionDescriptor> functions;
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
    return false;
}

inline const char* AdvancedColorPaletteFunctionId(ColorPalette value) {
    switch (value) {
    case ColorPalette::cyclic_escape:
        return "heatmap";
    case ColorPalette::phase_wheel:
        return "phase_wheel_palette";
    case ColorPalette::banded_escape:
        return "banded_heatmap";
    }
    return nullptr;
}

inline bool TryParseAdvancedColorPaletteFunctionId(const std::string& functionId, ColorPalette* outValue) {
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
            functionId == "root_proximity";
    }
    if (std::string(laneId) == "shape") {
        return functionId == "identity" ||
            functionId == "offset_scale" ||
            functionId == "repeat" ||
            functionId == "posterize" ||
            functionId == "mirror_repeat";
    }
    if (std::string(laneId) == "palette") {
        return functionId == "heatmap" ||
            functionId == "phase_wheel_palette" ||
            functionId == "banded_heatmap";
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
    return false;
}

} // namespace color_pipeline_core
