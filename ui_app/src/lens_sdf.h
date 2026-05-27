#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

enum class SdfFieldSourceKind {
    mask_derived,
};

enum class SdfSignConvention {
    negative_inside_positive_outside,
};

struct SdfFieldView {
    int width{0};
    int height{0};
    float pixel_scale{1.0f};
    SdfSignConvention sign_convention{SdfSignConvention::negative_inside_positive_outside};
    SdfFieldSourceKind source_kind{SdfFieldSourceKind::mask_derived};
    const float* signed_distance_px{nullptr};
    std::size_t signed_distance_count{0};
};

struct SdfFieldResult {
    int width{0};
    int height{0};
    float pixel_scale{1.0f};
    SdfSignConvention sign_convention{SdfSignConvention::negative_inside_positive_outside};
    SdfFieldSourceKind source_kind{SdfFieldSourceKind::mask_derived};
    std::vector<float> signed_distance_px;

    void Clear();
    SdfFieldView View() const;
};

enum class LensSdfBackend {
    cpu_chamfer,
    cuda_jfa,
    auto_backend,
};

struct LensSdfBackendReport {
    LensSdfBackend requested{LensSdfBackend::cpu_chamfer};
    LensSdfBackend used{LensSdfBackend::cpu_chamfer};
    bool fallback_used{false};
};

enum class LensSdfQualityMode {
    requested,
    interactive_adaptive,
};

struct LensSdfEffectiveDownsample {
    int requested_downsample{1};
    int effective_downsample{1};
    LensSdfQualityMode quality_mode{LensSdfQualityMode::requested};
};

int NormalizeLensDownsamplePow2(int value);
const char* LensSdfQualityModeId(LensSdfQualityMode mode);

LensSdfEffectiveDownsample ResolveEffectiveLensSdfDownsample(
    int requested_downsample,
    bool preview_active,
    bool force_full_quality,
    double previous_field_ms,
    double target_frame_ms);

bool DownsampleMaskPow2(
    const uint8_t* inMask,
    int inW,
    int inH,
    int downsample,
    std::vector<uint8_t>& outMask,
    int& outW,
    int& outH);

void ComputeSignedDistanceSdfChamfer(
    const uint8_t* mask,
    int width,
    int height,
    float maxAbsPx,
    std::vector<uint32_t>& outRgba);

bool ComputeSignedDistanceSdfFieldChamfer(
    const uint8_t* mask,
    int width,
    int height,
    SdfFieldResult& outField);

void BuildSignedDistanceSdfRgba(
    const SdfFieldView& field,
    float maxAbsPx,
    std::vector<uint32_t>& outRgba);

bool SampleSignedDistanceSdfChamfer(
    const uint8_t* mask,
    int width,
    int height,
    int x,
    int y,
    float& outSignedPx,
    bool& outInside);

bool SampleSignedDistanceSdfField(
    const SdfFieldView& field,
    int x,
    int y,
    float& outSignedPx,
    bool& outInside);

bool ComputeLensSdfFieldForMask(
    const uint8_t* mask,
    int width,
    int height,
    int downsample,
    SdfFieldResult& outField);

bool ComputeLensSdfFieldForMaskWithBackend(
    const uint8_t* mask,
    int width,
    int height,
    int downsample,
    LensSdfBackend backend,
    SdfFieldResult& outField,
    LensSdfBackendReport* outReport = nullptr);

bool ComputeLensSdfRgbaForMask(
    const uint8_t* mask,
    int width,
    int height,
    int downsample,
    float maxAbsPx,
    std::vector<uint32_t>& outRgba,
    int& outWidth,
    int& outHeight);
