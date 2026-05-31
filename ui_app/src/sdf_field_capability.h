#pragma once

#include <array>
#include <string_view>

enum class SdfFieldProducerKind {
    none,
    lens_sdf,
    lens_field_v2,
    authored_sdf_pack,
    sdf_pack_scene,
};

inline const char* SdfFieldProducerKindId(SdfFieldProducerKind kind) {
    switch (kind) {
    case SdfFieldProducerKind::lens_sdf: return "lens_sdf";
    case SdfFieldProducerKind::lens_field_v2: return "lens_field_v2";
    case SdfFieldProducerKind::authored_sdf_pack: return "authored_sdf_pack";
    case SdfFieldProducerKind::sdf_pack_scene: return "sdf_pack_scene";
    case SdfFieldProducerKind::none:
    default:
        return "none";
    }
}

inline constexpr std::array<const char*, 6> kSdfFieldCapabilitySignalIds = {
    "sdf_signed_distance",
    "sdf_inside_outside",
    "sdf_boundary_band",
    "sdf_normal_angle",
    "sdf_curvature",
    "lens_field_v2_distance",
};

inline bool SdfFieldCapabilitySupportsSignalId(std::string_view signalId) {
    for (const char* supported : kSdfFieldCapabilitySignalIds) {
        if (signalId == supported) {
            return true;
        }
    }
    return false;
}
