#include "schema_binding.h"

#include "enum_id_utils.h"
#include "explaino_seed.h"
#include "fractal_family_rules.h"
#include "imgui.h"
#include "ui_schema_grouping.h"
#include "view_hp_sync.h"

#include <cmath>
#include <cstring>
#include <string>
#include <vector>

namespace {

std::string EnumIdOrEmpty(const char* id) {
    return id ? std::string(id) : std::string();
}

template <typename EnumT, typename ParseFn>
bool ParseAndAssignEnumId(const std::string& id, EnumT* outValue, ParseFn parseFn) {
    return outValue && parseFn(id, outValue);
}

template <typename T>
void ClampNumericValue(T* value, const NumericControlRange& range) {
    if (!value) {
        return;
    }
    if (range.has_hard_min && *value < static_cast<T>(range.hard_min)) {
        *value = static_cast<T>(range.hard_min);
    }
    if (range.has_hard_max && *value > static_cast<T>(range.hard_max)) {
        *value = static_cast<T>(range.hard_max);
    }
}

bool IsCameraFloatBindingPath(const std::string& path) {
    return path == "fractal.view.center.x" ||
        path == "fractal.view.center.y" ||
        path == "fractal.view.zoom";
}

double ClampDouble(double value, double minValue, double maxValue) {
    if (value < minValue) {
        return minValue;
    }
    if (value > maxValue) {
        return maxValue;
    }
    return value;
}

double LocalLog2(double value) {
    return std::log(value) / std::log(2.0);
}

double LocalExp2(double value) {
    return std::exp(value * std::log(2.0));
}

double SafeCameraUiZoomFromLog2(double log2Zoom) {
    return LocalExp2(ClampDouble(log2Zoom, LocalLog2(1.0e-30), kMaxLog2Zoom));
}

bool TryResolveCounterfactualPairRootFamilyForPolyKindLocal(PolyKind kind, CounterfactualPairRootFamily* outFamily) {
    if (kind == PolyKind::z3_minus_1) {
        if (outFamily) *outFamily = CounterfactualPairRootFamily::cubic_unit_roots;
        return true;
    }
    if (kind == PolyKind::z4_minus_1) {
        if (outFamily) *outFamily = CounterfactualPairRootFamily::quartic_unit_roots;
        return true;
    }
    return false;
}

bool TryResolveProjectionAndFlowRootFamilyForPolyKindLocal(PolyKind kind, ProjectionAndFlowRootFamily* outFamily) {
    if (kind == PolyKind::z3_minus_1) {
        if (outFamily) *outFamily = ProjectionAndFlowRootFamily::cubic_unit_roots;
        return true;
    }
    if (kind == PolyKind::z4_minus_1) {
        if (outFamily) *outFamily = ProjectionAndFlowRootFamily::quartic_unit_roots;
        return true;
    }
    return false;
}

void SyncCounterfactualPairRootFamilyPresetLocal(KernelParams& params) {
    switch (params.counterfactual_pair_root_family) {
    case CounterfactualPairRootFamily::cubic_unit_roots:
        params.poly_kind = PolyKind::z3_minus_1;
        params.poly_coeffs[0] = -1.0f;
        params.poly_coeffs[1] = 0.0f;
        params.poly_coeffs[2] = 0.0f;
        params.poly_coeffs[3] = 1.0f;
        params.poly_coeffs[4] = 0.0f;
        break;
    case CounterfactualPairRootFamily::quartic_unit_roots:
        params.poly_kind = PolyKind::z4_minus_1;
        params.poly_coeffs[0] = -1.0f;
        params.poly_coeffs[1] = 0.0f;
        params.poly_coeffs[2] = 0.0f;
        params.poly_coeffs[3] = 0.0f;
        params.poly_coeffs[4] = 1.0f;
        break;
    }
}

void SyncProjectionAndFlowRootFamilyPresetLocal(KernelParams& params) {
    switch (params.projection_and_flow_root_family) {
    case ProjectionAndFlowRootFamily::cubic_unit_roots:
        params.poly_kind = PolyKind::z3_minus_1;
        params.poly_coeffs[0] = -1.0f;
        params.poly_coeffs[1] = 0.0f;
        params.poly_coeffs[2] = 0.0f;
        params.poly_coeffs[3] = 1.0f;
        params.poly_coeffs[4] = 0.0f;
        break;
    case ProjectionAndFlowRootFamily::quartic_unit_roots:
        params.poly_kind = PolyKind::z4_minus_1;
        params.poly_coeffs[0] = -1.0f;
        params.poly_coeffs[1] = 0.0f;
        params.poly_coeffs[2] = 0.0f;
        params.poly_coeffs[3] = 0.0f;
        params.poly_coeffs[4] = 1.0f;
        break;
    }
}

void SyncCameraUiMirrorFromHp(ViewState& view) {
    double zoom = SafeCameraUiZoomFromLog2(view.log2_zoom);
    zoom = ClampDouble(zoom, 1.0e-30, 1.0e30);
    view.zoom = static_cast<float>(zoom);
    view.center.x = static_cast<float>(view.center_hp_x);
    view.center.y = static_cast<float>(view.center_hp_y);
}

const char* FloatControlDisplayFormat(const UISchemaControl& control, const UISchemaBinding& binding) {
    if (binding.path == "fractal.view.zoom" || control.logarithmic) {
        return "%.6g";
    }
    return control.type == "slider_float" ? "%.5f" : "%.3f";
}

const char* FloatControlInputFormat(const UISchemaControl& control, const UISchemaBinding& binding) {
    if (binding.path == "fractal.view.zoom" || control.logarithmic) {
        return "%.9g";
    }
    return "%.5f";
}

bool IsAllowedColoringModeForBinding(const BindingContext& ctx, ColoringMode mode) {
    return !ctx.view || IsColoringModeAllowedForFractal(ctx.view->fractal_type, mode);
}

template <typename MatchFn>
bool TrySelectAllowedColorPipeline(
    const BindingContext& ctx,
    MatchFn match,
    ColorPipelineSelection* outPipeline,
    ColoringMode* outMode) {
    auto matchesPipeline = [&](const ColorPipelineSelection& pipeline, ColoringMode* outMirroredMode = nullptr) {
        ColoringMode mirroredMode = ColoringMode::root_basin;
        if (!TryMirroredColoringModeForPipeline(pipeline, &mirroredMode)) {
            return false;
        }
        if (ctx.view && !IsColorPipelineAllowedForFractal(ctx.view->fractal_type, pipeline)) {
            return false;
        }
        if (!match(pipeline)) {
            return false;
        }
        if (outMirroredMode) {
            *outMirroredMode = mirroredMode;
        }
        return true;
    };

    if (ctx.params) {
        ColoringMode currentMode = ColoringMode::root_basin;
        if (matchesPipeline(ctx.params->color_pipeline, &currentMode)) {
            if (outPipeline) *outPipeline = ctx.params->color_pipeline;
            if (outMode) *outMode = currentMode;
            return true;
        }
    }

    if (ctx.view) {
        const ColoringMode defaultMode = DefaultColoringModeForFractal(ctx.view->fractal_type);
        for (const ColorPipelineSelection& pipeline : kSelectableColorPipelines) {
            ColoringMode mirroredMode = ColoringMode::root_basin;
            if (matchesPipeline(pipeline, &mirroredMode) && mirroredMode == defaultMode) {
                if (outPipeline) *outPipeline = pipeline;
                if (outMode) *outMode = mirroredMode;
                return true;
            }
        }
    }

    for (const ColorPipelineSelection& pipeline : kSelectableColorPipelines) {
        ColoringMode mirroredMode = ColoringMode::root_basin;
        if (matchesPipeline(pipeline, &mirroredMode)) {
            if (outPipeline) *outPipeline = pipeline;
            if (outMode) *outMode = mirroredMode;
            return true;
        }
    }

    return false;
}

template <typename MatchFn>
bool ApplySelectedColorPipeline(BindingContext* ctx, MatchFn match) {
    if (!ctx || !ctx->params) {
        return false;
    }

    ColorPipelineSelection nextPipeline{};
    ColoringMode nextMode = ColoringMode::root_basin;
    if (!TrySelectAllowedColorPipeline(*ctx, match, &nextPipeline, &nextMode)) {
        return false;
    }

    ctx->params->color_pipeline = nextPipeline;
    ctx->params->coloring_mode = nextMode;
    return true;
}

bool SetCounterfactualPairPolyKind(BindingContext* ctx, const std::string& id) {
    if (!ctx || !ctx->params) {
        return false;
    }

    PolyKind nextKind = ctx->params->poly_kind;
    if (!ParseAndAssignEnumId(id, &nextKind, TryParsePolyKindId)) {
        return false;
    }
    if (ctx->view && ctx->view->fractal_type == FractalType::counterfactual_pair) {
        CounterfactualPairRootFamily nextFamily = ctx->params->counterfactual_pair_root_family;
        if (!TryResolveCounterfactualPairRootFamilyForPolyKindLocal(nextKind, &nextFamily)) {
            return false;
        }
        ctx->params->counterfactual_pair_root_family = nextFamily;
    }
    ctx->params->poly_kind = nextKind;
    return true;
}

bool SetCounterfactualPairRootFamily(BindingContext* ctx, const std::string& id) {
    if (!ctx || !ctx->params) {
        return false;
    }

    CounterfactualPairRootFamily nextFamily = ctx->params->counterfactual_pair_root_family;
    if (!ParseAndAssignEnumId(id, &nextFamily, TryParseCounterfactualPairRootFamilyId)) {
        return false;
    }
    ctx->params->counterfactual_pair_root_family = nextFamily;
    if (ctx->view && ctx->view->fractal_type == FractalType::counterfactual_pair) {
        SyncCounterfactualPairRootFamilyPresetLocal(*ctx->params);
    }
    return true;
}

bool SetCounterfactualPairFrame(BindingContext* ctx, const std::string& id) {
    return ctx && ctx->params &&
        ParseAndAssignEnumId(id, &ctx->params->counterfactual_pair_frame, TryParseCounterfactualPairFrameId);
}

bool SetProjectionAndFlowRootFamily(BindingContext* ctx, const std::string& id) {
    if (!ctx || !ctx->params) {
        return false;
    }

    ProjectionAndFlowRootFamily nextFamily = ctx->params->projection_and_flow_root_family;
    if (!ParseAndAssignEnumId(id, &nextFamily, TryParseProjectionAndFlowRootFamilyId)) {
        return false;
    }
    ctx->params->projection_and_flow_root_family = nextFamily;
    if (ctx->view && ctx->view->fractal_type == FractalType::projection_and_flow) {
        SyncProjectionAndFlowRootFamilyPresetLocal(*ctx->params);
    }
    return true;
}

bool SetColoringMode(BindingContext* ctx, const std::string& id) {
    if (!ctx || !ctx->params) {
        return false;
    }

    ColoringMode mode = ColoringMode::root_basin;
    if (!ParseAndAssignEnumId(id, &mode, TryParseColoringModeId)) {
        return false;
    }
    if (!IsAllowedColoringModeForBinding(*ctx, mode)) {
        return false;
    }
    ctx->params->coloring_mode = mode;
    ctx->params->color_pipeline = ColorPipelineForLegacyMode(mode);
    return true;
}

bool SetColorSignal(BindingContext* ctx, const std::string& id) {
    if (!ctx || !ctx->params) {
        return false;
    }

    ColorSignal signal = ColorSignal::root_index;
    if (!ParseAndAssignEnumId(id, &signal, TryParseColorSignalId)) {
        return false;
    }
    return ApplySelectedColorPipeline(ctx,
        [signal](const ColorPipelineSelection& pipeline) { return pipeline.signal == signal; });
}

bool SetColorPalette(BindingContext* ctx, const std::string& id) {
    if (!ctx || !ctx->params) {
        return false;
    }

    ColorPalette palette = ColorPalette::root_classic;
    if (!ParseAndAssignEnumId(id, &palette, TryParseColorPaletteId)) {
        return false;
    }
    return ApplySelectedColorPipeline(ctx,
        [palette](const ColorPipelineSelection& pipeline) { return pipeline.palette == palette; });
}

bool SetColorGrading(BindingContext* ctx, const std::string& id) {
    if (!ctx || !ctx->params) {
        return false;
    }

    ColorGradingPreset grading = ColorGradingPreset::basin_default;
    if (!ParseAndAssignEnumId(id, &grading, TryParseColorGradingPresetId)) {
        return false;
    }
    return ApplySelectedColorPipeline(ctx,
        [grading](const ColorPipelineSelection& pipeline) { return pipeline.grading == grading; });
}

bool SetFractalType(BindingContext* ctx, const std::string& id) {
    if (!ctx || !ctx->view) {
        return false;
    }

    FractalType fractalType = ctx->view->fractal_type;
    if (!TryParseFractalTypeId(id, &fractalType)) {
        return false;
    }
    ctx->view->fractal_type = fractalType;
    if (ctx->params &&
        (fractalType == ExplainoCanonicalFractalType() || IsExplainoLegacyProjectionSelector(fractalType))) {
        ApplyExplainoAxisRegistryDefaults(fractalType, *ctx->params);
    }
    return true;
}

} // namespace

bool TryGetFloatControlDisplayValue(const UISchemaBinding& binding, const BindingContext& ctx, double* outValue) {
    if (!outValue) {
        return false;
    }

    if (binding.path == "fractal.view.center.x") {
        if (!ctx.view) {
            return false;
        }
        *outValue = ctx.view->center_hp_x;
        return true;
    }
    if (binding.path == "fractal.view.center.y") {
        if (!ctx.view) {
            return false;
        }
        *outValue = ctx.view->center_hp_y;
        return true;
    }
    if (binding.path == "fractal.view.zoom") {
        if (!ctx.view) {
            return false;
        }
        *outValue = SafeCameraUiZoomFromLog2(ctx.view->log2_zoom);
        return true;
    }

    float value = 0.0f;
    if (!ctx.GetFloatValue(binding.path, value)) {
        return false;
    }
    *outValue = static_cast<double>(value);
    return true;
}

bool TryGetFloatControlDragValue(const UISchemaBinding& binding, const BindingContext& ctx, double* outValue) {
    if (!outValue) {
        return false;
    }

    if (binding.path == "fractal.view.zoom") {
        if (!ctx.view) {
            return false;
        }
        *outValue = ClampDouble(ctx.view->log2_zoom, LocalLog2(kMinZoom), kMaxLog2Zoom);
        return true;
    }

    return TryGetFloatControlDisplayValue(binding, ctx, outValue);
}

bool ApplyFloatControlEdit(const UISchemaBinding& binding, BindingContext& ctx, const NumericControlRange& range, double value) {
    double nextValue = value;
    ClampNumericValue(&nextValue, range);

    if (binding.path == "fractal.view.center.x") {
        if (!ctx.view) {
            return false;
        }
        ctx.view->center_hp_x = nextValue;
        SyncCameraUiMirrorFromHp(*ctx.view);
        ctx.edited_camera_hp_authority = true;
        return true;
    }
    if (binding.path == "fractal.view.center.y") {
        if (!ctx.view) {
            return false;
        }
        ctx.view->center_hp_y = nextValue;
        SyncCameraUiMirrorFromHp(*ctx.view);
        ctx.edited_camera_hp_authority = true;
        return true;
    }
    if (binding.path == "fractal.view.zoom") {
        if (!ctx.view) {
            return false;
        }
        ctx.view->log2_zoom = LocalLog2((std::fmax)(1.0e-30, nextValue));
        SyncCameraUiMirrorFromHp(*ctx.view);
        ctx.edited_camera_hp_authority = true;
        return true;
    }

    float* target = nullptr;
    if (!ctx.BindFloat(binding.path, &target) || !target) {
        return false;
    }
    float nextFloat = static_cast<float>(nextValue);
    ClampNumericValue(&nextFloat, range);
    *target = nextFloat;
    return true;
}

bool ApplyFloatControlDragEdit(const UISchemaBinding& binding, BindingContext& ctx, const NumericControlRange& range, double value) {
    if (binding.path == "fractal.view.zoom") {
        if (!ctx.view) {
            return false;
        }

        const double minLog2Zoom = range.has_hard_min ? LocalLog2((std::fmax)(1.0e-30, range.hard_min)) : LocalLog2(kMinZoom);
        const double maxLog2Zoom = range.has_hard_max ? LocalLog2((std::fmax)(1.0e-30, range.hard_max)) : kMaxLog2Zoom;
        ctx.view->log2_zoom = ClampDouble(value, minLog2Zoom, maxLog2Zoom);
        SyncCameraUiMirrorFromHp(*ctx.view);
        ctx.edited_camera_hp_authority = true;
        return true;
    }

    return ApplyFloatControlEdit(binding, ctx, range, value);
}

bool ShouldSyncViewHpFromSchemaUiMirrors(const BindingContext& ctx, Float2 uiCenterBefore, float uiZoomBefore) {
    if (!ctx.view) {
        return false;
    }
    const bool cameraUiMirrorChanged =
        ctx.view->center.x != uiCenterBefore.x ||
        ctx.view->center.y != uiCenterBefore.y ||
        ctx.view->zoom != uiZoomBefore;
    return cameraUiMirrorChanged && !ctx.edited_camera_hp_authority;
}

NumericControlRange ResolveNumericControlRange(const UISchemaControl& control) {
    NumericControlRange range;
    if (control.has_ui_min) {
        range.widget_min = control.ui_min;
        range.has_widget_min = true;
    } else if (control.has_min) {
        range.widget_min = control.min;
        range.has_widget_min = true;
    }
    if (control.has_ui_max) {
        range.widget_max = control.ui_max;
        range.has_widget_max = true;
    } else if (control.has_max) {
        range.widget_max = control.max;
        range.has_widget_max = true;
    }
    if (control.has_min) {
        range.hard_min = control.min;
        range.has_hard_min = true;
    }
    if (control.has_max) {
        range.hard_max = control.max;
        range.has_hard_max = true;
    }
    return range;
}

NumericDragWidgetBounds ResolveNumericDragWidgetBounds(const UISchemaControl& control) {
    NumericDragWidgetBounds bounds;
    const NumericControlRange range = ResolveNumericControlRange(control);
    if (range.has_hard_min && range.has_hard_max && range.hard_max > range.hard_min) {
        bounds.min = range.hard_min;
        bounds.max = range.hard_max;
        bounds.has_bounds = true;
    }
    return bounds;
}

NumericDragWidgetBounds ResolveFloatControlDragWidgetBounds(const UISchemaControl& control, const UISchemaBinding& binding) {
    (void)binding;
    return ResolveNumericDragWidgetBounds(control);
}

std::vector<const UISchemaOption*> ResolveVisibleEnumOptions(const UISchemaControl& control, const BindingContext& ctx) {
    std::vector<const UISchemaOption*> options;
    options.reserve(control.options.size());

    const bool filterColoringModeOptions = control.value_type == "enum" &&
        control.has_binding &&
        control.binding.path == "fractal.params.coloring_mode" &&
        ctx.view;
    const bool filterColorSignalOptions = control.value_type == "enum" &&
        control.has_binding &&
        control.binding.path == "fractal.params.color_signal" &&
        ctx.params;
    const bool filterColorPaletteOptions = control.value_type == "enum" &&
        control.has_binding &&
        control.binding.path == "fractal.params.color_palette" &&
        ctx.params;
    const bool filterColorGradingOptions = control.value_type == "enum" &&
        control.has_binding &&
        control.binding.path == "fractal.params.color_grading" &&
        ctx.params;

    for (const auto& option : control.options) {
        if (filterColoringModeOptions) {
            ColoringMode mode = ColoringMode::smooth_escape;
            if (!TryParseColoringModeId(option.id, &mode)) {
                continue;
            }
            if (!IsColoringModeAllowedForFractal(ctx.view->fractal_type, mode)) {
                continue;
            }
        }
        if (filterColorSignalOptions) {
            ColorSignal signal = ColorSignal::root_index;
            if (!TryParseColorSignalId(option.id, &signal)) {
                continue;
            }
            if (!TrySelectAllowedColorPipeline(ctx,
                    [signal](const ColorPipelineSelection& pipeline) { return pipeline.signal == signal; },
                    nullptr,
                    nullptr)) {
                continue;
            }
        }
        if (filterColorPaletteOptions) {
            ColorPalette palette = ColorPalette::root_classic;
            if (!TryParseColorPaletteId(option.id, &palette)) {
                continue;
            }
            if (!TrySelectAllowedColorPipeline(ctx,
                    [palette](const ColorPipelineSelection& pipeline) { return pipeline.palette == palette; },
                    nullptr,
                    nullptr)) {
                continue;
            }
        }
        if (filterColorGradingOptions) {
            ColorGradingPreset grading = ColorGradingPreset::basin_default;
            if (!TryParseColorGradingPresetId(option.id, &grading)) {
                continue;
            }
            if (!TrySelectAllowedColorPipeline(ctx,
                    [grading](const ColorPipelineSelection& pipeline) { return pipeline.grading == grading; },
                    nullptr,
                    nullptr)) {
                continue;
            }
        }
        options.push_back(&option);
    }

    return options;
}

std::string BindingContext::GetEnumId(const std::string& path) const {
    if (params && path == "fractal.params.poly_kind") {
        return EnumIdOrEmpty(PolyKindId(params->poly_kind));
    }
    if (params && path == "fractal.params.counterfactual_pair_root_family") {
        return EnumIdOrEmpty(CounterfactualPairRootFamilyId(params->counterfactual_pair_root_family));
    }
    if (params && path == "fractal.params.counterfactual_pair_frame") {
        return EnumIdOrEmpty(CounterfactualPairFrameId(params->counterfactual_pair_frame));
    }
    if (params && path == "fractal.params.projection_and_flow_root_family") {
        return EnumIdOrEmpty(ProjectionAndFlowRootFamilyId(params->projection_and_flow_root_family));
    }
    if (params && path == "fractal.params.transcendental_func") {
        return EnumIdOrEmpty(TranscendentalFuncId(params->transcendental_func));
    }
    if (params && path == "fractal.params.mcmullen_preset") {
        return EnumIdOrEmpty(McMullenPresetId(params->mcmullen_preset));
    }
    if (params && path == "fractal.params.coloring_mode") {
        return EnumIdOrEmpty(ColoringModeId(params->coloring_mode));
    }
    if (params && path == "fractal.params.color_signal") {
        return EnumIdOrEmpty(ColorSignalId(params->color_pipeline.signal));
    }
    if (params && path == "fractal.params.color_palette") {
        return EnumIdOrEmpty(ColorPaletteId(params->color_pipeline.palette));
    }
    if (params && path == "fractal.params.color_grading") {
        return EnumIdOrEmpty(ColorGradingPresetId(params->color_pipeline.grading));
    }
    if (view && path == "fractal.view.fractal_type") {
        return EnumIdOrEmpty(FractalTypeId(ResolveExplainoPublicFractalType(view->fractal_type)));
    }
    if (view && path == "fractal.view.camera_behavior") {
        return EnumIdOrEmpty(CameraBehaviorId(view->camera_behavior));
    }
    if (render && path == "fractal.render.sample_tier") {
        return EnumIdOrEmpty(SampleTierId(render->sample_tier));
    }
    if (view && path == "fractal.view.param_anim_target") {
        return std::string(view->param_anim_target);
    }
    return {};
}

bool BindingContext::SetEnumId(const std::string& path, const std::string& id) {
    if (params && path == "fractal.params.poly_kind") {
        return SetCounterfactualPairPolyKind(this, id);
    }
    if (params && path == "fractal.params.counterfactual_pair_root_family") {
        return SetCounterfactualPairRootFamily(this, id);
    }
    if (params && path == "fractal.params.counterfactual_pair_frame") {
        return SetCounterfactualPairFrame(this, id);
    }
    if (params && path == "fractal.params.projection_and_flow_root_family") {
        return SetProjectionAndFlowRootFamily(this, id);
    }
    if (params && path == "fractal.params.transcendental_func") {
        return ParseAndAssignEnumId(id, &params->transcendental_func, TryParseTranscendentalFuncId);
    }
    if (params && path == "fractal.params.mcmullen_preset") {
        return ParseAndAssignEnumId(id, &params->mcmullen_preset, TryParseMcMullenPresetId);
    }
    if (params && path == "fractal.params.coloring_mode") {
        return SetColoringMode(this, id);
    }
    if (params && path == "fractal.params.color_signal") {
        return SetColorSignal(this, id);
    }
    if (params && path == "fractal.params.color_palette") {
        return SetColorPalette(this, id);
    }
    if (params && path == "fractal.params.color_grading") {
        return SetColorGrading(this, id);
    }
    if (view && path == "fractal.view.fractal_type") {
        return SetFractalType(this, id);
    }
    if (view && path == "fractal.view.camera_behavior") {
        return ParseAndAssignEnumId(id, &view->camera_behavior, TryParseCameraBehaviorId);
    }
    if (render && path == "fractal.render.sample_tier") {
        return ParseAndAssignEnumId(id, &render->sample_tier, TryParseSampleTierId);
    }
    if (view && path == "fractal.view.param_anim_target") {
        if (id.size() >= sizeof(view->param_anim_target)) return false;
        std::memcpy(view->param_anim_target, id.c_str(), id.size() + 1);
        return true;
    }
    return false;
}

bool BindingContext::GetBoolValue(const std::string& path, bool& out) const {
    bool* ptr = nullptr;
    BindingContext* self = const_cast<BindingContext*>(this);
    if (!self->BindBool(path, &ptr) || !ptr) return false;
    out = *ptr;
    return true;
}

bool BindingContext::GetIntValue(const std::string& path, int& out) const {
    int* ptr = nullptr;
    BindingContext* self = const_cast<BindingContext*>(this);
    if (!self->BindInt(path, &ptr) || !ptr) return false;
    out = *ptr;
    return true;
}

bool BindingContext::GetFloatValue(const std::string& path, float& out) const {
    float* ptr = nullptr;
    BindingContext* self = const_cast<BindingContext*>(this);
    if (!self->BindFloat(path, &ptr) || !ptr) return false;
    out = *ptr;
    return true;
}

bool BindingContext::GetDoubleValue(const std::string& path, double& out) const {
    double* ptr = nullptr;
    BindingContext* self = const_cast<BindingContext*>(this);
    if (!self->BindDouble(path, &ptr) || !ptr) return false;
    out = *ptr;
    return true;
}

bool BindingContext::EvalVisibleIf(const UISchemaPredicate& pred) const {
    if (pred.op.empty() || pred.path.empty()) return false;

    // Enum predicates
    std::string curEnum = GetEnumId(pred.path);
    if (!curEnum.empty()) {
        if (pred.op == "eq") return curEnum == pred.value;
        if (pred.op == "neq") return curEnum != pred.value;
        if (pred.op == "in") {
            // Comma-separated list: "a,b,c" (whitespace tolerated).
            std::string v = pred.value;
            size_t i = 0;
            while (i < v.size()) {
                while (i < v.size() && (v[i] == ' ' || v[i] == '\t' || v[i] == '\n' || v[i] == '\r' || v[i] == ',')) ++i;
                size_t j = i;
                while (j < v.size() && v[j] != ',') ++j;
                std::string tok = v.substr(i, j - i);
                // Trim trailing whitespace
                while (!tok.empty() && (tok.back() == ' ' || tok.back() == '\t' || tok.back() == '\n' || tok.back() == '\r')) tok.pop_back();
                if (!tok.empty() && tok == curEnum) return true;
                i = j + 1;
            }
            return false;
        }
        return false;
    }

    // Bool predicates
    bool curB = false;
    if (GetBoolValue(pred.path, curB)) {
        bool rhs = (pred.value == "true" || pred.value == "1");
        if (pred.op == "eq") return curB == rhs;
        if (pred.op == "neq") return curB != rhs;
        return false;
    }

    // Numeric predicates (int/float)
    double curN = 0.0;
    {
        int curI = 0;
        float curF = 0.0f;
        double curD = 0.0;
        if (GetIntValue(pred.path, curI)) curN = (double)curI;
        else if (GetFloatValue(pred.path, curF)) curN = (double)curF;
        else if (GetDoubleValue(pred.path, curD)) curN = curD;
        else return false;
    }

    double rhsN = 0.0;
    try {
        rhsN = std::stod(pred.value);
    } catch (...) {
        return false;
    }

    if (pred.op == "eq") return curN == rhsN;
    if (pred.op == "neq") return curN != rhsN;
    if (pred.op == "lt") return curN < rhsN;
    if (pred.op == "lte") return curN <= rhsN;
    if (pred.op == "gt") return curN > rhsN;
    if (pred.op == "gte") return curN >= rhsN;
    return false;
}

bool BindingContext::BindFloat(const std::string& path, float** outPtr) {
    if (view) {
        if (path == "fractal.view.center.x") { *outPtr = &view->center.x; return true; }
        if (path == "fractal.view.center.y") { *outPtr = &view->center.y; return true; }
        if (path == "fractal.view.zoom") { *outPtr = &view->zoom; return true; }
        if (path == "fractal.view.rotation") { *outPtr = &view->rotation_degrees; return true; }
        if (path == "fractal.view.dive_speed") { *outPtr = &view->dive_speed; return true; }
        if (path == "fractal.view.explaino_phase") { *outPtr = &view->explaino_phase; return true; }
        if (path == "fractal.view.explaino_seed_drift") { *outPtr = &view->explaino_seed_drift; return true; }
        if (path == "fractal.view.explaino_seed_rate") { *outPtr = &view->explaino_seed_rate; return true; }
        if (path == "fractal.view.explaino_phase_strength") { *outPtr = &view->explaino_phase_strength; return true; }
        if (path == "fractal.view.param_anim_rate") { *outPtr = &view->param_anim_rate; return true; }
    }
    if (params) {
        if (path == "fractal.params.epsilon") { *outPtr = &params->epsilon; return true; }
        if (path == "fractal.params.counterfactual_pair_offset_x") { *outPtr = &params->counterfactual_pair_offset_x; return true; }
        if (path == "fractal.params.counterfactual_pair_offset_y") { *outPtr = &params->counterfactual_pair_offset_y; return true; }
        if (path == "fractal.params.counterfactual_pair_reconvergence_ratio") { *outPtr = &params->counterfactual_pair_reconvergence_ratio; return true; }
        if (path == "fractal.params.projection_and_flow_target_radius") { *outPtr = &params->projection_and_flow_target_radius; return true; }
        if (path == "fractal.params.projection_and_flow_pressure_threshold") { *outPtr = &params->projection_and_flow_pressure_threshold; return true; }
        if (path == "fractal.params.nova_alpha") { *outPtr = &params->nova_alpha; return true; }
        if (path == "fractal.params.phoenix_p_real") { *outPtr = &params->phoenix_p_real; return true; }
        if (path == "fractal.params.phoenix_p_imag") { *outPtr = &params->phoenix_p_imag; return true; }
        if (path == "fractal.params.lambda_real") { *outPtr = &params->lambda_real; return true; }
        if (path == "fractal.params.lambda_imag") { *outPtr = &params->lambda_imag; return true; }
        if (path == "fractal.params.exposure") { *outPtr = &params->exposure; return true; }
        if (path == "fractal.params.multibrot_power_float") { *outPtr = &params->multibrot_power_float; return true; }
        if (path == "fractal.params.multibrot_power") { *outPtr = &params->multibrot_power_float; return true; }
        if (path == "fractal.params.color_saturation") { *outPtr = &params->color_saturation; return true; }
        if (path == "fractal.params.color_contrast") { *outPtr = &params->color_contrast; return true; }
        if (path == "fractal.params.color_tint_r") { *outPtr = &params->color_tint_r; return true; }
        if (path == "fractal.params.color_tint_g") { *outPtr = &params->color_tint_g; return true; }
        if (path == "fractal.params.color_tint_b") { *outPtr = &params->color_tint_b; return true; }
        if (path == "fractal.params.explaino_warp_strength") { *outPtr = &params->explaino_warp_strength; return true; }
        if (path == "fractal.params.explaino_mix") { *outPtr = &params->explaino_mix; return true; }
        if (path == "fractal.params.explaino_root_spread") { *outPtr = &params->explaino_root_spread; return true; }
        if (path == "fractal.params.explaino_damping") { *outPtr = &params->explaino_damping; return true; }
        if (path == "fractal.params.explaino_cluster_radius") { *outPtr = &params->explaino_cluster_radius; return true; }
        if (path == "fractal.params.momentum_beta") { *outPtr = &params->momentum_beta; return true; }
        if (path == "fractal.params.joy_coupling") { *outPtr = &params->joy_coupling; return true; }
        if (path == "fractal.params.fold_coupling") { *outPtr = &params->fold_coupling; return true; }
        if (path == "fractal.params.bell_coupling") { *outPtr = &params->bell_coupling; return true; }
        if (path == "fractal.params.balance_void") { *outPtr = &params->balance_void; return true; }
        if (path == "fractal.params.symmetry_tension") { *outPtr = &params->symmetry_tension; return true; }
        if (path == "fractal.params.field_curvature") { *outPtr = &params->field_curvature; return true; }
        if (path == "fractal.params.ripple_amplitude") { *outPtr = &params->ripple_amplitude; return true; }
        if (path == "fractal.params.splice_offset") { *outPtr = &params->splice_offset; return true; }
        if (path == "fractal.params.vortex_strength") { *outPtr = &params->vortex_strength; return true; }
        if (path == "fractal.params.tension_strength") { *outPtr = &params->tension_strength; return true; }
        // Short-name aliases for param_anim_target resolution.
        // These let the animation system resolve "damping" -> explaino_damping, etc.
        if (path == "fractal.params.damping") { *outPtr = &params->explaino_damping; return true; }
        if (path == "fractal.params.warp_strength") { *outPtr = &params->explaino_warp_strength; return true; }
        if (path == "fractal.params.root_spread") { *outPtr = &params->explaino_root_spread; return true; }
        if (path == "fractal.params.mix") { *outPtr = &params->explaino_mix; return true; }
        if (path == "fractal.params.poly_coeffs.0") { *outPtr = &params->poly_coeffs[0]; return true; }
        if (path == "fractal.params.poly_coeffs.1") { *outPtr = &params->poly_coeffs[1]; return true; }
        if (path == "fractal.params.poly_coeffs.2") { *outPtr = &params->poly_coeffs[2]; return true; }
        if (path == "fractal.params.poly_coeffs.3") { *outPtr = &params->poly_coeffs[3]; return true; }
        if (path == "fractal.params.poly_coeffs.4") { *outPtr = &params->poly_coeffs[4]; return true; }
    }
    if (render) {
        if (path == "fractal.render.preview_target_fps") { *outPtr = &render->preview_target_fps; return true; }
        if (path == "fractal.render.preview_min_scale") { *outPtr = &render->preview_min_scale; return true; }
    }
    return false;
}

bool BindingContext::BindDouble(const std::string& path, double** outPtr) {
    if (!params) return false;
    if (path == "fractal.params.explaino_seed") { *outPtr = &params->explaino_seed; return true; }
    if (path == "fractal.params.explaino_seed_b") { *outPtr = &params->explaino_seed_b; return true; }
    return false;
}

bool BindingContext::BindInt(const std::string& path, int** outPtr) {
    if (params) {
        if (path == "fractal.params.max_iter") { *outPtr = &params->max_iter; return true; }
        if (path == "fractal.params.multibrot_power") { *outPtr = &params->multibrot_power; return true; }
    }
    if (render) {
        if (path == "fractal.render.resolution.x") { *outPtr = &render->resolution.x; return true; }
        if (path == "fractal.render.resolution.y") { *outPtr = &render->resolution.y; return true; }
        if (path == "fractal.render.block_size") { *outPtr = &render->block_size; return true; }
        if (path == "fractal.render.device_id") { *outPtr = &render->device_id; return true; }
        if (path == "fractal.render.interaction_debounce_ms") { *outPtr = &render->interaction_debounce_ms; return true; }
    }
    if (lens && path == "fractal.lens.downsample") { *outPtr = &lens->downsample; return true; }
    return false;
}

bool BindingContext::BindBool(const std::string& path, bool** outPtr) {
    if (view) {
        if (path == "fractal.view.auto_refresh") { *outPtr = &view->auto_refresh; return true; }
        if (path == "fractal.view.auto_dive") { *outPtr = &view->auto_dive; return true; }
        if (path == "fractal.view.auto_max_iter") { *outPtr = &view->auto_max_iter; return true; }
        if (path == "fractal.view.explaino_alive") { *outPtr = &view->explaino_alive; return true; }
        if (path == "fractal.view.explaino_seed_tween") { *outPtr = &view->explaino_seed_tween; return true; }
        if (path == "fractal.view.auto_increment_seed") { *outPtr = &view->auto_increment_seed; return true; }
    }
    if (render) {
        if (path == "fractal.render.benchmark") { *outPtr = &render->benchmark; return true; }
    }
    if (lens && path == "fractal.lens.enabled") { *outPtr = &lens->enabled; return true; }
    return false;
}

namespace {

bool IsValidBoolPredicateValue(const std::string& value) {
    return value == "true" || value == "false" || value == "1" || value == "0";
}

bool ValidateVisibleIfPredicate(const UISchemaControl& control, BindingContext& ctx, std::string* outError) {
    if (!control.has_visible_if) {
        return true;
    }

    const UISchemaPredicate& pred = control.visible_if;
    if (pred.op.empty() || pred.path.empty()) {
        if (outError) *outError = "Invalid visible_if predicate for control: " + control.id;
        return false;
    }

    const std::string enumValue = ctx.GetEnumId(pred.path);
    if (!enumValue.empty()) {
        if (pred.op == "eq" || pred.op == "neq" || pred.op == "in") {
            return true;
        }
        if (outError) *outError = "Invalid visible_if enum operator for control: " + control.id + " (path: " + pred.path + ", op: " + pred.op + ")";
        return false;
    }

    bool boolValue = false;
    if (ctx.GetBoolValue(pred.path, boolValue)) {
        if ((pred.op == "eq" || pred.op == "neq") && IsValidBoolPredicateValue(pred.value)) {
            return true;
        }
        if (outError) *outError = "Invalid visible_if bool predicate for control: " + control.id + " (path: " + pred.path + ", op: " + pred.op + ", value: " + pred.value + ")";
        return false;
    }

    int intValue = 0;
    float floatValue = 0.0f;
    double doubleValue = 0.0;
    if (ctx.GetIntValue(pred.path, intValue) || ctx.GetFloatValue(pred.path, floatValue) || ctx.GetDoubleValue(pred.path, doubleValue)) {
        if (!(pred.op == "eq" || pred.op == "neq" || pred.op == "lt" || pred.op == "lte" || pred.op == "gt" || pred.op == "gte")) {
            if (outError) *outError = "Invalid visible_if numeric operator for control: " + control.id + " (path: " + pred.path + ", op: " + pred.op + ")";
            return false;
        }
        try {
            (void)std::stod(pred.value);
        } catch (...) {
            if (outError) *outError = "Invalid visible_if numeric value for control: " + control.id + " (path: " + pred.path + ", value: " + pred.value + ")";
            return false;
        }
        return true;
    }

    if (outError) *outError = "Unknown visible_if binding path: " + pred.path + " (control: " + control.id + ")";
    return false;
}

bool ValidateIntComboOptions(const UISchemaControl& control, std::string* outError) {
    if (control.value_type != "int" || control.type != "combo") {
        return true;
    }

    for (const auto& option : control.options) {
        try {
            (void)std::stoi(option.id);
        } catch (...) {
            if (outError) *outError = "Invalid int combo option id: " + option.id + " (control: " + control.id + ")";
            return false;
        }
    }
    return true;
}

bool ApplyBoolSchemaDefault(const UISchemaControl& control, BindingContext& ctx, bool* ioDirty) {
    bool* value = nullptr;
    if (!ctx.BindBool(control.binding.path, &value) || !value) return false;

    bool newValue = *value;
    if (control.def.is_bool()) newValue = control.def.as_bool();
    else if (control.def.is_number()) newValue = (control.def.as_number() != 0.0);
    else if (control.def.is_string()) newValue = (control.def.as_string() == "true" || control.def.as_string() == "1");
    else return false;

    if (*value == newValue) return false;
    *value = newValue;
    if (ioDirty) *ioDirty = true;
    return true;
}

bool ApplyIntSchemaDefault(const UISchemaControl& control, BindingContext& ctx, bool* ioDirty) {
    int* value = nullptr;
    if (!ctx.BindInt(control.binding.path, &value) || !value) return false;

    int newValue = *value;
    if (control.def.is_number()) newValue = static_cast<int>(control.def.as_number());
    else if (control.def.is_string()) {
        try {
            newValue = std::stoi(control.def.as_string());
        } catch (...) {
            return false;
        }
    } else {
        return false;
    }

    if (*value == newValue) return false;
    *value = newValue;
    if (ioDirty) *ioDirty = true;
    return true;
}

bool ApplyFloatSchemaDefault(const UISchemaControl& control, BindingContext& ctx, bool* ioDirty) {
    double currentValue = 0.0;
    if (!TryGetFloatControlDisplayValue(control.binding, ctx, &currentValue)) return false;

    double newValue = currentValue;
    if (control.def.is_number()) newValue = control.def.as_number();
    else if (control.def.is_string()) {
        try {
            newValue = std::stod(control.def.as_string());
        } catch (...) {
            return false;
        }
    } else {
        return false;
    }

    if (currentValue == newValue) return false;
    if (!ApplyFloatControlEdit(control.binding, ctx, ResolveNumericControlRange(control), newValue)) {
        return false;
    }
    if (ioDirty) *ioDirty = true;
    return true;
}

bool ApplyDoubleSchemaDefault(const UISchemaControl& control, BindingContext& ctx, bool* ioDirty) {
    double* value = nullptr;
    if (!ctx.BindDouble(control.binding.path, &value) || !value) return false;

    double newValue = *value;
    if (control.def.is_number()) newValue = control.def.as_number();
    else if (control.def.is_string()) {
        try {
            newValue = std::stod(control.def.as_string());
        } catch (...) {
            return false;
        }
    } else {
        return false;
    }

    if (control.binding.path == "fractal.params.explaino_seed" && ctx.view && ctx.params) {
        if (ExplainoSeedCombined(*ctx.view, *ctx.params) == newValue) return false;
        ExplainoSeedSetCombined(*ctx.view, *ctx.params, newValue);
        if (ioDirty) *ioDirty = true;
        return true;
    }

    if (*value == newValue) return false;
    *value = newValue;
    if (ioDirty) *ioDirty = true;
    return true;
}

bool ApplyEnumSchemaDefault(const UISchemaControl& control, BindingContext& ctx, bool* ioDirty) {
    if (!control.def.is_string()) return false;

    const std::string currentValue = ctx.GetEnumId(control.binding.path);
    const std::string& wantedValue = control.def.as_string();
    if (currentValue == wantedValue) return false;

    const bool applied = ctx.SetEnumId(control.binding.path, wantedValue);
    if (applied && ioDirty) *ioDirty = true;
    return applied;
}

bool IsKnownActionBindingPath(const std::string& path) {
    return path == "fractal.actions.render_once" ||
        path == "fractal.actions.reset_view" ||
        path == "fractal.actions.reset_all" ||
        path == "fractal.actions.load_state" ||
        path == "fractal.actions.capture_finding" ||
        path == "fractal.actions.capture_diagnostic" ||
        path == "fractal.actions.next_seed" ||
        path == "fractal.actions.prev_seed";
}

bool ValidateEnumBindingPath(const UISchemaControl& control, BindingContext& ctx, std::string* outError) {
    if (!ctx.GetEnumId(control.binding.path).empty()) {
        return true;
    }

    ViewState viewCopy{};
    KernelParams paramsCopy{};
    RenderSettings renderCopy{};
    LensSettings lensCopy{};
    BindingContext probe = ctx;
    if (ctx.view) {
        viewCopy = *ctx.view;
        probe.view = &viewCopy;
    }
    if (ctx.params) {
        paramsCopy = *ctx.params;
        probe.params = &paramsCopy;
    }
    if (ctx.render) {
        renderCopy = *ctx.render;
        probe.render = &renderCopy;
    }
    if (ctx.lens) {
        lensCopy = *ctx.lens;
        probe.lens = &lensCopy;
    }

    for (const auto& option : control.options) {
        if (probe.SetEnumId(control.binding.path, option.id)) {
            return true;
        }
    }

    if (outError) {
        *outError = "Unknown enum binding path: " + control.binding.path + " (control: " + control.id + ")";
    }
    return false;
}

bool ValidateParamBinding(const UISchemaControl& control, BindingContext& ctx, std::string* outError) {
    if (!ValidateIntComboOptions(control, outError)) {
        return false;
    }
    const std::string& path = control.binding.path;
    if (control.value_type == "bool") {
        bool* value = nullptr;
        if (!ctx.BindBool(path, &value) || !value) {
            if (outError) *outError = "Bind failed for bool path: " + path + " (control: " + control.id + ")";
            return false;
        }
        return true;
    }
    if (control.value_type == "int") {
        int* value = nullptr;
        if (!ctx.BindInt(path, &value) || !value) {
            if (outError) *outError = "Bind failed for int path: " + path + " (control: " + control.id + ")";
            return false;
        }
        return true;
    }
    if (control.value_type == "float") {
        float* value = nullptr;
        if (!ctx.BindFloat(path, &value) || !value) {
            if (outError) *outError = "Bind failed for float path: " + path + " (control: " + control.id + ")";
            return false;
        }
        return true;
    }
    if (control.value_type == "double") {
        double* value = nullptr;
        if (!ctx.BindDouble(path, &value) || !value) {
            if (outError) *outError = "Bind failed for double path: " + path + " (control: " + control.id + ")";
            return false;
        }
        return true;
    }
    if (control.value_type == "enum") {
        return ValidateEnumBindingPath(control, ctx, outError);
    }
    return true;
}

} // namespace

bool ApplySchemaDefaultForControl(const UISchemaControl& c, BindingContext& ctx, bool* ioDirty) {
    if (!c.has_binding || c.binding.kind != "param" || !c.has_default) return false;

    if (c.value_type == "bool") return ApplyBoolSchemaDefault(c, ctx, ioDirty);
    if (c.value_type == "int") return ApplyIntSchemaDefault(c, ctx, ioDirty);
    if (c.value_type == "float") return ApplyFloatSchemaDefault(c, ctx, ioDirty);
    if (c.value_type == "double") return ApplyDoubleSchemaDefault(c, ctx, ioDirty);
    if (c.value_type == "enum") return ApplyEnumSchemaDefault(c, ctx, ioDirty);
    return false;
}

void ApplySchemaDefaults(const UISchema& schema, BindingContext& ctx, bool* ioDirty) {
    for (const auto& panel : schema.panels) {
        for (const auto& ctrl : panel.controls) {
            ApplySchemaDefaultForControl(ctrl, ctx, ioDirty);
        }
    }
}

bool ValidateSchemaBindings(const UISchema& schema, BindingContext& ctx, std::string* outError) {
    for (const auto& panel : schema.panels) {
        for (const auto& c : panel.controls) {
            if (!ValidateVisibleIfPredicate(c, ctx, outError)) {
                return false;
            }

            if (!c.has_binding) continue;

            const auto& b = c.binding;
            if (b.path.empty() || b.kind.empty()) {
                if (outError) *outError = "Schema binding missing kind/path for control: " + c.id;
                return false;
            }

            if (b.kind == "action") {
                if (!IsKnownActionBindingPath(b.path)) {
                    if (outError) *outError = "Unknown action binding path: " + b.path + " (control: " + c.id + ")";
                    return false;
                }
                continue;
            }

            if (b.kind != "param") {
                if (outError) *outError = "Unknown binding kind: " + b.kind + " (control: " + c.id + ")";
                return false;
            }

            if (!ValidateParamBinding(c, ctx, outError)) {
                return false;
            }
        }
    }
    return true;
}

namespace {

void MarkDirtyIfChanged(bool changed, bool* ioDirty) {
    if (changed && ioDirty) *ioDirty = true;
}

void MarkCurrentItemInteraction(bool changed, bool* ioInteracted) {
    if ((changed || ImGui::IsItemActivated() || ImGui::IsItemActive() || ImGui::IsItemDeactivatedAfterEdit()) && ioInteracted) {
        *ioInteracted = true;
    }
}

void RenderControlHelp(const UISchemaControl& control) {
    if (!control.has_help) {
        return;
    }

    ImGui::TextDisabled("?");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(300.0f);
        ImGui::TextUnformatted(control.help.c_str());
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
    ImGui::SameLine();
}

bool RenderDiagnosticLabel(const UISchemaControl& control, const char* detail) {
    ImGui::TextDisabled("%s (%s)", control.label.c_str(), detail);
    return false;
}

bool RenderActionControl(
    const UISchemaControl& control,
    const UISchemaBinding& binding,
    bool* ioRenderOnce,
    bool* ioInteracted) {
    if (binding.kind != "action") {
        return RenderDiagnosticLabel(control, "bad action binding");
    }

    if (ImGui::Button(control.label.c_str())) {
        if (ioInteracted) *ioInteracted = true;
        if (binding.path == "fractal.actions.render_once" && ioRenderOnce) {
            *ioRenderOnce = true;
        }
    }
    return true;
}

bool RenderCheckboxControl(
    const UISchemaControl& control,
    BindingContext& ctx,
    const UISchemaBinding& binding,
    bool* ioDirty,
    bool* ioInteracted) {
    bool* value = nullptr;
    if (!ctx.BindBool(binding.path, &value) || !value) {
        return RenderDiagnosticLabel(control, "bind failed");
    }

    const bool changed = ImGui::Checkbox(control.label.c_str(), value);
    MarkDirtyIfChanged(changed, ioDirty);
    MarkCurrentItemInteraction(changed, ioInteracted);
    return changed;
}

bool RenderIntControl(
    const UISchemaControl& control,
    BindingContext& ctx,
    const UISchemaBinding& binding,
    bool* ioDirty,
    bool* ioInteracted) {
    int* value = nullptr;
    if (!ctx.BindInt(binding.path, &value) || !value) {
        return RenderDiagnosticLabel(control, "bind failed");
    }

    const NumericControlRange range = ResolveNumericControlRange(control);
    const int minValue = range.has_widget_min ? static_cast<int>(range.widget_min) : 0;
    const int maxValue = range.has_widget_max ? static_cast<int>(range.widget_max) : (control.type == "slider_int" ? 100 : 0);
    const NumericDragWidgetBounds dragBounds = ResolveNumericDragWidgetBounds(control);

    bool changed = false;
    if (control.type == "slider_int") {
        changed = ImGui::SliderInt(control.label.c_str(), value, minValue, maxValue);
    } else {
        const float speed = control.has_step ? static_cast<float>(control.step) : 1.0f;
        const int dragMin = dragBounds.has_bounds ? static_cast<int>(dragBounds.min) : 0;
        const int dragMax = dragBounds.has_bounds ? static_cast<int>(dragBounds.max) : 0;
        changed = ImGui::DragInt(control.label.c_str(), value, speed, dragMin, dragMax);
    }
    ImGui::SameLine();
    const std::string inputLabel = "##value_input_" + control.id;
    const bool typedChanged = ImGui::InputInt(inputLabel.c_str(), value, 0, 0);
    if (changed || typedChanged) {
        ClampNumericValue(value, range);
        changed = true;
    }
    MarkDirtyIfChanged(changed, ioDirty);
    MarkCurrentItemInteraction(changed, ioInteracted);
    return changed;
}

bool RenderCameraZoomControl(
    const UISchemaControl& control,
    BindingContext& ctx,
    const UISchemaBinding& binding,
    const NumericControlRange& range,
    bool* ioDirty,
    bool* ioInteracted) {
    double displayedValue = 0.0;
    double dragValue = 0.0;
    if (!TryGetFloatControlDisplayValue(binding, ctx, &displayedValue) ||
        !TryGetFloatControlDragValue(binding, ctx, &dragValue)) {
        return RenderDiagnosticLabel(control, "bind failed");
    }

    const double minLog2Zoom = range.has_hard_min ? LocalLog2((std::fmax)(1.0e-30, range.hard_min)) : LocalLog2(kMinZoom);
    const double maxLog2Zoom = range.has_hard_max ? LocalLog2((std::fmax)(1.0e-30, range.hard_max)) : kMaxLog2Zoom;
    const float speed = control.has_step ? static_cast<float>(control.step) : 0.01f;
    const char* inputFormat = FloatControlInputFormat(control, binding);

    bool changed = false;
    if (control.type == "slider_float") {
        changed = ImGui::SliderScalar(control.label.c_str(), ImGuiDataType_Double, &dragValue, &minLog2Zoom, &maxLog2Zoom, "2^(%.3f)");
    } else {
        changed = ImGui::DragScalar(control.label.c_str(), ImGuiDataType_Double, &dragValue, speed, &minLog2Zoom, &maxLog2Zoom, "2^(%.3f)");
    }
    ImGui::SameLine();
    const std::string inputLabel = "##value_input_" + control.id;
    const bool typedChanged = ImGui::InputDouble(inputLabel.c_str(), &displayedValue, 0.0, 0.0, inputFormat);

    if (changed && !ApplyFloatControlDragEdit(binding, ctx, range, dragValue)) {
        return RenderDiagnosticLabel(control, "camera edit failed");
    }
    if (typedChanged && !ApplyFloatControlEdit(binding, ctx, range, displayedValue)) {
        return RenderDiagnosticLabel(control, "camera edit failed");
    }

    changed = changed || typedChanged;
    MarkDirtyIfChanged(changed, ioDirty);
    MarkCurrentItemInteraction(changed, ioInteracted);
    return changed;
}

bool RenderFloatControl(
    const UISchemaControl& control,
    BindingContext& ctx,
    const UISchemaBinding& binding,
    bool* ioDirty,
    bool* ioInteracted) {
    if (binding.path == "fractal.view.zoom") {
        return RenderCameraZoomControl(control, ctx, binding, ResolveNumericControlRange(control), ioDirty, ioInteracted);
    }

    double displayedValue = 0.0;
    if (!TryGetFloatControlDisplayValue(binding, ctx, &displayedValue)) {
        return RenderDiagnosticLabel(control, "bind failed");
    }
    float value = static_cast<float>(displayedValue);

    const NumericControlRange range = ResolveNumericControlRange(control);
    const float minValue = range.has_widget_min ? static_cast<float>(range.widget_min) : 0.0f;
    const float maxValue = range.has_widget_max ? static_cast<float>(range.widget_max) : (control.type == "slider_float" ? 1.0f : 0.0f);
    const NumericDragWidgetBounds dragBounds = ResolveFloatControlDragWidgetBounds(control, binding);
    const float speed = control.has_step ? static_cast<float>(control.step) : 0.01f;
    const ImGuiSliderFlags flags = control.logarithmic ? ImGuiSliderFlags_Logarithmic : 0;
    const char* displayFormat = FloatControlDisplayFormat(control, binding);
    const char* inputFormat = FloatControlInputFormat(control, binding);

    bool changed = false;
    if (control.type == "slider_float") {
        changed = ImGui::SliderFloat(control.label.c_str(), &value, minValue, maxValue, displayFormat, flags);
    } else {
        const float dragMin = dragBounds.has_bounds ? static_cast<float>(dragBounds.min) : 0.0f;
        const float dragMax = dragBounds.has_bounds ? static_cast<float>(dragBounds.max) : 0.0f;
        changed = ImGui::DragFloat(control.label.c_str(), &value, speed, dragMin, dragMax, displayFormat, flags);
    }
    ImGui::SameLine();
    const std::string inputLabel = "##value_input_" + control.id;
    const bool typedChanged = ImGui::InputFloat(inputLabel.c_str(), &value, 0.0f, 0.0f, inputFormat);
    if (changed || typedChanged) {
        if (!ApplyFloatControlEdit(binding, ctx, range, static_cast<double>(value))) {
            return RenderDiagnosticLabel(control, IsCameraFloatBindingPath(binding.path) ? "camera edit failed" : "edit failed");
        }
        changed = true;
    }
    MarkDirtyIfChanged(changed, ioDirty);
    MarkCurrentItemInteraction(changed, ioInteracted);
    return changed;
}

bool RenderExplainoSeedDoubleControl(
    const UISchemaControl& control,
    BindingContext& ctx,
    const NumericControlRange& range,
    double speed,
    const char* valueFormat,
    bool* ioDirty,
    bool* ioInteracted) {
    double displayed = ExplainoSeedCombined(*ctx.view, *ctx.params);
    const NumericDragWidgetBounds dragBounds = ResolveNumericDragWidgetBounds(control);
    bool changed = false;
    if (control.type == "slider_double") {
        const double minValue = range.has_widget_min ? range.widget_min : 0.0;
        const double maxValue = range.has_widget_max ? range.widget_max : 1.0;
        changed = ImGui::SliderScalar(control.label.c_str(), ImGuiDataType_Double, &displayed, &minValue, &maxValue, valueFormat);
    } else {
        const double* dragMin = dragBounds.has_bounds ? &dragBounds.min : nullptr;
        const double* dragMax = dragBounds.has_bounds ? &dragBounds.max : nullptr;
        changed = ImGui::DragScalar(control.label.c_str(), ImGuiDataType_Double, &displayed, static_cast<float>(speed), dragMin, dragMax, valueFormat);
    }
    ImGui::SameLine();
    const std::string inputLabel = "##value_input_" + control.id;
    const bool typedChanged = ImGui::InputDouble(inputLabel.c_str(), &displayed, 0.0, 0.0, valueFormat);
    if (changed || typedChanged) {
        ClampNumericValue(&displayed, range);
        changed = true;
    }
    if (changed) {
        ExplainoSeedSetCombined(*ctx.view, *ctx.params, displayed);
    }
    MarkDirtyIfChanged(changed, ioDirty);
    MarkCurrentItemInteraction(changed, ioInteracted);
    return changed;
}

bool RenderDoubleControl(
    const UISchemaControl& control,
    BindingContext& ctx,
    const UISchemaBinding& binding,
    bool* ioDirty,
    bool* ioInteracted) {
    double* value = nullptr;
    if (!ctx.BindDouble(binding.path, &value) || !value) {
        return RenderDiagnosticLabel(control, "bind failed");
    }

    const NumericControlRange range = ResolveNumericControlRange(control);
    const double minValue = range.has_widget_min ? range.widget_min : 0.0;
    const double maxValue = range.has_widget_max ? range.widget_max : (control.type == "slider_double" ? 1.0 : 0.0);
    const NumericDragWidgetBounds dragBounds = ResolveNumericDragWidgetBounds(control);
    const double speed = control.has_step ? control.step : 0.001;
    const char* valueFormat = "%.6f";

    if (binding.path == "fractal.params.explaino_seed" && ctx.view && ctx.params) {
        return RenderExplainoSeedDoubleControl(control, ctx, range, speed, valueFormat, ioDirty, ioInteracted);
    }

    bool changed = false;
    if (control.type == "slider_double") {
        changed = ImGui::SliderScalar(control.label.c_str(), ImGuiDataType_Double, value, &minValue, &maxValue, valueFormat);
    } else {
        const double* dragMin = dragBounds.has_bounds ? &dragBounds.min : nullptr;
        const double* dragMax = dragBounds.has_bounds ? &dragBounds.max : nullptr;
        changed = ImGui::DragScalar(control.label.c_str(), ImGuiDataType_Double, value, static_cast<float>(speed), dragMin, dragMax, valueFormat);
    }
    ImGui::SameLine();
    const std::string inputLabel = "##value_input_" + control.id;
    const bool typedChanged = ImGui::InputDouble(inputLabel.c_str(), value, 0.0, 0.0, valueFormat);
    if (changed || typedChanged) {
        ClampNumericValue(value, range);
        changed = true;
    }
    MarkDirtyIfChanged(changed, ioDirty);
    MarkCurrentItemInteraction(changed, ioInteracted);
    return changed;
}

bool RenderIntComboControl(
    const UISchemaControl& control,
    BindingContext& ctx,
    const UISchemaBinding& binding,
    bool* ioDirty,
    bool* ioInteracted) {
    int* value = nullptr;
    if (!ctx.BindInt(binding.path, &value) || !value) {
        return RenderDiagnosticLabel(control, "bind failed");
    }

    int currentIndex = 0;
    for (int index = 0; index < static_cast<int>(control.options.size()); ++index) {
        try {
            const int optionValue = std::stoi(control.options[index].id);
            if (optionValue == *value) {
                currentIndex = index;
                break;
            }
        } catch (...) {
        }
    }

    std::vector<const char*> labels;
    labels.reserve(control.options.size());
    for (const auto& option : control.options) {
        try {
            (void)std::stoi(option.id);
        } catch (...) {
            return RenderDiagnosticLabel(control, "invalid option id");
        }
        labels.push_back(option.label.c_str());
    }

    bool changed = false;
    if (!labels.empty() && ImGui::Combo(control.label.c_str(), &currentIndex, labels.data(), static_cast<int>(labels.size()))) {
        if (currentIndex >= 0 && currentIndex < static_cast<int>(control.options.size())) {
            try {
                const int newValue = std::stoi(control.options[currentIndex].id);
                if (newValue != *value) {
                    *value = newValue;
                    changed = true;
                }
            } catch (...) {
                return RenderDiagnosticLabel(control, "invalid option id");
            }
        }
    }
    MarkDirtyIfChanged(changed, ioDirty);
    MarkCurrentItemInteraction(changed, ioInteracted);
    return changed;
}

bool RenderGroupedEnumComboControl(
    const UISchemaControl& control,
    BindingContext& ctx,
    const UISchemaBinding& binding,
    const std::string& currentId,
    bool* ioDirty,
    bool* ioInteracted) {
    const std::vector<std::string> groups = CollectOptionGroups(control);
    if (groups.size() <= 1) {
        return false;
    }

    std::string currentGroup = OptionGroupForId(control, currentId);
    if (currentGroup.empty()) {
        currentGroup = groups.front();
    }

    int groupIndex = 0;
    for (int index = 0; index < static_cast<int>(groups.size()); ++index) {
        if (groups[index] == currentGroup) {
            groupIndex = index;
            break;
        }
    }

    std::vector<const char*> groupLabels;
    groupLabels.reserve(groups.size());
    for (const auto& group : groups) {
        groupLabels.push_back(group.c_str());
    }

    std::string selectedId = currentId;
    bool groupChanged = false;
    bool groupInteracted = false;
    if (!groupLabels.empty() && ImGui::Combo("Category", &groupIndex, groupLabels.data(), static_cast<int>(groupLabels.size()))) {
        groupInteracted = true;
        const std::vector<const UISchemaOption*> groupedOptions = OptionsForGroup(control, groups[groupIndex]);
        if (!groupedOptions.empty()) {
            groupChanged = ctx.SetEnumId(binding.path, groupedOptions.front()->id);
            MarkDirtyIfChanged(groupChanged, ioDirty);
            selectedId = groupedOptions.front()->id;
        }
    }
    groupInteracted = groupInteracted || ImGui::IsItemActivated() || ImGui::IsItemActive() || ImGui::IsItemDeactivatedAfterEdit();

    const std::vector<const UISchemaOption*> groupedOptions = OptionsForGroup(control, groups[groupIndex]);
    int currentIndex = 0;
    for (int index = 0; index < static_cast<int>(groupedOptions.size()); ++index) {
        if (groupedOptions[index]->id == selectedId) {
            currentIndex = index;
            break;
        }
    }

    std::vector<const char*> labels;
    labels.reserve(groupedOptions.size());
    for (const UISchemaOption* option : groupedOptions) {
        labels.push_back(option->label.c_str());
    }

    bool valueChanged = false;
    if (!labels.empty() && ImGui::Combo(control.label.c_str(), &currentIndex, labels.data(), static_cast<int>(labels.size()))) {
        if (currentIndex >= 0 && currentIndex < static_cast<int>(groupedOptions.size())) {
            valueChanged = ctx.SetEnumId(binding.path, groupedOptions[currentIndex]->id);
            MarkDirtyIfChanged(valueChanged, ioDirty);
        }
    }
    const bool valueInteracted = valueChanged || ImGui::IsItemActivated() || ImGui::IsItemActive() || ImGui::IsItemDeactivatedAfterEdit();
    if ((groupChanged || valueChanged || groupInteracted || valueInteracted) && ioInteracted) {
        *ioInteracted = true;
    }
    return groupChanged || valueChanged;
}

bool RenderEnumComboControl(
    const UISchemaControl& control,
    BindingContext& ctx,
    const UISchemaBinding& binding,
    bool* ioDirty,
    bool* ioInteracted) {
    const std::string currentId = ctx.GetEnumId(binding.path);
    const std::vector<const UISchemaOption*> visibleOptions = ResolveVisibleEnumOptions(control, ctx);
    if (HasGroupedOptions(control)) {
        RenderGroupedEnumComboControl(control, ctx, binding, currentId, ioDirty, ioInteracted);
        return true;
    }

    if (visibleOptions.empty()) {
        return RenderDiagnosticLabel(control, "no visible options");
    }

    int currentIndex = 0;
    for (int index = 0; index < static_cast<int>(visibleOptions.size()); ++index) {
        if (visibleOptions[index]->id == currentId) {
            currentIndex = index;
            break;
        }
    }

    std::vector<const char*> labels;
    labels.reserve(visibleOptions.size());
    for (const UISchemaOption* option : visibleOptions) {
        labels.push_back(option->label.c_str());
    }

    bool changed = false;
    if (!labels.empty() && ImGui::Combo(control.label.c_str(), &currentIndex, labels.data(), static_cast<int>(labels.size()))) {
        if (currentIndex >= 0 && currentIndex < static_cast<int>(visibleOptions.size())) {
            changed = ctx.SetEnumId(binding.path, visibleOptions[currentIndex]->id);
        }
    }
    MarkDirtyIfChanged(changed, ioDirty);
    MarkCurrentItemInteraction(changed, ioInteracted);
    return changed;
}

bool RenderComboControl(
    const UISchemaControl& control,
    BindingContext& ctx,
    const UISchemaBinding& binding,
    bool* ioDirty,
    bool* ioInteracted) {
    if (control.value_type == "int") {
        return RenderIntComboControl(control, ctx, binding, ioDirty, ioInteracted);
    }
    return RenderEnumComboControl(control, ctx, binding, ioDirty, ioInteracted);
}

} // namespace

bool RenderControlFromSchema(const UISchemaControl& c, BindingContext& ctx, bool* ioDirty, bool* ioRenderOnce, bool* ioInteracted) {
    if (c.has_visible_if) {
        if (!ctx.EvalVisibleIf(c.visible_if)) return false;
    }

    ImGui::PushID(c.id.c_str());

    RenderControlHelp(c);

    if (!c.has_binding) {
        RenderDiagnosticLabel(c, "UNBOUND");
        ImGui::PopID();
        return false;
    }

    const auto& b = c.binding;

    bool result = false;
    if (c.type == "button") {
        result = RenderActionControl(c, b, ioRenderOnce, ioInteracted);
    } else if (b.kind != "param") {
        result = RenderDiagnosticLabel(c, "bad param binding");
    } else if (c.type == "checkbox") {
        result = RenderCheckboxControl(c, ctx, b, ioDirty, ioInteracted);
    } else if (c.type == "slider_int" || c.type == "drag_int") {
        result = RenderIntControl(c, ctx, b, ioDirty, ioInteracted);
    } else if (c.type == "slider_float" || c.type == "drag_float") {
        result = RenderFloatControl(c, ctx, b, ioDirty, ioInteracted);
    } else if (c.type == "slider_double" || c.type == "drag_double") {
        result = RenderDoubleControl(c, ctx, b, ioDirty, ioInteracted);
    } else if (c.type == "combo") {
        result = RenderComboControl(c, ctx, b, ioDirty, ioInteracted);
    } else {
        ImGui::TextDisabled("%s (unsupported control type: %s)", c.label.c_str(), c.type.c_str());
        result = false;
    }

    ImGui::PopID();
    return result;
}
