#pragma once

#include "enum_id_utils.h"
#include "fractal_family_rules.h"

#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

struct ColorPipelineCurrentFieldValidity {
    std::string capability_id;
    std::string status = "not_observed";
    std::string reason;
};

struct ColorPipelineQualityObservation {
    std::string observation_id;
    std::string status = "not_measured";
    std::string note;
    double value = 0.0;
    bool has_value = false;
};

struct ColorPipelineCapabilityRuntimeObservation {
    bool frame_observed = false;
    bool frame_valid = false;
    bool sdf_field_requested = false;
    bool sdf_field_valid = false;
    std::string sdf_field_producer_id = "none";
    std::string sdf_field_fail_closed_reason;
};

struct ColorPipelineProducerCapabilitySnapshot {
    bool initialized = false;
    std::string schema_id = "viewer.color_pipeline_producer_capabilities.v1";
    std::uint64_t producer_generation = 0;
    std::string snapshot_id;
    std::string producer_id = "unknown";
    std::string evaluator_id = "unknown";
    std::string fractal_precision_tier = "unknown";
    std::string color_metric_arithmetic_tier = "unknown";
    std::string sdf_field_producer_id = "none";
    std::vector<std::string> static_capability_ids;
    std::vector<ColorPipelineCurrentFieldValidity> current_field_validity;
    std::vector<ColorPipelineQualityObservation> quality_observations;
};

struct ColorPipelineRecipeApplicability {
    std::string recipe_id;
    bool available = false;
    std::string reason_code = "capability_snapshot_unavailable";
    std::string reason;
    std::string capability_snapshot_id;
    std::uint64_t producer_generation = 0;
    std::vector<std::string> required_capability_ids;
    std::vector<std::string> missing_capability_ids;
};

inline const char* ColorPipelineNumericBackendId(NumericBackend backend) {
    switch (backend) {
    case NumericBackend::float64:
        return "float64";
    case NumericBackend::float32:
    default:
        return "float32";
    }
}

inline const char* ColorPipelineIterationStrategyId(IterationStrategy strategy) {
    switch (strategy) {
    case IterationStrategy::direct:
    default:
        return "direct";
    }
}

inline std::string ColorPipelineSourceCapabilityId(std::string_view functionId) {
    return std::string("color_pipeline.source.") + std::string(functionId);
}

inline bool ColorPipelineProducerSupportsSourceFunction(
    FractalType fractalType,
    std::string_view functionId) {
    ColorPipelineSelection pipeline{};
    if (functionId == "smooth_escape_ramp") {
        pipeline = {ColorSignal::smooth_escape, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
    } else if (functionId == "phase_orbit") {
        pipeline = {ColorSignal::phase_angle, ColorPalette::phase_wheel, ColorGradingPreset::phase_default};
    } else if (functionId == "root_phase") {
        pipeline = {ColorSignal::root_phase, ColorPalette::phase_wheel, ColorGradingPreset::phase_default};
    } else if (functionId == "root_proximity") {
        pipeline = {ColorSignal::root_proximity, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
    } else if (functionId == "root_log_proximity_v1") {
        pipeline = {ColorSignal::root_log_proximity_v1, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
    } else if (functionId == "sdf_normal_angle") {
        pipeline = {ColorSignal::sdf_normal_angle, ColorPalette::phase_wheel, ColorGradingPreset::phase_default};
    } else if (functionId == "sdf_curvature") {
        pipeline = {ColorSignal::sdf_curvature, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
    } else if (functionId == "lens_field_v2_distance") {
        pipeline = {ColorSignal::lens_field_v2_distance, ColorPalette::cyclic_escape, ColorGradingPreset::escape_default};
    } else {
        return false;
    }
    return IsColorPipelineAllowedForFractal(fractalType, pipeline);
}

inline void AddUniqueColorPipelineCapability(
    std::vector<std::string>* ioCapabilities,
    const std::string& capabilityId) {
    if (!ioCapabilities || capabilityId.empty() ||
        std::find(ioCapabilities->begin(), ioCapabilities->end(), capabilityId) != ioCapabilities->end()) {
        return;
    }
    ioCapabilities->push_back(capabilityId);
}

inline std::uint64_t HashColorPipelineCapabilityText(std::uint64_t hash, std::string_view value) {
    constexpr std::uint64_t kPrime = 1099511628211ull;
    for (unsigned char ch : value) {
        hash ^= static_cast<std::uint64_t>(ch);
        hash *= kPrime;
    }
    hash ^= 0xffu;
    hash *= kPrime;
    return hash;
}

inline std::string BuildColorPipelineCapabilitySnapshotId(
    const ColorPipelineProducerCapabilitySnapshot& snapshot) {
    std::uint64_t hash = 1469598103934665603ull;
    hash = HashColorPipelineCapabilityText(hash, snapshot.schema_id);
    hash = HashColorPipelineCapabilityText(hash, snapshot.producer_id);
    hash = HashColorPipelineCapabilityText(hash, snapshot.evaluator_id);
    hash = HashColorPipelineCapabilityText(hash, snapshot.fractal_precision_tier);
    hash = HashColorPipelineCapabilityText(hash, snapshot.color_metric_arithmetic_tier);
    hash = HashColorPipelineCapabilityText(hash, snapshot.sdf_field_producer_id);
    for (const std::string& capabilityId : snapshot.static_capability_ids) {
        hash = HashColorPipelineCapabilityText(hash, capabilityId);
    }
    for (const ColorPipelineCurrentFieldValidity& validity : snapshot.current_field_validity) {
        hash = HashColorPipelineCapabilityText(hash, validity.capability_id);
        hash = HashColorPipelineCapabilityText(hash, validity.status);
        hash = HashColorPipelineCapabilityText(hash, validity.reason);
    }
    std::ostringstream out;
    out << "cap:" << std::hex << std::setw(16) << std::setfill('0') << hash;
    return out.str();
}

inline ColorPipelineProducerCapabilitySnapshot BuildColorPipelineProducerCapabilitySnapshot(
    const ColorPipelineProducerCapabilitySnapshot& previous,
    FractalType fractalType,
    const ResolvedEvalMode& resolvedEval,
    const ColorPipelineCapabilityRuntimeObservation& observation) {
    ColorPipelineProducerCapabilitySnapshot snapshot{};
    snapshot.initialized = true;
    const char* fractalTypeId = FractalTypeId(fractalType);
    snapshot.producer_id = fractalTypeId ? fractalTypeId : "unknown";
    snapshot.fractal_precision_tier = ColorPipelineNumericBackendId(resolvedEval.backend);
    snapshot.evaluator_id = std::string("cuda.") +
        ColorPipelineIterationStrategyId(resolvedEval.strategy) + "." +
        snapshot.fractal_precision_tier;
    snapshot.color_metric_arithmetic_tier = "float32";
    snapshot.sdf_field_producer_id = observation.sdf_field_producer_id.empty()
        ? "none"
        : observation.sdf_field_producer_id;

    AddUniqueColorPipelineCapability(
        &snapshot.static_capability_ids,
        "color_pipeline.recipe_application");
    constexpr std::string_view kSourceFunctions[] = {
        "smooth_escape_ramp",
        "phase_orbit",
        "root_phase",
        "root_proximity",
        "root_log_proximity_v1",
        "sdf_normal_angle",
        "sdf_curvature",
        "lens_field_v2_distance",
    };
    for (std::string_view functionId : kSourceFunctions) {
        if (ColorPipelineProducerSupportsSourceFunction(fractalType, functionId)) {
            AddUniqueColorPipelineCapability(
                &snapshot.static_capability_ids,
                ColorPipelineSourceCapabilityId(functionId));
        }
    }
    if (ColorPipelineProducerSupportsSourceFunction(fractalType, "sdf_normal_angle")) {
        AddUniqueColorPipelineCapability(&snapshot.static_capability_ids, "sdf.field.signed_distance");
        AddUniqueColorPipelineCapability(&snapshot.static_capability_ids, "sdf.field.normal_angle");
        AddUniqueColorPipelineCapability(&snapshot.static_capability_ids, "sdf.field.curvature");
        AddUniqueColorPipelineCapability(&snapshot.static_capability_ids, "sdf.field.lens_field_v2_response");
    }
    std::sort(snapshot.static_capability_ids.begin(), snapshot.static_capability_ids.end());

    snapshot.current_field_validity.push_back({
        "renderer.frame",
        observation.frame_observed ? (observation.frame_valid ? "valid" : "invalid") : "not_observed",
        observation.frame_observed && !observation.frame_valid ? "latest renderer frame is invalid" : "",
    });
    const std::string sdfStatus = !observation.sdf_field_requested
        ? "not_requested"
        : (observation.sdf_field_valid ? "valid" : "invalid");
    const std::string sdfReason = observation.sdf_field_requested && !observation.sdf_field_valid
        ? (observation.sdf_field_fail_closed_reason.empty()
            ? "latest SDF field evaluation is invalid"
            : observation.sdf_field_fail_closed_reason)
        : "";
    for (const char* capabilityId : {
             "sdf.field.signed_distance",
             "sdf.field.normal_angle",
             "sdf.field.curvature",
             "sdf.field.lens_field_v2_response"}) {
        snapshot.current_field_validity.push_back({capabilityId, sdfStatus, sdfReason});
    }
    if (ColorPipelineProducerSupportsSourceFunction(fractalType, "root_phase") ||
        ColorPipelineProducerSupportsSourceFunction(fractalType, "root_proximity") ||
        ColorPipelineProducerSupportsSourceFunction(fractalType, "root_log_proximity_v1")) {
        snapshot.current_field_validity.push_back({
            "root_field.authoritative",
            "not_observed",
            "per-frame root metric validity is not exported by the renderer in v1",
        });
    }

    snapshot.snapshot_id = BuildColorPipelineCapabilitySnapshotId(snapshot);
    snapshot.producer_generation =
        previous.initialized && previous.snapshot_id == snapshot.snapshot_id
            ? previous.producer_generation
            : previous.producer_generation + 1;
    if (snapshot.producer_generation == 0) {
        snapshot.producer_generation = 1;
    }
    return snapshot;
}

inline bool HasColorPipelineProducerCapability(
    const ColorPipelineProducerCapabilitySnapshot& snapshot,
    std::string_view capabilityId) {
    return std::find(
        snapshot.static_capability_ids.begin(),
        snapshot.static_capability_ids.end(),
        capabilityId) != snapshot.static_capability_ids.end();
}

inline const ColorPipelineCurrentFieldValidity* FindColorPipelineCurrentFieldValidity(
    const ColorPipelineProducerCapabilitySnapshot& snapshot,
    std::string_view capabilityId) {
    for (const ColorPipelineCurrentFieldValidity& validity : snapshot.current_field_validity) {
        if (validity.capability_id == capabilityId) {
            return &validity;
        }
    }
    return nullptr;
}

inline std::string_view ColorPipelineRecipeQualificationFailure(std::string_view recipeId) {
    (void)recipeId;
    return {};
}

inline ColorPipelineRecipeApplicability EvaluateColorPipelineRecipeApplicability(
    const ColorPipelineProducerCapabilitySnapshot& snapshot,
    const std::string& recipeId,
    const std::vector<std::string>& requiredCapabilityIds,
    std::string_view expectedSnapshotId = {}) {
    ColorPipelineRecipeApplicability result{};
    result.recipe_id = recipeId;
    result.capability_snapshot_id = snapshot.snapshot_id;
    result.producer_generation = snapshot.producer_generation;
    result.required_capability_ids = requiredCapabilityIds;
    if (!snapshot.initialized) {
        result.reason = "runtime-owned capability snapshot is not initialized";
        return result;
    }
    if (!expectedSnapshotId.empty() && expectedSnapshotId != snapshot.snapshot_id) {
        result.reason_code = "capability_snapshot_stale";
        result.reason = "recipe was resolved against a different producer capability snapshot";
        return result;
    }
    for (const std::string& capabilityId : requiredCapabilityIds) {
        if (!HasColorPipelineProducerCapability(snapshot, capabilityId)) {
            result.missing_capability_ids.push_back(capabilityId);
        }
    }
    if (!result.missing_capability_ids.empty()) {
        result.reason_code = "missing_required_capability";
        result.reason = "active producer does not support every required recipe capability";
        return result;
    }
    const std::string_view qualificationFailure =
        ColorPipelineRecipeQualificationFailure(recipeId);
    if (!qualificationFailure.empty()) {
        result.reason_code = "recipe_qualification_failed";
        result.reason = std::string(qualificationFailure);
        return result;
    }
    result.available = true;
    result.reason_code = "available";
    result.reason = "all required static producer capabilities are available";
    return result;
}

inline std::string ColorPipelineCapabilityJsonEscape(std::string_view value) {
    std::ostringstream out;
    for (unsigned char ch : value) {
        switch (ch) {
        case '"': out << "\\\""; break;
        case '\\': out << "\\\\"; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default:
            if (ch < 0x20) {
                out << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                    << static_cast<int>(ch) << std::dec;
            } else {
                out << static_cast<char>(ch);
            }
            break;
        }
    }
    return out.str();
}

inline void WriteColorPipelineCapabilityJsonString(
    std::ostringstream& out,
    std::string_view value) {
    out << '"' << ColorPipelineCapabilityJsonEscape(value) << '"';
}

inline std::string BuildColorPipelineRecipeCapabilityReportJson(
    const ColorPipelineProducerCapabilitySnapshot& snapshot,
    const std::vector<ColorPipelineRecipeApplicability>& applicability) {
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema_id\": \"viewer.color_pipeline_recipe_capability_report.v1\",\n";
    out << "  \"snapshot\": {\n";
    out << "    \"schema_id\": ";
    WriteColorPipelineCapabilityJsonString(out, snapshot.schema_id);
    out << ",\n    \"snapshot_id\": ";
    WriteColorPipelineCapabilityJsonString(out, snapshot.snapshot_id);
    out << ",\n    \"producer_generation\": " << snapshot.producer_generation;
    out << ",\n    \"producer_id\": ";
    WriteColorPipelineCapabilityJsonString(out, snapshot.producer_id);
    out << ",\n    \"evaluator_id\": ";
    WriteColorPipelineCapabilityJsonString(out, snapshot.evaluator_id);
    out << ",\n    \"fractal_precision_tier\": ";
    WriteColorPipelineCapabilityJsonString(out, snapshot.fractal_precision_tier);
    out << ",\n    \"color_metric_arithmetic_tier\": ";
    WriteColorPipelineCapabilityJsonString(out, snapshot.color_metric_arithmetic_tier);
    out << ",\n    \"sdf_field_producer_id\": ";
    WriteColorPipelineCapabilityJsonString(out, snapshot.sdf_field_producer_id);
    out << ",\n    \"static_capability_ids\": [";
    for (std::size_t index = 0; index < snapshot.static_capability_ids.size(); ++index) {
        if (index > 0) out << ", ";
        WriteColorPipelineCapabilityJsonString(out, snapshot.static_capability_ids[index]);
    }
    out << "],\n    \"current_field_validity\": [";
    for (std::size_t index = 0; index < snapshot.current_field_validity.size(); ++index) {
        if (index > 0) out << ",";
        const ColorPipelineCurrentFieldValidity& validity = snapshot.current_field_validity[index];
        out << "\n      {\"capability_id\": ";
        WriteColorPipelineCapabilityJsonString(out, validity.capability_id);
        out << ", \"status\": ";
        WriteColorPipelineCapabilityJsonString(out, validity.status);
        out << ", \"reason\": ";
        WriteColorPipelineCapabilityJsonString(out, validity.reason);
        out << "}";
    }
    if (!snapshot.current_field_validity.empty()) out << "\n    ";
    out << "],\n    \"quality_observations\": [";
    for (std::size_t index = 0; index < snapshot.quality_observations.size(); ++index) {
        if (index > 0) out << ",";
        const ColorPipelineQualityObservation& observation = snapshot.quality_observations[index];
        out << "\n      {\"observation_id\": ";
        WriteColorPipelineCapabilityJsonString(out, observation.observation_id);
        out << ", \"status\": ";
        WriteColorPipelineCapabilityJsonString(out, observation.status);
        out << ", \"note\": ";
        WriteColorPipelineCapabilityJsonString(out, observation.note);
        out << ", \"value\": ";
        if (observation.has_value) out << observation.value;
        else out << "null";
        out << "}";
    }
    if (!snapshot.quality_observations.empty()) out << "\n    ";
    out << "]\n  },\n  \"recipe_applicability\": [";
    for (std::size_t index = 0; index < applicability.size(); ++index) {
        if (index > 0) out << ",";
        const ColorPipelineRecipeApplicability& item = applicability[index];
        out << "\n    {\"recipe_id\": ";
        WriteColorPipelineCapabilityJsonString(out, item.recipe_id);
        out << ", \"available\": " << (item.available ? "true" : "false");
        out << ", \"reason_code\": ";
        WriteColorPipelineCapabilityJsonString(out, item.reason_code);
        out << ", \"reason\": ";
        WriteColorPipelineCapabilityJsonString(out, item.reason);
        out << ", \"capability_snapshot_id\": ";
        WriteColorPipelineCapabilityJsonString(out, item.capability_snapshot_id);
        out << ", \"producer_generation\": " << item.producer_generation;
        out << ", \"required_capability_ids\": [";
        for (std::size_t requiredIndex = 0;
             requiredIndex < item.required_capability_ids.size();
             ++requiredIndex) {
            if (requiredIndex > 0) out << ", ";
            WriteColorPipelineCapabilityJsonString(out, item.required_capability_ids[requiredIndex]);
        }
        out << "], \"missing_capability_ids\": [";
        for (std::size_t missingIndex = 0;
             missingIndex < item.missing_capability_ids.size();
             ++missingIndex) {
            if (missingIndex > 0) out << ", ";
            WriteColorPipelineCapabilityJsonString(out, item.missing_capability_ids[missingIndex]);
        }
        out << "]}";
    }
    if (!applicability.empty()) out << "\n  ";
    out << "]\n}";
    return out.str();
}
