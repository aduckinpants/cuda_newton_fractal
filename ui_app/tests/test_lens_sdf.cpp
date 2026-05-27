#include "../src/lens_sdf.h"
#include "../src/sdf_field_signal.h"

#include <cmath>
#include <iostream>
#include <vector>

int main() {
    if (NormalizeLensDownsamplePow2(0) != 1 ||
        NormalizeLensDownsamplePow2(1) != 1 ||
        NormalizeLensDownsamplePow2(2) != 2 ||
        NormalizeLensDownsamplePow2(3) != 4 ||
        NormalizeLensDownsamplePow2(4) != 4 ||
        NormalizeLensDownsamplePow2(5) != 8 ||
        NormalizeLensDownsamplePow2(9) != 16 ||
        NormalizeLensDownsamplePow2(16) != 16 ||
        NormalizeLensDownsamplePow2(17) != 16) {
        std::cerr << "Lens downsample normalization should match visible pow2 controls through 16x\n";
        return 1;
    }

    {
        const LensSdfEffectiveDownsample requested =
            ResolveEffectiveLensSdfDownsample(1, false, false, 10.0, 33.333);
        if (requested.requested_downsample != 1 ||
            requested.effective_downsample != 1 ||
            requested.quality_mode != LensSdfQualityMode::requested) {
            std::cerr << "Settled SDF field quality should use requested downsample\n";
            return 1;
        }

        const LensSdfEffectiveDownsample normalized =
            ResolveEffectiveLensSdfDownsample(3, false, false, 10.0, 33.333);
        if (normalized.requested_downsample != 4 ||
            normalized.effective_downsample != 4 ||
            normalized.quality_mode != LensSdfQualityMode::requested) {
            std::cerr << "Requested SDF downsample should normalize to the visible pow2 authority\n";
            return 1;
        }

        const LensSdfEffectiveDownsample lowCost =
            ResolveEffectiveLensSdfDownsample(1, true, false, 1.0, 33.333);
        if (lowCost.effective_downsample != 1 ||
            lowCost.quality_mode != LensSdfQualityMode::requested) {
            std::cerr << "Low-cost preview should not degrade SDF field quality\n";
            return 1;
        }

        const LensSdfEffectiveDownsample adaptive =
            ResolveEffectiveLensSdfDownsample(1, true, false, 14.0, 33.333);
        if (adaptive.requested_downsample != 1 ||
            adaptive.effective_downsample != 2 ||
            adaptive.quality_mode != LensSdfQualityMode::interactive_adaptive) {
            std::cerr << "Over-budget preview should adapt effective SDF field downsample\n";
            return 1;
        }

        const LensSdfEffectiveDownsample severe =
            ResolveEffectiveLensSdfDownsample(1, true, false, 70.0, 33.333);
        if (severe.effective_downsample < 4 ||
            severe.quality_mode != LensSdfQualityMode::interactive_adaptive) {
            std::cerr << "Severe preview should adapt SDF field quality beyond 2x when needed\n";
            return 1;
        }

        const LensSdfEffectiveDownsample requestedAlreadyCoarse =
            ResolveEffectiveLensSdfDownsample(4, true, false, 14.0, 33.333);
        if (requestedAlreadyCoarse.requested_downsample != 4 ||
            requestedAlreadyCoarse.effective_downsample != 4 ||
            requestedAlreadyCoarse.quality_mode != LensSdfQualityMode::requested) {
            std::cerr << "Adaptive policy should not improve or rewrite a coarse requested downsample\n";
            return 1;
        }

        const LensSdfEffectiveDownsample forcedFullQuality =
            ResolveEffectiveLensSdfDownsample(1, true, true, 70.0, 33.333);
        if (forcedFullQuality.effective_downsample != 1 ||
            forcedFullQuality.quality_mode != LensSdfQualityMode::requested) {
            std::cerr << "Forced full-quality paths should use requested SDF field quality\n";
            return 1;
        }

        if (std::string(LensSdfQualityModeId(LensSdfQualityMode::interactive_adaptive)) != "interactive_adaptive") {
            std::cerr << "SDF field quality mode id should be stable for automation reports\n";
            return 1;
        }
    }

    {
        const int srcW = 5;
        const int srcH = 3;
        const uint8_t source[srcW * srcH] = {
            1, 2, 3, 4, 5,
            6, 7, 8, 9, 10,
            11, 12, 13, 14, 15,
        };
        std::vector<uint8_t> downsampled;
        int outW = 0;
        int outH = 0;
        if (!DownsampleMaskPow2(source, srcW, srcH, 2, downsampled, outW, outH)) {
            std::cerr << "DownsampleMaskPow2 should accept valid masks\n";
            return 1;
        }
        if (outW != 3 || outH != 2 || downsampled.size() != 6) {
            std::cerr << "DownsampleMaskPow2 should ceil odd dimensions while downsampling\n";
            return 1;
        }
        if (downsampled[0] != 1 || downsampled[1] != 3 || downsampled[2] != 5 ||
            downsampled[3] != 11 || downsampled[4] != 13 || downsampled[5] != 15) {
            std::cerr << "DownsampleMaskPow2 should use deterministic top-left sampling with edge coverage\n";
            return 1;
        }
    }

    const int w = 5;
    const int h = 5;
    std::vector<uint8_t> mask(static_cast<size_t>(w) * static_cast<size_t>(h), 0);
    mask[2 + 2 * w] = 255;

    float signedCenter = 0.0f;
    float signedCorner = 0.0f;
    bool centerInside = false;
    bool cornerInside = false;

    if (!SampleSignedDistanceSdfChamfer(mask.data(), w, h, 2, 2, signedCenter, centerInside)) {
        std::cerr << "Center SDF sample should succeed\n";
        return 1;
    }
    if (!SampleSignedDistanceSdfChamfer(mask.data(), w, h, 0, 0, signedCorner, cornerInside)) {
        std::cerr << "Corner SDF sample should succeed\n";
        return 1;
    }
    if (!centerInside || cornerInside) {
        std::cerr << "Center should be inside, corner should be outside\n";
        return 1;
    }
    if (!(signedCenter < 0.0f) || !(signedCorner > 0.0f)) {
        std::cerr << "Signed distance should be negative inside and positive outside\n";
        return 1;
    }

    std::vector<uint32_t> rgba;
    ComputeSignedDistanceSdfChamfer(mask.data(), w, h, 8.0f, rgba);
    if (rgba.size() != static_cast<size_t>(w) * static_cast<size_t>(h)) {
        std::cerr << "SDF RGBA output should match mask size\n";
        return 1;
    }
    if (rgba[2 + 2 * w] == rgba[0]) {
        std::cerr << "Inside and outside SDF colors should differ\n";
        return 1;
    }

    {
        SdfFieldResult field;
        if (!ComputeSignedDistanceSdfFieldChamfer(mask.data(), w, h, field)) {
            std::cerr << "ComputeSignedDistanceSdfFieldChamfer should accept valid masks\n";
            return 1;
        }
        SdfFieldView view = field.View();
        if (view.width != w || view.height != h || view.signed_distance_count != static_cast<size_t>(w) * static_cast<size_t>(h)) {
            std::cerr << "SdfFieldView should expose scalar dimensions and count\n";
            return 1;
        }
        if (view.sign_convention != SdfSignConvention::negative_inside_positive_outside ||
            view.source_kind != SdfFieldSourceKind::mask_derived ||
            std::fabs(view.pixel_scale - 1.0f) > 0.0001f) {
            std::cerr << "SdfFieldView should expose source metadata and sign convention\n";
            return 1;
        }
        float fieldCenter = 0.0f;
        float fieldCorner = 0.0f;
        bool fieldCenterInside = false;
        bool fieldCornerInside = false;
        if (!SampleSignedDistanceSdfField(view, 2, 2, fieldCenter, fieldCenterInside) ||
            !SampleSignedDistanceSdfField(view, 0, 0, fieldCorner, fieldCornerInside)) {
            std::cerr << "SampleSignedDistanceSdfField should sample valid field coordinates\n";
            return 1;
        }
        if (!fieldCenterInside || fieldCornerInside || !(fieldCenter < 0.0f) || !(fieldCorner > 2.7f && fieldCorner < 2.9f)) {
            std::cerr << "Scalar SDF field should preserve inside/outside sign and diagonal distance\n";
            return 1;
        }
        SdfFieldSignalConfig signalConfig;
        signalConfig.boundary_band_px = 3.0f;
        SdfFieldSignalSample centerSignals;
        SdfFieldSignalSample rightSignals;
        if (!SampleSdfFieldSignals(view, 2, 2, signalConfig, centerSignals) ||
            !SampleSdfFieldSignals(view, 3, 2, signalConfig, rightSignals)) {
            std::cerr << "SdfFieldView should produce reusable signal samples\n";
            return 1;
        }
        if (!centerSignals.inside || centerSignals.inside_outside != 1.0f ||
            std::fabs(centerSignals.signed_distance_px - fieldCenter) > 0.0001f) {
            std::cerr << "SDF signal sample should preserve signed distance and inside/outside authority\n";
            return 1;
        }
        if (!(centerSignals.boundary_band > 0.6f && centerSignals.boundary_band < 0.7f) ||
            !(rightSignals.boundary_band > 0.6f && rightSignals.boundary_band < 0.7f)) {
            std::cerr << "SDF signal sample should expose a normalized boundary band\n";
            return 1;
        }
        if (!std::isfinite(rightSignals.normal_angle_radians) || std::fabs(rightSignals.normal_angle_radians) > 0.0001f) {
            std::cerr << "SDF signal sample should expose an approximate outward normal angle\n";
            return 1;
        }
        if (!std::isfinite(centerSignals.curvature_estimate) || !(centerSignals.curvature_estimate > 0.0f)) {
            std::cerr << "SDF signal sample should expose a finite curvature estimate\n";
            return 1;
        }
        if (ResolveSdfFieldSignalValue(centerSignals, SdfFieldSignalKind::signed_distance_px) != centerSignals.signed_distance_px ||
            ResolveSdfFieldSignalValue(centerSignals, SdfFieldSignalKind::inside_outside) != centerSignals.inside_outside ||
            ResolveSdfFieldSignalValue(centerSignals, SdfFieldSignalKind::boundary_band) != centerSignals.boundary_band) {
            std::cerr << "SDF signal resolver should return the requested signal value\n";
            return 1;
        }
        std::vector<uint32_t> rgbaFromField;
        BuildSignedDistanceSdfRgba(view, 8.0f, rgbaFromField);
        if (rgbaFromField != rgba) {
            std::cerr << "RGBA Lens SDF visualization should be derived from scalar field data without changing output\n";
            return 1;
        }
        SdfFieldResult invalidField;
        if (ComputeSignedDistanceSdfFieldChamfer(nullptr, w, h, invalidField) ||
            invalidField.width != 0 ||
            !invalidField.signed_distance_px.empty()) {
            std::cerr << "Invalid scalar SDF field input should fail closed and clear output\n";
            return 1;
        }
    }

    {
        std::vector<uint8_t> allOutside(static_cast<size_t>(w) * static_cast<size_t>(h), 0);
        std::vector<uint8_t> allInside(static_cast<size_t>(w) * static_cast<size_t>(h), 255);
        std::vector<uint32_t> outsideRgba;
        std::vector<uint32_t> insideRgba;
        SdfFieldResult outsideField;
        SdfFieldResult insideField;
        ComputeSignedDistanceSdfChamfer(allOutside.data(), w, h, 8.0f, outsideRgba);
        ComputeSignedDistanceSdfChamfer(allInside.data(), w, h, 8.0f, insideRgba);
        if (!ComputeSignedDistanceSdfFieldChamfer(allOutside.data(), w, h, outsideField) ||
            !ComputeSignedDistanceSdfFieldChamfer(allInside.data(), w, h, insideField)) {
            std::cerr << "Uniform-mask scalar SDF fields should be representable\n";
            return 1;
        }
        if (outsideRgba.size() != static_cast<size_t>(w) * static_cast<size_t>(h) ||
            insideRgba.size() != static_cast<size_t>(w) * static_cast<size_t>(h)) {
            std::cerr << "Uniform-mask SDF output should still match the input size\n";
            return 1;
        }
        if (outsideRgba[0] == insideRgba[0]) {
            std::cerr << "Uniform inside and uniform outside masks should not collapse to the same SDF visualization\n";
            return 1;
        }
        if (!(outsideField.signed_distance_px[0] > 1.0e8f) ||
            !(insideField.signed_distance_px[0] < -1.0e8f) ||
            !std::isfinite(outsideField.signed_distance_px[0]) ||
            !std::isfinite(insideField.signed_distance_px[0])) {
            std::cerr << "Uniform-mask scalar fields should preserve finite saturated sign authority\n";
            return 1;
        }
    }

    {
        const int srcW = 8;
        const int srcH = 4;
        std::vector<uint8_t> mask2(static_cast<size_t>(srcW) * static_cast<size_t>(srcH), 0);
        for (int y = 0; y < srcH; ++y) {
            for (int x = 0; x < srcW; ++x) {
                if (x >= 2 && x < 6) {
                    mask2[static_cast<size_t>(y) * static_cast<size_t>(srcW) + static_cast<size_t>(x)] = 255;
                }
            }
        }
        std::vector<uint32_t> lensRgba;
        int lensW = 0;
        int lensH = 0;
        if (!ComputeLensSdfRgbaForMask(mask2.data(), srcW, srcH, 2, 8.0f, lensRgba, lensW, lensH)) {
            std::cerr << "ComputeLensSdfRgbaForMask should accept valid masks\n";
            return 1;
        }
        if (lensW != 4 || lensH != 2 || lensRgba.size() != 8) {
            std::cerr << "ComputeLensSdfRgbaForMask should honor downsample in output dimensions\n";
            return 1;
        }
        std::vector<uint8_t> lowMask;
        int lowW = 0;
        int lowH = 0;
        std::vector<uint32_t> expectedRgba;
        if (!DownsampleMaskPow2(mask2.data(), srcW, srcH, 2, lowMask, lowW, lowH)) {
            std::cerr << "DownsampleMaskPow2 should support expected Lens SDF comparison\n";
            return 1;
        }
        ComputeSignedDistanceSdfChamfer(lowMask.data(), lowW, lowH, 4.0f, expectedRgba);
        if (lensRgba != expectedRgba) {
            std::cerr << "ComputeLensSdfRgbaForMask should normalize SDF scale in low-resolution pixels\n";
            return 1;
        }
        SdfFieldResult lensField;
        if (!ComputeLensSdfFieldForMask(mask2.data(), srcW, srcH, 2, lensField)) {
            std::cerr << "ComputeLensSdfFieldForMask should expose downsampled scalar field authority\n";
            return 1;
        }
        if (lensField.width != 4 || lensField.height != 2 ||
            lensField.signed_distance_px.size() != 8 ||
            std::fabs(lensField.pixel_scale - 2.0f) > 0.0001f) {
            std::cerr << "ComputeLensSdfFieldForMask should report low dimensions and source pixel scale\n";
            return 1;
        }
        if (ComputeLensSdfRgbaForMask(nullptr, srcW, srcH, 2, 8.0f, lensRgba, lensW, lensH)) {
            std::cerr << "ComputeLensSdfRgbaForMask should reject invalid masks\n";
            return 1;
        }
    }

    std::cout << "test_lens_sdf: all passed\n";
    return 0;
}
