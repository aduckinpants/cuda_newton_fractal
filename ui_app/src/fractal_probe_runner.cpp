#include "fractal_probe_runner.h"

#include "explaino_seed.h"
#include "explaino_seed_curve.h"
#include "diagnostics_state_io.h"
#include "finding_state_actions.h"
#include "fractal_derived_fields.h"
#include "fractal_family_rules.h"
#include "runtime_reset.h"
#include "schema_binding.h"
#include "view_hp_sync.h"

#include <algorithm>
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

Cx CxAdd(Cx left, Cx right) { return {left.x + right.x, left.y + right.y}; }
Cx CxSub(Cx left, Cx right) { return {left.x - right.x, left.y - right.y}; }
Cx CxMul(Cx left, Cx right) { return {left.x * right.x - left.y * right.y, left.x * right.y + left.y * right.x}; }
Cx CxScale(Cx value, float scale) { return {value.x * scale, value.y * scale}; }
float CxAbs2(Cx value) { return value.x * value.x + value.y * value.y; }
float CxAbs(Cx value) { return std::sqrt(CxAbs2(value)); }

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

void PolyEvalRealCoeffsDeg4(const float coeffs[5], Cx z, Cx* outP, Cx* outDp) {
    const Cx z2 = CxMul(z, z);
    const Cx z3 = CxMul(z2, z);
    const Cx z4 = CxMul(z2, z2);

    Cx P{coeffs[0], 0.0f};
    P = CxAdd(P, CxScale(z, coeffs[1]));
    P = CxAdd(P, CxScale(z2, coeffs[2]));
    P = CxAdd(P, CxScale(z3, coeffs[3]));
    P = CxAdd(P, CxScale(z4, coeffs[4]));

    Cx dP{coeffs[1], 0.0f};
    dP = CxAdd(dP, CxScale(z, 2.0f * coeffs[2]));
    dP = CxAdd(dP, CxScale(z2, 3.0f * coeffs[3]));
    dP = CxAdd(dP, CxScale(z3, 4.0f * coeffs[4]));

    if (outP) *outP = P;
    if (outDp) *outDp = dP;
}

void PolyEvalRealCoeffsDeg4D2(const float coeffs[5], Cx z, Cx* outP, Cx* outDp, Cx* outD2p) {
    const Cx z2 = CxMul(z, z);
    const Cx z3 = CxMul(z2, z);
    const Cx z4 = CxMul(z2, z2);

    Cx P{coeffs[0], 0.0f};
    P = CxAdd(P, CxScale(z, coeffs[1]));
    P = CxAdd(P, CxScale(z2, coeffs[2]));
    P = CxAdd(P, CxScale(z3, coeffs[3]));
    P = CxAdd(P, CxScale(z4, coeffs[4]));

    Cx dP{coeffs[1], 0.0f};
    dP = CxAdd(dP, CxScale(z, 2.0f * coeffs[2]));
    dP = CxAdd(dP, CxScale(z2, 3.0f * coeffs[3]));
    dP = CxAdd(dP, CxScale(z3, 4.0f * coeffs[4]));

    Cx d2P{2.0f * coeffs[2], 0.0f};
    d2P = CxAdd(d2P, CxScale(z, 6.0f * coeffs[3]));
    d2P = CxAdd(d2P, CxScale(z2, 12.0f * coeffs[4]));

    if (outP) *outP = P;
    if (outDp) *outDp = dP;
    if (outD2p) *outD2p = d2P;
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
    const ViewState& view = state.view;
    const KernelParams& params = state.params;

    if (params.max_iter <= 0) {
        if (outError) *outError = "max_iter must be > 0";
        return false;
    }
    if (!std::isfinite(params.epsilon) || params.epsilon <= 0.0f) {
        if (outError) *outError = "epsilon must be finite and > 0";
        return false;
    }
    if (!IsColoringModeAllowedForFractal(view.fractal_type, params.coloring_mode)) {
        if (outError) *outError = "selected coloring_mode is not valid for fractal_type";
        return false;
    }
    if ((view.fractal_type == FractalType::lambda_map || view.fractal_type == FractalType::explaino_lambda) &&
        (!std::isfinite(params.lambda_real) || !std::isfinite(params.lambda_imag) ||
         std::fabs(params.lambda_real) > 4.0f || std::fabs(params.lambda_imag) > 4.0f)) {
        if (outError) *outError = "lambda_real/lambda_imag must be finite and in [-4,4]";
        return false;
    }
    if (view.fractal_type == FractalType::nova) {
        if (!std::isfinite(params.nova_alpha) || params.nova_alpha <= 0.0f || params.nova_alpha > 5.0f) {
            if (outError) *outError = "nova_alpha must be finite and in (0,5]";
            return false;
        }
    }
    if (IsExplainoFamily(view.fractal_type)) {
        if (!std::isfinite(params.explaino_seed) || !std::isfinite(params.explaino_seed_b)) {
            if (outError) *outError = "explaino_seed and explaino_seed_b must be finite";
            return false;
        }
        if (!std::isfinite(params.explaino_mix) || params.explaino_mix < 0.0f || params.explaino_mix > 1.0f) {
            if (outError) *outError = "explaino_mix must be finite and in [0,1]";
            return false;
        }
        if (!std::isfinite(params.explaino_warp_strength) || params.explaino_warp_strength < 0.0f || params.explaino_warp_strength > 5.0f) {
            if (outError) *outError = "explaino_warp_strength must be finite and in [0,5]";
            return false;
        }
    }
    return true;
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
    const FractalType ft = view.fractal_type;

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

    if (ft == FractalType::nova) {
        Cx cConst = coord;
        z = {0.0f, 0.0f};
        for (; it < maxIter; ++it) {
            Cx P, dP;
            PolyEvalRealCoeffsDeg4(params.poly_coeffs, z, &P, &dP);
            pAbs = CxAbs(P);
            if (pAbs < eps) { status = FractalProbeSampleStatus::converged; break; }
            if (CxAbs2(dP) < 1.0e-20f) break;
            z = CxAdd(CxSub(z, CxScale(CxDiv(P, dP), params.nova_alpha)), cConst);
            if (!IsFiniteCx(z)) { status = FractalProbeSampleStatus::nonfinite; break; }
            if (CxAbs2(z) > 4.0f) { status = FractalProbeSampleStatus::escaped; break; }
        }
        SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, pAbs, true, params, false);
        return true;
    }

    if (ft == FractalType::mandelbrot || ft == FractalType::julia || ft == FractalType::lambda_map) {
        Cx cConst{0.0f, 0.0f};
        if (ft == FractalType::mandelbrot) {
            z = {0.0f, 0.0f};
            cConst = coord;
        } else if (ft == FractalType::julia) {
            z = coord;
            cConst = {-0.7f, 0.27015f};
        } else {
            z = coord;
        }
        const Cx lambdaConst{params.lambda_real, params.lambda_imag};
        for (; it < maxIter; ++it) {
            if (ft == FractalType::lambda_map) {
                const Cx oneMinusZ{1.0f - z.x, -z.y};
                z = CxMul(lambdaConst, CxMul(z, oneMinusZ));
            } else {
                z = CxAdd(CxMul(z, z), cConst);
            }
            if (!IsFiniteCx(z)) { status = FractalProbeSampleStatus::nonfinite; break; }
            if (CxAbs2(z) > 4.0f) { status = FractalProbeSampleStatus::escaped; break; }
        }
        SetFinalSample(outSample, sequenceIndex, gridX, gridY, coordX, coordY, it, status, z, 0.0f, false, params, false);
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

} // namespace

bool RunFractalProbeRequest(const FractalProbeRequest& request,
    const std::string& exePath,
    FractalProbeResponse* outResponse,
    std::string* outError) {
    if (outError) outError->clear();
    if (!outResponse) {
        if (outError) *outError = "RunFractalProbeRequest requires outResponse";
        return false;
    }

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
    response.ok = true;
    response.runtime.exe_path = exePath;
    response.runtime.device_id = baseState.render.device_id;
    response.operator_context = request.operator_context;

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
        if (!ApplyOverridesWithFractalTypeFirst(sequenceVariants[sequenceIndex], &working, false, outError)) return false;
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
            response.samples.push_back(sample);
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

    *outResponse = std::move(response);
    return true;
}