# Black Body Chromaticity Palette Campaign

Status: planning reconciliation on `codex/blackbody-chromaticity-palette` from clean merged base `3e6dc8d`. Parameter Motion remains paused. This umbrella plan sequences bounded child slices; the current contract is planning-only and authorizes no product mutation.

## Explicit User Asks

- [done] Prioritize a reusable Black Body palette ahead of paused Parameter Motion implementation.
- [done] Treat it as an artistic black-body-inspired scalar mapping, not physical temperature or radiance.
- [done] Specify a typed Palette function and one graph-authoritative curated recipe.
- [done] Vendor official CIE observer data and make LUT generation reproducible.
- [done] Incorporate both external review rounds and the fresh blind hostile review without widening the product.
- [deferred-to-child-slices] Implement, prove, checkpoint, and push the palette and recipe through the bounded contracts below.

## Current Phase

Planning reconciliation. Parameter Motion is merged and pushed as paused documentation. A fresh blind review rejected the first campaign-spanning contract because it mixed uncheckpointed planning, reference, runtime, and closure work and left display, CUDA storage, non-default replay, compatibility, evidence, and performance authority ambiguous. Those findings are resolved below. Next: checkpoint this planning-only surface, then open Slice A under a fresh contract.

## Phase Checklist

- [x] Merge and push accepted Parameter Motion planning without activating implementation.
- [x] Open `codex/blackbody-chromaticity-palette` from merged `master`.
- [x] Record approved reference, runtime, recipe, and proof semantics.
- [x] Run a fresh blind hostile review.
- [x] Reconcile its findings into bounded child slices.
- [ ] Validate and checkpoint this planning-only contract.
- [ ] Slice A - reference data, deterministic generator, LUT, and independent numerical court.
- [ ] Slice B - append-only runtime Palette, CUDA parity, state/capture, and non-default published replay.
- [ ] Slice C - graph recipe, visual/occupancy evidence, interleaved performance, preservation, hardening, and closure.
- [ ] Stop for replan before additional palettes, color management, graph UI, SDF growth, or Parameter Motion implementation.

## Campaign Slice Contracts

The long-running goal sequences the work. Every mutation slice gets a fresh checked-in plan/contract, hostile audit, proof receipts, checkpoint, rearward review, and push.

### Slice A - Reference And LUT

Planned surfaces:

- `docs/notes/blackbody_chromaticity_palette_reference_lut_PHASED_PLAN.md`
- `docs/contracts/blackbody_chromaticity_palette_reference_lut.contract.json`

This native/reference slice vendors official inputs, implements the deterministic generator, emits the generated host/device initializer and metadata, and proves source checksum, coordinate identities, independent chromaticity witnesses, freshness, linear-RGB authority, and 4096-point accuracy. It does not change viewer behavior.

### Slice B - Runtime Palette

Planned surfaces:

- `docs/notes/blackbody_chromaticity_palette_runtime_PHASED_PLAN.md`
- `docs/contracts/blackbody_chromaticity_palette_runtime.contract.json`

This viewer-first slice adds enum id 9, typed metadata, exact compatibility rows, cache authority, host/device LUT sampling, a real NVCC parity kernel, UI controls, state/capture/report support, and a published non-default replay witness. It stops after the standalone Palette is proven.

### Slice C - Recipe And Qualification

Planned surfaces:

- `docs/notes/blackbody_thermal_ridges_recipe_qualification_PHASED_PLAN.md`
- `docs/contracts/blackbody_thermal_ridges_recipe_qualification.contract.json`

This viewer-first slice adds the graph recipe, freezes/verifies existing recipe parity, writes durable strip/audition/occupancy/performance artifacts, runs public Apply/capture/replay, and closes the campaign.

## Scope Lock

In scope:

- One CIE/Planck generator and one generated 512-entry RGB LUT.
- One append-only Palette: `blackbody_palette_v1` / `Black Body`.
- Two Kelvin endpoints and one mapping enum.
- One recipe: `blackbody_thermal_ridges` / `Black Body Thermal Ridges`.
- Existing Color Pipeline state, graph receipt, Capture Finding, replay, and source-measurement authorities.
- Palette strips, four-way audition, quantitative source occupancy, and interleaved timing.

Out of scope:

- Physical-temperature, radiance, or display-colorimetry claims for fractal renders.
- A global display-transfer/color-management repair.
- Alternative observers, chromatic adaptation, Delta E, HDR, histogram equalization, or dynamic normalization.
- New Shape primitives, graph editor, Salticid execution, SDF ops, fractal lanes, or Parameter Motion implementation.
- Existing recipe/palette/state-wire behavior changes.

## Reference Contract

### Observer And Spectrum

- CIE 1931 colour-matching functions, 2-degree observer.
- Dataset page: `https://cie.co.at/datatable/cie-1931-colour-matching-functions-2-degree-observer`.
- DOI: `10.25039/CIE.DS.xvudnb9b`.
- CSV SHA-256: `fa663e3535a7e0763a745993a1f0a192eb0275ac46ad2d1befd7626841e713c1`.
- CC BY-SA 4.0; official metadata and attribution ship beside the CSV.
- 360 through 830 nm inclusive at 1 nm; linear interpolation and zero extrapolation.

```text
B(lambda,T) proportional to 1 / (lambda^5 * expm1(c2/(lambda*T)))
c2 = 1.438776877e-2 metre kelvin
```

Trapezoid-integrate `B*xbar`, `B*ybar`, and `B*zbar`, then normalize XYZ to `Y=1`.

### XYZ To Internal Linear RGB

Use the exact W3C CSS Color 4 XYZ-D65 to linear-sRGB rational matrix:

```text
[ 12831/3959       -329/214          -1974/3959      ]
[ -851781/878810    1648619/878810    36519/878810   ]
[ 705/12673        -2585/12673        705/667         ]
```

Metadata records rational and float64 coefficients. No display-encoded sRGB transfer is applied.

```text
rgb0 = matrix * XYZ
lift = max(0, -min(rgb0.r, rgb0.g, rgb0.b))
rgb1 = rgb0 + lift * (1,1,1)
rgb2 = rgb1 / max(rgb1.r, rgb1.g, rgb1.b)
result = clamp(rgb2, 0, 1)
```

This translates along the neutral axis into nonnegative RGB and projects onto `max(R,G,B)=1`.

### Display Authority Boundary

The LUT and Palette output are internal `color.linear_rgb`. The current viewer ultimately quantizes the pipeline result into an untagged UNORM target and owns no general linear-to-display transfer. Therefore numerical evidence may claim internal linear-RGB correctness; screenshots are artistic/behavioral evidence only and may not claim displayed CIE chromaticity fidelity. This campaign will not hide sRGB encoding inside one palette because that would lie about its type and create split display authority. General output color management is deferred.

### LUT Artifact

- 512 float32 RGB entries, uniform in reciprocal temperature from 800 through 40000 K.
- Generated initializer/header and JSON metadata are checked in and byte-freshness tested.
- Metadata records generator version, source hashes, constants, matrix, endpoints, neutral-lift count/max/temperature, and witnesses at 800, 1600, 3000, 6500, 12000, and 40000 K.
- Witnesses include XYZ, xy, pre-gamut RGB, lift, final RGB, and LUT RGB.
- At least three independently encoded broad xy witnesses, including approximately 2856 K and 6500 K, catch gross wavelength/matrix/orientation errors without sharing generator code.

## Runtime Contract

```text
q(T) = ((1/T) - (1/800)) / ((1/40000) - (1/800))
x(T) = 511*q(T)
```

Append `ColorPalette::blackbody_palette_v1 = 9`. Function id `blackbody_palette_v1`, label `Black Body`, typed `scalar.unit -> color.linear_rgb`.

| Path | Type | Bounds/default |
| --- | --- | --- |
| `palette.temperature_0_k` | float | `800..40000`, default `1600`, step `50` |
| `palette.temperature_1_k` | float | `800..40000`, default `12000`, step `50` |
| `palette.temperature_mapping` | enum | `reciprocal_temperature | linear_kelvin`, default reciprocal |

Equal/reversed endpoints are valid. Finite input clamps to `[0,1]`; nonfinite becomes zero. Materialization derives non-persisted `x0=x(T0)`, `x1=x(T1)`, and `dx=x1-x0`. State stores only endpoints/mapping; every row rebuild, load, endpoint edit, and mapping edit reconstructs the cache.

Reciprocal mode evaluates `x=fma(u,dx,x0)` with no Kelvin or reciprocal. Linear mode evaluates `T=lerp(T0,T1,u)` then `x=x(T)`. Clamp x, guard entry 511, otherwise manually floor and interpolate two entries.

### CUDA LUT Ownership

The generated initializer is the single byte authority. Host code gets an `inline constexpr` mirror. Each CUDA translation unit gets an internal-linkage `__device__ __constant__` LUT from the same initializer because the build does not use relocatable device code. This avoids upload lifecycle and cross-TU linkage. A dedicated NVCC `.cu` test launches a deterministic sampling kernel and compares device RGB to host evaluation with maximum channel error `2e-6`; the MSVC coloring test is not CUDA proof.

## Compatibility Contract

UI-Salt is normal authority; fallback descriptors, enum bridges, schedule reverse mapping, validation, state/capture, and family gates stay in parity.

Direct scalar-unit source court:

- `smooth_escape_ramp`
- `root_proximity`
- `sdf_boundary_band`
- `lens_field_v2_distance`

Signed source court:

- `root_log_proximity_v1 -> signed_unit_map_v1 -> blackbody_palette_v1`
- `sdf_curvature -> signed_unit_map_v1 -> blackbody_palette_v1`

Category, phase, and raw signed routes fail closed. Slice B must name every required `resolution_case`, `compat`, specialized override, and mirrored hardcoded route before mutation.

## Recipe Contract

Add enabled `blackbody_thermal_ridges`:

```text
source.escape_signal = smooth_escape_ramp
-> shape.thermal_ridges = mirror_repeat(frequency=4, phase=0)
-> palette.blackbody = blackbody_palette_v1(1600,12000,reciprocal_temperature)
-> grading.thermal_finish = contrast_lift(exposure=1,saturation=1)
```

It uses existing `recipe_v2_graph` Resolve/Prepare/Commit only. No id switch, direct tuple reconstruction, or special case. The four-way repeat/mirror-repeat crossed with forward/reversed endpoints audition is evidence only and cannot alter the locked recipe.

## Test And Evidence Contract

### Numerical And Backend

- Source checksum/domain and deterministic byte freshness.
- `q(800)=0`, `q(40000)=1`, monotonicity, and `q(T_i) ~= i/511`.
- 4096 temperatures compare interpolated LUT output to the complete float64 pipeline; max channel error `<=0.002`.
- Independent xy witnesses and a linear-RGB-not-sRGB guard.
- Deterministic reversal, equal-endpoint, and mapping-endpoint metamorphic cases.
- Change only T0, only T1, then mapping; each result equals a fresh reference.
- Dedicated NVCC host/device court, max channel error `2e-6`.

### State And Runtime

Mandatory non-default replay uses T0=2400 K, T1=18000 K, and `linear_kelvin`. It captures `state.json` and Finding `fractal-state.json`, reloads, proves numeric readback and cache reconstruction, and matches frame hashes. Defaults-only replay cannot close the slice.

The runtime test exercises actual Palette row controls and the public recipe selector/Apply path. Existing recipe parity is bit-exact under the frozen backend, precision, scene, dimensions, frame, and warm-up court. No physical mouse.

### Durable Artifacts

Slice C contract must assert:

- `artifacts/blackbody_chromaticity_palette/reference/blackbody_lut_validation.json`
- `artifacts/blackbody_chromaticity_palette/strips/blackbody_palette_strips.json` and four PNGs
- `artifacts/blackbody_chromaticity_palette/audition/blackbody_recipe_audition.json` and four PNGs
- `artifacts/blackbody_chromaticity_palette/qualification/blackbody_qualification.json`
- `artifacts/blackbody_chromaticity_palette/performance/blackbody_performance.json`
- `artifacts/blackbody_chromaticity_palette/preservation/existing_recipe_parity.json`

Strips sample the actual host runtime Palette and independently catch LUT collapse. Qualification also records existing `viewer.color_source_measurement.v1` shaped bins. Locked scene requires `p95-p05>=0.20` and `>=8/32` occupied bins.

## Performance Contract

- Fixed Mandelbrot state, CUDA direct renderer, f32, full quality.
- 1024x768 and 2048x1536.
- One persistent published process per size.
- Five alternating Heatmap/Black Body warm-up pairs, then twenty measured pairs.
- Artifact records runtime hash, device/driver/runtime identity when reported, state/receipt hashes, all samples, medians, MADs, p95s, and paired deltas.
- Renderer timing excludes UI/capture/startup.

Acceptance:

- Heatmap pre/post medians differ by no more than `max(5%,0.25 ms)`.
- Black Body paired median overhead `<=1.5 ms` at 1024 and `<=6 ms` at 2048.
- Paired MAD `<=max(0.25 ms,20% of overhead budget)`. One fresh-process rerun is allowed; a second noisy result is `unproven` and blocks closure.
- No performance-improvement claim.

## Proof Ledger

| Item | Status | Evidence |
| --- | --- | --- |
| Parameter Motion merge | complete | `master` and `origin/master` at paused planning head `3e6dc8d` |
| Campaign branch | complete | `codex/blackbody-chromaticity-palette` |
| Initial contract | rejected | blind review found workflow and authority gaps |
| Reconciled umbrella plan | complete | bounded Slice A/B/C contracts defined |
| Slice A | pending | child plan/contract not opened |
| Slice B | pending | child plan/contract not opened |
| Slice C | pending | child plan/contract not opened |

## Action Hostile Review

- Action ID: action-20260905-blackbody-planning-reconciliation-2
- Suspected Failure Mode: a campaign-spanning contract could bypass checkpoints while attractive screenshots hide wrong color-space, device, state, cache, compatibility, or timing authority.
- Correct Owner/Action: checkpoint this planning reconciliation only, then require fresh bounded contracts for reference/LUT, runtime Palette, and recipe qualification.
- Proof Surface: plan sync, contract validation, code-quality baseline, recorded blind findings, diff check, hostile-audit validation, checkpoint receipt, rearward review, and pushed clean branch.
- Blocked Action: any product, generator, metadata, runtime, state, recipe, or test mutation before Slice A is separately locked.

## Hostile Audit

- Status: complete
- Outcome: the first contract was rejected; all ten findings are resolved here or made mandatory child-slice gates.
- Slice boundary: planning reconciliation only; no product mutation authorized.

## Audit Passes

- [x] Pass 1 - fresh reviewer checked plan/contract against code and workflow.
- [x] Pass 2 - local reconciliation checked display packing, typed metadata, CUDA topology, state serializers, compatibility, measurement, and evidence.
- [x] Pass 3 - clean re-read of the repaired state confirmed truthful asks, bounded checkpoints, exact owners, durable artifacts, and stop boundaries; no additional real issue found.

## Audit Findings

- [x] Future deliverables were marked complete; they now delegate to child slices.
- [x] One contract spanned five uncheckpointed phases; three child contracts are required.
- [x] No display-transfer owner exists; numerical claims stop at internal linear RGB.
- [x] CUDA storage was unspecified; TU-local constant storage and NVCC parity are locked.
- [x] Default replay could hide loss; non-default replay is mandatory.
- [x] Recipe/compatibility were underlocked; semantic nodes and source courts are explicit.
- [x] Visual/performance proof lacked artifacts; durable JSON/PNG surfaces are required.
- [x] Timing lacked inputs/noise rules; both are fixed.
- [x] Scope was broad; this contract is narrow and child contracts must enumerate files.
- [x] LUT proof was self-referential; independent xy witnesses are mandatory.

## Stop Point

After Slice C: `Preplanned sliced work is exhausted; stop for replan before more product mutation.`
