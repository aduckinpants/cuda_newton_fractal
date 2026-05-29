#include "../src/schema_binding.h"

#include "../src/color_pipeline_core.h"
#include "../src/color_pipeline_window.h"
#include "../src/enum_id_utils.h"
#include "../src/imgui_stack_editor.h"

#include "../src/explaino_seed.h"
#include "../third_party/imgui/imgui.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace {

constexpr std::size_t kExpectedFractalCount = 47;

struct ImGuiTestContext {
    ImGuiContext* context = nullptr;

    ImGuiTestContext() {
        context = ImGui::CreateContext();
        ImGui::SetCurrentContext(context);
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(640.0f, 480.0f);
        io.DeltaTime = 1.0f / 60.0f;
        unsigned char* pixels = nullptr;
        int width = 0;
        int height = 0;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
    }

    ~ImGuiTestContext() {
        if (context) {
            ImGui::SetCurrentContext(context);
            ImGui::DestroyContext(context);
        }
    }
};

void BeginFrame() {
    ImGui::NewFrame();
    ImGui::Begin("SchemaBindingTest");
}

void EndFrame() {
    ImGui::End();
    ImGui::Render();
}

BindingContext MakeBindingContext(ViewState* view, KernelParams* params, RenderSettings* render, LensSettings* lens) {
    BindingContext ctx;
    ctx.view = view;
    ctx.params = params;
    ctx.render = render;
    ctx.lens = lens;
    return ctx;
}

UISchemaControl MakeBoundControl(const char* id, const char* type, const char* label, const char* valueType, const char* kind, const char* path) {
    UISchemaControl control;
    control.id = id;
    control.type = type;
    control.label = label;
    control.value_type = valueType;
    control.has_binding = true;
    control.binding.kind = kind;
    control.binding.path = path;
    return control;
}

bool NearlyEqual(double left, double right, double eps = 1.0e-6) {
    const double delta = left - right;
    return delta < eps && delta > -eps;
}

void NoteTestUiAutomationControlId(void* userData, const char* controlId) {
    if (!userData) {
        return;
    }
    auto* output = static_cast<std::string*>(userData);
    *output = controlId ? controlId : "";
}

constexpr double kTestMaxLog2Zoom = 1020.0;

double TestLog2(double value) {
    return std::log(value) / std::log(2.0);
}

double TestExp2(double value) {
    return std::exp(value * std::log(2.0));
}

double TestSafeZoomFromLog2(double log2Zoom) {
    double clamped = log2Zoom;
    const double minLog2 = TestLog2(1.0e-30);
    if (clamped < minLog2) {
        clamped = minLog2;
    }
    if (clamped > kTestMaxLog2Zoom) {
        clamped = kTestMaxLog2Zoom;
    }
    return TestExp2(clamped);
}

void SyncTestViewUiFromHp(ViewState& view) {
    double zoom = TestSafeZoomFromLog2(view.log2_zoom);
    if (zoom < 1.0e-30) {
        zoom = 1.0e-30;
    }
    if (zoom > 1.0e30) {
        zoom = 1.0e30;
    }
    view.zoom = static_cast<float>(zoom);
    view.center.x = static_cast<float>(view.center_hp_x);
    view.center.y = static_cast<float>(view.center_hp_y);
}

std::filesystem::path FindRepoRoot() {
    std::filesystem::path cur = std::filesystem::current_path();
    for (int index = 0; index < 8; ++index) {
        if (std::filesystem::exists(cur / "ui" / "fractal_binding_surface_v1.ui_schema.json") &&
            std::filesystem::exists(cur / "ui_app" / "src" / "schema_binding.cpp")) {
            return cur;
        }
        if (!cur.has_parent_path() || cur.parent_path() == cur) {
            break;
        }
        cur = cur.parent_path();
    }
    return {};
}

bool ReadTextFile(const std::filesystem::path& path, std::string* outText) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) {
        return false;
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    *outText = ss.str();
    return true;
}

bool LoadCurrentSchemaJson(json_min::Value* outRoot) {
    const std::filesystem::path root = FindRepoRoot();
    if (root.empty()) {
        std::cerr << "Could not locate repo root for schema-binding visible-control matrix\n";
        return false;
    }
    std::string text;
    const std::filesystem::path schemaPath = root / "ui" / "fractal_binding_surface_v1.ui_schema.json";
    if (!ReadTextFile(schemaPath, &text)) {
        std::cerr << "Could not read schema for schema-binding visible-control matrix: " << schemaPath.string() << "\n";
        return false;
    }
    json_min::ParseResult parsed = json_min::Parse(text);
    if (!parsed.error.empty()) {
        std::cerr << "Could not parse schema for schema-binding visible-control matrix: " << parsed.error << "\n";
        return false;
    }
    *outRoot = parsed.value;
    return true;
}

bool GetJsonStringField(const json_min::Value& object, const char* key, std::string* outValue) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_string()) {
        return false;
    }
    if (outValue) {
        *outValue = value->as_string();
    }
    return true;
}

bool GetJsonNumberField(const json_min::Value& object, const char* key, double* outValue) {
    const json_min::Value* value = object.get(key);
    if (!value || !value->is_number()) {
        return false;
    }
    if (outValue) {
        *outValue = value->as_number();
    }
    return true;
}

const json_min::Value* FindSchemaControlById(const json_min::Value& root, const char* controlId) {
    const json_min::Value* panels = root.get("panels");
    if (!panels || !panels->is_array()) {
        return nullptr;
    }
    for (const json_min::Value& panel : panels->as_array()) {
        if (!panel.is_object()) {
            continue;
        }
        const json_min::Value* controls = panel.get("controls");
        if (!controls || !controls->is_array()) {
            continue;
        }
        for (const json_min::Value& control : controls->as_array()) {
            if (!control.is_object()) {
                continue;
            }
            std::string id;
            if (GetJsonStringField(control, "id", &id) && id == controlId) {
                return &control;
            }
        }
    }
    return nullptr;
}

bool ReadControlBindingAndType(
    const json_min::Value& control,
    std::string* outBindingPath,
    std::string* outValueType) {
    if (!GetJsonStringField(control, "value_type", outValueType)) {
        return false;
    }
    const json_min::Value* binding = control.get("binding");
    if (!binding || !binding->is_object()) {
        return false;
    }
    std::string bindingKind;
    if (!GetJsonStringField(*binding, "kind", &bindingKind) || bindingKind != "param") {
        return false;
    }
    return GetJsonStringField(*binding, "path", outBindingPath);
}

bool IsControlVisibleForFractal(const json_min::Value& control, FractalType fractalType) {
    const json_min::Value* visibleIf = control.get("visible_if");
    if (!visibleIf || !visibleIf->is_object()) {
        return true;
    }
    UISchemaPredicate predicate{};
    if (!GetJsonStringField(*visibleIf, "op", &predicate.op) ||
        !GetJsonStringField(*visibleIf, "path", &predicate.path) ||
        !GetJsonStringField(*visibleIf, "value", &predicate.value)) {
        return false;
    }
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    LensSettings lens{};
    view.fractal_type = fractalType;
    BindingContext ctx = MakeBindingContext(&view, &params, &render, &lens);
    return ctx.EvalVisibleIf(predicate);
}

bool ControlBindingResolves(const std::string& valueType, const std::string& bindingPath, BindingContext& ctx) {
    if (valueType == "float") {
        float* value = nullptr;
        return ctx.BindFloat(bindingPath, &value) && value;
    }
    if (valueType == "double") {
        double* value = nullptr;
        return ctx.BindDouble(bindingPath, &value) && value;
    }
    if (valueType == "int") {
        int* value = nullptr;
        return ctx.BindInt(bindingPath, &value) && value;
    }
    if (valueType == "bool") {
        bool* value = nullptr;
        return ctx.BindBool(bindingPath, &value) && value;
    }
    if (valueType == "enum") {
        return !ctx.GetEnumId(bindingPath).empty();
    }
    return false;
}

bool ValidateNovaAlphaSchemaRange() {
    json_min::Value schemaRoot;
    if (!LoadCurrentSchemaJson(&schemaRoot)) {
        return false;
    }
    const json_min::Value* control = FindSchemaControlById(schemaRoot, "nova_alpha");
    if (!control || !control->is_object()) {
        std::cerr << "Nova Alpha schema range missing control\n";
        return false;
    }
    std::string bindingPath;
    std::string valueType;
    if (!ReadControlBindingAndType(*control, &bindingPath, &valueType) ||
        bindingPath != "fractal.params.nova_alpha" || valueType != "float") {
        std::cerr << "Nova Alpha schema range found wrong binding/value type\n";
        return false;
    }
    double minValue = 0.0;
    double maxValue = 0.0;
    if (!GetJsonNumberField(*control, "min", &minValue) || !GetJsonNumberField(*control, "max", &maxValue)) {
        std::cerr << "Nova Alpha schema range must expose hard min/max\n";
        return false;
    }
    if (!NearlyEqual(minValue, 0.01) || !NearlyEqual(maxValue, 2.0)) {
        std::cerr << "Nova Alpha schema range must match the kernel-valid (0,2] domain\n";
        return false;
    }
    if (!IsControlVisibleForFractal(*control, FractalType::nova) ||
        !IsControlVisibleForFractal(*control, FractalType::explaino_nova)) {
        std::cerr << "Nova Alpha schema range must stay visible on Nova owner lanes\n";
        return false;
    }
    return true;
}

bool ExplainoCommonControlExpectedVisible(const char* controlId, FractalType fractalType) {
    if (std::string_view(controlId) == "epsilon") {
        return fractalType != FractalType::explaino_julia &&
            fractalType != FractalType::explaino_lambda &&
            fractalType != FractalType::explaino_rational_escape &&
            fractalType != FractalType::explaino_collatz_direct;
    }
    if (std::string_view(controlId) == "explaino_warp_strength") {
        return true;
    }
    if (std::string_view(controlId) == "explaino_root_spread") {
        return fractalType != FractalType::explaino_transcendental &&
            fractalType != FractalType::explaino_lambda &&
            fractalType != FractalType::explaino_collatz_direct;
    }
    if (std::string_view(controlId) == "explaino_damping") {
        return fractalType != FractalType::explaino_julia &&
            fractalType != FractalType::explaino_lambda &&
            fractalType != FractalType::explaino_rational_escape &&
            fractalType != FractalType::explaino_collatz_direct;
    }
    return true;
}

bool ValidateHiddenControlMatrixCase(
    const json_min::Value& schemaRoot,
    const char* controlId,
    FractalType fractalType) {
    const json_min::Value* control = FindSchemaControlById(schemaRoot, controlId);
    if (!control) {
        std::cerr << "Hidden-control matrix missing control " << controlId << "\n";
        return false;
    }
    if (IsControlVisibleForFractal(*control, fractalType)) {
        const char* fractalId = FractalTypeId(fractalType);
        std::cerr << "Hidden-control matrix still exposes dead control " << controlId
                  << " on fractal " << (fractalId ? fractalId : "<unknown>") << "\n";
        return false;
    }
    return true;
}

bool ValidateVisibleControlMatrixCase(
    const json_min::Value& schemaRoot,
    const char* controlId,
    FractalType fractalType,
    const char* bindingPath,
    const char* expectedValueType) {
    const json_min::Value* control = FindSchemaControlById(schemaRoot, controlId);
    if (!control) {
        std::cerr << "Visible-control matrix missing control " << controlId << "\n";
        return false;
    }
    std::string actualBindingPath;
    std::string actualValueType;
    if (!ReadControlBindingAndType(*control, &actualBindingPath, &actualValueType) || actualBindingPath != bindingPath) {
        std::cerr << "Visible-control matrix found wrong binding for " << controlId << "\n";
        return false;
    }
    if (expectedValueType && actualValueType != expectedValueType) {
        std::cerr << "Visible-control matrix found wrong value type for " << controlId << "\n";
        return false;
    }
    if (!IsControlVisibleForFractal(*control, fractalType)) {
        const char* fractalId = FractalTypeId(fractalType);
        std::cerr << "Visible-control matrix hid " << controlId
                  << " on owner fractal " << (fractalId ? fractalId : "<unknown>") << "\n";
        return false;
    }

    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    LensSettings lens{};
    view.fractal_type = fractalType;
    BindingContext ctx = MakeBindingContext(&view, &params, &render, &lens);
    if (!ControlBindingResolves(actualValueType, actualBindingPath, ctx)) {
        std::cerr << "Visible-control matrix could not resolve binding for " << controlId << "\n";
        return false;
    }
    return true;
}

bool StartsWith(const std::string& value, const char* prefix) {
    return value.rfind(prefix, 0) == 0;
}

bool IsGlobalFixedFormulaBindingPath(const std::string& bindingPath) {
    return bindingPath == "fractal.params.max_iter" ||
        bindingPath == "fractal.params.coloring_mode" ||
        bindingPath == "fractal.params.color_grading" ||
        bindingPath == "fractal.params.exposure" ||
        StartsWith(bindingPath, "fractal.params.color_") ||
        StartsWith(bindingPath, "fractal.view.") ||
        StartsWith(bindingPath, "fractal.render.") ||
        StartsWith(bindingPath, "fractal.lens.");
}

bool ReadVisibleIfPredicate(const json_min::Value& object, UISchemaPredicate* outPredicate) {
    const json_min::Value* visibleIf = object.get("visible_if");
    if (!visibleIf || !visibleIf->is_object()) {
        return false;
    }
    UISchemaPredicate predicate{};
    if (!GetJsonStringField(*visibleIf, "op", &predicate.op) ||
        !GetJsonStringField(*visibleIf, "path", &predicate.path) ||
        !GetJsonStringField(*visibleIf, "value", &predicate.value)) {
        return false;
    }
    if (outPredicate) {
        *outPredicate = predicate;
    }
    return true;
}

bool EvalRawVisibleIfForFractal(const json_min::Value& object, FractalType fractalType) {
    const json_min::Value* visibleIf = object.get("visible_if");
    if (!visibleIf) {
        return true;
    }
    UISchemaPredicate predicate{};
    if (!ReadVisibleIfPredicate(object, &predicate)) {
        return false;
    }
    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    LensSettings lens{};
    view.fractal_type = fractalType;
    BindingContext ctx = MakeBindingContext(&view, &params, &render, &lens);
    return ctx.EvalVisibleIf(predicate);
}

bool RawAnimationOptionVisibleForFractal(const json_min::Value& option, FractalType fractalType) {
    std::string optionId;
    if (!GetJsonStringField(option, "id", &optionId)) {
        return false;
    }
    const ExplainoAxisDescriptor* axis = FindExplainoAxisDescriptor(optionId);
    if (axis && !option.get("visible_if")) {
        return fractalType == ExplainoCanonicalFractalType() || fractalType == axis->carrier_fractal_type;
    }
    return EvalRawVisibleIfForFractal(option, fractalType);
}

bool HasVisibleEnumOptionForFractal(const json_min::Value& control, const char* optionId, FractalType fractalType) {
    const json_min::Value* options = control.get("options");
    if (!options || !options->is_array()) {
        return false;
    }
    for (const json_min::Value& option : options->as_array()) {
        if (!option.is_object()) {
            continue;
        }
        std::string id;
        if (GetJsonStringField(option, "id", &id) && id == optionId) {
            return RawAnimationOptionVisibleForFractal(option, fractalType);
        }
    }
    return false;
}


const json_min::Value* FindFractalTypeControl(const json_min::Value& root) {
    return FindSchemaControlById(root, "fractal_type");
}

bool SchemaFractalOptionsMatchEnumIds(const json_min::Value& schemaRoot) {
    const json_min::Value* fractalTypeControl = FindFractalTypeControl(schemaRoot);
    if (!fractalTypeControl) {
        std::cerr << "All-fractal inventory could not find fractal_type selector\n";
        return false;
    }
    const json_min::Value* options = fractalTypeControl->get("options");
    if (!options || !options->is_array()) {
        std::cerr << "All-fractal inventory found malformed fractal_type options\n";
        return false;
    }
    std::vector<std::string> schemaIds;
    for (const json_min::Value& option : options->as_array()) {
        if (!option.is_object()) {
            continue;
        }
        std::string id;
        if (!GetJsonStringField(option, "id", &id)) {
            std::cerr << "All-fractal inventory found a fractal_type option without an id\n";
            return false;
        }
        schemaIds.push_back(id);
    }
    if (schemaIds.size() != kExpectedFractalCount || std::size(enum_id_utils::kFractalTypeIds) != kExpectedFractalCount) {
        std::cerr << "All-fractal inventory expected exactly 47 schema and enum fractal ids\n";
        return false;
    }
    for (const auto& enumId : enum_id_utils::kFractalTypeIds) {
        bool found = false;
        for (const std::string& schemaId : schemaIds) {
            if (schemaId == enumId.id) {
                found = true;
                break;
            }
        }
        if (!found) {
            std::cerr << "All-fractal inventory schema selector is missing enum id " << enumId.id << "\n";
            return false;
        }
    }
    return true;
}

bool ValidateAndExportAllFractalControlDescriptor(const json_min::Value& schemaRoot);

bool IsFamilySurfaceBindingPath(const std::string& bindingPath) {
    if (bindingPath == "fractal.view.fractal_type") {
        return false;
    }
    return !IsGlobalFixedFormulaBindingPath(bindingPath);
}

bool ValidateGeneratedAllFractalControlInventory() {
    json_min::Value schemaRoot;
    if (!LoadCurrentSchemaJson(&schemaRoot)) {
        return false;
    }
    if (!SchemaFractalOptionsMatchEnumIds(schemaRoot)) {
        return false;
    }
    const json_min::Value* panels = schemaRoot.get("panels");
    if (!panels || !panels->is_array()) {
        std::cerr << "All-fractal inventory could not read schema panels\n";
        return false;
    }

    int laneCount = 0;
    int visibleFamilyControlCells = 0;
    for (const auto& enumId : enum_id_utils::kFractalTypeIds) {
        ++laneCount;
        for (const json_min::Value& panel : panels->as_array()) {
            if (!panel.is_object()) {
                continue;
            }
            const json_min::Value* controls = panel.get("controls");
            if (!controls || !controls->is_array()) {
                continue;
            }
            for (const json_min::Value& control : controls->as_array()) {
                if (!control.is_object()) {
                    continue;
                }
                std::string bindingPath;
                std::string valueType;
                if (!ReadControlBindingAndType(control, &bindingPath, &valueType)) {
                    continue;
                }
                if (!IsFamilySurfaceBindingPath(bindingPath) || !IsControlVisibleForFractal(control, enumId.value)) {
                    continue;
                }
                ViewState view{};
                KernelParams params{};
                RenderSettings render{};
                LensSettings lens{};
                view.fractal_type = enumId.value;
                BindingContext ctx = MakeBindingContext(&view, &params, &render, &lens);
                if (!ControlBindingResolves(valueType, bindingPath, ctx)) {
                    std::string controlId;
                    GetJsonStringField(control, "id", &controlId);
                    std::cerr << "All-fractal inventory found visible unbound family control "
                              << controlId << " on " << enumId.id << " at " << bindingPath << "\n";
                    return false;
                }
                ++visibleFamilyControlCells;
            }
        }
    }
    if (laneCount != kExpectedFractalCount) {
        std::cerr << "All-fractal inventory did not visit all 47 fractal lanes\n";
        return false;
    }
    if (visibleFamilyControlCells < 200) {
        std::cerr << "All-fractal inventory visited too few visible family-control cells: "
                  << visibleFamilyControlCells << "\n";
        return false;
    }
    return ValidateAndExportAllFractalControlDescriptor(schemaRoot);
}


void WriteJsonEscapedString(std::ostream& out, const std::string& value) {
    out << '"';
    for (char ch : value) {
        if (ch == '"' || ch == '\\') {
            out << '\\' << ch;
        } else if (ch == '\n') {
            out << "\\n";
        } else if (ch == '\r') {
            out << "\\r";
        } else if (ch == '\t') {
            out << "\\t";
        } else {
            out << ch;
        }
    }
    out << '"';
}

std::string JsonScalarAsDescriptorValue(const json_min::Value* value) {
    if (!value) {
        return "null";
    }
    if (value->is_string()) {
        return value->as_string();
    }
    if (value->is_number()) {
        std::ostringstream ss;
        ss << value->as_number();
        return ss.str();
    }
    if (value->is_bool()) {
        return value->as_bool() ? "true" : "false";
    }
    return "null";
}

std::string CandidateValueForControl(const json_min::Value& control, const std::string& valueType) {
    const json_min::Value* defaultValue = control.get("default");
    if (valueType == "enum") {
        const std::string defaultId = JsonScalarAsDescriptorValue(defaultValue);
        const json_min::Value* options = control.get("options");
        if (options && options->is_array()) {
            for (const json_min::Value& option : options->as_array()) {
                std::string optionId;
                if (option.is_object() && GetJsonStringField(option, "id", &optionId) && optionId != defaultId) {
                    return optionId;
                }
            }
        }
        return defaultId;
    }
    double defaultNumber = 0.0;
    GetJsonNumberField(control, "default", &defaultNumber);
    double step = 1.0;
    GetJsonNumberField(control, "step", &step);
    double candidate = defaultNumber + step;
    double maxValue = 0.0;
    if (GetJsonNumberField(control, "max", &maxValue) || GetJsonNumberField(control, "ui_max", &maxValue)) {
        if (candidate > maxValue) {
            candidate = defaultNumber - step;
        }
    }
    std::ostringstream ss;
    ss << candidate;
    return ss.str();
}

const char* SensitivityClassForControl(const std::string& controlId, const std::string& valueType) {
    if (controlId == "mcmullen_preset") {
        return "preset_switch";
    }
    if (valueType == "enum") {
        return "enum_or_mode";
    }
    return "render_sensitive";
}

bool ValidateAndExportAllFractalControlDescriptor(const json_min::Value& schemaRoot) {
    const std::filesystem::path root = FindRepoRoot();
    if (root.empty()) {
        std::cerr << "Descriptor export could not locate repo root\n";
        return false;
    }
    const json_min::Value* panels = schemaRoot.get("panels");
    if (!panels || !panels->is_array()) {
        std::cerr << "Descriptor export could not read schema panels\n";
        return false;
    }

    const std::filesystem::path outPath = root / "artifacts" / "analysis" / "phase9_10_all45_control_surface_descriptor.json";
    std::filesystem::create_directories(outPath.parent_path());
    std::ofstream out(outPath, std::ios::out | std::ios::binary);
    if (!out) {
        std::cerr << "Descriptor export could not open " << outPath.string() << "\n";
        return false;
    }

    int laneCount = 0;
    int visibleControlCells = 0;
    int mcmullenDirectControlCount = 0;
    out << "{\n  \"ok\": true,\n  \"fractal_count\": " << kExpectedFractalCount << ",\n  \"lanes\": [\n";
    bool firstLane = true;
    for (const auto& enumId : enum_id_utils::kFractalTypeIds) {
        if (!firstLane) {
            out << ",\n";
        }
        firstLane = false;
        ++laneCount;
        out << "    { \"fractal_id\": ";
        WriteJsonEscapedString(out, enumId.id);
        out << ", \"controls\": [";
        bool firstControl = true;
        for (const json_min::Value& panel : panels->as_array()) {
            if (!panel.is_object()) continue;
            const json_min::Value* controls = panel.get("controls");
            if (!controls || !controls->is_array()) continue;
            for (const json_min::Value& control : controls->as_array()) {
                if (!control.is_object()) continue;
                std::string controlId;
                std::string controlType;
                std::string bindingPath;
                std::string valueType;
                if (!GetJsonStringField(control, "id", &controlId) ||
                    !GetJsonStringField(control, "type", &controlType) ||
                    !ReadControlBindingAndType(control, &bindingPath, &valueType) ||
                    !IsFamilySurfaceBindingPath(bindingPath) ||
                    !IsControlVisibleForFractal(control, enumId.value)) {
                    continue;
                }
                ++visibleControlCells;
                if (enumId.value == FractalType::mcmullen &&
                    (controlId == "mcmullen_m" || controlId == "mcmullen_n" || controlId == "mcmullen_lambda")) {
                    ++mcmullenDirectControlCount;
                }
                if (!firstControl) {
                    out << ",";
                }
                firstControl = false;
                out << "\n        { \"id\": ";
                WriteJsonEscapedString(out, controlId);
                out << ", \"owner_lane\": ";
                WriteJsonEscapedString(out, enumId.id);
                out << ", \"binding_path\": ";
                WriteJsonEscapedString(out, bindingPath);
                out << ", \"control_type\": ";
                WriteJsonEscapedString(out, controlType);
                out << ", \"value_type\": ";
                WriteJsonEscapedString(out, valueType);
                out << ", \"default_value\": ";
                WriteJsonEscapedString(out, JsonScalarAsDescriptorValue(control.get("default")));
                out << ", \"candidate_value\": ";
                WriteJsonEscapedString(out, CandidateValueForControl(control, valueType));
                out << ", \"expected_sensitivity\": ";
                WriteJsonEscapedString(out, SensitivityClassForControl(controlId, valueType));
                out << " }";
            }
        }
        if (!firstControl) {
            out << "\n      ";
        }
        out << "] }";
    }
    out << "\n  ],\n  \"visible_family_control_cells\": " << visibleControlCells
        << ",\n  \"mcmullen_direct_control_count\": " << mcmullenDirectControlCount << "\n}\n";
    out.close();
    if (!out) {
        std::cerr << "Descriptor export failed while writing " << outPath.string() << "\n";
        return false;
    }

    if (laneCount != kExpectedFractalCount) {
        std::cerr << "Descriptor export did not visit all 47 fractal lanes\n";
        return false;
    }
    if (visibleControlCells < 200) {
        std::cerr << "Descriptor export visited too few visible family controls: " << visibleControlCells << "\n";
        return false;
    }
    if (mcmullenDirectControlCount != 3) {
        std::cerr << "Descriptor export expected McMullen m/n/lambda direct controls, found " << mcmullenDirectControlCount << "\n";
        return false;
    }

    std::string descriptorText;
    if (!ReadTextFile(outPath, &descriptorText)) {
        std::cerr << "Descriptor export could not read back " << outPath.string() << "\n";
        return false;
    }
    json_min::ParseResult parsedDescriptor = json_min::Parse(descriptorText);
    if (!parsedDescriptor.error.empty()) {
        std::cerr << "Descriptor export wrote malformed JSON: " << parsedDescriptor.error << "\n";
        return false;
    }
    const json_min::Value* okValue = parsedDescriptor.value.get("ok");
    double parsedFractalCount = 0.0;
    double parsedMcMullenDirectCount = 0.0;
    if (!okValue || !okValue->is_bool() || !okValue->as_bool() ||
        !GetJsonNumberField(parsedDescriptor.value, "fractal_count", &parsedFractalCount) ||
        !NearlyEqual(parsedFractalCount, static_cast<double>(kExpectedFractalCount)) ||
        !GetJsonNumberField(parsedDescriptor.value, "mcmullen_direct_control_count", &parsedMcMullenDirectCount) ||
        !NearlyEqual(parsedMcMullenDirectCount, 3.0)) {
        std::cerr << "Descriptor export read-back did not preserve expected top-level authority fields\n";
        return false;
    }
    return true;
}

bool ValidateVisibleControlMatrix() {
    json_min::Value schemaRoot;
    if (!LoadCurrentSchemaJson(&schemaRoot)) {
        return false;
    }

    struct MatrixCase {
        const char* control_id;
        FractalType fractal_type;
        const char* binding_path;
        const char* value_type;
    };

    const MatrixCase cases[] = {
        {"julia_c_real", FractalType::julia, "fractal.params.julia_c_real", "float"},
        {"julia_c_imag", FractalType::julia, "fractal.params.julia_c_imag", "float"},
        {"explaino_julia_constant_mode", FractalType::explaino_julia, "fractal.params.explaino_julia_constant_mode", "enum"},
        {"epsilon", FractalType::newton, "fractal.params.epsilon", "float"},
        {"poly_kind", FractalType::newton, "fractal.params.poly_kind", "enum"},
        {"epsilon", FractalType::halley, "fractal.params.epsilon", "float"},
        {"poly_kind", FractalType::halley, "fractal.params.poly_kind", "enum"},
        {"epsilon", FractalType::nova, "fractal.params.epsilon", "float"},
        {"nova_alpha", FractalType::nova, "fractal.params.nova_alpha", "float"},
        {"poly_kind", FractalType::nova, "fractal.params.poly_kind", "enum"},
        {"phoenix_p_real", FractalType::phoenix, "fractal.params.phoenix_p_real", "float"},
        {"phoenix_p_imag", FractalType::phoenix, "fractal.params.phoenix_p_imag", "float"},
        {"lambda_real", FractalType::lambda_map, "fractal.params.lambda_real", "float"},
        {"lambda_imag", FractalType::lambda_map, "fractal.params.lambda_imag", "float"},
        {"multibrot_power_float", FractalType::multibrot, "fractal.params.multibrot_power_float", "float"},
        {"multibrot_power_imag", FractalType::multibrot, "fractal.params.multibrot_power_imag", "float"},
        {"burning_ship_fold_mix", FractalType::burning_ship, "fractal.params.burning_ship_fold_mix", "float"},
        {"celtic_abs_mix", FractalType::celtic_mandelbrot, "fractal.params.celtic_abs_mix", "float"},
        {"perpendicular_fold_mix", FractalType::perpendicular_burning_ship, "fractal.params.perpendicular_fold_mix", "float"},
        {"multicorn_power", FractalType::multicorn, "fractal.params.multibrot_power", "int"},
        {"mcmullen_preset", FractalType::mcmullen, "fractal.params.mcmullen_preset", "enum"},
        {"mcmullen_m", FractalType::mcmullen, "fractal.params.mcmullen_m", "int"},
        {"mcmullen_n", FractalType::mcmullen, "fractal.params.mcmullen_n", "int"},
        {"mcmullen_lambda", FractalType::mcmullen, "fractal.params.mcmullen_lambda", "float"},
        {"collatz_transition_strength", FractalType::collatz, "fractal.params.collatz_transition_strength", "float"},
        {"collatz_transition_strength", FractalType::explaino_collatz_direct, "fractal.params.collatz_transition_strength", "float"},
        {"spider_feedback", FractalType::spider, "fractal.params.spider_feedback", "float"},
        {"explaino_rational_escape_denominator_power", FractalType::explaino_rational_escape, "fractal.params.explaino_rational_escape_denominator_power", "int"},
        {"magnet_seed_real", FractalType::magnet, "fractal.params.magnet_seed_real", "float"},
        {"magnet_seed_imag", FractalType::magnet, "fractal.params.magnet_seed_imag", "float"},
        {"magnet_relaxation", FractalType::magnet, "fractal.params.magnet_relaxation", "float"},
        {"magnet_bailout", FractalType::magnet, "fractal.params.magnet_bailout", "float"},
        {"projection_and_flow_root_family", FractalType::projection_and_flow, "fractal.params.projection_and_flow_root_family", "enum"},
        {"projection_and_flow_target_radius", FractalType::projection_and_flow, "fractal.params.projection_and_flow_target_radius", "float"},
        {"projection_and_flow_pressure_threshold", FractalType::projection_and_flow, "fractal.params.projection_and_flow_pressure_threshold", "float"},
        {"projection_and_flow_root_family", FractalType::explaino_projection_and_flow, "fractal.params.projection_and_flow_root_family", "enum"},
        {"projection_and_flow_target_radius", FractalType::explaino_projection_and_flow, "fractal.params.projection_and_flow_target_radius", "float"},
        {"projection_and_flow_pressure_threshold", FractalType::explaino_projection_and_flow, "fractal.params.projection_and_flow_pressure_threshold", "float"},
        {"counterfactual_pair_root_family", FractalType::counterfactual_pair, "fractal.params.counterfactual_pair_root_family", "enum"},
        {"counterfactual_pair_root_family", FractalType::explaino_counterfactual_pair, "fractal.params.counterfactual_pair_root_family", "enum"},
        {"counterfactual_pair_frame", FractalType::counterfactual_pair, "fractal.params.counterfactual_pair_frame", "enum"},
        {"counterfactual_pair_offset_x", FractalType::counterfactual_pair, "fractal.params.counterfactual_pair_offset_x", "float"},
        {"counterfactual_pair_offset_y", FractalType::counterfactual_pair, "fractal.params.counterfactual_pair_offset_y", "float"},
        {"counterfactual_pair_reconvergence_ratio", FractalType::counterfactual_pair, "fractal.params.counterfactual_pair_reconvergence_ratio", "float"},
        {"counterfactual_pair_frame", FractalType::explaino_counterfactual_pair, "fractal.params.counterfactual_pair_frame", "enum"},
        {"counterfactual_pair_offset_x", FractalType::explaino_counterfactual_pair, "fractal.params.counterfactual_pair_offset_x", "float"},
        {"counterfactual_pair_offset_y", FractalType::explaino_counterfactual_pair, "fractal.params.counterfactual_pair_offset_y", "float"},
        {"counterfactual_pair_reconvergence_ratio", FractalType::explaino_counterfactual_pair, "fractal.params.counterfactual_pair_reconvergence_ratio", "float"},
        {"transcendental_func", FractalType::explaino_transcendental, "fractal.params.transcendental_func", "enum"},
    };

    for (const MatrixCase& testCase : cases) {
        if (!ValidateVisibleControlMatrixCase(schemaRoot, testCase.control_id, testCase.fractal_type, testCase.binding_path, testCase.value_type)) {
            return false;
        }
    }

    const MatrixCase explainoCommonCases[] = {
        {"auto_increment_seed", FractalType::explaino, "fractal.view.auto_increment_seed", "bool"},
        {"epsilon", FractalType::explaino, "fractal.params.epsilon", "float"},
        {"explaino_seed", FractalType::explaino, "fractal.params.explaino_seed", "double"},
        {"explaino_warp_strength", FractalType::explaino, "fractal.params.explaino_warp_strength", "float"},
        {"explaino_root_spread", FractalType::explaino, "fractal.params.explaino_root_spread", "float"},
        {"explaino_phase_strength", FractalType::explaino, "fractal.view.explaino_phase_strength", "float"},
        {"explaino_damping", FractalType::explaino, "fractal.params.explaino_damping", "float"},
        {"explaino_phase", FractalType::explaino, "fractal.view.explaino_phase", "float"},
        {"explaino_seed_drift", FractalType::explaino, "fractal.view.explaino_seed_drift", "float"},
        {"explaino_seed_tween", FractalType::explaino, "fractal.view.explaino_seed_tween", "bool"},
    };

    for (const ExplainoSelectorDescriptor& selector : kExplainoSelectorRegistry) {
        for (const MatrixCase& commonCase : explainoCommonCases) {
            if (ExplainoCommonControlExpectedVisible(commonCase.control_id, selector.fractal_type)) {
                if (!ValidateVisibleControlMatrixCase(schemaRoot, commonCase.control_id, selector.fractal_type, commonCase.binding_path, commonCase.value_type)) {
                    return false;
                }
            } else if (!ValidateHiddenControlMatrixCase(schemaRoot, commonCase.control_id, selector.fractal_type)) {
                return false;
            }
        }
    }

    return true;
}

bool ValidateEnumComboEditMatrix() {
    json_min::Value schemaRoot;
    if (!LoadCurrentSchemaJson(&schemaRoot)) {
        return false;
    }

    struct EnumComboCase {
        const char* control_id;
        FractalType fractal_type;
        const char* binding_path;
        const char* edit_id;
    };

    const EnumComboCase cases[] = {
        {"poly_kind", FractalType::newton, "fractal.params.poly_kind", "custom"},
        {"poly_kind", FractalType::nova, "fractal.params.poly_kind", "custom"},
        {"poly_kind", FractalType::halley, "fractal.params.poly_kind", "custom"},
        {"mcmullen_preset", FractalType::mcmullen, "fractal.params.mcmullen_preset", "custom"},
        {"counterfactual_pair_root_family", FractalType::counterfactual_pair, "fractal.params.counterfactual_pair_root_family", "quartic_unit_roots"},
        {"counterfactual_pair_root_family", FractalType::explaino_counterfactual_pair, "fractal.params.counterfactual_pair_root_family", "quartic_unit_roots"},
        {"counterfactual_pair_frame", FractalType::counterfactual_pair, "fractal.params.counterfactual_pair_frame", "view_relative"},
        {"counterfactual_pair_frame", FractalType::explaino_counterfactual_pair, "fractal.params.counterfactual_pair_frame", "view_relative"},
        {"projection_and_flow_root_family", FractalType::projection_and_flow, "fractal.params.projection_and_flow_root_family", "quartic_unit_roots"},
        {"projection_and_flow_root_family", FractalType::explaino_projection_and_flow, "fractal.params.projection_and_flow_root_family", "quartic_unit_roots"},
        {"transcendental_func", FractalType::explaino_transcendental, "fractal.params.transcendental_func", "f_cosh"},
    };

    for (const EnumComboCase& testCase : cases) {
        const json_min::Value* control = FindSchemaControlById(schemaRoot, testCase.control_id);
        std::string bindingPath;
        std::string valueType;
        if (!control || !ReadControlBindingAndType(*control, &bindingPath, &valueType) ||
            valueType != "enum" || bindingPath != testCase.binding_path) {
            std::cerr << "Enum/combo matrix found wrong schema control for " << testCase.control_id << "\n";
            return false;
        }

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        view.fractal_type = testCase.fractal_type;
        BindingContext ctx = MakeBindingContext(&view, &params, &render, &lens);
        if (!IsControlVisibleForFractal(*control, testCase.fractal_type)) {
            std::cerr << "Enum/combo matrix hid " << testCase.control_id << " on its owner lane\n";
            return false;
        }
        if (!HasVisibleEnumOptionForFractal(*control, testCase.edit_id, testCase.fractal_type)) {
            std::cerr << "Enum/combo matrix did not expose option " << testCase.edit_id << " for " << testCase.control_id << "\n";
            return false;
        }
        if (!ctx.SetEnumId(testCase.binding_path, testCase.edit_id) || ctx.GetEnumId(testCase.binding_path) != testCase.edit_id) {
            std::cerr << "Enum/combo matrix edit did not round-trip through BindingContext for " << testCase.control_id << "\n";
            return false;
        }
    }

    return true;
}

bool ValidateFixedAndPresetSurfaceClassifications() {
    json_min::Value schemaRoot;
    if (!LoadCurrentSchemaJson(&schemaRoot)) {
        return false;
    }
    const json_min::Value* panels = schemaRoot.get("panels");
    if (!panels || !panels->is_array()) {
        std::cerr << "Fixed-formula classification could not read schema panels\n";
        return false;
    }

    const FractalType fixedFormulaLanes[] = {
        FractalType::mandelbrot,
        FractalType::burning_ship,
        FractalType::spider,
        FractalType::celtic_mandelbrot,
        FractalType::perpendicular_burning_ship,
        FractalType::collatz,
    };

    for (FractalType fixedLane : fixedFormulaLanes) {
        for (const json_min::Value& panel : panels->as_array()) {
            if (!panel.is_object()) continue;
            const json_min::Value* controls = panel.get("controls");
            if (!controls || !controls->is_array()) continue;
            for (const json_min::Value& control : controls->as_array()) {
                if (!control.is_object() || !control.get("visible_if")) {
                    continue;
                }
                std::string bindingPath;
                std::string valueType;
                if (!ReadControlBindingAndType(control, &bindingPath, &valueType)) {
                    continue;
                }
                if (!IsControlVisibleForFractal(control, fixedLane)) {
                    continue;
                }
                std::string controlId;
                GetJsonStringField(control, "id", &controlId);
                const bool isExpectedOwnerControl =
                    (fixedLane == FractalType::spider &&
                        controlId == "spider_feedback" &&
                        bindingPath == "fractal.params.spider_feedback") ||
                    (fixedLane == FractalType::collatz &&
                        controlId == "collatz_transition_strength" &&
                        bindingPath == "fractal.params.collatz_transition_strength") ||
                    (fixedLane == FractalType::burning_ship &&
                        controlId == "burning_ship_fold_mix" &&
                        bindingPath == "fractal.params.burning_ship_fold_mix") ||
                    (fixedLane == FractalType::celtic_mandelbrot &&
                        controlId == "celtic_abs_mix" &&
                        bindingPath == "fractal.params.celtic_abs_mix") ||
                    (fixedLane == FractalType::perpendicular_burning_ship &&
                        controlId == "perpendicular_fold_mix" &&
                        bindingPath == "fractal.params.perpendicular_fold_mix");
                if (!isExpectedOwnerControl && !IsGlobalFixedFormulaBindingPath(bindingPath)) {
                    const char* fractalId = FractalTypeId(fixedLane);
                    std::cerr << "Fixed-formula lane " << (fractalId ? fractalId : "<unknown>")
                              << " unexpectedly exposes family control " << controlId << "\n";
                    return false;
                }
            }
        }
    }

    for (const json_min::Value& panel : panels->as_array()) {
        if (!panel.is_object()) continue;
        const json_min::Value* controls = panel.get("controls");
        if (!controls || !controls->is_array()) continue;
        for (const json_min::Value& control : controls->as_array()) {
            if (!control.is_object() || !control.get("visible_if")) {
                continue;
            }
            std::string bindingPath;
            std::string valueType;
            if (!ReadControlBindingAndType(control, &bindingPath, &valueType)) {
                continue;
            }
            if (!IsControlVisibleForFractal(control, FractalType::mcmullen)) {
                continue;
            }
            std::string controlId;
            GetJsonStringField(control, "id", &controlId);
            const bool isExpectedMcMullenControl =
                controlId == "mcmullen_preset" ||
                controlId == "mcmullen_m" ||
                controlId == "mcmullen_n" ||
                controlId == "mcmullen_lambda";
            if (!isExpectedMcMullenControl && !IsGlobalFixedFormulaBindingPath(bindingPath)) {
                std::cerr << "McMullen direct-control classification found an unexpected family control: " << controlId << "\n";
                return false;
            }
        }
    }

    return true;
}

bool ValidateAnimationTargetVisibilityMirrorsControls() {
    json_min::Value schemaRoot;
    if (!LoadCurrentSchemaJson(&schemaRoot)) {
        return false;
    }

    const json_min::Value* animTarget = FindSchemaControlById(schemaRoot, "param_anim_target");
    if (!animTarget) {
        std::cerr << "Animation target visibility mirror could not find the param_anim_target combo\n";
        return false;
    }

    struct MirrorCase {
        const char* option_id;
        const char* control_id;
    };

    const MirrorCase cases[] = {
        {"seed", "explaino_seed"},
        {"damping", "explaino_damping"},
        {"warp_strength", "explaino_warp_strength"},
        {"root_spread", "explaino_root_spread"},
        {"mix", "explaino_mix"},
        {"nova_alpha", "nova_alpha"},
        {"phoenix_p_real", "phoenix_p_real"},
        {"burning_ship_fold_mix", "burning_ship_fold_mix"},
        {"celtic_abs_mix", "celtic_abs_mix"},
        {"perpendicular_fold_mix", "perpendicular_fold_mix"},
        {"multibrot_power", "multibrot_power_float"},
        {"julia_c_real", "julia_c_real"},
        {"julia_c_imag", "julia_c_imag"},
        {"lambda_real", "lambda_real"},
        {"magnet_relaxation", "magnet_relaxation"},
        {"magnet_bailout", "magnet_bailout"},
        {"collatz_transition_strength", "collatz_transition_strength"},
        {"momentum_beta", "momentum_beta"},
        {"joy_coupling", "joy_coupling"},
        {"fold_coupling", "fold_coupling"},
        {"bell_coupling", "bell_coupling"},
        {"ripple_amplitude", "ripple_amplitude"},
        {"splice_offset", "splice_offset"},
        {"vortex_strength", "vortex_strength"},
        {"tension_strength", "tension_strength"},
        {"explaino_phase", "explaino_phase"},
    };

    for (const auto& fractalId : enum_id_utils::kFractalTypeIds) {
        if (!HasVisibleEnumOptionForFractal(*animTarget, "none", fractalId.value)) {
            std::cerr << "Animation target visibility mirror hid the None option\n";
            return false;
        }

        for (const MirrorCase& testCase : cases) {
            const json_min::Value* ownerControl = FindSchemaControlById(schemaRoot, testCase.control_id);
            if (!ownerControl) {
                std::cerr << "Animation target visibility mirror missing owner control " << testCase.control_id << "\n";
                return false;
            }
            const bool optionVisible = HasVisibleEnumOptionForFractal(*animTarget, testCase.option_id, fractalId.value);
            const bool ownerVisible = IsControlVisibleForFractal(*ownerControl, fractalId.value);
            if (optionVisible != ownerVisible) {
                std::cerr << "Animation target option " << testCase.option_id
                          << " no longer mirrors owner control " << testCase.control_id
                          << " on fractal " << fractalId.id << "\n";
                return false;
            }
        }
    }

    return true;
}

} // namespace

int main() {
    {
        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        BindingContext ctx = MakeBindingContext(&view, &params, &render, &lens);

        if (ctx.GetEnumId("fractal.params.poly_kind") != "z3_minus_1") {
            std::cerr << "Expected poly kind enum round-trip to start at z3_minus_1\n";
            return 1;
        }
        if (ctx.GetEnumId("fractal.params.counterfactual_pair_root_family") != "cubic_unit_roots") {
            std::cerr << "Expected Counterfactual Pair root family enum round-trip to start at cubic_unit_roots\n";
            return 1;
        }
        if (ctx.GetEnumId("fractal.params.counterfactual_pair_frame") != "world_absolute") {
            std::cerr << "Expected Counterfactual Pair frame enum round-trip to start at world_absolute\n";
            return 1;
        }
        if (ctx.GetEnumId("fractal.params.projection_and_flow_root_family") != "cubic_unit_roots") {
            std::cerr << "Expected Projection-and-Flow root family enum round-trip to start at cubic_unit_roots\n";
            return 1;
        }
        if (ctx.GetEnumId("fractal.params.explaino_root_authority") != "generated") {
            std::cerr << "Expected Explaino root authority to default to generated mode\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.params.explaino_root_authority", "custom") ||
            ctx.GetEnumId("fractal.params.explaino_root_authority") != "custom" ||
            params.explaino_root_count != 4) {
            std::cerr << "Expected Explaino root authority custom mode to create a bounded editable root set\n";
            return 1;
        }
        float* explainoRoot0X = nullptr;
        float* explainoRoot0Y = nullptr;
        if (!ctx.BindFloat("fractal.params.explaino_roots.0.x", &explainoRoot0X) ||
            !ctx.BindFloat("fractal.params.explaino_roots.0.y", &explainoRoot0Y) ||
            explainoRoot0X != &params.explaino_roots[0].x ||
            explainoRoot0Y != &params.explaino_roots[0].y) {
            std::cerr << "Expected safe Explaino root editor controls to bind to bounded root coordinate fields\n";
            return 1;
        }
        bool customRootsActive = false;
        if (!ctx.GetBoolValue("fractal.params.explaino_custom_roots_active", customRootsActive) || !customRootsActive) {
            std::cerr << "Expected custom root visibility predicate to activate in custom root authority mode\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.params.explaino_root_authority", "generated") ||
            ctx.GetEnumId("fractal.params.explaino_root_authority") != "generated" ||
            params.explaino_root_count != 0 ||
            !ctx.GetBoolValue("fractal.params.explaino_custom_roots_active", customRootsActive) || customRootsActive) {
            std::cerr << "Expected generated root authority to hide custom roots and clear explicit root count\n";
            return 1;
        }
        view.fractal_type = FractalType::newton;
        params.poly_kind = PolyKind::z3_minus_1;
        bool customPolyActive = true;
        if (!ctx.GetBoolValue("fractal.params.poly_coefficients_custom_active", customPolyActive) || customPolyActive) {
            std::cerr << "Expected polynomial coefficient editor predicate to stay false for preset polynomial modes\n";
            return 1;
        }
        params.poly_kind = PolyKind::custom;
        if (!ctx.GetBoolValue("fractal.params.poly_coefficients_custom_active", customPolyActive) || !customPolyActive) {
            std::cerr << "Expected polynomial coefficient editor predicate to activate only for custom polynomial mode\n";
            return 1;
        }
        view.fractal_type = FractalType::explaino;
        if (ctx.GetEnumId("fractal.view.fractal_type") != "explaino") {
            std::cerr << "Expected fractal type enum round-trip to start at explaino\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.params.coloring_mode", "smooth_escape") ||
            ctx.GetEnumId("fractal.params.coloring_mode") != "smooth_escape") {
            std::cerr << "Expected coloring mode enum round-trip to accept smooth_escape\n";
            return 1;
        }
        float* smoothInteriorStrength = nullptr;
        if (!ctx.BindFloat("fractal.params.color_smooth_escape_interior_strength", &smoothInteriorStrength) ||
            smoothInteriorStrength != &params.color_smooth_escape_interior_strength) {
            std::cerr << "Expected smooth-escape interior strength to bind through the public Color panel path\n";
            return 1;
        }
        *smoothInteriorStrength = 0.65f;
        if (!NearlyEqual(params.color_smooth_escape_interior_strength, 0.65f)) {
            std::cerr << "Expected smooth-escape interior strength edits to write the live KernelParams field\n";
            return 1;
        }
        UISchemaPredicate smoothInteriorVisible{};
        smoothInteriorVisible.op = "eq";
        smoothInteriorVisible.path = "fractal.params.coloring_mode";
        smoothInteriorVisible.value = "smooth_escape";
        if (!ctx.EvalVisibleIf(smoothInteriorVisible)) {
            std::cerr << "Expected smooth-escape interior strength to be visible in smooth_escape mode\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.params.coloring_mode", "iteration_count") ||
            ctx.EvalVisibleIf(smoothInteriorVisible)) {
            std::cerr << "Expected smooth-escape interior strength to hide outside smooth_escape mode\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.params.coloring_mode", "smooth_escape")) {
            std::cerr << "Expected coloring mode to return to smooth_escape after visibility proof\n";
            return 1;
        }
        if (ctx.GetEnumId("fractal.params.color_signal") != "smooth_escape" ||
            ctx.GetEnumId("fractal.params.color_palette") != "cyclic_escape" ||
            ctx.GetEnumId("fractal.params.color_grading") != "escape_default") {
            std::cerr << "Expected legacy coloring mode edits to keep the split color pipeline in sync\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.params.color_signal", "phase_angle") ||
            ctx.GetEnumId("fractal.params.color_signal") != "phase_angle" ||
            ctx.GetEnumId("fractal.params.color_palette") != "phase_wheel" ||
            ctx.GetEnumId("fractal.params.color_grading") != "phase_default" ||
            ctx.GetEnumId("fractal.params.coloring_mode") != "phase") {
            std::cerr << "Expected split color signal edits to coerce the rest of the color pipeline onto a valid exact runtime combination\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.params.color_signal", "escape_magnitude") ||
            ctx.GetEnumId("fractal.params.color_signal") != "escape_magnitude" ||
            ctx.GetEnumId("fractal.params.color_palette") != "cyclic_escape" ||
            ctx.GetEnumId("fractal.params.color_grading") != "escape_default" ||
            ctx.GetEnumId("fractal.params.coloring_mode") != "smooth_escape") {
            std::cerr << "Expected widened split color signal edits to accept escape_magnitude through the runtime-supported programmable mirror\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.params.color_signal", "orbit_stripe") ||
            ctx.GetEnumId("fractal.params.color_signal") != "orbit_stripe" ||
            ctx.GetEnumId("fractal.params.color_palette") != "phase_wheel" ||
            ctx.GetEnumId("fractal.params.color_grading") != "phase_default" ||
            ctx.GetEnumId("fractal.params.coloring_mode") != "phase") {
            std::cerr << "Expected widened split color signal edits to accept orbit_stripe through the runtime-supported programmable mirror\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.params.coloring_mode", "smooth_escape") ||
            !ctx.SetEnumId("fractal.params.color_signal", "root_proximity") ||
            ctx.GetEnumId("fractal.params.color_signal") != "root_proximity" ||
            ctx.GetEnumId("fractal.params.color_palette") != "cyclic_escape" ||
            ctx.GetEnumId("fractal.params.color_grading") != "escape_default" ||
            ctx.GetEnumId("fractal.params.coloring_mode") != "smooth_escape") {
            std::cerr << "Expected basin-capable split color signal edits to accept family-gated root_proximity through the current source-only programmable mirror\n";
            return 1;
        }
        view.fractal_type = FractalType::mandelbrot;
        if (ctx.SetEnumId("fractal.params.color_signal", "root_proximity")) {
            std::cerr << "Expected escape-time families to reject family-gated root_proximity instead of silently coercing it\n";
            return 1;
        }
        view.fractal_type = FractalType::explaino;
        if (!ctx.SetEnumId("fractal.params.coloring_mode", "smooth_escape") ||
            !ctx.SetEnumId("fractal.params.color_grading", "phase_default") ||
            ctx.GetEnumId("fractal.params.coloring_mode") != "phase" ||
            ctx.GetEnumId("fractal.params.color_signal") != "phase_angle" ||
            ctx.GetEnumId("fractal.params.color_palette") != "phase_wheel" ||
            ctx.GetEnumId("fractal.params.color_grading") != "phase_default") {
            std::cerr << "Expected public coloring mode plus grading edits to stay on a coherent runtime-supported color pipeline\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.view.fractal_type", "lambda") ||
            ctx.GetEnumId("fractal.view.fractal_type") != "lambda") {
            std::cerr << "Expected fractal type enum round-trip to accept lambda\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.view.fractal_type", "magnet") ||
            ctx.GetEnumId("fractal.view.fractal_type") != "magnet") {
            std::cerr << "Expected fractal type enum round-trip to accept magnet\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.view.fractal_type", "generic_equation_pack") ||
            ctx.GetEnumId("fractal.view.fractal_type") != "generic_equation_pack") {
            std::cerr << "Expected fractal type enum round-trip to accept generic_equation_pack\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.view.fractal_type", "sdf_pack_scene") ||
            ctx.GetEnumId("fractal.view.fractal_type") != "sdf_pack_scene") {
            std::cerr << "Expected fractal type enum round-trip to accept sdf_pack_scene\n";
            return 1;
        }
        float* juliaCReal = nullptr;
        float* juliaCImag = nullptr;
        float* explainoJuliaCReal = nullptr;
        float* explainoJuliaCImag = nullptr;
        if (!ctx.BindFloat("fractal.params.julia_c_real", &juliaCReal) ||
            !ctx.BindFloat("fractal.params.julia_c_imag", &juliaCImag) ||
            juliaCReal != &params.julia_c_real ||
            juliaCImag != &params.julia_c_imag ||
            !NearlyEqual(*juliaCReal, -0.7f) ||
            !NearlyEqual(*juliaCImag, 0.27015f)) {
            std::cerr << "Expected standalone Julia constant controls to bind to explicit KernelParams fields\n";
            return 1;
        }
        if (ctx.GetEnumId("fractal.params.explaino_julia_constant_mode") != "seeded" ||
            !ctx.SetEnumId("fractal.params.explaino_julia_constant_mode", "custom") ||
            ctx.GetEnumId("fractal.params.explaino_julia_constant_mode") != "custom" ||
            !ctx.BindFloat("fractal.params.explaino_julia_c_real", &explainoJuliaCReal) ||
            !ctx.BindFloat("fractal.params.explaino_julia_c_imag", &explainoJuliaCImag) ||
            explainoJuliaCReal != &params.explaino_julia_c_real ||
            explainoJuliaCImag != &params.explaino_julia_c_imag) {
            std::cerr << "Expected Explaino Julia authority controls to bind and round-trip through explicit KernelParams fields\n";
            return 1;
        }
        view.fractal_type = FractalType::explaino_julia;
        bool customActive = false;
        if (!ctx.GetBoolValue("fractal.params.explaino_julia_custom_constants_active", customActive) || !customActive) {
            std::cerr << "Expected Explaino Julia custom-constants visibility predicate to activate only in custom mode on explaino_julia\n";
            return 1;
        }
        view.fractal_type = FractalType::explaino_all;
        if (!ctx.GetBoolValue("fractal.params.explaino_julia_custom_constants_active", customActive) || customActive) {
            std::cerr << "Expected Explaino Julia custom-constants visibility predicate to stay false on explaino_all\n";
            return 1;
        }
        float* magnetSeedReal = nullptr;
        float* magnetSeedImag = nullptr;
        float* magnetRelaxation = nullptr;
        float* magnetBailout = nullptr;
        if (!ctx.BindFloat("fractal.params.magnet_seed_real", &magnetSeedReal) ||
            !ctx.BindFloat("fractal.params.magnet_seed_imag", &magnetSeedImag) ||
            !ctx.BindFloat("fractal.params.magnet_relaxation", &magnetRelaxation) ||
            !ctx.BindFloat("fractal.params.magnet_bailout", &magnetBailout) ||
            magnetSeedReal != &params.magnet_seed_real ||
            magnetSeedImag != &params.magnet_seed_imag ||
            magnetRelaxation != &params.magnet_relaxation ||
            magnetBailout != &params.magnet_bailout ||
            !NearlyEqual(*magnetRelaxation, 1.0f) || !NearlyEqual(*magnetBailout, 12.0f)) {
            std::cerr << "Expected Magnet schema bindings to own the bounded Type I numeric controls\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.view.fractal_type", "projection_and_flow") ||
            ctx.GetEnumId("fractal.view.fractal_type") != "projection_and_flow") {
            std::cerr << "Expected fractal type enum round-trip to accept projection_and_flow\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.view.fractal_type", "counterfactual_pair") ||
            ctx.GetEnumId("fractal.view.fractal_type") != "counterfactual_pair") {
            std::cerr << "Expected fractal type enum round-trip to accept counterfactual_pair\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.view.fractal_type", "explaino_counterfactual_pair") ||
            ctx.GetEnumId("fractal.view.fractal_type") != "explaino_counterfactual_pair") {
            std::cerr << "Expected fractal type enum round-trip to preserve explaino_counterfactual_pair as an explicit Explaino carrier\n";
            return 1;
        }
        params.poly_kind = PolyKind::custom;
        params.explaino_root_count = 3;
        if (!ctx.SetEnumId("fractal.params.counterfactual_pair_root_family", "quartic_unit_roots") ||
            ctx.GetEnumId("fractal.params.counterfactual_pair_root_family") != "quartic_unit_roots" ||
            ctx.GetEnumId("fractal.view.fractal_type") != "explaino_counterfactual_pair" ||
            params.poly_kind != PolyKind::custom ||
            params.explaino_root_count != 3) {
            std::cerr << "Expected Explaino Counterfactual root-family binding edits to stay on the explicit Explaino carrier until derived-field refresh\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.view.fractal_type", "explaino_projection_and_flow") ||
            ctx.GetEnumId("fractal.view.fractal_type") != "explaino_projection_and_flow") {
            std::cerr << "Expected fractal type enum round-trip to preserve explaino_projection_and_flow as an explicit Explaino carrier\n";
            return 1;
        }
        view.explaino_phase = 1.0f;
        params.explaino_seed = 3.0;
        params.explaino_root_spread = 0.5f;
        params.poly_kind = PolyKind::custom;
        params.poly_coeffs[0] = 1.0f;
        params.poly_coeffs[1] = 0.0f;
        params.poly_coeffs[2] = 0.0f;
        params.poly_coeffs[3] = 1.0f;
        params.poly_coeffs[4] = 1.0f;
        params.explaino_root_count = 3;
        if (!ctx.SetEnumId("fractal.params.projection_and_flow_root_family", "quartic_unit_roots") ||
            ctx.GetEnumId("fractal.params.projection_and_flow_root_family") != "quartic_unit_roots" ||
            params.poly_kind != PolyKind::custom ||
            params.explaino_root_count != 3) {
            std::cerr << "Expected explaino_projection_and_flow root family edits to preserve Explaino carrier authority instead of preset-syncing back to the standalone lane\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.view.fractal_type", "counterfactual_pair") ||
            ctx.GetEnumId("fractal.view.fractal_type") != "counterfactual_pair") {
            std::cerr << "Expected schema binding to switch back to counterfactual_pair before exercising the standalone root-family owner seam\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.params.counterfactual_pair_root_family", "quartic_unit_roots") ||
            ctx.GetEnumId("fractal.params.counterfactual_pair_root_family") != "quartic_unit_roots" ||
            params.poly_kind != PolyKind::z4_minus_1 ||
            !NearlyEqual(params.poly_coeffs[0], -1.0f) ||
            !NearlyEqual(params.poly_coeffs[3], 0.0f) ||
            !NearlyEqual(params.poly_coeffs[4], 1.0f)) {
            std::cerr << "Expected Counterfactual Pair root family edits to own the shipped polynomial preset\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.params.counterfactual_pair_frame", "view_relative") ||
            ctx.GetEnumId("fractal.params.counterfactual_pair_frame") != "view_relative") {
            std::cerr << "Expected Counterfactual Pair frame enum round-trip to accept view_relative\n";
            return 1;
        }
        float* pairOffsetX = nullptr;
        float* pairOffsetY = nullptr;
        float* pairReconvergenceRatio = nullptr;
        if (!ctx.BindFloat("fractal.params.counterfactual_pair_offset_x", &pairOffsetX) || !pairOffsetX || !NearlyEqual(*pairOffsetX, 0.16f) ||
            !ctx.BindFloat("fractal.params.counterfactual_pair_offset_y", &pairOffsetY) || !pairOffsetY || !NearlyEqual(*pairOffsetY, 0.08f) ||
            !ctx.BindFloat("fractal.params.counterfactual_pair_reconvergence_ratio", &pairReconvergenceRatio) || !pairReconvergenceRatio ||
            !NearlyEqual(*pairReconvergenceRatio, 0.60f)) {
            std::cerr << "Expected Counterfactual Pair float controls to bind to the shipped gap and reconvergence params\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.view.fractal_type", "projection_and_flow") ||
            ctx.GetEnumId("fractal.view.fractal_type") != "projection_and_flow") {
            std::cerr << "Expected schema binding to switch to projection_and_flow before exercising the Projection-and-Flow owner seams\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.params.projection_and_flow_root_family", "quartic_unit_roots") ||
            ctx.GetEnumId("fractal.params.projection_and_flow_root_family") != "quartic_unit_roots" ||
            params.poly_kind != PolyKind::z4_minus_1 ||
            !NearlyEqual(params.poly_coeffs[0], -1.0f) ||
            !NearlyEqual(params.poly_coeffs[3], 0.0f) ||
            !NearlyEqual(params.poly_coeffs[4], 1.0f)) {
            std::cerr << "Expected Projection-and-Flow root family edits to own the shipped polynomial preset\n";
            return 1;
        }
        float* projectionRadius = nullptr;
        float* projectionPressureThreshold = nullptr;
        if (!ctx.BindFloat("fractal.params.projection_and_flow_target_radius", &projectionRadius) || !projectionRadius || !NearlyEqual(*projectionRadius, 1.0f) ||
            !ctx.BindFloat("fractal.params.projection_and_flow_pressure_threshold", &projectionPressureThreshold) || !projectionPressureThreshold ||
            !NearlyEqual(*projectionPressureThreshold, 1.0f)) {
            std::cerr << "Expected Projection-and-Flow float controls to bind to the shipped target-radius and pressure-threshold params\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.view.fractal_type", "explaino_vortex") ||
            ctx.GetEnumId("fractal.view.fractal_type") != "explaino_vortex" ||
            !NearlyEqual(params.ripple_amplitude, 0.0f) ||
            !NearlyEqual(params.splice_offset, 0.0f) ||
            !NearlyEqual(params.vortex_strength, 0.3f) ||
            !NearlyEqual(params.tension_strength, 0.0f) ||
            !NearlyEqual(params.balance_void, 0.0f) ||
            !NearlyEqual(params.symmetry_tension, 0.0f) ||
            !NearlyEqual(params.field_curvature, 0.0f)) {
            std::cerr << "Expected explaino_vortex selection to preserve its explicit public identity while loading the vortex preset vector\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.view.fractal_type", "explaino_balance_void") ||
            ctx.GetEnumId("fractal.view.fractal_type") != "explaino_balance_void") {
            std::cerr << "Expected explaino_balance_void selection to preserve its explicit public family identity\n";
            return 1;
        }
        float* balanceVoidParam = nullptr;
        float* symmetryTensionParam = nullptr;
        float* fieldCurvatureParam = nullptr;
        if (!ctx.BindFloat("fractal.params.balance_void", &balanceVoidParam) ||
            !ctx.BindFloat("fractal.params.symmetry_tension", &symmetryTensionParam) ||
            !ctx.BindFloat("fractal.params.field_curvature", &fieldCurvatureParam) ||
            balanceVoidParam != &params.balance_void ||
            symmetryTensionParam != &params.symmetry_tension ||
            fieldCurvatureParam != &params.field_curvature) {
            std::cerr << "Expected ExplainO-BalanceVoid schema bindings to own dedicated family-axis params instead of reusing grading-only balance_void_grade paths\n";
            return 1;
        }
        if (!NearlyEqual(*balanceVoidParam, 0.0f) ||
            !NearlyEqual(*symmetryTensionParam, 0.0f) ||
            !NearlyEqual(*fieldCurvatureParam, 0.0f)) {
            std::cerr << "Expected ExplainO-BalanceVoid family axes to start from the neutral Explaino collapse point\n";
            return 1;
        }
        bool familyDirty = false;
        UISchemaControl symmetryDefault = MakeBoundControl("symmetry_tension", "slider_float", "Symmetry Tension", "float", "param", "fractal.params.symmetry_tension");
        symmetryDefault.has_default = true;
        symmetryDefault.def = json_min::Value{0.125};
        if (!ApplySchemaDefaultForControl(symmetryDefault, ctx, &familyDirty) ||
            !familyDirty ||
            !NearlyEqual(params.symmetry_tension, 0.125f)) {
            std::cerr << "Expected ExplainO-BalanceVoid schema defaults to drive the dedicated family-axis owner path\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.view.camera_behavior", "orbit") ||
            ctx.GetEnumId("fractal.view.camera_behavior") != "orbit") {
            std::cerr << "Expected camera behavior enum round-trip to accept orbit\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.render.sample_tier", "standard") ||
            ctx.GetEnumId("fractal.render.sample_tier") != "standard") {
            std::cerr << "Expected sample tier enum round-trip to accept standard\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.params.mcmullen_preset", "z4_z2") ||
            ctx.GetEnumId("fractal.params.mcmullen_preset") != "z4_z2") {
            std::cerr << "Expected McMullen preset enum round-trip to accept z4_z2\n";
            return 1;
        }
        if (ctx.SetEnumId("fractal.view.fractal_type", "not_real")) {
            std::cerr << "Invalid enum ids should fail instead of silently falling back\n";
            return 1;
        }
        if (ctx.SetEnumId("fractal.params.color_palette", "not_real")) {
            std::cerr << "Invalid split-color enum ids should fail instead of silently falling back\n";
            return 1;
        }
        if (!ctx.GetEnumId("fractal.unknown.path").empty()) {
            std::cerr << "Unknown enum paths should return an empty id\n";
            return 1;
        }

        UISchemaControl coloringMode = MakeBoundControl("coloring_mode", "combo", "Coloring Mode", "enum", "param", "fractal.params.coloring_mode");
        coloringMode.options = {
            {"root_basin", "Root Basins (Root-Finding)", ""},
            {"joy_basins", "Joy Basins (All paths succeed)", ""},
            {"iteration_count", "Iteration Count", ""},
            {"smooth_escape", "Smooth Escape", ""},
            {"phase", "Phase (Hue Wheel)", ""},
            {"iteration_bands", "Iteration Bands", ""},
        };

        view.fractal_type = FractalType::explaino;
        const std::vector<const UISchemaOption*> explainoOptions = ResolveVisibleEnumOptions(coloringMode, ctx);
        if (explainoOptions.size() != 6) {
            std::cerr << "Expected basin-capable fractals to expose all coloring options\n";
            return 1;
        }

        view.fractal_type = FractalType::mandelbrot;
        const std::vector<const UISchemaOption*> mandelbrotOptions = ResolveVisibleEnumOptions(coloringMode, ctx);
        if (mandelbrotOptions.size() != 4) {
            std::cerr << "Expected escape-time fractals to hide basin-only coloring options\n";
            return 1;
        }
        for (const UISchemaOption* option : mandelbrotOptions) {
            if (!option) {
                std::cerr << "Expected visible coloring options to remain addressable\n";
                return 1;
            }
            if (option->id == "root_basin" || option->id == "joy_basins") {
                std::cerr << "Escape-time fractals should not expose basin-only coloring options\n";
                return 1;
            }
        }

        UISchemaControl animTarget = MakeBoundControl("param_anim_target", "combo", "Animate Parameter", "enum", "param", "fractal.view.param_anim_target");
        UISchemaOption noneTarget{"none", "None", ""};
        UISchemaOption magnetTarget{"magnet_relaxation", "Magnet Relaxation", ""};
        magnetTarget.has_visible_if = true;
        magnetTarget.visible_if.op = "eq";
        magnetTarget.visible_if.path = "fractal.view.fractal_type";
        magnetTarget.visible_if.value = "magnet";
        UISchemaOption rippleTarget{"ripple_amplitude", "Ripple Amplitude", ""};
        rippleTarget.has_visible_if = true;
        rippleTarget.visible_if.op = "in";
        rippleTarget.visible_if.path = "fractal.view.fractal_type";
        rippleTarget.visible_if.value = "explaino_all,explaino_ripple";
        animTarget.options = { noneTarget, magnetTarget, rippleTarget };

        view.fractal_type = FractalType::mandelbrot;
        const std::vector<const UISchemaOption*> mandelbrotAnimOptions = ResolveVisibleEnumOptions(animTarget, ctx);
        if (mandelbrotAnimOptions.size() != 1 || mandelbrotAnimOptions[0]->id != "none") {
            std::cerr << "Expected option-level visible_if to hide inapplicable animation targets on Mandelbrot\n";
            return 1;
        }

        view.fractal_type = FractalType::magnet;
        const std::vector<const UISchemaOption*> magnetAnimOptions = ResolveVisibleEnumOptions(animTarget, ctx);
        if (magnetAnimOptions.size() != 2 || magnetAnimOptions[1]->id != "magnet_relaxation") {
            std::cerr << "Expected option-level visible_if to expose Magnet animation targets only on Magnet\n";
            return 1;
        }

        view.fractal_type = FractalType::explaino_ripple;
        const std::vector<const UISchemaOption*> rippleAnimOptions = ResolveVisibleEnumOptions(animTarget, ctx);
        if (rippleAnimOptions.size() != 2 || rippleAnimOptions[1]->id != "ripple_amplitude") {
            std::cerr << "Expected option-level visible_if to expose owner Explaino axis animation targets on owner lanes\n";
            return 1;
        }

        UISchemaControl colorSignal = MakeBoundControl("color_signal", "combo", "Color Signal", "enum", "param", "fractal.params.color_signal");
        colorSignal.options = {
            {"root_index", "Root Index", ""},
            {"iteration_count", "Iteration Count", ""},
            {"smooth_escape", "Smooth Escape", ""},
            {"phase_angle", "Phase Angle", ""},
            {"iteration_bands", "Iteration Bands", ""},
            {"escape_magnitude", "Escape Magnitude", ""},
            {"orbit_stripe", "Orbit Stripe", ""},
            {"root_proximity", "Root Proximity", ""},
        };

        view.fractal_type = FractalType::explaino;
        const std::vector<const UISchemaOption*> explainoSignalOptions = ResolveVisibleEnumOptions(colorSignal, ctx);
        if (explainoSignalOptions.size() != 8) {
            std::cerr << "Expected basin-capable fractals to expose the widened split color signal set including root_proximity\n";
            return 1;
        }
        bool foundEscapeMagnitude = false;
        bool foundOrbitStripe = false;
        bool foundRootProximity = false;
        for (const UISchemaOption* option : explainoSignalOptions) {
            if (!option) {
                std::cerr << "Expected widened split color signal options to remain addressable\n";
                return 1;
            }
            if (option->id == "escape_magnitude") foundEscapeMagnitude = true;
            if (option->id == "orbit_stripe") foundOrbitStripe = true;
            if (option->id == "root_proximity") foundRootProximity = true;
        }
        if (!foundEscapeMagnitude || !foundOrbitStripe || !foundRootProximity) {
            std::cerr << "Expected basin-capable fractals to expose all widened split color source ids\n";
            return 1;
        }

        view.fractal_type = FractalType::mandelbrot;
        const std::vector<const UISchemaOption*> mandelbrotSignalOptions = ResolveVisibleEnumOptions(colorSignal, ctx);
        if (mandelbrotSignalOptions.size() != 6) {
            std::cerr << "Expected escape-time fractals to expose the widened reusable split color signals while still hiding basin-only ones\n";
            return 1;
        }
        for (const UISchemaOption* option : mandelbrotSignalOptions) {
            if (!option) {
                std::cerr << "Expected visible split color signal options to remain addressable\n";
                return 1;
            }
            if (option->id == "root_index" || option->id == "root_proximity") {
                std::cerr << "Escape-time fractals should not expose basin-only split color signals\n";
                return 1;
            }
        }
    }

    {
        UISchemaControl centerX = MakeBoundControl("center_x", "drag_float", "Center X", "float", "param", "fractal.view.center.x");
        centerX.has_ui_min = true;
        centerX.ui_min = -2.0;
        centerX.has_ui_max = true;
        centerX.ui_max = 2.0;

        NumericControlRange centerRange = ResolveNumericControlRange(centerX);
        if (!centerRange.has_widget_min || !centerRange.has_widget_max ||
            centerRange.widget_min != -2.0 || centerRange.widget_max != 2.0) {
            std::cerr << "Expected drag controls to expose explicit widget ranges from ui_min/ui_max\n";
            return 1;
        }
        if (centerRange.has_hard_min || centerRange.has_hard_max) {
            std::cerr << "Expected ui_min/ui_max-only drag controls to remain unclamped\n";
            return 1;
        }
        NumericDragWidgetBounds centerDragBounds = ResolveNumericDragWidgetBounds(centerX);
        if (centerDragBounds.has_bounds) {
            std::cerr << "Expected ui-only camera center drags to avoid in-widget clamp bounds\n";
            return 1;
        }

        UISchemaControl zoom = MakeBoundControl("zoom", "drag_float", "Zoom", "float", "param", "fractal.view.zoom");
        zoom.has_min = true;
        zoom.min = 1.0e-12;

        NumericControlRange zoomRange = ResolveNumericControlRange(zoom);
        if (zoomRange.has_widget_max ||
            !zoomRange.has_hard_min || zoomRange.hard_min != 1.0e-12 || zoomRange.has_hard_max) {
            std::cerr << "Expected zoom drags to keep their true hard minimum without a UI maximum cap\n";
            return 1;
        }
        NumericDragWidgetBounds zoomDragBounds = ResolveNumericDragWidgetBounds(zoom);
        if (zoomDragBounds.has_bounds) {
            std::cerr << "Expected zoom drags to avoid in-widget clamp bounds when only a one-sided hard limit exists\n";
            return 1;
        }
        UISchemaBinding zoomBoundsBinding;
        zoomBoundsBinding.kind = "param";
        zoomBoundsBinding.path = "fractal.view.zoom";
        NumericDragWidgetBounds cameraZoomDragBounds = ResolveFloatControlDragWidgetBounds(zoom, zoomBoundsBinding);
        if (cameraZoomDragBounds.has_bounds) {
            std::cerr << "Expected dedicated camera zoom drag handling to own widget bounds instead of reusing the generic float-drag clamp path\n";
            return 1;
        }

        UISchemaControl epsilon = MakeBoundControl("epsilon", "slider_float", "Epsilon", "float", "param", "fractal.params.epsilon");
        epsilon.has_min = true;
        epsilon.min = 0.0001;
        epsilon.has_max = true;
        epsilon.max = 1.0;

        NumericControlRange epsilonRange = ResolveNumericControlRange(epsilon);
        if (!epsilonRange.has_widget_min || !epsilonRange.has_widget_max ||
            !epsilonRange.has_hard_min || !epsilonRange.has_hard_max) {
            std::cerr << "Expected slider controls to keep hard min/max as the widget range when no ui_min/ui_max override exists\n";
            return 1;
        }
        if (epsilonRange.widget_min != 0.0001 || epsilonRange.widget_max != 1.0 ||
            epsilonRange.hard_min != 0.0001 || epsilonRange.hard_max != 1.0) {
            std::cerr << "Expected hard min/max to round-trip through numeric range resolution\n";
            return 1;
        }

        UISchemaControl maxIter = MakeBoundControl("max_iter", "slider_int", "Max Iterations", "int", "param", "fractal.params.max_iter");
        maxIter.has_min = true;
        maxIter.min = 1.0;
        maxIter.has_ui_min = true;
        maxIter.ui_min = 1.0;
        maxIter.has_ui_max = true;
        maxIter.ui_max = 5000.0;

        NumericControlRange maxIterRange = ResolveNumericControlRange(maxIter);
        if (!maxIterRange.has_widget_min || !maxIterRange.has_widget_max ||
            !maxIterRange.has_hard_min || maxIterRange.has_hard_max) {
            std::cerr << "Expected mixed int ranges to keep a hard minimum with a UI-only maximum\n";
            return 1;
        }
        if (maxIterRange.widget_min != 1.0 || maxIterRange.widget_max != 5000.0 ||
            maxIterRange.hard_min != 1.0) {
            std::cerr << "Expected max_iter range resolution to preserve the runtime minimum and UI-only slider cap\n";
            return 1;
        }

        UISchemaControl explainoSeed = MakeBoundControl("explaino_seed", "slider_double", "Explaino Seed", "double", "param", "fractal.params.explaino_seed");
        explainoSeed.has_ui_min = true;
        explainoSeed.ui_min = -10.0;
        explainoSeed.has_ui_max = true;
        explainoSeed.ui_max = 10.0;

        NumericControlRange explainoSeedRange = ResolveNumericControlRange(explainoSeed);
        if (!explainoSeedRange.has_widget_min || !explainoSeedRange.has_widget_max ||
            explainoSeedRange.has_hard_min || explainoSeedRange.has_hard_max) {
            std::cerr << "Expected explaino seed sliders to keep UI-only bounds with no hard clamp\n";
            return 1;
        }

        UISchemaControl explainoDamping = MakeBoundControl("explaino_damping", "slider_float", "Newton Damping", "float", "param", "fractal.params.explaino_damping");
        explainoDamping.has_ui_min = true;
        explainoDamping.ui_min = 0.01;
        explainoDamping.has_ui_max = true;
        explainoDamping.ui_max = 10.0;

        NumericControlRange explainoDampingRange = ResolveNumericControlRange(explainoDamping);
        if (!explainoDampingRange.has_widget_min || !explainoDampingRange.has_widget_max ||
            explainoDampingRange.widget_min != 0.01 || explainoDampingRange.widget_max != 10.0 ||
            explainoDampingRange.has_hard_min || explainoDampingRange.has_hard_max) {
            std::cerr << "Expected Newton damping sliders to keep their shipped UI span without a hard clamp\n";
            return 1;
        }

        UISchemaControl lambdaReal = MakeBoundControl("lambda_real", "drag_float", "Lambda Real", "float", "param", "fractal.params.lambda_real");
        lambdaReal.has_min = true;
        lambdaReal.min = -4.0;
        lambdaReal.has_max = true;
        lambdaReal.max = 4.0;
        NumericDragWidgetBounds lambdaRealDragBounds = ResolveNumericDragWidgetBounds(lambdaReal);
        if (!lambdaRealDragBounds.has_bounds || lambdaRealDragBounds.min != -4.0 || lambdaRealDragBounds.max != 4.0) {
            std::cerr << "Expected truly clamped drags to keep their bilateral hard bounds in the widget\n";
            return 1;
        }

        UISchemaControl momentumBeta = MakeBoundControl("momentum_beta", "slider_float", "Momentum Beta", "float", "param", "fractal.params.momentum_beta");
        momentumBeta.has_ui_min = true;
        momentumBeta.ui_min = -1.0;
        momentumBeta.has_ui_max = true;
        momentumBeta.ui_max = 1.0;

        NumericControlRange momentumBetaRange = ResolveNumericControlRange(momentumBeta);
        if (!momentumBetaRange.has_widget_min || !momentumBetaRange.has_widget_max ||
            momentumBetaRange.widget_min != -1.0 || momentumBetaRange.widget_max != 1.0 ||
            momentumBetaRange.has_hard_min || momentumBetaRange.has_hard_max) {
            std::cerr << "Expected momentum beta to use a signed UI-only slider range with no hard clamp\n";
            return 1;
        }

        FunctionParamDescriptor phaseOffsetParam = MakeColorPipelineFloatParam(
            "signal.phase_offset",
            "Phase Offset",
            "Rotate the sampled phase before downstream palette work.",
            -3.141592653589793,
            3.141592653589793,
            0.01,
            0.0);
        NumericControlRange phaseOffsetRange = ResolveColorPipelineNumericControlRange(phaseOffsetParam);
        if (!phaseOffsetRange.has_widget_min || !phaseOffsetRange.has_widget_max ||
            !phaseOffsetRange.has_hard_min || !phaseOffsetRange.has_hard_max ||
            phaseOffsetRange.widget_min != -3.141592653589793 ||
            phaseOffsetRange.widget_max != 3.141592653589793) {
            std::cerr << "Expected color-pipeline float params to preserve their widget and hard bounds through the shared numeric range path\n";
            return 1;
        }
        float clampedPhaseOffset = 9.0f;
        ClampColorPipelineNumericValue(&clampedPhaseOffset, phaseOffsetRange);
        if (!NearlyEqual(clampedPhaseOffset, static_cast<float>(phaseOffsetRange.hard_max))) {
            std::cerr << "Expected color-pipeline float params to clamp edits to their declared hard maximum\n";
            return 1;
        }

        FunctionParamDescriptor bandCountParam = MakeColorPipelineIntParam(
            "signal.band_count",
            "Band Count",
            "Choose how many bands to carve out of the escape signal.",
            2,
            24,
            1,
            8);
        NumericControlRange bandCountRange = ResolveColorPipelineNumericControlRange(bandCountParam);
        int clampedBandCount = 64;
        ClampColorPipelineNumericValue(&clampedBandCount, bandCountRange);
        if (clampedBandCount != 24) {
            std::cerr << "Expected color-pipeline int params to clamp edits to their declared hard maximum\n";
            return 1;
        }

        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        view.center_hp_x = 1.25;
        view.center_hp_y = -0.75;
        view.log2_zoom = TestLog2(32.0);
        SyncTestViewUiFromHp(view);

        BindingContext cameraCtx = MakeBindingContext(&view, &params, &render, &lens);

        UISchemaBinding centerXBinding;
        centerXBinding.kind = "param";
        centerXBinding.path = "fractal.view.center.x";

        double displayedValue = 0.0;
        if (!TryGetFloatControlDisplayValue(centerXBinding, cameraCtx, &displayedValue) ||
            !NearlyEqual(displayedValue, 1.25, 1.0e-12)) {
            std::cerr << "Expected camera center controls to read back from the HP center authority\n";
            return 1;
        }

        view.center.x = 999.0f;
        if (!TryGetFloatControlDisplayValue(centerXBinding, cameraCtx, &displayedValue) ||
            !NearlyEqual(displayedValue, 1.25, 1.0e-12)) {
            std::cerr << "Expected camera center controls to ignore stale UI mirror values\n";
            return 1;
        }

        NumericControlRange cameraCenterRange{};
        if (!ApplyFloatControlEdit(centerXBinding, cameraCtx, cameraCenterRange, -2.5) ||
            !NearlyEqual(view.center_hp_x, -2.5, 1.0e-12) ||
            !NearlyEqual(view.center.x, -2.5f, 1.0e-6) ||
            !NearlyEqual(view.center_hp_y, -0.75, 1.0e-12)) {
            std::cerr << "Expected camera center edits to update HP authority and resync the UI mirror\n";
            return 1;
        }

        UISchemaBinding zoomBinding;
        zoomBinding.kind = "param";
        zoomBinding.path = "fractal.view.zoom";

        view.log2_zoom = TestLog2(32.0);
        SyncTestViewUiFromHp(view);
        view.zoom = 0.0f;
        if (!TryGetFloatControlDisplayValue(zoomBinding, cameraCtx, &displayedValue) ||
            !NearlyEqual(displayedValue, 32.0, 1.0e-12)) {
            std::cerr << "Expected camera zoom controls to read back from log2_zoom instead of a stale float mirror\n";
            return 1;
        }

        double zoomDragValue = 0.0;
        if (!TryGetFloatControlDragValue(zoomBinding, cameraCtx, &zoomDragValue) ||
            !NearlyEqual(zoomDragValue, TestLog2(32.0), 1.0e-12)) {
            std::cerr << "Expected live camera zoom drags to operate in log2 zoom space instead of the displayed linear zoom value\n";
            return 1;
        }

        NumericControlRange cameraZoomRange{};
        cameraZoomRange.has_hard_min = true;
        cameraZoomRange.hard_min = 1.0e-12;
        cameraCtx.edited_camera_hp_authority = false;
        if (!ApplyFloatControlDragEdit(zoomBinding, cameraCtx, cameraZoomRange, TestLog2(64.0)) ||
            !NearlyEqual(view.log2_zoom, TestLog2(64.0), 1.0e-12) ||
            !NearlyEqual(TestSafeZoomFromLog2(view.log2_zoom), 64.0, 1.0e-9) ||
            !NearlyEqual(view.zoom, 64.0f, 1.0e-4) ||
            !cameraCtx.edited_camera_hp_authority) {
            std::cerr << "Expected live camera zoom drags to write log2_zoom directly and resync the displayed zoom mirror\n";
            return 1;
        }

        if (!ApplyFloatControlDragEdit(zoomBinding, cameraCtx, cameraZoomRange, TestLog2(1.0e-20)) ||
            TestSafeZoomFromLog2(view.log2_zoom) < 1.0e-12 ||
            view.zoom < 1.0e-12f) {
            std::cerr << "Expected live camera zoom drags to respect the hard zoom floor while staying on the log2 authority path\n";
            return 1;
        }

        if (!ApplyFloatControlEdit(zoomBinding, cameraCtx, cameraZoomRange, 1.0e9) ||
            !NearlyEqual(TestSafeZoomFromLog2(view.log2_zoom), 1.0e9, 1.0) ||
            view.zoom <= 0.0f) {
            std::cerr << "Expected camera zoom edits to update log2_zoom and preserve a positive synced zoom value\n";
            return 1;
        }

        if (!ApplyFloatControlEdit(zoomBinding, cameraCtx, cameraZoomRange, 0.0) ||
            TestSafeZoomFromLog2(view.log2_zoom) < 1.0e-12 ||
            view.zoom < 1.0e-12f) {
            std::cerr << "Expected camera zoom edits to respect only the hard zoom floor when clamped\n";
            return 1;
        }

        view.center_hp_x = 0.123456789012345;
        view.center_hp_y = -0.987654321098765;
        view.log2_zoom = TestLog2(32.0);
        SyncTestViewUiFromHp(view);
        cameraCtx.edited_camera_hp_authority = false;
        const Float2 uiCenterBefore = view.center;
        const float uiZoomBefore = view.zoom;
        const double originalCenterHpX = view.center_hp_x;
        const double originalCenterHpY = view.center_hp_y;
        if (!ApplyFloatControlEdit(zoomBinding, cameraCtx, cameraZoomRange, 1000000.0)) {
            std::cerr << "Expected zoom regression witness to reuse the HP-aware camera zoom edit seam\n";
            return 1;
        }
        if (!cameraCtx.edited_camera_hp_authority) {
            std::cerr << "Expected HP-aware camera edits to mark the binding context so the live controls window can skip stale post-panel resync\n";
            return 1;
        }
        if (ShouldSyncViewHpFromSchemaUiMirrors(cameraCtx, uiCenterBefore, uiZoomBefore)) {
            std::cerr << "Expected live schema camera edits to skip the old post-panel SyncViewHpFromUi round-trip\n";
            return 1;
        }
        ViewState forcedLegacyResync = view;
        forcedLegacyResync.center_hp_x = static_cast<double>(forcedLegacyResync.center.x);
        forcedLegacyResync.center_hp_y = static_cast<double>(forcedLegacyResync.center.y);
        forcedLegacyResync.log2_zoom = TestLog2((std::fmax)(1.0e-30, static_cast<double>(forcedLegacyResync.zoom)));
        if (NearlyEqual(forcedLegacyResync.center_hp_x, originalCenterHpX, 1.0e-12) ||
            NearlyEqual(forcedLegacyResync.center_hp_y, originalCenterHpY, 1.0e-12)) {
            std::cerr << "Expected the legacy post-panel SyncViewHpFromUi path to visibly collapse HP center precision for the regression witness\n";
            return 1;
        }
    }

    {
        ViewState view{};
        bool* boolValue = nullptr;
        BindingContext viewOnly = MakeBindingContext(&view, nullptr, nullptr, nullptr);
        if (!viewOnly.BindBool("fractal.view.auto_refresh", &boolValue) || boolValue != &view.auto_refresh) {
            std::cerr << "Expected view-only bool bindings to succeed without a render context\n";
            return 1;
        }

        RenderSettings render{};
        boolValue = nullptr;
        BindingContext renderOnly = MakeBindingContext(nullptr, nullptr, &render, nullptr);
        if (!renderOnly.BindBool("fractal.render.benchmark", &boolValue) || boolValue != &render.benchmark) {
            std::cerr << "Expected render-only bool bindings to succeed without a view context\n";
            return 1;
        }

        LensSettings lens{};
        boolValue = nullptr;
        BindingContext lensOnly = MakeBindingContext(nullptr, nullptr, nullptr, &lens);
        if (!lensOnly.BindBool("fractal.lens.enabled", &boolValue) || boolValue != &lens.enabled) {
            std::cerr << "Expected lens-only bool bindings to succeed without view/render contexts\n";
            return 1;
        }
        float* lensFloatValue = nullptr;
        if (!lensOnly.BindFloat("fractal.lens.sdf_overlay_opacity", &lensFloatValue) ||
            lensFloatValue != &lens.sdf_overlay_opacity) {
            std::cerr << "Expected lens-only overlay opacity binding to succeed without view/render contexts\n";
            return 1;
        }
        if (lensOnly.GetEnumId("fractal.lens.sdf_overlay_mode") != "off") {
            std::cerr << "Expected lens-only overlay mode enum binding to expose the default off id\n";
            return 1;
        }
        if (!lensOnly.SetEnumId("fractal.lens.sdf_overlay_mode", "field_debug") ||
            lens.sdf_overlay_mode != LensSdfOverlayMode::field_debug) {
            std::cerr << "Expected lens-only overlay mode enum binding to accept field_debug\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        BindingContext ctx = MakeBindingContext(&view, &params, &render, &lens);

        UISchemaPredicate invalidPath{};
        invalidPath.op = "eq";
        invalidPath.path = "fractal.view.not_real";
        invalidPath.value = "explaino";
        if (ctx.EvalVisibleIf(invalidPath)) {
            std::cerr << "Invalid visible_if paths should fail closed\n";
            return 1;
        }

        UISchemaPredicate invalidOp{};
        invalidOp.op = "mystery_op";
        invalidOp.path = "fractal.view.fractal_type";
        invalidOp.value = "explaino";
        if (ctx.EvalVisibleIf(invalidOp)) {
            std::cerr << "Invalid visible_if operators should fail closed\n";
            return 1;
        }

        UISchemaPredicate invalidNumeric{};
        invalidNumeric.op = "gt";
        invalidNumeric.path = "fractal.view.zoom";
        invalidNumeric.value = "not_a_number";
        if (ctx.EvalVisibleIf(invalidNumeric)) {
            std::cerr << "Invalid visible_if numeric values should fail closed\n";
            return 1;
        }
    }

    if (!ValidateGeneratedAllFractalControlInventory()) {
        return 1;
    }

    if (!ValidateVisibleControlMatrix()) {
        return 1;
    }

    if (!ValidateEnumComboEditMatrix()) {
        return 1;
    }

    if (!ValidateFixedAndPresetSurfaceClassifications()) {
        return 1;
    }

    if (!ValidateAnimationTargetVisibilityMirrorsControls()) {
        return 1;
    }

    if (!ValidateNovaAlphaSchemaRange()) {
        return 1;
    }

    {
        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        BindingContext ctx = MakeBindingContext(&view, &params, &render, &lens);
        bool dirty = false;

        UISchemaControl boolControl = MakeBoundControl("auto_refresh", "checkbox", "Continuous Render", "bool", "param", "fractal.view.auto_refresh");
        boolControl.has_default = true;
        boolControl.def = json_min::Value{true};
        if (!ApplySchemaDefaultForControl(boolControl, ctx, &dirty) || !view.auto_refresh || !dirty) {
            std::cerr << "Expected bool defaults to apply through binding context\n";
            return 1;
        }

        dirty = false;
        render.resolution.x = 640;
        UISchemaControl intControl = MakeBoundControl("width", "slider_int", "Width", "int", "param", "fractal.render.resolution.x");
        intControl.has_default = true;
        intControl.def = json_min::Value{2048.0};
        if (!ApplySchemaDefaultForControl(intControl, ctx, &dirty) || render.resolution.x != 2048 || !dirty) {
            std::cerr << "Expected int defaults to apply through binding context\n";
            return 1;
        }

        render.resolution = {2048, 1536};
        int longEdge = 0;
        if (ctx.GetEnumId("fractal.render.resolution.aspect_preset") != "4:3" ||
            !ctx.GetIntValue("fractal.render.resolution.long_edge", longEdge) || longEdge != 2048) {
            std::cerr << "Expected computed resolution bindings to derive 4:3 and long edge 2048 from default resolution\n";
            return 1;
        }
        if (!ctx.SetEnumId("fractal.render.resolution.aspect_preset", "16:9") ||
            render.resolution.x != 2048 || render.resolution.y != 1152) {
            std::cerr << "Expected aspect preset writes to update RenderSettings.resolution without a second persisted source\n";
            return 1;
        }
        UISchemaControl longEdgeDefault = MakeBoundControl("resolution_long_edge", "slider_int", "Long Edge", "int", "param", "fractal.render.resolution.long_edge");
        longEdgeDefault.has_min = true;
        longEdgeDefault.min = 256.0;
        longEdgeDefault.has_max = true;
        longEdgeDefault.max = 4096.0;
        longEdgeDefault.has_default = true;
        longEdgeDefault.def = json_min::Value{1280.0};
        dirty = false;
        if (!ApplySchemaDefaultForControl(longEdgeDefault, ctx, &dirty) ||
            render.resolution.x != 1280 || render.resolution.y != 720 || !dirty) {
            std::cerr << "Expected computed long-edge defaults to preserve the active aspect and write resolution.x/y\n";
            return 1;
        }
        render.resolution = {1000, 700};
        if (ctx.GetEnumId("fractal.render.resolution.aspect_preset") != "custom" ||
            !ctx.GetIntValue("fractal.render.resolution.long_edge", longEdge) || longEdge != 1000) {
            std::cerr << "Expected non-preset dimensions to derive the Custom aspect mode and current long edge\n";
            return 1;
        }

        dirty = false;
        UISchemaControl seedControl = MakeBoundControl("seed", "drag_double", "Seed", "double", "param", "fractal.params.explaino_seed");
        seedControl.has_default = true;
        seedControl.def = json_min::Value{7.25};
        if (!ApplySchemaDefaultForControl(seedControl, ctx, &dirty) || !dirty) {
            std::cerr << "Expected explaino seed defaults to apply through combined-seed binding\n";
            return 1;
        }
        if (ExplainoSeedCombined(view, params) != 7.25) {
            std::cerr << "Expected explaino combined seed to track schema default application\n";
            return 1;
        }

        dirty = false;
        view.center_hp_x = 0.0;
        view.center_hp_y = 0.0;
        view.log2_zoom = 0.0;
        SyncTestViewUiFromHp(view);
        UISchemaControl centerDefault = MakeBoundControl("center_x", "drag_float", "Center X", "float", "param", "fractal.view.center.x");
        centerDefault.has_default = true;
        centerDefault.def = json_min::Value{1.5};
        if (!ApplySchemaDefaultForControl(centerDefault, ctx, &dirty) || !dirty ||
            !NearlyEqual(view.center_hp_x, 1.5, 1.0e-12) || !NearlyEqual(view.center.x, 1.5f, 1.0e-6)) {
            std::cerr << "Expected camera center defaults to update HP authority and the synced UI mirror\n";
            return 1;
        }

        dirty = false;
        UISchemaControl zoomDefault = MakeBoundControl("zoom", "drag_float", "Zoom", "float", "param", "fractal.view.zoom");
        zoomDefault.has_default = true;
        zoomDefault.has_min = true;
        zoomDefault.min = 1.0e-12;
        zoomDefault.def = json_min::Value{1000000.0};
        if (!ApplySchemaDefaultForControl(zoomDefault, ctx, &dirty) || !dirty ||
            !NearlyEqual(TestSafeZoomFromLog2(view.log2_zoom), 1000000.0, 1.0) || view.zoom <= 0.0f) {
            std::cerr << "Expected camera zoom defaults to update log2_zoom and keep the UI mirror synced\n";
            return 1;
        }

        dirty = false;
        UISchemaControl enumControl = MakeBoundControl("fractal_type", "combo", "Fractal Type", "enum", "param", "fractal.view.fractal_type");
        enumControl.has_default = true;
        enumControl.def = json_min::Value{std::string("nova")};
        if (!ApplySchemaDefaultForControl(enumControl, ctx, &dirty) || view.fractal_type != FractalType::nova || !dirty) {
            std::cerr << "Expected enum defaults to apply through binding context\n";
            return 1;
        }
    }

    {
        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        BindingContext ctx = MakeBindingContext(&view, &params, &render, &lens);

        UISchema schema;
        UISchemaPanel panel;
        panel.id = "panel";
        panel.label = "Panel";

        UISchemaControl actionControl = MakeBoundControl("bad_action", "button", "Bad Action", "", "action", "fractal.actions.not_real");
        panel.controls.push_back(actionControl);
        schema.panels.push_back(panel);

        std::string error;
        if (ValidateSchemaBindings(schema, ctx, &error)) {
            std::cerr << "Unknown action bindings should fail validation\n";
            return 1;
        }
        if (error.find("Unknown action binding path") == std::string::npos) {
            std::cerr << "Expected unknown action binding validation error details\n";
            return 1;
        }

        schema.panels.clear();
        panel.controls.clear();
        UISchemaControl enumControl = MakeBoundControl("sample_tier", "combo", "Sample Tier", "enum", "param", "fractal.render.sample_tier");
        enumControl.options = {
            {"tier_auto", "Auto", ""},
            {"fast", "Fast", ""},
            {"standard", "Standard", ""},
        };
        panel.controls.push_back(enumControl);
        schema.panels.push_back(panel);
        error.clear();
        if (!ValidateSchemaBindings(schema, ctx, &error)) {
            std::cerr << "Supported enum binding paths should validate cleanly: " << error << "\n";
            return 1;
        }

        schema.panels.clear();
        panel.controls.clear();
        UISchemaControl invalidVisibleIf = MakeBoundControl("bad_visible_if", "slider_float", "Bad VisibleIf", "float", "param", "fractal.params.exposure");
        invalidVisibleIf.has_visible_if = true;
        invalidVisibleIf.visible_if.op = "eq";
        invalidVisibleIf.visible_if.path = "fractal.view.not_real";
        invalidVisibleIf.visible_if.value = "explaino";
        panel.controls.push_back(invalidVisibleIf);
        schema.panels.push_back(panel);
        error.clear();
        if (ValidateSchemaBindings(schema, ctx, &error)) {
            std::cerr << "Invalid visible_if predicates should fail schema validation\n";
            return 1;
        }
        if (error.find("visible_if") == std::string::npos) {
            std::cerr << "Expected invalid visible_if validation error details\n";
            return 1;
        }

        schema.panels.clear();
        panel.controls.clear();
        UISchemaControl invalidOptionVisibleIf = MakeBoundControl("bad_option_visible_if", "combo", "Bad Option VisibleIf", "enum", "param", "fractal.view.param_anim_target");
        invalidOptionVisibleIf.options = {
            {"none", "None", ""},
            {"bad", "Bad", ""},
        };
        invalidOptionVisibleIf.options[1].has_visible_if = true;
        invalidOptionVisibleIf.options[1].visible_if.op = "eq";
        invalidOptionVisibleIf.options[1].visible_if.path = "fractal.view.not_real";
        invalidOptionVisibleIf.options[1].visible_if.value = "magnet";
        panel.controls.push_back(invalidOptionVisibleIf);
        schema.panels.push_back(panel);
        error.clear();
        if (ValidateSchemaBindings(schema, ctx, &error)) {
            std::cerr << "Invalid option visible_if predicates should fail schema validation\n";
            return 1;
        }
        if (error.find("option: bad_option_visible_if.bad") == std::string::npos) {
            std::cerr << "Expected invalid option visible_if validation error details\n";
            return 1;
        }

        schema.panels.clear();
        panel.controls.clear();
        UISchemaControl invalidIntCombo = MakeBoundControl("bad_int_combo", "combo", "Bad Int Combo", "int", "param", "fractal.render.device_id");
        invalidIntCombo.options = {
            {"0", "Zero", ""},
            {"not_an_int", "Broken", ""},
        };
        panel.controls.push_back(invalidIntCombo);
        schema.panels.push_back(panel);
        error.clear();
        if (ValidateSchemaBindings(schema, ctx, &error)) {
            std::cerr << "Invalid int combo option ids should fail schema validation\n";
            return 1;
        }
        if (error.find("not_an_int") == std::string::npos) {
            std::cerr << "Expected invalid int combo validation error details\n";
            return 1;
        }
    }

    {
        ColorPipelineRenderInteractionState interactionState{};
        NoteColorPipelineInteractionSnapshot(false, false, true, false, &interactionState);
        NoteColorPipelineInteractionSnapshot(false, false, false, false, &interactionState);

        if (!interactionState.has_active_item) {
            std::cerr << "Expected a slider-side active drag to survive the combined slider-plus-input seam so end-of-frame apply stays suppressed while dragging\n";
            return 1;
        }
    }

    {
        const std::vector<ColorPipelineLaneCatalog>& coreCatalogs = color_pipeline_core::GetColorPipelineLaneCatalogs();
        if (coreCatalogs.size() != 4 ||
            coreCatalogs[0].lane_id != std::string("source") ||
            coreCatalogs[1].lane_id != std::string("shape") ||
            coreCatalogs[2].lane_id != std::string("palette") ||
            coreCatalogs[3].lane_id != std::string("grading")) {
            std::cerr << "Expected the extracted advanced color core to own the shipped Source / Shape / Palette / Grading lane catalog\n";
            return 1;
        }
        const ColorPipelineLaneCatalog* coreSourceCatalog = color_pipeline_core::FindColorPipelineLaneCatalog("source");
        if (!coreSourceCatalog ||
            coreSourceCatalog->default_function_id != std::string("smooth_escape_ramp") ||
            coreSourceCatalog->functions.size() != 13 ||
            coreSourceCatalog->functions[0].id != "smooth_escape_ramp" ||
            coreSourceCatalog->functions[1].id != "phase_orbit" ||
            coreSourceCatalog->functions[2].id != "banded_signal" ||
            coreSourceCatalog->functions[3].id != "escape_magnitude" ||
            coreSourceCatalog->functions[4].id != "orbit_stripe" ||
            coreSourceCatalog->functions[5].id != "root_proximity" ||
            coreSourceCatalog->functions[6].id != "root_index" ||
            coreSourceCatalog->functions[7].id != "sdf_signed_distance" ||
            coreSourceCatalog->functions[8].id != "sdf_inside_outside" ||
            coreSourceCatalog->functions[9].id != "sdf_boundary_band" ||
            coreSourceCatalog->functions[10].id != "sdf_normal_angle" ||
            coreSourceCatalog->functions[11].id != "sdf_curvature" ||
            coreSourceCatalog->functions[12].id != "lens_field_v2_distance") {
            std::cerr << "Expected the extracted advanced color core to widen the shipped Source catalog through runtime-real source rows, including root_index and Lens SDF source tuples\n";
            return 1;
        }
        const FunctionDescriptor* coreEscapeMagnitudeDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreSourceCatalog, "escape_magnitude");
        if (!coreEscapeMagnitudeDescriptor ||
            coreEscapeMagnitudeDescriptor->parameters.size() != 3 ||
            coreEscapeMagnitudeDescriptor->parameters[0].path != "signal.magnitude_scale" ||
            coreEscapeMagnitudeDescriptor->parameters[1].path != "signal.magnitude_bias" ||
            coreEscapeMagnitudeDescriptor->parameters[2].path != "signal.blend_weight") {
            std::cerr << "Expected escape_magnitude to carry stable magnitude-scale, magnitude-bias, and blend-weight source parameters\n";
            return 1;
        }
        const FunctionDescriptor* coreOrbitStripeDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreSourceCatalog, "orbit_stripe");
        if (!coreOrbitStripeDescriptor ||
            coreOrbitStripeDescriptor->parameters.size() != 3 ||
            coreOrbitStripeDescriptor->parameters[0].path != "signal.stripe_frequency" ||
            coreOrbitStripeDescriptor->parameters[1].path != "signal.phase_offset" ||
            coreOrbitStripeDescriptor->parameters[2].path != "signal.blend_weight") {
            std::cerr << "Expected orbit_stripe to carry stable stripe-frequency, phase-offset, and blend-weight source parameters\n";
            return 1;
        }
        const FunctionDescriptor* coreRootProximityDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreSourceCatalog, "root_proximity");
        if (!coreRootProximityDescriptor ||
            coreRootProximityDescriptor->parameters.size() != 3 ||
            coreRootProximityDescriptor->parameters[0].path != "signal.proximity_scale" ||
            coreRootProximityDescriptor->parameters[1].path != "signal.proximity_bias" ||
            coreRootProximityDescriptor->parameters[2].path != "signal.blend_weight") {
            std::cerr << "Expected root_proximity to carry stable proximity-scale, proximity-bias, and blend-weight source parameters\n";
            return 1;
        }
        const FunctionDescriptor* coreRootIndexDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreSourceCatalog, "root_index");
        if (!coreRootIndexDescriptor || !coreRootIndexDescriptor->parameters.empty()) {
            std::cerr << "Expected root_index to expose a stable parameterless basin source row\n";
            return 1;
        }
        const ColorPipelineLaneCatalog* corePaletteCatalog = color_pipeline_core::FindColorPipelineLaneCatalog("palette");
        if (!corePaletteCatalog ||
            corePaletteCatalog->default_function_id != std::string("heatmap") ||
            corePaletteCatalog->functions.size() != 6 ||
            corePaletteCatalog->functions[0].id != "heatmap" ||
            corePaletteCatalog->functions[1].id != "phase_wheel_palette" ||
            corePaletteCatalog->functions[2].id != "banded_heatmap" ||
            corePaletteCatalog->functions[3].id != "explaino_cmap" ||
            corePaletteCatalog->functions[4].id != "root_classic_palette" ||
            corePaletteCatalog->functions[5].id != "joy_root_palette") {
            std::cerr << "Expected the extracted advanced color core to widen the shipped Palette catalog with explaino_cmap, root_classic_palette, and joy_root_palette as runtime-real rows\n";
            return 1;
        }
        const FunctionDescriptor* coreExplainoCmapDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*corePaletteCatalog, "explaino_cmap");
        if (!coreExplainoCmapDescriptor ||
            coreExplainoCmapDescriptor->parameters.size() != 5 ||
            coreExplainoCmapDescriptor->parameters[0].path != "palette.seed_scale" ||
            coreExplainoCmapDescriptor->parameters[1].path != "palette.seed_phase" ||
            coreExplainoCmapDescriptor->parameters[2].path != "palette.colorfulness" ||
            coreExplainoCmapDescriptor->parameters[3].path != "palette.blend_weight" ||
            coreExplainoCmapDescriptor->parameters[4].path != "palette.blend_mode") {
            std::cerr << "Expected explaino_cmap to expose stable seed-scale, seed-phase, colorfulness, and Palette blend parameter paths\n";
            return 1;
        }
        const FunctionDescriptor* coreRootClassicDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*corePaletteCatalog, "root_classic_palette");
        if (!coreRootClassicDescriptor || !coreRootClassicDescriptor->parameters.empty()) {
            std::cerr << "Expected root_classic_palette to expose a stable parameterless basin palette row\n";
            return 1;
        }
        const FunctionDescriptor* coreJoyRootDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*corePaletteCatalog, "joy_root_palette");
        if (!coreJoyRootDescriptor || !coreJoyRootDescriptor->parameters.empty()) {
            std::cerr << "Expected joy_root_palette to expose a stable parameterless joy-basins palette row\n";
            return 1;
        }
        const ColorPipelineLaneCatalog* coreShapeCatalog = color_pipeline_core::FindColorPipelineLaneCatalog("shape");
        if (!coreShapeCatalog ||
            coreShapeCatalog->default_function_id != std::string("identity") ||
            coreShapeCatalog->functions.size() != 7 ||
            coreShapeCatalog->functions[0].id != "identity" ||
            coreShapeCatalog->functions[1].id != "offset_scale" ||
            coreShapeCatalog->functions[2].id != "repeat" ||
            coreShapeCatalog->functions[3].id != "posterize" ||
            coreShapeCatalog->functions[4].id != "mirror_repeat" ||
            coreShapeCatalog->functions[5].id != "bias_gain_curve" ||
            coreShapeCatalog->functions[6].id != "smooth_window") {
            std::cerr << "Expected the extracted advanced color core to widen the shipped Shape catalog with smooth_window as the final runtime-real row\n";
            return 1;
        }
        const FunctionDescriptor* coreRepeatDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreShapeCatalog, "repeat");
        if (!coreRepeatDescriptor ||
            coreRepeatDescriptor->parameters.size() != 2 ||
            coreRepeatDescriptor->parameters[0].path != "shape.frequency" ||
            coreRepeatDescriptor->parameters[1].path != "shape.phase") {
            std::cerr << "Expected the extracted advanced color core to preserve repeat parameter ordering and meaning\n";
            return 1;
        }
        const FunctionDescriptor* corePosterizeDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreShapeCatalog, "posterize");
        if (!corePosterizeDescriptor ||
            corePosterizeDescriptor->parameters.size() != 2 ||
            corePosterizeDescriptor->parameters[0].path != "shape.steps" ||
            corePosterizeDescriptor->parameters[1].path != "shape.mix") {
            std::cerr << "Expected posterize to expose stable steps and mix parameter paths\n";
            return 1;
        }
        const FunctionDescriptor* coreMirrorRepeatDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreShapeCatalog, "mirror_repeat");
        if (!coreMirrorRepeatDescriptor ||
            coreMirrorRepeatDescriptor->parameters.size() != 2 ||
            coreMirrorRepeatDescriptor->parameters[0].path != "shape.frequency" ||
            coreMirrorRepeatDescriptor->parameters[1].path != "shape.phase") {
            std::cerr << "Expected mirror_repeat to reuse the stable repeat frequency and phase parameter paths\n";
            return 1;
        }
        const FunctionDescriptor* coreBiasGainDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreShapeCatalog, "bias_gain_curve");
        if (!coreBiasGainDescriptor ||
            coreBiasGainDescriptor->parameters.size() != 2 ||
            coreBiasGainDescriptor->parameters[0].path != "shape.bias" ||
            coreBiasGainDescriptor->parameters[1].path != "shape.gain") {
            std::cerr << "Expected bias_gain_curve to expose stable bias and gain parameter paths\n";
            return 1;
        }
        const FunctionDescriptor* coreSmoothWindowDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreShapeCatalog, "smooth_window");
        if (!coreSmoothWindowDescriptor ||
            coreSmoothWindowDescriptor->parameters.size() != 3 ||
            coreSmoothWindowDescriptor->parameters[0].path != "shape.center" ||
            coreSmoothWindowDescriptor->parameters[1].path != "shape.width" ||
            coreSmoothWindowDescriptor->parameters[2].path != "shape.softness") {
            std::cerr << "Expected smooth_window to expose stable center, width, and softness parameter paths\n";
            return 1;
        }
        const ColorPipelineLaneCatalog* coreGradingCatalog = color_pipeline_core::FindColorPipelineLaneCatalog("grading");
        if (!coreGradingCatalog ||
            coreGradingCatalog->default_function_id != std::string("contrast_lift") ||
            coreGradingCatalog->functions.size() != 8 ||
            coreGradingCatalog->functions[0].id != "contrast_lift" ||
            coreGradingCatalog->functions[1].id != "phase_finish" ||
            coreGradingCatalog->functions[2].id != "band_finish" ||
            coreGradingCatalog->functions[3].id != "basin_default" ||
            coreGradingCatalog->functions[4].id != "neutral_finish" ||
            coreGradingCatalog->functions[5].id != "tone_map_finish" ||
            coreGradingCatalog->functions[6].id != "grade_glow" ||
            coreGradingCatalog->functions[7].id != "balance_void_grade") {
            std::cerr << "Expected the extracted advanced color core to ship contrast_lift, phase_finish, band_finish, basin_default, neutral_finish, tone_map_finish, grade_glow, and balance_void_grade as runtime-real Grading rows\n";
            return 1;
        }
        const FunctionDescriptor* coreContrastLiftDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreGradingCatalog, "contrast_lift");
        if (!coreContrastLiftDescriptor ||
            coreContrastLiftDescriptor->parameters.size() != 2 ||
            coreContrastLiftDescriptor->parameters[0].path != "grade.exposure" ||
            coreContrastLiftDescriptor->parameters[1].path != "grade.saturation") {
            std::cerr << "Expected contrast_lift to expose stable exposure and saturation grading parameter paths\n";
            return 1;
        }
        const FunctionDescriptor* corePhaseFinishDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreGradingCatalog, "phase_finish");
        if (!corePhaseFinishDescriptor ||
            corePhaseFinishDescriptor->parameters.size() != 2 ||
            corePhaseFinishDescriptor->parameters[0].path != "grade.saturation" ||
            corePhaseFinishDescriptor->parameters[1].path != "grade.contrast") {
            std::cerr << "Expected phase_finish to expose stable saturation and contrast grading parameter paths\n";
            return 1;
        }
        const FunctionDescriptor* coreBandFinishDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreGradingCatalog, "band_finish");
        if (!coreBandFinishDescriptor ||
            coreBandFinishDescriptor->parameters.size() != 2 ||
            coreBandFinishDescriptor->parameters[0].path != "grade.saturation" ||
            coreBandFinishDescriptor->parameters[1].path != "grade.contrast") {
            std::cerr << "Expected band_finish to expose only real saturation and contrast grading owner paths\n";
            return 1;
        }
        const FunctionDescriptor* coreBasinDefaultDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreGradingCatalog, "basin_default");
        if (!coreBasinDefaultDescriptor || !coreBasinDefaultDescriptor->parameters.empty()) {
            std::cerr << "Expected basin_default to stay parameterless so the advanced color grading lane does not invent fake basin controls\n";
            return 1;
        }
        const FunctionDescriptor* coreNeutralFinishDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreGradingCatalog, "neutral_finish");
        if (!coreNeutralFinishDescriptor ||
            coreNeutralFinishDescriptor->parameters.size() != 3 ||
            coreNeutralFinishDescriptor->parameters[0].path != "grade.exposure" ||
            coreNeutralFinishDescriptor->parameters[1].path != "grade.saturation" ||
            coreNeutralFinishDescriptor->parameters[2].path != "grade.contrast") {
            std::cerr << "Expected neutral_finish to expose stable exposure, saturation, and contrast grading owner paths\n";
            return 1;
        }
        const FunctionDescriptor* coreToneMapFinishDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreGradingCatalog, "tone_map_finish");
        if (!coreToneMapFinishDescriptor ||
            coreToneMapFinishDescriptor->parameters.size() != 3 ||
            coreToneMapFinishDescriptor->parameters[0].path != "grade.exposure" ||
            coreToneMapFinishDescriptor->parameters[1].path != "grade.saturation" ||
            coreToneMapFinishDescriptor->parameters[2].path != "grade.contrast") {
            std::cerr << "Expected tone_map_finish to expose stable exposure, saturation, and contrast grading owner paths\n";
            return 1;
        }
        const FunctionDescriptor* coreGradeGlowDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreGradingCatalog, "grade_glow");
        if (!coreGradeGlowDescriptor ||
            coreGradeGlowDescriptor->parameters.size() != 4 ||
            coreGradeGlowDescriptor->parameters[0].path != "grade.exposure" ||
            coreGradeGlowDescriptor->parameters[1].path != "grade.saturation" ||
            coreGradeGlowDescriptor->parameters[2].path != "grade.contrast" ||
            coreGradeGlowDescriptor->parameters[3].path != "grade.glow") {
            std::cerr << "Expected grade_glow to expose stable exposure, saturation, contrast, and glow grading owner paths\n";
            return 1;
        }
        const FunctionDescriptor* coreBalanceVoidDescriptor = color_pipeline_core::FindColorPipelineFunctionDescriptor(*coreGradingCatalog, "balance_void_grade");
        if (!coreBalanceVoidDescriptor ||
            coreBalanceVoidDescriptor->parameters.size() != 3 ||
            coreBalanceVoidDescriptor->parameters[0].path != "grade.balance_void" ||
            coreBalanceVoidDescriptor->parameters[1].path != "grade.chroma_tension" ||
            coreBalanceVoidDescriptor->parameters[2].path != "grade.accent_bias") {
            std::cerr << "Expected balance_void_grade to expose dedicated balance_void, chroma_tension, and accent_bias grading owner paths\n";
            return 1;
        }
        const char* bridgeSourceFunctionId = nullptr;
        const char* bridgePaletteFunctionId = nullptr;
        const ColorPipelineSelection bandsPipeline = {
            ColorSignal::iteration_bands,
            ColorPalette::banded_escape,
            ColorGradingPreset::bands_default,
        };
        if (!color_pipeline_core::TryBuildColorPipelineScheduleBridgeIds(
                bandsPipeline,
                &bridgeSourceFunctionId,
                &bridgePaletteFunctionId) ||
            std::string(bridgeSourceFunctionId ? bridgeSourceFunctionId : "") != "banded_signal" ||
            std::string(bridgePaletteFunctionId ? bridgePaletteFunctionId : "") != "banded_heatmap") {
            std::cerr << "Expected the extracted advanced color core to preserve the shipped schedule bridge ids for runtime-backed tuples\n";
            return 1;
        }
        const ColorPipelineSelection explainoPipeline = {
            ColorSignal::smooth_escape,
            ColorPalette::explaino_cmap,
            ColorGradingPreset::escape_default,
        };
        if (!color_pipeline_core::TryBuildColorPipelineScheduleBridgeIds(
                explainoPipeline,
                &bridgeSourceFunctionId,
                &bridgePaletteFunctionId) ||
            std::string(bridgeSourceFunctionId ? bridgeSourceFunctionId : "") != "smooth_escape_ramp" ||
            std::string(bridgePaletteFunctionId ? bridgePaletteFunctionId : "") != "explaino_cmap") {
            std::cerr << "Expected the extracted advanced color core to bridge explaino_cmap through the smooth-escape runtime tuple\n";
            return 1;
        }
        const ColorPipelineSelection rootClassicPipeline = {
            ColorSignal::root_index,
            ColorPalette::root_classic,
            ColorGradingPreset::basin_default,
        };
        if (!color_pipeline_core::TryBuildColorPipelineScheduleBridgeIds(
                rootClassicPipeline,
                &bridgeSourceFunctionId,
                &bridgePaletteFunctionId) ||
            std::string(bridgeSourceFunctionId ? bridgeSourceFunctionId : "") != "root_index" ||
            std::string(bridgePaletteFunctionId ? bridgePaletteFunctionId : "") != "root_classic_palette") {
            std::cerr << "Expected the extracted advanced color core to bridge the default basin runtime tuple through root_index and root_classic_palette\n";
            return 1;
        }
        const ColorPipelineSelection joyRootPipeline = {
            ColorSignal::root_index,
            ColorPalette::joy,
            ColorGradingPreset::basin_default,
        };
        if (!color_pipeline_core::TryBuildColorPipelineScheduleBridgeIds(
                joyRootPipeline,
                &bridgeSourceFunctionId,
                &bridgePaletteFunctionId) ||
            std::string(bridgeSourceFunctionId ? bridgeSourceFunctionId : "") != "root_index" ||
            std::string(bridgePaletteFunctionId ? bridgePaletteFunctionId : "") != "joy_root_palette") {
            std::cerr << "Expected the extracted advanced color core to bridge the joy-basins runtime tuple through root_index and joy_root_palette\n";
            return 1;
        }

        ColorPipelineSelection rebuiltSelection{};
        ColoringMode rebuiltMode = ColoringMode::root_basin;
        if (!color_pipeline_core::TryBuildColorPipelineSelectionFromLaneIds(
                "escape_magnitude",
                "explaino_cmap",
                &rebuiltSelection,
                &rebuiltMode) ||
            rebuiltSelection.signal != ColorSignal::escape_magnitude ||
            rebuiltSelection.palette != ColorPalette::explaino_cmap ||
            rebuiltSelection.grading != ColorGradingPreset::escape_default ||
            rebuiltMode != ColoringMode::smooth_escape) {
            std::cerr << "Expected the reusable core to rebuild the shipped escape_magnitude + explaino_cmap tuple without window-owned logic\n";
            return 1;
        }

        const ColorPipelineLaneCatalog* sourceLaneCatalog = FindColorPipelineLaneCatalog("source");
        const ColorPipelineLaneCatalog* paletteLaneCatalog = FindColorPipelineLaneCatalog("palette");
        if (!sourceLaneCatalog || !paletteLaneCatalog) {
            std::cerr << "Expected Source and Palette lane catalogs to stay discoverable for core-owned authority extraction tests\n";
            return 1;
        }

        ColorPipelineRowState phaseOrbitRow;
        if (!color_pipeline_core::BuildColorPipelineRowFromFunctionId(
            *sourceLaneCatalog,
                "phase_orbit",
                17,
                &phaseOrbitRow)) {
            std::cerr << "Expected the reusable core to build a phase_orbit row from catalog metadata\n";
            return 1;
        }
        KernelParams importedSourceParams{};
        importedSourceParams.color_phase_signal_offset = 0.75f;
        importedSourceParams.color_phase_wrap_cycles = 2.5f;
        if (!color_pipeline_core::ImportSupportedColorPipelineParamsFromLive(&phaseOrbitRow, importedSourceParams)) {
            std::cerr << "Expected the reusable core to import phase_orbit live params into a schedule row\n";
            return 1;
        }
        double importedPhaseOffset = 0.0;
        double importedWrapCycles = 0.0;
        if (!color_pipeline_core::TryGetColorPipelineParamNumber(phaseOrbitRow, "signal.phase_offset", &importedPhaseOffset) ||
            !color_pipeline_core::TryGetColorPipelineParamNumber(phaseOrbitRow, "signal.wrap_cycles", &importedWrapCycles) ||
            std::fabs(importedPhaseOffset - 0.75) > 1.0e-9 ||
            std::fabs(importedWrapCycles - 2.5) > 1.0e-9) {
            std::cerr << "Expected phase_orbit row params to round-trip through the reusable core import helper\n";
            return 1;
        }

        ColorPipelineRowState explainoPaletteRow;
        if (!color_pipeline_core::BuildColorPipelineRowFromFunctionId(
            *paletteLaneCatalog,
                "explaino_cmap",
                19,
                &explainoPaletteRow) ||
            !color_pipeline_core::SetColorPipelineParamNumber(&explainoPaletteRow, "palette.seed_scale", 1.5) ||
            !color_pipeline_core::SetColorPipelineParamNumber(&explainoPaletteRow, "palette.seed_phase", 0.25) ||
            !color_pipeline_core::SetColorPipelineParamNumber(&explainoPaletteRow, "palette.colorfulness", 0.6)) {
            std::cerr << "Expected the reusable core to build and edit explaino_cmap row params without window-owned helpers\n";
            return 1;
        }
        KernelParams appliedPaletteParams{};
        appliedPaletteParams.color_heatmap_cycle_scale = 3.0f;
        appliedPaletteParams.color_heatmap_saturation = 0.2f;
        appliedPaletteParams.color_phase_palette_offset = 1.0f;
        appliedPaletteParams.color_iteration_band_emphasis = 0.3f;
        appliedPaletteParams.color_iteration_band_palette_offset = 0.4f;
        bool paletteRowChanged = false;
        if (!color_pipeline_core::ApplySupportedColorPipelineRowParamsToLive(
                explainoPaletteRow,
                &appliedPaletteParams,
                &paletteRowChanged) ||
            !paletteRowChanged ||
            std::fabs(appliedPaletteParams.color_explaino_palette_seed_scale - 1.5f) > 1.0e-6f ||
            std::fabs(appliedPaletteParams.color_explaino_palette_seed_phase - 0.25f) > 1.0e-6f ||
            std::fabs(appliedPaletteParams.color_explaino_palette_colorfulness - 0.6f) > 1.0e-6f ||
            std::fabs(appliedPaletteParams.color_heatmap_cycle_scale - 1.0f) > 1.0e-6f ||
            std::fabs(appliedPaletteParams.color_heatmap_saturation - 1.0f) > 1.0e-6f ||
            std::fabs(appliedPaletteParams.color_phase_palette_offset - 0.0f) > 1.0e-6f ||
            std::fabs(appliedPaletteParams.color_iteration_band_emphasis - 1.0f) > 1.0e-6f ||
            std::fabs(appliedPaletteParams.color_iteration_band_palette_offset - 0.0f) > 1.0e-6f) {
            std::cerr << "Expected the reusable core to own explaino_cmap apply/reset behavior for the shipped palette tuple\n";
            return 1;
        }

        ViewState view{};
        KernelParams params{};
        ColorPipelineWindowState windowState{};
        if (windowState.open) {
            std::cerr << "Expected advanced color pipeline windows to start closed\n";
            return 1;
        }
        OpenColorPipelineWindow(&windowState);
        if (!windowState.open) {
            std::cerr << "Expected the advanced color pipeline helper to open the window state\n";
            return 1;
        }
        if (!EnsureColorPipelineWindowInitialized(&windowState)) {
            std::cerr << "Expected the advanced color pipeline window to initialize a fixed draft editor state\n";
            return 1;
        }
        if (!windowState.initialized || windowState.lanes.size() != 4 || windowState.next_row_id != 5) {
            std::cerr << "Expected the advanced color pipeline window to initialize exactly four schedule lanes with stable row ids\n";
            return 1;
        }
        if (windowState.lanes[0].label != "Source" ||
            windowState.lanes[0].rows.size() != 1 ||
            windowState.lanes[0].rows[0].ui_row_id != 1 ||
            windowState.lanes[1].label != "Shape" ||
            windowState.lanes[1].rows.size() != 1 ||
            windowState.lanes[1].rows[0].ui_row_id != 2 ||
            windowState.lanes[2].label != "Palette" ||
            windowState.lanes[2].rows.size() != 1 ||
            windowState.lanes[2].rows[0].ui_row_id != 3 ||
            windowState.lanes[3].label != "Grading" ||
            windowState.lanes[3].rows.size() != 1 ||
            windowState.lanes[3].rows[0].ui_row_id != 4) {
            std::cerr << "Expected the advanced color pipeline draft lanes to keep deterministic Source / Shape / Palette / Grading starter rows\n";
            return 1;
        }
        if (windowState.lanes[0].rows[0].function_id != "smooth_escape_ramp" ||
            windowState.lanes[1].rows[0].function_id != "identity" ||
            windowState.lanes[2].rows[0].function_id != "heatmap" ||
            windowState.lanes[3].rows[0].function_id != "contrast_lift") {
            std::cerr << "Expected the advanced color pipeline draft editor to start from Source / Shape / Palette / Grading starter rows instead of a fixed legacy trio\n";
            return 1;
        }

        std::vector<std::size_t> visibleParamIndexes;
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[0].rows[0], &visibleParamIndexes) ||
            visibleParamIndexes.size() != 3 ||
            visibleParamIndexes[0] != 0 ||
            visibleParamIndexes[1] != 1 ||
            visibleParamIndexes[2] != 2) {
            std::cerr << "Expected the shipped smooth_escape_ramp signal to expose its runtime-backed parameter controls including blend weight\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 0, "root_index") ||
            !windowState.lanes[0].rows[0].parameter_values.empty()) {
            std::cerr << "Expected the shipped Source lane to accept a parameterless root_index row once basin import is supported\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[0].rows[0], &visibleParamIndexes) ||
            !visibleParamIndexes.empty()) {
            std::cerr << "Expected root_index to stay parameterless in the live Source lane\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 0, "smooth_escape_ramp")) {
            std::cerr << "Expected the Source lane to switch back to smooth_escape_ramp after root_index coverage\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[1].rows[0], &visibleParamIndexes) ||
            !visibleParamIndexes.empty()) {
            std::cerr << "Expected the starter Identity shape row to stay out of the current live-runtime-backed control surface\n";
            return 1;
        }
        const ColorPipelineLaneCatalog* shapeCatalog = FindColorPipelineLaneCatalog("shape");
        if (!shapeCatalog ||
            shapeCatalog->functions.size() != 7 ||
            shapeCatalog->functions[0].id != "identity" ||
            shapeCatalog->functions[1].id != "offset_scale" ||
            shapeCatalog->functions[2].id != "repeat" ||
            shapeCatalog->functions[3].id != "posterize" ||
            shapeCatalog->functions[4].id != "mirror_repeat" ||
            shapeCatalog->functions[5].id != "bias_gain_curve" ||
            shapeCatalog->functions[6].id != "smooth_window") {
            std::cerr << "Expected the shipped Shape catalog to expose Identity plus the real offset_scale, repeat, posterize, mirror_repeat, bias_gain_curve, and smooth_window rows\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 1, "offset_scale") ||
            windowState.lanes[1].rows[0].parameter_values.size() != 2 ||
            windowState.lanes[1].rows[0].parameter_values[0].path != "shape.offset" ||
            !CollectRenderableColorPipelineParamIndexes(windowState.lanes[1].rows[0], &visibleParamIndexes) ||
            visibleParamIndexes.size() != 2 ||
            visibleParamIndexes[0] != 0 ||
            visibleParamIndexes[1] != 1) {
            std::cerr << "Expected offset_scale to expose its runtime-backed Shape parameter controls\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 1, "identity")) {
            std::cerr << "Expected the Shape lane to switch back to Identity after offset_scale coverage\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 1, "repeat")) {
            std::cerr << "Expected the shipped Shape lane to allow selecting repeat once it is runtime-backed\n";
            return 1;
        }
        if (windowState.lanes[1].rows[0].parameter_values.size() != 2) {
            std::cerr << "Expected repeat to initialize two Shape parameters\n";
            return 1;
        }
        if (windowState.lanes[1].rows[0].parameter_values[0].path != "shape.frequency" ||
            windowState.lanes[1].rows[0].parameter_values[1].path != "shape.phase") {
            std::cerr << "Expected repeat to expose frequency and phase parameter paths in order\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[1].rows[0], &visibleParamIndexes)) {
            std::cerr << "Expected repeat renderability collection to succeed\n";
            return 1;
        }
        if (visibleParamIndexes.size() != 2 ||
            visibleParamIndexes[0] != 0 ||
            visibleParamIndexes[1] != 1) {
            std::cerr << "Expected repeat to mark both Shape parameters as live-renderable\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 1, "identity")) {
            std::cerr << "Expected the Shape lane to switch back to Identity after repeat coverage\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[2].rows[0], &visibleParamIndexes) ||
            visibleParamIndexes.size() != 4 ||
            visibleParamIndexes[0] != 0 ||
            visibleParamIndexes[1] != 1 ||
            visibleParamIndexes[2] != 2 ||
            visibleParamIndexes[3] != 3) {
            std::cerr << "Expected the shipped heatmap palette to expose its runtime-backed parameter and blend controls\n";
            return 1;
        }
        const ColorPipelineLaneCatalog* paletteCatalog = FindColorPipelineLaneCatalog("palette");
        if (!paletteCatalog ||
            paletteCatalog->functions.size() != 6 ||
            paletteCatalog->functions[0].id != "heatmap" ||
            paletteCatalog->functions[1].id != "phase_wheel_palette" ||
            paletteCatalog->functions[2].id != "banded_heatmap" ||
            paletteCatalog->functions[3].id != "explaino_cmap" ||
            paletteCatalog->functions[4].id != "root_classic_palette" ||
            paletteCatalog->functions[5].id != "joy_root_palette") {
            std::cerr << "Expected the shipped Palette catalog to expose heatmap, phase_wheel_palette, banded_heatmap, explaino_cmap, root_classic_palette, and joy_root_palette\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 2, "explaino_cmap") ||
            windowState.lanes[2].rows[0].parameter_values.size() != 5 ||
            windowState.lanes[2].rows[0].parameter_values[0].path != "palette.seed_scale" ||
            windowState.lanes[2].rows[0].parameter_values[1].path != "palette.seed_phase" ||
            windowState.lanes[2].rows[0].parameter_values[2].path != "palette.colorfulness" ||
            windowState.lanes[2].rows[0].parameter_values[3].path != "palette.blend_weight" ||
            windowState.lanes[2].rows[0].parameter_values[4].path != "palette.blend_mode") {
            std::cerr << "Expected the shipped Palette lane to accept explaino_cmap with runtime blend controls once its backend exists\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[2].rows[0], &visibleParamIndexes) ||
            visibleParamIndexes.size() != 5 ||
            visibleParamIndexes[0] != 0 ||
            visibleParamIndexes[1] != 1 ||
            visibleParamIndexes[2] != 2 ||
            visibleParamIndexes[3] != 3 ||
            visibleParamIndexes[4] != 4) {
            std::cerr << "Expected explaino_cmap to expose its live Palette and blend controls\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 2, "root_classic_palette") ||
            !windowState.lanes[2].rows[0].parameter_values.empty()) {
            std::cerr << "Expected the shipped Palette lane to accept a parameterless root_classic_palette row once basin import is supported\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[2].rows[0], &visibleParamIndexes) ||
            !visibleParamIndexes.empty()) {
            std::cerr << "Expected root_classic_palette to stay parameterless in the live Palette lane\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 2, "joy_root_palette") ||
            !windowState.lanes[2].rows[0].parameter_values.empty()) {
            std::cerr << "Expected the shipped Palette lane to accept a parameterless joy_root_palette row once joy-basins import is supported\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[2].rows[0], &visibleParamIndexes) ||
            !visibleParamIndexes.empty()) {
            std::cerr << "Expected joy_root_palette to stay parameterless in the live Palette lane\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 2, "explaino_cmap")) {
            std::cerr << "Expected the Palette lane to switch back to explaino_cmap before exercising schedule-style row editing\n";
            return 1;
        }
        if (!AddColorPipelineLaneRow(&windowState, 2, "explaino_cmap") ||
            windowState.lanes[2].rows.size() != 2 ||
            windowState.lanes[2].rows[1].function_id != "explaino_cmap") {
            std::cerr << "Expected the schedule-style Palette lane to support appending explaino_cmap rows once runtime-backed\n";
            return 1;
        }
        if (!RemoveColorPipelineLaneRow(&windowState, 2, 1) ||
            windowState.lanes[2].rows.size() != 1 ||
            windowState.lanes[2].rows[0].function_id != "explaino_cmap") {
            std::cerr << "Expected explaino_cmap rows to participate in the same schedule-style Palette row editing surface\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 2, "heatmap")) {
            std::cerr << "Expected the Palette lane to switch back to heatmap after explaino_cmap coverage\n";
            return 1;
        }

        if (SelectColorPipelineLaneFunction(&windowState, 0, "phase_angle")) {
            std::cerr << "Expected the raw legacy enum ids to stop working as advanced color function ids\n";
            return 1;
        }
        if (SelectColorPipelineLaneFunction(&windowState, 0, "basin_index") ||
            SelectColorPipelineLaneFunction(&windowState, 1, "classic_basin_palette") ||
            SelectColorPipelineLaneFunction(&windowState, 2, "phase_finish") ||
            SelectColorPipelineLaneFunction(&windowState, 2, "basin_balance")) {
            std::cerr << "Expected the advanced color pipeline catalog to stop shipping the legacy simple-panel tuple pieces\n";
            return 1;
        }

        if (!SelectColorPipelineLaneFunction(&windowState, 0, "banded_signal")) {
            std::cerr << "Expected advanced color pipeline lane function selection to accept known lane-local functions\n";
            return 1;
        }
        if (windowState.lanes[0].rows[0].function_id != "banded_signal" ||
            windowState.lanes[0].rows[0].parameter_values.size() != 3 ||
            windowState.lanes[0].rows[0].parameter_values[0].path != "signal.band_count" ||
            windowState.lanes[0].rows[0].parameter_values[2].path != "signal.blend_weight") {
            std::cerr << "Expected advanced color pipeline lane edits to swap the live-backed descriptor parameter surface including Source blend weight\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[0].rows[0], &visibleParamIndexes) ||
            visibleParamIndexes.size() != 3 ||
            visibleParamIndexes[0] != 0 ||
            visibleParamIndexes[1] != 1 ||
            visibleParamIndexes[2] != 2) {
            std::cerr << "Expected runtime-backed advanced color functions to expose only their real renderable parameter controls including Source blend weight\n";
            return 1;
        }
        if (!AddColorPipelineLaneRow(&windowState, 1, "offset_scale") ||
            windowState.lanes[1].rows.size() != 2 ||
            windowState.lanes[1].rows[1].function_id != "offset_scale" ||
            windowState.lanes[1].rows[1].ui_row_id != 6) {
            std::cerr << "Expected the schedule-style Shape lane to support appending a second shipped row with a stable row id\n";
            return 1;
        }
        if (!MoveColorPipelineLaneRow(&windowState, 1, 1, -1) ||
            windowState.lanes[1].rows[0].function_id != "offset_scale" ||
            windowState.lanes[1].rows[1].function_id != "identity") {
            std::cerr << "Expected schedule-style lane rows to support reorder operations\n";
            return 1;
        }
        if (!RemoveColorPipelineLaneRow(&windowState, 1, 1) ||
            windowState.lanes[1].rows.size() != 1 ||
            windowState.lanes[1].rows[0].function_id != "offset_scale") {
            std::cerr << "Expected schedule-style lane rows to support removing non-last rows\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 1, "repeat") ||
            !AddColorPipelineLaneRow(&windowState, 1, "repeat")) {
            std::cerr << "Expected the shipped Shape lane to accept repeat once the runtime backend supports it\n";
            return 1;
        }
        if (!RemoveColorPipelineLaneRow(&windowState, 1, 1) ||
            windowState.lanes[1].rows.size() != 1 ||
            windowState.lanes[1].rows[0].function_id != "repeat") {
            std::cerr << "Expected repeat rows to participate in the same schedule-style Shape row editing surface\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 1, "posterize") ||
            windowState.lanes[1].rows[0].parameter_values.size() != 2 ||
            windowState.lanes[1].rows[0].parameter_values[0].path != "shape.steps" ||
            windowState.lanes[1].rows[0].parameter_values[1].path != "shape.mix") {
            std::cerr << "Expected the shipped Shape lane to accept posterize once its runtime backend exists\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[1].rows[0], &visibleParamIndexes) ||
            visibleParamIndexes.size() != 2 ||
            visibleParamIndexes[0] != 0 ||
            visibleParamIndexes[1] != 1) {
            std::cerr << "Expected posterize to expose both live Shape controls\n";
            return 1;
        }
        if (!AddColorPipelineLaneRow(&windowState, 1, "posterize") ||
            windowState.lanes[1].rows.size() != 2 ||
            windowState.lanes[1].rows[1].function_id != "posterize") {
            std::cerr << "Expected the schedule-style Shape lane to support appending posterize rows once runtime-backed\n";
            return 1;
        }
        if (!RemoveColorPipelineLaneRow(&windowState, 1, 1) ||
            windowState.lanes[1].rows.size() != 1 ||
            windowState.lanes[1].rows[0].function_id != "posterize") {
            std::cerr << "Expected posterize rows to participate in the same schedule-style Shape row editing surface\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 1, "mirror_repeat") ||
            windowState.lanes[1].rows[0].parameter_values.size() != 2 ||
            windowState.lanes[1].rows[0].parameter_values[0].path != "shape.frequency" ||
            windowState.lanes[1].rows[0].parameter_values[1].path != "shape.phase") {
            std::cerr << "Expected the shipped Shape lane to accept mirror_repeat once its runtime backend exists\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[1].rows[0], &visibleParamIndexes) ||
            visibleParamIndexes.size() != 2 ||
            visibleParamIndexes[0] != 0 ||
            visibleParamIndexes[1] != 1) {
            std::cerr << "Expected mirror_repeat to expose the reused repeat Shape controls\n";
            return 1;
        }
        if (!AddColorPipelineLaneRow(&windowState, 1, "mirror_repeat") ||
            windowState.lanes[1].rows.size() != 2 ||
            windowState.lanes[1].rows[1].function_id != "mirror_repeat") {
            std::cerr << "Expected the schedule-style Shape lane to support appending mirror_repeat rows once runtime-backed\n";
            return 1;
        }
        if (!RemoveColorPipelineLaneRow(&windowState, 1, 1) ||
            windowState.lanes[1].rows.size() != 1 ||
            windowState.lanes[1].rows[0].function_id != "mirror_repeat") {
            std::cerr << "Expected mirror_repeat rows to participate in the same schedule-style Shape row editing surface\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 1, "bias_gain_curve") ||
            windowState.lanes[1].rows[0].parameter_values.size() != 2 ||
            windowState.lanes[1].rows[0].parameter_values[0].path != "shape.bias" ||
            windowState.lanes[1].rows[0].parameter_values[1].path != "shape.gain") {
            std::cerr << "Expected the shipped Shape lane to accept bias_gain_curve once its runtime backend exists\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[1].rows[0], &visibleParamIndexes) ||
            visibleParamIndexes.size() != 2 ||
            visibleParamIndexes[0] != 0 ||
            visibleParamIndexes[1] != 1) {
            std::cerr << "Expected bias_gain_curve to expose both live Shape controls\n";
            return 1;
        }
        if (!AddColorPipelineLaneRow(&windowState, 1, "bias_gain_curve") ||
            windowState.lanes[1].rows.size() != 2 ||
            windowState.lanes[1].rows[1].function_id != "bias_gain_curve") {
            std::cerr << "Expected the schedule-style Shape lane to support appending bias_gain_curve rows once runtime-backed\n";
            return 1;
        }
        if (!RemoveColorPipelineLaneRow(&windowState, 1, 1) ||
            windowState.lanes[1].rows.size() != 1 ||
            windowState.lanes[1].rows[0].function_id != "bias_gain_curve") {
            std::cerr << "Expected bias_gain_curve rows to participate in the same schedule-style Shape row editing surface\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 1, "smooth_window") ||
            windowState.lanes[1].rows[0].parameter_values.size() != 3 ||
            windowState.lanes[1].rows[0].parameter_values[0].path != "shape.center" ||
            windowState.lanes[1].rows[0].parameter_values[1].path != "shape.width" ||
            windowState.lanes[1].rows[0].parameter_values[2].path != "shape.softness") {
            std::cerr << "Expected the shipped Shape lane to accept smooth_window once its runtime backend exists\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[1].rows[0], &visibleParamIndexes) ||
            visibleParamIndexes.size() != 3 ||
            visibleParamIndexes[0] != 0 ||
            visibleParamIndexes[1] != 1 ||
            visibleParamIndexes[2] != 2) {
            std::cerr << "Expected smooth_window to expose its three live Shape controls\n";
            return 1;
        }
        if (!AddColorPipelineLaneRow(&windowState, 1, "smooth_window") ||
            windowState.lanes[1].rows.size() != 2 ||
            windowState.lanes[1].rows[1].function_id != "smooth_window") {
            std::cerr << "Expected the schedule-style Shape lane to support appending smooth_window rows once runtime-backed\n";
            return 1;
        }
        if (!RemoveColorPipelineLaneRow(&windowState, 1, 1) ||
            windowState.lanes[1].rows.size() != 1 ||
            windowState.lanes[1].rows[0].function_id != "smooth_window") {
            std::cerr << "Expected smooth_window rows to participate in the same schedule-style Shape row editing surface\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 1, "identity") ||
            windowState.lanes[1].rows[0].function_id != "identity") {
            std::cerr << "Expected the Shape lane to keep accepting lane-local function changes after reorder/remove coverage\n";
            return 1;
        }
        params.coloring_mode = ColoringMode::smooth_escape;
        params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
        if (!SyncColorPipelineWindowFromLiveState(&windowState, view.fractal_type, &params) ||
            !windowState.live_snapshot.valid ||
            !windowState.live_snapshot.draft_import_supported ||
            windowState.lanes[0].rows[0].function_id != "smooth_escape_ramp" ||
            windowState.lanes[2].rows[0].function_id != "heatmap") {
            std::cerr << "Expected syncing from a supported non-basin live tuple to import the smooth-escape draft before root tuple-switch coverage\n";
            return 1;
        }
        struct ShippedGradingCandidateExpectation {
            const char* functionId;
            const char* failureMessage;
        };
        const ShippedGradingCandidateExpectation shippedGradingCandidates[] = {
            {"neutral_finish", "Expected selecting neutral_finish from a shipped smooth-escape tuple to stay out of draft-only labeling\n"},
            {"tone_map_finish", "Expected selecting tone_map_finish from a shipped smooth-escape tuple to stay out of draft-only labeling\n"},
            {"grade_glow", "Expected selecting grade_glow from a shipped smooth-escape tuple to stay out of draft-only labeling\n"},
            {"balance_void_grade", "Expected selecting balance_void_grade from a shipped smooth-escape tuple to stay out of draft-only labeling\n"},
        };
        for (const ShippedGradingCandidateExpectation& expectation : shippedGradingCandidates) {
            const ColorPipelineDraftApplyState candidateState = DescribeColorPipelineCandidateApplyState(
                windowState,
                3,
                expectation.functionId,
                view.fractal_type,
                &params);
            if (candidateState.status != ColorPipelineDraftApplyStatus::can_apply) {
                std::cerr << expectation.failureMessage;
                return 1;
            }
        }
        ColorPipelineWindowState invalidGradingMessageState = windowState;
        invalidGradingMessageState.lanes[3].rows[0].function_id = "missing_grade";
        const ColorPipelineDraftApplyState invalidGradingMessageApplyState = DescribeColorPipelineDraftApplyState(
            invalidGradingMessageState,
            view.fractal_type,
            &params);
        if (invalidGradingMessageApplyState.status != ColorPipelineDraftApplyStatus::unsupported_tuple ||
            invalidGradingMessageApplyState.message.find("balance_void_grade") == std::string::npos) {
            std::cerr << "Expected unsupported Grading guidance to list balance_void_grade inside the shipped Grading stack boundary\n";
            return 1;
        }
        const ColorPipelineDraftApplyState rootIndexCandidateState = DescribeColorPipelineCandidateApplyState(
            windowState,
            0,
            "root_index",
            view.fractal_type,
            &params);
        if (rootIndexCandidateState.status != ColorPipelineDraftApplyStatus::can_apply) {
            std::cerr << "Expected selecting root_index from a supported single-row editor state to auto-complete the matching Palette row instead of reading as draft-only\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 0, "root_index") ||
            windowState.lanes[0].rows[0].function_id != "root_index" ||
            windowState.lanes[2].rows[0].function_id != "root_classic_palette") {
            std::cerr << "Expected selecting root_index to co-switch the matching root_classic_palette row\n";
            return 1;
        }
        if (!HasColorPipelineDraftEdits(windowState)) {
            std::cerr << "Expected tuple-aware co-switching to count as a real draft edit\n";
            return 1;
        }
        bool rootTupleChanged = false;
        if (!ApplyColorPipelineDraftToLiveState(&windowState, view.fractal_type, &params, &rootTupleChanged) ||
            !rootTupleChanged ||
            params.coloring_mode != ColoringMode::root_basin ||
            params.color_pipeline.signal != ColorSignal::root_index ||
            params.color_pipeline.palette != ColorPalette::root_classic ||
            params.color_pipeline.grading != ColorGradingPreset::basin_default) {
            std::cerr << "Expected applying the tuple-aware root selection to move the live runtime from a non-basin tuple into root_index plus root_classic\n";
            return 1;
        }
        const ColorPipelineDraftApplyState basinDefaultCandidateState = DescribeColorPipelineCandidateApplyState(
            windowState,
            3,
            "basin_default",
            view.fractal_type,
            &params);
        if (basinDefaultCandidateState.status != ColorPipelineDraftApplyStatus::matches_live) {
            std::cerr << "Expected the shipped basin_default row to stop reading as draft-only once the root-basin tuple is aligned\n";
            return 1;
        }
        const ColorPipelineDraftApplyState rootClassicCandidateState = DescribeColorPipelineCandidateApplyState(
            windowState,
            2,
            "root_classic_palette",
            view.fractal_type,
            &params);
        if (rootClassicCandidateState.status != ColorPipelineDraftApplyStatus::matches_live) {
            std::cerr << "Expected the matching root_classic_palette candidate to stop reading as draft-only once the tuple is aligned\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 2, "heatmap") ||
            windowState.lanes[2].rows[0].function_id != "heatmap") {
            std::cerr << "Expected the Palette lane to switch back to heatmap after root_classic tuple coverage\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 2, "root_classic_palette") ||
            windowState.lanes[2].rows[0].function_id != "root_classic_palette" ||
            windowState.lanes[0].rows[0].function_id != "root_index") {
            std::cerr << "Expected selecting root_classic_palette to co-switch the matching root_index row\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 2, "heatmap") ||
            !SelectColorPipelineLaneFunction(&windowState, 0, "smooth_escape_ramp")) {
            std::cerr << "Expected the test to restore the smooth-escape tuple after root_classic co-switch coverage\n";
            return 1;
        }
        params.coloring_mode = ColoringMode::smooth_escape;
        params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
        if (!SyncColorPipelineWindowFromLiveState(&windowState, view.fractal_type, &params) ||
            windowState.lanes[0].rows[0].function_id != "smooth_escape_ramp" ||
            windowState.lanes[2].rows[0].function_id != "heatmap") {
            std::cerr << "Expected syncing from a supported non-basin live tuple to restore the smooth-escape draft before joy_root coverage\n";
            return 1;
        }
        const ColorPipelineDraftApplyState joyRootCandidateState = DescribeColorPipelineCandidateApplyState(
            windowState,
            2,
            "joy_root_palette",
            view.fractal_type,
            &params);
        if (joyRootCandidateState.status != ColorPipelineDraftApplyStatus::can_apply) {
            std::cerr << "Expected selecting joy_root_palette from a supported non-basin editor state to auto-complete root_index instead of reading as draft-only\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 2, "joy_root_palette") ||
            windowState.lanes[2].rows[0].function_id != "joy_root_palette" ||
            windowState.lanes[0].rows[0].function_id != "root_index") {
            std::cerr << "Expected selecting joy_root_palette to co-switch the matching root_index row\n";
            return 1;
        }
        bool joyRootTupleChanged = false;
        if (!ApplyColorPipelineDraftToLiveState(&windowState, view.fractal_type, &params, &joyRootTupleChanged) ||
            !joyRootTupleChanged ||
            params.coloring_mode != ColoringMode::joy_basins ||
            params.color_pipeline.signal != ColorSignal::root_index ||
            params.color_pipeline.palette != ColorPalette::joy ||
            params.color_pipeline.grading != ColorGradingPreset::basin_default) {
            std::cerr << "Expected applying joy_root_palette to move the live runtime from a non-basin tuple into the joy-basins root palette\n";
            return 1;
        }
        const ColorPipelineDraftApplyState joyRootAlignedState = DescribeColorPipelineCandidateApplyState(
            windowState,
            2,
            "joy_root_palette",
            view.fractal_type,
            &params);
        if (joyRootAlignedState.status != ColorPipelineDraftApplyStatus::matches_live) {
            std::cerr << "Expected the matching joy_root_palette candidate to stop reading as draft-only once the joy tuple is aligned\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 2, "heatmap") ||
            !SelectColorPipelineLaneFunction(&windowState, 0, "smooth_escape_ramp")) {
            std::cerr << "Expected the test to restore the smooth-escape tuple after joy_root co-switch coverage\n";
            return 1;
        }
        if (SelectColorPipelineLaneFunction(&windowState, 0, "not_real")) {
            std::cerr << "Unknown advanced color lane functions should fail instead of silently falling back\n";
            return 1;
        }
        if (windowState.lanes[0].rows[0].function_id != "smooth_escape_ramp") {
            std::cerr << "Failed advanced color lane selections should preserve the current function choice\n";
            return 1;
        }

        params.coloring_mode = ColoringMode::root_basin;
        params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::root_basin);
        if (!SyncColorPipelineWindowFromLiveState(&windowState, view.fractal_type, &params)) {
            std::cerr << "Expected root-basin live tuples to sync once root_index and root_classic_palette are part of the advanced bridge\n";
            return 1;
        }
        if (!windowState.live_snapshot.valid ||
            !windowState.live_snapshot.draft_import_supported ||
            windowState.live_snapshot.lanes.size() != 4 ||
            windowState.live_snapshot.lanes[0].rows.size() != 1 ||
            windowState.live_snapshot.lanes[0].rows[0].function_id != "root_index" ||
            !windowState.live_snapshot.lanes[0].rows[0].parameter_values.empty() ||
            windowState.live_snapshot.lanes[2].rows.size() != 1 ||
            windowState.live_snapshot.lanes[2].rows[0].function_id != "root_classic_palette" ||
            !windowState.live_snapshot.lanes[2].rows[0].parameter_values.empty() ||
            windowState.live_snapshot.lanes[3].rows.size() != 1 ||
            windowState.live_snapshot.lanes[3].rows[0].function_id != "basin_default" ||
            !windowState.live_snapshot.lanes[3].rows[0].parameter_values.empty()) {
            std::cerr << "Expected root-basin live tuples to import as a supported root_index plus root_classic_palette and basin_default snapshot\n";
            return 1;
        }
        params.coloring_mode = ColoringMode::joy_basins;
        params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::joy_basins);
        if (!SyncColorPipelineWindowFromLiveState(&windowState, view.fractal_type, &params)) {
            std::cerr << "Expected joy-basins live tuples to sync once joy_root_palette is part of the advanced bridge\n";
            return 1;
        }
        if (!windowState.live_snapshot.valid ||
            !windowState.live_snapshot.draft_import_supported ||
            windowState.live_snapshot.lanes.size() != 4 ||
            windowState.live_snapshot.lanes[0].rows[0].function_id != "root_index" ||
            windowState.live_snapshot.lanes[2].rows[0].function_id != "joy_root_palette" ||
            windowState.live_snapshot.lanes[3].rows.size() != 1 ||
            windowState.live_snapshot.lanes[3].rows[0].function_id != "basin_default" ||
            !windowState.live_snapshot.lanes[3].rows[0].parameter_values.empty()) {
            std::cerr << "Expected joy-basins live tuples to import as a supported root_index plus joy_root_palette and basin_default snapshot\n";
            return 1;
        }

        if (!SelectColorPipelineLaneFunction(&windowState, 0, "phase_orbit") ||
            !SelectColorPipelineLaneFunction(&windowState, 2, "phase_wheel_palette") ||
            !HasColorPipelineDraftEdits(windowState)) {
            std::cerr << "Expected the draft editor to keep diverging independently from the supported live root-basin snapshot\n";
            return 1;
        }

        params.coloring_mode = ColoringMode::smooth_escape;
        params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
        params.color_grading_stack_count = 0;
        if (!SyncColorPipelineWindowFromLiveState(&windowState, view.fractal_type, &params)) {
            std::cerr << "Expected supported live tuples to resync the snapshot without clobbering an already-diverged draft\n";
            return 1;
        }
        if (!windowState.live_snapshot.valid ||
            !windowState.live_snapshot.draft_import_supported ||
            windowState.live_snapshot.lanes.size() != 4 ||
            windowState.live_snapshot.lanes[0].rows[0].function_id != "smooth_escape_ramp" ||
            windowState.live_snapshot.lanes[1].rows[0].function_id != "identity" ||
            windowState.live_snapshot.lanes[2].rows[0].function_id != "heatmap" ||
            windowState.live_snapshot.lanes[3].rows[0].function_id != "contrast_lift") {
            std::cerr << "Expected the live snapshot to refresh to the newly supported runtime tuple\n";
            return 1;
        }
        if (windowState.lanes[0].rows[0].function_id != "phase_orbit" ||
            windowState.lanes[2].rows[0].function_id != "phase_wheel_palette" ||
            !HasColorPipelineDraftEdits(windowState)) {
            std::cerr << "Expected a diverged draft to survive when the live tuple becomes bridge-supported later\n";
            return 1;
        }

        ColorPipelineWindowState stableSyncWindowState{};
        params.coloring_mode = ColoringMode::phase;
        params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::phase);
        params.color_shape = ColorPipelineShape::offset_scale;
        params.color_shape_offset = 0.25f;
        params.color_shape_scale = 1.5f;
        if (!SyncColorPipelineWindowFromLiveState(&stableSyncWindowState, view.fractal_type, &params)) {
            std::cerr << "Expected the advanced color pipeline draft to import a supported live tuple before row-id stability coverage\n";
            return 1;
        }
        const std::uint64_t stableSourceRowId = stableSyncWindowState.lanes[0].rows[0].ui_row_id;
        const std::uint64_t stableShapeRowId = stableSyncWindowState.lanes[1].rows[0].ui_row_id;
        const std::uint64_t stablePaletteRowId = stableSyncWindowState.lanes[2].rows[0].ui_row_id;
        if (!SyncColorPipelineWindowFromLiveState(&stableSyncWindowState, view.fractal_type, &params)) {
            std::cerr << "Expected unchanged supported live tuples to keep syncing successfully during row-id stability coverage\n";
            return 1;
        }
        if (stableSyncWindowState.lanes[0].rows[0].ui_row_id != stableSourceRowId ||
            stableSyncWindowState.lanes[1].rows[0].ui_row_id != stableShapeRowId ||
            stableSyncWindowState.lanes[2].rows[0].ui_row_id != stablePaletteRowId) {
            std::cerr << "Expected unchanged supported live sync to preserve row ids instead of rebuilding the programmable draft between frames\n";
            return 1;
        }

        ColorPipelineWindowState explainoSyncWindowState{};
        params.coloring_mode = ColoringMode::smooth_escape;
        params.color_pipeline = {ColorSignal::smooth_escape, ColorPalette::explaino_cmap, ColorGradingPreset::escape_default};
        params.color_shape = ColorPipelineShape::identity;
        params.color_shape_offset = 0.0f;
        params.color_shape_scale = 1.0f;
        params.color_explaino_palette_seed_scale = 1.5f;
        params.color_explaino_palette_seed_phase = 0.25f;
        params.color_explaino_palette_colorfulness = 0.8f;
        if (!SyncColorPipelineWindowFromLiveState(&explainoSyncWindowState, view.fractal_type, &params)) {
            std::cerr << "Expected the advanced color pipeline draft to import the explaino_cmap runtime tuple once it is supported\n";
            return 1;
        }
        if (!explainoSyncWindowState.live_snapshot.valid ||
            !explainoSyncWindowState.live_snapshot.draft_import_supported ||
            explainoSyncWindowState.live_snapshot.lanes.size() != 4 ||
            explainoSyncWindowState.live_snapshot.lanes[2].rows[0].function_id != "explaino_cmap" ||
            explainoSyncWindowState.live_snapshot.lanes[2].rows[0].parameter_values.size() != 5 ||
            explainoSyncWindowState.live_snapshot.lanes[2].rows[0].parameter_values[0].path != "palette.seed_scale" ||
            explainoSyncWindowState.live_snapshot.lanes[3].rows.size() != 1 ||
            explainoSyncWindowState.live_snapshot.lanes[3].rows[0].function_id != "contrast_lift" ||
            explainoSyncWindowState.live_snapshot.lanes[3].rows[0].parameter_values.size() != 2 ||
            explainoSyncWindowState.live_snapshot.lanes[3].rows[0].parameter_values[0].path != "grade.exposure" ||
            explainoSyncWindowState.live_snapshot.lanes[3].rows[0].parameter_values[1].path != "grade.saturation") {
            std::cerr << "Expected explaino_cmap live sync to import the dedicated Palette owner fields plus the bounded contrast_lift grading lane\n";
            return 1;
        }

        ColorPipelineWindowState phaseSyncWindowState{};
        params.coloring_mode = ColoringMode::phase;
        params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::phase);
        params.color_shape = ColorPipelineShape::offset_scale;
        params.color_shape_stack_count = 0;
        params.color_shape_offset = 0.25f;
        params.color_shape_scale = 1.5f;
        if (!SyncColorPipelineWindowFromLiveState(&phaseSyncWindowState, view.fractal_type, &params)) {
            std::cerr << "Expected coherent phase tuples to import once phase_finish is shipped as a runtime-real grading row\n";
            return 1;
        }
        if (!phaseSyncWindowState.live_snapshot.valid ||
            !phaseSyncWindowState.live_snapshot.draft_import_supported ||
            phaseSyncWindowState.live_snapshot.lanes.size() != 4 ||
            phaseSyncWindowState.live_snapshot.lanes[0].rows[0].function_id != "phase_orbit" ||
            phaseSyncWindowState.live_snapshot.lanes[1].rows[0].function_id != "offset_scale" ||
            phaseSyncWindowState.live_snapshot.lanes[2].rows[0].function_id != "phase_wheel_palette" ||
            phaseSyncWindowState.live_snapshot.lanes[2].rows[0].parameter_values.size() != 4 ||
            phaseSyncWindowState.live_snapshot.lanes[2].rows[0].parameter_values[0].path != "palette.phase_offset" ||
            phaseSyncWindowState.live_snapshot.lanes[2].rows[0].parameter_values[1].path != "palette.saturation" ||
            phaseSyncWindowState.live_snapshot.lanes[2].rows[0].parameter_values[2].path != "palette.blend_weight" ||
            phaseSyncWindowState.live_snapshot.lanes[2].rows[0].parameter_values[3].path != "palette.blend_mode" ||
            phaseSyncWindowState.live_snapshot.lanes[3].rows.size() != 1 ||
            phaseSyncWindowState.live_snapshot.lanes[3].rows[0].function_id != "phase_finish" ||
            phaseSyncWindowState.live_snapshot.lanes[3].rows[0].parameter_values.size() != 2 ||
            phaseSyncWindowState.live_snapshot.lanes[3].rows[0].parameter_values[0].path != "grade.saturation" ||
            std::fabs(phaseSyncWindowState.live_snapshot.lanes[3].rows[0].parameter_values[0].number_value - 1.15) > 1.0e-6 ||
            phaseSyncWindowState.live_snapshot.lanes[3].rows[0].parameter_values[1].path != "grade.contrast" ||
            std::fabs(phaseSyncWindowState.live_snapshot.lanes[3].rows[0].parameter_values[1].number_value - 1.10) > 1.0e-6) {
            std::cerr << "Expected phase tuples to import the live phase_finish grading row and its mirrored owner values\n";
            return 1;
        }

        ColorPipelineWindowState bandsSyncWindowState{};
        params.coloring_mode = ColoringMode::iteration_bands;
        params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::iteration_bands);
        params.color_shape = ColorPipelineShape::identity;
        params.color_shape_offset = 0.0f;
        params.color_shape_scale = 1.0f;
        params.color_saturation = 0.75f;
        params.color_contrast = 1.8f;
        if (!SyncColorPipelineWindowFromLiveState(&bandsSyncWindowState, view.fractal_type, &params)) {
            std::cerr << "Expected coherent iteration-bands tuples to import once band_finish is shipped as a runtime-real grading row\n";
            return 1;
        }
        if (!bandsSyncWindowState.live_snapshot.valid ||
            !bandsSyncWindowState.live_snapshot.draft_import_supported ||
            bandsSyncWindowState.live_snapshot.lanes.size() != 4 ||
            bandsSyncWindowState.live_snapshot.lanes[0].rows[0].function_id != "banded_signal" ||
            bandsSyncWindowState.live_snapshot.lanes[2].rows[0].function_id != "banded_heatmap" ||
            bandsSyncWindowState.live_snapshot.lanes[3].rows.size() != 1 ||
            bandsSyncWindowState.live_snapshot.lanes[3].rows[0].function_id != "band_finish" ||
            bandsSyncWindowState.live_snapshot.lanes[3].rows[0].parameter_values.size() != 2 ||
            bandsSyncWindowState.live_snapshot.lanes[3].rows[0].parameter_values[0].path != "grade.saturation" ||
            std::fabs(bandsSyncWindowState.live_snapshot.lanes[3].rows[0].parameter_values[0].number_value - 0.75) > 1.0e-6 ||
            bandsSyncWindowState.live_snapshot.lanes[3].rows[0].parameter_values[1].path != "grade.contrast" ||
            std::fabs(bandsSyncWindowState.live_snapshot.lanes[3].rows[0].parameter_values[1].number_value - 1.8) > 1.0e-6) {
            std::cerr << "Expected iteration-bands tuples to import the live band_finish grading row and its mirrored owner values\n";
            return 1;
        }

        ColorPipelineWindowState legacyControlOwnershipState{};
        if (ShouldDisableLegacyColorPanelControlWhileAdvancedWindowOpen(
                legacyControlOwnershipState,
                "fractal.params.coloring_mode") ||
            ShouldDisableLegacyColorPanelControlWhileAdvancedWindowOpen(
                legacyControlOwnershipState,
                "fractal.params.color_grading")) {
            std::cerr << "Expected the simple Coloring Mode and Grading controls to remain available while the advanced color window stays closed\n";
            return 1;
        }
        legacyControlOwnershipState.open = true;
        if (!ShouldDisableLegacyColorPanelControlWhileAdvancedWindowOpen(
                legacyControlOwnershipState,
                "fractal.params.coloring_mode") ||
            !ShouldDisableLegacyColorPanelControlWhileAdvancedWindowOpen(
                legacyControlOwnershipState,
                "fractal.params.color_grading") ||
            ShouldDisableLegacyColorPanelControlWhileAdvancedWindowOpen(
                legacyControlOwnershipState,
                "fractal.params.color_saturation")) {
            std::cerr << "Expected the advanced color window to own only the legacy Coloring Mode and Grading controls while it is open\n";
            return 1;
        }

        ColorPipelineWindowState invalidLiveWindowState{};
        params.coloring_mode = ColoringMode::phase;
        params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
        params.color_shape = ColorPipelineShape::identity;
        params.color_shape_offset = 0.0f;
        params.color_shape_scale = 1.0f;
        if (SyncColorPipelineWindowFromLiveState(&invalidLiveWindowState, view.fractal_type, &params)) {
            std::cerr << "Expected out-of-sync live color state to fail snapshot import instead of pretending it was valid\n";
            return 1;
        }
        if (invalidLiveWindowState.live_snapshot.valid) {
            std::cerr << "Expected invalid live color state to leave the live snapshot unavailable for import\n";
            return 1;
        }
        const ColorPipelineDraftApplyState invalidLiveApplyState = DescribeColorPipelineDraftApplyState(
            invalidLiveWindowState,
            view.fractal_type,
            &params);
        if (invalidLiveApplyState.status != ColorPipelineDraftApplyStatus::can_apply) {
            std::cerr << "Expected the default supported draft to remain applicable so the advanced window can repair an invalid live color state\n";
            return 1;
        }
        ColorPipelineRenderInteractionState invalidLiveInteractionState{};
        if (ShouldAutoApplySupportedColorPipelineDraft(
                invalidLiveWindowState,
                invalidLiveApplyState,
                invalidLiveInteractionState,
                &params)) {
            std::cerr << "Expected supported auto-apply to stay dormant until the user actually interacts with the advanced color window\n";
            return 1;
        }
        invalidLiveInteractionState.interacted = true;
        if (!ShouldAutoApplySupportedColorPipelineDraft(
                invalidLiveWindowState,
                invalidLiveApplyState,
                invalidLiveInteractionState,
                &params)) {
            std::cerr << "Expected supported auto-apply to become eligible once the user has interacted with the advanced color window\n";
            return 1;
        }
        if (!ApplyColorPipelineDraftToLiveState(&invalidLiveWindowState, view.fractal_type, &params)) {
            std::cerr << "Expected the supported draft to repair an invalid live color state\n";
            return 1;
        }
        if (params.coloring_mode != ColoringMode::smooth_escape ||
            params.color_pipeline.signal != ColorSignal::smooth_escape ||
            params.color_pipeline.palette != ColorPalette::cyclic_escape ||
            params.color_pipeline.grading != ColorGradingPreset::escape_default ||
            !invalidLiveWindowState.live_snapshot.valid ||
            !invalidLiveWindowState.live_snapshot.draft_import_supported) {
            std::cerr << "Expected invalid live recovery to repair the runtime tuple and restore an importable live snapshot\n";
            return 1;
        }
    }

    {
        ImGuiTestContext imgui;
        ViewState view{};
        KernelParams params{};
        ColorPipelineWindowState windowState{};
        params.coloring_mode = ColoringMode::phase;
        params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::phase);
        params.color_shape = ColorPipelineShape::offset_scale;
        params.color_shape_stack_count = 0;
        params.color_shape_offset = 0.25f;
        params.color_shape_scale = 1.5f;
        if (!EnsureColorPipelineWindowInitialized(&windowState)) {
            std::cerr << "Expected the advanced color pipeline draft editor to initialize before param mutation coverage\n";
            return 1;
        }
        auto setParam = [](ColorPipelineRowState& row, const char* path, double value) {
            for (ColorPipelineParamState& param : row.parameter_values) {
                if (param.path == path) {
                    param.number_value = value;
                    return true;
                }
            }
            return false;
        };
        std::vector<std::size_t> visibleParamIndexes;
        if (!SelectColorPipelineLaneFunction(&windowState, 0, "phase_orbit") ||
            !SelectColorPipelineLaneFunction(&windowState, 1, "repeat") ||
            !SelectColorPipelineLaneFunction(&windowState, 2, "phase_wheel_palette")) {
            std::cerr << "Expected the live programmable editor RED to construct a legal phase tuple with the repeat Shape row\n";
            return 1;
        }
        if (!setParam(windowState.lanes[0].rows[0], "signal.phase_offset", 1.25) ||
            !setParam(windowState.lanes[0].rows[0], "signal.wrap_cycles", 2.5) ||
            !setParam(windowState.lanes[1].rows[0], "shape.frequency", 6.0) ||
            !setParam(windowState.lanes[1].rows[0], "shape.phase", 0.2) ||
            !setParam(windowState.lanes[2].rows[0], "palette.phase_offset", -0.75)) {
            std::cerr << "Expected the live programmable editor RED to expose the current phase and Shape parameter controls\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[1].rows[0], &visibleParamIndexes) ||
            visibleParamIndexes.size() != 2 ||
            visibleParamIndexes[0] != 0 ||
            visibleParamIndexes[1] != 1) {
            std::cerr << "Expected offset_scale to expose only its real runtime-backed Shape controls\n";
            return 1;
        }
        const ColorPipelineDraftApplyState phaseApplyState = DescribeColorPipelineDraftApplyState(
            windowState,
            view.fractal_type,
            &params);
        if (windowState.lanes[3].rows[0].function_id != "phase_finish" ||
            phaseApplyState.status != ColorPipelineDraftApplyStatus::can_apply) {
            std::cerr << "Expected phase tuple auto-complete to promote the shipped phase_finish grading row into an applyable draft\n";
            return 1;
        }
        if (!ApplyColorPipelineDraftToLiveState(&windowState, view.fractal_type, &params) ||
            params.coloring_mode != ColoringMode::phase ||
            params.color_pipeline.signal != ColorSignal::phase_angle ||
            params.color_pipeline.palette != ColorPalette::phase_wheel ||
            params.color_pipeline.grading != ColorGradingPreset::phase_default ||
            std::fabs(params.color_saturation - 1.15f) > 1.0e-6f ||
            std::fabs(params.color_contrast - 1.10f) > 1.0e-6f) {
            std::cerr << "Expected the advanced editor to apply the shipped phase_finish tuple through the mirrored legacy grading owners\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 0, "smooth_escape_ramp") ||
            !SelectColorPipelineLaneFunction(&windowState, 2, "heatmap")) {
            std::cerr << "Expected the test to restore a shipped smooth-escape tuple before continuing shape-owner apply coverage\n";
            return 1;
        }

        if (!SelectColorPipelineLaneFunction(&windowState, 1, "posterize") ||
            !setParam(windowState.lanes[1].rows[0], "shape.steps", 5.0) ||
            !setParam(windowState.lanes[1].rows[0], "shape.mix", 0.65)) {
            std::cerr << "Expected the live programmable editor to expose the posterize Shape controls once runtime-backed\n";
            return 1;
        }
        if (!ApplyColorPipelineDraftToLiveState(&windowState, view.fractal_type, &params)) {
            std::cerr << "Expected the live programmable editor to apply the posterize Shape tuple\n";
            return 1;
        }
        if (params.color_shape != ColorPipelineShape::posterize ||
            params.color_shape_posterize_steps != 5 ||
            !NearlyEqual(params.color_shape_posterize_mix, 0.65) ||
            !NearlyEqual(params.color_shape_offset, 0.0) ||
            !NearlyEqual(params.color_shape_scale, 1.0) ||
            !NearlyEqual(params.color_shape_repeat_frequency, 8.0) ||
            !NearlyEqual(params.color_shape_repeat_phase, 0.0) ||
            !windowState.live_snapshot.valid ||
            windowState.live_snapshot.lanes[1].rows[0].function_id != "posterize" ||
            HasColorPipelineDraftEdits(windowState)) {
            std::cerr << "Expected live programmable apply to write posterize owner fields, reset other Shape owners, and resync the live snapshot\n";
            return 1;
        }

        if (!SelectColorPipelineLaneFunction(&windowState, 1, "mirror_repeat") ||
            !setParam(windowState.lanes[1].rows[0], "shape.frequency", 3.0) ||
            !setParam(windowState.lanes[1].rows[0], "shape.phase", 0.15)) {
            std::cerr << "Expected the live programmable editor to expose the mirror_repeat Shape controls once runtime-backed\n";
            return 1;
        }
        if (!ApplyColorPipelineDraftToLiveState(&windowState, view.fractal_type, &params)) {
            std::cerr << "Expected the live programmable editor to apply the mirror_repeat Shape tuple\n";
            return 1;
        }
        if (params.color_shape != ColorPipelineShape::mirror_repeat ||
            !NearlyEqual(params.color_shape_repeat_frequency, 3.0) ||
            !NearlyEqual(params.color_shape_repeat_phase, 0.15) ||
            !NearlyEqual(params.color_shape_offset, 0.0) ||
            !NearlyEqual(params.color_shape_scale, 1.0) ||
            !windowState.live_snapshot.valid ||
            windowState.live_snapshot.lanes[1].rows[0].function_id != "mirror_repeat" ||
            HasColorPipelineDraftEdits(windowState)) {
            std::cerr << "Expected live programmable apply to write the mirror_repeat Shape choice through the reused repeat owner fields and resync the live snapshot\n";
            return 1;
        }

        if (!SelectColorPipelineLaneFunction(&windowState, 1, "bias_gain_curve") ||
            !setParam(windowState.lanes[1].rows[0], "shape.bias", 0.25) ||
            !setParam(windowState.lanes[1].rows[0], "shape.gain", 0.75)) {
            std::cerr << "Expected the live programmable editor to expose the bias_gain_curve Shape controls once runtime-backed\n";
            return 1;
        }
        if (!ApplyColorPipelineDraftToLiveState(&windowState, view.fractal_type, &params)) {
            std::cerr << "Expected the live programmable editor to apply the bias_gain_curve Shape tuple\n";
            return 1;
        }
        if (params.color_shape != ColorPipelineShape::bias_gain_curve ||
            !NearlyEqual(params.color_shape_bias, 0.25) ||
            !NearlyEqual(params.color_shape_gain, 0.75) ||
            !NearlyEqual(params.color_shape_offset, 0.0) ||
            !NearlyEqual(params.color_shape_scale, 1.0) ||
            !NearlyEqual(params.color_shape_repeat_frequency, 8.0) ||
            !NearlyEqual(params.color_shape_repeat_phase, 0.0) ||
            params.color_shape_posterize_steps != 6 ||
            !NearlyEqual(params.color_shape_posterize_mix, 1.0) ||
            !windowState.live_snapshot.valid ||
            windowState.live_snapshot.lanes[1].rows[0].function_id != "bias_gain_curve" ||
            HasColorPipelineDraftEdits(windowState)) {
            std::cerr << "Expected live programmable apply to write the bias_gain_curve owner fields, reset other Shape owners, and resync the live snapshot\n";
            return 1;
        }

        if (!SelectColorPipelineLaneFunction(&windowState, 1, "smooth_window") ||
            !setParam(windowState.lanes[1].rows[0], "shape.center", 0.35) ||
            !setParam(windowState.lanes[1].rows[0], "shape.width", 0.4) ||
            !setParam(windowState.lanes[1].rows[0], "shape.softness", 0.05)) {
            std::cerr << "Expected the live programmable editor to expose the smooth_window Shape controls once runtime-backed\n";
            return 1;
        }
        if (!ApplyColorPipelineDraftToLiveState(&windowState, view.fractal_type, &params)) {
            std::cerr << "Expected the live programmable editor to apply the smooth_window Shape tuple\n";
            return 1;
        }
        if (params.color_shape != ColorPipelineShape::smooth_window ||
            !NearlyEqual(params.color_shape_window_center, 0.35) ||
            !NearlyEqual(params.color_shape_window_width, 0.4) ||
            !NearlyEqual(params.color_shape_window_softness, 0.05) ||
            !NearlyEqual(params.color_shape_offset, 0.0) ||
            !NearlyEqual(params.color_shape_scale, 1.0) ||
            !NearlyEqual(params.color_shape_repeat_frequency, 8.0) ||
            !NearlyEqual(params.color_shape_repeat_phase, 0.0) ||
            params.color_shape_posterize_steps != 6 ||
            !NearlyEqual(params.color_shape_posterize_mix, 1.0) ||
            !NearlyEqual(params.color_shape_bias, 0.5) ||
            !NearlyEqual(params.color_shape_gain, 0.5) ||
            !windowState.live_snapshot.valid ||
            windowState.live_snapshot.lanes[1].rows[0].function_id != "smooth_window" ||
            HasColorPipelineDraftEdits(windowState)) {
            std::cerr << "Expected live programmable apply to write the smooth_window owner fields, reset other Shape owners, and resync the live snapshot\n";
            return 1;
        }

        if (!SelectColorPipelineLaneFunction(&windowState, 1, "offset_scale") ||
            !setParam(windowState.lanes[1].rows[0], "shape.offset", 0.25) ||
            !setParam(windowState.lanes[1].rows[0], "shape.scale", 1.5) ||
            !AddColorPipelineLaneRow(&windowState, 1, "repeat") ||
            windowState.lanes[1].rows.size() != 2 ||
            !setParam(windowState.lanes[1].rows[1], "shape.frequency", 6.0) ||
            !setParam(windowState.lanes[1].rows[1], "shape.phase", 0.2)) {
            std::cerr << "Expected the backend-recovery RED to construct a supported two-row Shape lane with independent owner packs\n";
            return 1;
        }
        if (!ApplyColorPipelineDraftToLiveState(&windowState, view.fractal_type, &params)) {
            std::cerr << "Expected a supported two-row Shape lane to apply to live runtime state instead of failing at the single-row bridge\n";
            return 1;
        }
        if (params.color_shape_stack_count != 2 ||
            params.color_shape_stack[0].shape != ColorPipelineShape::offset_scale ||
            !NearlyEqual(params.color_shape_stack[0].params.offset, 0.25) ||
            !NearlyEqual(params.color_shape_stack[0].params.scale, 1.5) ||
            params.color_shape_stack[1].shape != ColorPipelineShape::repeat ||
            !NearlyEqual(params.color_shape_stack[1].params.repeat_frequency, 6.0) ||
            !NearlyEqual(params.color_shape_stack[1].params.repeat_phase, 0.2) ||
            !windowState.live_snapshot.valid ||
            !windowState.live_snapshot.draft_import_supported ||
            windowState.live_snapshot.lanes.size() < 3 ||
            windowState.live_snapshot.lanes[1].rows.size() != 2 ||
            windowState.live_snapshot.lanes[1].rows[0].function_id != "offset_scale" ||
            windowState.live_snapshot.lanes[1].rows[1].function_id != "repeat" ||
            HasColorPipelineDraftEdits(windowState)) {
            std::cerr << "Expected live programmable apply to persist and resync a supported two-row Shape lane instead of collapsing back to one row\n";
            return 1;
        }

        if (!SelectColorPipelineLaneFunction(&windowState, 0, "root_index") ||
            !SelectColorPipelineLaneFunction(&windowState, 1, "identity") ||
            !SelectColorPipelineLaneFunction(&windowState, 2, "root_classic_palette") ||
            !AddColorPipelineLaneRow(&windowState, 0, "root_index") ||
            windowState.lanes[0].rows.size() != 2 ||
            !AddColorPipelineLaneRow(&windowState, 2, "joy_root_palette") ||
            windowState.lanes[2].rows.size() != 2) {
            std::cerr << "Expected the root-basin pair RED to construct a two-row Source/Palette schedule with paired root rows\n";
            return 1;
        }
        bool rootBasinScheduleChanged = false;
        if (!ApplyColorPipelineDraftToLiveState(&windowState, view.fractal_type, &params, &rootBasinScheduleChanged)) {
            std::cerr << "Expected a supported two-row root-basin schedule to apply to live runtime state instead of failing at the single-row Source/Palette bridge\n";
            return 1;
        }
        if (!rootBasinScheduleChanged ||
            params.coloring_mode != ColoringMode::joy_basins ||
            params.color_pipeline.signal != ColorSignal::root_index ||
            params.color_pipeline.palette != ColorPalette::joy ||
            params.color_pipeline.grading != ColorGradingPreset::basin_default ||
            !windowState.live_snapshot.valid ||
            !windowState.live_snapshot.draft_import_supported ||
            windowState.live_snapshot.lanes.size() < 3 ||
            windowState.live_snapshot.lanes[0].rows.size() != 2 ||
            windowState.live_snapshot.lanes[0].rows[0].function_id != "root_index" ||
            windowState.live_snapshot.lanes[0].rows[1].function_id != "root_index" ||
            windowState.live_snapshot.lanes[2].rows.size() != 2 ||
            windowState.live_snapshot.lanes[2].rows[0].function_id != "root_classic_palette" ||
            windowState.live_snapshot.lanes[2].rows[1].function_id != "joy_root_palette" ||
            HasColorPipelineDraftEdits(windowState)) {
            std::cerr << "Expected live programmable apply to persist and resync a bounded two-row root-basin pair schedule while mirroring the final valid pair into the legacy runtime tuple\n";
            return 1;
        }
        if (!RemoveColorPipelineLaneRow(&windowState, 0, 1) ||
            !RemoveColorPipelineLaneRow(&windowState, 2, 1) ||
            windowState.lanes[0].rows.size() != 1 ||
            windowState.lanes[2].rows.size() != 1) {
            std::cerr << "Expected the root-basin pair coverage to restore the draft editor to one Source row and one Palette row before later single-row tuple tests\n";
            return 1;
        }

        if (!SelectColorPipelineLaneFunction(&windowState, 0, "smooth_escape_ramp") ||
            !SelectColorPipelineLaneFunction(&windowState, 1, "identity") ||
            !SelectColorPipelineLaneFunction(&windowState, 2, "heatmap") ||
            !setParam(windowState.lanes[0].rows[0], "signal.scale", 0.5) ||
            !setParam(windowState.lanes[0].rows[0], "signal.bias", 0.25) ||
            !AddColorPipelineLaneRow(&windowState, 0, "escape_magnitude") ||
            windowState.lanes[0].rows.size() != 2 ||
            !setParam(windowState.lanes[0].rows[1], "signal.magnitude_scale", 1.5) ||
            !setParam(windowState.lanes[0].rows[1], "signal.magnitude_bias", -0.25) ||
            !setParam(windowState.lanes[0].rows[1], "signal.blend_weight", 0.25)) {
            std::cerr << "Expected the Source weighted-blend RED to construct a supported two-row generic Source lane with per-row Source params and blend weight\n";
            return 1;
        }
        const ColorPipelineDraftApplyState weightedSourceApplyState = DescribeColorPipelineDraftApplyState(
            windowState,
            view.fractal_type,
            &params);
        if (weightedSourceApplyState.status != ColorPipelineDraftApplyStatus::can_apply) {
            std::cerr << "Expected the shipped two-row weighted Source stack to stay live-applicable instead of reading as draft-only\n";
            return 1;
        }
        bool sourceStackChanged = false;
        if (!ApplyColorPipelineDraftToLiveState(&windowState, view.fractal_type, &params, &sourceStackChanged)) {
            std::cerr << "Expected a supported two-row generic Source lane to apply to live runtime state instead of failing at the single-row Source bridge\n";
            return 1;
        }
        if (!sourceStackChanged ||
            params.coloring_mode != ColoringMode::smooth_escape ||
            params.color_pipeline.signal != ColorSignal::escape_magnitude ||
            params.color_pipeline.palette != ColorPalette::cyclic_escape ||
            params.color_pipeline.grading != ColorGradingPreset::escape_default ||
            params.color_source_stack_count != 2 ||
            params.color_source_stack[0].signal != ColorSignal::smooth_escape ||
            !NearlyEqual(params.color_source_stack[0].params.scale, 0.5) ||
            !NearlyEqual(params.color_source_stack[0].params.bias, 0.25) ||
            params.color_source_stack[1].signal != ColorSignal::escape_magnitude ||
            !NearlyEqual(params.color_source_stack[1].params.magnitude_scale, 1.5) ||
            !NearlyEqual(params.color_source_stack[1].params.magnitude_bias, -0.25) ||
            !NearlyEqual(params.color_source_stack[1].params.blend_weight, 0.25) ||
            !windowState.live_snapshot.valid ||
            !windowState.live_snapshot.draft_import_supported ||
            windowState.live_snapshot.lanes.size() < 3 ||
            windowState.live_snapshot.lanes[0].rows.size() != 2 ||
            windowState.live_snapshot.lanes[0].rows[0].function_id != "smooth_escape_ramp" ||
            windowState.live_snapshot.lanes[0].rows[1].function_id != "escape_magnitude" ||
            HasColorPipelineDraftEdits(windowState)) {
            std::cerr << "Expected live programmable apply to persist and resync a two-row generic Source lane while mirroring the final valid Source row into the legacy runtime tuple\n";
            return 1;
        }
        if (!RemoveColorPipelineLaneRow(&windowState, 0, 1) ||
            windowState.lanes[0].rows.size() != 1) {
            std::cerr << "Expected Source weighted-blend coverage to restore the draft editor to one Source row before later tuple tests\n";
            return 1;
        }

        if (!SelectColorPipelineLaneFunction(&windowState, 0, "smooth_escape_ramp") ||
            !SelectColorPipelineLaneFunction(&windowState, 1, "identity") ||
            !SelectColorPipelineLaneFunction(&windowState, 2, "heatmap") ||
            !setParam(windowState.lanes[2].rows[0], "palette.cycle_scale", 1.0) ||
            !setParam(windowState.lanes[2].rows[0], "palette.saturation", 1.0) ||
            !AddColorPipelineLaneRow(&windowState, 2, "explaino_cmap") ||
            windowState.lanes[2].rows.size() != 2 ||
            !setParam(windowState.lanes[2].rows[1], "palette.seed_scale", 1.5) ||
            !setParam(windowState.lanes[2].rows[1], "palette.seed_phase", 0.25) ||
            !setParam(windowState.lanes[2].rows[1], "palette.colorfulness", 0.8) ||
            !setParam(windowState.lanes[2].rows[1], "palette.blend_weight", 0.35)) {
            std::cerr << "Expected the Palette blend stack RED to construct a two-row runtime-backed Palette lane\n";
            return 1;
        }
        bool paletteStackChanged = false;
        if (!ApplyColorPipelineDraftToLiveState(&windowState, view.fractal_type, &params, &paletteStackChanged)) {
            std::cerr << "Expected a supported two-row Palette blend stack to apply instead of failing at the single-row Palette bridge\n";
            return 1;
        }
        if (!paletteStackChanged ||
            params.coloring_mode != ColoringMode::smooth_escape ||
            params.color_pipeline.signal != ColorSignal::smooth_escape ||
            params.color_pipeline.palette != ColorPalette::explaino_cmap ||
            params.color_pipeline.grading != ColorGradingPreset::escape_default ||
            params.color_palette_stack_count != 2 ||
            !NearlyEqual(params.color_palette_stack[1].params.blend_weight, 0.35) ||
            !windowState.live_snapshot.valid ||
            !windowState.live_snapshot.draft_import_supported ||
            windowState.live_snapshot.lanes.size() < 3 ||
            windowState.live_snapshot.lanes[2].rows.size() != 2 ||
            windowState.live_snapshot.lanes[2].rows[0].function_id != "heatmap" ||
            windowState.live_snapshot.lanes[2].rows[1].function_id != "explaino_cmap" ||
            HasColorPipelineDraftEdits(windowState)) {
            std::cerr << "Expected live programmable apply to persist and resync a two-row Palette blend stack while mirroring the final Palette row into the legacy runtime tuple\n";
            return 1;
        }
        if (!RemoveColorPipelineLaneRow(&windowState, 2, 1) ||
            windowState.lanes[2].rows.size() != 1) {
            std::cerr << "Expected Palette blend stack coverage to restore the draft editor to one Palette row before later single-row tuple tests\n";
            return 1;
        }

        if (!SelectColorPipelineLaneFunction(&windowState, 0, "smooth_escape_ramp") ||
            !SelectColorPipelineLaneFunction(&windowState, 1, "identity") ||
            !SelectColorPipelineLaneFunction(&windowState, 2, "explaino_cmap") ||
            !setParam(windowState.lanes[2].rows[0], "palette.seed_scale", 1.5) ||
            !setParam(windowState.lanes[2].rows[0], "palette.seed_phase", 0.25) ||
            !setParam(windowState.lanes[2].rows[0], "palette.colorfulness", 0.8)) {
            std::cerr << "Expected the live programmable editor to expose the explaino_cmap Palette controls once runtime-backed\n";
            return 1;
        }
        if (!CollectRenderableColorPipelineParamIndexes(windowState.lanes[2].rows[0], &visibleParamIndexes) ||
            visibleParamIndexes.size() != 5 ||
            visibleParamIndexes[0] != 0 ||
            visibleParamIndexes[1] != 1 ||
            visibleParamIndexes[2] != 2 ||
            visibleParamIndexes[3] != 3 ||
            visibleParamIndexes[4] != 4) {
            std::cerr << "Expected explaino_cmap to expose its dedicated live Palette and blend controls\n";
            return 1;
        }
        if (!ApplyColorPipelineDraftToLiveState(&windowState, view.fractal_type, &params)) {
            std::cerr << "Expected the live programmable editor to apply the explaino_cmap Palette tuple\n";
            return 1;
        }
        if (params.coloring_mode != ColoringMode::smooth_escape ||
            params.color_pipeline.signal != ColorSignal::smooth_escape ||
            params.color_pipeline.palette != ColorPalette::explaino_cmap ||
            params.color_pipeline.grading != ColorGradingPreset::escape_default ||
            !NearlyEqual(params.color_explaino_palette_seed_scale, 1.5) ||
            !NearlyEqual(params.color_explaino_palette_seed_phase, 0.25) ||
            !NearlyEqual(params.color_explaino_palette_colorfulness, 0.8) ||
            !NearlyEqual(params.color_heatmap_cycle_scale, 1.0) ||
            !NearlyEqual(params.color_heatmap_saturation, 1.0) ||
            !NearlyEqual(params.color_phase_palette_offset, 0.0) ||
            !NearlyEqual(params.color_iteration_band_emphasis, 1.0) ||
            !NearlyEqual(params.color_iteration_band_palette_offset, 0.0) ||
            !windowState.live_snapshot.valid ||
            windowState.live_snapshot.lanes[2].rows[0].function_id != "explaino_cmap" ||
            HasColorPipelineDraftEdits(windowState)) {
            std::cerr << "Expected live programmable apply to write the explaino_cmap owner fields, reset other Palette owners, and resync the live snapshot\n";
            return 1;
        }
        if (!setParam(windowState.lanes[3].rows[0], "grade.exposure", 1.6) ||
            !setParam(windowState.lanes[3].rows[0], "grade.saturation", 0.85) ||
            !SelectColorPipelineLaneFunction(&windowState, 2, "heatmap")) {
            std::cerr << "Expected the live programmable editor to support same-grading escape tuple switches during grading-preservation coverage\n";
            return 1;
        }
        if (windowState.lanes[0].rows[0].function_id != "smooth_escape_ramp" ||
            windowState.lanes[2].rows[0].function_id != "heatmap" ||
            windowState.lanes[3].rows[0].function_id != "contrast_lift" ||
            std::fabs(windowState.lanes[3].rows[0].parameter_values[0].number_value - 1.6) > 1.0e-6 ||
            std::fabs(windowState.lanes[3].rows[0].parameter_values[1].number_value - 0.85) > 1.0e-6) {
            std::cerr << "Expected escape tuple switches that keep contrast_lift selected to preserve the current grading row values\n";
            return 1;
        }
        if (!SelectColorPipelineLaneFunction(&windowState, 2, "phase_wheel_palette")) {
            std::cerr << "Expected the rebuilt programmable editor to accept phase_wheel_palette during tuple auto-complete coverage\n";
            return 1;
        }
        if (windowState.lanes[0].rows[0].function_id != "phase_orbit" ||
            windowState.lanes[2].rows[0].function_id != "phase_wheel_palette" ||
            windowState.lanes[3].rows[0].function_id != "phase_finish") {
            std::cerr << "Expected selecting phase_wheel_palette to co-switch the matching phase_orbit and phase_finish rows instead of leaving a dead preview-only tuple\n";
            return 1;
        }
        const ColorPipelineDraftApplyState phaseWheelApplyState = DescribeColorPipelineDraftApplyState(windowState, view.fractal_type, &params);
        if (phaseWheelApplyState.status != ColorPipelineDraftApplyStatus::can_apply) {
            std::cerr << "Expected phase auto-complete selections to stay live-applicable once the matching grading row is shipped\n";
            return 1;
        }
        if (!AddColorPipelineLaneRow(&windowState, 1, "identity")) {
            std::cerr << "Expected the schedule editor RED to support stacking a second enabled Shape row for validation coverage\n";
            return 1;
        }
        const ColorPipelineDraftApplyState stackedShapeState = DescribeColorPipelineDraftApplyState(windowState, view.fractal_type, &params);
        if (stackedShapeState.status != ColorPipelineDraftApplyStatus::can_apply) {
            std::cerr << "Expected stacked phase tuples to stay live-applicable once the matching grading row is shipped\n";
            return 1;
        }
        if (!RemoveColorPipelineLaneRow(&windowState, 1, 1)) {
            std::cerr << "Expected the schedule editor RED to restore the single-row Shape bridge after validation coverage\n";
            return 1;
        }

        if (!SelectColorPipelineLaneFunction(&windowState, 2, "banded_heatmap")) {
            std::cerr << "Expected the rebuilt programmable editor to accept banded_heatmap during banded tuple auto-complete coverage\n";
            return 1;
        }
        if (windowState.lanes[0].rows[0].function_id != "banded_signal" ||
            windowState.lanes[2].rows[0].function_id != "banded_heatmap" ||
            windowState.lanes[3].rows[0].function_id != "band_finish") {
            std::cerr << "Expected selecting banded_heatmap to co-switch the matching banded_signal and band_finish rows instead of leaving a draft-only tuple\n";
            return 1;
        }
        const ColorPipelineDraftApplyState bandedApplyState = DescribeColorPipelineDraftApplyState(windowState, view.fractal_type, &params);
        if (bandedApplyState.status != ColorPipelineDraftApplyStatus::can_apply) {
            std::cerr << "Expected banded auto-complete selections to stay live-applicable once the matching grading row is shipped\n";
            return 1;
        }
        if (!setParam(windowState.lanes[3].rows[0], "grade.saturation", 0.75) ||
            !setParam(windowState.lanes[3].rows[0], "grade.contrast", 1.8) ||
            !ApplyColorPipelineDraftToLiveState(&windowState, view.fractal_type, &params)) {
            std::cerr << "Expected the live programmable editor to apply the banded_heatmap tuple through band_finish\n";
            return 1;
        }
        if (params.coloring_mode != ColoringMode::iteration_bands ||
            params.color_pipeline.signal != ColorSignal::iteration_bands ||
            params.color_pipeline.palette != ColorPalette::banded_escape ||
            params.color_pipeline.grading != ColorGradingPreset::bands_default ||
            !NearlyEqual(params.color_saturation, 0.75) ||
            !NearlyEqual(params.color_contrast, 1.8) ||
            !windowState.live_snapshot.valid ||
            windowState.live_snapshot.lanes[3].rows[0].function_id != "band_finish" ||
            HasColorPipelineDraftEdits(windowState)) {
            std::cerr << "Expected live programmable apply to write the iteration-bands tuple and resync band_finish without draft residue\n";
            return 1;
        }

        if (!SelectColorPipelineLaneFunction(&windowState, 0, "smooth_escape_ramp") ||
            !SelectColorPipelineLaneFunction(&windowState, 1, "offset_scale") ||
            !SelectColorPipelineLaneFunction(&windowState, 2, "explaino_cmap")) {
            std::cerr << "Expected the live programmable editor to restore a shipped explaino tuple after the preview-only phase check\n";
            return 1;
        }
        if (!setParam(windowState.lanes[0].rows[0], "signal.scale", 0.5) ||
            !setParam(windowState.lanes[1].rows[0], "shape.scale", 2.0)) {
            std::cerr << "Expected the live programmable editor to expose shipped explaino and Shape controls for apply-state coverage\n";
            return 1;
        }
        const ColorPipelineDraftApplyState validApplyState = DescribeColorPipelineDraftApplyState(windowState, view.fractal_type, &params);
        if (validApplyState.status != ColorPipelineDraftApplyStatus::can_apply) {
            std::cerr << "Expected valid shipped explaino drafts to classify as live-applicable\n";
            return 1;
        }

        ColorPipelineRenderInteractionState interactionState{};
        if (ShouldAutoApplySupportedColorPipelineDraft(windowState, validApplyState, interactionState, &params)) {
            std::cerr << "Expected supported end-of-frame apply to stay dormant until the user interacts with the programmable controls\n";
            return 1;
        }
        interactionState.interacted = true;
        if (!ShouldAutoApplySupportedColorPipelineDraft(windowState, validApplyState, interactionState, &params)) {
            std::cerr << "Expected supported end-of-frame apply to become eligible after the user interacts with the programmable controls\n";
            return 1;
        }
        interactionState.has_active_item = true;
        if (ShouldAutoApplySupportedColorPipelineDraft(
                windowState,
                validApplyState,
                interactionState,
                &params)) {
            std::cerr << "Expected active programmable-control drags to bypass the end-of-frame apply helper so supported slider edits can use a direct live path instead\n";
            return 1;
        }
        interactionState.has_active_item = false;
    }

    {
        ImGuiTestContext imgui;
        ViewState view{};
        KernelParams params{};
        ColorPipelineWindowState windowState{};
        params.coloring_mode = ColoringMode::phase;
        params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::phase);
        params.color_shape = ColorPipelineShape::offset_scale;
        params.color_shape_stack_count = 0;
        params.color_shape_offset = 0.25f;
        params.color_shape_scale = 1.5f;

        BeginFrame();
        ColorPipelineWindowState closedWindowState{};
        if (RenderColorPipelineWindow(&closedWindowState, view.fractal_type, &params)) {
            std::cerr << "Expected the advanced color pipeline window helper to stay hidden while closed\n";
            return 1;
        }
        EndFrame();

        BeginFrame();
        ColorPipelineWindowState openWindowState{};
        openWindowState.open = true;
        if (!RenderColorPipelineWindow(&openWindowState, view.fractal_type, &params)) {
            std::cerr << "Expected the advanced color pipeline window helper to render once opened\n";
            return 1;
        }
        if (!openWindowState.open) {
            std::cerr << "Expected the advanced color pipeline window helper to preserve the open state without user dismissal\n";
            return 1;
        }
        if (!openWindowState.initialized ||
            !openWindowState.live_snapshot.valid ||
            !openWindowState.live_snapshot.draft_import_supported ||
            openWindowState.live_snapshot.lanes.size() != 4 ||
            openWindowState.lanes.size() != 4 ||
            openWindowState.lanes[0].rows.size() != 1 ||
            openWindowState.lanes[0].rows[0].function_id != "phase_orbit" ||
            openWindowState.lanes[1].rows.size() != 1 ||
            openWindowState.lanes[1].rows[0].function_id != "offset_scale" ||
            openWindowState.lanes[2].rows.size() != 1 ||
            openWindowState.lanes[2].rows[0].function_id != "phase_wheel_palette" ||
            openWindowState.lanes[3].rows.size() != 1 ||
            openWindowState.lanes[3].rows[0].function_id != "phase_finish") {
            std::cerr << "Expected the advanced color pipeline window to import the shipped phase tuple instead of preserving the starter draft during render\n";
            return 1;
        }
        if (params.color_pipeline.signal != ColorSignal::phase_angle ||
            params.color_pipeline.palette != ColorPalette::phase_wheel ||
            params.color_pipeline.grading != ColorGradingPreset::phase_default) {
            std::cerr << "Expected rendering the live programmable window to preserve the current runtime tuple\n";
            return 1;
        }
        if (HasColorPipelineDraftEdits(openWindowState)) {
            std::cerr << "Expected the advanced color pipeline window to keep the imported phase tuple aligned with the live runtime state\n";
            return 1;
        }
        EndFrame();

        BeginFrame();
        ColorPipelineWindowState explainoRenderWindowState{};
        explainoRenderWindowState.open = true;
        params.coloring_mode = ColoringMode::smooth_escape;
        params.color_pipeline = {ColorSignal::smooth_escape, ColorPalette::explaino_cmap, ColorGradingPreset::escape_default};
        params.color_shape = ColorPipelineShape::identity;
        params.color_shape_offset = 0.0f;
        params.color_shape_scale = 1.0f;
        params.color_explaino_palette_seed_scale = 1.5f;
        params.color_explaino_palette_seed_phase = 0.25f;
        params.color_explaino_palette_colorfulness = 0.8f;
        if (!RenderColorPipelineWindow(&explainoRenderWindowState, view.fractal_type, &params)) {
            std::cerr << "Expected the advanced color pipeline window to render a live explaino_cmap tuple\n";
            return 1;
        }
        EndFrame();
        if (!explainoRenderWindowState.live_snapshot.valid ||
            !explainoRenderWindowState.live_snapshot.draft_import_supported ||
            explainoRenderWindowState.lanes.size() != 4 ||
            explainoRenderWindowState.lanes[2].rows.size() != 1 ||
            explainoRenderWindowState.lanes[2].rows[0].function_id != "explaino_cmap" ||
            explainoRenderWindowState.lanes[2].rows[0].parameter_values.size() != 5 ||
            explainoRenderWindowState.lanes[3].rows.size() != 1 ||
            explainoRenderWindowState.lanes[3].rows[0].function_id != "contrast_lift" ||
            explainoRenderWindowState.lanes[3].rows[0].parameter_values.size() != 2 ||
            !NearlyEqual(explainoRenderWindowState.lanes[3].rows[0].parameter_values[0].number_value, 1.0) ||
            !NearlyEqual(explainoRenderWindowState.lanes[3].rows[0].parameter_values[1].number_value, 1.0) ||
            !NearlyEqual(explainoRenderWindowState.lanes[2].rows[0].parameter_values[0].number_value, 1.5) ||
            !NearlyEqual(explainoRenderWindowState.lanes[2].rows[0].parameter_values[1].number_value, 0.25) ||
            !NearlyEqual(explainoRenderWindowState.lanes[2].rows[0].parameter_values[2].number_value, 0.8) ||
            params.color_pipeline.palette != ColorPalette::explaino_cmap ||
            !NearlyEqual(params.color_explaino_palette_seed_scale, 1.5) ||
            !NearlyEqual(params.color_explaino_palette_seed_phase, 0.25) ||
            !NearlyEqual(params.color_explaino_palette_colorfulness, 0.8)) {
            std::cerr << "Expected opening the advanced color pipeline window on a live explaino_cmap tuple to preserve the runtime and visible explaino controls\n";
            return 1;
        }

        BeginFrame();
        ColorPipelineWindowState unsupportedStartupWindowState{};
        unsupportedStartupWindowState.open = true;
        params = KernelParams{};
        view.fractal_type = FractalType::explaino;
        if (!RenderColorPipelineWindow(&unsupportedStartupWindowState, view.fractal_type, &params)) {
            std::cerr << "Expected the advanced color pipeline window to render even when startup begins on an unsupported live tuple\n";
            return 1;
        }
        EndFrame();
        if (params.coloring_mode != ColoringMode::root_basin ||
            params.color_pipeline.signal != ColorSignal::root_index ||
            params.color_pipeline.palette != ColorPalette::root_classic ||
            params.color_pipeline.grading != ColorGradingPreset::basin_default ||
            !unsupportedStartupWindowState.live_snapshot.valid ||
            !unsupportedStartupWindowState.live_snapshot.draft_import_supported ||
            unsupportedStartupWindowState.lanes.size() != 4 ||
            unsupportedStartupWindowState.lanes[0].rows.size() != 1 ||
            unsupportedStartupWindowState.lanes[0].rows[0].function_id != "root_index" ||
            unsupportedStartupWindowState.lanes[2].rows.size() != 1 ||
            unsupportedStartupWindowState.lanes[2].rows[0].function_id != "root_classic_palette" ||
            unsupportedStartupWindowState.lanes[3].rows.size() != 1 ||
            unsupportedStartupWindowState.lanes[3].rows[0].function_id != "basin_default" ||
            !unsupportedStartupWindowState.lanes[0].rows[0].parameter_values.empty() ||
            !unsupportedStartupWindowState.lanes[2].rows[0].parameter_values.empty() ||
            !unsupportedStartupWindowState.lanes[3].rows[0].parameter_values.empty()) {
            std::cerr << "Expected opening the advanced color pipeline window from the default basin tuple to import root_index, root_classic_palette, and basin_default as a supported live advanced row\n";
            return 1;
        }

        BeginFrame();
        ColorPipelineWindowState invalidLiveRenderState{};
        invalidLiveRenderState.open = true;
        params.coloring_mode = ColoringMode::phase;
        params.color_pipeline = ColorPipelineForLegacyMode(ColoringMode::smooth_escape);
        params.color_shape = ColorPipelineShape::identity;
        params.color_shape_offset = 0.0f;
        params.color_shape_scale = 1.0f;
        if (!RenderColorPipelineWindow(&invalidLiveRenderState, view.fractal_type, &params)) {
            std::cerr << "Expected the advanced color pipeline window to keep rendering while surfacing an invalid live color state\n";
            return 1;
        }
        EndFrame();
        if (params.coloring_mode != ColoringMode::phase ||
            params.color_pipeline.signal != ColorSignal::smooth_escape ||
            params.color_pipeline.palette != ColorPalette::cyclic_escape ||
            params.color_pipeline.grading != ColorGradingPreset::escape_default ||
            invalidLiveRenderState.live_snapshot.valid ||
            invalidLiveRenderState.lanes.size() != 4 ||
            invalidLiveRenderState.lanes[0].rows.size() != 1 ||
            invalidLiveRenderState.lanes[0].rows[0].function_id != "smooth_escape_ramp" ||
            invalidLiveRenderState.lanes[3].rows.size() != 1 ||
            invalidLiveRenderState.lanes[3].rows[0].function_id != "contrast_lift") {
            std::cerr << "Expected opening the advanced color pipeline window on an invalid live state to preserve the runtime until the user edits the draft\n";
            return 1;
        }

        auto setParam = [](ColorPipelineRowState& row, const char* path, double value) {
            for (ColorPipelineParamState& param : row.parameter_values) {
                if (param.path == path) {
                    param.number_value = value;
                    return true;
                }
            }
            return false;
        };
        auto findParamState = [](ColorPipelineRowState& row, const char* path) -> ColorPipelineParamState* {
            for (ColorPipelineParamState& param : row.parameter_values) {
                if (param.path == path) {
                    return &param;
                }
            }
            return nullptr;
        };
        auto findParamDescriptor = [](const FunctionDescriptor& descriptor, const char* path) -> const FunctionParamDescriptor* {
            for (const FunctionParamDescriptor& param : descriptor.parameters) {
                if (param.path == path) {
                    return &param;
                }
            }
            return nullptr;
        };
        if (!SelectColorPipelineLaneFunction(&explainoRenderWindowState, 1, "offset_scale") ||
            !setParam(explainoRenderWindowState.lanes[0].rows[0], "signal.scale", 0.625) ||
            !setParam(explainoRenderWindowState.lanes[1].rows[0], "shape.scale", 2.0)) {
            std::cerr << "Expected the advanced color pipeline window render test to find the supported live-backed controls before auto-apply coverage\n";
            return 1;
        }
        const ColorPipelineLaneCatalog* shapeCatalog = FindColorPipelineLaneCatalog("shape");
        if (!shapeCatalog) {
            std::cerr << "Expected the shape catalog to remain available for direct control coverage\n";
            return 1;
        }
        const FunctionDescriptor* offsetScaleDescriptor = FindColorPipelineFunctionDescriptor(*shapeCatalog, "offset_scale");
        if (!offsetScaleDescriptor) {
            std::cerr << "Expected the offset_scale descriptor to remain available for direct control coverage\n";
            return 1;
        }
        const FunctionParamDescriptor* shapeScaleDescriptor = findParamDescriptor(*offsetScaleDescriptor, "shape.scale");
        ColorPipelineParamState* shapeScaleParam = findParamState(explainoRenderWindowState.lanes[1].rows[0], "shape.scale");
        if (!shapeScaleDescriptor || !shapeScaleParam) {
            std::cerr << "Expected the offset_scale row to expose the live-backed shape.scale control\n";
            return 1;
        }

        bool directControlDirty = false;
        ColorPipelineRenderInteractionState activeDirectControlInteraction{};
        if (!CommitColorPipelineNumericParamEdit(
                &explainoRenderWindowState,
                view.fractal_type,
                &params,
                ResolveColorPipelineNumericControlRange(*shapeScaleDescriptor),
                2.0,
                shapeScaleParam,
                true,
                true,
                true,
                false,
                &directControlDirty,
                &activeDirectControlInteraction)) {
            std::cerr << "Expected active advanced color numeric edits to keep the viewport live while the control remains held\n";
            return 1;
        }
        if (!directControlDirty ||
            !activeDirectControlInteraction.interacted ||
            !activeDirectControlInteraction.has_active_item ||
            !NearlyEqual(params.color_smooth_escape_scale, 0.625) ||
            !NearlyEqual(params.color_shape_scale, 2.0) ||
            params.color_pipeline.signal != ColorSignal::smooth_escape ||
            params.color_pipeline.palette != ColorPalette::explaino_cmap ||
            params.color_pipeline.grading != ColorGradingPreset::escape_default ||
            !NearlyEqual(shapeScaleParam->number_value, 2.0) ||
            HasColorPipelineDraftEdits(explainoRenderWindowState)) {
            std::cerr << "Expected active advanced color numeric edits to mutate the live runtime immediately without leaving draft residue\n";
            return 1;
        }

        directControlDirty = false;
        ColorPipelineRenderInteractionState directControlInteraction{};
        if (!CommitColorPipelineNumericParamEdit(
                &explainoRenderWindowState,
                view.fractal_type,
                &params,
                ResolveColorPipelineNumericControlRange(*shapeScaleDescriptor),
                2.25,
                shapeScaleParam,
                true,
                false,
                false,
                true,
                &directControlDirty,
                &directControlInteraction)) {
            std::cerr << "Expected released numeric edits to keep the supported live-backed draft synced without a separate apply toggle\n";
            return 1;
        }
        if (!NearlyEqual(params.color_smooth_escape_scale, 0.625) ||
            !NearlyEqual(params.color_shape_scale, 2.25) ||
            params.color_pipeline.signal != ColorSignal::smooth_escape ||
            params.color_pipeline.palette != ColorPalette::explaino_cmap ||
            params.color_pipeline.grading != ColorGradingPreset::escape_default ||
            !directControlDirty ||
            !directControlInteraction.interacted ||
            directControlInteraction.has_active_item ||
            !NearlyEqual(shapeScaleParam->number_value, 2.25) ||
            HasColorPipelineDraftEdits(explainoRenderWindowState)) {
            std::cerr << "Expected released numeric edits to keep the live runtime and draft aligned after the last drag step\n";
            return 1;
        }

        BeginFrame();
        if (!SelectColorPipelineLaneFunction(&openWindowState, 2, "banded_heatmap")) {
            std::cerr << "Expected advanced color pipeline lane changes to work after the window has rendered\n";
            return 1;
        }
        if (!RenderColorPipelineWindow(&openWindowState, view.fractal_type, &params)) {
            std::cerr << "Expected the advanced color pipeline editor to keep rendering after a lane function change\n";
            return 1;
        }
        if (!HasColorPipelineDraftEdits(openWindowState) ||
            openWindowState.lanes[0].rows[0].function_id != "banded_signal" ||
            openWindowState.lanes[2].rows[0].function_id != "banded_heatmap" ||
            openWindowState.lanes[2].rows[0].parameter_values.size() != 4 ||
            openWindowState.lanes[2].rows[0].parameter_values[0].path != "palette.band_emphasis" ||
            openWindowState.lanes[2].rows[0].parameter_values[2].path != "palette.blend_weight" ||
            openWindowState.lanes[2].rows[0].parameter_values[3].path != "palette.blend_mode" ||
            params.color_pipeline.signal != ColorSignal::smooth_escape ||
            params.color_pipeline.palette != ColorPalette::explaino_cmap ||
            params.color_pipeline.grading != ColorGradingPreset::escape_default) {
            std::cerr << "Expected advanced color pipeline renders to honor switched draft descriptors without mutating the current live explaino tuple\n";
            return 1;
        }
        EndFrame();

        openWindowState.lanes[2].rows[0].function_id = "not_real";
        ClearColorPipelineValidationMessages(&openWindowState);
        BeginFrame();
        if (!RenderColorPipelineWindow(&openWindowState, view.fractal_type, &params)) {
            std::cerr << "Expected the advanced color pipeline window to keep rendering even when a lane function id is invalid\n";
            return 1;
        }
        EndFrame();
        bool foundUnknownFunctionMessage = false;
        for (const std::string& message : openWindowState.validation_messages) {
            if (message.find("Unknown advanced color function 'not_real'") != std::string::npos) {
                foundUnknownFunctionMessage = true;
                break;
            }
        }
        if (!foundUnknownFunctionMessage) {
            std::cerr << "Expected invalid advanced color lane function ids to surface an explicit validation message\n";
            return 1;
        }
        openWindowState.lanes[2].rows[0].function_id = "phase_wheel_palette";
        ClearColorPipelineValidationMessages(&openWindowState);
    }

    {
        std::uint64_t nextRowId = 1;
        std::uint64_t missingRowId = 0;
        if (!EnsureImGuiStackEditorRowId(&missingRowId, &nextRowId)) {
            std::cerr << "Expected missing row ids to be assigned through the shared stack-editor helper\n";
            return 1;
        }
        if (missingRowId != 1 || nextRowId != 2) {
            std::cerr << "Expected the first assigned row id to consume the next shared stack-editor id\n";
            return 1;
        }
        if (!EnsureImGuiStackEditorRowId(&missingRowId, &nextRowId)) {
            std::cerr << "Expected already-assigned row ids to remain valid when rechecked\n";
            return 1;
        }
        if (missingRowId != 1 || nextRowId != 2) {
            std::cerr << "Expected existing row ids to stay stable when the helper rechecks them\n";
            return 1;
        }

        std::uint64_t existingRowId = 7;
        if (!EnsureImGuiStackEditorRowId(&existingRowId, &nextRowId)) {
            std::cerr << "Expected the shared helper to accept pre-existing stable row ids\n";
            return 1;
        }
        if (existingRowId != 7 || nextRowId != 8) {
            std::cerr << "Expected the shared helper to advance the next id past any reused stable row id\n";
            return 1;
        }
    }

    {
        ImGuiTestContext imgui;
        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        LensSettings lens{};
        BindingContext ctx = MakeBindingContext(&view, &params, &render, &lens);

        BeginFrame();
        std::vector<std::string> emptyValidationMessages;
        if (RenderImGuiStackEditorValidationBox("Validation", emptyValidationMessages)) {
            std::cerr << "Expected the shared stack-editor validation box to stay hidden when no messages exist\n";
            return 1;
        }
        EndFrame();

        BeginFrame();
        std::vector<std::string> validationMessages{
            "Source lane needs at least one enabled row.",
            "Palette lane has a duplicate function."
        };
        if (!RenderImGuiStackEditorValidationBox("Validation", validationMessages)) {
            std::cerr << "Expected the shared stack-editor validation box to render when messages exist\n";
            return 1;
        }
        EndFrame();

        BeginFrame();
        bool rowEnabled = true;
        ImGuiStackEditorRowChromeSpec rowSpec;
        rowSpec.tree_node_id = "binding";
        rowSpec.header_label = "mean -> fractal.params.exposure";
        rowSpec.stable_row_id = 42;
        rowSpec.enabled = &rowEnabled;
        ImGuiStackEditorRowChromeResult rowChrome = RenderImGuiStackEditorRowChrome(rowSpec);
        if (!rowChrome.open) {
            std::cerr << "Expected shared stack-editor rows to open by default when first rendered\n";
            return 1;
        }
        if (rowChrome.changed || rowChrome.remove_requested || rowChrome.move_up_requested || rowChrome.move_down_requested) {
            std::cerr << "Expected idle shared stack-editor row chrome to report no actions without clicks\n";
            return 1;
        }
        if (!rowEnabled) {
            std::cerr << "Expected idle shared stack-editor row chrome to preserve the enabled flag\n";
            return 1;
        }
        EndFrame();

        BeginFrame();
        UISchemaControl unbound;
        unbound.id = "unbound";
        unbound.type = "checkbox";
        unbound.label = "Unbound";
        if (RenderControlFromSchema(unbound, ctx, nullptr, nullptr, nullptr)) {
            std::cerr << "Unbound controls should render as diagnostics and report no change\n";
            return 1;
        }
        EndFrame();

        BeginFrame();
        UISchemaControl badAction = MakeBoundControl("bad_action", "button", "Bad Action", "", "param", "fractal.view.auto_refresh");
        if (RenderControlFromSchema(badAction, ctx, nullptr, nullptr, nullptr)) {
            std::cerr << "Buttons with non-action bindings should fail closed\n";
            return 1;
        }
        EndFrame();

        BeginFrame();
        bool renderOnce = false;
        bool interacted = false;
        UISchemaControl renderOnceButton = MakeBoundControl("render_once", "button", "Render Once", "", "action", "fractal.actions.render_once");
        if (!RenderControlFromSchema(renderOnceButton, ctx, nullptr, &renderOnce, &interacted)) {
            std::cerr << "Valid action buttons should render successfully even without a click\n";
            return 1;
        }
        if (renderOnce || interacted) {
            std::cerr << "Idle action buttons should not mark render-once or interaction flags\n";
            return 1;
        }
        EndFrame();

        BeginFrame();
        std::string notedControlId;
        ctx.note_ui_automation_rect = &NoteTestUiAutomationControlId;
        ctx.ui_automation_user_data = &notedControlId;
        UISchemaControl sharedAxisFloat = MakeBoundControl(
            "ripple_amplitude",
            "slider_float",
            "Ripple",
            "float",
            "param",
            "fractal.params.ripple_amplitude");
        sharedAxisFloat.has_min = true;
        sharedAxisFloat.min = 0.0;
        sharedAxisFloat.has_max = true;
        sharedAxisFloat.max = 1.0;
        if (RenderControlFromSchema(sharedAxisFloat, ctx, nullptr, nullptr, nullptr)) {
            std::cerr << "Expected idle schema-driven float controls to render without reporting a data change\n";
            return 1;
        }
        if (notedControlId != "fractal_control.ripple_amplitude.primary") {
            std::cerr << "Expected schema-driven float controls to publish their primary UI automation id during render\n";
            return 1;
        }
        EndFrame();

        BeginFrame();
        bool automationDirty = false;
        bool automationInteracted = false;
        bool automationConsumed = false;
        std::string automationError;
        const std::string automationControlId = "fractal_control.ripple_amplitude.primary";
        ctx.ui_automation_set_control_id = &automationControlId;
        ctx.ui_automation_set_control_value = 0.375;
        ctx.ui_automation_set_consumed = &automationConsumed;
        ctx.ui_automation_set_error = &automationError;
        if (!RenderControlFromSchema(sharedAxisFloat, ctx, &automationDirty, nullptr, &automationInteracted)) {
            std::cerr << "Visible schema-driven set-value automation should apply through the float edit path\n";
            return 1;
        }
        if (!automationConsumed || !automationDirty || !automationInteracted || !automationError.empty() || !NearlyEqual(params.ripple_amplitude, 0.375)) {
            std::cerr << "Schema-driven set-value automation should consume, dirty, interact, and write ripple_amplitude\n";
            return 1;
        }
        ctx.ui_automation_set_control_id = nullptr;
        ctx.ui_automation_set_consumed = nullptr;
        ctx.ui_automation_set_error = nullptr;
        EndFrame();

        BeginFrame();
        bool boolAutomationDirty = false;
        bool boolAutomationInteracted = false;
        bool boolAutomationConsumed = false;
        std::string boolAutomationError;
        const std::string boolAutomationControlId = "fractal_control.lens_enabled.primary";
        UISchemaControl lensEnabledControl = MakeBoundControl(
            "lens_enabled",
            "checkbox",
            "Lens Enabled",
            "bool",
            "param",
            "fractal.lens.enabled");
        ctx.ui_automation_set_control_id = &boolAutomationControlId;
        ctx.ui_automation_set_control_value = 1.0;
        ctx.ui_automation_set_consumed = &boolAutomationConsumed;
        ctx.ui_automation_set_error = &boolAutomationError;
        if (!RenderControlFromSchema(lensEnabledControl, ctx, &boolAutomationDirty, nullptr, &boolAutomationInteracted)) {
            std::cerr << "Visible schema-driven set-value automation should apply through the bool edit path\n";
            return 1;
        }
        if (!boolAutomationConsumed || !boolAutomationDirty || !boolAutomationInteracted || !boolAutomationError.empty() || !lens.enabled) {
            std::cerr << "Schema-driven bool set-value automation should consume, dirty, interact, and write lens.enabled\n";
            return 1;
        }
        ctx.ui_automation_set_control_id = nullptr;
        ctx.ui_automation_set_consumed = nullptr;
        ctx.ui_automation_set_error = nullptr;
        EndFrame();

        UISchemaPredicate lensDownsampleApplicable;
        lensDownsampleApplicable.op = "eq";
        lensDownsampleApplicable.path = "fractal.lens.downsample_applicable";
        lensDownsampleApplicable.value = "true";
        lens.enabled = false;
        params.color_source_stack_count = 0;
        params.color_pipeline.signal = ColorSignal::smooth_escape;
        if (ctx.EvalVisibleIf(lensDownsampleApplicable)) {
            std::cerr << "Lens Downsample applicability should be false when neither Lens nor SDF Source rows use the field\n";
            return 1;
        }
        lens.enabled = true;
        if (!ctx.EvalVisibleIf(lensDownsampleApplicable)) {
            std::cerr << "Lens Downsample must be applicable when Lens visualization is enabled\n";
            return 1;
        }
        lens.enabled = false;
        params.color_pipeline.signal = ColorSignal::sdf_signed_distance;
        if (!ctx.EvalVisibleIf(lensDownsampleApplicable)) {
            std::cerr << "Lens Downsample must be applicable when the active Color Pipeline signal uses SDF\n";
            return 1;
        }
        params.color_pipeline.signal = ColorSignal::smooth_escape;
        params.color_source_stack_count = 1;
        params.color_source_stack[0].signal = ColorSignal::sdf_boundary_band;
        if (!ctx.EvalVisibleIf(lensDownsampleApplicable)) {
            std::cerr << "Lens Downsample must be applicable when Source stack rows use SDF\n";
            return 1;
        }
        params.color_source_stack_count = 0;
        lens.sdf_overlay_mode = LensSdfOverlayMode::field_debug;
        if (!ctx.EvalVisibleIf(lensDownsampleApplicable)) {
            std::cerr << "Lens Downsample must be applicable when the normal viewport SDF overlay uses the field\n";
            return 1;
        }
        UISchemaPredicate lensOverlayActive;
        lensOverlayActive.op = "eq";
        lensOverlayActive.path = "fractal.lens.sdf_overlay_active";
        lensOverlayActive.value = "true";
        if (!ctx.EvalVisibleIf(lensOverlayActive)) {
            std::cerr << "Lens overlay active predicate must become true for non-off overlay modes\n";
            return 1;
        }

        BeginFrame();
        bool downsampleAutomationDirty = false;
        bool downsampleAutomationInteracted = false;
        bool downsampleAutomationConsumed = false;
        std::string downsampleAutomationError;
        const std::string downsampleControlId = "fractal_control.lens_downsample.primary";
        UISchemaControl lensDownsampleControl = MakeBoundControl(
            "lens_downsample",
            "combo",
            "Lens Downsample",
            "int",
            "param",
            "fractal.lens.downsample");
        lensDownsampleControl.has_visible_if = true;
        lensDownsampleControl.visible_if = lensDownsampleApplicable;
        lensDownsampleControl.options = {
            {"1", "1x (Full)", ""},
            {"2", "2x", ""},
            {"4", "4x", ""},
            {"8", "8x", ""},
            {"16", "16x", ""},
        };
        lens.downsample = 2;
        ctx.ui_automation_set_control_id = &downsampleControlId;
        ctx.ui_automation_set_control_value = 4.0;
        ctx.ui_automation_set_consumed = &downsampleAutomationConsumed;
        ctx.ui_automation_set_error = &downsampleAutomationError;
        if (!RenderControlFromSchema(lensDownsampleControl, ctx, &downsampleAutomationDirty, nullptr, &downsampleAutomationInteracted)) {
            std::cerr << "Visible Lens Downsample combo set-value automation should apply through the int option edit path\n";
            return 1;
        }
        if (!downsampleAutomationConsumed || !downsampleAutomationDirty || !downsampleAutomationInteracted || !downsampleAutomationError.empty() || lens.downsample != 4) {
            std::cerr << "Lens Downsample combo set-value automation should consume, dirty, interact, and write lens.downsample\n";
            return 1;
        }
        ctx.ui_automation_set_control_id = nullptr;
        ctx.ui_automation_set_consumed = nullptr;
        ctx.ui_automation_set_error = nullptr;
        EndFrame();
        BeginFrame();
        bool novaAlphaDirty = false;
        bool novaAlphaInteracted = false;
        bool novaAlphaConsumed = false;
        std::string novaAlphaError;
        const std::string novaAlphaControlId = "fractal_control.nova_alpha.primary";
        UISchemaControl novaAlphaCapControl = MakeBoundControl(
            "nova_alpha",
            "slider_float",
            "Nova Alpha",
            "float",
            "param",
            "fractal.params.nova_alpha");
        novaAlphaCapControl.has_min = true;
        novaAlphaCapControl.min = 0.01;
        novaAlphaCapControl.has_max = true;
        novaAlphaCapControl.max = 2.0;
        params.nova_alpha = 0.5f;
        ctx.ui_automation_set_control_id = &novaAlphaControlId;
        ctx.ui_automation_set_control_value = 3.0;
        ctx.ui_automation_set_consumed = &novaAlphaConsumed;
        ctx.ui_automation_set_error = &novaAlphaError;
        if (!RenderControlFromSchema(novaAlphaCapControl, ctx, &novaAlphaDirty, nullptr, &novaAlphaInteracted)) {
            std::cerr << "Visible Nova Alpha set-value automation should apply through the float edit path\n";
            return 1;
        }
        if (!novaAlphaConsumed || !novaAlphaDirty || !novaAlphaInteracted || !novaAlphaError.empty() ||
            !NearlyEqual(params.nova_alpha, 2.0)) {
            std::cerr << "Nova Alpha set-value automation should clamp over-cap values to 2.0\n";
            return 1;
        }
        ctx.ui_automation_set_control_id = nullptr;
        ctx.ui_automation_set_consumed = nullptr;
        ctx.ui_automation_set_error = nullptr;
        EndFrame();

        BeginFrame();
        bool intAutomationDirty = false;
        bool intAutomationInteracted = false;
        bool intAutomationConsumed = false;
        std::string intAutomationError;
        const std::string intAutomationControlId = "fractal_control.width.primary";
        UISchemaControl widthControl = MakeBoundControl(
            "width",
            "slider_int",
            "Width",
            "int",
            "param",
            "fractal.render.resolution.x");
        widthControl.has_min = true;
        widthControl.min = 16.0;
        widthControl.has_max = true;
        widthControl.max = 4096.0;
        render.resolution.x = 320;
        ctx.ui_automation_set_control_id = &intAutomationControlId;
        ctx.ui_automation_set_control_value = 640.0;
        ctx.ui_automation_set_consumed = &intAutomationConsumed;
        ctx.ui_automation_set_error = &intAutomationError;
        if (!RenderControlFromSchema(widthControl, ctx, &intAutomationDirty, nullptr, &intAutomationInteracted)) {
            std::cerr << "Visible schema-driven set-value automation should apply through the int edit path\n";
            return 1;
        }
        if (!intAutomationConsumed || !intAutomationDirty || !intAutomationInteracted || !intAutomationError.empty() || render.resolution.x != 640) {
            std::cerr << "Schema-driven int set-value automation should consume, dirty, interact, and write width\n";
            return 1;
        }
        ctx.ui_automation_set_control_id = nullptr;
        ctx.ui_automation_set_consumed = nullptr;
        ctx.ui_automation_set_error = nullptr;
        EndFrame();

        BeginFrame();
        bool longEdgeAutomationDirty = false;
        bool longEdgeAutomationInteracted = false;
        bool longEdgeAutomationConsumed = false;
        std::string longEdgeAutomationError;
        const std::string longEdgeAutomationControlId = "fractal_control.resolution_long_edge.primary";
        UISchemaControl longEdgeControl = MakeBoundControl(
            "resolution_long_edge",
            "slider_int",
            "Long Edge",
            "int",
            "param",
            "fractal.render.resolution.long_edge");
        longEdgeControl.has_min = true;
        longEdgeControl.min = 256.0;
        longEdgeControl.has_max = true;
        longEdgeControl.max = 4096.0;
        longEdgeControl.has_step = true;
        longEdgeControl.step = 16.0;
        render.resolution = {2048, 1152};
        ctx.ui_automation_set_control_id = &longEdgeAutomationControlId;
        ctx.ui_automation_set_control_value = 1280.0;
        ctx.ui_automation_set_consumed = &longEdgeAutomationConsumed;
        ctx.ui_automation_set_error = &longEdgeAutomationError;
        if (!RenderControlFromSchema(longEdgeControl, ctx, &longEdgeAutomationDirty, nullptr, &longEdgeAutomationInteracted)) {
            std::cerr << "Visible schema-driven long-edge set-value automation should apply through the computed int edit path\n";
            return 1;
        }
        if (!longEdgeAutomationConsumed || !longEdgeAutomationDirty || !longEdgeAutomationInteracted || !longEdgeAutomationError.empty() ||
            render.resolution.x != 1280 || render.resolution.y != 720) {
            std::cerr << "Computed long-edge automation should consume, dirty, interact, and write resolution.x/y\n";
            return 1;
        }
        ctx.ui_automation_set_control_id = nullptr;
        ctx.ui_automation_set_consumed = nullptr;
        ctx.ui_automation_set_error = nullptr;
        EndFrame();

        BeginFrame();
        bool doubleAutomationDirty = false;
        bool doubleAutomationInteracted = false;
        bool doubleAutomationConsumed = false;
        std::string doubleAutomationError;
        const std::string doubleAutomationControlId = "fractal_control.explaino_seed_b.primary";
        UISchemaControl seedBControl = MakeBoundControl(
            "explaino_seed_b",
            "slider_double",
            "Explaino Seed B",
            "double",
            "param",
            "fractal.params.explaino_seed_b");
        seedBControl.has_ui_min = true;
        seedBControl.ui_min = -10.0;
        seedBControl.has_ui_max = true;
        seedBControl.ui_max = 10.0;
        params.explaino_seed_b = 1.0;
        ctx.ui_automation_set_control_id = &doubleAutomationControlId;
        ctx.ui_automation_set_control_value = 2.25;
        ctx.ui_automation_set_consumed = &doubleAutomationConsumed;
        ctx.ui_automation_set_error = &doubleAutomationError;
        if (!RenderControlFromSchema(seedBControl, ctx, &doubleAutomationDirty, nullptr, &doubleAutomationInteracted)) {
            std::cerr << "Visible schema-driven set-value automation should apply through the double edit path\n";
            return 1;
        }
        if (!doubleAutomationConsumed || !doubleAutomationDirty || !doubleAutomationInteracted || !doubleAutomationError.empty() || !NearlyEqual(params.explaino_seed_b, 2.25)) {
            std::cerr << "Schema-driven double set-value automation should consume, dirty, interact, and write explaino_seed_b\n";
            return 1;
        }
        ctx.ui_automation_set_control_id = nullptr;
        ctx.ui_automation_set_consumed = nullptr;
        ctx.ui_automation_set_error = nullptr;
        EndFrame();

        BeginFrame();
        bool combinedSeedAutomationDirty = false;
        bool combinedSeedAutomationInteracted = false;
        bool combinedSeedAutomationConsumed = false;
        std::string combinedSeedAutomationError;
        const std::string combinedSeedAutomationControlId = "fractal_control.explaino_seed.primary";
        UISchemaControl combinedSeedControl = MakeBoundControl(
            "explaino_seed",
            "slider_double",
            "Explaino Seed",
            "double",
            "param",
            "fractal.params.explaino_seed");
        combinedSeedControl.has_ui_min = true;
        combinedSeedControl.ui_min = -10.0;
        combinedSeedControl.has_ui_max = true;
        combinedSeedControl.ui_max = 10.0;
        ExplainoSeedSetCombined(view, params, 0.0);
        ctx.ui_automation_set_control_id = &combinedSeedAutomationControlId;
        ctx.ui_automation_set_control_value = 3.5;
        ctx.ui_automation_set_consumed = &combinedSeedAutomationConsumed;
        ctx.ui_automation_set_error = &combinedSeedAutomationError;
        if (!RenderControlFromSchema(combinedSeedControl, ctx, &combinedSeedAutomationDirty, nullptr, &combinedSeedAutomationInteracted)) {
            std::cerr << "Visible schema-driven set-value automation should apply through the combined Explaino seed double path\n";
            return 1;
        }
        if (!combinedSeedAutomationConsumed || !combinedSeedAutomationDirty || !combinedSeedAutomationInteracted ||
            !combinedSeedAutomationError.empty() || !NearlyEqual(ExplainoSeedCombined(view, params), 3.5)) {
            std::cerr << "Schema-driven combined Explaino seed set-value automation should consume, dirty, interact, and write the combined seed\n";
            return 1;
        }
        ctx.ui_automation_set_control_id = nullptr;
        ctx.ui_automation_set_consumed = nullptr;
        ctx.ui_automation_set_error = nullptr;
        EndFrame();

        BeginFrame();
        UISchemaControl badCheckbox = MakeBoundControl("bad_checkbox", "checkbox", "Bad Checkbox", "bool", "param", "fractal.params.not_a_real_bool");
        if (RenderControlFromSchema(badCheckbox, ctx, nullptr, nullptr, nullptr)) {
            std::cerr << "Checkboxes with invalid binding paths should fail closed\n";
            return 1;
        }
        EndFrame();

        BeginFrame();
        UISchemaControl unsupported = MakeBoundControl("unsupported", "mystery_control", "Unsupported", "float", "param", "fractal.params.exposure");
        if (RenderControlFromSchema(unsupported, ctx, nullptr, nullptr, nullptr)) {
            std::cerr << "Unsupported control types should fail closed\n";
            return 1;
        }
        EndFrame();
    }

    std::cout << "test_schema_binding: all passed\n";
    return 0;
}
