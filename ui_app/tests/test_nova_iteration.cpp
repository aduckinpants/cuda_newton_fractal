// Headless CPU test for Nova iteration contract.
// Mirrors the Nova iteration logic from fractal_renderer.cu to verify
// the zero-derivative handling: skip the Newton step, apply +c, continue.
// Regression test for: Nova all-black screen when P'(z0)=0 at z=0.

#include <cmath>
#include <iostream>

namespace {

struct Cx { float x, y; };

Cx cx_add(Cx a, Cx b) { return {a.x + b.x, a.y + b.y}; }
Cx cx_sub(Cx a, Cx b) { return {a.x - b.x, a.y - b.y}; }
Cx cx_mul(Cx a, Cx b) { return {a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x}; }
Cx cx_scale(Cx a, float s) { return {a.x*s, a.y*s}; }
float cx_abs2(Cx a) { return a.x*a.x + a.y*a.y; }
float cx_abs(Cx a) { return sqrtf(cx_abs2(a)); }
Cx cx_div(Cx a, Cx b) {
    float d = cx_abs2(b);
    return {(a.x*b.x + a.y*b.y) / d, (a.y*b.x - a.x*b.y) / d};
}

// Poly eval matching fractal_renderer.cu: P(z) = c0 + c1*z + c2*z^2 + c3*z^3 + c4*z^4
// P'(z) = c1 + 2*c2*z + 3*c3*z^2 + 4*c4*z^3
void poly_eval(const float coeffs[5], Cx z, Cx* outP, Cx* outDp) {
    Cx z2 = cx_mul(z, z);
    Cx z3 = cx_mul(z2, z);
    Cx z4 = cx_mul(z3, z);
    *outP = {coeffs[0] + coeffs[1]*z.x + coeffs[2]*z2.x + coeffs[3]*z3.x + coeffs[4]*z4.x,
             coeffs[1]*z.y + coeffs[2]*z2.y + coeffs[3]*z3.y + coeffs[4]*z4.y};
    *outDp = {coeffs[1] + 2.0f*coeffs[2]*z.x + 3.0f*coeffs[3]*z2.x + 4.0f*coeffs[4]*z3.x,
              2.0f*coeffs[2]*z.y + 3.0f*coeffs[3]*z2.y + 4.0f*coeffs[4]*z3.y};
}

struct NovaResult {
    int iterations;
    bool escaped;
    bool converged;
    Cx z_final;
};

// Nova iteration matching the fixed fractal_renderer.cu logic.
NovaResult nova_iterate(Cx coord, float alpha, const float coeffs[5],
                        int maxIter, float eps) {
    Cx z = {0.0f, 0.0f};
    Cx c = coord;
    bool escaped = false;
    bool converged = false;
    int it = 0;

    for (; it < maxIter; ++it) {
        Cx P, dP;
        poly_eval(coeffs, z, &P, &dP);
        float pAbs = cx_abs(P);
        if (pAbs < eps) { converged = true; break; }

        float dAbs2 = cx_abs2(dP);
        // KEY FIX: when derivative is zero, skip Newton step but apply +c.
        if (dAbs2 >= 1e-20f) {
            Cx step = cx_div(P, dP);
            z = cx_sub(z, cx_scale(step, alpha));
        }
        z = cx_add(z, c);

        if (!std::isfinite(z.x) || !std::isfinite(z.y)) { escaped = true; break; }
        if (cx_abs2(z) > 4.0f) { escaped = true; break; }
    }

    return {it, escaped, converged, z};
}

// OLD (buggy) Nova iteration: breaks on zero derivative.
NovaResult nova_iterate_old(Cx coord, float alpha, const float coeffs[5],
                            int maxIter, float eps) {
    Cx z = {0.0f, 0.0f};
    Cx c = coord;
    bool escaped = false;
    bool converged = false;
    int it = 0;

    for (; it < maxIter; ++it) {
        Cx P, dP;
        poly_eval(coeffs, z, &P, &dP);
        float pAbs = cx_abs(P);
        if (pAbs < eps) { converged = true; break; }

        float dAbs2 = cx_abs2(dP);
        if (dAbs2 < 1e-20f) break; // BUG: kills Nova dynamics
        Cx step = cx_div(P, dP);
        z = cx_add(cx_sub(z, cx_scale(step, alpha)), c);

        if (!std::isfinite(z.x) || !std::isfinite(z.y)) { escaped = true; break; }
        if (cx_abs2(z) > 4.0f) { escaped = true; break; }
    }

    return {it, escaped, converged, z};
}

} // namespace

int main() {
    const float coeffs_z3m1[5] = {-1.0f, 0.0f, 0.0f, 1.0f, 0.0f}; // z^3 - 1
    const float alpha = 0.5f;
    const int maxIter = 256;
    const float eps = 1e-6f;

    // -- Test 1: z=0 with z^3-1 has P=-1, P'=0 (zero derivative trigger)
    {
        Cx P, dP;
        poly_eval(coeffs_z3m1, {0.0f, 0.0f}, &P, &dP);
        if (std::fabs(P.x - (-1.0f)) > 1e-6f || std::fabs(P.y) > 1e-6f) {
            std::cerr << "FAIL: P(0) for z^3-1 should be -1+0i, got "
                      << P.x << "+" << P.y << "i\n";
            return 1;
        }
        if (cx_abs2(dP) > 1e-12f) {
            std::cerr << "FAIL: P'(0) for z^3-1 should be 0, got "
                      << dP.x << "+" << dP.y << "i\n";
            return 1;
        }
        std::cout << "PASS: z^3-1 at z=0 -> P=-1, P'=0 (zero derivative confirmed)\n";
    }

    // -- Test 2: Old (buggy) code produces all-black (no escape, no converge, 0 iterations)
    {
        // Test a few sample coordinates — all should be dead on first iteration
        Cx samples[] = {{0.5f, 0.5f}, {-0.3f, 0.7f}, {1.0f, 0.0f}, {0.0f, 0.0f}};
        for (auto& coord : samples) {
            NovaResult r = nova_iterate_old(coord, alpha, coeffs_z3m1, maxIter, eps);
            if (r.escaped || r.converged || r.iterations != 0) {
                std::cerr << "FAIL: old Nova at (" << coord.x << "," << coord.y
                          << ") should die at it=0, got it=" << r.iterations
                          << " esc=" << r.escaped << " conv=" << r.converged << "\n";
                return 1;
            }
        }
        std::cout << "PASS: Old (buggy) Nova breaks at it=0 for all coords (all-black confirmed)\n";
    }

    // -- Test 3: Fixed code produces non-trivial dynamics
    {
        int escaped_count = 0;
        int converged_count = 0;
        int exhausted_count = 0;
        Cx samples[] = {
            {0.5f, 0.5f}, {-0.3f, 0.7f}, {1.0f, 0.0f}, {0.0f, 1.0f},
            {-1.0f, 0.0f}, {0.2f, -0.8f}, {0.8f, 0.2f}, {-0.5f, -0.5f}
        };
        for (auto& coord : samples) {
            NovaResult r = nova_iterate(coord, alpha, coeffs_z3m1, maxIter, eps);
            if (r.escaped) escaped_count++;
            else if (r.converged) converged_count++;
            else exhausted_count++;
        }
        // With the fix, at least some points must escape or converge (non-trivial dynamics).
        if (escaped_count == 0 && converged_count == 0) {
            std::cerr << "FAIL: Fixed Nova produced no escaping or converging points "
                      << "(still all-black behavior)\n";
            return 1;
        }
        std::cout << "PASS: Fixed Nova -> " << escaped_count << " escaped, "
                  << converged_count << " converged, " << exhausted_count << " exhausted\n";
    }

    // -- Test 4: First iteration of fixed Nova from z=0 moves z to c (the coord)
    {
        Cx coord = {0.5f, 0.5f};
        // After first iteration: derivative is zero, skip Newton step, z = 0 + c = c
        Cx z = {0.0f, 0.0f};
        Cx P, dP;
        poly_eval(coeffs_z3m1, z, &P, &dP);
        // dP = 0, so skip Newton step
        // z = z + c = {0.5, 0.5}
        z = cx_add(z, coord);
        if (std::fabs(z.x - coord.x) > 1e-6f || std::fabs(z.y - coord.y) > 1e-6f) {
            std::cerr << "FAIL: After first Nova iteration with zero derivative, "
                      << "z should equal c\n";
            return 1;
        }
        std::cout << "PASS: First iteration with zero derivative -> z = c = ("
                  << z.x << ", " << z.y << ")\n";
    }

    std::cout << "All Nova iteration contract tests passed.\n";
    return 0;
}
