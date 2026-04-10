// test_generic_function_parser.cpp — GF-4 expression parser tests.
// Validates recursive-descent parser for GenericFunctionDesc expressions.

#include "generic_function_parser.h"
#include "generic_function_cpu_eval.h"
#include <cstdio>
#include <cmath>
#include <map>
#include <string>

static int g_pass = 0, g_fail = 0;

static void check(bool cond, const char* msg, int line) {
    if (cond) { g_pass++; }
    else { g_fail++; printf("  FAIL [line %d]: %s\n", line, msg); }
}
#define CHECK(c, m) check((c), (m), __LINE__)

// Helper: parse, evaluate at z, compare to expected.
static void check_eval(const std::string& expr,
                       const std::map<std::string, double>& params,
                       double zr, double zi,
                       double exp_re, double exp_im,
                       double tol, const char* desc) {
    GFParseResult pr = ParseGenericFunctionExpression(expr, params);
    if (!pr.ok) {
        g_fail++;
        printf("  FAIL: parse '%s': %s (pos %d)\n", expr.c_str(), pr.error.c_str(), pr.error_pos);
        return;
    }
    GFCpuComplex z = {zr, zi};
    GFCpuComplex result = gf_cpu_eval(pr.desc, z);
    double err_re = std::fabs(result.x - exp_re);
    double err_im = std::fabs(result.y - exp_im);
    if (err_re < tol && err_im < tol) {
        g_pass++;
    } else {
        g_fail++;
        printf("  FAIL: %s: '%s' at (%g,%g) -> (%g,%g), expected (%g,%g)\n",
               desc, expr.c_str(), zr, zi, result.x, result.y, exp_re, exp_im);
    }
}

// ---- Test groups ----

static void test_simple_expressions() {
    printf("--- test_simple_expressions ---\n");
    std::map<std::string, double> empty;

    // z at (2, 3) -> (2, 3)
    check_eval("z", empty, 2.0, 3.0, 2.0, 3.0, 1e-10, "z");

    // z + z at (1, 2) -> (2, 4)
    check_eval("z + z", empty, 1.0, 2.0, 2.0, 4.0, 1e-10, "z+z");

    // z * z at (1, 1) -> (0, 2)
    check_eval("z * z", empty, 1.0, 1.0, 0.0, 2.0, 1e-10, "z*z");

    // (1 + 0i)^2 = 1
    check_eval("z * z", empty, 1.0, 0.0, 1.0, 0.0, 1e-10, "1^2");
}

static void test_constants() {
    printf("--- test_constants ---\n");
    std::map<std::string, double> empty;

    // Literal number: 3.5 (should be constant regardless of z)
    check_eval("3.5", empty, 99.0, 99.0, 3.5, 0.0, 1e-10, "literal 3.5");

    // 2 * z at (3, 0) -> (6, 0)
    check_eval("2 * z", empty, 3.0, 0.0, 6.0, 0.0, 1e-10, "2*z");
}

static void test_params() {
    printf("--- test_params ---\n");
    std::map<std::string, double> p;
    p["K"] = 3.0;

    // z^K at (2, 0) = 2^3 = 8
    check_eval("z ^ K", p, 2.0, 0.0, 8.0, 0.0, 1e-6, "z^K");

    // c as complex param: c_real=1, c_imag=2 -> z + c at (0,0) = (1,2)
    std::map<std::string, double> p2;
    p2["c_real"] = 1.0;
    p2["c_imag"] = 2.0;
    check_eval("z + c", p2, 0.0, 0.0, 1.0, 2.0, 1e-10, "z+c complex");
}

static void test_power() {
    printf("--- test_power ---\n");
    std::map<std::string, double> empty;

    // z^3 at (2, 0) = 8
    check_eval("z ^ 3", empty, 2.0, 0.0, 8.0, 0.0, 1e-6, "z^3 real");

    // z^2 at (0, 1) = i^2 = -1
    check_eval("z ^ 2", empty, 0.0, 1.0, -1.0, 0.0, 1e-6, "i^2");

    // z^0 = 1
    check_eval("z ^ 0", empty, 5.0, 3.0, 1.0, 0.0, 1e-6, "z^0");
}

static void test_unary_neg() {
    printf("--- test_unary_neg ---\n");
    std::map<std::string, double> empty;

    // -z at (2, 3) -> (-2, -3)
    check_eval("-z", empty, 2.0, 3.0, -2.0, -3.0, 1e-10, "-z");

    // -(z * z) at (1, 1) = -(0, 2) = (0, -2)
    check_eval("-(z * z)", empty, 1.0, 1.0, 0.0, -2.0, 1e-10, "-(z*z)");
}

static void test_transcendental() {
    printf("--- test_transcendental ---\n");
    std::map<std::string, double> empty;

    // sin(z) at (0, 0) = 0
    check_eval("sin(z)", empty, 0.0, 0.0, 0.0, 0.0, 1e-10, "sin(0)");

    // cos(z) at (0, 0) = 1
    check_eval("cos(z)", empty, 0.0, 0.0, 1.0, 0.0, 1e-10, "cos(0)");

    // exp(z) at (0, 0) = 1
    check_eval("exp(z)", empty, 0.0, 0.0, 1.0, 0.0, 1e-10, "exp(0)");

    // sin(z) * z at (1, 0) -> sin(1) * 1 = 0.8414...
    check_eval("sin(z) * z", empty, 1.0, 0.0, std::sin(1.0), 0.0, 1e-6, "sin(z)*z");
}

static void test_division() {
    printf("--- test_division ---\n");
    std::map<std::string, double> empty;

    // z / z at (3, 4) = 1
    check_eval("z / z", empty, 3.0, 4.0, 1.0, 0.0, 1e-6, "z/z");

    // 1 / z at (2, 0) = 0.5
    check_eval("1 / z", empty, 2.0, 0.0, 0.5, 0.0, 1e-6, "1/z");
}

static void test_newton_expression() {
    printf("--- test_newton_expression ---\n");
    std::map<std::string, double> empty;

    // Newton's method step for z^3-1: z - (z^3 - 1) / (3 * z^2)
    // At z=1 (exact root): step should return 1.
    check_eval("z - (z ^ 3 - 1) / (3 * z ^ 2)", empty,
               1.0, 0.0, 1.0, 0.0, 1e-6, "newton step at root");

    // At z=2: step = 2 - (8-1)/(3*4) = 2 - 7/12 = 17/12 ≈ 1.4167
    check_eval("z - (z ^ 3 - 1) / (3 * z ^ 2)", empty,
               2.0, 0.0, 17.0 / 12.0, 0.0, 1e-6, "newton step at z=2");
}

static void test_iterate_expression() {
    printf("--- test_iterate_expression ---\n");
    std::map<std::string, double> empty;

    // iterate(z * z, 3) at z=2: 2 -> 4 -> 16 -> 256
    // The CPU evaluator runs the iterate loop, so result should be 256.
    GFParseResult pr = ParseGenericFunctionExpression("iterate(z * z, 3)", empty);
    CHECK(pr.ok, "parse iterate(z*z, 3) succeeds");
    if (pr.ok) {
        GFCpuComplex z = {2.0, 0.0};
        GFCpuComplex result = gf_cpu_eval(pr.desc, z);
        CHECK(std::fabs(result.x - 256.0) < 1e-6, "iterate(z*z,3) at z=2 = 256");
    }

    // iterate(z^2 + c, 10) with c = -1
    std::map<std::string, double> p;
    p["c_real"] = -1.0;
    p["c_imag"] = 0.0;
    pr = ParseGenericFunctionExpression("iterate(z ^ 2 + c, 10)", p);
    CHECK(pr.ok, "parse iterate(z^2+c,10) succeeds");
    if (pr.ok) {
        // z=0: 0 -> -1 -> 0 -> -1 -> 0 -> ...  (period-2 orbit)
        GFCpuComplex z = {0.0, 0.0};
        GFCpuComplex result = gf_cpu_eval(pr.desc, z);
        // After 10 iterations (even number): should be -1 or 0 depending on even/odd.
        // 0 -> 0^2-1=-1 -> (-1)^2-1=0 -> -1 -> 0 -> ...
        // iter 10 (even, 0-based): z should be = 0^2-1 = -1 (since we start from 1)
        // Actually: CPU eval loop: k=0: cur=eval(z=0)=-1, k=1: cur=eval(z=-1)=0, ...
        // k=even -> -1, k=odd -> 0. k=9 (last) -> 0
        CHECK(std::fabs(result.x) < 1e-6 || std::fabs(result.x + 1.0) < 1e-6,
              "iterate(z^2+c,10) at z=0,c=-1 is periodic {0,-1}");
    }
}

static void test_compose_expression() {
    printf("--- test_compose_expression ---\n");
    std::map<std::string, double> empty;

    // compose(z^2, z + 1) at z=2 should be (2+1)^2 = 9
    check_eval("compose(z ^ 2, z + 1)", empty, 2.0, 0.0, 9.0, 0.0, 1e-6, "compose(z^2,z+1)");
}

static void test_parse_errors() {
    printf("--- test_parse_errors ---\n");
    std::map<std::string, double> empty;

    // Empty expression.
    GFParseResult pr = ParseGenericFunctionExpression("", empty);
    CHECK(!pr.ok, "empty expression fails");

    // Unknown identifier.
    pr = ParseGenericFunctionExpression("foo", empty);
    CHECK(!pr.ok, "unknown identifier fails");
    CHECK(pr.error.find("unknown") != std::string::npos, "error mentions 'unknown'");

    // Unknown function.
    pr = ParseGenericFunctionExpression("tan(z)", empty);
    CHECK(!pr.ok, "unknown function 'tan' fails");
    CHECK(pr.error.find("unknown function") != std::string::npos, "error mentions 'unknown function'");

    // Missing closing paren.
    pr = ParseGenericFunctionExpression("(z + 1", empty);
    CHECK(!pr.ok, "missing ')' fails");

    // Missing iterate count.
    pr = ParseGenericFunctionExpression("iterate(z)", empty);
    CHECK(!pr.ok, "iterate with one arg fails");

    // Trailing junk.
    pr = ParseGenericFunctionExpression("z + 1 @", empty);
    CHECK(!pr.ok, "trailing junk fails");

    // Valid expressions should succeed.
    pr = ParseGenericFunctionExpression("z", empty);
    CHECK(pr.ok, "plain 'z' succeeds");

    pr = ParseGenericFunctionExpression("z ^ 3 - 1", empty);
    CHECK(pr.ok, "'z^3 - 1' succeeds");
}

static void test_operator_precedence() {
    printf("--- test_operator_precedence ---\n");
    std::map<std::string, double> empty;

    // 2 + 3 * z at z=4 -> 2 + 12 = 14 (not (2+3)*4 = 20)
    check_eval("2 + 3 * z", empty, 4.0, 0.0, 14.0, 0.0, 1e-6, "2+3*z precedence");

    // z^2 + 1 at z=3 -> 9+1 = 10
    check_eval("z ^ 2 + 1", empty, 3.0, 0.0, 10.0, 0.0, 1e-6, "z^2+1");

    // -z^2 at z=3 -> -(3^2) = -9 (unary minus binds tighter than +/- but weaker than ^?)
    // Actually in our grammar: unary := '-' unary | power, and power := atom '^' int
    // So -z^2 parses as -(z^2) = -9
    check_eval("-z ^ 2", empty, 3.0, 0.0, -9.0, 0.0, 1e-6, "-z^2 = -(z^2)");
}

int main() {
    printf("=== GF-4: Expression parser tests ===\n\n");

    test_simple_expressions();
    test_constants();
    test_params();
    test_power();
    test_unary_neg();
    test_transcendental();
    test_division();
    test_newton_expression();
    test_iterate_expression();
    test_compose_expression();
    test_parse_errors();
    test_operator_precedence();

    printf("\n=== GF-4 parser summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
