#include "explaino_sidecar_measurement.h"

#include "explaino_seed.h"
#include "fractal_derived_fields.h"
#include "fractal_runtime_validation.h"
#include "view_hp_sync.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>

namespace {

constexpr double kPi = 3.14159265358979323846;
constexpr double kMeasurementSweepFraction = 0.01;
constexpr double kMinFloatSweep = 1.0e-6;
constexpr double kGainEpsilon = 1.0e-9;

struct MeasurementState {
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    LensSettings lens{};
};

BindingContext BindMeasurementState(MeasurementState* state) {
    BindingContext ctx;
    ctx.view = state ? &state->view : nullptr;
    ctx.params = state ? &state->params : nullptr;
    ctx.render = state ? &state->render : nullptr;
    ctx.lens = state ? &state->lens : nullptr;
    return ctx;
}

bool ValidateMeasurementContext(const BindingContext& ctx, std::string* outError) {
    if (!ctx.view || !ctx.params || !ctx.render) {
        if (outError) *outError = "BuildSidecarMeasurementBatch requires view, params, and render bindings";
        return false;
    }
    return true;
}

bool IsSweepableNumericType(const std::string& type) {
    return type == "float" || type == "double" || type == "int";
}

bool RequiresViewHpSync(const std::string& path) {
    return path == "fractal.view.center.x" ||
        path == "fractal.view.center.y" ||
        path == "fractal.view.zoom" ||
        path == "fractal.view.rotation";
}

void RefreshDerivedState(MeasurementState* ioState) {
    if (!ioState) return;
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
}

void EnsureViewHpInitialized(ViewState* ioView) {
    if (!ioView) return;

    const bool hpInvalid = !std::isfinite(ioView->center_hp_x) ||
        !std::isfinite(ioView->center_hp_y) ||
        !std::isfinite(ioView->log2_zoom);
    const bool hpLooksUnset = std::fabs(ioView->center_hp_x - static_cast<double>(ioView->center.x)) < 1.0e-12 &&
        std::fabs(ioView->center_hp_y - static_cast<double>(ioView->center.y)) < 1.0e-12 &&
        std::fabs(ioView->log2_zoom) < 1.0e-12 &&
        std::fabs(ioView->zoom - 1.0f) > 1.0e-6f;

    if (hpInvalid || hpLooksUnset) {
        SyncViewHpFromUi(*ioView);
    }
}

bool ReadNumericValue(const BindingContext& ctx,
    const SidecarParamSurfaceEntry& entry,
    double* outValue,
    std::string* outError) {
    if (!outValue) {
        if (outError) *outError = "ReadNumericValue requires outValue";
        return false;
    }

    if (entry.type == "int") {
        int value = 0;
        if (!ctx.GetIntValue(entry.path, value)) {
            if (outError) *outError = "Unknown numeric binding path for sidecar measurement: " + entry.path;
            return false;
        }
        *outValue = static_cast<double>(value);
        return true;
    }

    if (entry.type == "double") {
        double value = 0.0;
        if (!ctx.GetDoubleValue(entry.path, value)) {
            if (outError) *outError = "Unknown numeric binding path for sidecar measurement: " + entry.path;
            return false;
        }
        *outValue = value;
        return true;
    }

    if (entry.type == "float") {
        float value = 0.0f;
        if (!ctx.GetFloatValue(entry.path, value)) {
            if (outError) *outError = "Unknown numeric binding path for sidecar measurement: " + entry.path;
            return false;
        }
        *outValue = static_cast<double>(value);
        return true;
    }

    if (outError) *outError = "Unsupported measurement param type: " + entry.type;
    return false;
}

double ClampNumericValue(const SidecarParamSurfaceEntry& entry, double value) {
    if (entry.has_min) value = std::max(value, entry.min_value);
    if (entry.has_max) value = std::min(value, entry.max_value);
    if (entry.type == "int") value = std::round(value);
    return value;
}

double ResolveSweepStep(const SidecarParamSurfaceEntry& entry,
    double currentValue,
    std::string* outError) {
    if (entry.type == "int") {
        double step = 0.0;
        if (entry.has_step && std::isfinite(entry.step_value) && entry.step_value > 0.0) {
            step = std::round(entry.step_value);
        } else if (entry.has_declared_span && std::isfinite(entry.declared_span) && entry.declared_span > 0.0) {
            step = std::round(std::max(1.0, entry.declared_span * kMeasurementSweepFraction));
        } else {
            step = 1.0;
        }
        return std::max(1.0, step);
    }

    if (entry.has_step && std::isfinite(entry.step_value) && entry.step_value > 0.0) {
        return entry.step_value;
    }
    if (entry.has_declared_span && std::isfinite(entry.declared_span) && entry.declared_span > 0.0) {
        return std::max(entry.declared_span * kMeasurementSweepFraction, kMinFloatSweep);
    }
    if (std::isfinite(currentValue) && std::fabs(currentValue) > 0.0) {
        return std::max(std::fabs(currentValue) * kMeasurementSweepFraction, kMinFloatSweep);
    }

    if (outError) *outError = "Cannot resolve measurement step for param: " + entry.path;
    return 0.0;
}

bool ApplyNumericValue(const SidecarParamSurfaceEntry& entry,
    double value,
    MeasurementState* ioState,
    std::string* outError) {
    if (!ioState) {
        if (outError) *outError = "ApplyNumericValue requires state";
        return false;
    }

    BindingContext ctx = BindMeasurementState(ioState);
    if (entry.type == "int") {
        int* ptr = nullptr;
        if (!ctx.BindInt(entry.path, &ptr) || !ptr) {
            if (outError) *outError = "Unknown numeric binding path for sidecar measurement: " + entry.path;
            return false;
        }
        *ptr = static_cast<int>(std::lround(value));
        return true;
    }

    if (entry.type == "double") {
        double* ptr = nullptr;
        if (!ctx.BindDouble(entry.path, &ptr) || !ptr) {
            if (outError) *outError = "Unknown numeric binding path for sidecar measurement: " + entry.path;
            return false;
        }
        *ptr = value;
        if (RequiresViewHpSync(entry.path)) SyncViewHpFromUi(ioState->view);
        RefreshDerivedState(ioState);
        return true;
    }

    if (entry.type == "float") {
        float* ptr = nullptr;
        if (!ctx.BindFloat(entry.path, &ptr) || !ptr) {
            if (outError) *outError = "Unknown numeric binding path for sidecar measurement: " + entry.path;
            return false;
        }
        *ptr = static_cast<float>(value);
        if (RequiresViewHpSync(entry.path)) SyncViewHpFromUi(ioState->view);
        RefreshDerivedState(ioState);
        return true;
    }

    if (outError) *outError = "Unsupported measurement param type: " + entry.type;
    return false;
}

std::vector<Double2> BuildMeasurementCoordinates(const ViewState& view) {
    const double zoom = std::max(kMinZoom, SafeZoomFromLog2(view.log2_zoom));
    const double scale = 2.0 / zoom;
    const double angle = static_cast<double>(view.rotation_degrees) * (kPi / 180.0);
    const double cs = std::cos(angle);
    const double sn = std::sin(angle);

    const Double2 normalized[] = {
        {0.0, 0.0},
        {-0.35, 0.0},
        {0.35, 0.0},
        {0.0, -0.35},
        {0.0, 0.35},
    };

    std::vector<Double2> coords;
    coords.reserve(sizeof(normalized) / sizeof(normalized[0]));
    for (const Double2& point : normalized) {
        const double dx = point.x * scale;
        const double dy = point.y * scale;
        const double rotatedX = dx * cs - dy * sn;
        const double rotatedY = dx * sn + dy * cs;
        coords.push_back({view.center_hp_x + rotatedX, view.center_hp_y + rotatedY});
    }
    return coords;
}

SidecarMeasurementAggregate SummarizeResults(const std::vector<FractalSampleResult>& results) {
    SidecarMeasurementAggregate aggregate;
    if (results.empty()) return aggregate;

    for (const FractalSampleResult& result : results) {
        aggregate.mean_iterations += static_cast<double>(result.iterations);
        aggregate.mean_residual += static_cast<double>(result.residual);
        aggregate.converged_fraction += result.converged ? 1.0 : 0.0;
        aggregate.escaped_fraction += result.escaped ? 1.0 : 0.0;
    }

    const double denom = static_cast<double>(results.size());
    aggregate.mean_iterations /= denom;
    aggregate.mean_residual /= denom;
    aggregate.converged_fraction /= denom;
    aggregate.escaped_fraction /= denom;
    return aggregate;
}

double ComputeAggregateDelta(const SidecarMeasurementAggregate& baseline,
    const SidecarMeasurementAggregate& variant) {
    double score = 0.0;
    score += std::fabs(variant.mean_iterations - baseline.mean_iterations);
    score += std::log1p(std::fabs(variant.mean_residual - baseline.mean_residual));
    score += 25.0 * std::fabs(variant.converged_fraction - baseline.converged_fraction);
    score += 25.0 * std::fabs(variant.escaped_fraction - baseline.escaped_fraction);
    return score;
}

double ComputeDiffMagnitude(const SidecarParamSurfaceEntry& entry,
    double currentValue,
    double minusValue,
    double plusValue) {
    if (!entry.has_declared_span || !std::isfinite(entry.declared_span) || entry.declared_span <= 0.0) {
        return 0.0;
    }

    const double minusDelta = std::fabs(currentValue - minusValue);
    const double plusDelta = std::fabs(plusValue - currentValue);
    return std::max(minusDelta, plusDelta) / entry.declared_span;
}

bool SampleAndSummarize(const SidecarMeasurementHost& host,
    const std::vector<Double2>& coords,
    const MeasurementState& state,
    SidecarMeasurementAggregate* outAggregate,
    std::string* outError) {
    if (!outAggregate) {
        if (outError) *outError = "SampleAndSummarize requires outAggregate";
        return false;
    }
    if (!ValidateFractalRuntimeState(state.view, state.params, outError)) return false;

    std::vector<FractalSampleResult> results;
    if (!host.Sample(coords, state.view, state.params, state.render, &results, outError)) {
        return false;
    }
    if (results.size() != coords.size()) {
        if (outError) {
            *outError = "Sidecar measurement host returned " + std::to_string(results.size()) +
                " results for " + std::to_string(coords.size()) + " coords";
        }
        return false;
    }

    *outAggregate = SummarizeResults(results);
    return true;
}

bool MeasurementRowOrder(const SidecarMeasurementRow& left, const SidecarMeasurementRow& right) {
    if (left.information_gain_estimate != right.information_gain_estimate) {
        return left.information_gain_estimate > right.information_gain_estimate;
    }
    if (left.label != right.label) return left.label < right.label;
    return left.path < right.path;
}

} // namespace

bool BuildSidecarMeasurementBatch(
    const SidecarHypothesisSpace& space,
    const BindingContext& ctx,
    const SidecarMeasurementHost& host,
    SidecarMeasurementBatch* outBatch,
    std::string* outError) {
    if (!outBatch) {
        if (outError) *outError = "BuildSidecarMeasurementBatch requires outBatch";
        return false;
    }
    if (!ValidateMeasurementContext(ctx, outError)) {
        *outBatch = {};
        return false;
    }

    MeasurementState baselineState;
    baselineState.view = *ctx.view;
    baselineState.params = *ctx.params;
    baselineState.render = *ctx.render;
    if (ctx.lens) baselineState.lens = *ctx.lens;
    EnsureViewHpInitialized(&baselineState.view);
    RefreshDerivedState(&baselineState);

    const BindingContext baselineCtx = BindMeasurementState(&baselineState);
    const std::vector<Double2> baselineCoords = BuildMeasurementCoordinates(baselineState.view);
    SidecarMeasurementAggregate baselineAggregate;
    if (!SampleAndSummarize(host, baselineCoords, baselineState, &baselineAggregate, outError)) {
        *outBatch = {};
        return false;
    }

    SidecarMeasurementBatch next;
    next.coordinate_count = static_cast<int>(baselineCoords.size());

    double decodeStabilitySum = 0.0;
    int exploredCount = 0;

    for (const SidecarParamSurfaceEntry& entry : space.applicable_parameters) {
        if (!IsSweepableNumericType(entry.type)) continue;

        double currentValue = 0.0;
        if (!ReadNumericValue(baselineCtx, entry, &currentValue, outError)) {
            *outBatch = {};
            return false;
        }

        const double requestedStep = ResolveSweepStep(entry, currentValue, outError);
        if (!(requestedStep > 0.0) || !std::isfinite(requestedStep)) {
            *outBatch = {};
            return false;
        }

        const double minusValue = ClampNumericValue(entry, currentValue - requestedStep);
        const double plusValue = ClampNumericValue(entry, currentValue + requestedStep);

        MeasurementState minusState = baselineState;
        MeasurementState plusState = baselineState;
        if (!ApplyNumericValue(entry, minusValue, &minusState, outError) ||
            !ApplyNumericValue(entry, plusValue, &plusState, outError)) {
            *outBatch = {};
            return false;
        }

        SidecarMeasurementAggregate minusAggregate;
        SidecarMeasurementAggregate plusAggregate;
        if (!SampleAndSummarize(host, BuildMeasurementCoordinates(minusState.view), minusState, &minusAggregate, outError) ||
            !SampleAndSummarize(host, BuildMeasurementCoordinates(plusState.view), plusState, &plusAggregate, outError)) {
            *outBatch = {};
            return false;
        }

        const double minusDelta = ComputeAggregateDelta(baselineAggregate, minusAggregate);
        const double plusDelta = ComputeAggregateDelta(baselineAggregate, plusAggregate);
        const double totalDelta = minusDelta + plusDelta;

        SidecarMeasurementRow row;
        row.label = entry.label;
        row.path = entry.path;
        row.type = entry.type;
        row.current_value = currentValue;
        row.step_value = requestedStep;
        row.baseline = baselineAggregate;
        row.minus_variant = minusAggregate;
        row.plus_variant = plusAggregate;
        row.information_gain_estimate = 0.5 * totalDelta;
        row.decode_stability = totalDelta <= kGainEpsilon
            ? 1.0
            : std::max(0.0, 1.0 - (std::fabs(plusDelta - minusDelta) / totalDelta));

        next.total_information_gain_estimate += row.information_gain_estimate;
        next.total_diff_magnitude += ComputeDiffMagnitude(entry, currentValue, minusValue, plusValue);
        decodeStabilitySum += row.decode_stability;
        if (row.information_gain_estimate > kGainEpsilon) {
            ++exploredCount;
        }
        next.rows.push_back(std::move(row));
    }

    std::sort(next.rows.begin(), next.rows.end(), MeasurementRowOrder);
    if (!next.rows.empty()) {
        const double denom = static_cast<double>(next.rows.size());
        next.mean_information_gain_estimate = next.total_information_gain_estimate / denom;
        next.explored_fraction = static_cast<double>(exploredCount) / denom;
        next.mean_decode_stability = decodeStabilitySum / denom;
    } else {
        next.mean_decode_stability = 1.0;
    }

    *outBatch = std::move(next);
    return true;
}

SidecarOrientationInputs BuildSidecarOrientationInputs(const SidecarMeasurementBatch& batch) {
    SidecarOrientationInputs inputs;
    inputs.slime_energy_delta = batch.total_information_gain_estimate;
    inputs.busy_beaver_metrics = batch.explored_fraction;
    inputs.decode_stability = batch.mean_decode_stability;
    inputs.diff_magnitude = batch.total_diff_magnitude;
    return inputs;
}