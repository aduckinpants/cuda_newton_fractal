#pragma once

#include <cstdint>

constexpr int SDF_PACK_MAX_AST_NODES = 64;
constexpr int SDF_PACK_MAX_PARAMS = 32;

enum class SdfPackNodeOp : std::int32_t {
    invalid = 0,
    circle,
    box,
    capsule,
    union_op,
    intersect_op,
    subtract,
    smooth_union,
    translate,
    rotate,
    scale,
    repeat,
};

enum class SdfPackScalarKind : std::int32_t {
    constant = 0,
    param,
};

struct SdfPackScalarExpr {
    SdfPackScalarKind kind{SdfPackScalarKind::constant};
    double value{0.0};
    int param_index{-1};
};

struct SdfPackVec2Expr {
    SdfPackScalarExpr x;
    SdfPackScalarExpr y;
};

struct SdfPackRuntimeNode {
    SdfPackNodeOp op{SdfPackNodeOp::invalid};
    int child{-1};
    int child_a{-1};
    int child_b{-1};
    SdfPackVec2Expr center;
    SdfPackVec2Expr half_size;
    SdfPackVec2Expr point_a;
    SdfPackVec2Expr point_b;
    SdfPackVec2Expr offset;
    SdfPackVec2Expr period;
    SdfPackScalarExpr radius;
    SdfPackScalarExpr k;
    SdfPackScalarExpr angle;
    SdfPackScalarExpr factor;
};

struct SdfPackRuntimeDesc {
    SdfPackRuntimeNode nodes[SDF_PACK_MAX_AST_NODES];
    int node_count{0};
    int root_node{-1};
    double params[SDF_PACK_MAX_PARAMS]{};
    int param_count{0};
};

struct SdfPackGpuPoint {
    double x{0.0};
    double y{0.0};
};

struct SdfPackGpuSample {
    bool ok{false};
    double distance{0.0};
    int error_code{0};
};

enum SdfPackEvalErrorCode {
    SDF_PACK_EVAL_OK = 0,
    SDF_PACK_EVAL_INVALID_DESC = 1,
    SDF_PACK_EVAL_INVALID_NODE = 2,
    SDF_PACK_EVAL_INVALID_PARAM = 3,
    SDF_PACK_EVAL_STACK_OVERFLOW = 4,
    SDF_PACK_EVAL_INVALID_GEOMETRY = 5,
};
