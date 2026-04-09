// Headless test for generic param animation resolution.
// Verifies that every param_anim_target option in the schema dropdown
// resolves to a valid float pointer via BindFloat, without needing
// per-target enum wiring.

#include "../src/fractal_types.h"
#include "../src/schema_binding.h"
#include "../src/param_anim_dynamics.h"

#include <cstdio>
#include <cstring>
#include <cmath>

static int g_fail_count = 0;

#define CHECK(cond, msg, ...) do { \
    if (!(cond)) { \
        std::printf("FAIL: " msg "\n", ##__VA_ARGS__); \
        ++g_fail_count; \
    } \
} while(0)

// All animatable targets from the schema dropdown (excluding "none" and "seed"
// which are special-cased). Each must resolve via BindFloat.
static const char* kFloatTargets[] = {
    "damping",
    "warp_strength",
    "root_spread",
    "mix",
    "nova_alpha",
    "phoenix_p_real",
    "multibrot_power",
    "lambda_real",
    "momentum_beta",
    "joy_coupling",
    "fold_coupling",
    "bell_coupling",
    "ripple_amplitude",
    "splice_offset",
    "explaino_phase",
};
static constexpr int kNumFloatTargets = sizeof(kFloatTargets) / sizeof(kFloatTargets[0]);

static void test_all_targets_resolve() {
    std::printf("  test_all_targets_resolve...\n");

    ViewState view{};
    KernelParams params{};
    BindingContext ctx;
    ctx.view = &view;
    ctx.params = &params;

    for (int i = 0; i < kNumFloatTargets; ++i) {
        const char* name = kFloatTargets[i];
        float* ptr = nullptr;
        bool found = false;

        std::string paramPath = std::string("fractal.params.") + name;
        if (ctx.BindFloat(paramPath, &ptr) && ptr) { found = true; }
        else {
            std::string viewPath = std::string("fractal.view.") + name;
            if (ctx.BindFloat(viewPath, &ptr) && ptr) { found = true; }
        }

        CHECK(found, "target '%s' did not resolve via BindFloat", name);
    }
}

static void test_all_targets_distinct() {
    std::printf("  test_all_targets_distinct...\n");

    ViewState view{};
    KernelParams params{};
    BindingContext ctx;
    ctx.view = &view;
    ctx.params = &params;

    float* ptrs[kNumFloatTargets]{};
    for (int i = 0; i < kNumFloatTargets; ++i) {
        const char* name = kFloatTargets[i];
        float* ptr = nullptr;
        std::string paramPath = std::string("fractal.params.") + name;
        if (!ctx.BindFloat(paramPath, &ptr) || !ptr) {
            std::string viewPath = std::string("fractal.view.") + name;
            ctx.BindFloat(viewPath, &ptr);
        }
        ptrs[i] = ptr;
    }

    for (int i = 0; i < kNumFloatTargets; ++i) {
        for (int j = i + 1; j < kNumFloatTargets; ++j) {
            if (ptrs[i] && ptrs[j]) {
                CHECK(ptrs[i] != ptrs[j],
                    "targets '%s' and '%s' resolve to the same pointer",
                    kFloatTargets[i], kFloatTargets[j]);
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

    for (int i = 0; i < kNumFloatTargets; ++i) {
        bool ok = ctx.SetEnumId("fractal.view.param_anim_target", kFloatTargets[i]);
        CHECK(ok, "SetEnumId failed for '%s'", kFloatTargets[i]);
        std::string got = ctx.GetEnumId("fractal.view.param_anim_target");
        CHECK(got == kFloatTargets[i],
            "GetEnumId returned '%s', expected '%s'", got.c_str(), kFloatTargets[i]);
    }
}

int main() {
    std::printf("param_anim_generic tests:\n");

    test_all_targets_resolve();
    test_all_targets_distinct();
    test_apply_increments_target();
    test_none_and_unknown_noop();
    test_set_enum_stores_string();

    if (g_fail_count == 0) {
        std::printf("param_anim_generic: all tests passed.\n");
    } else {
        std::printf("param_anim_generic: %d FAILURES\n", g_fail_count);
    }
    return g_fail_count;
}
