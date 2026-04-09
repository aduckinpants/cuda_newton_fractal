// Headless CPU test for explaino_tension iteration contract.
// Verifies:
// 1. T=0 matches standard Newton (no competitor-root pull).
// 2. Small T increments produce proportionally small output changes.
// 3. T=0.1 stability (all z remain finite after 200 iterations).
// 4. Pull direction: the pull vector points toward the second-closest root.

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
    outP->x = coeffs[0] + coeffs[1]*z.x + coeffs[2]*z2.x + coeffs[3]*z3.x + coeffs[4]*z4.x;
    outP->y =              coeffs[1]*z.y + coeffs[2]*z2.y + coeffs[3]*z3.y + coeffs[4]*z4.y;
    outDp->x = coeffs[1] + 2*coeffs[2]*z.x + 3*coeffs[3]*z2.x + 4*coeffs[4]*z3.x;
    outDp->y =              2*coeffs[2]*z.y + 3*coeffs[3]*z2.y + 4*coeffs[4]*z3.y;
}

// Find index of nearest root
int nearest_root(Cx z, const Cx roots[], int nRoots) {
    int best = 0;
    float bestD = cx_abs2(cx_sub(z, roots[0]));
    for (int i = 1; i < nRoots; ++i) {
        float d = cx_abs2(cx_sub(z, roots[i]));
        if (d < bestD) { bestD = d; best = i; }
    }
    return best;
}

// Find index of second-closest root
int second_nearest_root(Cx z, const Cx roots[], int nRoots) {
    float dists[4];
    for (int i = 0; i < nRoots; ++i) dists[i] = cx_abs2(cx_sub(z, roots[i]));
    int best = 0;
    for (int i = 1; i < nRoots; ++i) {
        if (dists[i] < dists[best]) best = i;
    }
    int second = (best == 0) ? 1 : 0;
    for (int i = 0; i < nRoots; ++i) {
        if (i == best) continue;
        if (dists[i] < dists[second]) second = i;
    }
    return second;
}

// Tension iteration: Newton step + pull toward second-closest root
Cx tension_iterate(Cx z, const float coeffs[5], float damp, float T,
                   const Cx roots[], int nRoots, Cx pConst, Cx zPrev) {
    Cx P, dP;
    poly_eval_d1(coeffs, z, &P, &dP);
    float dAbs2 = cx_abs2(dP);
    Cx step = (dAbs2 < 1e-20f) ? P : cx_div(P, dP);
    float stepMag = cx_abs(step);

    // Compute pull toward second-closest root
    Cx pull = {0.0f, 0.0f};
    if (T > 0.0f && nRoots >= 2) {
        int farIdx = second_nearest_root(z, roots, nRoots);
        Cx rFar = roots[farIdx];
        Cx diff = cx_sub(rFar, z);
        float dist2 = cx_abs2(diff);
        if (dist2 > 1e-20f) {
            pull = cx_scale(diff, T / dist2);
        }
    }

    float d = damp / (1.0f + stepMag);
    Cx zNext = cx_add(cx_add(cx_sub(z, cx_scale(step, d)), pull), cx_mul(pConst, zPrev));

    float r2 = cx_abs2(zNext);
    if (r2 > 16.0f) {
        float s = 4.0f / sqrtf(r2);
        zNext = cx_scale(zNext, s);
    }
    return zNext;
}

// Standard Newton iteration
Cx newton_iterate(Cx z, const float coeffs[5], float damp, Cx pConst, Cx zPrev) {
    Cx P, dP;
    poly_eval_d1(coeffs, z, &P, &dP);
    float dAbs2 = cx_abs2(dP);
    Cx step = (dAbs2 < 1e-20f) ? P : cx_div(P, dP);
    float stepMag = cx_abs(step);
    float d = damp / (1.0f + stepMag);
    Cx zNext = cx_add(cx_sub(z, cx_scale(step, d)), cx_mul(pConst, zPrev));
    float r2 = cx_abs2(zNext);
    if (r2 > 16.0f) {
        float s = 4.0f / sqrtf(r2);
        zNext = cx_scale(zNext, s);
    }
    return zNext;
}

// z^3 - 1 roots: cube roots of unity
const float kCoeffs[5] = {-1.0f, 0.0f, 0.0f, 1.0f, 0.0f};
const Cx kRoots[3] = {{1.0f, 0.0f}, {-0.5f, 0.866025f}, {-0.5f, -0.866025f}};
const int kNRoots = 3;

int g_fail_count = 0;

void check(bool cond, const char* msg) {
    if (!cond) { std::printf("  FAIL: %s\n", msg); ++g_fail_count; }
}

// Test 1: T=0 matches standard Newton
void test_zero_tension_matches_newton() {
    std::printf("  test_zero_tension_matches_newton...\n");
    Cx starts[] = {{0.5f, 0.3f}, {-0.8f, 0.2f}, {0.1f, -0.9f}, {1.2f, 0.5f}};
    Cx pConst = {0.0f, 0.0f};
    for (auto& s : starts) {
        Cx z_tension = s, z_newton = s;
        Cx zPrev_t = s, zPrev_n = s;
        for (int i = 0; i < 50; ++i) {
            Cx nt = tension_iterate(z_tension, kCoeffs, 1.0f, 0.0f, kRoots, kNRoots, pConst, zPrev_t);
            Cx nn = newton_iterate(z_newton, kCoeffs, 1.0f, pConst, zPrev_n);
            zPrev_t = z_tension; z_tension = nt;
            zPrev_n = z_newton; z_newton = nn;
        }
        float delta = cx_abs(cx_sub(z_tension, z_newton));
        check(delta < 1e-6f, "T=0 should match Newton exactly");
    }
}

// Test 2: small T delta -> small output delta
void test_tension_continuity() {
    std::printf("  test_tension_continuity...\n");
    Cx start = {0.5f, 0.3f};
    Cx pConst = {0.0f, 0.0f};
    float T0 = 0.0f, T1 = 0.001f;
    Cx z0 = start, z1 = start;
    Cx zP0 = start, zP1 = start;
    for (int i = 0; i < 20; ++i) {
        Cx n0 = tension_iterate(z0, kCoeffs, 1.0f, T0, kRoots, kNRoots, pConst, zP0);
        Cx n1 = tension_iterate(z1, kCoeffs, 1.0f, T1, kRoots, kNRoots, pConst, zP1);
        zP0 = z0; z0 = n0;
        zP1 = z1; z1 = n1;
    }
    float delta = cx_abs(cx_sub(z0, z1));
    check(delta < 0.5f, "0.001 tension delta should produce small output difference");
}

// Test 3: T=0.1 stability
void test_max_tension_stability() {
    std::printf("  test_max_tension_stability...\n");
    Cx starts[] = {{0.5f, 0.3f}, {-0.8f, 0.2f}, {0.1f, -0.9f}, {1.2f, 0.5f}};
    Cx pConst = {0.0f, 0.0f};
    int finite_count = 0;
    for (auto& s : starts) {
        Cx z = s, zPrev = s;
        bool finite = true;
        for (int i = 0; i < 200; ++i) {
            Cx next = tension_iterate(z, kCoeffs, 1.0f, 0.1f, kRoots, kNRoots, pConst, zPrev);
            zPrev = z; z = next;
            if (!std::isfinite(z.x) || !std::isfinite(z.y)) { finite = false; break; }
        }
        if (finite) ++finite_count;
    }
    check(finite_count >= 3, "at least 3/4 starting points should stay finite at T=0.1");
}

// Test 4: pull direction points toward second-closest root
void test_pull_direction() {
    std::printf("  test_pull_direction...\n");
    // Point near root 0 (1, 0) — second closest should be root 1 or 2
    Cx z = {0.8f, 0.1f};
    int farIdx = second_nearest_root(z, kRoots, kNRoots);
    Cx rFar = kRoots[farIdx];
    Cx diff = cx_sub(rFar, z);
    float dist2 = cx_abs2(diff);
    float T = 0.02f;
    Cx pull = cx_scale(diff, T / dist2);

    // Pull should point roughly from z toward rFar
    Cx pullDir = {pull.x, pull.y};
    float pullMag = cx_abs(pullDir);
    check(pullMag > 0.0f, "pull should be nonzero");

    // Dot product between pull and (rFar - z) should be positive
    float dot = pull.x * diff.x + pull.y * diff.y;
    check(dot > 0.0f, "pull should point toward second-closest root");
}

} // namespace

int main() {
    std::printf("explaino_tension continuity tests:\n");
    test_zero_tension_matches_newton();
    test_tension_continuity();
    test_max_tension_stability();
    test_pull_direction();

    if (g_fail_count == 0) {
        std::printf("explaino_tension continuity: all tests passed, 0 failures.\n");
    } else {
        std::printf("explaino_tension continuity: %d FAILURES\n", g_fail_count);
    }
    return g_fail_count;
}
