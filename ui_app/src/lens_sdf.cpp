#include "lens_sdf.h"

#include <cmath>

namespace {

void DownsampleMask2x(const uint8_t* inMask, int inW, int inH, std::vector<uint8_t>& outMask, int& outW, int& outH) {
    outW = (inW + 1) / 2;
    outH = (inH + 1) / 2;
    outMask.assign(static_cast<size_t>(outW) * static_cast<size_t>(outH), 0);
    for (int y = 0; y < outH; ++y) {
        int sampleY = y * 2;
        if (sampleY >= inH) sampleY = inH - 1;
        for (int x = 0; x < outW; ++x) {
            int sampleX = x * 2;
            if (sampleX >= inW) sampleX = inW - 1;
            outMask[static_cast<size_t>(y) * static_cast<size_t>(outW) + static_cast<size_t>(x)] =
                inMask[static_cast<size_t>(sampleY) * static_cast<size_t>(inW) + static_cast<size_t>(sampleX)];
        }
    }
}

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

void SdfFieldResult::Clear() {
    width = 0;
    height = 0;
    pixel_scale = 1.0f;
    sign_convention = SdfSignConvention::negative_inside_positive_outside;
    source_kind = SdfFieldSourceKind::mask_derived;
    signed_distance_px.clear();
}

SdfFieldView SdfFieldResult::View() const {
    return SdfFieldView{
        width,
        height,
        pixel_scale,
        sign_convention,
        source_kind,
        signed_distance_px.empty() ? nullptr : signed_distance_px.data(),
        signed_distance_px.size(),
    };
}

int NormalizeLensDownsamplePow2(int value) {
    if (value <= 1) return 1;
    if (value <= 2) return 2;
    if (value <= 4) return 4;
    if (value <= 8) return 8;
    return 16;
}

bool DownsampleMaskPow2(
    const uint8_t* inMask,
    int inW,
    int inH,
    int downsample,
    std::vector<uint8_t>& outMask,
    int& outW,
    int& outH) {
    outMask.clear();
    outW = 0;
    outH = 0;
    if (!inMask || inW <= 0 || inH <= 0) {
        return false;
    }

    const int ds = NormalizeLensDownsamplePow2(downsample);
    if (ds <= 1) {
        outW = inW;
        outH = inH;
        outMask.assign(inMask, inMask + static_cast<size_t>(inW) * static_cast<size_t>(inH));
        return true;
    }

    std::vector<uint8_t> tmpA;
    std::vector<uint8_t> tmpB;
    const uint8_t* current = inMask;
    int currentW = inW;
    int currentH = inH;
    int steps = 0;
    for (int x = ds; x > 1; x >>= 1) ++steps;
    for (int step = 0; step < steps; ++step) {
        std::vector<uint8_t>& target = (step % 2 == 0) ? tmpA : tmpB;
        DownsampleMask2x(current, currentW, currentH, target, currentW, currentH);
        current = target.data();
    }

    outW = currentW;
    outH = currentH;
    outMask.assign(current, current + static_cast<size_t>(currentW) * static_cast<size_t>(currentH));
    return true;
}

bool ComputeSignedDistanceSdfFieldChamfer(
    const uint8_t* mask,
    int width,
    int height,
    SdfFieldResult& outField) {
    outField.Clear();
    if (!mask || width <= 0 || height <= 0) {
        return false;
    }

    std::vector<float> dToInside;
    std::vector<float> dToOutside;
    BuildDistanceFields(mask, width, height, dToInside, dToOutside);

    const size_t count = static_cast<size_t>(width) * static_cast<size_t>(height);
    outField.width = width;
    outField.height = height;
    outField.pixel_scale = 1.0f;
    outField.sign_convention = SdfSignConvention::negative_inside_positive_outside;
    outField.source_kind = SdfFieldSourceKind::mask_derived;
    outField.signed_distance_px.resize(count);
    for (size_t index = 0; index < count; ++index) {
        outField.signed_distance_px[index] = dToInside[index] - dToOutside[index];
    }
    return true;
}

void BuildSignedDistanceSdfRgba(
    const SdfFieldView& field,
    float maxAbsPx,
    std::vector<uint32_t>& outRgba) {
    if (!field.signed_distance_px ||
        field.width <= 0 ||
        field.height <= 0 ||
        field.signed_distance_count != static_cast<size_t>(field.width) * static_cast<size_t>(field.height)) {
        outRgba.clear();
        return;
    }

    outRgba.resize(field.signed_distance_count);
    float denom = fmaxf(1.0f, maxAbsPx);
    for (size_t index = 0; index < field.signed_distance_count; ++index) {
        float signedPx = field.signed_distance_px[index];
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

void ComputeSignedDistanceSdfChamfer(
    const uint8_t* mask,
    int width,
    int height,
    float maxAbsPx,
    std::vector<uint32_t>& outRgba) {
    SdfFieldResult field;
    if (!ComputeSignedDistanceSdfFieldChamfer(mask, width, height, field)) {
        outRgba.clear();
        return;
    }
    BuildSignedDistanceSdfRgba(field.View(), maxAbsPx, outRgba);
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

bool SampleSignedDistanceSdfField(
    const SdfFieldView& field,
    int x,
    int y,
    float& outSignedPx,
    bool& outInside) {
    if (!field.signed_distance_px ||
        field.width <= 0 ||
        field.height <= 0 ||
        field.signed_distance_count != static_cast<size_t>(field.width) * static_cast<size_t>(field.height)) {
        return false;
    }
    if (x < 0 || y < 0 || x >= field.width || y >= field.height) return false;

    size_t index = static_cast<size_t>(y) * static_cast<size_t>(field.width) + static_cast<size_t>(x);
    outSignedPx = field.signed_distance_px[index];
    outInside = outSignedPx < 0.0f;
    return true;
}

bool ComputeLensSdfFieldForMask(
    const uint8_t* mask,
    int width,
    int height,
    int downsample,
    SdfFieldResult& outField) {
    outField.Clear();

    std::vector<uint8_t> lowMask;
    int outWidth = 0;
    int outHeight = 0;
    if (!DownsampleMaskPow2(mask, width, height, downsample, lowMask, outWidth, outHeight)) {
        return false;
    }

    if (!ComputeSignedDistanceSdfFieldChamfer(lowMask.data(), outWidth, outHeight, outField)) {
        outField.Clear();
        return false;
    }
    outField.pixel_scale = static_cast<float>(NormalizeLensDownsamplePow2(downsample));
    return true;
}

bool ComputeLensSdfRgbaForMask(
    const uint8_t* mask,
    int width,
    int height,
    int downsample,
    float maxAbsPx,
    std::vector<uint32_t>& outRgba,
    int& outWidth,
    int& outHeight) {
    outRgba.clear();
    outWidth = 0;
    outHeight = 0;

    SdfFieldResult field;
    if (!ComputeLensSdfFieldForMask(mask, width, height, downsample, field)) {
        return false;
    }

    const int normalizedDownsample = NormalizeLensDownsamplePow2(downsample);
    const float lowMaxAbsPx = maxAbsPx / static_cast<float>(normalizedDownsample);
    BuildSignedDistanceSdfRgba(field.View(), lowMaxAbsPx, outRgba);
    outWidth = field.width;
    outHeight = field.height;
    return outRgba.size() == static_cast<size_t>(outWidth) * static_cast<size_t>(outHeight);
}
