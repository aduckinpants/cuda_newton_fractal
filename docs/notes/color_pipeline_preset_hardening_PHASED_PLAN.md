# Color Pipeline Preset Copy Hardening

## Current Phase

Complete - Color Pipeline preset copy hardening is implemented, validated, hostile-audited, and closed on this branch.

## Phase Checklist

- [x] Phase 0 - branch from clean pushed `master` after preset workflow truth closed and rearward review returned `ok`
- [x] Phase 1 - create this checked-in phased plan and contract, then lock the active hardening slice
- [x] Phase 2 - add RED coverage for remaining visible preset-summary implementation wording
- [x] Phase 3 - replace remaining user-facing draft/live-bridge status copy without renaming internal implementation seams
- [x] Phase 4 - prove preset controls, SDF/Color Pipeline behavior, and no-mouse runtime proof still pass
- [x] Phase 5 - hostile review, validation, checkpoint, receipts, rearward review, push, and merge-back if green

## Explicit User Asks

- [x] Review and harden the just-merged Color Pipeline preset workflow before moving on.
- [x] Do not start the next feature while the preset workflow still has visible implementation-wording leakage.
- [x] Keep this hardening bounded to copy/tests and behavior preservation.
- [x] Do not use physical mouse automation.

## Scope

In scope:

- Tighten copy regression coverage so the preset summary path cannot reintroduce visible `Draft ...` status wording or `Live bridge:` wording.
- Replace remaining user-facing status messages in `DescribeColorPipelineDraftApplyState(...)` with product-facing row-stack wording.
- Preserve internal type/function names for the existing draft/live implementation seam.
- Rerun the focused native/window, copy, runtime publish, and no-mouse runtime preset proof rails.

Out of scope:

- New Color Pipeline functions or rows.
- Full preset manager or Factorio-style workflow UI.
- SDF masks/gates, boundary-masked phase source, or broader SDF performance work.
- Generic Equation Pack productization, Salticid adapter, SDF-native lanes, perturbation, or new fractal work.
- Physical mouse automation.

## Authority Decision

This is a hardening repair on the already-shipped preset workflow, not a new feature slice. The existing row editor and `ColorPipelineDraftApplyState` implementation remain internal authority. Only visible status text and regression coverage should change unless the hostile review finds a blocking defect.

## Proof Ledger

- Start authority: `master` at `5a5a1f8`, clean and pushed; rearward review artifact returned `ok`; branch `codex/color-pipeline-preset-hardening`.
- Hostile read finding: the prior copy guard blocked `Draft Source...` and `Live bridge:`, but `DescribeColorPipelineDraftApplyState(...)` still returned visible strings starting with `Draft ...` in the summary path.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "Color Pipeline preset copy hardening" --profile runtime --plan docs/notes/color_pipeline_preset_hardening_PHASED_PLAN.md --contract docs/contracts/color_pipeline_preset_hardening.contract.json` succeeded with checkpoint token `ck:27c96aa3`.

- RED proof: `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_preset_hardening_copy_red --log artifacts/logs/color_pipeline_preset_hardening_copy_red.log --out-json artifacts/validation/color_pipeline_preset_hardening_copy_red.json --heartbeat-seconds 30 --timeout-seconds 120 -- py -3.14 -m pytest tests/test_color_pipeline_window_copy.py -q` failed on `Draft matches the live runtime selection.` as expected.
- Implementation: `DescribeColorPipelineDraftApplyState(...)` now reports `Row stack matches the live runtime selection.` or `Row stack differs from the live runtime selection.` while preserving internal draft/live implementation names.
- GREEN proof: copy pytest passed (`1 passed`), native `test_color_pipeline_window` passed (`passed=198 failed=0`), runtime publish passed, and no-mouse runtime preset proof passed (`1 passed`).
- Guardrail proof: contract validation, phased-plan sync, hostile-audit validation, code-quality baseline, and `git diff --check` passed for this hardening slice.
- Hostile readback proof: `rg -n '"Draft |"Live bridge:|Draft Source|Row stack matches|Row stack differs' ui_app\src\color_pipeline_window.h tests\test_color_pipeline_window_copy.py -S` finds only the test guard plus the new row-stack product copy in the UI source.
- Rearward repair: review artifact `artifacts/hooks/viewer_host_rearward_review/0e7e7a502aa669500b965ccf281e6e7200d25fe3.json` flagged the prior closeout wording as stale; this plan now uses closed-state wording only.

## Hostile Audit

- Status: complete
- Required posture: assume this hardening either misses another visible implementation string, changes internal apply behavior accidentally, weakens runtime preset proof, or widens into broader Color Pipeline redesign unless diff and tests prove otherwise.

## Audit Passes

- [x] Pass 1 - inspect changed UI copy and copy-regression test for the exact implementation wording class found in review.
- [x] Pass 2 - inspect native/window behavior to ensure apply-state status semantics did not change.
- [x] Pass 3 - clean re-read the repaired state, published runtime proof, and source diff for no feature widening or additional workflow mistake.

## Audit Findings

- [x] Finding - the prior preset-copy guard did not catch visible status strings beginning with `Draft ...`; RED coverage now fails that wording class.
- [x] Repair - user-facing status messages now use row-stack wording, with no internal implementation rename.
- [x] Clean re-read - focused native/window, runtime publish, no-mouse runtime preset proof, source readback, contract validation, plan sync, code quality, and diff check confirmed the repaired state and did not expose another feature-widening or behavior regression.

## Validation Targets

- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_preset_hardening_window --log artifacts/logs/color_pipeline_preset_hardening_window.log --out-json artifacts/validation/color_pipeline_preset_hardening_window.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_window`
- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_preset_hardening_copy_pytest --log artifacts/logs/color_pipeline_preset_hardening_copy_pytest.log --out-json artifacts/validation/color_pipeline_preset_hardening_copy_pytest.json --heartbeat-seconds 30 --timeout-seconds 120 -- py -3.14 -m pytest tests/test_color_pipeline_window_copy.py -q`
- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_preset_hardening_publish --log artifacts/logs/color_pipeline_preset_hardening_publish.log --out-json artifacts/validation/color_pipeline_preset_hardening_publish.json --heartbeat-seconds 30 --timeout-seconds 600 -- ui_app/build_vsdevcmd.cmd`
- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_preset_hardening_runtime --log artifacts/logs/color_pipeline_preset_hardening_runtime.log --out-json artifacts/validation/color_pipeline_preset_hardening_runtime.json --heartbeat-seconds 30 --timeout-seconds 300 -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_color_pipeline_presets.py`
- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/color_pipeline_preset_hardening.contract.json --out-json artifacts/validation/color_pipeline_preset_hardening_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/color_pipeline_preset_hardening_PHASED_PLAN.md --out-json artifacts/validation/color_pipeline_preset_hardening_hostile_audit.json`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/color_pipeline_preset_hardening_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label color_pipeline_preset_hardening_diff_check --log artifacts/logs/color_pipeline_preset_hardening_diff_check.log --out-json artifacts/validation/color_pipeline_preset_hardening_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`
