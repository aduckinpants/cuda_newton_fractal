#include "../src/explaino_sidecar_divergence.h"

#include <cmath>
#include <iostream>
#include <string>

namespace {

bool NearlyEqual(double left, double right, double eps = 1.0e-9) {
    const double delta = left - right;
    return delta < eps && delta > -eps;
}

SidecarOrientationVector MakeOrientation(
    std::uint64_t importSignature,
    std::uint64_t projectionHash,
    double fieldEmbeddingStats,
    double slimeEnergyDelta,
    double busyBeaverMetrics,
    double decodeStability,
    double diffMagnitude) {
    SidecarOrientationVector orientation;
    orientation.import_signature = importSignature;
    orientation.pack_projection_hash = projectionHash;
    orientation.field_embedding_stats = fieldEmbeddingStats;
    orientation.slime_energy_delta = slimeEnergyDelta;
    orientation.busy_beaver_metrics = busyBeaverMetrics;
    orientation.decode_stability = decodeStability;
    orientation.diff_magnitude = diffMagnitude;
    return orientation;
}

} // namespace

int main() {
    {
        SidecarStateDivergence divergence;
        std::string error;
        if (!BuildSidecarStateDivergence(
                nullptr,
                MakeOrientation(11u, 17u, 3.0, 2.0, 0.5, 0.75, 1.0),
                &divergence,
                &error)) {
            std::cerr << "Expected divergence build without previous orientation to succeed explicitly: " << error << "\n";
            return 1;
        }
        if (divergence.status != SidecarStateDivergenceStatus::unavailable || !NearlyEqual(divergence.scalar_divergence, 0.0)) {
            std::cerr << "Expected missing previous orientation to produce an explicit unavailable divergence state\n";
            return 1;
        }
    }

    {
        const SidecarOrientationVector previous = MakeOrientation(11u, 17u, 3.0, 2.0, 0.5, 0.75, 1.0);
        SidecarStateDivergence divergence;
        std::string error;
        if (!BuildSidecarStateDivergence(
                &previous,
                MakeOrientation(11u, 17u, 3.0, 8.0, 0.8, 0.25, 5.0),
                &divergence,
                &error)) {
            std::cerr << "Expected stable divergence state to build: " << error << "\n";
            return 1;
        }
        if (divergence.status != SidecarStateDivergenceStatus::stable ||
            divergence.import_changed ||
            divergence.projection_changed ||
            !NearlyEqual(divergence.scalar_divergence, 0.0)) {
            std::cerr << "Expected unchanged import/projection hashes to keep divergence stable even when measurement deltas move\n";
            return 1;
        }
    }

    {
        const SidecarOrientationVector previous = MakeOrientation(11u, 17u, 3.0, 2.0, 0.5, 0.75, 1.0);
        SidecarStateDivergence divergence;
        std::string error;
        if (!BuildSidecarStateDivergence(
                &previous,
                MakeOrientation(21u, 17u, 3.0, 8.0, 0.8, 0.25, 5.0),
                &divergence,
                &error)) {
            std::cerr << "Expected diverged state to build when the import signature changes: " << error << "\n";
            return 1;
        }
        if (divergence.status != SidecarStateDivergenceStatus::diverged ||
            !divergence.import_changed ||
            !(divergence.scalar_divergence > 0.0)) {
            std::cerr << "Expected import-signature changes to produce a positive divergence magnitude\n";
            return 1;
        }
    }

    {
        const SidecarOrientationVector previous = MakeOrientation(11u, 17u, 3.0, 2.0, 0.5, 0.75, 1.0);
        SidecarStateDivergence divergence;
        std::string error;
        if (BuildSidecarStateDivergence(
                &previous,
                MakeOrientation(11u, 17u, 3.0, NAN, 0.8, 0.25, 5.0),
                &divergence,
                &error)) {
            std::cerr << "Expected non-finite divergence inputs to fail fast\n";
            return 1;
        }
        if (error.find("non-finite") == std::string::npos) {
            std::cerr << "Expected divergence validation failure to mention non-finite orientation data\n";
            return 1;
        }
    }

    std::cout << "test_explaino_sidecar_divergence: all passed\n";
    return 0;
}