#pragma once

#include "fractal_types.h"

// FractalSampleResult: per-point iteration result from the CUDA sample device
// function. Plain C++ struct — no CUDA dependency. Usable from both host and
// device code.
//
// This is the canonical output of fractal_sample_device(). It captures the
// iteration outcome without pixel mapping, coloring, or framebuffer concerns.

struct FractalSampleResult {
    int iterations;     // iteration count at exit
    float final_z_x;   // Re(z) at exit
    float final_z_y;   // Im(z) at exit
    float residual;     // |P(z)| or escape magnitude at exit
    bool converged;     // root-finding convergence
    bool escaped;       // escape-time divergence
    TerminationKind termination_kind{TerminationKind::none};
    bool has_far_field_delta{false};
    float far_field_delta{0.0f};
};

// Slice A widens the sample seam without replacing the shipped legacy result.
// The richer payload keeps the sampled coordinate beside the legacy result so
// later slices can add new consumers while current callers still project back
// through FractalSampleResult unchanged.
struct FractalSampleEvidence {
    Double2 sample_coord{0.0, 0.0};
    FractalSampleResult legacy_result{};
};

inline FractalSampleResult BuildLegacySampleResult(const FractalSampleEvidence& evidence) {
    return evidence.legacy_result;
}
