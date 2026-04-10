// test_generic_function_eval.cu
// GF-2 validation: CUDA device evaluator for GenericFunctionDesc.
// Tests gf_device_sample at known points for z^3-1, z^2+c, derivatives.

#include "../src/generic_function_types.h"
#include "../src/generic_function_eval.cuh"

#include <cstdio>
#include <cmath>
#include <cstring>

static int gPass = 0;
static int gFail = 0;

#define CHECK(name, cond) do { \
    if (cond) { ++gPass; } else { \
        ++gFail; printf("  FAIL: %s (%s:%d)\n", name, __FILE__, __LINE__); \
    } \
} while(0)

#define CHECK_CLOSE(name, actual, expected, tol) do { \
    double _err = fabs((double)(actual) - (double)(expected)); \
    if (_err <= (tol)) { ++gPass; } else { \
        ++gFail; printf("  FAIL: %s: expected %.12g, got %.12g (err=%.4e)\n", \
                        name, (double)(expected), (double)(actual), _err); \
    } \
} while(0)

// --- Descriptor builder helpers (same as CPU test) ---

static GenericFunctionDesc make_empty() {
    GenericFunctionDesc d;
    memset(&d, 0, sizeof(d));
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

// --- Kernel: evaluate one sample per thread ---

__global__ void gf_test_kernel(
    const GenericFunctionDesc* func,
    const double2* coords,
    GenericSampleResult* results,
    int count,
    double epsilon,
    double escape_radius)
{
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx >= count) return;
    GFComplex z = {coords[idx].x, coords[idx].y};
    results[idx] = gf_device_sample(*func, z, epsilon, escape_radius);
}

// Host helper: run kernel for N points.
static bool run_gf_kernel(
    const GenericFunctionDesc& func,
    const double2* hCoords,
    GenericSampleResult* hResults,
    int count,
    double epsilon = 1e-10,
    double escape_radius = 1e10)
{
    // Set sufficient CUDA stack for gf_device_eval's local arrays + recursion.
    cudaDeviceSetLimit(cudaLimitStackSize, 16384);

    GenericFunctionDesc* dFunc = nullptr;
    double2* dCoords = nullptr;
    GenericSampleResult* dResults = nullptr;

    cudaMalloc(&dFunc, sizeof(GenericFunctionDesc));
    cudaMalloc(&dCoords, count * sizeof(double2));
    cudaMalloc(&dResults, count * sizeof(GenericSampleResult));

    cudaMemcpy(dFunc, &func, sizeof(GenericFunctionDesc), cudaMemcpyHostToDevice);
    cudaMemcpy(dCoords, hCoords, count * sizeof(double2), cudaMemcpyHostToDevice);

    int threads = 256;
    int blocks = (count + threads - 1) / threads;
    gf_test_kernel<<<blocks, threads>>>(dFunc, dCoords, dResults, count, epsilon, escape_radius);

    cudaError_t err = cudaDeviceSynchronize();
    if (err != cudaSuccess) {
        printf("  CUDA error: %s\n", cudaGetErrorString(err));
        cudaFree(dFunc); cudaFree(dCoords); cudaFree(dResults);
        return false;
    }

    cudaMemcpy(hResults, dResults, count * sizeof(GenericSampleResult), cudaMemcpyDeviceToHost);
    cudaFree(dFunc); cudaFree(dCoords); cudaFree(dResults);
    return true;
}

// =====================================================================
// Test: z^3 - 1 at known points
// =====================================================================
static void test_z3_minus_1() {
    printf("--- z^3-1 (CUDA) ---\n");

    GenericFunctionDesc d = make_empty();
    int n_z = add_node(d, GFNodeOp::gf_var_z);
    int p_exp = add_param(d, 3.0);
    int n_pow = add_node(d, GFNodeOp::gf_pow_int, n_z, -1, p_exp);
    int p_one = add_param(d, 1.0);
    int n_one = add_node(d, GFNodeOp::gf_const_real, -1, -1, p_one);
    int n_root = add_node(d, GFNodeOp::gf_sub, n_pow, n_one);
    d.root_node = n_root;

    double2 coords[] = {
        {1.0, 0.0},          // z=1: 1^3-1=0
        {0.0, 1.0},          // z=i: i^3-1 = -i-1 = (-1,-1)
        {2.0, 0.0},          // z=2: 8-1=7
        {-0.5, 0.8660254},   // cube root of unity: ~0
    };
    int N = 4;
    GenericSampleResult results[4];

    if (!run_gf_kernel(d, coords, results, N)) {
        printf("  CUDA kernel failed\n"); ++gFail; return;
    }

    CHECK_CLOSE("z^3-1 at z=1 re", results[0].value_x, 0.0, 1e-10);
    CHECK_CLOSE("z^3-1 at z=1 im", results[0].value_y, 0.0, 1e-10);
    CHECK_CLOSE("z^3-1 at z=i re", results[1].value_x, -1.0, 1e-10);
    CHECK_CLOSE("z^3-1 at z=i im", results[1].value_y, -1.0, 1e-10);
    CHECK_CLOSE("z^3-1 at z=2 re", results[2].value_x, 7.0, 1e-10);
    CHECK_CLOSE("z^3-1 at omega re", results[3].value_x, 0.0, 1e-6);
    CHECK_CLOSE("z^3-1 at omega im", results[3].value_y, 0.0, 1e-6);
}

// =====================================================================
// Test: z^2 + c single evaluation
// =====================================================================
static void test_z2_plus_c() {
    printf("--- z^2+c (CUDA) ---\n");

    GenericFunctionDesc d = make_empty();
    int n_z = add_node(d, GFNodeOp::gf_var_z);
    int p_exp = add_param(d, 2.0);
    int n_pow = add_node(d, GFNodeOp::gf_pow_int, n_z, -1, p_exp);
    int p_cre = add_param(d, -0.75);
    int p_cim = add_param(d, 0.1);
    int n_c = add_node(d, GFNodeOp::gf_const_complex, -1, -1, p_cre);
    int n_root = add_node(d, GFNodeOp::gf_add, n_pow, n_c);
    d.root_node = n_root;

    double2 coords[] = {
        {0.0, 0.0},   // 0 + (-0.75,0.1) = (-0.75,0.1)
        {1.0, 0.0},   // 1 + (-0.75,0.1) = (0.25,0.1)
        {1.0, 1.0},   // (1+i)^2=2i + (-0.75,0.1) = (-0.75,2.1)
    };
    int N = 3;
    GenericSampleResult results[3];

    if (!run_gf_kernel(d, coords, results, N)) {
        printf("  CUDA kernel failed\n"); ++gFail; return;
    }

    CHECK_CLOSE("z^2+c at z=0 re", results[0].value_x, -0.75, 1e-10);
    CHECK_CLOSE("z^2+c at z=0 im", results[0].value_y, 0.1, 1e-10);
    CHECK_CLOSE("z^2+c at z=1 re", results[1].value_x, 0.25, 1e-10);
    CHECK_CLOSE("z^2+c at z=1+i re", results[2].value_x, -0.75, 1e-10);
    CHECK_CLOSE("z^2+c at z=1+i im", results[2].value_y, 2.1, 1e-10);
}

// =====================================================================
// Test: derivative of z^2 (analytic: 2z)
// =====================================================================
static void test_derivative_z2() {
    printf("--- d/dz(z^2) (CUDA) ---\n");

    GenericFunctionDesc d = make_empty();
    int n_z = add_node(d, GFNodeOp::gf_var_z);
    int p_exp = add_param(d, 2.0);
    int n_root = add_node(d, GFNodeOp::gf_pow_int, n_z, -1, p_exp);
    d.root_node = n_root;

    double2 coords[] = {
        {3.0, 0.0},   // f'(3) = 6
        {1.0, 1.0},   // f'(1+i) = 2+2i
    };
    int N = 2;
    GenericSampleResult results[2];

    if (!run_gf_kernel(d, coords, results, N)) {
        printf("  CUDA kernel failed\n"); ++gFail; return;
    }

    // Within 1% of analytic
    CHECK_CLOSE("d/dz(z^2) at z=3 re", results[0].derivative_x, 6.0, 0.06);
    CHECK_CLOSE("d/dz(z^2) at z=3 im", results[0].derivative_y, 0.0, 0.06);
    CHECK_CLOSE("d/dz(z^2) at z=1+i re", results[1].derivative_x, 2.0, 0.02);
    CHECK_CLOSE("d/dz(z^2) at z=1+i im", results[1].derivative_y, 2.0, 0.02);
}

// =====================================================================
// Test: derivative of z^3-1 (analytic: 3z^2)
// =====================================================================
static void test_derivative_z3() {
    printf("--- d/dz(z^3-1) (CUDA) ---\n");

    GenericFunctionDesc d = make_empty();
    int n_z = add_node(d, GFNodeOp::gf_var_z);
    int p_exp = add_param(d, 3.0);
    int n_pow = add_node(d, GFNodeOp::gf_pow_int, n_z, -1, p_exp);
    int p_one = add_param(d, 1.0);
    int n_one = add_node(d, GFNodeOp::gf_const_real, -1, -1, p_one);
    int n_root = add_node(d, GFNodeOp::gf_sub, n_pow, n_one);
    d.root_node = n_root;

    // f'(z) = 3z^2. At z=1: f'=3. At z=i: f'=3*(-1)=-3.
    double2 coords[] = {
        {1.0, 0.0},   // f'(1) = 3
        {0.0, 1.0},   // f'(i) = 3*i^2 = -3
    };
    int N = 2;
    GenericSampleResult results[2];

    if (!run_gf_kernel(d, coords, results, N)) {
        printf("  CUDA kernel failed\n"); ++gFail; return;
    }

    CHECK_CLOSE("d/dz(z^3-1) at z=1 re", results[0].derivative_x, 3.0, 0.03);
    CHECK_CLOSE("d/dz(z^3-1) at z=1 im", results[0].derivative_y, 0.0, 0.03);
    CHECK_CLOSE("d/dz(z^3-1) at z=i re", results[1].derivative_x, -3.0, 0.03);
    CHECK_CLOSE("d/dz(z^3-1) at z=i im", results[1].derivative_y, 0.0, 0.03);
}

// =====================================================================
// Test: sin(z) * z (transcendental)
// =====================================================================
static void test_sin_z_times_z() {
    printf("--- sin(z)*z (CUDA) ---\n");

    GenericFunctionDesc d = make_empty();
    int n_z1 = add_node(d, GFNodeOp::gf_var_z);
    int n_sin = add_node(d, GFNodeOp::gf_sin, n_z1);
    int n_z2 = add_node(d, GFNodeOp::gf_var_z);
    int n_root = add_node(d, GFNodeOp::gf_mul, n_sin, n_z2);
    d.root_node = n_root;

    double pi2 = 3.14159265358979323846 / 2.0;
    double2 coords[] = {
        {0.0, 0.0},    // sin(0)*0 = 0
        {pi2, 0.0},    // sin(pi/2)*pi/2 = pi/2
    };
    int N = 2;
    GenericSampleResult results[2];

    if (!run_gf_kernel(d, coords, results, N)) {
        printf("  CUDA kernel failed\n"); ++gFail; return;
    }

    CHECK_CLOSE("sin(z)*z at z=0 re", results[0].value_x, 0.0, 1e-10);
    CHECK_CLOSE("sin(z)*z at z=pi/2 re", results[1].value_x, pi2, 1e-8);
    CHECK_CLOSE("sin(z)*z at z=pi/2 im", results[1].value_y, 0.0, 1e-8);
}

// Note: compose/iterate tests live in the host-level test (GF-3),
// since the device evaluator is flat-only (non-recursive).

// =====================================================================
// Test: convergence and divergence classification
// =====================================================================
static void test_convergence_divergence() {
    printf("--- convergence/divergence (CUDA) ---\n");

    // z^2 at z near 0 → converged; at z=1e6 → diverged
    GenericFunctionDesc d = make_empty();
    int n_z = add_node(d, GFNodeOp::gf_var_z);
    int p_exp = add_param(d, 2.0);
    int n_root = add_node(d, GFNodeOp::gf_pow_int, n_z, -1, p_exp);
    d.root_node = n_root;

    double2 coords[] = {
        {1e-6, 0.0},  // f(z)=1e-12, should converge
        {1e6, 0.0},   // f(z)=1e12, should diverge
    };
    GenericSampleResult results[2];

    if (!run_gf_kernel(d, coords, results, 2, 1e-10, 1e10)) {
        printf("  CUDA kernel failed\n"); ++gFail; return;
    }

    CHECK("converge at z=1e-6", results[0].converged);
    CHECK("not diverge at z=1e-6", !results[0].diverged);
    CHECK("not converge at z=1e6", !results[1].converged);
    CHECK("diverge at z=1e6", results[1].diverged);
}

// =====================================================================
// Test: determinism (same input twice → same output)
// =====================================================================
static void test_determinism() {
    printf("--- determinism (CUDA) ---\n");

    GenericFunctionDesc d = make_empty();
    int n_z = add_node(d, GFNodeOp::gf_var_z);
    int p_exp = add_param(d, 3.0);
    int n_pow = add_node(d, GFNodeOp::gf_pow_int, n_z, -1, p_exp);
    int p_one = add_param(d, 1.0);
    int n_one = add_node(d, GFNodeOp::gf_const_real, -1, -1, p_one);
    int n_root = add_node(d, GFNodeOp::gf_sub, n_pow, n_one);
    d.root_node = n_root;

    double2 coords[] = {{1.23, -0.456}, {-0.789, 2.345}};
    GenericSampleResult r1[2], r2[2];

    if (!run_gf_kernel(d, coords, r1, 2)) { ++gFail; return; }
    if (!run_gf_kernel(d, coords, r2, 2)) { ++gFail; return; }

    CHECK("deterministic[0] value_x", r1[0].value_x == r2[0].value_x);
    CHECK("deterministic[0] value_y", r1[0].value_y == r2[0].value_y);
    CHECK("deterministic[1] value_x", r1[1].value_x == r2[1].value_x);
    CHECK("deterministic[1] value_y", r1[1].value_y == r2[1].value_y);
}

int main() {
    printf("=== Generic Function CUDA Evaluator Tests (GF-2) ===\n\n");

    test_z3_minus_1();
    test_z2_plus_c();
    test_derivative_z2();
    test_derivative_z3();
    test_sin_z_times_z();
    // compose/iterate tested at host level (GF-3), not in flat device evaluator
    test_convergence_divergence();
    test_determinism();

    printf("\n=== GF-2 summary: %d passed, %d failed ===\n", gPass, gFail);
    fflush(stdout);
    return gFail > 0 ? 1 : 0;
}
