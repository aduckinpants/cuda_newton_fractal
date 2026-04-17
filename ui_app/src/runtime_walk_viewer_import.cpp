#include "runtime_walk_viewer_import.h"

#include "fractal_family_rules.h"
#include "enum_id_utils.h"
#include "json_min.h"
#include "runtime_walk.h"
#include "runtime_walk_bootstrap.h"
#include "runtime_walk_viewer_session.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace {

using ImportRecords = std::vector<RuntimeWalkViewerImportSessionRecord>;

std::filesystem::path ResolveAbsolutePath(const std::string& path, const std::filesystem::path& relativeRoot = {}) {
    if (path.empty()) return {};
    std::filesystem::path candidate(path);
    if (candidate.is_absolute()) return candidate.lexically_normal();
    if (!relativeRoot.empty()) return (relativeRoot / candidate).lexically_normal();
    return std::filesystem::absolute(candidate).lexically_normal();
}

std::string NormalizePathString(const std::filesystem::path& path) {
    if (path.empty()) return {};
    return path.lexically_normal().string();
}

bool ReadTextFile(const std::filesystem::path& path, std::string* outText, std::string* outError) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        if (outError) *outError = "Failed to open import-session file: " + path.string();
        return false;
    }
    std::ostringstream text;
    text << file.rdbuf();
    if (!file.good() && !file.eof()) {
        if (outError) *outError = "Failed to read import-session file: " + path.string();
        return false;
    }
    if (outText) *outText = text.str();
    return true;
}

bool IsUsableImportSessionRecord(const RuntimeWalkViewerImportSessionRecord& record) {
    if (record.request_json_path.empty() || !std::filesystem::exists(record.request_json_path)) return false;
    if (!record.bundle_json_path.empty() && !std::filesystem::exists(record.bundle_json_path)) return false;
    if (!record.base_state_json_path.empty() && !std::filesystem::exists(record.base_state_json_path)) return false;
    if (!record.synthesized_base_state_json_path.empty() && !std::filesystem::exists(record.synthesized_base_state_json_path)) return false;
    return true;
}

bool WriteTextFile(const std::filesystem::path& path, const std::string& text, std::string* outError) {
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    if (ec) {
        if (outError) *outError = "Failed to create import-session directory: " + path.parent_path().string();
        return false;
    }
    std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file) {
        if (outError) *outError = "Failed to write import-session file: " + path.string();
        return false;
    }
    file << text;
    if (!file.good()) {
        if (outError) *outError = "Failed to flush import-session file: " + path.string();
        return false;
    }
    return true;
}

std::string JsonEscape(const std::string& value) {
    std::string out;
    out.reserve(value.size() + 8u);
    for (unsigned char ch : value) {
        switch (ch) {
        case '\\': out += "\\\\"; break;
        case '"': out += "\\\""; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if (ch < 0x20u) {
                char buffer[7];
                std::snprintf(buffer, sizeof(buffer), "\\u%04x", static_cast<unsigned int>(ch));
                out += buffer;
            } else {
                out.push_back(static_cast<char>(ch));
            }
            break;
        }
    }
    return out;
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

bool GetOptionalString(const json_min::Value& object, const char* key, std::string* outValue) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_string()) return false;
    if (outValue) *outValue = value->as_string();
    return true;
}

bool GetOptionalNumber(const json_min::Value& object, const char* key, double* outValue, std::string* outError) {
    const json_min::Value* value = object.get(key);
    if (!value) return true;
    if (!value->is_number()) {
        if (outError) *outError = std::string("Invalid number field: ") + key;
        return false;
    }
    const double number = value->as_number();
    if (!std::isfinite(number)) {
        if (outError) *outError = std::string("Non-finite number field: ") + key;
        return false;
    }
    if (outValue) *outValue = number;
    return true;
}

std::filesystem::path SessionsRoot(const std::string& exeDir) {
    return ResolveAbsolutePath(exeDir) / "diagnostics" / "runtime_walk_sessions";
}

std::filesystem::path RecentIndexPath(const std::string& exeDir) {
    return SessionsRoot(exeDir) / "recent_sessions.json";
}

std::filesystem::path ImportSelectionManifestPath(const std::string& sessionDir) {
    return ResolveAbsolutePath(sessionDir) / "runtime_walk_import_selection_manifest.json";
}

std::filesystem::path ImportReceiptPath(const std::string& sessionDir) {
    return ResolveAbsolutePath(sessionDir) / "runtime_walk_import_receipt.json";
}

std::string BuildTimestampUtc() {
    const auto now = std::chrono::system_clock::now();
    const std::time_t raw = std::chrono::system_clock::to_time_t(now);
    std::tm utc{};
#if defined(_WIN32)
    gmtime_s(&utc, &raw);
#else
    gmtime_r(&raw, &utc);
#endif
    std::ostringstream out;
    out << std::put_time(&utc, "%Y-%m-%dT%H:%M:%SZ");
    return out.str();
}

std::string Hex64(std::uint64_t value) {
    std::ostringstream out;
    out << std::hex << std::setw(16) << std::setfill('0') << value;
    return out.str();
}

std::string StableSessionId(const RuntimeWalkViewerImportRequest& request,
    const std::string& resolvedBundlePath,
    const std::string& resolvedFitsPath) {
    constexpr std::uint64_t kOffset = 1469598103934665603ull;
    constexpr std::uint64_t kPrime = 1099511628211ull;
    std::uint64_t hash = kOffset;
    const std::array<std::string, 12> fields = {
        RuntimeWalkAuthorityModeId(request.authority_mode),
        NormalizePathString(ResolveAbsolutePath(request.base_state_json_path)),
        NormalizePathString(ResolveAbsolutePath(resolvedBundlePath)),
        NormalizePathString(ResolveAbsolutePath(resolvedFitsPath)),
        NormalizePathString(ResolveAbsolutePath(request.mapping_profile_json_path)),
        request.mapping_profile_id,
        NormalizePathString(ResolveAbsolutePath(request.orientation_inputs_json_path)),
        NormalizePathString(ResolveAbsolutePath(request.rtk_manifest_json_path)),
        NormalizePathString(ResolveAbsolutePath(request.rtk_harvest_summary_json_path)),
        std::to_string(request.transport_options.sample_count),
        std::to_string(request.transport_options.motion_scale),
        std::to_string(request.transport_options.warp_scale),
    };
    for (const std::string& field : fields) {
        for (unsigned char ch : field) {
            hash ^= static_cast<std::uint64_t>(ch);
            hash *= kPrime;
        }
        hash ^= 0xffu;
        hash *= kPrime;
    }
    return Hex64(hash).substr(0, 12);
}

bool ParseRecentIndexJson(const std::string& jsonText,
    const std::filesystem::path& indexDir,
    ImportRecords* outRecords,
    std::string* outError) {
    if (outError) outError->clear();
    if (outRecords) outRecords->clear();

    json_min::ParseResult parsed = json_min::Parse(jsonText);
    if (!parsed.error.empty() || !parsed.value.is_object()) {
        if (outError) *outError = "Failed to parse runtime-walk recent-session index JSON";
        return false;
    }

    const json_min::Value* sessions = parsed.value.get("sessions");
    if (!sessions || !sessions->is_array()) {
        if (outError) *outError = "runtime-walk recent-session index requires array field: sessions";
        return false;
    }

    ImportRecords records;
    for (const json_min::Value& entry : sessions->as_array()) {
        if (!entry.is_object()) {
            if (outError) *outError = "runtime-walk recent-session entries must be objects";
            return false;
        }

        RuntimeWalkViewerImportSessionRecord record;
        if (!GetRequiredString(entry, "session_id", &record.session_id, outError)) return false;
        std::string sessionDir;
        if (!GetRequiredString(entry, "session_dir", &sessionDir, outError)) return false;
        std::string requestPath;
        if (!GetRequiredString(entry, "request_json", &requestPath, outError)) return false;
        std::string authorityModeText;
        if (GetOptionalString(entry, "authority_mode", &authorityModeText)) {
            if (!TryParseRuntimeWalkAuthorityModeId(authorityModeText, &record.authority_mode)) {
                if (outError) *outError = "Unknown runtime-walk import recent-session authority_mode: " + authorityModeText;
                return false;
            }
        }
        GetOptionalString(entry, "base_state_json", &record.base_state_json_path);
        GetOptionalString(entry, "synthesized_base_state_json", &record.synthesized_base_state_json_path);
        GetOptionalString(entry, "comparison_fits", &record.comparison_fits_path);
        GetOptionalString(entry, "bundle_json", &record.bundle_json_path);
        GetOptionalString(entry, "rtk_manifest_json", &record.rtk_manifest_json_path);
        GetOptionalString(entry, "rtk_harvest_summary_json", &record.rtk_harvest_summary_json_path);
        GetOptionalString(entry, "mapping_profile_json", &record.mapping_profile_json_path);
        GetOptionalString(entry, "mapping_profile_id", &record.mapping_profile_id);
        GetOptionalString(entry, "orientation_inputs_json", &record.orientation_inputs_json_path);
        GetOptionalString(entry, "source_request_json", &record.source_request_json_path);
        GetOptionalString(entry, "source_bundle_json", &record.source_bundle_json_path);
        GetOptionalString(entry, "discovery_source", &record.discovery_source);
        GetOptionalString(entry, "transport_generation_mode", &record.transport_generation_mode);
        const json_min::Value* sampleCountValue = entry.get("transport_sample_count");
        if (sampleCountValue && sampleCountValue->is_number()) {
            record.transport_sample_count = static_cast<std::size_t>(std::max(0.0, sampleCountValue->as_number()));
        }
        const json_min::Value* generatedValue = entry.get("transport_generated");
        if (generatedValue && generatedValue->is_bool()) {
            record.transport_generated = generatedValue->as_bool();
        }
        if (!GetOptionalNumber(entry, "transport_motion_scale", &record.transport_motion_scale, outError)) return false;
        if (!GetOptionalNumber(entry, "transport_warp_scale", &record.transport_warp_scale, outError)) return false;
        const json_min::Value* loadSucceededValue = entry.get("viewer_load_succeeded");
        if (loadSucceededValue && loadSucceededValue->is_bool()) {
            record.viewer_load_succeeded = loadSucceededValue->as_bool();
        }
        record.session_dir = NormalizePathString(ResolveAbsolutePath(sessionDir, indexDir));
        record.request_json_path = NormalizePathString(ResolveAbsolutePath(requestPath, indexDir));
        record.request_exists = IsUsableImportSessionRecord(record);
        records.push_back(record);
    }

    if (outRecords) *outRecords = records;
    return true;
}

bool LoadRecentIndex(const std::string& exeDir, ImportRecords* outRecords, std::string* outError) {
    if (outError) outError->clear();
    if (outRecords) outRecords->clear();
    const std::filesystem::path path = RecentIndexPath(exeDir);
    if (!std::filesystem::exists(path)) return true;
    std::string text;
    if (!ReadTextFile(path, &text, outError)) return false;
    return ParseRecentIndexJson(text, path.parent_path(), outRecords, outError);
}

const RuntimeWalkViewerImportSessionRecord* FindMatchingRecentRecord(const ImportRecords& records,
    const std::string& requestJsonPath) {
    const std::string normalizedRequest = NormalizePathString(ResolveAbsolutePath(requestJsonPath));
    if (normalizedRequest.empty()) return nullptr;
    for (const RuntimeWalkViewerImportSessionRecord& record : records) {
        if (NormalizePathString(ResolveAbsolutePath(record.request_json_path)) == normalizedRequest) {
            return &record;
        }
    }
    return nullptr;
}

std::string SerializeRecentIndex(const ImportRecords& records) {
    std::ostringstream out;
    out << "{\n";
    out << "  \"version\": 1,\n";
    out << "  \"latest_session_id\": \"" << JsonEscape(records.empty() ? std::string() : records.front().session_id) << "\",\n";
    out << "  \"sessions\": [\n";
    for (std::size_t index = 0; index < records.size(); ++index) {
        const RuntimeWalkViewerImportSessionRecord& record = records[index];
        out << "    {\n";
        out << "      \"session_id\": \"" << JsonEscape(record.session_id) << "\",\n";
        out << "      \"session_dir\": \"" << JsonEscape(record.session_dir) << "\",\n";
        out << "      \"request_json\": \"" << JsonEscape(record.request_json_path) << "\",\n";
        out << "      \"authority_mode\": \"" << JsonEscape(RuntimeWalkAuthorityModeId(record.authority_mode)) << "\",\n";
        out << "      \"base_state_json\": \"" << JsonEscape(record.base_state_json_path) << "\",\n";
        out << "      \"synthesized_base_state_json\": \"" << JsonEscape(record.synthesized_base_state_json_path) << "\",\n";
        out << "      \"comparison_fits\": \"" << JsonEscape(record.comparison_fits_path) << "\",\n";
        out << "      \"bundle_json\": \"" << JsonEscape(record.bundle_json_path) << "\",\n";
        out << "      \"rtk_manifest_json\": \"" << JsonEscape(record.rtk_manifest_json_path) << "\",\n";
        out << "      \"rtk_harvest_summary_json\": \"" << JsonEscape(record.rtk_harvest_summary_json_path) << "\",\n";
        out << "      \"mapping_profile_json\": \"" << JsonEscape(record.mapping_profile_json_path) << "\",\n";
        out << "      \"mapping_profile_id\": \"" << JsonEscape(record.mapping_profile_id) << "\",\n";
        out << "      \"orientation_inputs_json\": \"" << JsonEscape(record.orientation_inputs_json_path) << "\",\n";
        out << "      \"source_request_json\": \"" << JsonEscape(record.source_request_json_path) << "\",\n";
        out << "      \"source_bundle_json\": \"" << JsonEscape(record.source_bundle_json_path) << "\",\n";
        out << "      \"discovery_source\": \"" << JsonEscape(record.discovery_source) << "\",\n";
        out << "      \"transport_generation_mode\": \"" << JsonEscape(record.transport_generation_mode) << "\",\n";
        out << "      \"transport_sample_count\": " << record.transport_sample_count << ",\n";
        out << "      \"transport_generated\": " << (record.transport_generated ? "true" : "false") << ",\n";
        out << "      \"transport_motion_scale\": " << record.transport_motion_scale << ",\n";
        out << "      \"transport_warp_scale\": " << record.transport_warp_scale << ",\n";
        out << "      \"viewer_load_succeeded\": " << (record.viewer_load_succeeded ? "true" : "false") << "\n";
        out << "    }";
        if (index + 1u < records.size()) out << ",";
        out << "\n";
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

bool WriteRecentIndex(const std::string& exeDir, const ImportRecords& records, std::string* outError) {
    return WriteTextFile(RecentIndexPath(exeDir), SerializeRecentIndex(records), outError);
}

void UpsertRecentRecord(const RuntimeWalkViewerImportSessionRecord& record, ImportRecords* ioRecords) {
    if (!ioRecords) return;
    ioRecords->erase(
        std::remove_if(ioRecords->begin(), ioRecords->end(),
            [&](const RuntimeWalkViewerImportSessionRecord& existing) {
                return existing.session_id == record.session_id;
            }),
        ioRecords->end());
    ioRecords->insert(ioRecords->begin(), record);
    constexpr std::size_t kMaxRecords = 8u;
    if (ioRecords->size() > kMaxRecords) {
        ioRecords->resize(kMaxRecords);
    }
}

bool LoadBundleTValues(const std::string& bundlePath, std::vector<double>* outTValues, std::string* outError) {
    if (outError) outError->clear();
    if (outTValues) outTValues->clear();
    RuntimeWalkBundle bundle;
    if (!LoadRuntimeWalkBundleFile(bundlePath, &bundle, outError)) return false;
    if (bundle.samples.size() < 2u) {
        if (outError) *outError = "runtime-walk import bundle requires at least two samples";
        return false;
    }
    if (outTValues) {
        outTValues->reserve(bundle.samples.size());
        for (const RuntimeWalkBundleSample& sample : bundle.samples) {
            outTValues->push_back(sample.t);
        }
    }
    return true;
}

bool ReadTransportSampleCount(const std::string& bundlePath,
    std::size_t* outCount,
    std::string* outError) {
    if (outError) outError->clear();
    if (outCount) *outCount = 0;
    RuntimeWalkBundle bundle;
    if (!LoadRuntimeWalkBundleFile(bundlePath, &bundle, outError)) return false;
    if (outCount) *outCount = bundle.samples.size();
    return true;
}

std::string SerializeRuntimeWalkRequestJson(const RuntimeWalkRequest& request) {
    std::ostringstream out;
    out << "{\n";
    out << "  \"version\": 1,\n";
    out << "  \"authority_mode\": \"" << JsonEscape(RuntimeWalkAuthorityModeId(request.authority_mode)) << "\",\n";
    out << "  \"base_state_json\": \"" << JsonEscape(request.base_state_json_path) << "\",\n";
    out << "  \"bundle_json\": \"" << JsonEscape(request.bundle_json_path) << "\",\n";
    out << "  \"out_dir\": \"" << JsonEscape(request.output_dir) << "\",\n";
    out << "  \"ticks\": " << request.t_values.size();
    if (!request.comparison_fits_path.empty()) {
        out << ",\n  \"comparison_fits\": \"" << JsonEscape(request.comparison_fits_path) << "\"";
    }
    if (!request.rtk_manifest_json_path.empty()) {
        out << ",\n  \"rtk_manifest_json\": \"" << JsonEscape(request.rtk_manifest_json_path) << "\"";
    }
    if (!request.rtk_harvest_summary_json_path.empty()) {
        out << ",\n  \"rtk_harvest_summary_json\": \"" << JsonEscape(request.rtk_harvest_summary_json_path) << "\"";
    }
    if (!request.mapping_profile_json_path.empty()) {
        out << ",\n  \"mapping_profile_json\": \"" << JsonEscape(request.mapping_profile_json_path) << "\"";
    }
    if (!request.mapping_profile_id.empty()) {
        out << ",\n  \"mapping_profile_id\": \"" << JsonEscape(request.mapping_profile_id) << "\"";
    }
    if (!request.orientation_inputs_json_path.empty()) {
        out << ",\n  \"orientation_inputs_json\": \"" << JsonEscape(request.orientation_inputs_json_path) << "\"";
    }
    if (request.transport_generated) {
        out << ",\n  \"transport_generated\": true";
    }
    if (!request.transport_generation_mode.empty()) {
        out << ",\n  \"transport_generation_mode\": \"" << JsonEscape(request.transport_generation_mode) << "\"";
    }
    if (request.transport_sample_count > 0u) {
        out << ",\n  \"transport_sample_count\": " << request.transport_sample_count;
    }
    out << ",\n  \"transport_motion_scale\": " << request.transport_motion_scale;
    out << ",\n  \"transport_warp_scale\": " << request.transport_warp_scale;
    out << "\n}\n";
    return out.str();
}

std::string SerializeImportSelectionManifest(const RuntimeWalkViewerImportSessionRecord& record,
    const RuntimeWalkRequest& request) {
    std::ostringstream out;
    out << "{\n";
    out << "  \"version\": 1,\n";
    out << "  \"session_id\": \"" << JsonEscape(record.session_id) << "\",\n";
    out << "  \"authority_mode\": \"" << JsonEscape(RuntimeWalkAuthorityModeId(record.authority_mode)) << "\",\n";
    out << "  \"discovery_source\": \"" << JsonEscape(record.discovery_source) << "\",\n";
    out << "  \"base_state_json\": \"" << JsonEscape(request.base_state_json_path) << "\",\n";
    out << "  \"synthesized_base_state_json\": \"" << JsonEscape(record.synthesized_base_state_json_path) << "\",\n";
    out << "  \"comparison_fits\": \"" << JsonEscape(record.comparison_fits_path) << "\",\n";
    out << "  \"source_request_json\": \"" << JsonEscape(record.source_request_json_path) << "\",\n";
    out << "  \"source_bundle_json\": \"" << JsonEscape(record.source_bundle_json_path) << "\",\n";
    out << "  \"generated_request_json\": \"" << JsonEscape(record.request_json_path) << "\",\n";
    out << "  \"bundle_json\": \"" << JsonEscape(request.bundle_json_path) << "\",\n";
    out << "  \"transport_generated\": " << (record.transport_generated ? "true" : "false") << ",\n";
    out << "  \"transport_generation_mode\": \"" << JsonEscape(record.transport_generation_mode) << "\",\n";
    out << "  \"transport_sample_count\": " << record.transport_sample_count << ",\n";
    out << "  \"transport_motion_scale\": " << record.transport_motion_scale << ",\n";
    out << "  \"transport_warp_scale\": " << record.transport_warp_scale << ",\n";
    out << "  \"viewer_load_succeeded\": " << (record.viewer_load_succeeded ? "true" : "false") << ",\n";
    out << "  \"mapping_profile_json\": \"" << JsonEscape(record.mapping_profile_json_path) << "\",\n";
    out << "  \"mapping_profile_id\": \"" << JsonEscape(record.mapping_profile_id) << "\",\n";
    out << "  \"orientation_inputs_json\": \"" << JsonEscape(record.orientation_inputs_json_path) << "\",\n";
    out << "  \"rtk_manifest_json\": \"" << JsonEscape(record.rtk_manifest_json_path) << "\",\n";
    out << "  \"rtk_harvest_summary_json\": \"" << JsonEscape(record.rtk_harvest_summary_json_path) << "\"\n";
    out << "}\n";
    return out.str();
}

std::string SerializeImportReceipt(const RuntimeWalkViewerImportSessionRecord& record) {
    std::ostringstream out;
    out << "{\n";
    out << "  \"version\": 1,\n";
    out << "  \"ok\": true,\n";
    out << "  \"created_utc\": \"" << BuildTimestampUtc() << "\",\n";
    out << "  \"session_id\": \"" << JsonEscape(record.session_id) << "\",\n";
    out << "  \"authority_mode\": \"" << JsonEscape(RuntimeWalkAuthorityModeId(record.authority_mode)) << "\",\n";
    out << "  \"request_json\": \"" << JsonEscape(record.request_json_path) << "\",\n";
    out << "  \"transport_generated\": " << (record.transport_generated ? "true" : "false") << ",\n";
    out << "  \"transport_generation_mode\": \"" << JsonEscape(record.transport_generation_mode) << "\",\n";
    out << "  \"transport_sample_count\": " << record.transport_sample_count << ",\n";
    out << "  \"transport_motion_scale\": " << record.transport_motion_scale << ",\n";
    out << "  \"transport_warp_scale\": " << record.transport_warp_scale << ",\n";
    out << "  \"viewer_load_succeeded\": " << (record.viewer_load_succeeded ? "true" : "false") << "\n";
    out << "}\n";
    return out.str();
}

struct ResolvedImportSources {
    std::string request_json_path;
    std::string bundle_json_path;
    std::string comparison_fits_path;
    std::string rtk_manifest_json_path;
    std::string rtk_harvest_summary_json_path;
    std::string source_request_json_path;
    std::string source_bundle_json_path;
    std::string discovery_source;
    std::string transport_generation_mode;
    std::vector<double> t_values;
    std::size_t transport_sample_count = 0;
    RuntimeWalkTransportSynthesisOptions transport_options{};
    bool transport_generated = false;
};

bool ResolveMappingProfilePath(const RuntimeWalkViewerImportRequest& request,
    std::string* outPath,
    std::string* outError) {
    if (!request.mapping_profile_json_path.empty()) {
        if (outPath) *outPath = NormalizePathString(ResolveAbsolutePath(request.mapping_profile_json_path));
        return true;
    }
    return ResolveDefaultRuntimeWalkFitsMappingProfilePath(request.exe_dir, outPath, outError);
}

std::string ResolveMappingProfileId(const RuntimeWalkViewerImportRequest& request) {
    if (!request.mapping_profile_id.empty()) return request.mapping_profile_id;
    return "explaino_default";
}

void ApplyDefaultTransportOptions(RuntimeWalkTransportSynthesisOptions* ioOptions) {
    if (!ioOptions) return;
    if (ioOptions->sample_count < 9u) ioOptions->sample_count = 33u;
    if (!(ioOptions->motion_scale > 0.0)) ioOptions->motion_scale = 0.75;
    if (ioOptions->warp_scale < 0.0) ioOptions->warp_scale = 0.0;
    if (ioOptions->warp_scale > 1.0) ioOptions->warp_scale = 1.0;
}

bool RefreshMappingProfileDisplayStateInternal(const std::string& exeDir,
    RuntimeWalkViewerImportPanelState* ioPanel,
    std::string* outError) {
    if (outError) outError->clear();
    if (!ioPanel) {
        if (outError) *outError = "Runtime-walk FITS import panel output is required";
        return false;
    }

    ioPanel->mapping_binding_summaries.clear();
    ioPanel->resolved_mapping_profile_json_path.clear();
    ioPanel->resolved_mapping_profile_base_fractal_type.clear();

    RuntimeWalkViewerImportRequest request{};
    request.exe_dir = exeDir;
    request.mapping_profile_json_path = ioPanel->mapping_profile_json_path;
    request.mapping_profile_id = ioPanel->mapping_profile_id;

    std::string mappingProfilePath;
    if (!ResolveMappingProfilePath(request, &mappingProfilePath, outError)) return false;
    const std::string mappingProfileId = ResolveMappingProfileId(request);

    RuntimeWalkFitsMappingCatalog catalog;
    if (!LoadRuntimeWalkFitsMappingCatalogFile(mappingProfilePath, &catalog, outError)) return false;

    const RuntimeWalkFitsMappingProfile* matchedProfile = nullptr;
    for (const RuntimeWalkFitsMappingProfile& profile : catalog.profiles) {
        if (profile.id == mappingProfileId) {
            matchedProfile = &profile;
            break;
        }
    }
    if (!matchedProfile) {
        if (outError) *outError = "Unknown runtime-walk FITS mapping profile: " + mappingProfileId;
        return false;
    }

    ioPanel->mapping_profile_id = matchedProfile->id;
    ioPanel->resolved_mapping_profile_json_path = mappingProfilePath;
    ioPanel->resolved_mapping_profile_base_fractal_type = FractalTypeId(matchedProfile->base_fractal_type);
    for (const RuntimeWalkFitsMappingBinding& binding : matchedProfile->bindings) {
        std::ostringstream line;
        line.setf(std::ios::fixed);
        line.precision(3);
        line << binding.source_signal << " -> " << binding.target_path
             << " | selector=" << binding.target_selector
             << " | weight=" << binding.weight
             << " | scale=" << binding.scale
             << " | offset=" << binding.offset;
        if (binding.has_clamp) {
            line << " | clamp=[" << binding.clamp_min << ", " << binding.clamp_max << "]";
        }
        if (!binding.enabled) {
            line << " | disabled";
        }
        ioPanel->mapping_binding_summaries.push_back(line.str());
    }
    return true;
}

bool TryResolveRecentMatch(const RuntimeWalkViewerImportRequest& request,
    const ImportRecords& records,
    ResolvedImportSources* outResolved) {
    if (!outResolved) return false;
    if (request.comparison_fits_path.empty()) return false;
    const std::string normalizedFits = NormalizePathString(ResolveAbsolutePath(request.comparison_fits_path));
    for (const RuntimeWalkViewerImportSessionRecord& record : records) {
        if (NormalizePathString(ResolveAbsolutePath(record.comparison_fits_path)) != normalizedFits) continue;
        if (!record.request_json_path.empty() && std::filesystem::exists(record.request_json_path)) {
            outResolved->request_json_path = NormalizePathString(ResolveAbsolutePath(record.request_json_path));
            outResolved->discovery_source = "recent_match_request";
            outResolved->transport_generation_mode = record.transport_generation_mode;
            outResolved->transport_sample_count = record.transport_sample_count;
            return true;
        }
        if (!record.bundle_json_path.empty() && std::filesystem::exists(record.bundle_json_path)) {
            outResolved->bundle_json_path = NormalizePathString(ResolveAbsolutePath(record.bundle_json_path));
            outResolved->discovery_source = "recent_match_bundle";
            outResolved->transport_generation_mode = record.transport_generation_mode;
            outResolved->transport_sample_count = record.transport_sample_count;
            return true;
        }
    }
    return false;
}

bool ShouldForceGeneratedTransport(const RuntimeWalkViewerImportRequest& request) {
    return request.authority_mode == RuntimeWalkAuthorityMode::synthesized_fits_base &&
        request.request_json_path.empty() &&
        request.bundle_json_path.empty();
}

bool TryResolveSiblingCandidates(const RuntimeWalkViewerImportRequest& request, ResolvedImportSources* outResolved) {
    if (!outResolved) return false;
    if (request.comparison_fits_path.empty()) return false;
    const std::filesystem::path fitsPath = ResolveAbsolutePath(request.comparison_fits_path);
    if (fitsPath.empty()) return false;
    const std::array<const char*, 2> requestNames = {
        "runtime_walk_viewer_request.json",
        "runtime_walk_request.json",
    };
    const std::array<const char*, 2> bundleNames = {
        "mr_zipper_branch.json",
        "bundle.json",
    };
    for (std::filesystem::path dir = fitsPath.parent_path();
         !dir.empty() && dir.has_relative_path();
         dir = dir.parent_path()) {
        for (const char* name : requestNames) {
            const std::filesystem::path candidate = dir / name;
            if (std::filesystem::exists(candidate)) {
                outResolved->request_json_path = NormalizePathString(candidate);
                outResolved->discovery_source = "sibling_request";
                return true;
            }
        }
        for (const char* name : bundleNames) {
            const std::filesystem::path candidate = dir / name;
            if (std::filesystem::exists(candidate)) {
                outResolved->bundle_json_path = NormalizePathString(candidate);
                outResolved->discovery_source = "sibling_bundle";
                return true;
            }
        }
        if (dir == dir.root_path()) break;
    }
    return false;
}

bool ResolveImportSources(const RuntimeWalkViewerImportRequest& request,
    ResolvedImportSources* outResolved,
    std::string* outError) {
    if (outError) outError->clear();
    if (!outResolved) {
        if (outError) *outError = "Runtime walk viewer import resolution output is required";
        return false;
    }
    *outResolved = {};
    outResolved->transport_options = request.transport_options;
    ApplyDefaultTransportOptions(&outResolved->transport_options);

    if (!ValidateRuntimeWalkViewerImportBaseState(request.authority_mode, request.base_state_json_path, request.base_fractal_type, outError)) {
        return false;
    }

    ImportRecords records;
    if (!LoadRecentIndex(request.exe_dir, &records, outError)) return false;

    if (!request.request_json_path.empty()) {
        outResolved->request_json_path = NormalizePathString(ResolveAbsolutePath(request.request_json_path));
        outResolved->discovery_source = "explicit_request";
    } else if (!request.bundle_json_path.empty()) {
        outResolved->bundle_json_path = NormalizePathString(ResolveAbsolutePath(request.bundle_json_path));
        outResolved->discovery_source = "explicit_bundle";
    } else if (ShouldForceGeneratedTransport(request) && !request.comparison_fits_path.empty()) {
        outResolved->comparison_fits_path = NormalizePathString(ResolveAbsolutePath(request.comparison_fits_path));
        outResolved->rtk_manifest_json_path = NormalizePathString(ResolveAbsolutePath(request.rtk_manifest_json_path));
        outResolved->rtk_harvest_summary_json_path = NormalizePathString(ResolveAbsolutePath(request.rtk_harvest_summary_json_path));
        outResolved->discovery_source = "generated_transport";
        outResolved->transport_generated = true;
        return true;
    } else if (TryResolveRecentMatch(request, records, outResolved)) {
    } else if (TryResolveSiblingCandidates(request, outResolved)) {
    } else if (!request.comparison_fits_path.empty()) {
        outResolved->comparison_fits_path = NormalizePathString(ResolveAbsolutePath(request.comparison_fits_path));
        outResolved->rtk_manifest_json_path = NormalizePathString(ResolveAbsolutePath(request.rtk_manifest_json_path));
        outResolved->rtk_harvest_summary_json_path = NormalizePathString(ResolveAbsolutePath(request.rtk_harvest_summary_json_path));
        outResolved->discovery_source = "generated_transport";
        outResolved->transport_generated = true;
        return true;
    } else {
        if (outError) *outError = "Load FITS requires a FITS path or an authored runtime-walk request/bundle";
        return false;
    }

    RuntimeWalkRequest sourceRequest{};
    std::filesystem::path requestDir;
    if (!outResolved->request_json_path.empty()) {
        if (!LoadRuntimeWalkRequestFile(outResolved->request_json_path, &sourceRequest, outError)) return false;
        requestDir = std::filesystem::path(outResolved->request_json_path).parent_path();
        outResolved->source_request_json_path = outResolved->request_json_path;
        outResolved->source_bundle_json_path = NormalizePathString(
            ResolveAbsolutePath(sourceRequest.bundle_json_path, requestDir));
        outResolved->bundle_json_path = outResolved->source_bundle_json_path;
        outResolved->t_values = sourceRequest.t_values;
        if (!ReadTransportSampleCount(outResolved->bundle_json_path, &outResolved->transport_sample_count, outError)) return false;
        outResolved->comparison_fits_path = !request.comparison_fits_path.empty()
            ? NormalizePathString(ResolveAbsolutePath(request.comparison_fits_path))
            : NormalizePathString(ResolveAbsolutePath(sourceRequest.comparison_fits_path, requestDir));
        outResolved->rtk_manifest_json_path = !request.rtk_manifest_json_path.empty()
            ? NormalizePathString(ResolveAbsolutePath(request.rtk_manifest_json_path))
            : NormalizePathString(ResolveAbsolutePath(sourceRequest.rtk_manifest_json_path, requestDir));
        outResolved->rtk_harvest_summary_json_path = !request.rtk_harvest_summary_json_path.empty()
            ? NormalizePathString(ResolveAbsolutePath(request.rtk_harvest_summary_json_path))
            : NormalizePathString(ResolveAbsolutePath(sourceRequest.rtk_harvest_summary_json_path, requestDir));
    } else {
        outResolved->bundle_json_path = NormalizePathString(ResolveAbsolutePath(outResolved->bundle_json_path));
        outResolved->source_bundle_json_path = outResolved->bundle_json_path;
        outResolved->comparison_fits_path = NormalizePathString(ResolveAbsolutePath(request.comparison_fits_path));
        outResolved->rtk_manifest_json_path = NormalizePathString(ResolveAbsolutePath(request.rtk_manifest_json_path));
        outResolved->rtk_harvest_summary_json_path = NormalizePathString(ResolveAbsolutePath(request.rtk_harvest_summary_json_path));
        if (!LoadBundleTValues(outResolved->bundle_json_path, &outResolved->t_values, outError)) return false;
        outResolved->transport_sample_count = outResolved->t_values.size();
    }

    if (outResolved->t_values.size() < 2u) {
        if (outError) *outError = "Load FITS requires a runtime-walk request or bundle with at least two t values";
        return false;
    }
    return true;
}

} // namespace

bool RefreshMappingProfileDisplayState(const std::string& exeDir,
    RuntimeWalkViewerImportPanelState* ioPanel,
    std::string* outError) {
    return RefreshMappingProfileDisplayStateInternal(exeDir, ioPanel, outError);
}

bool ValidateRuntimeWalkViewerImportBaseState(RuntimeWalkAuthorityMode authorityMode,
    const std::string& baseStateJsonPath,
    FractalType baseStateFractalType,
    std::string* outError) {
    if (outError) outError->clear();
    if (authorityMode == RuntimeWalkAuthorityMode::synthesized_fits_base) {
        return true;
    }
    if (baseStateJsonPath.empty()) {
        if (outError) *outError = "Load FITS requires a loaded capture state first";
        return false;
    }
    if (!IsExplainoFamily(baseStateFractalType)) {
        if (outError) *outError = "Load FITS requires an Explaino-family capture state";
        return false;
    }
    return true;
}

bool BuildRuntimeWalkViewerImportSession(const RuntimeWalkViewerImportRequest& request,
    RuntimeWalkViewerImportSessionRecord* outRecord,
    std::string* outError) {
    if (outError) outError->clear();
    if (!outRecord) {
        if (outError) *outError = "Runtime walk viewer import output is required";
        return false;
    }

    ResolvedImportSources resolved;
    if (!ResolveImportSources(request, &resolved, outError)) return false;

    const std::string sessionId = StableSessionId(request, resolved.bundle_json_path, resolved.comparison_fits_path);
    const std::filesystem::path sessionsRoot = SessionsRoot(request.exe_dir);
    const std::filesystem::path sessionDir = sessionsRoot / sessionId;
    const std::filesystem::path requestPath = sessionDir / "runtime_walk_viewer_request.json";
    const std::filesystem::path selectionManifestPath = sessionDir / "runtime_walk_import_selection_manifest.json";
    const std::filesystem::path receiptPath = sessionDir / "runtime_walk_import_receipt.json";
    const std::filesystem::path outputDir = sessionDir / "output";
    const std::filesystem::path orientationInputsPath = sessionDir / "orientation_inputs.json";
    const std::filesystem::path synthesizedStatePath = sessionDir / "state.json";

    RuntimeWalkRequest generatedRequest{};
    generatedRequest.authority_mode = request.authority_mode;
    generatedRequest.bundle_json_path = resolved.bundle_json_path;
    generatedRequest.output_dir = NormalizePathString(outputDir);
    generatedRequest.t_values = resolved.t_values;
    generatedRequest.comparison_fits_path = resolved.comparison_fits_path;
    generatedRequest.rtk_manifest_json_path = resolved.rtk_manifest_json_path;
    generatedRequest.rtk_harvest_summary_json_path = resolved.rtk_harvest_summary_json_path;
    generatedRequest.transport_generated = resolved.transport_generated;
    generatedRequest.transport_generation_mode = resolved.transport_generation_mode;
    generatedRequest.transport_sample_count = resolved.transport_sample_count;
    generatedRequest.transport_motion_scale = resolved.transport_options.motion_scale;
    generatedRequest.transport_warp_scale = resolved.transport_options.warp_scale;

    RuntimeWalkViewerImportSessionRecord record{};
    record.session_id = sessionId;
    record.session_dir = NormalizePathString(sessionDir);
    record.authority_mode = request.authority_mode;
    record.request_json_path = NormalizePathString(requestPath);
    record.comparison_fits_path = generatedRequest.comparison_fits_path;
    record.bundle_json_path = generatedRequest.bundle_json_path;
    record.rtk_manifest_json_path = generatedRequest.rtk_manifest_json_path;
    record.rtk_harvest_summary_json_path = generatedRequest.rtk_harvest_summary_json_path;
    record.source_request_json_path = resolved.source_request_json_path;
    record.source_bundle_json_path = resolved.source_bundle_json_path;
    record.discovery_source = resolved.discovery_source;
    record.transport_generation_mode = resolved.transport_generation_mode;
    record.transport_sample_count = resolved.transport_sample_count;
    record.transport_motion_scale = resolved.transport_options.motion_scale;
    record.transport_warp_scale = resolved.transport_options.warp_scale;
    record.transport_generated = resolved.transport_generated;
    record.request_exists = true;

    RuntimeWalkFitsMappingCatalog catalog;
    RuntimeWalkFitsOrientationInputs inputs;
    std::string mappingProfilePath;
    std::string mappingProfileId;
    bool haveOrientationContext = false;
    if (request.authority_mode == RuntimeWalkAuthorityMode::synthesized_fits_base || resolved.transport_generated) {
        if (generatedRequest.comparison_fits_path.empty()) {
            if (outError) *outError = "Synthesized FITS runtime-walk import requires a FITS input";
            return false;
        }
        if (!ResolveMappingProfilePath(request, &mappingProfilePath, outError)) return false;
        mappingProfileId = ResolveMappingProfileId(request);
        if (!LoadRuntimeWalkFitsMappingCatalogFile(mappingProfilePath, &catalog, outError)) return false;

        if (!request.orientation_inputs_json_path.empty()) {
            if (!LoadRuntimeWalkFitsOrientationInputsFile(request.orientation_inputs_json_path, &inputs, outError)) return false;
        } else {
            if (!ExtractRuntimeWalkFitsOrientationInputs(request.exe_dir,
                    generatedRequest.comparison_fits_path,
                    orientationInputsPath.string(),
                    outError)) {
                return false;
            }
            if (!LoadRuntimeWalkFitsOrientationInputsFile(orientationInputsPath.string(), &inputs, outError)) return false;
        }

        generatedRequest.mapping_profile_json_path = mappingProfilePath;
        generatedRequest.mapping_profile_id = mappingProfileId;
        generatedRequest.orientation_inputs_json_path = !request.orientation_inputs_json_path.empty()
            ? NormalizePathString(ResolveAbsolutePath(request.orientation_inputs_json_path))
            : NormalizePathString(orientationInputsPath);

        record.mapping_profile_json_path = generatedRequest.mapping_profile_json_path;
        record.mapping_profile_id = generatedRequest.mapping_profile_id;
        record.orientation_inputs_json_path = generatedRequest.orientation_inputs_json_path;
        haveOrientationContext = true;
    }

    if (request.authority_mode == RuntimeWalkAuthorityMode::synthesized_fits_base) {
        ViewState synthView{};
        KernelParams synthParams{};
        RenderSettings synthRender{};
        if (!SynthesizeRuntimeWalkBaseState(catalog, mappingProfileId, inputs, &synthView, &synthParams, &synthRender, outError)) {
            return false;
        }
        if (!WriteRuntimeWalkSynthesizedStateJson(synthesizedStatePath.string(), synthView, synthParams, synthRender, outError)) {
            return false;
        }

        generatedRequest.base_state_json_path = NormalizePathString(synthesizedStatePath);
        record.base_state_json_path = generatedRequest.base_state_json_path;
        record.synthesized_base_state_json_path = generatedRequest.base_state_json_path;
    } else {
        generatedRequest.base_state_json_path = NormalizePathString(ResolveAbsolutePath(request.base_state_json_path));
        record.base_state_json_path = generatedRequest.base_state_json_path;
    }

    if (resolved.transport_generated) {
        if (!haveOrientationContext) {
            if (outError) *outError = "Generated FITS runtime-walk transport requires orientation inputs";
            return false;
        }
        RuntimeWalkBundle synthesizedBundle;
        if (!SynthesizeRuntimeWalkTransportBundle(catalog, mappingProfileId, inputs, resolved.transport_options, &synthesizedBundle, outError)) {
            return false;
        }
        const std::filesystem::path bundlePath = sessionDir / "mr_zipper_branch.json";
        if (!WriteRuntimeWalkBundleJsonFile(bundlePath.string(), synthesizedBundle, outError)) {
            return false;
        }
        generatedRequest.bundle_json_path = NormalizePathString(bundlePath);
        generatedRequest.t_values.clear();
        generatedRequest.t_values.reserve(synthesizedBundle.samples.size());
        for (const RuntimeWalkBundleSample& sample : synthesizedBundle.samples) {
            generatedRequest.t_values.push_back(sample.t);
        }
        record.bundle_json_path = generatedRequest.bundle_json_path;
        record.discovery_source = "generated_transport";
        record.transport_generation_mode = "closed_loop_default";
        record.transport_sample_count = synthesizedBundle.samples.size();
        record.transport_motion_scale = resolved.transport_options.motion_scale;
        record.transport_warp_scale = resolved.transport_options.warp_scale;
        record.transport_generated = true;
        generatedRequest.transport_generated = true;
        generatedRequest.transport_generation_mode = record.transport_generation_mode;
        generatedRequest.transport_sample_count = record.transport_sample_count;
        generatedRequest.transport_motion_scale = record.transport_motion_scale;
        generatedRequest.transport_warp_scale = record.transport_warp_scale;
    }

    if (!WriteTextFile(requestPath, SerializeRuntimeWalkRequestJson(generatedRequest), outError)) return false;
    if (!WriteTextFile(selectionManifestPath, SerializeImportSelectionManifest(record, generatedRequest), outError)) return false;
    if (!WriteTextFile(receiptPath, SerializeImportReceipt(record), outError)) return false;

    ImportRecords records;
    if (!LoadRecentIndex(request.exe_dir, &records, outError)) return false;
    UpsertRecentRecord(record, &records);
    if (!WriteRecentIndex(request.exe_dir, records, outError)) return false;

    *outRecord = record;
    return true;
}

bool LoadLatestRuntimeWalkViewerImportSession(const std::string& exeDir,
    RuntimeWalkViewerImportSessionRecord* outRecord,
    std::string* outError) {
    if (outError) outError->clear();
    if (!outRecord) {
        if (outError) *outError = "Runtime walk latest-session output is required";
        return false;
    }
    ImportRecords records;
    if (!LoadRecentIndex(exeDir, &records, outError)) return false;
    if (records.empty()) {
        if (outError) *outError = "No runtime-walk FITS import sessions have been generated yet";
        return false;
    }

    for (RuntimeWalkViewerImportSessionRecord record : records) {
        record.request_exists = IsUsableImportSessionRecord(record);
        if (!record.request_exists || !record.viewer_load_succeeded) continue;
        *outRecord = record;
        return true;
    }

    const RuntimeWalkViewerImportSessionRecord& latest = records.front();
    if (outError) *outError = "Latest runtime-walk FITS import session is stale or missing required artifacts: " + latest.request_json_path;
    return false;
}

bool LoadRecentRuntimeWalkViewerImportSessions(const std::string& exeDir,
    std::vector<RuntimeWalkViewerImportSessionRecord>* outRecords,
    std::string* outError) {
    return LoadRecentIndex(exeDir, outRecords, outError);
}

void PrimeRuntimeWalkViewerImportPanel(const std::string& exeDir,
    const std::string& currentLoadedStatePath,
    const RuntimeWalkViewerSession& session,
    RuntimeWalkViewerImportPanelState* ioPanel) {
    if (!ioPanel) return;
    ioPanel->open = true;
    ioPanel->base_state_json_path = currentLoadedStatePath;
    ioPanel->authority_mode = RuntimeWalkAuthorityMode::synthesized_fits_base;
    ioPanel->request_json_path.clear();
    ioPanel->bundle_json_path.clear();
    ApplyDefaultTransportOptions(&ioPanel->transport_options);
    ioPanel->status_text.clear();
    if (session.loaded) {
        if (ioPanel->comparison_fits_path.empty()) {
            ioPanel->comparison_fits_path = session.asset.companion.comparison_fits_path;
        }
        if (ioPanel->mapping_profile_json_path.empty()) {
            ioPanel->mapping_profile_json_path = session.asset.authority.mapping_profile_json_path;
        }
        if (ioPanel->mapping_profile_id.empty()) {
            ioPanel->mapping_profile_id = session.asset.authority.mapping_profile_id;
        }
        if (session.asset.request.transport_sample_count > 0u) {
            ioPanel->transport_options.sample_count = session.asset.request.transport_sample_count;
        } else if (!session.asset.tick_snapshots.empty()) {
            ioPanel->transport_options.sample_count = session.asset.tick_snapshots.size();
        }
        ioPanel->transport_options.motion_scale = session.asset.request.transport_motion_scale;
        ioPanel->transport_options.warp_scale = session.asset.request.transport_warp_scale;
    }
    std::string mappingError;
    if (!RefreshMappingProfileDisplayState(exeDir, ioPanel, &mappingError) && ioPanel->status_text.empty()) {
        ioPanel->status_text = mappingError;
    }
    std::string error;
    std::vector<RuntimeWalkViewerImportSessionRecord> recent;
    if (LoadRecentRuntimeWalkViewerImportSessions(exeDir, &recent, &error)) {
        ioPanel->recent_sessions = recent;
        if (session.loaded) {
            if (const RuntimeWalkViewerImportSessionRecord* recentRecord = FindMatchingRecentRecord(recent, session.request_json_path)) {
                if (recentRecord->transport_sample_count > 0u) {
                    ioPanel->transport_options.sample_count = recentRecord->transport_sample_count;
                }
                ioPanel->transport_options.motion_scale = recentRecord->transport_motion_scale;
                ioPanel->transport_options.warp_scale = recentRecord->transport_warp_scale;
            }
        }
    } else if (ioPanel->status_text.empty()) {
        ioPanel->status_text = error;
    }
}

bool NoteRuntimeWalkViewerImportSessionLoadSucceeded(const std::string& requestJsonPath,
    std::string* outError) {
    if (outError) outError->clear();
    const std::filesystem::path requestPath = ResolveAbsolutePath(requestJsonPath);
    if (requestPath.empty()) {
        if (outError) *outError = "Runtime-walk session load receipt requires a request path";
        return false;
    }
    const std::filesystem::path sessionDir = requestPath.parent_path();
    const std::filesystem::path sessionsRoot = sessionDir.parent_path();
    const std::filesystem::path exeDir = sessionsRoot.parent_path().parent_path();
    const std::filesystem::path recentIndexPath = sessionsRoot / "recent_sessions.json";

    RuntimeWalkViewerImportSessionRecord matched;
    bool found = false;
    if (std::filesystem::exists(recentIndexPath)) {
        ImportRecords records;
        if (!LoadRecentIndex(exeDir.string(), &records, outError)) return false;
        for (RuntimeWalkViewerImportSessionRecord& record : records) {
            if (NormalizePathString(ResolveAbsolutePath(record.request_json_path)) != NormalizePathString(requestPath)) continue;
            record.viewer_load_succeeded = true;
            matched = record;
            found = true;
            if (!WriteRecentIndex(exeDir.string(), records, outError)) return false;
            break;
        }
    }
    if (!found) {
        return true;
    }

    RuntimeWalkRequest request;
    if (!LoadRuntimeWalkRequestFile(requestJsonPath, &request, outError)) return false;
    if (!WriteTextFile(ImportSelectionManifestPath(matched.session_dir),
            SerializeImportSelectionManifest(matched, request),
            outError)) {
        return false;
    }
    if (!WriteTextFile(ImportReceiptPath(matched.session_dir),
            SerializeImportReceipt(matched),
            outError)) {
        return false;
    }
    return true;
}
