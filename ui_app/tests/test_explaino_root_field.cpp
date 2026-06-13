#include "../src/explaino_root_field.h"

#include "../src/explaino_root_sdf_field.h"
#include "../src/fractal_derived_fields.h"
#include "../src/fractal_family_rules.h"

#include <cmath>
#include <iostream>
#include <limits>
#include <string>

namespace {

int gPass = 0;
int gFail = 0;

#define CHECK(name, cond) do { \
    if (cond) { \
        ++gPass; \
    } else { \
        ++gFail; \
        std::cerr << "  FAIL: " << (name) << " (" << __FILE__ << ":" << __LINE__ << ")\n"; \
    } \
} while (0)

bool Near(float a, float b, float tol = 1.0e-6f) {
    return std::fabs(a - b) <= tol * (std::max)(1.0f, (std::max)(std::fabs(a), std::fabs(b)));
}

float Fract(float value) {
    return value - std::floor(value);
}

void CheckRootsMatch(const KernelParams& params, const ExplainoRootFieldDescriptor& desc, int count) {
    CHECK("descriptor root count matches", desc.active_count == count);
    CHECK("descriptor max count reserves N-root capacity", desc.max_count == kExplainoRootFieldMaxRoots);
    for (int index = 0; index < count; ++index) {
        CHECK("descriptor base root x matches params", Near(desc.base_roots[index].x, params.explaino_roots[index].x));
        CHECK("descriptor base root y matches params", Near(desc.base_roots[index].y, params.explaino_roots[index].y));
        CHECK("descriptor effective root x starts as base", Near(desc.effective_roots[index].x, desc.base_roots[index].x));
        CHECK("descriptor effective root y starts as base", Near(desc.effective_roots[index].y, desc.base_roots[index].y));
    }
}

void TestRegularNgonGeneratedDescriptor() {
    const int counts[] = {2, 3, 4, 5, 8, 16};
    for (int count : counts) {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::explaino_root_sdf;
        view.explaino_phase = 0.125f;
        params.explaino_seed = 0.5;
        params.explaino_root_spread = 0.25f;
        params.explaino_generated_root_layout = ExplainoGeneratedRootLayout::regular_ngon_v1;
        params.explaino_generated_root_count = count;

        ExplainoRootFieldDescriptor desc{};
        std::string error;
        CHECK("regular N-gon descriptor resolves",
            ResolveExplainoRootFieldDescriptor(view, params, &desc, &error));
        CHECK("regular N-gon descriptor layout",
            desc.layout_kind == ExplainoRootFieldLayoutKind::regular_ngon_v1);
        CHECK("regular N-gon descriptor source",
            desc.source_kind == ExplainoRootFieldSourceKind::generated);
        CHECK("regular N-gon active count matches request", desc.active_count == count);
        CHECK("regular N-gon does not write custom root count", params.explaino_root_count == 0);

        const float radius = 0.85f + 0.95f * 0.25f;
        const float rotation = static_cast<float>(6.2831853071795864769 *
            (view.explaino_phase + Fract(static_cast<float>(params.explaino_seed * 0.6180339887498949))));
        for (int index = 0; index < count; ++index) {
            const float angle = rotation + static_cast<float>(6.2831853071795864769 * index / count);
            CHECK("regular N-gon x coordinate",
                Near(desc.base_roots[index].x, radius * std::cos(angle), 1.0e-5f));
            CHECK("regular N-gon y coordinate",
                Near(desc.base_roots[index].y, radius * std::sin(angle), 1.0e-5f));
            CHECK("regular N-gon effective starts at base x",
                Near(desc.effective_roots[index].x, desc.base_roots[index].x));
            CHECK("regular N-gon effective starts at base y",
                Near(desc.effective_roots[index].y, desc.base_roots[index].y));
        }
        CHECK("regular N-gon descriptor hash present", desc.base_root_hash != 0);
        CHECK("regular N-gon descriptor effective hash starts equal", desc.base_root_hash == desc.effective_root_hash);
    }
}

void TestRegularNgonRejectsInvalidCounts() {
    const int counts[] = {1, 17};
    for (int count : counts) {
        ViewState view{};
        KernelParams params{};
        view.fractal_type = FractalType::explaino_root_sdf;
        params.explaino_generated_root_layout = ExplainoGeneratedRootLayout::regular_ngon_v1;
        params.explaino_generated_root_count = count;

        ExplainoRootFieldDescriptor desc{};
        std::string error;
        CHECK("regular N-gon invalid count fails",
            !ResolveExplainoRootFieldDescriptor(view, params, &desc, &error));
        CHECK("regular N-gon invalid count reports reason",
            error.find("root count") != std::string::npos);
    }
}

void TestGeneratedLegacyFourRootParity() {
    ViewState view{};
    KernelParams params{};
    view.fractal_type = FractalType::explaino;
    params.explaino_seed = 0.375;
    UpdateExplainoPolynomial(view, params, nullptr);

    ExplainoRootFieldDescriptor desc{};
    std::string error;
    CHECK("generated ExplainO descriptor resolves",
        ResolveExplainoRootFieldDescriptor(view, params, &desc, &error));
    CHECK("generated ExplainO roots are existing four-root authority", params.explaino_root_count == 4);
    CHECK("generated ExplainO descriptor layout", desc.layout_kind == ExplainoRootFieldLayoutKind::legacy_quartic_v1);
    CHECK("generated ExplainO descriptor source", desc.source_kind == ExplainoRootFieldSourceKind::generated);
    CHECK("generated ExplainO descriptor hash present", desc.base_root_hash != 0);
    CHECK("generated ExplainO descriptor effective hash starts equal", desc.base_root_hash == desc.effective_root_hash);
    CheckRootsMatch(params, desc, 4);
}

void TestRootSdfUsesRootLayoutWithoutExplainoFamilySemantics() {
    ViewState view{};
    KernelParams params{};
    view.fractal_type = FractalType::explaino_root_sdf;
    params.explaino_seed = 0.625;
    UpdateExplainoPolynomial(view, params, nullptr);

    ExplainoRootFieldDescriptor desc{};
    std::string error;
    CHECK("root-SDF is not escape-time ExplainO family", !IsExplainoFamily(view.fractal_type));
    CHECK("root-SDF supports root layout authority", UsesExplainoRootLayoutAuthority(view.fractal_type));
    CHECK("root-SDF descriptor resolves", ResolveExplainoRootFieldDescriptor(view, params, &desc, &error));
    CHECK("root-SDF generated descriptor layout", desc.layout_kind == ExplainoRootFieldLayoutKind::legacy_quartic_v1);
    CHECK("root-SDF generated descriptor source", desc.source_kind == ExplainoRootFieldSourceKind::generated);
    CheckRootsMatch(params, desc, 4);
}

void TestCustomRootOrderIsPreserved() {
    ViewState view{};
    KernelParams params{};
    view.fractal_type = FractalType::explaino_root_sdf;
    params.explaino_root_authority = ExplainoRootAuthority::custom;
    params.explaino_root_count = 3;
    params.explaino_roots[0] = {-0.25f, 0.75f};
    params.explaino_roots[1] = {0.5f, -0.125f};
    params.explaino_roots[2] = {1.25f, 0.375f};
    UpdateExplainoPolynomial(view, params, nullptr);

    ExplainoRootFieldDescriptor desc{};
    std::string error;
    CHECK("custom root-SDF descriptor resolves", ResolveExplainoRootFieldDescriptor(view, params, &desc, &error));
    CHECK("custom descriptor layout", desc.layout_kind == ExplainoRootFieldLayoutKind::custom);
    CHECK("custom descriptor source", desc.source_kind == ExplainoRootFieldSourceKind::custom);
    CheckRootsMatch(params, desc, 3);
    CHECK("custom root order root 0", Near(desc.base_roots[0].x, -0.25f) && Near(desc.base_roots[0].y, 0.75f));
    CHECK("custom root order root 1", Near(desc.base_roots[1].x, 0.5f) && Near(desc.base_roots[1].y, -0.125f));
    CHECK("custom root order root 2", Near(desc.base_roots[2].x, 1.25f) && Near(desc.base_roots[2].y, 0.375f));
}

void TestEffectiveRootsDoNotOverwriteAuthorityDescriptor() {
    ViewState view{};
    KernelParams params{};
    view.fractal_type = FractalType::explaino_root_sdf;
    view.explaino_phase = 0.25f;
    params.explaino_seed = 0.125;
    params.explaino_root_sdf_h_source = ExplainoRootSdfHSource::phase_sine;
    params.explaino_root_sdf_h_amplitude = 0.2f;
    params.explaino_root_sdf_h_frequency = 1.0f;
    UpdateExplainoPolynomial(view, params, nullptr);

    ExplainoRootSdfResolvedScene scene{};
    std::string error;
    CHECK("root-SDF effective scene resolves",
        ResolveExplainoRootSdfScene(view, params, &scene, &error));
    CHECK("h source changes effective roots", scene.base_root_hash != scene.effective_root_hash);

    ExplainoRootFieldDescriptor desc{};
    CHECK("root descriptor resolves after h modulation",
        ResolveExplainoRootFieldDescriptor(view, params, &desc, &error));
    CheckRootsMatch(params, desc, 4);
    CHECK("descriptor stays on base authority hash", desc.base_root_hash == scene.base_root_hash);
    CHECK("descriptor does not import local h effective roots", desc.effective_root_hash == scene.base_root_hash);
}

void TestMalformedCapturedRootsFailClearly() {
    ViewState view{};
    KernelParams params{};
    view.fractal_type = FractalType::explaino_root_sdf;
    params.explaino_root_authority = ExplainoRootAuthority::custom;
    params.explaino_root_count = 3;
    params.explaino_roots[0] = {0.0f, 0.0f};
    params.explaino_roots[1] = {std::numeric_limits<float>::quiet_NaN(), 0.0f};
    params.explaino_roots[2] = {0.0f, 1.0f};

    ExplainoRootFieldDescriptor desc{};
    std::string error;
    CHECK("nonfinite captured root fails",
        !ResolveExplainoRootFieldDescriptor(view, params, &desc, &error));
    CHECK("nonfinite captured root reports reason",
        error.find("nonfinite") != std::string::npos);
}

} // namespace

int main() {
    TestRegularNgonGeneratedDescriptor();
    TestRegularNgonRejectsInvalidCounts();
    TestGeneratedLegacyFourRootParity();
    TestRootSdfUsesRootLayoutWithoutExplainoFamilySemantics();
    TestCustomRootOrderIsPreserved();
    TestEffectiveRootsDoNotOverwriteAuthorityDescriptor();
    TestMalformedCapturedRootsFailClearly();

    std::cout << "test_explaino_root_field: " << gPass << " passed, " << gFail << " failed\n";
    return gFail == 0 ? 0 : 1;
}
