#include "runtime_walk_field_slime.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>

namespace {

RuntimeWalkFieldSlimeConfig NormalizeConfig(RuntimeWalkFieldSlimeConfig config) {
    config.min_marbles = std::max(1, config.min_marbles);
    config.max_marbles = std::max(config.min_marbles, config.max_marbles);
    config.max_steps = std::max(1, config.max_steps);
    config.grid_resolution = std::max(2, config.grid_resolution);
    config.gradient_sensitivity = std::max(0.0, config.gradient_sensitivity);
    config.hysteresis = std::clamp(config.hysteresis, 0.0, 0.95);
    config.export_cadence = std::max(1, config.export_cadence);
    config.finite_difference_step = std::clamp(config.finite_difference_step, 1.0e-7, 0.05);
    config.step_scale = std::clamp(config.step_scale, 1.0e-7, 0.25);
    return config;
}

bool IsFiniteSample(const FractalSampleResult& sample) {
    return std::isfinite(sample.final_z_x) &&
        std::isfinite(sample.final_z_y) &&
        std::isfinite(sample.residual);
}

Double2 Add(Double2 lhs, Double2 rhs) {
    return {lhs.x + rhs.x, lhs.y + rhs.y};
}

Double2 Mul(Double2 value, double scale) {
    return {value.x * scale, value.y * scale};
}

Double2 Normalize(Double2 value) {
    const double length = std::sqrt(value.x * value.x + value.y * value.y);
    if (!(length > 1.0e-12)) return {0.0, 0.0};
    return {value.x / length, value.y / length};
}

double HashUnit(std::uint32_t seed, std::uint32_t index, std::uint32_t channel) {
    std::uint32_t x = seed ^ (index * 0x9e3779b9u) ^ (channel * 0x85ebca6bu);
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    x ^= x >> 16;
    return static_cast<double>(x) / static_cast<double>(0xffffffffu);
}

double WorldScaleForView(const ViewState& view) {
    return 2.0 / std::max(1.0e-30, std::pow(2.0, view.log2_zoom));
}

int QuantizeCell(const RuntimeWalkFieldSlimeConfig& config, const ViewState& view, Double2 world) {
    const double scale = WorldScaleForView(view);
    const double u = std::clamp(0.5 + (world.x - view.center_hp_x) / std::max(1.0e-12, scale * 2.0), 0.0, 0.999999);
    const double v = std::clamp(0.5 + (world.y - view.center_hp_y) / std::max(1.0e-12, scale * 2.0), 0.0, 0.999999);
    const int x = std::clamp(static_cast<int>(u * config.grid_resolution), 0, config.grid_resolution - 1);
    const int y = std::clamp(static_cast<int>(v * config.grid_resolution), 0, config.grid_resolution - 1);
    return y * config.grid_resolution + x;
}

RuntimeWalkFieldSlimeMarble BuildSeededMarble(const RuntimeWalkFieldSlimeConfig& config,
    const ViewState& view,
    std::uint32_t seed,
    int index,
    Double2 anchor,
    const char* stopReason) {
    const double scale = WorldScaleForView(view);
    const double u = HashUnit(seed, static_cast<std::uint32_t>(index), 1u);
    const double v = HashUnit(seed, static_cast<std::uint32_t>(index), 2u);
    const double angle = 6.28318530717958647692 * u;
    const double radius = scale * 0.18 * std::sqrt(v);
    RuntimeWalkFieldSlimeMarble marble;
    marble.marble_id = index;
    marble.world = {anchor.x + std::cos(angle) * radius, anchor.y + std::sin(angle) * radius};
    marble.previous_world = marble.world;
    marble.active = true;
    marble.stop_reason = stopReason;
    marble.cell_id = QuantizeCell(config, view, marble.world);
    return marble;
}

void EnsureMarblePopulation(const RuntimeWalkFieldSlimeConfig& config,
    const ViewState& view,
    RuntimeWalkFieldSlimeState* ioState,
    int desiredCount) {
    const int clampedCount = std::clamp(desiredCount, config.min_marbles, config.max_marbles);
    if (static_cast<int>(ioState->marbles.size()) > clampedCount) {
        ioState->marbles.resize(static_cast<std::size_t>(clampedCount));
    }
    const Double2 anchor = ioState->traveler.cluster_id >= 0
        ? ioState->traveler.centroid_world
        : Double2{view.center_hp_x, view.center_hp_y};
    for (int index = static_cast<int>(ioState->marbles.size()); index < clampedCount; ++index) {
        ioState->marbles.push_back(BuildSeededMarble(config, view, ioState->seed, index, anchor, "adaptive_seed"));
    }
    ioState->actual_sample_count = static_cast<int>(ioState->marbles.size());
}
void RebuildCellsAndTraveler(const RuntimeWalkFieldSlimeConfig& config,
    const ViewState& view,
    RuntimeWalkFieldSlimeState* ioState) {
    struct Accum {
        int count = 0;
        double weight = 0.0;
        Double2 weighted_sum{0.0, 0.0};
    };

    std::map<int, Accum> accumByCell;
    for (RuntimeWalkFieldSlimeMarble& marble : ioState->marbles) {
        if (!marble.active) continue;
        marble.cell_id = QuantizeCell(config, view, marble.world);
        const double weight = std::max(1.0e-6, marble.score);
        Accum& accum = accumByCell[marble.cell_id];
        accum.count += 1;
        accum.weight += weight;
        accum.weighted_sum.x += marble.world.x * weight;
        accum.weighted_sum.y += marble.world.y * weight;
    }

    ioState->cells.clear();
    ioState->cells.reserve(accumByCell.size());
    RuntimeWalkFieldSlimeCell bestCell{};
    bool haveBest = false;
    for (const auto& it : accumByCell) {
        RuntimeWalkFieldSlimeCell cell;
        cell.cell_id = it.first;
        cell.marble_count = it.second.count;
        cell.score = it.second.weight;
        cell.centroid_world = {
            it.second.weighted_sum.x / std::max(1.0e-12, it.second.weight),
            it.second.weighted_sum.y / std::max(1.0e-12, it.second.weight),
        };
        ioState->cells.push_back(cell);
        if (!haveBest || cell.score > bestCell.score) {
            bestCell = cell;
            haveBest = true;
        }
    }

    if (!haveBest) {
        ioState->traveler = {};
        return;
    }

    if (ioState->traveler.cluster_id >= 0) {
        for (const RuntimeWalkFieldSlimeCell& cell : ioState->cells) {
            if (cell.cell_id == ioState->traveler.cluster_id &&
                    cell.score >= bestCell.score * (1.0 - config.hysteresis)) {
                bestCell = cell;
                break;
            }
        }
    }

    ioState->traveler.cluster_id = bestCell.cell_id;
    ioState->traveler.marble_count = bestCell.marble_count;
    ioState->traveler.centroid_world = bestCell.centroid_world;
    ioState->traveler.confidence = bestCell.score / std::max(1.0e-12, static_cast<double>(ioState->marbles.size()));
    for (RuntimeWalkFieldSlimeMarble& marble : ioState->marbles) {
        marble.traveler_cluster_id = (marble.cell_id == bestCell.cell_id) ? bestCell.cell_id : -1;
    }
}

bool WriteParentDirs(const std::string& path, std::string* outError) {
    std::filesystem::path fsPath(path);
    std::error_code ec;
    if (!fsPath.parent_path().empty()) {
        std::filesystem::create_directories(fsPath.parent_path(), ec);
        if (ec) {
            if (outError) *outError = "Failed to create CSV directory: " + fsPath.parent_path().string();
            return false;
        }
    }
    return true;
}

bool PublishCsvTempFile(const std::string& tempPath,
    const std::string& finalPath,
    std::string* outError) {
    std::error_code ec;
    std::filesystem::remove(finalPath, ec);
    ec.clear();
    std::filesystem::rename(tempPath, finalPath, ec);
    if (ec) {
        if (outError) *outError = "Failed to publish field slime CSV: " + finalPath;
        return false;
    }
    return true;
}
} // namespace

int ComputeRuntimeWalkFieldSlimeAdaptiveMarbleCount(const RuntimeWalkFieldSlimeConfig& rawConfig,
    double gradientMagnitude,
    double tangentCurvature,
    double bindingVelocity,
    double clusterSpread) {
    const RuntimeWalkFieldSlimeConfig config = NormalizeConfig(rawConfig);
    const double pressure = std::max(0.0,
        0.40 * gradientMagnitude +
        0.25 * tangentCurvature +
        0.20 * bindingVelocity +
        0.15 * clusterSpread);
    const double response = 1.0 - std::exp(-pressure * std::max(0.0, config.gradient_sensitivity));
    const int span = config.max_marbles - config.min_marbles;
    return std::clamp(config.min_marbles + static_cast<int>(std::lround(response * span)), config.min_marbles, config.max_marbles);
}

bool RuntimeWalkFieldSlimeNeedsResetForSeed(const RuntimeWalkFieldSlimeState& state,
    std::uint32_t seed) {
    return state.seed != seed || state.marbles.empty();
}

bool InitializeRuntimeWalkFieldSlime(const RuntimeWalkFieldSlimeConfig& rawConfig,
    const ViewState& view,
    const KernelParams&,
    const RenderSettings&,
    std::uint32_t seed,
    RuntimeWalkFieldSlimeState* outState,
    std::string* outError) {
    if (outError) outError->clear();
    if (!outState) {
        if (outError) *outError = "Runtime-walk field slime state output is required";
        return false;
    }

    const RuntimeWalkFieldSlimeConfig config = NormalizeConfig(rawConfig);
    RuntimeWalkFieldSlimeState state;
    state.seed = seed;
    state.export_sequence = 0;
    state.export_due = false;
    state.marbles.reserve(static_cast<std::size_t>(config.max_marbles));
    EnsureMarblePopulation(config, view, &state, config.min_marbles);
    RebuildCellsAndTraveler(config, view, &state);
    *outState = state;
    return true;
}

bool StepRuntimeWalkFieldSlime(const SidecarMeasurementHost& measurementHost,
    const RuntimeWalkFieldSlimeConfig& rawConfig,
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    double t,
    RuntimeWalkFieldSlimeState* ioState,
    std::string* outError) {
    if (outError) outError->clear();
    if (!ioState) {
        if (outError) *outError = "Runtime-walk field slime state is required";
        return false;
    }
    const RuntimeWalkFieldSlimeConfig config = NormalizeConfig(rawConfig);
    ioState->export_due = false;
    ioState->t = t;
    const double eps = config.finite_difference_step * WorldScaleForView(view);
    const int stepCount = std::max(1, config.max_steps);

    for (int step = 0; step < stepCount; ++step) {
        std::vector<Double2> coords;
        coords.reserve(ioState->marbles.size() * 3u);
        for (const RuntimeWalkFieldSlimeMarble& marble : ioState->marbles) {
            coords.push_back(marble.world);
            coords.push_back({marble.world.x + eps, marble.world.y});
            coords.push_back({marble.world.x, marble.world.y + eps});
        }

        std::vector<FractalSampleResult> samples;
        if (!measurementHost.Sample(coords, view, params, render, &samples, outError)) {
            return false;
        }
        if (samples.size() != coords.size()) {
            if (outError) {
                std::ostringstream msg;
                msg << "field slime measurement host returned " << samples.size() << " results for " << coords.size() << " coords";
                *outError = msg.str();
            }
            return false;
        }

        double totalGradient = 0.0;
        int measuredFiniteCount = 0;
        for (std::size_t index = 0; index < ioState->marbles.size(); ++index) {
            RuntimeWalkFieldSlimeMarble& marble = ioState->marbles[index];
            const FractalSampleResult& center = samples[index * 3u];
            const FractalSampleResult& xPlus = samples[index * 3u + 1u];
            const FractalSampleResult& yPlus = samples[index * 3u + 2u];
            if (!IsFiniteSample(center) || !IsFiniteSample(xPlus) || !IsFiniteSample(yPlus)) {
                marble.active = false;
                marble.stop_reason = "nonfinite_sample";
                continue;
            }

            const double gx = (static_cast<double>(xPlus.residual) - static_cast<double>(center.residual)) / std::max(1.0e-12, eps);
            const double gy = (static_cast<double>(yPlus.residual) - static_cast<double>(center.residual)) / std::max(1.0e-12, eps);
            const double gradient = std::sqrt(gx * gx + gy * gy);
            totalGradient += gradient;
            ++measuredFiniteCount;
            Double2 tangent = Normalize({-gy, gx});
            if (std::fabs(tangent.x) < 1.0e-12 && std::fabs(tangent.y) < 1.0e-12) {
                const double angle = 6.28318530717958647692 * HashUnit(ioState->seed, static_cast<std::uint32_t>(marble.marble_id), 9u);
                tangent = {std::cos(angle), std::sin(angle)};
            }
            marble.previous_world = marble.world;
            marble.tangent = tangent;
            marble.tangent_angle = std::atan2(tangent.y, tangent.x);
            marble.residual = center.residual;
            marble.iterations = center.iterations;
            marble.score = gradient / (1.0 + std::fabs(static_cast<double>(center.residual))) + 0.001 * static_cast<double>(center.iterations);
            marble.world = Add(marble.world, Mul(tangent, config.step_scale * WorldScaleForView(view) * (1.0 + 0.15 * std::min(8.0, marble.score))));
            marble.active = true;
            marble.stop_reason = "field_step";
        }
        const double meanGradient = totalGradient / std::max(1.0, static_cast<double>(measuredFiniteCount));
        const int desiredCount = ComputeRuntimeWalkFieldSlimeAdaptiveMarbleCount(config, meanGradient, 0.0, 0.0, 0.0);
        EnsureMarblePopulation(config, view, ioState, desiredCount);
    }

    RebuildCellsAndTraveler(config, view, ioState);
    ioState->actual_sample_count = static_cast<int>(ioState->marbles.size());
    ioState->export_sequence += 1;
    ioState->export_due = (ioState->export_sequence % config.export_cadence) == 0;
    return true;
}

bool WriteRuntimeWalkFieldSlimeCsv(const RuntimeWalkFieldSlimeState& state,
    const std::string& flowLinesCsvPath,
    const std::string& fieldCellsCsvPath,
    std::string* outError) {
    if (outError) outError->clear();
    if (flowLinesCsvPath.empty() || fieldCellsCsvPath.empty()) {
        if (outError) *outError = "Runtime-walk field slime CSV export requires both output paths";
        return false;
    }
    if (!WriteParentDirs(flowLinesCsvPath, outError) || !WriteParentDirs(fieldCellsCsvPath, outError)) return false;

    const std::string flowTempPath = flowLinesCsvPath + ".tmp";
    const std::string cellsTempPath = fieldCellsCsvPath + ".tmp";
    std::ofstream flow(flowTempPath, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!flow) {
        if (outError) *outError = "Failed to open field slime flow CSV: " + flowTempPath;
        return false;
    }
    flow << "t,export_sequence,actual_sample_count,marble_id,cell_id,traveler_cluster_id,world_x,world_y,previous_world_x,previous_world_y,tangent_x,tangent_y,tangent_angle,residual,iterations,score,active,stop_reason\n";
    for (const RuntimeWalkFieldSlimeMarble& marble : state.marbles) {
        flow << state.t << ','
            << state.export_sequence << ','
            << state.actual_sample_count << ','
            << marble.marble_id << ','
            << marble.cell_id << ','
            << marble.traveler_cluster_id << ','
            << marble.world.x << ','
            << marble.world.y << ','
            << marble.previous_world.x << ','
            << marble.previous_world.y << ','
            << marble.tangent.x << ','
            << marble.tangent.y << ','
            << marble.tangent_angle << ','
            << marble.residual << ','
            << marble.iterations << ','
            << marble.score << ','
            << (marble.active ? 1 : 0) << ','
            << marble.stop_reason << "\n";
    }
    flow.close();
    if (!flow) {
        if (outError) *outError = "Failed to write field slime flow CSV: " + flowTempPath;
        return false;
    }

    std::ofstream cells(cellsTempPath, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!cells) {
        if (outError) *outError = "Failed to open field slime cells CSV: " + cellsTempPath;
        return false;
    }
    cells << "t,export_sequence,actual_sample_count,cell_id,marble_count,centroid_x,centroid_y,score,traveler_cluster_id,traveler_centroid_x,traveler_centroid_y,cluster_confidence\n";
    for (const RuntimeWalkFieldSlimeCell& cell : state.cells) {
        cells << state.t << ','
            << state.export_sequence << ','
            << state.actual_sample_count << ','
            << cell.cell_id << ','
            << cell.marble_count << ','
            << cell.centroid_world.x << ','
            << cell.centroid_world.y << ','
            << cell.score << ','
            << state.traveler.cluster_id << ','
            << state.traveler.centroid_world.x << ','
            << state.traveler.centroid_world.y << ','
            << state.traveler.confidence << "\n";
    }
    cells.close();
    if (!cells) {
        if (outError) *outError = "Failed to write field slime cells CSV: " + cellsTempPath;
        return false;
    }
    return PublishCsvTempFile(flowTempPath, flowLinesCsvPath, outError) &&
        PublishCsvTempFile(cellsTempPath, fieldCellsCsvPath, outError);
}
