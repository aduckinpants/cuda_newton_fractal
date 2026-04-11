#include "explaino_sidecar_model.h"

#include "explaino_seed.h"
#include "fractal_family_rules.h"

#include <algorithm>
#include <cctype>
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

void HashTag(std::uint64_t* ioHash, unsigned char tag) {
    HashBytes(ioHash, &tag, sizeof(tag));
}

void HashStringField(std::uint64_t* ioHash, unsigned char tag, const std::string& value) {
    const std::uint64_t size = static_cast<std::uint64_t>(value.size());
    HashTag(ioHash, tag);
    HashBytes(ioHash, &size, sizeof(size));
    if (size > 0) {
        HashBytes(ioHash, value.data(), static_cast<std::size_t>(size));
    }
}

void HashBoolField(std::uint64_t* ioHash, unsigned char tag, bool value) {
    const unsigned char byte = value ? 1u : 0u;
    HashTag(ioHash, tag);
    HashBytes(ioHash, &byte, sizeof(byte));
}

void HashDoubleField(std::uint64_t* ioHash, unsigned char tag, double value) {
    static_assert(sizeof(double) == sizeof(std::uint64_t), "double hash assumes IEEE-754 64-bit doubles");
    std::uint64_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    HashTag(ioHash, tag);
    HashBytes(ioHash, &bits, sizeof(bits));
}

std::string TrimAscii(const std::string& value) {
    std::size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])) != 0) {
        ++start;
    }
    std::size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])) != 0) {
        --end;
    }
    return value.substr(start, end - start);
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
    entry.has_min = param.has_min;
    entry.min_value = param.min_value;
    entry.has_max = param.has_max;
    entry.max_value = param.max_value;
    entry.has_default = param.has_default;
    entry.default_value = param.default_value;
    entry.has_step = param.has_step;
    entry.step_value = param.step_value;
    entry.has_cost_hint = param.has_cost_hint;
    entry.cost_hint = param.cost_hint;

    if (param.has_cost_hint) {
        if (!std::isfinite(param.cost_hint) || param.cost_hint < 0.0) {
            if (outError) *outError = "Invalid cost_hint for sidecar param: " + param.path;
            return false;
        }
    }

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

bool ValidateRequiredEnumSelection(
    const FunctionParamDescriptor& param,
    const BindingContext& ctx,
    std::string* outError) {
    if (!param.required || param.type != "enum") {
        return true;
    }

    if (param.options.empty()) {
        if (outError) *outError = "Required enum sidecar param has no supported options: " + param.path;
        return false;
    }

    const std::string currentEnum = ctx.GetEnumId(param.path);
    if (currentEnum.empty()) {
        if (outError) *outError = "Missing required enum selection for sidecar param: " + param.path;
        return false;
    }

    for (const auto& option : param.options) {
        if (option.id == currentEnum) {
            return true;
        }
    }

    if (outError) {
        *outError = "Unsupported required enum selection for sidecar param: " + param.path + "=" + currentEnum;
    }
    return false;
}

bool SidecarParamOrder(const SidecarParamSurfaceEntry& left, const SidecarParamSurfaceEntry& right) {
    if (left.has_declared_span != right.has_declared_span) return left.has_declared_span && !right.has_declared_span;
    if (left.has_declared_span && right.has_declared_span && left.declared_span != right.declared_span) {
        return left.declared_span > right.declared_span;
    }
    if (left.label != right.label) return left.label < right.label;
    return left.path < right.path;
}

bool ParsePredicateBool(const std::string& value, bool* outValue) {
    if (!outValue) return false;
    const std::string trimmed = TrimAscii(value);
    if (trimmed == "true" || trimmed == "1") {
        *outValue = true;
        return true;
    }
    if (trimmed == "false" || trimmed == "0") {
        *outValue = false;
        return true;
    }
    return false;
}

bool ParsePredicateDouble(const std::string& value, double* outValue) {
    if (!outValue) return false;
    const std::string trimmed = TrimAscii(value);
    if (trimmed.empty()) return false;
    std::size_t consumed = 0;
    try {
        const double parsed = std::stod(trimmed, &consumed);
        if (consumed != trimmed.size()) return false;
        *outValue = parsed;
        return true;
    } catch (...) {
        return false;
    }
}

bool EvaluateSidecarPredicateStrict(
    const BindingContext& ctx,
    const UISchemaPredicate& pred,
    bool* outApplicable,
    std::string* outError) {
    if (!outApplicable) {
        if (outError) *outError = "EvaluateSidecarPredicateStrict requires outApplicable";
        return false;
    }
    if (pred.op.empty() || pred.path.empty()) {
        if (outError) *outError = "Sidecar applicable_when must include both op and path";
        return false;
    }

    const std::string currentEnum = ctx.GetEnumId(pred.path);
    if (!currentEnum.empty()) {
        if (pred.op == "eq") {
            *outApplicable = currentEnum == pred.value;
            return true;
        }
        if (pred.op == "neq") {
            *outApplicable = currentEnum != pred.value;
            return true;
        }
        if (pred.op == "in") {
            const std::string values = pred.value;
            std::size_t start = 0;
            while (start <= values.size()) {
                const std::size_t comma = values.find(',', start);
                const std::size_t end = (comma == std::string::npos) ? values.size() : comma;
                const std::string token = TrimAscii(values.substr(start, end - start));
                if (!token.empty() && token == currentEnum) {
                    *outApplicable = true;
                    return true;
                }
                if (comma == std::string::npos) break;
                start = comma + 1;
            }
            *outApplicable = false;
            return true;
        }
        if (outError) *outError = "Unsupported enum applicable_when op for sidecar param: " + pred.op;
        return false;
    }

    bool currentBool = false;
    if (ctx.GetBoolValue(pred.path, currentBool)) {
        bool rhsBool = false;
        if (!ParsePredicateBool(pred.value, &rhsBool)) {
            if (outError) *outError = "Invalid bool applicable_when value for sidecar param: " + pred.value;
            return false;
        }
        if (pred.op == "eq") {
            *outApplicable = currentBool == rhsBool;
            return true;
        }
        if (pred.op == "neq") {
            *outApplicable = currentBool != rhsBool;
            return true;
        }
        if (outError) *outError = "Unsupported bool applicable_when op for sidecar param: " + pred.op;
        return false;
    }

    double currentNumeric = 0.0;
    int currentInt = 0;
    float currentFloat = 0.0f;
    double currentDouble = 0.0;
    if (ctx.GetIntValue(pred.path, currentInt)) {
        currentNumeric = static_cast<double>(currentInt);
    } else if (ctx.GetFloatValue(pred.path, currentFloat)) {
        currentNumeric = static_cast<double>(currentFloat);
    } else if (ctx.GetDoubleValue(pred.path, currentDouble)) {
        currentNumeric = currentDouble;
    } else {
        if (outError) *outError = "Unknown applicable_when binding path for sidecar param: " + pred.path;
        return false;
    }

    double rhsNumeric = 0.0;
    if (!ParsePredicateDouble(pred.value, &rhsNumeric)) {
        if (outError) *outError = "Invalid numeric applicable_when value for sidecar param: " + pred.value;
        return false;
    }

    if (pred.op == "eq") {
        *outApplicable = currentNumeric == rhsNumeric;
        return true;
    }
    if (pred.op == "neq") {
        *outApplicable = currentNumeric != rhsNumeric;
        return true;
    }
    if (pred.op == "lt") {
        *outApplicable = currentNumeric < rhsNumeric;
        return true;
    }
    if (pred.op == "lte") {
        *outApplicable = currentNumeric <= rhsNumeric;
        return true;
    }
    if (pred.op == "gt") {
        *outApplicable = currentNumeric > rhsNumeric;
        return true;
    }
    if (pred.op == "gte") {
        *outApplicable = currentNumeric >= rhsNumeric;
        return true;
    }
    if (outError) *outError = "Unsupported numeric applicable_when op for sidecar param: " + pred.op;
    return false;
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
    HashStringField(&hash, 0x01u, CurrentFractalTypeToken(ctx));
    HashDoubleField(&hash, 0x02u, CurrentOrientationSeed(ctx));
    return hash;
}

std::uint64_t HashProjection(const SidecarHypothesisSpace& space) {
    std::uint64_t hash = kFnvOffset;
    HashStringField(&hash, 0x10u, space.function_id);
    for (const auto& param : space.applicable_parameters) {
        HashStringField(&hash, 0x11u, param.path);
        HashStringField(&hash, 0x12u, param.type);
        HashBoolField(&hash, 0x13u, param.has_declared_span);
        if (param.has_declared_span) {
            HashDoubleField(&hash, 0x14u, param.declared_span);
        }
        HashBoolField(&hash, 0x15u, param.has_cost_hint);
        if (param.has_cost_hint) {
            HashDoubleField(&hash, 0x16u, param.cost_hint);
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
        if (!ValidateRequiredEnumSelection(param, ctx, outError)) {
            *outSpace = {};
            return false;
        }
        if (param.has_applicable_when) {
            bool applicable = false;
            if (!EvaluateSidecarPredicateStrict(ctx, param.applicable_when, &applicable, outError)) {
                *outSpace = {};
                return false;
            }
            if (!applicable) {
                continue;
            }
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
    HashDoubleField(&hash, 0x20u, orientation.field_embedding_stats);
    HashDoubleField(&hash, 0x21u, orientation.slime_energy_delta);
    HashDoubleField(&hash, 0x22u, orientation.busy_beaver_metrics);
    HashDoubleField(&hash, 0x23u, orientation.decode_stability);
    HashDoubleField(&hash, 0x24u, orientation.diff_magnitude);
    return hash;
}