#pragma once

#include "fractal_types.h"

#include <cstddef>

struct ExplainoVariantBenchmarkCase {
    const char* case_id;
    FractalType fractal_type;
    const char* param_name;
    float param_value;
    bool zero_axis;
};

struct ExplainoVariantBenchmarkResult {
    const char* case_id;
    FractalType fractal_type;
    const char* param_name;
    float param_value;
    bool zero_axis;
    SampleTier sample_tier;
    ResolvedEvalMode resolved_eval;
    int width;
    int height;
    int repeats;
    double avg_ms;
    double min_ms;
    double max_ms;
    double gpu_ms_per_1m;
    double avg_iters;
    double avg_converged_iters;
    double converged_fraction;
};

const ExplainoVariantBenchmarkCase* GetExplainoVariantBenchmarkCases(std::size_t* outCount);

bool BuildExplainoVariantBenchmarkState(const ExplainoVariantBenchmarkCase& benchmarkCase,
    int width,
    int height,
    ViewState* outView,
    KernelParams* outParams,
    RenderSettings* outRender,
    const char** outError);

void WriteExplainoVariantBenchmarkCsv(const char* path,
    const ExplainoVariantBenchmarkResult* results,
    std::size_t resultCount);