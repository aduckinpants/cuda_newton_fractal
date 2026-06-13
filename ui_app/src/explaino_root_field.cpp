#include "explaino_root_field.h"

#include "fractal_family_rules.h"

#include <cmath>
#include <cstring>

namespace {

bool IsFiniteRoot(const Float2& root) {
    return std::isfinite(root.x) && std::isfinite(root.y);
}

std::uint64_t HashRoots(const Float2* roots, int count) {
    std::uint64_t hash = 1469598103934665603ull;
    auto mixByte = [&hash](unsigned char value) {
        hash ^= static_cast<std::uint64_t>(value);
        hash *= 1099511628211ull;
    };
    for (std::size_t byteIndex = 0; byteIndex < sizeof(count); ++byteIndex) {
        mixByte(static_cast<unsigned char>(
            (static_cast<unsigned int>(count) >> (byteIndex * 8u)) & 0xffu));
    }

    for (int index = 0; index < count; ++index) {
        std::uint32_t packed[2]{};
        static_assert(sizeof(packed[0]) == sizeof(roots[index].x), "unexpected float size");
        std::memcpy(&packed[0], &roots[index].x, sizeof(packed[0]));
        std::memcpy(&packed[1], &roots[index].y, sizeof(packed[1]));
        for (std::uint32_t word : packed) {
            mixByte(static_cast<unsigned char>((word >> 0) & 0xffu));
            mixByte(static_cast<unsigned char>((word >> 8) & 0xffu));
            mixByte(static_cast<unsigned char>((word >> 16) & 0xffu));
            mixByte(static_cast<unsigned char>((word >> 24) & 0xffu));
        }
    }
    return hash;
}

ExplainoRootFieldLayoutKind LayoutFor(const ViewState& view, const KernelParams& params, int count) {
    if (params.explaino_root_authority == ExplainoRootAuthority::custom) {
        return ExplainoRootFieldLayoutKind::custom;
    }
    if (count == 4) {
        return ExplainoRootFieldLayoutKind::legacy_quartic_v1;
    }
    if (UsesExplainoRootLayoutAuthority(view.fractal_type) && count > 0) {
        return ExplainoRootFieldLayoutKind::family_local;
    }
    return ExplainoRootFieldLayoutKind::none;
}

ExplainoRootFieldSourceKind SourceFor(const KernelParams& params) {
    return params.explaino_root_authority == ExplainoRootAuthority::custom
        ? ExplainoRootFieldSourceKind::custom
        : ExplainoRootFieldSourceKind::generated;
}

} // namespace

const char* ExplainoRootFieldLayoutKindId(ExplainoRootFieldLayoutKind kind) {
    switch (kind) {
    case ExplainoRootFieldLayoutKind::none: return "none";
    case ExplainoRootFieldLayoutKind::legacy_quartic_v1: return "legacy_quartic_v1";
    case ExplainoRootFieldLayoutKind::custom: return "custom";
    case ExplainoRootFieldLayoutKind::family_local: return "family_local";
    }
    return "unknown";
}

const char* ExplainoRootFieldSourceKindId(ExplainoRootFieldSourceKind kind) {
    switch (kind) {
    case ExplainoRootFieldSourceKind::none: return "none";
    case ExplainoRootFieldSourceKind::generated: return "generated";
    case ExplainoRootFieldSourceKind::custom: return "custom";
    }
    return "unknown";
}

bool ResolveExplainoRootFieldDescriptor(
    const ViewState& view,
    const KernelParams& params,
    ExplainoRootFieldDescriptor* outDescriptor,
    std::string* outError) {
    if (!outDescriptor) {
        if (outError) *outError = "missing ExplainO root-field descriptor output";
        return false;
    }

    *outDescriptor = ExplainoRootFieldDescriptor{};
    if (!UsesExplainoRootLayoutAuthority(view.fractal_type)) {
        return true;
    }

    if (params.explaino_root_count < 0 ||
        params.explaino_root_count > kExplainoRootFieldMaxRoots ||
        params.explaino_root_count > 4) {
        if (outError) *outError = "ExplainO root count is outside the current authority surface";
        return false;
    }

    for (int index = 0; index < params.explaino_root_count; ++index) {
        if (!IsFiniteRoot(params.explaino_roots[index])) {
            if (outError) *outError = "ExplainO root descriptor contains a nonfinite root";
            return false;
        }
        outDescriptor->base_roots[index] = params.explaino_roots[index];
        outDescriptor->effective_roots[index] = params.explaino_roots[index];
    }

    outDescriptor->active_count = params.explaino_root_count;
    if (outDescriptor->active_count == 0) {
        return true;
    }

    outDescriptor->layout_kind = LayoutFor(view, params, outDescriptor->active_count);
    outDescriptor->source_kind = SourceFor(params);
    outDescriptor->base_root_hash = HashRoots(outDescriptor->base_roots, outDescriptor->active_count);
    outDescriptor->effective_root_hash = HashRoots(outDescriptor->effective_roots, outDescriptor->active_count);
    return true;
}
