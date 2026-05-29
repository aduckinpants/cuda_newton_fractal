# SDF Pack Scene Lane Shell

## Current Phase

Closed - SDF pack scene lane shell implemented and validated; receipts and rearward review are recorded as closure artifacts.

## Phase Checklist

- [x] Phase 1 - create this checked-in phased plan and contract, then lock the active viewer-first slice with `ck:cc3f95e7`.
- [x] Phase 2 - record the mini-sprint planning workflow idea as deferred workflow debt without changing hooks or protocols.
- [x] Phase 3 - add RED tests for `sdf_pack_scene` dropdown identity, built-in pack control visibility, built-in pack parsing, field sensitivity, and state/report authority.
- [x] Phase 4 - implement the `sdf_pack_scene` lane shell backed by the authored SDF pack field producer and the built-in `sdf_smooth_lattice_2d` pack.
- [x] Phase 5 - preserve normal Color Pipeline consumption, capture/replay authority, and no-mouse automation/reporting for the lane.
- [x] Phase 6 - publish runtime and collect bounded 1024/2048 long-edge performance witnesses for `sdf_smooth_lattice_2d`.
- [x] Phase 7 - hostile audit, focused validation, receipts, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [x] Start from clean `master` at `cf9754e` and branch `codex/sdf-pack-scene-lane-shell`.
- [x] Add normal fractal dropdown entry `sdf_pack_scene`, label `SDF Pack Scene`, group `SDF`.
- [x] Add built-in/default pack `sdf_smooth_lattice_2d` under `docs/examples/sdf_packs/` using only shipped SDF pack ops.
- [x] Expose `period`, `radius`, `smooth_blend`, `rotation`, `offset_x`, and `offset_y` through the normal left-panel flow and no-mouse automation seams.
- [x] Render the lane as a field-primary SDF scene, not an overlay on another fractal.
- [x] Preserve normal Color Pipeline behavior so existing SDF/color source rows, palettes, and graders color the lane.
- [x] Persist and replay selected lane, built-in pack id, backend preference/used backend, and control values through state/capture/finding replay.
- [x] Report active fractal type, active pack id, SDF backend used, field dimensions/downsample, and enough timing for bounded FPS claims.
- [x] Record the mini-sprint planning workflow idea as deferred workflow debt only.
- [x] Stop for a fresh replan before adding new SDF composition families, new ops, recursive folds, apollonian fields, or broader pack-catalog UX.

## Scope

In scope:

- One selectable `sdf_pack_scene` lane.
- One curated built-in SDF pack, `sdf_smooth_lattice_2d`.
- Normal left-panel control exposure for the built-in pack controls.
- Existing authored SDF pack parser/lowerer/CPU/CUDA field producer reuse.
- Existing Color Pipeline source/palette/grading consumption of the produced SDF field.
- State/capture/replay/reporting/runtime proof for this lane.

Out of scope:

- Arbitrary user pack catalog or authoring UX.
- New SDF pack ops such as fold, mirror, polar repeat, inversion, or apollonian math.
- New recursive composition families.
- Per-pack renderer rewrites or one-off CUDA kernels.
- Broader Color Pipeline composition redesign.
- Workflow hook/protocol/tool changes for mini-sprint planning.
- Physical mouse automation.

## Proof Ledger

- Bootstrap authority: branch started from clean `master` at `cf9754e` with rearward review `ok`.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "SDF pack scene lane shell" --profile runtime --plan docs/notes/sdf_pack_scene_lane_shell_PHASED_PLAN.md --contract docs/contracts/sdf_pack_scene_lane_shell.contract.json` appended `ck:cc3f95e7` and locked the active contract.
- Deferred workflow debt: mini-sprint planning formalization is recorded in `DEFERRED_THREADS.md` and no hook/protocol mutation is in scope.
- Current substrate: authored SDF packs already have parser/lowerer, CPU reference, CUDA evaluator, field producer bridge, viewer controls, persistence, live Color Pipeline/overlay consumption, and capture/replay authority.
- RED proof: `artifacts/validation/sdf_pack_scene_red_fractal_types.json` failed before implementation because `FractalType::sdf_pack_scene` did not exist.
- Native product proof: focused `test_ui_schema`, `test_schema_binding`, `test_fractal_catalog_authority`, `test_sdf_pack`, `test_sdf_pack_field_producer`, `test_sdf_pack_viewer_ui`, and `test_fractal_family_rules` are green through logged artifacts under `artifacts/validation/`.
- Full native proof: `artifacts/validation/sdf_pack_scene_full_native_final_long.json` is green after the lens-semantics repair; the earlier 1800-second full-native run timed out and is not counted as closure proof.
- Runtime publish proof: `artifacts/validation/sdf_pack_scene_runtime_publish_final.json` rebuilt the published runtime at `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Published runtime proof: `artifacts/validation/sdf_pack_scene_runtime_pytest_lane_final.json` proves no-mouse selector identity, built-in control sensitivity, Color Pipeline frame change, and replay hash parity for `sdf_pack_scene`.
- Performance witness: `artifacts/sdf_pack_scene_lane_shell/perf/sdf_pack_scene_perf_summary.json` records bounded `sdf_smooth_lattice_2d` rows at 1024x768 and 2048x1536; this slice claims only those measured built-in-pack timings.
- Built-in pack op boundary: `docs/examples/sdf_packs/sdf_smooth_lattice_2d.sdf_pack.json` uses only shipped SDF pack ops already present in the parser/evaluator.
- Closure validators passed before checkpoint: contract validation, phased-plan sync, hostile-audit validation, code-quality baseline, logged diff check, full native helper suite, runtime publish, public runtime pytest lane, and bounded performance witness.

## Action Hostile Review

- Action ID: sdf_pack_scene_lane_shell_initial_mutation_v1
- Suspected Failure Mode: adding a dropdown id without a field-primary renderer or without persisted pack authority could create another visible-but-dead lane.
- Correct Owner/Action: mutate only the SDF pack scene lane shell surfaces, built-in pack metadata, focused tests, and deferred backlog note required by this slice.
- Proof Surface: native schema/binding/SDF pack/field/state/report tests plus published no-mouse runtime proof for selector identity, control sensitivity, Color Pipeline rendering, capture/replay parity, reports, and bounded performance.
- Blocked Action: do not add new SDF ops, recursive/apollonian composition families, broad pack-catalog UX, hook/protocol workflow changes, or physical mouse automation.

## Hostile Audit

- Status: complete

Required questions:

- Did the lane actually appear in the normal fractal dropdown and remain selected as `sdf_pack_scene`? Yes, covered by schema/binding tests and published no-mouse runtime proof.
- Did the built-in pack controls actually render in the normal left-panel flow and change the field/rendered frame? Yes, covered by native report tests and no-mouse edits for all six controls.
- Did the implementation reuse the authored SDF pack field producer instead of adding a bespoke renderer monolith? Yes, the lane feeds the existing authored SDF pack field producer and uses only a black base frame for field-primary composition.
- Did Color Pipeline rows/palettes/graders still color the lane through the existing SDF field path? Yes, the runtime proof edits `color_pipeline.source.sdf_signed_distance.signal.scale.primary` and gets a frame-hash change.
- Did capture/replay persist lane id, built-in pack id, backend, downsample, and control values? Yes, state/replay tests persist `sdf_pack_scene`, `pack_id`, pack params, and replay matching pixels; backend/downsample are reported in runtime automation/perf artifacts.
- Did reports identify the active pack id and backend used on the published runtime path? Yes, `sdf_pack` report exposes `pack_id`; runtime automation reports `lens_sdf_field_source_pack_id` and `lens_sdf_pack_backend_used`.
- Did the slice avoid new SDF ops, recursive/apollonian fields, broader catalog UX, and workflow tooling changes? Yes, only deferred backlog text was added for workflow debt and the built-in pack uses shipped ops.
- Did performance claims stay bounded to measured `sdf_smooth_lattice_2d` evidence? Yes, only the 1024x768 and 2048x1536 built-in pack witness rows are claimed.

## Audit Passes

- [x] Pass 1 - pre-mutation seam audit over dropdown/schema, fractal type ids, SDF pack producer, state IO, report, and runtime harness.
- [x] Pass 2 - diff audit after implementation found real omissions in state/report authority and Lens semantics coverage; both were repaired with regression proof.
- [x] Pass 3 - clean re-read of the repaired state after full native, runtime publish, runtime proof, performance witness, and `git diff --check` found no additional real defect in the scoped lane shell.

## Audit Findings

- [x] Finding 1 - pre-lock workflow bridge: the closed Step 6 contract did not scope successor plan/contract files, while `viewer_host_begin_work_slice.py` requires those files to exist before replacing the lock. Repaired with the narrowest preflight: re-lock current Step 6 truth, widen only docs contract/plan scope, add this plan/contract, then immediately replace the active lock with `sdf_pack_scene_lane_shell`.
- [x] Finding 2 - the first implementation added a shipped `sdf_pack_scene` fractal type without adding a Lens semantics descriptor. Full native caught this in `test_fractal_family_rules`; repaired by adding explicit `sdf_field_membership` semantics and a regression assertion that legacy basin/escape mask booleans fail closed for the field-primary lane.
- [x] Finding 3 - the initial SDF pack viewer state serialization was replay-authoritative through `pack_json` but did not expose an explicit `pack_id` marker for report/state readers. Repaired in `SerializeSdfPackViewerStateJson` and covered by `test_sdf_pack_viewer_ui` plus the published runtime replay proof.
- [x] Finding 4 - the default full-native wrapper timeout of 1800 seconds was too low for this full helper suite on the current machine and produced a timeout after passing late generic CUDA tests. Re-ran the exact full native helper with a 5400-second timeout; `sdf_pack_scene_full_native_final_long` passed and is the closure native proof.
- [x] Clean re-read - after the repairs, the scoped diff keeps the lane field-primary, reuses the authored SDF pack field producer, avoids new SDF ops/workflow tooling, and preserves existing Color Pipeline SDF consumption through the no-mouse runtime proof.
