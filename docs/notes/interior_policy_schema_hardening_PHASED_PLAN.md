# Interior Policy Schema Hardening

## Current Phase

Closed - checkpointed, receipt-backed, and merged to `master`; final git status is the authority for push and clean-tree state.

## Phase Checklist

- [x] Phase 1 - create and lock the checked-in plan/contract
- [x] Phase 2 - prove the public UI/schema surface still advertises the stale muted-anchor model
- [x] Phase 3 - repair the public help text, schema guard, and focused native schema-test target
- [x] Phase 4 - validate schema/native guardrails and hostile-audit the repaired seam
- [x] Phase 5 - checkpoint, receipts, merge-back, push, and clean-tree closeout

## Explicit User Asks

- [closed] Hostile-review the recent work again for gaps like the one that caused the interior-control regression.
- [closed] Resolve found gaps instead of only documenting them.
- [closed] Keep the repair narrow and do not reopen unrelated color-pipeline redesign.
- [closed] Then move forward to the next planned work item after this plan is closed on `master`.

## Scope

In scope:

- Update `Interior Strength` schema help so it matches the current black-to-palette runtime policy.
- Add a schema/unit guard that rejects stale “muted interior anchor” wording.
- Add the missing `test_ui_schema` focused target to the native helper script so this exact schema rail can run directly.
- Preserve the current runtime implementation unless proof finds a new behavior defect.

Out of scope:

- New color controls.
- Palette redesign.
- Fractal formula changes.
- Runtime harness redesign beyond this focused native-test target.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on `master` at `7718e40`; tree clean/even with origin.
- Hostile review finding: `ui/fractal_binding_surface_v1.ui_schema.json` still says `Interior Strength` controls the live smooth palette instead of a `muted interior anchor`, which contradicts the current black-to-palette policy.
- Repair target: schema help now names the endpoints as pure black at `0` and active Color Pipeline palette/source color at `1`, and `test_ui_schema` rejects the stale muted-anchor phrase.
- Validation preflight finding: `ui_app/build_tests_vsdevcmd.cmd test_ui_schema` was the right focused rail for this slice, but the helper did not expose `test_ui_schema` as a focused target even though it builds/runs that executable in the full suite. This is now repaired as a focused native-harness gap.
- Focused native proof: `py -3.14 tools/viewer_host_run_logged_command.py --label interior_policy_schema_hardening_native_final --log artifacts/logs/interior_policy_schema_hardening_native_final.log --out-json artifacts/validation/interior_policy_schema_hardening_native_final.json --heartbeat-seconds 30 --timeout-seconds 600 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_ui_schema` passed; log tail reports `test_ui_schema: all passed`.
- Contract proof input: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/interior_policy_schema_hardening.contract.json --out-json artifacts/validation/interior_policy_schema_hardening_contract.json` passed.
- Plan sync proof: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Hostile-audit proof: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/interior_policy_schema_hardening_PHASED_PLAN.md --out-json artifacts/validation/interior_policy_schema_hardening_hostile_audit.json` passed with two real findings and clean re-audit evidence.
- Code-quality proof: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/interior_policy_schema_hardening_code_quality.json` passed baseline check.
- Diff proof: `py -3.14 tools/viewer_host_run_logged_command.py --label interior_policy_schema_hardening_diff_check --log artifacts/logs/interior_policy_schema_hardening_diff_check.log --out-json artifacts/validation/interior_policy_schema_hardening_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.
- Runtime publish proof: `py -3.14 tools/viewer_host_run_logged_command.py --label interior_policy_schema_hardening_runtime_publish --log artifacts/logs/interior_policy_schema_hardening_runtime_publish.log --out-json artifacts/validation/interior_policy_schema_hardening_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 1200 -- cmd /c ui_app\build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Published-runtime proof: `py -3.14 tools/viewer_host_run_logged_command.py --label interior_policy_schema_hardening_runtime_proof_final --log artifacts/logs/interior_policy_schema_hardening_runtime_proof_final.log --out-json artifacts/validation/interior_policy_schema_hardening_runtime_proof_final.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 tools\viewer_host_runtime_pytest_lane.py tests\test_fractal_runtime_multibrot_interior_tone.py` passed with `4 passed`.
- Checkpoint commit: `b5a964a` (`Harden interior policy schema contract`) on `codex/interior-policy-schema-hardening`, fast-forward merged to `master`.
- Receipts: validation and contract-proof receipts were written for `b5a964a`.
- Feature branch push: `codex/interior-policy-schema-hardening` pushed to `origin`.

## Hostile Audit

- Status: complete

## Audit Passes

- [x] Pass 1 - proved the public schema text no longer advertises stale muted-anchor semantics and the native test rejects that phrase.
- [x] Pass 2 - found and repaired the missing `test_ui_schema` focused native target, then proved that exact focused rail green.
- [x] Pass 3 - clean re-read of the repaired diff confirmed the slice only changes schema help, the focused schema test, the native target dispatch, and the slice plan/contract.

## Audit Findings

- [x] Real finding - public schema help still described a muted interior anchor after runtime policy changed to black-to-palette.
- [x] Real finding - the native helper full suite built `test_ui_schema`, but the focused target dispatch rejected `test_ui_schema`, blocking direct proof of this exact rail.
- [x] Clean re-read - the repaired state is confirmed by the focused native rail and did not expose another runtime-coloring, palette-redesign, or formula-behavior change.

## Action Hostile Review

- Action ID: interior-policy-schema-hardening
- Suspected failure mode: runtime behavior can be corrected while public UI/help text and tests still describe the previous policy, leaving future agents and users with conflicting authority.
- Correct owner/action: make the UI schema text match the black-to-palette policy, add a native schema test that rejects the old phrase, and ensure the focused schema rail is directly callable.
- Proof surface: `test_ui_schema`, schema/native helper rail, hostile-audit validator, code-quality baseline, diff check.
