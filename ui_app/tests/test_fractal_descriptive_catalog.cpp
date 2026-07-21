#include "../src/fractal_descriptive_catalog.h"
#include "../src/json_min.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>

namespace {

bool Expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << message << "\n";
        return false;
    }
    return true;
}

std::string ReadText(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream text;
    text << in.rdbuf();
    return text.str();
}

const json_min::Value* FindEntry(const json_min::Array& entries, const std::string& selector) {
    for (const auto& entry : entries) {
        const auto* value = entry.get("selector_id");
        if (value && value->is_string() && value->as_string() == selector) {
            return &entry;
        }
    }
    return nullptr;
}

bool TestSchemaCoverageAndDeterminism() {
    const std::string first = BuildFractalDescriptiveCatalogJson();
    const std::string second = BuildFractalDescriptiveCatalogJson();
    if (!Expect(first == second, "repeated catalog serialization must be byte-identical")) return false;
    if (!Expect(!first.empty() && first.back() == '\n', "catalog bytes must have one deterministic trailing newline")) return false;

    const auto parsed = json_min::Parse(first);
    if (!Expect(parsed.error.empty() && parsed.value.is_object(), "catalog JSON must parse as an object")) return false;
    const auto* schemaVersion = parsed.value.get("schema_version");
    const auto* entriesValue = parsed.value.get("entries");
    if (!Expect(schemaVersion && schemaVersion->is_number() && schemaVersion->as_number() == 1.0, "schema_version must be 1")) return false;
    if (!Expect(entriesValue && entriesValue->is_array(), "entries must be an array")) return false;
    const auto& entries = entriesValue->as_array();
    if (!Expect(entries.size() == FractalCatalogCount(), "every live catalog row must be exported")) return false;

    constexpr uint32_t knownCapabilityFlags =
        FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::sample_probe) |
        FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::schema_control_surface) |
        FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::param_animation_surface) |
        FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::smooth_escape_coloring) |
        FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::color_pipeline_frame_coloring) |
        FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::generic_equation_pack) |
        FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::root_basin_coloring) |
        FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::sdf_pack_scene) |
        FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::field_primary_sdf);
    constexpr uint32_t knownRuntimeFlags =
        FractalCatalogRuntimeFlagMask(FractalCatalogRuntimeFlag::escape_time) |
        FractalCatalogRuntimeFlagMask(FractalCatalogRuntimeFlag::basin_coloring) |
        FractalCatalogRuntimeFlagMask(FractalCatalogRuntimeFlag::explaino_family) |
        FractalCatalogRuntimeFlagMask(FractalCatalogRuntimeFlag::perturbation_reference_orbit);
    std::set<std::string> selectors;
    std::size_t reviewedCount = 0;
    for (std::size_t index = 0; index < entries.size(); ++index) {
        const auto& exported = entries[index];
        const auto& live = kFractalCatalog[index];
        const auto* selector = exported.get("selector_id");
        const auto* displayName = exported.get("display_name");
        const auto* category = exported.get("category");
        const auto* family = exported.get("family");
        const auto* growth = exported.get("formula_growth_surface");
        const auto* capabilityFlags = exported.get("capability_flags");
        const auto* runtimeFlags = exported.get("runtime_flags");
        const auto* status = exported.get("description_status");
        const auto* description = exported.get("description");
        if (!Expect(selector && selector->is_string() && selector->as_string() == live.selector_id, "selector order must follow kFractalCatalog")) return false;
        if (!Expect(selectors.insert(selector->as_string()).second, "selector IDs must be unique")) return false;
        if (!Expect(displayName && displayName->is_string() && displayName->as_string() == live.display_name, "display name must come from live catalog")) return false;
        if (!Expect(category && category->is_string() && category->as_string() != "unknown", "every live category must have an explicit V1 string")) return false;
        if (!Expect(family && family->is_string() && family->as_string() != "unknown", "every live family must have an explicit V1 string")) return false;
        if (!Expect(growth && growth->is_string() && growth->as_string() != "unknown", "every live formula growth surface must have an explicit V1 string")) return false;
        if (!Expect((live.capability_flags & ~knownCapabilityFlags) == 0u, "live capability flags contain an unhandled bit")) return false;
        if (!Expect((live.runtime_flags & ~knownRuntimeFlags) == 0u, "live runtime flags contain an unhandled bit")) return false;
        if (!Expect(capabilityFlags && capabilityFlags->is_array(), "capability_flags must be an ordered string array")) return false;
        if (!Expect(runtimeFlags && runtimeFlags->is_array(), "runtime_flags must be an ordered string array")) return false;
        if (!Expect(status && status->is_string() && description, "description status and value are required")) return false;
        if (status->as_string() == "reviewed") {
            ++reviewedCount;
            if (!Expect(description->is_object(), "reviewed descriptions must be objects")) return false;
            for (const char* field : {"math_summary", "recurrence_or_field_model", "state_order", "termination_or_classification", "interpretation_notes"}) {
                const auto* value = description->get(field);
                if (!Expect(value && value->is_string() && !value->as_string().empty(), std::string("reviewed field missing: ") + field)) return false;
            }
            const auto* refs = description->get("source_refs");
            if (!Expect(refs && refs->is_array() && !refs->as_array().empty(), "reviewed source_refs must be nonempty")) return false;
        } else {
            if (!Expect(status->as_string() == "unavailable" && description->is_null(), "unreviewed entries must fail softly as unavailable/null")) return false;
        }
    }
    if (!Expect(reviewedCount == 2, "exactly two selectors are reviewed in this campaign")) return false;
    if (!Expect(FindEntry(entries, "explaino_all")->get("description_status")->as_string() == "reviewed", "explaino_all must be reviewed")) return false;
    if (!Expect(FindEntry(entries, "explaino_magnet_root_well")->get("description_status")->as_string() == "reviewed", "explaino_magnet_root_well must be reviewed")) return false;
    if (!Expect(FindEntry(entries, "lambda") != nullptr && FindEntry(entries, "lambda_map") == nullptr, "live lambda identity must remain lambda")) return false;

    for (const char* forbidden : {"\"generated_at\":", "\"timestamp\":", "\"branch\":", "\"commit\":", "C:\\\\", "D:\\\\"}) {
        if (!Expect(first.find(forbidden) == std::string::npos, std::string("volatile provenance field leaked: ") + forbidden)) return false;
    }
    return true;
}

bool TestExistingFlagMeaningsAndOrdering() {
    const auto parsed = json_min::Parse(BuildFractalDescriptiveCatalogJson());
    const auto& entries = parsed.value.get("entries")->as_array();
    const auto* explainoAll = FindEntry(entries, "explaino_all");
    const auto* rootSdf = FindEntry(entries, "explaino_root_sdf");
    if (!Expect(explainoAll && rootSdf, "expected live selectors must exist")) return false;

    const auto& explainoCapabilities = explainoAll->get("capability_flags")->as_array();
    const std::vector<std::string> expectedExplainoCapabilities = {
        "sample_probe", "schema_control_surface", "param_animation_surface",
        "smooth_escape_coloring", "color_pipeline_frame_coloring", "root_basin_coloring"};
    if (!Expect(explainoCapabilities.size() == expectedExplainoCapabilities.size(), "explaino_all capability flag count changed")) return false;
    for (std::size_t index = 0; index < expectedExplainoCapabilities.size(); ++index) {
        if (!Expect(explainoCapabilities[index].as_string() == expectedExplainoCapabilities[index], "capability flags must use declared bit order")) return false;
    }

    const auto& sdfCapabilities = rootSdf->get("capability_flags")->as_array();
    if (!Expect(sdfCapabilities.back().as_string() == "field_primary_sdf", "field-primary SDF meaning must be serialized unchanged")) return false;
    return true;
}

bool TestSentenceEvidenceLedger() {
    const std::filesystem::path ledgerPath = std::filesystem::path("..") / "docs" / "fractal_descriptive_catalog_evidence.v1.json";
    const std::string ledgerText = ReadText(ledgerPath);
    if (!Expect(!ledgerText.empty(), "tracked sentence evidence ledger must exist")) return false;
    const auto parsed = json_min::Parse(ledgerText);
    if (!Expect(parsed.error.empty() && parsed.value.is_object(), "evidence ledger must parse")) return false;
    const auto* claimsValue = parsed.value.get("claims");
    if (!Expect(claimsValue && claimsValue->is_array(), "evidence ledger claims must be an array")) return false;

    std::map<std::string, const json_min::Value*> claims;
    for (const auto& claim : claimsValue->as_array()) {
        const auto* id = claim.get("claim_id");
        if (!Expect(id && id->is_string() && claims.emplace(id->as_string(), &claim).second, "claim IDs must be present and unique")) return false;
    }

    std::set<std::string> usedClaims;
    for (std::size_t descriptionIndex = 0; descriptionIndex < FractalReviewedDescriptionCount(); ++descriptionIndex) {
        const auto& description = FractalReviewedDescriptionAt(descriptionIndex);
        for (std::size_t fieldIndex = 0; fieldIndex < description.field_count; ++fieldIndex) {
            const auto& field = description.fields[fieldIndex];
            for (std::size_t sentenceIndex = 0; sentenceIndex < field.sentence_count; ++sentenceIndex) {
                const auto& sentence = field.sentences[sentenceIndex];
                const auto found = claims.find(sentence.claim_id);
                if (!Expect(found != claims.end(), std::string("missing claim: ") + sentence.claim_id)) return false;
                if (!Expect(usedClaims.insert(sentence.claim_id).second, std::string("claim reused ambiguously: ") + sentence.claim_id)) return false;
                const auto& claim = *found->second;
                if (!Expect(claim.get("selector")->as_string() == description.selector_id, "claim selector mismatch")) return false;
                if (!Expect(claim.get("description_field")->as_string() == field.field_name, "claim field mismatch")) return false;
                if (!Expect(claim.get("sentence_index")->as_number() == static_cast<double>(sentenceIndex), "claim sentence index mismatch")) return false;
                if (!Expect(claim.get("claim_text")->as_string() == sentence.text, "claim text mismatch")) return false;
                if (!Expect(claim.get("review_disposition")->as_string() == "accepted", "only accepted evidence may authorize prose")) return false;
                const std::string classification = claim.get("classification")->as_string();
                if (!Expect(classification == "direct" || classification == "derived" || classification == "editorial_paraphrase", "claim classification invalid")) return false;
            }
        }
        for (std::size_t refIndex = 0; refIndex < description.source_ref_count; ++refIndex) {
            const std::string sourceRef = description.source_refs[refIndex];
            if (!Expect(sourceRef.rfind("ui_app/", 0) == 0 && sourceRef.find(':') == std::string::npos, "source refs must be concise repository-relative references")) return false;
            const std::size_t separator = sourceRef.find('#');
            if (!Expect(separator != std::string::npos && separator > 0 && separator + 1 < sourceRef.size(), "source refs must identify a repository-relative file and symbol")) return false;
            const std::string sourceFile = sourceRef.substr(0, separator);
            const std::string sourceSymbol = sourceRef.substr(separator + 1);
            bool authorizedByAcceptedClaim = false;
            for (const auto& pair : claims) {
                const auto& claim = *pair.second;
                const auto* selector = claim.get("selector");
                const auto* disposition = claim.get("review_disposition");
                const auto* claimFile = claim.get("source_file");
                const auto* claimSymbol = claim.get("source_symbol");
                if (selector && selector->is_string() && selector->as_string() == description.selector_id &&
                    disposition && disposition->is_string() && disposition->as_string() == "accepted" &&
                    claimFile && claimFile->is_string() && claimFile->as_string() == sourceFile &&
                    claimSymbol && claimSymbol->is_string() && claimSymbol->as_string().rfind(sourceSymbol, 0) == 0 &&
                    usedClaims.count(pair.first) != 0) {
                    authorizedByAcceptedClaim = true;
                }
            }
            if (!Expect(authorizedByAcceptedClaim, "public source ref is not derived from an accepted current claim: " + sourceRef)) return false;
        }
    }

    for (const auto& pair : claims) {
        const auto* disposition = pair.second->get("review_disposition");
        if (disposition && disposition->is_string() && disposition->as_string() != "accepted") {
            if (!Expect(usedClaims.count(pair.first) == 0, "rejected or superseded evidence authorized current prose")) return false;
        }
        const auto* selector = pair.second->get("selector");
        const bool reviewedSelector = selector && selector->is_string() &&
            (selector->as_string() == "explaino_all" || selector->as_string() == "explaino_magnet_root_well");
        if (reviewedSelector && disposition && disposition->is_string() && disposition->as_string() == "accepted") {
            if (!Expect(usedClaims.count(pair.first) == 1, "accepted reviewed evidence is not mapped to exactly one current sentence: " + pair.first)) return false;
        }
    }
    return true;
}

bool TestFileOutputSemantics() {
    const auto root = std::filesystem::temp_directory_path() / "fractal_descriptive_catalog_v1_test";
    std::error_code ec;
    std::filesystem::remove_all(root, ec);
    std::filesystem::create_directories(root, ec);
    const auto target = root / "catalog.json";
    std::string error;
    if (!Expect(WriteFractalDescriptiveCatalogJsonFile(target.string(), &error), "file export should succeed: " + error)) return false;
    if (!Expect(ReadText(target) == BuildFractalDescriptiveCatalogJson(), "file and stdout bytes must be identical")) return false;

    {
        std::ofstream existing(target, std::ios::binary | std::ios::trunc);
        existing << "old";
    }
    {
        std::ofstream stale(target.string() + ".tmp", std::ios::binary | std::ios::trunc);
        stale << "stale";
    }
    error.clear();
    if (!Expect(WriteFractalDescriptiveCatalogJsonFile(target.string(), &error), "existing target and stale owned temp must be replaceable")) return false;
    if (!Expect(ReadText(target) == BuildFractalDescriptiveCatalogJson(), "replacement bytes must be exact")) return false;
    if (!Expect(!std::filesystem::exists(target.string() + ".tmp"), "successful export must clean its temp")) return false;

    const auto missingTarget = root / "missing" / "catalog.json";
    error.clear();
    if (!Expect(!WriteFractalDescriptiveCatalogJsonFile(missingTarget.string(), &error) && !error.empty(), "missing parent must fail clearly")) return false;
    if (!Expect(!std::filesystem::exists(missingTarget.string() + ".tmp"), "missing-parent failure must not leave a temp")) return false;

    const auto blockedTarget = root / "blocked";
    std::filesystem::create_directories(blockedTarget, ec);
    {
        std::ofstream child(blockedTarget / "child.txt");
        child << "keep";
    }
    error.clear();
    if (!Expect(!WriteFractalDescriptiveCatalogJsonFile(blockedTarget.string(), &error) && !error.empty(), "failed replacement must return a diagnostic")) return false;
    if (!Expect(std::filesystem::is_directory(blockedTarget), "failed replacement must preserve the blocked target")) return false;
    if (!Expect(!std::filesystem::exists(blockedTarget.string() + ".tmp"), "failed replacement must clean its owned temp")) return false;

    std::filesystem::remove_all(root, ec);
    return true;
}

} // namespace

int main() {
    if (!TestSchemaCoverageAndDeterminism()) return 1;
    if (!TestExistingFlagMeaningsAndOrdering()) return 1;
    if (!TestSentenceEvidenceLedger()) return 1;
    if (!TestFileOutputSemantics()) return 1;
    std::cout << "test_fractal_descriptive_catalog: PASS\n";
    return 0;
}
