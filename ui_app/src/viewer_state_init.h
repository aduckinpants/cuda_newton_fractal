#pragma once

#include "explaino_sidecar_model.h"
#include "fractal_types.h"
#include "viewer_cli.h"
#include "ui_schema.h"
#include "schema_binding.h"

int ApplyCliOverrides(const ViewerCliArgs& cli,
                      ViewState& view, KernelParams& params,
                      RenderSettings& render,
                      SidecarOrientationVector* outLoadedOrientation,
                      bool* outHasLoadedOrientation,
                      bool* dirty);

// Apply CLI overrides (fractal type, explaino params, resolution, sweep, etc.)
// to runtime state after schema defaults have been applied.
// Returns 0 on success, nonzero on fatal error.
int ApplyCliOverrides(const ViewerCliArgs& cli,
                      ViewState& view, KernelParams& params,
                      RenderSettings& render, bool* dirty);
