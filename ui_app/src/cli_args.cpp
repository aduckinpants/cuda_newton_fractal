#include "cli_args.h"

#include "enum_id_utils.h"

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cmath>

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

bool TryGetArgValues(const std::vector<std::string>& args, const char* flag, std::vector<std::string>* outValues) {
    if (outValues) {
        outValues->clear();
    }
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] != flag) {
            continue;
        }
        if (i + 1 >= args.size()) {
            return false;
        }
        if (outValues) {
            outValues->push_back(args[i + 1]);
        }
        ++i;
    }
    return true;
}

bool TryParseDoubleArg(const std::vector<std::string>& args, const char* flag, double* outValue) {
    std::string text;
    if (!TryGetArgValue(args, flag, &text)) return false;
    char* end = nullptr;
    errno = 0;
    double value = std::strtod(text.c_str(), &end);
    if (!end || end == text.c_str() || *end != '\0') return false;
    if (errno == ERANGE || !std::isfinite(value)) return false;
    if (outValue) *outValue = value;
    return true;
}

bool TryParseIntArg(const std::vector<std::string>& args, const char* flag, int* outValue) {
    std::string text;
    if (!TryGetArgValue(args, flag, &text)) return false;
    char* end = nullptr;
    errno = 0;
    long value = std::strtol(text.c_str(), &end, 10);
    if (!end || end == text.c_str() || *end != '\0') return false;
    if (errno == ERANGE || value < static_cast<long>(INT_MIN) || value > static_cast<long>(INT_MAX)) return false;
    if (outValue) *outValue = (int)value;
    return true;
}

bool TryParseFractalTypeArg(const std::vector<std::string>& args, FractalType* outType) {
    if (!outType) return false;
    std::string text;
    if (!TryGetArgValue(args, "--fractal-type", &text)) return false;
    return TryParseFractalTypeId(text, outType);
}
