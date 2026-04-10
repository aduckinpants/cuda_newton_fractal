#include "../src/generic_function_types.h"
#include "../src/generic_function_cpu_eval.h"
#include <cstdio>
#include <cmath>

static int failures = 0;

static void check_close(const char* label, double actual, double expected, double tol = 1e-10) {
    double err = std::fabs(actual - expected);
    if (err > tol) {
        printf("*** FAIL: %s: expected %.12g, got %.12g (err=%.4e)\n", label, expected, actual, err);
        ++failures;
    }
}

static void check_complex(const char* label, GFCpuComplex actual, double ex, double ey, double tol = 1e-10) {
    double err = std::sqrt((actual.x - ex)*(actual.x - ex) + (actual.y - ey)*(actual.y - ey));
    if (err > tol) {
        printf("*** FAIL: %s: expected (%.12g, %.12g), got (%.12g, %.12g) (err=%.4e)\n",
               label, ex, ey, actual.x, actual.y, err);
        ++failures;
    }
}

// --- Descriptor builder helpers ---

static GenericFunctionDesc make_empty() {
    GenericFunctionDesc d = {};
    d.node_count = 0;
    d.param_count = 0;
    d.root_node = 0;
    d.max_iterate = 1;
    return d;
}

static int add_node(GenericFunctionDesc& d, GFNodeOp op, int left = -1, int right = -1, int param = -1) {
    int idx = d.node_count++;
    d.nodes[idx] = {op, left, right, param};
    return idx;
}

static int add_param(GenericFunctionDesc& d, double val) {
    int idx = d.param_count++;
    d.params[idx] = val;
    return idx;
}

// =========================================================================
// Test: validation
// =========================================================================
static void test_validation() {
    printf("--- Validation tests ---\n");

    // Empty descriptor is invalid
    GenericFunctionDesc d = {};
    d.node_count = 0;
    auto r = ValidateGenericFunctionDesc(d);
    if (r.valid) { printf("*** FAIL: empty desc should be invalid\n"); ++failures; }

    // Single var_z node is valid
    d = make_empty();
    add_node(d, GFNodeOp::gf_var_z);
    r = ValidateGenericFunctionDesc(d);
    if (!r.valid) { printf("*** FAIL: single z node should be valid: %s\n", r.error); ++failures; }

    // Self-referencing node
    d = make_empty();
    d.nodes[0] = {GFNodeOp::gf_add, 0, 0, -1};
    d.node_count = 1;
    r = ValidateGenericFunctionDesc(d);
    if (r.valid) { printf("*** FAIL: self-referencing should be invalid\n"); ++failures; }

    // Out-of-range child
    d = make_empty();
    d.nodes[0] = {GFNodeOp::gf_add, 5, -1, -1};
    d.node_count = 1;
    r = ValidateGenericFunctionDesc(d);
    if (r.valid) { printf("*** FAIL: out-of-range child should be invalid\n"); ++failures; }

    // Missing param for const_real
    d = make_empty();
    d.nodes[0] = {GFNodeOp::gf_const_real, -1, -1, 0};
    d.node_count = 1;
    d.param_count = 0;
    r = ValidateGenericFunctionDesc(d);
    if (r.valid) { printf("*** FAIL: const_real with no params should be invalid\n"); ++failures; }

    printf("  validation: %d failures so far\n", failures);
}

// =========================================================================
// Test: z^3 - 1
// Root of z^3-1 at z=1 is exactly (1,0)->(1,0)-1 = (0,0).
// At z = (0.5, 0.866...) = e^{i*2pi/3}, z^3 = 1, so z^3-1 = 0.
// =========================================================================
static void test_z3_minus_1() {
    printf("--- z^3 - 1 ---\n");

    // Tree: sub(pow_int(z, 3), const_real(1))
    //   node 0: gf_var_z
    //   node 1: gf_pow_int(child_left=0, param=exponent_idx)  where params[0]=3
    //   node 2: gf_const_real(param=1_idx) where params[1]=1.0
    //   node 3: gf_sub(child_left=1, child_right=2)
    GenericFunctionDesc d = make_empty();
    int n_z = add_node(d, GFNodeOp::gf_var_z);
    int p_exp = add_param(d, 3.0);
    int n_pow = add_node(d, GFNodeOp::gf_pow_int, n_z, -1, p_exp);
    int p_one = add_param(d, 1.0);
    int n_one = add_node(d, GFNodeOp::gf_const_real, -1, -1, p_one);
    int n_root = add_node(d, GFNodeOp::gf_sub, n_pow, n_one);
    d.root_node = n_root;

    auto r = ValidateGenericFunctionDesc(d);
    if (!r.valid) { printf("*** FAIL: z^3-1 desc invalid: %s\n", r.error); ++failures; return; }

    // z = 1: (1)^3 - 1 = 0
    auto v1 = gf_cpu_eval(d, {1.0, 0.0});
    check_complex("z^3-1 at z=1", v1, 0.0, 0.0);

    // z = i: i^3 = -i, -i - 1 = (-1, -1)
    auto v2 = gf_cpu_eval(d, {0.0, 1.0});
    check_complex("z^3-1 at z=i", v2, -1.0, -1.0);

    // z = 2: 8 - 1 = 7
    auto v3 = gf_cpu_eval(d, {2.0, 0.0});
    check_complex("z^3-1 at z=2", v3, 7.0, 0.0);

    // cube root of unity: z = e^{i*2pi/3} = (-0.5, sqrt(3)/2)
    double s3h = std::sqrt(3.0) / 2.0;
    auto v4 = gf_cpu_eval(d, {-0.5, s3h});
    check_complex("z^3-1 at omega", v4, 0.0, 0.0, 1e-9);

    printf("  z^3-1: %d failures so far\n", failures);
}

// =========================================================================
// Test: sin(z) * z
// =========================================================================
static void test_sin_z_times_z() {
    printf("--- sin(z) * z ---\n");

    // Tree: mul(sin(z), z)
    //   node 0: gf_var_z
    //   node 1: gf_sin(child_left=0)
    //   node 2: gf_var_z
    //   node 3: gf_mul(child_left=1, child_right=2)
    GenericFunctionDesc d = make_empty();
    int n_z1 = add_node(d, GFNodeOp::gf_var_z);
    int n_sin = add_node(d, GFNodeOp::gf_sin, n_z1);
    int n_z2 = add_node(d, GFNodeOp::gf_var_z);
    int n_root = add_node(d, GFNodeOp::gf_mul, n_sin, n_z2);
    d.root_node = n_root;

    auto r = ValidateGenericFunctionDesc(d);
    if (!r.valid) { printf("*** FAIL: sin(z)*z desc invalid: %s\n", r.error); ++failures; return; }

    // z = 0: sin(0)*0 = 0
    auto v1 = gf_cpu_eval(d, {0.0, 0.0});
    check_complex("sin(z)*z at z=0", v1, 0.0, 0.0);

    // z = pi/2: sin(pi/2)*(pi/2) = 1 * pi/2
    double pi2 = 3.14159265358979323846 / 2.0;
    auto v2 = gf_cpu_eval(d, {pi2, 0.0});
    check_complex("sin(z)*z at z=pi/2", v2, pi2, 0.0, 1e-9);

    // z = pi: sin(pi)*pi ~ 0
    double pi = 3.14159265358979323846;
    auto v3 = gf_cpu_eval(d, {pi, 0.0});
    check_complex("sin(z)*z at z=pi", v3, 0.0, 0.0, 1e-9);

    // z = (1, 1): sin(1+i)*(1+i)
    // sin(1+i) = sin(1)cosh(1) + i*cos(1)sinh(1) = 1.29846...+ i*0.63496...
    double sinRe = std::sin(1.0) * std::cosh(1.0);
    double sinIm = std::cos(1.0) * std::sinh(1.0);
    // Multiply by (1+i): (a+bi)(1+i) = (a-b) + (a+b)i
    double expectRe = sinRe - sinIm;
    double expectIm = sinRe + sinIm;
    auto v4 = gf_cpu_eval(d, {1.0, 1.0});
    check_complex("sin(z)*z at z=1+i", v4, expectRe, expectIm, 1e-9);

    printf("  sin(z)*z: %d failures so far\n", failures);
}

// =========================================================================
// Test: z^2 + c (Mandelbrot-style quadratic with constant c)
// =========================================================================
static void test_z2_plus_c() {
    printf("--- z^2 + c ---\n");

    // Tree: add(pow_int(z, 2), const_complex(c_re, c_im))
    //   node 0: gf_var_z
    //   node 1: gf_pow_int(child_left=0, param=0) where params[0]=2
    //   node 2: gf_const_complex(param=1) where params[1]=c_re, params[2]=c_im
    //   node 3: gf_add(child_left=1, child_right=2)
    GenericFunctionDesc d = make_empty();
    int n_z = add_node(d, GFNodeOp::gf_var_z);
    int p_exp = add_param(d, 2.0);
    int n_pow = add_node(d, GFNodeOp::gf_pow_int, n_z, -1, p_exp);
    int p_cre = add_param(d, -0.75);
    int p_cim = add_param(d, 0.1);
    int n_c = add_node(d, GFNodeOp::gf_const_complex, -1, -1, p_cre);
    int n_root = add_node(d, GFNodeOp::gf_add, n_pow, n_c);
    d.root_node = n_root;

    auto r = ValidateGenericFunctionDesc(d);
    if (!r.valid) { printf("*** FAIL: z^2+c desc invalid: %s\n", r.error); ++failures; return; }

    // z = 0: 0 + (-0.75 + 0.1i) = (-0.75, 0.1)
    auto v1 = gf_cpu_eval(d, {0.0, 0.0});
    check_complex("z^2+c at z=0", v1, -0.75, 0.1);

    // z = 1: 1 + (-0.75 + 0.1i) = (0.25, 0.1)
    auto v2 = gf_cpu_eval(d, {1.0, 0.0});
    check_complex("z^2+c at z=1", v2, 0.25, 0.1);

    // z = (1,1): (1+i)^2 = 2i, + c = (-0.75, 2.1)
    auto v3 = gf_cpu_eval(d, {1.0, 1.0});
    check_complex("z^2+c at z=1+i", v3, -0.75, 2.1);

    printf("  z^2+c: %d failures so far\n", failures);
}

// =========================================================================
// Test: depth-3 composition: exp(sin(z^2))
// =========================================================================
static void test_depth3_composition() {
    printf("--- exp(sin(z^2)) depth-3 composition ---\n");

    // Using compose nodes:
    //   node 0: gf_var_z
    //   node 1: gf_pow_int(child_left=0, param=0) where params[0]=2   -> z^2
    //   node 2: gf_var_z  (placeholder z for sin subtree)
    //   node 3: gf_sin(child_left=2)                                   -> sin(z)
    //   node 4: gf_compose(child_left=3, child_right=1)               -> sin(z^2)
    //   node 5: gf_var_z  (placeholder z for exp subtree)
    //   node 6: gf_exp(child_left=5)                                   -> exp(z)
    //   node 7: gf_compose(child_left=6, child_right=4)               -> exp(sin(z^2))
    GenericFunctionDesc d = make_empty();
    int n_z0 = add_node(d, GFNodeOp::gf_var_z);
    int p_two = add_param(d, 2.0);
    int n_z2  = add_node(d, GFNodeOp::gf_pow_int, n_z0, -1, p_two);

    int n_z1 = add_node(d, GFNodeOp::gf_var_z);
    int n_sin = add_node(d, GFNodeOp::gf_sin, n_z1);
    int n_sin_z2 = add_node(d, GFNodeOp::gf_compose, n_sin, n_z2);

    int n_z3 = add_node(d, GFNodeOp::gf_var_z);
    int n_exp = add_node(d, GFNodeOp::gf_exp, n_z3);
    int n_root = add_node(d, GFNodeOp::gf_compose, n_exp, n_sin_z2);
    d.root_node = n_root;

    auto r = ValidateGenericFunctionDesc(d);
    if (!r.valid) { printf("*** FAIL: depth-3 compose desc invalid: %s\n", r.error); ++failures; return; }

    // z = 0: exp(sin(0)) = exp(0) = 1
    auto v1 = gf_cpu_eval(d, {0.0, 0.0});
    check_complex("exp(sin(z^2)) at z=0", v1, 1.0, 0.0, 1e-9);

    // z = 1: exp(sin(1)) = exp(0.84147...) = 2.3197...
    double expected = std::exp(std::sin(1.0));
    auto v2 = gf_cpu_eval(d, {1.0, 0.0});
    check_complex("exp(sin(z^2)) at z=1", v2, expected, 0.0, 1e-9);

    // z = sqrt(pi/2): z^2 = pi/2, sin(pi/2) = 1, exp(1) = e
    double sqrtPi2 = std::sqrt(3.14159265358979323846 / 2.0);
    auto v3 = gf_cpu_eval(d, {sqrtPi2, 0.0});
    check_complex("exp(sin(z^2)) at z=sqrt(pi/2)", v3, std::exp(1.0), 0.0, 1e-8);

    printf("  depth-3 compose: %d failures so far\n", failures);
}

// =========================================================================
// Test: gf_iterate (apply z^2+c three times from z=0)
// Mandelbrot orbit: z0=0, z1=c, z2=c^2+c, z3=(c^2+c)^2+c
// =========================================================================
static void test_iterate() {
    printf("--- iterate: 3x (z^2 + c) from z=0 ---\n");

    // Inner function: z^2 + c  where c is a fixed complex constant
    // We'll use c = (0.25, 0.0)
    //   node 0: gf_var_z
    //   node 1: gf_pow_int(child_left=0, param=0) where params[0]=2
    //   node 2: gf_const_complex(param=1) where params[1]=0.25, params[2]=0.0
    //   node 3: gf_add(child_left=1, child_right=2)
    //   node 4: gf_iterate(child_left=3, param=3) where params[3]=3
    GenericFunctionDesc d = make_empty();
    int n_z = add_node(d, GFNodeOp::gf_var_z);
    int p_exp = add_param(d, 2.0);
    int n_z2 = add_node(d, GFNodeOp::gf_pow_int, n_z, -1, p_exp);
    int p_cre = add_param(d, 0.25);
    int p_cim = add_param(d, 0.0);
    int n_c = add_node(d, GFNodeOp::gf_const_complex, -1, -1, p_cre);
    int n_step = add_node(d, GFNodeOp::gf_add, n_z2, n_c);
    int p_N = add_param(d, 3.0);
    int n_root = add_node(d, GFNodeOp::gf_iterate, n_step, -1, p_N);
    d.root_node = n_root;

    auto r = ValidateGenericFunctionDesc(d);
    if (!r.valid) { printf("*** FAIL: iterate desc invalid: %s\n", r.error); ++failures; return; }

    // Starting from z=0, iterate z^2 + 0.25 three times:
    // z0 = 0
    // z1 = 0^2 + 0.25 = 0.25
    // z2 = 0.25^2 + 0.25 = 0.3125
    // z3 = 0.3125^2 + 0.25 = 0.34765625
    auto v = gf_cpu_eval(d, {0.0, 0.0});
    check_complex("iterate 3x z^2+0.25 from z=0", v, 0.34765625, 0.0, 1e-12);

    // From z=1: z1=1.25, z2=1.8125, z3=3.535...
    // z1 = 1 + 0.25 = 1.25
    // z2 = 1.5625 + 0.25 = 1.8125
    // z3 = 3.28515625 + 0.25 = 3.53515625
    auto v2 = gf_cpu_eval(d, {1.0, 0.0});
    check_complex("iterate 3x z^2+0.25 from z=1", v2, 3.53515625, 0.0, 1e-12);

    printf("  iterate: %d failures so far\n", failures);
}

// =========================================================================
// Test: forward-difference derivative for z^2
// f(z) = z^2, f'(z) = 2z
// =========================================================================
static void test_derivative() {
    printf("--- forward-difference derivative ---\n");

    GenericFunctionDesc d = make_empty();
    int n_z = add_node(d, GFNodeOp::gf_var_z);
    int p_exp = add_param(d, 2.0);
    int n_root = add_node(d, GFNodeOp::gf_pow_int, n_z, -1, p_exp);
    d.root_node = n_root;

    // f'(z) = 2z. At z=(3,0), f'=6.
    auto dv1 = gf_cpu_derivative(d, {3.0, 0.0});
    check_complex("d/dz(z^2) at z=3", dv1, 6.0, 0.0, 1e-4);

    // At z=(1,1), f'(z)=2(1+i)=(2,2)
    auto dv2 = gf_cpu_derivative(d, {1.0, 1.0});
    check_complex("d/dz(z^2) at z=1+i", dv2, 2.0, 2.0, 1e-4);

    printf("  derivative: %d failures so far\n", failures);
}

// =========================================================================
// Test: remaining unary/binary ops
// =========================================================================
static void test_misc_ops() {
    printf("--- misc ops ---\n");

    // neg: -(1+2i) = (-1,-2)
    {
        GenericFunctionDesc d = make_empty();
        int n_z = add_node(d, GFNodeOp::gf_var_z);
        int n_root = add_node(d, GFNodeOp::gf_neg, n_z);
        d.root_node = n_root;
        auto v = gf_cpu_eval(d, {1.0, 2.0});
        check_complex("neg(1+2i)", v, -1.0, -2.0);
    }

    // conj: conj(1+2i) = (1,-2)
    {
        GenericFunctionDesc d = make_empty();
        int n_z = add_node(d, GFNodeOp::gf_var_z);
        int n_root = add_node(d, GFNodeOp::gf_conj, n_z);
        d.root_node = n_root;
        auto v = gf_cpu_eval(d, {1.0, 2.0});
        check_complex("conj(1+2i)", v, 1.0, -2.0);
    }

    // abs2: |3+4i|^2 = 25
    {
        GenericFunctionDesc d = make_empty();
        int n_z = add_node(d, GFNodeOp::gf_var_z);
        int n_root = add_node(d, GFNodeOp::gf_abs2, n_z);
        d.root_node = n_root;
        auto v = gf_cpu_eval(d, {3.0, 4.0});
        check_complex("|3+4i|^2", v, 25.0, 0.0);
    }

    // abs: |3+4i| = 5
    {
        GenericFunctionDesc d = make_empty();
        int n_z = add_node(d, GFNodeOp::gf_var_z);
        int n_root = add_node(d, GFNodeOp::gf_abs, n_z);
        d.root_node = n_root;
        auto v = gf_cpu_eval(d, {3.0, 4.0});
        check_complex("|3+4i|", v, 5.0, 0.0);
    }

    // div: (1+0i)/(0+1i) = (0,-1)
    {
        GenericFunctionDesc d = make_empty();
        int p_re = add_param(d, 1.0);
        int p_im = add_param(d, 0.0);
        int n_one = add_node(d, GFNodeOp::gf_const_complex, -1, -1, p_re);
        int n_z = add_node(d, GFNodeOp::gf_var_z);
        int n_root = add_node(d, GFNodeOp::gf_div, n_one, n_z);
        d.root_node = n_root;
        auto v = gf_cpu_eval(d, {0.0, 1.0});
        check_complex("1/i", v, 0.0, -1.0);
    }

    // cos: cos(0) = 1
    {
        GenericFunctionDesc d = make_empty();
        int n_z = add_node(d, GFNodeOp::gf_var_z);
        int n_root = add_node(d, GFNodeOp::gf_cos, n_z);
        d.root_node = n_root;
        auto v = gf_cpu_eval(d, {0.0, 0.0});
        check_complex("cos(0)", v, 1.0, 0.0);
    }

    // exp: exp(0) = 1
    {
        GenericFunctionDesc d = make_empty();
        int n_z = add_node(d, GFNodeOp::gf_var_z);
        int n_root = add_node(d, GFNodeOp::gf_exp, n_z);
        d.root_node = n_root;
        auto v = gf_cpu_eval(d, {0.0, 0.0});
        check_complex("exp(0)", v, 1.0, 0.0);
    }

    // log: log(e) = (1, 0)
    {
        GenericFunctionDesc d = make_empty();
        int n_z = add_node(d, GFNodeOp::gf_var_z);
        int n_root = add_node(d, GFNodeOp::gf_log, n_z);
        d.root_node = n_root;
        auto v = gf_cpu_eval(d, {std::exp(1.0), 0.0});
        check_complex("log(e)", v, 1.0, 0.0, 1e-9);
    }

    // pow_real: z^0.5 at z=4 = 2
    {
        GenericFunctionDesc d = make_empty();
        int n_z = add_node(d, GFNodeOp::gf_var_z);
        int p_half = add_param(d, 0.5);
        int n_root = add_node(d, GFNodeOp::gf_pow_real, n_z, -1, p_half);
        d.root_node = n_root;
        auto v = gf_cpu_eval(d, {4.0, 0.0});
        check_complex("4^0.5", v, 2.0, 0.0, 1e-9);
    }

    // var_z_conj: conj(3+4i) via leaf = (3,-4)
    {
        GenericFunctionDesc d = make_empty();
        int n_root = add_node(d, GFNodeOp::gf_var_z_conj);
        d.root_node = n_root;
        auto v = gf_cpu_eval(d, {3.0, 4.0});
        check_complex("z_conj(3+4i)", v, 3.0, -4.0);
    }

    printf("  misc ops: %d failures so far\n", failures);
}

// =========================================================================
// Test: GenericSampleResult via gf_cpu_sample
// =========================================================================
static void test_sample_result() {
    printf("--- sample result ---\n");

    // z^2 at z=(1e-6, 0): value ~ (1e-12, 0), converged=true, diverged=false
    GenericFunctionDesc d = make_empty();
    int n_z = add_node(d, GFNodeOp::gf_var_z);
    int p_exp = add_param(d, 2.0);
    int n_root = add_node(d, GFNodeOp::gf_pow_int, n_z, -1, p_exp);
    d.root_node = n_root;

    auto sr = gf_cpu_sample(d, {1e-6, 0.0});
    check_close("sample value_x", sr.value_x, 1e-12, 1e-18);
    check_close("sample abs2", sr.abs2, 1e-24, 1e-30);
    if (!sr.converged) { printf("*** FAIL: expected converged at z=1e-6\n"); ++failures; }
    if (sr.diverged) { printf("*** FAIL: unexpected diverged at z=1e-6\n"); ++failures; }

    // z^2 at z=(1e6, 0): value ~ (1e12, 0), diverged=true
    auto sr2 = gf_cpu_sample(d, {1e6, 0.0});
    if (sr2.diverged != true) { printf("*** FAIL: expected diverged at z=1e6\n"); ++failures; }

    printf("  sample result: %d failures so far\n", failures);
}

// =========================================================================
// Test: POD / trivially copyable
// =========================================================================
static void test_pod() {
    printf("--- POD checks ---\n");

    // GenericFunctionDesc should be trivially copyable (memcpy-safe for CUDA)
    static_assert(sizeof(GFNode) == 4 * sizeof(int), "GFNode size mismatch");
    // GenericFunctionDesc must contain arrays of fixed size
    static_assert(sizeof(GenericFunctionDesc) > sizeof(GFNode) * MAX_GF_NODES, "desc too small");

    printf("  POD: ok\n");
}

int main() {
    printf("=== Generic Function Type System Tests (GF-1) ===\n\n");

    test_pod();
    test_validation();
    test_z3_minus_1();
    test_sin_z_times_z();
    test_z2_plus_c();
    test_depth3_composition();
    test_iterate();
    test_derivative();
    test_misc_ops();
    test_sample_result();

    printf("\n=== Summary: %d failure(s) ===\n", failures);
    return failures > 0 ? 1 : 0;
}
