# Diagnostics Capture Output Paths

## Current Phase

Closed - diagnostics capture output paths repaired and validated

## Phase Checklist

- [x] Phase 1 - inspect diagnostics capture path authority and write failing output-path tests
- [x] Phase 2 - implement unique default bundle paths plus explicit `--diagnostics-out-dir`
- [x] Phase 3 - validate focused tests, hostile-audit the touched seams, and repair any finding
- [x] Phase 4 - run required validation rails, checkpoint, receipts, push, and clean-tree closeout

## Explicit User Asks

- [done] Start implementation of the five-step mini-sprint in linear order.
- [done] Implement Step 1: diagnostics capture output paths.
- [done] Stop diagnostic captures from overwriting `runtime/diagnostics/last` as the only output.
- [done] Add deterministic explicit diagnostics output directory support where appropriate.

## Scope

In scope:

- Diagnostic capture output path resolution.
- Default unique timestamp/collision-safe diagnostic bundles.
- Explicit `--diagnostics-out-dir` support for diagnostic capture.
- Compatibility handling for `runtime/diagnostics/last`.
- Focused native/headless tests and docs/help text if exposed to users.

Out of scope:

- Finding capture behavior.
- Render/camera/capture quality changes.
- Lens SDF, selector/view presets, camera/dive, smooth-escape tuning.
- Color Pipeline behavior.
- Physical mouse automation.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on branch `codex/diagnostics-capture-output-paths` at `d7b07ba`.
- Starting state: clean branch from pushed `master`.
- RED tests:
  - `cmd /c ui_app\build_tests_vsdevcmd.cmd test_viewer_cli` failed because `ViewerCliArgs` did not expose `have_diagnostics_out_dir` / `diagnostics_out_dir`.
  - `cmd /c ui_app\build_tests_vsdevcmd.cmd test_diagnostics_capture` failed because `CaptureDiagnosticsLastBundle(...)` still returned `diagnostics/last` and repeated captures overwrote the only durable output directory.
- First GREEN:
  - `cmd /c ui_app\build_tests_vsdevcmd.cmd test_viewer_cli` passed after adding the explicit diagnostics output flag and focused test target.
  - `cmd /c ui_app\build_tests_vsdevcmd.cmd test_diagnostics_capture` passed after default capture began writing a unique archive plus a `diagnostics/last` compatibility mirror.
  - `cmd /c ui_app\build_vsdevcmd.cmd` passed and published `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
  - `py -3.14 -m pytest tests/test_diagnostics_capture.py` passed with no-mouse default archive, explicit output-directory, and invalid-output-directory command proofs.
- Hostile audit: complete; one workflow defect was found and repaired.
- Validation passed:
  - `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/diagnostics_capture_output_paths.contract.json --out-json artifacts/validation/diagnostics_capture_output_paths_contract.json`
  - `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
  - `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/diagnostics_capture_output_paths_PHASED_PLAN.md --out-json artifacts/validation/diagnostics_capture_output_paths_hostile_audit.json`
  - `cmd /c ui_app\build_tests_vsdevcmd.cmd test_viewer_cli`
  - `cmd /c ui_app\build_tests_vsdevcmd.cmd test_diagnostics_capture`
  - `cmd /c ui_app\build_vsdevcmd.cmd`
  - `py -3.14 -m pytest tests/test_diagnostics_capture.py`
  - `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/diagnostics_capture_output_paths_code_quality.json`
  - `py -3.14 tools/viewer_host_run_logged_command.py --label diagnostics_capture_output_paths_diff_check --log artifacts/logs/diagnostics_capture_output_paths_diff_check.log --out-json artifacts/validation/diagnostics_capture_output_paths_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`

## Hostile Audit

- Status: complete

## Audit Passes

- [x] Pass 1 - inspect diff and touched diagnostics seams for path overwrite, fallback, and compatibility bugs.
- [x] Pass 2 - verify the implementation did not touch finding capture, render quality, Color Pipeline, or unrelated capture paths; clean re-read found no additional real defect.
- [x] Pass 3 - verify tests prove default unique output and explicit `--diagnostics-out-dir` behavior instead of only helper internals; confirmed the repaired state with native and published-runtime proof.

## Audit Findings

- [x] Real finding - The initial contract required `cmd /c ui_app\build_tests_vsdevcmd.cmd test_diagnostics_capture`, but the focused build target did not exist. Repaired by adding focused `test_viewer_cli` and `test_diagnostics_capture` targets before relying on the rail.
- [x] Clean re-read - Re-read the repaired diagnostics path code and confirmed default capture returns the unique archive while updating `diagnostics/last` only as compatibility.
- [x] Clean re-read - Re-read the touched CLI/main seams and confirmed explicit output-directory routing is limited to diagnostic capture and does not change finding capture, render quality, Color Pipeline, or physical mouse automation.

## Action Hostile Review

- Action ID: diagnostics-capture-output-paths-red
- Suspected failure mode: The slice could add helper-only path logic but leave the real diagnostic capture command overwriting `runtime/diagnostics/last`.
- Correct owner/action: Add RED coverage against the real diagnostics capture seam before implementation.
- Proof surface: Focused tests proving unique default bundles, explicit output directory behavior, compatibility for `last`, and useful failure on invalid paths.
- Blocked action: Broad capture/render changes, finding capture behavior changes, Color Pipeline changes, or physical mouse automation.

## Notes

- Parent campaign plan: `docs/notes/top_five_backlog_campaign_PHASED_PLAN.md`.
- Step 1 current intent: default diagnostic capture should create durable unique output, while `last` may remain only as a compatibility mirror/pointer.
