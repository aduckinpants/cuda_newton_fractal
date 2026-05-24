#include "../src/lens_sdf.h"

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
        std::vector<uint8_t> allOutside(static_cast<size_t>(w) * static_cast<size_t>(h), 0);
        std::vector<uint8_t> allInside(static_cast<size_t>(w) * static_cast<size_t>(h), 255);
        std::vector<uint32_t> outsideRgba;
        std::vector<uint32_t> insideRgba;
        ComputeSignedDistanceSdfChamfer(allOutside.data(), w, h, 8.0f, outsideRgba);
        ComputeSignedDistanceSdfChamfer(allInside.data(), w, h, 8.0f, insideRgba);
        if (outsideRgba.size() != static_cast<size_t>(w) * static_cast<size_t>(h) ||
            insideRgba.size() != static_cast<size_t>(w) * static_cast<size_t>(h)) {
            std::cerr << "Uniform-mask SDF output should still match the input size\n";
            return 1;
        }
        if (outsideRgba[0] == insideRgba[0]) {
            std::cerr << "Uniform inside and uniform outside masks should not collapse to the same SDF visualization\n";
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
        if (ComputeLensSdfRgbaForMask(nullptr, srcW, srcH, 2, 8.0f, lensRgba, lensW, lensH)) {
            std::cerr << "ComputeLensSdfRgbaForMask should reject invalid masks\n";
            return 1;
        }
    }

    std::cout << "test_lens_sdf: all passed\n";
    return 0;
}
