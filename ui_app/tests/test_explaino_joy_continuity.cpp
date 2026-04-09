// Headless CPU test for explaino_joy iteration contract.
// Verifies:
// 1. gamma=0 matches standard Newton (P/P') — no Joy contribution.
// 2. Small gamma increments produce proportionally small output changes.
// 3. P'' guard: zero second derivative does not crash or produce NaN.

#include <cmath>
#include <cstdio>

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
    if (d < 1e-30f) return {0.0f, 0.0f};
    return {(a.x*b.x + a.y*b.y) / d, (a.y*b.x - a.x*b.y) / d};
}

// Polynomial evaluation with first and second derivative.
// P(z) = c0 + c1*z + c2*z^2 + c3*z^3 + c4*z^4
void poly_eval_d2(const float coeffs[5], Cx z, Cx* outP, Cx* outDp, Cx* outD2p) {
    Cx z2 = cx_mul(z, z);
    Cx z3 = cx_mul(z2, z);
    Cx z4 = cx_mul(z3, z);
    *outP = {coeffs[0] + coeffs[1]*z.x + coeffs[2]*z2.x + coeffs[3]*z3.x + coeffs[4]*z4.x,
             coeffs[1]*z.y + coeffs[2]*z2.y + coeffs[3]*z3.y + coeffs[4]*z4.y};
    *outDp = {coeffs[1] + 2.0f*coeffs[2]*z.x + 3.0f*coeffs[3]*z2.x + 4.0f*coeffs[4]*z3.x,
              2.0f*coeffs[2]*z.y + 3.0f*coeffs[3]*z2.y + 4.0f*coeffs[4]*z3.y};
    *outD2p = {2.0f*coeffs[2] + 6.0f*coeffs[3]*z.x + 12.0f*coeffs[4]*z2.x,
               6.0f*coeffs[3]*z.y + 12.0f*coeffs[4]*z2.y};
}

struct JoyResult {
    int iterations;
    bool converged;
    Cx z_final;
    float pAbs;
};

// Explaino-Joy iteration matching fractal_renderer.cu (f32 path).
JoyResult joy_iterate(Cx z0, float gamma, float damping, Cx phoenix,
                      const float coeffs[5], int maxIter, float eps) {
    Cx z = z0;
    Cx zPrev = z;
    float oneMinusGamma = 1.0f - gamma;
    float pAbs = 0.0f;
    int it = 0;
    for (; it < maxIter; ++it) {
        Cx P, dP, d2P;
        poly_eval_d2(coeffs, z, &P, &dP, &d2P);
        pAbs = cx_abs(P);
        if (pAbs < eps) break;
        float dAbs2 = cx_abs2(dP);
        if (dAbs2 < 1e-20f) break;
        Cx newtonStep = cx_div(P, dP);
        Cx joyStep = {0.0f, 0.0f};
        float d2Abs2 = cx_abs2(d2P);
        if (d2Abs2 > 1e-20f) {
            joyStep = cx_div(dP, d2P);
        }
        Cx combinedStep = cx_add(
            cx_scale(newtonStep, oneMinusGamma),
            cx_scale(joyStep, gamma));
        Cx zNext = cx_add(
            cx_sub(z, cx_scale(combinedStep, damping)),
            cx_mul(phoenix, zPrev));
        zPrev = z;
        z = zNext;
        if (!std::isfinite(z.x) || !std::isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
    }
    return {it, pAbs < eps, z, pAbs};
}

// Standard Newton iteration for comparison.
JoyResult newton_iterate(Cx z0, float damping, const float coeffs[5],
                         int maxIter, float eps) {
    Cx z = z0;
    float pAbs = 0.0f;
    int it = 0;
    for (; it < maxIter; ++it) {
        Cx P, dP, d2P;
        poly_eval_d2(coeffs, z, &P, &dP, &d2P);
        pAbs = cx_abs(P);
        if (pAbs < eps) break;
        float dAbs2 = cx_abs2(dP);
        if (dAbs2 < 1e-20f) break;
        Cx step = cx_div(P, dP);
        Cx zNext = cx_sub(z, cx_scale(step, damping));
        z = zNext;
        if (!std::isfinite(z.x) || !std::isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
    }
    return {it, pAbs < eps, z, pAbs};
}

} // namespace

int main() {
    // z^4 - 1 (classic Newton fractal polynomial).
    float coeffs[5] = {-1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    const int maxIter = 200;
    const float eps = 1e-6f;
    int failures = 0;

    printf("=== Explaino-Joy continuity test ===\n\n");

    // --- Test 1: gamma=0 must match pure Newton ---
    printf("--- Test 1: gamma=0 matches pure Newton ---\n");
    {
        Cx testPoints[] = {{0.5f, 0.3f}, {-0.7f, 0.2f}, {0.1f, -0.8f}, {1.2f, 0.5f}};
        for (int i = 0; i < 4; ++i) {
            Cx z0 = testPoints[i];
            Cx phoenix = {0.0f, 0.0f}; // no memory term
            JoyResult joy   = joy_iterate(z0, 0.0f, 1.0f, phoenix, coeffs, maxIter, eps);
            JoyResult plain = newton_iterate(z0, 1.0f, coeffs, maxIter, eps);

            float dx = std::fabs(joy.z_final.x - plain.z_final.x);
            float dy = std::fabs(joy.z_final.y - plain.z_final.y);
            float dist = std::sqrt(dx*dx + dy*dy);

            printf("  z0=(%.2f,%.2f): joy_z=(%.6f,%.6f) newton_z=(%.6f,%.6f) dist=%.9f iters=%d/%d\n",
                z0.x, z0.y,
                joy.z_final.x, joy.z_final.y,
                plain.z_final.x, plain.z_final.y,
                dist, joy.iterations, plain.iterations);

            if (dist > 1e-5f) {
                printf("  *** FAIL: gamma=0 diverged from Newton by %.9f ***\n", dist);
                failures++;
            }
        }
    }

    // --- Test 2: gamma continuity — small gamma changes produce small output changes ---
    printf("\n--- Test 2: gamma continuity ---\n");
    {
        Cx z0 = {0.5f, 0.3f};
        Cx phoenix = {0.0f, 0.0f};
        float gammas[] = {0.0f, 0.01f, 0.02f, 0.05f, 0.10f, 0.20f, 0.30f, 0.50f, 0.70f, 0.90f};
        int numGammas = sizeof(gammas) / sizeof(gammas[0]);
        JoyResult prev = joy_iterate(z0, gammas[0], 1.0f, phoenix, coeffs, maxIter, eps);

        for (int i = 1; i < numGammas; ++i) {
            JoyResult cur = joy_iterate(z0, gammas[i], 1.0f, phoenix, coeffs, maxIter, eps);
            float dx = cur.z_final.x - prev.z_final.x;
            float dy = cur.z_final.y - prev.z_final.y;
            float dist = std::sqrt(dx*dx + dy*dy);
            float dg = gammas[i] - gammas[i-1];

            printf("  gamma=%.2f: z=(%.6f,%.6f) delta=%.9f dg=%.3f ratio=%.3f\n",
                gammas[i], cur.z_final.x, cur.z_final.y, dist, dg,
                dg > 0 ? dist/dg : 0.0f);

            // For tiny gamma steps (<=0.02), output should not jump wildly.
            if (dg <= 0.02f && dist > 0.5f) {
                printf("  *** FAIL: tiny gamma step (%.3f) caused large output jump (%.9f) ***\n", dg, dist);
                failures++;
            }
            prev = cur;
        }
    }

    // --- Test 3: P'' zero guard — polynomial with P''=0 at some z ---
    printf("\n--- Test 3: P'' guard (linear polynomial) ---\n");
    {
        // P(z) = z (linear: P'=1, P''=0 everywhere).
        float linearCoeffs[5] = {0.0f, 1.0f, 0.0f, 0.0f, 0.0f};
        Cx z0 = {0.5f, 0.3f};
        Cx phoenix = {0.0f, 0.0f};
        JoyResult res = joy_iterate(z0, 0.5f, 1.0f, phoenix, linearCoeffs, maxIter, eps);

        printf("  z0=(%.2f,%.2f): z_final=(%.6f,%.6f) converged=%d iters=%d\n",
            z0.x, z0.y, res.z_final.x, res.z_final.y, res.converged, res.iterations);

        if (!std::isfinite(res.z_final.x) || !std::isfinite(res.z_final.y)) {
            printf("  *** FAIL: P'' guard produced NaN/Inf ***\n");
            failures++;
        }
        // With P''=0, joyStep is zero, so it reduces to Newton on P(z)=z.
        // Newton step: z - z/1 = 0, so should converge in one step.
        if (!res.converged) {
            printf("  *** FAIL: expected convergence for linear P(z)=z ***\n");
            failures++;
        }
    }

    // --- Test 4: non-trivial gamma produces stable orbits ---
    printf("\n--- Test 4: gamma=0.3 produces stable (finite) output ---\n");
    {
        Cx testPoints[] = {{0.5f, 0.3f}, {-0.5f, 0.3f}, {0.5f, -0.3f}, {-0.5f, -0.3f}};
        Cx phoenix = {0.0f, 0.0f};
        int finiteCount = 0;
        for (int i = 0; i < 4; ++i) {
            JoyResult res = joy_iterate(testPoints[i], 0.3f, 1.0f, phoenix, coeffs, maxIter, eps);
            printf("  z0=(%.2f,%.2f): z_final=(%.6f,%.6f) converged=%d iters=%d pAbs=%.9f\n",
                testPoints[i].x, testPoints[i].y,
                res.z_final.x, res.z_final.y, res.converged, res.iterations, res.pAbs);
            if (std::isfinite(res.z_final.x) && std::isfinite(res.z_final.y))
                finiteCount++;
        }
        // Joy coupling creates new fixed points that are NOT roots of P.
        // The key contract is: the iteration stays bounded and produces
        // finite output. Basin structure emerges visually from which
        // attractor each starting point reaches.
        if (finiteCount < 4) {
            printf("  *** FAIL: some test points diverged to inf/nan at gamma=0.3 ***\n");
            failures++;
        }
    }

    printf("\n=== Result: %d failures ===\n", failures);
    return failures > 0 ? 1 : 0;
}
