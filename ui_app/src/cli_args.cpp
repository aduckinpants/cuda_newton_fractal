#include "cli_args.h"

#include <cstdlib>

bool HasArg(const std::vector<std::string>& args, const char* flag) {
    for (const auto& arg : args) {
        if (arg == flag) return true;
    }
    return false;
}

bool TryGetArgValue(const std::vector<std::string>& args, const char* flag, std::string* outValue) {
    for (size_t i = 0; i + 1 < args.size(); ++i) {
        if (args[i] == flag) {
            if (outValue) *outValue = args[i + 1];
            return true;
        }
    }
    return false;
}

bool TryParseDoubleArg(const std::vector<std::string>& args, const char* flag, double* outValue) {
    std::string text;
    if (!TryGetArgValue(args, flag, &text)) return false;
    char* end = nullptr;
    double value = std::strtod(text.c_str(), &end);
    if (!end || *end != '\0') return false;
    if (outValue) *outValue = value;
    return true;
}

bool TryParseIntArg(const std::vector<std::string>& args, const char* flag, int* outValue) {
    std::string text;
    if (!TryGetArgValue(args, flag, &text)) return false;
    char* end = nullptr;
    long value = std::strtol(text.c_str(), &end, 10);
    if (!end || *end != '\0') return false;
    if (outValue) *outValue = (int)value;
    return true;
}

bool TryParseFractalTypeArg(const std::vector<std::string>& args, FractalType* outType) {
    std::string text;
    if (!TryGetArgValue(args, "--fractal-type", &text)) return false;

    if (text == "newton") { if (outType) *outType = FractalType::newton; return true; }
    if (text == "nova") { if (outType) *outType = FractalType::nova; return true; }
    if (text == "mandelbrot") { if (outType) *outType = FractalType::mandelbrot; return true; }
    if (text == "julia") { if (outType) *outType = FractalType::julia; return true; }
    if (text == "burning_ship") { if (outType) *outType = FractalType::burning_ship; return true; }
    if (text == "multibrot") { if (outType) *outType = FractalType::multibrot; return true; }
    if (text == "phoenix") { if (outType) *outType = FractalType::phoenix; return true; }
    if (text == "explaino") { if (outType) *outType = FractalType::explaino; return true; }
    if (text == "explaino_y") { if (outType) *outType = FractalType::explaino_y; return true; }
    if (text == "explaino_fp") { if (outType) *outType = FractalType::explaino_fp; return true; }
    if (text == "explaino_nova") { if (outType) *outType = FractalType::explaino_nova; return true; }
    if (text == "explaino_halley") { if (outType) *outType = FractalType::explaino_halley; return true; }
    if (text == "explaino_dual") { if (outType) *outType = FractalType::explaino_dual; return true; }
    if (text == "explaino_mult") { if (outType) *outType = FractalType::explaino_mult; return true; }
    if (text == "explaino_phoenix") { if (outType) *outType = FractalType::explaino_phoenix; return true; }
    if (text == "explaino_transcendental") { if (outType) *outType = FractalType::explaino_transcendental; return true; }
    if (text == "explaino_inertial") { if (outType) *outType = FractalType::explaino_inertial; return true; }
    if (text == "explaino_julia") { if (outType) *outType = FractalType::explaino_julia; return true; }
    if (text == "explaino_rational") { if (outType) *outType = FractalType::explaino_rational; return true; }
    if (text == "explaino_joy") { if (outType) *outType = FractalType::explaino_joy; return true; }
    if (text == "explaino_fold") { if (outType) *outType = FractalType::explaino_fold; return true; }
    if (text == "explaino_bell") { if (outType) *outType = FractalType::explaino_bell; return true; }
    if (text == "explaino_ripple") { if (outType) *outType = FractalType::explaino_ripple; return true; }
    if (text == "explaino_splice") { if (outType) *outType = FractalType::explaino_splice; return true; }
    if (text == "explaino_vortex") { if (outType) *outType = FractalType::explaino_vortex; return true; }
    if (text == "multicorn") { if (outType) *outType = FractalType::multicorn; return true; }
    if (text == "halley") { if (outType) *outType = FractalType::halley; return true; }
    if (text == "collatz") { if (outType) *outType = FractalType::collatz; return true; }
    if (text == "explaino_collatz") { if (outType) *outType = FractalType::explaino_collatz; return true; }
    if (text == "mcmullen") { if (outType) *outType = FractalType::mcmullen; return true; }
    if (text == "lambda") { if (outType) *outType = FractalType::lambda_map; return true; }
    if (text == "explaino_lambda") { if (outType) *outType = FractalType::explaino_lambda; return true; }
    if (text == "explaino_rational_escape") { if (outType) *outType = FractalType::explaino_rational_escape; return true; }
    if (text == "spider") { if (outType) *outType = FractalType::spider; return true; }
    if (text == "celtic_mandelbrot") { if (outType) *outType = FractalType::celtic_mandelbrot; return true; }
    if (text == "perpendicular_burning_ship") { if (outType) *outType = FractalType::perpendicular_burning_ship; return true; }
    return false;
}
