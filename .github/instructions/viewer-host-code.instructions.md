---
description: "Use when editing CUDA, C++, or DX11/ImGui viewer-host code in this repo. Covers extraction from main.cpp, schema authority, no-implicit-fallback behavior, and headless test seams."
name: "Viewer Host Code Rules"
applyTo: "ui_app/src/**/*.{cpp,cu,h}"
---
# Viewer Host Code Rules

- Treat `ui_app/src/main.cpp` as a monolith to extract from, not a place to keep accumulating new systems.
- Prefer small focused modules such as `viewer_input`, `viewer_hud`, `surface_provider`, `viewport_*`, and `schema_binding`.
- Reuse or preserve the existing high-precision camera path (`center_hp_x/y`, `log2_zoom`) rather than reintroducing float-only view math.
- Keep `ViewState` and `KernelParams` separate from viewer-model types.
- Unknown binding paths, enum ids, or invalid parameters must produce explicit errors or validation failures, not silent fallback.
- When a behavior can be isolated, add or update a focused headless test before changing the implementation.
- Do not create a second UI source of truth; schema drives layout, C++ structs drive runtime state.
- Parameter animation targets are resolved at runtime via BindFloat — do NOT add enums, switch/case, or manual dispatch. To make a new float param animatable: add a BindFloat entry in schema_binding.cpp + a dropdown option in the schema JSON. See `param_anim_dynamics.cpp` and `test_param_anim_generic.cpp`.