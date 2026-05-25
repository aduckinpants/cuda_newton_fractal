#include "diagnostics_state_io.h"

#define COLOR_PIPELINE_WINDOW_NO_IMGUI
#include "color_pipeline_window.h"
#undef COLOR_PIPELINE_WINDOW_NO_IMGUI
#include "enum_id_utils.h"
#include "escape_time_specialized_formulas.h"
#include "explaino_seed.h"
#include "fractal_family_rules.h"
#include "json_min.h"

#include <cmath>
#include <cstring>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace {

bool ReadTextFile(const std::filesystem::path& path, std::string* outText, std::string* outError) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        if (outError) *outError = "Failed to open state file: " + path.string();
        return false;
    }

    std::ostringstream text;
    text << file.rdbuf();
    if (!file.good() && !file.eof()) {
        if (outError) *outError = "Failed to read state file: " + path.string();
        return false;
    }
    if (outText) *outText = text.str();
    return true;
}

bool GetRequiredObject(const json_min::Value& object, const char* key, const json_min::Value** outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_object()) {
        if (outError) *outError = std::string("Missing or invalid object field: ") + key;
        return false;
    }
    if (outValue) *outValue = value;
    return true;
}

bool GetRequiredArray(const json_min::Value& object, const char* key, const json_min::Value** outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_array()) {
        if (outError) *outError = std::string("Missing or invalid array field: ") + key;
        return false;
    }
    if (outValue) *outValue = value;
    return true;
}

bool GetRequiredNumber(const json_min::Value& object, const char* key, double* outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_number()) {
        if (outError) *outError = std::string("Missing or invalid number field: ") + key;
        return false;
    }
    if (outValue) *outValue = value->as_number();
    return true;
}

bool GetRequiredBool(const json_min::Value& object, const char* key, bool* outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_bool()) {
        if (outError) *outError = std::string("Missing or invalid bool field: ") + key;
        return false;
    }
    if (outValue) *outValue = value->as_bool();
    return true;
}

bool GetRequiredString(const json_min::Value& object, const char* key, std::string* outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_string()) {
        if (outError) *outError = std::string("Missing or invalid string field: ") + key;
        return false;
    }
    if (outValue) *outValue = value->as_string();
    return true;
}

bool TryGetOptionalString(const json_min::Value& object, const char* key, std::string* outValue) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_string()) return false;
    if (outValue) *outValue = value->as_string();
    return true;
}

bool GetOptionalNumber(const json_min::Value& object, const char* key, double* outValue, bool* outPresent, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value) {
        if (outPresent) *outPresent = false;
        return true;
    }
    if (!value->is_number()) {
        if (outError) *outError = std::string("Invalid optional number field: ") + key;
        return false;
    }
    if (outValue) *outValue = value->as_number();
    if (outPresent) *outPresent = true;
    return true;
}

bool GetOptionalBool(const json_min::Value& object, const char* key, bool* outValue, bool* outPresent, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value) {
        if (outPresent) *outPresent = false;
        return true;
    }
    if (!value->is_bool()) {
        if (outError) *outError = std::string("Invalid optional bool field: ") + key;
        return false;
    }
    if (outValue) *outValue = value->as_bool();
    if (outPresent) *outPresent = true;
    return true;
}

bool ParseOptionalLensDownsample(const json_min::Value& lensObject, LensSettings* ioLens, std::string* outError) {
    double downsampleRaw = static_cast<double>(ioLens->downsample);
    bool hasDownsample = false;
    if (!GetOptionalNumber(lensObject, "downsample", &downsampleRaw, &hasDownsample, outError)) return false;
    if (hasDownsample) {
        if (!std::isfinite(downsampleRaw) ||
            std::floor(downsampleRaw) != downsampleRaw ||
            downsampleRaw < 1.0 ||
            downsampleRaw > static_cast<double>(INT_MAX)) {
            if (outError) *outError = "Invalid lens.downsample field";
            return false;
        }
        ioLens->downsample = static_cast<int>(downsampleRaw);
    }
    return true;
}

bool ParseOptionalLensOverlayMode(const json_min::Value& lensObject, LensSettings* ioLens, std::string* outError) {
    std::string overlayModeId;
    if (TryGetOptionalString(lensObject, "sdf_overlay_mode", &overlayModeId)) {
        if (!TryParseLensSdfOverlayModeId(overlayModeId, &ioLens->sdf_overlay_mode)) {
            if (outError) *outError = "Unknown lens.sdf_overlay_mode: " + overlayModeId;
            return false;
        }
    } else if (const json_min::Value* overlayModeValue = lensObject.get("sdf_overlay_mode")) {
        if (!overlayModeValue->is_string()) {
            if (outError) *outError = "Invalid lens.sdf_overlay_mode field";
            return false;
        }
    }
    return true;
}

bool ParseOptionalLensFloat(const json_min::Value& lensObject, const char* key, float* outValue, std::string* outError) {
    double rawValue = static_cast<double>(*outValue);
    if (!GetOptionalNumber(lensObject, key, &rawValue, nullptr, outError)) return false;
    if (!std::isfinite(rawValue)) {
        if (outError) *outError = "Invalid nonfinite Lens field";
        return false;
    }
    *outValue = static_cast<float>(rawValue);
    return true;
}

bool ParseOptionalLensState(const json_min::Value& root, LensSettings* outLens, std::string* outError) {
    if (!outLens) return true;
    LensSettings lens{};
    const json_min::Value* lensObject = root.get("lens");
    if (!lensObject) {
        *outLens = lens;
        return true;
    }
    if (!lensObject->is_object()) {
        if (outError) *outError = "lens must be an object";
        return false;
    }
    if (!GetOptionalBool(*lensObject, "enabled", &lens.enabled, nullptr, outError)) return false;
    if (!ParseOptionalLensDownsample(*lensObject, &lens, outError)) return false;
    if (!ParseOptionalLensOverlayMode(*lensObject, &lens, outError)) return false;
    if (!ParseOptionalLensFloat(*lensObject, "sdf_overlay_opacity", &lens.sdf_overlay_opacity, outError)) return false;
    if (!ParseOptionalLensFloat(*lensObject, "sdf_overlay_band_px", &lens.sdf_overlay_band_px, outError)) return false;
    *outLens = lens;
    return true;
}

bool ParseFixedFloatArray(const json_min::Value& value, const char* fieldName, float* outValues, size_t expectedCount, std::string* outError) {
    if (!value.is_array()) {
        if (outError) *outError = std::string(fieldName) + " must be an array";
        return false;
    }
    if (value.as_array().size() != expectedCount) {
        if (outError) *outError = std::string(fieldName) + " must contain exactly " + std::to_string(expectedCount) + " numbers";
        return false;
    }
    for (size_t index = 0; index < expectedCount; ++index) {
        const json_min::Value& entry = value.as_array()[index];
        if (!entry.is_number()) {
            if (outError) *outError = std::string(fieldName) + " must contain only numbers";
            return false;
        }
        outValues[index] = static_cast<float>(entry.as_number());
    }
    return true;
}

bool ParseOptionalExplainoRoots(const json_min::Value& paramsObject, int rootCount, KernelParams* outParams, bool* outPresent, std::string* outError) {
    if (outPresent) *outPresent = false;
    const json_min::Value* rootsValue = paramsObject.get("explaino_roots");
    if (!rootsValue) return true;
    if (outPresent) *outPresent = true;
    if (!rootsValue->is_array()) {
        if (outError) *outError = "explaino_roots must be an array";
        return false;
    }
    if (rootCount < 0 || rootCount > 4) {
        if (outError) *outError = "explaino_root_count must be within [0, 4] when explaino_roots is present";
        return false;
    }
    if (rootsValue->as_array().size() != static_cast<size_t>(rootCount)) {
        if (outError) *outError = "explaino_roots length must match explaino_root_count";
        return false;
    }
    for (int index = 0; index < rootCount; ++index) {
        const json_min::Value& rootValue = rootsValue->as_array()[static_cast<size_t>(index)];
        if (!rootValue.is_object()) {
            if (outError) *outError = "explaino_roots entries must be objects";
            return false;
        }
        double x = 0.0;
        double y = 0.0;
        if (!GetRequiredNumber(rootValue, "x", &x, outError)) return false;
        if (!GetRequiredNumber(rootValue, "y", &y, outError)) return false;
        outParams->explaino_roots[index] = {static_cast<float>(x), static_cast<float>(y)};
    }
    return true;
}

bool SidecarOrientationFieldsAreFinite(const SidecarOrientationVector& orientation) {
    return std::isfinite(orientation.field_embedding_stats) &&
           std::isfinite(orientation.slime_energy_delta) &&
           std::isfinite(orientation.busy_beaver_metrics) &&
           std::isfinite(orientation.decode_stability) &&
           std::isfinite(orientation.diff_magnitude);
}

bool ParseSidecarHashField(const json_min::Value& object,
    const char* key,
    std::uint64_t* outValue,
    std::string* outError) {
    constexpr double kMaxExactJsonInteger = 9007199254740991.0;

    const json_min::Value* value = object.get(key);
    if (!value) {
        if (outError) *outError = std::string("Missing sidecar_orientation field: ") + key;
        return false;
    }

    if (value->is_string()) {
        const std::string text = value->as_string();
        if (text.empty()) {
            if (outError) *outError = std::string("Invalid sidecar_orientation field: ") + key;
            return false;
        }

        std::size_t consumed = 0;
        try {
            const unsigned long long parsed = std::stoull(text, &consumed, 10);
            if (consumed != text.size()) {
                if (outError) *outError = std::string("Invalid sidecar_orientation field: ") + key;
                return false;
            }
            if (outValue) *outValue = static_cast<std::uint64_t>(parsed);
            return true;
        } catch (...) {
            if (outError) *outError = std::string("Invalid sidecar_orientation field: ") + key;
            return false;
        }
    }

    if (!value->is_number()) {
        if (outError) *outError = std::string("Invalid sidecar_orientation field: ") + key;
        return false;
    }

    const double numeric = value->as_number();
    if (numeric < 0.0 || std::floor(numeric) != numeric || numeric > kMaxExactJsonInteger) {
        if (outError) {
            *outError = std::string("sidecar_orientation.") + key +
                " must be an exact non-negative integer; use a quoted decimal string for full 64-bit values";
        }
        return false;
    }

    if (outValue) *outValue = static_cast<std::uint64_t>(numeric);
    return true;
}

bool ParseOptionalSidecarOrientation(const json_min::Value& root,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    std::string* outError) {
    if (outOrientation) *outOrientation = {};
    if (outHasOrientation) *outHasOrientation = false;

    const json_min::Value* orientationObject = root.get("sidecar_orientation");
    if (!orientationObject) return true;
    if (!orientationObject->is_object()) {
        if (outError) *outError = "Missing or invalid object field: sidecar_orientation";
        return false;
    }

    SidecarOrientationVector orientation{};
    if (!ParseSidecarHashField(*orientationObject, "import_signature", &orientation.import_signature, outError)) return false;
    if (!ParseSidecarHashField(*orientationObject, "pack_projection_hash", &orientation.pack_projection_hash, outError)) return false;
    if (!GetRequiredNumber(*orientationObject, "field_embedding_stats", &orientation.field_embedding_stats, outError)) return false;
    if (!GetRequiredNumber(*orientationObject, "slime_energy_delta", &orientation.slime_energy_delta, outError)) return false;
    if (!GetRequiredNumber(*orientationObject, "busy_beaver_metrics", &orientation.busy_beaver_metrics, outError)) return false;
    if (!GetRequiredNumber(*orientationObject, "decode_stability", &orientation.decode_stability, outError)) return false;
    if (!GetRequiredNumber(*orientationObject, "diff_magnitude", &orientation.diff_magnitude, outError)) return false;
    if (!SidecarOrientationFieldsAreFinite(orientation)) {
        if (outError) *outError = "sidecar_orientation must contain only finite values";
        return false;
    }

    if (outOrientation) *outOrientation = orientation;
    if (outHasOrientation) *outHasOrientation = true;
    return true;
}

bool ParseFractalType(const std::string& text, FractalType* outType) {
    return TryParseFractalTypeId(text, outType);
}

bool ParsePolyKind(int rawValue, PolyKind* outKind) {
    if (rawValue == static_cast<int>(PolyKind::z3_minus_1)) { if (outKind) *outKind = PolyKind::z3_minus_1; return true; }
    if (rawValue == static_cast<int>(PolyKind::z4_minus_1)) { if (outKind) *outKind = PolyKind::z4_minus_1; return true; }
    if (rawValue == static_cast<int>(PolyKind::custom)) { if (outKind) *outKind = PolyKind::custom; return true; }
    return false;
}

bool ParseColoringMode(const std::string& text, ColoringMode* outMode) {
    return TryParseColoringModeId(text, outMode);
}

bool ParseColorSignal(const std::string& text, ColorSignal* outSignal) {
    return TryParseColorSignalId(text, outSignal);
}

bool ParseColorPalette(const std::string& text, ColorPalette* outPalette) {
    return TryParseColorPaletteId(text, outPalette);
}

bool ParseColorGradingPreset(const std::string& text, ColorGradingPreset* outGrading) {
    return TryParseColorGradingPresetId(text, outGrading);
}

bool ParseColorPipelineShape(const std::string& text, ColorPipelineShape* outShape) {
    return TryParseColorPipelineShapeId(text, outShape);
}

bool ParseSampleTier(const std::string& text, SampleTier* outTier) {
    return TryParseSampleTierId(text, outTier);
}

bool ParseCounterfactualPairRootFamily(const std::string& text, CounterfactualPairRootFamily* outRootFamily) {
    return TryParseCounterfactualPairRootFamilyId(text, outRootFamily);
}

bool ParseCounterfactualPairFrame(const std::string& text, CounterfactualPairFrame* outFrame) {
    return TryParseCounterfactualPairFrameId(text, outFrame);
}

bool ParseProjectionAndFlowRootFamily(const std::string& text, ProjectionAndFlowRootFamily* outRootFamily) {
    return TryParseProjectionAndFlowRootFamilyId(text, outRootFamily);
}

bool TryResolveCounterfactualPairRootFamilyForPolyKindLocal(PolyKind kind, CounterfactualPairRootFamily* outRootFamily) {
    if (kind == PolyKind::z3_minus_1) {
        if (outRootFamily) *outRootFamily = CounterfactualPairRootFamily::cubic_unit_roots;
        return true;
    }
    if (kind == PolyKind::z4_minus_1) {
        if (outRootFamily) *outRootFamily = CounterfactualPairRootFamily::quartic_unit_roots;
        return true;
    }
    return false;
}

bool TryResolveProjectionAndFlowRootFamilyForPolyKindLocal(PolyKind kind, ProjectionAndFlowRootFamily* outRootFamily) {
    if (kind == PolyKind::z3_minus_1) {
        if (outRootFamily) *outRootFamily = ProjectionAndFlowRootFamily::cubic_unit_roots;
        return true;
    }
    if (kind == PolyKind::z4_minus_1) {
        if (outRootFamily) *outRootFamily = ProjectionAndFlowRootFamily::quartic_unit_roots;
        return true;
    }
    return false;
}

void SyncCounterfactualPairRootFamilyPresetLocal(KernelParams* ioParams) {
    if (!ioParams) {
        return;
    }
    switch (ioParams->counterfactual_pair_root_family) {
    case CounterfactualPairRootFamily::cubic_unit_roots:
        ioParams->poly_kind = PolyKind::z3_minus_1;
        ioParams->poly_coeffs[0] = -1.0f;
        ioParams->poly_coeffs[1] = 0.0f;
        ioParams->poly_coeffs[2] = 0.0f;
        ioParams->poly_coeffs[3] = 1.0f;
        ioParams->poly_coeffs[4] = 0.0f;
        break;
    case CounterfactualPairRootFamily::quartic_unit_roots:
        ioParams->poly_kind = PolyKind::z4_minus_1;
        ioParams->poly_coeffs[0] = -1.0f;
        ioParams->poly_coeffs[1] = 0.0f;
        ioParams->poly_coeffs[2] = 0.0f;
        ioParams->poly_coeffs[3] = 0.0f;
        ioParams->poly_coeffs[4] = 1.0f;
        break;
    }
}

void SyncProjectionAndFlowRootFamilyPresetLocal(KernelParams* ioParams) {
    if (!ioParams) {
        return;
    }
    switch (ioParams->projection_and_flow_root_family) {
    case ProjectionAndFlowRootFamily::cubic_unit_roots:
        ioParams->poly_kind = PolyKind::z3_minus_1;
        ioParams->poly_coeffs[0] = -1.0f;
        ioParams->poly_coeffs[1] = 0.0f;
        ioParams->poly_coeffs[2] = 0.0f;
        ioParams->poly_coeffs[3] = 1.0f;
        ioParams->poly_coeffs[4] = 0.0f;
        break;
    case ProjectionAndFlowRootFamily::quartic_unit_roots:
        ioParams->poly_kind = PolyKind::z4_minus_1;
        ioParams->poly_coeffs[0] = -1.0f;
        ioParams->poly_coeffs[1] = 0.0f;
        ioParams->poly_coeffs[2] = 0.0f;
        ioParams->poly_coeffs[3] = 0.0f;
        ioParams->poly_coeffs[4] = 1.0f;
        break;
    }
}

void ResetLegacyColorSourceMirror(KernelParams* ioParams) {
    if (!ioParams) {
        return;
    }
    ioParams->color_smooth_escape_scale = 1.0f;
    ioParams->color_smooth_escape_bias = 0.0f;
    ioParams->color_phase_signal_offset = 0.0f;
    ioParams->color_phase_wrap_cycles = 1.0f;
    ioParams->color_iteration_band_count = 8;
    ioParams->color_iteration_band_softness = 0.35f;
    ioParams->color_escape_magnitude_scale = 1.0f;
    ioParams->color_escape_magnitude_bias = 0.0f;
    ioParams->color_orbit_stripe_frequency = 1.0f;
    ioParams->color_orbit_stripe_phase = 0.0f;
    ioParams->color_root_proximity_scale = 1.0f;
    ioParams->color_root_proximity_bias = 0.0f;
}

void ApplyLegacyColorSourceMirrorParams(const ColorPipelineSourceStackEntry& sourceEntry, KernelParams* ioParams) {
    switch (sourceEntry.signal) {
    case ColorSignal::smooth_escape:
        ioParams->color_smooth_escape_scale = sourceEntry.params.scale;
        ioParams->color_smooth_escape_bias = sourceEntry.params.bias;
        break;
    case ColorSignal::phase_angle:
        ioParams->color_phase_signal_offset = sourceEntry.params.phase_offset;
        ioParams->color_phase_wrap_cycles = sourceEntry.params.wrap_cycles;
        break;
    case ColorSignal::iteration_bands:
        ioParams->color_iteration_band_count = sourceEntry.params.band_count;
        ioParams->color_iteration_band_softness = sourceEntry.params.softness;
        break;
    case ColorSignal::escape_magnitude:
        ioParams->color_escape_magnitude_scale = sourceEntry.params.magnitude_scale;
        ioParams->color_escape_magnitude_bias = sourceEntry.params.magnitude_bias;
        break;
    case ColorSignal::orbit_stripe:
        ioParams->color_orbit_stripe_frequency = sourceEntry.params.stripe_frequency;
        ioParams->color_orbit_stripe_phase = sourceEntry.params.stripe_phase;
        break;
    case ColorSignal::root_proximity:
        ioParams->color_root_proximity_scale = sourceEntry.params.proximity_scale;
        ioParams->color_root_proximity_bias = sourceEntry.params.proximity_bias;
        break;
    case ColorSignal::root_index:
    case ColorSignal::iteration_count:
    default:
        break;
    }
}

void MirrorLegacyColorSourceFromStackEntry(const ColorPipelineSourceStackEntry& sourceEntry, KernelParams* ioParams) {
    if (!ioParams) return;
    ResetLegacyColorSourceMirror(ioParams);
    ioParams->color_pipeline.signal = sourceEntry.signal;
    ApplyLegacyColorSourceMirrorParams(sourceEntry, ioParams);
}

void ClearColorSourceStack(KernelParams* ioParams) {
    if (!ioParams) {
        return;
    }
    ioParams->color_source_stack_count = 0;
    for (ColorPipelineSourceStackEntry& sourceEntry : ioParams->color_source_stack) {
        sourceEntry = {};
    }
}

void ResetLegacyColorShapeMirror(KernelParams* ioParams) {
    if (!ioParams) {
        return;
    }
    ioParams->color_shape = ColorPipelineShape::identity;
    ioParams->color_shape_offset = 0.0f;
    ioParams->color_shape_scale = 1.0f;
    ioParams->color_shape_repeat_frequency = 8.0f;
    ioParams->color_shape_repeat_phase = 0.0f;
    ioParams->color_shape_posterize_steps = 6;
    ioParams->color_shape_posterize_mix = 1.0f;
    ioParams->color_shape_bias = 0.5f;
    ioParams->color_shape_gain = 0.5f;
    ioParams->color_shape_window_center = 0.5f;
    ioParams->color_shape_window_width = 1.0f;
    ioParams->color_shape_window_softness = 0.0f;
}

void MirrorLegacyColorShapeFromStackEntry(const ColorPipelineShapeStackEntry& shapeEntry, KernelParams* ioParams) {
    if (!ioParams) return;
    ResetLegacyColorShapeMirror(ioParams);
    ioParams->color_shape = shapeEntry.shape;
    switch (shapeEntry.shape) {
    case ColorPipelineShape::offset_scale:
        ioParams->color_shape_offset = shapeEntry.params.offset;
        ioParams->color_shape_scale = shapeEntry.params.scale;
        break;
    case ColorPipelineShape::repeat:
    case ColorPipelineShape::mirror_repeat:
        ioParams->color_shape_repeat_frequency = shapeEntry.params.repeat_frequency;
        ioParams->color_shape_repeat_phase = shapeEntry.params.repeat_phase;
        break;
    case ColorPipelineShape::posterize:
        ioParams->color_shape_posterize_steps = shapeEntry.params.posterize_steps;
        ioParams->color_shape_posterize_mix = shapeEntry.params.posterize_mix;
        break;
    case ColorPipelineShape::bias_gain_curve:
        ioParams->color_shape_bias = shapeEntry.params.bias;
        ioParams->color_shape_gain = shapeEntry.params.gain;
        break;
    case ColorPipelineShape::smooth_window:
        ioParams->color_shape_window_center = shapeEntry.params.window_center;
        ioParams->color_shape_window_width = shapeEntry.params.window_width;
        ioParams->color_shape_window_softness = shapeEntry.params.window_softness;
        break;
    case ColorPipelineShape::identity:
    default:
        break;
    }
}

void ClearColorShapeStack(KernelParams* ioParams) {
    if (!ioParams) {
        return;
    }
    ioParams->color_shape_stack_count = 0;
    for (ColorPipelineShapeStackEntry& shapeEntry : ioParams->color_shape_stack) {
        shapeEntry = {};
    }
}

void ClearColorRootBasinPairs(KernelParams* ioParams) {
    if (!ioParams) {
        return;
    }
    ioParams->color_root_basin_pair_count = 0;
    for (ColorPipelineSelection& pairSelection : ioParams->color_root_basin_pairs) {
        pairSelection = {};
    }
}

bool ParseColorRootBasinPairEntry(const json_min::Value& entryValue,
                                  ColorPipelineSelection* outSelection,
                                  std::string* outError) {
    if (!outSelection) {
        if (outError) *outError = "color_root_basin_pairs requires output storage";
        return false;
    }
    if (!entryValue.is_object()) {
        if (outError) *outError = "color_root_basin_pairs entries must be objects";
        return false;
    }

    std::string signalId;
    std::string paletteId;
    std::string gradingId;
    if (!GetRequiredString(entryValue, "signal", &signalId, outError) ||
        !GetRequiredString(entryValue, "palette", &paletteId, outError) ||
        !GetRequiredString(entryValue, "grading", &gradingId, outError)) {
        return false;
    }

    ColorPipelineSelection selection{};
    if (!ParseColorSignal(signalId, &selection.signal)) {
        if (outError) *outError = "Unknown color_root_basin_pairs signal: " + signalId;
        return false;
    }
    if (!ParseColorPalette(paletteId, &selection.palette)) {
        if (outError) *outError = "Unknown color_root_basin_pairs palette: " + paletteId;
        return false;
    }
    if (!ParseColorGradingPreset(gradingId, &selection.grading)) {
        if (outError) *outError = "Unknown color_root_basin_pairs grading: " + gradingId;
        return false;
    }

    const char* sourceFunctionId = nullptr;
    const char* paletteFunctionId = nullptr;
    if (!TryBuildColorPipelineScheduleBridgeIds(selection, &sourceFunctionId, &paletteFunctionId) ||
        !sourceFunctionId ||
        !paletteFunctionId ||
        std::strcmp(sourceFunctionId, "root_index") != 0 ||
        (std::strcmp(paletteFunctionId, "root_classic_palette") != 0 &&
         std::strcmp(paletteFunctionId, "joy_root_palette") != 0)) {
        if (outError) *outError = "color_root_basin_pairs only supports root_index paired with root_classic_palette or joy_root_palette";
        return false;
    }

    *outSelection = selection;
    return true;
}

bool ParseOptionalColorRootBasinPairs(const json_min::Value& paramsObject,
                                      KernelParams* ioParams,
                                      std::string* outError) {
    if (!ioParams) {
        return false;
    }
    ClearColorRootBasinPairs(ioParams);
    const json_min::Value* pairsValue = paramsObject.get("color_root_basin_pairs");
    if (!pairsValue) {
        return true;
    }
    if (!pairsValue->is_array()) {
        if (outError) *outError = "Field color_root_basin_pairs must be an array";
        return false;
    }

    const auto& pairArray = pairsValue->as_array();
    if (pairArray.size() > static_cast<std::size_t>(kColorPipelineMaxRootBasinPairCount)) {
        if (outError) *outError = "color_root_basin_pairs exceeds the supported maximum row count";
        return false;
    }

    ioParams->color_root_basin_pair_count = static_cast<int>(pairArray.size());
    for (std::size_t index = 0; index < pairArray.size(); ++index) {
        if (!ParseColorRootBasinPairEntry(pairArray[index], &ioParams->color_root_basin_pairs[index], outError)) {
            return false;
        }
    }
    return true;
}

bool ParseColorSourceStackEntry(const json_min::Value& entryValue,
                                ColorPipelineSourceStackEntry* outEntry,
                                std::string* outError) {
    if (!outEntry) {
        if (outError) *outError = "color_source_stack requires output storage";
        return false;
    }
    if (!entryValue.is_object()) {
        if (outError) *outError = "color_source_stack entries must be objects";
        return false;
    }

    std::string signalId;
    if (!GetRequiredString(entryValue, "signal", &signalId, outError)) {
        return false;
    }

    ColorPipelineSourceStackEntry entry;
    if (!ParseColorSignal(signalId, &entry.signal)) {
        if (outError) *outError = std::string("Unknown color_source_stack signal id: ") + signalId;
        return false;
    }
    const char* functionId = AdvancedColorSignalFunctionId(entry.signal);
    if (!functionId || std::strcmp(functionId, "root_index") == 0) {
        if (outError) *outError = "color_source_stack does not allow root_index; root_index stays on color_root_basin_pairs";
        return false;
    }

    double scale = entry.params.scale;
    double bias = entry.params.bias;
    double phaseOffset = entry.params.phase_offset;
    double wrapCycles = entry.params.wrap_cycles;
    double bandCountRaw = static_cast<double>(entry.params.band_count);
    bool hasBandCount = false;
    double softness = entry.params.softness;
    double magnitudeScale = entry.params.magnitude_scale;
    double magnitudeBias = entry.params.magnitude_bias;
    double stripeFrequency = entry.params.stripe_frequency;
    double stripePhase = entry.params.stripe_phase;
    double proximityScale = entry.params.proximity_scale;
    double proximityBias = entry.params.proximity_bias;
    double sdfBoundaryWidthPx = entry.params.sdf_boundary_width_px;
    double blendWeight = entry.params.blend_weight;
    if (!GetOptionalNumber(entryValue, "scale", &scale, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "bias", &bias, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "phase_offset", &phaseOffset, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "wrap_cycles", &wrapCycles, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "band_count", &bandCountRaw, &hasBandCount, outError) ||
        !GetOptionalNumber(entryValue, "softness", &softness, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "magnitude_scale", &magnitudeScale, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "magnitude_bias", &magnitudeBias, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "stripe_frequency", &stripeFrequency, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "stripe_phase", &stripePhase, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "proximity_scale", &proximityScale, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "proximity_bias", &proximityBias, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "sdf_boundary_width_px", &sdfBoundaryWidthPx, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "blend_weight", &blendWeight, nullptr, outError)) {
        return false;
    }
    if (hasBandCount) {
        if (!std::isfinite(bandCountRaw) || std::floor(bandCountRaw) != bandCountRaw) {
            if (outError) *outError = "Invalid integer field: color_source_stack.band_count";
            return false;
        }
        entry.params.band_count = static_cast<int>(bandCountRaw);
    }
    entry.params.scale = static_cast<float>(scale);
    entry.params.bias = static_cast<float>(bias);
    entry.params.phase_offset = static_cast<float>(phaseOffset);
    entry.params.wrap_cycles = static_cast<float>(wrapCycles);
    entry.params.softness = static_cast<float>(softness);
    entry.params.magnitude_scale = static_cast<float>(magnitudeScale);
    entry.params.magnitude_bias = static_cast<float>(magnitudeBias);
    entry.params.stripe_frequency = static_cast<float>(stripeFrequency);
    entry.params.stripe_phase = static_cast<float>(stripePhase);
    entry.params.proximity_scale = static_cast<float>(proximityScale);
    entry.params.proximity_bias = static_cast<float>(proximityBias);
    entry.params.sdf_boundary_width_px = static_cast<float>(sdfBoundaryWidthPx);
    entry.params.blend_weight = static_cast<float>(blendWeight);
    *outEntry = entry;
    return true;
}

bool ParseOptionalColorSourceStack(const json_min::Value& paramsObject,
                                   KernelParams* ioParams,
                                   std::string* outError) {
    if (!ioParams) {
        return false;
    }
    ClearColorSourceStack(ioParams);
    const json_min::Value* stackValue = paramsObject.get("color_source_stack");
    if (!stackValue) {
        return true;
    }
    if (ioParams->color_root_basin_pair_count > 0) {
        if (outError) *outError = "color_source_stack cannot be combined with color_root_basin_pairs";
        return false;
    }
    if (!stackValue->is_array()) {
        if (outError) *outError = "Field color_source_stack must be an array";
        return false;
    }

    const auto& stackArray = stackValue->as_array();
    if (stackArray.size() > static_cast<std::size_t>(kColorPipelineMaxSourceStackCount)) {
        if (outError) *outError = "color_source_stack exceeds the supported maximum row count";
        return false;
    }

    ioParams->color_source_stack_count = static_cast<int>(stackArray.size());
    for (std::size_t index = 0; index < stackArray.size(); ++index) {
        if (!ParseColorSourceStackEntry(stackArray[index], &ioParams->color_source_stack[index], outError)) {
            return false;
        }
    }
    if (!stackArray.empty()) {
        const ColorPipelineSourceStackEntry& finalEntry = ioParams->color_source_stack[stackArray.size() - 1];
        if (finalEntry.signal != ioParams->color_pipeline.signal) {
            if (outError) *outError = "color_source_stack final entry must mirror the saved flat color pipeline signal";
            return false;
        }
        MirrorLegacyColorSourceFromStackEntry(finalEntry, ioParams);
    }
    return true;
}

bool ParseColorShapeStackEntryShape(const json_min::Value& entryValue,
                                    ColorPipelineShapeStackEntry* outEntry,
                                    std::string* outError) {
    std::string shapeId;
    if (!GetRequiredString(entryValue, "shape", &shapeId, outError)) {
        return false;
    }
    if (!ParseColorPipelineShape(shapeId, &outEntry->shape)) {
        if (outError) *outError = std::string("Unknown color_shape_stack shape id: ") + shapeId;
        return false;
    }
    return true;
}

void ApplyColorShapeStackEntryNumbers(ColorPipelineShapeRuntimeParams* outParams,
                                      double offset,
                                      double scale,
                                      double repeatFrequency,
                                      double repeatPhase,
                                      double posterizeMix,
                                      double bias,
                                      double gain,
                                      double windowCenter,
                                      double windowWidth,
                                      double windowSoftness) {
    outParams->offset = static_cast<float>(offset);
    outParams->scale = static_cast<float>(scale);
    outParams->repeat_frequency = static_cast<float>(repeatFrequency);
    outParams->repeat_phase = static_cast<float>(repeatPhase);
    outParams->posterize_mix = static_cast<float>(posterizeMix);
    outParams->bias = static_cast<float>(bias);
    outParams->gain = static_cast<float>(gain);
    outParams->window_center = static_cast<float>(windowCenter);
    outParams->window_width = static_cast<float>(windowWidth);
    outParams->window_softness = static_cast<float>(windowSoftness);
}

bool LoadColorShapeStackEntryNumbers(const json_min::Value& entryValue,
                                     ColorPipelineShapeRuntimeParams* outParams,
                                     std::string* outError) {
    double offset = outParams->offset;
    double scale = outParams->scale;
    double repeatFrequency = outParams->repeat_frequency;
    double repeatPhase = outParams->repeat_phase;
    double posterizeMix = outParams->posterize_mix;
    double bias = outParams->bias;
    double gain = outParams->gain;
    double windowCenter = outParams->window_center;
    double windowWidth = outParams->window_width;
    double windowSoftness = outParams->window_softness;
    if (!GetOptionalNumber(entryValue, "offset", &offset, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "scale", &scale, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "repeat_frequency", &repeatFrequency, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "repeat_phase", &repeatPhase, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "posterize_mix", &posterizeMix, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "bias", &bias, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "gain", &gain, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "window_center", &windowCenter, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "window_width", &windowWidth, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "window_softness", &windowSoftness, nullptr, outError)) {
        return false;
    }
    ApplyColorShapeStackEntryNumbers(
        outParams, offset, scale, repeatFrequency, repeatPhase, posterizeMix, bias, gain, windowCenter, windowWidth, windowSoftness);
    return true;
}

bool LoadColorShapeStackPosterizeSteps(const json_min::Value& entryValue,
                                       ColorPipelineShapeRuntimeParams* outParams,
                                       std::string* outError) {
    double posterizeStepsRaw = static_cast<double>(outParams->posterize_steps);
    bool hasPosterizeSteps = false;
    if (!GetOptionalNumber(entryValue, "posterize_steps", &posterizeStepsRaw, &hasPosterizeSteps, outError)) {
        return false;
    }
    if (!hasPosterizeSteps) {
        return true;
    }
    if (!std::isfinite(posterizeStepsRaw) || std::floor(posterizeStepsRaw) != posterizeStepsRaw) {
        if (outError) *outError = "Invalid integer field: color_shape_stack.posterize_steps";
        return false;
    }
    outParams->posterize_steps = static_cast<int>(posterizeStepsRaw);
    return true;
}

bool ParseColorShapeStackEntry(const json_min::Value& entryValue,
                               ColorPipelineShapeStackEntry* outEntry,
                               std::string* outError) {
    if (!entryValue.is_object()) {
        if (outError) *outError = "color_shape_stack entries must be objects";
        return false;
    }
    *outEntry = {};
    return ParseColorShapeStackEntryShape(entryValue, outEntry, outError) &&
           LoadColorShapeStackEntryNumbers(entryValue, &outEntry->params, outError) &&
           LoadColorShapeStackPosterizeSteps(entryValue, &outEntry->params, outError);
}

bool ParseOptionalColorShapeStack(const json_min::Value& paramsObject, KernelParams* ioParams, std::string* outError) {
    if (!ioParams) {
        return false;
    }
    ClearColorShapeStack(ioParams);
    const json_min::Value* stackValue = paramsObject.get("color_shape_stack");
    if (!stackValue) {
        return true;
    }
    if (!stackValue->is_array()) {
        if (outError) *outError = "Field color_shape_stack must be an array";
        return false;
    }

    const auto& stackArray = stackValue->as_array();
    if (stackArray.size() > static_cast<std::size_t>(kColorPipelineMaxShapeStackCount)) {
        if (outError) *outError = "color_shape_stack exceeds the supported maximum row count";
        return false;
    }

    ioParams->color_shape_stack_count = static_cast<int>(stackArray.size());
    for (std::size_t index = 0; index < stackArray.size(); ++index) {
        if (!ParseColorShapeStackEntry(stackArray[index], &ioParams->color_shape_stack[index], outError)) {
            return false;
        }
    }
    if (!stackArray.empty()) {
        MirrorLegacyColorShapeFromStackEntry(ioParams->color_shape_stack[stackArray.size() - 1], ioParams);
    }
    return true;
}


void ResetLegacyColorGradingMirror(KernelParams* ioParams) {
    if (!ioParams) return;
    ioParams->color_contrast_lift_exposure = 1.0f;
    ioParams->color_contrast_lift_saturation = 1.0f;
    ioParams->color_glow = 0.25f;
    ioParams->color_balance_void = 0.0f;
    ioParams->color_chroma_tension = 0.0f;
    ioParams->color_accent_bias = 0.0f;
}

void MirrorLegacyColorGradingFromStackEntry(const ColorPipelineGradingStackEntry& gradingEntry, KernelParams* ioParams) {
    if (!ioParams) return;
    ResetLegacyColorGradingMirror(ioParams);
    if (gradingEntry.grading == ColorGradingPreset::escape_default) {
        ioParams->color_contrast_lift_exposure = gradingEntry.params.exposure;
        ioParams->color_contrast_lift_saturation = gradingEntry.params.saturation;
    } else if (gradingEntry.grading == ColorGradingPreset::phase_default ||
               gradingEntry.grading == ColorGradingPreset::bands_default) {
        ioParams->color_saturation = gradingEntry.params.saturation;
        ioParams->color_contrast = gradingEntry.params.contrast;
    } else if (gradingEntry.grading == ColorGradingPreset::neutral_default ||
               gradingEntry.grading == ColorGradingPreset::tone_map_default) {
        ioParams->exposure = gradingEntry.params.exposure;
        ioParams->color_saturation = gradingEntry.params.saturation;
        ioParams->color_contrast = gradingEntry.params.contrast;
    } else if (gradingEntry.grading == ColorGradingPreset::glow_default) {
        ioParams->exposure = gradingEntry.params.exposure;
        ioParams->color_saturation = gradingEntry.params.saturation;
        ioParams->color_contrast = gradingEntry.params.contrast;
        ioParams->color_glow = gradingEntry.params.glow;
    } else if (gradingEntry.grading == ColorGradingPreset::balance_void_default) {
        ioParams->color_balance_void = gradingEntry.params.balance_void;
        ioParams->color_chroma_tension = gradingEntry.params.chroma_tension;
        ioParams->color_accent_bias = gradingEntry.params.accent_bias;
    }
}

void ResetLegacyColorPaletteMirror(KernelParams* ioParams) {
    if (!ioParams) return;
    ioParams->color_heatmap_cycle_scale = 1.0f;
    ioParams->color_heatmap_saturation = 1.0f;
    ioParams->color_phase_palette_offset = 0.0f;
    ioParams->color_iteration_band_emphasis = 1.0f;
    ioParams->color_iteration_band_palette_offset = 0.0f;
    ioParams->color_explaino_palette_seed_scale = 1.0f;
    ioParams->color_explaino_palette_seed_phase = 0.0f;
    ioParams->color_explaino_palette_colorfulness = 1.0f;
}

void MirrorLegacyColorPaletteFromStackEntry(const ColorPipelinePaletteStackEntry& paletteEntry, KernelParams* ioParams) {
    if (!ioParams) return;
    ResetLegacyColorPaletteMirror(ioParams);
    ioParams->color_pipeline.palette = paletteEntry.palette;
    if (paletteEntry.palette == ColorPalette::cyclic_escape) {
        ioParams->color_heatmap_cycle_scale = paletteEntry.params.cycle_scale;
        ioParams->color_heatmap_saturation = paletteEntry.params.saturation;
    } else if (paletteEntry.palette == ColorPalette::phase_wheel) {
        ioParams->color_phase_palette_offset = paletteEntry.params.phase_offset;
    } else if (paletteEntry.palette == ColorPalette::banded_escape) {
        ioParams->color_iteration_band_emphasis = paletteEntry.params.band_emphasis;
        ioParams->color_iteration_band_palette_offset = paletteEntry.params.phase_offset;
    } else if (paletteEntry.palette == ColorPalette::explaino_cmap) {
        ioParams->color_explaino_palette_seed_scale = paletteEntry.params.seed_scale;
        ioParams->color_explaino_palette_seed_phase = paletteEntry.params.seed_phase;
        ioParams->color_explaino_palette_colorfulness = paletteEntry.params.colorfulness;
    }
}

void ClearColorPaletteStack(KernelParams* ioParams) {
    if (!ioParams) return;
    ioParams->color_palette_stack_count = 0;
    for (ColorPipelinePaletteStackEntry& paletteEntry : ioParams->color_palette_stack) {
        paletteEntry = {};
    }
}

bool ParseColorPaletteStackEntry(const json_min::Value& entryValue,
                                 ColorPipelinePaletteStackEntry* outEntry,
                                 std::string* outError) {
    if (!entryValue.is_object()) {
        if (outError) *outError = "color_palette_stack entries must be objects";
        return false;
    }
    std::string paletteId;
    if (!GetRequiredString(entryValue, "palette", &paletteId, outError)) return false;
    ColorPipelinePaletteStackEntry entry;
    if (!ParseColorPalette(paletteId, &entry.palette)) {
        if (outError) *outError = std::string("Unknown color_palette_stack palette id: ") + paletteId;
        return false;
    }
    const char* functionId = AdvancedColorPaletteFunctionId(entry.palette);
    if (!functionId || !IsSupportedColorPipelinePaletteStackFunctionId(functionId)) {
        if (outError) *outError = "color_palette_stack only supports shipped non-basin Palette rows";
        return false;
    }
    double cycleScale = entry.params.cycle_scale;
    double saturation = entry.params.saturation;
    double phaseOffset = entry.params.phase_offset;
    double bandEmphasis = entry.params.band_emphasis;
    double seedScale = entry.params.seed_scale;
    double seedPhase = entry.params.seed_phase;
    double colorfulness = entry.params.colorfulness;
    double blendWeight = entry.params.blend_weight;
    if (!GetOptionalNumber(entryValue, "cycle_scale", &cycleScale, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "saturation", &saturation, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "phase_offset", &phaseOffset, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "band_emphasis", &bandEmphasis, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "seed_scale", &seedScale, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "seed_phase", &seedPhase, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "colorfulness", &colorfulness, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "blend_weight", &blendWeight, nullptr, outError)) {
        return false;
    }
    std::string blendModeId = "normal";
    TryGetOptionalString(entryValue, "blend_mode", &blendModeId);
    if (!TryParseColorPaletteBlendModeId(blendModeId, &entry.params.blend_mode)) {
        if (outError) *outError = std::string("Unknown color_palette_stack blend_mode: ") + blendModeId;
        return false;
    }
    entry.params.cycle_scale = static_cast<float>(cycleScale);
    entry.params.saturation = static_cast<float>(saturation);
    entry.params.phase_offset = static_cast<float>(phaseOffset);
    entry.params.band_emphasis = static_cast<float>(bandEmphasis);
    entry.params.seed_scale = static_cast<float>(seedScale);
    entry.params.seed_phase = static_cast<float>(seedPhase);
    entry.params.colorfulness = static_cast<float>(colorfulness);
    entry.params.blend_weight = static_cast<float>(blendWeight);
    *outEntry = entry;
    return true;
}

bool ParseOptionalColorPaletteStack(const json_min::Value& paramsObject, KernelParams* ioParams, std::string* outError) {
    if (!ioParams) return false;
    ClearColorPaletteStack(ioParams);
    const json_min::Value* stackValue = paramsObject.get("color_palette_stack");
    if (!stackValue) return true;
    if (ioParams->color_root_basin_pair_count > 0) {
        if (outError) *outError = "color_palette_stack cannot be combined with color_root_basin_pairs";
        return false;
    }
    if (!stackValue->is_array()) {
        if (outError) *outError = "Field color_palette_stack must be an array";
        return false;
    }
    const auto& stackArray = stackValue->as_array();
    if (stackArray.size() > static_cast<std::size_t>(kColorPipelineMaxPaletteStackCount)) {
        if (outError) *outError = "color_palette_stack exceeds the supported maximum row count";
        return false;
    }
    ioParams->color_palette_stack_count = static_cast<int>(stackArray.size());
    for (std::size_t index = 0; index < stackArray.size(); ++index) {
        if (!ParseColorPaletteStackEntry(stackArray[index], &ioParams->color_palette_stack[index], outError)) {
            return false;
        }
    }
    if (!stackArray.empty()) {
        MirrorLegacyColorPaletteFromStackEntry(ioParams->color_palette_stack[stackArray.size() - 1], ioParams);
    }
    return true;
}

void ClearColorGradingStack(KernelParams* ioParams) {
    if (!ioParams) {
        return;
    }
    ioParams->color_grading_stack_count = 0;
    for (ColorPipelineGradingStackEntry& gradingEntry : ioParams->color_grading_stack) {
        gradingEntry = {};
    }
}

bool ParseColorGradingStackEntry(const json_min::Value& entryValue,
                                 ColorPipelineGradingStackEntry* outEntry,
                                 std::string* outError) {
    if (!entryValue.is_object()) {
        if (outError) *outError = "color_grading_stack entries must be objects";
        return false;
    }
    std::string gradingId;
    if (!GetRequiredString(entryValue, "grading", &gradingId, outError)) {
        return false;
    }
    ColorPipelineGradingStackEntry entry;
    if (!ParseColorGradingPreset(gradingId, &entry.grading)) {
        if (outError) *outError = std::string("Unknown color_grading_stack grading id: ") + gradingId;
        return false;
    }
    const char* functionId = AdvancedColorGradingFunctionId(entry.grading);
    if (!functionId || !IsSupportedColorPipelineGradingFunctionId(functionId)) {
        if (outError) *outError = "color_grading_stack only supports shipped Grading rows";
        return false;
    }
    double exposure = entry.params.exposure;
    double saturation = entry.params.saturation;
    double contrast = entry.params.contrast;
    double glow = entry.params.glow;
    double balanceVoid = entry.params.balance_void;
    double chromaTension = entry.params.chroma_tension;
    double accentBias = entry.params.accent_bias;
    if (!GetOptionalNumber(entryValue, "exposure", &exposure, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "saturation", &saturation, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "contrast", &contrast, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "glow", &glow, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "balance_void", &balanceVoid, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "chroma_tension", &chromaTension, nullptr, outError) ||
        !GetOptionalNumber(entryValue, "accent_bias", &accentBias, nullptr, outError)) {
        return false;
    }
    entry.params.exposure = static_cast<float>(exposure);
    entry.params.saturation = static_cast<float>(saturation);
    entry.params.contrast = static_cast<float>(contrast);
    entry.params.glow = static_cast<float>(glow);
    entry.params.balance_void = static_cast<float>(balanceVoid);
    entry.params.chroma_tension = static_cast<float>(chromaTension);
    entry.params.accent_bias = static_cast<float>(accentBias);
    *outEntry = entry;
    return true;
}

bool ParseOptionalColorGradingStack(const json_min::Value& paramsObject, KernelParams* ioParams, std::string* outError) {
    if (!ioParams) {
        return false;
    }
    ClearColorGradingStack(ioParams);
    const json_min::Value* stackValue = paramsObject.get("color_grading_stack");
    if (!stackValue) {
        return true;
    }
    if (!stackValue->is_array()) {
        if (outError) *outError = "Field color_grading_stack must be an array";
        return false;
    }

    const auto& stackArray = stackValue->as_array();
    if (stackArray.size() > static_cast<std::size_t>(kColorPipelineMaxGradingStackCount)) {
        if (outError) *outError = "color_grading_stack exceeds the supported maximum row count";
        return false;
    }

    ioParams->color_grading_stack_count = static_cast<int>(stackArray.size());
    for (std::size_t index = 0; index < stackArray.size(); ++index) {
        if (!ParseColorGradingStackEntry(stackArray[index], &ioParams->color_grading_stack[index], outError)) {
            return false;
        }
    }
    if (!stackArray.empty()) {
        MirrorLegacyColorGradingFromStackEntry(ioParams->color_grading_stack[stackArray.size() - 1], ioParams);
    }
    return true;
}

bool ParseIntField(const json_min::Value& object, const char* key, int* outValue, std::string* outError) {
    double rawValue = 0.0;
    if (!GetRequiredNumber(object, key, &rawValue, outError)) return false;
    if (!std::isfinite(rawValue) || std::floor(rawValue) != rawValue) {
        if (outError) *outError = std::string("Invalid integer field: ") + key;
        return false;
    }
    if (rawValue < static_cast<double>(INT_MIN) || rawValue > static_cast<double>(INT_MAX)) {
        if (outError) *outError = std::string("Out-of-range integer field: ") + key;
        return false;
    }
    if (outValue) *outValue = static_cast<int>(rawValue);
    return true;
}

bool SidecarAutoDemoPolicyFieldsAreValid(const SidecarAutoDemoControllerPolicy& policy, std::string* outError) {
    if (!std::isfinite(policy.paced_loop_interval_seconds) || policy.paced_loop_interval_seconds <= 0.0) {
        if (outError) *outError = "sidecar_auto_demo_policy.paced_loop_interval_seconds must be > 0";
        return false;
    }
    if (!std::isfinite(policy.stop_demonstrated_fraction) ||
        policy.stop_demonstrated_fraction < 0.0 ||
        policy.stop_demonstrated_fraction > 1.0) {
        if (outError) *outError = "sidecar_auto_demo_policy.stop_demonstrated_fraction must be within [0, 1]";
        return false;
    }
    if (policy.stop_uncertain_count < 0) {
        if (outError) *outError = "sidecar_auto_demo_policy.stop_uncertain_count must be >= 0";
        return false;
    }
    return true;
}

bool ParseOptionalSidecarAutoDemoPolicy(const json_min::Value& root,
    SidecarAutoDemoControllerPolicy* outPolicy,
    bool* outHasPolicy,
    std::string* outError) {
    if (outPolicy) *outPolicy = {};
    if (outHasPolicy) *outHasPolicy = false;

    const json_min::Value* policyObject = root.get("sidecar_auto_demo_policy");
    if (!policyObject) return true;
    if (!policyObject->is_object()) {
        if (outError) *outError = "Missing or invalid object field: sidecar_auto_demo_policy";
        return false;
    }

    SidecarAutoDemoControllerPolicy policy{};
    if (!GetRequiredBool(*policyObject, "enabled", &policy.enabled, outError)) return false;
    if (!GetRequiredBool(*policyObject, "allow_runtime_mutation", &policy.allow_runtime_mutation, outError)) return false;
    if (!GetRequiredBool(*policyObject, "run_paced_loop", &policy.run_paced_loop, outError)) return false;
    if (!GetRequiredNumber(*policyObject, "paced_loop_interval_seconds", &policy.paced_loop_interval_seconds, outError)) return false;
    if (!GetRequiredNumber(*policyObject, "stop_demonstrated_fraction", &policy.stop_demonstrated_fraction, outError)) return false;
    if (!ParseIntField(*policyObject, "stop_uncertain_count", &policy.stop_uncertain_count, outError)) return false;
    if (!SidecarAutoDemoPolicyFieldsAreValid(policy, outError)) return false;

    if (outPolicy) *outPolicy = policy;
    if (outHasPolicy) *outHasPolicy = true;
    return true;
}

bool ParseSidecarMutationStringField(const json_min::Value& object,
    size_t index,
    const char* key,
    std::string* outValue,
    std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_string()) {
        if (outError) {
            *outError = "Invalid sidecar_mutation_history[" + std::to_string(index) + "]." + key;
        }
        return false;
    }
    if (outValue) *outValue = value->as_string();
    return true;
}

bool ParseSidecarMutationNumberField(const json_min::Value& object,
    size_t index,
    const char* key,
    double* outValue,
    std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_number() || !std::isfinite(value->as_number())) {
        if (outError) {
            *outError = "Invalid sidecar_mutation_history[" + std::to_string(index) + "]." + key;
        }
        return false;
    }
    if (outValue) *outValue = value->as_number();
    return true;
}

bool ParseOptionalSidecarMutationHistory(const json_min::Value& root,
    SidecarAutoDemoMutationHistory* outHistory,
    bool* outHasHistory,
    std::string* outError) {
    if (outHistory) outHistory->clear();
    if (outHasHistory) *outHasHistory = false;

    const json_min::Value* historyArray = root.get("sidecar_mutation_history");
    if (!historyArray) return true;
    if (!historyArray->is_array()) {
        if (outError) *outError = "Missing or invalid array field: sidecar_mutation_history";
        return false;
    }

    SidecarAutoDemoMutationHistory parsedHistory;
    parsedHistory.reserve(historyArray->as_array().size());
    for (size_t index = 0; index < historyArray->as_array().size(); ++index) {
        const json_min::Value& entry = historyArray->as_array()[index];
        if (!entry.is_object()) {
            if (outError) {
                *outError = "Invalid sidecar_mutation_history[" + std::to_string(index) + "]";
            }
            return false;
        }

        SidecarAutoDemoMutationRecord record;
        if (!ParseSidecarMutationStringField(entry, index, "label", &record.label, outError)) return false;
        if (!ParseSidecarMutationStringField(entry, index, "path", &record.path, outError)) return false;
        if (!ParseSidecarMutationStringField(entry, index, "type", &record.type, outError)) return false;
        if (!ParseSidecarMutationNumberField(entry, index, "target_value", &record.target_value, outError)) return false;
        if (!ParseSidecarMutationNumberField(entry, index, "utility", &record.utility, outError)) return false;
        parsedHistory.push_back(std::move(record));
    }

    if (outHistory) *outHistory = std::move(parsedHistory);
    if (outHasHistory) *outHasHistory = true;
    return true;
}

bool ParsePositiveUInt64Field(const json_min::Value& object,
    const char* key,
    std::uint64_t* outValue,
    std::string* outError) {
    constexpr double kMaxExactJsonInteger = 9007199254740991.0;

    const json_min::Value* value = object.get(key);
    if (!value || !value->is_number()) {
        if (outError) *outError = std::string("Missing or invalid integer field: ") + key;
        return false;
    }

    const double numeric = value->as_number();
    if (!std::isfinite(numeric) || numeric <= 0.0 || std::floor(numeric) != numeric || numeric > kMaxExactJsonInteger) {
        if (outError) *outError = std::string("Invalid positive integer field: ") + key;
        return false;
    }

    if (outValue) *outValue = static_cast<std::uint64_t>(numeric);
    return true;
}

bool FindColorPipelineLaneCatalogIndex(const std::string& laneId, std::size_t* outIndex) {
    const std::vector<ColorPipelineLaneCatalog>& catalogs = GetColorPipelineLaneCatalogs();
    for (std::size_t index = 0; index < catalogs.size(); ++index) {
        if (catalogs[index].lane_id == laneId) {
            if (outIndex) *outIndex = index;
            return true;
        }
    }
    return false;
}

bool FindColorPipelineParamDescriptorIndex(const FunctionDescriptor& descriptor,
    const std::string& path,
    std::size_t* outIndex) {
    for (std::size_t index = 0; index < descriptor.parameters.size(); ++index) {
        if (descriptor.parameters[index].path == path) {
            if (outIndex) *outIndex = index;
            return true;
        }
    }
    return false;
}

bool IsValidColorPipelineEnumValue(const FunctionParamDescriptor& descriptor, const std::string& value) {
    if (descriptor.type != "enum") {
        return false;
    }
    if (descriptor.options.empty()) {
        return true;
    }
    for (const UISchemaOption& option : descriptor.options) {
        if (option.id == value) {
            return true;
        }
    }
    return false;
}

bool ParseColorPipelineDraftParam(const json_min::Value& paramObject,
    const FunctionParamDescriptor& descriptor,
    ColorPipelineParamState* ioParam,
    std::string* outError) {
    if (!ioParam) {
        if (outError) *outError = "color_pipeline_draft parameter parse requires output storage";
        return false;
    }

    std::string path;
    std::string type;
    if (!GetRequiredString(paramObject, "path", &path, outError)) return false;
    if (!GetRequiredString(paramObject, "type", &type, outError)) return false;
    if (path != descriptor.path) {
        if (outError) *outError = "color_pipeline_draft parameter path mismatch for " + descriptor.path;
        return false;
    }
    if (type != descriptor.type) {
        if (outError) *outError = "color_pipeline_draft parameter type mismatch for " + descriptor.path;
        return false;
    }

    ioParam->path = path;
    ioParam->type = type;
    if (descriptor.type == "bool") {
        if (!GetRequiredBool(paramObject, "bool_value", &ioParam->bool_value, outError)) return false;
        return true;
    }
    if (descriptor.type == "enum") {
        if (!GetRequiredString(paramObject, "enum_value", &ioParam->enum_value, outError)) return false;
        if (!IsValidColorPipelineEnumValue(descriptor, ioParam->enum_value)) {
            if (outError) *outError = "Unknown color_pipeline_draft enum value for " + descriptor.path;
            return false;
        }
        return true;
    }

    double numberValue = 0.0;
    if (!GetRequiredNumber(paramObject, "number_value", &numberValue, outError)) return false;
    if (!std::isfinite(numberValue)) {
        if (outError) *outError = "color_pipeline_draft numeric parameter must be finite for " + descriptor.path;
        return false;
    }
    if (descriptor.type == "int") {
        const double rounded = std::round(numberValue);
        if (std::fabs(numberValue - rounded) > 1.0e-6) {
            if (outError) *outError = "color_pipeline_draft integer parameter must be integral for " + descriptor.path;
            return false;
        }
    }
    ioParam->number_value = numberValue;
    return true;
}

bool ParseColorPipelineDraftRow(const json_min::Value& rowObject,
    const ColorPipelineLaneCatalog& catalog,
    ColorPipelineRowState* outRow,
    std::string* outError) {
    if (!outRow) {
        if (outError) *outError = "color_pipeline_draft row parse requires output storage";
        return false;
    }

    std::uint64_t uiRowId = 0;
    bool enabled = true;
    std::string functionId;
    const json_min::Value* parameterValues = nullptr;
    if (!ParsePositiveUInt64Field(rowObject, "ui_row_id", &uiRowId, outError)) return false;
    if (!GetRequiredBool(rowObject, "enabled", &enabled, outError)) return false;
    if (!GetRequiredString(rowObject, "function_id", &functionId, outError)) return false;
    if (!GetRequiredArray(rowObject, "parameter_values", &parameterValues, outError)) return false;

    const FunctionDescriptor* descriptor = FindColorPipelineFunctionDescriptor(catalog, functionId);
    if (!descriptor) {
        if (outError) *outError = "Unknown advanced color function '" + functionId + "' for lane " + catalog.label;
        return false;
    }

    ColorPipelineRowState row;
    if (!BuildColorPipelineRowFromFunctionId(catalog, functionId.c_str(), uiRowId, &row, outError)) {
        return false;
    }
    row.enabled = enabled;

    if (parameterValues->as_array().size() > descriptor->parameters.size()) {
        if (outError) *outError = "color_pipeline_draft parameter count exceeds descriptor for function '" + functionId + "'";
        return false;
    }

    std::vector<bool> seenParams(descriptor->parameters.size(), false);
    for (const json_min::Value& paramValue : parameterValues->as_array()) {
        if (!paramValue.is_object()) {
            if (outError) *outError = "color_pipeline_draft parameter_values entries must be objects";
            return false;
        }

        std::string path;
        if (!GetRequiredString(paramValue, "path", &path, outError)) return false;
        std::size_t paramIndex = 0;
        if (!FindColorPipelineParamDescriptorIndex(*descriptor, path, &paramIndex)) {
            if (outError) *outError = "Unknown color_pipeline_draft parameter path '" + path + "' for function '" + functionId + "'";
            return false;
        }
        if (seenParams[paramIndex]) {
            if (outError) *outError = "Duplicate color_pipeline_draft parameter path '" + path + "' for function '" + functionId + "'";
            return false;
        }
        if (!ParseColorPipelineDraftParam(paramValue, descriptor->parameters[paramIndex], &row.parameter_values[paramIndex], outError)) {
            return false;
        }
        seenParams[paramIndex] = true;
    }

    for (std::size_t index = 0; index < seenParams.size(); ++index) {
        if (!seenParams[index]) {
            const std::string& missingPath = descriptor->parameters[index].path;
            if (missingPath == "palette.blend_weight" ||
                missingPath == "palette.blend_mode" ||
                missingPath == "signal.blend_weight" ||
                missingPath == "signal.boundary_width_px") {
                continue;
            }
            if (outError) *outError = "Missing color_pipeline_draft parameter path '" + descriptor->parameters[index].path + "' for function '" + functionId + "'";
            return false;
        }
    }

    *outRow = std::move(row);
    return true;
}

bool ParseColorPipelineDraftLane(const json_min::Value& laneObject,
    const ColorPipelineLaneCatalog& catalog,
    ColorPipelineLaneState* outLane,
    std::string* outError) {
    if (!outLane) {
        if (outError) *outError = "color_pipeline_draft lane parse requires output storage";
        return false;
    }

    std::string laneId;
    const json_min::Value* rowsValue = nullptr;
    if (!GetRequiredString(laneObject, "lane_id", &laneId, outError)) return false;
    if (laneId != catalog.lane_id) {
        if (outError) *outError = "color_pipeline_draft lane_id mismatch for lane '" + std::string(catalog.lane_id) + "'";
        return false;
    }

    const json_min::Value* labelValue = laneObject.get("label");
    if (labelValue && !labelValue->is_string()) {
        if (outError) *outError = "Invalid color_pipeline_draft lane label for lane '" + laneId + "'";
        return false;
    }

    if (!GetRequiredArray(laneObject, "rows", &rowsValue, outError)) return false;
    if (rowsValue->as_array().empty()) {
        if (outError) *outError = "color_pipeline_draft lane '" + laneId + "' must contain at least one row";
        return false;
    }

    ColorPipelineLaneState lane;
    lane.lane_id = catalog.lane_id;
    lane.label = catalog.label;
    lane.rows.reserve(rowsValue->as_array().size());
    for (const json_min::Value& rowValue : rowsValue->as_array()) {
        if (!rowValue.is_object()) {
            if (outError) *outError = "color_pipeline_draft rows entries must be objects";
            return false;
        }
        ColorPipelineRowState row;
        if (!ParseColorPipelineDraftRow(rowValue, catalog, &row, outError)) {
            return false;
        }
        lane.rows.push_back(std::move(row));
    }

    *outLane = std::move(lane);
    return true;
}

bool ParseOptionalColorPipelineDraft(const json_min::Value& root,
    FractalType liveFractalType,
    const KernelParams& liveParams,
    ColorPipelineWindowState* outColorPipelineWindow,
    std::string* outError) {
    if (outColorPipelineWindow) {
        *outColorPipelineWindow = {};
    }

    const json_min::Value* draftObject = root.get("color_pipeline_draft");
    if (!draftObject) {
        if (!outColorPipelineWindow) {
            return true;
        }
        ColorPipelineWindowState rebuiltState;
        std::string snapshotError;
        if (!TryBuildColorPipelineLiveSnapshot(liveFractalType, liveParams, &rebuiltState.live_snapshot, &snapshotError)) {
            if (outError) *outError = "Saved color pipeline live snapshot cannot be rebuilt: " + snapshotError;
            return false;
        }
        if (!EnsureColorPipelineWindowInitialized(&rebuiltState)) {
            if (outError) *outError = "Saved color pipeline window could not initialize the shipped programmable lanes";
            return false;
        }
        if (rebuiltState.live_snapshot.valid &&
            rebuiltState.live_snapshot.draft_import_supported &&
            !rebuiltState.live_snapshot.lanes.empty()) {
            rebuiltState.lanes = rebuiltState.live_snapshot.lanes;
            rebuiltState.initialized = true;
            if (!EnsureColorPipelineWindowInitialized(&rebuiltState)) {
                if (outError) *outError = "Saved color pipeline live snapshot could not assign stable row ids";
                return false;
            }
        }
        *outColorPipelineWindow = std::move(rebuiltState);
        return true;
    }
    if (!draftObject->is_object()) {
        if (outError) *outError = "Missing or invalid object field: color_pipeline_draft";
        return false;
    }

    const std::vector<ColorPipelineLaneCatalog>& catalogs = GetColorPipelineLaneCatalogs();
    const json_min::Value* lanesValue = nullptr;
    std::uint64_t nextRowId = 0;
    if (!ParsePositiveUInt64Field(*draftObject, "next_row_id", &nextRowId, outError)) return false;
    if (!GetRequiredArray(*draftObject, "lanes", &lanesValue, outError)) return false;
    bool allowLegacyMissingGradingLane = false;
    if (lanesValue->as_array().size() != catalogs.size()) {
        if (lanesValue->as_array().size() + 1 == catalogs.size()) {
            bool sawGradingLane = false;
            bool onlyKnownLanes = true;
            for (const json_min::Value& laneValue : lanesValue->as_array()) {
                if (!laneValue.is_object()) {
                    onlyKnownLanes = false;
                    break;
                }
                std::string laneId;
                if (!GetRequiredString(laneValue, "lane_id", &laneId, outError)) return false;
                if (laneId == "grading") {
                    sawGradingLane = true;
                }
                std::size_t catalogIndex = 0;
                if (!FindColorPipelineLaneCatalogIndex(laneId, &catalogIndex)) {
                    onlyKnownLanes = false;
                    break;
                }
            }
            allowLegacyMissingGradingLane = onlyKnownLanes && !sawGradingLane &&
                FindColorPipelineLaneCatalog("grading") != nullptr;
        }
    }
    if (lanesValue->as_array().size() != catalogs.size() && !allowLegacyMissingGradingLane) {
        if (outError) *outError = "color_pipeline_draft must contain exactly the shipped programmable lanes";
        return false;
    }

    ColorPipelineWindowState parsedState;
    parsedState.initialized = true;
    parsedState.next_row_id = nextRowId;
    parsedState.lanes.resize(catalogs.size());
    std::vector<bool> seenLanes(catalogs.size(), false);
    std::uint64_t maxRowId = 0;

    for (const json_min::Value& laneValue : lanesValue->as_array()) {
        if (!laneValue.is_object()) {
            if (outError) *outError = "color_pipeline_draft lanes entries must be objects";
            return false;
        }

        std::string laneId;
        if (!GetRequiredString(laneValue, "lane_id", &laneId, outError)) return false;
        std::size_t catalogIndex = 0;
        if (!FindColorPipelineLaneCatalogIndex(laneId, &catalogIndex)) {
            if (outError) *outError = "Unknown color_pipeline_draft lane id: " + laneId;
            return false;
        }
        if (seenLanes[catalogIndex]) {
            if (outError) *outError = "Duplicate color_pipeline_draft lane id: " + laneId;
            return false;
        }

        ColorPipelineLaneState lane;
        if (!ParseColorPipelineDraftLane(laneValue, catalogs[catalogIndex], &lane, outError)) {
            return false;
        }
        for (const ColorPipelineRowState& row : lane.rows) {
            if (row.ui_row_id >= parsedState.next_row_id) {
                if (outError) *outError = "color_pipeline_draft.next_row_id must be greater than every ui_row_id";
                return false;
            }
            if (row.ui_row_id > maxRowId) {
                maxRowId = row.ui_row_id;
            }
        }
        parsedState.lanes[catalogIndex] = std::move(lane);
        seenLanes[catalogIndex] = true;
    }

    for (std::size_t index = 0; index < seenLanes.size(); ++index) {
        if (!seenLanes[index]) {
            if (allowLegacyMissingGradingLane && catalogs[index].lane_id == std::string("grading")) {
                const std::uint64_t insertedRowId = parsedState.next_row_id;
                ColorPipelineLaneState lane;
                if (!BuildColorPipelineLaneWithSingleRow(
                        catalogs[index],
                        catalogs[index].default_function_id,
                        insertedRowId,
                        &lane,
                        outError)) {
                    return false;
                }
                parsedState.lanes[index] = std::move(lane);
                seenLanes[index] = true;
                maxRowId = insertedRowId;
                parsedState.next_row_id = insertedRowId + 1;
                continue;
            }
            if (outError) *outError = "Missing color_pipeline_draft lane id: " + std::string(catalogs[index].lane_id);
            return false;
        }
    }

    if (maxRowId + 1 > parsedState.next_row_id) {
        if (outError) *outError = "color_pipeline_draft.next_row_id must be greater than every ui_row_id";
        return false;
    }

    std::string snapshotError;
    if (!TryBuildColorPipelineLiveSnapshot(liveFractalType, liveParams, &parsedState.live_snapshot, &snapshotError)) {
        if (outError) *outError = "Saved color_pipeline_draft cannot be restored: " + snapshotError;
        return false;
    }
    if (!parsedState.live_snapshot.valid) {
        if (outError) *outError = "Saved color_pipeline_draft is not supported for the saved fractal/color tuple";
        return false;
    }

    if (outColorPipelineWindow) {
        *outColorPipelineWindow = std::move(parsedState);
    }
    return true;
}

bool RequirePositiveIntField(int value, const char* key, std::string* outError) {
    if (value > 0) return true;
    if (outError) *outError = std::string(key) + " must be > 0";
    return false;
}

bool IsFindingStateRelativePathAllowed(const std::filesystem::path& relativePath) {
    if (relativePath.empty()) return false;
    if (relativePath.is_absolute() || relativePath.has_root_name() || relativePath.has_root_directory()) {
        return false;
    }
    const std::filesystem::path normalized = relativePath.lexically_normal();
    if (normalized.empty() || normalized == "." || normalized.filename().empty()) return false;
    for (const auto& part : normalized) {
        if (part == "..") return false;
    }
    return true;
}

} // namespace

bool DiagnosticsStateJsonHasExplicitExplainoRoots(const std::string& text,
    bool* outHasExplicitExplainoRoots,
    std::string* outError) {
    if (outError) outError->clear();
    if (!outHasExplicitExplainoRoots) {
        if (outError) *outError = "DiagnosticsStateJsonHasExplicitExplainoRoots requires non-null output pointer";
        return false;
    }
    *outHasExplicitExplainoRoots = false;

    json_min::ParseResult parseResult = json_min::Parse(text);
    if (!parseResult.error.empty()) {
        if (outError) *outError = "JSON parse failed: " + parseResult.error;
        return false;
    }
    if (!parseResult.value.is_object()) {
        if (outError) *outError = "State JSON root must be an object";
        return false;
    }

    const json_min::Value* paramsObject = nullptr;
    if (!GetRequiredObject(parseResult.value, "params", &paramsObject, outError)) return false;
    const json_min::Value* rootsValue = paramsObject->get("explaino_roots");
    if (!rootsValue) return true;
    if (!rootsValue->is_array()) {
        if (outError) *outError = "explaino_roots must be an array";
        return false;
    }
    *outHasExplicitExplainoRoots = true;
    return true;
}

bool DiagnosticsStateFileHasExplicitExplainoRoots(const std::string& path,
    bool* outHasExplicitExplainoRoots,
    std::string* outError) {
    std::string text;
    if (!ReadTextFile(std::filesystem::path(path), &text, outError)) return false;
    return DiagnosticsStateJsonHasExplicitExplainoRoots(text, outHasExplicitExplainoRoots, outError);
}

bool LoadDiagnosticsStateJson(const std::string& text,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    std::string* outError) {
    return LoadDiagnosticsStateJson(text, ioView, ioParams, ioRender, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, outError);
}

bool LoadDiagnosticsStateJson(const std::string& text,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    ColorPipelineWindowState* outColorPipelineWindow,
    std::string* outError) {
    return LoadDiagnosticsStateJson(text, ioView, ioParams, ioRender, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, outColorPipelineWindow, outError);
}

bool LoadDiagnosticsStateJson(const std::string& text,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    ColorPipelineWindowState* outColorPipelineWindow,
    LensSettings* outLens,
    std::string* outError) {
    return LoadDiagnosticsStateJson(text, ioView, ioParams, ioRender, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, outLens, outColorPipelineWindow, outError);
}

bool LoadDiagnosticsStateJson(const std::string& text,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    std::string* outError) {
    return LoadDiagnosticsStateJson(
        text,
        ioView,
        ioParams,
        ioRender,
        outOrientation,
        outHasOrientation,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        outError);
}

bool LoadDiagnosticsStateJson(const std::string& text,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    SidecarAutoDemoControllerPolicy* outControllerPolicy,
    bool* outHasControllerPolicy,
    std::string* outError) {
    return LoadDiagnosticsStateJson(
        text,
        ioView,
        ioParams,
        ioRender,
        outOrientation,
        outHasOrientation,
        outControllerPolicy,
        outHasControllerPolicy,
        nullptr,
        nullptr,
        outError);
}

bool LoadDiagnosticsStateJson(const std::string& text,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    SidecarAutoDemoControllerPolicy* outControllerPolicy,
    bool* outHasControllerPolicy,
    SidecarAutoDemoMutationHistory* outMutationHistory,
    bool* outHasMutationHistory,
    std::string* outError) {
    return LoadDiagnosticsStateJson(
        text,
        ioView,
        ioParams,
        ioRender,
        outOrientation,
        outHasOrientation,
        outControllerPolicy,
        outHasControllerPolicy,
        outMutationHistory,
        outHasMutationHistory,
        nullptr,
        outError);
}

bool LoadDiagnosticsStateJson(const std::string& text,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    SidecarAutoDemoControllerPolicy* outControllerPolicy,
    bool* outHasControllerPolicy,
    SidecarAutoDemoMutationHistory* outMutationHistory,
    bool* outHasMutationHistory,
    ColorPipelineWindowState* outColorPipelineWindow,
    std::string* outError) {
    return LoadDiagnosticsStateJson(
        text,
        ioView,
        ioParams,
        ioRender,
        outOrientation,
        outHasOrientation,
        outControllerPolicy,
        outHasControllerPolicy,
        outMutationHistory,
        outHasMutationHistory,
        nullptr,
        outColorPipelineWindow,
        outError);
}

bool LoadDiagnosticsStateJson(const std::string& text,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    SidecarAutoDemoControllerPolicy* outControllerPolicy,
    bool* outHasControllerPolicy,
    SidecarAutoDemoMutationHistory* outMutationHistory,
    bool* outHasMutationHistory,
    LensSettings* outLens,
    ColorPipelineWindowState* outColorPipelineWindow,
    std::string* outError) {
    if (outError) outError->clear();
    if (outOrientation) *outOrientation = {};
    if (outHasOrientation) *outHasOrientation = false;
    if (outControllerPolicy) *outControllerPolicy = {};
    if (outHasControllerPolicy) *outHasControllerPolicy = false;
    if (outMutationHistory) outMutationHistory->clear();
    if (outHasMutationHistory) *outHasMutationHistory = false;
    if (outColorPipelineWindow) *outColorPipelineWindow = {};
    if (outLens) *outLens = {};
    if (!ioView || !ioParams || !ioRender) {
        if (outError) *outError = "LoadDiagnosticsStateJson requires non-null output pointers";
        return false;
    }

    json_min::ParseResult parseResult = json_min::Parse(text);
    if (!parseResult.error.empty()) {
        if (outError) *outError = "JSON parse failed: " + parseResult.error;
        return false;
    }
    if (!parseResult.value.is_object()) {
        if (outError) *outError = "State JSON root must be an object";
        return false;
    }

    const json_min::Value& root = parseResult.value;
    if (!ParseOptionalSidecarOrientation(root, outOrientation, outHasOrientation, outError)) return false;
    if (!ParseOptionalSidecarAutoDemoPolicy(root, outControllerPolicy, outHasControllerPolicy, outError)) return false;
    if (!ParseOptionalSidecarMutationHistory(root, outMutationHistory, outHasMutationHistory, outError)) return false;
    LensSettings nextLens{};
    if (!ParseOptionalLensState(root, outLens ? &nextLens : nullptr, outError)) return false;
    int stateVersion = 0;
    if (!ParseIntField(root, "state_version", &stateVersion, outError)) return false;
    if (stateVersion != 1 && stateVersion != 2 && stateVersion != 3) {
        if (outError) *outError = "Unsupported state_version: " + std::to_string(stateVersion);
        return false;
    }

    std::string fractalTypeId;
    if (!GetRequiredString(root, "fractal_type", &fractalTypeId, outError)) return false;

    ViewState nextView = *ioView;
    KernelParams nextParams = *ioParams;
    RenderSettings nextRender = *ioRender;

    if (!ParseFractalType(fractalTypeId, &nextView.fractal_type)) {
        if (outError) *outError = "Unknown fractal_type: " + fractalTypeId;
        return false;
    }
    const json_min::Value* viewObject = nullptr;
    const json_min::Value* paramsObject = nullptr;
    const json_min::Value* renderObject = nullptr;
    if (!GetRequiredObject(root, "view", &viewObject, outError)) return false;
    if (!GetRequiredObject(root, "params", &paramsObject, outError)) return false;
    if (!GetRequiredObject(root, "render", &renderObject, outError)) return false;

    double centerX = 0.0;
    double centerY = 0.0;
    double zoom = 0.0;
    double rotationDegrees = 0.0;
    double centerHpX = 0.0;
    double centerHpY = 0.0;
    double log2Zoom = 0.0;
    double explainoPhase = 0.0;
    double explainoSeedDrift = 0.0;
    bool explainoSeedTween = true;
    bool autoIncrementSeed = nextView.auto_increment_seed;
    double explainoSeedRate = nextView.explaino_seed_rate;
    double explainoPhaseStrength = nextView.explaino_phase_strength;
    if (!GetRequiredNumber(*viewObject, "center_x", &centerX, outError)) return false;
    if (!GetRequiredNumber(*viewObject, "center_y", &centerY, outError)) return false;
    if (!GetRequiredNumber(*viewObject, "zoom", &zoom, outError)) return false;
    if (!GetRequiredNumber(*viewObject, "rotation_degrees", &rotationDegrees, outError)) return false;
    if (!GetRequiredNumber(*viewObject, "center_hp_x", &centerHpX, outError)) return false;
    if (!GetRequiredNumber(*viewObject, "center_hp_y", &centerHpY, outError)) return false;
    if (!GetRequiredNumber(*viewObject, "log2_zoom", &log2Zoom, outError)) return false;
    if (!GetRequiredNumber(*viewObject, "explaino_phase", &explainoPhase, outError)) return false;
    if (!GetRequiredNumber(*viewObject, "explaino_seed_drift", &explainoSeedDrift, outError)) return false;
    if (!GetRequiredBool(*viewObject, "explaino_seed_tween", &explainoSeedTween, outError)) return false;
    bool autoMaxIter = DefaultAutoMaxIterForFractal(nextView.fractal_type);
    if (!GetOptionalBool(*viewObject, "auto_max_iter", &autoMaxIter, nullptr, outError)) return false;
    if (!GetOptionalBool(*viewObject, "auto_increment_seed", &autoIncrementSeed, nullptr, outError)) return false;
    if (!GetOptionalNumber(*viewObject, "explaino_seed_rate", &explainoSeedRate, nullptr, outError)) return false;
    if (!GetOptionalNumber(*viewObject, "explaino_phase_strength", &explainoPhaseStrength, nullptr, outError)) return false;

    nextView.center.x = static_cast<float>(centerX);
    nextView.center.y = static_cast<float>(centerY);
    nextView.zoom = static_cast<float>(zoom);
    nextView.rotation_degrees = static_cast<float>(rotationDegrees);
    nextView.center_hp_x = centerHpX;
    nextView.center_hp_y = centerHpY;
    nextView.log2_zoom = log2Zoom;
    nextView.explaino_phase = static_cast<float>(explainoPhase);
    nextView.explaino_seed_drift = static_cast<float>(explainoSeedDrift);
    nextView.explaino_seed_tween = explainoSeedTween;
    nextView.auto_max_iter = autoMaxIter;
    nextView.auto_increment_seed = autoIncrementSeed;
    nextView.explaino_seed_rate = static_cast<float>(explainoSeedRate);
    nextView.explaino_phase_strength = static_cast<float>(explainoPhaseStrength);

    int maxIter = 0;
    int rawPolyKind = 0;
    double epsilon = 0.0;
    double exposure = 0.0;
    std::string coloringModeId;
    bool hasLegacyColoringModeId = false;
    std::string colorSignalId;
    std::string colorShapeId;
    std::string colorPaletteId;
    std::string colorGradingId;
    double novaAlpha = 0.0;
    double phoenixPReal = 0.0;
    double phoenixPImag = 0.0;
    double juliaCReal = static_cast<double>(nextParams.julia_c_real);
    double juliaCImag = static_cast<double>(nextParams.julia_c_imag);
    ExplainoJuliaConstantMode explainoJuliaConstantMode = nextParams.explaino_julia_constant_mode;
    double explainoJuliaCReal = static_cast<double>(nextParams.explaino_julia_c_real);
    double explainoJuliaCImag = static_cast<double>(nextParams.explaino_julia_c_imag);
    int multibrotPower = 0;
    double multibrotPowerFloat = static_cast<double>(nextParams.multibrot_power_float);
    double multibrotPowerImag = static_cast<double>(nextParams.multibrot_power_imag);
    double collatzTransitionStrength = static_cast<double>(nextParams.collatz_transition_strength);
    double spiderFeedback = static_cast<double>(nextParams.spider_feedback);
    double burningShipFoldMix = static_cast<double>(nextParams.burning_ship_fold_mix);
    double celticAbsMix = static_cast<double>(nextParams.celtic_abs_mix);
    double perpendicularFoldMix = static_cast<double>(nextParams.perpendicular_fold_mix);
    int explainoRationalEscapeDenominatorPower = nextParams.explaino_rational_escape_denominator_power;
    double lambdaReal = static_cast<double>(nextParams.lambda_real);
    double lambdaImag = static_cast<double>(nextParams.lambda_imag);
    double magnetSeedReal = static_cast<double>(nextParams.magnet_seed_real);
    double magnetSeedImag = static_cast<double>(nextParams.magnet_seed_imag);
    double magnetRelaxation = static_cast<double>(nextParams.magnet_relaxation);
    double magnetBailout = static_cast<double>(nextParams.magnet_bailout);
    std::string counterfactualPairRootFamilyId;
    std::string counterfactualPairFrameId;
    std::string projectionAndFlowRootFamilyId;
    const bool hasCounterfactualPairRootFamilyId =
        TryGetOptionalString(*paramsObject, "counterfactual_pair_root_family", &counterfactualPairRootFamilyId);
    const bool hasCounterfactualPairFrameId =
        TryGetOptionalString(*paramsObject, "counterfactual_pair_frame", &counterfactualPairFrameId);
    const bool hasProjectionAndFlowRootFamilyId =
        TryGetOptionalString(*paramsObject, "projection_and_flow_root_family", &projectionAndFlowRootFamilyId);
    double counterfactualPairOffsetX = static_cast<double>(nextParams.counterfactual_pair_offset_x);
    double counterfactualPairOffsetY = static_cast<double>(nextParams.counterfactual_pair_offset_y);
    double counterfactualPairReconvergenceRatio =
        static_cast<double>(nextParams.counterfactual_pair_reconvergence_ratio);
    double projectionAndFlowTargetRadius = static_cast<double>(nextParams.projection_and_flow_target_radius);
    double projectionAndFlowPressureThreshold = static_cast<double>(nextParams.projection_and_flow_pressure_threshold);
    double explainoSeed = 0.0;
    double explainoSeedB = nextParams.explaino_seed_b;
    double explainoMix = nextParams.explaino_mix;
    double explainoWarpStrength = 0.0;
    double explainoRootSpread = nextParams.explaino_root_spread;
    ExplainoRootAuthority explainoRootAuthority = nextParams.explaino_root_authority;
    double explainoDamping = nextParams.explaino_damping;
    int explainoRootCount = 0;
    double joyCoupling = nextParams.joy_coupling;
    double foldCoupling = nextParams.fold_coupling;
    double bellCoupling = nextParams.bell_coupling;
    double balanceVoid = nextParams.balance_void;
    double symmetryTension = nextParams.symmetry_tension;
    double fieldCurvature = nextParams.field_curvature;
    double rippleAmplitude = nextParams.ripple_amplitude;
    double spliceOffset = nextParams.splice_offset;
    double vortexStrength = nextParams.vortex_strength;
    double tensionStrength = nextParams.tension_strength;
    const json_min::Value* polyCoeffsArray = nullptr;
    const json_min::Value* polyCoeffsBArray = nullptr;
    if (!ParseIntField(*paramsObject, "max_iter", &maxIter, outError)) return false;
    if (!GetRequiredNumber(*paramsObject, "epsilon", &epsilon, outError)) return false;
    if (!GetRequiredNumber(*paramsObject, "exposure", &exposure, outError)) return false;
    if (!ParseIntField(*paramsObject, "poly_kind", &rawPolyKind, outError)) return false;
    if (stateVersion >= 2) {
        if (!GetRequiredNumber(*paramsObject, "nova_alpha", &novaAlpha, outError)) return false;
        if (!GetRequiredNumber(*paramsObject, "phoenix_p_real", &phoenixPReal, outError)) return false;
        if (!GetRequiredNumber(*paramsObject, "phoenix_p_imag", &phoenixPImag, outError)) return false;
        if (!GetOptionalNumber(*paramsObject, "julia_c_real", &juliaCReal, nullptr, outError)) return false;
        if (!GetOptionalNumber(*paramsObject, "julia_c_imag", &juliaCImag, nullptr, outError)) return false;
        if (const json_min::Value* explainoJuliaModeValue = paramsObject->get("explaino_julia_constant_mode")) {
            if (!explainoJuliaModeValue->is_string()) {
                if (outError) *outError = "Invalid explaino_julia_constant_mode field";
                return false;
            }
            const std::string modeId = explainoJuliaModeValue->as_string();
            if (!TryParseExplainoJuliaConstantModeId(modeId, &explainoJuliaConstantMode)) {
                if (outError) *outError = "Unknown explaino_julia_constant_mode: " + modeId;
                return false;
            }
        }
        if (!GetOptionalNumber(*paramsObject, "explaino_julia_c_real", &explainoJuliaCReal, nullptr, outError)) return false;
        if (!GetOptionalNumber(*paramsObject, "explaino_julia_c_imag", &explainoJuliaCImag, nullptr, outError)) return false;
        if (!ParseIntField(*paramsObject, "multibrot_power", &multibrotPower, outError)) return false;
        if (!GetOptionalNumber(*paramsObject, "multibrot_power_float", &multibrotPowerFloat, nullptr, outError)) return false;
        if (!GetOptionalNumber(*paramsObject, "multibrot_power_imag", &multibrotPowerImag, nullptr, outError)) return false;
        if (!GetOptionalNumber(*paramsObject, "collatz_transition_strength", &collatzTransitionStrength, nullptr, outError)) return false;
        if (!GetOptionalNumber(*paramsObject, "spider_feedback", &spiderFeedback, nullptr, outError)) return false;
        if (!GetOptionalNumber(*paramsObject, "burning_ship_fold_mix", &burningShipFoldMix, nullptr, outError)) return false;
        if (!GetOptionalNumber(*paramsObject, "celtic_abs_mix", &celticAbsMix, nullptr, outError)) return false;
        if (!GetOptionalNumber(*paramsObject, "perpendicular_fold_mix", &perpendicularFoldMix, nullptr, outError)) return false;
        if (const json_min::Value* rationalDenominatorPowerValue = paramsObject->get("explaino_rational_escape_denominator_power")) {
            if (!rationalDenominatorPowerValue->is_number() ||
                !std::isfinite(rationalDenominatorPowerValue->as_number()) ||
                std::floor(rationalDenominatorPowerValue->as_number()) != rationalDenominatorPowerValue->as_number() ||
                rationalDenominatorPowerValue->as_number() < static_cast<double>(INT_MIN) ||
                rationalDenominatorPowerValue->as_number() > static_cast<double>(INT_MAX)) {
                if (outError) *outError = "Invalid explaino_rational_escape_denominator_power field";
                return false;
            }
            explainoRationalEscapeDenominatorPower = static_cast<int>(rationalDenominatorPowerValue->as_number());
        }
        if (!GetOptionalNumber(*paramsObject, "lambda_real", &lambdaReal, nullptr, outError)) return false;
        if (!GetOptionalNumber(*paramsObject, "lambda_imag", &lambdaImag, nullptr, outError)) return false;
        if (!GetOptionalNumber(*paramsObject, "magnet_seed_real", &magnetSeedReal, nullptr, outError)) return false;
        if (!GetOptionalNumber(*paramsObject, "magnet_seed_imag", &magnetSeedImag, nullptr, outError)) return false;
        if (!GetOptionalNumber(*paramsObject, "magnet_relaxation", &magnetRelaxation, nullptr, outError)) return false;
        if (!GetOptionalNumber(*paramsObject, "magnet_bailout", &magnetBailout, nullptr, outError)) return false;
        if (const json_min::Value* rootAuthorityValue = paramsObject->get("explaino_root_authority")) {
            if (!rootAuthorityValue->is_string()) {
                if (outError) *outError = "Invalid explaino_root_authority field";
                return false;
            }
            const std::string rootAuthorityId = rootAuthorityValue->as_string();
            if (!TryParseExplainoRootAuthorityId(rootAuthorityId, &explainoRootAuthority)) {
                if (outError) *outError = "Unknown explaino_root_authority: " + rootAuthorityId;
                return false;
            }
        }
    }
    if (!GetOptionalNumber(*paramsObject, "counterfactual_pair_offset_x", &counterfactualPairOffsetX, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "counterfactual_pair_offset_y", &counterfactualPairOffsetY, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "counterfactual_pair_reconvergence_ratio", &counterfactualPairReconvergenceRatio, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "projection_and_flow_target_radius", &projectionAndFlowTargetRadius, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "projection_and_flow_pressure_threshold", &projectionAndFlowPressureThreshold, nullptr, outError)) return false;
    const bool hasColorSignalId = TryGetOptionalString(*paramsObject, "color_signal", &colorSignalId);
    const bool hasColorShapeId = TryGetOptionalString(*paramsObject, "color_shape", &colorShapeId);
    const bool hasColorPaletteId = TryGetOptionalString(*paramsObject, "color_palette", &colorPaletteId);
    const bool hasColorGradingId = TryGetOptionalString(*paramsObject, "color_grading", &colorGradingId);
    const bool hasAnyExplicitColorPipeline = hasColorSignalId || hasColorPaletteId || hasColorGradingId;
    hasLegacyColoringModeId = stateVersion >= 2 && TryGetOptionalString(*paramsObject, "coloring_mode", &coloringModeId);
    if (hasAnyExplicitColorPipeline && !(hasColorSignalId && hasColorPaletteId && hasColorGradingId)) {
        if (outError) *outError = "color_signal, color_palette, and color_grading must all be provided together";
        return false;
    }
    if (stateVersion >= 2 && !hasAnyExplicitColorPipeline && !hasLegacyColoringModeId) {
        if (outError) *outError = "Missing or invalid string field: coloring_mode";
        return false;
    }
    if (!GetRequiredNumber(*paramsObject, "explaino_seed", &explainoSeed, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "explaino_seed_b", &explainoSeedB, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "explaino_mix", &explainoMix, nullptr, outError)) return false;
    if (!GetRequiredNumber(*paramsObject, "explaino_warp_strength", &explainoWarpStrength, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "explaino_root_spread", &explainoRootSpread, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "explaino_damping", &explainoDamping, nullptr, outError)) return false;
    if (!ParseIntField(*paramsObject, "explaino_root_count", &explainoRootCount, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "joy_coupling", &joyCoupling, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "fold_coupling", &foldCoupling, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "bell_coupling", &bellCoupling, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "balance_void", &balanceVoid, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "symmetry_tension", &symmetryTension, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "field_curvature", &fieldCurvature, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "ripple_amplitude", &rippleAmplitude, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "splice_offset", &spliceOffset, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "vortex_strength", &vortexStrength, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "tension_strength", &tensionStrength, nullptr, outError)) return false;
    if (!GetRequiredArray(*paramsObject, "poly_coeffs", &polyCoeffsArray, outError)) return false;
    polyCoeffsBArray = paramsObject->get("poly_coeffs_b");

    if (!ParsePolyKind(rawPolyKind, &nextParams.poly_kind)) {
        if (outError) *outError = "Unknown poly_kind: " + std::to_string(rawPolyKind);
        return false;
    }
    if (UsesExplainoCustomPolynomialAuthority(nextView.fractal_type) && nextParams.poly_kind != PolyKind::custom) {
        if (outError) *outError = "poly_kind must be custom for fractal_type " + fractalTypeId;
        return false;
    }
    if (hasCounterfactualPairRootFamilyId) {
        if (!ParseCounterfactualPairRootFamily(counterfactualPairRootFamilyId, &nextParams.counterfactual_pair_root_family)) {
            if (outError) *outError = "Unknown counterfactual_pair_root_family: " + counterfactualPairRootFamilyId;
            return false;
        }
    } else if (nextView.fractal_type == FractalType::counterfactual_pair &&
        !TryResolveCounterfactualPairRootFamilyForPolyKindLocal(nextParams.poly_kind, &nextParams.counterfactual_pair_root_family)) {
        if (outError) *outError = "counterfactual_pair requires a supported root-family polynomial preset";
        return false;
    }
    if (hasCounterfactualPairFrameId &&
        !ParseCounterfactualPairFrame(counterfactualPairFrameId, &nextParams.counterfactual_pair_frame)) {
        if (outError) *outError = "Unknown counterfactual_pair_frame: " + counterfactualPairFrameId;
        return false;
    }
    if (hasProjectionAndFlowRootFamilyId) {
        if (!ParseProjectionAndFlowRootFamily(projectionAndFlowRootFamilyId, &nextParams.projection_and_flow_root_family)) {
            if (outError) *outError = "Unknown projection_and_flow_root_family: " + projectionAndFlowRootFamilyId;
            return false;
        }
    } else if (IsProjectionAndFlowCarrier(nextView.fractal_type) &&
        !TryResolveProjectionAndFlowRootFamilyForPolyKindLocal(nextParams.poly_kind, &nextParams.projection_and_flow_root_family)) {
        if (outError) *outError = "projection_and_flow requires a supported root-family polynomial preset";
        return false;
    }
    nextParams.counterfactual_pair_offset_x = static_cast<float>(counterfactualPairOffsetX);
    nextParams.counterfactual_pair_offset_y = static_cast<float>(counterfactualPairOffsetY);
    nextParams.counterfactual_pair_reconvergence_ratio = static_cast<float>(counterfactualPairReconvergenceRatio);
    nextParams.projection_and_flow_target_radius = static_cast<float>(projectionAndFlowTargetRadius);
    nextParams.projection_and_flow_pressure_threshold = static_cast<float>(projectionAndFlowPressureThreshold);
    if (stateVersion >= 2) {
        nextParams.nova_alpha = static_cast<float>(novaAlpha);
        nextParams.phoenix_p_real = static_cast<float>(phoenixPReal);
        nextParams.phoenix_p_imag = static_cast<float>(phoenixPImag);
        nextParams.julia_c_real = static_cast<float>(juliaCReal);
        nextParams.julia_c_imag = static_cast<float>(juliaCImag);
        nextParams.explaino_julia_constant_mode = explainoJuliaConstantMode;
        nextParams.explaino_julia_c_real = static_cast<float>(explainoJuliaCReal);
        nextParams.explaino_julia_c_imag = static_cast<float>(explainoJuliaCImag);
        nextParams.multibrot_power = multibrotPower;
        nextParams.multibrot_power_float = static_cast<float>(multibrotPowerFloat);
        nextParams.multibrot_power_imag = static_cast<float>(multibrotPowerImag);
        nextParams.collatz_transition_strength = static_cast<float>(collatzTransitionStrength);
        nextParams.spider_feedback = static_cast<float>(spiderFeedback);
        nextParams.burning_ship_fold_mix = static_cast<float>(burningShipFoldMix);
        nextParams.celtic_abs_mix = static_cast<float>(celticAbsMix);
        nextParams.perpendicular_fold_mix = static_cast<float>(perpendicularFoldMix);
        nextParams.explaino_rational_escape_denominator_power = explainoRationalEscapeDenominatorPower;
        nextParams.lambda_real = static_cast<float>(lambdaReal);
        nextParams.lambda_imag = static_cast<float>(lambdaImag);
        nextParams.magnet_seed_real = static_cast<float>(magnetSeedReal);
        nextParams.magnet_seed_imag = static_cast<float>(magnetSeedImag);
        nextParams.magnet_relaxation = static_cast<float>(magnetRelaxation);
        nextParams.magnet_bailout = static_cast<float>(magnetBailout);
    }
    if (stateVersion >= 2 && !hasAnyExplicitColorPipeline) {
        if (!ParseColoringMode(coloringModeId, &nextParams.coloring_mode)) {
            if (outError) *outError = "Unknown coloring_mode: " + coloringModeId;
            return false;
        }
    } else {
        nextParams.coloring_mode = DefaultColoringModeForFractal(nextView.fractal_type);
    }
    if (stateVersion >= 2 && !paramsObject->get("multibrot_power_float")) {
        nextParams.multibrot_power_float = static_cast<float>(nextParams.multibrot_power);
    }
    if (!IsColoringModeAllowedForFractal(nextView.fractal_type, nextParams.coloring_mode)) {
        if (outError) {
            std::string coloringModeName = "unknown";
            switch (nextParams.coloring_mode) {
            case ColoringMode::root_basin: coloringModeName = "root_basin"; break;
            case ColoringMode::iteration_count: coloringModeName = "iteration_count"; break;
            case ColoringMode::smooth_escape: coloringModeName = "smooth_escape"; break;
            case ColoringMode::joy_basins: coloringModeName = "joy_basins"; break;
            case ColoringMode::phase: coloringModeName = "phase"; break;
            case ColoringMode::iteration_bands: coloringModeName = "iteration_bands"; break;
            }
            *outError = "coloring_mode " + coloringModeName + " is not allowed for fractal_type " + fractalTypeId;
        }
        return false;
    }
    if (hasAnyExplicitColorPipeline) {
        ColorPipelineSelection explicitPipeline{};
        if (!ParseColorSignal(colorSignalId, &explicitPipeline.signal)) {
            if (outError) *outError = "Unknown color_signal: " + colorSignalId;
            return false;
        }
        if (!ParseColorPalette(colorPaletteId, &explicitPipeline.palette)) {
            if (outError) *outError = "Unknown color_palette: " + colorPaletteId;
            return false;
        }
        if (!ParseColorGradingPreset(colorGradingId, &explicitPipeline.grading)) {
            if (outError) *outError = "Unknown color_grading: " + colorGradingId;
            return false;
        }
        if (hasColorShapeId && !ParseColorPipelineShape(colorShapeId, &nextParams.color_shape)) {
            if (outError) *outError = "Unknown color_shape: " + colorShapeId;
            return false;
        }
        ColoringMode derivedMode = ColoringMode::root_basin;
        if (!TryMirroredColoringModeForPipeline(explicitPipeline, &derivedMode)) {
            if (outError) *outError = "Unsupported split-color combination in saved state";
            return false;
        }
        if (!IsColoringModeAllowedForFractal(nextView.fractal_type, derivedMode)) {
            if (outError) *outError = "split-color combination is not allowed for fractal_type " + fractalTypeId;
            return false;
        }
        nextParams.color_pipeline = explicitPipeline;
        nextParams.coloring_mode = derivedMode;
    } else {
        nextParams.color_pipeline = stateVersion >= 2
            ? ColorPipelineForLegacyMode(nextParams.coloring_mode)
            : DefaultColorPipelineForFractal(nextView.fractal_type);
    }
    if (!ParseOptionalColorRootBasinPairs(*paramsObject, &nextParams, outError)) return false;
    if (nextParams.color_root_basin_pair_count > 0) {
        const ColorPipelineSelection& finalPair =
            nextParams.color_root_basin_pairs[nextParams.color_root_basin_pair_count - 1];
        if (finalPair.signal != nextParams.color_pipeline.signal ||
            finalPair.palette != nextParams.color_pipeline.palette ||
            finalPair.grading != nextParams.color_pipeline.grading) {
            if (outError) *outError = "color_root_basin_pairs final entry must mirror the saved flat color pipeline tuple";
            return false;
        }
    }
    if (!RequirePositiveIntField(maxIter, "max_iter", outError)) return false;
    nextParams.max_iter = maxIter;
    nextParams.epsilon = static_cast<float>(epsilon);
    nextParams.exposure = static_cast<float>(exposure);
    nextParams.explaino_seed = explainoSeed;
    nextParams.explaino_seed_b = explainoSeedB;
    nextParams.explaino_mix = static_cast<float>(explainoMix);
    nextParams.explaino_warp_strength = static_cast<float>(explainoWarpStrength);
    nextParams.explaino_root_spread = static_cast<float>(explainoRootSpread);
    nextParams.explaino_root_authority = explainoRootAuthority;
    nextParams.explaino_damping = static_cast<float>(explainoDamping);
    nextParams.explaino_root_count = explainoRootCount;
    for (Float2& root : nextParams.explaino_roots) {
        root = {0.0f, 0.0f};
    }
    if (!ParseOptionalExplainoRoots(*paramsObject, nextParams.explaino_root_count, &nextParams, nullptr, outError)) return false;
    if (!UsesExplainoCustomPolynomialAuthority(nextView.fractal_type)) {
        nextParams.explaino_root_authority = ExplainoRootAuthority::generated;
        nextParams.explaino_root_count = 0;
        for (Float2& root : nextParams.explaino_roots) {
            root = {0.0f, 0.0f};
        }
    }
    nextParams.joy_coupling = static_cast<float>(joyCoupling);
    nextParams.fold_coupling = static_cast<float>(foldCoupling);
    nextParams.bell_coupling = static_cast<float>(bellCoupling);
    nextParams.balance_void = static_cast<float>(balanceVoid);
    nextParams.symmetry_tension = static_cast<float>(symmetryTension);
    nextParams.field_curvature = static_cast<float>(fieldCurvature);
    nextParams.ripple_amplitude = static_cast<float>(rippleAmplitude);
    nextParams.splice_offset = static_cast<float>(spliceOffset);
    nextParams.vortex_strength = static_cast<float>(vortexStrength);
    nextParams.tension_strength = static_cast<float>(tensionStrength);
    if (IsExplainoFamily(nextView.fractal_type)) {
        ExplainoSeedNormalize(nextView, nextParams);
    }

    // Color grading (v3+, optional for backward compat)
    double colorSaturation = nextParams.color_saturation;
    double colorContrast = nextParams.color_contrast;
    double colorGlow = nextParams.color_glow;
    double colorBalanceVoid = nextParams.color_balance_void;
    double colorChromaTension = nextParams.color_chroma_tension;
    double colorAccentBias = nextParams.color_accent_bias;
    double colorTintR = nextParams.color_tint_r;
    double colorTintG = nextParams.color_tint_g;
    double colorTintB = nextParams.color_tint_b;
    double colorPhaseSignalOffset = nextParams.color_phase_signal_offset;
    double colorPhaseWrapCycles = nextParams.color_phase_wrap_cycles;
    double colorPhasePaletteOffset = nextParams.color_phase_palette_offset;
    double colorShapeOffset = nextParams.color_shape_offset;
    double colorShapeScale = nextParams.color_shape_scale;
    double colorShapeRepeatFrequency = nextParams.color_shape_repeat_frequency;
    double colorShapeRepeatPhase = nextParams.color_shape_repeat_phase;
    int colorShapePosterizeSteps = nextParams.color_shape_posterize_steps;
    double colorShapePosterizeStepsRaw = static_cast<double>(colorShapePosterizeSteps);
    double colorShapePosterizeMix = nextParams.color_shape_posterize_mix;
    double colorShapeBias = nextParams.color_shape_bias;
    double colorShapeGain = nextParams.color_shape_gain;
    double colorShapeWindowCenter = nextParams.color_shape_window_center;
    double colorShapeWindowWidth = nextParams.color_shape_window_width;
    double colorShapeWindowSoftness = nextParams.color_shape_window_softness;
    int colorIterationBandCount = nextParams.color_iteration_band_count;
    double colorIterationBandCountRaw = static_cast<double>(colorIterationBandCount);
    double colorIterationBandSoftness = nextParams.color_iteration_band_softness;
    double colorIterationBandEmphasis = nextParams.color_iteration_band_emphasis;
    double colorIterationBandPaletteOffset = nextParams.color_iteration_band_palette_offset;
    double colorSmoothEscapeScale = nextParams.color_smooth_escape_scale;
    double colorSmoothEscapeBias = nextParams.color_smooth_escape_bias;
    double colorSmoothEscapeInteriorStrength = nextParams.color_smooth_escape_interior_strength;
    double colorEscapeMagnitudeScale = nextParams.color_escape_magnitude_scale;
    double colorEscapeMagnitudeBias = nextParams.color_escape_magnitude_bias;
    double colorOrbitStripeFrequency = nextParams.color_orbit_stripe_frequency;
    double colorOrbitStripePhase = nextParams.color_orbit_stripe_phase;
    double colorRootProximityScale = nextParams.color_root_proximity_scale;
    double colorRootProximityBias = nextParams.color_root_proximity_bias;
    double colorHeatmapCycleScale = nextParams.color_heatmap_cycle_scale;
    double colorHeatmapSaturation = nextParams.color_heatmap_saturation;
    double colorExplainoPaletteSeedScale = nextParams.color_explaino_palette_seed_scale;
    double colorExplainoPaletteSeedPhase = nextParams.color_explaino_palette_seed_phase;
    double colorExplainoPaletteColorfulness = nextParams.color_explaino_palette_colorfulness;
    double colorContrastLiftExposure = nextParams.color_contrast_lift_exposure;
    double colorContrastLiftSaturation = nextParams.color_contrast_lift_saturation;
    if (!GetOptionalNumber(*paramsObject, "color_saturation", &colorSaturation, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_contrast", &colorContrast, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_glow", &colorGlow, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_balance_void", &colorBalanceVoid, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_chroma_tension", &colorChromaTension, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_accent_bias", &colorAccentBias, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_tint_r", &colorTintR, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_tint_g", &colorTintG, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_tint_b", &colorTintB, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_phase_signal_offset", &colorPhaseSignalOffset, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_phase_wrap_cycles", &colorPhaseWrapCycles, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_phase_palette_offset", &colorPhasePaletteOffset, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_shape_offset", &colorShapeOffset, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_shape_scale", &colorShapeScale, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_shape_repeat_frequency", &colorShapeRepeatFrequency, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_shape_repeat_phase", &colorShapeRepeatPhase, nullptr, outError)) return false;
    bool hasColorShapePosterizeSteps = false;
    if (!GetOptionalNumber(*paramsObject, "color_shape_posterize_steps", &colorShapePosterizeStepsRaw, &hasColorShapePosterizeSteps, outError)) return false;
    if (hasColorShapePosterizeSteps) {
        if (!std::isfinite(colorShapePosterizeStepsRaw) || std::floor(colorShapePosterizeStepsRaw) != colorShapePosterizeStepsRaw) {
            if (outError) *outError = "Invalid integer field: color_shape_posterize_steps";
            return false;
        }
        if (colorShapePosterizeStepsRaw < static_cast<double>(INT_MIN) ||
            colorShapePosterizeStepsRaw > static_cast<double>(INT_MAX)) {
            if (outError) *outError = "Out-of-range integer field: color_shape_posterize_steps";
            return false;
        }
        colorShapePosterizeSteps = static_cast<int>(colorShapePosterizeStepsRaw);
    }
    if (!GetOptionalNumber(*paramsObject, "color_shape_posterize_mix", &colorShapePosterizeMix, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_shape_bias", &colorShapeBias, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_shape_gain", &colorShapeGain, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_shape_window_center", &colorShapeWindowCenter, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_shape_window_width", &colorShapeWindowWidth, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_shape_window_softness", &colorShapeWindowSoftness, nullptr, outError)) return false;
    bool hasColorIterationBandCount = false;
    if (!GetOptionalNumber(*paramsObject, "color_iteration_band_count", &colorIterationBandCountRaw, &hasColorIterationBandCount, outError)) return false;
    if (hasColorIterationBandCount) {
        if (!std::isfinite(colorIterationBandCountRaw) || std::floor(colorIterationBandCountRaw) != colorIterationBandCountRaw) {
            if (outError) *outError = "Invalid integer field: color_iteration_band_count";
            return false;
        }
        if (colorIterationBandCountRaw < static_cast<double>(INT_MIN) ||
            colorIterationBandCountRaw > static_cast<double>(INT_MAX)) {
            if (outError) *outError = "Out-of-range integer field: color_iteration_band_count";
            return false;
        }
        colorIterationBandCount = static_cast<int>(colorIterationBandCountRaw);
    }
    if (!GetOptionalNumber(*paramsObject, "color_iteration_band_softness", &colorIterationBandSoftness, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_iteration_band_emphasis", &colorIterationBandEmphasis, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_iteration_band_palette_offset", &colorIterationBandPaletteOffset, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_smooth_escape_scale", &colorSmoothEscapeScale, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_smooth_escape_bias", &colorSmoothEscapeBias, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_smooth_escape_interior_strength", &colorSmoothEscapeInteriorStrength, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_escape_magnitude_scale", &colorEscapeMagnitudeScale, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_escape_magnitude_bias", &colorEscapeMagnitudeBias, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_orbit_stripe_frequency", &colorOrbitStripeFrequency, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_orbit_stripe_phase", &colorOrbitStripePhase, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_root_proximity_scale", &colorRootProximityScale, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_root_proximity_bias", &colorRootProximityBias, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_heatmap_cycle_scale", &colorHeatmapCycleScale, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_heatmap_saturation", &colorHeatmapSaturation, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_explaino_palette_seed_scale", &colorExplainoPaletteSeedScale, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_explaino_palette_seed_phase", &colorExplainoPaletteSeedPhase, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_explaino_palette_colorfulness", &colorExplainoPaletteColorfulness, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_contrast_lift_exposure", &colorContrastLiftExposure, nullptr, outError)) return false;
    if (!GetOptionalNumber(*paramsObject, "color_contrast_lift_saturation", &colorContrastLiftSaturation, nullptr, outError)) return false;
    nextParams.color_saturation = static_cast<float>(colorSaturation);
    nextParams.color_contrast = static_cast<float>(colorContrast);
    nextParams.color_glow = static_cast<float>(colorGlow);
    nextParams.color_balance_void = static_cast<float>(colorBalanceVoid);
    nextParams.color_chroma_tension = static_cast<float>(colorChromaTension);
    nextParams.color_accent_bias = static_cast<float>(colorAccentBias);
    nextParams.color_tint_r = static_cast<float>(colorTintR);
    nextParams.color_tint_g = static_cast<float>(colorTintG);
    nextParams.color_tint_b = static_cast<float>(colorTintB);
    nextParams.color_phase_signal_offset = static_cast<float>(colorPhaseSignalOffset);
    nextParams.color_phase_wrap_cycles = static_cast<float>(colorPhaseWrapCycles);
    nextParams.color_phase_palette_offset = static_cast<float>(colorPhasePaletteOffset);
    nextParams.color_shape_offset = static_cast<float>(colorShapeOffset);
    nextParams.color_shape_scale = static_cast<float>(colorShapeScale);
    nextParams.color_shape_repeat_frequency = static_cast<float>(colorShapeRepeatFrequency);
    nextParams.color_shape_repeat_phase = static_cast<float>(colorShapeRepeatPhase);
    nextParams.color_shape_posterize_steps = colorShapePosterizeSteps;
    nextParams.color_shape_posterize_mix = static_cast<float>(colorShapePosterizeMix);
    nextParams.color_shape_bias = static_cast<float>(colorShapeBias);
    nextParams.color_shape_gain = static_cast<float>(colorShapeGain);
    nextParams.color_shape_window_center = static_cast<float>(colorShapeWindowCenter);
    nextParams.color_shape_window_width = static_cast<float>(colorShapeWindowWidth);
    nextParams.color_shape_window_softness = static_cast<float>(colorShapeWindowSoftness);
    nextParams.color_iteration_band_count = colorIterationBandCount;
    nextParams.color_iteration_band_softness = static_cast<float>(colorIterationBandSoftness);
    nextParams.color_iteration_band_emphasis = static_cast<float>(colorIterationBandEmphasis);
    nextParams.color_iteration_band_palette_offset = static_cast<float>(colorIterationBandPaletteOffset);
    nextParams.color_smooth_escape_scale = static_cast<float>(colorSmoothEscapeScale);
    nextParams.color_smooth_escape_bias = static_cast<float>(colorSmoothEscapeBias);
    nextParams.color_smooth_escape_interior_strength = static_cast<float>(colorSmoothEscapeInteriorStrength);
    nextParams.color_escape_magnitude_scale = static_cast<float>(colorEscapeMagnitudeScale);
    nextParams.color_escape_magnitude_bias = static_cast<float>(colorEscapeMagnitudeBias);
    nextParams.color_orbit_stripe_frequency = static_cast<float>(colorOrbitStripeFrequency);
    nextParams.color_orbit_stripe_phase = static_cast<float>(colorOrbitStripePhase);
    nextParams.color_root_proximity_scale = static_cast<float>(colorRootProximityScale);
    nextParams.color_root_proximity_bias = static_cast<float>(colorRootProximityBias);
    nextParams.color_heatmap_cycle_scale = static_cast<float>(colorHeatmapCycleScale);
    nextParams.color_heatmap_saturation = static_cast<float>(colorHeatmapSaturation);
    nextParams.color_explaino_palette_seed_scale = static_cast<float>(colorExplainoPaletteSeedScale);
    nextParams.color_explaino_palette_seed_phase = static_cast<float>(colorExplainoPaletteSeedPhase);
    nextParams.color_explaino_palette_colorfulness = static_cast<float>(colorExplainoPaletteColorfulness);
    nextParams.color_contrast_lift_exposure = static_cast<float>(colorContrastLiftExposure);
    nextParams.color_contrast_lift_saturation = static_cast<float>(colorContrastLiftSaturation);
    if (!ParseOptionalColorSourceStack(*paramsObject, &nextParams, outError)) return false;
    if (!ParseOptionalColorShapeStack(*paramsObject, &nextParams, outError)) return false;
    if (!ParseOptionalColorPaletteStack(*paramsObject, &nextParams, outError)) return false;
    if (!ParseOptionalColorGradingStack(*paramsObject, &nextParams, outError)) return false;

    // explaino_cluster_radius (optional for backward compat)
    double explainoClusterRadius = nextParams.explaino_cluster_radius;
    if (!GetOptionalNumber(*paramsObject, "explaino_cluster_radius", &explainoClusterRadius, nullptr, outError)) return false;
    nextParams.explaino_cluster_radius = static_cast<float>(explainoClusterRadius);

    // transcendental_func (optional for backward compat)
    {
        const json_min::Value* tfVal = paramsObject->get("transcendental_func");
        if (tfVal) {
            if (!tfVal->is_string()) {
                if (outError) *outError = "Invalid transcendental_func field";
                return false;
            }
            const std::string tfStr = tfVal->as_string();
            if (tfStr == "f_sin") nextParams.transcendental_func = TranscendentalFunc::f_sin;
            else if (tfStr == "f_exp_minus_1") nextParams.transcendental_func = TranscendentalFunc::f_exp_minus_1;
            else if (tfStr == "f_cosh") nextParams.transcendental_func = TranscendentalFunc::f_cosh;
            else {
                if (outError) *outError = "Unknown transcendental_func: " + tfStr;
                return false;
            }
        }
    }

    // momentum_beta (optional for backward compat)
    {
        const json_min::Value* mbVal = paramsObject->get("momentum_beta");
        if (mbVal) {
            if (!mbVal->is_number()) {
                if (outError) *outError = "Invalid momentum_beta field";
                return false;
            }
            nextParams.momentum_beta = static_cast<float>(mbVal->as_number());
        }
    }

    // mcmullen_preset (optional for backward compat)
    {
        const json_min::Value* mpVal = paramsObject->get("mcmullen_preset");
        if (mpVal) {
            if (!mpVal->is_string()) {
                if (outError) *outError = "Invalid mcmullen_preset field";
                return false;
            }
            std::string mpStr = mpVal->as_string();
            if (!TryParseMcMullenPresetId(mpStr, &nextParams.mcmullen_preset)) {
                if (outError) *outError = "Unknown mcmullen_preset: " + mpStr;
                return false;
            }
        }
    }

    bool hasMcMullenM = false;
    bool hasMcMullenN = false;
    bool hasMcMullenLambda = false;
    double mcmullenM = static_cast<double>(nextParams.mcmullen_m);
    double mcmullenN = static_cast<double>(nextParams.mcmullen_n);
    double mcmullenLambda = static_cast<double>(nextParams.mcmullen_lambda);
    if (!GetOptionalNumber(*paramsObject, "mcmullen_m", &mcmullenM, &hasMcMullenM, outError) ||
        !GetOptionalNumber(*paramsObject, "mcmullen_n", &mcmullenN, &hasMcMullenN, outError) ||
        !GetOptionalNumber(*paramsObject, "mcmullen_lambda", &mcmullenLambda, &hasMcMullenLambda, outError)) {
        return false;
    }
    if (hasMcMullenM) {
        const int rounded = static_cast<int>(std::lround(mcmullenM));
        if (std::fabs(mcmullenM - static_cast<double>(rounded)) > 1.0e-6 || rounded < 2 || rounded > 8) {
            if (outError) *outError = "Invalid mcmullen_m field";
            return false;
        }
        nextParams.mcmullen_m = rounded;
    }
    if (hasMcMullenN) {
        const int rounded = static_cast<int>(std::lround(mcmullenN));
        if (std::fabs(mcmullenN - static_cast<double>(rounded)) > 1.0e-6 || rounded < 1 || rounded > 8) {
            if (outError) *outError = "Invalid mcmullen_n field";
            return false;
        }
        nextParams.mcmullen_n = rounded;
    }
    if (hasMcMullenLambda) {
        if (mcmullenLambda < -1.0 || mcmullenLambda > 1.0) {
            if (outError) *outError = "Invalid mcmullen_lambda field";
            return false;
        }
        nextParams.mcmullen_lambda = static_cast<float>(mcmullenLambda);
    }
    if (nextParams.mcmullen_preset != McMullenPreset::custom) {
        const McMullenPresetConfig config = ResolveMcMullenPresetConfig(nextParams.mcmullen_preset);
        nextParams.mcmullen_m = config.m;
        nextParams.mcmullen_n = config.n;
        nextParams.mcmullen_lambda = config.lambda;
    }

    if (!ParseFixedFloatArray(*polyCoeffsArray, "poly_coeffs", nextParams.poly_coeffs, 5, outError)) return false;
    for (float& coeff : nextParams.poly_coeffs_b) {
        coeff = 0.0f;
    }
    if (polyCoeffsBArray && !ParseFixedFloatArray(*polyCoeffsBArray, "poly_coeffs_b", nextParams.poly_coeffs_b, 5, outError)) return false;
    if (nextView.fractal_type == FractalType::counterfactual_pair) {
        SyncCounterfactualPairRootFamilyPresetLocal(&nextParams);
    }
    if (nextView.fractal_type == FractalType::projection_and_flow) {
        SyncProjectionAndFlowRootFamilyPresetLocal(&nextParams);
    }

    int width = 0;
    int height = 0;
    int blockSize = 0;
    int deviceId = 0;
    double interactionDebounceMs = static_cast<double>(nextRender.interaction_debounce_ms);
    double previewTargetFps = nextRender.preview_target_fps;
    double previewMinScale = nextRender.preview_min_scale;
    std::string sampleTierId;
    const bool hasSampleTierId = TryGetOptionalString(*renderObject, "sample_tier", &sampleTierId);
    if (!ParseIntField(*renderObject, "width", &width, outError)) return false;
    if (!ParseIntField(*renderObject, "height", &height, outError)) return false;
    if (!ParseIntField(*renderObject, "block_size", &blockSize, outError)) return false;
    if (!ParseIntField(*renderObject, "device_id", &deviceId, outError)) return false;
    if (!GetOptionalNumber(*renderObject, "interaction_debounce_ms", &interactionDebounceMs, nullptr, outError)) return false;
    if (!GetOptionalNumber(*renderObject, "preview_target_fps", &previewTargetFps, nullptr, outError)) return false;
    if (!GetOptionalNumber(*renderObject, "preview_min_scale", &previewMinScale, nullptr, outError)) return false;
    if (hasSampleTierId) {
        SampleTier sampleTier{};
        if (!ParseSampleTier(sampleTierId, &sampleTier)) {
            if (outError) *outError = "Unknown sample_tier: " + sampleTierId;
            return false;
        }
        nextRender.sample_tier = sampleTier;
    }
    if (!RequirePositiveIntField(width, "width", outError)) return false;
    if (!RequirePositiveIntField(height, "height", outError)) return false;
    nextRender.resolution.x = width;
    nextRender.resolution.y = height;
    nextRender.block_size = blockSize;
    nextRender.device_id = deviceId;
    nextRender.interaction_debounce_ms = static_cast<int>(interactionDebounceMs);
    nextRender.preview_target_fps = static_cast<float>(previewTargetFps);
    nextRender.preview_min_scale = static_cast<float>(previewMinScale);

    ColorPipelineWindowState nextColorPipelineWindow;
    if (!ParseOptionalColorPipelineDraft(root, nextView.fractal_type, nextParams, &nextColorPipelineWindow, outError)) {
        return false;
    }

    *ioView = nextView;
    *ioParams = nextParams;
    *ioRender = nextRender;
    if (outLens) *outLens = nextLens;
    if (outColorPipelineWindow) *outColorPipelineWindow = std::move(nextColorPipelineWindow);
    return true;
}

bool LoadDiagnosticsStateFile(const std::string& path,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    std::string* outError) {
    return LoadDiagnosticsStateFile(path, ioView, ioParams, ioRender, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, outError);
}

bool LoadDiagnosticsStateFile(const std::string& path,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    ColorPipelineWindowState* outColorPipelineWindow,
    std::string* outError) {
    return LoadDiagnosticsStateFile(path, ioView, ioParams, ioRender, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, outColorPipelineWindow, outError);
}

bool LoadDiagnosticsStateFile(const std::string& path,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    ColorPipelineWindowState* outColorPipelineWindow,
    LensSettings* outLens,
    std::string* outError) {
    return LoadDiagnosticsStateFile(path, ioView, ioParams, ioRender, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, outLens, outColorPipelineWindow, outError);
}

bool LoadDiagnosticsStateFile(const std::string& path,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    std::string* outError) {
    return LoadDiagnosticsStateFile(
        path,
        ioView,
        ioParams,
        ioRender,
        outOrientation,
        outHasOrientation,
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        outError);
}

bool LoadDiagnosticsStateFile(const std::string& path,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    SidecarAutoDemoControllerPolicy* outControllerPolicy,
    bool* outHasControllerPolicy,
    std::string* outError) {
    return LoadDiagnosticsStateFile(
        path,
        ioView,
        ioParams,
        ioRender,
        outOrientation,
        outHasOrientation,
        outControllerPolicy,
        outHasControllerPolicy,
        nullptr,
        nullptr,
        outError);
}

bool LoadDiagnosticsStateFile(const std::string& path,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    SidecarAutoDemoControllerPolicy* outControllerPolicy,
    bool* outHasControllerPolicy,
    SidecarAutoDemoMutationHistory* outMutationHistory,
    bool* outHasMutationHistory,
    std::string* outError) {
    return LoadDiagnosticsStateFile(
        path,
        ioView,
        ioParams,
        ioRender,
        outOrientation,
        outHasOrientation,
        outControllerPolicy,
        outHasControllerPolicy,
        outMutationHistory,
        outHasMutationHistory,
        nullptr,
        outError);
}

bool LoadDiagnosticsStateFile(const std::string& path,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    SidecarAutoDemoControllerPolicy* outControllerPolicy,
    bool* outHasControllerPolicy,
    SidecarAutoDemoMutationHistory* outMutationHistory,
    bool* outHasMutationHistory,
    ColorPipelineWindowState* outColorPipelineWindow,
    std::string* outError) {
    return LoadDiagnosticsStateFile(
        path,
        ioView,
        ioParams,
        ioRender,
        outOrientation,
        outHasOrientation,
        outControllerPolicy,
        outHasControllerPolicy,
        outMutationHistory,
        outHasMutationHistory,
        nullptr,
        outColorPipelineWindow,
        outError);
}

bool LoadDiagnosticsStateFile(const std::string& path,
    ViewState* ioView,
    KernelParams* ioParams,
    RenderSettings* ioRender,
    SidecarOrientationVector* outOrientation,
    bool* outHasOrientation,
    SidecarAutoDemoControllerPolicy* outControllerPolicy,
    bool* outHasControllerPolicy,
    SidecarAutoDemoMutationHistory* outMutationHistory,
    bool* outHasMutationHistory,
    LensSettings* outLens,
    ColorPipelineWindowState* outColorPipelineWindow,
    std::string* outError) {
    if (outError) outError->clear();
    std::string text;
    if (!ReadTextFile(std::filesystem::path(path), &text, outError)) return false;
    return LoadDiagnosticsStateJson(
        text,
        ioView,
        ioParams,
        ioRender,
        outOrientation,
        outHasOrientation,
        outControllerPolicy,
        outHasControllerPolicy,
        outMutationHistory,
        outHasMutationHistory,
        outLens,
        outColorPipelineWindow,
        outError);
}

bool ResolveFindingStateJsonPath(const std::string& selectedPath,
    std::string* outStateJsonPath,
    std::string* outError) {
    if (outError) outError->clear();

    const std::filesystem::path inputPath = std::filesystem::path(selectedPath).lexically_normal();
    if (!std::filesystem::exists(inputPath)) {
        if (outError) *outError = "Selected path does not exist: " + inputPath.string();
        return false;
    }

    if (inputPath.filename() == "state.json") {
        if (outStateJsonPath) *outStateJsonPath = inputPath.string();
        return true;
    }

    if (inputPath.filename() != "finding.json") {
        if (outError) *outError = "Select either state.json or finding.json";
        return false;
    }

    std::string text;
    if (!ReadTextFile(inputPath, &text, outError)) return false;

    json_min::ParseResult parseResult = json_min::Parse(text);
    if (!parseResult.error.empty()) {
        if (outError) *outError = "JSON parse failed: " + parseResult.error;
        return false;
    }
    if (!parseResult.value.is_object()) {
        if (outError) *outError = "Finding metadata must be a JSON object";
        return false;
    }

    std::string stateFileName;
    if (!GetRequiredString(parseResult.value, "state_file", &stateFileName, outError)) return false;
    const std::filesystem::path relativeStatePath(stateFileName);
    if (!IsFindingStateRelativePathAllowed(relativeStatePath)) {
        if (outError) *outError = "state_file must stay within the finding directory";
        return false;
    }
    const std::filesystem::path statePath = (inputPath.parent_path() / relativeStatePath).lexically_normal();
    if (!std::filesystem::exists(statePath)) {
        if (outError) *outError = "Finding metadata points to missing state file: " + statePath.string();
        return false;
    }
    if (outStateJsonPath) *outStateJsonPath = statePath.string();
    return true;
}
