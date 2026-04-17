#include "runtime_walk_viewer_import.h"

#include "fractal_family_rules.h"
#include "json_min.h"
#include "runtime_walk.h"

#include <algorithm>
#include <array>
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

std::filesystem::path SessionsRoot(const std::string& exeDir) {
    return ResolveAbsolutePath(exeDir) / "diagnostics" / "runtime_walk_sessions";
}

std::filesystem::path RecentIndexPath(const std::string& exeDir) {
    return SessionsRoot(exeDir) / "recent_sessions.json";
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
    const std::array<std::string, 5> fields = {
        NormalizePathString(ResolveAbsolutePath(request.base_state_json_path)),
        NormalizePathString(ResolveAbsolutePath(resolvedBundlePath)),
        NormalizePathString(ResolveAbsolutePath(resolvedFitsPath)),
        NormalizePathString(ResolveAbsolutePath(request.rtk_manifest_json_path)),
        NormalizePathString(ResolveAbsolutePath(request.rtk_harvest_summary_json_path)),
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
        GetOptionalString(entry, "base_state_json", &record.base_state_json_path);
        GetOptionalString(entry, "comparison_fits", &record.comparison_fits_path);
        GetOptionalString(entry, "bundle_json", &record.bundle_json_path);
        GetOptionalString(entry, "rtk_manifest_json", &record.rtk_manifest_json_path);
        GetOptionalString(entry, "rtk_harvest_summary_json", &record.rtk_harvest_summary_json_path);
        GetOptionalString(entry, "source_request_json", &record.source_request_json_path);
        GetOptionalString(entry, "source_bundle_json", &record.source_bundle_json_path);
        GetOptionalString(entry, "discovery_source", &record.discovery_source);
        record.session_dir = NormalizePathString(ResolveAbsolutePath(sessionDir, indexDir));
        record.request_json_path = NormalizePathString(ResolveAbsolutePath(requestPath, indexDir));
        record.request_exists = !record.request_json_path.empty() && std::filesystem::exists(record.request_json_path);
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
        out << "      \"base_state_json\": \"" << JsonEscape(record.base_state_json_path) << "\",\n";
        out << "      \"comparison_fits\": \"" << JsonEscape(record.comparison_fits_path) << "\",\n";
        out << "      \"bundle_json\": \"" << JsonEscape(record.bundle_json_path) << "\",\n";
        out << "      \"rtk_manifest_json\": \"" << JsonEscape(record.rtk_manifest_json_path) << "\",\n";
        out << "      \"rtk_harvest_summary_json\": \"" << JsonEscape(record.rtk_harvest_summary_json_path) << "\",\n";
        out << "      \"source_request_json\": \"" << JsonEscape(record.source_request_json_path) << "\",\n";
        out << "      \"source_bundle_json\": \"" << JsonEscape(record.source_bundle_json_path) << "\",\n";
        out << "      \"discovery_source\": \"" << JsonEscape(record.discovery_source) << "\"\n";
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

std::string SerializeRuntimeWalkRequestJson(const RuntimeWalkRequest& request) {
    std::ostringstream out;
    out << "{\n";
    out << "  \"version\": 1,\n";
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
    out << "\n}\n";
    return out.str();
}

std::string SerializeImportSelectionManifest(const RuntimeWalkViewerImportSessionRecord& record,
    const RuntimeWalkRequest& request) {
    std::ostringstream out;
    out << "{\n";
    out << "  \"version\": 1,\n";
    out << "  \"session_id\": \"" << JsonEscape(record.session_id) << "\",\n";
    out << "  \"discovery_source\": \"" << JsonEscape(record.discovery_source) << "\",\n";
    out << "  \"base_state_json\": \"" << JsonEscape(request.base_state_json_path) << "\",\n";
    out << "  \"comparison_fits\": \"" << JsonEscape(record.comparison_fits_path) << "\",\n";
    out << "  \"source_request_json\": \"" << JsonEscape(record.source_request_json_path) << "\",\n";
    out << "  \"source_bundle_json\": \"" << JsonEscape(record.source_bundle_json_path) << "\",\n";
    out << "  \"generated_request_json\": \"" << JsonEscape(record.request_json_path) << "\",\n";
    out << "  \"bundle_json\": \"" << JsonEscape(request.bundle_json_path) << "\",\n";
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
    out << "  \"request_json\": \"" << JsonEscape(record.request_json_path) << "\"\n";
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
    std::vector<double> t_values;
};

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
            return true;
        }
        if (!record.bundle_json_path.empty() && std::filesystem::exists(record.bundle_json_path)) {
            outResolved->bundle_json_path = NormalizePathString(ResolveAbsolutePath(record.bundle_json_path));
            outResolved->discovery_source = "recent_match_bundle";
            return true;
        }
    }
    return false;
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

    if (!ValidateRuntimeWalkViewerImportBaseState(request.base_state_json_path, request.base_fractal_type, outError)) {
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
    } else if (TryResolveRecentMatch(request, records, outResolved)) {
    } else if (TryResolveSiblingCandidates(request, outResolved)) {
    } else {
        if (outError) *outError = "Load FITS could not discover a compatible runtime-walk request or bundle; browse one in the import panel";
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
    }

    if (outResolved->t_values.size() < 2u) {
        if (outError) *outError = "Load FITS requires a runtime-walk request or bundle with at least two t values";
        return false;
    }
    return true;
}

} // namespace

bool ValidateRuntimeWalkViewerImportBaseState(const std::string& baseStateJsonPath,
    FractalType baseStateFractalType,
    std::string* outError) {
    if (outError) outError->clear();
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

    RuntimeWalkRequest generatedRequest{};
    generatedRequest.base_state_json_path = NormalizePathString(ResolveAbsolutePath(request.base_state_json_path));
    generatedRequest.bundle_json_path = resolved.bundle_json_path;
    generatedRequest.output_dir = NormalizePathString(outputDir);
    generatedRequest.t_values = resolved.t_values;
    generatedRequest.comparison_fits_path = resolved.comparison_fits_path;
    generatedRequest.rtk_manifest_json_path = resolved.rtk_manifest_json_path;
    generatedRequest.rtk_harvest_summary_json_path = resolved.rtk_harvest_summary_json_path;

    RuntimeWalkViewerImportSessionRecord record{};
    record.session_id = sessionId;
    record.session_dir = NormalizePathString(sessionDir);
    record.request_json_path = NormalizePathString(requestPath);
    record.base_state_json_path = generatedRequest.base_state_json_path;
    record.comparison_fits_path = generatedRequest.comparison_fits_path;
    record.bundle_json_path = generatedRequest.bundle_json_path;
    record.rtk_manifest_json_path = generatedRequest.rtk_manifest_json_path;
    record.rtk_harvest_summary_json_path = generatedRequest.rtk_harvest_summary_json_path;
    record.source_request_json_path = resolved.source_request_json_path;
    record.source_bundle_json_path = resolved.source_bundle_json_path;
    record.discovery_source = resolved.discovery_source;
    record.request_exists = true;

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
    RuntimeWalkViewerImportSessionRecord latest = records.front();
    latest.request_exists = !latest.request_json_path.empty() && std::filesystem::exists(latest.request_json_path);
    if (!latest.request_exists) {
        if (outError) *outError = "Latest runtime-walk FITS import session is stale or missing: " + latest.request_json_path;
        return false;
    }
    *outRecord = latest;
    return true;
}

bool LoadRecentRuntimeWalkViewerImportSessions(const std::string& exeDir,
    std::vector<RuntimeWalkViewerImportSessionRecord>* outRecords,
    std::string* outError) {
    return LoadRecentIndex(exeDir, outRecords, outError);
}
