#include "../src/basin_coloring.h"

#include <iostream>

namespace {

struct Cx {
    float x;
    float y;
};

struct Cxd {
    double x;
    double y;
};

struct Color {
    unsigned char x;
    unsigned char y;
    unsigned char z;
    unsigned char w;
};

bool Equals(Color left, Color right) {
    return left.x == right.x && left.y == right.y && left.z == right.z && left.w == right.w;
}

} // namespace

int main() {
    if (ResolvePolynomialRootCount(PolyKind::z3_minus_1) != 3 ||
        ResolvePolynomialRootCount(PolyKind::z4_minus_1) != 4 ||
        ResolvePolynomialRootCount(PolyKind::custom) != 0) {
        std::cerr << "Basin helper should preserve polynomial root-count resolution\n";
        return 1;
    }

    if (NearestRootIndexUnitRoots(Cx{0.95f, 0.05f}, 4) != 2 ||
        NearestRootIndexUnitRoots(Cx{0.05f, 0.95f}, 4) != 3 ||
        NearestRootIndexUnitRoots(Cxd{-0.95, 0.05}, 4) != 0) {
        std::cerr << "Basin helper should preserve unit-root indexing for float and double paths\n";
        return 1;
    }

    const Float2 roots[3]{{1.0f, 0.0f}, {0.0f, 1.0f}, {-1.0f, 0.0f}};
    if (NearestRootIndexList(Cx{0.1f, 0.9f}, roots, 3) != 1 ||
        NearestRootIndexList(Cxd{-0.8, 0.1}, roots, 3) != 2) {
        std::cerr << "Basin helper should preserve explicit-root indexing for float and double paths\n";
        return 1;
    }

    if (!Equals(PaletteRoot<Color>(0), Color{255, 64, 64, 255}) ||
        !Equals(PaletteRoot<Color>(5), Color{64, 255, 255, 255}) ||
        !Equals(PaletteJoyRoot<Color>(0), Color{255, 140, 80, 255}) ||
        !Equals(PaletteJoyRoot<Color>(7), Color{255, 235, 170, 255})) {
        std::cerr << "Basin helper should preserve the existing root palettes\n";
        return 1;
    }

    std::cout << "test_basin_coloring: all passed\n";
    return 0;
}