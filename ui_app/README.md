# CUDA Newton Fractal — UI Demo (ImGui + DX11)

This is the interactive UI front-end for the CUDA Newton fractal demo.

The UI is schema-driven: it loads the editable UI schema at startup and generates the control surface from `panels[]/controls[]` + bindings.

## One-time setup (Dear ImGui)

```powershell
# Run from the ui_app folder (path can be the workspace junction)
cd .\
powershell -ExecutionPolicy Bypass -File .\setup_imgui.ps1
```

## Build

Uses the same Visual Studio dev environment path as the core demo.

```powershell
cd .\
.\build_vsdevcmd.cmd
```

## Run

```powershell
cd .\
.\fractal_ui.exe
```

## UI schema (fail-fast dev workflow)

By default (dev-friendly, fail-fast), the app loads the editable schema:

- `..\ui\fractal_binding_surface_v1.ui_schema.json` (relative to `ui_app`)

To make schema selection fully deterministic in scripts/CI, pass an explicit schema path:

- `--ui-schema ..\ui\fractal_binding_surface_v1.ui_schema.json`

Canonical schema is intentionally not part of the fast dev loop.

To tweak the UI without changing code:

1) Edit the schema:
	- `..\ui\fractal_binding_surface_v1.ui_schema.json`
2) Validate quickly:
	- `..\fractal_ui.exe --validate-ui --ui-schema ..\ui\fractal_binding_surface_v1.ui_schema.json`
3) Optional (packaging later): re-canonicalize for release tooling:
	- `python project\imgui_spec\validate_schema.py --in ..\ui\fractal_binding_surface_v1.ui_schema.json --out ..\ui\fractal_binding_surface_v1.ui_schema.canonical.json`

Notes:
- Schema `default` values are applied at app startup.
- `visible_if` currently supports `eq/neq/lt/lte/gt/gte` (where meaningful) and fails-open if a predicate cannot be evaluated.

Notes:
- The build scripts `cd` to their own directory, so they work via the workspace junction path and avoid accidental builds against a different external folder.
- If build fails on missing Windows SDK libs (e.g., `d3d11.lib`), install the Windows 11 SDK component in Visual Studio.
