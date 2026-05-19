#include "runtime_walk.h"

#include "diagnostics_capture.h"
#include "explaino_seed.h"
#include "explaino_sidecar_cuda_sample_host.h"
#include "explaino_sidecar_measurement.h"
#include "explaino_sidecar_window.h"
#include "finding_state_actions.h"
#include "fractal_derived_fields.h"
#include "fractal_family_rules.h"
#include "lens_sdf.h"
#include "schema_binding.h"
#include "view_hp_sync.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

namespace {

struct ReferencePoint {
    int low_x = 0;
    int low_y = 0;
    int render_x = 0;
    int render_y = 0;
};

struct ProbeUv {
    float u = 0.5f;
    float v = 0.5f;
};

constexpr ProbeUv kProbeUvs[5] = {
    {0.5f, 0.5f},
    {0.25f, 0.25f},
    {0.75f, 0.25f},
    {0.25f, 0.75f},
    {0.75f, 0.75f},
};

const char* FractalTypeId(FractalType fractalType) {
    switch (fractalType) {
    case FractalType::newton: return "newton";
    case FractalType::nova: return "nova";
    case FractalType::mandelbrot: return "mandelbrot";
    case FractalType::julia: return "julia";
    case FractalType::burning_ship: return "burning_ship";
    case FractalType::multibrot: return "multibrot";
    case FractalType::phoenix: return "phoenix";
    case FractalType::explaino: return "explaino";
    case FractalType::explaino_all: return "explaino_all";
    case FractalType::explaino_y: return "explaino_y";
    case FractalType::explaino_fp: return "explaino_fp";
    case FractalType::explaino_nova: return "explaino_nova";
    case FractalType::explaino_halley: return "explaino_halley";
    case FractalType::explaino_dual: return "explaino_dual";
    case FractalType::explaino_mult: return "explaino_mult";
    case FractalType::explaino_phoenix: return "explaino_phoenix";
    case FractalType::explaino_transcendental: return "explaino_transcendental";
    case FractalType::explaino_inertial: return "explaino_inertial";
    case FractalType::explaino_julia: return "explaino_julia";
    case FractalType::explaino_rational: return "explaino_rational";
    case FractalType::explaino_joy: return "explaino_joy";
    case FractalType::explaino_fold: return "explaino_fold";
    case FractalType::explaino_bell: return "explaino_bell";
    case FractalType::explaino_ripple: return "explaino_ripple";
    case FractalType::explaino_splice: return "explaino_splice";
    case FractalType::explaino_vortex: return "explaino_vortex";
    case FractalType::explaino_tension: return "explaino_tension";
    case FractalType::explaino_balance_void: return "explaino_balance_void";
    case FractalType::multicorn: return "multicorn";
    case FractalType::halley: return "halley";
    case FractalType::collatz: return "collatz";
    case FractalType::explaino_collatz: return "explaino_collatz";
    case FractalType::mcmullen: return "mcmullen";
    case FractalType::lambda_map: return "lambda";
    case FractalType::explaino_lambda: return "explaino_lambda";
    case FractalType::explaino_rational_escape: return "explaino_rational_escape";
    case FractalType::spider: return "spider";
    case FractalType::celtic_mandelbrot: return "celtic_mandelbrot";
    case FractalType::perpendicular_burning_ship: return "perpendicular_burning_ship";
    case FractalType::magnet: return "magnet";
    case FractalType::counterfactual_pair: return "counterfactual_pair";
    case FractalType::explaino_counterfactual_pair: return "explaino_counterfactual_pair";
    case FractalType::projection_and_flow: return "projection_and_flow";
    case FractalType::explaino_projection_and_flow: return "explaino_projection_and_flow";
    }
    return "unknown";
}

std::string JsonEscape(const std::string& text) {
    std::string escaped;
    escaped.reserve(text.size() + 16);
    for (unsigned char ch : text) {
        switch (ch) {
        case '\\': escaped += "\\\\"; break;
        case '"': escaped += "\\\""; break;
        case '\b': escaped += "\\b"; break;
        case '\f': escaped += "\\f"; break;
        case '\n': escaped += "\\n"; break;
        case '\r': escaped += "\\r"; break;
        case '\t': escaped += "\\t"; break;
        default:
            if (ch < 0x20u) {
                char buffer[7];
                std::snprintf(buffer, sizeof(buffer), "\\u%04x", static_cast<unsigned int>(ch));
                escaped += buffer;
            } else {
                escaped.push_back(static_cast<char>(ch));
            }
            break;
        }
    }
    return escaped;
}

bool WriteTextFile(const std::filesystem::path& path, const std::string& text, std::string* outError) {
    std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file) {
        if (outError) *outError = "Failed to open output file for write: " + path.string();
        return false;
    }
    file.write(text.data(), static_cast<std::streamsize>(text.size()));
    if (!file.good()) {
        if (outError) *outError = "Failed to write output file: " + path.string();
        return false;
    }
    return true;
}

bool WriteBmp32Bgra(const std::filesystem::path& path, const uint32_t* rgba, int width, int height, std::string* outError) {
    if (outError) outError->clear();
    if (!rgba || width <= 0 || height <= 0) {
        if (outError) *outError = "Invalid BMP payload";
        return false;
    }

#pragma pack(push, 1)
    struct BmpFileHeader {
        uint16_t bfType;
        uint32_t bfSize;
        uint16_t bfReserved1;
        uint16_t bfReserved2;
        uint32_t bfOffBits;
    };
    struct BmpInfoHeader {
        uint32_t biSize;
        int32_t biWidth;
        int32_t biHeight;
        uint16_t biPlanes;
        uint16_t biBitCount;
        uint32_t biCompression;
        uint32_t biSizeImage;
        int32_t biXPelsPerMeter;
        int32_t biYPelsPerMeter;
        uint32_t biClrUsed;
        uint32_t biClrImportant;
    };
#pragma pack(pop)

    const uint32_t rowBytes = static_cast<uint32_t>(width) * 4u;
    const uint32_t dataBytes = rowBytes * static_cast<uint32_t>(height);

    BmpFileHeader fileHeader{};
    fileHeader.bfType = 0x4D42u;
    fileHeader.bfOffBits = sizeof(BmpFileHeader) + sizeof(BmpInfoHeader);
    fileHeader.bfSize = fileHeader.bfOffBits + dataBytes;

    BmpInfoHeader infoHeader{};
    infoHeader.biSize = sizeof(BmpInfoHeader);
    infoHeader.biWidth = width;
    infoHeader.biHeight = -height;
    infoHeader.biPlanes = 1;
    infoHeader.biBitCount = 32;
    infoHeader.biCompression = 0;
    infoHeader.biSizeImage = dataBytes;
    infoHeader.biXPelsPerMeter = 2835;
    infoHeader.biYPelsPerMeter = 2835;

    std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file) {
        if (outError) *outError = "Failed to open BMP output path: " + path.string();
        return false;
    }

    file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
    file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));

    std::vector<uint8_t> row(static_cast<std::size_t>(rowBytes), 0);
    for (int y = 0; y < height; ++y) {
        const uint32_t* src = rgba + static_cast<std::size_t>(y) * static_cast<std::size_t>(width);
        for (int x = 0; x < width; ++x) {
            const uint32_t pixel = src[x];
            const std::size_t offset = static_cast<std::size_t>(x) * 4u;
            row[offset + 0] = static_cast<uint8_t>((pixel >> 16) & 0xFFu);
            row[offset + 1] = static_cast<uint8_t>((pixel >> 8) & 0xFFu);
            row[offset + 2] = static_cast<uint8_t>(pixel & 0xFFu);
            row[offset + 3] = static_cast<uint8_t>((pixel >> 24) & 0xFFu);
        }
        file.write(reinterpret_cast<const char*>(row.data()), static_cast<std::streamsize>(row.size()));
        if (!file.good()) {
            if (outError) *outError = "Failed while writing BMP output path: " + path.string();
            return false;
        }
    }
    return true;
}

void DownsampleMask2x(const uint8_t* inMask, int inW, int inH, std::vector<uint8_t>* outMask, int* outW, int* outH) {
    const int reducedW = (inW + 1) / 2;
    const int reducedH = (inH + 1) / 2;
    outMask->assign(static_cast<std::size_t>(reducedW) * static_cast<std::size_t>(reducedH), 0);
    for (int y = 0; y < reducedH; ++y) {
        int sampleY = y * 2;
        if (sampleY >= inH) sampleY = inH - 1;
        for (int x = 0; x < reducedW; ++x) {
            int sampleX = x * 2;
            if (sampleX >= inW) sampleX = inW - 1;
            (*outMask)[static_cast<std::size_t>(y) * static_cast<std::size_t>(reducedW) + static_cast<std::size_t>(x)] =
                inMask[static_cast<std::size_t>(sampleY) * static_cast<std::size_t>(inW) + static_cast<std::size_t>(sampleX)];
        }
    }
    *outW = reducedW;
    *outH = reducedH;
}

void DownsampleMaskPow2(const uint8_t* inMask, int inW, int inH, int downsample, std::vector<uint8_t>* outMask, int* outW, int* outH) {
    int ds = 1;
    if (downsample <= 1) ds = 1;
    else if (downsample <= 2) ds = 2;
    else if (downsample <= 4) ds = 4;
    else ds = 8;

    if (ds <= 1) {
        *outW = inW;
        *outH = inH;
        outMask->assign(inMask, inMask + static_cast<std::size_t>(inW) * static_cast<std::size_t>(inH));
        return;
    }

    std::vector<uint8_t> tmpA;
    std::vector<uint8_t> tmpB;
    const uint8_t* current = inMask;
    int currentW = inW;
    int currentH = inH;
    int steps = 0;
    for (int x = ds; x > 1; x >>= 1) ++steps;
    for (int step = 0; step < steps; ++step) {
        std::vector<uint8_t>* target = (step % 2 == 0) ? &tmpA : &tmpB;
        DownsampleMask2x(current, currentW, currentH, target, &currentW, &currentH);
        current = target->data();
    }

    *outW = currentW;
    *outH = currentH;
    outMask->assign(current, current + static_cast<std::size_t>(currentW) * static_cast<std::size_t>(currentH));
}

ReferencePoint WorldToReferencePoint(
    double worldX,
    double worldY,
    double baseCx,
    double baseCy,
    double baseLog2,
    double aspect,
    const RenderSettings& render,
    int downsample,
    int lensW,
    int lensH) {
    ReferencePoint point;
    const double zoom = SafeZoomFromLog2(baseLog2);
    const double base = 2.0 / std::max(1.0e-30, zoom);
    const double nx = (worldX - baseCx) / (base * aspect);
    const double ny = (worldY - baseCy) / base;
    const double px = (nx / 2.0 + 0.5) * static_cast<double>(render.resolution.x);
    const double py = (ny / 2.0 + 0.5) * static_cast<double>(render.resolution.y);
    point.render_x = std::clamp(static_cast<int>(std::floor(px)), 0, std::max(0, render.resolution.x - 1));
    point.render_y = std::clamp(static_cast<int>(std::floor(py)), 0, std::max(0, render.resolution.y - 1));
    const double ds = (downsample > 0) ? static_cast<double>(downsample) : 1.0;
    point.low_x = std::clamp(static_cast<int>(std::floor(px / ds)), 0, std::max(0, lensW - 1));
    point.low_y = std::clamp(static_cast<int>(std::floor(py / ds)), 0, std::max(0, lensH - 1));
    return point;
}

std::string RelativePathString(const std::filesystem::path& root, const std::filesystem::path& path) {
    std::error_code ec;
    std::filesystem::path relative = std::filesystem::relative(path, root, ec);
    return ec ? path.string() : relative.generic_string();
}

bool BuildHeadlessSidecarState(
    const EngineFunctionCatalog& engineCatalog,
    BindingContext& bind,
    CudaSidecarMeasurementHost& measurementHost,
    const SidecarBudgetState* previousBudget,
    const SidecarExplorationCompleteness* previousCompleteness,
    const SidecarOrientationVector* previousOrientation,
    const SidecarSlimeTrace* previousTrace,
    const SidecarAutoDemoControllerPolicy& sidecarControllerPolicy,
    ExplainoSidecarWindowState* outState,
    std::string* outError) {
    return BuildExplainoSidecarWindowState(
        engineCatalog,
        bind,
        &measurementHost,
        previousBudget,
        previousCompleteness,
        previousOrientation,
        previousTrace,
        &sidecarControllerPolicy,
        outState,
        outError);
}

std::string ChannelName(std::size_t index) {
    static const char* kNames[13] = {
        "delta_accuracy",
        "delta_depth",
        "delta_domain_reasoning",
        "delta_narrative_coherence",
        "delta_structure",
        "delta_style",
        "delta_overall",
        "batch_stability_a",
        "batch_stability_b",
        "max_parallel_a_norm",
        "max_parallel_b_norm",
        "outlier_fraction",
        "pattern_overlap_flag",
    };
    return kNames[index];
}

} // namespace

int RunRuntimeWalkRequest(const std::string& exeDir,
    const std::string& requestJsonPath,
    const EngineFunctionCatalog& engineCatalog,
    BindingContext& bind,
    CudaSidecarMeasurementHost& measurementHost,
    const SidecarAutoDemoControllerPolicy& defaultSidecarControllerPolicy,
    LensSettings& lens) {
    RuntimeWalkRequest request;
    std::string error;
    if (!LoadRuntimeWalkRequestFile(requestJsonPath, &request, &error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }

    RuntimeWalkBundle bundle;
    if (!LoadRuntimeWalkBundleFile(request.bundle_json_path, &bundle, &error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }

    ViewState baseView{};
    KernelParams baseParams{};
    RenderSettings render{};
    SidecarOrientationVector loadedOrientation{};
    bool hasLoadedOrientation = false;
    SidecarAutoDemoControllerPolicy loadedControllerPolicy{};
    bool hasLoadedControllerPolicy = false;
    SidecarAutoDemoMutationHistory loadedMutationHistory;
    bool hasLoadedMutationHistory = false;
    std::string resolvedStatePath;
    if (!LoadFindingSelectionIntoRuntime(
            request.base_state_json_path,
            &baseView,
            &baseParams,
            &render,
            &loadedOrientation,
            &hasLoadedOrientation,
            &loadedControllerPolicy,
            &hasLoadedControllerPolicy,
            &loadedMutationHistory,
            &hasLoadedMutationHistory,
            &resolvedStatePath,
            &error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }

    if (!IsExplainoFamily(baseView.fractal_type)) {
        std::fprintf(stderr, "runtime walk requires an Explaino-family base state\n");
        return 1;
    }

    if (render.resolution.x <= 0 || render.resolution.y <= 0) {
        render.resolution = {1024, 768};
    }
    if (render.block_size <= 0) render.block_size = 256;
    if (render.device_id < 0) render.device_id = 0;
    render.benchmark = false;
    lens.enabled = true;

    const SidecarAutoDemoControllerPolicy sidecarControllerPolicy =
        hasLoadedControllerPolicy ? loadedControllerPolicy : defaultSidecarControllerPolicy;

    std::filesystem::path outputRoot = std::filesystem::path(request.output_dir).lexically_normal();
    std::error_code ec;
    std::filesystem::create_directories(outputRoot, ec);
    if (ec) {
        std::fprintf(stderr, "Failed to create runtime walk output directory: %s\n", outputRoot.string().c_str());
        return 1;
    }
    const std::filesystem::path referenceDir = outputRoot / "reference";
    const std::filesystem::path ticksRoot = outputRoot / "ticks";
    std::filesystem::create_directories(referenceDir, ec);
    std::filesystem::create_directories(ticksRoot, ec);
    if (ec) {
        std::fprintf(stderr, "Failed to create runtime walk subdirectories under %s\n", outputRoot.string().c_str());
        return 1;
    }

    ViewState referenceView = baseView;
    KernelParams referenceParams = baseParams;
    if (IsExplainoFamily(referenceView.fractal_type)) {
        UpdateExplainoPolynomial(referenceView, referenceParams, nullptr);
    }
    if (referenceView.auto_max_iter) {
        referenceParams.max_iter = ComputeAutoMaxIter(referenceView.log2_zoom, referenceView.fractal_type);
    }

    std::vector<uint32_t> referenceRgba(static_cast<std::size_t>(render.resolution.x) * static_cast<std::size_t>(render.resolution.y));
    std::vector<uint8_t> referenceMask(static_cast<std::size_t>(render.resolution.x) * static_cast<std::size_t>(render.resolution.y));
    RenderStats referenceStats{};
    const char* renderError = nullptr;
    if (!RenderFractalCUDA(referenceView, referenceParams, render, referenceRgba.data(), referenceMask.data(), &referenceStats, &renderError)) {
        std::fprintf(stderr, "%s\n", renderError ? renderError : "RenderFractalCUDA failed during runtime walk reference render");
        return 1;
    }

    DiagnosticsCaptureResult referenceCapture;
    if (!CaptureDiagnosticsBundleToDir(referenceDir.string(),
            referenceView,
            referenceParams,
            render,
            referenceStats,
            referenceRgba.data(),
            referenceRgba.size(),
            hasLoadedOrientation ? &loadedOrientation : nullptr,
            &sidecarControllerPolicy,
            hasLoadedMutationHistory ? &loadedMutationHistory : nullptr,
            &referenceCapture,
            &error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }

    std::vector<uint32_t> referenceLensSdfRgba;
    ComputeSignedDistanceSdfChamfer(referenceMask.data(), render.resolution.x, render.resolution.y, 48.0f, referenceLensSdfRgba);
    const std::filesystem::path referenceLensSdfPath = referenceDir / "lens_sdf.bmp";
    if (!WriteBmp32Bgra(referenceLensSdfPath, referenceLensSdfRgba.data(), render.resolution.x, render.resolution.y, &error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }

    const int referenceDownsample = (lens.downsample <= 1) ? 1 : (lens.downsample <= 2 ? 2 : (lens.downsample <= 4 ? 4 : 8));
    std::vector<uint8_t> referenceLensMaskLow;
    int referenceLensW = 0;
    int referenceLensH = 0;
    DownsampleMaskPow2(referenceMask.data(), render.resolution.x, render.resolution.y, referenceDownsample, &referenceLensMaskLow, &referenceLensW, &referenceLensH);

    const double baseCx = referenceView.center_hp_x;
    const double baseCy = referenceView.center_hp_y;
    const double baseLog2 = referenceView.log2_zoom;
    const double aspect = render.resolution.y > 0
        ? static_cast<double>(render.resolution.x) / static_cast<double>(render.resolution.y)
        : 1.0;

    std::ostringstream report;
    report.setf(std::ios::fixed);
    report << std::setprecision(8);
    report << "{\n";
    report << "  \"version\": 1,\n";
    report << "  \"kind\": \"explaino_saved_runtime_walk\",\n";
    report << "  \"base_state_json\": \"" << JsonEscape(resolvedStatePath) << "\",\n";
    report << "  \"bundle_json\": \"" << JsonEscape(request.bundle_json_path) << "\",\n";
    if (!request.comparison_fits_path.empty()) {
        report << "  \"comparison_fits\": \"" << JsonEscape(request.comparison_fits_path) << "\",\n";
    }
    report << "  \"bundle\": {\n";
    report << "    \"field_name\": \"" << JsonEscape(bundle.field_name) << "\",\n";
    report << "    \"sample_count\": " << bundle.samples.size() << ",\n";
    report << "    \"channel_names\": [";
    for (std::size_t index = 0; index < 13; ++index) {
        if (index) report << ", ";
        report << "\"" << ChannelName(index) << "\"";
    }
    report << "]\n";
    report << "  },\n";
    report << "  \"reference_view\": {\n";
    report << "    \"frame_bmp\": \"" << JsonEscape(RelativePathString(outputRoot, std::filesystem::path(referenceCapture.frame_bmp_path))) << "\",\n";
    report << "    \"lens_sdf_bmp\": \"" << JsonEscape(RelativePathString(outputRoot, referenceLensSdfPath)) << "\",\n";
    report << "    \"state_json\": \"" << JsonEscape(RelativePathString(outputRoot, std::filesystem::path(referenceCapture.state_json_path))) << "\",\n";
    report << "    \"render\": {\n";
    report << "      \"last_render_ms\": " << static_cast<double>(referenceStats.last_render_ms) << ",\n";
    report << "      \"last_iters_avg\": " << referenceStats.last_iters_avg << ",\n";
    report << "      \"device_id\": " << referenceStats.last_device_id << "\n";
    report << "    },\n";
    report << "    \"lens\": {\n";
    report << "      \"downsample\": " << referenceDownsample << ",\n";
    report << "      \"lens_low_size\": [" << referenceLensW << ", " << referenceLensH << "]\n";
    report << "    }\n";
    report << "  },\n";
    report << "  \"trace\": [\n";

    ExplainoSidecarWindowState previousSidecarState;
    bool previousSidecarStateValid = false;
    SidecarBudgetState previousBudgetState;
    bool previousBudgetStateValid = false;
    SidecarOrientationVector previousOrientation = loadedOrientation;
    bool hasPreviousOrientation = hasLoadedOrientation;

    double maxOrientationDelta = 0.0;
    double maxResidualMagnitude = 0.0;
    int bestSaddleTick = 0;
    double bestSaddleValue = 1.0e30;

    for (std::size_t tickIndex = 0; tickIndex < request.t_values.size(); ++tickIndex) {
        const double t = request.t_values[tickIndex];
        RuntimeWalkSnapshot snapshot;
        if (!EvaluateRuntimeWalkSnapshot(bundle, t, baseView, baseParams, render, &snapshot, &error)) {
            std::fprintf(stderr, "%s\n", error.c_str());
            return 1;
        }

        ViewState tickView = baseView;
        KernelParams tickParams = baseParams;
        ApplyRuntimeWalkSnapshot(snapshot, &tickView, &tickParams);
        if (IsExplainoFamily(tickView.fractal_type)) {
            UpdateExplainoPolynomial(tickView, tickParams, nullptr);
        }
        if (tickView.auto_max_iter) {
            tickParams.max_iter = ComputeAutoMaxIter(tickView.log2_zoom, tickView.fractal_type);
        }

        bind.view = &tickView;
        bind.params = &tickParams;
        bind.render = &render;
        bind.lens = &lens;

        ExplainoSidecarWindowState tickSidecarState;
        const SidecarOrientationVector* previousOrientationPtr = hasPreviousOrientation ? &previousOrientation : nullptr;
        const SidecarBudgetState* previousBudgetPtr = previousBudgetStateValid ? &previousBudgetState : nullptr;
        const SidecarExplorationCompleteness* previousCompletenessPtr =
            previousSidecarStateValid ? &previousSidecarState.completeness : nullptr;
        const SidecarSlimeTrace* previousTracePtr =
            (previousSidecarStateValid && !previousSidecarState.trace.function_id.empty()) ? &previousSidecarState.trace : nullptr;
        if (!BuildHeadlessSidecarState(
                engineCatalog,
                bind,
                measurementHost,
                previousBudgetPtr,
                previousCompletenessPtr,
                previousOrientationPtr,
                previousTracePtr,
                sidecarControllerPolicy,
                &tickSidecarState,
                &error)) {
            std::fprintf(stderr, "%s\n", error.c_str());
            return 1;
        }
        previousSidecarState = tickSidecarState;
        previousSidecarStateValid = true;
        if (!tickSidecarState.budget.function_id.empty()) {
            previousBudgetState = tickSidecarState.budget;
            previousBudgetStateValid = true;
        }
        if (tickSidecarState.has_orientation) {
            previousOrientation = tickSidecarState.orientation;
            hasPreviousOrientation = true;
        }

        std::vector<uint32_t> rgba(static_cast<std::size_t>(render.resolution.x) * static_cast<std::size_t>(render.resolution.y));
        std::vector<uint8_t> mask(static_cast<std::size_t>(render.resolution.x) * static_cast<std::size_t>(render.resolution.y));
        RenderStats stats{};
        if (!RenderFractalCUDA(tickView, tickParams, render, rgba.data(), mask.data(), &stats, &renderError)) {
            std::fprintf(stderr, "%s\n", renderError ? renderError : "RenderFractalCUDA failed during runtime walk tick render");
            return 1;
        }

        std::vector<uint8_t> lensMaskLow;
        int lensW = 0;
        int lensH = 0;
        DownsampleMaskPow2(mask.data(), render.resolution.x, render.resolution.y, referenceDownsample, &lensMaskLow, &lensW, &lensH);

        float minAbsSigned = 1.0e30f;
        int nearCount = 0;
        int insideCount = 0;
        constexpr float kNearEpsPx = 2.0f;
        for (int sampleIndex = 0; sampleIndex < 5; ++sampleIndex) {
            int x = static_cast<int>(std::floor(kProbeUvs[sampleIndex].u * static_cast<float>(lensW)));
            int y = static_cast<int>(std::floor(kProbeUvs[sampleIndex].v * static_cast<float>(lensH)));
            x = std::clamp(x, 0, std::max(0, lensW - 1));
            y = std::clamp(y, 0, std::max(0, lensH - 1));
            float signedPx = 0.0f;
            bool inside = false;
            if (SampleSignedDistanceSdfChamfer(lensMaskLow.data(), lensW, lensH, x, y, signedPx, inside)) {
                minAbsSigned = std::min(minAbsSigned, std::fabs(signedPx));
                if (std::fabs(signedPx) <= kNearEpsPx) ++nearCount;
            }
            if (inside) ++insideCount;
        }

        const ReferencePoint referencePoint = WorldToReferencePoint(
            snapshot.center_hp_x,
            snapshot.center_hp_y,
            baseCx,
            baseCy,
            baseLog2,
            aspect,
            render,
            referenceDownsample,
            referenceLensW,
            referenceLensH);

        std::array<float, 5> referenceSignedPx{};
        std::array<bool, 5> referenceInside{};
        float referenceMinAbsSigned = 1.0e30f;
        int referenceNearCount = 0;
        int referenceInsideCount = 0;
        static constexpr int kSampleOffset = 4;
        const int offsets[5][2] = {
            {0, 0},
            {-kSampleOffset, -kSampleOffset},
            {+kSampleOffset, -kSampleOffset},
            {-kSampleOffset, +kSampleOffset},
            {+kSampleOffset, +kSampleOffset},
        };
        for (int sampleIndex = 0; sampleIndex < 5; ++sampleIndex) {
            const int x = std::clamp(referencePoint.low_x + offsets[sampleIndex][0], 0, std::max(0, referenceLensW - 1));
            const int y = std::clamp(referencePoint.low_y + offsets[sampleIndex][1], 0, std::max(0, referenceLensH - 1));
            float signedPx = 0.0f;
            bool inside = false;
            if (SampleSignedDistanceSdfChamfer(referenceLensMaskLow.data(), referenceLensW, referenceLensH, x, y, signedPx, inside)) {
                referenceMinAbsSigned = std::min(referenceMinAbsSigned, std::fabs(signedPx));
                if (std::fabs(signedPx) <= kNearEpsPx) ++referenceNearCount;
            }
            referenceSignedPx[static_cast<std::size_t>(sampleIndex)] = signedPx;
            referenceInside[static_cast<std::size_t>(sampleIndex)] = inside;
            if (inside) ++referenceInsideCount;
        }

        char tickName[32];
        std::snprintf(tickName, sizeof(tickName), "tick_%04u", static_cast<unsigned int>(tickIndex));
        const std::filesystem::path tickDir = ticksRoot / tickName;
        DiagnosticsCaptureResult tickCapture;
        const SidecarOrientationVector* tickOrientation =
            tickSidecarState.has_orientation ? &tickSidecarState.orientation : nullptr;
        if (!CaptureDiagnosticsBundleToDir(tickDir.string(),
                tickView,
                tickParams,
                render,
                stats,
                rgba.data(),
                rgba.size(),
                tickOrientation,
                &sidecarControllerPolicy,
                hasLoadedMutationHistory ? &loadedMutationHistory : nullptr,
                &tickCapture,
                &error)) {
            std::fprintf(stderr, "%s\n", error.c_str());
            return 1;
        }

        std::vector<uint32_t> tickLensSdfRgba;
        ComputeSignedDistanceSdfChamfer(mask.data(), render.resolution.x, render.resolution.y, 48.0f, tickLensSdfRgba);
        const std::filesystem::path tickLensPath = tickDir / "lens_sdf.bmp";
        if (!WriteBmp32Bgra(tickLensPath, tickLensSdfRgba.data(), render.resolution.x, render.resolution.y, &error)) {
            std::fprintf(stderr, "%s\n", error.c_str());
            return 1;
        }

        const double orientationDelta = tickSidecarState.divergence.scalar_divergence;
        const double decodeStability = tickSidecarState.has_orientation ? tickSidecarState.orientation.decode_stability : 1.0;
        const double slimeEnergy = tickSidecarState.has_orientation ? tickSidecarState.orientation.slime_energy_delta : 0.0;
        const double busyBeaver = tickSidecarState.has_orientation ? tickSidecarState.orientation.busy_beaver_metrics : 0.0;
        const double diffMagnitude = tickSidecarState.has_orientation ? tickSidecarState.orientation.diff_magnitude : 0.0;
        const double residualMagnitude = std::isfinite(minAbsSigned) ? static_cast<double>(minAbsSigned) : 0.0;

        if (orientationDelta > maxOrientationDelta) maxOrientationDelta = orientationDelta;
        if (residualMagnitude > maxResidualMagnitude) maxResidualMagnitude = residualMagnitude;
        if (residualMagnitude < bestSaddleValue) {
            bestSaddleValue = residualMagnitude;
            bestSaddleTick = static_cast<int>(tickIndex);
        }

        report << "    {\n";
        report << "      \"tick\": " << tickIndex << ",\n";
        report << "      \"t\": " << t << ",\n";
        report << "      \"state\": {\n";
        report << "        \"fractal_type\": \"" << FractalTypeId(ResolveExplainoPublicFractalType(tickView.fractal_type)) << "\",\n";
        report << "        \"center_hp_x\": " << tickView.center_hp_x << ",\n";
        report << "        \"center_hp_y\": " << tickView.center_hp_y << ",\n";
        report << "        \"log2_zoom\": " << tickView.log2_zoom << ",\n";
        report << "        \"combined_seed\": " << ExplainoSeedCombined(tickView, tickParams) << ",\n";
        report << "        \"explaino_phase\": " << static_cast<double>(tickView.explaino_phase) << ",\n";
        report << "        \"explaino_seed_b\": " << static_cast<double>(tickParams.explaino_seed_b) << ",\n";
        report << "        \"explaino_mix\": " << static_cast<double>(tickParams.explaino_mix) << ",\n";
        report << "        \"explaino_warp_strength\": " << static_cast<double>(tickParams.explaino_warp_strength) << "\n";
        report << "      },\n";
        report << "      \"branch\": {\n";
        report << "        \"nearest_marker_id\": \"" << JsonEscape(snapshot.branch.nearest_marker_id) << "\",\n";
        report << "        \"nearest_marker_label\": \"" << JsonEscape(snapshot.branch.nearest_marker_label) << "\",\n";
        report << "        \"parent_id\": \"" << JsonEscape(snapshot.branch.parent_id) << "\",\n";
        report << "        \"sticky\": " << (snapshot.branch.sticky ? "true" : "false") << ",\n";
        report << "        \"distance\": " << snapshot.branch.distance << ",\n";
        report << "        \"proximity\": " << snapshot.branch.proximity << "\n";
        report << "      },\n";
        report << "      \"observables\": {\n";
        report << "        \"seed01\": " << snapshot.seed01 << ",\n";
        report << "        \"orientation_delta\": " << orientationDelta << ",\n";
        report << "        \"decode_stability\": " << decodeStability << ",\n";
        report << "        \"slime_energy_delta\": " << slimeEnergy << ",\n";
        report << "        \"busy_beaver_metrics\": " << busyBeaver << ",\n";
        report << "        \"diff_magnitude\": " << diffMagnitude << ",\n";
        report << "        \"residual_magnitude\": " << residualMagnitude << "\n";
        report << "      },\n";
        report << "      \"render\": {\n";
        report << "        \"last_render_ms\": " << static_cast<double>(stats.last_render_ms) << ",\n";
        report << "        \"last_iters_avg\": " << stats.last_iters_avg << ",\n";
        report << "        \"device_id\": " << stats.last_device_id << "\n";
        report << "      },\n";
        report << "      \"artifacts\": {\n";
        report << "        \"frame_bmp\": \"" << JsonEscape(RelativePathString(outputRoot, std::filesystem::path(tickCapture.frame_bmp_path))) << "\",\n";
        report << "        \"state_json\": \"" << JsonEscape(RelativePathString(outputRoot, std::filesystem::path(tickCapture.state_json_path))) << "\",\n";
        report << "        \"lens_sdf_bmp\": \"" << JsonEscape(RelativePathString(outputRoot, tickLensPath)) << "\"\n";
        report << "      },\n";
        report << "      \"reference_trace\": {\n";
        report << "        \"render_xy\": [" << referencePoint.render_x << ", " << referencePoint.render_y << "],\n";
        report << "        \"low_xy\": [" << referencePoint.low_x << ", " << referencePoint.low_y << "],\n";
        report << "        \"saddle\": {\n";
        report << "          \"min_abs_signed_px\": " << static_cast<double>(referenceMinAbsSigned) << ",\n";
        report << "          \"near_count\": " << referenceNearCount << ",\n";
        report << "          \"mixed_inside\": " << ((referenceInsideCount > 0 && referenceInsideCount < 5) ? "true" : "false") << "\n";
        report << "        },\n";
        report << "        \"samples_signed_px\": [";
        for (int sampleIndex = 0; sampleIndex < 5; ++sampleIndex) {
            if (sampleIndex) report << ", ";
            report << static_cast<double>(referenceSignedPx[static_cast<std::size_t>(sampleIndex)]);
        }
        report << "],\n";
        report << "        \"samples_inside\": [";
        for (int sampleIndex = 0; sampleIndex < 5; ++sampleIndex) {
            if (sampleIndex) report << ", ";
            report << (referenceInside[static_cast<std::size_t>(sampleIndex)] ? "true" : "false");
        }
        report << "]\n";
        report << "      }\n";
        report << "    }";
        if (tickIndex + 1 < request.t_values.size()) report << ",";
        report << "\n";
    }

    report << "  ],\n";
    report << "  \"summary\": {\n";
    report << "    \"tick_count\": " << request.t_values.size() << ",\n";
    report << "    \"best_saddle_tick\": " << bestSaddleTick << ",\n";
    report << "    \"best_saddle_min_abs_signed_px\": " << bestSaddleValue << ",\n";
    report << "    \"max_orientation_delta\": " << maxOrientationDelta << ",\n";
    report << "    \"max_residual_magnitude\": " << maxResidualMagnitude << "\n";
    report << "  }\n";
    report << "}\n";

    std::ostringstream manifest;
    manifest.setf(std::ios::fixed);
    manifest << std::setprecision(8);
    manifest << "{\n";
    manifest << "  \"version\": 1,\n";
    manifest << "  \"main_line\": {\n";
    manifest << "    \"tick_count\": " << request.t_values.size() << ",\n";
    manifest << "    \"t_values\": [";
    for (std::size_t index = 0; index < request.t_values.size(); ++index) {
        if (index) manifest << ", ";
        manifest << request.t_values[index];
    }
    manifest << "]\n";
    manifest << "  },\n";
    manifest << "  \"branch_markers\": [\n";
    for (std::size_t index = 0; index < bundle.branch_markers.size(); ++index) {
        const RuntimeWalkBranchMarker& marker = bundle.branch_markers[index];
        manifest << "    {\n";
        manifest << "      \"id\": \"" << JsonEscape(marker.id) << "\",\n";
        manifest << "      \"label\": \"" << JsonEscape(marker.label) << "\",\n";
        manifest << "      \"parent_id\": \"" << JsonEscape(marker.parent_id) << "\",\n";
        manifest << "      \"t\": " << marker.t << ",\n";
        manifest << "      \"sticky_radius\": " << marker.sticky_radius << ",\n";
        manifest << "      \"sticky_interval\": [" << (marker.t - marker.sticky_radius) << ", " << (marker.t + marker.sticky_radius) << "]\n";
        manifest << "    }";
        if (index + 1 < bundle.branch_markers.size()) manifest << ",";
        manifest << "\n";
    }
    manifest << "  ]\n";
    manifest << "}\n";

    const std::filesystem::path reportPath = outputRoot / "runtime_walk_report.json";
    const std::filesystem::path manifestPath = outputRoot / "runtime_walk_branch_manifest.json";
    if (!WriteTextFile(reportPath, report.str(), &error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }
    if (!WriteTextFile(manifestPath, manifest.str(), &error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }
    return 0;
}
