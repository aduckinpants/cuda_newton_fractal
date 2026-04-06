#include "lens_sdf.h"

#include <cmath>

namespace {

void RelaxChamferForward(std::vector<float>& distances, int width, int height) {
    const float w1 = 1.0f;
    const float w2 = 1.41421356f;
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            size_t index = static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x);
            float best = distances[index];
            if (x > 0) best = fminf(best, distances[index - 1] + w1);
            if (y > 0) best = fminf(best, distances[index - static_cast<size_t>(width)] + w1);
            if (x > 0 && y > 0) best = fminf(best, distances[index - static_cast<size_t>(width) - 1] + w2);
            if (x + 1 < width && y > 0) best = fminf(best, distances[index - static_cast<size_t>(width) + 1] + w2);
            distances[index] = best;
        }
    }
}

void RelaxChamferBackward(std::vector<float>& distances, int width, int height) {
    const float w1 = 1.0f;
    const float w2 = 1.41421356f;
    for (int y = height - 1; y >= 0; --y) {
        for (int x = width - 1; x >= 0; --x) {
            size_t index = static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x);
            float best = distances[index];
            if (x + 1 < width) best = fminf(best, distances[index + 1] + w1);
            if (y + 1 < height) best = fminf(best, distances[index + static_cast<size_t>(width)] + w1);
            if (x + 1 < width && y + 1 < height) best = fminf(best, distances[index + static_cast<size_t>(width) + 1] + w2);
            if (x > 0 && y + 1 < height) best = fminf(best, distances[index + static_cast<size_t>(width) - 1] + w2);
            distances[index] = best;
        }
    }
}

void BuildDistanceFields(const uint8_t* mask, int width, int height, std::vector<float>& dToInside, std::vector<float>& dToOutside) {
    const float inf = 1.0e9f;
    const size_t count = static_cast<size_t>(width) * static_cast<size_t>(height);
    dToInside.assign(count, inf);
    dToOutside.assign(count, inf);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            size_t index = static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x);
            bool inside = mask[index] > 127;
            if (inside) {
                dToInside[index] = 0.0f;
            } else {
                dToOutside[index] = 0.0f;
            }
        }
    }

    RelaxChamferForward(dToInside, width, height);
    RelaxChamferBackward(dToInside, width, height);
    RelaxChamferForward(dToOutside, width, height);
    RelaxChamferBackward(dToOutside, width, height);
}

} // namespace

void ComputeSignedDistanceSdfChamfer(
    const uint8_t* mask,
    int width,
    int height,
    float maxAbsPx,
    std::vector<uint32_t>& outRgba) {
    if (!mask || width <= 0 || height <= 0) {
        outRgba.clear();
        return;
    }

    std::vector<float> dToInside;
    std::vector<float> dToOutside;
    BuildDistanceFields(mask, width, height, dToInside, dToOutside);

    const size_t count = static_cast<size_t>(width) * static_cast<size_t>(height);
    outRgba.resize(count);
    float denom = fmaxf(1.0f, maxAbsPx);
    for (size_t index = 0; index < count; ++index) {
        float signedPx = dToInside[index] - dToOutside[index];
        float value = 0.5f + (signedPx / (2.0f * denom));
        if (value < 0.0f) value = 0.0f;
        if (value > 1.0f) value = 1.0f;
        uint8_t c = static_cast<uint8_t>(value * 255.0f + 0.5f);

        if (fabsf(signedPx) < 0.75f) {
            outRgba[index] = static_cast<uint32_t>(255)
                | (static_cast<uint32_t>(64) << 8)
                | (static_cast<uint32_t>(64) << 16)
                | (static_cast<uint32_t>(255) << 24);
        } else {
            outRgba[index] = static_cast<uint32_t>(c)
                | (static_cast<uint32_t>(c) << 8)
                | (static_cast<uint32_t>(c) << 16)
                | (static_cast<uint32_t>(255) << 24);
        }
    }
}

bool SampleSignedDistanceSdfChamfer(
    const uint8_t* mask,
    int width,
    int height,
    int x,
    int y,
    float& outSignedPx,
    bool& outInside) {
    if (!mask || width <= 0 || height <= 0) return false;
    if (x < 0 || y < 0 || x >= width || y >= height) return false;

    std::vector<float> dToInside;
    std::vector<float> dToOutside;
    BuildDistanceFields(mask, width, height, dToInside, dToOutside);

    size_t index = static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x);
    outSignedPx = dToInside[index] - dToOutside[index];
    outInside = mask[index] > 127;
    return true;
}