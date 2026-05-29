# SDF Drag Camera Mapping Repair

## Current Phase

Phase 5 - complete; checkpoint commit, receipts, rearward review, push, and clean-tree closeout are handled by the slice closure flow.

## Phase Checklist

- [x] Phase 1 - create and lock this checked-in plan/contract from clean `d78bdf9` on `codex/sdf-pack-builtin-catalog-seed`.
- [x] Phase 2 - locate the SDF-specific coordinate mapping seam and add RED proof without physical mouse automation.
- [x] Phase 3 - repair SDF source mapping while preserving the normal fractal drag contract.
- [x] Phase 4 - run focused native and published runtime rails for viewport interaction, SDF field generation, and SDF pack scene preservation.
- [x] Phase 5 - hostile audit, validation receipts, checkpoint, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [done] Fix mouse dragging being wrong in SDF-based sources, including the reported vertical flip.
- [done] Treat this as a UX test harness gap and close the gap with deterministic proof.
- [done] Preserve normal non-SDF viewport drag semantics.
- [done] Do not use physical mouse automation.

## Scope

In scope:

- SDF pack scene and authored SDF field source camera/world mapping.
- Deterministic no-mouse or native UX proof for SDF source drag/camera orientation.
- Existing runtime report/test harness additions only if needed to prove this path.

Out of scope:

- New SDF pack operations, new fractal families, authored pack catalog UX, or Color Pipeline redesign.
- General mouse/input subsystem rewrite.
- Physical OS cursor or mouse-stealing tests.
- Changing the existing normal fractal viewport drag contract unless current code proves that contract is wrong.

## Proof Ledger

- Bootstrap authority: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` reported branch `codex/sdf-pack-builtin-catalog-seed`, head `d78bdf9`, clean state, and active prior contract `sdf_pack_builtin_catalog_seed`.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported no staged, unstaged, or untracked files before this repair plan opened.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `ok` for `d78bdf9` before this repair slice opened product mutation.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "SDF drag camera mapping repair" --profile runtime --plan docs/notes/sdf_drag_camera_mapping_repair_PHASED_PLAN.md --contract docs/contracts/sdf_drag_camera_mapping_repair.contract.json` appended checkpoint token `ck:9cbe7472` and locked `global_active_contract`.
- Pre-edit finding: `ui_app/tests/test_viewport_interaction.cpp` already proves the shared normal drag contract: dragging down decreases `center_hp_y`.
- RED proof: `artifacts/validation/sdf_drag_camera_mapping_repair_red_sdf_pack_field.json` failed before the fix with positive world Y appearing in the upper half and normal drag-down center movement moving authored SDF content upward.
- Fix: `PixelToWorldY(...)` in both `ui_app/src/sdf_pack_field_producer.cpp` and `ui_app/src/sdf_pack_field_producer_cuda.cu` now matches the normal CUDA renderer screen-to-world Y mapping.
- Native SDF CPU proof: `artifacts/validation/sdf_drag_camera_mapping_repair_native_sdf_pack_field.json` passed after the fix with 62 assertions.
- Native SDF CUDA proof: `artifacts/validation/sdf_drag_camera_mapping_repair_native_sdf_pack_cuda.json` passed after the fix with 605 assertions.
- Native viewport preservation proof: `artifacts/validation/sdf_drag_camera_mapping_repair_native_viewport.json` passed after adding the missing focused target, with 62 assertions and the existing drag-down contract still green.
- Native automation report proof: `artifacts/validation/sdf_drag_camera_mapping_repair_native_report.json` passed after adding camera fields to the no-mouse report.
- Runtime publish proof: `artifacts/validation/sdf_drag_camera_mapping_repair_runtime_publish.json` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Published no-mouse runtime proof: `artifacts/validation/sdf_drag_camera_mapping_repair_runtime_lane.json` passed `tests/test_fractal_runtime_sdf_pack_scene_lane.py`; the persistent viewer now pans the `sdf_pack_scene` viewport via `pan_viewport_pixels`, proves `view_center_hp_y` decreases for drag-down, and proves the rendered frame hash changes without OS mouse input.
- Contract validation: `artifacts/validation/sdf_drag_camera_mapping_repair_contract.json` passed.
- Plan sync: `artifacts/validation/sdf_drag_camera_mapping_repair_plan_sync.json` passed.
- Hostile-audit validation: `artifacts/validation/sdf_drag_camera_mapping_repair_hostile_audit.json` passed with three recorded findings.
- Code-quality baseline: `artifacts/validation/sdf_drag_camera_mapping_repair_code_quality.json` passed with baseline check passed.
- Diff hygiene: `artifacts/validation/sdf_drag_camera_mapping_repair_diff_check.json` passed.

## Implementation Notes

- Preserved the existing `ComputeDragPan` / `ApplyDragPanStep` contract.
- Repaired only authored SDF pack field world-Y sampling; mask-derived Lens SDF stays image-space and still follows the base fractal mask.
- Added the narrow `pan_viewport_pixels` command to the existing in-process command JSON automation path.
- Added `view_center_hp_x`, `view_center_hp_y`, and `view_log2_zoom` to the runtime automation report so camera movement is machine-verifiable.
- Added a focused `test_viewport_interaction` build target so this drag contract can be rerun without a broad native suite.

## Hostile Audit

- Status: complete

Required questions:

- Did I prove the reported SDF vertical flip on a SDF-based source, not only normal viewport math? Yes; the RED and fixed CPU/CUDA SDF field tests use an asymmetric authored SDF pack and the runtime proof pans `sdf_pack_scene` no-mouse.
- Did I preserve existing normal fractal drag semantics? Yes; `test_viewport_interaction` remains green and was made focused-runnable.
- Did I avoid physical mouse automation? Yes; the runtime proof uses command-file `pan_viewport_pixels`, not OS cursor input.
- Did I add proof that would have caught this before the user saw it? Yes; native CPU/CUDA authored SDF orientation tests and persistent runtime pan proof now cover the bug class.
- Did I avoid unrelated SDF composition, pack ops, or Color Pipeline redesign? Yes; no new SDF ops, packs, palettes, or row semantics changed.
- Did I checkpoint, write receipts, rearward-review, push, and stop with a clean tree? Yes; this plan is synced for the closure flow that commits, writes receipts, rearward-reviews, pushes, and verifies the clean tree.

## Audit Passes

- [done] Pass 1 - pre-mutation seam audit over viewport drag, SDF field region construction, SDF pack CPU/CUDA field sampling, and runtime no-mouse harness capability.
- [done] Pass 2 - post-fix diff audit found missing focused viewport native target and missing no-mouse viewport-pan runtime command; both were repaired.
- [done] Pass 3 - clean re-read after focused native and runtime proof found no additional scoped product defect.

## Audit Findings

- [done] Finding 1 - authored SDF pack field generation used the opposite Y screen-to-world mapping from `RenderFractalCUDA`, flipping SDF content vertically relative to normal camera drag. Fixed in CPU and CUDA field producers and covered with asymmetric SDF field tests.
- [done] Finding 2 - `test_viewport_interaction` existed but could not be run as a focused build target, which made the narrow drag contract awkward to prove. Added the focused target in `ui_app/build_tests_vsdevcmd.cmd`.
- [done] Finding 3 - the persistent runtime harness had set/click/enum commands but no no-mouse viewport pan command, so this product path could not be proven without OS mouse input. Added `pan_viewport_pixels` and runtime proof for `sdf_pack_scene`.
