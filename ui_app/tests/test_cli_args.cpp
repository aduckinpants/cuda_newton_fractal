#include <cstdio>
#include <string>
#include <vector>
#include "../src/cli_args.h"

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

// --- HasArg ---

bool TestHasArgPresent() {
    std::vector<std::string> args = {"app.exe", "--verbose", "--width", "800"};
    ASSERT(HasArg(args, "--verbose"), "should find --verbose");
    ASSERT(HasArg(args, "--width"), "should find --width");
    return true;
}

bool TestHasArgMissing() {
    std::vector<std::string> args = {"app.exe", "--verbose"};
    ASSERT(!HasArg(args, "--missing"), "should not find --missing");
    return true;
}

bool TestHasArgEmpty() {
    std::vector<std::string> args;
    ASSERT(!HasArg(args, "--flag"), "empty args should find nothing");
    return true;
}

// --- TryGetArgValue ---

bool TestTryGetArgValue() {
    std::vector<std::string> args = {"app.exe", "--file", "out.json", "--count", "5"};
    std::string value;
    ASSERT(TryGetArgValue(args, "--file", &value), "should find --file");
    ASSERT(value == "out.json", "value should be out.json");
    ASSERT(TryGetArgValue(args, "--count", &value), "should find --count");
    ASSERT(value == "5", "value should be 5");
    return true;
}

bool TestTryGetArgValueMissing() {
    std::vector<std::string> args = {"app.exe", "--file", "out.json"};
    std::string value;
    ASSERT(!TryGetArgValue(args, "--missing", &value), "should not find --missing");
    return true;
}

bool TestTryGetArgValueNoTrailing() {
    std::vector<std::string> args = {"app.exe", "--file"};
    std::string value;
    ASSERT(!TryGetArgValue(args, "--file", &value), "flag at end without value should fail");
    return true;
}

bool TestTryGetArgValueNull() {
    std::vector<std::string> args = {"app.exe", "--file", "out.json"};
    ASSERT(TryGetArgValue(args, "--file", nullptr), "null outValue should still return true");
    return true;
}

// --- TryParseDoubleArg ---

bool TestTryParseDoubleArg() {
    std::vector<std::string> args = {"app.exe", "--zoom", "3.14", "--neg", "-2.5"};
    double val = 0.0;
    ASSERT(TryParseDoubleArg(args, "--zoom", &val), "should parse --zoom");
    ASSERT(val > 3.13 && val < 3.15, "zoom should be ~3.14");
    ASSERT(TryParseDoubleArg(args, "--neg", &val), "should parse --neg");
    ASSERT(val < -2.49 && val > -2.51, "neg should be ~-2.5");
    return true;
}

bool TestTryParseDoubleArgBadValue() {
    std::vector<std::string> args = {"app.exe", "--zoom", "notanumber"};
    double val = 0.0;
    ASSERT(!TryParseDoubleArg(args, "--zoom", &val), "non-numeric should fail");
    return true;
}

bool TestTryParseDoubleArgTrailingGarbage() {
    std::vector<std::string> args = {"app.exe", "--zoom", "3.14xyz"};
    double val = 0.0;
    ASSERT(!TryParseDoubleArg(args, "--zoom", &val), "trailing garbage should fail");
    return true;
}

bool TestTryParseDoubleArgMissing() {
    std::vector<std::string> args = {"app.exe"};
    double val = 0.0;
    ASSERT(!TryParseDoubleArg(args, "--zoom", &val), "missing flag should fail");
    return true;
}

// --- TryParseIntArg ---

bool TestTryParseIntArg() {
    std::vector<std::string> args = {"app.exe", "--width", "1024", "--offset", "-7"};
    int val = 0;
    ASSERT(TryParseIntArg(args, "--width", &val), "should parse --width");
    ASSERT(val == 1024, "width should be 1024");
    ASSERT(TryParseIntArg(args, "--offset", &val), "should parse --offset");
    ASSERT(val == -7, "offset should be -7");
    return true;
}

bool TestTryParseIntArgBadValue() {
    std::vector<std::string> args = {"app.exe", "--width", "abc"};
    int val = 0;
    ASSERT(!TryParseIntArg(args, "--width", &val), "non-numeric should fail");
    return true;
}

bool TestTryParseIntArgTrailingGarbage() {
    std::vector<std::string> args = {"app.exe", "--width", "800px"};
    int val = 0;
    ASSERT(!TryParseIntArg(args, "--width", &val), "trailing garbage should fail");
    return true;
}

// --- TryParseFractalTypeArg ---

bool TestParseFractalTypeNewton() {
    std::vector<std::string> args = {"app.exe", "--fractal-type", "newton"};
    FractalType type{};
    ASSERT(TryParseFractalTypeArg(args, &type), "should parse newton");
    ASSERT(type == FractalType::newton, "type should be newton");
    return true;
}

bool TestParseFractalTypeMandelbrot() {
    std::vector<std::string> args = {"app.exe", "--fractal-type", "mandelbrot"};
    FractalType type{};
    ASSERT(TryParseFractalTypeArg(args, &type), "should parse mandelbrot");
    ASSERT(type == FractalType::mandelbrot, "type should be mandelbrot");
    return true;
}

bool TestParseFractalTypeLambda() {
    std::vector<std::string> args = {"app.exe", "--fractal-type", "lambda"};
    FractalType type{};
    ASSERT(TryParseFractalTypeArg(args, &type), "should parse lambda -> lambda_map");
    ASSERT(type == FractalType::lambda_map, "type should be lambda_map");
    return true;
}

bool TestParseFractalTypePerpendicularBurningShip() {
    std::vector<std::string> args = {"app.exe", "--fractal-type", "perpendicular_burning_ship"};
    FractalType type{};
    ASSERT(TryParseFractalTypeArg(args, &type), "should parse perpendicular_burning_ship");
    ASSERT(type == FractalType::perpendicular_burning_ship, "type should be perpendicular_burning_ship");
    return true;
}

bool TestParseFractalTypeUnknown() {
    std::vector<std::string> args = {"app.exe", "--fractal-type", "nonexistent"};
    FractalType type{};
    ASSERT(!TryParseFractalTypeArg(args, &type), "unknown type should fail");
    return true;
}

bool TestParseFractalTypeMissing() {
    std::vector<std::string> args = {"app.exe"};
    FractalType type{};
    ASSERT(!TryParseFractalTypeArg(args, &type), "missing --fractal-type should fail");
    return true;
}

bool TestParseFractalTypeAllValues() {
    // Exhaustive: every string the CLI accepts must map to the right enum.
    struct Case { const char* name; FractalType expected; };
    Case cases[] = {
        {"newton", FractalType::newton},
        {"nova", FractalType::nova},
        {"mandelbrot", FractalType::mandelbrot},
        {"julia", FractalType::julia},
        {"burning_ship", FractalType::burning_ship},
        {"multibrot", FractalType::multibrot},
        {"phoenix", FractalType::phoenix},
        {"explaino", FractalType::explaino},
        {"explaino_y", FractalType::explaino_y},
        {"explaino_fp", FractalType::explaino_fp},
        {"explaino_nova", FractalType::explaino_nova},
        {"explaino_halley", FractalType::explaino_halley},
        {"explaino_dual", FractalType::explaino_dual},
        {"explaino_mult", FractalType::explaino_mult},
        {"explaino_phoenix", FractalType::explaino_phoenix},
        {"explaino_transcendental", FractalType::explaino_transcendental},
        {"explaino_inertial", FractalType::explaino_inertial},
        {"explaino_julia", FractalType::explaino_julia},
        {"explaino_rational", FractalType::explaino_rational},
        {"explaino_joy", FractalType::explaino_joy},
        {"multicorn", FractalType::multicorn},
        {"halley", FractalType::halley},
        {"collatz", FractalType::collatz},
        {"explaino_collatz", FractalType::explaino_collatz},
        {"mcmullen", FractalType::mcmullen},
        {"lambda", FractalType::lambda_map},
        {"explaino_lambda", FractalType::explaino_lambda},
        {"explaino_rational_escape", FractalType::explaino_rational_escape},
        {"spider", FractalType::spider},
        {"celtic_mandelbrot", FractalType::celtic_mandelbrot},
        {"perpendicular_burning_ship", FractalType::perpendicular_burning_ship},
    };

    for (const auto& c : cases) {
        std::vector<std::string> args = {"app.exe", "--fractal-type", c.name};
        FractalType type{};
        ASSERT(TryParseFractalTypeArg(args, &type),
            (std::string("should parse: ") + c.name).c_str());
        ASSERT(type == c.expected,
            (std::string("wrong enum for: ") + c.name).c_str());
    }
    return true;
}

#define RUN(fn) do { \
    if (fn()) { std::fprintf(stderr, "  PASS: %s\n", #fn); } \
    else { std::fprintf(stderr, "  FAIL: %s\n", #fn); } \
} while (0)

int main() {
    RUN(TestHasArgPresent);
    RUN(TestHasArgMissing);
    RUN(TestHasArgEmpty);
    RUN(TestTryGetArgValue);
    RUN(TestTryGetArgValueMissing);
    RUN(TestTryGetArgValueNoTrailing);
    RUN(TestTryGetArgValueNull);
    RUN(TestTryParseDoubleArg);
    RUN(TestTryParseDoubleArgBadValue);
    RUN(TestTryParseDoubleArgTrailingGarbage);
    RUN(TestTryParseDoubleArgMissing);
    RUN(TestTryParseIntArg);
    RUN(TestTryParseIntArgBadValue);
    RUN(TestTryParseIntArgTrailingGarbage);
    RUN(TestParseFractalTypeNewton);
    RUN(TestParseFractalTypeMandelbrot);
    RUN(TestParseFractalTypeLambda);
    RUN(TestParseFractalTypePerpendicularBurningShip);
    RUN(TestParseFractalTypeUnknown);
    RUN(TestParseFractalTypeMissing);
    RUN(TestParseFractalTypeAllValues);

    std::fprintf(stderr, "test_cli_args: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}
