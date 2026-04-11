#pragma once

#include "function_descriptor.h"
#include "schema_binding.h"

#include <cstdint>
#include <string>
#include <vector>

struct SidecarOrientationVector {
    std::uint64_t import_signature{0};
    std::uint64_t pack_projection_hash{0};
    double field_embedding_stats{0.0};
    double slime_energy_delta{0.0};
    double busy_beaver_metrics{0.0};
    double decode_stability{1.0};
    double diff_magnitude{0.0};
};

struct SidecarOrientationInputs {
    double slime_energy_delta{0.0};
    double busy_beaver_metrics{0.0};
    double decode_stability{1.0};
    double diff_magnitude{0.0};
};

struct SidecarParamSurfaceEntry {
    std::string path;
    std::string label;
    std::string type;
    std::string help;
    bool has_default{false};
    json_min::Value default_value;
    bool has_declared_span{false};
    double declared_span{0.0};
    bool has_step{false};
    double step_value{0.0};
};

struct SidecarHypothesisSpace {
    std::string function_id;
    std::vector<SidecarParamSurfaceEntry> applicable_parameters;
};

bool BuildSidecarHypothesisSpace(
    const EngineFunctionCatalog& catalog,
    const std::string& functionId,
    const BindingContext& ctx,
    SidecarHypothesisSpace* outSpace,
    std::string* outError);

SidecarOrientationVector ComputeSidecarOrientationVector(
    const BindingContext& ctx,
    const SidecarHypothesisSpace& space,
    const SidecarOrientationInputs& inputs = {});

std::uint64_t HashSidecarOrientationVector(const SidecarOrientationVector& orientation);