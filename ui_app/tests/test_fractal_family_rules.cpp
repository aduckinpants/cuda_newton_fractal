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

    return 0;
}