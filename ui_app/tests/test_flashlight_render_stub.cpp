#include "fractal_types.h"

#include <cstddef>

bool RenderFractalCUDA(
    const ViewState&,
    const KernelParams&,
    const RenderSettings&,
    uint32_t* outRGBA,
    uint8_t* outMask,
    RenderStats* outStats,
    const char** outError) {
    if (outRGBA) {
        outRGBA[0] = 0xFF000000u;
    }
    if (outMask) {
        outMask[0] = 0u;
    }
    if (outStats) {
        *outStats = RenderStats{};
    }
    if (outError) {
        *outError = "RenderFractalCUDA stub should not be called in this unit test";
    }
    return false;
}
