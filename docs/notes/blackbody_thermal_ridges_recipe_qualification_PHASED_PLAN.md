# Black Body Thermal Ridges Recipe Qualification Slice

Status: complete with a narrow post-checkpoint contract-proof binding repair. Product implementation and all qualification thresholds remain unchanged.

## Explicit User Asks

- [x] Add exactly one enabled graph-authoritative recipe: `blackbody_thermal_ridges` / `Black Body Thermal Ridges`.
- [x] Use the existing `smooth_escape_ramp -> mirror_repeat -> blackbody_palette_v1 -> contrast_lift` functions with the locked values.
- [x] Preserve every existing recipe and prove the new recipe through the public selector/Apply path, capture, reload, and replay.
- [x] Produce durable palette-strip, four-way audition, source-occupancy, sensitivity, preservation, and interleaved-performance evidence.
- [x] Complete hostile hardening, validation, receipts, rearward review, push, and stop at the campaign replan boundary.

## Current Phase

Slice C is complete at this checkpoint. The graph-only recipe, public Apply path, evidence bundle, nine-recipe preservation, source-generic measurement, Capture Finding replay, exact CUDA-event performance court, full native rail, Root Glow regression, and published-runtime proof are green. No further product mutation is authorized under this campaign; machine receipts and committed-HEAD rearward review are post-commit closure operations.

## Phase Checklist

- [x] Commit and lock the Slice C plan/contract.
- [x] Freeze the pre-recipe public recipe inventory, graph receipts, representative frame hashes, and timing inputs.
- [x] Add `blackbody_thermal_ridges` only through UI-Salt recipe and `recipe_v2_graph` metadata authority.
- [x] Extend the existing Black Body CUDA witness with host-runtime strip sample export and render four PNG strips through a bounded Python evidence tool.
- [x] Run the locked four-way repeat/mirror and endpoint-direction audition without changing the recipe.
- [x] Prove public Apply, exact graph receipt, quantitative source occupancy, fixed perturbation sensitivity, Capture Finding, and replay.
- [x] Run paired/interleaved 1024 and 2048 performance witnesses and preserve honest thresholds/results.
- [x] Reprove every pre-existing recipe and complete three-pass hostile audit.
- [x] Validate, checkpoint, receipt, rearward-review, push, and stop for replan.

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
| `palette.temperature_1_k` | +20% (12000 -> 14400); the high-temperature chromaticity tail is deliberately less sensitive per kelvin |
| `palette.temperature_mapping` | reciprocal -> linear |
| `grade.exposure` | +0.10 |
| `grade.saturation` | +0.10 |

Capture Finding must preserve authoritative rows/endpoints/mapping in `state.json` and review truth in `fractal-state.json`; replay the archived state twice with identical frame hashes. Derived LUT coordinates remain absent from replay state.

## Performance Court

Use the actual CUDA device shape/palette evaluators in the focused Black Body CUDA executable. Run one CUDA process with exact row-major signal workloads at:

- 1024x768;
- 2048x1536.

The control evaluates the current default identity/Heatmap path. The candidate evaluates the locked mirror-repeat/Black Body path. CUDA events surround only device evaluation; input generation, allocation, process startup, report serialization, and UI work are excluded. This isolates the incremental recipe color-path cost that the sub-millisecond noise gate is intended to judge. The published viewer separately proves normal selector/Apply, graph authority, output, state, capture, and replay. No end-to-end viewer FPS or timing claim is authorized.

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
- `ui_app/src/main.cpp` only for selecting the first single active renderer-backed Source row when automation-report measurement is enabled; the measurement builder, schema, and normal non-automation render path remain unchanged
- `ui_app/src/color_pipeline_core.h` only to require the hardcoded fallback recipes as an unchanged prefix while validating and accepting additional materialized recipes generically
- `ui_app/src/color_pipeline_metadata_parity.cpp` only to report the full authoritative materialized recipe count while preserving exact parity checks for the hardcoded fallback prefix
- `ui_app/tests/test_color_pipeline_core.cpp` for the append-only metadata-authority regression
- `tests/test_fractal_runtime_blackbody_thermal_ridges.py` (new)
- `HANDOFF_LOG.md` and `artifacts/`

No other production C++ source change is permitted. Any further production mutation triggers another contract revision and explicit classification before proceeding.

## Proof Ledger

| Item | Status | Evidence |
| --- | --- | --- |
| Slice A | complete | `9b9ec78`, pushed, rearward `ok` |
| Slice B | complete | `0437e6c`, pushed, rearward `ok` |
| Slice C bootstrap | complete | `3db7ed3`; plan/contract committed under docs-only authority |
| Preservation baseline | complete | `artifacts/blackbody_chromaticity_palette/preservation/existing_recipe_parity.json`; nine public recipe baselines frozen before metadata mutation |
| Recipe metadata | complete | exact graph materializer test passes; generated metadata contains no adapter or tuple bypass |
| Visual/qualification/performance | complete | qualification `passed`, performance `proven`, audition evidence-only/recipe-unchanged, preservation `passed` |
| Closure | complete at checkpoint | contract/plan/code-quality/native/published runtime/hostile/diff rails green; post-commit receipts and rearward review follow |

## Hostile Audit

- Status: complete
- Required posture: assume attractive output can hide recipe bypass, stale metadata, source collapse, wrong sampler evidence, replay drift, or misleading timing.
- Scope: planning/bootstrap plus completed product mutation, public-runtime proof, preservation, and performance hardening.

## Audit Passes

- [x] Pass 1 - planning review locked baseline freeze, graph-only authority, semantic ids, exact recipe overrides, and generated metadata freshness before mutation.
- [x] Pass 2 - planning review found strip and timing authority needed stronger boundaries; strip evidence uses the actual host sampler and performance uses the actual CUDA device evaluators with exact workloads.
- [x] Pass 3 - clean re-read of the repaired planning state confirmed bounded product surfaces, fixed thresholds, durable artifacts, and an explicit campaign stop boundary.
- [x] Pass 4 - product diff review found and repaired append-only recipe rejection, source-measurement over-specialization, and public/runtime identity confusion.
- [x] Pass 5 - runtime proof found and repaired invalid Capture Finding quality-tier comparison, high-temperature sensitivity witness weakness, and float32 receipt formatting brittleness.
- [x] Pass 6 - clean re-read plus full native, Root Glow regression, materializer, public Apply/capture/replay, and CUDA-event courts found no further defect; recipe remains metadata-only and the audition did not mutate it.

## Audit Findings

- [x] Test-only strip generation could have silently sampled the Python reference instead of shipped host arithmetic; evidence now originates from the shared host runtime sampler and records its identity/hash.
- [x] Sequential baseline-then-palette timing could confuse clock drift with palette cost; the performance court alternates CUDA-event pairs in one process.
- [x] Qualification assumed `viewer.color_source_measurement.v1` was source-generic at the live request seam, but `DispatchRenderFrame` requests only `root_log_proximity_v1`; generalize selection only for a single active renderer-backed Source row under automation reporting, then prove the existing Root Glow measurement remains unchanged.
- [x] Runtime metadata installation required the materialized recipe count to equal the nine-row hardcoded fallback, so the staged ten-recipe contract was rejected and the UI silently used fallback. Repair the validator generically: preserve the fallback as an exact prefix, validate every materialized row against supported lane functions, accept append-only metadata recipes, and prove no C++ Black Body recipe id exists.
- [x] The first published performance court trusted interactive `last_render_ms`, but Render Once forces viewport-sized full-quality work while Apply traverses preview/settle frames; exact 1024/2048 workloads and a 0.30 ms MAD were therefore not simultaneously observable. Keep public runtime behavior proof, but measure incremental device evaluator cost with interleaved CUDA events and no UI/report overhead.
- [x] The predeclared +10% high-temperature endpoint perturbation produced only `0.00790` mean absolute normalized RGB change because black-body chromaticity compresses at high kelvin. A bounded audition fixed the witness at +20% (`12000 -> 14400`), which produces `0.01122` without changing the recipe default or the locked `0.01` qualification threshold.
- [x] The first recipe test incorrectly compared the 1024x768 float32 qualification frame with Capture Finding output. Capture Finding intentionally promotes to 4096x3072 `standard`/float64; the repaired court instead proves two archived-state replays agree and their pixels are exactly equal to the archived `frame.png`.
- [x] Frozen graph-receipt equality was brittle to equivalent float32 values serialized with different decimal widths (`1.14999997616` versus `1.149999976158142`). Preservation now canonicalizes only floating receipt leaves to nine decimal places; structure, ids, topology, booleans, enums, and committed frame/row evidence remain exact.
- [x] The first committed contract bound qualification/performance secondary JSON directly, but validation receipts index command-primary evidence and therefore left those assertions unprovable. The repair binds each claim to the exact published JUnit case that creates and asserts the durable JSON artifact; no product code or qualification threshold changed.
## Stop Point

After this slice closes: `Preplanned sliced work is exhausted; stop for replan before more product mutation.`
