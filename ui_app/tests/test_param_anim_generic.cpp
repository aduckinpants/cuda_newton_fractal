// Headless test for generic param animation resolution.
// Verifies that every param_anim_target option in the schema dropdown
// resolves to a valid float pointer via BindFloat, without needing
// per-target enum wiring.

#include "../src/fractal_types.h"
#include "../src/schema_binding.h"
#include "../src/param_anim_dynamics.h"
#include "../src/json_min.h"
#include "../src/ui_schema.h"

#include <cstdio>
#include <cstring>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

static int g_fail_count = 0;

#define CHECK(cond, msg, ...) do { \
    if (!(cond)) { \
        std::printf("FAIL: " msg "\n", ##__VA_ARGS__); \
        ++g_fail_count; \
    } \
} while(0)

static std::filesystem::path FindRepoRoot() {
    std::filesystem::path cur = std::filesystem::current_path();
    for (int index = 0; index < 8; ++index) {
        if (std::filesystem::exists(cur / "ui" / "fractal_binding_surface_v1.ui_schema.json") &&
            std::filesystem::exists(cur / "ui_app" / "src" / "ui_schema.cpp")) {
            return cur;
        }
        if (!cur.has_parent_path() || cur.parent_path() == cur) break;
        cur = cur.parent_path();
    }
    return {};
}

static bool ReadTextFile(const std::filesystem::path& path, std::string* outText) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    if (!file) return false;
    std::ostringstream ss;
    ss << file.rdbuf();
    *outText = ss.str();
    return true;
}

static const UISchemaControl* FindControlById(const UISchema& schema, const char* id) {
    for (const auto& panel : schema.panels) {
        for (const auto& control : panel.controls) {
            if (control.id == id) return &control;
        }
    }
    return nullptr;
}

static const UISchemaControl* LoadParamAnimTargetControl(UISchema* outSchema) {
    const std::filesystem::path repoRoot = FindRepoRoot();
    CHECK(!repoRoot.empty(), "could not find repo root");
    if (repoRoot.empty()) return nullptr;

    std::string text;
    const std::filesystem::path schemaPath = repoRoot / "ui" / "fractal_binding_surface_v1.ui_schema.json";
    CHECK(ReadTextFile(schemaPath, &text), "could not read schema file");
    if (text.empty()) return nullptr;

    json_min::ParseResult parse = json_min::Parse(text);
    CHECK(parse.error.empty(), "schema parse failed: %s", parse.error.c_str());
    if (!parse.error.empty()) return nullptr;

    UISchemaLoadResult load = LoadUISchemaFromJson(parse.value);
    CHECK(load.error.empty(), "schema load failed: %s", load.error.c_str());
    if (!load.error.empty()) return nullptr;

    *outSchema = std::move(load.schema);
    const UISchemaControl* control = FindControlById(*outSchema, "param_anim_target");
    CHECK(control != nullptr, "param_anim_target control missing from schema");
    return control;
}

static bool HasVisibleOption(const std::vector<const UISchemaOption*>& options, const char* id) {
    for (const UISchemaOption* option : options) {
        if (option && option->id == id) return true;
    }
    return false;
}

static std::vector<std::string> LoadSchemaFloatTargetIds() {
    UISchema schema;
    const UISchemaControl* control = LoadParamAnimTargetControl(&schema);
    if (!control) return {};

    std::vector<std::string> ids;
    for (const UISchemaOption& option : control->options) {
        if (option.id == "none" || option.id == "seed") continue;
        ids.push_back(option.id);
    }
    return ids;
}

static bool ResolveFloatTarget(const char* name, BindingContext& ctx, float** outPtr) {
    if (!name || !outPtr) return false;
    *outPtr = nullptr;

    std::string paramPath = std::string("fractal.params.") + name;
    if (ctx.BindFloat(paramPath, outPtr) && *outPtr) return true;

    std::string viewPath = std::string("fractal.view.") + name;
    if (ctx.BindFloat(viewPath, outPtr) && *outPtr) return true;

    return false;
}

static void test_all_targets_resolve() {
    std::printf("  test_all_targets_resolve...\n");

    ViewState view{};
    KernelParams params{};
    BindingContext ctx;
    ctx.view = &view;
    ctx.params = &params;

    const std::vector<std::string> targetIds = LoadSchemaFloatTargetIds();
    CHECK(!targetIds.empty(), "schema did not expose any float animation targets");

    for (const std::string& name : targetIds) {
        float* ptr = nullptr;
        CHECK(ResolveFloatTarget(name.c_str(), ctx, &ptr), "target '%s' did not resolve via BindFloat", name.c_str());
    }
}

static void test_all_targets_distinct() {
    std::printf("  test_all_targets_distinct...\n");

    ViewState view{};
    KernelParams params{};
    BindingContext ctx;
    ctx.view = &view;
    ctx.params = &params;

    const std::vector<std::string> targetIds = LoadSchemaFloatTargetIds();
    CHECK(!targetIds.empty(), "schema did not expose any float animation targets");

    std::vector<float*> ptrs(targetIds.size(), nullptr);
    for (size_t i = 0; i < targetIds.size(); ++i) {
        ResolveFloatTarget(targetIds[i].c_str(), ctx, &ptrs[i]);
    }

    for (size_t i = 0; i < targetIds.size(); ++i) {
        for (size_t j = i + 1; j < targetIds.size(); ++j) {
            if (ptrs[i] && ptrs[j]) {
                CHECK(ptrs[i] != ptrs[j],
                    "targets '%s' and '%s' resolve to the same pointer",
                    targetIds[i].c_str(), targetIds[j].c_str());
            }
        }
    }
}

static void test_apply_increments_target() {
    std::printf("  test_apply_increments_target...\n");

    ViewState view{};
    KernelParams params{};

    // Pick a few representative targets to verify animation actually modifies the value
    struct Case { const char* name; float* field; };
    Case cases[] = {
        {"splice_offset", &params.splice_offset},
        {"ripple_amplitude", &params.ripple_amplitude},
        {"joy_coupling", &params.joy_coupling},
        {"magnet_relaxation", &params.magnet_relaxation},
        {"vortex_strength", &params.vortex_strength},
        {"tension_strength", &params.tension_strength},
        {"damping", &params.explaino_damping},
    };

    for (auto& c : cases) {
        float before = *c.field;
        std::strncpy(view.param_anim_target, c.name, sizeof(view.param_anim_target) - 1);
        view.param_anim_target[sizeof(view.param_anim_target) - 1] = '\0';
        view.param_anim_rate = 1.0f;

        bool changed = ApplyParamAnimDynamics(1.0, view, params);
        float after = *c.field;

        CHECK(changed, "ApplyParamAnimDynamics returned false for '%s'", c.name);
        CHECK(std::fabs(after - before - 1.0f) < 0.001f,
            "'%s': expected delta ~1.0, got %f", c.name, after - before);
    }
}

static void test_schema_filters_targets_by_current_fractal() {
    std::printf("  test_schema_filters_targets_by_current_fractal...\n");

    UISchema schema;
    const UISchemaControl* control = LoadParamAnimTargetControl(&schema);
    if (!control) return;

    ViewState view{};
    KernelParams params{};
    BindingContext ctx;
    ctx.view = &view;
    ctx.params = &params;

    view.fractal_type = FractalType::mandelbrot;
    std::vector<const UISchemaOption*> mandelbrotOptions = ResolveVisibleEnumOptions(*control, ctx);
    CHECK(HasVisibleOption(mandelbrotOptions, "none"), "mandelbrot should retain the None animation option");
    CHECK(!HasVisibleOption(mandelbrotOptions, "magnet_relaxation"), "mandelbrot must not show Magnet animation targets");
    CHECK(!HasVisibleOption(mandelbrotOptions, "ripple_amplitude"), "mandelbrot must not show Explaino axis animation targets");
    CHECK(!HasVisibleOption(mandelbrotOptions, "nova_alpha"), "mandelbrot must not show Nova animation targets");

    view.fractal_type = FractalType::magnet;
    std::vector<const UISchemaOption*> magnetOptions = ResolveVisibleEnumOptions(*control, ctx);
    CHECK(HasVisibleOption(magnetOptions, "none"), "magnet should retain the None animation option");
    CHECK(HasVisibleOption(magnetOptions, "magnet_relaxation"), "magnet should show Magnet Relaxation animation target");
    CHECK(HasVisibleOption(magnetOptions, "magnet_bailout"), "magnet should show Magnet Bailout animation target");
    CHECK(!HasVisibleOption(magnetOptions, "ripple_amplitude"), "magnet must not show Explaino axis animation targets");
    CHECK(!HasVisibleOption(magnetOptions, "nova_alpha"), "magnet must not show Nova animation targets");

    view.fractal_type = FractalType::explaino_all;
    std::vector<const UISchemaOption*> explainoAllOptions = ResolveVisibleEnumOptions(*control, ctx);
    CHECK(HasVisibleOption(explainoAllOptions, "ripple_amplitude"), "explaino_all should show Ripple axis animation target");
    CHECK(HasVisibleOption(explainoAllOptions, "splice_offset"), "explaino_all should show Splice axis animation target");
    CHECK(HasVisibleOption(explainoAllOptions, "vortex_strength"), "explaino_all should show Vortex axis animation target");
    CHECK(HasVisibleOption(explainoAllOptions, "tension_strength"), "explaino_all should show Tension axis animation target");

    view.fractal_type = FractalType::explaino_ripple;
    std::vector<const UISchemaOption*> rippleOptions = ResolveVisibleEnumOptions(*control, ctx);
    CHECK(HasVisibleOption(rippleOptions, "seed"), "explaino_ripple should show Explaino seed animation target");
    CHECK(HasVisibleOption(rippleOptions, "damping"), "explaino_ripple should show common Explaino damping target");
    CHECK(HasVisibleOption(rippleOptions, "ripple_amplitude"), "explaino_ripple should show its owner-axis animation target");
    CHECK(HasVisibleOption(rippleOptions, "phoenix_p_real"), "explaino_ripple should show its visible Phoenix real target");
    CHECK(!HasVisibleOption(rippleOptions, "magnet_relaxation"), "explaino_ripple must not show Magnet animation targets");
    CHECK(!HasVisibleOption(rippleOptions, "splice_offset"), "explaino_ripple must not show Splice-only targets");
}

static void test_none_and_unknown_noop() {
    std::printf("  test_none_and_unknown_noop...\n");

    ViewState view{};
    KernelParams params{};
    view.param_anim_rate = 1.0f;

    // "none" should not animate
    std::strncpy(view.param_anim_target, "none", sizeof(view.param_anim_target));
    CHECK(!ApplyParamAnimDynamics(1.0, view, params), "expected false for 'none'");

    // Unknown target should not animate
    std::strncpy(view.param_anim_target, "bogus_param", sizeof(view.param_anim_target));
    CHECK(!ApplyParamAnimDynamics(1.0, view, params), "expected false for unknown target");
}

static void test_set_enum_stores_string() {
    std::printf("  test_set_enum_stores_string...\n");

    ViewState view{};
    BindingContext ctx;
    ctx.view = &view;

    const std::vector<std::string> targetIds = LoadSchemaFloatTargetIds();
    CHECK(!targetIds.empty(), "schema did not expose any float animation targets");

    for (const std::string& targetId : targetIds) {
        bool ok = ctx.SetEnumId("fractal.view.param_anim_target", targetId);
        CHECK(ok, "SetEnumId failed for '%s'", targetId.c_str());
        std::string got = ctx.GetEnumId("fractal.view.param_anim_target");
        CHECK(got == targetId,
            "GetEnumId returned '%s', expected '%s'", got.c_str(), targetId.c_str());
    }
}

int main() {
    std::printf("param_anim_generic tests:\n");

    test_all_targets_resolve();
    test_all_targets_distinct();
    test_apply_increments_target();
    test_schema_filters_targets_by_current_fractal();
    test_none_and_unknown_noop();
    test_set_enum_stores_string();

    if (g_fail_count == 0) {
        std::printf("param_anim_generic: all tests passed.\n");
    } else {
        std::printf("param_anim_generic: %d FAILURES\n", g_fail_count);
    }
    return g_fail_count;
}
