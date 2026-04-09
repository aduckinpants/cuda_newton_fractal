// Headless CPU test for explaino_bell iteration contract.
// Verifies:
// 1. beta=0 matches standard Newton (no decomposition).
// 2. Small beta increments produce proportionally small output changes.
// 3. beta=1.0 (pure reaction channel) produces finite, stable output.
// 4. Measurement-reaction decomposition produces orthogonal channels.

#include <algorithm>
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

void poly_eval_d1(const float coeffs[5], Cx z, Cx* outP, Cx* outDp) {
    Cx z2 = cx_mul(z, z);
    Cx z3 = cx_mul(z2, z);
    Cx z4 = cx_mul(z3, z);
    *outP = {coeffs[0] + coeffs[1]*z.x + coeffs[2]*z2.x + coeffs[3]*z3.x + coeffs[4]*z4.x,
             coeffs[1]*z.y + coeffs[2]*z2.y + coeffs[3]*z3.y + coeffs[4]*z4.y};
    *outDp = {coeffs[1] + 2.0f*coeffs[2]*z.x + 3.0f*coeffs[3]*z2.x + 4.0f*coeffs[4]*z3.x,
              2.0f*coeffs[2]*z.y + 3.0f*coeffs[3]*z2.y + 4.0f*coeffs[4]*z3.y};
}

struct BellResult {
    int iterations;
    bool converged;
    Cx z_final;
    float pAbs;
};

// Explaino-Bell iteration matching fractal_renderer.cu (f32 path).
BellResult bell_iterate(Cx z0, float beta, float damping, Cx phoenix,
                        const float coeffs[5], int maxIter, float eps) {
    Cx z = z0;
    Cx zPrev = z;
    float pAbs = 0.0f;
    float bestP = 1.0e30f;
    int bestIt = 0;
    int it = 0;
    for (; it < maxIter; ++it) {
        Cx P, dP;
        poly_eval_d1(coeffs, z, &P, &dP);
        pAbs = cx_abs(P);
        if (pAbs < bestP) { bestP = pAbs; bestIt = it; }
        if (pAbs < eps) break;
        float dAbs2 = cx_abs2(dP);
        Cx newtonStep = (dAbs2 < 1e-20f) ? P : cx_div(P, dP);
        // Decompose step by P-phase
        float pMag = std::max(1e-20f, pAbs);
        Cx pHat = {P.x / pMag, P.y / pMag};
        float dotPar = newtonStep.x * pHat.x + newtonStep.y * pHat.y;
        Cx sParallel = {dotPar * pHat.x, dotPar * pHat.y};
        Cx combinedStep = {newtonStep.x - beta * sParallel.x,
                           newtonStep.y - beta * sParallel.y};
        float stepMag = std::sqrt(std::max(0.0f, cx_abs2(combinedStep)));
        float damp = damping / (1.0f + stepMag);
        Cx zNext = cx_add(
            cx_sub(z, cx_scale(combinedStep, damp)),
            cx_mul(phoenix, zPrev));
        zPrev = z;
        z = zNext;
        float r2 = cx_abs2(z);
        if (r2 > 16.0f) {
            float r = std::sqrt(r2);
            float s = 4.0f / std::max(1e-12f, r);
            z = cx_scale(z, s);
        }
        if (!std::isfinite(z.x) || !std::isfinite(z.y)) { z = {0.0f, 0.0f}; break; }
    }
    if (pAbs >= eps) it = bestIt;
    return {it, pAbs < eps, z, pAbs};
}

// Standard Newton iteration for comparison.
BellResult newton_iterate(Cx z0, float damping, const float coeffs[5],
                          int maxIter, float eps) {
    Cx z = z0;
    float pAbs = 0.0f;
    int it = 0;
    for (; it < maxIter; ++it) {
        Cx P, dP;
        poly_eval_d1(coeffs, z, &P, &dP);
        pAbs = cx_abs(P);
        if (pAbs < eps) break;
        float dAbs2 = cx_abs2(dP);
        if (dAbs2 < 1e-20f) break;
        Cx step = cx_div(P, dP);
        float stepMag = std::sqrt(std::max(0.0f, cx_abs2(step)));
        float damp = damping / (1.0f + stepMag);
        Cx zNext = cx_sub(z, cx_scale(step, damp));
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

    printf("=== Explaino-Bell continuity test ===\n\n");

    // --- Test 1: beta=0 must match pure Newton ---
    printf("--- Test 1: beta=0 matches pure Newton ---\n");
    {
        Cx testPoints[] = {{0.5f, 0.3f}, {-0.7f, 0.2f}, {0.1f, -0.8f}, {1.2f, 0.5f}};
        for (int i = 0; i < 4; ++i) {
            Cx z0 = testPoints[i];
            Cx phoenix = {0.0f, 0.0f};
            BellResult bell  = bell_iterate(z0, 0.0f, 1.0f, phoenix, coeffs, maxIter, eps);
            BellResult plain = newton_iterate(z0, 1.0f, coeffs, maxIter, eps);

            float dx = std::fabs(bell.z_final.x - plain.z_final.x);
            float dy = std::fabs(bell.z_final.y - plain.z_final.y);
            float dist = std::sqrt(dx*dx + dy*dy);

            printf("  z0=(%.2f,%.2f): bell_z=(%.6f,%.6f) newton_z=(%.6f,%.6f) dist=%.9f iters=%d/%d\n",
                z0.x, z0.y,
                bell.z_final.x, bell.z_final.y,
                plain.z_final.x, plain.z_final.y,
                dist, bell.iterations, plain.iterations);

            if (dist > 1e-5f) {
                printf("  *** FAIL: beta=0 diverged from Newton by %.9f ***\n", dist);
                failures++;
            }
        }
    }

    // --- Test 2: beta continuity ---
    // Use a starting point deep inside a basin (1.5,0.1) so tiny beta
    // perturbation cannot cause basin switching.
    printf("\n--- Test 2: beta continuity ---\n");
    {
        Cx z0 = {1.5f, 0.1f};
        Cx phoenix = {0.0f, 0.0f};
        float betas[] = {0.0f, 0.01f, 0.02f, 0.05f, 0.10f, 0.20f, 0.30f, 0.50f, 0.70f, 0.90f, 1.0f};
        int numBetas = sizeof(betas) / sizeof(betas[0]);
        BellResult prev = bell_iterate(z0, betas[0], 1.0f, phoenix, coeffs, maxIter, eps);

        for (int i = 1; i < numBetas; ++i) {
            BellResult cur = bell_iterate(z0, betas[i], 1.0f, phoenix, coeffs, maxIter, eps);
            float dx = cur.z_final.x - prev.z_final.x;
            float dy = cur.z_final.y - prev.z_final.y;
            float dist = std::sqrt(dx*dx + dy*dy);
            float db = betas[i] - betas[i-1];

            printf("  beta=%.2f: z=(%.6f,%.6f) delta=%.9f db=%.3f\n",
                betas[i], cur.z_final.x, cur.z_final.y, dist, db);

            if (db <= 0.02f && dist > 0.5f) {
                printf("  *** FAIL: tiny beta step (%.3f) caused large output jump (%.9f) ***\n", db, dist);
                failures++;
            }
            prev = cur;
        }
    }

    // --- Test 3: beta=1.0 (pure reaction channel) finite output ---
    printf("\n--- Test 3: beta=1.0 (pure reaction) stability ---\n");
    {
        Cx testPoints[] = {{0.5f, 0.3f}, {-0.5f, 0.3f}, {0.5f, -0.3f}, {-0.5f, -0.3f}};
        Cx phoenix = {0.0f, 0.0f};
        int finiteCount = 0;
        for (int i = 0; i < 4; ++i) {
            BellResult res = bell_iterate(testPoints[i], 1.0f, 1.0f, phoenix, coeffs, maxIter, eps);
            printf("  z0=(%.2f,%.2f): z_final=(%.6f,%.6f) converged=%d iters=%d pAbs=%.9f\n",
                testPoints[i].x, testPoints[i].y,
                res.z_final.x, res.z_final.y, res.converged, res.iterations, res.pAbs);
            if (std::isfinite(res.z_final.x) && std::isfinite(res.z_final.y))
                finiteCount++;
        }
        if (finiteCount < 4) {
            printf("  *** FAIL: some test points diverged to NaN/Inf at beta=1.0 ***\n");
            failures++;
        }
    }

    // --- Test 4: decomposition orthogonality check ---
    // At a sample point, verify that sParallel and sPerp are actually orthogonal
    // (their dot product = 0 within float tolerance).
    printf("\n--- Test 4: measurement-reaction orthogonality ---\n");
    {
        Cx z = {0.5f, 0.3f};
        Cx P, dP;
        poly_eval_d1(coeffs, z, &P, &dP);
        float pAbs = cx_abs(P);
        Cx step = cx_div(P, dP);
        float pMag = std::max(1e-20f, pAbs);
        Cx pHat = {P.x / pMag, P.y / pMag};
        float dotPar = step.x * pHat.x + step.y * pHat.y;
        Cx sParallel = {dotPar * pHat.x, dotPar * pHat.y};
        Cx sPerp = {step.x - sParallel.x, step.y - sParallel.y};
        // Check orthogonality: dot(sParallel, sPerp) should be ~0
        float ortho = sParallel.x * sPerp.x + sParallel.y * sPerp.y;
        // Check reconstruction: sParallel + sPerp = step
        float reconErr = std::sqrt((step.x - sParallel.x - sPerp.x)*(step.x - sParallel.x - sPerp.x) +
                                   (step.y - sParallel.y - sPerp.y)*(step.y - sParallel.y - sPerp.y));

        printf("  step=(%.6f,%.6f) P=(%.6f,%.6f)\n", step.x, step.y, P.x, P.y);
        printf("  sParallel=(%.6f,%.6f) sPerp=(%.6f,%.6f)\n",
            sParallel.x, sParallel.y, sPerp.x, sPerp.y);
        printf("  orthogonality dot=%.9e  reconstruction_error=%.9e\n", ortho, reconErr);

        if (std::fabs(ortho) > 1e-5f) {
            printf("  *** FAIL: sParallel and sPerp not orthogonal (dot=%.9e) ***\n", ortho);
            failures++;
        }
        if (reconErr > 1e-6f) {
            printf("  *** FAIL: sParallel + sPerp != step (err=%.9e) ***\n", reconErr);
            failures++;
        }
    }

    printf("\n=== Result: %d failures ===\n", failures);
    return failures > 0 ? 1 : 0;
}
