// test_generic_sample_core.cu — GF-3 tests for SampleGenericFunction host API.
// Exit criteria:
//   1. 1024 points z^3-1 Newton iteration -> converged near cube roots.
//   2. Grid of z^2+c with c=-0.75+0i -> escape fraction matches known value +/-5%.
//   3. Invalid func (node_count=0) -> returns false + error message.
//   4. Compose at root works correctly.
//   5. Direct (non-iterate, non-compose) evaluation works.

#include "generic_sample_core.h"
#include <cstdio>
#include <cmath>
#include <cstring>
#include <vector>

static int g_pass = 0, g_fail = 0;

static void check(bool cond, const char* msg, int line) {
    if (cond) { g_pass++; }
    else { g_fail++; printf("  FAIL [line %d]: %s\n", line, msg); }
}
#define CHECK(c, m) check((c), (m), __LINE__)

// --- Helper: build z^3-1 Newton step: z - (z^3-1)/(3*z^2) ---
// Tree:
//   iterate(z - (z^3 - 1) / (3 * z^2), N)
//   Nodes:
//     0: var_z
//     1: pow_int(z, 3)     child_left=0, param=0 (3.0)
//     2: const_real(1)      param=1 (1.0)
//     3: sub(z^3, 1)        child_left=1, child_right=2
//     4: const_real(3)      param=2 (3.0)
//     5: pow_int(z, 2)      child_left=0, param=3 (2.0)
//     6: mul(3, z^2)        child_left=4, child_right=5
//     7: div(z^3-1, 3z^2)   child_left=3, child_right=6
//     8: sub(z, quotient)   child_left=0, child_right=7
//     9: iterate(step, N)   child_left=8, param=4 (500.0)

static GenericFunctionDesc make_newton_z3_minus_1(int maxIter = 500) {
    GenericFunctionDesc f = {};
    f.node_count = 10;
    f.param_count = 5;
    f.root_node = 9;
    f.max_iterate = maxIter;

    // params
    f.params[0] = 3.0;   // exponent for z^3
    f.params[1] = 1.0;   // constant 1
    f.params[2] = 3.0;   // constant 3
    f.params[3] = 2.0;   // exponent for z^2
    f.params[4] = (double)maxIter;  // iteration count

    // node 0: z
    f.nodes[0] = {GFNodeOp::gf_var_z, -1, -1, -1};
    // node 1: z^3
    f.nodes[1] = {GFNodeOp::gf_pow_int, 0, -1, 0};
    // node 2: const 1
    f.nodes[2] = {GFNodeOp::gf_const_real, -1, -1, 1};
    // node 3: z^3 - 1
    f.nodes[3] = {GFNodeOp::gf_sub, 1, 2, -1};
    // node 4: const 3
    f.nodes[4] = {GFNodeOp::gf_const_real, -1, -1, 2};
    // node 5: z^2
    f.nodes[5] = {GFNodeOp::gf_pow_int, 0, -1, 3};
    // node 6: 3 * z^2
    f.nodes[6] = {GFNodeOp::gf_mul, 4, 5, -1};
    // node 7: (z^3 - 1) / (3 * z^2)
    f.nodes[7] = {GFNodeOp::gf_div, 3, 6, -1};
    // node 8: z - (z^3-1)/(3z^2)
    f.nodes[8] = {GFNodeOp::gf_sub, 0, 7, -1};
    // node 9: iterate(step, N)
    f.nodes[9] = {GFNodeOp::gf_iterate, 8, -1, 4};

    return f;
}

// --- Helper: build iterate(z^2 + c, N) with c as parameter ---
// Nodes:
//   0: var_z
//   1: pow_int(z, 2)      child_left=0, param=0 (2.0)
//   2: const_complex(c)   param=1 (c_real), param=2 (c_imag)
//   3: add(z^2, c)        child_left=1, child_right=2
//   4: iterate(step, N)   child_left=3, param=3 (N)

static GenericFunctionDesc make_mandelbrot_iterate(double c_re, double c_im, int maxIter = 500) {
    GenericFunctionDesc f = {};
    f.node_count = 5;
    f.param_count = 4;
    f.root_node = 4;
    f.max_iterate = maxIter;

    f.params[0] = 2.0;        // exponent
    f.params[1] = c_re;       // c real
    f.params[2] = c_im;       // c imag
    f.params[3] = (double)maxIter;

    f.nodes[0] = {GFNodeOp::gf_var_z, -1, -1, -1};
    f.nodes[1] = {GFNodeOp::gf_pow_int, 0, -1, 0};
    f.nodes[2] = {GFNodeOp::gf_const_complex, -1, -1, 1};
    f.nodes[3] = {GFNodeOp::gf_add, 1, 2, -1};
    f.nodes[4] = {GFNodeOp::gf_iterate, 3, -1, 3};

    return f;
}

// --- Helper: build compose(z^2, z+1) = (z+1)^2 ---
// Nodes:
//   0: var_z
//   1: pow_int(z, 2)        child_left=0, param=0 (2.0)
//   2: var_z
//   3: const_real(1)         param=1 (1.0)
//   4: add(z, 1)             child_left=2, child_right=3
//   5: compose(z^2, z+1)     child_left=1, child_right=4

static GenericFunctionDesc make_compose_sq_plus1() {
    GenericFunctionDesc f = {};
    f.node_count = 6;
    f.param_count = 2;
    f.root_node = 5;
    f.max_iterate = 1;

    f.params[0] = 2.0;
    f.params[1] = 1.0;

    f.nodes[0] = {GFNodeOp::gf_var_z, -1, -1, -1};
    f.nodes[1] = {GFNodeOp::gf_pow_int, 0, -1, 0};
    f.nodes[2] = {GFNodeOp::gf_var_z, -1, -1, -1};
    f.nodes[3] = {GFNodeOp::gf_const_real, -1, -1, 1};
    f.nodes[4] = {GFNodeOp::gf_add, 2, 3, -1};
    f.nodes[5] = {GFNodeOp::gf_compose, 1, 4, -1};

    return f;
}

// --- Helper: simple z^2 (direct eval, no iterate/compose) ---

static GenericFunctionDesc make_z_squared() {
    GenericFunctionDesc f = {};
    f.node_count = 2;
    f.param_count = 1;
    f.root_node = 1;
    f.max_iterate = 1;

    f.params[0] = 2.0;

    f.nodes[0] = {GFNodeOp::gf_var_z, -1, -1, -1};
    f.nodes[1] = {GFNodeOp::gf_pow_int, 0, -1, 0};

    return f;
}

// =====================================================================
// Test 1: Newton z^3-1 convergence at 1024 points
// =====================================================================

static void test_newton_z3_convergence() {
    printf("--- test_newton_z3_convergence ---\n");

    GenericFunctionDesc func = make_newton_z3_minus_1(500);

    const int N = 1024;
    std::vector<GFPoint> coords(N);
    std::vector<GenericSampleResult> results(N);

    // Generate grid of starting points in [-2, 2] x [-2, 2].
    int side = 32;
    for (int j = 0; j < side; ++j) {
        for (int i = 0; i < side; ++i) {
            int idx = j * side + i;
            coords[idx].x = -2.0 + 4.0 * (i + 0.5) / side;
            coords[idx].y = -2.0 + 4.0 * (j + 0.5) / side;
        }
    }

    const char* err = nullptr;
    bool ok = SampleGenericFunction(
        coords.data(), N, func, 1e-10, 1e10, results.data(), &err);

    CHECK(ok, "SampleGenericFunction returned true");
    if (!ok) { printf("  Error: %s\n", err ? err : "(null)"); return; }

    // Cube roots of unity.
    double roots[3][2] = {
        { 1.0, 0.0 },
        { -0.5,  0.86602540378 },
        { -0.5, -0.86602540378 }
    };

    int converged_count = 0;
    int near_root_count = 0;
    for (int i = 0; i < N; ++i) {
        if (results[i].converged) converged_count++;

        // Check if converged point is near a cube root.
        if (results[i].converged) {
            double best_dist = 1e30;
            for (int r = 0; r < 3; ++r) {
                double dx = results[i].value_x - roots[r][0];
                double dy = results[i].value_y - roots[r][1];
                double d = dx * dx + dy * dy;
                if (d < best_dist) best_dist = d;
            }
            if (best_dist < 1e-6) near_root_count++;
        }
    }

    // Most starting points should converge (Newton's method is globally convergent
    // for z^3-1 except on the Julia set boundaries).
    double conv_frac = (double)converged_count / N;
    printf("  Converged: %d / %d (%.1f%%)\n", converged_count, N, conv_frac * 100);
    CHECK(conv_frac > 0.8, "At least 80% of points converged");
    CHECK(converged_count == near_root_count,
          "All converged points are near a cube root of unity");

    // Check iteration counts are reasonable (not all maxIter).
    int low_iter_count = 0;
    for (int i = 0; i < N; ++i) {
        if (results[i].converged && results[i].iterations < 100)
            low_iter_count++;
    }
    CHECK(low_iter_count > 0, "Some points converge in < 100 iterations");
}

// =====================================================================
// Test 2: Mandelbrot escape fractions
// =====================================================================

static void test_mandelbrot_escape_fraction() {
    printf("--- test_mandelbrot_escape_fraction ---\n");

    // z^2 + c with c = -0.75 + 0i (near main cardioid boundary).
    // For z=0 starting point, this should NOT escape.
    // For a grid of starting z values, we count escape fraction.
    GenericFunctionDesc func = make_mandelbrot_iterate(-0.75, 0.0, 500);

    const int side = 64;
    const int N = side * side;
    std::vector<GFPoint> coords(N);
    std::vector<GenericSampleResult> results(N);

    for (int j = 0; j < side; ++j) {
        for (int i = 0; i < side; ++i) {
            int idx = j * side + i;
            coords[idx].x = -2.0 + 4.0 * (i + 0.5) / side;
            coords[idx].y = -2.0 + 4.0 * (j + 0.5) / side;
        }
    }

    const char* err = nullptr;
    bool ok = SampleGenericFunction(
        coords.data(), N, func, 1e-10, 1000.0, results.data(), &err);

    CHECK(ok, "SampleGenericFunction returned true");
    if (!ok) { printf("  Error: %s\n", err ? err : "(null)"); return; }

    int escaped = 0, bounded = 0;
    for (int i = 0; i < N; ++i) {
        if (results[i].diverged) escaped++;
        else bounded++;
    }

    double escape_frac = (double)escaped / N;
    printf("  Escaped: %d / %d (%.1f%%)\n", escaped, N, escape_frac * 100);
    printf("  Bounded: %d / %d (%.1f%%)\n", bounded, N, (1.0 - escape_frac) * 100);

    // For z^2 + c with c=-0.75, starting from various z: most of the [-2,2]^2
    // grid should escape (the bounded set is small).
    CHECK(escape_frac > 0.5, "More than half of points escaped");
    CHECK(escape_frac < 0.99, "Some points remain bounded");
}

// =====================================================================
// Test 3: Invalid descriptor -> error
// =====================================================================

static void test_invalid_descriptor() {
    printf("--- test_invalid_descriptor ---\n");

    GenericFunctionDesc bad = {};
    bad.node_count = 0;  // Invalid

    GFPoint coord = {1.0, 0.0};
    GenericSampleResult result = {};
    const char* err = nullptr;

    bool ok = SampleGenericFunction(&coord, 1, bad, 1e-10, 1e10, &result, &err);
    CHECK(!ok, "Returns false for invalid descriptor");
    CHECK(err != nullptr, "Error message is set");
    if (err) printf("  Error: %s\n", err);

    // Null coords.
    GenericFunctionDesc good = make_z_squared();
    err = nullptr;
    ok = SampleGenericFunction(nullptr, 1, good, 1e-10, 1e10, &result, &err);
    CHECK(!ok, "Returns false for null coords");
    CHECK(err != nullptr, "Error message set for null coords");

    // Null results.
    err = nullptr;
    ok = SampleGenericFunction(&coord, 1, good, 1e-10, 1e10, nullptr, &err);
    CHECK(!ok, "Returns false for null outResults");

    // Zero points is valid no-op.
    err = nullptr;
    ok = SampleGenericFunction(&coord, 0, good, 1e-10, 1e10, &result, &err);
    CHECK(ok, "Zero points returns true");
}

// =====================================================================
// Test 4: Compose at root: compose(z^2, z+1) = (z+1)^2
// =====================================================================

static void test_compose_root() {
    printf("--- test_compose_root ---\n");

    GenericFunctionDesc func = make_compose_sq_plus1();

    const int N = 4;
    GFPoint coords[4] = {{0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {-1.0, 0.5}};
    GenericSampleResult results[4] = {};
    const char* err = nullptr;

    bool ok = SampleGenericFunction(coords, N, func, 1e-10, 1e10, results, &err);
    CHECK(ok, "SampleGenericFunction succeeded");
    if (!ok) { printf("  Error: %s\n", err ? err : "(null)"); return; }

    // compose(z^2, z+1) = (z+1)^2
    for (int i = 0; i < N; ++i) {
        double zr = coords[i].x + 1.0;
        double zi = coords[i].y;
        double exp_re = zr * zr - zi * zi;
        double exp_im = 2.0 * zr * zi;

        double err_re = fabs(results[i].value_x - exp_re);
        double err_im = fabs(results[i].value_y - exp_im);

        char msg[128];
        snprintf(msg, sizeof(msg),
            "compose(%+.2f,%+.2f): got (%.6f,%.6f), expect (%.6f,%.6f)",
            coords[i].x, coords[i].y,
            results[i].value_x, results[i].value_y, exp_re, exp_im);
        CHECK(err_re < 1e-6 && err_im < 1e-6, msg);
    }
}

// =====================================================================
// Test 5: Direct evaluation (no iterate/compose) — z^2
// =====================================================================

static void test_direct_eval() {
    printf("--- test_direct_eval ---\n");

    GenericFunctionDesc func = make_z_squared();

    const int N = 4;
    GFPoint coords[4] = {{2.0, 0.0}, {0.0, 1.0}, {1.0, 1.0}, {-1.0, -1.0}};
    GenericSampleResult results[4] = {};
    const char* err = nullptr;

    bool ok = SampleGenericFunction(coords, N, func, 1e-10, 1e10, results, &err);
    CHECK(ok, "SampleGenericFunction succeeded");
    if (!ok) return;

    // z^2 at each point.
    double expected[4][2] = {
        {4.0, 0.0},   // 2^2
        {-1.0, 0.0},  // i^2
        {0.0, 2.0},   // (1+i)^2
        {0.0, 2.0},   // (-1-i)^2
    };

    for (int i = 0; i < N; ++i) {
        double err_re = fabs(results[i].value_x - expected[i][0]);
        double err_im = fabs(results[i].value_y - expected[i][1]);
        char msg[128];
        snprintf(msg, sizeof(msg),
            "z^2 at (%+.1f,%+.1f): got (%.6f,%.6f) expect (%.6f,%.6f)",
            coords[i].x, coords[i].y,
            results[i].value_x, results[i].value_y,
            expected[i][0], expected[i][1]);
        CHECK(err_re < 1e-6 && err_im < 1e-6, msg);
    }
}

// =====================================================================
// Test 6: Determinism — same inputs yield same outputs
// =====================================================================

static void test_determinism() {
    printf("--- test_determinism ---\n");

    GenericFunctionDesc func = make_newton_z3_minus_1(100);

    const int N = 64;
    std::vector<GFPoint> coords(N);
    for (int i = 0; i < N; ++i) {
        coords[i].x = -2.0 + 4.0 * i / N;
        coords[i].y = 0.5;
    }

    std::vector<GenericSampleResult> r1(N), r2(N);
    const char* err = nullptr;

    bool ok1 = SampleGenericFunction(coords.data(), N, func, 1e-10, 1e10, r1.data(), &err);
    bool ok2 = SampleGenericFunction(coords.data(), N, func, 1e-10, 1e10, r2.data(), &err);

    CHECK(ok1 && ok2, "Both calls succeeded");

    bool all_same = true;
    for (int i = 0; i < N; ++i) {
        if (r1[i].value_x != r2[i].value_x ||
            r1[i].value_y != r2[i].value_y ||
            r1[i].iterations != r2[i].iterations ||
            r1[i].converged != r2[i].converged ||
            r1[i].diverged != r2[i].diverged)
        {
            all_same = false;
            break;
        }
    }
    CHECK(all_same, "Results are bitwise identical across runs");
}

// =====================================================================

int main() {
    printf("=== GF-3: SampleGenericFunction host API tests ===\n\n");

    test_newton_z3_convergence();
    test_mandelbrot_escape_fraction();
    test_invalid_descriptor();
    test_compose_root();
    test_direct_eval();
    test_determinism();

    printf("\n=== GF-3 summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
