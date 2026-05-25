# SDF Field Pack Near-Term TODO

Status: living roadmap. The SDF field-pack system is partially shipped as
headless/native substrate, live viewer Color Pipeline input, Capture Finding parity, normal viewport overlay, SDF Source row customization, capture/replay authority, phase-signal metadata, Color Pipeline fractal-switch preservation, and realtime pacing telemetry. The active measured follow-up is SDF postprocess signal specialization in `docs/notes/sdf_postprocess_signal_specialization_PHASED_PLAN.md`; broader composition UX and SDF-native fractal lanes remain deferred behind that FPS work.

Shipped since this roadmap was first written:

- Lens SDF downsample/control truth.
- Reusable scalar `SdfFieldResult` / `SdfFieldView` authority for mask-derived
  Lens SDF fields.
- Family-aware Lens SDF semantics authority.
- Authored `sdf.pack` parser and CPU reference evaluator.
- CUDA SDF pack evaluator plus descriptor hardening.
- Source-neutral SDF signal sampling consumed by flashlight probe/report and
  runtime-walk headless/report outputs.
- CUDA-backed Lens SDF field generation for the live client, with CPU fallback/reference.
- Live Color Pipeline SDF source rows for signed distance, inside/outside, boundary band, normal angle, and curvature.
- Capture Finding parity for active SDF Color Pipeline source-row pixels.
- Shared Lens Downsample visibility/control authority when Color Pipeline SDF rows use the field.
- Normal viewport SDF overlay productization for boundary, band, and field-debug modes.
- SDF Source row customization: visible Scale/Bias/Blend Weight controls, `sdf_boundary_band` boundary width, and a Source-section alias for shared SDF Field Downsample.
- Capture/replay authority: top-level Lens state in `state.json`, arbitrary explicit `--load-state-json` filenames, and a fast SDF/non-SDF replay smoke matrix.
- Phase-signal metadata: `sdf_normal_angle` is classified as phase-like, while signed distance, boundary band, and curvature remain scalar.
- Color Pipeline fractal-switch preservation: compatible fractal selector changes keep supported live/source-stack Color Pipeline state, while unsupported switches project to target defaults and clear stale source rows.
- SDF realtime pacing telemetry: live reports now split base render, SDF field, SDF postprocess, and SDF total timing; pacing now reacts to visible SDF frame cost.

Active next slice:

- SDF postprocess signal specialization: scalar-only SDF Source rows should not pay for normal/curvature neighborhood sampling. This is the smallest measured performance step after the realtime pacing repair.
- Per-row SDF downsample and GPU Color Pipeline postprocess remain follow-ups until scalar CPU authority is specialized and proven.
- Color Pipeline composition/preset UX, boundary-masked normal-angle, and SDF-backed masks/gates remain planned product work after the current FPS slice.

Still deferred:

- SDF-native selectable fractal lanes.
- Authored SDF pack UI/live viewport integration.

This document slots the SDF composition / field-pack idea into the current
viewer-host roadmap. It is meant to let a future session start from repo truth
instead of reconstructing the idea from chat.

## Short Answer

The right slot is a new SDF field substrate that sits between the existing Lens
SDF cleanup and the later Generic Equation Pack productization work:

```text
Current Lens SDF
  -> mask-derived field producer

SDF Field Substrate
  -> common signed-distance field view, metadata, tests, and consumers

Authored SDF Packs
  -> AST-defined field compositions lowered to CPU/CUDA evaluators

Consumers
  -> shipped headless/report probes, live Color Pipeline, Lens overlay,
     and capture integration now; authored-pack UI and later SDF-native
     fractal lanes remain deferred
```

Do not put authored SDF packs directly inside the current Lens SDF aux-window
path. Lens SDF should become one field producer. Authored SDF packs should
become another field producer. They should meet at a shared field interface.

## Existing Surfaces To Respect

Current repo surfaces:

- `ui_app/src/lens_sdf.h`
- `ui_app/src/lens_sdf.cpp`
- `ui_app/tests/test_lens_sdf.cpp`
- `ui_app/src/fractal_renderer.cu`
- `ui_app/src/fractal_family_rules.h`
- `ui_app/src/main.cpp`
- `ui_app/src/flashlight_probe.cpp`
- `ui_app/src/runtime_walk_headless.cpp`
- `ui/fractal_binding_surface_v1.ui_schema.json`
- `docs/notes/lens_and_flashlight_writeup.md`
- `docs/notes/lens_sdf_phase2_planning_report.md`
- `docs/notes/generic_cuda_equation_pack_PAUSE_README.md`

Salticid reference surfaces, read-only from this repo:

- `C:\code\salticid-cuda\ide_ui_dx11\ui_app\src\lens_sdf.cpp`
- `C:\code\salticid-cuda\ide_ui_dx11\ui_app\src\lens_sdf_chamfer.h`
- `C:\code\salticid-cuda\ide_ui_dx11\ui_app\src\lens_sdf_chamfer.cpp`
- `C:\code\salticid-cuda\ide_ui_dx11\ui_app\tests\test_lens_sdf_chamfer.cpp`
- `C:\code\salticid-cuda\content\packs\sdf\docs\README.md`
- `C:\code\salticid-cuda\content\packs\sdf\operators.d\sdf.json`
- `C:\code\salticid-cuda\content\packs\sdf\cuda\sdf_cuda_geometry_kernels.inl`

Important distinction:

- Current Lens SDF is mask-derived: binary or semantic fractal mask to signed
  distance field.
- Salticid's SDF operator pack is authored analytic SDF composition: primitives,
  transforms, combinators, and field operations.
- These should share a downstream field interface, but they should not share the
  same source authority.

## Non-Goals

Do not start this work by:

- adding a new renderer monolith path
- replacing `RenderFractalCUDA`
- editing Salticid from this repo
- treating arbitrary user shader code as a safe pack format
- building a 3D raymarcher as the first user-facing SDF feature
- claiming the current CPU chamfer path is enough for live high-resolution
  product use
- mixing this with unrelated selector, perturbation, or Color Pipeline redesign

## Core Architecture

### Field View

Add a common field view that every SDF producer can emit:

```cpp
enum class SdfFieldSourceKind {
    mask_derived,
    authored_pack,
    builtin_analytic,
};

enum class SdfSignConvention {
    negative_inside_positive_outside,
};

struct SdfFieldView {
    int width;
    int height;
    float pixel_scale;
    SdfSignConvention sign_convention;
    SdfFieldSourceKind source_kind;
    std::span<const float> signed_distance_px;
    std::span<const uint16_t> material_id; // optional later
};
```

This struct is illustrative, not a mandate to change the repo's C++ standard.
Use the repo-compatible view/storage type when implementing it. The important
contract is that the interface must separate:

- semantics: what the field means
- generation: how the field was computed
- presentation: how it is colored, blended, probed, or captured

### Field Producers

Start with two producer families:

1. Mask-derived Lens SDF
   - Input: current fractal mask / semantic partition.
   - Output: signed-distance field.
   - First implementation may stay CPU chamfer for correctness and tests.
   - Product direction is GPU-resident or lower-resolution field generation.

2. Authored SDF Pack
   - Input: bounded AST pack describing a signed field function.
   - Output: signed-distance field sampled over the current view region.
   - Initial pack workbench can be offline/headless before viewport integration.

### Field Consumers

Consumers should not care whether the field came from a fractal mask or authored
SDF composition:

- Lens overlay
- Lens aux/debug windows
- Flashlight/probe sampling
- Color Pipeline signals
- capture/finding metadata
- later SDF-native fractal lanes

## SDF Pack Shape

Use a field-specific AST pack, not free-form shader code.

Suggested v1 shape:

```json
{
  "schema": 1,
  "pack_id": "sdf_smooth_capsule_union_demo",
  "name": "Smooth Capsule Union Demo",
  "kind": "sdf_scene_2d",
  "params": [
    { "id": "blend", "type": "float", "default": 0.15, "range": [0.0, 2.0] }
  ],
  "controls": [
    { "param": "blend", "label": "Blend", "ui_min": 0.0, "ui_max": 1.0 }
  ],
  "region": {
    "center": [0.0, 0.0],
    "half_height": 1.5
  },
  "ast": {
    "op": "smooth_union",
    "k": { "param": "blend" },
    "a": { "op": "circle", "center": [0.0, 0.0], "radius": 0.5 },
    "b": { "op": "capsule", "a": [-1.0, 0.0], "b": [1.0, 0.0], "radius": 0.2 }
  }
}
```

Initial op families:

- primitives: `circle`, `box`, `rounded_box`, `segment`, `capsule`, `line`,
  `polygon`
- combinators: `union`, `intersect`, `subtract`, `smooth_union`,
  `smooth_intersect`, `smooth_subtract`
- transforms: `translate`, `rotate`, `scale`, `mirror`, `repeat`
- modifiers: `shell`, `onion`, `round`, `elongate`, `displace`
- query outputs: `distance`, `inside_mask`, `boundary_band`, `normal`,
  `curvature_estimate`

Hard limits should be explicit from day one:

- maximum AST nodes
- maximum params
- bounded numeric ranges
- no unbounded loops
- no file access
- no dynamic kernel registration in v1
- deterministic parser/lowerer errors

## Can This Drive Fractal Subtypes?

Yes, if the first subtypes are chosen carefully.

Good first SDF-native subtype candidates:

- `sdf_recursive_fold_2d`
- `sdf_smooth_lattice_2d`
- `sdf_apollonian_field_2d`
- `sdf_domain_repeat_2d`
- `sdf_orbit_trap_overlay`

Good hybrid candidates:

- SDF orbit traps for existing escape-time fractals.
- SDF masks that vary parameters spatially.
- SDF boundary bands as Color Pipeline signals.
- SDF-defined perturbation or probe regions.

Bad first candidates:

- arbitrary shader scenes
- full 3D raymarching
- CPU full-frame field generation as the normal live path
- dynamic CUDA kernel registration
- fractal distance estimators disguised as simple 2D composition

Performance expectation:

- A bounded 2D analytic SDF AST evaluated once per pixel can be fast enough on
  CUDA for viewport use.
- Mask-derived CPU chamfer is acceptable as a correctness/reference path, but
  should not be the final high-resolution live path.
- Deep scene graphs, 3D raymarching, and unbounded user expressions need a
  separate performance and pacing plan.

## Recommended Slice Order And Current State

### Slice 1 - Lens SDF Truth Cleanup - Shipped

Difficulty: low/medium.
Reward: medium trust, required substrate cleanup.

Goal:

- Make `fractal.lens.downsample` truthful or remove/hide it.
- Prefer wiring it as real behavior using the Salticid `lens_sdf_chamfer`
  pattern.
- Share downsample helpers instead of duplicating them in probe/headless paths.

Expected edits:

- `ui_app/src/lens_sdf.h`
- `ui_app/src/lens_sdf.cpp`
- possibly new `ui_app/src/lens_sdf_chamfer.*` or a refactor of the existing file
- `ui_app/src/main.cpp`
- `ui_app/src/flashlight_probe.cpp`
- `ui_app/src/runtime_walk_headless.cpp`
- `ui_app/tests/test_lens_sdf.cpp`
- schema/binding tests if visibility changes

Required proof:

- native tests for downsample normalization and mask downsampling
- exact signed-distance sample tests based on the Salticid chamfer rail
- live/viewer or headless proof that changing `fractal.lens.downsample`
  changes the Lens SDF work resolution or reported field output
- proof the base fractal render is unchanged by Lens SDF downsample changes

Exit criteria:

- visible `lens.downsample` is no longer ambiguous
- duplicate downsample logic is reduced or explicitly justified
- current Lens SDF remains mask-derived and behavior-compatible by default

Current state:

- Shipped. See `docs/notes/lens_sdf_truth_cleanup_PHASED_PLAN.md`.

### Slice 2 - SDF Field Interface Extraction - Shipped

Difficulty: medium.
Reward: high foundation.

Goal:

- Extract a reusable signed-distance field data interface from the Lens SDF
  path.
- Keep RGBA visualization as presentation, not authority.

Expected edits:

- new field view/result header near `lens_sdf.*`
- `lens_sdf.*` emits scalar distance data plus optional debug RGBA
- tests assert sign convention, dimensions, no-boundary behavior, and stable
  output metadata

Required proof:

- all-existing Lens SDF tests still pass
- new scalar-field tests prove center/outside/diagonal distances
- no hidden fallback for invalid masks or impossible dimensions

Exit criteria:

- downstream code can consume a field without parsing debug colors
- sign convention is documented and tested

Current state:

- Shipped. See `docs/notes/sdf_field_interface_extraction_PHASED_PLAN.md`.

### Slice 3 - Lens Semantics Authority - Shipped

Difficulty: medium.
Reward: medium/high correctness.

Goal:

- Centralize what "inside" means per fractal family.
- Preserve basin root-parity behavior but make it explicit.

Expected edits:

- `ui_app/src/fractal_family_rules.h`
- `ui_app/src/fractal_renderer.cu`
- tests covering every current `FractalType`
- documentation labels for semantic partitions

Required proof:

- every shipped fractal maps to one explicit lens semantics descriptor
- basin synthetic root parity is named, tested, and not hidden as convergence
- unsupported future types fail closed or are visibly unsupported

Exit criteria:

- Lens SDF, probes, and reports can use one semantic vocabulary

Current state:

- Shipped. See `docs/notes/lens_semantics_authority_PHASED_PLAN.md`.

### Slice 4 - Authored SDF Pack Parser And CPU Reference - Shipped

Difficulty: medium/high.
Reward: high strategic.

Goal:

- Add an AST-defined authored `sdf.pack` workbench path.
- Start with CPU reference evaluation, not viewport integration.

Expected edits:

- new pack parser/lowerer beside generic equation-pack code
- examples under `docs/examples/sdf_packs/`
- native tests for malformed packs, bounds, duplicate params, invalid ops,
  oversized ASTs, and deterministic errors

Required proof:

- reference evaluator samples simple circle, box, capsule, smooth union, and
  repeat scenes
- controls override params and change field samples
- malformed pack never falls back to a different scene

Exit criteria:

- authored SDF packs have a deterministic source-of-truth schema

Current state:

- Shipped. See `docs/notes/sdf_pack_parser_cpu_reference_PHASED_PLAN.md`.

### Slice 5 - CUDA SDF Pack Evaluator And Hardening - Shipped

Difficulty: medium/high.
Reward: high, unlocks live potential.

Goal:

- Lower bounded SDF ASTs to a CUDA evaluator path.
- Keep CPU reference parity as the correctness rail.

Required proof:

- CPU/CUDA parity for primitive, transform, combinator, and bounded recursive
  compositions
- bounded performance witness on representative viewport-sized samples
- no regression to existing `generic.sample` and Color Pipeline rails

Exit criteria:

- SDF packs are real GPU field execution, not docs-only expressions

Current state:

- Shipped as native/headless evaluator substrate and hardening. See
  `docs/notes/sdf_pack_cuda_evaluator_PHASED_PLAN.md`.

### Slice 5 - CUDA Lens SDF Backend - Shipped

Difficulty: medium/high.
Reward: high performance unblock.

Goal:

- Add a CUDA-backed Lens SDF field producer beside the current CPU chamfer path.
- Keep CPU chamfer as fallback/reference until measured client proof says the
  GPU path is safe as default.

Required proof:

- deterministic native parity tests for mask shapes and odd dimensions
- semantic sign/boundary agreement with CPU reference
- live/headless backend reporting
- bounded performance witness before claiming client improvement

Exit criteria:

- Lens SDF generation can report a CUDA backend when available without changing
  base fractal rendering or the existing RGBA visualization contract.

### Slice 6 - Color Pipeline And Probe Consumption - Partially Shipped

Difficulty: medium.
Reward: high user-facing reuse.

Goal:

- Let Color Pipeline and probe/report surfaces consume SDF field signals without
  caring about producer source.

Candidate signals:

- signed distance
- boundary band
- inside/outside
- approximate normal angle
- curvature estimate
- material id, later

Required proof:

- existing Color Pipeline rails remain green
- SDF signal control changes live frame or sampled output
- no OS mouse automation
- no hard dependency on authored packs for Lens SDF

Exit criteria:

- SDF fields become a reusable color/probe substrate

Current state:

- Probe/report consumption is partially shipped: flashlight probe/report and
  runtime-walk headless/report paths consume source-neutral SDF signal samples.
- Live Color Pipeline SDF source rows are shipped on `codex/color-pipeline-sdf-source-rows` and proved by `tests/test_fractal_runtime_color_pipeline_sdf_rows.py`.
- Normal viewport SDF overlays are shipped and proved by `tests/test_fractal_runtime_sdf_viewport_overlay.py`.
- The active customization follow-up is `docs/notes/color_pipeline_sdf_source_customization_PHASED_PLAN.md`.
- See `docs/notes/sdf_field_signal_consumption_PHASED_PLAN.md`, `docs/notes/sdf_runtime_walk_signals_PHASED_PLAN.md`, `docs/notes/color_pipeline_sdf_source_rows_PHASED_PLAN.md`, and `docs/notes/sdf_viewport_overlay_productization_PHASED_PLAN.md`.

### Slice 7 - Viewport Overlay Productization - Shipped

Difficulty: medium/high.
Reward: high product value.

Goal:

- Make Lens/SDF overlay visible in the normal viewport, with aux windows kept as
  diagnostics.

Initial overlay modes:

- `boundary`
- `band`
- `field_debug`

Required proof:

- base fractal remains visible under overlay
- overlay mode/opacity controls are live and state-backed
- capture/finding behavior is explicit: either includes overlay metadata or
  intentionally captures base-only

Exit criteria:

- Lens SDF stops being aux-window-only

Current state:

- Shipped. Normal viewport SDF overlays for boundary, band, and field-debug modes are proved by `tests/test_fractal_runtime_sdf_viewport_overlay.py`.

### Slice 8 - First SDF-Native Fractal Lane - Deferred

Difficulty: medium/high.
Reward: high proof of concept.

Goal:

- Add one built-in SDF-native lane after field, pack, CUDA, and color/probe
  seams exist.

Recommended first lane:

- `sdf_recursive_fold_2d` or `sdf_smooth_lattice_2d`

Why these first:

- bounded 2D field evaluation
- meaningful sliders
- visually distinct output
- avoids 3D raymarch complexity

Required proof:

- normal fractal dropdown/left-panel integration
- saved-state round trip
- Color Pipeline compatibility
- no-mouse runtime proof
- FPS/performance witness

Exit criteria:

- SDF can honestly drive a selectable fractal subtype without a renderer
  monolith rewrite

Current state:

- Deferred. No SDF-native selectable fractal lane is shipped yet.

## Open Design Questions

1. Should `lens.downsample` remain an integer power-of-two control, or should it
   be replaced with `overlay_quality` / `field_resolution_scale`?
2. Should authored SDF packs use world-space units, pixel-space units, or both
   with explicit metadata?
3. How much of Salticid's analytic SDF operator namespace should be mirrored
   here before the Salticid adapter exists?
4. Should SDF pack examples live beside equation packs or under a separate
   `docs/examples/sdf_packs/` root?
5. Should SDF fields feed Color Pipeline before viewport overlay, or should the
   overlay prove the product path first?
6. What is the first SDF-native lane that gives the best visual payoff without
   requiring a 3D renderer?

## Validation Defaults For Future Slices

Use focused rails first:

- native SDF unit tests
- schema/binding tests for any visible control changes
- Color Pipeline focused rails when field signals touch coloring
- no-mouse runtime proof for viewer-visible behavior
- runtime publish only when the slice changes the viewer path
- full native helper suite before closeout when shared renderer/schema seams are touched

Never use physical cursor automation for this work.

## Suggested Backlog Slot

Recommended immediate ordering:

1. Diagnostics capture output path cleanup.
2. Lens SDF truth cleanup. Shipped.
3. SDF field interface extraction. Shipped.
4. Lens semantics authority. Shipped.
5. Authored SDF pack parser / CPU reference. Shipped.
6. CUDA SDF pack evaluator and hardening. Shipped.
7. Headless/report SDF signal consumption. Partially shipped for flashlight and runtime-walk reports.
8. CUDA Lens SDF backend. Shipped.
9. Live Color Pipeline SDF rows. Shipped.
10. Viewport overlay. Shipped.
11. SDF Source row customization and Source-section field-resolution UX. Shipped.
12. Capture/replay authority smoke matrix. Shipped.
13. Color Pipeline phase-signal metadata. Shipped.
14. Color Pipeline fractal-switch preservation regression. Shipped.
15. SDF realtime pacing telemetry. Shipped.
16. SDF postprocess signal specialization. Active measured FPS follow-up before broader SDF UX/function-library growth.
17. First SDF-native fractal lane. Deferred.

This makes the SDF idea a near-term substrate campaign, not a side quest that
blocks all other product polish. It also keeps the first implementation wins
small enough to review honestly.
