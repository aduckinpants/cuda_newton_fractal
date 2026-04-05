#include "../src/sweep_player.h"

#include <cmath>
#include <iostream>

static bool NearlyEqual(double a, double b, double eps = 1.0e-9) {
    return std::fabs(a - b) <= eps;
}

int main() {
    {
        SweepPlayerConfig config{};
        config.enabled = true;
        config.seed_start = 0.70;
        config.seed_stop = 0.80;
        config.seed_step = 0.05;

        std::vector<double> seeds;
        std::string error;
        if (!BuildSweepSeedList(config, &seeds, &error)) {
            std::cerr << "BuildSweepSeedList failed: " << error << "\n";
            return 1;
        }
        if (seeds.size() != 3 || !NearlyEqual(seeds[0], 0.70) || !NearlyEqual(seeds[1], 0.75) || !NearlyEqual(seeds[2], 0.80)) {
            std::cerr << "Sweep seeds were not inclusive or ordered correctly\n";
            return 1;
        }
    }

    {
        SweepPlayerConfig config{};
        config.enabled = true;
        config.seed_start = 0.70;
        config.seed_stop = 0.80;
        config.seed_step = 0.05;
        config.dwell_seconds = 0.5;
        config.loop = false;

        SweepPlayerState state{};
        std::string error;
        if (!InitializeSweepPlayer(config, &state, &error)) {
            std::cerr << "InitializeSweepPlayer failed: " << error << "\n";
            return 1;
        }

        double seed = 0.0;
        if (!SweepPlayerCurrentSeed(state, &seed) || !NearlyEqual(seed, 0.70)) {
            std::cerr << "Initial sweep seed incorrect\n";
            return 1;
        }

        bool changed = false;
        if (!AdvanceSweepPlayer(config, 0.25, &state, &changed) || changed) {
            std::cerr << "Sweep advanced too early\n";
            return 1;
        }

        if (!AdvanceSweepPlayer(config, 0.30, &state, &changed) || !changed) {
            std::cerr << "Sweep did not advance after dwell threshold\n";
            return 1;
        }
        if (!SweepPlayerCurrentSeed(state, &seed) || !NearlyEqual(seed, 0.75)) {
            std::cerr << "Sweep current seed after first advance incorrect\n";
            return 1;
        }

        if (!AdvanceSweepPlayer(config, 1.00, &state, &changed) || !changed || !state.finished) {
            std::cerr << "Sweep should finish on final seed without looping\n";
            return 1;
        }
        if (!SweepPlayerCurrentSeed(state, &seed) || !NearlyEqual(seed, 0.80)) {
            std::cerr << "Final sweep seed incorrect\n";
            return 1;
        }
    }

    return 0;
}