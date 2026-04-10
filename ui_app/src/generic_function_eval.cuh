#pragma once
// CUDA device evaluator for GenericFunctionDesc expression trees.
// Fully non-recursive stack-based implementation for CUDA device stack safety.
// Depends on generic_function_types.h only (no fractal_types.h).

#include "generic_function_types.h"
#include <cuda_runtime.h>
#include <math.h>

// ---- Device complex helpers (double-precision, GF-specific) ----

struct GFComplex {
    double x, y;
};

__device__ __forceinline__ GFComplex gf_add(GFComplex a, GFComplex b) { return {a.x + b.x, a.y + b.y}; }
__device__ __forceinline__ GFComplex gf_sub(GFComplex a, GFComplex b) { return {a.x - b.x, a.y - b.y}; }
__device__ __forceinline__ GFComplex gf_mul(GFComplex a, GFComplex b) { return {a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x}; }
__device__ __forceinline__ GFComplex gf_div(GFComplex a, GFComplex b) {
    double d = b.x*b.x + b.y*b.y;
    if (d == 0.0) return {1e300, 1e300};
    return {(a.x*b.x + a.y*b.y)/d, (a.y*b.x - a.x*b.y)/d};
}
__device__ __forceinline__ double gf_abs2(GFComplex a) { return a.x*a.x + a.y*a.y; }

// ---- Non-recursive flat evaluator ----
// Evaluates a flat subtree (no compose/iterate) at input z.
// Compose and iterate are handled at the host level by decomposing
// into multiple flat evaluations (GF-3).

__device__ GFComplex gf_device_eval_flat(
    const GFNode* nodes, int nodeCount,
    const double* params,
    int rootNode, GFComplex z)
{
    if (rootNode < 0 || rootNode >= nodeCount) return {0.0, 0.0};

    // Phase 1: post-order DFS to build evaluation order.
    int evalOrder[MAX_GF_NODES];
    int evalCount = 0;
    {
        struct Frame { int node; bool expanded; };
        Frame work[MAX_GF_NODES];
        int wp = 0;
        work[wp++] = {rootNode, false};

        while (wp > 0 && evalCount < MAX_GF_NODES) {
            Frame& top = work[wp - 1];
            if (!top.expanded) {
                top.expanded = true;
                const GFNode& n = nodes[top.node];
                if (n.child_right >= 0 && n.child_right < nodeCount && wp < MAX_GF_NODES)
                    work[wp++] = {n.child_right, false};
                if (n.child_left >= 0 && n.child_left < nodeCount && wp < MAX_GF_NODES)
                    work[wp++] = {n.child_left, false};
            } else {
                evalOrder[evalCount++] = top.node;
                wp--;
            }
        }
    }

    // Phase 2: evaluate in post-order.
    GFComplex results[MAX_GF_NODES];
    for (int i = 0; i < nodeCount; ++i) results[i] = {0.0, 0.0};

    for (int i = 0; i < evalCount; ++i) {
        int ni = evalOrder[i];
        const GFNode& n = nodes[ni];
        GFComplex L = (n.child_left >= 0)  ? results[n.child_left]  : GFComplex{0.0, 0.0};
        GFComplex R = (n.child_right >= 0) ? results[n.child_right] : GFComplex{0.0, 0.0};

        switch (n.op) {
        case GFNodeOp::gf_var_z:       results[ni] = z; break;
        case GFNodeOp::gf_var_z_conj:  results[ni] = {z.x, -z.y}; break;
        case GFNodeOp::gf_const_real:  results[ni] = {params[n.param_index], 0.0}; break;
        case GFNodeOp::gf_const_complex:
            results[ni] = {params[n.param_index], params[n.param_index + 1]}; break;
        case GFNodeOp::gf_param:       results[ni] = {params[n.param_index], 0.0}; break;
        case GFNodeOp::gf_add: results[ni] = gf_add(L, R); break;
        case GFNodeOp::gf_sub: results[ni] = gf_sub(L, R); break;
        case GFNodeOp::gf_mul: results[ni] = gf_mul(L, R); break;
        case GFNodeOp::gf_div: results[ni] = gf_div(L, R); break;
        case GFNodeOp::gf_neg:  results[ni] = {-L.x, -L.y}; break;
        case GFNodeOp::gf_conj: results[ni] = {L.x, -L.y}; break;
        case GFNodeOp::gf_abs2: results[ni] = {gf_abs2(L), 0.0}; break;
        case GFNodeOp::gf_abs:  results[ni] = {sqrt(gf_abs2(L)), 0.0}; break;
        case GFNodeOp::gf_sin:
            results[ni] = {sin(L.x)*cosh(L.y), cos(L.x)*sinh(L.y)}; break;
        case GFNodeOp::gf_cos:
            results[ni] = {cos(L.x)*cosh(L.y), -sin(L.x)*sinh(L.y)}; break;
        case GFNodeOp::gf_exp: {
            double r = exp(L.x);
            results[ni] = {r*cos(L.y), r*sin(L.y)};
        } break;
        case GFNodeOp::gf_log: {
            double r = sqrt(gf_abs2(L));
            results[ni] = {log(r), atan2(L.y, L.x)};
        } break;
        case GFNodeOp::gf_pow_int: {
            int ex = (int)params[n.param_index];
            bool neg = ex < 0;
            int absEx = neg ? -ex : ex;
            GFComplex result = {1.0, 0.0};
            for (int k = 0; k < absEx && k < 100; ++k) result = gf_mul(result, L);
            if (neg) result = gf_div({1.0, 0.0}, result);
            results[ni] = result;
        } break;
        case GFNodeOp::gf_pow_real: {
            double r = sqrt(gf_abs2(L));
            double theta = atan2(L.y, L.x);
            double ex = params[n.param_index];
            double newR = pow(r, ex);
            double newTheta = theta * ex;
            results[ni] = {newR*cos(newTheta), newR*sin(newTheta)};
        } break;
        case GFNodeOp::gf_compose:
        case GFNodeOp::gf_iterate:
            // Not supported in flat device evaluator — handled at host level.
            results[ni] = {0.0, 0.0};
            break;
        case GFNodeOp::gf_pack2: results[ni] = {L.x, R.x}; break;
        default: results[ni] = {0.0, 0.0}; break;
        }
    }

    return results[rootNode];
}

// ---- Top-level device eval ----

__device__ __forceinline__ GFComplex gf_device_eval_root(
    const GenericFunctionDesc& desc, GFComplex z)
{
    return gf_device_eval_flat(
        desc.nodes, desc.node_count,
        desc.params,
        desc.root_node, z);
}

// ---- Forward-difference derivative ----

__device__ __forceinline__ GFComplex gf_device_derivative(
    const GenericFunctionDesc& desc, GFComplex z)
{
    double mag = sqrt(gf_abs2(z));
    double h = 1e-8 * fmax(mag, 1.0);
    GFComplex fz  = gf_device_eval_root(desc, z);
    GFComplex fzh = gf_device_eval_root(desc, {z.x + h, z.y});
    return {(fzh.x - fz.x) / h, (fzh.y - fz.y) / h};
}

// ---- Full sample result at a point ----

__device__ GenericSampleResult gf_device_sample(
    const GenericFunctionDesc& desc, GFComplex z,
    double epsilon, double escape_radius)
{
    GFComplex fz = gf_device_eval_root(desc, z);
    GFComplex dfz = gf_device_derivative(desc, z);
    double a2 = gf_abs2(fz);
    double eps2 = epsilon * epsilon;
    double esc2 = escape_radius * escape_radius;

    GenericSampleResult r;
    r.value_x = fz.x;
    r.value_y = fz.y;
    r.abs2 = a2;
    r.derivative_x = dfz.x;
    r.derivative_y = dfz.y;
    r.iterations = 0;
    r.converged = a2 < eps2;
    r.diverged = a2 > esc2;
    return r;
}
