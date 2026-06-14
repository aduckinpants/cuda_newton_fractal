#include "explaino_root_field.h"

#include "explaino_seed.h"
#include "explaino_seed_curve.h"
#include "fractal_family_rules.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace {

bool IsFiniteRoot(const Float2& root) {
    return std::isfinite(root.x) && std::isfinite(root.y);
}

float Clamp01(float value) {
    if (!std::isfinite(value)) {
        return 0.0f;
    }
    return (std::max)(0.0f, (std::min)(1.0f, value));
}

double Fract(double value) {
    return value - std::floor(value);
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
    case ExplainoRootFieldLayoutKind::regular_ngon_v1: return "regular_ngon_v1";
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

bool TryParseExplainoRootFieldLayoutKindId(const std::string& id, ExplainoRootFieldLayoutKind* outKind) {
    if (id == "none") {
        if (outKind) *outKind = ExplainoRootFieldLayoutKind::none;
        return true;
    }
    if (id == "legacy_quartic_v1") {
        if (outKind) *outKind = ExplainoRootFieldLayoutKind::legacy_quartic_v1;
        return true;
    }
    if (id == "custom") {
        if (outKind) *outKind = ExplainoRootFieldLayoutKind::custom;
        return true;
    }
    if (id == "family_local") {
        if (outKind) *outKind = ExplainoRootFieldLayoutKind::family_local;
        return true;
    }
    if (id == "regular_ngon_v1") {
        if (outKind) *outKind = ExplainoRootFieldLayoutKind::regular_ngon_v1;
        return true;
    }
    return false;
}

bool TryParseExplainoRootFieldSourceKindId(const std::string& id, ExplainoRootFieldSourceKind* outKind) {
    if (id == "none") {
        if (outKind) *outKind = ExplainoRootFieldSourceKind::none;
        return true;
    }
    if (id == "generated") {
        if (outKind) *outKind = ExplainoRootFieldSourceKind::generated;
        return true;
    }
    if (id == "custom") {
        if (outKind) *outKind = ExplainoRootFieldSourceKind::custom;
        return true;
    }
    return false;
}

struct ScopedExplainoSeedShape { float a = 0.0f; float b = 0.0f; float c = 0.0f; float d = 0.0f; };

static uint32_t RootPatternHashU32(uint32_t x) {
    x ^= x >> 16; x *= 0x7feb352dU; x ^= x >> 15; x *= 0x846ca68bU; x ^= x >> 16; return x;
}

static float RootPatternHash01(uint32_t x) {
    return static_cast<float>(RootPatternHashU32(x) & 0x00ffffffU) / static_cast<float>(0x01000000U);
}

static float RootPatternLerp(float a, float b, float t) { return a + (b - a) * t; }

static ScopedExplainoSeedShape RootPatternShapeForSeed(uint32_t seed, float phase, float spread, float phaseStrength) {
    const float r0 = RootPatternHash01(seed ^ 0x13579bdu);
    const float r1 = RootPatternHash01(seed ^ 0x2468aceu);
    const float r2 = RootPatternHash01(seed ^ 0xdeadbeefu);
    const float r3 = RootPatternHash01(seed ^ 0x9e3779b9u);
    const float baseR = 0.85f + 0.95f * spread;
    const float ps = 0.35f * phaseStrength;
    const float aAngle = (r0 * 6.2831853f) + ps * std::sin(phase * (0.15f + 0.20f * r2));
    const float cAngle = (r1 * 6.2831853f) + ps * std::cos(phase * (0.13f + 0.23f * r3));
    ScopedExplainoSeedShape out{};
    out.a = baseR * std::cos(aAngle);
    out.b = (0.25f + 0.95f * std::fabs(std::sin(aAngle + 0.7f))) * (0.65f + 0.45f * r2);
    out.c = baseR * std::cos(cAngle);
    out.d = (0.25f + 0.95f * std::fabs(std::sin(cAngle - 0.4f))) * (0.65f + 0.45f * r3);
    return out;
}

static ScopedExplainoSeedShape RootPatternShapeForCombinedSeed(double combinedSeed, bool seedTween, float phase, float spread, float phaseStrength) {
    if (!std::isfinite(combinedSeed)) combinedSeed = 0.0;
    const double seedFloor = std::floor(combinedSeed);
    float driftFrac = static_cast<float>(combinedSeed - seedFloor);
    if (!std::isfinite(driftFrac)) driftFrac = 0.0f;
    driftFrac = Clamp01(driftFrac);
    const int seedBase = static_cast<int>(seedFloor);
    const ScopedExplainoSeedShape shape0 = RootPatternShapeForSeed(static_cast<uint32_t>(seedBase), phase, spread, phaseStrength);
    if (!seedTween || driftFrac <= 0.0f) return shape0;
    const ScopedExplainoSeedShape shape1 = RootPatternShapeForSeed(static_cast<uint32_t>(seedBase + 1), phase, spread, phaseStrength);
    const float tweenFrac = static_cast<float>(ExplainoWedgeTween(static_cast<double>(driftFrac)));
    ScopedExplainoSeedShape out{};
    out.a = RootPatternLerp(shape0.a, shape1.a, tweenFrac);
    out.b = RootPatternLerp(shape0.b, shape1.b, tweenFrac);
    out.c = RootPatternLerp(shape0.c, shape1.c, tweenFrac);
    out.d = RootPatternLerp(shape0.d, shape1.d, tweenFrac);
    return out;
}

static void WriteLegacyQuarticRootsForShape(FractalType fractalType, float clusterRadius, const ScopedExplainoSeedShape& shape, Float2 roots[4]) {
    if (fractalType == FractalType::explaino_mult) {
        const float halfClusterRadius = std::fmax(0.0f, std::fmin(2.0f, clusterRadius)) * 0.5f;
        roots[0] = {shape.a + halfClusterRadius, shape.b};
        roots[1] = {shape.a - halfClusterRadius, shape.b};
        roots[2] = {shape.c, shape.d + halfClusterRadius};
        roots[3] = {shape.c, shape.d - halfClusterRadius};
        return;
    }
    roots[0] = {shape.a, shape.b}; roots[1] = {shape.a, -shape.b}; roots[2] = {shape.c, shape.d}; roots[3] = {shape.c, -shape.d};
}

bool ResolveExplainoRootPatternDescriptor(
    const ViewState& view,
    const KernelParams& params,
    ExplainoRootPatternRef patternRef,
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

    const bool useSecondary = patternRef == ExplainoRootPatternRef::secondary;
    const ExplainoGeneratedRootLayout generatedLayout = useSecondary
        ? params.explaino_secondary_root_pattern_layout
        : params.explaino_generated_root_layout;
    const int generatedCount = useSecondary
        ? params.explaino_secondary_root_pattern_count
        : params.explaino_generated_root_count;
    const double seedCombined = useSecondary ? params.explaino_secondary_root_pattern_seed : ExplainoSeedCombined(view, params);
    const float rootSpread = useSecondary ? params.explaino_secondary_root_pattern_spread : params.explaino_root_spread;
    const float phase = useSecondary ? params.explaino_secondary_root_pattern_phase : view.explaino_phase;
    const float phaseStrength = useSecondary ? params.explaino_secondary_root_pattern_phase_strength : view.explaino_phase_strength;

    if (params.explaino_root_authority == ExplainoRootAuthority::generated &&
        UsesExplainoRootLayoutAuthority(view.fractal_type) &&
        useSecondary &&
        generatedLayout == ExplainoGeneratedRootLayout::legacy_quartic_v1) {
        const ScopedExplainoSeedShape shape = RootPatternShapeForCombinedSeed(seedCombined, view.explaino_seed_tween, phase, Clamp01(rootSpread), phaseStrength);
        WriteLegacyQuarticRootsForShape(view.fractal_type, params.explaino_cluster_radius, shape, outDescriptor->base_roots);
        for (int index = 0; index < 4; ++index) {
            if (!IsFiniteRoot(outDescriptor->base_roots[index])) {
                if (outError) *outError = "legacy_quartic_v1 generated a nonfinite scoped root";
                return false;
            }
            outDescriptor->effective_roots[index] = outDescriptor->base_roots[index];
        }
        outDescriptor->active_count = 4;
        outDescriptor->layout_kind = ExplainoRootFieldLayoutKind::legacy_quartic_v1;
        outDescriptor->source_kind = ExplainoRootFieldSourceKind::generated;
        outDescriptor->base_root_hash = HashRoots(outDescriptor->base_roots, outDescriptor->active_count);
        outDescriptor->effective_root_hash = HashRoots(outDescriptor->effective_roots, outDescriptor->active_count);
        return true;
    }

    if (params.explaino_root_authority == ExplainoRootAuthority::generated &&
        UsesExplainoRootLayoutAuthority(view.fractal_type) &&
        generatedLayout == ExplainoGeneratedRootLayout::regular_ngon_v1) {
        const int count = generatedCount;
        if (count < 2 || count > kExplainoRootFieldMaxRoots) {
            if (outError) *outError = "regular_ngon_v1 generated root count must be in [2, 16]";
            return false;
        }
        constexpr double kTau = 6.283185307179586476925286766559;
        constexpr double kGoldenConjugate = 0.6180339887498949;
        const float radius = 0.85f + 0.95f * Clamp01(rootSpread);
        const double seedPhase = Fract(seedCombined * kGoldenConjugate);
        const double rotation = kTau * (static_cast<double>(phase) * static_cast<double>(phaseStrength) + seedPhase);
        for (int index = 0; index < count; ++index) {
            const double angle = rotation + kTau * static_cast<double>(index) / static_cast<double>(count);
            const Float2 root{
                static_cast<float>(static_cast<double>(radius) * std::cos(angle)),
                static_cast<float>(static_cast<double>(radius) * std::sin(angle)),
            };
            if (!IsFiniteRoot(root)) {
                if (outError) *outError = "regular_ngon_v1 generated a nonfinite root";
                return false;
            }
            outDescriptor->base_roots[index] = root;
            outDescriptor->effective_roots[index] = root;
        }
        outDescriptor->active_count = count;
        outDescriptor->layout_kind = ExplainoRootFieldLayoutKind::regular_ngon_v1;
        outDescriptor->source_kind = ExplainoRootFieldSourceKind::generated;
        outDescriptor->base_root_hash = HashRoots(outDescriptor->base_roots, count);
        outDescriptor->effective_root_hash = HashRoots(outDescriptor->effective_roots, count);
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

bool ResolveExplainoRootFieldDescriptor(
    const ViewState& view,
    const KernelParams& params,
    ExplainoRootFieldDescriptor* outDescriptor,
    std::string* outError) {
    return ResolveExplainoRootPatternDescriptor(
        view,
        params,
        params.explaino_root_field_pattern_ref,
        outDescriptor,
        outError);
}
