# Advanced Color Pipeline Follow-Up UX Truth Repair

## Current Phase

Complete - follow-up UX truth repair now matches the shipped advanced-color authority in the bounded editor support surfaces, and the native rails plus hostile audit are clean on this head.

## Phase Checklist

- [x] Phase 1 - add focused REDs that prove shipped candidates can still inherit false `(draft only)` labeling from unrelated unsupported state, and prove the stale Shape and summary copy defects on current HEAD.
- [x] Phase 2 - repair the candidate-label/support-text seams so shipped rows and shipped ordered Shape behavior stop reading as unsupported or single-row-only, while unsupported tuples remain truthfully draft-only.
- [x] Phase 3 - rerun the bounded native rails, complete hostile audit passes, checkpoint, write receipts, and clear the stale-plan gate on the committed head.

## Explicit User Asks

- [done] Follow the bootstrap/contract pattern from the initial kickoff instead of patching ad hoc.
- [done] Address the follow-up hostile-review findings that still create painful UX truth mismatches in the advanced color pipeline editor.
- [done] Keep shipped candidates and shipped Shape behavior from reading as draft-only or single-row-only when repo authority says they are shipped.
- [done] Preserve truthful draft-only behavior for genuinely unsupported tuples or unsupported candidates.
- [done] Stay within the bounded advanced-color editor/runtime bridge seams and avoid unrelated workflow, archive, ExplainO-family, hook, or broad UI work.

## Proof Ledger

- Bootstrap on 2026-05-14 re-proved this repo is on `feature/advanced-color-pipeline-draft-editor-reframe` at clean `HEAD=9176bb6`, with the closed `advanced_color_pipeline_shipped_draft_label_truth_repair` contract still locked and therefore unable to authorize new mutation.
- Launch authority reread before mutation: `AGENTS.md`, `AGENT_WORKING_PROTOCOL.md`, `AGENT_TERMINAL_PROTOCOL.md`, `spec_intake/_STATUS.md`, `DEFERRED_THREADS.md`, `KNOWN_ISSUES.md`, `docs/notes/advanced_color_feature_restart_inventory.md`, `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`, `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md`, `docs/notes/advanced_color_library_foundation_oracle_and_inventory.md`, `docs/notes/advanced_color_library_foundation_phase8g_balance_void_grade_owner_proof_PHASED_PLAN.md`, `docs/notes/advanced_color_pipeline_active_drag_and_palette_grading_preserve_PHASED_PLAN.md`, `docs/notes/advanced_color_pipeline_live_drag_runtime_restore_PHASED_PLAN.md`, `docs/notes/advanced_color_closed_slice_truth_repair_PHASED_PLAN.md`, and the relevant `HANDOFF_LOG.md` entries for `ck:1ba27a24`, `ck:ba348492`, `ck:e5d03961`, and `ck:0264d37b`.
- Bootstrap repo proof: branch `feature/advanced-color-pipeline-draft-editor-reframe`, `HEAD=9176bb62544985e9c47f9832052c1dcd04757838`, clean tree, active locked contract `advanced_color_pipeline_shipped_draft_label_truth_repair`, and no dirty carryover state.
- Review proof on current HEAD found four bounded UX truth defects: candidate labels still derive from full-draft apply state rather than shipped-row truth, Shape row help still says only one live-backed Shape row participates, unsupported Shape error text still says stacked Shape recipes stay draft-only, and the window summary still describes three lane stacks / Source-Shape-Palette-only drafting even though the shipped editor has four active lanes including Grading.
- First RED proof: `py -3.14 tools/viewer_host_run_logged_command.py --label advanced_color_followup_ux_red --log artifacts/advanced_color_followup_ux_red.log -- cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_red` failed on the new helper seams in `test_color_pipeline_window.cpp`, proving the candidate-badge and summary/help truth repairs were not yet implemented on current HEAD.
- Implemented bounded repair in `ui_app/src/color_pipeline_window.h`: extracted testable summary/help/error text helpers, replaced the stale Shape error/help and three-lane summary copy with shipped ordered-Shape / four-lane text, and added `ShouldColorPipelineCandidateUseDraftOnlyLabel(...)` so shipped runtime-backed candidates stop inheriting a false `(draft only)` badge from unrelated unsupported draft state while genuinely unsupported candidate selections still badge as draft-only.
- Native proof after the repair is green:
  - `py -3.14 tools/viewer_host_run_logged_command.py --label advanced_color_followup_ux_red_rerun --log artifacts/advanced_color_followup_ux_red_rerun.log -- cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_red`
  - `py -3.14 tools/viewer_host_run_logged_command.py --label advanced_color_followup_ux_owner --log artifacts/advanced_color_followup_ux_owner.log -- cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner`
- Contract and plan scaffolding are green on the repaired head:
  - `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/advanced_color_pipeline_followup_ux_truth_repair.contract.json --out-json artifacts/validation/advanced_color_pipeline_followup_ux_truth_repair_contract.json`
  - `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- Remaining closeout proof for this slice is limited to checkpoint, receipts, and the post-closeout stale-plan gate; no broader or runtime witness is required for this bounded editor support-state/text seam unless a later rail disproves that assumption.

## Hostile Audit

- Status: complete
- Required posture: assume the remaining UX truth defects are deeper than copy until the candidate-label branch, help/error text, summary surfaces, and regression coverage all agree on the repaired head.

## Audit Passes

- [x] Pass 1 - RED-first hostile review complete. The new helper regressions failed on current HEAD because the candidate-badge truth helper and the summary/help string helpers did not exist yet, proving the reviewed UX defects were real and still uncovered.
- [x] Pass 2 - clean re-read of the repaired diff complete. The repaired state now routes both stale Shape error sites through one shared ordered-Shape message, updates the summary to four editor lanes including Grading, and keeps unsupported candidate selections draft-only while suppressing only the false inherited badge on shipped runtime-backed candidates.
- [x] Pass 3 - second clean re-read on the repaired head after the owner rail and stale-string grep found no additional real defect, no remaining single-row Shape lie, and no remaining three-lane summary lie in the bounded seam.

## Audit Findings

- [x] Real support-truth defect found: combo labels were using full-draft candidate apply state directly, so a shipped runtime-backed candidate could still inherit a false `(draft only)` badge from unrelated unsupported draft state.
- [x] Real copy-truth defects found: the Shape help text still claimed only one live-backed Shape row participated, the unsupported Shape message still claimed stacked Shape recipes stayed draft-only, and the summary still described a three-lane Source / Shape / Palette-only draft surface.
- [x] Bounded repair confirmed: the fix is not a cosmetic string hide. Unsupported candidate selections still receive `(draft only)` in supported contexts, while only the false inherited shipped badge is suppressed and the stale shipped-behavior text now matches the ordered-Shape and four-lane editor authority.

## Notes

- Expected owner files for this slice:
  - `docs/contracts/advanced_color_pipeline_followup_ux_truth_repair.contract.json`
  - `docs/notes/advanced_color_pipeline_followup_ux_truth_repair_PHASED_PLAN.md`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/src/color_pipeline_core.h`
  - `ui_app/tests/test_color_pipeline_window.cpp`
  - `ui_app/tests/test_schema_binding.cpp`
  - `tests/test_fractal_runtime_explaino_escape_variants.py`
  - `HANDOFF_LOG.md`
- Known non-blocking bootstrap friction: `viewer_host_session_bootstrap.py --audit` still reports the pre-existing code-quality baseline regression in `ui_app/src/escape_time_coloring.h`; this slice does not claim to repair that unrelated baseline debt.
- Exit criteria:
  - shipped candidates no longer inherit false `(draft only)` labels from unrelated unsupported rows
  - shipped ordered Shape behavior no longer reads as single-row-only or stacked-draft-only in the editor surfaces touched here
  - unsupported candidates remain truthfully draft-only
  - bounded native rails, validators, checkpoint, receipts, hostile audit, and stale-plan gate all pass on the committed head
