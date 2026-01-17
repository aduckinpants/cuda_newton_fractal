#include "json_min.h"

#include <cctype>
#include <charconv>
#include <cstdlib>

namespace json_min {

static inline void skip_ws(std::string_view s, size_t& i) {
    while (i < s.size() && (s[i] == ' ' || s[i] == '\t' || s[i] == '\n' || s[i] == '\r')) i++;
}

bool Value::is_null() const { return std::holds_alternative<std::nullptr_t>(v); }
bool Value::is_bool() const { return std::holds_alternative<bool>(v); }
bool Value::is_number() const { return std::holds_alternative<double>(v); }
bool Value::is_string() const { return std::holds_alternative<std::string>(v); }
bool Value::is_array() const { return std::holds_alternative<Array>(v); }
bool Value::is_object() const { return std::holds_alternative<Object>(v); }

const bool& Value::as_bool() const { return std::get<bool>(v); }
const double& Value::as_number() const { return std::get<double>(v); }
const std::string& Value::as_string() const { return std::get<std::string>(v); }
const Array& Value::as_array() const { return std::get<Array>(v); }
const Object& Value::as_object() const { return std::get<Object>(v); }

const Value* Value::get(std::string_view key) const {
    if (!is_object()) return nullptr;
    const auto& o = as_object();
    auto it = o.find(std::string(key));
    if (it == o.end()) return nullptr;
    return &it->second;
}

static bool parse_literal(std::string_view s, size_t& i, std::string_view lit) {
    if (s.substr(i, lit.size()) == lit) {
        i += lit.size();
        return true;
    }
    return false;
}

static bool parse_string(std::string_view s, size_t& i, std::string& out, std::string& err) {
    if (i >= s.size() || s[i] != '"') {
        err = "Expected string opening quote";
        return false;
    }
    i++; // skip quote
    out.clear();
    while (i < s.size()) {
        char c = s[i++];
        if (c == '"') return true;
        if (c == '\\') {
            if (i >= s.size()) {
                err = "Invalid escape (eof)";
                return false;
            }
            char e = s[i++];
            switch (e) {
            case '"': out.push_back('"'); break;
            case '\\': out.push_back('\\'); break;
            case '/': out.push_back('/'); break;
            case 'b': out.push_back('\b'); break;
            case 'f': out.push_back('\f'); break;
            case 'n': out.push_back('\n'); break;
            case 'r': out.push_back('\r'); break;
            case 't': out.push_back('\t'); break;
            case 'u':
                // Minimal parser: we do not support \uXXXX; reject to avoid mis-decoding.
                err = "\\uXXXX escapes not supported in this minimal JSON parser";
                return false;
            default:
                err = "Unknown escape sequence";
                return false;
            }
        } else {
            out.push_back(c);
        }
    }
    err = "Unterminated string";
    return false;
}

static bool parse_number(std::string_view s, size_t& i, double& out, std::string& err) {
    size_t start = i;
    if (i < s.size() && (s[i] == '-' || s[i] == '+')) i++;

    bool any = false;
    while (i < s.size() && std::isdigit((unsigned char)s[i])) {
        i++;
        any = true;
    }

    if (i < s.size() && s[i] == '.') {
        i++;
        while (i < s.size() && std::isdigit((unsigned char)s[i])) {
            i++;
            any = true;
        }
    }

    if (!any) {
        err = "Invalid number";
        return false;
    }

    if (i < s.size() && (s[i] == 'e' || s[i] == 'E')) {
        i++;
        if (i < s.size() && (s[i] == '-' || s[i] == '+')) i++;
        bool expAny = false;
        while (i < s.size() && std::isdigit((unsigned char)s[i])) {
            i++;
            expAny = true;
        }
        if (!expAny) {
            err = "Invalid exponent";
            return false;
        }
    }

    std::string tmp(s.substr(start, i - start));
    char* endp = nullptr;
    out = std::strtod(tmp.c_str(), &endp);
    if (!endp || *endp != '\0') {
        err = "Number parse failed";
        return false;
    }
    return true;
}

static bool parse_value(std::string_view s, size_t& i, Value& out, std::string& err);

static bool parse_array(std::string_view s, size_t& i, Array& out, std::string& err) {
    if (i >= s.size() || s[i] != '[') {
        err = "Expected '['";
        return false;
    }
    i++;
    skip_ws(s, i);
    out.clear();

    if (i < s.size() && s[i] == ']') {
        i++;
        return true;
    }

    while (i < s.size()) {
        Value v;
        if (!parse_value(s, i, v, err)) return false;
        out.push_back(std::move(v));
        skip_ws(s, i);
        if (i >= s.size()) {
            err = "Unterminated array";
            return false;
        }
        if (s[i] == ',') {
            i++;
            skip_ws(s, i);
            continue;
        }
        if (s[i] == ']') {
            i++;
            return true;
        }
        err = "Expected ',' or ']'";
        return false;
    }

    err = "Unterminated array";
    return false;
}

static bool parse_object(std::string_view s, size_t& i, Object& out, std::string& err) {
    if (i >= s.size() || s[i] != '{') {
        err = "Expected '{'";
        return false;
    }
    i++;
    skip_ws(s, i);
    out.clear();

    if (i < s.size() && s[i] == '}') {
        i++;
        return true;
    }

    while (i < s.size()) {
        std::string key;
        if (!parse_string(s, i, key, err)) return false;
        skip_ws(s, i);
        if (i >= s.size() || s[i] != ':') {
            err = "Expected ':'";
            return false;
        }
        i++;
        skip_ws(s, i);

        Value v;
        if (!parse_value(s, i, v, err)) return false;

        out.emplace(std::move(key), std::move(v));

        skip_ws(s, i);
        if (i >= s.size()) {
            err = "Unterminated object";
            return false;
        }
        if (s[i] == ',') {
            i++;
            skip_ws(s, i);
            continue;
        }
        if (s[i] == '}') {
            i++;
            return true;
        }
        err = "Expected ',' or '}'";
        return false;
    }

    err = "Unterminated object";
    return false;
}

static bool parse_value(std::string_view s, size_t& i, Value& out, std::string& err) {
    skip_ws(s, i);
    if (i >= s.size()) {
        err = "Unexpected end of input";
        return false;
    }

    char c = s[i];
    if (c == 'n') {
        if (!parse_literal(s, i, "null")) {
            err = "Invalid literal";
            return false;
        }
        out.v = nullptr;
        return true;
    }
    if (c == 't') {
        if (!parse_literal(s, i, "true")) {
            err = "Invalid literal";
            return false;
        }
        out.v = true;
        return true;
    }
    if (c == 'f') {
        if (!parse_literal(s, i, "false")) {
            err = "Invalid literal";
            return false;
        }
        out.v = false;
        return true;
    }
    if (c == '"') {
        std::string str;
        if (!parse_string(s, i, str, err)) return false;
        out.v = std::move(str);
        return true;
    }
    if (c == '[') {
        Array arr;
        if (!parse_array(s, i, arr, err)) return false;
        out.v = std::move(arr);
        return true;
    }
    if (c == '{') {
        Object obj;
        if (!parse_object(s, i, obj, err)) return false;
        out.v = std::move(obj);
        return true;
    }

    // number
    double num = 0.0;
    if (!parse_number(s, i, num, err)) return false;
    out.v = num;
    return true;
}

ParseResult Parse(std::string_view input) {
    ParseResult r;
    size_t i = 0;
    std::string err;
    Value v;
    if (!parse_value(input, i, v, err)) {
        r.error = err;
        return r;
    }
    skip_ws(input, i);
    if (i != input.size()) {
        r.error = "Trailing characters after JSON";
        return r;
    }
    r.value = std::move(v);
    return r;
}

} // namespace json_min
