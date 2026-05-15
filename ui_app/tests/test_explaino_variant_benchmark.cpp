#include "../src/explaino_variant_benchmark.h"
#include "../src/enum_id_utils.h"
#include "../src/fractal_family_rules.h"

#include <cmath>
#include <cstring>
#include <iostream>
#include <set>
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
} while(0)

float ExpectedDefaultParam(FractalType fractalType) {
    const ExplainoAxisDescriptor* axis = FindExplainoSingleAxisProjectionDescriptor(fractalType);
    return axis ? axis->default_value : 0.0f;
}

float ReadUniqueParam(const KernelParams& params, const char* paramName) {
    if (std::strcmp(paramName, "none") == 0) return 0.0f;
    if (std::strcmp(paramName, "ripple_amplitude") == 0) return params.ripple_amplitude;
    if (std::strcmp(paramName, "splice_offset") == 0) return params.splice_offset;
    if (std::strcmp(paramName, "vortex_strength") == 0) return params.vortex_strength;
    if (std::strcmp(paramName, "tension_strength") == 0) return params.tension_strength;
    return -999.0f;
}

void TestBenchmarkCaseTable() {
    std::size_t count = 0;
    const ExplainoVariantBenchmarkCase* cases = GetExplainoVariantBenchmarkCases(&count);
    std::size_t singleAxisProjectionCount = 0;
    for (const auto& selector : kExplainoSelectorRegistry) {
        if (selector.role == ExplainoSelectorRole::legacy_projection_single_axis) {
            ++singleAxisProjectionCount;
        }
    }
    CHECK("benchmark case table exists", cases != nullptr);
    CHECK("benchmark case count", count == 1u + singleAxisProjectionCount * 2u);
    if (!cases || count != 1u + singleAxisProjectionCount * 2u) return;

    std::set<std::string> caseIds;
    int baselineCount = 0;
    int zeroAxisCount = 0;
    int defaultCount = 0;
    int rippleCount = 0;
    int spliceCount = 0;
    int vortexCount = 0;
    int tensionCount = 0;

    for (std::size_t index = 0; index < count; ++index) {
        const ExplainoVariantBenchmarkCase& benchCase = cases[index];
        CHECK("case id present", benchCase.case_id != nullptr && benchCase.case_id[0] != '\0');
        CHECK("fractal type id present", FractalTypeId(benchCase.fractal_type) != nullptr);
        CHECK("case ids unique", caseIds.insert(benchCase.case_id ? benchCase.case_id : "").second);

        if (benchCase.fractal_type == FractalType::explaino) {
            ++baselineCount;
            CHECK("baseline uses none param", std::strcmp(benchCase.param_name, "none") == 0);
            CHECK("baseline param is zero", std::fabs(benchCase.param_value) < 1.0e-6f);
            CHECK("baseline is not zero-axis row", !benchCase.zero_axis);
            continue;
        }

        if (benchCase.zero_axis) {
            ++zeroAxisCount;
            CHECK("zero-axis param is zero", std::fabs(benchCase.param_value) < 1.0e-6f);
        } else {
            ++defaultCount;
            CHECK("default param matches shipped default", std::fabs(benchCase.param_value - ExpectedDefaultParam(benchCase.fractal_type)) < 1.0e-6f);
        }

        switch (benchCase.fractal_type) {
        case FractalType::explaino_ripple: ++rippleCount; break;
        case FractalType::explaino_splice: ++spliceCount; break;
        case FractalType::explaino_vortex: ++vortexCount; break;
        case FractalType::explaino_tension: ++tensionCount; break;
        default:
            CHECK("only supported explaino variants appear", false);
            break;
        }
    }

    CHECK("exactly one baseline row", baselineCount == 1);
    CHECK("four zero-axis rows", zeroAxisCount == 4);
    CHECK("four default rows", defaultCount == 4);
    CHECK("ripple has two rows", rippleCount == 2);
    CHECK("splice has two rows", spliceCount == 2);
    CHECK("vortex has two rows", vortexCount == 2);
    CHECK("tension has two rows", tensionCount == 2);
}

void TestBuildBenchmarkState() {
    std::size_t count = 0;
    const ExplainoVariantBenchmarkCase* cases = GetExplainoVariantBenchmarkCases(&count);
    if (!cases) {
        CHECK("benchmark cases available for state build", false);
        return;
    }

    for (std::size_t index = 0; index < count; ++index) {
        ViewState view{};
        KernelParams params{};
        RenderSettings render{};
        const char* error = nullptr;

        const bool ok = BuildExplainoVariantBenchmarkState(cases[index], 640, 360, &view, &params, &render, &error);
        CHECK("state build ok", ok);
        if (!ok) {
            std::cerr << "    error: " << (error ? error : "unknown") << "\n";
            continue;
        }

        CHECK("state fractal type matches", view.fractal_type == cases[index].fractal_type);
        CHECK("state resolution width", render.resolution.x == 640);
        CHECK("state resolution height", render.resolution.y == 360);
        CHECK("state block size", render.block_size == 256);
        CHECK("state device id", render.device_id == 0);
        CHECK("state sample tier fast", render.sample_tier == SampleTier::fast);
        CHECK("state explaino roots ready", params.explaino_root_count == 4);
        CHECK("state uses custom polynomial", params.poly_kind == PolyKind::custom);
        CHECK("state log2 zoom finite", std::isfinite(view.log2_zoom));
        CHECK("state resolved eval fast", render.resolved_eval.backend == NumericBackend::float32 && render.resolved_eval.strategy == IterationStrategy::direct);
        CHECK("unique param applied", std::fabs(ReadUniqueParam(params, cases[index].param_name) - cases[index].param_value) < 1.0e-6f);
    }
}

void TestBuildBenchmarkStateRejectsBadArgs() {
    std::size_t count = 0;
    const ExplainoVariantBenchmarkCase* cases = GetExplainoVariantBenchmarkCases(&count);
    if (!cases || count == 0) {
        CHECK("benchmark cases available for bad-arg tests", false);
        return;
    }

    ViewState view{};
    KernelParams params{};
    RenderSettings render{};
    const char* error = nullptr;

    CHECK("reject null view", !BuildExplainoVariantBenchmarkState(cases[0], 640, 360, nullptr, &params, &render, &error));
    CHECK("reject bad width", !BuildExplainoVariantBenchmarkState(cases[0], 0, 360, &view, &params, &render, &error));
    CHECK("reject bad height", !BuildExplainoVariantBenchmarkState(cases[0], 640, 0, &view, &params, &render, &error));
}

} // namespace

int main() {
    TestBenchmarkCaseTable();
    TestBuildBenchmarkState();
    TestBuildBenchmarkStateRejectsBadArgs();
    std::cout << "test_explaino_variant_benchmark: " << gPass << " passed, " << gFail << " failed\n";
    return gFail == 0 ? 0 : 1;
}