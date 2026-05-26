# Color Pipeline Preset Workflow Truth

## Current Phase

Complete - Color Pipeline preset workflow truth is implemented, validated, hostile-audited, and closed for checkpoint/receipt flow.

## Phase Checklist

- [x] Phase 0 - branch from clean pushed `master` after UI-Salt recipe expansion closed and rearward review returned `ok`
- [x] Phase 1 - create this checked-in phased plan and contract, then lock the active slice
- [x] Phase 2 - sync roadmap truth for shipped UI-Salt backend authority and remaining Color Pipeline preset workflow gaps
- [x] Phase 3 - add RED tests for product-facing preset workflow truth, recipe identity, and implementation-wording leakage
- [x] Phase 4 - implement the smallest preset workflow repair on top of the existing materialized recipe authority and draft/live bridge
- [x] Phase 5 - prove behavior preservation for current Color Pipeline rows, SDF source stacks, capture/replay paths, and UI-Salt recipe authority
- [x] Phase 6 - hostile review, validation, checkpoint, receipts, rearward review, push, and merge-back

## Explicit User Asks

- [x] Start the next step after UI-Salt metadata-backed catalog, compatibility, companion, and recipe expansion seams were merged.
- [x] Stop treating the current Color Pipeline preset/composition UX as done merely because backend metadata authority landed.
- [x] Make the preset workflow truthful and less implementation-leaky without widening into a full UI redesign.
- [x] Preserve Color Pipeline behavior, SDF behavior, capture/replay authority, and existing no-mouse test discipline.
- [x] Keep deferred items deferred: Factorio-style workflow UI, new function library expansion, SDF gates/masks, boundary-masked phase source, Generic Equation Pack productization, Salticid adapter, SDF-native lanes, perturbation, and new fractal work.

## Scope

In scope:

- Sync roadmap/status docs so they say the UI-Salt backend authority is shipped and the remaining near-term gap is product-facing preset workflow truth.
- Add native RED/green coverage around the current preset workflow surface: active recipe identity, recipe list/display metadata, recipe application through existing rows, and unsupported recipe failure.
- Add the smallest product-facing preset workflow surface needed to use the materialized recipes without presenting them as only a draft/live internal bridge.
- Reduce or replace misleading `Draft`/`Live bridge` copy where the UI is showing shipped preset recipes, while keeping technical fail-closed text where a row really is unsupported.
- Preserve the existing Source, Shape, Palette, Grading row model and draft/live apply mechanics as the implementation seam.

Out of scope:

- Full Color Pipeline redesign.
- Factorio-style schedule/workflow layout implementation.
- New Color Pipeline functions, rows, masks, gates, operands, or SDF beauty modes.
- Generic Equation Pack viewport integration or Salticid `sample_fn` adapter.
- Runtime Salticid dependency.
- SDF-native fractal lanes, perturbation zoom, or new fractal families.
- Physical mouse automation.

## Authority Decision

UI-Salt materialized metadata is already the backend authority for static function descriptors, compatibility lookup, companion suggestions, and recipe expansion. This slice does not reopen those seams. It makes the normal user-facing preset workflow align with that authority by giving active recipes a clear product surface and removing stale wording that implies the shipped recipe path is only temporary internal scaffolding.

The existing `ColorPipelineWindowState` draft rows and `ApplyColorPipelineDraftToLiveState(...)` remain the write/apply authority for this slice. Recipes may populate those rows, but this slice must not create a second persisted preset source of truth.

## Proof Ledger

- Start authority: `master` at `d908c54`, clean and pushed; branch `codex/color-pipeline-preset-workflow-truth`; rearward review artifact for `d908c543ac571a2c90f9b9822dae7fa644c2cba5` returned `ok` before branch start.
- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8`, `py -3.14 tools/viewer_host_repo_status.py`, and `py -3.14 tools/viewer_host_rearward_review.py` ran before this plan was created.
- Initial seam read: current window tests already prove recipe expansion can populate draft rows, but the visible window still presents the surface through `Draft Source / Shape / Palette / Grading` and `Live bridge` wording rather than a clean preset workflow.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "Color Pipeline preset workflow truth" --profile runtime --plan docs/notes/color_pipeline_preset_workflow_truth_PHASED_PLAN.md --contract docs/contracts/color_pipeline_preset_workflow_truth.contract.json` succeeded with checkpoint token `ck:564f2b33`.
- Roadmap truth sync: `spec_intake/_STATUS.md`, `DEFERRED_THREADS.md`, `KNOWN_ISSUES.md`, `docs/notes/color_pipeline_composition_preset_ux_review_PHASED_PLAN.md`, and the UI-Salt backend seam plans now distinguish shipped metadata authority from the active preset-workflow product gap.
- RED native: `artifacts/validation/color_pipeline_preset_window_red.json` failed because `BuildColorPipelineRecipeApplyControlId(...)` and `ColorPipelineWindowSupportedPresetSummaryText()` did not exist.
- RED runtime: `artifacts/validation/color_pipeline_preset_runtime_red.json` failed because the published viewer did not expose `color_pipeline.recipe.default_smooth_escape.apply`.
- GREEN native: `artifacts/validation/color_pipeline_preset_window.json` passed with `test_color_pipeline_window: passed=198 failed=0`.
- GREEN core preservation: `artifacts/validation/color_pipeline_preset_core.json` passed with `test_color_pipeline_core: passed=2093 failed=0`.
- Runtime publish: `artifacts/validation/color_pipeline_preset_publish.json` passed and staged `D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe`.
- Runtime preset proof: `artifacts/validation/color_pipeline_preset_runtime.json` passed with `1 passed`; the visible `color_pipeline.recipe.sdf_normal_angle_diagnostic.apply` control applied the preset and changed live rows/frame hash without OS mouse input.
- Runtime preservation: `artifacts/validation/color_pipeline_preset_runtime_preservation.json` passed with `6 passed`, covering UI-Salt contract report, SDF source rows, and the preset workflow proof together.
- Materializer preservation: `artifacts/validation/color_pipeline_preset_materializer_pytest.json` passed with `7 passed`.
- Hostile-review defect repaired: stale visible `Live bridge:` copy remained below the new preset controls, and the backlog priority table had duplicate rank numbers after the roadmap insertion; fixed both and added `tests/test_color_pipeline_window_copy.py`.
- Copy regression: `artifacts/validation/color_pipeline_preset_copy_pytest.json` passed with `1 passed`.
- Contract validation: `artifacts/validation/color_pipeline_preset_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed after repairing an older UI-Salt metadata catalog checklist drift.
- Code-quality baseline: `artifacts/validation/color_pipeline_preset_code_quality.json` passed with score `94/100`.
- Diff check: `artifacts/validation/color_pipeline_preset_diff_check.json` passed.
- Hostile audit validator: `artifacts/validation/color_pipeline_preset_hostile_audit.json` passed after the audit repairs and clean re-read.

## Hostile Audit

- Status: complete
- Required posture: assume this slice accidentally rebrands an unfinished UI as done, hides unsupported combinations, silently resets user-authored source stacks, regresses SDF rows, or expands into a broad Color Pipeline redesign unless tests and diff scope prove otherwise.

## Audit Passes

- [x] Pass 1 - inspected docs/status surfaces after roadmap sync; found and repaired duplicate backlog rank numbers caused by inserting the active preset row.
- [x] Pass 2 - inspected native/window diff and tests; found and repaired stale visible `Live bridge:` copy below the new preset controls, then added a copy regression.
- [x] Pass 3 - clean re-read of the republished runtime proof confirmed the preset controls are visible/clickable, UI-Salt contract authority remains materialized, and SDF source rows still pass without capture/replay regression.

## Audit Findings

- [x] Real finding: roadmap truth insertion duplicated backlog rank `9`; fixed the rank sequence in `DEFERRED_THREADS.md`.
- [x] Real finding: visible summary text still used `Live bridge:` after the preset controls were added; fixed the copy and added `tests/test_color_pipeline_window_copy.py`.
- [x] Clean re-read after repair confirmed no additional real defect in the republished runtime proof or source-copy guard.

## Validation Targets

- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_preset_window --log artifacts/logs/color_pipeline_preset_window.log --out-json artifacts/validation/color_pipeline_preset_window.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_window`
- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_preset_core --log artifacts/logs/color_pipeline_preset_core.log --out-json artifacts/validation/color_pipeline_preset_core.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core`
- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_preset_materializer_pytest --log artifacts/logs/color_pipeline_preset_materializer_pytest.log --out-json artifacts/validation/color_pipeline_preset_materializer_pytest.json --heartbeat-seconds 30 --timeout-seconds 600 -- py -3.14 -m pytest tests/test_ui_salt_materializer.py -q`
- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_preset_copy_pytest --log artifacts/logs/color_pipeline_preset_copy_pytest.log --out-json artifacts/validation/color_pipeline_preset_copy_pytest.json --heartbeat-seconds 30 --timeout-seconds 120 -- py -3.14 -m pytest tests/test_color_pipeline_window_copy.py -q`
- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_preset_publish --log artifacts/logs/color_pipeline_preset_publish.log --out-json artifacts/validation/color_pipeline_preset_publish.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_vsdevcmd.cmd`
- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_preset_runtime_preservation --log artifacts/logs/color_pipeline_preset_runtime_preservation.log --out-json artifacts/validation/color_pipeline_preset_runtime_preservation.json --heartbeat-seconds 30 --timeout-seconds 900 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_ui_salt_contract.py tests/test_fractal_runtime_color_pipeline_sdf_rows.py tests/test_fractal_runtime_color_pipeline_presets.py`
- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/color_pipeline_preset_workflow_truth.contract.json --out-json artifacts/validation/color_pipeline_preset_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/color_pipeline_preset_workflow_truth_PHASED_PLAN.md --out-json artifacts/validation/color_pipeline_preset_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/color_pipeline_preset_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_preset_diff_check --log artifacts/logs/color_pipeline_preset_diff_check.log --out-json artifacts/validation/color_pipeline_preset_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`

## Stop Conditions

- Stop if the existing draft/live bridge cannot support product-facing preset application without inventing a second state authority.
- Stop if a proposed UI change requires a broad Color Pipeline redesign or new function library expansion.
- Stop if runtime/no-mouse proof cannot distinguish recipe preset application from the old internal draft-only path.
- Stop if SDF source-stack behavior or capture/replay authority regresses.

## Open Implementation Questions

- Which existing window seam should own the visible preset list: a small recipe selector near the draft rows, or a compact preset apply row above the existing lanes?
- Which automation/report field is the narrowest honest proof that the published viewer exposes the active recipe authority without requiring physical mouse input?
- How much `Draft` wording can be removed safely in this slice while still preserving truthful labels for unsupported custom rows?
