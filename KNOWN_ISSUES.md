# Known Issues

Catalogued issues for the CUDA fractal viewer clone repo.
Priority tiers: **P0** (blocks demo quality), **P1** (materially degrades experience), **P2** (nice to fix).

---

## P0 — Nova violates its own escape-time contract

**Status:** open
**Area:** renderer / presets / schema

Nova is visibly wrong because the shipped code mixes two incompatible contracts:
- The iteration loop implements Nova as an escape-time hybrid (`z = z - alpha * f(z)/f'(z) + c`, `z0 = 0`, `c = coord`, escape radius 2).
- The preset layer still defaults Nova to `joy_basins` coloring.
- The renderer still routes Nova through the root-finding coloring branch instead of the escape-time coloring branch.
- The schema help text and visibility rules still describe Nova as basin-colored alongside Newton and the Explaino family.

Result:
- Nova does not present like a proper escape-time fractal.
- The default UI state lands on a misleading coloring mode.
- Renderer comments and actual branch structure disagree, which makes future work riskier.

Required fix shape:
1. Treat Nova as escape-time everywhere: presets, schema visibility/help text, and renderer coloring dispatch.
2. Add focused headless tests for Nova preset defaults and coloring validity.
3. Re-capture diagnostics for Nova after the fix to choose a better default view and exposure.

**Files:** `ui_app/src/fractal_renderer.cu`, `ui_app/src/fractal_derived_fields.cpp`, `ui/fractal_binding_surface_v1.ui_schema*.json`

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

## P2 — Sweep mode combined-seed fix not fully validated

**Status:** partially addressed (code patched, helper tests green, sweep-mode not visually re-verified)
**Area:** sweep viewer

The sweep-mode seed path was patched to use `ExplainoSeedSetCombined()` and pause/step controls were added, but the fix has not been visually confirmed in a live sweep session since the build succeeded.

**File:** `ui_app/src/main.cpp` — sweep block
