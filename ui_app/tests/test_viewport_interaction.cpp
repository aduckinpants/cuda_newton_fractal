// test_viewport_interaction.cpp
// Headless tests for viewport math: zoom-around-cursor, drag-pan, auto-dive.
// These verify the double-precision camera math extracted from main.cpp.

#include "../src/viewport_interaction.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

static int g_passed = 0;
static int g_failed = 0;

#define ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::fprintf(stderr, "%s:%d: FAIL: %s\n", __FILE__, __LINE__, (msg)); \
            g_failed++; \
            return false; \
        } \
        g_passed++; \
    } while (0)

#define ASSERT_NEAR(a, b, tol, msg) \
    ASSERT(std::fabs((double)(a) - (double)(b)) < (tol), msg)

// --- ClampF ---

bool TestClampFInRange() {
    ASSERT(ClampF(0.5f, 0.0f, 1.0f) == 0.5f, "value in range unchanged");
    return true;
}

bool TestClampFBelowMin() {
    ASSERT(ClampF(-1.0f, 0.0f, 1.0f) == 0.0f, "below min clamped to min");
    return true;
}

bool TestClampFAboveMax() {
    ASSERT(ClampF(2.0f, 0.0f, 1.0f) == 1.0f, "above max clamped to max");
    return true;
}

bool TestClampFAtBoundary() {
    ASSERT(ClampF(0.0f, 0.0f, 1.0f) == 0.0f, "at min boundary");
    ASSERT(ClampF(1.0f, 0.0f, 1.0f) == 1.0f, "at max boundary");
    return true;
}

// --- ApplyAutoDiveStep ---

bool TestAutoDiveDisabled() {
    ViewState view{};
    view.auto_dive = false;
    view.camera_behavior = CameraBehavior::complexity;
    view.dive_speed = 1.0f;
    view.log2_zoom = 0.0;
    ASSERT(!ApplyAutoDiveStep(view), "disabled auto-dive should not modify view");
    ASSERT_NEAR(view.log2_zoom, 0.0, 1e-15, "log2_zoom unchanged");
    return true;
}

bool TestAutoDiveManualCamera() {
    ViewState view{};
    view.auto_dive = true;
    view.camera_behavior = CameraBehavior::manual;
    view.dive_speed = 1.0f;
    view.log2_zoom = 0.0;
    ASSERT(!ApplyAutoDiveStep(view), "manual camera should not auto-dive");
    return true;
}

bool TestAutoDiveOffCamera() {
    ViewState view{};
    view.auto_dive = true;
    view.camera_behavior = CameraBehavior::off;
    view.dive_speed = 1.0f;
    view.log2_zoom = 0.0;
    ASSERT(!ApplyAutoDiveStep(view), "off camera should not auto-dive");
    return true;
}

bool TestAutoDiveZeroSpeed() {
    ViewState view{};
    view.auto_dive = true;
    view.camera_behavior = CameraBehavior::complexity;
    view.dive_speed = 0.0f;
    view.log2_zoom = 5.0;
    ASSERT(!ApplyAutoDiveStep(view), "zero speed should not auto-dive");
    ASSERT_NEAR(view.log2_zoom, 5.0, 1e-15, "log2_zoom unchanged at zero speed");
    return true;
}

bool TestAutoDiveNegativeSpeed() {
    ViewState view{};
    view.auto_dive = true;
    view.camera_behavior = CameraBehavior::complexity;
    view.dive_speed = -5.0f;
    view.log2_zoom = 5.0;
    ASSERT(!ApplyAutoDiveStep(view), "negative speed should not auto-dive");
    return true;
}

bool TestAutoDiveNormalStep() {
    ViewState view{};
    view.auto_dive = true;
    view.camera_behavior = CameraBehavior::complexity;
    view.dive_speed = 1.0f;
    view.log2_zoom = 10.0;
    double oldZoom = view.log2_zoom;
    
    ASSERT(ApplyAutoDiveStep(view), "should modify view");
    ASSERT(view.log2_zoom > oldZoom, "zoom should increase (dive in)");
    
    // The step is small: 0.002 * speed = 0.002, so factor = 1.002
    // log2(1.002) ~ 0.00289, so delta should be small
    double delta = view.log2_zoom - oldZoom;
    ASSERT(delta > 0.001 && delta < 0.01, "step size in expected range");
    return true;
}

bool TestAutoDiveHighSpeed() {
    ViewState view{};
    view.auto_dive = true;
    view.camera_behavior = CameraBehavior::complexity;
    view.dive_speed = 100.0f;
    view.log2_zoom = 10.0;
    double oldZoom = view.log2_zoom;
    
    ASSERT(ApplyAutoDiveStep(view), "should modify view");
    double delta = view.log2_zoom - oldZoom;
    // speed=100 -> factor=1.2 -> log2(1.2) ~ 0.263
    ASSERT(delta > 0.1 && delta < 0.5, "high speed step in expected range");
    return true;
}

bool TestAutoDiveSyncsUiFields() {
    ViewState view{};
    view.auto_dive = true;
    view.camera_behavior = CameraBehavior::complexity;
    view.dive_speed = 1.0f;
    view.log2_zoom = 5.0;
    view.center_hp_x = 1.5;
    view.center_hp_y = -0.3;
    
    ApplyAutoDiveStep(view);
    
    // After dive, UI float fields should be synced from HP fields
    ASSERT_NEAR(view.center.x, 1.5f, 0.01f, "center.x synced");
    ASSERT_NEAR(view.center.y, -0.3f, 0.01f, "center.y synced");
    ASSERT(view.zoom > 0.0f, "zoom should be positive after sync");
    return true;
}

// --- ComputeZoomAroundCursor ---

bool TestZoomCenterCursorInvariant() {
    // Zooming at the exact center (u=0.5, v=0.5) should not move the center.
    double cx = 1.0, cy = -2.0, log2z = 5.0;
    auto r = ComputeZoomAroundCursor(cx, cy, log2z, 0.5f, 0.5f, 1.0f, 1.0);
    
    ASSERT_NEAR(r.new_center_hp_x, cx, 1e-10, "center-zoom should not move x");
    ASSERT_NEAR(r.new_center_hp_y, cy, 1e-10, "center-zoom should not move y");
    ASSERT(r.new_log2_zoom > log2z, "zoom should increase on positive wheel");
    return true;
}

bool TestZoomOutDecreasesZoom() {
    double cx = 0.0, cy = 0.0, log2z = 10.0;
    auto r = ComputeZoomAroundCursor(cx, cy, log2z, 0.5f, 0.5f, -1.0f, 1.0);
    
    ASSERT(r.new_log2_zoom < log2z, "negative wheel should decrease zoom");
    return true;
}

bool TestZoomFixedPointProperty() {
    // The world point under the cursor should remain (approximately) the same
    // after zoom. This is the core "zoom toward cursor" invariant.
    double cx = 0.5, cy = -0.3, log2z = 3.0;
    float u = 0.7f, v = 0.2f;
    double aspect = 16.0 / 9.0;
    
    // Compute world point before zoom
    double zoomBefore = SafeZoomFromLog2(log2z);
    double baseBefore = 2.0 / fmax(1e-30, zoomBefore);
    double nx = ((double)u - 0.5) * 2.0;
    double ny = ((double)v - 0.5) * 2.0;
    double worldXBefore = cx + nx * baseBefore * aspect;
    double worldYBefore = cy + ny * baseBefore;
    
    auto r = ComputeZoomAroundCursor(cx, cy, log2z, u, v, 3.0f, aspect);
    
    // Compute world point after zoom (same cursor position, new center + zoom)
    double zoomAfter = SafeZoomFromLog2(r.new_log2_zoom);
    double baseAfter = 2.0 / fmax(1e-30, zoomAfter);
    double worldXAfter = r.new_center_hp_x + nx * baseAfter * aspect;
    double worldYAfter = r.new_center_hp_y + ny * baseAfter;
    
    ASSERT_NEAR(worldXBefore, worldXAfter, 1e-10, "world X under cursor preserved");
    ASSERT_NEAR(worldYBefore, worldYAfter, 1e-10, "world Y under cursor preserved");
    return true;
}

bool TestZoomAspectRatioEffect() {
    // With aspect > 1, zooming at an off-center x should shift x more than y.
    double cx = 0.0, cy = 0.0, log2z = 5.0;
    
    auto r_wide = ComputeZoomAroundCursor(cx, cy, log2z, 0.75f, 0.5f, 2.0f, 2.0);
    auto r_square = ComputeZoomAroundCursor(cx, cy, log2z, 0.75f, 0.5f, 2.0f, 1.0);
    
    // With wider aspect, the center shift should be larger in x
    double dx_wide = std::fabs(r_wide.new_center_hp_x - cx);
    double dx_square = std::fabs(r_square.new_center_hp_x - cx);
    ASSERT(dx_wide > dx_square, "wider aspect should shift x more");
    
    // y should be unaffected by cursor at v=0.5
    ASSERT_NEAR(r_wide.new_center_hp_y, cy, 1e-10, "center cursor v=0.5 should not shift y");
    return true;
}

bool TestZoomReversibility() {
    // Zoom in then out by same amount should approximately restore the state.
    double cx = 1.0, cy = -0.5, log2z = 8.0;
    float u = 0.3f, v = 0.6f;
    double aspect = 1.333;
    
    auto r1 = ComputeZoomAroundCursor(cx, cy, log2z, u, v, 5.0f, aspect);
    auto r2 = ComputeZoomAroundCursor(r1.new_center_hp_x, r1.new_center_hp_y,
                                       r1.new_log2_zoom, u, v, -5.0f, aspect);
    
    ASSERT_NEAR(r2.new_center_hp_x, cx, 1e-8, "zoom in+out should restore x");
    ASSERT_NEAR(r2.new_center_hp_y, cy, 1e-8, "zoom in+out should restore y");
    ASSERT_NEAR(r2.new_log2_zoom, log2z, 1e-8, "zoom in+out should restore log2_zoom");
    return true;
}

// --- ComputeDragPan ---

bool TestDragPanZeroDelta() {
    auto r = ComputeDragPan(1.0, -1.0, 5.0, 0.0f, 0.0f, 1024, 768);
    ASSERT_NEAR(r.new_center_hp_x, 1.0, 1e-15, "zero drag should not move x");
    ASSERT_NEAR(r.new_center_hp_y, -1.0, 1e-15, "zero drag should not move y");
    return true;
}

bool TestDragPanRight() {
    // Dragging right (positive dx) should move center LEFT (grab semantics).
    double cx = 0.0, cy = 0.0;
    auto r = ComputeDragPan(cx, cy, 0.0, 100.0f, 0.0f, 1024, 768);
    ASSERT(r.new_center_hp_x < cx, "drag right should shift center left");
    ASSERT_NEAR(r.new_center_hp_y, cy, 1e-15, "horizontal drag should not affect y");
    return true;
}

bool TestDragPanDown() {
    // Dragging down (positive dy) should move center UP (grab semantics).
    double cx = 0.0, cy = 0.0;
    auto r = ComputeDragPan(cx, cy, 0.0, 0.0f, 100.0f, 1024, 768);
    ASSERT_NEAR(r.new_center_hp_x, cx, 1e-15, "vertical drag should not affect x");
    ASSERT(r.new_center_hp_y < cy, "drag down should shift center up");
    return true;
}

bool TestDragPanScalesWithZoom() {
    // At deeper zoom (higher log2_zoom), same pixel drag = smaller world delta.
    double cx = 0.0, cy = 0.0;
    auto r_shallow = ComputeDragPan(cx, cy, 0.0, 50.0f, 50.0f, 1024, 768);
    auto r_deep = ComputeDragPan(cx, cy, 10.0, 50.0f, 50.0f, 1024, 768);
    
    double dx_shallow = std::fabs(r_shallow.new_center_hp_x - cx);
    double dx_deep = std::fabs(r_deep.new_center_hp_x - cx);
    ASSERT(dx_deep < dx_shallow, "deeper zoom should produce smaller world pan");
    ASSERT(dx_deep < dx_shallow * 0.01, "10 stops of zoom should scale pan by ~1000x");
    return true;
}

bool TestDragPanAspectRatio() {
    // A full-width drag on a wider viewport should cover more world space.
    // Use full-width drags (drag_dx == resolution_x) to isolate the aspect effect.
    double cx = 0.0, cy = 0.0;
    // 16:9 aspect: full-width drag
    auto r_wide = ComputeDragPan(cx, cy, 5.0, 1920.0f, 0.0f, 1920, 1080);
    // 1:1 aspect: full-width drag
    auto r_square = ComputeDragPan(cx, cy, 5.0, 1024.0f, 0.0f, 1024, 1024);
    
    double dx_wide = std::fabs(r_wide.new_center_hp_x);
    double dx_square = std::fabs(r_square.new_center_hp_x);
    // Full-width drag: dxWorld = 2 * base * aspect; wider aspect => bigger delta
    ASSERT(dx_wide > dx_square * 1.5, "wider viewport full-width drag covers more world space");
    return true;
}

bool TestDragPanReversibility() {
    // Pan right then left by same amount should return to start.
    double cx = 1.5, cy = -0.3;
    auto r1 = ComputeDragPan(cx, cy, 8.0, 200.0f, 150.0f, 1024, 768);
    auto r2 = ComputeDragPan(r1.new_center_hp_x, r1.new_center_hp_y, 8.0,
                              -200.0f, -150.0f, 1024, 768);
    ASSERT_NEAR(r2.new_center_hp_x, cx, 1e-12, "pan+reverse should restore x");
    ASSERT_NEAR(r2.new_center_hp_y, cy, 1e-12, "pan+reverse should restore y");
    return true;
}

bool TestApplyDragPanStepZeroDeltaNoOp() {
    ViewState view{};
    view.center_hp_x = 1.25;
    view.center_hp_y = -0.75;
    view.log2_zoom = 6.0;
    SyncViewUiFromHp(view);

    const float centerXBefore = view.center.x;
    const float centerYBefore = view.center.y;
    ASSERT(!ApplyDragPanStep(view, 0.0f, 0.0f, 1024, 768), "zero drag delta should not report a viewport change");
    ASSERT_NEAR(view.center_hp_x, 1.25, 1e-15, "zero drag delta should preserve center_hp_x");
    ASSERT_NEAR(view.center_hp_y, -0.75, 1e-15, "zero drag delta should preserve center_hp_y");
    ASSERT_NEAR(view.center.x, centerXBefore, 1e-7f, "zero drag delta should preserve synced center.x");
    ASSERT_NEAR(view.center.y, centerYBefore, 1e-7f, "zero drag delta should preserve synced center.y");
    return true;
}

bool TestApplyDragPanStepUpdatesView() {
    ViewState view{};
    view.center_hp_x = 0.0;
    view.center_hp_y = 0.0;
    view.log2_zoom = 0.0;

    ASSERT(ApplyDragPanStep(view, 128.0f, 64.0f, 1024, 512), "non-zero drag delta should report a viewport change");
    ASSERT(view.center_hp_x < 0.0, "dragging right should move the center left");
    ASSERT(view.center_hp_y < 0.0, "dragging down should move the center up");
    ASSERT(view.zoom > 0.0f, "non-zero drag delta should keep UI fields synced");
    return true;
}

// --- Viewport display layout ---

bool TestViewportLayoutUsesFittedImageRectForWideAvailableRegion() {
    ViewportDisplayLayout layout = ComputeViewportDisplayLayout({2048, 1152}, {2048, 1536});
    ASSERT(layout.image_size.x == 1536, "4:3 source in a 16:9 region should fit to displayed image width, not full available width");
    ASSERT(layout.image_size.y == 1152, "4:3 source in a 16:9 region should use full available height");
    return true;
}

bool TestViewportLayoutKeepsWideSourceAtFullWideRegion() {
    ViewportDisplayLayout layout = ComputeViewportDisplayLayout({2048, 1152}, {2048, 1152});
    ASSERT(layout.image_size.x == 2048, "matching wide source should use full available width");
    ASSERT(layout.image_size.y == 1152, "matching wide source should use full available height");
    return true;
}

bool TestViewportLayoutScalesPreviewFrameToSameDisplayedRect() {
    ViewportDisplayLayout layout = ComputeViewportDisplayLayout({2048, 1152}, {1024, 768});
    ASSERT(layout.image_size.x == 1536, "half-res 4:3 preview should draw into the same 4:3 image rect");
    ASSERT(layout.image_size.y == 1152, "half-res 4:3 preview should preserve displayed image height");
    return true;
}

bool TestViewportLayoutRejectsInvalidInput() {
    ViewportDisplayLayout layout = ComputeViewportDisplayLayout({2048, 1152}, {0, 768});
    ASSERT(layout.image_size.x == 0, "invalid source width should produce no image width");
    ASSERT(layout.image_size.y == 0, "invalid source width should produce no image height");
    return true;
}

// --- Combined interaction tests ---

bool TestZoomThenPanConsistency() {
    // Zoom in, then pan; verify the world-space displacement is correctly scaled.
    double cx = 0.0, cy = 0.0, log2z = 0.0;
    
    // Pan at zoom=1 (log2=0)
    auto pan_z1 = ComputeDragPan(cx, cy, 0.0, 100.0f, 0.0f, 512, 512);
    double world_dx_z1 = std::fabs(pan_z1.new_center_hp_x);
    
    // Pan at zoom=1024 (log2=10)
    auto pan_z1024 = ComputeDragPan(cx, cy, 10.0, 100.0f, 0.0f, 512, 512);
    double world_dx_z1024 = std::fabs(pan_z1024.new_center_hp_x);
    
    // Zoom factor = 2^10 = 1024; world delta should scale inversely
    double ratio = world_dx_z1 / world_dx_z1024;
    ASSERT_NEAR(ratio, 1024.0, 50.0, "pan world delta should scale with zoom ratio");
    return true;
}

bool TestDeepZoomPrecision() {
    // At very deep zoom, verify no precision loss in center coordinates.
    double cx = 0.123456789012345, cy = -0.987654321098765;
    double deep_log2z = 40.0;  // zoom ~ 10^12
    
    auto r = ComputeZoomAroundCursor(cx, cy, deep_log2z, 0.5f, 0.5f, 1.0f, 1.0);
    
    // Center zoom at exact center should preserve coordinates perfectly.
    ASSERT_NEAR(r.new_center_hp_x, cx, 1e-14, "deep zoom preserves x precision");
    ASSERT_NEAR(r.new_center_hp_y, cy, 1e-14, "deep zoom preserves y precision");
    return true;
}

#define RUN(fn) do { \
    if (fn()) { std::fprintf(stderr, "  PASS: %s\n", #fn); } \
    else { std::fprintf(stderr, "  FAIL: %s\n", #fn); } \
} while (0)

int main() {
    // ClampF
    RUN(TestClampFInRange);
    RUN(TestClampFBelowMin);
    RUN(TestClampFAboveMax);
    RUN(TestClampFAtBoundary);
    
    // ApplyAutoDiveStep
    RUN(TestAutoDiveDisabled);
    RUN(TestAutoDiveManualCamera);
    RUN(TestAutoDiveOffCamera);
    RUN(TestAutoDiveZeroSpeed);
    RUN(TestAutoDiveNegativeSpeed);
    RUN(TestAutoDiveNormalStep);
    RUN(TestAutoDiveHighSpeed);
    RUN(TestAutoDiveSyncsUiFields);
    
    // ComputeZoomAroundCursor
    RUN(TestZoomCenterCursorInvariant);
    RUN(TestZoomOutDecreasesZoom);
    RUN(TestZoomFixedPointProperty);
    RUN(TestZoomAspectRatioEffect);
    RUN(TestZoomReversibility);
    
    // ComputeDragPan
    RUN(TestDragPanZeroDelta);
    RUN(TestDragPanRight);
    RUN(TestDragPanDown);
    RUN(TestDragPanScalesWithZoom);
    RUN(TestDragPanAspectRatio);
    RUN(TestDragPanReversibility);
    RUN(TestApplyDragPanStepZeroDeltaNoOp);
    RUN(TestApplyDragPanStepUpdatesView);

    // Viewport display layout
    RUN(TestViewportLayoutUsesFittedImageRectForWideAvailableRegion);
    RUN(TestViewportLayoutKeepsWideSourceAtFullWideRegion);
    RUN(TestViewportLayoutScalesPreviewFrameToSameDisplayedRect);
    RUN(TestViewportLayoutRejectsInvalidInput);
    
    // Combined
    RUN(TestZoomThenPanConsistency);
    RUN(TestDeepZoomPrecision);
    
    std::fprintf(stderr, "test_viewport_interaction: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed > 0 ? 1 : 0;
}
