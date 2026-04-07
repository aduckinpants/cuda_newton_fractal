#pragma once

#include <string>
#include <vector>

#include "fractal_types.h"

bool HasArg(const std::vector<std::string>& args, const char* flag);
bool TryGetArgValue(const std::vector<std::string>& args, const char* flag, std::string* outValue);
bool TryParseDoubleArg(const std::vector<std::string>& args, const char* flag, double* outValue);
bool TryParseIntArg(const std::vector<std::string>& args, const char* flag, int* outValue);
bool TryParseFractalTypeArg(const std::vector<std::string>& args, FractalType* outType);
