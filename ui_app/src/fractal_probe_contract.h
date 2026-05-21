#pragma once

#include "fractal_types.h"
#include "json_min.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

enum class FractalProbeMode {
    point_set = 0,
    grid = 1,
    sequence_point_set = 2,
    sequence_grid = 3,
};

enum class FractalProbeOutputMode {
    json = 0,
    ndjson = 1,
};

enum class FractalProbeExecutionBackendPreference {
    default_backend = 0,
    cpu = 1,
    cuda = 2,
};

enum class FractalProbeSequenceMode {
    axes = 0,
    variant_crossfade = 1,
};

enum class FractalProbeSampleStatus {
    escaped = 0,
    converged = 1,
    bounded = 2,
    pole = 3,
    nonfinite = 4,
    invalid_param = 5,
};

struct FractalProbeScalar {
    enum class Kind {
        boolean = 0,
        number = 1,
        string = 2,
    };

    Kind kind{Kind::number};
    bool bool_value{false};
    double number_value{0.0};
    std::string string_value;

    static FractalProbeScalar Boolean(bool value) {
        FractalProbeScalar scalar;
        scalar.kind = Kind::boolean;
        scalar.bool_value = value;
        return scalar;
    }

    static FractalProbeScalar Number(double value) {
        FractalProbeScalar scalar;
        scalar.kind = Kind::number;
        scalar.number_value = value;
        return scalar;
    }

    static FractalProbeScalar String(const std::string& value) {
        FractalProbeScalar scalar;
        scalar.kind = Kind::string;
        scalar.string_value = value;
        return scalar;
    }
};

struct FractalProbeOverride {
    std::string path;
    FractalProbeScalar value;
};

struct FractalProbePoint {
    double x{0.0};
    double y{0.0};
};

struct FractalProbeRegion {
    double center_x{0.0};
    double center_y{0.0};
    double span_x{0.0};
    double span_y{0.0};
    int grid_width{0};
    int grid_height{0};
};

struct FractalProbeSequenceAxis {
    std::string path;
    std::vector<FractalProbeScalar> values;
};

struct FractalProbeVariantCrossfade {
    std::string from_variant_id;
    std::string to_variant_id;
    int steps{0};
};

struct FractalProbeSequence {
    FractalProbeSequenceMode mode{FractalProbeSequenceMode::axes};
    bool zip_paths{false};
    std::vector<FractalProbeSequenceAxis> axes;
    FractalProbeVariantCrossfade variant_crossfade;
};

struct FractalProbeOperatorContext {
    std::string source;
    std::string operator_name;
    std::string why;
};

struct FractalProbeExecutionOptions {
    FractalProbeExecutionBackendPreference backend_preference{
        FractalProbeExecutionBackendPreference::default_backend};
};

struct FractalProbeRequest {
    int request_version{0};
    std::string request_id;
    std::string function_id; // default: "fractal.sample"; unknown ids fail fast
    std::string state_token; // V2-C: references accumulated session state
    FractalProbeMode mode{FractalProbeMode::point_set};
    FractalProbeOutputMode output_mode{FractalProbeOutputMode::json};
    FractalProbeExecutionOptions execution;
    std::string base_state_load_path;
    std::vector<FractalProbeOverride> overrides;
    bool has_region{false};
    FractalProbeRegion region;
    std::vector<FractalProbePoint> points;
    bool has_sequence{false};
    FractalProbeSequence sequence;
    std::vector<std::string> metrics;
    FractalProbeOperatorContext operator_context;

    // Generic function fields (used when function_id == "generic.sample").
    bool has_function{false};
    std::string generic_expression;
    bool has_generic_ast{false};
    json_min::Value generic_ast;
    std::string generic_iterate_count_param;
    std::map<std::string, double> generic_params;
    double generic_epsilon{1e-6};
    double generic_escape_radius{1000.0};
};

struct FractalProbeRuntimeInfo {
    std::string exe_path;
    std::string fractal_type;
    int device_id{0};
    std::string backend_used;
};

struct FractalProbeCost {
    double gpu_ms{0.0};
    int sample_count{0};
};

struct FractalProbeSummary {
    int sample_count{0};
    double mean_iterations{0.0};
    double escape_fraction{0.0};
    double converged_fraction{0.0};
    double nonfinite_fraction{0.0};
    double pole_fraction{0.0};
    double mean_abs2{0.0};
    double diverged_fraction{0.0};
    int best_sequence_index{-1};
};

struct FractalProbeMetricSelection {
    bool include_iterations{true};
    bool include_status{true};
    bool include_final_z{true};
    bool include_final_abs2{true};
    bool include_residual{true};
    bool include_root_index{true};
    bool include_summary_mean_iterations{true};
    bool include_summary_escape_fraction{true};
    bool include_summary_converged_fraction{true};
    bool include_summary_nonfinite_fraction{true};
    bool include_summary_pole_fraction{true};
    bool include_summary_best_sequence_index{true};
    bool include_derivative{true};
    bool include_value{true};
    bool include_abs2{true};
    bool include_summary_mean_abs2{true};
    bool include_summary_diverged_fraction{true};
};

struct FractalProbeSequenceResult {
    int sequence_index{0};
    std::vector<std::pair<std::string, FractalProbeScalar>> applied;
    double mean_iterations{0.0};
    double escape_fraction{0.0};
    double converged_fraction{0.0};
    double nonfinite_fraction{0.0};
    double pole_fraction{0.0};
};

struct FractalProbeSample {
    int sequence_index{0};
    int grid_x{-1};
    int grid_y{-1};
    double coord_x{0.0};
    double coord_y{0.0};
    int iterations{0};
    FractalProbeSampleStatus status{FractalProbeSampleStatus::bounded};
    TerminationKind termination_kind{TerminationKind::none};
    double final_z_x{0.0};
    double final_z_y{0.0};
    double final_abs2{0.0};
    bool has_residual{false};
    double residual{0.0};
    bool has_far_field_delta{false};
    double far_field_delta{0.0};
    bool has_root_index{false};
    int root_index{-1};
    double derivative_x{0.0};
    double derivative_y{0.0};
};

struct FractalProbeResponse {
    int response_version{1};
    std::string request_id;
    std::string function_id; // echoes back the resolved function_id
    bool ok{false};
    FractalProbeRuntimeInfo runtime;
    FractalProbeCost cost;
    FractalProbeSummary summary;
    FractalProbeMetricSelection metric_selection;
    std::vector<FractalProbeSequenceResult> sequence_results;
    std::vector<FractalProbeSample> samples;
    FractalProbeOperatorContext operator_context;
    std::string error;
};

bool ParseFractalProbeRequestJson(const std::string& text,
    FractalProbeRequest* outRequest,
    std::string* outError);

bool ParseFractalProbeRequestFromValue(const json_min::Value& value,
    FractalProbeRequest* outRequest,
    std::string* outError);

std::string SerializeFractalProbeResponseJson(const FractalProbeResponse& response);
std::string SerializeFractalProbeNdjsonSampleBatchJson(
    const std::string& requestId,
    const std::string& functionId,
    int sequenceIndex,
    int rowIndex,
    const std::vector<FractalProbeSample>& samples,
    const FractalProbeMetricSelection& selection);
std::string SerializeFractalProbeNdjsonSummaryJson(
    const FractalProbeResponse& response,
    const std::string& stateToken);

FractalProbeMetricSelection BuildFractalProbeMetricSelection(const std::vector<std::string>& metrics);
bool FractalProbeSelectionIncludesAnySampleMetrics(const FractalProbeMetricSelection& selection);

const char* FractalProbeModeId(FractalProbeMode mode);
const char* FractalProbeOutputModeId(FractalProbeOutputMode mode);
const char* FractalProbeSampleStatusId(FractalProbeSampleStatus status);
const char* TerminationKindId(TerminationKind terminationKind);
