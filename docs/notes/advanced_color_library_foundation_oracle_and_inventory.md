# Advanced Color Library Foundation Oracle And Inventory

## Purpose

This note locks the terminology, authority split, stronger architecture rules,
and initial balanced library inventory for the advanced color foundation work.
It exists so later implementation slices can point at one checked-in source for
what "Explaino CMap" means, what does not count as the same thing, what the
window is allowed to own, and what the initial four-category foundation is
supposed to ship.

This note is for `cuda_newton_fractal_clone` and its fractal engine. It is not
a Salticid-wide architecture rewrite. External references below are oracle and
contrast surfaces only.

## Authority Split

The planning and implementation must keep these surfaces distinct.

### 1. Legacy ExplainO CMap

- Authority reference:
  - `c:\code\salticid-cuda\carl-ca-python\ca_viewer.py`
  - `C:\Users\Adam\Desktop\b3\whatisthis\nine\archive\closed_docs\EXPLAINO_COLORMAP_RESEARCH_STUB.md`
- Meaning:
  - A scalar LUT lineage built from `explaino_seed()` plus phase-shifted RGB
    sampling and nonlinear channel shaping.
  - This is the oracle for the programmable palette entry named
    `explaino_cmap`.
- Non-meaning:
  - It is not the Newton-basin/root/joy palette lineage.
  - It is not the generic viewport colormap surface.

### 2. Root Classic Palette

- Authority reference:
  - current basin/root palette lineage in
    `c:\code\salticid-cuda\content\packs\explaino\cuda\explaino_cuda_helpers.inl`
  - older lineage in `c:\code\salticid-cuda\cuda_core\src\ops_special.cu`
  - current repo implementation assets in
    `c:\code\cuda_newton_fractal_clone\ui_app\src\basin_coloring.h`
- Meaning:
  - A basin/root coloring family tied to root classification and convergence
    structure.
  - This should become a separate family-gated programmable palette entry such
    as `root_classic_palette`.

### 3. Joy Root Palette

- Authority reference:
  - joy/exact ExplainO basin palette lineage in
    `c:\code\salticid-cuda\content\packs\explaino\cuda\explaino_cuda_helpers.inl`
  - current repo implementation assets in
    `c:\code\cuda_newton_fractal_clone\ui_app\src\basin_coloring.h`
- Meaning:
  - A warm joy/basin palette family for root-finding style rendering.
  - This should become a separate family-gated programmable palette entry such
    as `joy_root_palette`.

### 4. Generic Viewport Colormap Surface

- Authority reference:
  - `c:\code\salticid-cuda\ide_ui_dx11\ui_app\src\viewport_colormap.cpp`
  - `c:\code\salticid-cuda\ide_ui_dx11\ui_app\src\viewport_colormap.h`
- Meaning:
  - A generic user-facing colormap enum surface.
  - It is a useful contrast/control surface for terminology and future reuse.
- Non-meaning:
  - It does not currently contain ExplainO.
  - It is not the oracle for `explaino_cmap`.
  - It should not be auto-imported into the programmable palette catalog just
    because it exists nearby.

## Architecture Contract

The reusable color-pipeline core should become separately owned enough that it
could plausibly become its own DLL or static library later.

### Window Ownership Rules

`c:\code\cuda_newton_fractal_clone\ui_app\src\color_pipeline_window.h` must
stop being the authority for:

- category identity
- function identity
- parameter meaning
- runtime applicability
- import/apply behavior
- reset/default behavior
- serialization truth

The window may own presentation concerns only:

- row-stack rendering
- transient interaction state
- descriptor consumption
- adapter invocation

### Core Ownership Rules

The extracted reusable core should own:

- category registry data
- function descriptors and parameter metadata
- family gating and runtime applicability
- import/apply/reset/default adapters
- persistence/load serialization authority

Any new feature added during this effort must prove the extracted abstraction.
It must not bypass the core by adding new hard-coded category/function logic
back into the window.

### Whimsy And Naming Rules

Internal design guidance may absolutely use the Balance/Void compass and the
Kung Fu Panda-style visual axis to guide taste, defaults, and examples.

Shipping implementation must not expose:

- franchise names
- character-theme labels
- hard-coded story/theme presets as public function identity

Shipping names should stay neutral and production-facing.

## Initial Four-Category Foundation

The foundation categories are:

- Source
- Shape
- Palette
- Grading

Extra categories stay deferred until this foundation is real, descriptor-driven,
runtime-backed, serialized, and proven in the published D: runtime.

Recommended next categories after foundation closure:

- Blend
- Mask/Domain

## Initial Library Inventory

The inventory below is the initial planning target. A later implementation
slice may replace a weak candidate before shipping, but should not reduce the
foundation to token stubs.

### Source

- `smooth_escape_ramp` - broad reusable
- `phase_orbit` - broad reusable
- `banded_signal` - broad reusable
- `escape_magnitude` - broad reusable
- `orbit_stripe` - broad reusable
- `root_proximity` - family-gated

### Shape

- `identity` - broad reusable neutral row
- `offset_scale` - broad reusable
- `repeat` - broad reusable
- `posterize` - broad reusable
- `mirror_repeat` - broad reusable
- `bias_gain_curve` - broad reusable
- `smooth_window` - broad reusable

### Palette

- `heatmap` - broad reusable
- `phase_wheel_palette` - broad reusable
- `banded_heatmap` - broad reusable
- `explaino_cmap` - broad reusable, but anchored to the legacy ExplainO LUT
  oracle rather than the basin/root palette lineage
- `root_classic_palette` - family-gated
- `joy_root_palette` - family-gated

### Grading

- `neutral_finish` - broad reusable neutral row
- `contrast_lift` - broad reusable
- `phase_finish` - broad reusable
- `band_finish` - broad reusable
- `tone_map_finish` - broad reusable
- `balance_void_grade` - broad reusable bounded filmic color-manifold operator

## Balance/Void Grading Operator Spike

This belongs to Grading, not Palette.

Working neutral production names may include:

- `balance_void_grade`
- `filmic_balance_grade`
- `chroma_tension_grade`

The operator should be a compact parameter pack rather than one overloaded
slider.

Recommended starting parameters:

- `balance_void`
- `chroma_tension`
- `accent_bias`

Meaning:

- `balance_void` steers warm/organic/soft/harmonic behavior through neutral
  into cold/metallic/high-contrast/desaturated-midtone behavior
- `chroma_tension` steers saturation distribution, midtone compression,
  palette curvature, and softness versus blade-like separation
- `accent_bias` steers selective accent pressure such as red/cold highlight
  bias without hard-coding a palette

Requirements:

- must register through the same descriptor/catalog system as every other
  grading function
- must serialize and round-trip normally
- must work across multiple fractal families
- must not depend on ExplainO-specific hooks

## Separate Experimental Family Track: ExplainO-BalanceVoid

This is adjacent planning, not part of the reusable color-pipeline core.

The target pattern is one meta-family with multiple neutral-default axes, not
three separate new fractals and not one overloaded scalar.

Recommended starting controls:

- `balance_void`
- `symmetry_tension`
- `field_curvature`

Requirements:

- neutral defaults collapse exactly or near-exactly to original ExplainO
- the family acts as a rehearsal for a future `Explaino-All` pattern
- optional presets may exist as coordinates, but not as separate public fractal
  identities
- public shipping identity stays neutral; theme shorthand stays internal only

Ownership rule:

- fractal families may emit fields/signals
- the color pipeline may consume fields/signals
- the reusable color pipeline must remain generic and separately owned

## Quality Bar

Every visible function must be:

- runtime-real or absent
- meaningfully parameterized
- family-gated where needed
- serialized and round-trippable
- covered by schema/import/apply/state proofs
- proven against the published D: runtime/gallery captures

Forbidden outcomes:

- preview-only rows
- toy catalog entries
- hidden special cases in the window
- duplicated widget logic as a substitute for core abstraction
- conflating color grading with fractal-family geometry

## Immediate Follow-On Slice

After this note is checked in and validated, the next bounded implementation
slice is Phase 1 of the new foundation plan:

- extract the registry/catalog/adapters/serialization authority out of
  `ui_app/src/color_pipeline_window.h`
- make the window consume that core
- prove the extracted abstraction with deterministic tests before widening the
  shipped runtime catalog
