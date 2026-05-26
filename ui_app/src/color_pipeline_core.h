#pragma once

#include "fractal_types.h"
#include "function_descriptor.h"
#include "color_pipeline_metadata_contract.h"
#include "enum_id_utils.h"

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

inline json_min::Value MakeColorPipelineStringValue(const char* value) {
    json_min::Value out;
    out.v = std::string(value ? value : "");
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

inline FunctionParamDescriptor MakeColorPipelineEnumParam(
    const char* path,
    const char* label,
    const char* help,
    std::vector<UISchemaOption> options,
    const char* defaultValue) {
    FunctionParamDescriptor param;
    param.path = path ? path : "";
    param.type = "enum";
    param.label = label ? label : "";
    param.help = help ? help : "";
    param.options = std::move(options);
    param.has_default = true;
    param.default_value = MakeColorPipelineStringValue(defaultValue);
    return param;
}

inline const char* ColorPaletteBlendModeId(ColorPaletteBlendMode value) {
    switch (value) {
    case ColorPaletteBlendMode::normal:
        return "normal";
    }
    return nullptr;
}

inline bool TryParseColorPaletteBlendModeId(const std::string& id, ColorPaletteBlendMode* outValue) {
    if (id == "normal") {
        if (outValue) *outValue = ColorPaletteBlendMode::normal;
        return true;
    }
    return false;
}

inline std::vector<UISchemaOption> ColorPipelinePaletteBlendModeOptions() {
    return {{"normal", "Normal", ""}};
}

inline FunctionParamDescriptor MakeColorPipelinePaletteBlendWeightParam() {
    return MakeColorPipelineFloatParam(
        "palette.blend_weight",
        "Blend Weight",
        "Blend this Palette row into the accumulated Palette stack RGB.",
        0.0,
        1.0,
        0.01,
        1.0);
}

inline FunctionParamDescriptor MakeColorPipelinePaletteBlendModeParam() {
    return MakeColorPipelineEnumParam(
        "palette.blend_mode",
        "Blend Mode",
        "Choose how this Palette row combines with the accumulated Palette stack RGB.",
        ColorPipelinePaletteBlendModeOptions(),
        "normal");
}

inline FunctionParamDescriptor MakeColorPipelineSourceBlendWeightParam() {
    return MakeColorPipelineFloatParam(
        "signal.blend_weight",
        "Blend Weight",
        "Blend this Source row into the accumulated Source stack signal.",
        0.0,
        1.0,
        0.01,
        1.0);
}

inline FunctionParamDescriptor MakeColorPipelineSourceScaleParam(
    const char* label,
    const char* help,
    double minValue,
    double maxValue,
    double stepValue,
    double defaultValue) {
    return MakeColorPipelineFloatParam("signal.scale", label, help, minValue, maxValue, stepValue, defaultValue);
}

inline FunctionParamDescriptor MakeColorPipelineSourceBiasParam(
    const char* label,
    const char* help,
    double minValue,
    double maxValue,
    double stepValue,
    double defaultValue) {
    return MakeColorPipelineFloatParam("signal.bias", label, help, minValue, maxValue, stepValue, defaultValue);
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

enum class ColorPipelineSourceSignalKind {
    scalar,
    phase,
    categorical,
};

inline const char* ColorPipelineSourceSignalKindId(ColorPipelineSourceSignalKind value) {
    switch (value) {
    case ColorPipelineSourceSignalKind::scalar:
        return "scalar";
    case ColorPipelineSourceSignalKind::phase:
        return "phase";
    case ColorPipelineSourceSignalKind::categorical:
        return "categorical";
    }
    return "scalar";
}

inline ColorPipelineSourceSignalKind ColorPipelineSourceSignalKindForSignal(ColorSignal value) {
    switch (value) {
    case ColorSignal::phase_angle:
    case ColorSignal::orbit_stripe:
    case ColorSignal::sdf_normal_angle:
        return ColorPipelineSourceSignalKind::phase;
    case ColorSignal::root_index:
    case ColorSignal::sdf_inside_outside:
        return ColorPipelineSourceSignalKind::categorical;
    case ColorSignal::iteration_count:
    case ColorSignal::smooth_escape:
    case ColorSignal::iteration_bands:
    case ColorSignal::escape_magnitude:
    case ColorSignal::root_proximity:
    case ColorSignal::sdf_signed_distance:
    case ColorSignal::sdf_boundary_band:
    case ColorSignal::sdf_curvature:
        return ColorPipelineSourceSignalKind::scalar;
    }
    return ColorPipelineSourceSignalKind::scalar;
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
    case ColorSignal::sdf_signed_distance:
        return "sdf_signed_distance";
    case ColorSignal::sdf_inside_outside:
        return "sdf_inside_outside";
    case ColorSignal::sdf_boundary_band:
        return "sdf_boundary_band";
    case ColorSignal::sdf_normal_angle:
        return "sdf_normal_angle";
    case ColorSignal::sdf_curvature:
        return "sdf_curvature";
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
    if (functionId == "sdf_signed_distance") {
        if (outValue) *outValue = ColorSignal::sdf_signed_distance;
        return true;
    }
    if (functionId == "sdf_inside_outside") {
        if (outValue) *outValue = ColorSignal::sdf_inside_outside;
        return true;
    }
    if (functionId == "sdf_boundary_band") {
        if (outValue) *outValue = ColorSignal::sdf_boundary_band;
        return true;
    }
    if (functionId == "sdf_normal_angle") {
        if (outValue) *outValue = ColorSignal::sdf_normal_angle;
        return true;
    }
    if (functionId == "sdf_curvature") {
        if (outValue) *outValue = ColorSignal::sdf_curvature;
        return true;
    }
    if (functionId == "root_index") {
        if (outValue) *outValue = ColorSignal::root_index;
        return true;
    }
    return false;
}

inline ColorPipelineSourceSignalKind ColorPipelineSourceSignalKindForFunctionId(const char* functionId) {
    if (!functionId) {
        return ColorPipelineSourceSignalKind::scalar;
    }
    ColorSignal signal = ColorSignal::smooth_escape;
    if (!TryParseAdvancedColorSignalFunctionId(functionId, &signal)) {
        return ColorPipelineSourceSignalKind::scalar;
    }
    return ColorPipelineSourceSignalKindForSignal(signal);
}

inline bool ColorPipelineSourceFunctionIsPhaseSignal(const char* functionId) {
    return ColorPipelineSourceSignalKindForFunctionId(functionId) == ColorPipelineSourceSignalKind::phase;
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
    case ColorGradingPreset::basin_default:
        return "basin_default";
    case ColorGradingPreset::neutral_default:
        return "neutral_finish";
    case ColorGradingPreset::tone_map_default:
        return "tone_map_finish";
    case ColorGradingPreset::glow_default:
        return "grade_glow";
    case ColorGradingPreset::balance_void_default:
        return "balance_void_grade";
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
    if (functionId == "basin_default") {
        if (outValue) *outValue = ColorGradingPreset::basin_default;
        return true;
    }
    if (functionId == "neutral_finish") {
        if (outValue) *outValue = ColorGradingPreset::neutral_default;
        return true;
    }
    if (functionId == "tone_map_finish") {
        if (outValue) *outValue = ColorGradingPreset::tone_map_default;
        return true;
    }
    if (functionId == "grade_glow") {
        if (outValue) *outValue = ColorGradingPreset::glow_default;
        return true;
    }
    if (functionId == "balance_void_grade") {
        if (outValue) *outValue = ColorGradingPreset::balance_void_default;
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
                MakeColorPipelineSourceBlendWeightParam(),
            }),
        MakeColorPipelineFunction(
            "phase_orbit",
            "Phase Orbit",
            "Use orbit phase as the upstream signal.",
            {
                MakeColorPipelineFloatParam("signal.phase_offset", "Phase Offset", "Rotate the sampled phase before downstream palette work.", -3.141592653589793, 3.141592653589793, 0.01, 0.0),
                MakeColorPipelineFloatParam("signal.wrap_cycles", "Wrap Cycles", "Control how many hue cycles appear across one full rotation.", 0.5, 6.0, 0.01, 1.0),
                MakeColorPipelineSourceBlendWeightParam(),
            }),
        MakeColorPipelineFunction(
            "banded_signal",
            "Iteration Bands",
            "Quantize the escape signal into stepped iteration bands.",
            {
                MakeColorPipelineIntParam("signal.band_count", "Band Count", "Choose how many bands to carve out of the escape signal.", 2, 24, 1, 8),
                MakeColorPipelineFloatParam("signal.softness", "Softness", "Blend between hard posterization and soft band transitions.", 0.0, 1.0, 0.01, 0.35),
                MakeColorPipelineSourceBlendWeightParam(),
            }),
        MakeColorPipelineFunction(
            "escape_magnitude",
            "Escape Magnitude",
            "Use the orbit magnitude as the upstream escape-time source.",
            {
                MakeColorPipelineFloatParam("signal.magnitude_scale", "Magnitude Scale", "Expand or compress the escape-magnitude source before palette lookup.", 0.25, 4.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("signal.magnitude_bias", "Magnitude Bias", "Shift the escape-magnitude source before palette lookup.", -1.0, 1.0, 0.01, 0.0),
                MakeColorPipelineSourceBlendWeightParam(),
            }),
        MakeColorPipelineFunction(
            "orbit_stripe",
            "Orbit Stripe",
            "Fold orbit phase into a controllable stripe source before palette lookup.",
            {
                MakeColorPipelineFloatParam("signal.stripe_frequency", "Stripe Frequency", "Control how often orbit stripes repeat around the phase wheel.", 0.25, 12.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("signal.phase_offset", "Phase Offset", "Offset the stripe wave before palette lookup.", -3.141592653589793, 3.141592653589793, 0.01, 0.0),
                MakeColorPipelineSourceBlendWeightParam(),
            }),
        MakeColorPipelineFunction(
            "root_proximity",
            "Root Proximity",
            "Use nearest-root proximity as the upstream source on basin-capable fractal families.",
            {
                MakeColorPipelineFloatParam("signal.proximity_scale", "Proximity Scale", "Control how quickly root proximity falls off away from a root.", 0.25, 8.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("signal.proximity_bias", "Proximity Bias", "Shift the root-proximity source before palette lookup.", -1.0, 1.0, 0.01, 0.0),
                MakeColorPipelineSourceBlendWeightParam(),
            }),
        MakeColorPipelineFunction(
            "root_index",
            "Root Index",
            "Use the resolved nearest-root classification index as the upstream basin source.",
            {}),
        MakeColorPipelineFunction(
            "sdf_signed_distance",
            "SDF Signed Distance",
            "Use Lens SDF signed distance as the upstream color signal.",
            {
                MakeColorPipelineSourceScaleParam("Distance Scale", "Scale the signed-distance field before palette lookup.", -2.0, 2.0, 0.01, 0.05),
                MakeColorPipelineSourceBiasParam("Distance Bias", "Shift the signed-distance field before palette lookup.", -2.0, 2.0, 0.01, 0.5),
                MakeColorPipelineSourceBlendWeightParam(),
            }),
        MakeColorPipelineFunction(
            "sdf_inside_outside",
            "SDF Inside / Outside",
            "Use the Lens SDF inside/outside classification as the upstream color signal.",
            {
                MakeColorPipelineSourceScaleParam("Inside Scale", "Scale the inside/outside signal before palette lookup.", -2.0, 2.0, 0.01, 1.0),
                MakeColorPipelineSourceBiasParam("Inside Bias", "Shift the inside/outside signal before palette lookup.", -2.0, 2.0, 0.01, 0.0),
                MakeColorPipelineSourceBlendWeightParam(),
            }),
        MakeColorPipelineFunction(
            "sdf_boundary_band",
            "SDF Boundary Band",
            "Use the Lens SDF boundary band as the upstream color signal.",
            {
                MakeColorPipelineSourceScaleParam("Band Scale", "Scale the boundary-band signal before palette lookup.", -2.0, 2.0, 0.01, 1.0),
                MakeColorPipelineSourceBiasParam("Band Bias", "Shift the boundary-band signal before palette lookup.", -2.0, 2.0, 0.01, 0.0),
                MakeColorPipelineFloatParam("signal.boundary_width_px", "Boundary Width", "Set the SDF-field pixel radius used for the boundary-band source.", 0.25, 16.0, 0.25, 2.0),
                MakeColorPipelineSourceBlendWeightParam(),
            }),
        MakeColorPipelineFunction(
            "sdf_normal_angle",
            "SDF Normal Angle",
            "Use the Lens SDF normal angle as the upstream phase signal.",
            {
                MakeColorPipelineSourceScaleParam("Angle Scale", "Scale the normalized normal angle before palette lookup.", -2.0, 2.0, 0.01, 1.0),
                MakeColorPipelineSourceBiasParam("Angle Bias", "Shift the normalized normal angle before palette lookup.", -2.0, 2.0, 0.01, 0.0),
                MakeColorPipelineSourceBlendWeightParam(),
            }),
        MakeColorPipelineFunction(
            "sdf_curvature",
            "SDF Curvature",
            "Use the Lens SDF curvature estimate as the upstream color signal.",
            {
                MakeColorPipelineSourceScaleParam("Curvature Scale", "Scale the curvature estimate before palette lookup.", -2.0, 2.0, 0.01, 0.25),
                MakeColorPipelineSourceBiasParam("Curvature Bias", "Shift the curvature estimate before palette lookup.", -2.0, 2.0, 0.01, 0.5),
                MakeColorPipelineSourceBlendWeightParam(),
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
                MakeColorPipelinePaletteBlendWeightParam(),
                MakeColorPipelinePaletteBlendModeParam(),
            }),
        MakeColorPipelineFunction(
            "phase_wheel_palette",
            "Phase Wheel",
            "Wrap signal values around a phase wheel palette.",
            {
                MakeColorPipelineFloatParam("palette.phase_offset", "Phase Offset", "Rotate the wheel before it is applied to the incoming signal.", -3.141592653589793, 3.141592653589793, 0.01, 0.0),
                MakeColorPipelineFloatParam("palette.saturation", "Saturation", "Push or soften color separation in the wheel.", 0.0, 2.0, 0.01, 1.15),
                MakeColorPipelinePaletteBlendWeightParam(),
                MakeColorPipelinePaletteBlendModeParam(),
            }),
        MakeColorPipelineFunction(
            "banded_heatmap",
            "Banded Heatmap",
            "Map banded escape signals through the runtime band palette.",
            {
                MakeColorPipelineFloatParam("palette.band_emphasis", "Band Emphasis", "Increase or relax the contrast between neighboring bands.", 0.0, 2.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("palette.phase_offset", "Phase Offset", "Offset the band palette without changing the source signal.", -3.141592653589793, 3.141592653589793, 0.01, 0.0),
                MakeColorPipelinePaletteBlendWeightParam(),
                MakeColorPipelinePaletteBlendModeParam(),
            }),
        MakeColorPipelineFunction(
            "explaino_cmap",
            "Explaino CMap",
            "Map scalar signals through the legacy ExplainO seed colormap lineage.",
            {
                MakeColorPipelineFloatParam("palette.seed_scale", "Seed Scale", "Control how quickly the ExplainO seed palette cycles across the incoming signal.", 0.25, 4.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("palette.seed_phase", "Seed Phase", "Rotate the ExplainO seed palette without changing the upstream signal.", -1.0, 1.0, 0.01, 0.0),
                MakeColorPipelineFloatParam("palette.colorfulness", "Colorfulness", "Blend between the raw ExplainO seed channels and the full nonlinear legacy color transform.", 0.0, 1.0, 0.01, 1.0),
                MakeColorPipelinePaletteBlendWeightParam(),
                MakeColorPipelinePaletteBlendModeParam(),
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
                MakeColorPipelineFloatParam("grade.contrast", "Contrast", "Stretch the phase palette mid-tones.", 0.0, 3.0, 0.01, 1.10),
            }),
        MakeColorPipelineFunction(
            "band_finish",
            "Band Finish",
            "Apply the default banded grading profile through the legacy grading mirror.",
            {
                MakeColorPipelineFloatParam("grade.saturation", "Saturation", "Push or soften banded palette intensity.", 0.0, 2.0, 0.01, 1.15),
                MakeColorPipelineFloatParam("grade.contrast", "Contrast", "Stretch the banded palette mid-tones.", 0.0, 3.0, 0.01, 1.10),
            }),
        MakeColorPipelineFunction(
            "basin_default",
            "Basin Default",
            "Preserve the legacy basin grading defaults without exposing fake tuning controls.",
            {}),
        MakeColorPipelineFunction(
            "neutral_finish",
            "Neutral Finish",
            "Apply a neutral grading finish through the shared exposure, saturation, and contrast owners.",
            {
                MakeColorPipelineFloatParam("grade.exposure", "Exposure", "Set the overall neutral brightness.", 0.1, 3.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("grade.saturation", "Saturation", "Push or soften neutral palette intensity.", 0.0, 2.0, 0.01, 1.15),
                MakeColorPipelineFloatParam("grade.contrast", "Contrast", "Stretch neutral palette mid-tones.", 0.0, 3.0, 0.01, 1.10),
            }),
        MakeColorPipelineFunction(
            "tone_map_finish",
            "Tone Map Finish",
            "Apply tone mapping after tint, saturation, and contrast through the shared exposure, saturation, and contrast owners.",
            {
                MakeColorPipelineFloatParam("grade.exposure", "Exposure", "Set the overall tone-map finish exposure.", 0.1, 3.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("grade.saturation", "Saturation", "Push or soften tone-map finish palette intensity before tone mapping.", 0.0, 2.0, 0.01, 1.15),
                MakeColorPipelineFloatParam("grade.contrast", "Contrast", "Stretch tone-map finish mid-tones before tone mapping.", 0.0, 3.0, 0.01, 1.10),
            }),
        MakeColorPipelineFunction(
            "grade_glow",
            "Grade Glow",
            "Apply shared exposure, saturation, and contrast grading, then add a dedicated glow highlight finish through the runtime glow owner.",
            {
                MakeColorPipelineFloatParam("grade.exposure", "Exposure", "Set the overall grade-glow exposure before the highlight finish.", 0.1, 3.0, 0.01, 1.0),
                MakeColorPipelineFloatParam("grade.saturation", "Saturation", "Push or soften grade-glow palette intensity before the highlight finish.", 0.0, 2.0, 0.01, 1.15),
                MakeColorPipelineFloatParam("grade.contrast", "Contrast", "Stretch grade-glow mid-tones before the highlight finish.", 0.0, 3.0, 0.01, 1.10),
                MakeColorPipelineFloatParam("grade.glow", "Glow", "Control the strength of the runtime highlight bloom after grading.", 0.0, 2.0, 0.01, 0.25),
            }),
        MakeColorPipelineFunction(
            "balance_void_grade",
            "Balance Void Grade",
            "Apply the reusable balance-void grading manifold through dedicated runtime owners instead of the shared neutral or ExplainO-specific finish rails.",
            {
                MakeColorPipelineFloatParam("grade.balance_void", "Balance Void", "Bias the manifold warmer or cooler around the current luminance center.", -1.0, 1.0, 0.01, 0.0),
                MakeColorPipelineFloatParam("grade.chroma_tension", "Chroma Tension", "Tighten or relax the chroma stretch around the current accent band.", -1.0, 1.0, 0.01, 0.0),
                MakeColorPipelineFloatParam("grade.accent_bias", "Accent Bias", "Lift or suppress accent regions without reopening ExplainO-family grading work.", -1.0, 1.0, 0.01, 0.0),
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
            functionId == "root_index" ||
            functionId == "sdf_signed_distance" ||
            functionId == "sdf_inside_outside" ||
            functionId == "sdf_boundary_band" ||
            functionId == "sdf_normal_angle" ||
            functionId == "sdf_curvature";
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
    if (std::string(laneId) == "grading") {
        return functionId == "contrast_lift" ||
            functionId == "phase_finish" ||
            functionId == "band_finish" ||
            functionId == "basin_default" ||
            functionId == "neutral_finish" ||
            functionId == "tone_map_finish" ||
            functionId == "grade_glow" ||
            functionId == "balance_void_grade";
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

inline const std::vector<ColorPipelineLaneCatalog>& GetHardcodedColorPipelineLaneCatalogs() {
    static const std::vector<ColorPipelineLaneCatalog> catalogs = {
        {"source", "Source", "smooth_escape_ramp", FilterRuntimeBackedColorPipelineFunctions("source", BuildColorPipelineSignalFunctions())},
        {"shape", "Shape", "identity", FilterRuntimeBackedColorPipelineFunctions("shape", BuildColorPipelineShapeFunctions())},
        {"palette", "Palette", "heatmap", FilterRuntimeBackedColorPipelineFunctions("palette", BuildColorPipelinePaletteFunctions())},
        {"grading", "Grading", "contrast_lift", FilterRuntimeBackedColorPipelineFunctions("grading", BuildColorPipelineGradeFunctions())},
    };
    return catalogs;
}

struct ColorPipelineMetadataCatalogStorage {
    bool active = false;
    std::vector<std::string> lane_ids;
    std::vector<std::string> lane_labels;
    std::vector<std::string> lane_defaults;
    std::vector<ColorPipelineLaneCatalog> catalogs;
    std::vector<MaterializedColorPipelineCompatibility> compatibility;
};

inline bool TryBuildHardcodedColorPipelineSelectionFromLaneIds(
    const char* sourceFunctionId,
    const char* paletteFunctionId,
    ColorPipelineSelection* outPipeline,
    ColoringMode* outMode);

inline int CountHardcodedColorPipelineCompatibilityRows();

inline ColorPipelineMetadataCatalogStorage& MutableColorPipelineMetadataCatalogStorage() {
    static ColorPipelineMetadataCatalogStorage storage;
    return storage;
}

inline void RefreshColorPipelineMetadataCatalogPointers(ColorPipelineMetadataCatalogStorage* storage) {
    if (!storage) {
        return;
    }
    const std::size_t laneCount = storage->catalogs.size();
    for (std::size_t laneIndex = 0; laneIndex < laneCount; ++laneIndex) {
        storage->catalogs[laneIndex].lane_id = storage->lane_ids[laneIndex].c_str();
        storage->catalogs[laneIndex].label = storage->lane_labels[laneIndex].c_str();
        storage->catalogs[laneIndex].default_function_id = storage->lane_defaults[laneIndex].c_str();
    }
}

inline const FunctionDescriptor* FindColorPipelineFunctionDescriptorInCatalog(
    const ColorPipelineLaneCatalog& catalog,
    const std::string& functionId) {
    for (const FunctionDescriptor& descriptor : catalog.functions) {
        if (descriptor.id == functionId) {
            return &descriptor;
        }
    }
    return nullptr;
}

inline const FunctionParamDescriptor* FindColorPipelineParamDescriptorInFunction(
    const FunctionDescriptor& descriptor,
    const std::string& paramPath) {
    for (const FunctionParamDescriptor& param : descriptor.parameters) {
        if (param.path == paramPath) {
            return &param;
        }
    }
    return nullptr;
}

inline bool SetColorPipelineMetadataCatalogError(std::string* outError, const std::string& message) {
    if (outError) {
        *outError = message;
    }
    return false;
}

inline bool ConvertMaterializedColorPipelineParam(
    const MaterializedColorPipelineParam& source,
    const FunctionParamDescriptor& reference,
    FunctionParamDescriptor* outParam,
    std::string* outError) {
    if (!outParam) {
        return SetColorPipelineMetadataCatalogError(outError, "Materialized catalog param conversion requires output storage");
    }
    if (source.path != reference.path || source.type != reference.type || source.label != reference.label) {
        return SetColorPipelineMetadataCatalogError(
            outError,
            std::string("Materialized parameter does not preserve the hardcoded UI descriptor for '") + reference.path + "'");
    }
    if (source.has_min != reference.has_min ||
        source.has_max != reference.has_max ||
        source.has_step != reference.has_step) {
        return SetColorPipelineMetadataCatalogError(
            outError,
            std::string("Materialized parameter range flags do not preserve '") + reference.path + "'");
    }
    if (source.has_min && source.min_value != reference.min_value) {
        return SetColorPipelineMetadataCatalogError(
            outError,
            std::string("Materialized parameter min does not preserve '") + reference.path + "'");
    }
    if (source.has_max && source.max_value != reference.max_value) {
        return SetColorPipelineMetadataCatalogError(
            outError,
            std::string("Materialized parameter max does not preserve '") + reference.path + "'");
    }
    if (source.has_step && source.step_value != reference.step_value) {
        return SetColorPipelineMetadataCatalogError(
            outError,
            std::string("Materialized parameter step does not preserve '") + reference.path + "'");
    }

    FunctionParamDescriptor param;
    param.path = source.path;
    param.type = source.type;
    param.label = source.label;
    param.help = reference.help;
    param.has_min = source.has_min;
    param.min_value = source.min_value;
    param.has_max = source.has_max;
    param.max_value = source.max_value;
    param.has_step = source.has_step;
    param.step_value = source.step_value;
    param.has_default = source.has_default;
    if (source.has_default) {
        if (source.default_kind == "number") {
            if (source.type != "float" && source.type != "double" && source.type != "int") {
                return SetColorPipelineMetadataCatalogError(
                    outError,
                    std::string("Numeric default does not match materialized parameter type for '") + source.path + "'");
            }
            param.default_value = MakeColorPipelineNumberValue(source.number_default);
        } else if (source.default_kind == "bool") {
            if (source.type != "bool") {
                return SetColorPipelineMetadataCatalogError(
                    outError,
                    std::string("Bool default does not match materialized parameter type for '") + source.path + "'");
            }
            param.default_value = MakeColorPipelineBoolValue(source.bool_default);
        } else if (source.default_kind == "string") {
            if (source.type != "enum") {
                return SetColorPipelineMetadataCatalogError(
                    outError,
                    std::string("String default does not match materialized parameter type for '") + source.path + "'");
            }
            param.default_value = MakeColorPipelineStringValue(source.string_default.c_str());
        } else {
            return SetColorPipelineMetadataCatalogError(
                outError,
                std::string("Unknown materialized default kind for '") + source.path + "'");
        }
    }

    if (source.enum_options.size() != reference.options.size()) {
        return SetColorPipelineMetadataCatalogError(
            outError,
            std::string("Materialized enum options do not preserve '") + source.path + "'");
    }
    for (std::size_t optionIndex = 0; optionIndex < source.enum_options.size(); ++optionIndex) {
        if (source.enum_options[optionIndex] != reference.options[optionIndex].id) {
            return SetColorPipelineMetadataCatalogError(
                outError,
                std::string("Materialized enum option order does not preserve '") + source.path + "'");
        }
    }
    param.options = reference.options;

    *outParam = std::move(param);
    return true;
}

inline bool ConvertMaterializedColorPipelineFunction(
    const MaterializedColorPipelineFunction& source,
    const ColorPipelineLaneCatalog& referenceLane,
    const FunctionDescriptor& reference,
    FunctionDescriptor* outDescriptor,
    std::string* outError) {
    if (!outDescriptor) {
        return SetColorPipelineMetadataCatalogError(outError, "Materialized catalog function conversion requires output storage");
    }
    if (source.id != reference.id ||
        source.label != reference.name ||
        source.description != reference.description ||
        !source.runtime_backed ||
        !IsColorPipelineFunctionRuntimeBacked(referenceLane.lane_id, source.id)) {
        return SetColorPipelineMetadataCatalogError(
            outError,
            std::string("Materialized function does not preserve the hardcoded UI descriptor for '") + reference.id + "'");
    }
    if (source.params.size() != reference.parameters.size()) {
        return SetColorPipelineMetadataCatalogError(
            outError,
            std::string("Materialized parameter count does not preserve '") + reference.id + "'");
    }

    FunctionDescriptor descriptor;
    descriptor.id = source.id;
    descriptor.name = source.label;
    descriptor.description = source.description;
    descriptor.parameters.reserve(source.params.size());
    for (std::size_t paramIndex = 0; paramIndex < source.params.size(); ++paramIndex) {
        FunctionParamDescriptor param;
        if (!ConvertMaterializedColorPipelineParam(
                source.params[paramIndex],
                reference.parameters[paramIndex],
                &param,
                outError)) {
            return false;
        }
        descriptor.parameters.push_back(std::move(param));
    }
    *outDescriptor = std::move(descriptor);
    return true;
}

inline bool TryInstallColorPipelineMetadataCatalog(
    const MaterializedColorPipelineContract& contract,
    std::string* outError = nullptr) {
    const std::vector<ColorPipelineLaneCatalog>& hardcoded = GetHardcodedColorPipelineLaneCatalogs();
    if (contract.lanes.size() != hardcoded.size()) {
        return SetColorPipelineMetadataCatalogError(outError, "Materialized catalog lane count does not preserve the hardcoded catalog");
    }

    ColorPipelineMetadataCatalogStorage candidate;
    candidate.active = true;
    candidate.lane_ids.reserve(contract.lanes.size());
    candidate.lane_labels.reserve(contract.lanes.size());
    candidate.lane_defaults.reserve(contract.lanes.size());
    candidate.catalogs.reserve(contract.lanes.size());

    for (std::size_t laneIndex = 0; laneIndex < contract.lanes.size(); ++laneIndex) {
        const MaterializedColorPipelineLane& sourceLane = contract.lanes[laneIndex];
        const ColorPipelineLaneCatalog& referenceLane = hardcoded[laneIndex];
        if (sourceLane.id != referenceLane.lane_id ||
            sourceLane.label != referenceLane.label ||
            sourceLane.default_function_id != referenceLane.default_function_id) {
            return SetColorPipelineMetadataCatalogError(
                outError,
                std::string("Materialized lane does not preserve the hardcoded lane descriptor for '") + referenceLane.lane_id + "'");
        }
        if (sourceLane.functions.size() != referenceLane.functions.size()) {
            return SetColorPipelineMetadataCatalogError(
                outError,
                std::string("Materialized function count does not preserve lane '") + referenceLane.lane_id + "'");
        }

        candidate.lane_ids.push_back(sourceLane.id);
        candidate.lane_labels.push_back(sourceLane.label);
        candidate.lane_defaults.push_back(sourceLane.default_function_id);

        ColorPipelineLaneCatalog lane;
        lane.lane_id = candidate.lane_ids.back().c_str();
        lane.label = candidate.lane_labels.back().c_str();
        lane.default_function_id = candidate.lane_defaults.back().c_str();
        lane.functions.reserve(sourceLane.functions.size());

        for (std::size_t functionIndex = 0; functionIndex < sourceLane.functions.size(); ++functionIndex) {
            const MaterializedColorPipelineFunction& sourceFunction = sourceLane.functions[functionIndex];
            const FunctionDescriptor& referenceFunction = referenceLane.functions[functionIndex];
            FunctionDescriptor descriptor;
            if (!ConvertMaterializedColorPipelineFunction(
                    sourceFunction,
                    referenceLane,
                    referenceFunction,
                    &descriptor,
                    outError)) {
                return false;
            }
            lane.functions.push_back(std::move(descriptor));
        }
        candidate.catalogs.push_back(std::move(lane));
    }

    const int expectedCompatibilityCount = CountHardcodedColorPipelineCompatibilityRows();
    if (static_cast<int>(contract.compatibility.size()) != expectedCompatibilityCount) {
        return SetColorPipelineMetadataCatalogError(
            outError,
            "Materialized compatibility count does not preserve the hardcoded compatibility table");
    }
    candidate.compatibility.reserve(contract.compatibility.size());
    for (const MaterializedColorPipelineCompatibility& row : contract.compatibility) {
        ColorPipelineSelection expectedSelection;
        ColoringMode expectedMode = ColoringMode::smooth_escape;
        if (!TryBuildHardcodedColorPipelineSelectionFromLaneIds(
                row.source.c_str(),
                row.palette.c_str(),
                &expectedSelection,
                &expectedMode)) {
            return SetColorPipelineMetadataCatalogError(
                outError,
                std::string("Materialized compatibility row claims unsupported pair '") + row.source + "' + '" + row.palette + "'");
        }

        ColorPipelineSelection actualSelection;
        ColoringMode actualMode = ColoringMode::smooth_escape;
        if (!TryParseAdvancedColorSignalFunctionId(row.signal, &actualSelection.signal) ||
            !TryParseAdvancedColorPaletteFunctionId(row.palette_runtime, &actualSelection.palette) ||
            !TryParseAdvancedColorGradingFunctionId(row.grading, &actualSelection.grading) ||
            !TryParseColoringModeId(row.mode, &actualMode)) {
            return SetColorPipelineMetadataCatalogError(
                outError,
                std::string("Materialized compatibility row has an unknown runtime tuple for '") + row.source + "' + '" + row.palette + "'");
        }
        if (actualSelection.signal != expectedSelection.signal ||
            actualSelection.palette != expectedSelection.palette ||
            actualSelection.grading != expectedSelection.grading ||
            actualMode != expectedMode) {
            return SetColorPipelineMetadataCatalogError(
                outError,
                std::string("Materialized compatibility row does not preserve hardcoded tuple for '") + row.source + "' + '" + row.palette + "'");
        }
        if (row.reason.empty()) {
            return SetColorPipelineMetadataCatalogError(
                outError,
                std::string("Materialized compatibility row is missing a reason for '") + row.source + "' + '" + row.palette + "'");
        }
        candidate.compatibility.push_back(row);
    }

    ColorPipelineMetadataCatalogStorage& storage = MutableColorPipelineMetadataCatalogStorage();
    storage = std::move(candidate);
    RefreshColorPipelineMetadataCatalogPointers(&storage);
    if (outError) {
        outError->clear();
    }
    return true;
}

inline void ClearColorPipelineMetadataCatalogForTests() {
    MutableColorPipelineMetadataCatalogStorage() = ColorPipelineMetadataCatalogStorage{};
}

inline bool IsColorPipelineMetadataCatalogActive() {
    return MutableColorPipelineMetadataCatalogStorage().active;
}

inline std::string ColorPipelineCatalogAuthorityId() {
    return IsColorPipelineMetadataCatalogActive() ? "materialized_json" : "hardcoded";
}

inline int CountColorPipelineCatalogFunctions(const std::vector<ColorPipelineLaneCatalog>& catalogs) {
    int count = 0;
    for (const ColorPipelineLaneCatalog& catalog : catalogs) {
        count += static_cast<int>(catalog.functions.size());
    }
    return count;
}

inline bool IsColorPipelineMetadataCompatibilityActive() {
    const ColorPipelineMetadataCatalogStorage& storage = MutableColorPipelineMetadataCatalogStorage();
    return storage.active && !storage.compatibility.empty();
}

inline std::string ColorPipelineCompatibilityAuthorityId() {
    return IsColorPipelineMetadataCompatibilityActive() ? "materialized_json" : "hardcoded";
}

inline int CountActiveColorPipelineCompatibilityRows() {
    const ColorPipelineMetadataCatalogStorage& storage = MutableColorPipelineMetadataCatalogStorage();
    return IsColorPipelineMetadataCompatibilityActive() ? static_cast<int>(storage.compatibility.size()) : CountHardcodedColorPipelineCompatibilityRows();
}

inline const MaterializedColorPipelineCompatibility* FindActiveColorPipelineCompatibility(
    const std::string& sourceFunctionId,
    const std::string& paletteFunctionId) {
    const ColorPipelineMetadataCatalogStorage& storage = MutableColorPipelineMetadataCatalogStorage();
    if (!IsColorPipelineMetadataCompatibilityActive()) {
        return nullptr;
    }
    for (const MaterializedColorPipelineCompatibility& compatibility : storage.compatibility) {
        if (compatibility.source == sourceFunctionId && compatibility.palette == paletteFunctionId) {
            return &compatibility;
        }
    }
    return nullptr;
}

inline int CountHardcodedColorPipelineCompatibilityRows() {
    int count = 0;
    const std::vector<ColorPipelineLaneCatalog>& hardcoded = GetHardcodedColorPipelineLaneCatalogs();
    const ColorPipelineLaneCatalog* sourceLane = nullptr;
    const ColorPipelineLaneCatalog* paletteLane = nullptr;
    for (const ColorPipelineLaneCatalog& catalog : hardcoded) {
        if (std::strcmp(catalog.lane_id, "source") == 0) {
            sourceLane = &catalog;
        } else if (std::strcmp(catalog.lane_id, "palette") == 0) {
            paletteLane = &catalog;
        }
    }
    if (!sourceLane || !paletteLane) {
        return 0;
    }
    for (const FunctionDescriptor& sourceFunction : sourceLane->functions) {
        for (const FunctionDescriptor& paletteFunction : paletteLane->functions) {
            ColorPipelineSelection selection;
            ColoringMode mode = ColoringMode::smooth_escape;
            if (TryBuildHardcodedColorPipelineSelectionFromLaneIds(
                    sourceFunction.id.c_str(),
                    paletteFunction.id.c_str(),
                    &selection,
                    &mode)) {
                ++count;
            }
        }
    }
    return count;
}

inline const std::vector<ColorPipelineLaneCatalog>& GetColorPipelineLaneCatalogs() {
    const ColorPipelineMetadataCatalogStorage& storage = MutableColorPipelineMetadataCatalogStorage();
    if (storage.active) {
        return storage.catalogs;
    }
    return GetHardcodedColorPipelineLaneCatalogs();
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
    return FindColorPipelineFunctionDescriptorInCatalog(catalog, functionId);
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

inline double NormalizeImportedColorPipelineNumber(float value) {
    return std::round(static_cast<double>(value) * 1000000.0) / 1000000.0;
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

inline bool TryCopyColorPipelineParamValue(
    const std::vector<ColorPipelineParamState>& previousValues,
    const FunctionParamDescriptor& param,
    ColorPipelineParamState* ioValue) {
    if (!ioValue) {
        return false;
    }
    for (const ColorPipelineParamState& previousValue : previousValues) {
        if (previousValue.path != param.path || previousValue.type != param.type) {
            continue;
        }
        if (param.type == "bool") {
            ioValue->bool_value = previousValue.bool_value;
        } else if (param.type == "enum") {
            bool foundOption = false;
            for (const UISchemaOption& option : param.options) {
                if (option.id == previousValue.enum_value) {
                    foundOption = true;
                    break;
                }
            }
            if (!foundOption) {
                return false;
            }
            ioValue->enum_value = previousValue.enum_value;
        } else {
            ioValue->number_value = previousValue.number_value;
        }
        return true;
    }
    return false;
}

inline bool SetColorPipelineRowFunction(
    ColorPipelineRowState* ioRow,
    const FunctionDescriptor& descriptor,
    bool preserveExistingValues = true) {
    if (!ioRow) {
        return false;
    }

    const std::vector<ColorPipelineParamState> previousValues = preserveExistingValues ?
        ioRow->parameter_values :
        std::vector<ColorPipelineParamState>{};
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
        TryCopyColorPipelineParamValue(previousValues, param, &value);
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

inline bool TryGetColorPipelineParamEnum(
    const ColorPipelineRowState& row,
    const char* path,
    std::string* outValue,
    std::string* outError = nullptr) {
    if (!path || !outValue) {
        if (outError) *outError = "Advanced color enum parameter lookup requires a path and output storage";
        return false;
    }
    for (const ColorPipelineParamState& param : row.parameter_values) {
        if (param.path == path) {
            *outValue = param.enum_value;
            return true;
        }
    }
    if (outError) *outError = std::string("Missing advanced color parameter path '") + path + "' for function '" + row.function_id + "'";
    return false;
}

inline bool SetColorPipelineParamEnum(
    ColorPipelineRowState* ioRow,
    const char* path,
    const std::string& value,
    std::string* outError = nullptr) {
    if (!ioRow || !path) {
        if (outError) *outError = "Advanced color enum parameter import requires a row and parameter path";
        return false;
    }
    for (ColorPipelineParamState& param : ioRow->parameter_values) {
        if (param.path == path) {
            param.enum_value = value;
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
    if (ioRow->function_id == "phase_finish" || ioRow->function_id == "band_finish") {
        return SetColorPipelineParamNumber(ioRow, "grade.saturation", liveParams.color_saturation, outError) &&
            SetColorPipelineParamNumber(ioRow, "grade.contrast", liveParams.color_contrast, outError);
    }
    if (ioRow->function_id == "neutral_finish" || ioRow->function_id == "tone_map_finish") {
        return SetColorPipelineParamNumber(ioRow, "grade.exposure", liveParams.exposure, outError) &&
            SetColorPipelineParamNumber(ioRow, "grade.saturation", liveParams.color_saturation, outError) &&
            SetColorPipelineParamNumber(ioRow, "grade.contrast", liveParams.color_contrast, outError);
    }
    if (ioRow->function_id == "grade_glow") {
        return SetColorPipelineParamNumber(ioRow, "grade.exposure", liveParams.exposure, outError) &&
            SetColorPipelineParamNumber(ioRow, "grade.saturation", liveParams.color_saturation, outError) &&
            SetColorPipelineParamNumber(ioRow, "grade.contrast", liveParams.color_contrast, outError) &&
            SetColorPipelineParamNumber(ioRow, "grade.glow", liveParams.color_glow, outError);
    }
    if (ioRow->function_id == "balance_void_grade") {
        return SetColorPipelineParamNumber(ioRow, "grade.balance_void", NormalizeImportedColorPipelineNumber(liveParams.color_balance_void), outError) &&
            SetColorPipelineParamNumber(ioRow, "grade.chroma_tension", NormalizeImportedColorPipelineNumber(liveParams.color_chroma_tension), outError) &&
            SetColorPipelineParamNumber(ioRow, "grade.accent_bias", NormalizeImportedColorPipelineNumber(liveParams.color_accent_bias), outError);
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
    if (ioRow->function_id == "sdf_signed_distance" ||
        ioRow->function_id == "sdf_inside_outside" ||
        ioRow->function_id == "sdf_boundary_band" ||
        ioRow->function_id == "sdf_normal_angle" ||
        ioRow->function_id == "sdf_curvature") {
        return true;
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

inline bool TryBuildHardcodedColorPipelineSelectionFromLaneIds(
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
    if (std::strcmp(sourceFunctionId, "sdf_normal_angle") == 0 && std::strcmp(paletteFunctionId, "phase_wheel_palette") == 0) {
        *outPipeline = {ColorSignal::sdf_normal_angle, ColorPalette::phase_wheel, ColorGradingPreset::phase_default};
        *outMode = ColoringMode::phase;
        return true;
    }
    const bool sdfHeatmapSource =
        std::strcmp(sourceFunctionId, "sdf_signed_distance") == 0 ||
        std::strcmp(sourceFunctionId, "sdf_inside_outside") == 0 ||
        std::strcmp(sourceFunctionId, "sdf_boundary_band") == 0 ||
        std::strcmp(sourceFunctionId, "sdf_curvature") == 0;
    if (sdfHeatmapSource &&
        (std::strcmp(paletteFunctionId, "heatmap") == 0 ||
         std::strcmp(paletteFunctionId, "explaino_cmap") == 0)) {
        ColorSignal signal = ColorSignal::sdf_signed_distance;
        TryParseAdvancedColorSignalFunctionId(sourceFunctionId, &signal);
        *outPipeline = {
            signal,
            std::strcmp(paletteFunctionId, "explaino_cmap") == 0 ? ColorPalette::explaino_cmap : ColorPalette::cyclic_escape,
            ColorGradingPreset::escape_default,
        };
        *outMode = ColoringMode::smooth_escape;
        return true;
    }
    return false;
}

inline bool TryBuildMaterializedColorPipelineSelectionFromLaneIds(
    const char* sourceFunctionId,
    const char* paletteFunctionId,
    ColorPipelineSelection* outPipeline,
    ColoringMode* outMode) {
    if (!sourceFunctionId || sourceFunctionId[0] == '\0' ||
        !paletteFunctionId || paletteFunctionId[0] == '\0' ||
        !outPipeline || !outMode) {
        return false;
    }
    const MaterializedColorPipelineCompatibility* compatibility =
        FindActiveColorPipelineCompatibility(sourceFunctionId, paletteFunctionId);
    if (!compatibility) {
        return false;
    }
    ColorPipelineSelection selection;
    ColoringMode mode = ColoringMode::smooth_escape;
    if (!TryParseAdvancedColorSignalFunctionId(compatibility->signal, &selection.signal) ||
        !TryParseAdvancedColorPaletteFunctionId(compatibility->palette_runtime, &selection.palette) ||
        !TryParseAdvancedColorGradingFunctionId(compatibility->grading, &selection.grading) ||
        !TryParseColoringModeId(compatibility->mode, &mode)) {
        return false;
    }
    *outPipeline = selection;
    *outMode = mode;
    return true;
}

inline bool TryBuildColorPipelineSelectionFromLaneIds(
    const char* sourceFunctionId,
    const char* paletteFunctionId,
    ColorPipelineSelection* outPipeline,
    ColoringMode* outMode) {
    if (IsColorPipelineMetadataCompatibilityActive()) {
        return TryBuildMaterializedColorPipelineSelectionFromLaneIds(
            sourceFunctionId,
            paletteFunctionId,
            outPipeline,
            outMode);
    }
    return TryBuildHardcodedColorPipelineSelectionFromLaneIds(
        sourceFunctionId,
        paletteFunctionId,
        outPipeline,
        outMode);
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
    } else if (row.function_id == "phase_finish" || row.function_id == "band_finish") {
        double saturation = 0.0;
        double contrast = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "grade.saturation", &saturation, outError) ||
            !TryGetColorPipelineParamNumber(row, "grade.contrast", &contrast, outError) ||
            !ValidateColorPipelineParamRange("grade.saturation", saturation, 0.0, 2.0, outError) ||
            !ValidateColorPipelineParamRange("grade.contrast", contrast, 0.0, 3.0, outError)) {
            return false;
        }
        assignFloat(&ioParams->color_saturation, static_cast<float>(saturation));
        assignFloat(&ioParams->color_contrast, static_cast<float>(contrast));
    } else if (row.function_id == "neutral_finish" || row.function_id == "tone_map_finish") {
        double exposure = 0.0;
        double saturation = 0.0;
        double contrast = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "grade.exposure", &exposure, outError) ||
            !TryGetColorPipelineParamNumber(row, "grade.saturation", &saturation, outError) ||
            !TryGetColorPipelineParamNumber(row, "grade.contrast", &contrast, outError) ||
            !ValidateColorPipelineParamRange("grade.exposure", exposure, 0.1, 3.0, outError) ||
            !ValidateColorPipelineParamRange("grade.saturation", saturation, 0.0, 2.0, outError) ||
            !ValidateColorPipelineParamRange("grade.contrast", contrast, 0.0, 3.0, outError)) {
            return false;
        }
        assignFloat(&ioParams->exposure, static_cast<float>(exposure));
        assignFloat(&ioParams->color_saturation, static_cast<float>(saturation));
        assignFloat(&ioParams->color_contrast, static_cast<float>(contrast));
    } else if (row.function_id == "grade_glow") {
        double exposure = 0.0;
        double saturation = 0.0;
        double contrast = 0.0;
        double glow = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "grade.exposure", &exposure, outError) ||
            !TryGetColorPipelineParamNumber(row, "grade.saturation", &saturation, outError) ||
            !TryGetColorPipelineParamNumber(row, "grade.contrast", &contrast, outError) ||
            !TryGetColorPipelineParamNumber(row, "grade.glow", &glow, outError) ||
            !ValidateColorPipelineParamRange("grade.exposure", exposure, 0.1, 3.0, outError) ||
            !ValidateColorPipelineParamRange("grade.saturation", saturation, 0.0, 2.0, outError) ||
            !ValidateColorPipelineParamRange("grade.contrast", contrast, 0.0, 3.0, outError) ||
            !ValidateColorPipelineParamRange("grade.glow", glow, 0.0, 2.0, outError)) {
            return false;
        }
        assignFloat(&ioParams->exposure, static_cast<float>(exposure));
        assignFloat(&ioParams->color_saturation, static_cast<float>(saturation));
        assignFloat(&ioParams->color_contrast, static_cast<float>(contrast));
        assignFloat(&ioParams->color_glow, static_cast<float>(glow));
    } else if (row.function_id == "balance_void_grade") {
        double balanceVoid = 0.0;
        double chromaTension = 0.0;
        double accentBias = 0.0;
        if (!TryGetColorPipelineParamNumber(row, "grade.balance_void", &balanceVoid, outError) ||
            !TryGetColorPipelineParamNumber(row, "grade.chroma_tension", &chromaTension, outError) ||
            !TryGetColorPipelineParamNumber(row, "grade.accent_bias", &accentBias, outError) ||
            !ValidateColorPipelineParamRange("grade.balance_void", balanceVoid, -1.0, 1.0, outError) ||
            !ValidateColorPipelineParamRange("grade.chroma_tension", chromaTension, -1.0, 1.0, outError) ||
            !ValidateColorPipelineParamRange("grade.accent_bias", accentBias, -1.0, 1.0, outError)) {
            return false;
        }
        assignFloat(&ioParams->color_balance_void, static_cast<float>(balanceVoid));
        assignFloat(&ioParams->color_chroma_tension, static_cast<float>(chromaTension));
        assignFloat(&ioParams->color_accent_bias, static_cast<float>(accentBias));
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

    const bool isEscapeLikeGrading =
        pipeline.grading == ColorGradingPreset::escape_default ||
        pipeline.grading == ColorGradingPreset::neutral_default ||
        pipeline.grading == ColorGradingPreset::tone_map_default ||
        pipeline.grading == ColorGradingPreset::glow_default ||
        pipeline.grading == ColorGradingPreset::balance_void_default;

    if (pipeline.signal == ColorSignal::smooth_escape &&
        pipeline.palette == ColorPalette::cyclic_escape &&
        isEscapeLikeGrading) {
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
        isEscapeLikeGrading) {
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
        isEscapeLikeGrading) {
        if (outSourceFunctionId) *outSourceFunctionId = "root_proximity";
        if (outPaletteFunctionId) *outPaletteFunctionId = "heatmap";
        return true;
    }
    if (pipeline.signal == ColorSignal::smooth_escape &&
        pipeline.palette == ColorPalette::explaino_cmap &&
        isEscapeLikeGrading) {
        if (outSourceFunctionId) *outSourceFunctionId = "smooth_escape_ramp";
        if (outPaletteFunctionId) *outPaletteFunctionId = "explaino_cmap";
        return true;
    }
    if (pipeline.signal == ColorSignal::escape_magnitude &&
        pipeline.palette == ColorPalette::explaino_cmap &&
        isEscapeLikeGrading) {
        if (outSourceFunctionId) *outSourceFunctionId = "escape_magnitude";
        if (outPaletteFunctionId) *outPaletteFunctionId = "explaino_cmap";
        return true;
    }
    if (pipeline.signal == ColorSignal::root_proximity &&
        pipeline.palette == ColorPalette::explaino_cmap &&
        isEscapeLikeGrading) {
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
    if (pipeline.signal == ColorSignal::sdf_normal_angle &&
        pipeline.palette == ColorPalette::phase_wheel &&
        pipeline.grading == ColorGradingPreset::phase_default) {
        if (outSourceFunctionId) *outSourceFunctionId = "sdf_normal_angle";
        if (outPaletteFunctionId) *outPaletteFunctionId = "phase_wheel_palette";
        return true;
    }
    const bool isSdfHeatmapSignal =
        pipeline.signal == ColorSignal::sdf_signed_distance ||
        pipeline.signal == ColorSignal::sdf_inside_outside ||
        pipeline.signal == ColorSignal::sdf_boundary_band ||
        pipeline.signal == ColorSignal::sdf_curvature;
    if (isSdfHeatmapSignal &&
        (pipeline.palette == ColorPalette::cyclic_escape || pipeline.palette == ColorPalette::explaino_cmap) &&
        isEscapeLikeGrading) {
        if (outSourceFunctionId) *outSourceFunctionId = AdvancedColorSignalFunctionId(pipeline.signal);
        if (outPaletteFunctionId) {
            *outPaletteFunctionId = pipeline.palette == ColorPalette::explaino_cmap ? "explaino_cmap" : "heatmap";
        }
        return true;
    }
    return false;
}

} // namespace color_pipeline_core
