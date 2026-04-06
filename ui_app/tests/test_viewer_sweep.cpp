#include "../src/viewer_sweep.h"

#include "../src\explaino_seed.h"

#include <cmath>
#include <iostream>
#include <string>

static bool NearlyEqual(double a, double b, double eps = 1.0e-6) {
    return std::fabs(a - b) <= eps;
}

int main() {
    SweepPlayerConfig config{};
    config.enabled = true;
    config.seed_start = 0.70;
    config.seed_stop = 0.80;
    config.seed_step = 0.05;
    config.dwell_seconds = 0.5;

    SweepPlayerState state{};
    std::string error;
    if (!InitializeSweepPlayer(config, &state, &error)) {
        std::cerr << "InitializeSweepPlayer failed: " << error << "\n";
        return 1;
    }

    ViewState view{};
    view.fractal_type = FractalType::explaino;
    KernelParams params{};
    ExplainoSeedSetCombined(view, params, 0.70);

    bool dirty = false;
    if (!ApplySweepPlayback(config, false, false, 0.25, &state, &view, &params, &dirty)) {
        std::cerr << "ApplySweepPlayback failed before dwell threshold\n";
        return 1;
    }
    if (dirty || !NearlyEqual(ExplainoSeedCombined(view, params), 0.70)) {
        std::cerr << "Sweep playback should not change the combined seed before dwell threshold\n";
        return 1;
    }

    if (!ApplySweepPlayback(config, true, false, 1.00, &state, &view, &params, &dirty)) {
        std::cerr << "ApplySweepPlayback failed while paused\n";
        return 1;
    }
    if (dirty || !NearlyEqual(ExplainoSeedCombined(view, params), 0.70)) {
        std::cerr << "Paused sweep playback should not change the combined seed\n";
        return 1;
    }

    if (!ApplySweepPlayback(config, true, true, 0.01, &state, &view, &params, &dirty)) {
        std::cerr << "ApplySweepPlayback failed on single-step advance\n";
        return 1;
    }
    if (!dirty || !NearlyEqual(ExplainoSeedCombined(view, params), 0.75)) {
        std::cerr << "Single-step sweep playback should advance to the next combined seed\n";
        return 1;
    }
    if (params.poly_kind != PolyKind::custom || params.explaino_root_count != 4) {
        std::cerr << "Sweep playback should refresh the Explaino polynomial after applying a new seed\n";
        return 1;
    }

    if (!ApplySweepPlayback(config, false, false, 1.00, &state, &view, &params, &dirty)) {
        std::cerr << "ApplySweepPlayback failed on final advance\n";
        return 1;
    }
    if (!dirty || !state.finished || !NearlyEqual(ExplainoSeedCombined(view, params), 0.80)) {
        std::cerr << "Final sweep advance should land on the last combined seed and mark the sweep finished\n";
        return 1;
    }

    std::cout << "test_viewer_sweep: all passed\n";
    return 0;
}