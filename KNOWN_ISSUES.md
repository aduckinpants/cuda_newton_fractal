# Known Issues

Catalogued issues for the CUDA fractal viewer clone repo.
Priority tiers: **P0** (blocks demo quality), **P1** (materially degrades experience), **P2** (nice to fix).

---

## ~~P0 — Nova violates its own escape-time contract~~ RESOLVED

**Status:** fixed (common-fractal wave + Nova repair slice)

Nova now correctly treated as escape-time everywhere: presets, schema visibility, renderer coloring dispatch.
Focused headless tests added for Nova defaults and coloring validity.
Probe coverage confirms Nova is sampleable and escape-time classified.

---

## P0 — Dive depth is shallow and untuned

**Status:** open
**Area:** camera / auto-dive

The auto-dive loop (`ApplyAutoDivePerFrame`) applies a fixed per-frame zoom increment of `1.0 + 0.002 * speed`. At the default `dive_speed = 1.0` the viewer zooms in very slowly and the resulting depth never reaches visually dramatic levels in a reasonable session.

Root causes:
- The zoom-per-frame constant (0.002) was chosen conservatively during initial bringup.
- There is no acceleration curve; the dive rate does not increase as zoom deepens.
- There is no dt-aware timing; the increment is per-frame, so dive speed depends on framerate.
- There is no per-fractal tuning of dive targets or interesting-region steering.
- The camera behavior enum (`complexity`, `orbit`, `entropy`) is wired to the schema but only `complexity` is implemented, and its implementation is the same flat zoom increment.

Potential fixes:
1. Make `dlog2` proportional to `io.DeltaTime` so dive speed is framerate-independent.
2. Add an acceleration ramp: e.g. `dlog2 = base_rate * (1.0 + 0.1 * log2_zoom)`.
3. Set per-fractal dive-speed defaults (escape-time fractals benefit from deeper zoom than root-finders).
4. Implement the `complexity` camera behavior properly: steer toward high-iteration-count regions, not just straight zoom.

**File:** `ui_app/src/main.cpp` — `ApplyAutoDivePerFrame()`
**Depends on:** view_hp_sync, fractal_types.h

---

## P1 — Smooth-escape coloring needs per-type tuning

**Status:** partially addressed (cyclic palette landed in 34b7024)
**Area:** renderer / coloring

The 5-stop cyclic palette is a large improvement over the flat ramp, but:
- The band period (`nu * 0.025`) is a single global constant; escape-time families have different iteration profiles.
- Interior (non-escaped) pixels are solid black regardless of fractal type.
- No histogram equalization or adaptive normalization; at very deep zoom the visible band range collapses.

**File:** `ui_app/src/fractal_renderer.cu` — escape-time coloring block

---

## P1 — No view preset dropdown

**Status:** open (idea only, no prior sketch found)
**Area:** UI / schema

When switching fractal types, each type lands on a single hardcoded canonical view. There is no dropdown to pick from multiple interesting locations (e.g. Seahorse Valley, Elephant Valley, Mini-brot for Mandelbrot alone).

Design direction:
- A `view_preset` combo in the View panel, populated from a small JSON catalog per fractal type.
- Selecting a preset applies center + zoom + optional rotation.
- "Custom" entry preserves the current user-navigated view.

**File:** `ui_app/src/fractal_derived_fields.cpp` — `ApplyFractalViewPresetDefaults()`

---

## P1 — Camera behavior modes are stubs

**Status:** open
**Area:** camera

`CameraBehavior::complexity`, `orbit`, `entropy` are selectable in the UI but all three execute the same flat zoom increment. The original design intent was:
- `complexity` — steer toward high-iteration regions (minibrots, spiral arms).
- `orbit` — follow a periodic orbit path.
- `entropy` — seek visually complex regions based on render-buffer variance.

None of these have real implementations yet.

**File:** `ui_app/src/main.cpp` — `ApplyAutoDivePerFrame()`

---

## P1 — Advanced color reset and mode controls can desync live color state

**Status:** open
**Area:** advanced color / UI / reset semantics

Current symptoms:
- `Reset All` can leave the color-area tint and brightness/saturation/contrast style controls in a non-neutral state even after the rest of the fractal runtime is reset.
- The legacy color-mode dropdown path and the advanced-color pipeline dropdown path are still confusing and can interact in bad ways, leaving advanced color in partially adopted or stale live states.
- Under those desynchronized states, the `smooth_escape_ramp` source row can appear broken or no-op, but the root cause is not isolated yet between stale reset/default behavior and the dual-dropdown authority problem.

Likely ownership hotspots:
- `ui_app/src/main.cpp` — reset-all dispatch and the legacy `coloring_mode` UI path
- `ui_app/src/runtime_reset.cpp` — reset/default application
- `ui_app/src/color_pipeline_window.h` — draft/live adoption, lane selection, and advanced-color import/apply behavior
- `ui_app/src/escape_time_coloring.h` — runtime smooth-escape signal path once state reaches the renderer

Potential fixes:
1. Make `Reset All` re-neutralize the color-area tint and brightness/saturation/contrast style state at the same authority layer that resets the advanced-color lane owners.
2. Collapse or hard-disable conflicting dropdown combinations so one control surface owns the live color path at a time instead of letting legacy and advanced-color state fight.
3. Add a focused regression that toggles reset-all plus legacy/advanced-color dropdown transitions and proves the smooth escape source still changes rendered output.

**Files:** `ui_app/src/main.cpp`, `ui_app/src/runtime_reset.cpp`, `ui_app/src/color_pipeline_window.h`, `ui_app/src/escape_time_coloring.h`

---

## P2 — Perturbation deep zoom only for Mandelbrot / Julia

**Status:** open
**Area:** renderer

The perturbation reference-orbit path is implemented only for `Mandelbrot` and `Julia`. Other escape-time families (Burning Ship, Multibrot, Phoenix) lose precision at deep zoom because they run in float32.

**File:** `ui_app/src/fractal_renderer.cu` — perturbation block

---

## P2 — Phoenix parameterization is one-shot

**Status:** open
**Area:** presets

Phoenix has a single hardcoded `p = 0.5667 + 0i`. The canonical Phoenix fractal family has interesting structure at many different `p` values. A proper treatment would expose `p` as a UI slider (already bound to schema) and include 2-3 named presets.

**File:** `ui_app/src/fractal_derived_fields.cpp`

---

## P2 — Diagnostics capture is a single rolling bundle

**Status:** open
**Area:** tooling

Headless `--capture-diagnostic` always writes to `runtime/diagnostics/last/`. Concurrent or rapid sequential captures overwrite each other. A proper solution would timestamp each bundle or accept an `--out-dir` override.

**File:** `ui_app/src/diagnostics_capture.cpp`

---

## P2 — Sweep mode combined-seed fix live validation completed

**Status:** resolved (helper tests green and live sweep regression now covers motion plus Space-pause behavior)
**Area:** sweep viewer

The sweep-mode seed path now has both focused headless coverage and a live runtime regression. `ui_app/tests/test_viewer_sweep.cpp` pins combined-seed application plus pause/step semantics, and `tests/test_fractal_runtime_sweep_pause.py` verifies that the live viewer image changes while the sweep is running and becomes stable after a Space-key pause.

**Files:** `ui_app/src/main.cpp`, `ui_app/src/viewer_sweep.cpp`, `tests/test_fractal_runtime_sweep_pause.py`

---

## ~~P2 — Live GUI sweep regression test is fragile~~ RESOLVED

**Status:** resolved (2026-04-12 live-runtime proof hardening)
**Area:** testing

`tests/test_fractal_runtime_sweep_pause.py::test_runtime_sweep_changes_live_view_and_space_pauses_it` no longer relies on the stale `>1.0` whole-window diff threshold. The live harness now waits for a visible non-zero client area, retries until the window produces a real frame, and compares adjacent live intervals against the observed runtime signal. The headless sweep coverage in `ui_app/tests/test_viewer_sweep.cpp` remains the primary contract test, and the live runtime regression is now a trustworthy companion proof rather than a known-fragile threshold check.

Residual constraint: the live GUI runtime tests are still Windows-only and require a visible desktop session. That is an environment contract, not an open product bug.
