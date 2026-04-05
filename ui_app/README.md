# CUDA Newton Fractal — UI Demo (ImGui + DX11)

This is the interactive UI front-end for the CUDA Newton fractal demo.

The UI is schema-driven: it loads the canonical ImGuiSpec JSON schema at startup and generates the control surface from `panels[]/controls[]` + bindings.

## One-time setup (Dear ImGui)

```powershell
cd C:\code\cuda_newton_fractal_clone\ui_app
powershell -ExecutionPolicy Bypass -File .\setup_imgui.ps1
```

## Build

Uses the same Visual Studio dev environment path as the core demo.

```powershell
cd C:\code\cuda_newton_fractal_clone\ui_app
.\build_vsdevcmd.cmd
```

Default runtime location after build:

```powershell
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe
```

Focused helper tests:

```powershell
cd C:\code\cuda_newton_fractal_clone\ui_app
.\build_tests_vsdevcmd.cmd
```

## Run

```powershell
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe
```

Headless validation and diagnostics capture:

```powershell
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe --validate-ui
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe --capture-diagnostic --fractal-type explaino --explaino-seed 0.75
```

Diagnostics bundle output:

```powershell
D:\salt-fractal\cuda_newton_fractal_clone\runtime\diagnostics\last
```

Live sweep viewer:

```powershell
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe --fractal-type explaino --sweep-seed-start 0.70 --sweep-seed-stop 0.80 --sweep-seed-step 0.005 --sweep-dwell-ms 500 --sweep-loop
```

Deterministic seed sweep:

```powershell
cd C:\code\cuda_newton_fractal_clone
py -3.14 tools\reality_toolkit\scripts\run_fractal_explorer_seed_sweep.py --seed-start 0.70 --seed-stop 0.80 --seed-step 0.02 --out-dir D:\salt-fractal\cuda_newton_fractal_clone\artifacts\seed_sweep_2026-04-05
```

Publish older repo-local artifacts and logs:

```powershell
cd C:\code\cuda_newton_fractal_clone
pwsh -File .\tools\publish_repo_artifacts.ps1 -Label cleanup
```

## UI schema (single source of truth)

At startup, the app loads:

- `..\ui\fractal_binding_surface_v1.ui_schema.canonical.json` (relative to `ui_app`)  
	Published runtime path: `D:\salt-fractal\cuda_newton_fractal_clone\runtime\ui\fractal_binding_surface_v1.ui_schema.canonical.json`
	Source path: `C:\code\cuda_newton_fractal_clone\ui\fractal_binding_surface_v1.ui_schema.canonical.json`

To tweak the UI without changing code:

1) Edit the non-canonical schema (recommended):
	- `C:\code\cuda_newton_fractal_clone\ui\fractal_binding_surface_v1.ui_schema.json`
2) Re-canonicalize/validate:
	- `python C:\artifacts\imgui_spec\validate_schema.py --in C:\code\cuda_newton_fractal_clone\ui\fractal_binding_surface_v1.ui_schema.json --out C:\code\cuda_newton_fractal_clone\ui\fractal_binding_surface_v1.ui_schema.canonical.json`
3) Relaunch `fractal_ui.exe`

Notes:
- Schema `default` values are applied at app startup.
- `visible_if` currently supports `eq/neq/lt/lte/gt/gte` (where meaningful) and fails-open if a predicate cannot be evaluated.
- The Explaino family is selectable from schema and the headless CLI, but seed-base input is still CLI-first in this repo.

Notes:
- Runtime, diagnostics, and sweep outputs now default to `D:\salt-fractal\cuda_newton_fractal_clone\...`.
- If build fails on missing Windows SDK libs (e.g., `d3d11.lib`), install the Windows 11 SDK component in Visual Studio.
