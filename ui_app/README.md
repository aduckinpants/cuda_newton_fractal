# CUDA Newton Fractal — UI Demo (ImGui + DX11)

This is the interactive UI front-end for the CUDA Newton fractal demo.

The UI is schema-driven: it loads one checked-in JSON schema at startup and generates the control surface from `panels[]/controls[]` + bindings.

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

Stable runtime launcher after build:

```powershell
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd
```

The build tries to publish `fractal_ui.exe` first. If that file is locked, it falls back to `fractal_ui_dev.exe` and updates `fractal_ui.cmd` plus `fractal_ui_active.txt` to point at the latest successful runtime.
It also stages `fractal_ui_repo_root.txt` so manual finding capture can still locate the source-repo archive script when the app is launched from the published runtime directory.

Focused helper tests:

```powershell
cd C:\code\cuda_newton_fractal_clone\ui_app
.\build_tests_vsdevcmd.cmd
```

## Run

```powershell
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd
```

Headless validation and diagnostics capture:

```powershell
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --validate-ui
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --capture-diagnostic --fractal-type explaino --explaino-seed 0.75
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --load-state-json D:\salt-fractal\cuda_newton_fractal_clone\findings\manual\state.json --capture-diagnostic
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --capture-finding --finding-group manual_capture --finding-why "Headless review capture."
```

Callable headless surface reference:

- [../docs/callable_engine_surface.md](../docs/callable_engine_surface.md)

Representative callable examples:

```powershell
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --describe-functions
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --load-state-json state.json --explore-recommend
```

Diagnostics bundle output:

```powershell
D:\salt-fractal\cuda_newton_fractal_clone\runtime\diagnostics\last
```

Live sweep viewer:

```powershell
D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.cmd --fractal-type explaino --sweep-seed-start 0.70 --sweep-seed-stop 0.80 --sweep-seed-step 0.005 --sweep-dwell-ms 500 --sweep-loop
```

Deterministic seed sweep:

```powershell
cd C:\code\cuda_newton_fractal_clone
py -3.14 tools\reality_toolkit\scripts\run_fractal_explorer_seed_sweep.py --seed-start 0.70 --seed-stop 0.80 --seed-step 0.02 --out-dir D:\salt-fractal\cuda_newton_fractal_clone\artifacts\seed_sweep_2026-04-05
py -3.14 tools\reality_toolkit\scripts\run_fractal_explorer_seed_sweep.py --seed-start 0.70 --seed-stop 0.80 --seed-step 0.02 --archive-findings --finding-group explaino_seed_scout
```

Publish older repo-local artifacts and logs:

```powershell
cd C:\code\cuda_newton_fractal_clone
pwsh -File .\tools\publish_repo_artifacts.ps1 -Label cleanup
```

## UI schema (single source of truth)

At startup, the app loads:

- `..\ui\fractal_binding_surface_v1.ui_schema.json` (relative to `ui_app`)  
	Published runtime path: `D:\salt-fractal\cuda_newton_fractal_clone\runtime\ui\fractal_binding_surface_v1.ui_schema.json`
	Source path: `C:\code\cuda_newton_fractal_clone\ui\fractal_binding_surface_v1.ui_schema.json`

To tweak the UI without changing code:

1) Edit the schema:
	- `C:\code\cuda_newton_fractal_clone\ui\fractal_binding_surface_v1.ui_schema.json`
2) Rebuild or relaunch `fractal_ui.exe`

Notes:
- Schema `default` values are applied at app startup.
- `visible_if` currently supports `eq/neq/lt/lte/gt/gte` (where meaningful) and fails-open if a predicate cannot be evaluated.
- The Explaino family is selectable from schema and the headless CLI, but seed-base input is still CLI-first in this repo.
- Use `fractal_ui.cmd` for scripted or headless runs so the current active runtime stays stable even when `fractal_ui.exe` is locked.

Notes:
- Runtime, diagnostics, and sweep outputs now default to `D:\salt-fractal\cuda_newton_fractal_clone\...`.
- If build fails on missing Windows SDK libs (e.g., `d3d11.lib`), install the Windows 11 SDK component in Visual Studio.
- Cross-repo note: this repo's local `tools/reality_toolkit` is the beginning of a `reality-toolkit-fractals` extension surface. Use `nine` as the upstream analysis-toolkit reference and prefer its shared broker/viewer flow if this repo can register and retire sessions cleanly inside that surface. If that proves noisy or stomps in-flight work, fall back to a repo-local isolated broker stack.
