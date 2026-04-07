// test_json_min.cpp
// Edge-case tests for the minimal JSON parser (json_min.h/cpp).
// Covers: valid inputs, malformed inputs, error messages, type accessors,
// boundary conditions, and the explicit \uXXXX rejection.

#include "../src/json_min.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <string>

static int g_passed = 0;
static int g_failed = 0;

#define ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::fprintf(stderr, "%s:%d: FAIL: %s\n", __FILE__, __LINE__, (msg)); \
            g_failed++; \
            return false; \
        } \
        g_passed++; \
    } while (0)

using namespace json_min;

// --- Valid inputs ---

bool TestParseNull() {
    auto r = Parse("null");
    ASSERT(r.error.empty(), "null should parse without error");
    ASSERT(r.value.is_null(), "should be null");
    return true;
}

bool TestParseTrue() {
    auto r = Parse("true");
    ASSERT(r.error.empty(), "true should parse without error");
    ASSERT(r.value.is_bool(), "should be bool");
    ASSERT(r.value.as_bool() == true, "should be true");
    return true;
}

bool TestParseFalse() {
    auto r = Parse("false");
    ASSERT(r.error.empty(), "false should parse without error");
    ASSERT(r.value.is_bool(), "should be bool");
    ASSERT(r.value.as_bool() == false, "should be false");
    return true;
}

bool TestParseInteger() {
    auto r = Parse("42");
    ASSERT(r.error.empty(), "integer should parse");
    ASSERT(r.value.is_number(), "should be number");
    ASSERT(r.value.as_number() == 42.0, "value should be 42");
    return true;
}

bool TestParseNegativeInteger() {
    auto r = Parse("-7");
    ASSERT(r.error.empty(), "negative int should parse");
    ASSERT(r.value.as_number() == -7.0, "value should be -7");
    return true;
}

bool TestParseFloat() {
    auto r = Parse("3.14159");
    ASSERT(r.error.empty(), "float should parse");
    ASSERT(std::fabs(r.value.as_number() - 3.14159) < 1e-10, "value should be ~pi");
    return true;
}

bool TestParseScientific() {
    auto r = Parse("1.5e10");
    ASSERT(r.error.empty(), "scientific should parse");
    ASSERT(std::fabs(r.value.as_number() - 1.5e10) < 1e3, "value should be 1.5e10");
    return true;
}

bool TestParseScientificNegExp() {
    auto r = Parse("2.5E-3");
    ASSERT(r.error.empty(), "scientific negative exp should parse");
    ASSERT(std::fabs(r.value.as_number() - 0.0025) < 1e-10, "value should be 0.0025");
    return true;
}

bool TestParseString() {
    auto r = Parse("\"hello world\"");
    ASSERT(r.error.empty(), "string should parse");
    ASSERT(r.value.is_string(), "should be string");
    ASSERT(r.value.as_string() == "hello world", "content should match");
    return true;
}

bool TestParseStringEscapes() {
    auto r = Parse("\"line1\\nline2\\ttab\\\\backslash\\\"quote\"");
    ASSERT(r.error.empty(), "escaped string should parse");
    ASSERT(r.value.as_string() == "line1\nline2\ttab\\backslash\"quote", "escapes decoded");
    return true;
}

bool TestParseStringAllEscapes() {
    // Test all supported escape sequences: \", \\, \/, \b, \f, \n, \r, \t
    auto r = Parse("\"\\\"\\\\\\b\\f\\n\\r\\t\\/\"");
    ASSERT(r.error.empty(), "all escapes should parse");
    std::string expected;
    expected.push_back('"');
    expected.push_back('\\');
    expected.push_back('\b');
    expected.push_back('\f');
    expected.push_back('\n');
    expected.push_back('\r');
    expected.push_back('\t');
    expected.push_back('/');
    ASSERT(r.value.as_string() == expected, "all escape values correct");
    return true;
}

bool TestParseEmptyString() {
    auto r = Parse("\"\"");
    ASSERT(r.error.empty(), "empty string should parse");
    ASSERT(r.value.as_string().empty(), "should be empty");
    return true;
}

bool TestParseEmptyArray() {
    auto r = Parse("[]");
    ASSERT(r.error.empty(), "empty array should parse");
    ASSERT(r.value.is_array(), "should be array");
    ASSERT(r.value.as_array().size() == 0, "should have 0 elements");
    return true;
}

bool TestParseArray() {
    auto r = Parse("[1, \"two\", true, null]");
    ASSERT(r.error.empty(), "mixed array should parse");
    ASSERT(r.value.as_array().size() == 4, "4 elements");
    ASSERT(r.value.as_array()[0].as_number() == 1.0, "first is 1");
    ASSERT(r.value.as_array()[1].as_string() == "two", "second is \"two\"");
    ASSERT(r.value.as_array()[2].as_bool() == true, "third is true");
    ASSERT(r.value.as_array()[3].is_null(), "fourth is null");
    return true;
}

bool TestParseNestedArray() {
    auto r = Parse("[[1,2],[3,[4,5]]]");
    ASSERT(r.error.empty(), "nested array should parse");
    auto& outer = r.value.as_array();
    ASSERT(outer.size() == 2, "2 outer elements");
    ASSERT(outer[0].as_array().size() == 2, "first inner has 2");
    ASSERT(outer[1].as_array()[1].as_array()[0].as_number() == 4.0, "deep value is 4");
    return true;
}

bool TestParseEmptyObject() {
    auto r = Parse("{}");
    ASSERT(r.error.empty(), "empty object should parse");
    ASSERT(r.value.is_object(), "should be object");
    ASSERT(r.value.as_object().size() == 0, "should have 0 keys");
    return true;
}

bool TestParseObject() {
    auto r = Parse("{\"name\": \"test\", \"value\": 42}");
    ASSERT(r.error.empty(), "object should parse");
    ASSERT(r.value.get("name") != nullptr, "has 'name' key");
    ASSERT(r.value.get("name")->as_string() == "test", "name is 'test'");
    ASSERT(r.value.get("value")->as_number() == 42.0, "value is 42");
    return true;
}

bool TestParseNestedObject() {
    auto r = Parse("{\"outer\": {\"inner\": true}}");
    ASSERT(r.error.empty(), "nested object should parse");
    auto* inner = r.value.get("outer");
    ASSERT(inner != nullptr, "has outer");
    ASSERT(inner->get("inner") != nullptr, "has inner");
    ASSERT(inner->get("inner")->as_bool() == true, "inner is true");
    return true;
}

bool TestGetNonexistentKey() {
    auto r = Parse("{\"a\": 1}");
    ASSERT(r.error.empty(), "parse ok");
    ASSERT(r.value.get("b") == nullptr, "nonexistent key returns null");
    return true;
}

bool TestGetOnNonObject() {
    auto r = Parse("[1, 2, 3]");
    ASSERT(r.error.empty(), "parse ok");
    ASSERT(r.value.get("anything") == nullptr, "get on array returns null");
    return true;
}

// --- Whitespace handling ---

bool TestWhitespaceVariants() {
    auto r = Parse("  \t\n\r { \n \"key\" \t : \r\n \"value\" \n } \t ");
    ASSERT(r.error.empty(), "whitespace variants should parse");
    ASSERT(r.value.get("key")->as_string() == "value", "key should map to value");
    return true;
}

// --- Malformed inputs ---

bool TestEmptyInput() {
    auto r = Parse("");
    ASSERT(!r.error.empty(), "empty input should error");
    return true;
}

bool TestTrailingCharacters() {
    auto r = Parse("42 extra");
    ASSERT(!r.error.empty(), "trailing characters should error");
    ASSERT(r.error.find("Trailing") != std::string::npos, "error mentions trailing");
    return true;
}

bool TestTrailingCommaArray() {
    auto r = Parse("[1, 2,]");
    // Our parser does not allow trailing commas (strict JSON)
    ASSERT(!r.error.empty(), "trailing comma in array should error");
    return true;
}

bool TestTrailingCommaObject() {
    auto r = Parse("{\"a\": 1,}");
    ASSERT(!r.error.empty(), "trailing comma in object should error");
    return true;
}

bool TestMissingQuoteString() {
    auto r = Parse("{a: 1}");
    ASSERT(!r.error.empty(), "unquoted key should error");
    return true;
}

bool TestUnterminatedString() {
    auto r = Parse("\"hello");
    ASSERT(!r.error.empty(), "unterminated string should error");
    return true;
}

bool TestUnterminatedArray() {
    auto r = Parse("[1, 2");
    ASSERT(!r.error.empty(), "unterminated array should error");
    return true;
}

bool TestUnterminatedObject() {
    auto r = Parse("{\"a\": 1");
    ASSERT(!r.error.empty(), "unterminated object should error");
    return true;
}

bool TestMissingColon() {
    auto r = Parse("{\"key\" \"value\"}");
    ASSERT(!r.error.empty(), "missing colon should error");
    return true;
}

bool TestInvalidLiteral() {
    auto r = Parse("tru");
    ASSERT(!r.error.empty(), "truncated literal should error");
    return true;
}

bool TestInvalidLiteralNull() {
    auto r = Parse("nul");
    ASSERT(!r.error.empty(), "truncated null should error");
    return true;
}

bool TestBareWord() {
    auto r = Parse("undefined");
    ASSERT(!r.error.empty(), "bare word should error");
    return true;
}

bool TestInvalidNumber() {
    auto r = Parse("1e");
    ASSERT(!r.error.empty(), "number with missing exponent should error");
    return true;
}

bool TestUnicodeEscapeRejected() {
    auto r = Parse("\"hello \\u0041 world\"");
    ASSERT(!r.error.empty(), "\\uXXXX should be explicitly rejected");
    ASSERT(r.error.find("\\uXXXX") != std::string::npos, "error mentions \\uXXXX");
    return true;
}

bool TestUnknownEscape() {
    auto r = Parse("\"bad \\x escape\"");
    ASSERT(!r.error.empty(), "unknown escape should error");
    return true;
}

bool TestEscapeAtEOF() {
    auto r = Parse("\"trailing\\");
    ASSERT(!r.error.empty(), "escape at EOF should error");
    return true;
}

// --- Type accessor safety ---

bool TestTypePredicates() {
    ASSERT(Parse("null").value.is_null(), "null predicate");
    ASSERT(!Parse("null").value.is_bool(), "null is not bool");
    ASSERT(!Parse("null").value.is_number(), "null is not number");
    ASSERT(!Parse("null").value.is_string(), "null is not string");
    ASSERT(!Parse("null").value.is_array(), "null is not array");
    ASSERT(!Parse("null").value.is_object(), "null is not object");
    
    ASSERT(Parse("true").value.is_bool(), "bool predicate");
    ASSERT(Parse("42").value.is_number(), "number predicate");
    ASSERT(Parse("\"s\"").value.is_string(), "string predicate");
    ASSERT(Parse("[]").value.is_array(), "array predicate");
    ASSERT(Parse("{}").value.is_object(), "object predicate");
    return true;
}

// --- Number edge cases ---

bool TestParseZero() {
    auto r = Parse("0");
    ASSERT(r.error.empty(), "zero should parse");
    ASSERT(r.value.as_number() == 0.0, "value should be 0");
    return true;
}

bool TestParseNegativeZero() {
    auto r = Parse("-0");
    ASSERT(r.error.empty(), "negative zero should parse");
    // -0 == 0 in IEEE 754
    ASSERT(r.value.as_number() == 0.0, "value should be 0");
    return true;
}

bool TestParseLargeNumber() {
    auto r = Parse("1e308");
    ASSERT(r.error.empty(), "large number should parse");
    ASSERT(r.value.as_number() > 1e307, "value should be large");
    return true;
}

bool TestParseTinyNumber() {
    auto r = Parse("1e-308");
    ASSERT(r.error.empty(), "tiny number should parse");
    ASSERT(r.value.as_number() > 0.0, "value should be positive");
    ASSERT(r.value.as_number() < 1e-307, "value should be very small");
    return true;
}

// --- Nesting depth ---

bool TestModerateNesting() {
    // 50 levels of nesting should work fine
    std::string json;
    for (int i = 0; i < 50; i++) json += "[";
    json += "1";
    for (int i = 0; i < 50; i++) json += "]";
    
    auto r = Parse(json);
    ASSERT(r.error.empty(), "50 levels of nesting should parse");
    
    // Walk down to the value
    const Value* v = &r.value;
    for (int i = 0; i < 50; i++) {
        ASSERT(v->is_array(), "should be array at depth");
        v = &v->as_array()[0];
    }
    ASSERT(v->as_number() == 1.0, "innermost value should be 1");
    return true;
}

// --- Real-world schema fragment ---

bool TestSchemaFragment() {
    const char* json = R"({
        "version": 1,
        "groups": [
            {
                "id": "fractal",
                "label": "Fractal",
                "controls": [
                    {"id": "fractal_type", "type": "enum", "path": "fractal.view.fractal_type"}
                ]
            }
        ]
    })";
    
    auto r = Parse(json);
    ASSERT(r.error.empty(), "schema fragment should parse");
    ASSERT(r.value.get("version")->as_number() == 1.0, "version is 1");
    auto* groups = r.value.get("groups");
    ASSERT(groups != nullptr && groups->is_array(), "groups is array");
    ASSERT(groups->as_array().size() == 1, "one group");
    auto& g = groups->as_array()[0];
    ASSERT(g.get("id")->as_string() == "fractal", "group id");
    auto* controls = g.get("controls");
    ASSERT(controls->as_array().size() == 1, "one control");
    ASSERT(controls->as_array()[0].get("path")->as_string() == "fractal.view.fractal_type", "path");
    return true;
}

#define RUN(fn) do { \
    if (fn()) { std::fprintf(stderr, "  PASS: %s\n", #fn); } \
    else { std::fprintf(stderr, "  FAIL: %s\n", #fn); } \
} while (0)

int main() {
    // Valid inputs
    RUN(TestParseNull);
    RUN(TestParseTrue);
    RUN(TestParseFalse);
    RUN(TestParseInteger);
    RUN(TestParseNegativeInteger);
    RUN(TestParseFloat);
    RUN(TestParseScientific);
    RUN(TestParseScientificNegExp);
    RUN(TestParseString);
    RUN(TestParseStringEscapes);
    RUN(TestParseStringAllEscapes);
    RUN(TestParseEmptyString);
    RUN(TestParseEmptyArray);
    RUN(TestParseArray);
    RUN(TestParseNestedArray);
    RUN(TestParseEmptyObject);
    RUN(TestParseObject);
    RUN(TestParseNestedObject);
    RUN(TestGetNonexistentKey);
    RUN(TestGetOnNonObject);
    RUN(TestWhitespaceVariants);
    
    // Malformed inputs
    RUN(TestEmptyInput);
    RUN(TestTrailingCharacters);
    RUN(TestTrailingCommaArray);
    RUN(TestTrailingCommaObject);
    RUN(TestMissingQuoteString);
    RUN(TestUnterminatedString);
    RUN(TestUnterminatedArray);
    RUN(TestUnterminatedObject);
    RUN(TestMissingColon);
    RUN(TestInvalidLiteral);
    RUN(TestInvalidLiteralNull);
    RUN(TestBareWord);
    RUN(TestInvalidNumber);
    RUN(TestUnicodeEscapeRejected);
    RUN(TestUnknownEscape);
    RUN(TestEscapeAtEOF);
    
    // Type predicates
    RUN(TestTypePredicates);
    
    // Number edge cases
    RUN(TestParseZero);
    RUN(TestParseNegativeZero);
    RUN(TestParseLargeNumber);
    RUN(TestParseTinyNumber);
    
    // Nesting
    RUN(TestModerateNesting);
    
    // Real-world
    RUN(TestSchemaFragment);
    
    std::fprintf(stderr, "test_json_min: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}
