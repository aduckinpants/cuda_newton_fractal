#include "generic_equation_pack.h"

#include <cmath>
#include <cstring>
#include <set>

namespace {

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

bool GetOptionalInt(const json_min::Object& object,
    const char* key,
    int* outValue,
    bool* outPresent,
    std::string* outError) {
    auto it = object.find(key);
    if (it == object.end()) {
        if (outPresent) *outPresent = false;
        return true;
    }
    if (!it->second.is_number() || !std::isfinite(it->second.as_number()) ||
        std::floor(it->second.as_number()) != it->second.as_number()) {
        if (outError) *outError = std::string("Expected integer field: ") + key;
        return false;
    }
    if (outValue) *outValue = static_cast<int>(it->second.as_number());
    if (outPresent) *outPresent = true;
    return true;
}

bool ParseParams(const json_min::Object& root,
    std::map<std::string, double>* outParams,
    std::string* outError) {
    auto paramsIt = root.find("params");
    if (paramsIt == root.end()) return true;
    if (!paramsIt->second.is_object()) {
        if (outError) *outError = "params must be an object";
        return false;
    }
    for (const auto& kv : paramsIt->second.as_object()) {
        if (!kv.second.is_number() || !std::isfinite(kv.second.as_number())) {
            if (outError) *outError = "params values must be finite numbers";
            return false;
        }
        (*outParams)[kv.first] = kv.second.as_number();
    }
    return true;
}

bool ParseControls(const json_min::Object& root,
    std::vector<GenericEquationPackControl>* outControls,
    std::string* outError) {
    auto controlsIt = root.find("controls");
    if (controlsIt == root.end()) return true;
    if (!controlsIt->second.is_array()) {
        if (outError) *outError = "controls must be an array";
        return false;
    }

    std::set<std::string> ids;
    std::set<std::string> params;
    for (const json_min::Value& value : controlsIt->second.as_array()) {
        if (!value.is_object()) {
            if (outError) *outError = "controls entries must be objects";
            return false;
        }
        const json_min::Object& object = value.as_object();
        if (!RejectUnknownKeys(object, {"id", "param", "label", "min", "max", "step", "default"}, "control", outError)) {
            return false;
        }

        GenericEquationPackControl control;
        if (!GetRequiredString(object, "id", &control.id, outError)) return false;
        if (!GetRequiredString(object, "param", &control.param, outError)) return false;
        if (!GetOptionalString(object, "label", &control.label, outError)) return false;
        if (!ids.insert(control.id).second) {
            if (outError) *outError = "duplicate control id: " + control.id;
            return false;
        }
        if (!params.insert(control.param).second) {
            if (outError) *outError = "duplicate control param: " + control.param;
            return false;
        }

        if (!GetOptionalFiniteNumber(object, "min", &control.min_value, &control.has_min, outError)) return false;
        if (!GetOptionalFiniteNumber(object, "max", &control.max_value, &control.has_max, outError)) return false;
        if (!GetOptionalFiniteNumber(object, "step", &control.step_value, &control.has_step, outError)) return false;
        if (!GetOptionalFiniteNumber(object, "default", &control.default_value, &control.has_default_value, outError)) return false;
        outControls->push_back(control);
    }
    return true;
}

bool ParseRegion(const json_min::Object& root,
    GenericEquationPackRegion* outRegion,
    std::string* outError) {
    auto regionIt = root.find("region");
    if (regionIt == root.end()) return true;
    if (!regionIt->second.is_object()) {
        if (outError) *outError = "region must be an object";
        return false;
    }
    const json_min::Object& object = regionIt->second.as_object();
    if (!RejectUnknownKeys(object,
            {"center_x", "center_y", "span_x", "span_y", "grid_width", "grid_height"},
            "region",
            outError)) return false;

    GenericEquationPackRegion region;
    region.has_region = true;
    if (!GetRequiredFiniteNumber(object, "center_x", &region.center_x, outError)) return false;
    if (!GetRequiredFiniteNumber(object, "center_y", &region.center_y, outError)) return false;
    if (!GetRequiredFiniteNumber(object, "span_x", &region.span_x, outError)) return false;
    if (!GetRequiredFiniteNumber(object, "span_y", &region.span_y, outError)) return false;
    if (!GetRequiredInt(object, "grid_width", &region.grid_width, outError)) return false;
    if (!GetRequiredInt(object, "grid_height", &region.grid_height, outError)) return false;
    if (!(region.span_x > 0.0) || !(region.span_y > 0.0) || region.grid_width <= 0 || region.grid_height <= 0) {
        if (outError) *outError = "region spans and grid dimensions must be positive";
        return false;
    }
    if (outRegion) *outRegion = region;
    return true;
}

bool ParseFormula(const json_min::Object& root,
    GenericEquationPack* ioPack,
    std::string* outError) {
    auto formulaIt = root.find("formula");
    if (formulaIt == root.end() || !formulaIt->second.is_object()) {
        if (outError) *outError = "formula must be an object";
        return false;
    }
    const json_min::Object& formula = formulaIt->second.as_object();
    if (!RejectUnknownKeys(formula, {"kind", "ast", "iteration_param"}, "formula", outError)) {
        return false;
    }
    if (!GetRequiredString(formula, "kind", &ioPack->formula_kind, outError)) return false;
    if (ioPack->formula_kind != "direct" && ioPack->formula_kind != "iterate_map") {
        if (outError) *outError = "formula.kind must be direct or iterate_map";
        return false;
    }
    auto astIt = formula.find("ast");
    if (astIt == formula.end() || !astIt->second.is_object()) {
        if (outError) *outError = "formula.ast must be an object";
        return false;
    }
    ioPack->ast = astIt->second;
    if (ioPack->formula_kind == "iterate_map") {
        return GetRequiredString(formula, "iteration_param", &ioPack->iteration_param, outError);
    }
    return GetOptionalString(formula, "iteration_param", &ioPack->iteration_param, outError);
}

class AstLowerer {
public:
    explicit AstLowerer(const std::map<std::string, double>& sourceParams)
        : sourceParams_(sourceParams) {
        std::memset(&desc_, 0, sizeof(desc_));
        desc_.max_iterate = 1;
        for (int i = 0; i < MAX_GF_NODES; ++i) {
            desc_.nodes[i] = {GFNodeOp::gf_var_z, -1, -1, -1};
        }
    }

    GenericEquationLowerResult LowerDirect(const json_min::Value& ast) {
        int root = LowerNode(ast, 0);
        if (root < 0) return FailResult();
        desc_.root_node = root;
        return Finish();
    }

    GenericEquationLowerResult LowerIterate(const json_min::Value& ast, const std::string& iterationParam) {
        auto it = sourceParams_.find(iterationParam);
        if (it == sourceParams_.end()) {
            error_ = "unknown iteration_param: " + iterationParam;
            return FailResult();
        }
        if (!std::isfinite(it->second)) {
            error_ = "iteration_param value must be finite: " + iterationParam;
            return FailResult();
        }

        int body = LowerNode(ast, 0);
        if (body < 0) return FailResult();
        int countParam = AllocParam(it->second, "iteration_param " + iterationParam);
        if (countParam < 0) return FailResult();
        int iterate = AllocNode(GFNodeOp::gf_iterate, body, -1, countParam);
        if (iterate < 0) return FailResult();
        desc_.max_iterate = static_cast<int>(it->second);
        desc_.root_node = iterate;
        return Finish();
    }

private:
    int AllocParam(double value, const std::string& context) {
        if (!std::isfinite(value)) {
            error_ = context + " must be finite";
            return -1;
        }
        if (desc_.param_count >= MAX_GF_PARAMS) {
            error_ = "too many parameters (max " + std::to_string(MAX_GF_PARAMS) + ")";
            return -1;
        }
        int index = desc_.param_count++;
        desc_.params[index] = value;
        return index;
    }

    int AllocNode(GFNodeOp op, int childLeft, int childRight, int paramIndex) {
        if (desc_.node_count >= MAX_GF_NODES) {
            error_ = "AST too complex (max " + std::to_string(MAX_GF_NODES) + " nodes)";
            return -1;
        }
        int index = desc_.node_count++;
        desc_.nodes[index] = {op, childLeft, childRight, paramIndex};
        return index;
    }

    int ResolveScalarParam(const std::string& name) {
        auto it = sourceParams_.find(name);
        if (it == sourceParams_.end()) {
            error_ = "unknown scalar param: " + name;
            return -1;
        }
        return AllocParam(it->second, "param " + name);
    }

    int ResolveComplexParam(const std::string& name) {
        auto realIt = sourceParams_.find(name + "_real");
        auto imagIt = sourceParams_.find(name + "_imag");
        if (realIt == sourceParams_.end() || imagIt == sourceParams_.end()) {
            error_ = "unknown complex_param: " + name + " (requires " + name + "_real and " + name + "_imag)";
            return -1;
        }
        int realIndex = AllocParam(realIt->second, "complex_param " + name + "_real");
        if (realIndex < 0) return -1;
        int imagIndex = AllocParam(imagIt->second, "complex_param " + name + "_imag");
        if (imagIndex < 0) return -1;
        return realIndex;
    }

    bool RequireObject(const json_min::Value& value, const json_min::Object** outObject) {
        if (!value.is_object()) {
            error_ = "AST node must be an object";
            return false;
        }
        *outObject = &value.as_object();
        return true;
    }

    bool RequireStringField(const json_min::Object& object,
        const char* key,
        std::string* outValue) {
        auto it = object.find(key);
        if (it == object.end() || !it->second.is_string()) {
            error_ = std::string("AST node missing string field: ") + key;
            return false;
        }
        *outValue = it->second.as_string();
        return true;
    }

    bool RequireFiniteNumberField(const json_min::Object& object,
        const char* key,
        double* outValue) {
        auto it = object.find(key);
        if (it == object.end() || !it->second.is_number() || !std::isfinite(it->second.as_number())) {
            error_ = std::string("AST node missing finite number field: ") + key;
            return false;
        }
        *outValue = it->second.as_number();
        return true;
    }

    bool FindNodeField(const json_min::Object& object,
        const char* key,
        const json_min::Value** outValue) {
        auto it = object.find(key);
        if (it == object.end()) {
            error_ = std::string("AST node missing field: ") + key;
            return false;
        }
        *outValue = &it->second;
        return true;
    }

    int LowerUnary(const json_min::Object& object, GFNodeOp op, int depth) {
        const json_min::Value* arg = nullptr;
        if (!FindNodeField(object, "arg", &arg)) return -1;
        int child = LowerNode(*arg, depth + 1);
        if (child < 0) return -1;
        return AllocNode(op, child, -1, -1);
    }

    int LowerBinary(const json_min::Object& object, GFNodeOp op, int depth) {
        auto argsIt = object.find("args");
        if (argsIt == object.end() || !argsIt->second.is_array() || argsIt->second.as_array().size() != 2) {
            error_ = "binary AST op requires args array of length 2";
            return -1;
        }
        int left = LowerNode(argsIt->second.as_array()[0], depth + 1);
        if (left < 0) return -1;
        int right = LowerNode(argsIt->second.as_array()[1], depth + 1);
        if (right < 0) return -1;
        return AllocNode(op, left, right, -1);
    }

    int LowerPow(const json_min::Object& object, GFNodeOp op, int depth) {
        const json_min::Value* base = nullptr;
        if (!FindNodeField(object, "base", &base)) return -1;
        int child = LowerNode(*base, depth + 1);
        if (child < 0) return -1;

        double exponent = 0.0;
        auto exponentIt = object.find("exponent");
        auto exponentParamIt = object.find("exponent_param");
        if (exponentIt != object.end() && exponentParamIt != object.end()) {
            error_ = "pow AST node must use exponent or exponent_param, not both";
            return -1;
        }
        if (exponentIt != object.end()) {
            if (!exponentIt->second.is_number() || !std::isfinite(exponentIt->second.as_number())) {
                error_ = "pow exponent must be finite";
                return -1;
            }
            exponent = exponentIt->second.as_number();
        } else if (exponentParamIt != object.end()) {
            if (!exponentParamIt->second.is_string()) {
                error_ = "pow exponent_param must be a string";
                return -1;
            }
            auto sourceIt = sourceParams_.find(exponentParamIt->second.as_string());
            if (sourceIt == sourceParams_.end()) {
                error_ = "unknown pow exponent_param: " + exponentParamIt->second.as_string();
                return -1;
            }
            exponent = sourceIt->second;
            if (!std::isfinite(exponent)) {
                error_ = "pow exponent_param value must be finite";
                return -1;
            }
        } else {
            error_ = "pow AST node requires exponent or exponent_param";
            return -1;
        }

        if (op == GFNodeOp::gf_pow_int && std::floor(exponent) != exponent) {
            error_ = "pow_int exponent must be an integer";
            return -1;
        }
        int exponentParam = AllocParam(exponent, "pow exponent");
        if (exponentParam < 0) return -1;
        return AllocNode(op, child, -1, exponentParam);
    }

    int LowerNode(const json_min::Value& value, int depth) {
        if (depth > MAX_GF_NODES) {
            error_ = "AST too deeply nested";
            return -1;
        }
        const json_min::Object* object = nullptr;
        if (!RequireObject(value, &object)) return -1;
        std::string op;
        if (!RequireStringField(*object, "op", &op)) return -1;

        if (op == "var_z") {
            return AllocNode(GFNodeOp::gf_var_z, -1, -1, -1);
        }
        if (op == "var_z_conj") {
            return AllocNode(GFNodeOp::gf_var_z_conj, -1, -1, -1);
        }
        if (op == "const") {
            double valueNumber = 0.0;
            if (!RequireFiniteNumberField(*object, "value", &valueNumber)) return -1;
            int param = AllocParam(valueNumber, "const value");
            if (param < 0) return -1;
            return AllocNode(GFNodeOp::gf_const_real, -1, -1, param);
        }
        if (op == "param") {
            std::string name;
            if (!RequireStringField(*object, "name", &name)) return -1;
            int param = ResolveScalarParam(name);
            if (param < 0) return -1;
            return AllocNode(GFNodeOp::gf_const_real, -1, -1, param);
        }
        if (op == "complex_param") {
            std::string name;
            if (!RequireStringField(*object, "name", &name)) return -1;
            int param = ResolveComplexParam(name);
            if (param < 0) return -1;
            return AllocNode(GFNodeOp::gf_const_complex, -1, -1, param);
        }

        if (op == "add") return LowerBinary(*object, GFNodeOp::gf_add, depth);
        if (op == "sub") return LowerBinary(*object, GFNodeOp::gf_sub, depth);
        if (op == "mul") return LowerBinary(*object, GFNodeOp::gf_mul, depth);
        if (op == "div") return LowerBinary(*object, GFNodeOp::gf_div, depth);
        if (op == "compose") return LowerBinary(*object, GFNodeOp::gf_compose, depth);
        if (op == "neg") return LowerUnary(*object, GFNodeOp::gf_neg, depth);
        if (op == "conj") return LowerUnary(*object, GFNodeOp::gf_conj, depth);
        if (op == "abs") return LowerUnary(*object, GFNodeOp::gf_abs, depth);
        if (op == "sin") return LowerUnary(*object, GFNodeOp::gf_sin, depth);
        if (op == "cos") return LowerUnary(*object, GFNodeOp::gf_cos, depth);
        if (op == "exp") return LowerUnary(*object, GFNodeOp::gf_exp, depth);
        if (op == "log") return LowerUnary(*object, GFNodeOp::gf_log, depth);
        if (op == "pow_int") return LowerPow(*object, GFNodeOp::gf_pow_int, depth);
        if (op == "pow_real") return LowerPow(*object, GFNodeOp::gf_pow_real, depth);

        error_ = "unsupported AST op: " + op;
        return -1;
    }

    GenericEquationLowerResult Finish() {
        GFValidationResult validation = ValidateGenericFunctionDesc(desc_);
        if (!validation.valid) {
            GenericEquationLowerResult result;
            result.desc = desc_;
            result.ok = false;
            result.error = validation.error ? validation.error : "invalid GenericFunctionDesc";
            return result;
        }
        GenericEquationLowerResult result;
        result.desc = desc_;
        result.ok = true;
        return result;
    }

    GenericEquationLowerResult FailResult() const {
        GenericEquationLowerResult result;
        result.desc = desc_;
        result.ok = false;
        result.error = error_;
        return result;
    }

    const std::map<std::string, double>& sourceParams_;
    GenericFunctionDesc desc_{};
    std::string error_;
};

} // namespace

GenericEquationPackParseResult ParseGenericEquationPackJson(std::string_view text) {
    json_min::ParseResult parsed = json_min::Parse(text);
    if (!parsed.error.empty()) {
        GenericEquationPackParseResult result;
        result.ok = false;
        result.error = "JSON parse error: " + parsed.error;
        return result;
    }
    return ParseGenericEquationPackFromValue(parsed.value);
}

GenericEquationPackParseResult ParseGenericEquationPackFromValue(const json_min::Value& value) {
    GenericEquationPackParseResult result;
    if (!value.is_object()) {
        result.error = "equation pack must be an object";
        return result;
    }

    const json_min::Object& root = value.as_object();
    std::string error;
    if (!RejectUnknownKeys(root,
            {"schema_version", "pack_id", "name", "formula", "params", "controls", "epsilon", "escape_radius", "region"},
            "equation pack",
            &error)) {
        result.error = error;
        return result;
    }

    GenericEquationPack pack;
    if (!GetRequiredInt(root, "schema_version", &pack.schema_version, &error)) {
        result.error = error;
        return result;
    }
    if (pack.schema_version != 1) {
        result.error = "unsupported equation pack schema_version: " + std::to_string(pack.schema_version);
        return result;
    }
    if (!GetRequiredString(root, "pack_id", &pack.pack_id, &error)) {
        result.error = error;
        return result;
    }
    if (!GetRequiredString(root, "name", &pack.name, &error)) {
        result.error = error;
        return result;
    }

    if (!ParseFormula(root, &pack, &error)) {
        result.error = error;
        return result;
    }
    if (!ParseParams(root, &pack.params, &error)) {
        result.error = error;
        return result;
    }
    if (!ParseControls(root, &pack.controls, &error)) {
        result.error = error;
        return result;
    }
    bool present = false;
    if (!GetOptionalFiniteNumber(root, "epsilon", &pack.epsilon, &present, &error)) {
        result.error = error;
        return result;
    }
    if (present && !(pack.epsilon > 0.0)) {
        result.error = "epsilon must be positive";
        return result;
    }
    if (!GetOptionalFiniteNumber(root, "escape_radius", &pack.escape_radius, &present, &error)) {
        result.error = error;
        return result;
    }
    if (present && !(pack.escape_radius > 0.0)) {
        result.error = "escape_radius must be positive";
        return result;
    }
    if (!ParseRegion(root, &pack.region, &error)) {
        result.error = error;
        return result;
    }

    result.pack = pack;
    result.ok = true;
    return result;
}

GenericEquationLowerResult LowerGenericEquationPackToDesc(const GenericEquationPack& pack) {
    if (pack.formula_kind == "direct") {
        return LowerGenericEquationAstToDesc(pack.ast, pack.params);
    }
    if (pack.formula_kind == "iterate_map") {
        return LowerGenericEquationAstToDesc(pack.ast, pack.params, pack.iteration_param);
    }
    GenericEquationLowerResult result;
    result.ok = false;
    result.error = "formula.kind must be direct or iterate_map";
    return result;
}

GenericEquationLowerResult LowerGenericEquationAstToDesc(
    const json_min::Value& ast,
    const std::map<std::string, double>& params) {
    AstLowerer lowerer(params);
    return lowerer.LowerDirect(ast);
}

GenericEquationLowerResult LowerGenericEquationAstToDesc(
    const json_min::Value& ast,
    const std::map<std::string, double>& params,
    const std::string& iterationParam) {
    AstLowerer lowerer(params);
    return lowerer.LowerIterate(ast, iterationParam);
}
