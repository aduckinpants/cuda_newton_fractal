#include "flashlight_probe.h"

#include <cstdio>
#include <cstring>

static int g_passed = 0;
static int g_failed = 0;

static void Check(bool cond, const char* name) {
    if (cond) {
        std::printf("  PASS: %s\n", name);
        ++g_passed;
    } else {
        std::printf("  FAIL: %s\n", name);
        ++g_failed;
    }
}

static void TestNormalizeFlashlightProbeConfig() {
    FlashlightProbeConfig config;
    config.ticks = 9000;
    config.radius = -2.0;
    config.zoom_radius = 42.0;
    config.warp = 7.0;
    config.closure_ref_t = -5;
    const FlashlightProbeConfig normalized = NormalizeFlashlightProbeConfig(config);
    Check(normalized.ticks == 4096, "TestNormalizeFlashlightProbeConfig_Ticks");
    Check(normalized.radius == 0.0, "TestNormalizeFlashlightProbeConfig_Radius");
    Check(normalized.zoom_radius == 10.0, "TestNormalizeFlashlightProbeConfig_ZoomRadius");
    Check(normalized.warp == 1.0, "TestNormalizeFlashlightProbeConfig_Warp");
    Check(normalized.closure_ref_t == 0, "TestNormalizeFlashlightProbeConfig_ClosureRef");
}

static void TestFlashlightFnv1a32() {
    const char* text = "flashlight";
    const uint32_t hash = FlashlightFnv1a32(text, std::strlen(text));
    Check(hash == 3875930387u, "TestFlashlightFnv1a32_Value");
}

static void TestComputeConversationSpectrum8() {
    const auto spectrum = ComputeFlashlightConversationSpectrum8("control-question-seed");
    Check(spectrum[0] == 3444372188u, "TestComputeConversationSpectrum8_Lane0");
    Check(spectrum[7] == 1640858466u, "TestComputeConversationSpectrum8_Lane7");
}

static void TestNormalizeFlashlightLensDownsamplePow2() {
    Check(NormalizeFlashlightLensDownsamplePow2(1) == 1, "TestNormalizeFlashlightLensDownsamplePow2_One");
    Check(NormalizeFlashlightLensDownsamplePow2(3) == 4, "TestNormalizeFlashlightLensDownsamplePow2_Three");
    Check(NormalizeFlashlightLensDownsamplePow2(9) == 8, "TestNormalizeFlashlightLensDownsamplePow2_Nine");
}

static void TestFlashlightManifoldAtDeterministic() {
    const FlashlightManifoldStep a = FlashlightManifoldAt(1305844223u, 3, 4, 0.0, 0.61, 0.19, 4.0 / 3.0);
    const FlashlightManifoldStep b = FlashlightManifoldAt(1305844223u, 3, 4, 0.0, 0.61, 0.19, 4.0 / 3.0);
    Check(a.band == 3, "TestFlashlightManifoldAtDeterministic_Band");
    Check(a.band == b.band, "TestFlashlightManifoldAtDeterministic_SameBand");
    Check(a.log2_zoom_tick == b.log2_zoom_tick, "TestFlashlightManifoldAtDeterministic_SameZoom");
    Check(a.dx_world == b.dx_world, "TestFlashlightManifoldAtDeterministic_SameDx");
    Check(a.dy_world == b.dy_world, "TestFlashlightManifoldAtDeterministic_SameDy");
}

int main() {
    TestNormalizeFlashlightProbeConfig();
    TestFlashlightFnv1a32();
    TestComputeConversationSpectrum8();
    TestNormalizeFlashlightLensDownsamplePow2();
    TestFlashlightManifoldAtDeterministic();

    std::printf("test_flashlight_probe: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}
