#include "viewport_interaction.h"

#include <cmath>

ViewportDisplayLayout ComputeViewportDisplayLayout(
    const Int2& availablePixels,
    const Int2& sourceResolution) {
    ViewportDisplayLayout layout{};
    layout.source_resolution = sourceResolution;

    if (availablePixels.x <= 0 || availablePixels.y <= 0 ||
        sourceResolution.x <= 0 || sourceResolution.y <= 0) {
        return layout;
    }

    const double sx = static_cast<double>(availablePixels.x) / static_cast<double>(sourceResolution.x);
    const double sy = static_cast<double>(availablePixels.y) / static_cast<double>(sourceResolution.y);
    double scale = sx < sy ? sx : sy;
    if (!std::isfinite(scale) || scale <= 0.0) {
        scale = 1.0;
    }

    layout.image_size.x = static_cast<int>(std::floor(static_cast<double>(sourceResolution.x) * scale + 0.5));
    layout.image_size.y = static_cast<int>(std::floor(static_cast<double>(sourceResolution.y) * scale + 0.5));
    if (layout.image_size.x > availablePixels.x) layout.image_size.x = availablePixels.x;
    if (layout.image_size.y > availablePixels.y) layout.image_size.y = availablePixels.y;
    if (layout.image_size.x < 0) layout.image_size.x = 0;
    if (layout.image_size.y < 0) layout.image_size.y = 0;
    return layout;
}

bool ApplyAutoDiveStep(ViewState& view) {
    if (!view.auto_dive) return false;
    if (view.camera_behavior == CameraBehavior::manual || view.camera_behavior == CameraBehavior::off) return false;

    float speed = fmaxf(0.0f, view.dive_speed);
    if (speed <= 0.0f) return false;

    double zoomFactor = 1.0 + 0.002 * (double)speed;
    double dlog2 = Log2D(zoomFactor);
    view.log2_zoom = ClampInteractionLog2Zoom(view.log2_zoom + dlog2);
    SyncViewUiFromHp(view);
    return true;
}

ZoomAroundCursorResult ComputeZoomAroundCursor(
    double center_hp_x, double center_hp_y, double log2_zoom,
    float cursor_u, float cursor_v, float mouse_wheel, double aspect) {

    double zoomOld = SafeZoomFromLog2(log2_zoom);
    double baseOld = 2.0 / fmax(1e-30, zoomOld);
    double nx = ((double)cursor_u - 0.5) * 2.0;
    double ny = ((double)cursor_v - 0.5) * 2.0;

    double worldX = center_hp_x + nx * baseOld * aspect;
    double worldY = center_hp_y + ny * baseOld;

    double factor = pow(1.10, (double)mouse_wheel);
    double newLog2Zoom = ClampInteractionLog2Zoom(log2_zoom + Log2D(factor));
    double zoomNew = SafeZoomFromLog2(newLog2Zoom);
    double baseNew = 2.0 / fmax(1e-30, zoomNew);

    ZoomAroundCursorResult r;
    r.new_center_hp_x = worldX - nx * baseNew * aspect;
    r.new_center_hp_y = worldY - ny * baseNew;
    r.new_log2_zoom = newLog2Zoom;
    return r;
}

DragPanResult ComputeDragPan(
    double center_hp_x, double center_hp_y, double log2_zoom,
    float drag_dx, float drag_dy, int resolution_x, int resolution_y) {

    double aspect = (resolution_y > 0) ? (double)resolution_x / (double)resolution_y : 1.0;
    double zoomNow = SafeZoomFromLog2(log2_zoom);
    double base = 2.0 / fmax(1e-30, zoomNow);

    double dxWorld = ((double)drag_dx / (double)resolution_x) * 2.0 * base * aspect;
    double dyWorld = ((double)drag_dy / (double)resolution_y) * 2.0 * base;

    DragPanResult r;
    r.new_center_hp_x = center_hp_x - dxWorld;
    r.new_center_hp_y = center_hp_y - dyWorld;
    return r;
}

bool ApplyDragPanStep(
    ViewState& view,
    float drag_dx,
    float drag_dy,
    int resolution_x,
    int resolution_y) {
    if (drag_dx == 0.0f && drag_dy == 0.0f) {
        return false;
    }
    if (resolution_x <= 0 || resolution_y <= 0) {
        return false;
    }

    const DragPanResult pan = ComputeDragPan(
        view.center_hp_x,
        view.center_hp_y,
        view.log2_zoom,
        drag_dx,
        drag_dy,
        resolution_x,
        resolution_y);
    view.center_hp_x = pan.new_center_hp_x;
    view.center_hp_y = pan.new_center_hp_y;
    SyncViewUiFromHp(view);
    return true;
}
