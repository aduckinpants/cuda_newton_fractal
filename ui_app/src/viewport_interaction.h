#pragma once

#include "fractal_types.h"
#include "view_hp_sync.h"

// Pure viewport interaction math extracted from main.cpp.
// All functions are headless-testable (no ImGui, no D3D dependencies).

// Clamp a float to [lo, hi].
inline float ClampF(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

// Apply one frame of auto-dive zoom toward the current view center.
// Returns true if the view was modified.
bool ApplyAutoDiveStep(ViewState& view);

// Result of a zoom-around-cursor operation.
struct ZoomAroundCursorResult {
    double new_center_hp_x;
    double new_center_hp_y;
    double new_log2_zoom;
};

// Compute zoom-around-cursor: zooms by mouse wheel delta, keeping the
// world point under the cursor fixed.
//
// cursor_u, cursor_v: normalized cursor position in viewport [0,1].
// mouse_wheel: signed wheel delta (positive = zoom in).
// aspect: width/height ratio of the render resolution.
// current view state read from: center_hp_x, center_hp_y, log2_zoom.
ZoomAroundCursorResult ComputeZoomAroundCursor(
    double center_hp_x, double center_hp_y, double log2_zoom,
    float cursor_u, float cursor_v, float mouse_wheel, double aspect);

// Result of a drag-pan operation.
struct DragPanResult {
    double new_center_hp_x;
    double new_center_hp_y;
};

// Compute drag-pan: translates the view center by a pixel-space drag delta.
//
// drag_dx, drag_dy: drag delta in pixels.
// resolution_x, resolution_y: render resolution in pixels.
// current view state read from: center_hp_x, center_hp_y, log2_zoom.
DragPanResult ComputeDragPan(
    double center_hp_x, double center_hp_y, double log2_zoom,
    float drag_dx, float drag_dy, int resolution_x, int resolution_y);

// Apply one viewport drag-pan step to the current view.
// Returns true only when a non-zero drag delta changed the view.
bool ApplyDragPanStep(
    ViewState& view,
    float drag_dx,
    float drag_dy,
    int resolution_x,
    int resolution_y);
