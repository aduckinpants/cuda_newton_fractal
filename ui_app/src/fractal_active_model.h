#pragma once

#include <cstddef>
#include <string>

#include "fractal_types.h"

struct ActiveFractalModelReceiptContext {
    std::string state_json_sha256;
    std::string runtime_executable_sha256;
};

std::size_t ActiveFractalModelProviderCount();
const char* ActiveFractalModelProviderIdAt(std::size_t index);

bool BuildActiveFractalModelReceiptJson(
    const ViewState& view,
    const KernelParams& params,
    const RenderSettings& render,
    const ActiveFractalModelReceiptContext& context,
    std::string* outJson,
    std::string* outError);

bool WriteActiveFractalModelReceiptJsonFile(
    const std::string& path,
    const std::string& json,
    std::string* outError);
