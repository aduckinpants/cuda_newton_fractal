#pragma once

#include "fractal_types.h"

#include <cmath>

// Shared high-precision (HP) view math for both headless and interactive flows.
inline constexpr double kMinZoom = 1.0e-12;
inline constexpr double kMaxLog2Zoom = 1020.0;

double ClampD(double v, double lo, double hi);
double Log2D(double v);
double Exp2D(double v);
double SafeZoomFromLog2(double log2Zoom);
double ClampInteractionLog2Zoom(double log2Zoom);

void SyncViewHpFromUi(ViewState& view);
void SyncViewUiFromHp(ViewState& view);