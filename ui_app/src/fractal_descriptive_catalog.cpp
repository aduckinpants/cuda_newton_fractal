#include "fractal_descriptive_catalog.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>

namespace {

template <typename T, std::size_t N>
constexpr std::size_t ArraySize(const T (&)[N]) {
    return N;
}

constexpr FractalDescriptionSentence kExplainoAllMathSummary[] = {
    {"explaino_all.math_summary.0", "ExplainO All is the canonical public ExplainO selector: neutral registry axes resolve to the baseline ExplainO runtime, while any active registered axis keeps the canonical composed branch."},
};
constexpr FractalDescriptionSentence kExplainoAllRecurrence[] = {
    {"explaino_all.recurrence_or_field_model.0", "The composed branch evaluates a degree-four real-coefficient polynomial and its derivative, forms a Newton step, and can combine alternating coefficient sets, vortex rotation, a perpendicular ripple kick, tension and balance-field biases, adaptive damping, and an optional Phoenix memory term."},
};
constexpr FractalDescriptionSentence kExplainoAllStateOrder[] = {
    {"explaino_all.state_order.0", "With a neutral Phoenix coefficient the update depends on the current complex state; with a nonzero Phoenix coefficient it also uses the previous state and is second-order in the stored orbit state."},
};
constexpr FractalDescriptionSentence kExplainoAllTermination[] = {
    {"explaino_all.termination_or_classification.0", "The branch tests polynomial residual against epsilon, rescales states outside radius four, resets and stops on nonfinite state, and otherwise classifies an unconverged result at the best recorded iteration by snapping to the nearest available root."},
};
constexpr FractalDescriptionSentence kExplainoAllInterpretation[] = {
    {"explaino_all.interpretation_notes.0", "The selector names a configurable family composition, so serialized nonzero axes identify enabled recurrence terms but do not by themselves prove which term is visually dominant in a captured frame."},
};

constexpr FractalDescriptionField kExplainoAllFields[] = {
    {"math_summary", kExplainoAllMathSummary, ArraySize(kExplainoAllMathSummary)},
    {"recurrence_or_field_model", kExplainoAllRecurrence, ArraySize(kExplainoAllRecurrence)},
    {"state_order", kExplainoAllStateOrder, ArraySize(kExplainoAllStateOrder)},
    {"termination_or_classification", kExplainoAllTermination, ArraySize(kExplainoAllTermination)},
    {"interpretation_notes", kExplainoAllInterpretation, ArraySize(kExplainoAllInterpretation)},
};
constexpr const char* kExplainoAllSourceRefs[] = {
    "ui_app/src/fractal_family_rules.h#ResolveExplainoRuntimeFractalType",
    "ui_app/src/fractal_sample_device.inl#FractalSample",
};

constexpr FractalDescriptionSentence kMagnetRootWellMathSummary[] = {
    {"explaino_magnet_root_well.math_summary.0", "ExplainO Magnet Root Well is a root-field coloring consumer whose orbit dynamics use the engine's Magnet Type-I base recurrence."},
};
constexpr FractalDescriptionSentence kMagnetRootWellRecurrence[] = {
    {"explaino_magnet_root_well.recurrence_or_field_model.0", "For each pixel the coordinate is the recurrence parameter c and the configured Magnet seed is the initial z."},
    {"explaino_magnet_root_well.recurrence_or_field_model.1", "The squared rational update ((z^2 - 1 + c) / (2z - 2 + c))^2 is blended from the prior z by relaxation clamped to the interval 0.05 through 1.5."},
};
constexpr FractalDescriptionSentence kMagnetRootWellStateOrder[] = {
    {"explaino_magnet_root_well.state_order.0", "The Magnet update is first-order in z for fixed pixel coordinate, seed, and relaxation; root-pattern data is consumed later by coloring rather than fed back into the base recurrence."},
};
constexpr FractalDescriptionSentence kMagnetRootWellTermination[] = {
    {"explaino_magnet_root_well.termination_or_classification.0", "Iteration converges when the squared distance from z to the unit attractor falls below epsilon squared, and otherwise stops on the configured Magnet bailout, a nonfinite state, or the iteration limit."},
};
constexpr FractalDescriptionSentence kMagnetRootWellInterpretation[] = {
    {"explaino_magnet_root_well.interpretation_notes.0", "Root-field distance and nearest-root signals can shape coloring around configured, generated, or fallback unit roots."},
    {"explaino_magnet_root_well.interpretation_notes.1", "Those coloring signals are downstream of the Magnet recurrence, and continuous proximity alone does not establish discrete basins."},
};

constexpr FractalDescriptionField kMagnetRootWellFields[] = {
    {"math_summary", kMagnetRootWellMathSummary, ArraySize(kMagnetRootWellMathSummary)},
    {"recurrence_or_field_model", kMagnetRootWellRecurrence, ArraySize(kMagnetRootWellRecurrence)},
    {"state_order", kMagnetRootWellStateOrder, ArraySize(kMagnetRootWellStateOrder)},
    {"termination_or_classification", kMagnetRootWellTermination, ArraySize(kMagnetRootWellTermination)},
    {"interpretation_notes", kMagnetRootWellInterpretation, ArraySize(kMagnetRootWellInterpretation)},
};
constexpr const char* kMagnetRootWellSourceRefs[] = {
    "ui_app/src/fractal_family_rules.h#RootFieldConsumerBaseFractalType",
    "ui_app/src/escape_time_direct_formulas.h#InitEscapeTimeDirectState",
    "ui_app/src/escape_time_direct_formulas.h#StepEscapeTimeDirectState",
    "ui_app/src/fractal_sample_device.inl#FractalSample",
    "ui_app/src/escape_time_coloring.h#TryResolveRootFieldConsumerDistance",
};

#define FRACTAL_REVIEWED_DESCRIPTION(enum_name, selector, math_summary, recurrence, state_order, termination, interpretation, ref0, ref1, ref2, ref3) \
    constexpr FractalDescriptionSentence k##enum_name##MathSummary[] = {{selector ".math_summary.0", math_summary}}; \
    constexpr FractalDescriptionSentence k##enum_name##Recurrence[] = {{selector ".recurrence_or_field_model.0", recurrence}}; \
    constexpr FractalDescriptionSentence k##enum_name##StateOrder[] = {{selector ".state_order.0", state_order}}; \
    constexpr FractalDescriptionSentence k##enum_name##Termination[] = {{selector ".termination_or_classification.0", termination}}; \
    constexpr FractalDescriptionSentence k##enum_name##Interpretation[] = {{selector ".interpretation_notes.0", interpretation}}; \
    constexpr FractalDescriptionField k##enum_name##Fields[] = { \
        {"math_summary", k##enum_name##MathSummary, ArraySize(k##enum_name##MathSummary)}, \
        {"recurrence_or_field_model", k##enum_name##Recurrence, ArraySize(k##enum_name##Recurrence)}, \
        {"state_order", k##enum_name##StateOrder, ArraySize(k##enum_name##StateOrder)}, \
        {"termination_or_classification", k##enum_name##Termination, ArraySize(k##enum_name##Termination)}, \
        {"interpretation_notes", k##enum_name##Interpretation, ArraySize(k##enum_name##Interpretation)}, \
    }; \
    constexpr const char* k##enum_name##SourceRefs[] = {ref0, ref1, ref2, ref3}; \
    constexpr FractalReviewedDescription k##enum_name##Description = { \
        FractalType::enum_name, selector, k##enum_name##Fields, ArraySize(k##enum_name##Fields), \
        k##enum_name##SourceRefs, ref3 ? 4u : (ref2 ? 3u : (ref1 ? 2u : 1u)), \
    };

#include "fractal_descriptive_catalog_entries.inc"

#undef FRACTAL_REVIEWED_DESCRIPTION

constexpr FractalReviewedDescription kReviewedDescriptions[] = {
    {
        FractalType::explaino_all,
        "explaino_all",
        kExplainoAllFields,
        ArraySize(kExplainoAllFields),
        kExplainoAllSourceRefs,
        ArraySize(kExplainoAllSourceRefs),
    },
    {
        FractalType::explaino_magnet_root_well,
        "explaino_magnet_root_well",
        kMagnetRootWellFields,
        ArraySize(kMagnetRootWellFields),
        kMagnetRootWellSourceRefs,
        ArraySize(kMagnetRootWellSourceRefs),
    },
#define FRACTAL_REVIEWED_DESCRIPTION(enum_name, selector, math_summary, recurrence, state_order, termination, interpretation, ref0, ref1, ref2, ref3) \
    k##enum_name##Description,
#include "fractal_descriptive_catalog_entries.inc"
#undef FRACTAL_REVIEWED_DESCRIPTION
};

const char* CategoryId(FractalCatalogCategory value) {
    switch (value) {
    case FractalCatalogCategory::root_finding: return "root_finding";
    case FractalCatalogCategory::escape_time: return "escape_time";
    case FractalCatalogCategory::explaino: return "explaino";
    case FractalCatalogCategory::analysis: return "analysis";
    case FractalCatalogCategory::custom: return "custom";
    case FractalCatalogCategory::sdf: return "sdf";
    }
    return "unknown";
}

const char* FamilyId(FractalCatalogFamily value) {
    switch (value) {
    case FractalCatalogFamily::newton: return "newton";
    case FractalCatalogFamily::nova: return "nova";
    case FractalCatalogFamily::mandelbrot: return "mandelbrot";
    case FractalCatalogFamily::julia: return "julia";
    case FractalCatalogFamily::burning_ship: return "burning_ship";
    case FractalCatalogFamily::multibrot: return "multibrot";
    case FractalCatalogFamily::phoenix: return "phoenix";
    case FractalCatalogFamily::explaino: return "explaino";
    case FractalCatalogFamily::multicorn: return "multicorn";
    case FractalCatalogFamily::halley: return "halley";
    case FractalCatalogFamily::collatz: return "collatz";
    case FractalCatalogFamily::mcmullen: return "mcmullen";
    case FractalCatalogFamily::lambda_map: return "lambda_map";
    case FractalCatalogFamily::spider: return "spider";
    case FractalCatalogFamily::celtic_mandelbrot: return "celtic_mandelbrot";
    case FractalCatalogFamily::perpendicular_burning_ship: return "perpendicular_burning_ship";
    case FractalCatalogFamily::counterfactual_pair: return "counterfactual_pair";
    case FractalCatalogFamily::projection_and_flow: return "projection_and_flow";
    case FractalCatalogFamily::magnet: return "magnet";
    case FractalCatalogFamily::generic_equation_pack: return "generic_equation_pack";
    case FractalCatalogFamily::sdf_pack_scene: return "sdf_pack_scene";
    case FractalCatalogFamily::explaino_root_sdf: return "explaino_root_sdf";
    }
    return "unknown";
}

const char* FormulaGrowthSurfaceId(FractalCatalogFormulaGrowthSurface value) {
    switch (value) {
    case FractalCatalogFormulaGrowthSurface::native_2d_formula: return "native_2d_formula";
    case FractalCatalogFormulaGrowthSurface::native_composite_formula: return "native_composite_formula";
    case FractalCatalogFormulaGrowthSurface::generic_equation_pack: return "generic_equation_pack";
    case FractalCatalogFormulaGrowthSurface::sdf_pack_scene: return "sdf_pack_scene";
    case FractalCatalogFormulaGrowthSurface::field_primary_sdf: return "field_primary_sdf";
    }
    return "unknown";
}

void AppendJsonString(std::ostringstream& out, const char* raw) {
    const std::string value = raw ? std::string(raw) : std::string();
    out << '"';
    for (const unsigned char ch : value) {
        switch (ch) {
        case '"': out << "\\\""; break;
        case '\\': out << "\\\\"; break;
        case '\b': out << "\\b"; break;
        case '\f': out << "\\f"; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default:
            if (ch < 0x20) {
                constexpr char kHex[] = "0123456789abcdef";
                out << "\\u00" << kHex[(ch >> 4) & 0x0f] << kHex[ch & 0x0f];
            } else {
                out << static_cast<char>(ch);
            }
            break;
        }
    }
    out << '"';
}

struct NamedFlag {
    uint32_t mask;
    const char* id;
};

constexpr NamedFlag kCapabilityFlags[] = {
    {FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::sample_probe), "sample_probe"},
    {FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::schema_control_surface), "schema_control_surface"},
    {FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::param_animation_surface), "param_animation_surface"},
    {FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::smooth_escape_coloring), "smooth_escape_coloring"},
    {FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::color_pipeline_frame_coloring), "color_pipeline_frame_coloring"},
    {FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::generic_equation_pack), "generic_equation_pack"},
    {FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::root_basin_coloring), "root_basin_coloring"},
    {FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::sdf_pack_scene), "sdf_pack_scene"},
    {FractalCatalogCapabilityFlagMask(FractalCatalogCapabilityFlag::field_primary_sdf), "field_primary_sdf"},
};

constexpr NamedFlag kRuntimeFlags[] = {
    {FractalCatalogRuntimeFlagMask(FractalCatalogRuntimeFlag::escape_time), "escape_time"},
    {FractalCatalogRuntimeFlagMask(FractalCatalogRuntimeFlag::basin_coloring), "basin_coloring"},
    {FractalCatalogRuntimeFlagMask(FractalCatalogRuntimeFlag::explaino_family), "explaino_family"},
    {FractalCatalogRuntimeFlagMask(FractalCatalogRuntimeFlag::perturbation_reference_orbit), "perturbation_reference_orbit"},
};

template <std::size_t N>
void AppendFlags(std::ostringstream& out, uint32_t flags, const NamedFlag (&definitions)[N]) {
    out << '[';
    bool wrote = false;
    for (const auto& definition : definitions) {
        if ((flags & definition.mask) == 0u) continue;
        if (wrote) out << ", ";
        AppendJsonString(out, definition.id);
        wrote = true;
    }
    out << ']';
}

std::string JoinSentences(const FractalDescriptionField& field) {
    std::ostringstream out;
    for (std::size_t index = 0; index < field.sentence_count; ++index) {
        if (index > 0) out << ' ';
        out << field.sentences[index].text;
    }
    return out.str();
}

void AppendReviewedDescription(std::ostringstream& out, const FractalReviewedDescription& description) {
    out << "{\n";
    for (std::size_t fieldIndex = 0; fieldIndex < description.field_count; ++fieldIndex) {
        const auto& field = description.fields[fieldIndex];
        out << "        ";
        AppendJsonString(out, field.field_name);
        out << ": ";
        const std::string joined = JoinSentences(field);
        AppendJsonString(out, joined.c_str());
        out << ",\n";
    }
    out << "        \"source_refs\": [";
    for (std::size_t index = 0; index < description.source_ref_count; ++index) {
        if (index > 0) out << ", ";
        AppendJsonString(out, description.source_refs[index]);
    }
    out << "]\n      }";
}

bool SetFileError(const std::string& message, std::string* outError) {
    if (outError) *outError = message;
    return false;
}

bool RemoveOwnedTemp(const std::filesystem::path& temp, const std::string& stage, std::string* outError) {
    std::error_code removeError;
    std::filesystem::remove(temp, removeError);
    if (removeError) {
        return SetFileError(stage + ": failed to clean owned temporary file " + temp.string() + ": " + removeError.message(), outError);
    }
    return true;
}

} // namespace

std::size_t FractalReviewedDescriptionCount() {
    return ArraySize(kReviewedDescriptions);
}

const FractalReviewedDescription& FractalReviewedDescriptionAt(std::size_t index) {
    return kReviewedDescriptions[index];
}

const FractalReviewedDescription* FindFractalReviewedDescription(FractalType fractalType) {
    for (const auto& description : kReviewedDescriptions) {
        if (description.fractal_type == fractalType) return &description;
    }
    return nullptr;
}

std::string BuildFractalDescriptiveCatalogJson() {
    std::ostringstream out;
    out << "{\n  \"schema_version\": 1,\n  \"entries\": [";
    for (std::size_t index = 0; index < FractalCatalogCount(); ++index) {
        const auto& entry = kFractalCatalog[index];
        const auto* description = FindFractalReviewedDescription(entry.type);
        if (index > 0) out << ',';
        out << "\n    {\n      \"selector_id\": ";
        AppendJsonString(out, entry.selector_id);
        out << ",\n      \"display_name\": ";
        AppendJsonString(out, entry.display_name);
        out << ",\n      \"category\": ";
        AppendJsonString(out, CategoryId(entry.category));
        out << ",\n      \"family\": ";
        AppendJsonString(out, FamilyId(entry.family));
        out << ",\n      \"formula_growth_surface\": ";
        AppendJsonString(out, FormulaGrowthSurfaceId(entry.formula_growth_surface));
        out << ",\n      \"capability_flags\": ";
        AppendFlags(out, entry.capability_flags, kCapabilityFlags);
        out << ",\n      \"runtime_flags\": ";
        AppendFlags(out, entry.runtime_flags, kRuntimeFlags);
        out << ",\n      \"description_status\": ";
        AppendJsonString(out, description ? "reviewed" : "unavailable");
        out << ",\n      \"description\": ";
        if (description) {
            AppendReviewedDescription(out, *description);
        } else {
            out << "null";
        }
        out << "\n    }";
    }
    out << "\n  ]\n}\n";
    return out.str();
}

bool WriteFractalDescriptiveCatalogJsonFile(const std::string& path, std::string* outError) {
    if (path.empty()) return SetFileError("descriptive catalog output path is empty", outError);
    const std::filesystem::path target(path);
    const std::filesystem::path parent = target.parent_path();
    std::error_code statusError;
    if (!parent.empty() && (!std::filesystem::exists(parent, statusError) || !std::filesystem::is_directory(parent, statusError))) {
        return SetFileError("descriptive catalog parent directory does not exist: " + parent.string(), outError);
    }
    if (statusError) return SetFileError("descriptive catalog parent check failed: " + statusError.message(), outError);

    const std::filesystem::path temp(path + ".tmp");
    std::error_code removeError;
    std::filesystem::remove(temp, removeError);
    if (removeError) return SetFileError("descriptive catalog stale temporary cleanup failed: " + removeError.message(), outError);

    const std::string bytes = BuildFractalDescriptiveCatalogJson();
    {
        std::ofstream file(temp, std::ios::binary | std::ios::trunc);
        if (!file) return SetFileError("descriptive catalog temporary open failed: " + temp.string(), outError);
        file.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
        file.flush();
        if (!file.good()) {
            file.close();
            RemoveOwnedTemp(temp, "descriptive catalog write failure", nullptr);
            return SetFileError("descriptive catalog temporary write failed: " + temp.string(), outError);
        }
        file.close();
        if (file.fail()) {
            RemoveOwnedTemp(temp, "descriptive catalog close failure", nullptr);
            return SetFileError("descriptive catalog temporary close failed: " + temp.string(), outError);
        }
    }

    std::error_code existsError;
    const bool targetExists = std::filesystem::exists(target, existsError);
    if (existsError) {
        RemoveOwnedTemp(temp, "descriptive catalog target check failure", nullptr);
        return SetFileError("descriptive catalog target check failed: " + existsError.message(), outError);
    }
    if (targetExists) {
        std::error_code targetRemoveError;
        if (!std::filesystem::remove(target, targetRemoveError) || targetRemoveError) {
            RemoveOwnedTemp(temp, "descriptive catalog replacement failure", nullptr);
            return SetFileError("descriptive catalog existing target removal failed: " + target.string() + (targetRemoveError ? ": " + targetRemoveError.message() : std::string()), outError);
        }
    }

    std::error_code renameError;
    std::filesystem::rename(temp, target, renameError);
    if (!renameError) return true;

    std::error_code copyError;
    std::filesystem::copy_file(temp, target, std::filesystem::copy_options::overwrite_existing, copyError);
    if (copyError) {
        RemoveOwnedTemp(temp, "descriptive catalog replacement failure", nullptr);
        return SetFileError("descriptive catalog replacement failed after rename and copy fallback: " + renameError.message() + "; " + copyError.message(), outError);
    }
    if (!RemoveOwnedTemp(temp, "descriptive catalog copy fallback", outError)) return false;
    return true;
}
