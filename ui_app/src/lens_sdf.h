#pragma once

#include <cstdint>
#include <vector>

int NormalizeLensDownsamplePow2(int value);

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

bool SampleSignedDistanceSdfChamfer(
    const uint8_t* mask,
    int width,
    int height,
    int x,
    int y,
    float& outSignedPx,
    bool& outInside);

bool ComputeLensSdfRgbaForMask(
    const uint8_t* mask,
    int width,
    int height,
    int downsample,
    float maxAbsPx,
    std::vector<uint32_t>& outRgba,
    int& outWidth,
    int& outHeight);
