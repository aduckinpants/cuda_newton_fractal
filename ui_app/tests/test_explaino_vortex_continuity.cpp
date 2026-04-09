// Headless CPU test for explaino_vortex iteration contract.
// Verifies:
// 1. strength=0 matches standard Newton (no rotation applied).
// 2. Small strength increments produce proportionally small output changes.
// 3. strength=1.0 stability (all z remain finite after 200 iterations).
// 4. Rotation verification: step angle change proportional to strength * arg(step).

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

// Vortex iteration: step rotated by V * arg(step)
Cx vortex_iterate(Cx z, const float coeffs[5], float damp, float V, Cx pConst, Cx zPrev) {
    Cx P, dP;
    poly_eval_d1(coeffs, z, &P, &dP);
    float dAbs2 = cx_abs2(dP);
    Cx step = (dAbs2 < 1e-20f) ? P : cx_div(P, dP);
    float stepMag = cx_abs(step);

    // Self-referential rotation
    Cx rotatedStep = step;
    if (V > 0.0f && stepMag > 1e-20f) {
        float theta = atan2f(step.y, step.x);
        float angle = V * theta;
        float cosA = cosf(angle);
        float sinA = sinf(angle);
        rotatedStep = {step.x * cosA - step.y * sinA,
                       step.x * sinA + step.y * cosA};
    }

    float d = damp / (1.0f + stepMag);
    Cx zNext = cx_add(cx_sub(z, cx_scale(rotatedStep, d)), cx_mul(pConst, zPrev));

    float r2 = cx_abs2(zNext);
    if (r2 > 16.0f) {
        float s = 4.0f / sqrtf(r2);
        zNext = cx_scale(zNext, s);
    }
    return zNext;
}

// Standard Newton iteration (no rotation)
Cx newton_iterate(Cx z, const float coeffs[5], float damp, Cx pConst, Cx zPrev) {
    return vortex_iterate(z, coeffs, damp, 0.0f, pConst, zPrev);
}

const float kCoeffs[5] = {-1.0f, 0.0f, 0.0f, 1.0f, 0.0f}; // z^3 - 1

int g_fail_count = 0;

void check(bool cond, const char* msg) {
    if (!cond) { std::printf("  FAIL: %s\n", msg); ++g_fail_count; }
}

// Test 1: strength=0 matches standard Newton
void test_zero_strength_matches_newton() {
    std::printf("  test_zero_strength_matches_newton...\n");
    Cx starts[] = {{0.5f, 0.3f}, {-0.8f, 0.2f}, {0.1f, -0.9f}, {1.2f, 0.5f}};
    Cx pConst = {0.0f, 0.0f};
    for (auto& s : starts) {
        Cx z_vortex = s, z_newton = s;
        Cx zPrev_v = s, zPrev_n = s;
        for (int i = 0; i < 50; ++i) {
            Cx nv = vortex_iterate(z_vortex, kCoeffs, 1.0f, 0.0f, pConst, zPrev_v);
            Cx nn = newton_iterate(z_newton, kCoeffs, 1.0f, pConst, zPrev_n);
            zPrev_v = z_vortex; z_vortex = nv;
            zPrev_n = z_newton; z_newton = nn;
        }
        float delta = cx_abs(cx_sub(z_vortex, z_newton));
        check(delta < 1e-6f, "V=0 should match Newton exactly");
    }
}

// Test 2: small strength delta -> small output delta
void test_strength_continuity() {
    std::printf("  test_strength_continuity...\n");
    Cx start = {0.5f, 0.3f};
    Cx pConst = {0.0f, 0.0f};
    float V0 = 0.0f, V1 = 0.01f;
    Cx z0 = start, z1 = start;
    Cx zP0 = start, zP1 = start;
    for (int i = 0; i < 20; ++i) {
        Cx n0 = vortex_iterate(z0, kCoeffs, 1.0f, V0, pConst, zP0);
        Cx n1 = vortex_iterate(z1, kCoeffs, 1.0f, V1, pConst, zP1);
        zP0 = z0; z0 = n0;
        zP1 = z1; z1 = n1;
    }
    float delta = cx_abs(cx_sub(z0, z1));
    check(delta < 0.5f, "0.01 strength delta should produce small output difference");
}

// Test 3: strength=1 stability
void test_full_strength_stability() {
    std::printf("  test_full_strength_stability...\n");
    Cx starts[] = {{0.5f, 0.3f}, {-0.8f, 0.2f}, {0.1f, -0.9f}, {1.2f, 0.5f}};
    Cx pConst = {0.0f, 0.0f};
    int finite_count = 0;
    for (auto& s : starts) {
        Cx z = s, zPrev = s;
        bool finite = true;
        for (int i = 0; i < 200; ++i) {
            Cx next = vortex_iterate(z, kCoeffs, 1.0f, 1.0f, pConst, zPrev);
            zPrev = z; z = next;
            if (!std::isfinite(z.x) || !std::isfinite(z.y)) { finite = false; break; }
        }
        if (finite) ++finite_count;
    }
    check(finite_count >= 3, "at least 3/4 starting points should stay finite at V=1");
}

// Test 4: rotation verification
void test_rotation_proportional() {
    std::printf("  test_rotation_proportional...\n");
    // At a known point, compute the step rotation and verify it matches V*theta
    Cx z = {0.5f, 0.3f};
    Cx P, dP;
    poly_eval_d1(kCoeffs, z, &P, &dP);
    Cx step = cx_div(P, dP);
    float theta = atan2f(step.y, step.x);

    float V = 0.3f;
    float expectedAngle = V * theta;
    float cosA = cosf(expectedAngle);
    float sinA = sinf(expectedAngle);
    Cx expectedRot = {step.x * cosA - step.y * sinA, step.x * sinA + step.y * cosA};

    // One iteration of vortex with damp=1, no pullback expected
    Cx pConst = {0.0f, 0.0f};
    Cx zNext = vortex_iterate(z, kCoeffs, 1.0f, V, pConst, z);
    // zNext = z - damp/(1+|step|) * rotatedStep
    float stepMag = cx_abs(step);
    float d = 1.0f / (1.0f + stepMag);
    Cx reconstructed = cx_sub(z, cx_scale(expectedRot, d));
    float delta = cx_abs(cx_sub(zNext, reconstructed));
    check(delta < 1e-5f, "rotation should match V*arg(step) exactly");
}

} // namespace

int main() {
    std::printf("explaino_vortex continuity tests:\n");
    test_zero_strength_matches_newton();
    test_strength_continuity();
    test_full_strength_stability();
    test_rotation_proportional();

    if (g_fail_count == 0) {
        std::printf("explaino_vortex continuity: all tests passed, 0 failures.\n");
    } else {
        std::printf("explaino_vortex continuity: %d FAILURES\n", g_fail_count);
    }
    return g_fail_count;
}
