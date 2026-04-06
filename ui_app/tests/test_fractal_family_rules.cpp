#include "../src/fractal_family_rules.h"

#include <iostream>

int main() {
    {
        if (!IsEscapeTimeFamily(FractalType::nova)) {
            std::cerr << "Nova should be classified as an escape-time family\n";
            return 1;
        }
        if (SupportsBasinColoring(FractalType::nova)) {
            std::cerr << "Nova should not support basin coloring\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::nova) != ColoringMode::smooth_escape) {
            std::cerr << "Nova should default to smooth_escape\n";
            return 1;
        }
        if (IsColoringModeAllowedForFractal(FractalType::nova, ColoringMode::root_basin)) {
            std::cerr << "Nova should reject root_basin coloring\n";
            return 1;
        }
        if (IsColoringModeAllowedForFractal(FractalType::nova, ColoringMode::joy_basins)) {
            std::cerr << "Nova should reject joy_basins coloring\n";
            return 1;
        }
        if (!IsColoringModeAllowedForFractal(FractalType::nova, ColoringMode::iteration_count) ||
            !IsColoringModeAllowedForFractal(FractalType::nova, ColoringMode::smooth_escape)) {
            std::cerr << "Nova should allow iteration_count and smooth_escape\n";
            return 1;
        }
    }

    {
        if (!SupportsBasinColoring(FractalType::newton)) {
            std::cerr << "Newton should support basin coloring\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::newton) != ColoringMode::joy_basins) {
            std::cerr << "Newton should default to joy_basins\n";
            return 1;
        }
    }

    {
        if (!SupportsBasinColoring(FractalType::explaino_fp)) {
            std::cerr << "Explaino FP should support basin coloring\n";
            return 1;
        }
        if (!IsExplainoFamily(FractalType::explaino_y)) {
            std::cerr << "Explaino Y should stay in the Explaino family\n";
            return 1;
        }
    }

    {
        if (!IsExplainoFamily(FractalType::explaino_nova)) {
            std::cerr << "Explaino-Nova should stay in the Explaino family surface\n";
            return 1;
        }
        if (!IsEscapeTimeFamily(FractalType::explaino_nova)) {
            std::cerr << "Explaino-Nova should be classified as an escape-time family\n";
            return 1;
        }
        if (SupportsBasinColoring(FractalType::explaino_nova)) {
            std::cerr << "Explaino-Nova should not support basin coloring\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::explaino_nova) != ColoringMode::smooth_escape) {
            std::cerr << "Explaino-Nova should default to smooth_escape\n";
            return 1;
        }
        if (IsColoringModeAllowedForFractal(FractalType::explaino_nova, ColoringMode::root_basin) ||
            IsColoringModeAllowedForFractal(FractalType::explaino_nova, ColoringMode::joy_basins)) {
            std::cerr << "Explaino-Nova should reject basin coloring modes\n";
            return 1;
        }
        if (!IsColoringModeAllowedForFractal(FractalType::explaino_nova, ColoringMode::iteration_count) ||
            !IsColoringModeAllowedForFractal(FractalType::explaino_nova, ColoringMode::smooth_escape)) {
            std::cerr << "Explaino-Nova should allow escape-time coloring modes\n";
            return 1;
        }
    }

    {
        if (!IsEscapeTimeFamily(FractalType::phoenix)) {
            std::cerr << "Phoenix should stay in the escape-time family\n";
            return 1;
        }
        if (IsColoringModeAllowedForFractal(FractalType::phoenix, ColoringMode::joy_basins)) {
            std::cerr << "Phoenix should reject joy_basins coloring\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::phoenix) != ColoringMode::smooth_escape) {
            std::cerr << "Phoenix should default to smooth_escape\n";
            return 1;
        }
    }

    {
        if (!IsExplainoFamily(FractalType::explaino_halley)) {
            std::cerr << "Explaino-Halley should be in the Explaino family\n";
            return 1;
        }
        if (!SupportsBasinColoring(FractalType::explaino_halley)) {
            std::cerr << "Explaino-Halley should support basin coloring\n";
            return 1;
        }
        if (IsEscapeTimeFamily(FractalType::explaino_halley)) {
            std::cerr << "Explaino-Halley should not be escape-time\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::explaino_halley) != ColoringMode::joy_basins) {
            std::cerr << "Explaino-Halley should default to joy_basins\n";
            return 1;
        }
    }

    {
        if (!IsExplainoFamily(FractalType::explaino_dual)) {
            std::cerr << "Explaino-DualSeed should be in the Explaino family\n";
            return 1;
        }
        if (!SupportsBasinColoring(FractalType::explaino_dual)) {
            std::cerr << "Explaino-DualSeed should support basin coloring\n";
            return 1;
        }
        if (IsEscapeTimeFamily(FractalType::explaino_dual)) {
            std::cerr << "Explaino-DualSeed should not be escape-time\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::explaino_dual) != ColoringMode::joy_basins) {
            std::cerr << "Explaino-DualSeed should default to joy_basins\n";
            return 1;
        }
    }

    // Explaino-Julia: explaino family + escape-time + no basin coloring
    {
        if (!IsExplainoFamily(FractalType::explaino_julia)) {
            std::cerr << "Explaino-Julia should be in the Explaino family\n";
            return 1;
        }
        if (!IsEscapeTimeFamily(FractalType::explaino_julia)) {
            std::cerr << "Explaino-Julia should be escape-time\n";
            return 1;
        }
        if (SupportsBasinColoring(FractalType::explaino_julia)) {
            std::cerr << "Explaino-Julia should not support basin coloring\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::explaino_julia) != ColoringMode::smooth_escape) {
            std::cerr << "Explaino-Julia should default to smooth_escape\n";
            return 1;
        }
        if (IsColoringModeAllowedForFractal(FractalType::explaino_julia, ColoringMode::root_basin) ||
            IsColoringModeAllowedForFractal(FractalType::explaino_julia, ColoringMode::joy_basins)) {
            std::cerr << "Explaino-Julia should reject basin coloring modes\n";
            return 1;
        }
    }

    // Explaino-Rational: explaino family + basin coloring + not escape-time
    {
        if (!IsExplainoFamily(FractalType::explaino_rational)) {
            std::cerr << "Explaino-Rational should be in the Explaino family\n";
            return 1;
        }
        if (!SupportsBasinColoring(FractalType::explaino_rational)) {
            std::cerr << "Explaino-Rational should support basin coloring\n";
            return 1;
        }
        if (IsEscapeTimeFamily(FractalType::explaino_rational)) {
            std::cerr << "Explaino-Rational should not be escape-time\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::explaino_rational) != ColoringMode::joy_basins) {
            std::cerr << "Explaino-Rational should default to joy_basins\n";
            return 1;
        }
    }

    // Multicorn: not explaino + escape-time + no basin coloring
    {
        if (IsExplainoFamily(FractalType::multicorn)) {
            std::cerr << "Multicorn should not be in the Explaino family\n";
            return 1;
        }
        if (!IsEscapeTimeFamily(FractalType::multicorn)) {
            std::cerr << "Multicorn should be escape-time\n";
            return 1;
        }
        if (SupportsBasinColoring(FractalType::multicorn)) {
            std::cerr << "Multicorn should not support basin coloring\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::multicorn) != ColoringMode::smooth_escape) {
            std::cerr << "Multicorn should default to smooth_escape\n";
            return 1;
        }
    }

    // Halley: not explaino + basin coloring + not escape-time
    {
        if (IsExplainoFamily(FractalType::halley)) {
            std::cerr << "Halley should not be in the Explaino family\n";
            return 1;
        }
        if (!SupportsBasinColoring(FractalType::halley)) {
            std::cerr << "Halley should support basin coloring\n";
            return 1;
        }
        if (IsEscapeTimeFamily(FractalType::halley)) {
            std::cerr << "Halley should not be escape-time\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::halley) != ColoringMode::joy_basins) {
            std::cerr << "Halley should default to joy_basins\n";
            return 1;
        }
        if (!IsColoringModeAllowedForFractal(FractalType::halley, ColoringMode::root_basin)) {
            std::cerr << "Halley should allow root_basin coloring\n";
            return 1;
        }
    }

    // Collatz: not explaino + escape-time + no basin coloring
    {
        if (IsExplainoFamily(FractalType::collatz)) {
            std::cerr << "Collatz should not be in the Explaino family\n";
            return 1;
        }
        if (!IsEscapeTimeFamily(FractalType::collatz)) {
            std::cerr << "Collatz should be escape-time\n";
            return 1;
        }
        if (SupportsBasinColoring(FractalType::collatz)) {
            std::cerr << "Collatz should not support basin coloring\n";
            return 1;
        }
        if (DefaultColoringModeForFractal(FractalType::collatz) != ColoringMode::smooth_escape) {
            std::cerr << "Collatz should default to smooth_escape\n";
            return 1;
        }
        if (IsColoringModeAllowedForFractal(FractalType::collatz, ColoringMode::joy_basins)) {
            std::cerr << "Collatz should reject joy_basins coloring\n";
            return 1;
        }
    }

    return 0;
}