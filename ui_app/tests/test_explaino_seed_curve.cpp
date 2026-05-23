#include "../src/explaino_seed_curve.h"

#include <cmath>
#include <cstdio>

namespace {

int g_passed = 0;
int g_failed = 0;

void Check(bool condition, const char* name) {
    if (condition) {
        std::printf("  PASS: %s\n", name);
        ++g_passed;
    } else {
        std::printf("  FAIL: %s\n", name);
        ++g_failed;
    }
}

bool NearlyEqual(double left, double right, double eps = 1.0e-12) {
    return std::fabs(left - right) <= eps;
}

void TestWedgeCurveBasics() {
    const double pi = 3.141592653589793;
    const double x1 = 1.0 - 1.0 / pi;

    Check(NearlyEqual(ExplainoWedgeCumulativeRaw(0.0), 0.0),
        "TestWedgeCurveBasics_CumulativeRawZero");
    Check(ExplainoWedgeTotalArea() > 0.0,
        "TestWedgeCurveBasics_TotalAreaPositive");
    Check(NearlyEqual(ExplainoAreaFractionToX(0.0), 0.0),
        "TestWedgeCurveBasics_AreaFractionZero");
    Check(NearlyEqual(ExplainoAreaFractionToX(1.0), x1),
        "TestWedgeCurveBasics_AreaFractionOne");
    Check(NearlyEqual(ExplainoWedgeTween(0.0), 0.0),
        "TestWedgeCurveBasics_TweenZero");
    Check(NearlyEqual(ExplainoWedgeTween(1.0), 1.0),
        "TestWedgeCurveBasics_TweenOne");

    const double h25 = ExplainoWedgeTween(0.25);
    const double h50 = ExplainoWedgeTween(0.50);
    const double h75 = ExplainoWedgeTween(0.75);
    Check(h25 > 0.0 && h25 < h50 && h50 < h75 && h75 < 1.0,
        "TestWedgeCurveBasics_TweenStrictlyMonotone");
    Check(NearlyEqual(h50, 0.5, 1.0e-12),
        "TestWedgeCurveBasics_TweenMidpointSymmetry");
    Check(!NearlyEqual(h25, 0.25, 1.0e-6),
        "TestWedgeCurveBasics_TweenNonlinear");
}

void TestLogisticAreaUToSeedContract() {
    const double u25 = LogisticAreaUToSeed(0.25);
    const double u525 = LogisticAreaUToSeed(5.25);
    const double u75 = LogisticAreaUToSeed(0.75);
    const double warp25 = ExplainoCombinedSeedToWarpSeed(0.25);
    const double warp525 = ExplainoCombinedSeedToWarpSeed(5.25);

    Check(NearlyEqual(u25, 0.942416285139079, 1.0e-12),
        "TestLogisticAreaUToSeedContract_RegressionValue");
    Check(NearlyEqual(u25, u525, 1.0e-12),
        "TestLogisticAreaUToSeedContract_FractionalPeriodicity");
    Check(!NearlyEqual(u25, 0.25, 1.0e-6),
        "TestLogisticAreaUToSeedContract_NotLinearIdentity");
    Check(u25 >= 0.0 && u25 <= 1.0 && u75 >= 0.0 && u75 <= 1.0,
        "TestLogisticAreaUToSeedContract_Bounded");
    Check(!NearlyEqual(u25, u75, 1.0e-12),
        "TestLogisticAreaUToSeedContract_DistinctFractionsDiffer");
    Check(!NearlyEqual(warp25, warp525, 1.0e-12),
        "TestLogisticAreaUToSeedContract_WarpSeedIncludesIntegerComponent");
}

void TestHashHelpersDeterministic() {
    const std::uint64_t splitmixA = ExplainoSplitmix64(123456789ULL);
    const std::uint64_t splitmixB = ExplainoSplitmix64(123456789ULL);
    const std::uint64_t splitmixC = ExplainoSplitmix64(123456790ULL);

    Check(splitmixA == splitmixB,
        "TestHashHelpersDeterministic_SplitmixDeterministic");
    Check(splitmixA != splitmixC,
        "TestHashHelpersDeterministic_SplitmixSensitive");

    const std::uint64_t orbitA = ExplainoHashLogisticOrbit(0.2, 12);
    const std::uint64_t orbitB = ExplainoHashLogisticOrbit(0.2, 12);
    const std::uint64_t orbitC = ExplainoHashLogisticOrbit(0.21, 12);
    Check(orbitA == orbitB,
        "TestHashHelpersDeterministic_OrbitHashDeterministic");
    Check(orbitA != orbitC,
        "TestHashHelpersDeterministic_OrbitHashSensitive");
}

} // namespace

int main() {
    TestWedgeCurveBasics();
    TestLogisticAreaUToSeedContract();
    TestHashHelpersDeterministic();

    std::printf("test_explaino_seed_curve: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}
