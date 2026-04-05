#include "sweep_player.h"

#include <cmath>

bool BuildSweepSeedList(const SweepPlayerConfig& config, std::vector<double>* outSeeds, std::string* outError) {
    if (outError) outError->clear();
    if (!outSeeds) {
        if (outError) *outError = "BuildSweepSeedList: outSeeds is null";
        return false;
    }
    outSeeds->clear();

    if (!std::isfinite(config.seed_start) || !std::isfinite(config.seed_stop) || !std::isfinite(config.seed_step)) {
        if (outError) *outError = "Sweep seeds must be finite";
        return false;
    }
    if (config.seed_step == 0.0) {
        if (outError) *outError = "Sweep seed step must be non-zero";
        return false;
    }
    if ((config.seed_stop > config.seed_start && config.seed_step < 0.0) ||
        (config.seed_stop < config.seed_start && config.seed_step > 0.0)) {
        if (outError) *outError = "Sweep seed step direction does not reach stop";
        return false;
    }

    const double epsilon = 1.0e-12;
    double current = config.seed_start;
    if (config.seed_step > 0.0) {
        while (current <= config.seed_stop + epsilon) {
            outSeeds->push_back(current);
            current += config.seed_step;
        }
    } else {
        while (current >= config.seed_stop - epsilon) {
            outSeeds->push_back(current);
            current += config.seed_step;
        }
    }

    if (outSeeds->empty()) {
        if (outError) *outError = "Sweep seed list is empty";
        return false;
    }
    return true;
}

bool InitializeSweepPlayer(const SweepPlayerConfig& config, SweepPlayerState* outState, std::string* outError) {
    if (outError) outError->clear();
    if (!outState) {
        if (outError) *outError = "InitializeSweepPlayer: outState is null";
        return false;
    }
    if (!(config.dwell_seconds > 0.0) || !std::isfinite(config.dwell_seconds)) {
        if (outError) *outError = "Sweep dwell_seconds must be finite and > 0";
        return false;
    }

    SweepPlayerState state{};
    if (!BuildSweepSeedList(config, &state.seeds, outError)) {
        return false;
    }
    *outState = std::move(state);
    return true;
}

bool SweepPlayerCurrentSeed(const SweepPlayerState& state, double* outSeed) {
    if (!outSeed || state.seeds.empty() || state.current_index < 0 || state.current_index >= (int)state.seeds.size()) {
        return false;
    }
    *outSeed = state.seeds[(size_t)state.current_index];
    return true;
}

bool AdvanceSweepPlayer(const SweepPlayerConfig& config, double deltaSeconds, SweepPlayerState* ioState, bool* outChanged) {
    if (outChanged) *outChanged = false;
    if (!ioState) return false;
    if (ioState->finished || ioState->seeds.empty()) return true;
    if (!(deltaSeconds >= 0.0) || !std::isfinite(deltaSeconds)) return false;

    ioState->accumulated_seconds += deltaSeconds;
    if (ioState->accumulated_seconds < config.dwell_seconds) {
        return true;
    }

    while (ioState->accumulated_seconds >= config.dwell_seconds && !ioState->finished) {
        ioState->accumulated_seconds -= config.dwell_seconds;
        ++ioState->current_index;

        if (ioState->current_index >= (int)ioState->seeds.size()) {
            if (config.loop) {
                ioState->current_index = 0;
            } else {
                ioState->current_index = (int)ioState->seeds.size() - 1;
                ioState->finished = true;
            }
        }

        if (outChanged) *outChanged = true;
    }

    return true;
}