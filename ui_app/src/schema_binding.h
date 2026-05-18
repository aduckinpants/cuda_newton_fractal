#pragma once
#include "fractal_types.h"
#include "ui_schema.h"
#include <string>
#include <vector>

struct BindingContext {
    using NoteUiAutomationRectFn = void(*)(void* user_data, const char* control_id);

    ViewState* view = nullptr;
    KernelParams* params = nullptr;
    RenderSettings* render = nullptr;
    LensSettings* lens = nullptr;
    bool edited_camera_hp_authority = false;
    NoteUiAutomationRectFn note_ui_automation_rect = nullptr;
    void* ui_automation_user_data = nullptr;

    std::string GetEnumId(const std::string& path) const;
    bool SetEnumId(const std::string& path, const std::string& id);

    bool GetBoolValue(const std::string& path, bool& out) const;
    bool GetIntValue(const std::string& path, int& out) const;
    bool GetFloatValue(const std::string& path, float& out) const;
    bool GetDoubleValue(const std::string& path, double& out) const;

    bool EvalVisibleIf(const UISchemaPredicate& pred) const;

    bool BindFloat(const std::string& path, float** outPtr);
    bool BindDouble(const std::string& path, double** outPtr);
    bool BindInt(const std::string& path, int** outPtr);
    bool BindBool(const std::string& path, bool** outPtr);
};

struct NumericControlRange {
    double widget_min = 0.0;
    double widget_max = 0.0;
    double hard_min = 0.0;
    double hard_max = 0.0;
    bool has_widget_min = false;
    bool has_widget_max = false;
    bool has_hard_min = false;
    bool has_hard_max = false;
};

struct NumericDragWidgetBounds {
    double min = 0.0;
    double max = 0.0;
    bool has_bounds = false;
};

bool ApplySchemaDefaultForControl(const UISchemaControl& c, BindingContext& ctx, bool* ioDirty);
void ApplySchemaDefaults(const UISchema& schema, BindingContext& ctx, bool* ioDirty);
bool ValidateSchemaBindings(const UISchema& schema, BindingContext& ctx, std::string* outError);
NumericControlRange ResolveNumericControlRange(const UISchemaControl& c);
NumericDragWidgetBounds ResolveNumericDragWidgetBounds(const UISchemaControl& c);
NumericDragWidgetBounds ResolveFloatControlDragWidgetBounds(const UISchemaControl& c, const UISchemaBinding& binding);
std::vector<const UISchemaOption*> ResolveVisibleEnumOptions(const UISchemaControl& c, const BindingContext& ctx);
bool TryGetFloatControlDisplayValue(const UISchemaBinding& binding, const BindingContext& ctx, double* outValue);
bool TryGetFloatControlDragValue(const UISchemaBinding& binding, const BindingContext& ctx, double* outValue);
bool ApplyFloatControlEdit(const UISchemaBinding& binding, BindingContext& ctx, const NumericControlRange& range, double value);
bool ApplyFloatControlDragEdit(const UISchemaBinding& binding, BindingContext& ctx, const NumericControlRange& range, double value);
bool ShouldSyncViewHpFromSchemaUiMirrors(const BindingContext& ctx, Float2 uiCenterBefore, float uiZoomBefore);
bool RenderControlFromSchema(const UISchemaControl& c, BindingContext& ctx, bool* ioDirty, bool* ioRenderOnce, bool* ioInteracted);
