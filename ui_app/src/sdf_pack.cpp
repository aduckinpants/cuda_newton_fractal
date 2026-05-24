#include "sdf_pack.h"

#include <algorithm>
#include <cmath>
#include <set>

namespace {

struct Vec2 {
    double x{0.0};
    double y{0.0};
};

double Dot(Vec2 a, Vec2 b) {
    return a.x * b.x + a.y * b.y;
}

double Length(Vec2 v) {
    return std::sqrt(Dot(v, v));
}

Vec2 Sub(Vec2 a, Vec2 b) {
    return {a.x - b.x, a.y - b.y};
}

Vec2 Scale(Vec2 v, double s) {
    return {v.x * s, v.y * s};
}

bool RejectUnknownKeys(const json_min::Object& object,
    const std::set<std::string>& allowed,
    const std::string& where,
    std::string* outError) {
    for (const auto& entry : object) {
        if (allowed.find(entry.first) == allowed.end()) {
            if (outError) *outError = "Unknown " + where + " key: " + entry.first;
            return false;
        }
    }
    return true;
}

bool GetRequiredString(const json_min::Object& object,
    const char* key,
    std::string* outValue,
    std::string* outError) {
    auto it = object.find(key);
    if (it == object.end() || !it->second.is_string()) {
        if (outError) *outError = std::string("Missing string field: ") + key;
        return false;
    }
    if (outValue) *outValue = it->second.as_string();
    return true;
}

bool GetOptionalString(const json_min::Object& object,
    const char* key,
    std::string* outValue,
    std::string* outError) {
    auto it = object.find(key);
    if (it == object.end()) return true;
    if (!it->second.is_string()) {
        if (outError) *outError = std::string("Expected string field: ") + key;
        return false;
    }
    if (outValue) *outValue = it->second.as_string();
    return true;
}

bool GetRequiredFiniteNumber(const json_min::Object& object,
    const char* key,
    double* outValue,
    std::string* outError) {
    auto it = object.find(key);
    if (it == object.end() || !it->second.is_number() || !std::isfinite(it->second.as_number())) {
        if (outError) *outError = std::string("Missing finite number field: ") + key;
        return false;
    }
    if (outValue) *outValue = it->second.as_number();
    return true;
}

bool GetOptionalFiniteNumber(const json_min::Object& object,
    const char* key,
    double* outValue,
    bool* outPresent,
    std::string* outError) {
    auto it = object.find(key);
    if (it == object.end()) {
        if (outPresent) *outPresent = false;
        return true;
    }
    if (!it->second.is_number() || !std::isfinite(it->second.as_number())) {
        if (outError) *outError = std::string("Expected finite number field: ") + key;
        return false;
    }
    if (outValue) *outValue = it->second.as_number();
    if (outPresent) *outPresent = true;
    return true;
}

bool GetRequiredInt(const json_min::Object& object,
    const char* key,
    int* outValue,
    std::string* outError) {
    double raw = 0.0;
    if (!GetRequiredFiniteNumber(object, key, &raw, outError)) return false;
    if (std::floor(raw) != raw) {
        if (outError) *outError = std::string("Expected integer field: ") + key;
        return false;
    }
    if (outValue) *outValue = static_cast<int>(raw);
    return true;
}

bool GetArray2(const json_min::Value& value, const char* label, double* outA, double* outB, std::string* outError) {
    if (!value.is_array() || value.as_array().size() != 2 ||
        !value.as_array()[0].is_number() || !std::isfinite(value.as_array()[0].as_number()) ||
        !value.as_array()[1].is_number() || !std::isfinite(value.as_array()[1].as_number())) {
        if (outError) *outError = std::string(label) + " must be a two-number array";
        return false;
    }
    if (outA) *outA = value.as_array()[0].as_number();
    if (outB) *outB = value.as_array()[1].as_number();
    return true;
}

const SdfPackParam* FindParam(const std::vector<SdfPackParam>& params, const std::string& id) {
    for (const SdfPackParam& param : params) {
        if (param.id == id) return &param;
    }
    return nullptr;
}

bool ValidateScalarExpr(
    const json_min::Value& value,
    const std::vector<SdfPackParam>& params,
    std::string* outError);

bool ValidateVec2Expr(
    const json_min::Value& value,
    const std::vector<SdfPackParam>& params,
    const char* label,
    std::string* outError) {
    if (!value.is_array() || value.as_array().size() != 2) {
        if (outError) *outError = std::string(label) + " must be a two-value array";
        return false;
    }
    return ValidateScalarExpr(value.as_array()[0], params, outError) &&
        ValidateScalarExpr(value.as_array()[1], params, outError);
}

bool ValidateScalarExpr(
    const json_min::Value& value,
    const std::vector<SdfPackParam>& params,
    std::string* outError) {
    if (value.is_number()) {
        if (!std::isfinite(value.as_number())) {
            if (outError) *outError = "numeric expression must be finite";
            return false;
        }
        return true;
    }
    if (!value.is_object()) {
        if (outError) *outError = "numeric expression must be a number or param reference";
        return false;
    }
    const json_min::Object& object = value.as_object();
    std::string paramId;
    auto paramIt = object.find("param");
    if (paramIt != object.end()) {
        if (!RejectUnknownKeys(object, {"param"}, "param expression", outError)) return false;
        if (!paramIt->second.is_string()) {
            if (outError) *outError = "param expression must name a string param";
            return false;
        }
        paramId = paramIt->second.as_string();
    } else {
        if (!RejectUnknownKeys(object, {"op", "name"}, "param expression", outError)) return false;
        std::string op;
        if (!GetRequiredString(object, "op", &op, outError)) return false;
        if (op != "param") {
            if (outError) *outError = "numeric expression object op must be param";
            return false;
        }
        if (!GetRequiredString(object, "name", &paramId, outError)) return false;
    }
    if (!FindParam(params, paramId)) {
        if (outError) *outError = "unknown param reference: " + paramId;
        return false;
    }
    return true;
}

bool ValidateAstNode(
    const json_min::Value& value,
    const std::vector<SdfPackParam>& params,
    int* ioCount,
    std::string* outError) {
    if (!value.is_object()) {
        if (outError) *outError = "ast node must be an object";
        return false;
    }
    ++(*ioCount);
    if (*ioCount > SDF_PACK_MAX_AST_NODES) {
        if (outError) *outError = "SDF AST exceeds node limit";
        return false;
    }

    const json_min::Object& object = value.as_object();
    std::string op;
    if (!GetRequiredString(object, "op", &op, outError)) return false;

    auto requireNode = [&](const char* key) -> bool {
        auto it = object.find(key);
        if (it == object.end()) {
            if (outError) *outError = std::string("Missing ast child: ") + key;
            return false;
        }
        return ValidateAstNode(it->second, params, ioCount, outError);
    };
    auto requireScalar = [&](const char* key) -> bool {
        auto it = object.find(key);
        if (it == object.end()) {
            if (outError) *outError = std::string("Missing numeric field: ") + key;
            return false;
        }
        return ValidateScalarExpr(it->second, params, outError);
    };
    auto optionalVec2 = [&](const char* key) -> bool {
        auto it = object.find(key);
        if (it == object.end()) return true;
        return ValidateVec2Expr(it->second, params, key, outError);
    };
    auto requireVec2 = [&](const char* key) -> bool {
        auto it = object.find(key);
        if (it == object.end()) {
            if (outError) *outError = std::string("Missing vec2 field: ") + key;
            return false;
        }
        return ValidateVec2Expr(it->second, params, key, outError);
    };

    if (op == "circle") {
        return RejectUnknownKeys(object, {"op", "center", "radius"}, "circle ast", outError) &&
            optionalVec2("center") &&
            requireScalar("radius");
    }
    if (op == "box") {
        return RejectUnknownKeys(object, {"op", "center", "half_size"}, "box ast", outError) &&
            optionalVec2("center") &&
            requireVec2("half_size");
    }
    if (op == "capsule") {
        return RejectUnknownKeys(object, {"op", "a", "b", "radius"}, "capsule ast", outError) &&
            requireVec2("a") &&
            requireVec2("b") &&
            requireScalar("radius");
    }
    if (op == "union" || op == "intersect" || op == "subtract") {
        return RejectUnknownKeys(object, {"op", "a", "b"}, op + " ast", outError) &&
            requireNode("a") &&
            requireNode("b");
    }
    if (op == "smooth_union") {
        return RejectUnknownKeys(object, {"op", "a", "b", "k"}, "smooth_union ast", outError) &&
            requireNode("a") &&
            requireNode("b") &&
            requireScalar("k");
    }
    if (op == "translate") {
        return RejectUnknownKeys(object, {"op", "offset", "child"}, "translate ast", outError) &&
            requireVec2("offset") &&
            requireNode("child");
    }
    if (op == "rotate") {
        return RejectUnknownKeys(object, {"op", "angle", "child"}, "rotate ast", outError) &&
            requireScalar("angle") &&
            requireNode("child");
    }
    if (op == "scale") {
        return RejectUnknownKeys(object, {"op", "factor", "child"}, "scale ast", outError) &&
            requireScalar("factor") &&
            requireNode("child");
    }
    if (op == "repeat") {
        return RejectUnknownKeys(object, {"op", "period", "child"}, "repeat ast", outError) &&
            requireVec2("period") &&
            requireNode("child");
    }

    if (outError) *outError = "unknown SDF AST op: " + op;
    return false;
}

bool ParseParams(const json_min::Object& root, SdfPack* ioPack, std::string* outError) {
    auto paramsIt = root.find("params");
    if (paramsIt == root.end()) return true;
    if (!paramsIt->second.is_array()) {
        if (outError) *outError = "params must be an array";
        return false;
    }
    if (paramsIt->second.as_array().size() > SDF_PACK_MAX_PARAMS) {
        if (outError) *outError = "too many params";
        return false;
    }

    std::set<std::string> ids;
    for (const json_min::Value& value : paramsIt->second.as_array()) {
        if (!value.is_object()) {
            if (outError) *outError = "params entries must be objects";
            return false;
        }
        const json_min::Object& object = value.as_object();
        if (!RejectUnknownKeys(object, {"id", "type", "default", "range"}, "param", outError)) return false;

        SdfPackParam param;
        if (!GetRequiredString(object, "id", &param.id, outError)) return false;
        if (!GetRequiredString(object, "type", &param.type, outError)) return false;
        if (param.id.empty()) {
            if (outError) *outError = "param id must not be empty";
            return false;
        }
        if (param.type != "float") {
            if (outError) *outError = "SDF pack v1 params must be float";
            return false;
        }
        if (!ids.insert(param.id).second) {
            if (outError) *outError = "duplicate param id: " + param.id;
            return false;
        }
        if (!GetRequiredFiniteNumber(object, "default", &param.default_value, outError)) return false;
        auto rangeIt = object.find("range");
        if (rangeIt == object.end() ||
            !GetArray2(rangeIt->second, "range", &param.min_value, &param.max_value, outError)) {
            return false;
        }
        if (!(param.min_value <= param.max_value)) {
            if (outError) *outError = "param range min must be <= max";
            return false;
        }
        if (param.default_value < param.min_value || param.default_value > param.max_value) {
            if (outError) *outError = "param default outside range: " + param.id;
            return false;
        }
        ioPack->params.push_back(param);
    }
    return true;
}

bool ParseControls(const json_min::Object& root, SdfPack* ioPack, std::string* outError) {
    auto controlsIt = root.find("controls");
    if (controlsIt == root.end()) return true;
    if (!controlsIt->second.is_array()) {
        if (outError) *outError = "controls must be an array";
        return false;
    }

    std::set<std::string> params;
    for (const json_min::Value& value : controlsIt->second.as_array()) {
        if (!value.is_object()) {
            if (outError) *outError = "controls entries must be objects";
            return false;
        }
        const json_min::Object& object = value.as_object();
        if (!RejectUnknownKeys(object, {"param", "label", "ui_min", "ui_max"}, "control", outError)) return false;

        SdfPackControl control;
        if (!GetRequiredString(object, "param", &control.param, outError)) return false;
        if (!GetOptionalString(object, "label", &control.label, outError)) return false;
        if (!FindParam(ioPack->params, control.param)) {
            if (outError) *outError = "control references unknown param: " + control.param;
            return false;
        }
        if (!params.insert(control.param).second) {
            if (outError) *outError = "duplicate control param: " + control.param;
            return false;
        }
        if (!GetOptionalFiniteNumber(object, "ui_min", &control.ui_min, &control.has_ui_min, outError)) return false;
        if (!GetOptionalFiniteNumber(object, "ui_max", &control.ui_max, &control.has_ui_max, outError)) return false;
        if (control.has_ui_min != control.has_ui_max || (control.has_ui_min && !(control.ui_min <= control.ui_max))) {
            if (outError) *outError = "control ui_min/ui_max must both exist and be ordered";
            return false;
        }
        ioPack->controls.push_back(control);
    }
    return true;
}

bool ParseRegion(const json_min::Object& root, SdfPack* ioPack, std::string* outError) {
    auto regionIt = root.find("region");
    if (regionIt == root.end()) return true;
    if (!regionIt->second.is_object()) {
        if (outError) *outError = "region must be an object";
        return false;
    }
    const json_min::Object& object = regionIt->second.as_object();
    if (!RejectUnknownKeys(object, {"center", "half_height"}, "region", outError)) return false;

    SdfPackRegion region;
    region.has_region = true;
    auto centerIt = object.find("center");
    if (centerIt == object.end() ||
        !GetArray2(centerIt->second, "region.center", &region.center_x, &region.center_y, outError)) {
        return false;
    }
    if (!GetRequiredFiniteNumber(object, "half_height", &region.half_height, outError)) return false;
    if (!(region.half_height > 0.0)) {
        if (outError) *outError = "region.half_height must be positive";
        return false;
    }
    ioPack->region = region;
    return true;
}

bool EvalScalarExpr(
    const json_min::Value& value,
    const std::map<std::string, double>& params,
    double* outValue,
    std::string* outError);

bool EvalVec2Expr(
    const json_min::Value& value,
    const std::map<std::string, double>& params,
    const char* label,
    Vec2* outValue,
    std::string* outError) {
    if (!value.is_array() || value.as_array().size() != 2) {
        if (outError) *outError = std::string(label) + " must be a two-value array";
        return false;
    }
    return EvalScalarExpr(value.as_array()[0], params, &outValue->x, outError) &&
        EvalScalarExpr(value.as_array()[1], params, &outValue->y, outError);
}

bool EvalScalarExpr(
    const json_min::Value& value,
    const std::map<std::string, double>& params,
    double* outValue,
    std::string* outError) {
    if (value.is_number()) {
        if (!std::isfinite(value.as_number())) {
            if (outError) *outError = "numeric expression must be finite";
            return false;
        }
        if (outValue) *outValue = value.as_number();
        return true;
    }
    if (!value.is_object()) {
        if (outError) *outError = "numeric expression must be a number or param reference";
        return false;
    }
    const json_min::Object& object = value.as_object();
    std::string paramId;
    auto paramIt = object.find("param");
    if (paramIt != object.end()) {
        if (!paramIt->second.is_string()) {
            if (outError) *outError = "param expression must name a string param";
            return false;
        }
        paramId = paramIt->second.as_string();
    } else {
        std::string op;
        if (!GetRequiredString(object, "op", &op, outError)) return false;
        if (op != "param") {
            if (outError) *outError = "numeric expression object op must be param";
            return false;
        }
        if (!GetRequiredString(object, "name", &paramId, outError)) return false;
    }
    auto it = params.find(paramId);
    if (it == params.end()) {
        if (outError) *outError = "unknown param reference: " + paramId;
        return false;
    }
    if (outValue) *outValue = it->second;
    return true;
}

bool OptionalVec2Field(
    const json_min::Object& object,
    const char* key,
    const std::map<std::string, double>& params,
    Vec2 defaultValue,
    Vec2* outValue,
    std::string* outError) {
    auto it = object.find(key);
    if (it == object.end()) {
        *outValue = defaultValue;
        return true;
    }
    return EvalVec2Expr(it->second, params, key, outValue, outError);
}

bool RequiredVec2Field(
    const json_min::Object& object,
    const char* key,
    const std::map<std::string, double>& params,
    Vec2* outValue,
    std::string* outError) {
    auto it = object.find(key);
    if (it == object.end()) {
        if (outError) *outError = std::string("Missing vec2 field: ") + key;
        return false;
    }
    return EvalVec2Expr(it->second, params, key, outValue, outError);
}

bool RequiredScalarField(
    const json_min::Object& object,
    const char* key,
    const std::map<std::string, double>& params,
    double* outValue,
    std::string* outError) {
    auto it = object.find(key);
    if (it == object.end()) {
        if (outError) *outError = std::string("Missing numeric field: ") + key;
        return false;
    }
    return EvalScalarExpr(it->second, params, outValue, outError);
}

SdfPackScalarExpr SdfConst(double value) {
    SdfPackScalarExpr expr;
    expr.kind = SdfPackScalarKind::constant;
    expr.value = value;
    expr.param_index = -1;
    return expr;
}

SdfPackVec2Expr SdfVec2Const(double x, double y) {
    SdfPackVec2Expr expr;
    expr.x = SdfConst(x);
    expr.y = SdfConst(y);
    return expr;
}

bool LowerScalarExpr(
    const json_min::Value& value,
    const std::map<std::string, int>& paramIndices,
    SdfPackScalarExpr* outExpr,
    std::string* outError) {
    if (value.is_number()) {
        if (!std::isfinite(value.as_number())) {
            if (outError) *outError = "numeric expression must be finite";
            return false;
        }
        if (outExpr) *outExpr = SdfConst(value.as_number());
        return true;
    }
    if (!value.is_object()) {
        if (outError) *outError = "numeric expression must be a number or param reference";
        return false;
    }
    const json_min::Object& object = value.as_object();
    std::string paramId;
    auto paramIt = object.find("param");
    if (paramIt != object.end()) {
        if (!paramIt->second.is_string()) {
            if (outError) *outError = "param expression must name a string param";
            return false;
        }
        paramId = paramIt->second.as_string();
    } else {
        std::string op;
        if (!GetRequiredString(object, "op", &op, outError)) return false;
        if (op != "param") {
            if (outError) *outError = "numeric expression object op must be param";
            return false;
        }
        if (!GetRequiredString(object, "name", &paramId, outError)) return false;
    }
    auto indexIt = paramIndices.find(paramId);
    if (indexIt == paramIndices.end()) {
        if (outError) *outError = "unknown param reference: " + paramId;
        return false;
    }
    if (outExpr) {
        outExpr->kind = SdfPackScalarKind::param;
        outExpr->value = 0.0;
        outExpr->param_index = indexIt->second;
    }
    return true;
}

bool LowerVec2Expr(
    const json_min::Value& value,
    const std::map<std::string, int>& paramIndices,
    const char* label,
    SdfPackVec2Expr* outExpr,
    std::string* outError) {
    if (!value.is_array() || value.as_array().size() != 2) {
        if (outError) *outError = std::string(label) + " must be a two-value array";
        return false;
    }
    SdfPackVec2Expr expr;
    if (!LowerScalarExpr(value.as_array()[0], paramIndices, &expr.x, outError)) return false;
    if (!LowerScalarExpr(value.as_array()[1], paramIndices, &expr.y, outError)) return false;
    if (outExpr) *outExpr = expr;
    return true;
}

bool LowerOptionalVec2Field(
    const json_min::Object& object,
    const char* key,
    const std::map<std::string, int>& paramIndices,
    SdfPackVec2Expr defaultValue,
    SdfPackVec2Expr* outExpr,
    std::string* outError) {
    auto it = object.find(key);
    if (it == object.end()) {
        *outExpr = defaultValue;
        return true;
    }
    return LowerVec2Expr(it->second, paramIndices, key, outExpr, outError);
}

bool LowerRequiredVec2Field(
    const json_min::Object& object,
    const char* key,
    const std::map<std::string, int>& paramIndices,
    SdfPackVec2Expr* outExpr,
    std::string* outError) {
    auto it = object.find(key);
    if (it == object.end()) {
        if (outError) *outError = std::string("Missing vec2 field: ") + key;
        return false;
    }
    return LowerVec2Expr(it->second, paramIndices, key, outExpr, outError);
}

bool LowerRequiredScalarField(
    const json_min::Object& object,
    const char* key,
    const std::map<std::string, int>& paramIndices,
    SdfPackScalarExpr* outExpr,
    std::string* outError) {
    auto it = object.find(key);
    if (it == object.end()) {
        if (outError) *outError = std::string("Missing numeric field: ") + key;
        return false;
    }
    return LowerScalarExpr(it->second, paramIndices, outExpr, outError);
}

bool AppendRuntimeNode(
    const SdfPackRuntimeNode& node,
    SdfPackRuntimeDesc* ioDesc,
    int* outIndex,
    std::string* outError) {
    if (ioDesc->node_count >= SDF_PACK_MAX_AST_NODES) {
        if (outError) *outError = "SDF AST exceeds node limit";
        return false;
    }
    const int index = ioDesc->node_count++;
    ioDesc->nodes[index] = node;
    if (outIndex) *outIndex = index;
    return true;
}

bool LowerAstNodeToRuntime(
    const json_min::Value& value,
    const std::map<std::string, int>& paramIndices,
    SdfPackRuntimeDesc* ioDesc,
    int* outIndex,
    std::string* outError) {
    if (!value.is_object()) {
        if (outError) *outError = "ast node must be an object";
        return false;
    }
    const json_min::Object& object = value.as_object();
    std::string op;
    if (!GetRequiredString(object, "op", &op, outError)) return false;

    auto lowerChild = [&](const char* key, int* outChild) -> bool {
        auto it = object.find(key);
        if (it == object.end()) {
            if (outError) *outError = std::string("Missing ast child: ") + key;
            return false;
        }
        return LowerAstNodeToRuntime(it->second, paramIndices, ioDesc, outChild, outError);
    };

    SdfPackRuntimeNode node;
    node.center = SdfVec2Const(0.0, 0.0);

    if (op == "circle") {
        node.op = SdfPackNodeOp::circle;
        if (!LowerOptionalVec2Field(object, "center", paramIndices, SdfVec2Const(0.0, 0.0), &node.center, outError)) return false;
        if (!LowerRequiredScalarField(object, "radius", paramIndices, &node.radius, outError)) return false;
        return AppendRuntimeNode(node, ioDesc, outIndex, outError);
    }
    if (op == "box") {
        node.op = SdfPackNodeOp::box;
        if (!LowerOptionalVec2Field(object, "center", paramIndices, SdfVec2Const(0.0, 0.0), &node.center, outError)) return false;
        if (!LowerRequiredVec2Field(object, "half_size", paramIndices, &node.half_size, outError)) return false;
        return AppendRuntimeNode(node, ioDesc, outIndex, outError);
    }
    if (op == "capsule") {
        node.op = SdfPackNodeOp::capsule;
        if (!LowerRequiredVec2Field(object, "a", paramIndices, &node.point_a, outError)) return false;
        if (!LowerRequiredVec2Field(object, "b", paramIndices, &node.point_b, outError)) return false;
        if (!LowerRequiredScalarField(object, "radius", paramIndices, &node.radius, outError)) return false;
        return AppendRuntimeNode(node, ioDesc, outIndex, outError);
    }
    if (op == "union" || op == "intersect" || op == "subtract" || op == "smooth_union") {
        if (!lowerChild("a", &node.child_a)) return false;
        if (!lowerChild("b", &node.child_b)) return false;
        if (op == "union") {
            node.op = SdfPackNodeOp::union_op;
        } else if (op == "intersect") {
            node.op = SdfPackNodeOp::intersect_op;
        } else if (op == "subtract") {
            node.op = SdfPackNodeOp::subtract;
        } else {
            node.op = SdfPackNodeOp::smooth_union;
            if (!LowerRequiredScalarField(object, "k", paramIndices, &node.k, outError)) return false;
        }
        return AppendRuntimeNode(node, ioDesc, outIndex, outError);
    }
    if (op == "translate") {
        node.op = SdfPackNodeOp::translate;
        if (!LowerRequiredVec2Field(object, "offset", paramIndices, &node.offset, outError)) return false;
        if (!lowerChild("child", &node.child)) return false;
        return AppendRuntimeNode(node, ioDesc, outIndex, outError);
    }
    if (op == "rotate") {
        node.op = SdfPackNodeOp::rotate;
        if (!LowerRequiredScalarField(object, "angle", paramIndices, &node.angle, outError)) return false;
        if (!lowerChild("child", &node.child)) return false;
        return AppendRuntimeNode(node, ioDesc, outIndex, outError);
    }
    if (op == "scale") {
        node.op = SdfPackNodeOp::scale;
        if (!LowerRequiredScalarField(object, "factor", paramIndices, &node.factor, outError)) return false;
        if (!lowerChild("child", &node.child)) return false;
        return AppendRuntimeNode(node, ioDesc, outIndex, outError);
    }
    if (op == "repeat") {
        node.op = SdfPackNodeOp::repeat;
        if (!LowerRequiredVec2Field(object, "period", paramIndices, &node.period, outError)) return false;
        if (!lowerChild("child", &node.child)) return false;
        return AppendRuntimeNode(node, ioDesc, outIndex, outError);
    }

    if (outError) *outError = "unknown SDF AST op: " + op;
    return false;
}

bool EvalAstNode(
    const json_min::Value& value,
    Vec2 p,
    const std::map<std::string, double>& params,
    double* outDistance,
    std::string* outError) {
    if (!value.is_object()) {
        if (outError) *outError = "ast node must be an object";
        return false;
    }
    const json_min::Object& object = value.as_object();
    std::string op;
    if (!GetRequiredString(object, "op", &op, outError)) return false;

    if (op == "circle") {
        Vec2 center;
        double radius = 0.0;
        if (!OptionalVec2Field(object, "center", params, {0.0, 0.0}, &center, outError)) return false;
        if (!RequiredScalarField(object, "radius", params, &radius, outError)) return false;
        if (!(radius >= 0.0)) {
            if (outError) *outError = "circle radius must be non-negative";
            return false;
        }
        *outDistance = Length(Sub(p, center)) - radius;
        return true;
    }
    if (op == "box") {
        Vec2 center;
        Vec2 halfSize;
        if (!OptionalVec2Field(object, "center", params, {0.0, 0.0}, &center, outError)) return false;
        if (!RequiredVec2Field(object, "half_size", params, &halfSize, outError)) return false;
        if (!(halfSize.x >= 0.0) || !(halfSize.y >= 0.0)) {
            if (outError) *outError = "box half_size values must be non-negative";
            return false;
        }
        const Vec2 q{std::fabs(p.x - center.x) - halfSize.x, std::fabs(p.y - center.y) - halfSize.y};
        const Vec2 outside{(std::max)(q.x, 0.0), (std::max)(q.y, 0.0)};
        const double inside = (std::min)((std::max)(q.x, q.y), 0.0);
        *outDistance = Length(outside) + inside;
        return true;
    }
    if (op == "capsule") {
        Vec2 a;
        Vec2 b;
        double radius = 0.0;
        if (!RequiredVec2Field(object, "a", params, &a, outError)) return false;
        if (!RequiredVec2Field(object, "b", params, &b, outError)) return false;
        if (!RequiredScalarField(object, "radius", params, &radius, outError)) return false;
        if (!(radius >= 0.0)) {
            if (outError) *outError = "capsule radius must be non-negative";
            return false;
        }
        const Vec2 pa = Sub(p, a);
        const Vec2 ba = Sub(b, a);
        const double denom = Dot(ba, ba);
        const double h = denom > 0.0 ? std::clamp(Dot(pa, ba) / denom, 0.0, 1.0) : 0.0;
        *outDistance = Length(Sub(pa, Scale(ba, h))) - radius;
        return true;
    }

    auto evalChild = [&](const char* key, Vec2 point, double* out) -> bool {
        auto it = object.find(key);
        if (it == object.end()) {
            if (outError) *outError = std::string("Missing ast child: ") + key;
            return false;
        }
        return EvalAstNode(it->second, point, params, out, outError);
    };

    if (op == "union" || op == "intersect" || op == "subtract" || op == "smooth_union") {
        double da = 0.0;
        double db = 0.0;
        if (!evalChild("a", p, &da)) return false;
        if (!evalChild("b", p, &db)) return false;
        if (op == "union") {
            *outDistance = (std::min)(da, db);
            return true;
        }
        if (op == "intersect") {
            *outDistance = (std::max)(da, db);
            return true;
        }
        if (op == "subtract") {
            *outDistance = (std::max)(da, -db);
            return true;
        }
        double k = 0.0;
        if (!RequiredScalarField(object, "k", params, &k, outError)) return false;
        if (!(k > 0.0)) {
            *outDistance = (std::min)(da, db);
            return true;
        }
        const double h = std::clamp(0.5 + 0.5 * (db - da) / k, 0.0, 1.0);
        *outDistance = db * (1.0 - h) + da * h - k * h * (1.0 - h);
        return true;
    }
    if (op == "translate") {
        Vec2 offset;
        if (!RequiredVec2Field(object, "offset", params, &offset, outError)) return false;
        return evalChild("child", Sub(p, offset), outDistance);
    }
    if (op == "rotate") {
        double angle = 0.0;
        if (!RequiredScalarField(object, "angle", params, &angle, outError)) return false;
        const double c = std::cos(angle);
        const double s = std::sin(angle);
        const Vec2 q{c * p.x + s * p.y, -s * p.x + c * p.y};
        return evalChild("child", q, outDistance);
    }
    if (op == "scale") {
        double factor = 0.0;
        if (!RequiredScalarField(object, "factor", params, &factor, outError)) return false;
        if (!std::isfinite(factor) || std::fabs(factor) < 1.0e-12) {
            if (outError) *outError = "scale factor must be finite and non-zero";
            return false;
        }
        double d = 0.0;
        if (!evalChild("child", Scale(p, 1.0 / factor), &d)) return false;
        *outDistance = d * std::fabs(factor);
        return true;
    }
    if (op == "repeat") {
        Vec2 period;
        if (!RequiredVec2Field(object, "period", params, &period, outError)) return false;
        if (!(period.x > 0.0) || !(period.y > 0.0)) {
            if (outError) *outError = "repeat period values must be positive";
            return false;
        }
        const Vec2 q{
            p.x - period.x * std::round(p.x / period.x),
            p.y - period.y * std::round(p.y / period.y),
        };
        return evalChild("child", q, outDistance);
    }

    if (outError) *outError = "unknown SDF AST op: " + op;
    return false;
}

} // namespace

SdfPackParseResult ParseSdfPackJson(std::string_view text) {
    json_min::ParseResult parsed = json_min::Parse(text);
    if (!parsed.error.empty()) {
        SdfPackParseResult result;
        result.error = parsed.error;
        return result;
    }
    return ParseSdfPackFromValue(parsed.value);
}

SdfPackParseResult ParseSdfPackFromValue(const json_min::Value& value) {
    SdfPackParseResult result;
    if (!value.is_object()) {
        result.error = "SDF pack root must be an object";
        return result;
    }
    const json_min::Object& root = value.as_object();
    if (!RejectUnknownKeys(root, {"schema", "pack_id", "name", "kind", "params", "controls", "region", "ast"}, "root", &result.error)) {
        return result;
    }

    if (!GetRequiredInt(root, "schema", &result.pack.schema, &result.error)) return result;
    if (result.pack.schema != 1) {
        result.error = "SDF pack schema must be 1";
        return result;
    }
    if (!GetRequiredString(root, "pack_id", &result.pack.pack_id, &result.error)) return result;
    if (!GetRequiredString(root, "name", &result.pack.name, &result.error)) return result;
    if (!GetRequiredString(root, "kind", &result.pack.kind, &result.error)) return result;
    if (result.pack.kind != "sdf_scene_2d") {
        result.error = "SDF pack kind must be sdf_scene_2d";
        return result;
    }
    if (!ParseParams(root, &result.pack, &result.error)) return result;
    if (!ParseControls(root, &result.pack, &result.error)) return result;
    if (!ParseRegion(root, &result.pack, &result.error)) return result;

    auto astIt = root.find("ast");
    if (astIt == root.end() || !astIt->second.is_object()) {
        result.error = "ast must be an object";
        return result;
    }
    int nodeCount = 0;
    if (!ValidateAstNode(astIt->second, result.pack.params, &nodeCount, &result.error)) return result;
    result.pack.ast = astIt->second;
    result.ok = true;
    return result;
}

bool BuildSdfPackParamValues(
    const SdfPack& pack,
    const std::map<std::string, double>& overrides,
    std::map<std::string, double>* outValues,
    std::string* outError) {
    std::map<std::string, double> values;
    for (const SdfPackParam& param : pack.params) {
        values[param.id] = param.default_value;
    }
    for (const auto& overrideEntry : overrides) {
        const SdfPackParam* param = FindParam(pack.params, overrideEntry.first);
        if (!param) {
            if (outError) *outError = "unknown param override: " + overrideEntry.first;
            return false;
        }
        if (!std::isfinite(overrideEntry.second)) {
            if (outError) *outError = "param override must be finite: " + overrideEntry.first;
            return false;
        }
        if (overrideEntry.second < param->min_value || overrideEntry.second > param->max_value) {
            if (outError) *outError = "param override outside range: " + overrideEntry.first;
            return false;
        }
        values[overrideEntry.first] = overrideEntry.second;
    }
    if (outValues) *outValues = values;
    return true;
}

SdfPackSampleResult SampleSdfPackCpu(
    const SdfPack& pack,
    double x,
    double y,
    const std::map<std::string, double>& overrides) {
    SdfPackSampleResult result;
    if (!std::isfinite(x) || !std::isfinite(y)) {
        result.error = "sample coordinates must be finite";
        return result;
    }
    if (pack.schema != 1 || pack.kind != "sdf_scene_2d") {
        result.error = "invalid SDF pack";
        return result;
    }
    std::map<std::string, double> params;
    if (!BuildSdfPackParamValues(pack, overrides, &params, &result.error)) return result;
    if (!EvalAstNode(pack.ast, {x, y}, params, &result.distance, &result.error)) return result;
    result.ok = true;
    return result;
}

SdfPackLowerResult LowerSdfPackToRuntimeDesc(
    const SdfPack& pack,
    const std::map<std::string, double>& overrides) {
    SdfPackLowerResult result;
    if (pack.schema != 1 || pack.kind != "sdf_scene_2d") {
        result.error = "invalid SDF pack";
        return result;
    }
    if (!pack.ast.is_object()) {
        result.error = "ast must be an object";
        return result;
    }

    std::map<std::string, double> paramValues;
    if (!BuildSdfPackParamValues(pack, overrides, &paramValues, &result.error)) return result;

    std::map<std::string, int> paramIndices;
    result.desc.param_count = static_cast<int>(pack.params.size());
    if (result.desc.param_count > SDF_PACK_MAX_PARAMS) {
        result.error = "too many params";
        return result;
    }
    for (int i = 0; i < result.desc.param_count; ++i) {
        const SdfPackParam& param = pack.params[static_cast<size_t>(i)];
        paramIndices[param.id] = i;
        result.desc.params[i] = paramValues[param.id];
    }

    int root = -1;
    if (!LowerAstNodeToRuntime(pack.ast, paramIndices, &result.desc, &root, &result.error)) return result;
    result.desc.root_node = root;
    result.ok = true;
    return result;
}
