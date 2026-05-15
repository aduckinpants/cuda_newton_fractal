#include "explaino_variant_benchmark.h"

#include "enum_id_utils.h"
#include "fractal_derived_fields.h"
#include "fractal_family_rules.h"
#include "sample_tier_resolver.h"
#include "view_hp_sync.h"

#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>

namespace {

constexpr ExplainoVariantBenchmarkCase kBenchmarkCases[] = {
    {"explaino_baseline", FractalType::explaino, "none", 0.0f, false},
    {"explaino_ripple_zero", FractalType::explaino_ripple, "ripple_amplitude", 0.0f, true},
    {"explaino_ripple_default", FractalType::explaino_ripple, "ripple_amplitude", 0.15f, false},
    {"explaino_splice_zero", FractalType::explaino_splice, "splice_offset", 0.0f, true},
    {"explaino_splice_default", FractalType::explaino_splice, "splice_offset", 0.5f, false},
    {"explaino_vortex_zero", FractalType::explaino_vortex, "vortex_strength", 0.0f, true},
    {"explaino_vortex_default", FractalType::explaino_vortex, "vortex_strength", 0.3f, false},
    {"explaino_tension_zero", FractalType::explaino_tension, "tension_strength", 0.0f, true},
    {"explaino_tension_default", FractalType::explaino_tension, "tension_strength", 0.02f, false},
};

const char* NumericBackendIdLocal(NumericBackend backend) {
    switch (backend) {
    case NumericBackend::float32: return "float32";
    case NumericBackend::float64: return "float64";
    }
    return "unknown";
}

const char* IterationStrategyIdLocal(IterationStrategy strategy) {
    switch (strategy) {
    case IterationStrategy::direct: return "direct";
    }
    return "unknown";
}

bool ApplyUniqueParamOverride(const ExplainoVariantBenchmarkCase& benchmarkCase,
    KernelParams* ioParams,
    const char** outError) {
    if (!ioParams) {
        if (outError) *outError = "benchmark params pointer is null";
        return false;
    }

    if (benchmarkCase.fractal_type == FractalType::explaino) {
        if (std::strcmp(benchmarkCase.param_name, "none") != 0) {
            if (outError) *outError = "baseline explaino benchmark row must use param_name=none";
            return false;
        }
        return true;
    }

    const ExplainoAxisDescriptor* axis = FindExplainoSingleAxisProjectionDescriptor(benchmarkCase.fractal_type);
    if (!axis) {
        if (outError) *outError = "unsupported fractal type for explaino benchmark case";
        return false;
    }
    if (std::strcmp(benchmarkCase.param_name, axis->axis_id) != 0) {
        if (outError) *outError = "benchmark row uses unexpected param_name for its canonical Explaino axis";
        return false;
    }

    float* axisValue = ResolveExplainoAxisValue(*ioParams, axis->slot);
    if (!axisValue) {
        if (outError) *outError = "benchmark row could not resolve the canonical Explaino axis slot";
        return false;
    }
    *axisValue = benchmarkCase.param_value;
    return true;
}

} // namespace

const ExplainoVariantBenchmarkCase* GetExplainoVariantBenchmarkCases(std::size_t* outCount) {
    if (outCount) *outCount = sizeof(kBenchmarkCases) / sizeof(kBenchmarkCases[0]);
    return kBenchmarkCases;
}

bool BuildExplainoVariantBenchmarkState(const ExplainoVariantBenchmarkCase& benchmarkCase,
    int width,
    int height,
    ViewState* outView,
    KernelParams* outParams,
    RenderSettings* outRender,
    const char** outError) {
    if (outError) *outError = nullptr;
    if (!outView) {
        if (outError) *outError = "benchmark view pointer is null";
        return false;
    }
    if (!outParams) {
        if (outError) *outError = "benchmark params pointer is null";
        return false;
    }
    if (!outRender) {
        if (outError) *outError = "benchmark render pointer is null";
        return false;
    }
    if (width <= 0 || height <= 0) {
        if (outError) *outError = "benchmark resolution must be positive";
        return false;
    }

    ViewState view{};
    view.fractal_type = benchmarkCase.fractal_type;
    ApplyFractalViewPresetDefaults(view, nullptr);

    KernelParams params{};
    ApplyFractalPresetDefaults(view, params, nullptr);
    if (!ApplyUniqueParamOverride(benchmarkCase, &params, outError)) return false;
    UpdateExplainoPolynomial(view, params, nullptr);
    SyncViewHpFromUi(view);

    RenderSettings render{};
    render.resolution = {width, height};
    render.block_size = 256;
    render.device_id = 0;
    render.benchmark = false;
    render.sample_tier = SampleTier::fast;
    render.resolved_eval = ResolveSampleEvalMode(view.fractal_type, render.sample_tier, view.log2_zoom);

    *outView = view;
    *outParams = params;
    *outRender = render;
    return true;
}

void WriteExplainoVariantBenchmarkCsv(const char* path,
    const ExplainoVariantBenchmarkResult* results,
    std::size_t resultCount) {
    if (!path || path[0] == '\0' || !results) return;

    std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file) {
        std::fprintf(stderr, "Failed to open explaino benchmark CSV output: %s\n", path);
        return;
    }

    file << "case_id,fractal_type,param_name,param_value,zero_axis,sample_tier,resolved_backend,resolved_strategy,width,height,repeats,avg_ms,min_ms,max_ms,gpu_ms_per_1m,avg_iters,avg_converged_iters,converged_fraction\n";
    file << std::fixed << std::setprecision(6);
    for (std::size_t index = 0; index < resultCount; ++index) {
        const ExplainoVariantBenchmarkResult& result = results[index];
        file << result.case_id << ','
             << FractalTypeId(result.fractal_type) << ','
             << result.param_name << ','
             << result.param_value << ','
             << (result.zero_axis ? 1 : 0) << ','
             << SampleTierId(result.sample_tier) << ','
             << NumericBackendIdLocal(result.resolved_eval.backend) << ','
             << IterationStrategyIdLocal(result.resolved_eval.strategy) << ','
             << result.width << ','
             << result.height << ','
             << result.repeats << ','
             << result.avg_ms << ','
             << result.min_ms << ','
             << result.max_ms << ','
             << result.gpu_ms_per_1m << ','
             << result.avg_iters << ','
             << result.avg_converged_iters << ','
             << result.converged_fraction << '\n';
    }
}