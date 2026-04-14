#pragma once
// Recursive-descent parser for generic function expressions.
// Converts a string like "iterate(z^2 + c, 500)" into a GenericFunctionDesc.
// No CUDA dependency — usable from .cpp and .cu translation units.

#include "generic_function_types.h"
#include <string>
#include <map>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cmath>

struct GFParseResult {
    GenericFunctionDesc desc;
    bool ok;
    std::string error;
    int error_pos; // 0-based position in expression string
};

namespace gf_parser_detail {

// --- Tokenizer ---

enum class TokKind {
    Number, Ident, Plus, Minus, Star, Slash, Caret,
    LParen, RParen, Comma, End, Error
};

struct Token {
    TokKind kind;
    std::string text;
    double num_value;
    int pos; // position in source
};

struct Lexer {
    const char* src;
    int len;
    int pos;

    Lexer(const char* s, int n) : src(s), len(n), pos(0) {}

    void skip_ws() {
        while (pos < len && (src[pos] == ' ' || src[pos] == '\t' || src[pos] == '\n' || src[pos] == '\r'))
            pos++;
    }

    Token next() {
        skip_ws();
        if (pos >= len) return {TokKind::End, "", 0.0, pos};

        int start = pos;
        char c = src[pos];

        if (c == '+') { pos++; return {TokKind::Plus, "+", 0.0, start}; }
        if (c == '-') { pos++; return {TokKind::Minus, "-", 0.0, start}; }
        if (c == '*') { pos++; return {TokKind::Star, "*", 0.0, start}; }
        if (c == '/') { pos++; return {TokKind::Slash, "/", 0.0, start}; }
        if (c == '^') { pos++; return {TokKind::Caret, "^", 0.0, start}; }
        if (c == '(') { pos++; return {TokKind::LParen, "(", 0.0, start}; }
        if (c == ')') { pos++; return {TokKind::RParen, ")", 0.0, start}; }
        if (c == ',') { pos++; return {TokKind::Comma, ",", 0.0, start}; }

        // Number: digits, optional dot, optional exponent
        if (std::isdigit((unsigned char)c) || (c == '.' && pos + 1 < len && std::isdigit((unsigned char)src[pos + 1]))) {
            int nstart = pos;
            while (pos < len && std::isdigit((unsigned char)src[pos])) pos++;
            if (pos < len && src[pos] == '.') {
                pos++;
                while (pos < len && std::isdigit((unsigned char)src[pos])) pos++;
            }
            if (pos < len && (src[pos] == 'e' || src[pos] == 'E')) {
                pos++;
                if (pos < len && (src[pos] == '+' || src[pos] == '-')) pos++;
                while (pos < len && std::isdigit((unsigned char)src[pos])) pos++;
            }
            std::string numStr(src + nstart, pos - nstart);
            double val = std::strtod(numStr.c_str(), nullptr);
            return {TokKind::Number, numStr, val, nstart};
        }

        // Identifier: letter or underscore, then alphanumeric/underscore
        if (std::isalpha((unsigned char)c) || c == '_') {
            int istart = pos;
            while (pos < len && (std::isalnum((unsigned char)src[pos]) || src[pos] == '_'))
                pos++;
            std::string id(src + istart, pos - istart);
            return {TokKind::Ident, id, 0.0, istart};
        }

        pos++;
        return {TokKind::Error, std::string(1, c), 0.0, start};
    }
};

// --- Parser ---

struct Parser {
    Lexer lexer;
    Token cur;
    GenericFunctionDesc desc;
    const std::map<std::string, double>* params;
    std::string error;
    int error_pos;
    bool failed;

    int recursion_depth;
    static constexpr int kMaxRecursionDepth = 50;

    Parser(const char* src, int len, const std::map<std::string, double>* p)
        : lexer(src, len), params(p), error_pos(0), failed(false), recursion_depth(0)
    {
        std::memset(&desc, 0, sizeof(desc));
        desc.max_iterate = 1;
        advance();
    }

    void advance() { cur = lexer.next(); }

    void fail(const std::string& msg) {
        if (!failed) {
            failed = true;
            error = msg;
            error_pos = cur.pos;
        }
    }

    bool expect(TokKind kind, const char* what) {
        if (cur.kind != kind) {
            fail(std::string("expected ") + what + " at position " + std::to_string(cur.pos));
            return false;
        }
        advance();
        return true;
    }

    int alloc_node() {
        if (desc.node_count >= MAX_GF_NODES) {
            fail("expression too complex (max " + std::to_string(MAX_GF_NODES) + " nodes)");
            return -1;
        }
        return desc.node_count++;
    }

    int alloc_param(double value) {
        if (desc.param_count >= MAX_GF_PARAMS) {
            fail("too many parameters (max " + std::to_string(MAX_GF_PARAMS) + ")");
            return -1;
        }
        int idx = desc.param_count++;
        desc.params[idx] = value;
        return idx;
    }

    // Resolve an identifier to a node index.
    int resolve_ident(const std::string& name) {
        // Built-in variables.
        if (name == "z") {
            int ni = alloc_node();
            if (ni < 0) return -1;
            desc.nodes[ni] = {GFNodeOp::gf_var_z, -1, -1, -1};
            return ni;
        }
        if (name == "z_conj") {
            int ni = alloc_node();
            if (ni < 0) return -1;
            desc.nodes[ni] = {GFNodeOp::gf_var_z_conj, -1, -1, -1};
            return ni;
        }

        // Check for exact param match.
        if (params) {
            auto it = params->find(name);
            if (it != params->end()) {
                int pi = alloc_param(it->second);
                if (pi < 0) return -1;
                int ni = alloc_node();
                if (ni < 0) return -1;
                desc.nodes[ni] = {GFNodeOp::gf_const_real, -1, -1, pi};
                return ni;
            }

            // Check for complex param: name_real + name_imag.
            auto re_it = params->find(name + "_real");
            auto im_it = params->find(name + "_imag");
            if (re_it != params->end() && im_it != params->end()) {
                int pi = alloc_param(re_it->second);
                if (pi < 0) return -1;
                alloc_param(im_it->second); // consecutive
                int ni = alloc_node();
                if (ni < 0) return -1;
                desc.nodes[ni] = {GFNodeOp::gf_const_complex, -1, -1, pi};
                return ni;
            }
        }

        fail("unknown identifier '" + name + "'; valid: z, z_conj, sin, cos, exp, log, abs, conj, iterate, compose, or a param name");
        return -1;
    }

    // --- Grammar rules ---

    // expr := term (('+' | '-') term)*
    int parse_expr() {
        if (++recursion_depth > kMaxRecursionDepth) {
            fail("expression too deeply nested (max depth " + std::to_string(kMaxRecursionDepth) + ")");
            --recursion_depth;
            return -1;
        }
        int left = parse_term();
        if (failed) { --recursion_depth; return -1; }

        while (!failed && (cur.kind == TokKind::Plus || cur.kind == TokKind::Minus)) {
            GFNodeOp op = (cur.kind == TokKind::Plus) ? GFNodeOp::gf_add : GFNodeOp::gf_sub;
            advance();
            int right = parse_term();
            if (failed) { --recursion_depth; return -1; }
            int ni = alloc_node();
            if (ni < 0) { --recursion_depth; return -1; }
            desc.nodes[ni] = {op, left, right, -1};
            left = ni;
        }
        --recursion_depth;
        return left;
    }

    // term := unary (('*' | '/') unary)*
    int parse_term() {
        int left = parse_unary();
        if (failed) return -1;

        while (!failed && (cur.kind == TokKind::Star || cur.kind == TokKind::Slash)) {
            GFNodeOp op = (cur.kind == TokKind::Star) ? GFNodeOp::gf_mul : GFNodeOp::gf_div;
            advance();
            int right = parse_unary();
            if (failed) return -1;
            int ni = alloc_node();
            if (ni < 0) return -1;
            desc.nodes[ni] = {op, left, right, -1};
            left = ni;
        }
        return left;
    }

    // unary := '-' unary | power
    int parse_unary() {
        if (cur.kind == TokKind::Minus) {
            if (++recursion_depth > kMaxRecursionDepth) {
                fail("expression too deeply nested (max depth " + std::to_string(kMaxRecursionDepth) + ")");
                --recursion_depth;
                return -1;
            }
            advance();
            int child = parse_unary();
            --recursion_depth;
            if (failed) return -1;
            int ni = alloc_node();
            if (ni < 0) return -1;
            desc.nodes[ni] = {GFNodeOp::gf_neg, child, -1, -1};
            return ni;
        }
        return parse_power();
    }

    // power := atom ('^' (number | ident))?
    int parse_power() {
        int base = parse_atom();
        if (failed) return -1;

        if (cur.kind == TokKind::Caret) {
            advance();

            // The exponent can be a number or a param name.
            bool neg_exp = false;
            if (cur.kind == TokKind::Minus) {
                neg_exp = true;
                advance();
            }

            double exp_val = 0.0;
            if (cur.kind == TokKind::Number) {
                exp_val = cur.num_value;
                if (neg_exp) exp_val = -exp_val;
                advance();
            } else if (cur.kind == TokKind::Ident) {
                if (params) {
                    auto it = params->find(cur.text);
                    if (it != params->end()) {
                        exp_val = it->second;
                        if (neg_exp) exp_val = -exp_val;
                        advance();
                    } else {
                        fail("unknown exponent parameter '" + cur.text + "'");
                        return -1;
                    }
                } else {
                    fail("no params defined for exponent '" + cur.text + "'");
                    return -1;
                }
            } else {
                fail("expected exponent (number or param name) after '^'");
                return -1;
            }

            int pi = alloc_param(exp_val);
            if (pi < 0) return -1;

            // Use pow_int if exponent is integer, pow_real otherwise.
            bool is_int = (exp_val == std::floor(exp_val)) && std::abs(exp_val) < 1e6;
            GFNodeOp op = is_int ? GFNodeOp::gf_pow_int : GFNodeOp::gf_pow_real;

            int ni = alloc_node();
            if (ni < 0) return -1;
            desc.nodes[ni] = {op, base, -1, pi};
            return ni;
        }
        return base;
    }

    // atom := number | ident | func_call | '(' expr ')'
    int parse_atom() {
        // Number literal.
        if (cur.kind == TokKind::Number) {
            double val = cur.num_value;
            advance();
            int pi = alloc_param(val);
            if (pi < 0) return -1;
            int ni = alloc_node();
            if (ni < 0) return -1;
            desc.nodes[ni] = {GFNodeOp::gf_const_real, -1, -1, pi};
            return ni;
        }

        // Identifier or function call.
        if (cur.kind == TokKind::Ident) {
            std::string name = cur.text;
            advance();

            // Function call: name '(' args ')'
            if (cur.kind == TokKind::LParen) {
                return parse_func_call(name);
            }

            // Plain identifier (variable or param).
            return resolve_ident(name);
        }

        // Parenthesized expression.
        if (cur.kind == TokKind::LParen) {
            advance();
            int inner = parse_expr();
            if (failed) return -1;
            if (!expect(TokKind::RParen, "')'")) return -1;
            return inner;
        }

        fail("unexpected token '" + cur.text + "' at position " + std::to_string(cur.pos));
        return -1;
    }

    // func_call: we already consumed name and see '('.
    int parse_func_call(const std::string& name) {
        advance(); // consume '('

        // Unary functions: sin, cos, exp, log, abs, conj
        if (name == "sin" || name == "cos" || name == "exp" || name == "log" ||
            name == "abs" || name == "conj") {
            int arg = parse_expr();
            if (failed) return -1;
            if (!expect(TokKind::RParen, "')' after function argument")) return -1;

            GFNodeOp op;
            if      (name == "sin")  op = GFNodeOp::gf_sin;
            else if (name == "cos")  op = GFNodeOp::gf_cos;
            else if (name == "exp")  op = GFNodeOp::gf_exp;
            else if (name == "log")  op = GFNodeOp::gf_log;
            else if (name == "abs")  op = GFNodeOp::gf_abs;
            else                     op = GFNodeOp::gf_conj;

            int ni = alloc_node();
            if (ni < 0) return -1;
            desc.nodes[ni] = {op, arg, -1, -1};
            return ni;
        }

        // iterate(body, N)
        if (name == "iterate") {
            int body = parse_expr();
            if (failed) return -1;
            if (cur.kind != TokKind::Comma) {
                fail("iterate() requires two arguments: iterate(body, count)");
                return -1;
            }
            advance(); // consume ','

            if (cur.kind != TokKind::Number) {
                fail("iterate() count must be an integer literal");
                return -1;
            }
            int count = (int)cur.num_value;
            if (count <= 0) count = 1;
            if (count > 10000) count = 10000;
            advance();

            if (!expect(TokKind::RParen, "')' after iterate arguments")) return -1;

            int pi = alloc_param((double)count);
            if (pi < 0) return -1;

            // Update max_iterate as well.
            desc.max_iterate = count;

            int ni = alloc_node();
            if (ni < 0) return -1;
            desc.nodes[ni] = {GFNodeOp::gf_iterate, body, -1, pi};
            return ni;
        }

        // compose(f, g)
        if (name == "compose") {
            int f = parse_expr();
            if (failed) return -1;
            if (cur.kind != TokKind::Comma) {
                fail("compose() requires two arguments: compose(f, g)");
                return -1;
            }
            advance(); // consume ','
            int g = parse_expr();
            if (failed) return -1;
            if (!expect(TokKind::RParen, "')' after compose arguments")) return -1;

            int ni = alloc_node();
            if (ni < 0) return -1;
            desc.nodes[ni] = {GFNodeOp::gf_compose, f, g, -1};
            return ni;
        }

        fail("unknown function '" + name + "'; valid: sin, cos, exp, log, abs, conj, iterate, compose");
        return -1;
    }

    GFParseResult run() {
        int root = parse_expr();
        if (failed) return {desc, false, error, error_pos};
        if (cur.kind != TokKind::End) {
            fail("unexpected trailing input at position " + std::to_string(cur.pos));
            return {desc, false, error, error_pos};
        }
        desc.root_node = root;

        // Validate the built descriptor.
        GFValidationResult val = ValidateGenericFunctionDesc(desc);
        if (!val.valid) {
            return {desc, false, val.error ? val.error : "invalid expression tree", 0};
        }

        return {desc, true, "", 0};
    }
};

} // namespace gf_parser_detail

// --- Public API ---

inline GFParseResult ParseGenericFunctionExpression(
    const std::string& expression,
    const std::map<std::string, double>& params)
{
    gf_parser_detail::Parser parser(expression.c_str(), (int)expression.size(), &params);
    return parser.run();
}
