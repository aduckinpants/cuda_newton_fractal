#pragma once

#include "generic_equation_pack.h"
#include "generic_function_types.h"

#include <cstdint>
#include <map>
#include <string>
#include <vector>

struct ViewerUiAutomationRect;

struct GenericEquationPackWorkbenchPreviewSummary {
    bool ok = false;
    std::string error;
    std::string backend_used;
    int sample_count = 0;
    int converged_count = 0;
    int escaped_count = 0;
    int bounded_count = 0;
    int nonfinite_count = 0;
    double mean_iterations = 0.0;
    double mean_abs2 = 0.0;
    std::string result_hash;
    int image_width = 0;
    int image_height = 0;
    std::vector<std::uint32_t> pixels_rgba;
    std::string image_hash;
};

struct GenericEquationPackWorkbenchState {
    bool open = false;
    bool initialized = false;
    bool force_open_for_automation = false;
    bool have_pack = false;
    bool preview_dirty = false;
    std::string pack_path;
    std::string pack_load_error;
    GenericEquationPack pack;
    std::map<std::string, double> params;
    GenericEquationPackWorkbenchPreviewSummary last_preview;
};

struct GenericEquationPackWorkbenchControlReport {
    std::string control_id;
    std::string id;
    std::string param;
    std::string label;
    double value = 0.0;
    bool integer_value = false;
    bool has_min = false;
    bool has_max = false;
    bool has_step = false;
    bool has_default_value = false;
    double min_value = 0.0;
    double max_value = 0.0;
    double step_value = 0.0;
    double default_value = 0.0;
};

struct GenericEquationPackWorkbenchAutomationReport {
    bool window_open = false;
    bool initialized = false;
    bool force_open_for_automation = false;
    bool have_pack = false;
    std::string pack_path;
    std::string pack_id;
    std::string pack_name;
    std::string pack_load_error;
    std::vector<GenericEquationPackWorkbenchControlReport> controls;
    bool preview_ok = false;
    std::string preview_error;
    std::string preview_backend_used;
    int preview_sample_count = 0;
    int preview_converged_count = 0;
    int preview_escaped_count = 0;
    int preview_bounded_count = 0;
    int preview_nonfinite_count = 0;
    double preview_mean_iterations = 0.0;
    double preview_mean_abs2 = 0.0;
    std::string preview_result_hash;
    int preview_image_width = 0;
    int preview_image_height = 0;
    std::string preview_image_hash;
};

struct GenericEquationPackWorkbenchClickAutomation {
    const std::string* control_id = nullptr;
    bool* consumed = nullptr;
};

struct GenericEquationPackWorkbenchSetValueAutomation {
    const std::string* control_id = nullptr;
    double value = 0.0;
    bool* consumed = nullptr;
    std::string* error = nullptr;
};

std::string GenericEquationPackWorkbenchControlAutomationId(const GenericEquationPackControl& control);
std::string GenericEquationPackWorkbenchResetDefaultsAutomationId();
bool GenericEquationPackWorkbenchWantsSetValueControl(const std::string& controlId);
bool GenericEquationPackWorkbenchWantsClickControl(const std::string& controlId);

bool LoadGenericEquationPackWorkbenchJson(
    GenericEquationPackWorkbenchState* ioState,
    const std::string& jsonText,
    const std::string& sourcePath,
    std::string* outError);

bool LoadGenericEquationPackWorkbenchPack(
    GenericEquationPackWorkbenchState* ioState,
    const std::string& packPath,
    std::string* outError);

bool SetGenericEquationPackWorkbenchControlValue(
    GenericEquationPackWorkbenchState* ioState,
    const std::string& controlId,
    double value,
    std::string* outError);

bool ResetGenericEquationPackWorkbenchControlsToDefaults(
    GenericEquationPackWorkbenchState* ioState,
    std::string* outError);

bool RunGenericEquationPackWorkbenchPreview(
    GenericEquationPackWorkbenchState* ioState,
    std::string* outError);

GenericEquationPackWorkbenchAutomationReport BuildGenericEquationPackWorkbenchAutomationReport(
    const GenericEquationPackWorkbenchState& state);

#ifndef GENERIC_EQUATION_PACK_WORKBENCH_NO_IMGUI
void RenderGenericEquationPackWorkbench(
    GenericEquationPackWorkbenchState* ioState,
    std::vector<ViewerUiAutomationRect>* automationRects,
    GenericEquationPackWorkbenchSetValueAutomation* setValueAutomation,
    GenericEquationPackWorkbenchClickAutomation* clickAutomation,
    bool* outInteracted);
#endif
