# Known Issues

Catalogued issues for the CUDA fractal viewer clone repo.
Priority tiers: **P0** (blocks demo quality), **P1** (materially degrades experience), **P2** (nice to fix).

Last reconciled: 2026-05-28 on `codex/sdf-postprocess-roadmap-truth-sync` after the measured SDF postprocess optimization merged to `master`.

---

## ~~P0 - Nova violates its own escape-time contract~~ RESOLVED

**Status:** fixed (common-fractal wave + Nova repair slice)

Nova now correctly treated as escape-time everywhere: presets, schema visibility, renderer coloring dispatch.
Focused headless tests added for Nova defaults and coloring validity.
Probe coverage confirms Nova is sampleable and escape-time classified.

---

## P0 - Dive depth is shallow and untuned

**Status:** open
**Area:** camera / auto-dive

The auto-dive loop (`ApplyAutoDivePerFrame`) still applies a conservative flat zoom model. It is not dt-aware, does not accelerate with depth, and does not steer toward interesting regions.

Root causes:
- The zoom-per-frame constant was chosen conservatively during initial bringup.
- There is no acceleration curve; the dive rate does not increase as zoom deepens.
- There is no dt-aware timing; the increment is per-frame, so dive speed depends on framerate.
- There is no per-fractal tuning of dive targets or interesting-region steering.
- The camera behavior enum (`complexity`, `orbit`, `entropy`) is wired to the schema but still lacks real distinct behavior.

Potential fixes:
1. Make `dlog2` proportional to `io.DeltaTime` so dive speed is framerate-independent.
2. Add an acceleration ramp, bounded by stability and usability tests.
3. Set per-fractal dive-speed defaults and interesting-region presets.
4. Implement one real behavior mode first, preferably `complexity`, with deterministic proof around high-iteration or high-variance regions.

**File:** `ui_app/src/main.cpp` - `ApplyAutoDivePerFrame()`
**Depends on:** view preset/catalog work, `fractal_types.h`

---

## P1 - Smooth-escape coloring needs per-type tuning

**Status:** partially addressed (cyclic palette landed in 34b7024)
**Area:** renderer / coloring

The 5-stop cyclic palette is a large improvement over the flat ramp. `docs/notes/smooth_escape_interior_tone_PHASED_PLAN.md` repaired the forced-black smooth-escape interior path, `docs/notes/smooth_escape_collatz_luma_PHASED_PLAN.md` repaired measured Collatz-family low-luma / fast-escape black output, and `docs/notes/smooth_escape_low_unique_PHASED_PLAN.md` repaired the measured low-unique rows for Nova, McMullen, Magnet, Explaino-Nova, and Explaino-Rational-Escape. Remaining work:
- The band period (`nu * 0.025`) is a single global constant; escape-time families have different iteration profiles.
- No histogram equalization or adaptive normalization; at very deep zoom the visible band range collapses.
- Future visual tuning should use fresh measurement evidence rather than assuming the now-repaired low-unique rows are still current.

**File:** `ui_app/src/fractal_renderer.cu` - escape-time coloring block

---

## P1 - No view preset dropdown

**Status:** open
**Area:** UI / schema / catalog

When switching fractal types, each type lands on a single canonical view. There is no dropdown to pick from multiple interesting locations such as Seahorse Valley, Elephant Valley, Mini-brot, Phoenix bifurcation regions, Magnet examples, or Explaino family showcase views.

Design direction:
- A `view_preset` combo in the View panel, populated from a small catalog per fractal type.
- Selecting a preset applies center + zoom + optional rotation.
- `Custom` preserves the current user-navigated view.
- This pairs naturally with the categorized fractal selector; it should happen before adding many more catalog families.

**File:** `ui_app/src/fractal_derived_fields.cpp` - `ApplyFractalViewPresetDefaults()`

---

## P1 - Camera behavior modes are stubs

**Status:** open
**Area:** camera

`CameraBehavior::complexity`, `orbit`, and `entropy` are selectable in the UI, but they do not yet implement distinct user-visible behavior.

Original design intent:
- `complexity` - steer toward high-iteration or high-detail regions.
- `orbit` - follow a periodic orbit path.
- `entropy` - seek visually complex regions based on render-buffer variance.

**File:** `ui_app/src/main.cpp` - `ApplyAutoDivePerFrame()`

---

## P1 - Viewer responsiveness lacks end-to-end latency proof

**Status:** open measurement follow-up, not a current code-change license
**Area:** pacing / runtime harness

Later pacing/capture work repaired several concrete bugs: unknown-timing downscale, f32 over-aggression, f64/camera-center preview activation, nonzero live render timing, focused pacing helper target, and Capture Finding f64 output. The remaining gap is proof quality: current tests prove timing fields, preview dimensions, and settle behavior, but they do not fully measure end-to-end input-to-frame latency or whole-desktop responsiveness under extreme workloads.

Next correct step:
- Add a persistent no-mouse telemetry harness that records input mutation time, render start/end, report publish time, preview scale, target/live dimensions, and settle timing in one viewer process.
- Do not change pacing policy again until that harness shows the actual failure mode.
- For SDF Color Pipeline workloads specifically, scalar-only SDF postprocess optimization is shipped: scalar SDF Source rows no longer compute normal/curvature neighborhoods.
- SDF preview postprocess quality policy is shipped: interactive preview can reduce CPU SDF source samples through a reportable postprocess pixel step, while full-quality render and capture stay step 1.
- Full-quality downsampled-field postprocess cell reuse is shipped: shared `SDF Field Downsample` now reduces CPU postprocess samples by computing once per SDF field cell and expanding the same pixels the old nearest field-cell mapping produced.
- CUDA direct-scalar and field-signal SDF postprocess backends are shipped.
- Live-only adaptive SDF field resolution is shipped: interaction preview can temporarily use a higher effective SDF field downsample while settled, capture, and replay stay on the requested `LensSettings::downsample`.
- Field-generation stage telemetry, CUDA JFA buffer reuse, repeated median SDF witness reporting, measured CUDA SDF postprocess scratch-buffer reuse, row-local SDF field downsample UI/runtime authority, boundary-masked SDF Normal Angle Beauty, authored-pack live Color Pipeline/overlay field consumption, the first `sdf_pack_scene` selectable lane shell, the curated built-in SDF pack catalog seed, and the SDF drag camera mapping repair are shipped. The closed postprocess witness measured median improvements of `27%` to `62%` across identical SDF rows. The next unresolved SDF composition seam is typed Color Pipeline applicative glue for Source-row masks/weights; full authored-pack catalog/authoring UX, new SDF ops, and broader SDF-native productization remain later slices, not another debounce-policy guess.
- Do not treat SDF postprocess cost as a fractal kernel problem; the current telemetry distinguishes base render, field generation, and postprocess cost.

**Files:** `ui_app/src/viewer_render_pacing.*`, `ui_app/src/main.cpp`, `tests/test_fractal_runtime_resolution_pacing.py`

---

## P1 - Color Pipeline composition and effective-source UX are underdeveloped

**Status:** preset workflow truth, visible implementation-wording cleanup, SDF composition runtime-authority repairs, typed Source-row applicators, and mixed SDF/non-SDF Source rows for supported renderer-backed lanes are shipped; broader composition redesign remains deferred
**Area:** advanced color / SDF source rows / presets

The Color Pipeline has useful Source, Shape, Palette, and Grading rows, including SDF Source rows. Effective Source-stack summary/report clarity is shipped, UI-Salt materialized metadata now owns catalog descriptors, compatibility lookup, companion suggestions, and recipe expansion backend seams, product-facing preset workflow truth is shipped, and the current composition cleanup removes remaining visible draft/live-bridge wording without changing the internal row-stack implementation.

Remaining review direction:
- Preserve full-field `sdf_normal_angle` as a diagnostic phase view; SDF Normal Angle Beauty now provides the boundary-masked product-facing preset without deleting that diagnostic behavior.
- Treat SDF fields as typed Source signals now; SDF masks/gates and mixed SDF/non-SDF Source-row blending are shipped for supported renderer-backed lanes.
- Keep the repaired row-enabled/effective-selection and SDF-only Source-stack regressions green: disabled rows must stay inactive for compatibility/runtime, and `sdf_curvature` must keep blending with `sdf_normal_angle` through the runtime-backed SDF postprocess path.
- Keep row-local SDF Field Downsample authority explicit: SDF Source rows now default to shared `LensSettings::downsample` through `Inherit`, but individual rows can request `1x`, `2x`, `4x`, `8x`, or `16x` field resolution.
- Keep preset/workflow truth green so compatible fractal switches preserve authored work and unsupported rows project safely.
- Keep function picker/layout refinement as the next bounded UX step before adding more functions. Keep full preset manager UI, full authored SDF pack catalog/authoring UX, new SDF ops, and additional SDF-native lanes as separate slices.

---

## P1 - Advanced color reset and mode controls can desync live color state

**Status:** resolved by `ck:80387857`
**Area:** advanced color / UI / reset semantics

Resolution summary:
- `Reset All` restores Color-panel saturation, contrast, and tint defaults through the shared preset authority.
- While the advanced color window is open, simple `Coloring Mode` and `Grading` combos are disabled so the advanced editor owns the live color path.
- Existing smooth-escape runtime regressions stayed green without a renderer math change.

Checkpoint: `ck:80387857`

---

## P2 - Perturbation deep zoom only for Mandelbrot / Julia

**Status:** open
**Area:** renderer / precision

The perturbation reference-orbit path is implemented only for `Mandelbrot` and `Julia`. Other escape-time families such as Burning Ship, Multibrot, Phoenix, and Magnet still need recurrence-specific precision work before deep zoom can be trusted.

**File:** `ui_app/src/fractal_renderer.cu` - perturbation block

---

## ~~P2 - Phoenix parameterization is one-shot~~ RESOLVED

**Status:** resolved (parameter functionality campaign)
**Area:** presets / schema / runtime proof

Phoenix now exposes `phoenix_p_real` and `phoenix_p_imag` as runtime-backed controls and the no-mouse parameter proof campaign covers the visible control path. Future work may still add named Phoenix presets, but the old single-hardcoded-`p` control-surface bug is closed.

---

## ~~P2 - Diagnostics capture is a single rolling bundle~~ RESOLVED

**Status:** resolved
**Area:** tooling

Headless `--capture-diagnostic` now writes durable unique diagnostic archives by default, keeps `runtime/diagnostics/last/` as a compatibility mirror, and supports deterministic explicit output through `--diagnostics-out-dir` and `--out-dir`.

**Proof:** `docs/notes/diagnostics_capture_output_paths_PHASED_PLAN.md`

---

## ~~P2 - Lens SDF downsample/control truth is unresolved~~ RESOLVED

**Status:** resolved
**Area:** Lens SDF / control surface

`lens.downsample` is now wired into the live Lens SDF path, Lens SDF scalar field authority is extracted, and per-fractal Lens mask semantics are explicit.

**Proof:** `docs/notes/lens_sdf_truth_cleanup_PHASED_PLAN.md`, `docs/notes/sdf_field_interface_extraction_PHASED_PLAN.md`, `docs/notes/lens_semantics_authority_PHASED_PLAN.md`

---

## P2 - K4 low-tuning findings remain polish items

**Status:** open polish backlog
**Area:** defaults / visual tuning

`spec_intake/_STATUS.md` still records low-priority tuning findings:
- Collatz RGBA fast-escape pixels can render black.
- Some escape types have 3-12% max-iter exhaustion in the neither band.
- Nova / Explaino-Nova and Lambda / Explaino-Lambda defaults can be visually degenerate or low-interest.

These are not current parameter-authority bugs, but they are good low-risk polish candidates after view presets and color tuning are organized.

---

## P2 - Sweep mode combined-seed fix live validation completed

**Status:** resolved (helper tests green and live sweep regression covers motion plus Space-pause behavior)
**Area:** sweep viewer

The sweep-mode seed path now has focused headless coverage and a live runtime regression.

---

## ~~P2 - Live GUI sweep regression test is fragile~~ RESOLVED

**Status:** resolved (2026-04-12 live-runtime proof hardening)
**Area:** testing

`tests/test_fractal_runtime_sweep_pause.py::test_runtime_sweep_changes_live_view_and_space_pauses_it` no longer relies on the stale `>1.0` whole-window diff threshold. The live harness waits for a visible non-zero client area and compares against the observed runtime signal.

Residual constraint: the live GUI runtime tests are still Windows-only and require a visible desktop session. That is an environment contract, not an open product bug.
