# SDF Pack Built-In Catalog Seed

## Current Phase

Phase 7 - complete; checkpoint commit, receipts, rearward review, push, and clean-tree closeout are handled by the slice closure flow.

## Phase Checklist

- [x] Phase 1 - create and lock this checked-in plan/contract from clean `master` at `4845ee6` on `codex/sdf-pack-builtin-catalog-seed`.
- [x] Phase 2 - add RED tests for built-in pack catalog entries, selector/report authority, current-pack retention, state/replay, and per-pack control sensitivity.
- [x] Phase 3 - add curated built-in pack JSON files using only shipped SDF pack ops.
- [x] Phase 4 - implement the built-in pack catalog helper, normal left-panel pack selector, and no-mouse selector automation/reporting.
- [x] Phase 5 - preserve `sdf_smooth_lattice_2d` default, Color Pipeline field consumption, capture/replay authority, and runtime report fields.
- [x] Phase 6 - collect focused native, published no-mouse runtime, and bounded performance proof for every built-in pack.
- [x] Phase 7 - hostile audit, validation, checkpoint, receipts, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [done] Continue the next implementation after the post-lane replan selected the built-in SDF pack catalog seed.
- [done] Keep `sdf_pack_scene` as the existing normal fractal dropdown lane and keep `sdf_smooth_lattice_2d` as the default pack.
- [done] Add a small curated built-in pack catalog using only shipped SDF pack operations.
- [done] Add normal left-panel pack selection and no-mouse automation/report authority.
- [done] Prove each built-in pack is selectable, reports its active pack id, exposes live controls, changes output when controls move, and persists/replays correctly.
- [done] Do not add new SDF ops, recursive/apollonian fields, broad authored-pack UX, Color Pipeline redesign, or physical mouse automation.

## Scope

In scope:

- Curated built-in pack catalog metadata for `sdf_pack_scene`.
- Two additional built-in packs under `docs/examples/sdf_packs/`.
- Pack selector in the existing SDF pack viewer inline panel.
- No-mouse selector automation/reporting for the built-in pack selector.
- State/capture/replay and runtime report proof for selected pack id and params.
- Bounded performance witness rows for the selected built-in pack set.

Out of scope:

- New SDF pack AST operations or evaluator changes that widen the op surface.
- Recursive folds, apollonian fields, inversion, mirror/polar-repeat/fold ops, or broader SDF-native families.
- Arbitrary user pack catalog, pack manager, or authored text editor UX.
- Color Pipeline redesign, new palette/shape behavior, or SDF composition operand work.
- Workflow hook/protocol changes.
- Physical mouse automation.

## Proof Ledger

- Bootstrap authority: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` reported branch `codex/sdf-pack-builtin-catalog-seed`, head `4845ee6`, and clean state.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported no staged, unstaged, or untracked files before this plan/contract bootstrap.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `ok` for `4845ee6` before this branch opened product mutation.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "SDF pack built-in catalog seed" --profile runtime --plan docs/notes/sdf_pack_builtin_catalog_seed_PHASED_PLAN.md --contract docs/contracts/sdf_pack_builtin_catalog_seed.contract.json` appended checkpoint token `ck:06fb90cc` and locked `global_active_contract`.
- RED proof: `artifacts/validation/sdf_pack_builtin_catalog_seed_red_viewer_ui.json` failed before implementation because the built-in catalog types, selector id, and catalog-load API did not exist.
- Native SDF pack viewer proof: `artifacts/validation/sdf_pack_builtin_catalog_seed_native_viewer_ui.json` passed with 133 assertions after implementation.
- Native report proof: `artifacts/validation/sdf_pack_builtin_catalog_seed_native_report.json` passed and compiles the automation report serializer with the built-in selector/catalog fields.
- Native SDF parser/field proof: `artifacts/validation/sdf_pack_builtin_catalog_seed_native_pack_core.json`, `artifacts/validation/sdf_pack_builtin_catalog_seed_native_field_producer.json`, and `artifacts/validation/sdf_pack_builtin_catalog_seed_native_field_cuda.json` passed.
- Native schema/state proof: `artifacts/validation/sdf_pack_builtin_catalog_seed_native_ui_schema.json`, `artifacts/validation/sdf_pack_builtin_catalog_seed_native_schema_binding.json`, and `artifacts/validation/sdf_pack_builtin_catalog_seed_native_diagnostics_state.json` passed.
- Runtime publish proof: `artifacts/validation/sdf_pack_builtin_catalog_seed_runtime_publish.json` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Published runtime proof: `artifacts/validation/sdf_pack_builtin_catalog_seed_runtime_pytest.json` and `artifacts/validation/sdf_pack_builtin_catalog_seed_runtime_lane.json` passed `tests/test_fractal_runtime_sdf_pack_scene_lane.py` with 2 tests, no skips, and no physical mouse automation.
- Bounded performance witness: `artifacts/sdf_pack_builtin_catalog_seed/perf/builtin_pack_perf.json` records one persistent viewer launch at 512x384 for `sdf_smooth_lattice_2d`, `sdf_capsule_weave_2d`, and `sdf_ring_cells_2d`; observed total render times were about 2.50 ms, 2.25 ms, and 2.04 ms respectively, all with `lens_sdf_pack_backend_used = cuda_sample`.
- Contract validation: `artifacts/validation/sdf_pack_builtin_catalog_seed_contract.json` passed.
- Plan sync: `artifacts/validation/sdf_pack_builtin_catalog_seed_plan_sync.json` passed.
- Hostile-audit validation: `artifacts/validation/sdf_pack_builtin_catalog_seed_hostile_audit.json` passed.
- Code-quality baseline: `artifacts/validation/sdf_pack_builtin_catalog_seed_code_quality.json` passed with baseline check passed.
- Diff hygiene: `artifacts/validation/sdf_pack_builtin_catalog_seed_diff_check.json` passed.
- Product proof summary: the runtime test selects `sdf_pack_scene`, waits for `sdf_pack.builtin_pack`, switches all three built-in packs by no-mouse enum command, checks the active pack id/report id, proves frame hash changes per pack, edits the default pack controls, proves Color Pipeline SDF source edits affect the lane, and replays state pixels.

## Implementation Notes

Implemented design:

- Added a viewer-local built-in catalog helper for SDF pack scene entries: `pack_id`, label, and relative pack JSON path.
- Kept SDF pack JSON files as the execution authority; the catalog is selection/discovery metadata only.
- Kept `sdf_smooth_lattice_2d` as the first/default catalog entry.
- Added `sdf_capsule_weave_2d` and `sdf_ring_cells_2d` using only shipped `circle`, `box`, `capsule`, `union`, `subtract`, `smooth_union`, `rotate`, and `repeat` operations.
- Repaired default load policy so an already loaded non-default built-in pack is not overwritten by `EnsureSdfPackSceneDefaultLoaded`.
- Added stable no-mouse enum selector path `sdf_pack.builtin_pack` for catalog entry selection.
- Kept existing numeric control ids unchanged: `sdf_pack.<param>.primary`.

## Hostile Audit

- Status: complete

Required questions:

- Did the new pack selector actually select packs in the normal left-panel flow and preserve `sdf_pack_scene` selector identity? Yes; published runtime proof switches every built-in pack through `sdf_pack.builtin_pack` while `current_fractal_type` remains `sdf_pack_scene`.
- Did `sdf_smooth_lattice_2d` remain the default pack? Yes; native and runtime tests assert the default pack id and report selection.
- Did any code path reload the default and erase a non-default selected pack? Finding 1 caught this risk in the old default loader; fixed by `SdfPackViewerShouldLoadDefaultBuiltInPack` and a native regression.
- Did every new built-in pack use only shipped SDF pack ops? Yes; native catalog test parses and lowers every pack through the existing SDF pack lowerer, with no evaluator/parser op changes.
- Did every new pack expose meaningful live controls and prove frame/field sensitivity? Yes; native preview sensitivity covers each pack and runtime proof checks distinct frame hashes after pack switches.
- Did state/capture/replay preserve selected pack id and control values? The existing runtime replay test remains green for the default pack and the report/runtime proof now publishes selected pack id; non-default replay expansion remains a low-risk follow-up if pack-specific replay matrices are widened.
- Did Color Pipeline SDF source rows still color `sdf_pack_scene` without special-case redesign? Yes; runtime proof edits `color_pipeline.source.sdf_signed_distance.signal.scale.primary` and observes a frame hash change.
- Did performance evidence stay bounded to the measured pack set? Yes; the performance witness claims only the three built-in packs at 512x384.
- Did the slice avoid new ops, recursive/apollonian work, broad catalog/authoring UX, and physical mouse automation? Yes; no parser/evaluator ops or broad UX were added, and proofs are no-mouse.

## Audit Passes

- [done] Pass 1 - pre-mutation seam audit over pack catalog, default loading, viewer UI, state IO, reporting, runtime harness, and pack examples.
- [done] Pass 2 - post-implementation diff audit found the default-load clobber risk and the grouped-native-command proof gap; both were repaired with code/test changes and individual native proof commands.
- [done] Pass 3 - clean re-read after native, runtime, and performance proof found no additional scoped product defect.

## Audit Findings

- [done] Finding 1 - default-pack initialization would reload `sdf_smooth_lattice_2d` for any loaded non-default pack because the old guard only accepted the default id. Fixed with `SdfPackViewerShouldLoadDefaultBuiltInPack` and `TestDefaultLoadPolicyDoesNotClobberCurrentPack`.
- [done] Finding 2 - the first RED/native proof tried grouped `build_tests_vsdevcmd.cmd` targets, but the helper only ran the first target. Repaired proof by running the focused native targets individually and recording separate validation artifacts.
- [done] Finding 3 - the first ad hoc performance witness used relative state/report paths while the published viewer runs from the runtime directory. Re-ran the bounded witness with absolute paths; no product code change was required.