#include "explaino_root_sdf_field.h"

#include "enum_id_utils.h"
#include "explaino_root_field.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <limits>
#include <sstream>
#include <utility>

namespace {

constexpr double kTau = 6.283185307179586476925286766559;

float ClampFinite(float value, float fallback, float lo, float hi) {
    if (!std::isfinite(value)) {
        value = fallback;
    }
    return (std::max)(lo, (std::min)(hi, value));
}

bool IsFiniteRoot(const Float2& root) {
    return std::isfinite(root.x) && std::isfinite(root.y);
}

void HashBytes(std::uint64_t& hash, const void* ptr, std::size_t byteCount) {
    const auto* bytes = static_cast<const unsigned char*>(ptr);
    for (std::size_t index = 0; index < byteCount; ++index) {
        hash ^= static_cast<std::uint64_t>(bytes[index]);
        hash *= 1099511628211ull;
    }
}

std::uint64_t HashRoots(const Float2* roots, int rootCount) {
    std::uint64_t hash = 1469598103934665603ull;
    HashBytes(hash, &rootCount, sizeof(rootCount));
    for (int index = 0; index < rootCount; ++index) {
        HashBytes(hash, &roots[index].x, sizeof(roots[index].x));
        HashBytes(hash, &roots[index].y, sizeof(roots[index].y));
    }
    return hash;
}

void AppendJsonNumber(std::ostringstream& out, double value) {
    out << std::setprecision(17) << value;
}

std::string CircleAst(const Float2& center, float radius) {
    std::ostringstream out;
    out << "{\"op\":\"circle\",\"center\":[";
    AppendJsonNumber(out, center.x);
    out << ",";
    AppendJsonNumber(out, center.y);
    out << "],\"radius\":";
    AppendJsonNumber(out, radius);
    out << "}";
    return out.str();
}

std::string CapsuleAst(const Float2& a, const Float2& b, float radius) {
    std::ostringstream out;
    out << "{\"op\":\"capsule\",\"a\":[";
    AppendJsonNumber(out, a.x);
    out << ",";
    AppendJsonNumber(out, a.y);
    out << "],\"b\":[";
    AppendJsonNumber(out, b.x);
    out << ",";
    AppendJsonNumber(out, b.y);
    out << "],\"radius\":";
    AppendJsonNumber(out, radius);
    out << "}";
    return out.str();
}

std::string UnionAst(std::string left, std::string right, float smoothBlend) {
    std::ostringstream out;
    if (smoothBlend > 0.0f) {
        out << "{\"op\":\"smooth_union\",\"k\":";
        AppendJsonNumber(out, smoothBlend);
        out << ",\"a\":" << left << ",\"b\":" << right << "}";
    } else {
        out << "{\"op\":\"union\",\"a\":" << left << ",\"b\":" << right << "}";
    }
    return out.str();
}

std::string BuildRootSdfPackJson(const ExplainoRootSdfResolvedScene& scene, const KernelParams& params) {
    const float radius = ClampFinite(params.explaino_root_sdf_radius, 0.14f, 0.001f, 2.0f);
    const float bridgeWidth = ClampFinite(params.explaino_root_sdf_bridge_width, 0.06f, 0.0f, 2.0f);
    const float smoothBlend = ClampFinite(params.explaino_root_sdf_smooth_blend, 0.10f, 0.0f, 2.0f);

    std::string ast = CircleAst(scene.effective_roots[0], radius);
    for (int index = 1; index < scene.root_count; ++index) {
        ast = UnionAst(std::move(ast), CircleAst(scene.effective_roots[index], radius), smoothBlend);
    }
    if (bridgeWidth > 0.0f && scene.root_count >= 2) {
        if (scene.layout_kind == ExplainoRootFieldLayoutKind::regular_ngon_v1) {
            if (scene.root_count == 2) {
                ast = UnionAst(std::move(ast),
                    CapsuleAst(scene.effective_roots[0], scene.effective_roots[1], bridgeWidth),
                    smoothBlend);
            } else {
                for (int index = 0; index < scene.root_count; ++index) {
                    const int next = (index + 1) % scene.root_count;
                    ast = UnionAst(std::move(ast),
                        CapsuleAst(scene.effective_roots[index], scene.effective_roots[next], bridgeWidth),
                        smoothBlend);
                }
            }
        } else if (scene.root_count >= 3) {
            ast = UnionAst(std::move(ast),
                CapsuleAst(scene.effective_roots[0], scene.effective_roots[1], bridgeWidth),
                smoothBlend);
            if (scene.root_count >= 4) {
                ast = UnionAst(std::move(ast),
                    CapsuleAst(scene.effective_roots[2], scene.effective_roots[3], bridgeWidth),
                    smoothBlend);
            }
        }
    }

    std::ostringstream out;
    out << "{\"schema\":1,\"pack_id\":\"explaino_root_sdf_dynamic\","
        "\"name\":\"ExplainO Root SDF Dynamic\",\"kind\":\"sdf_scene_2d\","
        "\"ast\":" << ast << "}";
    return out.str();
}

} // namespace

bool ResolveExplainoRootSdfScene(
    const ViewState& view,
    const KernelParams& params,
    ExplainoRootSdfResolvedScene* outScene,
    std::string* outError) {
    if (!outScene) {
        if (outError) *outError = "ExplainO Root SDF scene output is required";
        return false;
    }
    *outScene = {};
    ExplainoRootFieldDescriptor descriptor{};
    if (!ResolveExplainoRootFieldDescriptor(view, params, &descriptor, outError)) {
        return false;
    }
    const int rootCount = descriptor.active_count;
    if (rootCount <= 0) {
        if (outError) *outError = "ExplainO Root SDF requires captured/generated roots";
        return false;
    }
    if (descriptor.layout_kind != ExplainoRootFieldLayoutKind::regular_ngon_v1 &&
        rootCount != 3 &&
        rootCount != 4) {
        if (outError) *outError = "ExplainO Root SDF legacy/custom roots require 3 or 4 roots";
        return false;
    }
    if (rootCount > kExplainoRootFieldMaxRoots) {
        if (outError) *outError = "ExplainO Root SDF root count exceeds descriptor capacity";
        return false;
    }
    outScene->root_count = rootCount;
    outScene->layout_kind = descriptor.layout_kind;
    for (int index = 0; index < rootCount; ++index) {
        if (!IsFiniteRoot(descriptor.base_roots[index])) {
            if (outError) *outError = "ExplainO Root SDF root coordinates must be finite";
            return false;
        }
        outScene->base_roots[index] = descriptor.base_roots[index];
        outScene->effective_roots[index] = descriptor.effective_roots[index];
    }
    if (descriptor.layout_kind == ExplainoRootFieldLayoutKind::regular_ngon_v1) {
        outScene->bridge_count = rootCount == 2 ? 1 : rootCount;
    } else {
        outScene->bridge_count = rootCount >= 4 ? 2 : 1;
    }
    if (ClampFinite(params.explaino_root_sdf_bridge_width, 0.06f, 0.0f, 2.0f) <= 0.0f) {
        outScene->bridge_count = 0;
    }

    if (params.explaino_root_sdf_h_source == ExplainoRootSdfHSource::phase_sine) {
        const float amplitude = ClampFinite(params.explaino_root_sdf_h_amplitude, 0.0f, 0.0f, 1.0f);
        const float frequency = ClampFinite(params.explaino_root_sdf_h_frequency, 1.0f, 0.1f, 16.0f);
        if (amplitude > 0.0f) {
            Float2 centroid{0.0f, 0.0f};
            for (int index = 0; index < rootCount; ++index) {
                centroid.x += outScene->base_roots[index].x;
                centroid.y += outScene->base_roots[index].y;
            }
            centroid.x /= static_cast<float>(rootCount);
            centroid.y /= static_cast<float>(rootCount);

            for (int index = 0; index < rootCount; ++index) {
                float dx = outScene->base_roots[index].x - centroid.x;
                float dy = outScene->base_roots[index].y - centroid.y;
                const float len = std::sqrt(dx * dx + dy * dy);
                if (len > std::numeric_limits<float>::epsilon()) {
                    dx /= len;
                    dy /= len;
                } else {
                    const float angle = static_cast<float>(kTau * static_cast<double>(index) /
                        static_cast<double>(rootCount));
                    dx = std::cos(angle);
                    dy = std::sin(angle);
                }
                const float phase = static_cast<float>(
                    kTau * static_cast<double>(frequency) * static_cast<double>(view.explaino_phase) +
                    kTau * static_cast<double>(index) / static_cast<double>(rootCount));
                const float offset = amplitude * std::sin(phase);
                outScene->effective_roots[index].x += dx * offset;
                outScene->effective_roots[index].y += dy * offset;
                if (!IsFiniteRoot(outScene->effective_roots[index])) {
                    if (outError) *outError = "ExplainO Root SDF h-mode produced nonfinite roots";
                    return false;
                }
            }
        }
    } else if (params.explaino_root_sdf_h_source != ExplainoRootSdfHSource::none) {
        if (outError) *outError = "ExplainO Root SDF h_source must be none or phase_sine";
        return false;
    }

    outScene->base_root_hash = descriptor.base_root_hash;
    outScene->effective_root_hash = HashRoots(outScene->effective_roots, rootCount);
    return true;
}

bool ComputeExplainoRootSdfFieldForViewport(
    const ViewState& view,
    const KernelParams& params,
    const SdfPackFieldRegion& region,
    int renderWidth,
    int renderHeight,
    int effectiveDownsample,
    SdfPackFieldBackend backend,
    SdfFieldResult& outField,
    SdfPackFieldReport* outPackReport,
    ExplainoRootSdfFieldReport* outRootReport,
    std::string* outError) {
    outField.Clear();
    ExplainoRootSdfResolvedScene scene;
    if (!ResolveExplainoRootSdfScene(view, params, &scene, outError)) {
        return false;
    }

    SdfPackParseResult parsed = ParseSdfPackJson(BuildRootSdfPackJson(scene, params));
    if (!parsed.ok) {
        if (outError) *outError = parsed.error.empty()
            ? "ExplainO Root SDF dynamic pack parse failed"
            : parsed.error;
        return false;
    }

    const int safeDownsample = NormalizeLensDownsamplePow2(effectiveDownsample);
    SdfPackFieldRequest request{};
    request.pack = &parsed.pack;
    request.width = (std::max)(1, (renderWidth + safeDownsample - 1) / safeDownsample);
    request.height = (std::max)(1, (renderHeight + safeDownsample - 1) / safeDownsample);
    request.region = region;

    if (!ComputeSdfPackFieldWithBackend(request, backend, outField, outPackReport, outError)) {
        return false;
    }
    outField.source_kind = SdfFieldSourceKind::explaino_root_sdf;
    outField.pixel_scale = static_cast<float>(safeDownsample);
    if (outRootReport) {
        outRootReport->root_count = scene.root_count;
        outRootReport->bridge_count = scene.bridge_count;
        outRootReport->root_layout_kind = ExplainoRootFieldLayoutKindId(scene.layout_kind);
        if (!outRootReport->root_layout_kind) {
            outRootReport->root_layout_kind = "none";
        }
        outRootReport->requested_generated_root_count = params.explaino_generated_root_count;
        outRootReport->h_source = ExplainoRootSdfHSourceId(params.explaino_root_sdf_h_source);
        if (!outRootReport->h_source) {
            outRootReport->h_source = "none";
        }
        outRootReport->base_root_hash = scene.base_root_hash;
        outRootReport->effective_root_hash = scene.effective_root_hash;
    }
    return true;
}
