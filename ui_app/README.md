# CUDA Newton Fractal — UI Demo (ImGui + DX11)

This is the interactive UI front-end for the CUDA Newton fractal demo.

The UI is schema-driven: it loads the canonical ImGuiSpec JSON schema at startup and generates the control surface from `panels[]/controls[]` + bindings.

## One-time setup (Dear ImGui)

```powershell
cd C:\artifacts\cuda_newton_fractal\ui_app
powershell -ExecutionPolicy Bypass -File .\setup_imgui.ps1
```

## Build

Uses the same Visual Studio dev environment path as the core demo.

```powershell
cd C:\artifacts\cuda_newton_fractal\ui_app
.\build_vsdevcmd.cmd
```

## Run

```powershell
cd C:\artifacts\cuda_newton_fractal\ui_app
.\fractal_ui.exe
```

## UI schema (single source of truth)

At startup, the app loads:

- `..\ui\fractal_binding_surface_v1.ui_schema.canonical.json` (relative to `ui_app`)  
  Full path: `C:\artifacts\cuda_newton_fractal\ui\fractal_binding_surface_v1.ui_schema.canonical.json`

To tweak the UI without changing code:

1) Edit the non-canonical schema (recommended):
	- `C:\artifacts\cuda_newton_fractal\ui\fractal_binding_surface_v1.ui_schema.json`
2) Re-canonicalize/validate:
	- `python C:\artifacts\imgui_spec\validate_schema.py --in C:\artifacts\cuda_newton_fractal\ui\fractal_binding_surface_v1.ui_schema.json --out C:\artifacts\cuda_newton_fractal\ui\fractal_binding_surface_v1.ui_schema.canonical.json`
3) Relaunch `fractal_ui.exe`

Notes:
- Schema `default` values are applied at app startup.
- `visible_if` currently supports `eq/neq/lt/lte/gt/gte` (where meaningful) and fails-open if a predicate cannot be evaluated.

Notes:
- All artifacts stay under `C:\artifacts\...` per workspace hygiene.
- If build fails on missing Windows SDK libs (e.g., `d3d11.lib`), install the Windows 11 SDK component in Visual Studio.
