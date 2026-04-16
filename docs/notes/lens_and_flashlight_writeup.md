# Lens SDF / Flashlight Phase 1 Technical Report

This report replaces the earlier descriptive note with an evidence-backed Phase 1 report for the lens SDF modernization thread.

It is written to support two follow-on actions:

1. a later checked-in implementation plan for SDF modernization
2. a later planning pass for the related "flashlight" / probe modernization

Important terminology note:

- there is no checked-in runtime symbol or UI label literally named `flashlight`
- in current code, the closest matching feature families are:
  - the runtime image-space `Mask` / `Lens SDF` feature
  - the Explaino sidecar lens/action/controller guidance stack

This report documents both, keeps them separate, and makes explicit where they do and do not share substrate.

## Evidence Bundle

Supporting artifacts generated for this report:

- `artifacts/lens_phase1_fractal_family_inventory.json`
- `artifacts/lens_phase1_fractal_family_inventory.csv`
- `artifacts/lens_phase1_inventory_summary.json`
- `artifacts/lens_phase1_runtime_call_path.txt`
- `artifacts/lens_phase1_flashlight_call_path.txt`
- `artifacts/lens_phase1_focus_tests.log`
- `artifacts/lens_phase1_bench_precision_tiers.csv`
- `artifacts/lens_phase1_bench_precision_tiers.log`
- `artifacts/lens_phase1_bench_precision_summary.json`

Measured validation used in this report:

- `D:\salt-fractal\cuda_newton_fractal_clone\build_tests\test_lens_sdf.exe`
- `D:\salt-fractal\cuda_newton_fractal_clone\build_tests\test_runtime_reset.exe`
- `D:\salt-fractal\cuda_newton_fractal_clone\build_tests\test_schema_binding.exe`
- `D:\salt-fractal\cuda_newton_fractal_clone\build_tests\test_fractal_family_rules.exe`
- `D:\salt-fractal\cuda_newton_fractal_clone\build_tests\test_fractal_sample_pipeline.exe`
- `D:\salt-fractal\cuda_newton_fractal_clone\build_tests\bench_precision_tiers.exe`

## 1. Runtime Lens Architecture

### 1.1 Control Surface and Binding Reality

The runtime lens feature is exposed through the viewer schema:

- `fractal.lens.enabled`
- `fractal.lens.downsample`

Evidence:

- schema entries in `ui/fractal_binding_surface_v1.ui_schema.json`
- integer and bool bindings in `ui_app/src/schema_binding.cpp`
- reset defaults in `ui_app/src/runtime_reset.cpp`

`LensSettings` is currently:

```cpp
struct LensSettings {
    bool enabled{false};
    int downsample{2};
};
```

Current meaning:

- `enabled` is live and activates mask allocation, mask upload, SDF generation, and aux-window display
- `downsample` is bound and reset, but it has no effect on the render path today

That is not an inference. The runtime call-path extract in `artifacts/lens_phase1_runtime_call_path.txt` shows `lens.enabled` read in `main.cpp`, while there is no corresponding read of `lens.downsample` in:

- `ui_app/src/main.cpp`
- `ui_app/src/fractal_renderer.cu`
- `ui_app/src/lens_sdf.cpp`

### 1.2 Exact Render Call Path

The active runtime path is:

1. UI toggles `fractal.lens.enabled`.
2. Binding resolves that control into `LensSettings.enabled`.
3. `DispatchRenderFrame(...)` in `ui_app/src/main.cpp` checks `lens.enabled`.
4. If enabled, host allocates a full-resolution `maskBuffer`.
5. `DispatchRenderFrame(...)` calls `RenderFractalCUDA(view, params, dispatchRender, rgba.data(), maskPtr, &newStats, &err)`.
6. CUDA renders normal RGBA output into `outRGBA`.
7. CUDA optionally writes a binary one-byte-per-pixel mask into `outMask`.
8. Host uploads the fractal RGBA image.
9. If lensing is enabled:
   - host uploads the binary mask as grayscale RGBA
   - host computes a full signed-distance visualization with `ComputeSignedDistanceSdfChamfer(...)`
   - host uploads the SDF RGBA texture
10. Main viewer renders the normal fractal viewport.
11. If lensing is enabled, viewer opens auxiliary windows:
   - `Mask`
   - `Lens SDF`

Important architectural point:

- the main viewport is not color-integrated with the lens result
- the lens output is a separate debug surface, not a composited visualization

### 1.3 GPU / CPU Work Split

Current production split:

- GPU:
  - fractal iteration
  - normal fractal color computation
  - optional binary mask generation
- CPU / host:
  - receives full RGBA image
  - receives full binary mask when lens is enabled
  - converts mask to uploaded aux texture
  - computes chamfer signed-distance field on CPU
  - converts SDF to RGBA on CPU
  - uploads SDF texture

This means the SDF is not a GPU-resident postprocess today. It is a host-side visualization pass bolted onto the main renderer.

### 1.4 Where the Full-Frame Copies Happen

For the normal render path:

- full-frame device-to-host copy of `outRGBA`

For the lens-enabled path:

- full-frame device-to-host copy of `outRGBA`
- full-frame device-to-host copy of `outMask`
- full-frame host walk to convert/upload mask texture
- full-frame host SDF build
- full-frame host SDF RGBA creation
- full-frame texture upload for mask
- full-frame texture upload for SDF

The code currently allocates the mask at `dispatchRender.resolution`, not at a reduced lens resolution.

### 1.5 What the Mask Actually Means

The current code path exposes two different mask semantics:

- explicit family rule helper in `ui_app/src/fractal_family_rules.h`
- renderer-side special-case override in `ui_app/src/fractal_renderer.cu`

The helper function says:

- basin-coloring family -> inside means `converged`
- escape-time family -> inside means `!escaped`
- otherwise -> `false`

But the renderer does something stricter:

- for basin-coloring families:
  - compute nearest root index
  - inside = even root index
  - outside = odd root index or unresolved root
- for non-basin families:
  - defer to `LensMaskInsideForFractal(...)`

This means the shipped runtime mask semantics are:

- basin families: synthetic root-parity partition
- escape-time families: bounded / non-escaped partition

This root-parity choice is deliberate. Without it, Newton-style families would mostly produce a uniform "converged" mask and the SDF would become almost useless as a boundary visualization.

### 1.6 How the SDF Is Computed

`ui_app/src/lens_sdf.cpp` is purely host-side.

It builds two chamfer distance fields:

- distance to inside
- distance to outside

Then computes:

```cpp
signedPx = dToInside[index] - dToOutside[index];
```

Current sign convention:

- inside pixels are negative
- outside pixels are positive
- boundary is near zero

That sign convention is locked by `test_lens_sdf.exe`.

The transform is approximate, not exact Euclidean:

- axis weight = `1.0`
- diagonal weight = `1.41421356`

### 1.7 How the SDF Is Visualized

Current visualization is not family-aware and not palette-aware.

The path:

1. normalize signed distance around `0.5`
2. clamp to `[0, 1]`
3. render grayscale
4. paint a red-ish contour band where `abs(signedPx) < 0.75`

The runtime hardcodes `maxAbsPx = 48.0f` in `main.cpp`.

So the current SDF output is:

- grayscale bulk field
- red-ish contour near the boundary
- entirely separate from the main fractal color surface

## 2. Current Fractal Support Reality

### 2.1 Catalog Counts

Source-derived counts from `artifacts/lens_phase1_inventory_summary.json`:

- total shipped `FractalType` values: `37`
- Explaino-family count: `22`
- basin-coloring count: `20`
- escape-time count: `17`
- current lens unclassified count: `0`

Important clarification:

- the runtime lens is not still limited to "3 Explaino family fractals"
- the current family rules already enumerate the full shipped catalog
- what is old is not the enum coverage itself; what is old is the visualization/control/performance design around that coverage

### 2.2 Full Catalog Classification

The table below is generated from the current source rules. It reflects shipped code, not intended future semantics.

| Fractal Type | Explaino | Basin | Escape | Current Lens Rule | Current Risk / Need |
|---|---:|---:|---:|---|---|
| `newton` | no | yes | no | root parity synthetic even-inside | explicit semantics doc + better overlay |
| `nova` | no | no | yes | bounded-or-not-escaped inside | overlay + performance modernization |
| `mandelbrot` | no | no | yes | bounded-or-not-escaped inside | overlay + performance modernization |
| `julia` | no | no | yes | bounded-or-not-escaped inside | overlay + performance modernization |
| `burning_ship` | no | no | yes | bounded-or-not-escaped inside | overlay + performance modernization |
| `multibrot` | no | no | yes | bounded-or-not-escaped inside | overlay + performance modernization |
| `phoenix` | no | no | yes | bounded-or-not-escaped inside | overlay + performance modernization |
| `explaino` | yes | yes | no | root parity synthetic even-inside | synthetic rule should be made explicit |
| `explaino_y` | yes | yes | no | root parity synthetic even-inside | synthetic rule should be made explicit |
| `explaino_fp` | yes | yes | no | root parity synthetic even-inside | synthetic rule should be made explicit |
| `explaino_nova` | yes | no | yes | bounded-or-not-escaped inside | split family semantics already exist |
| `explaino_halley` | yes | yes | no | root parity synthetic even-inside | synthetic rule should be made explicit |
| `explaino_dual` | yes | yes | no | root parity synthetic even-inside | synthetic rule should be made explicit |
| `explaino_mult` | yes | yes | no | root parity synthetic even-inside | synthetic rule should be made explicit |
| `explaino_phoenix` | yes | yes | no | root parity synthetic even-inside | synthetic rule should be made explicit |
| `explaino_transcendental` | yes | yes | no | root parity synthetic even-inside | synthetic rule should be made explicit |
| `explaino_inertial` | yes | yes | no | root parity synthetic even-inside | synthetic rule should be made explicit |
| `explaino_julia` | yes | no | yes | bounded-or-not-escaped inside | overlay + performance modernization |
| `explaino_rational` | yes | yes | no | root parity synthetic even-inside | synthetic rule should be made explicit |
| `multicorn` | no | no | yes | bounded-or-not-escaped inside | overlay + performance modernization |
| `halley` | no | yes | no | root parity synthetic even-inside | explicit semantics doc + better overlay |
| `collatz` | no | no | yes | bounded-or-not-escaped inside | overlay + performance modernization |
| `explaino_collatz` | yes | yes | no | root parity synthetic even-inside | synthetic rule should be made explicit |
| `mcmullen` | no | no | yes | bounded-or-not-escaped inside | overlay + performance modernization |
| `lambda_map` | no | no | yes | bounded-or-not-escaped inside | overlay + performance modernization |
| `explaino_lambda` | yes | no | yes | bounded-or-not-escaped inside | overlay + performance modernization |
| `explaino_rational_escape` | yes | no | yes | bounded-or-not-escaped inside | overlay + performance modernization |
| `spider` | no | no | yes | bounded-or-not-escaped inside | overlay + performance modernization |
| `celtic_mandelbrot` | no | no | yes | bounded-or-not-escaped inside | overlay + performance modernization |
| `perpendicular_burning_ship` | no | no | yes | bounded-or-not-escaped inside | overlay + performance modernization |
| `explaino_joy` | yes | yes | no | root parity synthetic even-inside | synthetic rule should be made explicit |
| `explaino_fold` | yes | yes | no | root parity synthetic even-inside | synthetic rule should be made explicit |
| `explaino_bell` | yes | yes | no | root parity synthetic even-inside | synthetic rule should be made explicit |
| `explaino_ripple` | yes | yes | no | root parity synthetic even-inside | synthetic rule should be made explicit |
| `explaino_splice` | yes | yes | no | root parity synthetic even-inside | synthetic rule should be made explicit |
| `explaino_vortex` | yes | yes | no | root parity synthetic even-inside | synthetic rule should be made explicit |
| `explaino_tension` | yes | yes | no | root parity synthetic even-inside | synthetic rule should be made explicit |

### 2.3 Coverage Conclusions

What is covered:

- every shipped `FractalType` is currently classified by the family rules
- the runtime lens path has no catalog holes in the current shipped enum set

What is still fragile:

- the semantics are split across:
  - `LensMaskInsideForFractal(...)`
  - renderer-side basin override logic
- the helper suggests one meaning for basin families, but production behavior uses a stronger special-case rule
- the SDF visualization itself is not modernized to reflect the broader catalog

## 3. Current Color Integration Reality

### 3.1 Renderer Color Path

Current renderer behavior in `fractal_renderer.cu`:

- choose basin-coloring branch when `SupportsBasinColoring(ft)` is true
- otherwise choose escape-time color generation
- apply fractal color grading
- write final RGBA to `outRGBA`

Mask generation happens after color generation and is logically separate.

So the lens does not participate in the main color pipeline. It is a second product emitted from the same sample pass.

### 3.2 Where Color and Mask Diverge

The divergence point is architectural, not just stylistic:

- fractal color output is a primary render result
- mask is an optional debug classification buffer
- SDF is a host-generated visualization derived from that classification buffer

This means:

- mask semantics can be synthetic while image coloring remains mathematically meaningful
- SDF coloration is not aware of fractal coloring mode
- aux windows are fed by separate textures, not by compositing into the viewport

### 3.3 Why the SDF Is Still Debug-Only

Three concrete reasons:

1. The SDF is computed on CPU after the main render, which makes it naturally feel like a debug sidecar rather than a primary effect.
2. The visualization code in `lens_sdf.cpp` is grayscale-plus-contour and has no concept of fractal coloring mode.
3. `main.cpp` presents the output only through `RenderAuxImageWindow("Mask", ...)` and `RenderAuxImageWindow("Lens SDF", ...)`.

So "color integration" is not blocked by missing color math alone. It is blocked by the fact that the feature was built as a secondary diagnostic path.

## 4. Current Performance Reality

### 4.1 What Was Directly Measured

`bench_precision_tiers.exe` produced fresh timing data in:

- `artifacts/lens_phase1_bench_precision_tiers.csv`
- `artifacts/lens_phase1_bench_precision_summary.json`

Measured examples at `1024x1024`:

- `mandelbrot_zoom_379264`
  - fast: `1.623 ms`
  - standard: `30.152 ms`
  - standard / fast ratio: `18.57x`
- `mandelbrot_zoom_4194304`
  - fast: `2.992 ms`
  - standard: `73.588 ms`
  - standard / fast ratio: `24.59x`
- `julia_zoom_126421`
  - fast: `0.378 ms`
  - standard: `7.405 ms`
  - standard / fast ratio: `19.57x`
- `julia_zoom_4194304`
  - fast: `0.550 ms`
  - standard: `7.367 ms`
  - standard / fast ratio: `13.40x`

### 4.2 What Those Numbers Do and Do Not Mean

These benchmark numbers measure base renderer cost under different precision tiers.

They do **not** directly measure lens overhead because the benchmark executable exercises `RenderFractalCUDA(...)` timing, not the full viewer path with:

- host mask upload
- CPU chamfer SDF build
- SDF texture upload
- aux-window presentation

So the current report can state measured renderer timing facts and measured precision-tier behavior, but it cannot honestly claim a measured lens-on vs lens-off delta from checked-in binaries alone.

### 4.3 What Can Be Inferred From Code Path Inspection

Although direct end-to-end lens cost is not currently measured by existing checked-in binaries, the additive overhead is structurally clear:

- one extra full-resolution mask device-to-host copy
- one full CPU pass to convert/upload mask texture
- one full CPU chamfer transform over the entire frame
- one full CPU RGBA conversion for the SDF
- one additional full texture upload

That means the current lens design adds pure post-render overhead on top of the base renderer timing, and it does so at full render resolution because `lens.downsample` is not wired.

### 4.4 Performance Conclusions

Performance bottlenecks are not hypothetical:

- the current SDF is host-bound
- it scales with full frame area
- it bypasses the sample-tier/renderer cost controls that already exist for the main fractal pass
- it does not exploit the declared `downsample` control

This is the clearest technical reason the SDF should be modernized before treating it as a first-class overlay.

## 5. Current Flashlight / Probe Reality

### 5.1 Terminology Resolution

Current repo reality:

- no literal runtime `flashlight` symbol
- no UI control labeled `flashlight`

Two real systems fit the user-facing concept:

1. runtime lens / SDF image-space inspection
2. Explaino sidecar lens/action/controller guidance

### 5.2 Runtime Image-Space Lens / SDF

Characteristics:

- image-space
- tied to fractal render output
- emits binary partition mask
- derives host-side SDF
- presented through debug aux windows

Current reusable seams:

- `LensSettings`
- family classification rules
- mask generation inside `RenderFractalCUDA(...)`
- `lens_sdf.cpp`

Current limitations:

- no composited overlay
- no palette-aware output
- no performance-aware resolution control
- semantics live partly in helper rules, partly in the renderer

### 5.3 Explaino Sidecar Lens / Action / Controller

Primary chain:

- `BuildExplainoSidecarWindowState(...)`
- `BuildSidecarLensProjection(...)`
- `BuildSidecarEnergyLandscape(...)`
- `BuildSidecarActionRecommendation(...)`
- `BuildSidecarAutoDemoControllerDecision(...)`
- `ApplySidecarAutoDemoControllerDecision(...)`

Characteristics:

- parameter-space, not image-space
- measurement-driven, not mask-driven
- uses information gain, uncertainty, stability, and cost
- can stay advisory or mutate runtime state

Current gating reality:

- the exploration/advisor surfaces are Explaino-family oriented
- this is not yet a general all-fractal guidance substrate

### 5.4 Shared Seams vs Separate Systems

Shared today:

- fractal catalog / binding context / runtime state
- some high-level family concepts
- the overloaded word `lens`

Not shared today:

- no shared mask/interest semantics API
- no shared overlay/presentation seam
- no shared "boundary of interest" descriptor
- no shared contract for exposing lens semantics to a broader probe surface

This means the runtime lens SDF and the sidecar "flashlight" are conceptually adjacent but architecturally separate.

### 5.5 Shared Substrate Needed for the Follow-On

If the later flashlight/probe modernization is meant to cover more than Explaino-sidecar behavior, the missing substrate is:

- one explicit lens semantics authority per fractal type
- one reusable description of what the lens boundary means for a family
- one presentation seam that can support:
  - image-space overlay
  - auxiliary debug views
  - future probe / inspection consumers
- one clear distinction between:
  - mathematical family classification
  - visualization partition choice
  - guidance / exploration decision logic

## 6. Risk and Constraint Analysis

### 6.1 Highest-Risk Assumptions

1. `lens.downsample` exists as a user-facing control but currently has no runtime effect.
2. Basin-family semantics are split between helper rules and renderer-specific override logic.
3. The production mask meaning for basin families is synthetic root parity, not literal convergence.
4. The SDF is host-side and full-resolution, which makes "just overlay it" a performance risk if done without redesign.
5. The word `lens` currently refers to two different subsystems with almost no implementation overlap.

### 6.2 Dead Settings and Partial Abstractions

Dead or partial today:

- `fractal.lens.downsample`
- `LensMaskInsideForFractal(...)` as a full production truth surface for basin families
- grayscale SDF visualization as if it were a finished product surface

### 6.3 Family-Rule Duplication

Current rule duplication exists at two levels:

- family classification in `fractal_family_rules.h`
- renderer-side basin override logic in `fractal_renderer.cu`

This is not yet a bug because the enum set is covered, but it is a maintenance risk for any future fractal additions or semantics changes.

### 6.4 Test Coverage Reality

Freshly rerun focused seams:

- `test_lens_sdf.exe`
- `test_runtime_reset.exe`
- `test_schema_binding.exe`
- `test_fractal_family_rules.exe`
- `test_fractal_sample_pipeline.exe`

What is covered well:

- SDF sign convention and non-empty output behavior
- lens reset defaults
- schema binding for lens controls
- catalog family classification
- sample/pipeline behavior across basin and escape-time representatives

What is under-covered:

- direct runtime lens-on vs lens-off performance delta
- actual visual correctness of lens output across the full 37-type catalog
- any real runtime behavior for `lens.downsample` because it currently has none
- color-integrated overlay behavior because it does not exist yet
- a generalized non-Explaino "flashlight" / guidance layer

## 7. Bottom Line

The current runtime lens is broader in fractal coverage than its original design era suggests, but it is still architected like a debug sidecar:

- full-resolution host mask path
- CPU chamfer SDF
- separate aux windows
- grayscale visualization
- dead resolution control

The current "flashlight" interpretation is also split:

- image-space runtime lens/SDF for visual inspection
- Explaino-sidecar parameter-space guidance for local exploration

Those systems are not yet unified. The most valuable result of this Phase 1 report is not just "document how they work"; it is making explicit that the modernization needs to solve two different problems:

1. turn the runtime lens from a debug path into a performant, color-integrated, family-explicit overlay
2. define a shared semantics substrate so the later flashlight/probe work does not become a second isolated lens system
