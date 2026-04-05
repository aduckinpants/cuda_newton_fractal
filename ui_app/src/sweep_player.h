#pragma once

#include <string>
#include <vector>

struct SweepPlayerConfig {
    bool enabled{false};
    double seed_start{0.0};
    double seed_stop{0.0};
    double seed_step{0.0};
    double dwell_seconds{0.5};
    bool loop{false};
};

struct SweepPlayerState {
    std::vector<double> seeds;
    int current_index{0};
    double accumulated_seconds{0.0};
    bool finished{false};
};

bool BuildSweepSeedList(const SweepPlayerConfig& config, std::vector<double>* outSeeds, std::string* outError);
bool InitializeSweepPlayer(const SweepPlayerConfig& config, SweepPlayerState* outState, std::string* outError);
bool SweepPlayerCurrentSeed(const SweepPlayerState& state, double* outSeed);
bool AdvanceSweepPlayer(const SweepPlayerConfig& config, double deltaSeconds, SweepPlayerState* ioState, bool* outChanged);