#pragma once
#include "fractal_types.h"
#include "ui_schema.h"
#include <string>
#include <vector>

struct BindingContext {
    ViewState* view = nullptr;
    KernelParams* params = nullptr;
    RenderSettings* render = nullptr;
    LensSettings* lens = nullptr;

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

bool ApplySchemaDefaultForControl(const UISchemaControl& c, BindingContext& ctx, bool* ioDirty);
void ApplySchemaDefaults(const UISchema& schema, BindingContext& ctx, bool* ioDirty);
bool ValidateSchemaBindings(const UISchema& schema, BindingContext& ctx, std::string* outError);
NumericControlRange ResolveNumericControlRange(const UISchemaControl& c);
bool RenderControlFromSchema(const UISchemaControl& c, BindingContext& ctx, bool* ioDirty, bool* ioRenderOnce, bool* ioInteracted);
