#include "view_hp_sync.h"

double ClampD(double v, double lo, double hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

double Log2D(double v) {
    return std::log(v) / std::log(2.0);
}

double Exp2D(double v) {
    return std::exp(v * std::log(2.0));
}

double SafeZoomFromLog2(double log2Zoom) {
    return Exp2D(ClampD(log2Zoom, Log2D(1.0e-30), kMaxLog2Zoom));
}

double ClampInteractionLog2Zoom(double log2Zoom) {
    return ClampD(log2Zoom, Log2D(kMinZoom), kMaxLog2Zoom);
}

void SyncViewHpFromUi(ViewState& view) {
    view.center_hp_x = static_cast<double>(view.center.x);
    view.center_hp_y = static_cast<double>(view.center.y);
    view.log2_zoom = Log2D(std::fmax(1.0e-30, static_cast<double>(view.zoom)));
}

void SyncViewUiFromHp(ViewState& view) {
    double zoom = SafeZoomFromLog2(view.log2_zoom);
    zoom = ClampD(zoom, 1.0e-30, 1.0e30);
    view.zoom = static_cast<float>(zoom);
    view.center.x = static_cast<float>(view.center_hp_x);
    view.center.y = static_cast<float>(view.center_hp_y);
}