#pragma once

#include "explaino_sidecar_model.h"

#include <string>
#include <vector>

struct ExplainoSidecarWindowRow {
    std::string label;
    std::string path;
    std::string type;
    std::string range_text;
    std::string default_text;
};

struct ExplainoSidecarWindowState {
    std::string title{"Explaino Sidecar"};
    std::string function_id;
    std::string fractal_type_id;
    std::string error_message;
    SidecarOrientationVector orientation{};
    std::vector<ExplainoSidecarWindowRow> rows;
};

bool BuildExplainoSidecarWindowState(
    const EngineFunctionCatalog& catalog,
    const BindingContext& ctx,
    ExplainoSidecarWindowState* outState,
    std::string* outError);

void RenderExplainoSidecarWindow(const ExplainoSidecarWindowState& state);