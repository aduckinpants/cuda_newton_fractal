#include "../src/view_hp_sync.h"

#include <cmath>
#include <iostream>

static bool NearlyEqual(double a, double b, double eps = 1.0e-9) {
    return std::fabs(a - b) <= eps;
}

int main() {
    // --- ClampD ---
    {
        if (ClampD(5.0, 0.0, 10.0) != 5.0) {
            std::cerr << "ClampD mid-range failed\n"; return 1;
        }
        if (ClampD(-1.0, 0.0, 10.0) != 0.0) {
            std::cerr << "ClampD below-lo failed\n"; return 1;
        }
        if (ClampD(20.0, 0.0, 10.0) != 10.0) {
            std::cerr << "ClampD above-hi failed\n"; return 1;
        }
        if (ClampD(0.0, 0.0, 0.0) != 0.0) {
            std::cerr << "ClampD degenerate range failed\n"; return 1;
        }
    }

    // --- Log2D / Exp2D roundtrip ---
    {
        double vals[] = {0.5, 1.0, 2.0, 1024.0, 1.0e-10, 1.0e+20};
        for (double v : vals) {
            double rt = Exp2D(Log2D(v));
            if (!NearlyEqual(rt, v, v * 1.0e-12)) {
                std::cerr << "Log2D/Exp2D roundtrip failed for " << v << "\n";
                return 1;
            }
        }
    }

    // --- SafeZoomFromLog2 ---
    {
        // log2(1.0) = 0
        if (!NearlyEqual(SafeZoomFromLog2(0.0), 1.0)) {
            std::cerr << "SafeZoomFromLog2(0) failed\n"; return 1;
        }
        // log2(4.0) = 2
        if (!NearlyEqual(SafeZoomFromLog2(2.0), 4.0)) {
            std::cerr << "SafeZoomFromLog2(2) failed\n"; return 1;
        }
        // Extreme positive clamps to kMaxLog2Zoom
        double huge = SafeZoomFromLog2(1.0e+30);
        double maxVal = Exp2D(kMaxLog2Zoom);
        if (!NearlyEqual(huge, maxVal, maxVal * 1.0e-12)) {
            std::cerr << "SafeZoomFromLog2 extreme positive clamp failed\n"; return 1;
        }
        // Extreme negative clamps to Log2D(1.0e-30)
        double tiny = SafeZoomFromLog2(-1.0e+30);
        if (tiny < 0.0 || tiny > 1.0e-20) {
            std::cerr << "SafeZoomFromLog2 extreme negative clamp failed\n"; return 1;
        }
    }

    // --- SyncViewHpFromUi basic ---
    {
        ViewState view{};
        view.center = {-0.745f, 0.186f};
        view.zoom = 38.0f;
        SyncViewHpFromUi(view);
        if (!NearlyEqual(view.center_hp_x, static_cast<double>(-0.745f), 1.0e-6)) {
            std::cerr << "SyncViewHpFromUi center_hp_x failed\n"; return 1;
        }
        if (!NearlyEqual(view.center_hp_y, static_cast<double>(0.186f), 1.0e-6)) {
            std::cerr << "SyncViewHpFromUi center_hp_y failed\n"; return 1;
        }
        double expected_log2 = Log2D(38.0);
        if (!NearlyEqual(view.log2_zoom, expected_log2, 1.0e-6)) {
            std::cerr << "SyncViewHpFromUi log2_zoom failed\n"; return 1;
        }
    }

    // --- SyncViewUiFromHp basic ---
    {
        ViewState view{};
        view.center_hp_x = -1.762;
        view.center_hp_y = -0.028;
        view.log2_zoom = Log2D(25.0);
        SyncViewUiFromHp(view);
        if (std::fabs(view.center.x - (-1.762f)) > 1.0e-4f) {
            std::cerr << "SyncViewUiFromHp center.x failed\n"; return 1;
        }
        if (std::fabs(view.center.y - (-0.028f)) > 1.0e-4f) {
            std::cerr << "SyncViewUiFromHp center.y failed\n"; return 1;
        }
        if (std::fabs(view.zoom - 25.0f) > 0.01f) {
            std::cerr << "SyncViewUiFromHp zoom failed\n"; return 1;
        }
    }

    // --- Roundtrip: UI -> HP -> UI preserves float precision ---
    {
        ViewState view{};
        view.center = {0.36f, -0.1f};
        view.zoom = 2.8f;
        SyncViewHpFromUi(view);
        SyncViewUiFromHp(view);
        if (std::fabs(view.center.x - 0.36f) > 1.0e-5f) {
            std::cerr << "Roundtrip center.x failed\n"; return 1;
        }
        if (std::fabs(view.center.y - (-0.1f)) > 1.0e-5f) {
            std::cerr << "Roundtrip center.y failed\n"; return 1;
        }
        if (std::fabs(view.zoom - 2.8f) > 0.01f) {
            std::cerr << "Roundtrip zoom failed\n"; return 1;
        }
    }

    // --- HP precision loss: double center survives HP->UI only to float ---
    {
        ViewState view{};
        view.center_hp_x = 1.23456789012345;
        view.center_hp_y = -9.87654321098765;
        view.log2_zoom = 10.0;
        SyncViewUiFromHp(view);
        // float can only store ~7 digits; the HP values should be truncated
        float fx = static_cast<float>(1.23456789012345);
        float fy = static_cast<float>(-9.87654321098765);
        if (view.center.x != fx || view.center.y != fy) {
            std::cerr << "HP->UI precision truncation mismatch\n"; return 1;
        }
    }

    // --- Zoom edge: very small zoom stays positive ---
    {
        ViewState view{};
        view.log2_zoom = -200.0;
        SyncViewUiFromHp(view);
        if (view.zoom <= 0.0f) {
            std::cerr << "Very small log2_zoom produced non-positive zoom\n"; return 1;
        }
    }

    std::cout << "test_view_hp_sync: all passed\n";
    return 0;
}
