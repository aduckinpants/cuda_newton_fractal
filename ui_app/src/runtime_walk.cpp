#include "runtime_walk.h"

#include "explaino_seed.h"
#include "json_min.h"
#include "view_hp_sync.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <sstream>

namespace {

constexpr std::array<double, 13> kExplainoSeedWeights = {
    3.0, 2.0, 1.0, 3.0, 1.0, 2.0, 4.0, 1.0, 1.0, 1.0, 1.0, 4.0, 4.0,
};

bool ReadTextFile(const std::string& path, std::string* outText, std::string* outError) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        if (outError) *outError = "Failed to open runtime walk JSON file: " + path;
        return false;
    }
    std::ostringstream text;
    text << file.rdbuf();
    if (!file.good() && !file.eof()) {
        if (outError) *outError = "Failed to read runtime walk JSON file: " + path;
        return false;
    }
    if (outText) *outText = text.str();
    return true;
}

bool GetRequiredObject(const json_min::Value& object, const char* key, const json_min::Value** outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_object()) {
        if (outError) *outError = std::string("Missing or invalid object field: ") + key;
        return false;
    }
    if (outValue) *outValue = value;
    return true;
}

bool GetRequiredArray(const json_min::Value& object, const char* key, const json_min::Value** outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_array()) {
        if (outError) *outError = std::string("Missing or invalid array field: ") + key;
        return false;
    }
    if (outValue) *outValue = value;
    return true;
}

bool GetRequiredString(const json_min::Value& object, const char* key, std::string* outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_string()) {
        if (outError) *outError = std::string("Missing or invalid string field: ") + key;
        return false;
    }
    if (outValue) *outValue = value->as_string();
    return true;
}

bool GetRequiredNumber(const json_min::Value& object, const char* key, double* outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_number()) {
        if (outError) *outError = std::string("Missing or invalid number field: ") + key;
        return false;
    }
    if (outValue) *outValue = value->as_number();
    return true;
}

bool ParseFiniteNumber(const json_min::Value& value, double* outValue, std::string* outError, const std::string& context) {
    if (!value.is_number()) {
        if (outError) *outError = "Expected number for " + context;
        return false;
    }
    const double numeric = value.as_number();
    if (!std::isfinite(numeric)) {
        if (outError) *outError = "Expected finite number for " + context;
        return false;
    }
    if (outValue) *outValue = numeric;
    return true;
}

double Clamp01(double value) {
    return ClampD(value, 0.0, 1.0);
}

double WeightedSeed01(const std::array<double, 13>& channels) {
    double numerator = 0.0;
    double denominator = 0.0;
    for (std::size_t i = 0; i < channels.size(); ++i) {
        numerator += channels[i] * kExplainoSeedWeights[i];
        denominator += kExplainoSeedWeights[i];
    }
    if (denominator <= 0.0) return 0.0;
    return Clamp01(numerator / denominator);
}

RuntimeWalkBranchAnnotation AnnotateBranch(const RuntimeWalkBundle& bundle, double t) {
    RuntimeWalkBranchAnnotation annotation;
    annotation.distance = 1.0e30;
    for (const RuntimeWalkBranchMarker& marker : bundle.branch_markers) {
        const double distance = std::fabs(marker.t - t);
        if (distance < annotation.distance) {
            annotation.distance = distance;
            annotation.nearest_marker_id = marker.id;
            annotation.nearest_marker_label = marker.label;
            annotation.parent_id = marker.parent_id;
            annotation.sticky = distance <= marker.sticky_radius;
            if (marker.sticky_radius > 0.0) {
                annotation.proximity = Clamp01(1.0 - distance / marker.sticky_radius);
            } else {
                annotation.proximity = 0.0;
            }
        }
    }
    if (bundle.branch_markers.empty()) {
        annotation.distance = 0.0;
    }
    return annotation;
}

bool InterpolateChannels(const RuntimeWalkBundle& bundle,
    double t,
    std::array<double, 13>* outChannels,
    std::string* outError) {
    if (bundle.samples.size() < 2) {
        if (outError) *outError = "Runtime walk bundle requires at least two samples";
        return false;
    }

    if (t <= bundle.samples.front().t) {
        if (outChannels) *outChannels = bundle.samples.front().channels;
        return true;
    }
    if (t >= bundle.samples.back().t) {
        if (outChannels) *outChannels = bundle.samples.back().channels;
        return true;
    }

    for (std::size_t index = 1; index < bundle.samples.size(); ++index) {
        const RuntimeWalkBundleSample& lhs = bundle.samples[index - 1];
        const RuntimeWalkBundleSample& rhs = bundle.samples[index];
        if (t > rhs.t) continue;
        const double span = rhs.t - lhs.t;
        if (!(span > 0.0)) {
            if (outError) *outError = "Runtime walk bundle samples must have strictly increasing t";
            return false;
        }
        const double localT = Clamp01((t - lhs.t) / span);
        if (outChannels) {
            for (std::size_t channel = 0; channel < lhs.channels.size(); ++channel) {
                (*outChannels)[channel] = lhs.channels[channel] + (rhs.channels[channel] - lhs.channels[channel]) * localT;
            }
        }
        return true;
    }

    if (outError) *outError = "Failed to interpolate runtime walk channels";
    return false;
}

bool BuildTickSchedule(const json_min::Value& root, std::vector<double>* outTValues, std::string* outError) {
    outTValues->clear();
    const json_min::Value* explicitValues = root.get("t_values");
    if (explicitValues) {
        if (!explicitValues->is_array()) {
            if (outError) *outError = "t_values must be an array";
            return false;
        }
        const json_min::Array& values = explicitValues->as_array();
        for (std::size_t index = 0; index < values.size(); ++index) {
            double value = 0.0;
            if (!ParseFiniteNumber(values[index], &value, outError, "t_values")) return false;
            outTValues->push_back(value);
        }
    } else {
        int tickCount = 9;
        const json_min::Value* ticksValue = root.get("ticks");
        if (ticksValue) {
            double rawTicks = 0.0;
            if (!ParseFiniteNumber(*ticksValue, &rawTicks, outError, "ticks")) return false;
            if (std::floor(rawTicks) != rawTicks || rawTicks < 2.0 || rawTicks > 4096.0) {
                if (outError) *outError = "ticks must be an integer within [2, 4096]";
                return false;
            }
            tickCount = static_cast<int>(rawTicks);
        }
        outTValues->reserve(static_cast<std::size_t>(tickCount));
        for (int index = 0; index < tickCount; ++index) {
            const double t = (tickCount <= 1) ? 0.0 : static_cast<double>(index) / static_cast<double>(tickCount - 1);
            outTValues->push_back(t);
        }
    }

    if (outTValues->size() < 2) {
        if (outError) *outError = "runtime walk request requires at least two t values";
        return false;
    }
    for (double value : *outTValues) {
        if (!std::isfinite(value)) {
            if (outError) *outError = "runtime walk t values must be finite";
            return false;
        }
    }
    return true;
}

} // namespace

const char* RuntimeWalkAuthorityModeId(RuntimeWalkAuthorityMode mode) {
    switch (mode) {
    case RuntimeWalkAuthorityMode::loaded_base_state:
        return "loaded_base_state";
    case RuntimeWalkAuthorityMode::synthesized_fits_base:
        return "synthesized_fits_base";
    default:
        return "loaded_base_state";
    }
}

bool TryParseRuntimeWalkAuthorityModeId(const std::string& text, RuntimeWalkAuthorityMode* outMode) {
    if (text == "loaded_base_state") {
        if (outMode) *outMode = RuntimeWalkAuthorityMode::loaded_base_state;
        return true;
    }
    if (text == "synthesized_fits_base") {
        if (outMode) *outMode = RuntimeWalkAuthorityMode::synthesized_fits_base;
        return true;
    }
    return false;
}

bool ParseRuntimeWalkBundleJson(const std::string& jsonText,
    RuntimeWalkBundle* outBundle,
    std::string* outError) {
    if (outError) outError->clear();
    if (outBundle) *outBundle = {};

    json_min::ParseResult parsed = json_min::Parse(jsonText);
    if (!parsed.error.empty() || !parsed.value.is_object()) {
        if (outError) *outError = parsed.error.empty() ? "Runtime walk bundle JSON must be an object" : parsed.error;
        return false;
    }

    RuntimeWalkBundle bundle;
    double version = 0.0;
    if (!GetRequiredNumber(parsed.value, "version", &version, outError)) return false;
    if (version != 1.0) {
        if (outError) *outError = "runtime walk bundle version must be 1";
        return false;
    }
    if (!GetRequiredString(parsed.value, "field_name", &bundle.field_name, outError)) return false;
    if (bundle.field_name != "mr_zipper_branch") {
        if (outError) *outError = "runtime walk bundle field_name must be mr_zipper_branch";
        return false;
    }

    const json_min::Value* samplesValue = nullptr;
    if (!GetRequiredArray(parsed.value, "samples", &samplesValue, outError)) return false;
    const json_min::Array& samples = samplesValue->as_array();
    if (samples.size() < 2) {
        if (outError) *outError = "runtime walk bundle requires at least two samples";
        return false;
    }
    bundle.samples.reserve(samples.size());
    double lastT = -1.0e300;
    for (std::size_t index = 0; index < samples.size(); ++index) {
        const json_min::Value& sampleValue = samples[index];
        if (!sampleValue.is_object()) {
            if (outError) *outError = "runtime walk bundle sample entries must be objects";
            return false;
        }
        RuntimeWalkBundleSample sample;
        if (!GetRequiredString(sampleValue, "id", &sample.id, outError)) return false;
        if (!GetRequiredNumber(sampleValue, "t", &sample.t, outError)) return false;
        if (!std::isfinite(sample.t) || sample.t <= lastT) {
            if (outError) *outError = "runtime walk bundle samples must have strictly increasing finite t values";
            return false;
        }
        const json_min::Value* channelsValue = nullptr;
        if (!GetRequiredArray(sampleValue, "channels", &channelsValue, outError)) return false;
        const json_min::Array& channels = channelsValue->as_array();
        if (channels.size() != sample.channels.size()) {
            if (outError) *outError = "runtime walk bundle channels must contain exactly 13 values";
            return false;
        }
        for (std::size_t channel = 0; channel < sample.channels.size(); ++channel) {
            if (!ParseFiniteNumber(channels[channel], &sample.channels[channel], outError, "channels")) return false;
        }
        bundle.samples.push_back(sample);
        lastT = sample.t;
    }

    const json_min::Value* markersValue = parsed.value.get("branch_markers");
    if (markersValue) {
        if (!markersValue->is_array()) {
            if (outError) *outError = "branch_markers must be an array";
            return false;
        }
        const json_min::Array& markers = markersValue->as_array();
        bundle.branch_markers.reserve(markers.size());
        for (std::size_t index = 0; index < markers.size(); ++index) {
            const json_min::Value& markerValue = markers[index];
            if (!markerValue.is_object()) {
                if (outError) *outError = "branch_markers entries must be objects";
                return false;
            }
            RuntimeWalkBranchMarker marker;
            if (!GetRequiredString(markerValue, "id", &marker.id, outError)) return false;
            if (!GetRequiredString(markerValue, "label", &marker.label, outError)) return false;
            if (!GetRequiredString(markerValue, "parent_id", &marker.parent_id, outError)) return false;
            if (!GetRequiredNumber(markerValue, "t", &marker.t, outError)) return false;
            const json_min::Value* stickyValue = markerValue.get("sticky_radius");
            if (stickyValue) {
                if (!ParseFiniteNumber(*stickyValue, &marker.sticky_radius, outError, "sticky_radius")) return false;
            }
            if (!(marker.sticky_radius >= 0.0)) {
                if (outError) *outError = "branch_markers sticky_radius must be >= 0";
                return false;
            }
            bundle.branch_markers.push_back(marker);
        }
    }

    if (outBundle) *outBundle = bundle;
    return true;
}

bool LoadRuntimeWalkBundleFile(const std::string& path,
    RuntimeWalkBundle* outBundle,
    std::string* outError) {
    std::string jsonText;
    if (!ReadTextFile(path, &jsonText, outError)) return false;
    return ParseRuntimeWalkBundleJson(jsonText, outBundle, outError);
}

bool ParseRuntimeWalkRequestJson(const std::string& jsonText,
    RuntimeWalkRequest* outRequest,
    std::string* outError) {
    if (outError) outError->clear();
    if (outRequest) *outRequest = {};

    json_min::ParseResult parsed = json_min::Parse(jsonText);
    if (!parsed.error.empty() || !parsed.value.is_object()) {
        if (outError) *outError = parsed.error.empty() ? "Runtime walk request JSON must be an object" : parsed.error;
        return false;
    }

    RuntimeWalkRequest request;
    double version = 0.0;
    if (!GetRequiredNumber(parsed.value, "version", &version, outError)) return false;
    if (version != 1.0) {
        if (outError) *outError = "runtime walk request version must be 1";
        return false;
    }
    const json_min::Value* authorityModeValue = parsed.value.get("authority_mode");
    if (authorityModeValue) {
        if (!authorityModeValue->is_string()) {
            if (outError) *outError = "authority_mode must be a string";
            return false;
        }
        if (!TryParseRuntimeWalkAuthorityModeId(authorityModeValue->as_string(), &request.authority_mode)) {
            if (outError) *outError = "Unknown runtime walk authority_mode: " + authorityModeValue->as_string();
            return false;
        }
    }
    if (!GetRequiredString(parsed.value, "base_state_json", &request.base_state_json_path, outError)) return false;
    if (!GetRequiredString(parsed.value, "bundle_json", &request.bundle_json_path, outError)) return false;
    if (!GetRequiredString(parsed.value, "out_dir", &request.output_dir, outError)) return false;
    const json_min::Value* comparisonFitsValue = parsed.value.get("comparison_fits");
    if (comparisonFitsValue) {
        if (!comparisonFitsValue->is_string()) {
            if (outError) *outError = "comparison_fits must be a string";
            return false;
        }
        request.comparison_fits_path = comparisonFitsValue->as_string();
    }
    const json_min::Value* rtkManifestValue = parsed.value.get("rtk_manifest_json");
    if (rtkManifestValue) {
        if (!rtkManifestValue->is_string()) {
            if (outError) *outError = "rtk_manifest_json must be a string";
            return false;
        }
        request.rtk_manifest_json_path = rtkManifestValue->as_string();
    }
    const json_min::Value* rtkHarvestValue = parsed.value.get("rtk_harvest_summary_json");
    if (rtkHarvestValue) {
        if (!rtkHarvestValue->is_string()) {
            if (outError) *outError = "rtk_harvest_summary_json must be a string";
            return false;
        }
        request.rtk_harvest_summary_json_path = rtkHarvestValue->as_string();
    }
    const json_min::Value* mappingProfileValue = parsed.value.get("mapping_profile_json");
    if (mappingProfileValue) {
        if (!mappingProfileValue->is_string()) {
            if (outError) *outError = "mapping_profile_json must be a string";
            return false;
        }
        request.mapping_profile_json_path = mappingProfileValue->as_string();
    }
    const json_min::Value* mappingProfileIdValue = parsed.value.get("mapping_profile_id");
    if (mappingProfileIdValue) {
        if (!mappingProfileIdValue->is_string()) {
            if (outError) *outError = "mapping_profile_id must be a string";
            return false;
        }
        request.mapping_profile_id = mappingProfileIdValue->as_string();
    }
    const json_min::Value* orientationInputsValue = parsed.value.get("orientation_inputs_json");
    if (orientationInputsValue) {
        if (!orientationInputsValue->is_string()) {
            if (outError) *outError = "orientation_inputs_json must be a string";
            return false;
        }
        request.orientation_inputs_json_path = orientationInputsValue->as_string();
    }
    if (!BuildTickSchedule(parsed.value, &request.t_values, outError)) return false;

    if (outRequest) *outRequest = request;
    return true;
}

bool LoadRuntimeWalkRequestFile(const std::string& path,
    RuntimeWalkRequest* outRequest,
    std::string* outError) {
    std::string jsonText;
    if (!ReadTextFile(path, &jsonText, outError)) return false;
    return ParseRuntimeWalkRequestJson(jsonText, outRequest, outError);
}

bool EvaluateRuntimeWalkSnapshot(const RuntimeWalkBundle& bundle,
    double t,
    const ViewState& baseView,
    const KernelParams& baseParams,
    const RenderSettings& render,
    RuntimeWalkSnapshot* outSnapshot,
    std::string* outError) {
    if (outError) outError->clear();
    if (!outSnapshot) {
        if (outError) *outError = "Runtime walk snapshot output is required";
        return false;
    }

    RuntimeWalkSnapshot snapshot;
    snapshot.t = t;
    if (!InterpolateChannels(bundle, t, &snapshot.channels, outError)) return false;

    snapshot.seed01 = WeightedSeed01(snapshot.channels);
    snapshot.branch = AnnotateBranch(bundle, t);

    const double baseCombinedSeed = ExplainoSeedCombined(baseView, baseParams);
    const double aspect = render.resolution.y > 0
        ? static_cast<double>(render.resolution.x) / static_cast<double>(render.resolution.y)
        : 1.0;
    const double zoom = SafeZoomFromLog2(baseView.log2_zoom);
    const double worldScale = 2.0 / std::max(1.0e-30, zoom);

    const double dxNorm = (snapshot.channels[0] - snapshot.channels[1]) + 0.5 * (snapshot.channels[9] - snapshot.channels[10]);
    const double dyNorm = (snapshot.channels[2] - snapshot.channels[3]) + 0.5 * (snapshot.channels[7] - snapshot.channels[8]);
    const double zoomNorm = snapshot.channels[4] - snapshot.channels[11];
    const double mixNorm = 0.5 * (snapshot.channels[4] + snapshot.channels[6]);
    const double warpNorm = 0.5 * (snapshot.channels[3] + snapshot.channels[12]);

    snapshot.dx_world = dxNorm * worldScale * aspect * 0.125;
    snapshot.dy_world = dyNorm * worldScale * 0.125;
    snapshot.dlog2_zoom = zoomNorm * 0.75;
    snapshot.center_hp_x = baseView.center_hp_x + snapshot.dx_world;
    snapshot.center_hp_y = baseView.center_hp_y + snapshot.dy_world;
    snapshot.log2_zoom = ClampD(baseView.log2_zoom + snapshot.dlog2_zoom, Log2D(kMinZoom), kMaxLog2Zoom);
    snapshot.phase = static_cast<double>(baseView.explaino_phase) + (snapshot.channels[5] - 0.5) * 3.14159265358979323846;
    snapshot.combined_seed = baseCombinedSeed + (snapshot.seed01 - 0.5) * 32.0;
    snapshot.seed_b = static_cast<double>(baseParams.explaino_seed_b) + (snapshot.channels[7] - snapshot.channels[8]) * 0.5;
    snapshot.mix = ClampD(static_cast<double>(baseParams.explaino_mix) + (mixNorm - 0.5) * 1.5, 0.0, 4.0);
    snapshot.warp_strength = ClampD(static_cast<double>(baseParams.explaino_warp_strength) + warpNorm * 0.35, 0.0, 1.0);

    *outSnapshot = snapshot;
    return true;
}

void ApplyRuntimeWalkSnapshot(const RuntimeWalkSnapshot& snapshot,
    ViewState* ioView,
    KernelParams* ioParams) {
    if (!ioView || !ioParams) return;
    ioView->center_hp_x = snapshot.center_hp_x;
    ioView->center_hp_y = snapshot.center_hp_y;
    ioView->log2_zoom = snapshot.log2_zoom;
    SyncViewUiFromHp(*ioView);
    ioView->explaino_phase = static_cast<float>(snapshot.phase);
    ExplainoSeedSetCombined(*ioView, *ioParams, snapshot.combined_seed);
    ioParams->explaino_seed_b = snapshot.seed_b;
    ioParams->explaino_mix = static_cast<float>(snapshot.mix);
    ioParams->explaino_warp_strength = static_cast<float>(snapshot.warp_strength);
}
