#pragma once

#include "fractal_probe_contract.h"

#include <string>

bool IsProbeSamplingImplementedForFractalTypeId(const std::string& fractalTypeId);

bool RunFractalProbeRequest(const FractalProbeRequest& request,
    const std::string& exePath,
    FractalProbeResponse* outResponse,
    std::string* outError);