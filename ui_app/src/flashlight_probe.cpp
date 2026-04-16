#include "flashlight_probe.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <vector>

#include "diagnostics_capture.h"
#include "enum_id_utils.h"
#include "fractal_derived_fields.h"
#include "lens_sdf.h"
#include "view_hp_sync.h"

namespace {

struct ProbeUv {
    float u;
    float v;
};

constexpr ProbeUv kProbeUvs[5] = {
    {0.5f, 0.5f},
    {0.25f, 0.25f},
    {0.75f, 0.25f},
    {0.25f, 0.75f},
    {0.75f, 0.75f},
};

bool TryReadTextFile(const std::string& path, std::string* outText, std::string* outError) {
    if (outError) outError->clear();
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        if (outError) *outError = "Failed to read flashlight seed text file: " + path;
        return false;
    }
    std::ostringstream text;
    text << file.rdbuf();
    if (!file.good() && !file.eof()) {
        if (outError) *outError = "Failed while reading flashlight seed text file: " + path;
        return false;
    }
    if (outText) *outText = text.str();
    return true;
}

bool WriteTextFile(const std::string& path, const std::string& text, std::string* outError) {
    if (outError) outError->clear();
    std::ofstream file(path, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file) {
        if (outError) *outError = "Failed to open output file for writing: " + path;
        return false;
    }
    file.write(text.data(), static_cast<std::streamsize>(text.size()));
    if (!file) {
        if (outError) *outError = "Failed to write output file: " + path;
        return false;
    }
    return true;
}

bool WriteBmp32Bgra(const std::string& path, const uint32_t* rgba, int width, int height, std::string* outError) {
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
        if (outError) *outError = "Failed to open BMP output path: " + path;
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
        if (!file) {
            if (outError) *outError = "Failed while writing BMP output path: " + path;
            return false;
        }
    }
    return true;
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
    const int ds = NormalizeFlashlightLensDownsamplePow2(downsample);
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

} // namespace

FlashlightProbeConfig NormalizeFlashlightProbeConfig(const FlashlightProbeConfig& config) {
    FlashlightProbeConfig normalized = config;
    normalized.ticks = std::clamp(normalized.ticks, 1, 4096);
    normalized.radius = ClampD(normalized.radius, 0.0, 10.0);
    normalized.zoom_radius = ClampD(normalized.zoom_radius, 0.0, 10.0);
    normalized.warp = ClampD(normalized.warp, 0.0, 1.0);
    normalized.closure_ref_t = std::max(0, normalized.closure_ref_t);
    return normalized;
}

uint32_t FlashlightFnv1a32(const void* data, std::size_t sizeBytes) {
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    uint32_t hash = 2166136261u;
    for (std::size_t i = 0; i < sizeBytes; ++i) {
        hash ^= static_cast<uint32_t>(bytes[i]);
        hash *= 16777619u;
    }
    return hash;
}

std::array<uint32_t, 8> ComputeFlashlightConversationSpectrum8(const std::string& text) {
    std::array<uint32_t, 8> spectrum{};
    spectrum.fill(2166136261u);
    for (std::size_t i = 0; i < text.size(); ++i) {
        uint32_t& lane = spectrum[i & 7u];
        lane ^= static_cast<uint8_t>(text[i]);
        lane *= 16777619u;
    }
    return spectrum;
}

int NormalizeFlashlightLensDownsamplePow2(int value) {
    if (value <= 1) return 1;
    if (value <= 2) return 2;
    if (value <= 4) return 4;
    return 8;
}

FlashlightManifoldStep FlashlightManifoldAt(
    uint32_t seed32,
    int walkT,
    int bands,
    double baseLog2,
    double radius,
    double zoomRadius,
    double aspect) {
    FlashlightManifoldStep step;
    if (bands < 1) bands = 1;
    step.band = walkT % bands;
    step.band_scale = std::pow(0.5, static_cast<double>(step.band));

    const uint32_t h0 = HashU32(seed32 ^ static_cast<uint32_t>(walkT * 0x9e3779b9u));
    const uint32_t h1 = HashU32(seed32 ^ static_cast<uint32_t>(walkT * 0x85ebca6bu) ^ 0x13579bdu);
    const uint32_t h2 = HashU32(seed32 ^ static_cast<uint32_t>(walkT * 0xc2b2ae35u) ^ 0x2468aceu);
    step.rx = (static_cast<double>(Hash01(h0)) - 0.5) * 2.0;
    step.ry = (static_cast<double>(Hash01(h1)) - 0.5) * 2.0;
    step.rz = (static_cast<double>(Hash01(h2)) - 0.5) * 2.0;
    step.log2_zoom_tick = ClampD(baseLog2 + zoomRadius * step.band_scale * step.rz, Log2D(kMinZoom), kMaxLog2Zoom);

    const double zoomTick = SafeZoomFromLog2(step.log2_zoom_tick);
    const double baseTick = 2.0 / std::max(1.0e-30, zoomTick);
    step.dx_world = radius * step.band_scale * step.rx * baseTick * aspect;
    step.dy_world = radius * step.band_scale * step.ry * baseTick;
    return step;
}

int RunFlashlightProbe(const std::string& exeDir,
    const FlashlightProbeConfig& rawConfig,
    ViewState& view,
    KernelParams& params,
    RenderSettings& render,
    LensSettings& lens) {
    const FlashlightProbeConfig config = NormalizeFlashlightProbeConfig(rawConfig);

    std::string text;
    std::string error;
    if (!TryReadTextFile(config.seed_path, &text, &error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }

    const uint32_t seed32 = FlashlightFnv1a32(text.data(), text.size());
    const std::array<uint32_t, 8> spectrum8 = ComputeFlashlightConversationSpectrum8(text);

    view.fractal_type = config.have_fractal_type ? config.fractal_type : FractalType::explaino_fp;
    lens.enabled = true;

    bool dirty = false;
    ApplyFractalViewPresetDefaults(view, &dirty);
    ApplyFractalPresetDefaults(view, params, &dirty);

    view.camera_behavior = CameraBehavior::manual;
    view.auto_dive = false;
    view.dive_speed = 0.0f;
    view.auto_refresh = false;
    view.auto_increment_seed = false;
    view.explaino_seed_tween = true;
    view.explaino_alive = false;
    view.explaino_phase = 0.0f;
    view.explaino_seed_drift = 0.0f;
    view.center_hp_x = 0.0;
    view.center_hp_y = 0.0;
    view.log2_zoom = 0.0;
    SyncViewUiFromHp(view);

    params.explaino_seed = static_cast<double>(seed32);
    params.explaino_warp_strength = static_cast<float>(config.warp);
    UpdateExplainoPolynomial(view, params, &dirty);

    if (render.resolution.x <= 0 || render.resolution.y <= 0) {
        render.resolution = {1024, 768};
    }
    if (render.block_size <= 0) render.block_size = 256;
    if (render.device_id < 0) render.device_id = 0;
    render.benchmark = false;

    const double baseCx = view.center_hp_x;
    const double baseCy = view.center_hp_y;
    const double baseLog2 = view.log2_zoom;
    const double aspect = render.resolution.y > 0
        ? static_cast<double>(render.resolution.x) / static_cast<double>(render.resolution.y)
        : 1.0;

    std::ostringstream json;
    json.setf(std::ios::fixed);
    json << std::setprecision(8);
    json << "{\n";
    json << "  \"version\": 1,\n";
    json << "  \"seed_path\": \"" << JsonEscape(config.seed_path) << "\",\n";
    json << "  \"conversation_seed32\": " << seed32 << ",\n";
    json << "  \"spectrum8_u32\": [";
    for (std::size_t i = 0; i < spectrum8.size(); ++i) {
        if (i) json << ", ";
        json << spectrum8[i];
    }
    json << "],\n";
    json << "  \"ticks\": " << config.ticks << ",\n";
    json << "  \"radius\": " << config.radius << ",\n";
    json << "  \"zoom_radius\": " << config.zoom_radius << ",\n";
    json << "  \"fractal_type\": \"" << FractalTypeId(view.fractal_type) << "\",\n";
    json << "  \"probe_params\": {\n";
    json << "    \"explaino_warp_strength\": " << static_cast<double>(params.explaino_warp_strength) << "\n";
    json << "  },\n";
    json << "  \"probe_safety\": {\n";
    json << "    \"camera_behavior\": \"manual\",\n";
    json << "    \"auto_dive\": false,\n";
    json << "    \"dive_speed\": 0.0,\n";
    json << "    \"auto_refresh\": false\n";
    json << "  },\n";
    json << "  \"base_camera\": {\n";
    json << "    \"center_hp_x\": " << baseCx << ",\n";
    json << "    \"center_hp_y\": " << baseCy << ",\n";
    json << "    \"log2_zoom\": " << baseLog2 << "\n";
    json << "  },\n";
    json << "  \"schedule\": {\n";
    json << "    \"bands\": 4,\n";
    json << "    \"closure_mode\": \"repeat_ref_tick\",\n";
    json << "    \"closure_last\": " << (config.closure_last ? "true" : "false") << ",\n";
    json << "    \"closure_ref_t\": " << config.closure_ref_t << "\n";
    json << "  },\n";
    json << "  \"trace\": [\n";

    int prevIters = 0;
    float prevMs = 0.0f;
    float prevSigned0 = 0.0f;
    int baseIters0 = 0;
    int peakIters = -1;
    int peakTick = 0;
    float bestSaddleMinAbs = 1.0e30f;
    int bestSaddleTick = 0;
    int bestSaddleNearCount = 0;
    int closureRefIters = 0;
    float closureRefSigned0 = 0.0f;
    bool closureRefComparable = false;
    int closureTickIters = 0;
    float closureTickSigned0 = 0.0f;
    bool closureTickComparable = false;

    std::vector<uint32_t> finalRgba;
    std::vector<uint8_t> finalMask;
    RenderStats finalStats{};

    for (int t = 0; t < config.ticks; ++t) {
        const bool closureTick = config.closure_last && (t == config.ticks - 1);
        const int walkT = closureTick ? config.closure_ref_t : t;
        const FlashlightManifoldStep step = FlashlightManifoldAt(seed32, walkT, 4, baseLog2, config.radius, config.zoom_radius, aspect);

        view.log2_zoom = step.log2_zoom_tick;
        view.center_hp_x = baseCx + step.dx_world;
        view.center_hp_y = baseCy + step.dy_world;
        SyncViewUiFromHp(view);

        view.explaino_seed_drift = static_cast<float>(walkT);
        view.explaino_phase = static_cast<float>(walkT) * 0.15f;
        UpdateExplainoPolynomial(view, params, &dirty);

        std::vector<uint32_t> rgba(static_cast<std::size_t>(render.resolution.x) * static_cast<std::size_t>(render.resolution.y));
        std::vector<uint8_t> mask(static_cast<std::size_t>(render.resolution.x) * static_cast<std::size_t>(render.resolution.y));
        RenderStats stats{};
        const char* renderError = nullptr;
        if (!RenderFractalCUDA(view, params, render, rgba.data(), mask.data(), &stats, &renderError)) {
            std::fprintf(stderr, "%s\n", renderError ? renderError : "RenderFractalCUDA failed during flashlight probe");
            return 1;
        }

        std::vector<uint8_t> lensMaskLow;
        int lensW = 0;
        int lensH = 0;
        const int ds = NormalizeFlashlightLensDownsamplePow2(lens.downsample);
        DownsampleMaskPow2(mask.data(), render.resolution.x, render.resolution.y, ds, &lensMaskLow, &lensW, &lensH);
        const float maxAbsPxLow = 48.0f / static_cast<float>(ds);

        float signed0 = 0.0f;
        bool inside0 = false;
        bool sampled0 = false;
        float minAbsSigned = 1.0e30f;
        int nearCount = 0;
        int insideCount = 0;
        constexpr float kNearEpsPx = 2.0f;

        json << "    {\n";
        json << "      \"t\": " << t << ",\n";
        json << "      \"band\": " << step.band << ",\n";
        json << "      \"closure\": " << (closureTick ? "true" : "false") << ",\n";
        if (closureTick) json << "      \"closure_ref_t\": " << walkT << ",\n";
        json << "      \"explaino_seed\": " << params.explaino_seed << ",\n";
        json << "      \"explaino_seed_drift\": " << static_cast<double>(view.explaino_seed_drift) << ",\n";
        json << "      \"camera\": {\n";
        json << "        \"center_hp_x\": " << view.center_hp_x << ",\n";
        json << "        \"center_hp_y\": " << view.center_hp_y << ",\n";
        json << "        \"log2_zoom\": " << view.log2_zoom << "\n";
        json << "      },\n";
        json << "      \"render\": {\n";
        json << "        \"last_render_ms\": " << static_cast<double>(stats.last_render_ms) << ",\n";
        json << "        \"last_iters_avg\": " << stats.last_iters_avg << ",\n";
        json << "        \"device_id\": " << stats.last_device_id << "\n";
        json << "      },\n";
        json << "      \"lens\": {\n";
        json << "        \"downsample\": " << ds << ",\n";
        json << "        \"lens_low_size\": [" << lensW << ", " << lensH << "],\n";
        json << "        \"sdf_max_abs_px_low\": " << static_cast<double>(maxAbsPxLow) << "\n";
        json << "      },\n";
        json << "      \"samples\": [\n";

        for (int i = 0; i < 5; ++i) {
            int x = static_cast<int>(std::floor(kProbeUvs[i].u * static_cast<float>(lensW)));
            int y = static_cast<int>(std::floor(kProbeUvs[i].v * static_cast<float>(lensH)));
            x = std::clamp(x, 0, std::max(0, lensW - 1));
            y = std::clamp(y, 0, std::max(0, lensH - 1));

            float signedPx = 0.0f;
            bool inside = false;
            const bool ok = SampleSignedDistanceSdfChamfer(lensMaskLow.data(), lensW, lensH, x, y, signedPx, inside);
            if (i == 0) {
                signed0 = signedPx;
                inside0 = inside;
                sampled0 = ok;
            }
            if (ok) {
                const float absSigned = std::fabs(signedPx);
                minAbsSigned = std::min(minAbsSigned, absSigned);
                if (absSigned <= kNearEpsPx) ++nearCount;
            }
            if (inside) ++insideCount;

            json << "        {\"u\": " << static_cast<double>(kProbeUvs[i].u)
                 << ", \"v\": " << static_cast<double>(kProbeUvs[i].v)
                 << ", \"x\": " << x
                 << ", \"y\": " << y
                 << ", \"signed_px\": " << static_cast<double>(signedPx)
                 << ", \"inside\": " << (inside ? "true" : "false")
                 << ", \"ok\": " << (ok ? "true" : "false") << "}";
            if (i + 1 < 5) json << ",";
            json << "\n";
        }

        const bool mixedInside = insideCount > 0 && insideCount < 5;
        const int dIters = t == 0 ? 0 : (stats.last_iters_avg - prevIters);
        const float dMs = t == 0 ? 0.0f : (stats.last_render_ms - prevMs);
        const float dSdf0 = t == 0 ? 0.0f : (signed0 - prevSigned0);

        json << "      ],\n";
        json << "      \"saddle\": {\n";
        json << "        \"min_abs_signed_px\": " << static_cast<double>(minAbsSigned) << ",\n";
        json << "        \"near_eps_px\": " << static_cast<double>(kNearEpsPx) << ",\n";
        json << "        \"near_count\": " << nearCount << ",\n";
        json << "        \"mixed_inside\": " << (mixedInside ? "true" : "false") << "\n";
        json << "      },\n";
        json << "      \"loss_proxy\": {\n";
        json << "        \"iters_avg\": " << stats.last_iters_avg << ",\n";
        json << "        \"render_ms\": " << static_cast<double>(stats.last_render_ms) << ",\n";
        json << "        \"sdf_center_signed_px\": " << static_cast<double>(signed0) << ",\n";
        json << "        \"sdf_center_inside\": " << (inside0 ? "true" : "false") << "\n";
        json << "      },\n";
        json << "      \"delta\": {\n";
        json << "        \"d_iters_avg\": " << dIters << ",\n";
        json << "        \"d_render_ms\": " << static_cast<double>(dMs) << ",\n";
        json << "        \"d_sdf_center_signed_px\": " << static_cast<double>(dSdf0) << "\n";
        json << "      }\n";
        json << "    }";
        if (t + 1 < config.ticks) json << ",";
        json << "\n";

        prevIters = stats.last_iters_avg;
        prevMs = stats.last_render_ms;
        if (sampled0) prevSigned0 = signed0;
        if (t == 0) baseIters0 = stats.last_iters_avg;
        if (stats.last_iters_avg > peakIters) {
            peakIters = stats.last_iters_avg;
            peakTick = t;
        }
        if (minAbsSigned < bestSaddleMinAbs) {
            bestSaddleMinAbs = minAbsSigned;
            bestSaddleTick = t;
            bestSaddleNearCount = nearCount;
        }
        if (!closureTick && t == config.closure_ref_t) {
            closureRefIters = stats.last_iters_avg;
            closureRefSigned0 = signed0;
            closureRefComparable = sampled0;
        }
        if (closureTick) {
            closureTickIters = stats.last_iters_avg;
            closureTickSigned0 = signed0;
            closureTickComparable = sampled0;
        }

        finalRgba = rgba;
        finalMask = mask;
        finalStats = stats;
    }

    const bool closureComparable = closureRefComparable && closureTickComparable;
    const int closureDIters = closureTickIters - closureRefIters;
    const float closureDSigned0 = closureTickSigned0 - closureRefSigned0;
    const bool closureOk = !config.closure_last
        || (closureDIters == 0 && (!closureComparable || std::fabs(closureDSigned0) <= 1.0e-6f));
    const int peakAmplitude = baseIters0 > 0 ? (peakIters - baseIters0) : peakIters;

    json << "  ],\n";
    json << "  \"summary\": {\n";
    json << "    \"base_iters_avg_t0\": " << baseIters0 << ",\n";
    json << "    \"peak_iters_avg\": " << peakIters << ",\n";
    json << "    \"peak_t\": " << peakTick << ",\n";
    json << "    \"peak_amplitude_over_t0\": " << peakAmplitude << ",\n";
    json << "    \"best_saddle_t\": " << bestSaddleTick << ",\n";
    json << "    \"best_saddle_min_abs_signed_px\": " << static_cast<double>(bestSaddleMinAbs) << ",\n";
    json << "    \"best_saddle_near_count\": " << bestSaddleNearCount << ",\n";
    json << "    \"closure\": {\n";
    json << "      \"enabled\": " << (config.closure_last ? "true" : "false") << ",\n";
    json << "      \"ref_t\": " << config.closure_ref_t << ",\n";
    json << "      \"closure_t\": " << (config.ticks - 1) << ",\n";
    json << "      \"ok\": " << (closureOk ? "true" : "false") << ",\n";
    json << "      \"d_iters_avg\": " << closureDIters << ",\n";
    json << "      \"signed_comparable\": " << (closureComparable ? "true" : "false") << ",\n";
    json << "      \"d_sdf_center_signed_px\": " << static_cast<double>(closureDSigned0) << "\n";
    json << "    }\n";
    json << "  }\n";
    json << "}\n";

    DiagnosticsCaptureResult capture;
    if (!CaptureDiagnosticsLastBundle(exeDir, view, params, render, finalStats, finalRgba.data(), finalRgba.size(), &capture, &error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }

    std::vector<uint32_t> lensSdfRgba;
    ComputeSignedDistanceSdfChamfer(finalMask.data(), render.resolution.x, render.resolution.y, 48.0f, lensSdfRgba);
    const std::string lensSdfPath = (std::filesystem::path(capture.output_dir) / "lens_sdf.bmp").string();
    if (!WriteBmp32Bgra(lensSdfPath, lensSdfRgba.data(), render.resolution.x, render.resolution.y, &error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }

    const std::string probePath = (std::filesystem::path(capture.output_dir) / "flashlight_probe.json").string();
    if (!WriteTextFile(probePath, json.str(), &error)) {
        std::fprintf(stderr, "%s\n", error.c_str());
        return 1;
    }
    return 0;
}
