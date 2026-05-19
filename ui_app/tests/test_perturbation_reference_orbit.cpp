#include "../src/perturbation_reference_orbit.h"

#include <cmath>
#include <iostream>
#include <vector>

namespace {

struct D2 {
    double x;
    double y;
};

bool NearlyEqual(double left, double right, double eps = 1.0e-12) {
    const double delta = left - right;
    return delta < eps && delta > -eps;
}

bool NearlyEqualPoint(D2 left, D2 right, double eps = 1.0e-12) {
    return NearlyEqual(left.x, right.x, eps) && NearlyEqual(left.y, right.y, eps);
}

} // namespace

int main() {
    if (!SupportsPerturbationReferenceOrbit(FractalType::mandelbrot) ||
        !SupportsPerturbationReferenceOrbit(FractalType::julia) ||
        SupportsPerturbationReferenceOrbit(FractalType::phoenix) ||
        SupportsPerturbationReferenceOrbit(FractalType::newton)) {
        std::cerr << "Perturbation helper should stay limited to Mandelbrot and Julia\n";
        return 1;
    }

    {
        ViewState view{};
        view.fractal_type = FractalType::mandelbrot;
        view.log2_zoom = PerturbationLog2ZoomThreshold() - 0.5;
        view.center_hp_x = 1.25;
        view.center_hp_y = -0.5;

        KernelParams params{};
        params.max_iter = 12;

        const PerturbationReferenceOrbitRequest request = ResolvePerturbationReferenceOrbitRequest(view, params);
        if (request.enabled || request.refLen != 0 || !NearlyEqual(request.refCx, 1.25) || !NearlyEqual(request.refCy, -0.5)) {
            std::cerr << "Low-zoom Mandelbrot should not request a perturbation reference orbit\n";
            return 1;
        }
    }

    {
        ViewState view{};
        view.fractal_type = FractalType::mandelbrot;
        view.log2_zoom = PerturbationLog2ZoomThreshold();
        view.center_hp_x = -0.75;
        view.center_hp_y = 0.125;

        KernelParams params{};
        params.max_iter = 7;

        const PerturbationReferenceOrbitRequest request = ResolvePerturbationReferenceOrbitRequest(view, params);
        if (!request.enabled || request.refLen != 8 || !NearlyEqual(request.refCx, -0.75) || !NearlyEqual(request.refCy, 0.125)) {
            std::cerr << "Deep Mandelbrot should request a perturbation reference orbit tied to max_iter and center_hp\n";
            return 1;
        }

        const PerturbationReferenceOrbitCacheKey key = MakePerturbationReferenceOrbitCacheKey(request);
        if (!MatchesPerturbationReferenceOrbitCacheKey(key, request)) {
            std::cerr << "Orbit cache key should match the request it was derived from\n";
            return 1;
        }

        PerturbationReferenceOrbitRequest movedRequest = request;
        movedRequest.refCx += 1.0e-6;
        if (MatchesPerturbationReferenceOrbitCacheKey(key, movedRequest)) {
            std::cerr << "Orbit cache key should reject changed reference centers\n";
            return 1;
        }
    }

    {
        const PerturbationReferenceOrbitRequest request{true, FractalType::mandelbrot, 1.0, 0.0, -0.7, 0.27015, 4};
        std::vector<D2> orbit;
        BuildPerturbationReferenceOrbit(request, &orbit);
        if (orbit.size() != 4 ||
            !NearlyEqualPoint(orbit[0], {0.0, 0.0}) ||
            !NearlyEqualPoint(orbit[1], {1.0, 0.0}) ||
            !NearlyEqualPoint(orbit[2], {2.0, 0.0}) ||
            !NearlyEqualPoint(orbit[3], {5.0, 0.0})) {
            std::cerr << "Mandelbrot reference orbit should follow z_{n+1}=z_n^2+c from z0=0\n";
            return 1;
        }
    }

    {
        const PerturbationReferenceOrbitRequest request{true, FractalType::julia, 0.0, 0.0, -0.7, 0.27015, 2};
        std::vector<D2> orbit;
        BuildPerturbationReferenceOrbit(request, &orbit);
        if (orbit.size() != 2 ||
            !NearlyEqualPoint(orbit[0], {0.0, 0.0}) ||
            !NearlyEqualPoint(orbit[1], {-0.7, 0.27015}, 1.0e-9)) {
            std::cerr << "Julia reference orbit should encode the reference z0 and the fixed Julia constant\n";
            return 1;
        }
    }

    {
        ViewState view{};
        view.fractal_type = FractalType::julia;
        view.log2_zoom = PerturbationLog2ZoomThreshold();
        view.center_hp_x = 0.0;
        view.center_hp_y = 0.0;
        KernelParams params{};
        params.max_iter = 1;
        params.julia_c_real = 0.285f;
        params.julia_c_imag = 0.01f;

        const PerturbationReferenceOrbitRequest request = ResolvePerturbationReferenceOrbitRequest(view, params);
        std::vector<D2> orbit;
        BuildPerturbationReferenceOrbit(request, &orbit);
        if (!request.enabled ||
            !NearlyEqual(request.juliaCx, 0.285, 1.0e-6) ||
            !NearlyEqual(request.juliaCy, 0.01, 1.0e-6) ||
            orbit.size() != 2 ||
            !NearlyEqualPoint(orbit[1], {0.285, 0.01}, 1.0e-6)) {
            std::cerr << "Julia perturbation reference orbit should use the user-owned Julia constant\n";
            return 1;
        }

        PerturbationReferenceOrbitCacheKey key = MakePerturbationReferenceOrbitCacheKey(request);
        PerturbationReferenceOrbitRequest changed = request;
        changed.juliaCx += 0.25;
        if (MatchesPerturbationReferenceOrbitCacheKey(key, changed)) {
            std::cerr << "Julia perturbation cache key should reject changed Julia constants\n";
            return 1;
        }
    }

    std::cout << "test_perturbation_reference_orbit: all passed\n";
    return 0;
}
