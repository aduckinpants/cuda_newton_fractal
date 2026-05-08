#pragma once
// Generic function type system for user-describable mathematical functions.
// No dependency on fractal_types.h, ViewState, KernelParams, or viewer code.
// Usable from both .cu and .cpp translation units.

#include <cstdint>

#ifndef __CUDACC__
#define GF_HOST_DEVICE
#else
#define GF_HOST_DEVICE __host__ __device__
#endif

enum class TerminationKind : std::uint8_t;

enum class GFNodeOp : int {
    // Leaf nodes
    gf_var_z = 0,         // The input complex variable z
    gf_var_z_conj,        // conjugate(z)
    gf_const_real,        // Real constant (from params[param_index])
    gf_const_complex,     // Complex constant (re=params[param_index], im=params[param_index+1])
    gf_param,             // Named parameter reference (index into param array)

    // Binary arithmetic
    gf_add, gf_sub, gf_mul, gf_div,

    // Unary complex
    gf_neg, gf_conj, gf_abs2, gf_abs,

    // Transcendental
    gf_sin, gf_cos, gf_exp, gf_log,
    gf_pow_int,           // z^n (integer exponent n = (int)params[param_index])
    gf_pow_real,          // z^r (real exponent r = params[param_index])

    // Combinators
    gf_compose,           // f(g(z)): evaluate child_right, then child_left with result as z
    gf_iterate,           // Apply child_left N times: params[param_index] = N

    // Aggregation
    gf_pack2,             // Pack two children into (re, im) output
};

struct GFNode {
    GFNodeOp op;
    int child_left;       // Index into node array, or -1
    int child_right;      // Index into node array, or -1
    int param_index;      // Index into param double array (for constants/params)
};

static constexpr int MAX_GF_NODES = 64;
static constexpr int MAX_GF_PARAMS = 32;

struct GenericFunctionDesc {
    GFNode nodes[MAX_GF_NODES];
    int node_count;
    double params[MAX_GF_PARAMS];
    int param_count;
    int root_node;        // Index of the root node in nodes[]
    int max_iterate;      // Max iterations for gf_iterate nodes (default 1)
};

// Coordinate struct for host API (same layout as CUDA double2).
struct GFPoint {
    double x, y;
};

struct GenericSampleResult {
    double value_x;       // Re(f(z))
    double value_y;       // Im(f(z))
    double abs2;          // |f(z)|^2
    double derivative_x;  // Re(f'(z))
    double derivative_y;  // Im(f'(z))
    int iterations;       // Iteration count (for gf_iterate nodes)
    bool converged;       // |f(z)| < epsilon after iteration
    bool diverged;        // |f(z)| > escape_radius
    TerminationKind termination_kind{};
    bool has_far_field_delta{false};
    double far_field_delta{0.0};
};

// --- Descriptor validation ---

struct GFValidationResult {
    bool valid;
    const char* error;    // Non-null if invalid
};

GF_HOST_DEVICE inline GFValidationResult ValidateGenericFunctionDesc(const GenericFunctionDesc& desc) {
    if (desc.node_count <= 0 || desc.node_count > MAX_GF_NODES)
        return {false, "node_count out of range [1, MAX_GF_NODES]"};
    if (desc.root_node < 0 || desc.root_node >= desc.node_count)
        return {false, "root_node out of range"};
    if (desc.param_count < 0 || desc.param_count > MAX_GF_PARAMS)
        return {false, "param_count out of range [0, MAX_GF_PARAMS]"};

    for (int i = 0; i < desc.node_count; ++i) {
        const GFNode& n = desc.nodes[i];
        if (n.child_left >= desc.node_count || n.child_right >= desc.node_count)
            return {false, "child index out of range"};
        if (n.child_left == i || n.child_right == i)
            return {false, "self-referencing node"};
        // Check param_index for ops that consume it
        bool uses_param =
            n.op == GFNodeOp::gf_const_real || n.op == GFNodeOp::gf_param ||
            n.op == GFNodeOp::gf_pow_int    || n.op == GFNodeOp::gf_pow_real ||
            n.op == GFNodeOp::gf_iterate;
        bool uses_param2 = n.op == GFNodeOp::gf_const_complex;
        if (uses_param && (n.param_index < 0 || n.param_index >= desc.param_count))
            return {false, "param_index out of range"};
        if (uses_param2 && (n.param_index < 0 || n.param_index + 1 >= desc.param_count))
            return {false, "param_index+1 out of range for complex constant"};
        if (n.op == GFNodeOp::gf_iterate) {
            double count = desc.params[n.param_index];
            if (!(count >= 1.0 && count <= 10000.0))
                return {false, "iterate count out of range [1, 10000]"};
            int count_int = (int)count;
            if (count != (double)count_int)
                return {false, "iterate count must be an integer"};
        }
    }
    return {true, nullptr};
}
