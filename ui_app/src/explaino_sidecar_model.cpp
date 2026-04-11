#include "explaino_sidecar_model.h"

#include "explaino_seed.h"
#include "fractal_family_rules.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace {

constexpr std::uint64_t kFnvOffset = 1469598103934665603ull;
constexpr std::uint64_t kFnvPrime = 1099511628211ull;

void HashBytes(std::uint64_t* ioHash, const void* data, std::size_t size) {
    const auto* bytes = static_cast<const unsigned char*>(data);
    for (std::size_t index = 0; index < size; ++index) {
        *ioHash ^= static_cast<std::uint64_t>(bytes[index]);
        *ioHash *= kFnvPrime;
    }
}

void HashString(std::uint64_t* ioHash, const std::string& value) {
    HashBytes(ioHash, value.data(), value.size());
}

void HashDouble(std::uint64_t* ioHash, double value) {
    static_assert(sizeof(double) == sizeof(std::uint64_t), "double hash assumes IEEE-754 64-bit doubles");
    std::uint64_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    HashBytes(ioHash, &bits, sizeof(bits));
}

bool ValidateSidecarContext(const BindingContext& ctx, std::string* outError) {
    if (!ctx.view || !ctx.params || !ctx.render || !ctx.lens) {
        if (outError) *outError = "Explaino sidecar model requires view, params, render, and lens bindings";
        return false;
    }
    return true;
}

const FunctionDescriptor* FindFunctionDescriptor(
    const EngineFunctionCatalog& catalog,
    const std::string& functionId) {
    for (const auto& function : catalog.functions) {
        if (function.id == functionId) return &function;
    }
    return nullptr;
}

bool TryBuildSurfaceEntry(
    const FunctionParamDescriptor& param,
    SidecarParamSurfaceEntry* outEntry,
    std::string* outError) {
    if (!outEntry) {
        if (outError) *outError = "TryBuildSurfaceEntry requires outEntry";
        return false;
    }

    SidecarParamSurfaceEntry entry;
    entry.path = param.path;
    entry.label = param.label;
    entry.type = param.type;
    entry.help = param.help;
    entry.has_default = param.has_default;
    entry.default_value = param.default_value;
    entry.has_step = param.has_step;
    entry.step_value = param.step_value;

    if (param.has_min && param.has_max) {
        if (!std::isfinite(param.min_value) || !std::isfinite(param.max_value)) {
            if (outError) *outError = "Non-finite declared range for sidecar param: " + param.path;
            return false;
        }
        if (param.max_value < param.min_value) {
            if (outError) *outError = "Invalid declared range for sidecar param: " + param.path;
            return false;
        }
        entry.has_declared_span = true;
        entry.declared_span = param.max_value - param.min_value;
    }

    *outEntry = std::move(entry);
    return true;
}

bool SidecarParamOrder(const SidecarParamSurfaceEntry& left, const SidecarParamSurfaceEntry& right) {
    if (left.has_declared_span != right.has_declared_span) return left.has_declared_span && !right.has_declared_span;
    if (left.has_declared_span && right.has_declared_span && left.declared_span != right.declared_span) {
        return left.declared_span > right.declared_span;
    }
    if (left.label != right.label) return left.label < right.label;
    return left.path < right.path;
}

std::string CurrentFractalTypeToken(const BindingContext& ctx) {
    return ctx.GetEnumId("fractal.view.fractal_type");
}

double CurrentOrientationSeed(const BindingContext& ctx) {
    if (!ctx.view || !ctx.params) return 0.0;
    if (!IsExplainoFamily(ctx.view->fractal_type)) return 0.0;
    return ExplainoSeedCombined(*ctx.view, *ctx.params);
}

std::uint64_t HashImportSignature(const BindingContext& ctx) {
    std::uint64_t hash = kFnvOffset;
    HashString(&hash, CurrentFractalTypeToken(ctx));
    HashDouble(&hash, CurrentOrientationSeed(ctx));
    return hash;
}

std::uint64_t HashProjection(const SidecarHypothesisSpace& space) {
    std::uint64_t hash = kFnvOffset;
    HashString(&hash, space.function_id);
    for (const auto& param : space.applicable_parameters) {
        HashString(&hash, param.path);
        HashString(&hash, param.type);
        HashBytes(&hash, &param.has_declared_span, sizeof(param.has_declared_span));
        if (param.has_declared_span) {
            HashDouble(&hash, param.declared_span);
        }
    }
    return hash;
}

} // namespace

bool BuildSidecarHypothesisSpace(
    const EngineFunctionCatalog& catalog,
    const std::string& functionId,
    const BindingContext& ctx,
    SidecarHypothesisSpace* outSpace,
    std::string* outError) {
    if (!outSpace) {
        if (outError) *outError = "BuildSidecarHypothesisSpace requires outSpace";
        return false;
    }
    if (!ValidateSidecarContext(ctx, outError)) {
        *outSpace = {};
        return false;
    }

    const FunctionDescriptor* function = FindFunctionDescriptor(catalog, functionId);
    if (!function) {
        *outSpace = {};
        if (outError) *outError = "Unknown sidecar function id: " + functionId;
        return false;
    }

    SidecarHypothesisSpace next;
    next.function_id = functionId;
    for (const auto& param : function->parameters) {
        if (param.has_applicable_when && !ctx.EvalVisibleIf(param.applicable_when)) {
            continue;
        }
        SidecarParamSurfaceEntry entry;
        if (!TryBuildSurfaceEntry(param, &entry, outError)) {
            *outSpace = {};
            return false;
        }
        next.applicable_parameters.push_back(std::move(entry));
    }

    std::stable_sort(next.applicable_parameters.begin(), next.applicable_parameters.end(), SidecarParamOrder);
    *outSpace = std::move(next);
    return true;
}

SidecarOrientationVector ComputeSidecarOrientationVector(
    const BindingContext& ctx,
    const SidecarHypothesisSpace& space,
    const SidecarOrientationInputs& inputs) {
    SidecarOrientationVector orientation;
    orientation.import_signature = HashImportSignature(ctx);
    orientation.pack_projection_hash = HashProjection(space);
    orientation.field_embedding_stats = static_cast<double>(space.applicable_parameters.size());
    orientation.slime_energy_delta = inputs.slime_energy_delta;
    orientation.busy_beaver_metrics = inputs.busy_beaver_metrics;
    orientation.decode_stability = inputs.decode_stability;
    orientation.diff_magnitude = inputs.diff_magnitude;
    return orientation;
}

std::uint64_t HashSidecarOrientationVector(const SidecarOrientationVector& orientation) {
    std::uint64_t hash = kFnvOffset;
    HashBytes(&hash, &orientation.import_signature, sizeof(orientation.import_signature));
    HashBytes(&hash, &orientation.pack_projection_hash, sizeof(orientation.pack_projection_hash));
    HashDouble(&hash, orientation.field_embedding_stats);
    HashDouble(&hash, orientation.slime_energy_delta);
    HashDouble(&hash, orientation.busy_beaver_metrics);
    HashDouble(&hash, orientation.decode_stability);
    HashDouble(&hash, orientation.diff_magnitude);
    return hash;
}