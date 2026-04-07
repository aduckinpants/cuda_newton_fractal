#include "../src/lens_sdf.h"

#include <iostream>
#include <vector>

int main() {
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

    std::cout << "test_lens_sdf: all passed\n";
    return 0;
}