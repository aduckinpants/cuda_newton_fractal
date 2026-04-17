#include "runtime_walk_viewer.h"

#include <algorithm>
#include <cmath>

namespace {

constexpr double kSplineTension = 0.5;

double ClampUnit(double value) {
    return std::clamp(value, 0.0, 1.0);
}

double TMin(const RuntimeWalkViewerAsset& asset) {
    return asset.tick_snapshots.empty() ? 0.0 : asset.tick_snapshots.front().t;
}

double TMax(const RuntimeWalkViewerAsset& asset) {
    return asset.tick_snapshots.empty() ? 0.0 : asset.tick_snapshots.back().t;
}

std::size_t FindNearestTickIndex(const RuntimeWalkViewerAsset& asset, double t) {
    if (asset.tick_snapshots.empty()) return 0;
    std::size_t bestIndex = 0;
    double bestDistance = std::fabs(asset.tick_snapshots.front().t - t);
    for (std::size_t index = 1; index < asset.tick_snapshots.size(); ++index) {
        const double distance = std::fabs(asset.tick_snapshots[index].t - t);
        if (distance < bestDistance) {
            bestDistance = distance;
            bestIndex = index;
        }
    }
    return bestIndex;
}

Double2 NormalizeVector(Double2 value) {
    const double length = std::sqrt(value.x * value.x + value.y * value.y);
    if (!(length > 1.0e-12)) return {0.0, 0.0};
    return {value.x / length, value.y / length};
}

Double2 Add(Double2 lhs, Double2 rhs) {
    return {lhs.x + rhs.x, lhs.y + rhs.y};
}

Double2 Sub(Double2 lhs, Double2 rhs) {
    return {lhs.x - rhs.x, lhs.y - rhs.y};
}

Double2 Mul(Double2 value, double scale) {
    return {value.x * scale, value.y * scale};
}

double Length(Double2 value) {
    return std::sqrt(value.x * value.x + value.y * value.y);
}

Double2 Rotate(Double2 value, double radians) {
    const double s = std::sin(radians);
    const double c = std::cos(radians);
    return {value.x * c - value.y * s, value.x * s + value.y * c};
}

Double2 Blend(Double2 lhs, double lhsWeight, Double2 rhs, double rhsWeight) {
    return Add(Mul(lhs, lhsWeight), Mul(rhs, rhsWeight));
}

Double2 CatmullRom(Double2 p0, Double2 p1, Double2 p2, Double2 p3, double t) {
    const double t2 = t * t;
    const double t3 = t2 * t;
    const double s = kSplineTension;
    return {
        0.5 * ((2.0 * p1.x) +
            (-p0.x + p2.x) * t +
            (2.0 * p0.x - 5.0 * p1.x + 4.0 * p2.x - p3.x) * t2 +
            (-p0.x + 3.0 * p1.x - 3.0 * p2.x + p3.x) * t3) * s +
            p1.x * (1.0 - s),
        0.5 * ((2.0 * p1.y) +
            (-p0.y + p2.y) * t +
            (2.0 * p0.y - 5.0 * p1.y + 4.0 * p2.y - p3.y) * t2 +
            (-p0.y + 3.0 * p1.y - 3.0 * p2.y + p3.y) * t3) * s +
            p1.y * (1.0 - s),
    };
}

Double2 SnapshotCenter(const RuntimeWalkSnapshot& snapshot) {
    return {snapshot.center_hp_x, snapshot.center_hp_y};
}

Double2 NormalizePoint(Double2 point, double minX, double maxX, double minY, double maxY) {
    const double spanX = std::max(1.0e-9, maxX - minX);
    const double spanY = std::max(1.0e-9, maxY - minY);
    const double u = 0.10 + 0.80 * ((point.x - minX) / spanX);
    const double v = 0.10 + 0.80 * (1.0 - ((point.y - minY) / spanY));
    return {ClampUnit(u), ClampUnit(v)};
}

void BuildSplinePath(const std::vector<Double2>& points, bool closedLoop, std::vector<Double2>* outPoints) {
    outPoints->clear();
    if (points.empty()) return;
    if (points.size() == 1u) {
        outPoints->push_back(points.front());
        return;
    }

    const int subdivisions = 12;
    const std::size_t segmentCount = closedLoop ? points.size() : points.size() - 1u;
    for (std::size_t segment = 0; segment < segmentCount; ++segment) {
        const auto at = [&](long long index) -> Double2 {
            if (closedLoop) {
                const long long size = static_cast<long long>(points.size());
                long long wrapped = index % size;
                if (wrapped < 0) wrapped += size;
                return points[static_cast<std::size_t>(wrapped)];
            }
            if (index < 0) return points.front();
            if (index >= static_cast<long long>(points.size())) return points.back();
            return points[static_cast<std::size_t>(index)];
        };

        const Double2 p0 = at(static_cast<long long>(segment) - 1);
        const Double2 p1 = at(static_cast<long long>(segment));
        const Double2 p2 = at(static_cast<long long>(segment) + 1);
        const Double2 p3 = at(static_cast<long long>(segment) + 2);
        const int startStep = (segment == 0u) ? 0 : 1;
        for (int step = startStep; step <= subdivisions; ++step) {
            const double t = static_cast<double>(step) / static_cast<double>(subdivisions);
            outPoints->push_back(CatmullRom(p0, p1, p2, p3, t));
        }
    }
    if (closedLoop && !outPoints->empty()) {
        outPoints->push_back(outPoints->front());
    }
}

double RuntimeSignal(const RuntimeWalkSnapshot& snapshot, const RuntimeWalkOverlayProviderInputs& inputs) {
    const double localGradient =
        std::fabs(snapshot.channels[0] - snapshot.channels[1]) +
        std::fabs(snapshot.channels[2] - snapshot.channels[3]) +
        0.5 * std::fabs(snapshot.channels[4] - snapshot.channels[11]) +
        0.5 * std::fabs(snapshot.channels[7] - snapshot.channels[8]);
    const double stability = ClampUnit(inputs.decode_stability);
    const double divergence = std::clamp(inputs.divergence, 0.0, 4.0);
    const double branchBoost = std::clamp(inputs.branch_proximity, 0.0, 1.0);
    return (0.35 + 0.65 * stability) * (localGradient + 0.25 * branchBoost) / (1.0 + 0.35 * divergence);
}

Double2 BuildBaseGradientDirection(const RuntimeWalkSnapshot& snapshot) {
    return NormalizeVector({
        (snapshot.channels[0] - snapshot.channels[1]) + 0.35 * (snapshot.channels[9] - snapshot.channels[10]),
        (snapshot.channels[2] - snapshot.channels[3]) + 0.35 * (snapshot.channels[7] - snapshot.channels[8]),
    });
}

Double2 BuildFlowFieldDirection(Double2 point,
    Double2 origin,
    Double2 tangent,
    Double2 normal,
    Double2 baseGradient,
    Double2 branchVector,
    const RuntimeWalkSnapshot& snapshot,
    const RuntimeWalkOverlayProviderInputs& inputs,
    double phase) {
    const Double2 offset = Sub(point, origin);
    const double radius = Length(offset);
    const Double2 radial = radius > 1.0e-9 ? NormalizeVector(offset) : Rotate(tangent, phase);
    const Double2 swirl = {-radial.y, radial.x};
    const double branchProximity = std::clamp(snapshot.branch.proximity, 0.0, 1.0);
    const double stability = std::clamp(inputs.decode_stability, 0.0, 1.0);
    const double swirlWeight = 0.35 + 0.40 * branchProximity + 0.10 * std::sin(phase + radius * 12.0);
    const double tangentWeight = 0.45 + 0.20 * stability;
    const double gradientWeight = 0.55 + 0.25 * std::fabs(snapshot.channels[4] - snapshot.channels[11]);
    const double branchWeight = 0.18 + 0.35 * branchProximity;
    const double normalWeight = 0.08 + 0.18 * std::fabs(snapshot.channels[7] - snapshot.channels[8]);
    Double2 direction = {0.0, 0.0};
    direction = Add(direction, Mul(tangent, tangentWeight));
    direction = Add(direction, Mul(baseGradient, gradientWeight));
    direction = Add(direction, Mul(swirl, swirlWeight));
    direction = Add(direction, Mul(branchVector, branchWeight));
    direction = Add(direction, Mul(normal, normalWeight * std::sin(phase + radius * 10.0)));
    if (radius > 1.0e-9) {
        direction = Add(direction, Mul(radial, 0.06 * std::cos(phase + radius * 7.0)));
    }
    return NormalizeVector(direction);
}

std::vector<Double2> BuildSeedOrigins(Double2 currentPoint,
    Double2 tangent,
    Double2 normal,
    int maxStrokeCount) {
    std::vector<Double2> seeds;
    const int clampedCount = std::max(4, maxStrokeCount);
    seeds.push_back(currentPoint);
    const int ringCount = std::max(3, clampedCount - 1);
    for (int index = 0; index < ringCount; ++index) {
        const double angle = (2.0 * 3.14159265358979323846 * static_cast<double>(index)) / static_cast<double>(ringCount);
        const double radius = 0.018 + 0.018 * static_cast<double>(index % 3);
        const Double2 seedOffset = Add(Mul(Rotate(tangent, angle), radius), Mul(Rotate(normal, angle * 0.5), radius * 0.35));
        Double2 seed = Add(currentPoint, seedOffset);
        seed.x = ClampUnit(seed.x);
        seed.y = ClampUnit(seed.y);
        seeds.push_back(seed);
    }
    return seeds;
}

} // namespace

bool BuildRuntimeWalkViewerAsset(const RuntimeWalkRequest& request,
    const RuntimeWalkBundle& bundle,
    const ViewState& baseView,
    const KernelParams& baseParams,
    const RenderSettings& baseRender,
    RuntimeWalkViewerAsset* outAsset,
    std::string* outError) {
    if (outError) outError->clear();
    if (!outAsset) {
        if (outError) *outError = "Runtime walk viewer asset output is required";
        return false;
    }
    if (request.t_values.size() < 2u) {
        if (outError) *outError = "Runtime walk viewer requires at least two t values";
        return false;
    }

    RuntimeWalkViewerAsset asset;
    asset.request = request;
    asset.bundle = bundle;
    asset.base_view = baseView;
    asset.base_params = baseParams;
    asset.base_render = baseRender;
    asset.authority.mode = request.authority_mode;
    asset.authority.resolved_base_state_json_path = request.base_state_json_path;
    asset.authority.mapping_profile_json_path = request.mapping_profile_json_path;
    asset.authority.mapping_profile_id = request.mapping_profile_id;
    asset.authority.orientation_inputs_json_path = request.orientation_inputs_json_path;
    if (request.authority_mode == RuntimeWalkAuthorityMode::synthesized_fits_base) {
        asset.authority.synthesized_base_state_json_path = request.base_state_json_path;
    }
    asset.companion.comparison_fits_path = request.comparison_fits_path;
    asset.companion.rtk_manifest_json_path = request.rtk_manifest_json_path;
    asset.companion.rtk_harvest_summary_json_path = request.rtk_harvest_summary_json_path;
    asset.tick_snapshots.reserve(request.t_values.size());

    for (double t : request.t_values) {
        RuntimeWalkSnapshot snapshot;
        if (!EvaluateRuntimeWalkSnapshot(bundle, t, baseView, baseParams, baseRender, &snapshot, outError)) {
            return false;
        }
        asset.tick_snapshots.push_back(snapshot);
    }

    *outAsset = asset;
    return true;
}

bool SeekRuntimeWalkViewerPlayback(const RuntimeWalkViewerAsset& asset,
    double t,
    RuntimeWalkViewerPlaybackState* ioState,
    bool* outChanged) {
    if (outChanged) *outChanged = false;
    if (!ioState) return false;
    if (asset.tick_snapshots.empty()) return false;

    const double clampedT = std::clamp(t, TMin(asset), TMax(asset));
    const std::size_t nearestTickIndex = FindNearestTickIndex(asset, clampedT);
    const bool changed =
        !ioState->loaded ||
        std::fabs(ioState->current_t - clampedT) > 1.0e-12 ||
        ioState->nearest_tick_index != nearestTickIndex;
    ioState->loaded = true;
    ioState->current_t = clampedT;
    ioState->nearest_tick_index = nearestTickIndex;
    if (outChanged) *outChanged = changed;
    return true;
}

bool AdvanceRuntimeWalkViewerPlayback(const RuntimeWalkViewerAsset& asset,
    double deltaSeconds,
    RuntimeWalkViewerPlaybackState* ioState,
    bool* outChanged) {
    if (outChanged) *outChanged = false;
    if (!ioState) return false;
    if (asset.tick_snapshots.empty()) return false;

    if (!ioState->loaded) {
        return SeekRuntimeWalkViewerPlayback(asset, TMin(asset), ioState, outChanged);
    }
    if (!ioState->playing) return true;

    const double minT = TMin(asset);
    const double maxT = TMax(asset);
    const double span = std::max(1.0e-12, maxT - minT);
    double nextT = ioState->current_t + std::max(0.0, deltaSeconds) * std::max(0.0, ioState->speed);
    if (nextT > maxT) {
        if (ioState->loop) {
            const double overflow = std::fmod(nextT - minT, span);
            nextT = minT + overflow;
        } else {
            nextT = maxT;
            ioState->playing = false;
        }
    }
    return SeekRuntimeWalkViewerPlayback(asset, nextT, ioState, outChanged);
}

bool StepRuntimeWalkViewerPlayback(const RuntimeWalkViewerAsset& asset,
    int direction,
    RuntimeWalkViewerPlaybackState* ioState,
    bool* outChanged) {
    if (outChanged) *outChanged = false;
    if (!ioState) return false;
    if (asset.tick_snapshots.empty()) return false;
    if (!ioState->loaded) {
        return SeekRuntimeWalkViewerPlayback(asset, TMin(asset), ioState, outChanged);
    }

    long long nextIndex = static_cast<long long>(ioState->nearest_tick_index) + static_cast<long long>(direction);
    const long long lastIndex = static_cast<long long>(asset.tick_snapshots.size()) - 1LL;
    if (ioState->loop) {
        if (nextIndex < 0) nextIndex = lastIndex;
        if (nextIndex > lastIndex) nextIndex = 0;
    } else {
        nextIndex = std::clamp(nextIndex, 0LL, lastIndex);
    }
    ioState->playing = false;
    return SeekRuntimeWalkViewerPlayback(asset, asset.tick_snapshots[static_cast<std::size_t>(nextIndex)].t, ioState, outChanged);
}

bool EvaluateRuntimeWalkViewerCurrentSnapshot(const RuntimeWalkViewerAsset& asset,
    const RuntimeWalkViewerPlaybackState& playback,
    RuntimeWalkSnapshot* outSnapshot,
    std::string* outError) {
    if (asset.tick_snapshots.empty()) {
        if (outError) *outError = "Runtime walk viewer asset has no snapshots";
        return false;
    }
    const double t = playback.loaded ? playback.current_t : TMin(asset);
    return EvaluateRuntimeWalkSnapshot(asset.bundle, t, asset.base_view, asset.base_params, asset.base_render, outSnapshot, outError);
}

void BuildRuntimeWalkOverlayPath(const RuntimeWalkViewerAsset& asset,
    const RuntimeWalkViewerPlaybackState& playback,
    RuntimeWalkOverlayPath* outPath) {
    if (!outPath) return;
    *outPath = {};
    if (asset.tick_snapshots.empty()) return;

    double minX = asset.tick_snapshots.front().center_hp_x;
    double maxX = minX;
    double minY = asset.tick_snapshots.front().center_hp_y;
    double maxY = minY;
    for (const RuntimeWalkSnapshot& snapshot : asset.tick_snapshots) {
        minX = std::min(minX, snapshot.center_hp_x);
        maxX = std::max(maxX, snapshot.center_hp_x);
        minY = std::min(minY, snapshot.center_hp_y);
        maxY = std::max(maxY, snapshot.center_hp_y);
    }
    if (std::fabs(maxX - minX) < 1.0e-9) {
        minX -= 0.5;
        maxX += 0.5;
    }
    if (std::fabs(maxY - minY) < 1.0e-9) {
        minY -= 0.5;
        maxY += 0.5;
    }

    outPath->raw_points.reserve(asset.tick_snapshots.size());
    for (const RuntimeWalkSnapshot& snapshot : asset.tick_snapshots) {
        outPath->raw_points.push_back(NormalizePoint(SnapshotCenter(snapshot), minX, maxX, minY, maxY));
    }
    BuildSplinePath(outPath->raw_points, false, &outPath->spline_points);
    BuildSplinePath(outPath->raw_points, true, &outPath->closed_loop_points);

    outPath->branch_marker_points.reserve(asset.bundle.branch_markers.size());
    for (const RuntimeWalkBranchMarker& marker : asset.bundle.branch_markers) {
        RuntimeWalkSnapshot snapshot;
        std::string ignoredError;
        if (EvaluateRuntimeWalkSnapshot(asset.bundle, marker.t, asset.base_view, asset.base_params, asset.base_render, &snapshot, &ignoredError)) {
            outPath->branch_marker_points.push_back(NormalizePoint(SnapshotCenter(snapshot), minX, maxX, minY, maxY));
        }
    }

    RuntimeWalkSnapshot currentSnapshot;
    std::string ignoredError;
    if (EvaluateRuntimeWalkViewerCurrentSnapshot(asset, playback, &currentSnapshot, &ignoredError)) {
        outPath->current_point = NormalizePoint(SnapshotCenter(currentSnapshot), minX, maxX, minY, maxY);
    } else {
        outPath->current_point = outPath->raw_points.front();
    }
}

bool BuildRuntimeWalkGradientOverlay(const RuntimeWalkViewerAsset& asset,
    const RuntimeWalkViewerPlaybackState& playback,
    const RuntimeWalkOverlayProviderConfig& config,
    const RuntimeWalkOverlayProviderInputs& inputs,
    RuntimeWalkGradientOverlay* outOverlay,
    std::string* outError) {
    if (outError) outError->clear();
    if (!outOverlay) {
        if (outError) *outError = "Runtime walk gradient overlay output is required";
        return false;
    }
    *outOverlay = {};
    if (config.kind == RuntimeWalkOverlayProviderKind::none) return true;
    if (config.kind == RuntimeWalkOverlayProviderKind::external_rtk_reserved) {
        if (outError) *outError = "External RTK overlay provider is reserved but not implemented";
        return false;
    }

    RuntimeWalkOverlayPath path;
    BuildRuntimeWalkOverlayPath(asset, playback, &path);
    RuntimeWalkSnapshot snapshot;
    if (!EvaluateRuntimeWalkViewerCurrentSnapshot(asset, playback, &snapshot, outError)) {
        return false;
    }
    if (path.raw_points.empty()) return true;

    const std::size_t currentIndex = std::min(playback.nearest_tick_index, asset.tick_snapshots.size() - 1u);
    const Double2 currentPoint = path.current_point;
    const Double2 prevPoint = path.raw_points[(currentIndex == 0u) ? 0u : currentIndex - 1u];
    const Double2 nextPoint = path.raw_points[std::min(currentIndex + 1u, path.raw_points.size() - 1u)];
    Double2 tangent = NormalizeVector(Sub(nextPoint, prevPoint));
    if (std::fabs(tangent.x) < 1.0e-9 && std::fabs(tangent.y) < 1.0e-9) {
        tangent = BuildBaseGradientDirection(snapshot);
    }
    const Double2 normal = {-tangent.y, tangent.x};
    const Double2 baseGradient = BuildBaseGradientDirection(snapshot);
    const Double2 branchVector = NormalizeVector(Mul(normal, 0.5 + config.branch_bias * std::clamp(snapshot.branch.proximity, 0.0, 1.0)));

    const double baseStrength = RuntimeSignal(snapshot, inputs);
    if (!(baseStrength >= config.threshold)) {
        return true;
    }

    const int maxStrokeCount = std::max(1, config.max_strokes);
    const int maxSteps = std::max(1, config.max_steps_per_stroke);
    const std::vector<Double2> seeds = BuildSeedOrigins(currentPoint, tangent, normal, maxStrokeCount);
    for (int strokeIndex = 0; strokeIndex < static_cast<int>(seeds.size()) && strokeIndex < maxStrokeCount; ++strokeIndex) {
        const double phase = 0.55 * static_cast<double>(strokeIndex);
        const Double2 seed = seeds[strokeIndex];
        RuntimeWalkGradientOverlayGuideStroke stroke;
        stroke.strength = baseStrength * (1.0 - 0.045 * static_cast<double>(strokeIndex));
        if (stroke.strength < config.threshold) continue;

        std::vector<RuntimeWalkGradientOverlayGuidePoint> backward;
        std::vector<RuntimeWalkGradientOverlayGuidePoint> forward;
        auto traceDirection = [&](double sign, std::vector<RuntimeWalkGradientOverlayGuidePoint>* outPoints) {
            Double2 cursor = seed;
            double stepStrength = stroke.strength;
            for (int stepIndex = 0; stepIndex < maxSteps; ++stepIndex) {
                if (stepStrength < config.threshold) break;
                Double2 direction = BuildFlowFieldDirection(cursor, currentPoint, tangent, normal, baseGradient, branchVector, snapshot, inputs, phase + sign * 0.35 * static_cast<double>(stepIndex));
                if (std::fabs(direction.x) < 1.0e-9 && std::fabs(direction.y) < 1.0e-9) break;
                const double stepLength = 0.015 + 0.006 * std::clamp(inputs.decode_stability, 0.0, 1.0);
                cursor = Add(cursor, Mul(direction, stepLength * sign));
                cursor.x = ClampUnit(cursor.x);
                cursor.y = ClampUnit(cursor.y);
                outPoints->push_back({cursor, stepStrength});
                stepStrength *= 0.82;
            }
        };

        traceDirection(-1.0, &backward);
        traceDirection(1.0, &forward);

        for (auto it = backward.rbegin(); it != backward.rend(); ++it) {
            stroke.points.push_back(*it);
        }
        stroke.points.push_back({seed, stroke.strength});
        for (const RuntimeWalkGradientOverlayGuidePoint& point : forward) {
            stroke.points.push_back(point);
        }

        if (stroke.points.size() >= 2u) {
            outOverlay->strokes.push_back(stroke);
        }
    }
    return true;
}
