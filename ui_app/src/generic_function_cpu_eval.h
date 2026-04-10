#pragma once
// CPU-only recursive reference evaluator for GenericFunctionDesc.
// Used by C++ tests. The CUDA device evaluator (GF-2) uses a stack-based
// approach in generic_function_math.cuh instead.

#include "generic_function_types.h"
#include <cmath>
#include <algorithm>

struct GFCpuComplex {
    double x, y;
};

inline GFCpuComplex gf_cpu_add(GFCpuComplex a, GFCpuComplex b) { return {a.x + b.x, a.y + b.y}; }
inline GFCpuComplex gf_cpu_sub(GFCpuComplex a, GFCpuComplex b) { return {a.x - b.x, a.y - b.y}; }
inline GFCpuComplex gf_cpu_mul(GFCpuComplex a, GFCpuComplex b) { return {a.x*b.x - a.y*b.y, a.x*b.y + a.y*b.x}; }
inline GFCpuComplex gf_cpu_div(GFCpuComplex a, GFCpuComplex b) {
    double d = b.x*b.x + b.y*b.y;
    if (d == 0.0) return {1e300, 1e300};
    return {(a.x*b.x + a.y*b.y)/d, (a.y*b.x - a.x*b.y)/d};
}
inline double gf_cpu_abs2(GFCpuComplex a) { return a.x*a.x + a.y*a.y; }

// Recursive evaluator with depth guard.
inline GFCpuComplex gf_cpu_eval_recursive(
    const GenericFunctionDesc& desc,
    int node_index,
    GFCpuComplex z,
    int depth = 0)
{
    if (depth > 64 || node_index < 0 || node_index >= desc.node_count)
        return {0.0, 0.0};

    const GFNode& n = desc.nodes[node_index];
    auto evalL = [&]() { return gf_cpu_eval_recursive(desc, n.child_left, z, depth + 1); };
    auto evalR = [&]() { return gf_cpu_eval_recursive(desc, n.child_right, z, depth + 1); };

    switch (n.op) {
    case GFNodeOp::gf_var_z:       return z;
    case GFNodeOp::gf_var_z_conj:  return {z.x, -z.y};
    case GFNodeOp::gf_const_real:  return {desc.params[n.param_index], 0.0};
    case GFNodeOp::gf_const_complex:
        return {desc.params[n.param_index], desc.params[n.param_index + 1]};
    case GFNodeOp::gf_param:       return {desc.params[n.param_index], 0.0};

    case GFNodeOp::gf_add: return gf_cpu_add(evalL(), evalR());
    case GFNodeOp::gf_sub: return gf_cpu_sub(evalL(), evalR());
    case GFNodeOp::gf_mul: return gf_cpu_mul(evalL(), evalR());
    case GFNodeOp::gf_div: return gf_cpu_div(evalL(), evalR());

    case GFNodeOp::gf_neg: { auto v = evalL(); return {-v.x, -v.y}; }
    case GFNodeOp::gf_conj: { auto v = evalL(); return {v.x, -v.y}; }
    case GFNodeOp::gf_abs2: { auto v = evalL(); return {gf_cpu_abs2(v), 0.0}; }
    case GFNodeOp::gf_abs:  { auto v = evalL(); return {std::sqrt(gf_cpu_abs2(v)), 0.0}; }

    case GFNodeOp::gf_sin: {
        auto v = evalL();
        return {std::sin(v.x)*std::cosh(v.y), std::cos(v.x)*std::sinh(v.y)};
    }
    case GFNodeOp::gf_cos: {
        auto v = evalL();
        return {std::cos(v.x)*std::cosh(v.y), -std::sin(v.x)*std::sinh(v.y)};
    }
    case GFNodeOp::gf_exp: {
        auto v = evalL();
        double r = std::exp(v.x);
        return {r*std::cos(v.y), r*std::sin(v.y)};
    }
    case GFNodeOp::gf_log: {
        auto v = evalL();
        double r = std::sqrt(gf_cpu_abs2(v));
        return {std::log(r), std::atan2(v.y, v.x)};
    }
    case GFNodeOp::gf_pow_int: {
        auto base = evalL();
        int ex = (int)desc.params[n.param_index];
        bool neg = ex < 0;
        int absEx = neg ? -ex : ex;
        GFCpuComplex result = {1.0, 0.0};
        for (int k = 0; k < absEx; ++k) result = gf_cpu_mul(result, base);
        if (neg) result = gf_cpu_div({1.0, 0.0}, result);
        return result;
    }
    case GFNodeOp::gf_pow_real: {
        auto base = evalL();
        double r = std::sqrt(gf_cpu_abs2(base));
        double theta = std::atan2(base.y, base.x);
        double ex = desc.params[n.param_index];
        double newR = std::pow(r, ex);
        double newTheta = theta * ex;
        return {newR*std::cos(newTheta), newR*std::sin(newTheta)};
    }

    case GFNodeOp::gf_compose: {
        // f(g(z)): evaluate g at z (child_right), then evaluate f at that result (child_left)
        GFCpuComplex g_result = gf_cpu_eval_recursive(desc, n.child_right, z, depth + 1);
        return gf_cpu_eval_recursive(desc, n.child_left, g_result, depth + 1);
    }
    case GFNodeOp::gf_iterate: {
        int N = (n.param_index >= 0) ? (int)desc.params[n.param_index] : desc.max_iterate;
        N = (std::max)(1, (std::min)(N, 10000));
        GFCpuComplex cur = z;
        for (int k = 0; k < N; ++k)
            cur = gf_cpu_eval_recursive(desc, n.child_left, cur, depth + 1);
        return cur;
    }
    case GFNodeOp::gf_pack2: {
        auto l = evalL();
        auto r = evalR();
        return {l.x, r.x};
    }
    default: return {0.0, 0.0};
    }
}

// Top-level convenience: evaluate the root node of a descriptor.
inline GFCpuComplex gf_cpu_eval(const GenericFunctionDesc& desc, GFCpuComplex z) {
    return gf_cpu_eval_recursive(desc, desc.root_node, z);
}

// Forward-difference derivative approximation.
inline GFCpuComplex gf_cpu_derivative(const GenericFunctionDesc& desc, GFCpuComplex z) {
    double h = 1e-8 * (std::max)(std::sqrt(gf_cpu_abs2(z)), 1.0);
    GFCpuComplex fz  = gf_cpu_eval(desc, z);
    GFCpuComplex fzh = gf_cpu_eval(desc, {z.x + h, z.y});
    return {(fzh.x - fz.x) / h, (fzh.y - fz.y) / h};
}

// Full sample result for a single point.
inline GenericSampleResult gf_cpu_sample(
    const GenericFunctionDesc& desc,
    GFCpuComplex z,
    double convergence_eps = 1e-10,
    double escape_radius = 1e10)
{
    GFCpuComplex fz = gf_cpu_eval(desc, z);
    GFCpuComplex dfz = gf_cpu_derivative(desc, z);
    double a2 = gf_cpu_abs2(fz);
    return {
        fz.x, fz.y, a2,
        dfz.x, dfz.y,
        0,  // iterations (set by caller if using iterate)
        a2 < convergence_eps * convergence_eps,
        a2 > escape_radius * escape_radius
    };
}
