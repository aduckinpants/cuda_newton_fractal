# Black Body Thermal Ridges Recipe Qualification Slice

Status: ready to lock as Slice C from clean pushed Slice B head `0437e6c`. This is the final viewer-first child slice of the Black Body campaign.

## Explicit User Asks

- [open] Add exactly one enabled graph-authoritative recipe: `blackbody_thermal_ridges` / `Black Body Thermal Ridges`.
- [open] Use the existing `smooth_escape_ramp -> mirror_repeat -> blackbody_palette_v1 -> contrast_lift` functions with the locked values.
- [open] Preserve every existing recipe and prove the new recipe through the public selector/Apply path, capture, reload, and replay.
- [open] Produce durable palette-strip, four-way audition, source-occupancy, sensitivity, preservation, and interleaved-performance evidence.
- [open] Complete hostile hardening, validation, receipts, rearward review, push, and stop at the campaign replan boundary.

## Current Phase

Slice C bootstrap only. Slice B is closed, pushed, and rearward `ok` at `0437e6c`. No recipe or test mutation may begin until this plan/contract is committed and locked under a fresh viewer-first checkpoint.

## Phase Checklist

- [ ] Commit and lock the Slice C plan/contract.
- [ ] Freeze the pre-recipe public recipe inventory, graph receipts, representative frame hashes, and timing inputs.
- [ ] Add `blackbody_thermal_ridges` only through UI-Salt recipe and `recipe_v2_graph` metadata authority.
- [ ] Extend the existing Black Body CUDA witness with host-runtime strip sample export and render four PNG strips through a bounded Python evidence tool.
- [ ] Run the locked four-way repeat/mirror and endpoint-direction audition without changing the recipe.
- [ ] Prove public Apply, exact graph receipt, quantitative source occupancy, fixed perturbation sensitivity, Capture Finding, and replay.
- [ ] Run paired/interleaved 1024 and 2048 performance witnesses and preserve honest thresholds/results.
- [ ] Reprove every pre-existing recipe and complete three-pass hostile audit.
- [ ] Validate, checkpoint, receipt, rearward-review, push, and stop for replan.

## Entry Gate And Accepted Base

Accepted base is the exact clean pushed Slice B head `0437e6ced13def14d51f5035dd7f9c16e2b55ab8`. Required evidence already exists:

- generated 1024-entry LUT provenance and dense numerical court from Slice A;
- append-only Palette id 9, typed routes, host/device parity, state/capture, and public non-default replay from Slice B;
- `recipe_v2_graph` Resolve/Prepare/Commit is current recipe application authority;
- the legacy recipe fallback is retired and unavailable in normal operation.

A shared recipe/application, binding, palette, or capture defect stops this slice for repair. A scene-specific qualification failure blocks only the new recipe from shipment; it does not invalidate the closed Palette.

## Locked Recipe

Add exactly:

```text
recipe id: blackbody_thermal_ridges
version: 1
label: Black Body Thermal Ridges

source.escape_signal = smooth_escape_ramp
  signal.scale = 1
  signal.bias = 0
  signal.blend_weight = 1

shape.thermal_ridges = mirror_repeat
  shape.frequency = 4
  shape.phase = 0

palette.blackbody = blackbody_palette_v1
  palette.temperature_0_k = 1600
  palette.temperature_1_k = 12000
  palette.temperature_mapping = reciprocal_temperature
  palette.blend_weight = 1
  palette.blend_mode = normal

grading.thermal_finish = contrast_lift
  grade.exposure = 1
  grade.saturation = 1
```

The recipe is authored only in `docs/ui_salt/color_pipeline_function_library.ui.salt`; generated metadata is refreshed by the materializer. It must resolve, prepare, and commit through existing graph authority. No C++ recipe-id switch, tuple reconstruction, hidden adapter, or special evaluator branch is allowed.

Semantic node ids above are stable. All overrides use version-stable descriptor parameter ids. The recipe canonical hash is calculated after alias normalization, deterministic node/override ordering, typed-value normalization, explicit default expansion, and exclusion of display-only text.

## Preservation Lock

Before adding the recipe, capture a baseline for all current public recipes using the current published runtime:

- ordered recipe ids and labels;
- materialized nodes, edges, overrides, adapters, and canonical content hashes;
- public selector/Apply receipts;
- authoritative committed rows and fingerprints;
- representative deterministic frame hashes at fixed scene, size, precision, and backend;
- state reload/replay hashes.

After the recipe is added, every pre-existing recipe must remain bit-identical in the same frame-byte hash domain. The recipe count may increase by exactly one. `state.json` remains row/replay authority; recipe provenance after reload remains truthful and must not be fabricated.

## Palette Strip Evidence

Use the actual shared host runtime sampler from `blackbody_palette_lut.h`, exposed only through the existing dedicated Black Body test executable. A bounded Python evidence tool invokes the executable to export sample JSON and renders 512x48 PNGs without introducing a product-visible source.

Required strips:

1. 1600 to 12000 K, reciprocal;
2. 1600 to 12000 K, linear;
3. 12000 to 1600 K, reciprocal;
4. 1600 to 1600 K, reciprocal.

`blackbody_palette_strips.json` records endpoint/mapping inputs, sample count, runtime sampler identity, RGB samples/hash, output PNG/hash, and endpoint/reversal/equal-endpoint checks. The evidence tool must fail if the test executable is absent or reports a different LUT metadata hash.

## Audition And Qualification

The four audition captures are evidence only:

| Shape | Endpoints |
| --- | --- |
| `repeat` | 1600 -> 12000 |
| `repeat` | 12000 -> 1600 |
| `mirror_repeat` | 1600 -> 12000 |
| `mirror_repeat` | 12000 -> 1600 |

They use one fixed Mandelbrot state, f32 CUDA direct rendering, 1024x768, full quality, and identical source/palette/grading values. The locked recipe remains mirror-repeat forward regardless of audition outcome.

Public qualification uses the normal recipe selector and Apply action, then proves:

- application authority `recipe_v2_graph`, fallback false;
- exact nodes, edges, normalized overrides, committed rows, and live-row fingerprint agreement;
- finite-pixel percentage 100%;
- existing `viewer.color_source_measurement.v1` shaped scalar `p95-p05 >= 0.20`;
- at least 8 of 32 shaped-signal/LUT-coordinate bins occupied;
- stationary frame difference exactly zero;
- unrelated UI selection changes no rendered pixels;
- each fixed owning-parameter perturbation produces mean absolute normalized RGB change >= 0.01.

Fixed perturbations:

| Parameter | Qualification perturbation |
| --- | --- |
| `shape.frequency` | +10% (4.0 -> 4.4) |
| `shape.phase` | +0.10 |
| `palette.temperature_0_k` | +10% (1600 -> 1760) |
| `palette.temperature_1_k` | +10% (12000 -> 13200) |
| `palette.temperature_mapping` | reciprocal -> linear |
| `grade.exposure` | +0.10 |
| `grade.saturation` | +0.10 |

Capture Finding must preserve authoritative rows/endpoints/mapping in `state.json` and review truth in `fractal-state.json`; replay the archived state twice with identical frame hashes. Derived LUT coordinates remain absent from replay state.

## Performance Court

Use one persistent published process per size, fixed Mandelbrot state, f32 CUDA direct, full quality:

- 1024x768;
- 2048x1536.

After five alternating Heatmap/Black-Body warm-up pairs, record twenty alternating measured pairs. Each pair applies/renders Heatmap then Black Body (or alternates the first member by pair index) so clock and scheduler drift affect both sides symmetrically. Record renderer timing separately from UI/capture/startup.

Artifact fields include runtime executable SHA-256, GPU/driver/runtime identity when reported, state and receipt fingerprints, ordered raw samples, per-mode median/MAD/p95, paired deltas, and acceptance classification.

Acceptance:

- Heatmap pre/post control median drift <= `max(5%, 0.25 ms)`;
- Black Body paired median overhead <= 1.5 ms at 1024 and <= 6 ms at 2048;
- paired MAD <= `max(0.25 ms, 20% of the relevant overhead budget)`.

One fresh-process rerun is allowed for a noisy result. A second noisy result is `unproven` and blocks recipe release. No performance-improvement claim is authorized.

## Durable Artifacts

Required generated evidence:

- `artifacts/blackbody_chromaticity_palette/reference/blackbody_lut_validation.json`;
- `artifacts/blackbody_chromaticity_palette/strips/blackbody_palette_strips.json` and four PNGs;
- `artifacts/blackbody_chromaticity_palette/audition/blackbody_recipe_audition.json` and four PNGs;
- `artifacts/blackbody_chromaticity_palette/qualification/blackbody_qualification.json`;
- `artifacts/blackbody_chromaticity_palette/performance/blackbody_performance.json`;
- `artifacts/blackbody_chromaticity_palette/preservation/existing_recipe_parity.json`.

Artifacts are evidence, not product authority. They must record the exact runtime/build/state inputs that produced them.

## Expected Mutation Surface

- `docs/ui_salt/color_pipeline_function_library.ui.salt`
- `docs/ui_salt/generated/color_pipeline_function_library.contract.v1.json`
- this plan/contract and umbrella truth sync
- focused existing materializer/native recipe tests
- `ui_app/tests/test_blackbody_palette_cuda.cu` and `ui_app/build_tests_vsdevcmd.cmd` only if needed for test-only strip export
- `tools/blackbody_palette_evidence.py` (new)
- `tests/test_fractal_runtime_blackbody_thermal_ridges.py` (new)
- `HANDOFF_LOG.md` and `artifacts/`

No production C++ source change is expected. Any required production mutation triggers a contract revision and explicit classification before proceeding.

## Proof Ledger

| Item | Status | Evidence |
| --- | --- | --- |
| Slice A | complete | `9b9ec78`, pushed, rearward `ok` |
| Slice B | complete | `0437e6c`, pushed, rearward `ok` |
| Slice C bootstrap | complete | `3db7ed3`; plan/contract committed under docs-only authority |
| Preservation baseline | pending | pre-recipe evidence not frozen |
| Recipe metadata | pending | no `blackbody_thermal_ridges` recipe exists |
| Visual/qualification/performance | pending | required artifacts absent |
| Closure | pending | validation, audit, receipts, rearward review, push |

## Hostile Audit

- Status: complete
- Required posture: assume attractive output can hide recipe bypass, stale metadata, source collapse, wrong sampler evidence, replay drift, or misleading timing.
- Scope: planning/bootstrap audit only. Product mutation reopens this audit under the locked Slice C checkpoint.

## Audit Passes

- [x] Pass 1 - planning review locked baseline freeze, graph-only authority, semantic ids, exact recipe overrides, and generated metadata freshness before mutation.
- [x] Pass 2 - planning review found strip and timing authority needed stronger boundaries; the plan now requires actual host-runtime sampler export and one persistent interleaved process per size.
- [x] Pass 3 - clean re-read of the repaired planning state confirmed bounded product surfaces, fixed thresholds, durable artifacts, and an explicit campaign stop boundary.

## Audit Findings

- [x] Test-only strip generation could have silently sampled the Python reference instead of shipped host arithmetic; evidence now originates from the shared host runtime sampler and records its identity/hash.
- [x] Sequential baseline-then-palette timing could confuse clock drift with palette cost; the performance court now alternates pairs in one persistent process per size.

## Stop Point

After this slice closes: `Preplanned sliced work is exhausted; stop for replan before more product mutation.`
