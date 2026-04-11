#include "fractal_probe_runner.h"

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
#include "runtime_reset.h"
#include "schema_binding.h"
#include "view_hp_sync.h"
#include "generic_function_parser.h"
#include "generic_function_types.h"
#include "generic_function_cpu_eval.h"

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

Cx CxAdd(Cx left, Cx right) { return {left.x + right.x, left.y + right.y}; }
Cx CxSub(Cx left, Cx right) { return {left.x - right.x, left.y - right.y}; }
Cx CxMul(Cx left, Cx right) { return {left.x * right.x - left.y * right.y, left.x * right.y + left.y * right.x}; }
Cx CxScale(Cx value, float scale) { return {value.x * scale, value.y * scale}; }
float CxAbs2(Cx value) { return value.x * value.x + value.y * value.y; }
float CxAbs(Cx value) { return std::sqrt(CxAbs2(value)); }
Cx CxAbsComponents(Cx value) { return {std::fabs(value.x), std::fabs(value.y)}; }

Cx CxPowInt(Cx value, int power) {
    Cx result = value;
    for (int index = 1; index < power; ++index) {
        result = CxMul(result, value);
    }
    return result;
}

Cx CxPowRealPrincipal(Cx value, float power) {
    const float radius2 = CxAbs2(value);
    if (radius2 <= 1.0e-30f) return {0.0f, 0.0f};
    const float radius = std::sqrt(radius2);
    const float theta = std::atan2(value.y, value.x);
    const float radiusPower = std::pow(radius, power);
    const float angle = power * theta;
    return {radiusPower * std::cos(angle), radiusPower * std::sin(angle)};
}

Cx UnitRoot(int index, int count) {
    const float angle = (2.0f * kPi) * (static_cast<float>(index) / static_cast<float>(std::max(1, count)));
    return {std::cos(angle), std::sin(angle)};
}

Cx CxDiv(Cx left, Cx right) {
    const float denom = right.x * right.x + right.y * right.y;
    if (denom == 0.0f) return {0.0f, 0.0f};
    return {(left.x * right.x + left.y * right.y) / denom,
            (left.y * right.x - left.x * right.y) / denom};
}

Cx CxRot(Cx value, float angle) {
    const float cs = std::cos(angle);
    const float sn = std::sin(angle);
    return {value.x * cs - value.y * sn, value.x * sn + value.y * cs};
}

float Hash01Host(uint32_t x) {
    return Hash01(x);
}

Cx ExplainoWarpStartHost(Cx coord, double seed, float phase, float strength) {
    const float s = std::max(0.0f, std::min(1.0f, strength));
    if (s <= 0.0f) return coord;

    std::uint64_t bits = 0;
    std::memcpy(&bits, &seed, sizeof(bits));
    const std::uint32_t u = static_cast<std::uint32_t>(bits ^ (bits >> 32));
    const float a0 = Hash01Host(u ^ 0x1234567u);
    const float a1 = Hash01Host(u ^ 0x89abcdefu);

    const float rot = s * (a0 * 2.0f - 1.0f) * 3.1415926f;
    Cx z = CxRot(coord, rot);

    const float freq = 2.0f + 6.0f * a1;
    const float k = 0.10f + 0.35f * a0;
    z.x += s * k * std::sin(z.y * freq + phase);
    z.y += s * k * std::sin(z.x * freq - phase);

    const Cx z2{z.x * z.x - z.y * z.y, 2.0f * z.x * z.y};
    const float push = s * (0.06f + 0.10f * a1);
    return {z.x + z2.x * push, z.y + z2.y * push};
}

bool IsFiniteCx(Cx value) {
    return std::isfinite(value.x) && std::isfinite(value.y);
}

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
        bool* ptr = nullptr;
        if (!ctx.BindBool(overrideValue.path, &ptr) || !ptr) {
            if (outError) *outError = "Unknown bool binding path: " + overrideValue.path;
            return false;
        }
        *ptr = overrideValue.value.bool_value;
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
            if (IsExplainoFamily(state.view.fractal_type)) {
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

bool SnapToKnownRoot(const KernelParams& params, Cx z, Cx* outRoot) {
    int rootIndex = -1;
    if (!ResolveKnownRootIndex(params, z, &rootIndex)) return false;
    if (params.explaino_root_count > 0) {
        if (outRoot) *outRoot = {params.explaino_roots[rootIndex].x, params.explaino_roots[rootIndex].y};
        return true;
    }

    int nRoots = 0;
    if (params.poly_kind == PolyKind::z3_minus_1) nRoots = 3;
    if (params.poly_kind == PolyKind::z4_minus_1) nRoots = 4;
    if (nRoots <= 0) return false;
    if (outRoot) *outRoot = UnitRoot(rootIndex, nRoots);
    return true;
}

void SetFinalSample(FractalProbeSample* outSample,
    int sequenceIndex,
    int gridX,
    int gridY,
    double coordX,
    double coordY,
    int iterations,
    FractalProbeSampleStatus status,
    Cx z,
    float residual,
    bool hasResidual,
    const KernelParams& params,
    bool tryRootIndex) {
    if (!outSample) return;
    outSample->sequence_index = sequenceIndex;
    outSample->grid_x = gridX;
    outSample->grid_y = gridY;
    outSample->coord_x = coordX;
    outSample->coord_y = coordY;
    outSample->iterations = iterations;
    outSample->status = status;
    outSample->final_z_x = z.x;
    outSample->final_z_y = z.y;
    outSample->final_abs2 = CxAbs2(z);
    outSample->has_residual = hasResidual;
    outSample->residual = residual;
    outSample->has_root_index = false;
    outSample->root_index = -1;
    if (tryRootIndex && (status == FractalProbeSampleStatus::converged)) {
        int rootIndex = -1;
        if (ResolveKnownRootIndex(params, z, &rootIndex)) {
            outSample->has_root_index = true;
            outSample->root_index = rootIndex;
        }
    }
}

bool SamplePoint(const ProbeState& state,
    int sequenceIndex,
    int gridX,
    int gridY,
    double coordX,
    double coordY,
    FractalProbeSample* outSample,
    std::string* outError) {
    const ViewState& view = state.view;
    const KernelParams& params = state.params;
    const int maxIter = std::max(1, params.max_iter);
    const float eps = std::max(1.0e-12f, params.epsilon);
    const Cx coord{static_cast<float>(coordX), static_cast<float>(coordY)};

    int it = 0;
    float pAbs = 0.0f;
    Cx z{0.0f, 0.0f};
    FractalProbeSampleStatus status = FractalProbeSampleStatus::bounded;
    FractalType ft = view.fractal_type;
    const bool isExplainoComposedVariant =
        ft == FractalType::explaino_ripple ||
        ft == FractalType::explaino_splice ||
        ft == FractalType::explaino_vortex ||
        ft == FractalType::explaino_tension;
    const bool hasExplainoComposedPerturbation =
        params.ripple_amplitude != 0.0f ||
        params.splice_offset != 0.0f ||
        params.vortex_strength != 0.0f ||
        params.tension_strength != 0.0f;
    if (isExplainoComposedVariant && !hasExplainoComposedPerturbation) {
        ft = FractalType::explaino;
    }

    auto explainoSeed = [&]() {
        const double combinedSeed = params.explaino_seed + static_cast<double>(view.explaino_seed_drift);
        return LogisticAreaUToSeed(combinedSeed);
    };

    if (ft == FractalType::newton) {
        z = coord;
        for (; it < maxIter; ++it) {
            Cx P, dP;
            PolyEvalRealCoeffsDeg4(params.poly_coeffs, z, &P, &dP);
            pAbs = CxAbs(P);
            if (pAbs < eps) { status = FractalProbeSampleStatus::converged; break; }
            if (CxAbs2(dP) < 1.0e-20f) break;
            z = CxSub(z, CxDiv(P, dP));
            if (!IsFiniteCx(z)) { status = FractalProbeSampleStatus::nonfinite; break; }
        }
        SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, pAbs, true, params, true);
        return true;
    }

    if (ft == FractalType::explaino || ft == FractalType::explaino_dual || ft == FractalType::explaino_mult) {
        z = ExplainoWarpStartHost(coord, explainoSeed(), view.explaino_phase, params.explaino_warp_strength);
        for (; it < maxIter; ++it) {
            Cx P, dP;
            PolyEvalRealCoeffsDeg4(params.poly_coeffs, z, &P, &dP);
            pAbs = CxAbs(P);
            if (pAbs < eps) { status = FractalProbeSampleStatus::converged; break; }
            if (CxAbs2(dP) < 1.0e-20f) break;
            z = CxSub(z, CxScale(CxDiv(P, dP), params.explaino_damping));
            if (!IsFiniteCx(z)) { status = FractalProbeSampleStatus::nonfinite; break; }
        }
        SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, pAbs, true, params, true);
        return true;
    }

    if (ft == FractalType::explaino_fp) {
        z = ExplainoWarpStartHost(coord, explainoSeed(), view.explaino_phase, params.explaino_warp_strength);
        for (; it < maxIter; ++it) {
            Cx P, dP;
            PolyEvalRealCoeffsDeg4(params.poly_coeffs, z, &P, &dP);
            pAbs = CxAbs(P);
            if (pAbs < eps) {
                status = FractalProbeSampleStatus::converged;
                break;
            }

            const Cx step = CxAbs2(dP) < 1.0e-20f ? P : CxDiv(P, dP);
            const float stepMag = std::sqrt(std::max(0.0f, CxAbs2(step)));
            const float damp = params.explaino_damping / (1.0f + stepMag);
            z = CxSub(z, CxScale(step, damp));

            const float r2 = CxAbs2(z);
            if (r2 > 16.0f) {
                const float scale = 4.0f / std::max(1.0e-12f, std::sqrt(r2));
                z = CxScale(z, scale);
            }

            if (!IsFiniteCx(z)) {
                status = FractalProbeSampleStatus::nonfinite;
                break;
            }
        }

        if (status != FractalProbeSampleStatus::converged) {
            Cx snappedRoot{0.0f, 0.0f};
            if (SnapToKnownRoot(params, z, &snappedRoot)) {
                z = snappedRoot;
                status = FractalProbeSampleStatus::converged;
            }
        }

        SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, pAbs, true, params, true);
        return true;
    }

    if (ft == FractalType::explaino_y) {
        z = ExplainoWarpStartHost(coord, explainoSeed(), view.explaino_phase, params.explaino_warp_strength);
        Cx zPrev = z;
        float bestP = std::numeric_limits<float>::max();
        int bestIt = 0;
        Cx bestZ = z;

        for (; it < maxIter; ++it) {
            const float localPhase = view.explaino_phase + 0.07f * static_cast<float>(it);
            const Cx zWarped = ExplainoWarpStartHost(z, explainoSeed(), localPhase, params.explaino_warp_strength * 0.30f);

            Cx P, dP;
            PolyEvalRealCoeffsDeg4(params.poly_coeffs, zWarped, &P, &dP);

            pAbs = CxAbs(P);
            if (pAbs < bestP) {
                bestP = pAbs;
                bestIt = it;
                bestZ = zWarped;
            }
            if (pAbs < eps) {
                z = zWarped;
                status = FractalProbeSampleStatus::converged;
                break;
            }

            const Cx step = CxAbs2(dP) < 1.0e-20f ? P : CxDiv(P, dP);
            const float stepMag = std::sqrt(std::max(0.0f, CxAbs2(step)));
            const float damp = 0.90f * params.explaino_damping / (1.0f + stepMag);
            const Cx newtonWarped = CxSub(zWarped, CxScale(step, damp));

            const float mix = 0.78f;
            Cx zNext = CxAdd(CxScale(z, 1.0f - mix), CxScale(newtonWarped, mix));
            const Cx velocity = CxSub(z, zPrev);
            zNext = CxAdd(zNext, CxScale(velocity, 0.10f));
            zNext = CxAdd(zNext, CxScale(coord, 0.045f));

            zPrev = z;
            z = zNext;

            const float r2 = CxAbs2(z);
            const float scale = 4.0f / std::sqrt(1.0f + r2 * 0.0625f);
            z = CxScale(z, scale);

            if (!IsFiniteCx(z)) {
                status = FractalProbeSampleStatus::nonfinite;
                break;
            }
        }

        if (status != FractalProbeSampleStatus::converged && status != FractalProbeSampleStatus::nonfinite) {
            z = bestZ;
            it = bestIt;
            pAbs = bestP;
            status = FractalProbeSampleStatus::converged;
        }

        SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, pAbs, true, params, true);
        return true;
    }

    if (ft == FractalType::halley || ft == FractalType::explaino_halley) {
        z = ft == FractalType::halley
            ? coord
            : ExplainoWarpStartHost(coord, explainoSeed(), view.explaino_phase, params.explaino_warp_strength);
        const float damping = ft == FractalType::halley ? 1.0f : params.explaino_damping;

        for (; it < maxIter; ++it) {
            Cx P, dP, d2P;
            PolyEvalRealCoeffsDeg4D2(params.poly_coeffs, z, &P, &dP, &d2P);

            pAbs = CxAbs(P);
            if (pAbs < eps) {
                status = FractalProbeSampleStatus::converged;
                break;
            }

            const Cx denominator = CxSub(CxScale(CxMul(dP, dP), 2.0f), CxMul(P, d2P));
            if (CxAbs2(denominator) < 1.0e-20f) break;

            const Cx numerator = CxScale(CxMul(P, dP), 2.0f);
            z = CxSub(z, CxScale(CxDiv(numerator, denominator), damping));
            if (!IsFiniteCx(z)) {
                status = FractalProbeSampleStatus::nonfinite;
                break;
            }
        }

        SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, pAbs, true, params, true);
        return true;
    }

    if (ft == FractalType::nova || ft == FractalType::explaino_nova) {
        Cx cConst = coord;
        z = {0.0f, 0.0f};
        for (; it < maxIter; ++it) {
            Cx P, dP;
            PolyEvalRealCoeffsDeg4(params.poly_coeffs, z, &P, &dP);
            pAbs = CxAbs(P);
            if (pAbs < eps) { status = FractalProbeSampleStatus::converged; break; }
            if (CxAbs2(dP) >= 1.0e-20f) {
                z = CxSub(z, CxScale(CxDiv(P, dP), params.nova_alpha));
            }
            z = CxAdd(z, cConst);
            if (!IsFiniteCx(z)) { status = FractalProbeSampleStatus::nonfinite; break; }
            if (CxAbs2(z) > 4.0f) { status = FractalProbeSampleStatus::escaped; break; }
        }
        SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, pAbs, true, params, false);
        return true;
    }

    if (UsesSharedEscapeTimeDirectFormula(ft)) {
        EscapeTimeDirectState<Cx> state = InitEscapeTimeDirectState(ft, coord);
        const float powerFloat = params.multibrot_power_float;
        const int powerInt = params.multibrot_power;
        const Cx lambdaConst{params.lambda_real, params.lambda_imag};
        const Cx phoenixP{params.phoenix_p_real, params.phoenix_p_imag};

        for (; it < maxIter; ++it) {
            StepEscapeTimeDirectState(ft, powerFloat, powerInt, lambdaConst, phoenixP, &state);
            z = state.z;
            if (!IsFiniteCx(state.z) || !IsFiniteCx(state.z_prev)) {
                status = FractalProbeSampleStatus::nonfinite;
                break;
            }
            if (CxAbs2(state.z) > DirectEscapeTimeRadiusSquared<float>()) {
                status = FractalProbeSampleStatus::escaped;
                break;
            }
        }

        SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, 0.0f, false, params, false);
        return true;
    }

    if (UsesSpecializedEscapeTimeFormula(ft)) {
        z = {0.0f, 0.0f};
        const McMullenPresetConfig mcmullenConfig = ResolveMcMullenPresetConfig(params.mcmullen_preset);
        z = coord;

        for (; it < maxIter; ++it) {
            if (ft == FractalType::mcmullen) {
                if (StepMcMullenEscapeState(mcmullenConfig, &z) == SpecializedEscapeStepResult::pole) {
                    status = FractalProbeSampleStatus::pole;
                    break;
                }
            } else {
                StepCollatzEscapeState(&z);
            }

            if (!IsFiniteCx(z)) {
                status = FractalProbeSampleStatus::nonfinite;
                break;
            }

            if (CxAbs2(z) > SpecializedEscapeRadiusSquared()) {
                status = FractalProbeSampleStatus::escaped;
                break;
            }
        }

        SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, 0.0f, false, params, false);
        return true;
    }

    if (ft == FractalType::explaino_phoenix) {
        z = ExplainoWarpStartHost(coord, explainoSeed(), view.explaino_phase, params.explaino_warp_strength);
        Cx zPrev = z;
        const Cx pConst{params.phoenix_p_real, params.phoenix_p_imag};
        float bestPF = 1.0e30f;
        int bestIt_phx = 0;

        for (; it < maxIter; ++it) {
            Cx P, dP;
            PolyEvalRealCoeffsDeg4(params.poly_coeffs, z, &P, &dP);

            pAbs = CxAbs(P);
            if (pAbs < bestPF) { bestPF = pAbs; bestIt_phx = it; }
            if (pAbs < eps) {
                status = FractalProbeSampleStatus::converged;
                break;
            }
            const float dAbs2 = CxAbs2(dP);
            const Cx step = (dAbs2 < 1.0e-20f) ? P : CxDiv(P, dP);
            const float stepMag = std::sqrt(std::max(0.0f, CxAbs2(step)));
            const float damp = params.explaino_damping / (1.0f + stepMag);
            const Cx zNext = CxAdd(CxSub(z, CxScale(step, damp)), CxMul(pConst, zPrev));
            zPrev = z;
            z = zNext;
            const float r2 = CxAbs2(z);
            if (r2 > 16.0f) {
                const float r = std::sqrt(r2);
                const float s = 4.0f / std::max(1e-12f, r);
                z = CxScale(z, s);
            }
            if (!IsFiniteCx(z)) {
                status = FractalProbeSampleStatus::nonfinite;
                break;
            }
        }

        if (status != FractalProbeSampleStatus::converged && status != FractalProbeSampleStatus::nonfinite) {
            it = bestIt_phx;
        }
        SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, pAbs, true, params, true);
        return true;
    }

    if (ft == FractalType::explaino_joy) {
        z = ExplainoWarpStartHost(coord, explainoSeed(), view.explaino_phase, params.explaino_warp_strength);
        Cx zPrev = z;
        const Cx pConst{params.phoenix_p_real, params.phoenix_p_imag};
        const float joyCoupling = params.joy_coupling;
        const float oneMinusGamma = 1.0f - joyCoupling;
        float bestPF = 1.0e30f;
        int bestIt_joy = 0;

        for (; it < maxIter; ++it) {
            Cx P, dP, d2P;
            PolyEvalRealCoeffsDeg4D2(params.poly_coeffs, z, &P, &dP, &d2P);

            pAbs = CxAbs(P);
            if (pAbs < bestPF) { bestPF = pAbs; bestIt_joy = it; }
            if (pAbs < eps) {
                status = FractalProbeSampleStatus::converged;
                break;
            }
            const float dAbs2 = CxAbs2(dP);
            const Cx newtonStep = (dAbs2 < 1.0e-20f) ? P : CxDiv(P, dP);
            Cx joyStep = {0.0f, 0.0f};
            if (CxAbs2(d2P) > 1.0e-20f) {
                joyStep = CxDiv(dP, d2P);
            }
            const Cx combinedStep = CxAdd(
                CxScale(newtonStep, oneMinusGamma),
                CxScale(joyStep, joyCoupling));
            const float stepMag = std::sqrt(std::max(0.0f, CxAbs2(combinedStep)));
            const float damp = params.explaino_damping / (1.0f + stepMag);
            const Cx zNext = CxAdd(
                CxSub(z, CxScale(combinedStep, damp)),
                CxMul(pConst, zPrev));
            zPrev = z;
            z = zNext;
            const float r2 = CxAbs2(z);
            if (r2 > 16.0f) {
                const float r = std::sqrt(r2);
                const float s = 4.0f / std::max(1e-12f, r);
                z = CxScale(z, s);
            }
            if (!IsFiniteCx(z)) {
                status = FractalProbeSampleStatus::nonfinite;
                break;
            }
        }

        if (status != FractalProbeSampleStatus::converged && status != FractalProbeSampleStatus::nonfinite) {
            it = bestIt_joy;
        }
        SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, pAbs, true, params, true);
        return true;
    }

    if (ft == FractalType::explaino_fold) {
        z = ExplainoWarpStartHost(coord, explainoSeed(), view.explaino_phase, params.explaino_warp_strength);
        Cx zPrev = z;
        const Cx pConst{params.phoenix_p_real, params.phoenix_p_imag};
        const float foldAlpha = params.fold_coupling;
        const float oneMinusAlpha = 1.0f - foldAlpha;
        float bestPF = 1.0e30f;
        int bestIt_fold = 0;

        for (; it < maxIter; ++it) {
            Cx P, dP;
            PolyEvalRealCoeffsDeg4(params.poly_coeffs, z, &P, &dP);

            pAbs = CxAbs(P);
            if (pAbs < bestPF) { bestPF = pAbs; bestIt_fold = it; }
            if (pAbs < eps) {
                status = FractalProbeSampleStatus::converged;
                break;
            }
            const float dAbs2 = CxAbs2(dP);
            const Cx newtonStep = (dAbs2 < 1.0e-20f) ? P : CxDiv(P, dP);
            const Cx foldedStep = {std::fabs(newtonStep.x), std::fabs(newtonStep.y)};
            const Cx combinedStep = CxAdd(
                CxScale(newtonStep, oneMinusAlpha),
                CxScale(foldedStep, foldAlpha));
            const float stepMag = std::sqrt(std::max(0.0f, CxAbs2(combinedStep)));
            const float damp = params.explaino_damping / (1.0f + stepMag);
            const Cx zNext = CxAdd(
                CxSub(z, CxScale(combinedStep, damp)),
                CxMul(pConst, zPrev));
            zPrev = z;
            z = zNext;
            const float r2 = CxAbs2(z);
            if (r2 > 16.0f) {
                const float r = std::sqrt(r2);
                const float s = 4.0f / std::max(1e-12f, r);
                z = CxScale(z, s);
            }
            if (!IsFiniteCx(z)) {
                status = FractalProbeSampleStatus::nonfinite;
                break;
            }
        }

        if (status != FractalProbeSampleStatus::converged && status != FractalProbeSampleStatus::nonfinite) {
            it = bestIt_fold;
        }
        SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, pAbs, true, params, true);
        return true;
    }

    if (ft == FractalType::explaino_bell) {
        z = ExplainoWarpStartHost(coord, explainoSeed(), view.explaino_phase, params.explaino_warp_strength);
        Cx zPrev = z;
        const Cx pConst{params.phoenix_p_real, params.phoenix_p_imag};
        const float bellBeta = params.bell_coupling;
        float bestPF = 1.0e30f;
        int bestIt_bell = 0;

        for (; it < maxIter; ++it) {
            Cx P, dP;
            PolyEvalRealCoeffsDeg4(params.poly_coeffs, z, &P, &dP);

            pAbs = CxAbs(P);
            if (pAbs < bestPF) { bestPF = pAbs; bestIt_bell = it; }
            if (pAbs < eps) {
                status = FractalProbeSampleStatus::converged;
                break;
            }
            const float dAbs2 = CxAbs2(dP);
            const Cx newtonStep = (dAbs2 < 1.0e-20f) ? P : CxDiv(P, dP);
            // Decompose step by P-phase direction
            const float pMag = std::max(1e-20f, pAbs);
            const Cx pHat = {P.x / pMag, P.y / pMag};
            const float dotPar = newtonStep.x * pHat.x + newtonStep.y * pHat.y;
            const Cx sParallel = {dotPar * pHat.x, dotPar * pHat.y};
            const Cx combinedStep = {newtonStep.x - bellBeta * sParallel.x,
                                     newtonStep.y - bellBeta * sParallel.y};
            const float stepMag = std::sqrt(std::max(0.0f, CxAbs2(combinedStep)));
            const float damp = params.explaino_damping / (1.0f + stepMag);
            const Cx zNext = CxAdd(
                CxSub(z, CxScale(combinedStep, damp)),
                CxMul(pConst, zPrev));
            zPrev = z;
            z = zNext;
            const float r2 = CxAbs2(z);
            if (r2 > 16.0f) {
                const float r = std::sqrt(r2);
                const float s = 4.0f / std::max(1e-12f, r);
                z = CxScale(z, s);
            }
            if (!IsFiniteCx(z)) {
                status = FractalProbeSampleStatus::nonfinite;
                break;
            }
        }

        if (status != FractalProbeSampleStatus::converged && status != FractalProbeSampleStatus::nonfinite) {
            it = bestIt_bell;
        }
        SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, pAbs, true, params, true);
        return true;
    }

    if (ft == FractalType::explaino_ripple ||
        ft == FractalType::explaino_splice ||
        ft == FractalType::explaino_vortex ||
        ft == FractalType::explaino_tension) {
        z = ExplainoWarpStartHost(coord, explainoSeed(), view.explaino_phase, params.explaino_warp_strength);
        Cx zPrev = z;
        const Cx pConst{params.phoenix_p_real, params.phoenix_p_imag};
        const float rippleA = params.ripple_amplitude;
        const float vortexStrength = params.vortex_strength;
        const float tensionStrength = params.tension_strength;
        const int nRootsForPull = params.explaino_root_count;
        const bool useSplice = params.splice_offset != 0.0f;
        float bestPF = 1.0e30f;
        int bestIt_composed = 0;
        const float kTwoPI = 6.2831853071795864f;
        const float kRipplePeriod = 8.0f;

        for (; it < maxIter; ++it) {
            const float* activeCoeffs = (useSplice && (it % 2) != 0) ? params.poly_coeffs_b : params.poly_coeffs;
            Cx P, dP;
            PolyEvalRealCoeffsDeg4(activeCoeffs, z, &P, &dP);

            pAbs = CxAbs(P);
            if (pAbs < bestPF) { bestPF = pAbs; bestIt_composed = it; }
            if (useSplice) {
                Cx PA, dPA;
                PolyEvalRealCoeffsDeg4(params.poly_coeffs, z, &PA, &dPA);
                const float paAbs = CxAbs(PA);
                if (paAbs < eps) {
                    pAbs = paAbs;
                    status = FractalProbeSampleStatus::converged;
                    break;
                }
            } else if (pAbs < eps) {
                status = FractalProbeSampleStatus::converged;
                break;
            }

            const float dAbs2 = CxAbs2(dP);
            const Cx newtonStep = (dAbs2 < 1.0e-20f) ? P : CxDiv(P, dP);
            const float stepMag = std::sqrt(std::max(0.0f, CxAbs2(newtonStep)));

            Cx composedStep = newtonStep;
            if (vortexStrength > 0.0f && stepMag > 1.0e-20f) {
                const float theta = std::atan2(newtonStep.y, newtonStep.x);
                const float angle = vortexStrength * theta;
                const float cosA = std::cos(angle);
                const float sinA = std::sin(angle);
                composedStep = {newtonStep.x * cosA - newtonStep.y * sinA,
                                newtonStep.x * sinA + newtonStep.y * cosA};
            }

            Cx kick = {0.0f, 0.0f};
            if (rippleA > 0.0f && stepMag > 1.0e-20f) {
                const Cx nHat = {-newtonStep.y / stepMag, newtonStep.x / stepMag};
                const float dpArg = std::atan2(dP.y, dP.x);
                const float wave = rippleA * std::sin(kTwoPI * (float)it / kRipplePeriod + dpArg);
                kick = {nHat.x * wave, nHat.y * wave};
            }

            Cx pull = {0.0f, 0.0f};
            if (tensionStrength > 0.0f && nRootsForPull >= 2) {
                int idxNearest = 0;
                float best1 = 1.0e30f;
                for (int r = 0; r < nRootsForPull; ++r) {
                    const float dx = z.x - params.explaino_roots[r].x;
                    const float dy = z.y - params.explaino_roots[r].y;
                    const float d2 = dx * dx + dy * dy;
                    if (d2 < best1) { best1 = d2; idxNearest = r; }
                }
                float best2 = 1.0e30f;
                int idx2 = (idxNearest == 0) ? 1 : 0;
                for (int r = 0; r < nRootsForPull; ++r) {
                    if (r == idxNearest) continue;
                    const float dx = z.x - params.explaino_roots[r].x;
                    const float dy = z.y - params.explaino_roots[r].y;
                    const float d2 = dx * dx + dy * dy;
                    if (d2 < best2) { best2 = d2; idx2 = r; }
                }
                const float fx = params.explaino_roots[idx2].x - z.x;
                const float fy = params.explaino_roots[idx2].y - z.y;
                const float dist2 = fx * fx + fy * fy;
                if (dist2 > 1.0e-20f) {
                    pull = {tensionStrength * fx / dist2, tensionStrength * fy / dist2};
                }
            }

            const float damp = params.explaino_damping / (1.0f + stepMag);
            const Cx zNext = CxAdd(
                CxAdd(CxAdd(CxSub(z, CxScale(composedStep, damp)), kick), pull),
                CxMul(pConst, zPrev));
            zPrev = z;
            z = zNext;
            const float r2 = CxAbs2(z);
            if (r2 > 16.0f) {
                const float r = std::sqrt(r2);
                const float s = 4.0f / std::max(1e-12f, r);
                z = CxScale(z, s);
            }
            if (!IsFiniteCx(z)) {
                status = FractalProbeSampleStatus::nonfinite;
                break;
            }
        }

        if (status != FractalProbeSampleStatus::converged && status != FractalProbeSampleStatus::nonfinite) {
            it = bestIt_composed;
        }
        SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, pAbs, true, params, true);
        return true;
    }

    if (ft == FractalType::explaino_ripple) {
        z = ExplainoWarpStartHost(coord, explainoSeed(), view.explaino_phase, params.explaino_warp_strength);
        Cx zPrev = z;
        const Cx pConst{params.phoenix_p_real, params.phoenix_p_imag};
        const float rippleA = params.ripple_amplitude;
        float bestPF = 1.0e30f;
        int bestIt_ripple = 0;
        const float kTwoPI = 6.2831853071795864f;
        const float kRipplePeriod = 8.0f;

        for (; it < maxIter; ++it) {
            Cx P, dP;
            PolyEvalRealCoeffsDeg4(params.poly_coeffs, z, &P, &dP);

            pAbs = CxAbs(P);
            if (pAbs < bestPF) { bestPF = pAbs; bestIt_ripple = it; }
            if (pAbs < eps) {
                status = FractalProbeSampleStatus::converged;
                break;
            }
            const float dAbs2 = CxAbs2(dP);
            const Cx newtonStep = (dAbs2 < 1.0e-20f) ? P : CxDiv(P, dP);
            const float stepMag = std::sqrt(std::max(0.0f, CxAbs2(newtonStep)));
            // Standing-wave kick perpendicular to step direction
            Cx kick = {0.0f, 0.0f};
            if (rippleA > 0.0f && stepMag > 1e-20f) {
                const Cx nHat = {-newtonStep.y / stepMag, newtonStep.x / stepMag};
                const float dpArg = std::atan2(dP.y, dP.x);
                const float wave = rippleA * std::sin(kTwoPI * (float)it / kRipplePeriod + dpArg);
                kick = {nHat.x * wave, nHat.y * wave};
            }
            const float damp = params.explaino_damping / (1.0f + stepMag);
            const Cx zNext = CxAdd(
                CxAdd(CxSub(z, CxScale(newtonStep, damp)), kick),
                CxMul(pConst, zPrev));
            zPrev = z;
            z = zNext;
            const float r2 = CxAbs2(z);
            if (r2 > 16.0f) {
                const float r = std::sqrt(r2);
                const float s = 4.0f / std::max(1e-12f, r);
                z = CxScale(z, s);
            }
            if (!IsFiniteCx(z)) {
                status = FractalProbeSampleStatus::nonfinite;
                break;
            }
        }

        if (status != FractalProbeSampleStatus::converged && status != FractalProbeSampleStatus::nonfinite) {
            it = bestIt_ripple;
        }
        SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, pAbs, true, params, true);
        return true;
    }

    if (ft == FractalType::explaino_splice) {
        z = ExplainoWarpStartHost(coord, explainoSeed(), view.explaino_phase, params.explaino_warp_strength);
        Cx zPrev = z;
        const Cx pConst{params.phoenix_p_real, params.phoenix_p_imag};
        float bestPF = 1.0e30f;
        int bestIt_splice = 0;

        for (; it < maxIter; ++it) {
            // Alternate polynomial: even iterations use P_A, odd use P_B
            const float* activeCoeffs = (it % 2 == 0) ? params.poly_coeffs : params.poly_coeffs_b;
            Cx P, dP;
            PolyEvalRealCoeffsDeg4(activeCoeffs, z, &P, &dP);

            pAbs = CxAbs(P);
            if (pAbs < bestPF) { bestPF = pAbs; bestIt_splice = it; }
            // Convergence check against primary polynomial P_A
            {
                Cx PA, dPA;
                PolyEvalRealCoeffsDeg4(params.poly_coeffs, z, &PA, &dPA);
                const float paAbs = CxAbs(PA);
                if (paAbs < eps) {
                    pAbs = paAbs;
                    status = FractalProbeSampleStatus::converged;
                    break;
                }
            }
            const float dAbs2 = CxAbs2(dP);
            const Cx step = (dAbs2 < 1.0e-20f) ? P : CxDiv(P, dP);
            const float stepMag = std::sqrt(std::max(0.0f, CxAbs2(step)));
            const float damp = params.explaino_damping / (1.0f + stepMag);
            const Cx zNext = CxAdd(
                CxSub(z, CxScale(step, damp)),
                CxMul(pConst, zPrev));
            zPrev = z;
            z = zNext;
            const float r2 = CxAbs2(z);
            if (r2 > 16.0f) {
                const float r = std::sqrt(r2);
                const float s = 4.0f / std::max(1e-12f, r);
                z = CxScale(z, s);
            }
            if (!IsFiniteCx(z)) {
                status = FractalProbeSampleStatus::nonfinite;
                break;
            }
        }

        if (status != FractalProbeSampleStatus::converged && status != FractalProbeSampleStatus::nonfinite) {
            it = bestIt_splice;
        }
        SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, pAbs, true, params, true);
        return true;
    }

    if (ft == FractalType::explaino_vortex) {
        z = ExplainoWarpStartHost(coord, explainoSeed(), view.explaino_phase, params.explaino_warp_strength);
        Cx zPrev = z;
        const Cx pConst{params.phoenix_p_real, params.phoenix_p_imag};
        const float V = params.vortex_strength;
        float bestPF = 1.0e30f;
        int bestIt_vortex = 0;

        for (; it < maxIter; ++it) {
            Cx P, dP;
            PolyEvalRealCoeffsDeg4(params.poly_coeffs, z, &P, &dP);

            pAbs = CxAbs(P);
            if (pAbs < bestPF) { bestPF = pAbs; bestIt_vortex = it; }
            if (pAbs < eps) {
                status = FractalProbeSampleStatus::converged;
                break;
            }
            const float dAbs2 = CxAbs2(dP);
            const Cx newtonStep = (dAbs2 < 1.0e-20f) ? P : CxDiv(P, dP);
            const float stepMag = std::sqrt(std::max(0.0f, CxAbs2(newtonStep)));
            // Self-referential rotation by V * arg(step)
            Cx rotStep = newtonStep;
            if (V > 0.0f && stepMag > 1e-20f) {
                const float theta = std::atan2(newtonStep.y, newtonStep.x);
                const float angle = V * theta;
                const float cosA = std::cos(angle);
                const float sinA = std::sin(angle);
                rotStep = {newtonStep.x * cosA - newtonStep.y * sinA,
                           newtonStep.x * sinA + newtonStep.y * cosA};
            }
            const float damp = params.explaino_damping / (1.0f + stepMag);
            const Cx zNext = CxAdd(
                CxSub(z, CxScale(rotStep, damp)),
                CxMul(pConst, zPrev));
            zPrev = z;
            z = zNext;
            const float r2 = CxAbs2(z);
            if (r2 > 16.0f) {
                const float r = std::sqrt(r2);
                const float s = 4.0f / std::max(1e-12f, r);
                z = CxScale(z, s);
            }
            if (!IsFiniteCx(z)) {
                status = FractalProbeSampleStatus::nonfinite;
                break;
            }
        }

        if (status != FractalProbeSampleStatus::converged && status != FractalProbeSampleStatus::nonfinite) {
            it = bestIt_vortex;
        }
        SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, pAbs, true, params, true);
        return true;
    }

    if (ft == FractalType::explaino_tension) {
        z = ExplainoWarpStartHost(coord, explainoSeed(), view.explaino_phase, params.explaino_warp_strength);
        Cx zPrev = z;
        const Cx pConst{params.phoenix_p_real, params.phoenix_p_imag};
        const float T = params.tension_strength;
        const int nRootsForPull = params.explaino_root_count;
        float bestPF = 1.0e30f;
        int bestIt_tension = 0;

        for (; it < maxIter; ++it) {
            Cx P, dP;
            PolyEvalRealCoeffsDeg4(params.poly_coeffs, z, &P, &dP);

            pAbs = CxAbs(P);
            if (pAbs < bestPF) { bestPF = pAbs; bestIt_tension = it; }
            if (pAbs < eps) {
                status = FractalProbeSampleStatus::converged;
                break;
            }
            const float dAbs2 = CxAbs2(dP);
            const Cx newtonStep = (dAbs2 < 1.0e-20f) ? P : CxDiv(P, dP);
            const float stepMag = std::sqrt(std::max(0.0f, CxAbs2(newtonStep)));
            const float damp = params.explaino_damping / (1.0f + stepMag);
            // Compute pull toward second-closest root
            Cx pull = {0.0f, 0.0f};
            if (T > 0.0f && nRootsForPull >= 2) {
                int idxNearest = 0;
                float best1 = 1e30f;
                for (int r = 0; r < nRootsForPull; ++r) {
                    float dx = z.x - params.explaino_roots[r].x;
                    float dy = z.y - params.explaino_roots[r].y;
                    float d2 = dx*dx + dy*dy;
                    if (d2 < best1) { best1 = d2; idxNearest = r; }
                }
                float best2 = 1e30f;
                int idx2 = (idxNearest == 0) ? 1 : 0;
                for (int r = 0; r < nRootsForPull; ++r) {
                    if (r == idxNearest) continue;
                    float dx = z.x - params.explaino_roots[r].x;
                    float dy = z.y - params.explaino_roots[r].y;
                    float d2 = dx*dx + dy*dy;
                    if (d2 < best2) { best2 = d2; idx2 = r; }
                }
                float fx = params.explaino_roots[idx2].x - z.x;
                float fy = params.explaino_roots[idx2].y - z.y;
                float dist2 = fx*fx + fy*fy;
                if (dist2 > 1e-20f) {
                    pull = {T * fx / dist2, T * fy / dist2};
                }
            }
            const Cx zNext = CxAdd(
                CxAdd(CxSub(z, CxScale(newtonStep, damp)), pull),
                CxMul(pConst, zPrev));
            zPrev = z;
            z = zNext;
            const float r2 = CxAbs2(z);
            if (r2 > 16.0f) {
                const float r = std::sqrt(r2);
                const float s = 4.0f / std::max(1e-12f, r);
                z = CxScale(z, s);
            }
            if (!IsFiniteCx(z)) {
                status = FractalProbeSampleStatus::nonfinite;
                break;
            }
        }

        if (status != FractalProbeSampleStatus::converged && status != FractalProbeSampleStatus::nonfinite) {
            it = bestIt_tension;
        }
        SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, pAbs, true, params, true);
        return true;
    }

    if (ft == FractalType::explaino_transcendental) {
        z = ExplainoWarpStartHost(coord, explainoSeed(), view.explaino_phase, params.explaino_warp_strength);

        for (; it < maxIter; ++it) {
            Cx F, dF;
            if (params.transcendental_func == TranscendentalFunc::f_sin) {
                const float sx = std::sin(z.x);
                const float cx = std::cos(z.x);
                const float shy = std::sinh(z.y);
                const float chy = std::cosh(z.y);
                F = {sx * chy, cx * shy};
                dF = {cx * chy, -sx * shy};
            } else if (params.transcendental_func == TranscendentalFunc::f_exp_minus_1) {
                const float ex = std::exp(z.x);
                const float cy = std::cos(z.y);
                const float sy = std::sin(z.y);
                F = {ex * cy - 1.0f, ex * sy};
                dF = {ex * cy, ex * sy};
            } else {
                const float chx = std::cosh(z.x);
                const float shx = std::sinh(z.x);
                const float cy = std::cos(z.y);
                const float sy = std::sin(z.y);
                F = {chx * cy, shx * sy};
                dF = {shx * cy, chx * sy};
            }

            pAbs = CxAbs(F);
            if (pAbs < eps) {
                status = FractalProbeSampleStatus::converged;
                break;
            }
            if (CxAbs2(dF) < 1.0e-20f) break;

            z = CxSub(z, CxScale(CxDiv(F, dF), params.explaino_damping));
            if (!IsFiniteCx(z)) {
                status = FractalProbeSampleStatus::nonfinite;
                break;
            }
        }

        SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, pAbs, true, params, true);
        return true;
    }

    if (ft == FractalType::explaino_inertial) {
        z = ExplainoWarpStartHost(coord, explainoSeed(), view.explaino_phase, params.explaino_warp_strength);
        Cx zPrev = z;
        Cx zPrev2 = z;

        for (; it < maxIter; ++it) {
            Cx P, dP;
            PolyEvalRealCoeffsDeg4(params.poly_coeffs, z, &P, &dP);

            pAbs = CxAbs(P);
            if (pAbs < eps) {
                status = FractalProbeSampleStatus::converged;
                break;
            }
            if (CxAbs2(dP) < 1.0e-20f) break;

            const Cx momentum = CxSub(zPrev, zPrev2);
            const Cx zNext = CxAdd(CxSub(z, CxScale(CxDiv(P, dP), params.explaino_damping)), CxScale(momentum, params.momentum_beta));
            zPrev2 = zPrev;
            zPrev = z;
            z = zNext;
            if (!IsFiniteCx(z)) {
                status = FractalProbeSampleStatus::nonfinite;
                break;
            }
        }

        SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, pAbs, true, params, true);
        return true;
    }

    if (ft == FractalType::explaino_collatz) {
        z = ExplainoWarpStartHost(coord, explainoSeed(), view.explaino_phase, params.explaino_warp_strength);

        for (; it < maxIter; ++it) {
            const ExplainoCollatzStepResult stepResult = StepExplainoCollatzNewton(params.explaino_damping, eps, &z, &pAbs);
            if (stepResult == ExplainoCollatzStepResult::converged) {
                status = FractalProbeSampleStatus::converged;
                break;
            }
            if (stepResult == ExplainoCollatzStepResult::degenerate) break;
            if (!IsFiniteCx(z)) {
                status = FractalProbeSampleStatus::nonfinite;
                break;
            }
        }

        SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, pAbs, true, params, true);
        return true;
    }

    if (ft == FractalType::explaino_julia || ft == FractalType::explaino_lambda || ft == FractalType::explaino_rational_escape || ft == FractalType::explaino_rational) {
        z = ExplainoWarpStartHost(coord, explainoSeed(), view.explaino_phase, params.explaino_warp_strength);
        if (ft == FractalType::explaino_julia) {
            const Cx cJ = (params.explaino_root_count > 0)
                ? Cx{params.explaino_roots[0].x, params.explaino_roots[0].y}
                : Cx{-0.7f, 0.27015f};
            for (; it < maxIter; ++it) {
                z = CxAdd(CxMul(z, z), cJ);
                if (!IsFiniteCx(z)) { status = FractalProbeSampleStatus::nonfinite; break; }
                if (CxAbs2(z) > 4.0f) { status = FractalProbeSampleStatus::escaped; break; }
            }
            SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, 0.0f, false, params, false);
            return true;
        }

        if (ft == FractalType::explaino_lambda) {
            const Cx lambdaConst{params.lambda_real, params.lambda_imag};
            for (; it < maxIter; ++it) {
                const Cx oneMinusZ{1.0f - z.x, -z.y};
                z = CxMul(lambdaConst, CxMul(z, oneMinusZ));
                if (!IsFiniteCx(z)) { status = FractalProbeSampleStatus::nonfinite; break; }
                if (CxAbs2(z) > 4.0f) { status = FractalProbeSampleStatus::escaped; break; }
            }
            SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, 0.0f, false, params, false);
            return true;
        }

        if (ft == FractalType::explaino_rational_escape) {
            for (; it < maxIter; ++it) {
                const float zAbs2 = CxAbs2(z);
                if (zAbs2 < 1.0e-20f) { status = FractalProbeSampleStatus::pole; break; }
                Cx P, dP;
                PolyEvalRealCoeffsDeg4(params.poly_coeffs, z, &P, &dP);
                const Cx z2 = CxMul(z, z);
                const Cx z3 = CxMul(z2, z);
                z = CxDiv(P, z3);
                if (!IsFiniteCx(z)) { status = FractalProbeSampleStatus::nonfinite; break; }
                if (CxAbs2(z) > 10000.0f) { status = FractalProbeSampleStatus::escaped; break; }
            }
            SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, 0.0f, false, params, false);
            return true;
        }

        for (; it < maxIter; ++it) {
            const float zAbs2 = CxAbs2(z);
            if (zAbs2 < 1.0e-20f) { status = FractalProbeSampleStatus::pole; break; }
            Cx P, dP;
            PolyEvalRealCoeffsDeg4(params.poly_coeffs, z, &P, &dP);
            const Cx zInv = CxDiv({1.0f, 0.0f}, z);
            const Cx F = CxAdd(P, CxScale(zInv, params.explaino_cluster_radius));
            const Cx zInv2 = CxMul(zInv, zInv);
            const Cx dF = CxSub(dP, CxScale(zInv2, params.explaino_cluster_radius));
            pAbs = CxAbs(F);
            if (pAbs < eps) { status = FractalProbeSampleStatus::converged; break; }
            if (CxAbs2(dF) < 1.0e-20f) break;
            z = CxSub(z, CxScale(CxDiv(F, dF), params.explaino_damping));
            if (!IsFiniteCx(z)) { status = FractalProbeSampleStatus::nonfinite; break; }
        }
        SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, pAbs, true, params, true);
        return true;
    }

    if (outError) *outError = "Probe sampling is not yet implemented for fractal_type " + std::to_string(static_cast<int>(ft));
    return false;
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

    VariantCrossfadeSpec spec;
    spec.fractal_type = fractalType;
    switch (fractalType) {
    case FractalType::explaino_ripple:
        spec.strength_path = "fractal.params.ripple_amplitude";
        break;
    case FractalType::explaino_splice:
        spec.strength_path = "fractal.params.splice_offset";
        break;
    case FractalType::explaino_vortex:
        spec.strength_path = "fractal.params.vortex_strength";
        break;
    case FractalType::explaino_tension:
        spec.strength_path = "fractal.params.tension_strength";
        break;
    default:
        if (outError) *outError = "variant_crossfade only supports explaino_ripple, explaino_splice, explaino_vortex, and explaino_tension";
        return false;
    }

    ProbeState defaultState;
    defaultState.view.fractal_type = fractalType;
    ApplyFractalTypeDefaults(&defaultState.view, &defaultState.params, nullptr);
    switch (fractalType) {
    case FractalType::explaino_ripple:
        spec.default_strength = defaultState.params.ripple_amplitude;
        break;
    case FractalType::explaino_splice:
        spec.default_strength = defaultState.params.splice_offset;
        break;
    case FractalType::explaino_vortex:
        spec.default_strength = defaultState.params.vortex_strength;
        break;
    case FractalType::explaino_tension:
        spec.default_strength = defaultState.params.tension_strength;
        break;
    default:
        break;
    }

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
    points.reserve(static_cast<size_t>(region.grid_width) * static_cast<size_t>(region.grid_height));
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

const FractalType kSupportedProbeFractalTypes[] = {
    FractalType::newton,
    FractalType::nova,
    FractalType::mandelbrot,
    FractalType::julia,
    FractalType::burning_ship,
    FractalType::multibrot,
    FractalType::phoenix,
    FractalType::explaino,
    FractalType::explaino_y,
    FractalType::explaino_fp,
    FractalType::explaino_nova,
    FractalType::explaino_halley,
    FractalType::explaino_dual,
    FractalType::explaino_mult,
    FractalType::explaino_phoenix,
    FractalType::explaino_joy,
    FractalType::explaino_fold,
    FractalType::explaino_bell,
    FractalType::explaino_ripple,
    FractalType::explaino_splice,
    FractalType::explaino_vortex,
    FractalType::explaino_tension,
    FractalType::explaino_transcendental,
    FractalType::explaino_inertial,
    FractalType::explaino_julia,
    FractalType::explaino_rational,
    FractalType::multicorn,
    FractalType::halley,
    FractalType::collatz,
    FractalType::explaino_collatz,
    FractalType::mcmullen,
    FractalType::lambda_map,
    FractalType::explaino_lambda,
    FractalType::explaino_rational_escape,
    FractalType::spider,
    FractalType::celtic_mandelbrot,
    FractalType::perpendicular_burning_ship,
};

bool IsSupportedProbeFractalType(FractalType fractalType) {
    for (FractalType candidate : kSupportedProbeFractalTypes) {
        if (fractalType == candidate) return true;
    }
    return false;
}

double ElapsedMilliseconds(ProbeClock::time_point startedAt) {
    return std::chrono::duration<double, std::milli>(ProbeClock::now() - startedAt).count();
}

} // namespace

bool IsProbeSamplingImplementedForFractalTypeId(const std::string& fractalTypeId) {
    FractalType fractalType = FractalType::newton;
    return TryParseFractalTypeId(fractalTypeId, &fractalType) && IsSupportedProbeFractalType(fractalType);
}

// --- generic.sample handler ---
// Uses the CPU expression evaluator. No CUDA dependency.

bool RunGenericSampleRequest(const FractalProbeRequest& request,
    const std::string& exePath,
    FractalProbeResponse* outResponse,
    std::string* outError) {

    const ProbeClock::time_point startedAt = ProbeClock::now();

    if (!request.has_function || request.generic_expression.empty()) {
        if (outError) *outError = "generic.sample requires a 'function' block with 'expression'";
        return false;
    }
    if (request.has_sequence && request.sequence.mode == FractalProbeSequenceMode::variant_crossfade) {
        if (outError) *outError = "generic.sample does not support sequence.mode=variant_crossfade";
        return false;
    }

    // Generate coordinates (reuse same grid/point logic).
    std::vector<FractalProbePoint> basePoints;
    std::vector<std::pair<int, int>> gridIndices;
    if (request.mode == FractalProbeMode::grid || request.mode == FractalProbeMode::sequence_grid) {
        basePoints = BuildGridPoints(request.region, &gridIndices);
    } else {
        basePoints = request.points;
        gridIndices.assign(basePoints.size(), {-1, -1});
    }

    // Expand sequence overrides.
    std::vector<std::vector<FractalProbeOverride>> sequenceVariants;
    if (request.has_sequence) {
        if (!ExpandSequenceOverrides(request, &sequenceVariants, outError)) return false;
    }
    if (sequenceVariants.empty()) sequenceVariants.push_back({});

    FractalProbeResponse response;
    response.request_id = request.request_id;
    response.function_id = "generic.sample";
    response.ok = true;
    response.runtime.exe_path = exePath;
    response.runtime.fractal_type = "generic";
    response.runtime.device_id = 0;
    response.metric_selection = BuildFractalProbeMetricSelection(request.metrics);
    response.operator_context = request.operator_context;
    const bool includeSamplePayloads = FractalProbeSelectionIncludesAnySampleMetrics(response.metric_selection);

    int globalCount = 0;
    double globalIterationSum = 0.0;
    int globalEscaped = 0;
    int globalConverged = 0;
    int globalNonfinite = 0;
    int globalPole = 0;
    double globalAbs2Sum = 0.0;
    double bestMeanIterations = -1.0;
    int bestSequenceIndex = -1;

    for (size_t sequenceIndex = 0; sequenceIndex < sequenceVariants.size(); ++sequenceIndex) {
        // Build params for this sequence step: start from base params, apply overrides.
        std::map<std::string, double> stepParams = request.generic_params;
        for (const auto& ov : sequenceVariants[sequenceIndex]) {
            // Expect paths like "function.params.c_real" → key = "c_real"
            const std::string prefix = "function.params.";
            if (ov.path.size() > prefix.size() && ov.path.substr(0, prefix.size()) == prefix) {
                std::string key = ov.path.substr(prefix.size());
                if (ov.value.kind == FractalProbeScalar::Kind::number) {
                    stepParams[key] = ov.value.number_value;
                }
            }
        }

        // Parse expression with this step's params.
        GFParseResult pr = ParseGenericFunctionExpression(request.generic_expression, stepParams);
        if (!pr.ok) {
            if (outError) *outError = "Expression parse error: " + pr.error + " (pos " + std::to_string(pr.error_pos) + ")";
            return false;
        }

        double eps = request.generic_epsilon;
        double esc = request.generic_escape_radius;

        FractalProbeSequenceResult sequenceResult;
        sequenceResult.sequence_index = static_cast<int>(sequenceIndex);
        sequenceResult.applied = ToAppliedPairs(sequenceVariants[sequenceIndex]);

        int seqCount = 0;
        double seqIterationSum = 0.0;
        int seqEscaped = 0, seqConverged = 0, seqNonfinite = 0, seqPole = 0;
        double seqAbs2Sum = 0.0;

        for (size_t pi = 0; pi < basePoints.size(); ++pi) {
            double cx = basePoints[pi].x;
            double cy = basePoints[pi].y;
            GFCpuComplex z = {cx, cy};

            // Check if root is iterate — use iteration loop with convergence tracking.
            const GFNode& rootNode = pr.desc.nodes[pr.desc.root_node];
            GenericSampleResult gsr;

            if (rootNode.op == GFNodeOp::gf_iterate) {
                int maxIter = (rootNode.param_index >= 0 && rootNode.param_index < pr.desc.param_count)
                    ? (int)pr.desc.params[rootNode.param_index] : pr.desc.max_iterate;
                if (maxIter <= 0) maxIter = pr.desc.max_iterate;
                if (maxIter <= 0) maxIter = 1;
                if (maxIter > 10000) maxIter = 10000;

                int subtree = rootNode.child_left;
                double eps2 = eps * eps;
                double esc2 = esc * esc;
                int iter = 0;
                bool conv = false, div = false;

                for (; iter < maxIter; ++iter) {
                    GFCpuComplex z_new = gf_cpu_eval_recursive(pr.desc, subtree, z);
                    double dx = z_new.x - z.x;
                    double dy = z_new.y - z.y;
                    double delta2 = dx * dx + dy * dy;
                    z = z_new;
                    if (delta2 < eps2) { conv = true; iter++; break; }
                    double a2 = gf_cpu_abs2(z);
                    if (a2 > esc2) { div = true; iter++; break; }
                }

                // Derivative of one-step function at final z.
                double mag = std::sqrt(gf_cpu_abs2(z));
                double h = 1e-8 * (std::max)(mag, 1.0);
                GFCpuComplex fz  = gf_cpu_eval_recursive(pr.desc, subtree, z);
                GFCpuComplex fzh = gf_cpu_eval_recursive(pr.desc, subtree, {z.x + h, z.y});

                gsr.value_x = z.x;
                gsr.value_y = z.y;
                gsr.abs2 = gf_cpu_abs2(z);
                gsr.derivative_x = (fzh.x - fz.x) / h;
                gsr.derivative_y = (fzh.y - fz.y) / h;
                gsr.iterations = iter;
                gsr.converged = conv;
                gsr.diverged = div;
            } else {
                // Direct evaluation (non-iterate).
                gsr = gf_cpu_sample(pr.desc, z, eps, esc);
            }

            // Map GenericSampleResult to FractalProbeSample.
            FractalProbeSample sample;
            sample.sequence_index = static_cast<int>(sequenceIndex);
            sample.grid_x = gridIndices[pi].first;
            sample.grid_y = gridIndices[pi].second;
            sample.coord_x = cx;
            sample.coord_y = cy;
            sample.iterations = gsr.iterations;
            sample.final_z_x = gsr.value_x;
            sample.final_z_y = gsr.value_y;
            sample.final_abs2 = gsr.abs2;
            sample.derivative_x = gsr.derivative_x;
            sample.derivative_y = gsr.derivative_y;

            if (gsr.converged) sample.status = FractalProbeSampleStatus::converged;
            else if (gsr.diverged) sample.status = FractalProbeSampleStatus::escaped;
            else if (!std::isfinite(gsr.value_x) || !std::isfinite(gsr.value_y))
                sample.status = FractalProbeSampleStatus::nonfinite;
            else sample.status = FractalProbeSampleStatus::bounded;

            if (includeSamplePayloads) {
                response.samples.push_back(sample);
            }
            AccumulateSummary(sample, &seqCount, &seqIterationSum, &seqEscaped, &seqConverged, &seqNonfinite, &seqPole);
            AccumulateSummary(sample, &globalCount, &globalIterationSum, &globalEscaped, &globalConverged, &globalNonfinite, &globalPole);
            seqAbs2Sum += gsr.abs2;
            globalAbs2Sum += gsr.abs2;
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
    response.summary.mean_abs2 = globalCount > 0 ? globalAbs2Sum / globalCount : 0.0;
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

    // Validate function_id.
    const std::string resolvedFunctionId = request.function_id.empty() ? "fractal.sample" : request.function_id;
    if (resolvedFunctionId != "fractal.sample" && resolvedFunctionId != "generic.sample") {
        if (outError) *outError = "Unknown function_id: " + resolvedFunctionId + "; valid: fractal.sample, generic.sample";
        return false;
    }

    // Dispatch generic.sample through the expression evaluator path.
    if (resolvedFunctionId == "generic.sample") {
        return RunGenericSampleRequest(request, exePath, outResponse, outError);
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

        for (size_t pointIndex = 0; pointIndex < basePoints.size(); ++pointIndex) {
            FractalProbeSample sample;
            const auto& point = basePoints[pointIndex];
            const auto& grid = gridIndices[pointIndex];
            if (!SamplePoint(working,
                    static_cast<int>(sequenceIndex),
                    grid.first,
                    grid.second,
                    point.x,
                    point.y,
                    &sample,
                    outError)) {
                return false;
            }
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
    response.summary.best_sequence_index = bestSequenceIndex < 0 ? 0 : bestSequenceIndex;
    response.cost.gpu_ms = ElapsedMilliseconds(startedAt);

    *outResponse = std::move(response);
    return true;
}