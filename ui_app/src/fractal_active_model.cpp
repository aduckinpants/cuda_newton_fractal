#include "fractal_active_model.h"

#include "enum_id_utils.h"
#include "fractal_family_rules.h"
#include "sample_tier_resolver.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <system_error>

namespace {

struct ActiveModelProvider {
    const char* provider_id;
    int provider_version;
    bool (*matches)(const ViewState&, FractalType);
    const char* (*unavailable_reason)(const KernelParams&);
    void (*append_model)(std::ostringstream&, const KernelParams&, const ResolvedEvalMode&);
};

bool Fail(const std::string& message, std::string* outError) {
    if (outError) *outError = message;
    return false;
}

bool IsSha256Hex(const std::string& value) {
    return value.size() == 64 && std::all_of(value.begin(), value.end(), [](unsigned char ch) {
        return std::isdigit(ch) || (ch >= 'a' && ch <= 'f');
    });
}

const char* NumericBackendId(NumericBackend backend) {
    switch (backend) {
    case NumericBackend::float32: return "float32";
    case NumericBackend::float64: return "float64";
    }
    return "unknown";
}

const char* IterationStrategyId(IterationStrategy strategy) {
    switch (strategy) {
    case IterationStrategy::direct: return "direct";
    }
    return "unknown";
}

void AppendJsonString(std::ostringstream& out, const std::string& value) {
    out << '"';
    for (const unsigned char ch : value) {
        switch (ch) {
        case '"': out << "\\\""; break;
        case '\\': out << "\\\\"; break;
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

int ClampDenominatorPower(int value) {
    return (std::max)(1, (std::min)(6, value));
}

bool MatchesPolynomialOverPowerEscape(const ViewState& view, FractalType runtimeFractalType) {
    return view.fractal_type == FractalType::explaino_rational_escape &&
        runtimeFractalType == FractalType::explaino_rational_escape;
}

const char* PolynomialOverPowerEscapeUnavailableReason(const KernelParams& params) {
    return params.explaino_warp_strength == 0.0f ? nullptr : "nonzero_warp_unsupported";
}

void AppendPolynomialOverPowerEscapeModel(
    std::ostringstream& out,
    const KernelParams& params,
    const ResolvedEvalMode& resolved) {
    out << "{\n"
        << "    \"model_id\": \"laurent_polynomial_escape_time.v1\",\n"
        << "    \"recurrence_id\": \"z_next_equals_real_polynomial_degree4_over_z_power\",\n"
        << "    \"coefficient_order\": \"ascending_power\",\n"
        << "    \"real_polynomial_coefficients\": ["
        << static_cast<double>(params.poly_coeffs[0]) << ", "
        << static_cast<double>(params.poly_coeffs[1]) << ", "
        << static_cast<double>(params.poly_coeffs[2]) << ", "
        << static_cast<double>(params.poly_coeffs[3]) << ", "
        << static_cast<double>(params.poly_coeffs[4]) << "],\n"
        << "    \"denominator_power\": " << ClampDenominatorPower(params.explaino_rational_escape_denominator_power) << ",\n"
        << "    \"max_iterations\": " << params.max_iter << ",\n"
        << "    \"pole_threshold_abs2\": "
        << (resolved.backend == NumericBackend::float64 ? 1.0e-30 : 1.0e-20) << ",\n"
        << "    \"escape_radius_abs2\": 10000,\n"
        << "    \"termination_kinds\": [\"pole\", \"escaped_radius\", \"nonfinite\", \"max_iterations\"],\n"
        << "    \"structural_singular_points\": [{\"real\": 0, \"imag\": 0, \"kind\": \"denominator_zero\"}]\n"
        << "  }";
}

constexpr ActiveModelProvider kProviders[] = {
    {
        "polynomial_over_power_escape.v1",
        1,
        MatchesPolynomialOverPowerEscape,
        PolynomialOverPowerEscapeUnavailableReason,
        AppendPolynomialOverPowerEscapeModel,
    },
};

const ActiveModelProvider* FindProvider(const ViewState& view, FractalType runtimeFractalType) {
    for (const ActiveModelProvider& provider : kProviders) {
        if (provider.matches(view, runtimeFractalType)) return &provider;
    }
    return nullptr;
}

bool RemoveOwnedTemp(const std::filesystem::path& temp) {
    std::error_code ignored;
    std::filesystem::remove(temp, ignored);
    return !ignored;
}

} // namespace

std::size_t ActiveFractalModelProviderCount() {
    return sizeof(kProviders) / sizeof(kProviders[0]);
}

const char* ActiveFractalModelProviderIdAt(std::size_t index) {
    return index < ActiveFractalModelProviderCount() ? kProviders[index].provider_id : nullptr;
}

bool BuildActiveFractalModelReceiptJson(
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const ActiveFractalModelReceiptContext& context,
    std::string* outJson,
    std::string* outError) {
    if (outJson) outJson->clear();
    if (outError) outError->clear();
    if (!outJson) return Fail("active-model receipt output pointer is required", outError);
    if (!IsSha256Hex(context.state_json_sha256)) {
        return Fail("active-model receipt requires a lowercase SHA-256 state binding", outError);
    }
    if (!IsSha256Hex(context.runtime_executable_sha256)) {
        return Fail("active-model receipt requires a lowercase SHA-256 runtime binding", outError);
    }

    const char* selectedId = FractalTypeId(view.fractal_type);
    const FractalType runtimeFractalType = ResolveExplainoRuntimeFractalType(view.fractal_type, params);
    const char* runtimeId = FractalTypeId(runtimeFractalType);
    if (!selectedId || !runtimeId) return Fail("active-model receipt requires live fractal identities", outError);

    const ResolvedEvalMode resolved = ResolveSampleEvalModeForRender(
        view.fractal_type, params, render.sample_tier, view.log2_zoom);
    const ActiveModelProvider* provider = FindProvider(view, runtimeFractalType);
    const char* unavailableReason = provider
        ? provider->unavailable_reason(params)
        : "unsupported_fractal_type";
    const bool available = provider && unavailableReason == nullptr;

    std::ostringstream out;
    out << std::setprecision(17);
    out << "{\n"
        << "  \"schema_version\": 1,\n"
        << "  \"state_binding\": {\n"
        << "    \"state_json_sha256\": \"" << context.state_json_sha256 << "\",\n"
        << "    \"runtime_executable_sha256\": \"" << context.runtime_executable_sha256 << "\"\n"
        << "  },\n"
        << "  \"selected_fractal_type\": ";
    AppendJsonString(out, selectedId);
    out << ",\n  \"resolved_runtime_fractal_type\": ";
    AppendJsonString(out, runtimeId);
    out << ",\n  \"provider\": {\n"
        << "    \"status\": \"" << (available ? "available" : "unavailable") << "\",\n"
        << "    \"provider_id\": ";
    if (provider) AppendJsonString(out, provider->provider_id); else out << "null";
    out << ",\n    \"provider_version\": ";
    if (provider) out << provider->provider_version; else out << "null";
    out << ",\n"
        << "    \"unavailable_reason\": ";
    if (available) out << "null"; else AppendJsonString(out, unavailableReason);
    out << "\n  },\n"
        << "  \"numeric_authority\": {\n"
        << "    \"requested_sample_tier\": ";
    AppendJsonString(out, SampleTierId(render.sample_tier) ? SampleTierId(render.sample_tier) : "unknown");
    out << ",\n    \"resolved_backend\": \"" << NumericBackendId(resolved.backend) << "\",\n"
        << "    \"iteration_strategy\": \"" << IterationStrategyId(resolved.strategy) << "\"\n"
        << "  },\n"
        << "  \"evaluation_authority\": {\n"
        << "    \"evaluation_surface\": \"fractal.sample\",\n"
        << "    \"state_binding_required\": true,\n"
        << "    \"runtime_binding_required\": true\n"
        << "  },\n"
        << "  \"participating_state\": [\n"
        << "    {\"path\": \"view.fractal_type\", \"value\": ";
    AppendJsonString(out, selectedId);
    out << "},\n"
        << "    {\"path\": \"view.log2_zoom\", \"value\": " << view.log2_zoom << "},\n"
        << "    {\"path\": \"render.sample_tier\", \"value\": ";
    AppendJsonString(out, SampleTierId(render.sample_tier) ? SampleTierId(render.sample_tier) : "unknown");
    out << "},\n"
        << "    {\"path\": \"params.poly_coeffs\", \"value\": ["
        << static_cast<double>(params.poly_coeffs[0]) << ", "
        << static_cast<double>(params.poly_coeffs[1]) << ", "
        << static_cast<double>(params.poly_coeffs[2]) << ", "
        << static_cast<double>(params.poly_coeffs[3]) << ", "
        << static_cast<double>(params.poly_coeffs[4]) << "]},\n"
        << "    {\"path\": \"params.explaino_rational_escape_denominator_power\", \"value\": "
        << params.explaino_rational_escape_denominator_power << "},\n"
        << "    {\"path\": \"params.explaino_warp_strength\", \"value\": "
        << static_cast<double>(params.explaino_warp_strength) << "},\n"
        << "    {\"path\": \"params.max_iter\", \"value\": " << params.max_iter << "}\n"
        << "  ],\n"
        << "  \"model\": ";
    if (!available) out << "null";
    else provider->append_model(out, params, resolved);
    out << "\n";
    out << "}\n";
    *outJson = out.str();
    return true;
}

bool WriteActiveFractalModelReceiptJsonFile(
    const std::string& path,
    const std::string& json,
    std::string* outError) {
    if (outError) outError->clear();
    if (path.empty()) return Fail("active-model receipt output path is empty", outError);
    const std::filesystem::path target(path);
    const std::filesystem::path parent = target.parent_path();
    if (!parent.empty() && !std::filesystem::exists(parent)) {
        return Fail("active-model receipt parent directory does not exist: " + parent.string(), outError);
    }
    const std::filesystem::path temp(path + ".tmp");
    std::error_code ignored;
    std::filesystem::remove(temp, ignored);
    {
        std::ofstream out(temp, std::ios::binary | std::ios::trunc);
        if (!out) return Fail("failed to open active-model receipt temporary file: " + temp.string(), outError);
        out.write(json.data(), static_cast<std::streamsize>(json.size()));
        if (!out.good()) {
            out.close();
            RemoveOwnedTemp(temp);
            return Fail("failed to write active-model receipt temporary file: " + temp.string(), outError);
        }
    }

    std::error_code renameError;
    std::filesystem::rename(temp, target, renameError);
    if (!renameError) return true;
    std::error_code copyError;
    std::filesystem::copy_file(temp, target, std::filesystem::copy_options::overwrite_existing, copyError);
    RemoveOwnedTemp(temp);
    if (copyError) {
        return Fail("active-model receipt replacement failed: " + renameError.message() + "; " + copyError.message(), outError);
    }
    return true;
}
