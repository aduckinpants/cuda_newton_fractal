#include "../src/fractal_parameter_surface_descriptor.h"
#include "../src/json_min.h"
#include "../src/ui_schema.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {

std::filesystem::path FindRepoRoot() {
    std::filesystem::path cur = std::filesystem::current_path();
    for (int i = 0; i < 8; ++i) {
        if (std::filesystem::exists(cur / "ui" / "fractal_binding_surface_v1.ui_schema.json")) {
            return cur;
        }
        if (!cur.has_parent_path()) break;
        cur = cur.parent_path();
    }
    return {};
}

bool ReadTextFile(const std::filesystem::path& path, std::string* outText) {
    std::ifstream in(path, std::ios::in | std::ios::binary);
    if (!in) return false;
    std::ostringstream ss;
    ss << in.rdbuf();
    if (outText) *outText = ss.str();
    return true;
}

bool LoadCurrentSchema(UISchema* outSchema) {
    const std::filesystem::path root = FindRepoRoot();
    if (root.empty()) {
        std::cerr << "Could not find repo root\n";
        return false;
    }
    std::string schemaText;
    if (!ReadTextFile(root / "ui" / "fractal_binding_surface_v1.ui_schema.json", &schemaText)) {
        std::cerr << "Could not read current UI schema\n";
        return false;
    }
    json_min::ParseResult parsed = json_min::Parse(schemaText);
    if (!parsed.error.empty()) {
        std::cerr << "Could not parse current UI schema: " << parsed.error << "\n";
        return false;
    }
    UISchemaLoadResult loaded = LoadUISchemaFromJson(parsed.value);
    if (!loaded.error.empty()) {
        std::cerr << "Could not load current UI schema: " << loaded.error << "\n";
        return false;
    }
    if (outSchema) *outSchema = std::move(loaded.schema);
    return true;
}

const json_min::Value* FindLane(const json_min::Value& root, const std::string& fractalId) {
    const json_min::Value* lanes = root.get("lanes");
    if (!lanes || !lanes->is_array()) return nullptr;
    for (const json_min::Value& lane : lanes->as_array()) {
        const json_min::Value* id = lane.get("fractal_id");
        if (id && id->is_string() && id->as_string() == fractalId) return &lane;
    }
    return nullptr;
}

const json_min::Value* FindControlOnLane(
    const json_min::Value& root,
    const std::string& fractalId,
    const std::string& controlId) {
    const json_min::Value* lane = FindLane(root, fractalId);
    if (!lane) return nullptr;
    const json_min::Value* controls = lane->get("controls");
    if (!controls || !controls->is_array()) return nullptr;
    for (const json_min::Value& control : controls->as_array()) {
        const json_min::Value* id = control.get("control_id");
        if (id && id->is_string() && id->as_string() == controlId) return &control;
    }
    return nullptr;
}

bool JsonBoolField(const json_min::Value& object, const char* field) {
    const json_min::Value* value = object.get(field);
    return value && value->is_bool() && value->as_bool();
}

bool JsonStringFieldEquals(const json_min::Value& object, const char* field, const char* expected) {
    const json_min::Value* value = object.get(field);
    return value && value->is_string() && value->as_string() == expected;
}

bool JsonNumberField(const json_min::Value& object, const char* field, double* outValue) {
    const json_min::Value* value = object.get(field);
    if (!value || !value->is_number()) return false;
    if (outValue) *outValue = value->as_number();
    return true;
}

bool ExpectOwnerControl(
    const json_min::Value& root,
    const char* ownerLane,
    const char* controlId,
    const char* bindingPath) {
    const json_min::Value* control = FindControlOnLane(root, ownerLane, controlId);
    if (!control) {
        std::cerr << "Missing descriptor control " << controlId << " on " << ownerLane << "\n";
        return false;
    }
    if (!JsonStringFieldEquals(*control, "binding_path", bindingPath) ||
        !JsonStringFieldEquals(*control, "runtime_binding_kind", "float") ||
        !JsonStringFieldEquals(*control, "state_io_key", controlId) ||
        !JsonBoolField(*control, "binding_resolves") ||
        !JsonBoolField(*control, "has_validation_range") ||
        !JsonBoolField(*control, "animatable")) {
        std::cerr << "Descriptor control " << controlId << " on " << ownerLane << " is missing authority fields\n";
        return false;
    }
    if (FindControlOnLane(root, "explaino_all", controlId)) {
        std::cerr << "Owner-specific descriptor control leaked onto explaino_all: " << controlId << "\n";
        return false;
    }
    return true;
}

bool ExpectCommonControl(
    const json_min::Value& root,
    const char* lane,
    const char* controlId,
    const char* bindingPath,
    const char* runtimeBindingKind) {
    const json_min::Value* control = FindControlOnLane(root, lane, controlId);
    if (!control) {
        std::cerr << "Missing descriptor control " << controlId << " on " << lane << "\n";
        return false;
    }
    if (!JsonStringFieldEquals(*control, "binding_path", bindingPath) ||
        !JsonStringFieldEquals(*control, "runtime_binding_kind", runtimeBindingKind) ||
        !JsonStringFieldEquals(*control, "state_io_key", controlId) ||
        !JsonBoolField(*control, "binding_resolves") ||
        !JsonBoolField(*control, "has_validation_range") ||
        !JsonBoolField(*control, "animatable")) {
        std::cerr << "Descriptor common control " << controlId << " on " << lane << " is missing authority fields\n";
        return false;
    }
    return true;
}

} // namespace

int main() {
    UISchema schema;
    if (!LoadCurrentSchema(&schema)) return 1;

    const std::string descriptorJson = SerializeFractalParameterSurfaceDescriptorJson(schema);
    json_min::ParseResult parsed = json_min::Parse(descriptorJson);
    if (!parsed.error.empty()) {
        std::cerr << "Parameter surface descriptor JSON did not parse: " << parsed.error << "\n";
        return 1;
    }

    double version = 0.0;
    double fractalCount = 0.0;
    double visibleCells = 0.0;
    if (!JsonNumberField(parsed.value, "version", &version) || version != 1.0 ||
        !JsonNumberField(parsed.value, "fractal_count", &fractalCount) || fractalCount < 40.0 ||
        !JsonNumberField(parsed.value, "visible_family_control_cells", &visibleCells) || visibleCells < 200.0) {
        std::cerr << "Parameter surface descriptor top-level counts are not credible\n";
        return 1;
    }

    if (!FindLane(parsed.value, "newton") || !FindLane(parsed.value, "perpendicular_burning_ship")) {
        std::cerr << "Parameter surface descriptor did not visit expected fractal lanes\n";
        return 1;
    }

    if (!ExpectOwnerControl(parsed.value, "burning_ship", "burning_ship_fold_mix", "fractal.params.burning_ship_fold_mix") ||
        !ExpectOwnerControl(parsed.value, "celtic_mandelbrot", "celtic_abs_mix", "fractal.params.celtic_abs_mix") ||
        !ExpectOwnerControl(parsed.value, "perpendicular_burning_ship", "perpendicular_fold_mix", "fractal.params.perpendicular_fold_mix") ||
        !ExpectOwnerControl(parsed.value, "spider", "spider_feedback", "fractal.params.spider_feedback") ||
        !ExpectOwnerControl(parsed.value, "collatz", "collatz_transition_strength", "fractal.params.collatz_transition_strength") ||
        !ExpectOwnerControl(parsed.value, "explaino_collatz_direct", "collatz_transition_strength", "fractal.params.collatz_transition_strength")) {
        return 1;
    }

    if (!ExpectCommonControl(parsed.value, "explaino_nova", "explaino_warp_strength", "fractal.params.explaino_warp_strength", "float") ||
        !ExpectCommonControl(parsed.value, "explaino_nova", "explaino_damping", "fractal.params.explaino_damping", "float")) {
        return 1;
    }

    const json_min::Value* phoenix = FindControlOnLane(parsed.value, "phoenix", "phoenix_p_real");
    if (!phoenix || !JsonStringFieldEquals(*phoenix, "binding_path", "fractal.params.phoenix_p_real") ||
        !JsonBoolField(*phoenix, "binding_resolves")) {
        std::cerr << "Parameter surface descriptor should include Phoenix p real on the Phoenix lane\n";
        return 1;
    }

    std::cout << "test_fractal_parameter_surface_descriptor: all passed\n";
    return 0;
}
