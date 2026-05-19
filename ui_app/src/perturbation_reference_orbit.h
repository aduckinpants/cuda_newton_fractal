#pragma once

#include "fractal_types.h"

#include <algorithm>
#include <cmath>
#include <vector>

struct PerturbationReferenceOrbitRequest {
    bool enabled{false};
    FractalType fractalType{FractalType::newton};
    double refCx{0.0};
    double refCy{0.0};
    double juliaCx{-0.7};
    double juliaCy{0.27015};
    int refLen{0};
};

struct PerturbationReferenceOrbitCacheKey {
    FractalType fractalType{FractalType::newton};
    double refCx{0.0};
    double refCy{0.0};
    double juliaCx{-0.7};
    double juliaCy{0.27015};
    int refLen{0};
};

inline constexpr bool SupportsPerturbationReferenceOrbit(FractalType fractalType) {
    return fractalType == FractalType::mandelbrot || fractalType == FractalType::julia;
}

inline double PerturbationLog2ZoomThreshold() {
    const double perturbZoomThreshold = 1.0e10;
    return std::log(perturbZoomThreshold) / std::log(2.0);
}

inline PerturbationReferenceOrbitRequest ResolvePerturbationReferenceOrbitRequest(
    const ViewState& view,
    const KernelParams& params) {
    PerturbationReferenceOrbitRequest request;
    request.fractalType = view.fractal_type;
    request.refCx = view.center_hp_x;
    request.refCy = view.center_hp_y;
    if (view.fractal_type == FractalType::julia) {
        request.juliaCx = static_cast<double>(params.julia_c_real);
        request.juliaCy = static_cast<double>(params.julia_c_imag);
    }

    if (SupportsPerturbationReferenceOrbit(view.fractal_type) && view.log2_zoom >= PerturbationLog2ZoomThreshold()) {
        request.enabled = true;
        request.refLen = std::max(1, params.max_iter) + 1;
    }

    return request;
}

inline PerturbationReferenceOrbitCacheKey MakePerturbationReferenceOrbitCacheKey(
    const PerturbationReferenceOrbitRequest& request) {
    return {request.fractalType, request.refCx, request.refCy, request.juliaCx, request.juliaCy, request.refLen};
}

inline bool MatchesPerturbationReferenceOrbitCacheKey(
    const PerturbationReferenceOrbitCacheKey& key,
    const PerturbationReferenceOrbitRequest& request) {
    return key.fractalType == request.fractalType &&
        key.refCx == request.refCx &&
        key.refCy == request.refCy &&
        key.juliaCx == request.juliaCx &&
        key.juliaCy == request.juliaCy &&
        key.refLen == request.refLen;
}

template <typename Point>
inline void BuildPerturbationReferenceOrbit(
    const PerturbationReferenceOrbitRequest& request,
    std::vector<Point>* outOrbit) {
    if (!outOrbit) return;

    outOrbit->clear();
    if (!request.enabled || request.refLen <= 0 || !SupportsPerturbationReferenceOrbit(request.fractalType)) {
        return;
    }

    outOrbit->resize(static_cast<size_t>(request.refLen));

    double zx = 0.0;
    double zy = 0.0;
    double cx = request.refCx;
    double cy = request.refCy;

    if (request.fractalType == FractalType::julia) {
        zx = request.refCx;
        zy = request.refCy;
        cx = request.juliaCx;
        cy = request.juliaCy;
    }

    (*outOrbit)[0] = {zx, zy};
    for (int index = 0; index < request.refLen - 1; ++index) {
        const double nextX = zx * zx - zy * zy + cx;
        const double nextY = 2.0 * zx * zy + cy;
        zx = nextX;
        zy = nextY;
        (*outOrbit)[static_cast<size_t>(index + 1)] = {zx, zy};
    }
}
