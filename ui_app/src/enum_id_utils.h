#pragma once

#include "fractal_types.h"

#include <cstddef>
#include <string_view>

namespace enum_id_utils {

template <typename EnumT>
struct EnumIdPair {
    EnumT value;
    const char* id;
};

template <typename EnumT, std::size_t N>
inline const char* LookupEnumId(EnumT value, const EnumIdPair<EnumT>(&pairs)[N]) {
    for (const auto& pair : pairs) {
        if (pair.value == value) return pair.id;
    }
    return nullptr;
}

template <typename EnumT, std::size_t N>
inline bool TryParseEnumId(std::string_view id, const EnumIdPair<EnumT>(&pairs)[N], EnumT* outValue) {
    for (const auto& pair : pairs) {
        if (id == pair.id) {
            if (outValue) *outValue = pair.value;
            return true;
        }
    }
    return false;
}

inline constexpr EnumIdPair<PolyKind> kPolyKindIds[] = {
    {PolyKind::z3_minus_1, "z3_minus_1"},
    {PolyKind::z4_minus_1, "z4_minus_1"},
    {PolyKind::custom, "custom"},
};

inline constexpr EnumIdPair<CounterfactualPairRootFamily> kCounterfactualPairRootFamilyIds[] = {
    {CounterfactualPairRootFamily::cubic_unit_roots, "cubic_unit_roots"},
    {CounterfactualPairRootFamily::quartic_unit_roots, "quartic_unit_roots"},
};

inline constexpr EnumIdPair<CounterfactualPairFrame> kCounterfactualPairFrameIds[] = {
    {CounterfactualPairFrame::world_absolute, "world_absolute"},
    {CounterfactualPairFrame::view_relative, "view_relative"},
};

inline constexpr EnumIdPair<ProjectionAndFlowRootFamily> kProjectionAndFlowRootFamilyIds[] = {
    {ProjectionAndFlowRootFamily::cubic_unit_roots, "cubic_unit_roots"},
    {ProjectionAndFlowRootFamily::quartic_unit_roots, "quartic_unit_roots"},
};

inline constexpr EnumIdPair<TranscendentalFunc> kTranscendentalFuncIds[] = {
    {TranscendentalFunc::f_sin, "f_sin"},
    {TranscendentalFunc::f_exp_minus_1, "f_exp_minus_1"},
    {TranscendentalFunc::f_cosh, "f_cosh"},
};

inline constexpr EnumIdPair<McMullenPreset> kMcMullenPresetIds[] = {
    {McMullenPreset::z3_z3, "z3_z3"},
    {McMullenPreset::z2_z2, "z2_z2"},
    {McMullenPreset::z4_z2, "z4_z2"},
    {McMullenPreset::z3_z2, "z3_z2"},
    {McMullenPreset::custom, "custom"},
};

inline constexpr EnumIdPair<ExplainoJuliaConstantMode> kExplainoJuliaConstantModeIds[] = {
    {ExplainoJuliaConstantMode::seeded, "seeded"},
    {ExplainoJuliaConstantMode::custom, "custom"},
};

inline constexpr EnumIdPair<ExplainoRootAuthority> kExplainoRootAuthorityIds[] = {
    {ExplainoRootAuthority::generated, "generated"},
    {ExplainoRootAuthority::custom, "custom"},
};

inline constexpr EnumIdPair<ColoringMode> kColoringModeIds[] = {
    {ColoringMode::root_basin, "root_basin"},
    {ColoringMode::iteration_count, "iteration_count"},
    {ColoringMode::smooth_escape, "smooth_escape"},
    {ColoringMode::joy_basins, "joy_basins"},
    {ColoringMode::phase, "phase"},
    {ColoringMode::iteration_bands, "iteration_bands"},
};

inline constexpr EnumIdPair<ColorSignal> kColorSignalIds[] = {
    {ColorSignal::root_index, "root_index"},
    {ColorSignal::iteration_count, "iteration_count"},
    {ColorSignal::smooth_escape, "smooth_escape"},
    {ColorSignal::phase_angle, "phase_angle"},
    {ColorSignal::iteration_bands, "iteration_bands"},
    {ColorSignal::escape_magnitude, "escape_magnitude"},
    {ColorSignal::orbit_stripe, "orbit_stripe"},
    {ColorSignal::root_proximity, "root_proximity"},
    {ColorSignal::sdf_signed_distance, "sdf_signed_distance"},
    {ColorSignal::sdf_inside_outside, "sdf_inside_outside"},
    {ColorSignal::sdf_boundary_band, "sdf_boundary_band"},
    {ColorSignal::sdf_normal_angle, "sdf_normal_angle"},
    {ColorSignal::sdf_curvature, "sdf_curvature"},
};

inline constexpr EnumIdPair<ColorPalette> kColorPaletteIds[] = {
    {ColorPalette::root_classic, "root_classic"},
    {ColorPalette::joy, "joy"},
    {ColorPalette::cyclic_escape, "cyclic_escape"},
    {ColorPalette::phase_wheel, "phase_wheel"},
    {ColorPalette::banded_escape, "banded_escape"},
    {ColorPalette::explaino_cmap, "explaino_cmap"},
};

inline constexpr EnumIdPair<ColorGradingPreset> kColorGradingPresetIds[] = {
    {ColorGradingPreset::basin_default, "basin_default"},
    {ColorGradingPreset::escape_default, "escape_default"},
    {ColorGradingPreset::phase_default, "phase_default"},
    {ColorGradingPreset::bands_default, "bands_default"},
    {ColorGradingPreset::neutral_default, "neutral_default"},
    {ColorGradingPreset::tone_map_default, "tone_map_default"},
    {ColorGradingPreset::glow_default, "glow_default"},
    {ColorGradingPreset::balance_void_default, "balance_void_default"},
};

inline constexpr EnumIdPair<ColorPipelineShape> kColorPipelineShapeIds[] = {
    {ColorPipelineShape::identity, "identity"},
    {ColorPipelineShape::offset_scale, "offset_scale"},
    {ColorPipelineShape::repeat, "repeat"},
    {ColorPipelineShape::posterize, "posterize"},
    {ColorPipelineShape::mirror_repeat, "mirror_repeat"},
    {ColorPipelineShape::bias_gain_curve, "bias_gain_curve"},
    {ColorPipelineShape::smooth_window, "smooth_window"},
};

inline constexpr EnumIdPair<FractalType> kFractalTypeIds[] = {
    {FractalType::newton, "newton"},
    {FractalType::nova, "nova"},
    {FractalType::mandelbrot, "mandelbrot"},
    {FractalType::julia, "julia"},
    {FractalType::burning_ship, "burning_ship"},
    {FractalType::multibrot, "multibrot"},
    {FractalType::phoenix, "phoenix"},
    {FractalType::explaino, "explaino"},
    {FractalType::explaino_all, "explaino_all"},
    {FractalType::explaino_y, "explaino_y"},
    {FractalType::explaino_fp, "explaino_fp"},
    {FractalType::explaino_nova, "explaino_nova"},
    {FractalType::explaino_halley, "explaino_halley"},
    {FractalType::explaino_dual, "explaino_dual"},
    {FractalType::explaino_mult, "explaino_mult"},
    {FractalType::explaino_phoenix, "explaino_phoenix"},
    {FractalType::explaino_transcendental, "explaino_transcendental"},
    {FractalType::explaino_inertial, "explaino_inertial"},
    {FractalType::explaino_julia, "explaino_julia"},
    {FractalType::explaino_rational, "explaino_rational"},
    {FractalType::multicorn, "multicorn"},
    {FractalType::halley, "halley"},
    {FractalType::collatz, "collatz"},
    {FractalType::explaino_collatz, "explaino_collatz"},
    {FractalType::explaino_collatz_direct, "explaino_collatz_direct"},
    {FractalType::mcmullen, "mcmullen"},
    {FractalType::lambda_map, "lambda"},
    {FractalType::explaino_lambda, "explaino_lambda"},
    {FractalType::explaino_rational_escape, "explaino_rational_escape"},
    {FractalType::spider, "spider"},
    {FractalType::celtic_mandelbrot, "celtic_mandelbrot"},
    {FractalType::perpendicular_burning_ship, "perpendicular_burning_ship"},
    {FractalType::explaino_joy, "explaino_joy"},
    {FractalType::explaino_fold, "explaino_fold"},
    {FractalType::explaino_bell, "explaino_bell"},
    {FractalType::explaino_ripple, "explaino_ripple"},
    {FractalType::explaino_splice, "explaino_splice"},
    {FractalType::explaino_vortex, "explaino_vortex"},
    {FractalType::explaino_tension, "explaino_tension"},
    {FractalType::explaino_balance_void, "explaino_balance_void"},
    {FractalType::counterfactual_pair, "counterfactual_pair"},
    {FractalType::explaino_counterfactual_pair, "explaino_counterfactual_pair"},
    {FractalType::projection_and_flow, "projection_and_flow"},
    {FractalType::explaino_projection_and_flow, "explaino_projection_and_flow"},
    {FractalType::magnet, "magnet"},
    {FractalType::generic_equation_pack, "generic_equation_pack"},
};

inline constexpr EnumIdPair<CameraBehavior> kCameraBehaviorIds[] = {
    {CameraBehavior::manual, "manual"},
    {CameraBehavior::complexity, "complexity"},
    {CameraBehavior::orbit, "orbit"},
    {CameraBehavior::entropy, "entropy"},
    {CameraBehavior::off, "off"},
};

inline constexpr EnumIdPair<SampleTier> kSampleTierIds[] = {
    {SampleTier::tier_auto, "tier_auto"},
    {SampleTier::fast, "fast"},
    {SampleTier::standard, "standard"},
};


inline constexpr EnumIdPair<LensSdfOverlayMode> kLensSdfOverlayModeIds[] = {
    {LensSdfOverlayMode::off, "off"},
    {LensSdfOverlayMode::boundary, "boundary"},
    {LensSdfOverlayMode::band, "band"},
    {LensSdfOverlayMode::field_debug, "field_debug"},
};
} // namespace enum_id_utils

inline const char* PolyKindId(PolyKind value) {
    return enum_id_utils::LookupEnumId(value, enum_id_utils::kPolyKindIds);
}

inline bool TryParsePolyKindId(std::string_view id, PolyKind* outValue) {
    return enum_id_utils::TryParseEnumId(id, enum_id_utils::kPolyKindIds, outValue);
}

inline const char* CounterfactualPairRootFamilyId(CounterfactualPairRootFamily value) {
    return enum_id_utils::LookupEnumId(value, enum_id_utils::kCounterfactualPairRootFamilyIds);
}

inline bool TryParseCounterfactualPairRootFamilyId(std::string_view id, CounterfactualPairRootFamily* outValue) {
    return enum_id_utils::TryParseEnumId(id, enum_id_utils::kCounterfactualPairRootFamilyIds, outValue);
}

inline const char* CounterfactualPairFrameId(CounterfactualPairFrame value) {
    return enum_id_utils::LookupEnumId(value, enum_id_utils::kCounterfactualPairFrameIds);
}

inline bool TryParseCounterfactualPairFrameId(std::string_view id, CounterfactualPairFrame* outValue) {
    return enum_id_utils::TryParseEnumId(id, enum_id_utils::kCounterfactualPairFrameIds, outValue);
}

inline const char* ProjectionAndFlowRootFamilyId(ProjectionAndFlowRootFamily value) {
    return enum_id_utils::LookupEnumId(value, enum_id_utils::kProjectionAndFlowRootFamilyIds);
}

inline bool TryParseProjectionAndFlowRootFamilyId(std::string_view id, ProjectionAndFlowRootFamily* outValue) {
    return enum_id_utils::TryParseEnumId(id, enum_id_utils::kProjectionAndFlowRootFamilyIds, outValue);
}

inline const char* TranscendentalFuncId(TranscendentalFunc value) {
    return enum_id_utils::LookupEnumId(value, enum_id_utils::kTranscendentalFuncIds);
}

inline bool TryParseTranscendentalFuncId(std::string_view id, TranscendentalFunc* outValue) {
    return enum_id_utils::TryParseEnumId(id, enum_id_utils::kTranscendentalFuncIds, outValue);
}

inline const char* McMullenPresetId(McMullenPreset value) {
    return enum_id_utils::LookupEnumId(value, enum_id_utils::kMcMullenPresetIds);
}

inline bool TryParseMcMullenPresetId(std::string_view id, McMullenPreset* outValue) {
    return enum_id_utils::TryParseEnumId(id, enum_id_utils::kMcMullenPresetIds, outValue);
}

inline const char* ExplainoJuliaConstantModeId(ExplainoJuliaConstantMode value) {
    return enum_id_utils::LookupEnumId(value, enum_id_utils::kExplainoJuliaConstantModeIds);
}

inline bool TryParseExplainoJuliaConstantModeId(std::string_view id, ExplainoJuliaConstantMode* outValue) {
    return enum_id_utils::TryParseEnumId(id, enum_id_utils::kExplainoJuliaConstantModeIds, outValue);
}

inline const char* ExplainoRootAuthorityId(ExplainoRootAuthority value) {
    return enum_id_utils::LookupEnumId(value, enum_id_utils::kExplainoRootAuthorityIds);
}

inline bool TryParseExplainoRootAuthorityId(std::string_view id, ExplainoRootAuthority* outValue) {
    return enum_id_utils::TryParseEnumId(id, enum_id_utils::kExplainoRootAuthorityIds, outValue);
}

inline const char* ColoringModeId(ColoringMode value) {
    return enum_id_utils::LookupEnumId(value, enum_id_utils::kColoringModeIds);
}

inline bool TryParseColoringModeId(std::string_view id, ColoringMode* outValue) {
    return enum_id_utils::TryParseEnumId(id, enum_id_utils::kColoringModeIds, outValue);
}

inline const char* ColorSignalId(ColorSignal value) {
    return enum_id_utils::LookupEnumId(value, enum_id_utils::kColorSignalIds);
}

inline bool TryParseColorSignalId(std::string_view id, ColorSignal* outValue) {
    return enum_id_utils::TryParseEnumId(id, enum_id_utils::kColorSignalIds, outValue);
}

inline const char* ColorPaletteId(ColorPalette value) {
    return enum_id_utils::LookupEnumId(value, enum_id_utils::kColorPaletteIds);
}

inline bool TryParseColorPaletteId(std::string_view id, ColorPalette* outValue) {
    return enum_id_utils::TryParseEnumId(id, enum_id_utils::kColorPaletteIds, outValue);
}

inline const char* ColorGradingPresetId(ColorGradingPreset value) {
    return enum_id_utils::LookupEnumId(value, enum_id_utils::kColorGradingPresetIds);
}

inline bool TryParseColorGradingPresetId(std::string_view id, ColorGradingPreset* outValue) {
    return enum_id_utils::TryParseEnumId(id, enum_id_utils::kColorGradingPresetIds, outValue);
}

inline const char* ColorPipelineShapeId(ColorPipelineShape value) {
    return enum_id_utils::LookupEnumId(value, enum_id_utils::kColorPipelineShapeIds);
}

inline bool TryParseColorPipelineShapeId(std::string_view id, ColorPipelineShape* outValue) {
    return enum_id_utils::TryParseEnumId(id, enum_id_utils::kColorPipelineShapeIds, outValue);
}

inline const char* FractalTypeId(FractalType value) {
    return enum_id_utils::LookupEnumId(value, enum_id_utils::kFractalTypeIds);
}

inline bool TryParseFractalTypeId(std::string_view id, FractalType* outValue) {
    return enum_id_utils::TryParseEnumId(id, enum_id_utils::kFractalTypeIds, outValue);
}

inline const char* CameraBehaviorId(CameraBehavior value) {
    return enum_id_utils::LookupEnumId(value, enum_id_utils::kCameraBehaviorIds);
}

inline bool TryParseCameraBehaviorId(std::string_view id, CameraBehavior* outValue) {
    return enum_id_utils::TryParseEnumId(id, enum_id_utils::kCameraBehaviorIds, outValue);
}

inline const char* SampleTierId(SampleTier value) {
    return enum_id_utils::LookupEnumId(value, enum_id_utils::kSampleTierIds);
}

inline bool TryParseSampleTierId(std::string_view id, SampleTier* outValue) {
    return enum_id_utils::TryParseEnumId(id, enum_id_utils::kSampleTierIds, outValue);
}

inline const char* LensSdfOverlayModeId(LensSdfOverlayMode value) {
    return enum_id_utils::LookupEnumId(value, enum_id_utils::kLensSdfOverlayModeIds);
}

inline bool TryParseLensSdfOverlayModeId(std::string_view id, LensSdfOverlayMode* outValue) {
    return enum_id_utils::TryParseEnumId(id, enum_id_utils::kLensSdfOverlayModeIds, outValue);
}
