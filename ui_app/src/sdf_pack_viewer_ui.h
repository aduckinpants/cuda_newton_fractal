#pragma once

#include "sdf_pack_field_producer.h"

#include <map>
#include <string>
#include <vector>

struct ViewerUiAutomationRect;

struct SdfPackViewerBuiltInPackCatalogEntry {
    std::string pack_id;
    std::string label;
    std::string relative_path;
};

struct SdfPackViewerBuiltInPackReport {
    std::string pack_id;
    std::string label;
    bool selected{false};
};

struct SdfPackViewerPreviewSummary {
    bool ok{false};
    std::string error;
    std::string backend_used;
    bool backend_fallback_used{false};
    int width{0};
    int height{0};
    float pixel_scale{1.0f};
    std::size_t sample_count{0};
    double min_signed_distance_px{0.0};
    double max_signed_distance_px{0.0};
    double mean_signed_distance_px{0.0};
    std::string field_hash;
};

struct SdfPackViewerState {
    bool open{false};
    bool initialized{false};
    bool force_open_for_automation{false};
    bool have_pack{false};
    bool use_as_sdf_field_source{false};
    bool preview_dirty{false};
    std::string pack_path;
    std::string pack_json;
    std::string pack_load_error;
    SdfPack pack;
    std::map<std::string, double> params;
    SdfPackFieldBackend backend_preference{SdfPackFieldBackend::auto_backend};
    SdfPackViewerPreviewSummary last_preview;
};

struct SdfPackViewerControlReport {
    std::string control_id;
    std::string param;
    std::string label;
    double value{0.0};
    bool has_min{false};
    bool has_max{false};
    bool has_default_value{false};
    double min_value{0.0};
    double max_value{0.0};
    double default_value{0.0};
};

struct SdfPackViewerAutomationReport {
    bool panel_open{false};
    bool initialized{false};
    bool force_open_for_automation{false};
    bool have_pack{false};
    bool use_as_sdf_field_source{false};
    std::string pack_path;
    std::string pack_id;
    std::string pack_name;
    std::string pack_load_error;
    std::string backend_preference;
    std::string built_in_pack_selector_control_id;
    std::string selected_built_in_pack_id;
    std::vector<SdfPackViewerBuiltInPackReport> built_in_packs;
    std::vector<SdfPackViewerControlReport> controls;
    bool preview_ok{false};
    std::string preview_error;
    std::string preview_backend_used;
    bool preview_backend_fallback_used{false};
    int preview_width{0};
    int preview_height{0};
    float preview_pixel_scale{1.0f};
    std::size_t preview_sample_count{0};
    double preview_min_signed_distance_px{0.0};
    double preview_max_signed_distance_px{0.0};
    double preview_mean_signed_distance_px{0.0};
    std::string preview_field_hash;
};

struct SdfPackViewerClickAutomation {
    const std::string* control_id{nullptr};
    bool* consumed{nullptr};
};

struct SdfPackViewerSetValueAutomation {
    const std::string* control_id{nullptr};
    double value{0.0};
    bool* consumed{nullptr};
    std::string* error{nullptr};
};

const std::vector<SdfPackViewerBuiltInPackCatalogEntry>& SdfPackViewerBuiltInPackCatalog();
const char* SdfPackViewerDefaultBuiltInPackId();
std::string SdfPackViewerBuiltInPackSelectorAutomationId();
bool SdfPackViewerShouldLoadDefaultBuiltInPack(const SdfPackViewerState& state);
bool SdfPackViewerWantsEnumControl(const std::string& controlId);
bool LoadSdfPackViewerBuiltInPack(
    SdfPackViewerState* ioState,
    const std::string& exeDir,
    const std::string& packId,
    std::string* outError);

std::string SdfPackViewerControlAutomationId(const SdfPackControl& control);
std::string SdfPackViewerResetDefaultsAutomationId();
std::string SdfPackViewerUseAsSdfFieldSourceAutomationId();
bool SdfPackViewerWantsSetValueControl(const std::string& controlId);
bool SdfPackViewerWantsClickControl(const std::string& controlId);

const char* SdfPackViewerBackendPreferenceId(SdfPackFieldBackend backend);
bool TryParseSdfPackViewerBackendPreferenceId(const std::string& id, SdfPackFieldBackend* outBackend);

bool LoadSdfPackViewerJson(
    SdfPackViewerState* ioState,
    const std::string& jsonText,
    const std::string& sourcePath,
    std::string* outError);

bool LoadSdfPackViewerPack(
    SdfPackViewerState* ioState,
    const std::string& packPath,
    std::string* outError);

bool SetSdfPackViewerControlValue(
    SdfPackViewerState* ioState,
    const std::string& controlId,
    double value,
    std::string* outError);

bool ResetSdfPackViewerControlsToDefaults(SdfPackViewerState* ioState, std::string* outError);

bool RunSdfPackViewerPreview(SdfPackViewerState* ioState, std::string* outError);

SdfPackViewerAutomationReport BuildSdfPackViewerAutomationReport(const SdfPackViewerState& state);

std::string SerializeSdfPackViewerStateJson(const SdfPackViewerState& state);
bool LoadSdfPackViewerStateJson(const std::string& stateJson, SdfPackViewerState* outState, std::string* outError);
bool MergeSdfPackViewerStateIntoDiagnosticsStateJson(
    const std::string& diagnosticsStateJson,
    const SdfPackViewerState& state,
    std::string* outMergedJson,
    std::string* outError);

#ifndef SDF_PACK_VIEWER_UI_NO_IMGUI
void RenderSdfPackViewerInlinePanel(
    SdfPackViewerState* ioState,
    const std::string& exeDir,
    std::vector<ViewerUiAutomationRect>* automationRects,
    SdfPackViewerSetValueAutomation* setValueAutomation,
    SdfPackViewerClickAutomation* clickAutomation,
    bool* outInteracted);
#endif
