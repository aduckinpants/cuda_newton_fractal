# Deferred Threads

Purpose: keep intentionally paused threads visible so they do not vanish between slices.

Active specs: see `spec_intake/_STATUS.md` for the current planning surface.
Agent protocol: see `AGENT_WORKING_PROTOCOL.md` for working rules.

## 1. Viewer Responsiveness / Adaptive Render Recovery

Status: deferred on 2026-04-06

Why paused:
- A viewport-capped gradual recovery experiment caused obvious stutter on normal Explaino panning.
- The live viewer path was reverted to the previous stepped-preview plus single settle-render behavior.

Current safe state:
- interaction-time preview scaling remains
- one full-quality settle render after debounce remains
- live width/height telemetry remains visible in the viewer UI
- Capture Finding rerenders at 4096x4096 instead of reusing the last live buffer

Resume constraints:
- do not change live pacing policy again without a focused runtime regression or telemetry harness for smoothness
- prefer instrumentation and measurement before policy changes
- keep capture-quality work separate from live-view responsiveness work

Key references:
- ui_app/src/viewer_render_pacing.h
- ui_app/src/viewer_render_pacing.cpp
- ui_app/src/main.cpp
- ui_app/tests/test_viewer_render_pacing.cpp

## 2. Explaino Design-Space Deep Dive

Status: deferred on 2026-04-06

Why paused:
- The observation lane and the family-growth lane should stay separate.
- Near-term priority is recognizable catalog growth without walking into a CUDA structure wall.

Resume constraints:
- use Salticid / CLI / broker probing first where possible
- keep joy/LUT/atlas work separate from new `FractalType` growth
- do not treat the legacy Explaino LUT as a new fractal type

Planning sources:
- spec_intake/ExplainoDesignSpace_DeepDive_2026-04-05.md
- spec_intake/ExplainoFamilyExpansion_V1_SpecIntake.md

Note: All 4 Explaino solver variants now landed (ripple, splice, vortex, tension).
The next Explaino work is the CUDA sample_fn extraction + Optimization Staging +
Reflexive sidecar initiative — see active specs in _STATUS.md.

## 3. Common Fractal Catalog Expansion + 2-Layer Dropdown

Status: queued, not started

User goals recorded for the next thread:
- make the fractal chooser a 2-layer / categorized UI
- change startup default to baseline `explaino` with `joy_basins` coloring (the current "Explaino joy" surface), not a new `explaino_joy` enum
- add more recognizable/common fractals without forcing a premature major renderer rewrite

Recommended next safe wave for the current 2D single-pass substrate:
- Spider
- Celtic Mandelbrot
- Perpendicular Burning Ship
- one preset-driven Magnet family only if the parameter surface stays explicit and bounded

Requested but not safe in the same bounded wave:
- Sierpinski gasket / carpet and Apollonian-style gaskets: these want an IFS / geometry contract, not just another complex-plane switch
- Menger sponge and Mandelbulb: these are 3D DE/raymarch work with a different camera, normal, and shading contract

Planning sources:
- spec_intake/CommonFractalCatalog_Deferred_2026-04-06.md
- spec_intake/FractalTypeDropdown_and_MultiFractalKernel_SpecIntake.md
- spec_intake/FractalCatalog_WaveTwo_SpecIntake.md

Stop rule:
- do at most one bounded catalog wave before the CUDA refactor thread below

## 4. CUDA Catalog Refactor Before Break-Wall

Status: partially complete (escape-time and specialized formula extraction landed),
        remaining work queued after active CUDA sample_fn initiative

Problem:
- the fractal renderer switch surface is growing faster than the shared math and family-rules seams

Completed seams:
- escape_time_direct_formulas.h (shared state-machine for 10 direct escape-time types)
- escape_time_specialized_formulas.h (McMullen, Collatz)
- perturbation_reference_orbit.h (perturbation cache/request/orbit)
- escape_time_coloring.h (palette + final color grading)
- explaino_collatz_formulas.h (Collatz residual/derivative/step)
- polynomial_eval_real_coeffs.h (Horner evaluators)
- basin_coloring.h (root-count, root-index, palette)
- fractal_runtime_validation.h (shared fail-fast param validation)

Remaining seams:
- explicit family defaults / visibility / validation outside the monolithic kernel switch
- cleaner organization for the future categorized dropdown surface

Timing rule:
- perform remaining refactoring before any 3D, IFS, distance-estimator, or Mandelbulb-style work
- the CUDA sample_fn extraction (K1-K5) will naturally drive further cleanup of the iteration path

## 5. Lens SDF Follow-Ups

Status: deferred on 2026-04-06

Why paused:
- The family-aware lens mask semantics are fixed for basin vs escape-time families.
- The remaining work is follow-up verification and control-surface cleanup, not a blocker for the selector/catalog thread.

Deferred follow-ups:
- visually verify Lens SDF output on escape-time families such as Phoenix, Mandelbrot, and Explaino-Lambda
- decide whether `lens.downsample` becomes a real render-path control or stays hidden/removed until it has behavior

Resume constraints:
- do not mix this with the organized-selector/common-fractal thread

## 6. CUDA-Resident sample_fn + Optimization Staging + Reflexive Sidecar

Status: **ACTIVE** — this is the current initiative (2026-04-09)

Three interconnected specs with a shared critical path. See `spec_intake/_STATUS.md`.

Critical path: K1-K3 (extract `fractal_sample_device()` from renderer)
  → enables Optimization Staging Phase 1 (measurement)
  → enables CLI Bridge V2 session protocol
  → enables Reflexive sidecar demonstrations

Specs:
- spec_intake/CliBridgeV2_GpuSampleFn_SpecIntake.md (K1-K5 kernel extraction + V2 session)
- spec_intake/OptimizationStaging_ExplainoZeroAxis_SpecIntake.md (zero-axis measurement + cost profiling)
- spec_intake/ExplainoAll_SmartSidecar_SpecIntake.md (Explaino Reflexive — engine explaining itself)

Design constraint: sample_fn is fully on CUDA. No CPU fallback.

First slice: K1 — extract `fractal_sample_device()` __device__ function from
`fractal_renderer.cu`, with a focused headless test proving equivalence to the
existing renderer output for at least one fractal type.
- keep any `lens.downsample` decision tied to explicit mask/SDF resolution behavior and tests

Key references:
- ui_app/src/fractal_family_rules.h
- ui_app/src/fractal_renderer.cu
- ui_app/src/lens_sdf.cpp
- ui_app/src/fractal_types.h
- ui_app/tests/test_fractal_family_rules.cpp
- ui_app/tests/test_lens_sdf.cpp

## 7. CLI Bridge V2 Multi-Client / Socket Transport Follow-On

Status: deferred on 2026-04-11

Why paused:
- V2-G landed as a bounded Windows named-pipe transport that reuses the existing one-line session protocol for one external client/session per process.
- The surviving V2-G spec text still implied concurrent callers, but the shipped implementation does not provide multi-client pipe fan-in or socket transport.
- That broader transport surface is a follow-on thread, not required to close the current V2-G slice.

Resume constraints:
- do not advertise concurrent callers again until the runtime actually supports multiple simultaneous clients or a socket-based equivalent
- if resumed, add focused tests for per-client session isolation and concurrent connect/close behavior
- preserve the existing JSON line protocol and fail-fast semantics; transport changes should not fork request/response behavior

Key references:
- spec_intake/CliBridgeV2_GpuSampleFn_SpecIntake.md
- ui_app/src/headless_modes.cpp
- ui_app/src/headless_modes.h
- tests/test_fractal_runtime_session.py

## 8. Current Pause Point

If work resumes from this file alone, the intended next order is:
1. keep live responsiveness stable and do not reopen pacing experimentation casually
2. start the common-fractal catalog planning/coding thread
3. land only one bounded wave of safe 2D additions
4. refactor the CUDA catalog seams before any deeper expansion