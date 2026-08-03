#include "fractal_probe_runner.h"

#include "basin_coloring.h"
#include "enum_id_utils.h"
#include "explaino_seed.h"
#include "explaino_seed_curve.h"
#include "diagnostics_state_io.h"
#include "explaino_collatz_formulas.h"
#include "finding_state_actions.h"
#include "fractal_derived_fields.h"
#include "escape_time_direct_formulas.h"
#include "escape_time_specialized_formulas.h"
#include "fractal_family_rules.h"
#include "polynomial_eval_real_coeffs.h"
#include "fractal_runtime_validation.h"
#include "function_descriptor.h"
#include "fractal_sample_result.h"
#include "runtime_reset.h"
#include "schema_binding.h"
#include "view_hp_sync.h"
#include "generic_function_parser.h"
#include "generic_function_types.h"
#include "generic_function_cpu_eval.h"
#include "generic_sample_core.h"
#include "generic_equation_pack.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace {

struct ProbeState {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    LensSettings lens{};
};

struct Cx {
    float x;
    float y;
};

constexpr float kPi = 3.14159265358979323846f;

bool IsViewUiPath(const std::string& path) {
    return path == "fractal.view.center.x" ||
        path == "fractal.view.center.y" ||
        path == "fractal.view.zoom" ||
        path == "fractal.view.rotation" ||
        path == "fractal.view.fractal_type";
}

bool ValidateProbeState(const ProbeState& state, std::string* outError) {
    return ValidateFractalRuntimeState(state.view, state.params, outError);
}

void ApplyFractalTypeDefaults(ViewState* ioView, KernelParams* ioParams, bool* ioSyncViewHp) {
    if (!ioView || !ioParams) return;
    bool dirty = false;
    ApplyFractalViewPresetDefaults(*ioView, &dirty);
    ApplyFractalPresetDefaults(*ioView, *ioParams, &dirty);
    if (IsExplainoFamily(ioView->fractal_type)) {
        UpdateExplainoPolynomial(*ioView, *ioParams, nullptr);
    } else if (ioParams->poly_kind != PolyKind::custom) {
        SetPolyPreset(*ioParams);
    }
    if (ioSyncViewHp) *ioSyncViewHp = true;
}

bool ApplyFractalTypeOverridePreservingState(const std::string& fractalTypeId,
    ProbeState* ioState,
    std::string* outError) {
    if (!ioState) {
        if (outError) *outError = "ApplyFractalTypeOverridePreservingState requires state";
        return false;
    }
    BindingContext ctx;
    ctx.view = &ioState->view;
    ctx.params = &ioState->params;
    ctx.render = &ioState->render;
    ctx.lens = &ioState->lens;
    if (!ctx.SetEnumId("fractal.view.fractal_type", fractalTypeId)) {
        if (outError) *outError = "Unknown enum id for fractal.view.fractal_type: " + fractalTypeId;
        return false;
    }
    return true;
}

bool ApplySingleOverride(const FractalProbeOverride& overrideValue,
    ProbeState* ioState,
    bool* ioSyncViewHp,
    bool allowFractalType,
    std::string* outError) {
    if (!ioState) {
        if (outError) *outError = "ApplySingleOverride requires state";
        return false;
    }
    if (!allowFractalType && overrideValue.path == "fractal.view.fractal_type") {
        if (outError) *outError = "sequence overrides may not vary fractal.view.fractal_type in V1";
        return false;
    }

    BindingContext ctx;
    ctx.view = &ioState->view;
    ctx.params = &ioState->params;
    ctx.render = &ioState->render;
    ctx.lens = &ioState->lens;

    if (overrideValue.path == "fractal.view.fractal_type") {
        if (overrideValue.value.kind != FractalProbeScalar::Kind::string) {
            if (outError) *outError = "fractal.view.fractal_type override requires a string enum id";
            return false;
        }
        if (!ctx.SetEnumId(overrideValue.path, overrideValue.value.string_value)) {
            if (outError) *outError = "Unknown enum id for fractal.view.fractal_type: " + overrideValue.value.string_value;
            return false;
        }
        ApplyFractalTypeDefaults(&ioState->view, &ioState->params, ioSyncViewHp);
        return true;
    }

    if (overrideValue.value.kind == FractalProbeScalar::Kind::string) {
        if (!ctx.SetEnumId(overrideValue.path, overrideValue.value.string_value)) {
            if (outError) *outError = "Unknown enum binding path or id: " + overrideValue.path;
            return false;
        }
        return true;
    }

    if (overrideValue.value.kind == FractalProbeScalar::Kind::boolean) {
        if (!ctx.SetBoolValue(overrideValue.path, overrideValue.value.bool_value)) {
            if (outError) *outError = "Unknown bool binding path: " + overrideValue.path;
            return false;
        }
        return true;
    }

    const double raw = overrideValue.value.number_value;
    int* intPtr = nullptr;
    if (ctx.BindInt(overrideValue.path, &intPtr) && intPtr) {
        if (!std::isfinite(raw) || std::floor(raw) != raw) {
            if (outError) *outError = "Integer binding requires an integral numeric value: " + overrideValue.path;
            return false;
        }
        *intPtr = static_cast<int>(raw);
        return true;
    }

    double* doublePtr = nullptr;
    if (ctx.BindDouble(overrideValue.path, &doublePtr) && doublePtr) {
        if (!std::isfinite(raw)) {
            if (outError) *outError = "Double binding requires a finite value: " + overrideValue.path;
            return false;
        }
        *doublePtr = raw;
        return true;
    }

    float* floatPtr = nullptr;
    if (ctx.BindFloat(overrideValue.path, &floatPtr) && floatPtr) {
        if (!std::isfinite(raw)) {
            if (outError) *outError = "Float binding requires a finite value: " + overrideValue.path;
            return false;
        }
        *floatPtr = static_cast<float>(raw);
        if (ioSyncViewHp && IsViewUiPath(overrideValue.path)) *ioSyncViewHp = true;
        return true;
    }

    if (outError) *outError = "Unknown numeric binding path: " + overrideValue.path;
    return false;
}

bool ApplyOverridesWithFractalTypeFirst(const std::vector<FractalProbeOverride>& overrides,
    ProbeState* ioState,
    bool allowFractalType,
    std::string* outError) {
    if (!ioState) {
        if (outError) *outError = "ApplyOverridesWithFractalTypeFirst requires state";
        return false;
    }

    bool syncViewHp = false;
    int fractalTypeIndex = -1;
    for (size_t index = 0; index < overrides.size(); ++index) {
        if (overrides[index].path == "fractal.view.fractal_type") {
            fractalTypeIndex = static_cast<int>(index);
        }
    }

    if (fractalTypeIndex >= 0) {
        if (!ApplySingleOverride(overrides[static_cast<size_t>(fractalTypeIndex)], ioState, &syncViewHp, allowFractalType, outError)) {
            return false;
        }
    }

    for (size_t index = 0; index < overrides.size(); ++index) {
        if (static_cast<int>(index) == fractalTypeIndex) continue;
        if (!ApplySingleOverride(overrides[index], ioState, &syncViewHp, allowFractalType, outError)) return false;
    }

    if (syncViewHp) {
        SyncViewHpFromUi(ioState->view);
    }
    if (ioState->params.poly_kind != PolyKind::custom) {
        SetPolyPreset(ioState->params);
    }
    if (IsExplainoFamily(ioState->view.fractal_type)) {
        ExplainoSeedNormalize(ioState->view, ioState->params);
        UpdateExplainoPolynomial(ioState->view, ioState->params, nullptr);
    }
    if (ioState->view.auto_max_iter) {
        ioState->params.max_iter = ComputeAutoMaxIter(ioState->view.log2_zoom, ioState->view.fractal_type);
    }
    return true;
}

bool ApplyCrossfadeSequenceOverrides(const std::vector<FractalProbeOverride>& overrides,
    ProbeState* ioState,
    std::string* outError) {
    if (!ioState) {
        if (outError) *outError = "ApplyCrossfadeSequenceOverrides requires state";
        return false;
    }

    bool syncViewHp = false;
    int fractalTypeIndex = -1;
    for (size_t index = 0; index < overrides.size(); ++index) {
        if (overrides[index].path == "fractal.view.fractal_type") {
            fractalTypeIndex = static_cast<int>(index);
        }
    }

    if (fractalTypeIndex >= 0) {
        const FractalProbeOverride& overrideValue = overrides[static_cast<size_t>(fractalTypeIndex)];
        if (overrideValue.value.kind != FractalProbeScalar::Kind::string) {
            if (outError) *outError = "fractal.view.fractal_type override requires a string enum id";
            return false;
        }
        if (!ApplyFractalTypeOverridePreservingState(overrideValue.value.string_value, ioState, outError)) return false;
    }

    for (size_t index = 0; index < overrides.size(); ++index) {
        if (static_cast<int>(index) == fractalTypeIndex) continue;
        if (!ApplySingleOverride(overrides[index], ioState, &syncViewHp, true, outError)) return false;
    }

    if (syncViewHp) {
        SyncViewHpFromUi(ioState->view);
    }
    if (ioState->params.poly_kind != PolyKind::custom) {
        SetPolyPreset(ioState->params);
    }
    if (IsExplainoFamily(ioState->view.fractal_type)) {
        ExplainoSeedNormalize(ioState->view, ioState->params);
        UpdateExplainoPolynomial(ioState->view, ioState->params, nullptr);
    }
    if (ioState->view.auto_max_iter) {
        ioState->params.max_iter = ComputeAutoMaxIter(ioState->view.log2_zoom, ioState->view.fractal_type);
    }
    return true;
}

bool BuildBaseState(const FractalProbeRequest& request, ProbeState* outState, std::string* outError) {
    if (!outState) {
        if (outError) *outError = "BuildBaseState requires outState";
        return false;
    }
    ProbeState state;
    state.view.fractal_type = FractalType::newton;
    bool dirty = false;
    ResetRuntimeStateForCurrentFractal(state.view, state.params, state.render, state.lens, &dirty);

    if (!request.base_state_load_path.empty()) {
        std::string resolvedPath;
        std::string loadError;
        if (!LoadFindingSelectionIntoRuntime(request.base_state_load_path, &state.view, &state.params, &state.render, &resolvedPath, &loadError)) {
            if (!LoadDiagnosticsStateFile(request.base_state_load_path, &state.view, &state.params, &state.render, &loadError)) {
                if (outError) *outError = loadError;
                return false;
            }
            bool hasExplicitExplainoRoots = false;
            if (!DiagnosticsStateFileHasExplicitExplainoRoots(request.base_state_load_path, &hasExplicitExplainoRoots, &loadError)) {
                if (outError) *outError = loadError;
                return false;
            }
            if (IsExplainoFamily(state.view.fractal_type) && !hasExplicitExplainoRoots) {
                UpdateExplainoPolynomial(state.view, state.params, nullptr);
            }
            SyncViewUiFromHp(state.view);
        }
    }

    if (!ApplyOverridesWithFractalTypeFirst(request.overrides, &state, true, outError)) return false;
    if (!ValidateProbeState(state, outError)) return false;

    *outState = state;
    return true;
}

bool ResolveKnownRootIndex(const KernelParams& params, Cx z, int* outRootIndex) {
    int nRoots = 0;
    if (params.poly_kind == PolyKind::z3_minus_1) nRoots = 3;
    if (params.poly_kind == PolyKind::z4_minus_1) nRoots = 4;
    if (params.explaino_root_count > 0) {
        int best = 0;
        float bestD2 = std::numeric_limits<float>::max();
        for (int index = 0; index < params.explaino_root_count; ++index) {
            const float dx = z.x - params.explaino_roots[index].x;
            const float dy = z.y - params.explaino_roots[index].y;
            const float d2 = dx * dx + dy * dy;
            if (d2 < bestD2) {
                bestD2 = d2;
                best = index;
            }
        }
        if (outRootIndex) *outRootIndex = best;
        return true;
    }
    if (nRoots > 0) {
        float angle = std::atan2(z.y, z.x);
        float t = (angle + 3.14159265358979323846f) / (2.0f * 3.14159265358979323846f);
        int rootIndex = static_cast<int>(std::floor(t * nRoots + 0.5f)) % nRoots;
        if (rootIndex < 0) rootIndex += nRoots;
        if (outRootIndex) *outRootIndex = rootIndex;
        return true;
    }
    return false;
}

FractalProbeSample MarshalFractalEvidenceToProbeSample(
    const FractalSampleEvidence& evidence,
    int sequenceIndex,
    int gridX,
    int gridY,
    const KernelParams& params) {
    const FractalSampleResult& result = evidence.legacy_result;
    FractalProbeSample sample{};
    sample.sequence_index = sequenceIndex;
    sample.grid_x = gridX;
    sample.grid_y = gridY;
    sample.coord_x = evidence.sample_coord.x;
    sample.coord_y = evidence.sample_coord.y;
    sample.iterations = result.iterations;
    sample.termination_kind = result.termination_kind;
    sample.final_z_x = result.final_z_x;
    sample.final_z_y = result.final_z_y;
    sample.final_abs2 =
        static_cast<double>(result.final_z_x) * static_cast<double>(result.final_z_x) +
        static_cast<double>(result.final_z_y) * static_cast<double>(result.final_z_y);
    sample.has_residual = std::isfinite(result.residual);
    sample.residual = result.residual;
    sample.has_far_field_delta = result.has_far_field_delta;
    sample.far_field_delta = result.far_field_delta;

    if (result.termination_kind == TerminationKind::pole) {
        sample.status = FractalProbeSampleStatus::pole;
    } else if (result.termination_kind == TerminationKind::nonfinite ||
        !std::isfinite(result.final_z_x) || !std::isfinite(result.final_z_y)) {
        sample.status = FractalProbeSampleStatus::nonfinite;
    } else if (result.converged || result.termination_kind == TerminationKind::root_converged) {
        sample.status = FractalProbeSampleStatus::converged;
    } else if (result.escaped || result.termination_kind == TerminationKind::escaped_radius ||
        result.termination_kind == TerminationKind::far_field_settled) {
        sample.status = FractalProbeSampleStatus::escaped;
    } else {
        sample.status = FractalProbeSampleStatus::bounded;
    }

    sample.has_root_index = false;
    sample.root_index = -1;
    if (sample.status == FractalProbeSampleStatus::converged) {
        const Cx finalZ{result.final_z_x, result.final_z_y};
        int rootIndex = -1;
        if (ResolveKnownRootIndex(params, finalZ, &rootIndex)) {
            sample.has_root_index = true;
            sample.root_index = rootIndex;
        }
    }
    return sample;
}

std::vector<std::pair<std::string, FractalProbeScalar>> ToAppliedPairs(const std::vector<FractalProbeOverride>& overrides) {
    std::vector<std::pair<std::string, FractalProbeScalar>> result;
    for (const auto& overrideValue : overrides) {
        result.push_back({overrideValue.path, overrideValue.value});
    }
    return result;
}

struct VariantCrossfadeSpec {
    FractalType fractal_type{FractalType::explaino};
    const ExplainoAxisDescriptor* axis{nullptr};
    const char* strength_path{nullptr};
    double default_strength{0.0};
};

bool ResolveVariantCrossfadeSpec(const std::string& variantId,
    VariantCrossfadeSpec* outSpec,
    std::string* outError) {
    FractalType fractalType = FractalType::explaino;
    if (!TryParseFractalTypeId(variantId, &fractalType)) {
        if (outError) *outError = "Unknown variant_crossfade fractal type: " + variantId;
        return false;
    }

    const ExplainoAxisDescriptor* axis = FindExplainoSingleAxisProjectionDescriptor(fractalType);
    if (!axis) {
        if (outError) *outError = "variant_crossfade only supports single-axis Explaino projection selectors";
        return false;
    }

    ProbeState defaultState;
    defaultState.view.fractal_type = fractalType;
    ApplyFractalTypeDefaults(&defaultState.view, &defaultState.params, nullptr);
    const float* defaultStrength = ResolveExplainoAxisValue(defaultState.params, axis->slot);

    VariantCrossfadeSpec spec;
    spec.fractal_type = fractalType;
    spec.axis = axis;
    spec.strength_path = axis->binding_path;
    spec.default_strength = defaultStrength ? static_cast<double>(*defaultStrength) : static_cast<double>(axis->default_value);

    if (outSpec) *outSpec = spec;
    return true;
}

bool ExpandVariantCrossfadeSequence(const FractalProbeSequence& sequence,
    std::vector<std::vector<FractalProbeOverride>>* outVariants,
    std::string* outError) {
    VariantCrossfadeSpec fromSpec;
    VariantCrossfadeSpec toSpec;
    if (!ResolveVariantCrossfadeSpec(sequence.variant_crossfade.from_variant_id, &fromSpec, outError)) return false;
    if (!ResolveVariantCrossfadeSpec(sequence.variant_crossfade.to_variant_id, &toSpec, outError)) return false;

    const int midpoint = sequence.variant_crossfade.steps / 2;
    const double midpointScale = static_cast<double>(midpoint);
    for (int step = 0; step < sequence.variant_crossfade.steps; ++step) {
        std::vector<FractalProbeOverride> variant;
        if (step < midpoint) {
            const double weight = static_cast<double>(midpoint - step) / midpointScale;
            variant.push_back({"fractal.view.fractal_type", FractalProbeScalar::String(FractalTypeId(fromSpec.fractal_type))});
            variant.push_back({fromSpec.strength_path, FractalProbeScalar::Number(fromSpec.default_strength * weight)});
        } else if (step > midpoint) {
            const double weight = static_cast<double>(step - midpoint) / midpointScale;
            variant.push_back({"fractal.view.fractal_type", FractalProbeScalar::String(FractalTypeId(toSpec.fractal_type))});
            variant.push_back({toSpec.strength_path, FractalProbeScalar::Number(toSpec.default_strength * weight)});
        } else {
            variant.push_back({"fractal.view.fractal_type", FractalProbeScalar::String("explaino")});
        }
        outVariants->push_back(std::move(variant));
    }
    return true;
}

bool ExpandCartesianAxes(const std::vector<FractalProbeSequenceAxis>& axes,
    size_t axisIndex,
    std::vector<FractalProbeOverride>* ioCurrent,
    std::vector<std::vector<FractalProbeOverride>>* outVariants) {
    if (axisIndex >= axes.size()) {
        outVariants->push_back(*ioCurrent);
        return true;
    }
    const auto& axis = axes[axisIndex];
    for (const auto& value : axis.values) {
        ioCurrent->push_back({axis.path, value});
        if (!ExpandCartesianAxes(axes, axisIndex + 1, ioCurrent, outVariants)) return false;
        ioCurrent->pop_back();
    }
    return true;
}

bool ExpandSequenceOverrides(const FractalProbeRequest& request,
    std::vector<std::vector<FractalProbeOverride>>* outVariants,
    std::string* outError) {
    if (!outVariants) {
        if (outError) *outError = "ExpandSequenceOverrides requires outVariants";
        return false;
    }
    outVariants->clear();
    if (!request.has_sequence) {
        outVariants->push_back({});
        return true;
    }

    if (request.sequence.mode == FractalProbeSequenceMode::variant_crossfade) {
        return ExpandVariantCrossfadeSequence(request.sequence, outVariants, outError);
    }

    if (request.sequence.zip_paths) {
        const size_t count = request.sequence.axes.front().values.size();
        for (size_t sampleIndex = 0; sampleIndex < count; ++sampleIndex) {
            std::vector<FractalProbeOverride> variant;
            for (const auto& axis : request.sequence.axes) {
                variant.push_back({axis.path, axis.values[sampleIndex]});
            }
            outVariants->push_back(std::move(variant));
        }
        return true;
    }

    std::vector<FractalProbeOverride> current;
    return ExpandCartesianAxes(request.sequence.axes, 0, &current, outVariants);
}

void AccumulateSummary(const FractalProbeSample& sample,
    int* ioCount,
    double* ioIterationSum,
    int* ioEscaped,
    int* ioConverged,
    int* ioNonfinite,
    int* ioPole) {
    if (ioCount) *ioCount += 1;
    if (ioIterationSum) *ioIterationSum += static_cast<double>(sample.iterations);
    if (sample.status == FractalProbeSampleStatus::escaped && ioEscaped) *ioEscaped += 1;
    if (sample.status == FractalProbeSampleStatus::converged && ioConverged) *ioConverged += 1;
    if (sample.status == FractalProbeSampleStatus::nonfinite && ioNonfinite) *ioNonfinite += 1;
    if (sample.status == FractalProbeSampleStatus::pole && ioPole) *ioPole += 1;
}

void FinalizeSummary(int count,
    double iterationSum,
    int escaped,
    int converged,
    int nonfinite,
    int pole,
    double* outMeanIterations,
    double* outEscapeFraction,
    double* outConvergedFraction,
    double* outNonfiniteFraction,
    double* outPoleFraction) {
    const double denom = count > 0 ? static_cast<double>(count) : 1.0;
    if (outMeanIterations) *outMeanIterations = iterationSum / denom;
    if (outEscapeFraction) *outEscapeFraction = static_cast<double>(escaped) / denom;
    if (outConvergedFraction) *outConvergedFraction = static_cast<double>(converged) / denom;
    if (outNonfiniteFraction) *outNonfiniteFraction = static_cast<double>(nonfinite) / denom;
    if (outPoleFraction) *outPoleFraction = static_cast<double>(pole) / denom;
}

std::vector<FractalProbePoint> BuildGridPoints(const FractalProbeRegion& region, std::vector<std::pair<int, int>>* outGridIndices) {
    std::vector<FractalProbePoint> points;
    if (outGridIndices) outGridIndices->clear();
    // Defense-in-depth: cap grid size even if JSON parser already validated.
    constexpr int64_t kMaxGridPoints = 4'000'000;
    const int64_t totalPoints = static_cast<int64_t>(region.grid_width) * static_cast<int64_t>(region.grid_height);
    if (totalPoints > kMaxGridPoints || totalPoints <= 0) return points;
    points.reserve(static_cast<size_t>(totalPoints));
    for (int gy = 0; gy < region.grid_height; ++gy) {
        for (int gx = 0; gx < region.grid_width; ++gx) {
            const double nx = (static_cast<double>(gx) + 0.5) / static_cast<double>(region.grid_width) - 0.5;
            const double ny = (static_cast<double>(gy) + 0.5) / static_cast<double>(region.grid_height) - 0.5;
            points.push_back({region.center_x + nx * region.span_x,
                              region.center_y + ny * region.span_y});
            if (outGridIndices) outGridIndices->push_back({gx, gy});
        }
    }
    return points;
}

std::string CurrentFractalTypeId(const ProbeState& state) {
    BindingContext ctx;
    ProbeState copy = state;
    ctx.view = &copy.view;
    ctx.params = &copy.params;
    ctx.render = &copy.render;
    ctx.lens = &copy.lens;
    return ctx.GetEnumId("fractal.view.fractal_type");
}

using ProbeClock = std::chrono::steady_clock;

double ElapsedMilliseconds(ProbeClock::time_point startedAt) {
    return std::chrono::duration<double, std::milli>(ProbeClock::now() - startedAt).count();
}

struct GenericSamplePreparedRequest {
    std::vector<FractalProbePoint> base_points;
    std::vector<std::pair<int, int>> grid_indices;
    std::vector<std::vector<FractalProbeOverride>> sequence_variants;
    FractalProbeMetricSelection metric_selection;
    bool include_sample_payloads{false};
    double epsilon{1.0e-6};
    double escape_radius{1000.0};
};

enum class GenericSampleBackendKind {
    cpu = 0,
    cuda = 1,
};

const char* GenericSampleBackendId(GenericSampleBackendKind backend) {
    switch (backend) {
    case GenericSampleBackendKind::cpu: return "cpu";
    case GenericSampleBackendKind::cuda: return "cuda";
    }
    return "cpu";
}

GenericSampleBackendKind ResolveDefaultGenericSampleBackend() {
    return GenericSampleBackendKind::cpu;
}

bool ResolveGenericSampleBackend(const FractalProbeRequest& request,
    GenericSampleBackendKind* outBackend,
    std::string* outError) {
    if (!outBackend) {
        if (outError) *outError = "ResolveGenericSampleBackend requires outBackend";
        return false;
    }

    switch (request.execution.backend_preference) {
    case FractalProbeExecutionBackendPreference::default_backend:
        *outBackend = ResolveDefaultGenericSampleBackend();
        return true;
    case FractalProbeExecutionBackendPreference::cpu:
        *outBackend = GenericSampleBackendKind::cpu;
        return true;
    case FractalProbeExecutionBackendPreference::cuda:
        *outBackend = GenericSampleBackendKind::cuda;
        return true;
    }

    if (outError) *outError = "Unsupported generic.sample execution backend preference";
    return false;
}

bool PrepareGenericSampleRequest(const FractalProbeRequest& request,
    GenericSamplePreparedRequest* outPrepared,
    std::string* outError) {
    if (!request.has_function || (request.generic_expression.empty() && !request.has_generic_ast)) {
        if (outError) *outError = "generic.sample requires a 'function' block with 'expression' or 'ast'";
        return false;
    }
    if (request.has_sequence && request.sequence.mode == FractalProbeSequenceMode::variant_crossfade) {
        if (outError) *outError = "generic.sample does not support sequence.mode=variant_crossfade";
        return false;
    }

    GenericSamplePreparedRequest prepared;
    if (request.mode == FractalProbeMode::grid || request.mode == FractalProbeMode::sequence_grid) {
        prepared.base_points = BuildGridPoints(request.region, &prepared.grid_indices);
        if (prepared.base_points.empty()) {
            if (outError) *outError = "Grid too large: grid_width * grid_height must be <= 4000000";
            return false;
        }
    } else {
        prepared.base_points = request.points;
        prepared.grid_indices.assign(prepared.base_points.size(), {-1, -1});
    }

    if (request.has_sequence) {
        if (!ExpandSequenceOverrides(request, &prepared.sequence_variants, outError)) {
            return false;
        }
    }
    if (prepared.sequence_variants.empty()) {
        prepared.sequence_variants.push_back({});
    }

    prepared.metric_selection = BuildFractalProbeMetricSelection(request.metrics);
    prepared.include_sample_payloads = FractalProbeSelectionIncludesAnySampleMetrics(prepared.metric_selection);
    prepared.epsilon = request.generic_epsilon;
    prepared.escape_radius = request.generic_escape_radius;
    *outPrepared = std::move(prepared);
    return true;
}

bool LowerGenericSampleFunctionForStep(
    const FractalProbeRequest& request,
    const std::map<std::string, double>& stepParams,
    GenericFunctionDesc* outDesc,
    std::string* outError) {
    if (!outDesc) {
        if (outError) *outError = "LowerGenericSampleFunctionForStep requires outDesc";
        return false;
    }

    if (request.has_generic_ast) {
        GenericEquationLowerResult lowered = request.generic_iterate_count_param.empty()
            ? LowerGenericEquationAstToDesc(request.generic_ast, stepParams)
            : LowerGenericEquationAstToDesc(request.generic_ast, stepParams, request.generic_iterate_count_param);
        if (!lowered.ok) {
            if (outError) *outError = "AST lower error: " + lowered.error;
            return false;
        }
        *outDesc = lowered.desc;
        return true;
    }

    GFParseResult parsed = ParseGenericFunctionExpression(request.generic_expression, stepParams);
    if (!parsed.ok) {
        if (outError) {
            *outError = "Expression parse error: " + parsed.error +
                " (pos " + std::to_string(parsed.error_pos) + ")";
        }
        return false;
    }
    *outDesc = parsed.desc;
    return true;
}

FractalProbeResponse BuildGenericSampleResponseSkeleton(const FractalProbeRequest& request,
    const std::string& exePath,
    const GenericSamplePreparedRequest& prepared,
    const char* backendUsed) {
    FractalProbeResponse response;
    response.request_id = request.request_id;
    response.function_id = "generic.sample";
    response.ok = true;
    response.runtime.exe_path = exePath;
    response.runtime.fractal_type = "generic";
    response.runtime.device_id = 0;
    response.runtime.backend_used = backendUsed ? backendUsed : "";
    response.metric_selection = prepared.metric_selection;
    response.operator_context = request.operator_context;
    return response;
}

std::map<std::string, double> BuildGenericSampleStepParams(
    const std::map<std::string, double>& baseParams,
    const std::vector<FractalProbeOverride>& overrides) {
    std::map<std::string, double> stepParams = baseParams;
    for (const auto& ov : overrides) {
        const std::string prefix = "function.params.";
        if (ov.path.size() > prefix.size() && ov.path.substr(0, prefix.size()) == prefix) {
            std::string key = ov.path.substr(prefix.size());
            if (ov.value.kind == FractalProbeScalar::Kind::number) {
                stepParams[key] = ov.value.number_value;
            }
        }
    }
    return stepParams;
}

GenericSampleResult RunGenericSampleCpuEvaluation(
    const GenericFunctionDesc& desc,
    GFCpuComplex z,
    double epsilon,
    double escape_radius) {
    const GFNode& rootNode = desc.nodes[desc.root_node];
    if (rootNode.op != GFNodeOp::gf_iterate) {
        return gf_cpu_sample(desc, z, epsilon, escape_radius);
    }

    int maxIter = (rootNode.param_index >= 0 && rootNode.param_index < desc.param_count)
        ? (int)desc.params[rootNode.param_index] : desc.max_iterate;

    int subtree = rootNode.child_left;
    double eps2 = epsilon * epsilon;
    double esc2 = escape_radius * escape_radius;
    int iter = 0;
    bool conv = false;
    bool div = false;

    for (; iter < maxIter; ++iter) {
        GFCpuComplex z_new = gf_cpu_eval_recursive(desc, subtree, z);
        double dx = z_new.x - z.x;
        double dy = z_new.y - z.y;
        double delta2 = dx * dx + dy * dy;
        z = z_new;
        if (delta2 < eps2) { conv = true; iter++; break; }
        double a2 = gf_cpu_abs2(z);
        if (a2 > esc2) { div = true; iter++; break; }
    }

    double mag = std::sqrt(gf_cpu_abs2(z));
    double h = 1e-8 * (std::max)(mag, 1.0);
    GFCpuComplex fz  = gf_cpu_eval_recursive(desc, subtree, z);
    GFCpuComplex fzh = gf_cpu_eval_recursive(desc, subtree, {z.x + h, z.y});

    GenericSampleResult result{};
    result.value_x = z.x;
    result.value_y = z.y;
    result.abs2 = gf_cpu_abs2(z);
    result.derivative_x = (fzh.x - fz.x) / h;
    result.derivative_y = (fzh.y - fz.y) / h;
    result.iterations = iter;
    result.converged = conv;
    result.diverged = div;
    return result;
}

bool RunGenericSampleBatchEvaluation(const GenericFunctionDesc& desc,
    const std::vector<FractalProbePoint>& points,
    GenericSampleBackendKind backend,
    double epsilon,
    double escape_radius,
    std::vector<GenericSampleResult>* outResults,
    std::string* outError) {
    if (!outResults) {
        if (outError) *outError = "RunGenericSampleBatchEvaluation requires outResults";
        return false;
    }

    outResults->clear();
    if (points.empty()) {
        return true;
    }

    if (backend == GenericSampleBackendKind::cpu) {
        outResults->reserve(points.size());
        for (const FractalProbePoint& point : points) {
            outResults->push_back(RunGenericSampleCpuEvaluation(
                desc,
                {point.x, point.y},
                epsilon,
                escape_radius));
        }
        return true;
    }

    std::vector<GFPoint> coords;
    coords.reserve(points.size());
    for (const FractalProbePoint& point : points) {
        coords.push_back({point.x, point.y});
    }

    outResults->assign(points.size(), GenericSampleResult{});
    const char* rawError = nullptr;
    if (!SampleGenericFunction(
            coords.data(),
            static_cast<int>(coords.size()),
            desc,
            epsilon,
            escape_radius,
            outResults->data(),
            &rawError)) {
        if (outError) {
            *outError = rawError ? rawError : "CUDA generic sample execution failed";
        }
        return false;
    }

    return true;
}

FractalProbeSample MarshalGenericSampleToProbeSample(
    const GenericSampleResult& gsr,
    int sequenceIndex,
    int gridX,
    int gridY,
    double coordX,
    double coordY) {
    FractalProbeSample sample;
    sample.sequence_index = sequenceIndex;
    sample.grid_x = gridX;
    sample.grid_y = gridY;
    sample.coord_x = coordX;
    sample.coord_y = coordY;
    sample.iterations = gsr.iterations;
    sample.final_z_x = gsr.value_x;
    sample.final_z_y = gsr.value_y;
    sample.final_abs2 = gsr.abs2;
    sample.derivative_x = gsr.derivative_x;
    sample.derivative_y = gsr.derivative_y;
    sample.termination_kind = gsr.termination_kind;
    sample.has_far_field_delta = gsr.has_far_field_delta;
    sample.far_field_delta = gsr.far_field_delta;

    if (gsr.converged) {
        sample.status = FractalProbeSampleStatus::converged;
    } else if (gsr.diverged) {
        sample.status = FractalProbeSampleStatus::escaped;
    } else if (!std::isfinite(gsr.value_x) || !std::isfinite(gsr.value_y)) {
        sample.status = FractalProbeSampleStatus::nonfinite;
    } else {
        sample.status = FractalProbeSampleStatus::bounded;
    }
    return sample;
}

} // namespace

// --- generic.sample handler ---
// Supports execution-layer CPU/CUDA selection without changing the math surface.

bool RunGenericSampleRequest(const FractalProbeRequest& request,
    const std::string& exePath,
    FractalProbeResponse* outResponse,
    std::string* outError) {

    const ProbeClock::time_point startedAt = ProbeClock::now();

    GenericSamplePreparedRequest prepared;
    if (!PrepareGenericSampleRequest(request, &prepared, outError)) {
        return false;
    }

    GenericSampleBackendKind backend = GenericSampleBackendKind::cpu;
    if (!ResolveGenericSampleBackend(request, &backend, outError)) {
        return false;
    }

    FractalProbeResponse response = BuildGenericSampleResponseSkeleton(
        request,
        exePath,
        prepared,
        GenericSampleBackendId(backend));

    int globalCount = 0;
    double globalIterationSum = 0.0;
    int globalEscaped = 0;
    int globalConverged = 0;
    int globalNonfinite = 0;
    int globalPole = 0;
    double globalAbs2Sum = 0.0;
    int globalFiniteAbs2Count = 0;
    double bestMeanIterations = -1.0;
    int bestSequenceIndex = -1;

    for (size_t sequenceIndex = 0; sequenceIndex < prepared.sequence_variants.size(); ++sequenceIndex) {
        std::map<std::string, double> stepParams = BuildGenericSampleStepParams(
            request.generic_params,
            prepared.sequence_variants[sequenceIndex]);

        GenericFunctionDesc desc{};
        if (!LowerGenericSampleFunctionForStep(request, stepParams, &desc, outError)) {
            return false;
        }

        double eps = prepared.epsilon;
        double esc = prepared.escape_radius;

        std::vector<GenericSampleResult> batchResults;
        std::string backendError;
        if (!RunGenericSampleBatchEvaluation(desc,
                prepared.base_points,
                backend,
                eps,
                esc,
                &batchResults,
                &backendError)) {
            if (outError) {
                *outError = std::string("generic.sample execution backend '") +
                    GenericSampleBackendId(backend) + "' failed: " + backendError;
            }
            return false;
        }

        FractalProbeSequenceResult sequenceResult;
        sequenceResult.sequence_index = static_cast<int>(sequenceIndex);
        sequenceResult.applied = ToAppliedPairs(prepared.sequence_variants[sequenceIndex]);

        int seqCount = 0;
        double seqIterationSum = 0.0;
        int seqEscaped = 0, seqConverged = 0, seqNonfinite = 0, seqPole = 0;
        for (size_t pi = 0; pi < prepared.base_points.size(); ++pi) {
            double cx = prepared.base_points[pi].x;
            double cy = prepared.base_points[pi].y;
            const GenericSampleResult& gsr = batchResults[pi];
            FractalProbeSample sample = MarshalGenericSampleToProbeSample(
                gsr,
                static_cast<int>(sequenceIndex),
                prepared.grid_indices[pi].first,
                prepared.grid_indices[pi].second,
                cx,
                cy);

            if (prepared.include_sample_payloads) {
                response.samples.push_back(sample);
            }
            AccumulateSummary(sample, &seqCount, &seqIterationSum, &seqEscaped, &seqConverged, &seqNonfinite, &seqPole);
            AccumulateSummary(sample, &globalCount, &globalIterationSum, &globalEscaped, &globalConverged, &globalNonfinite, &globalPole);
            if (std::isfinite(gsr.abs2)) {
                globalAbs2Sum += gsr.abs2;
                globalFiniteAbs2Count += 1;
            }
        }

        FinalizeSummary(seqCount, seqIterationSum, seqEscaped, seqConverged, seqNonfinite, seqPole,
            &sequenceResult.mean_iterations, &sequenceResult.escape_fraction,
            &sequenceResult.converged_fraction, &sequenceResult.nonfinite_fraction, &sequenceResult.pole_fraction);
        if (sequenceResult.mean_iterations > bestMeanIterations) {
            bestMeanIterations = sequenceResult.mean_iterations;
            bestSequenceIndex = static_cast<int>(sequenceIndex);
        }
        response.sequence_results.push_back(sequenceResult);
    }

    response.summary.sample_count = globalCount;
    response.cost.sample_count = globalCount;
    FinalizeSummary(globalCount, globalIterationSum, globalEscaped, globalConverged, globalNonfinite, globalPole,
        &response.summary.mean_iterations, &response.summary.escape_fraction,
        &response.summary.converged_fraction, &response.summary.nonfinite_fraction, &response.summary.pole_fraction);
    response.summary.best_sequence_index = bestSequenceIndex < 0 ? 0 : bestSequenceIndex;
    response.summary.mean_abs2 = globalFiniteAbs2Count > 0 ? globalAbs2Sum / static_cast<double>(globalFiniteAbs2Count) : 0.0;
    response.summary.diverged_fraction = globalCount > 0 ? static_cast<double>(globalEscaped) / globalCount : 0.0;
    response.cost.gpu_ms = ElapsedMilliseconds(startedAt);

    *outResponse = std::move(response);
    return true;
}

bool RunFractalProbeRequest(const FractalProbeRequest& request,
    const std::string& exePath,
    FractalProbeResponse* outResponse,
    std::string* outError) {
    if (outError) outError->clear();
    if (!outResponse) {
        if (outError) *outError = "RunFractalProbeRequest requires outResponse";
        return false;
    }

    if (request.function_id.empty()) {
        if (outError) *outError = "function_id is required; valid: " + DescribeRegisteredEngineFunctionIds();
        return false;
    }
    const std::string& resolvedFunctionId = request.function_id;
    const EngineFunctionRegistration* registration = FindEngineFunctionRegistration(resolvedFunctionId);
    if (!registration) {
        if (outError) *outError = "Unknown function_id: " + resolvedFunctionId + "; valid: " + DescribeRegisteredEngineFunctionIds();
        return false;
    }

    if (registration->execution_kind != EngineFunctionExecutionKind::generic_sampler &&
        request.execution.backend_preference != FractalProbeExecutionBackendPreference::default_backend) {
        if (outError) *outError = "execution.backend_preference is only supported for function_id: generic.sample";
        return false;
    }

    if (registration->execution_kind == EngineFunctionExecutionKind::generic_sampler) {
        return RunGenericSampleRequest(request, exePath, outResponse, outError);
    }
    if (registration->execution_kind != EngineFunctionExecutionKind::fractal_sampler) {
        if (outError) *outError = "Unhandled execution_kind " + std::to_string(static_cast<int>(registration->execution_kind)) + " for function_id: " + resolvedFunctionId;
        return false;
    }

    const ProbeClock::time_point startedAt = ProbeClock::now();

    ProbeState baseState;
    if (!BuildBaseState(request, &baseState, outError)) return false;

    std::vector<std::vector<FractalProbeOverride>> sequenceVariants;
    if (!ExpandSequenceOverrides(request, &sequenceVariants, outError)) return false;
    if (sequenceVariants.empty()) sequenceVariants.push_back({});

    std::vector<FractalProbePoint> basePoints;
    std::vector<std::pair<int, int>> gridIndices;
    if (request.mode == FractalProbeMode::grid || request.mode == FractalProbeMode::sequence_grid) {
        basePoints = BuildGridPoints(request.region, &gridIndices);
        if (basePoints.empty()) {
            if (outError) *outError = "Grid too large: grid_width * grid_height must be <= 4000000";
            return false;
        }
    } else {
        basePoints = request.points;
        gridIndices.assign(basePoints.size(), {-1, -1});
    }

    FractalProbeResponse response;
    response.request_id = request.request_id;
    response.function_id = resolvedFunctionId;
    response.ok = true;
    response.runtime.exe_path = exePath;
    response.runtime.device_id = baseState.render.device_id;
    response.runtime.backend_used = "cuda";
    response.metric_selection = BuildFractalProbeMetricSelection(request.metrics);
    response.operator_context = request.operator_context;
    const bool includeSamplePayloads = FractalProbeSelectionIncludesAnySampleMetrics(response.metric_selection);

    int globalCount = 0;
    double globalIterationSum = 0.0;
    int globalEscaped = 0;
    int globalConverged = 0;
    int globalNonfinite = 0;
    int globalPole = 0;
    double bestMeanIterations = -1.0;
    int bestSequenceIndex = -1;
    bool sawFloat32IterationArithmetic = false;
    bool sawFloat64IterationArithmetic = false;

    for (size_t sequenceIndex = 0; sequenceIndex < sequenceVariants.size(); ++sequenceIndex) {
        ProbeState working = baseState;
        if (request.has_sequence && request.sequence.mode == FractalProbeSequenceMode::variant_crossfade) {
            if (!ApplyCrossfadeSequenceOverrides(sequenceVariants[sequenceIndex], &working, outError)) return false;
        } else {
            if (!ApplyOverridesWithFractalTypeFirst(sequenceVariants[sequenceIndex], &working, false, outError)) return false;
        }
        if (!ValidateProbeState(working, outError)) return false;

        FractalProbeSequenceResult sequenceResult;
        sequenceResult.sequence_index = static_cast<int>(sequenceIndex);
        sequenceResult.applied = ToAppliedPairs(sequenceVariants[sequenceIndex]);

        int sequenceCount = 0;
        double sequenceIterationSum = 0.0;
        int sequenceEscaped = 0;
        int sequenceConverged = 0;
        int sequenceNonfinite = 0;
        int sequencePole = 0;

        std::vector<Double2> coordinates;
        coordinates.reserve(basePoints.size());
        for (const FractalProbePoint& point : basePoints) {
            coordinates.push_back({point.x, point.y});
        }
        std::vector<FractalSampleEvidence> evidence(basePoints.size());
        const char* rawSampleError = nullptr;
        if (!SampleFractalEvidencePoints(
                coordinates.data(),
                static_cast<int>(coordinates.size()),
                working.view,
                working.params,
                working.render,
                evidence.data(),
                &rawSampleError)) {
            if (outError) {
                *outError = rawSampleError ? rawSampleError : "CUDA fractal sample execution failed";
            }
            return false;
        }

        for (size_t pointIndex = 0; pointIndex < basePoints.size(); ++pointIndex) {
            const auto& grid = gridIndices[pointIndex];
            const FractalSampleEvidence& pointEvidence = evidence[pointIndex];
            FractalProbeSample sample = MarshalFractalEvidenceToProbeSample(
                pointEvidence,
                static_cast<int>(sequenceIndex),
                grid.first,
                grid.second,
                working.params);
            sawFloat64IterationArithmetic =
                sawFloat64IterationArithmetic || pointEvidence.used_float64_iteration_arithmetic;
            sawFloat32IterationArithmetic =
                sawFloat32IterationArithmetic || !pointEvidence.used_float64_iteration_arithmetic;
            if (includeSamplePayloads) {
                response.samples.push_back(sample);
            }
            AccumulateSummary(sample, &sequenceCount, &sequenceIterationSum, &sequenceEscaped, &sequenceConverged, &sequenceNonfinite, &sequencePole);
            AccumulateSummary(sample, &globalCount, &globalIterationSum, &globalEscaped, &globalConverged, &globalNonfinite, &globalPole);
        }

        FinalizeSummary(sequenceCount,
            sequenceIterationSum,
            sequenceEscaped,
            sequenceConverged,
            sequenceNonfinite,
            sequencePole,
            &sequenceResult.mean_iterations,
            &sequenceResult.escape_fraction,
            &sequenceResult.converged_fraction,
            &sequenceResult.nonfinite_fraction,
            &sequenceResult.pole_fraction);
        if (sequenceResult.mean_iterations > bestMeanIterations) {
            bestMeanIterations = sequenceResult.mean_iterations;
            bestSequenceIndex = static_cast<int>(sequenceIndex);
        }
        response.sequence_results.push_back(sequenceResult);
        response.runtime.fractal_type = CurrentFractalTypeId(working);
    }

    response.summary.sample_count = globalCount;
    response.cost.sample_count = globalCount;
    FinalizeSummary(globalCount,
        globalIterationSum,
        globalEscaped,
        globalConverged,
        globalNonfinite,
        globalPole,
        &response.summary.mean_iterations,
        &response.summary.escape_fraction,
        &response.summary.converged_fraction,
        &response.summary.nonfinite_fraction,
        &response.summary.pole_fraction);
    if (sawFloat32IterationArithmetic && sawFloat64IterationArithmetic) {
        response.runtime.iteration_arithmetic = "mixed";
    } else if (sawFloat64IterationArithmetic) {
        response.runtime.iteration_arithmetic = "float64";
    } else if (sawFloat32IterationArithmetic) {
        response.runtime.iteration_arithmetic = "float32";
    }
    response.summary.best_sequence_index = bestSequenceIndex < 0 ? 0 : bestSequenceIndex;
    response.cost.gpu_ms = ElapsedMilliseconds(startedAt);

    *outResponse = std::move(response);
    return true;
}
