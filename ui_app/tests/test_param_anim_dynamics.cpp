#include "../src/param_anim_dynamics.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__); \
        g_fail = 1; \
    } \
} while(0)

static void set_target(ViewState& view, const char* name) {
    std::strncpy(view.param_anim_target, name, sizeof(view.param_anim_target) - 1);
    view.param_anim_target[sizeof(view.param_anim_target) - 1] = '\0';
}

static void test_none_does_nothing() {
    ViewState view{};
    KernelParams params{};
    set_target(view, "none");
    view.param_anim_rate = 1.0f;
    float before = params.explaino_damping;
    bool changed = ApplyParamAnimDynamics(0.016, view, params);
    CHECK(!changed, "none target should not change anything");
    CHECK(params.explaino_damping == before, "damping should not change with none target");
}

static void test_damping_increments() {
    ViewState view{};
    KernelParams params{};
    params.explaino_damping = 1.0f;
    set_target(view, "damping");
    view.param_anim_rate = 0.5f;
    bool changed = ApplyParamAnimDynamics(1.0, view, params);
    CHECK(changed, "damping should have changed");
    CHECK(fabs(params.explaino_damping - 1.5f) < 1e-5f, "damping should be 1.0 + 0.5*1.0 = 1.5");
}

static void test_multibrot_power_increments() {
    ViewState view{};
    KernelParams params{};
    params.multibrot_power_float = 3.0f;
    set_target(view, "multibrot_power");
    view.param_anim_rate = 0.1f;
    bool changed = ApplyParamAnimDynamics(2.0, view, params);
    CHECK(changed, "multibrot_power should have changed");
    CHECK(fabs(params.multibrot_power_float - 3.2f) < 1e-5f, "multibrot_power should be 3.0 + 0.1*2.0 = 3.2");
}

static void test_seed_target_uses_explaino_combined() {
    ViewState view{};
    view.fractal_type = FractalType::explaino;
    KernelParams params{};
    params.explaino_seed = 0.0;
    view.explaino_seed_drift = 0.0f;
    set_target(view, "seed");
    view.param_anim_rate = 0.1f;
    bool changed = ApplyParamAnimDynamics(1.0, view, params);
    CHECK(changed, "seed target should have changed");
    // Combined seed should be ~0.1
    double combined = params.explaino_seed + (double)view.explaino_seed_drift;
    CHECK(fabs(combined - 0.1) < 1e-5, "combined seed should be ~0.1");
}

static void test_seed_target_non_explaino_noop() {
    ViewState view{};
    view.fractal_type = FractalType::mandelbrot;
    KernelParams params{};
    set_target(view, "seed");
    view.param_anim_rate = 1.0f;
    bool changed = ApplyParamAnimDynamics(1.0, view, params);
    CHECK(!changed, "seed target on non-explaino should be noop");
}

static void test_zero_rate_noop() {
    ViewState view{};
    KernelParams params{};
    params.nova_alpha = 0.5f;
    set_target(view, "nova_alpha");
    view.param_anim_rate = 0.0f;
    bool changed = ApplyParamAnimDynamics(1.0, view, params);
    CHECK(!changed, "zero rate should be noop");
    CHECK(params.nova_alpha == 0.5f, "nova_alpha should not change with zero rate");
}

static void test_negative_delta_noop() {
    ViewState view{};
    KernelParams params{};
    set_target(view, "damping");
    view.param_anim_rate = 1.0f;
    bool changed = ApplyParamAnimDynamics(-0.5, view, params);
    CHECK(!changed, "negative delta should be noop");
}

int main() {
    test_none_does_nothing();
    test_damping_increments();
    test_multibrot_power_increments();
    test_seed_target_uses_explaino_combined();
    test_seed_target_non_explaino_noop();
    test_zero_rate_noop();
    test_negative_delta_noop();

    if (g_fail) {
        fprintf(stderr, "param_anim_dynamics tests FAILED\n");
        return 1;
    }
    printf("param_anim_dynamics tests OK\n");
    return 0;
}
