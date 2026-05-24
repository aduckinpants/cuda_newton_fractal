#pragma once

#include "sdf_pack_runtime_types.h"

#include <math.h>

#ifndef __CUDACC__
#define SDF_PACK_HOST_DEVICE
#else
#define SDF_PACK_HOST_DEVICE __host__ __device__
#endif

struct SdfPackEvalVec2 {
    double x;
    double y;
};

SDF_PACK_HOST_DEVICE inline double sdf_pack_dot(SdfPackEvalVec2 a, SdfPackEvalVec2 b) {
    return a.x * b.x + a.y * b.y;
}

SDF_PACK_HOST_DEVICE inline SdfPackEvalVec2 sdf_pack_sub(SdfPackEvalVec2 a, SdfPackEvalVec2 b) {
    return {a.x - b.x, a.y - b.y};
}

SDF_PACK_HOST_DEVICE inline SdfPackEvalVec2 sdf_pack_scale(SdfPackEvalVec2 v, double s) {
    return {v.x * s, v.y * s};
}

SDF_PACK_HOST_DEVICE inline double sdf_pack_length(SdfPackEvalVec2 v) {
    return sqrt(sdf_pack_dot(v, v));
}

SDF_PACK_HOST_DEVICE inline double sdf_pack_clamp(double value, double lo, double hi) {
    return fmin(fmax(value, lo), hi);
}

SDF_PACK_HOST_DEVICE inline bool sdf_pack_eval_scalar(
    const SdfPackRuntimeDesc& desc,
    const SdfPackScalarExpr& expr,
    double* outValue,
    int* outError) {
    if (expr.kind == SdfPackScalarKind::constant) {
        *outValue = expr.value;
        return true;
    }
    if (expr.kind == SdfPackScalarKind::param) {
        if (expr.param_index < 0 || expr.param_index >= desc.param_count) {
            *outError = SDF_PACK_EVAL_INVALID_PARAM;
            return false;
        }
        *outValue = desc.params[expr.param_index];
        return true;
    }
    *outError = SDF_PACK_EVAL_INVALID_PARAM;
    return false;
}

SDF_PACK_HOST_DEVICE inline bool sdf_pack_eval_vec2(
    const SdfPackRuntimeDesc& desc,
    const SdfPackVec2Expr& expr,
    SdfPackEvalVec2* outValue,
    int* outError) {
    return sdf_pack_eval_scalar(desc, expr.x, &outValue->x, outError) &&
        sdf_pack_eval_scalar(desc, expr.y, &outValue->y, outError);
}

SDF_PACK_HOST_DEVICE inline bool sdf_pack_scalar_expr_is_valid(
    const SdfPackRuntimeDesc& desc,
    const SdfPackScalarExpr& expr) {
    if (expr.kind == SdfPackScalarKind::constant) return isfinite(expr.value);
    if (expr.kind == SdfPackScalarKind::param) return expr.param_index >= 0 && expr.param_index < desc.param_count;
    return false;
}

SDF_PACK_HOST_DEVICE inline bool sdf_pack_vec2_expr_is_valid(
    const SdfPackRuntimeDesc& desc,
    const SdfPackVec2Expr& expr) {
    return sdf_pack_scalar_expr_is_valid(desc, expr.x) &&
        sdf_pack_scalar_expr_is_valid(desc, expr.y);
}

SDF_PACK_HOST_DEVICE inline bool sdf_pack_desc_is_valid(const SdfPackRuntimeDesc& desc) {
    if (desc.node_count <= 0 || desc.node_count > SDF_PACK_MAX_AST_NODES) return false;
    if (desc.root_node < 0 || desc.root_node >= desc.node_count) return false;
    if (desc.param_count < 0 || desc.param_count > SDF_PACK_MAX_PARAMS) return false;
    for (int i = 0; i < desc.param_count; ++i) {
        if (!isfinite(desc.params[i])) return false;
    }
    for (int i = 0; i < desc.node_count; ++i) {
        const SdfPackRuntimeNode& n = desc.nodes[i];
        if (n.child < -1 || n.child_a < -1 || n.child_b < -1) return false;
        if (n.child >= desc.node_count || n.child_a >= desc.node_count || n.child_b >= desc.node_count) return false;
        if (n.child == i || n.child_a == i || n.child_b == i) return false;
        const bool binary =
            n.op == SdfPackNodeOp::union_op ||
            n.op == SdfPackNodeOp::intersect_op ||
            n.op == SdfPackNodeOp::subtract ||
            n.op == SdfPackNodeOp::smooth_union;
        const bool transform =
            n.op == SdfPackNodeOp::translate ||
            n.op == SdfPackNodeOp::rotate ||
            n.op == SdfPackNodeOp::scale ||
            n.op == SdfPackNodeOp::repeat;
        const bool primitive =
            n.op == SdfPackNodeOp::circle ||
            n.op == SdfPackNodeOp::box ||
            n.op == SdfPackNodeOp::capsule;
        if (binary && (n.child_a < 0 || n.child_b < 0 || n.child != -1)) return false;
        if (transform && (n.child < 0 || n.child_a != -1 || n.child_b != -1)) return false;
        if (primitive && (n.child != -1 || n.child_a != -1 || n.child_b != -1)) return false;
        if (!binary && !transform && !primitive) return false;
        if (n.op == SdfPackNodeOp::circle) {
            if (!sdf_pack_vec2_expr_is_valid(desc, n.center) || !sdf_pack_scalar_expr_is_valid(desc, n.radius)) return false;
        } else if (n.op == SdfPackNodeOp::box) {
            if (!sdf_pack_vec2_expr_is_valid(desc, n.center) || !sdf_pack_vec2_expr_is_valid(desc, n.half_size)) return false;
        } else if (n.op == SdfPackNodeOp::capsule) {
            if (!sdf_pack_vec2_expr_is_valid(desc, n.point_a) ||
                !sdf_pack_vec2_expr_is_valid(desc, n.point_b) ||
                !sdf_pack_scalar_expr_is_valid(desc, n.radius)) return false;
        } else if (n.op == SdfPackNodeOp::smooth_union) {
            if (!sdf_pack_scalar_expr_is_valid(desc, n.k)) return false;
        } else if (n.op == SdfPackNodeOp::translate) {
            if (!sdf_pack_vec2_expr_is_valid(desc, n.offset)) return false;
        } else if (n.op == SdfPackNodeOp::rotate) {
            if (!sdf_pack_scalar_expr_is_valid(desc, n.angle)) return false;
        } else if (n.op == SdfPackNodeOp::scale) {
            if (!sdf_pack_scalar_expr_is_valid(desc, n.factor)) return false;
        } else if (n.op == SdfPackNodeOp::repeat) {
            if (!sdf_pack_vec2_expr_is_valid(desc, n.period)) return false;
        }
    }
    return true;
}

SDF_PACK_HOST_DEVICE inline SdfPackGpuSample EvaluateSdfPackRuntimeDesc(
    const SdfPackRuntimeDesc& desc,
    double x,
    double y) {
    SdfPackGpuSample sample{};
    if (!sdf_pack_desc_is_valid(desc)) {
        sample.error_code = SDF_PACK_EVAL_INVALID_DESC;
        return sample;
    }

    struct Frame {
        int node;
        bool expanded;
        SdfPackEvalVec2 p;
    };

    Frame stack[SDF_PACK_MAX_AST_NODES];
    int stackCount = 0;
    double distances[SDF_PACK_MAX_AST_NODES];
    for (int i = 0; i < SDF_PACK_MAX_AST_NODES; ++i) distances[i] = 0.0;

    stack[stackCount++] = {desc.root_node, false, {x, y}};
    int error = SDF_PACK_EVAL_OK;

    while (stackCount > 0) {
        Frame& frame = stack[stackCount - 1];
        if (frame.node < 0 || frame.node >= desc.node_count) {
            error = SDF_PACK_EVAL_INVALID_NODE;
            break;
        }
        const SdfPackRuntimeNode& node = desc.nodes[frame.node];

        if (!frame.expanded) {
            frame.expanded = true;
            auto pushChild = [&](int child, SdfPackEvalVec2 childPoint) -> bool {
                if (child < 0 || child >= desc.node_count || stackCount >= SDF_PACK_MAX_AST_NODES) {
                    error = child < 0 || child >= desc.node_count
                        ? SDF_PACK_EVAL_INVALID_NODE
                        : SDF_PACK_EVAL_STACK_OVERFLOW;
                    return false;
                }
                stack[stackCount++] = {child, false, childPoint};
                return true;
            };

            if (node.op == SdfPackNodeOp::union_op ||
                node.op == SdfPackNodeOp::intersect_op ||
                node.op == SdfPackNodeOp::subtract ||
                node.op == SdfPackNodeOp::smooth_union) {
                if (!pushChild(node.child_b, frame.p)) break;
                if (!pushChild(node.child_a, frame.p)) break;
                continue;
            }
            if (node.op == SdfPackNodeOp::translate) {
                SdfPackEvalVec2 offset{};
                if (!sdf_pack_eval_vec2(desc, node.offset, &offset, &error)) break;
                if (!pushChild(node.child, sdf_pack_sub(frame.p, offset))) break;
                continue;
            }
            if (node.op == SdfPackNodeOp::rotate) {
                double angle = 0.0;
                if (!sdf_pack_eval_scalar(desc, node.angle, &angle, &error)) break;
                const double c = cos(angle);
                const double s = sin(angle);
                const SdfPackEvalVec2 q{c * frame.p.x + s * frame.p.y, -s * frame.p.x + c * frame.p.y};
                if (!pushChild(node.child, q)) break;
                continue;
            }
            if (node.op == SdfPackNodeOp::scale) {
                double factor = 0.0;
                if (!sdf_pack_eval_scalar(desc, node.factor, &factor, &error)) break;
                if (!(fabs(factor) >= 1.0e-12)) {
                    error = SDF_PACK_EVAL_INVALID_GEOMETRY;
                    break;
                }
                if (!pushChild(node.child, sdf_pack_scale(frame.p, 1.0 / factor))) break;
                continue;
            }
            if (node.op == SdfPackNodeOp::repeat) {
                SdfPackEvalVec2 period{};
                if (!sdf_pack_eval_vec2(desc, node.period, &period, &error)) break;
                if (!(period.x > 0.0) || !(period.y > 0.0)) {
                    error = SDF_PACK_EVAL_INVALID_GEOMETRY;
                    break;
                }
                const SdfPackEvalVec2 q{
                    frame.p.x - period.x * round(frame.p.x / period.x),
                    frame.p.y - period.y * round(frame.p.y / period.y),
                };
                if (!pushChild(node.child, q)) break;
                continue;
            }
        }

        double result = 0.0;
        if (node.op == SdfPackNodeOp::circle) {
            SdfPackEvalVec2 center{};
            double radius = 0.0;
            if (!sdf_pack_eval_vec2(desc, node.center, &center, &error)) break;
            if (!sdf_pack_eval_scalar(desc, node.radius, &radius, &error)) break;
            if (!(radius >= 0.0)) {
                error = SDF_PACK_EVAL_INVALID_GEOMETRY;
                break;
            }
            result = sdf_pack_length(sdf_pack_sub(frame.p, center)) - radius;
        } else if (node.op == SdfPackNodeOp::box) {
            SdfPackEvalVec2 center{};
            SdfPackEvalVec2 halfSize{};
            if (!sdf_pack_eval_vec2(desc, node.center, &center, &error)) break;
            if (!sdf_pack_eval_vec2(desc, node.half_size, &halfSize, &error)) break;
            if (!(halfSize.x >= 0.0) || !(halfSize.y >= 0.0)) {
                error = SDF_PACK_EVAL_INVALID_GEOMETRY;
                break;
            }
            const SdfPackEvalVec2 q{fabs(frame.p.x - center.x) - halfSize.x, fabs(frame.p.y - center.y) - halfSize.y};
            const SdfPackEvalVec2 outside{fmax(q.x, 0.0), fmax(q.y, 0.0)};
            const double inside = fmin(fmax(q.x, q.y), 0.0);
            result = sdf_pack_length(outside) + inside;
        } else if (node.op == SdfPackNodeOp::capsule) {
            SdfPackEvalVec2 a{};
            SdfPackEvalVec2 b{};
            double radius = 0.0;
            if (!sdf_pack_eval_vec2(desc, node.point_a, &a, &error)) break;
            if (!sdf_pack_eval_vec2(desc, node.point_b, &b, &error)) break;
            if (!sdf_pack_eval_scalar(desc, node.radius, &radius, &error)) break;
            if (!(radius >= 0.0)) {
                error = SDF_PACK_EVAL_INVALID_GEOMETRY;
                break;
            }
            const SdfPackEvalVec2 pa = sdf_pack_sub(frame.p, a);
            const SdfPackEvalVec2 ba = sdf_pack_sub(b, a);
            const double denom = sdf_pack_dot(ba, ba);
            const double h = denom > 0.0 ? sdf_pack_clamp(sdf_pack_dot(pa, ba) / denom, 0.0, 1.0) : 0.0;
            result = sdf_pack_length(sdf_pack_sub(pa, sdf_pack_scale(ba, h))) - radius;
        } else if (node.op == SdfPackNodeOp::union_op) {
            result = fmin(distances[node.child_a], distances[node.child_b]);
        } else if (node.op == SdfPackNodeOp::intersect_op) {
            result = fmax(distances[node.child_a], distances[node.child_b]);
        } else if (node.op == SdfPackNodeOp::subtract) {
            result = fmax(distances[node.child_a], -distances[node.child_b]);
        } else if (node.op == SdfPackNodeOp::smooth_union) {
            double k = 0.0;
            if (!sdf_pack_eval_scalar(desc, node.k, &k, &error)) break;
            const double da = distances[node.child_a];
            const double db = distances[node.child_b];
            if (!(k > 0.0)) {
                result = fmin(da, db);
            } else {
                const double h = sdf_pack_clamp(0.5 + 0.5 * (db - da) / k, 0.0, 1.0);
                result = db * (1.0 - h) + da * h - k * h * (1.0 - h);
            }
        } else if (node.op == SdfPackNodeOp::translate ||
            node.op == SdfPackNodeOp::rotate ||
            node.op == SdfPackNodeOp::repeat) {
            result = distances[node.child];
        } else if (node.op == SdfPackNodeOp::scale) {
            double factor = 0.0;
            if (!sdf_pack_eval_scalar(desc, node.factor, &factor, &error)) break;
            result = distances[node.child] * fabs(factor);
        } else {
            error = SDF_PACK_EVAL_INVALID_NODE;
            break;
        }

        distances[frame.node] = result;
        --stackCount;
    }

    if (error != SDF_PACK_EVAL_OK) {
        sample.error_code = error;
        return sample;
    }
    sample.ok = true;
    sample.error_code = SDF_PACK_EVAL_OK;
    sample.distance = distances[desc.root_node];
    return sample;
}
