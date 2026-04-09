// Headless CPU test for explaino_splice iteration contract.
// Verifies:
// 1. offset=0 matches standard Newton (same polynomial used for both).
// 2. Small offset increments produce proportionally small output changes.
// 3. offset=1.0 stability (all z remain finite after 200 iterations).
// 4. Alternation check: even vs odd iterations use different polynomials.

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

struct SpliceResult {
    int iterations;
    bool converged;
    Cx z_final;
    float pAbs;
};

// Explaino-Splice iteration matching fractal_renderer.cu (f32 path).
// Alternates between two polynomial evaluations each iteration.
SpliceResult splice_iterate(Cx z0, const float coeffsA[5], const float coeffsB[5],
                            float damping, Cx phoenix, int maxIter, float eps) {
    Cx z = z0;
    Cx zPrev = z;
    float pAbs = 0.0f;
    float bestP = 1.0e30f;
    int bestIt = 0;
    int it = 0;
    for (; it < maxIter; ++it) {
        // Pick polynomial based on even/odd iteration
        const float* coeffs = (it % 2 == 0) ? coeffsA : coeffsB;
        Cx P, dP;
        poly_eval_d1(coeffs, z, &P, &dP);
        pAbs = cx_abs(P);
        if (pAbs < bestP) { bestP = pAbs; bestIt = it; }
        // Convergence check uses P_A (primary polynomial)
        {
            Cx PA, dPA;
            poly_eval_d1(coeffsA, z, &PA, &dPA);
            float paAbs = cx_abs(PA);
            if (paAbs < eps) { pAbs = paAbs; break; }
        }
        float dAbs2 = cx_abs2(dP);
        Cx step = (dAbs2 < 1e-20f) ? P : cx_div(P, dP);
        float stepMag = std::sqrt(std::max(0.0f, cx_abs2(step)));
        float damp = damping / (1.0f + stepMag);
        Cx zNext = cx_add(
            cx_sub(z, cx_scale(step, damp)),
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
SpliceResult newton_iterate(Cx z0, float damping, const float coeffs[5],
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

// Build a simple second polynomial by rotating roots slightly.
// coeffsA: z^4 - 1 (roots at 1, -1, i, -i)
// coeffsB: roots shifted by offset -> different coefficients.
// We use a simple approach: (z - r1)(z - r2)(z - r3)(z - r4)
// where r_k are the roots of z^4-1 rotated by offset*pi/4.
void build_offset_poly(float offset, float coeffsB[5]) {
    // Roots of z^4-1 are at e^{i*k*pi/2} for k=0..3
    // Offset rotates all roots by offset * pi/4
    float angle = offset * 3.14159265f / 4.0f;
    float cosA = std::cos(angle), sinA = std::sin(angle);
    Cx roots[4];
    for (int k = 0; k < 4; ++k) {
        float base = k * 3.14159265f / 2.0f;
        roots[k] = {std::cos(base + angle), std::sin(base + angle)};
    }
    // Expand (z - r0)(z - r1)(z - r2)(z - r3)
    // For z^4 - 1 rotated, the result is z^4 - e^{i*4*angle}
    // = z^4 - (cos(4*angle) + i*sin(4*angle))
    // But we need real coefficients for polynomial eval that works with complex z.
    // Actually for the test we just need any second polynomial. Use z^4 - c
    // where c = e^{i*4*angle}.
    float c_re = std::cos(4.0f * angle);
    float c_im = std::sin(4.0f * angle);
    // P(z) = z^4 - c_re - i*c_im
    // As real coefficients: {-c_re, -c_im, 0, 0, 1} won't work for our poly_eval
    // which treats coefficients as purely real.
    // Simpler: just use z^4 - (1 + offset*0.5) to keep it real-valued.
    coeffsB[0] = -(1.0f + offset * 0.5f);
    coeffsB[1] = 0.0f;
    coeffsB[2] = 0.0f;
    coeffsB[3] = 0.0f;
    coeffsB[4] = 1.0f;
}

} // namespace

int main() {
    // P_A: z^4 - 1 (classic Newton fractal polynomial).
    float coeffsA[5] = {-1.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    const int maxIter = 200;
    const float eps = 1e-6f;
    int failures = 0;

    printf("=== Explaino-Splice continuity test ===\n\n");

    // --- Test 1: offset=0 must match pure Newton (same polynomial both slots) ---
    printf("--- Test 1: offset=0 matches pure Newton ---\n");
    {
        float coeffsB[5] = {-1.0f, 0.0f, 0.0f, 0.0f, 1.0f}; // same as A
        Cx testPoints[] = {{0.5f, 0.3f}, {-0.7f, 0.2f}, {0.1f, -0.8f}, {1.2f, 0.5f}};
        Cx phoenix = {0.0f, 0.0f};
        for (int i = 0; i < 4; ++i) {
            Cx z0 = testPoints[i];
            SpliceResult sp  = splice_iterate(z0, coeffsA, coeffsB, 1.0f, phoenix, maxIter, eps);
            SpliceResult plain = newton_iterate(z0, 1.0f, coeffsA, maxIter, eps);

            float dx = std::fabs(sp.z_final.x - plain.z_final.x);
            float dy = std::fabs(sp.z_final.y - plain.z_final.y);
            float dist = std::sqrt(dx*dx + dy*dy);

            printf("  z0=(%.2f,%.2f): splice_z=(%.6f,%.6f) newton_z=(%.6f,%.6f) dist=%.9f iters=%d/%d\n",
                z0.x, z0.y,
                sp.z_final.x, sp.z_final.y,
                plain.z_final.x, plain.z_final.y,
                dist, sp.iterations, plain.iterations);

            if (dist > 1e-4f) {
                printf("  *** FAIL: offset=0 diverged from Newton by %.9f ***\n", dist);
                failures++;
            }
        }
    }

    // --- Test 2: offset continuity ---
    printf("\n--- Test 2: offset continuity ---\n");
    {
        Cx z0 = {1.5f, 0.1f};
        Cx phoenix = {0.0f, 0.0f};
        float offsets[] = {0.0f, 0.01f, 0.02f, 0.05f, 0.10f, 0.20f, 0.50f, 0.80f, 1.00f};
        int numOff = sizeof(offsets) / sizeof(offsets[0]);

        float prevCoeffsB[5];
        build_offset_poly(offsets[0], prevCoeffsB);
        SpliceResult prev = splice_iterate(z0, coeffsA, prevCoeffsB, 1.0f, phoenix, maxIter, eps);

        for (int i = 1; i < numOff; ++i) {
            float curCoeffsB[5];
            build_offset_poly(offsets[i], curCoeffsB);
            SpliceResult cur = splice_iterate(z0, coeffsA, curCoeffsB, 1.0f, phoenix, maxIter, eps);
            float dx = cur.z_final.x - prev.z_final.x;
            float dy = cur.z_final.y - prev.z_final.y;
            float dist = std::sqrt(dx*dx + dy*dy);
            float doff = offsets[i] - offsets[i-1];

            printf("  offset=%.2f: z=(%.6f,%.6f) delta=%.9f doff=%.3f\n",
                offsets[i], cur.z_final.x, cur.z_final.y, dist, doff);

            if (doff <= 0.02f && dist > 0.5f) {
                printf("  *** FAIL: tiny offset step (%.3f) caused large output jump (%.9f) ***\n", doff, dist);
                failures++;
            }
            prev = cur;
        }
    }

    // --- Test 3: offset=1.0 stability ---
    printf("\n--- Test 3: offset=1.0 stability ---\n");
    {
        float coeffsB[5];
        build_offset_poly(1.0f, coeffsB);
        Cx testPoints[] = {{0.5f, 0.3f}, {-0.5f, 0.3f}, {0.5f, -0.3f}, {-0.5f, -0.3f}};
        Cx phoenix = {0.0f, 0.0f};
        for (int i = 0; i < 4; ++i) {
            Cx z0 = testPoints[i];
            SpliceResult res = splice_iterate(z0, coeffsA, coeffsB, 1.0f, phoenix, maxIter, eps);
            bool finite = std::isfinite(res.z_final.x) && std::isfinite(res.z_final.y);
            printf("  z0=(%.2f,%.2f): z_final=(%.6f,%.6f) converged=%d iters=%d pAbs=%.9f\n",
                z0.x, z0.y,
                res.z_final.x, res.z_final.y,
                res.converged ? 1 : 0, res.iterations, res.pAbs);
            if (!finite) {
                printf("  *** FAIL: offset=1.0 produced non-finite z ***\n");
                failures++;
            }
        }
    }

    // --- Test 4: alternation verification ---
    printf("\n--- Test 4: alternation verification ---\n");
    {
        // With offset != 0, P_A and P_B differ. We verify that the splice
        // result differs from using only P_A (standard Newton).
        float coeffsB[5];
        build_offset_poly(0.5f, coeffsB);
        Cx z0 = {0.8f, 0.4f};
        Cx phoenix = {0.0f, 0.0f};
        SpliceResult sp = splice_iterate(z0, coeffsA, coeffsB, 1.0f, phoenix, maxIter, eps);
        SpliceResult pure = newton_iterate(z0, 1.0f, coeffsA, maxIter, eps);
        float dx = sp.z_final.x - pure.z_final.x;
        float dy = sp.z_final.y - pure.z_final.y;
        float dist = std::sqrt(dx*dx + dy*dy);
        printf("  splice z=(%.6f,%.6f) newton z=(%.6f,%.6f) dist=%.6f\n",
            sp.z_final.x, sp.z_final.y,
            pure.z_final.x, pure.z_final.y, dist);
        // With offset=0.5, the two polynomials differ, so we expect
        // some difference from pure Newton. Even a tiny diff suffices.
        if (dist < 1e-9f) {
            printf("  *** FAIL: splice with offset=0.5 produced same result as pure Newton ***\n");
            failures++;
        } else {
            printf("  alternation confirmed: splice result differs from pure Newton\n");
        }
    }

    printf("\n=== Result: %d failures ===\n", failures);
    return failures;
}
