# Hard Denial Workflow Lock And FITS Re-Closure

## Current Phase

Complete

## Phase Checklist

- [x] Phase 1 - wire active contract state, mutation denial, and strict banner surfaces
- [x] Phase 2 - add contract-proof receipts, validators, and wrapper-based closure flow
- [x] Phase 3 - re-baseline the FITS defaults under the enforced contract and repair focused regressions
- [x] Phase 4 - hostile-audit the hardened workflow and checkpoint only after the required proof rails are green

## Notes

- Trigger:
  - repeated user-reported process drift and false closure on the FITS runtime-walk feature
- Required closure posture for this initiative:
  - mutation must be denied without an active locked contract
  - raw `apply_patch` must be denied
  - raw mutating shell must be denied
  - the repo must emit a strict banner on every prompt and every tool event
  - FITS runtime-walk remains reopened until the default warp path is gone and the contract validators pass
- Hostile-review repairs landed in the dirty slice so far:
  - fixed a `PreToolUse` early-return bug that skipped mutation denial for non-`task_complete` tools entirely
  - removed the stale `transport_warp_scale = 0.10` defaults from the runtime-walk request/import seams so the shipped FITS path no longer leaks warp motion by default
  - updated the native bootstrap helper test so the default mapping now proves warp stays neutral instead of expecting the old unsafe binding
  - corrected repo memory drift in `AGENTS.md`, `AGENT_WORKING_PROTOCOL.md`, `.github/copilot-instructions.md`, and `docs/PHASED_PLAN_CONTINUITY_PROTOCOL.md` so they no longer teach the pre-contract workflow
  - tightened `viewer_host_run_repo_mutation.py` so it can only delegate to approved repo wrappers, not arbitrary commands
  - moved locked-contract hash validation into the wrappers themselves so direct wrapper invocation cannot bypass contract drift protection
  - blocked shell-command chaining/redirection on the raw-shell allowlist so an allowed build/test prefix cannot smuggle a later mutating command
  - replaced freeform `required_acceptance_assertions` strings with structured machine-proof specs in both checked-in contracts
  - taught validation receipts to record parseable evidence entries instead of just raw command text
  - rewrote `viewer_host_write_contract_proof_receipt.py` to evaluate every required assertion individually against JUnit XML and validator JSON artifacts
  - hardened closure denial so a proof receipt must match the locked contract hash and carry an `ok=true` result for every required assertion id
  - hostile audit then found and repaired two additional proof-surface bugs on the repaired state:
    - validation artifacts were not checked against the hash recorded in the validation receipt, so proof could have been recomputed from drifted artifacts
    - required validation commands were still accepted by substring match and did not require a parseable evidence entry of their own
- Current stop point:
  - the per-assertion machine-proof closure model is implemented and validated; remaining work is only the normal checkpoint/handoff/receipt closure chain for this slice
- Required proof rails for this initiative:
  - `py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py -q --junitxml artifacts/pytest/test_viewer_host_checkpoint_guard.junit.xml`
  - `py -3.14 -m pytest tests/test_agent_workflow_tools.py -q --junitxml artifacts/pytest/test_agent_workflow_tools.junit.xml`
  - `py -3.14 -m pytest tests/test_viewer_host_contract_tools.py -q --junitxml artifacts/pytest/test_viewer_host_contract_tools.junit.xml`
  - `py -3.14 -m pytest tests/test_viewer_host_contract_proof.py -q --junitxml artifacts/pytest/test_viewer_host_contract_proof.junit.xml`
  - `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/hard_denial_workflow_lock_and_fits_reclosure.contract.json --out-json artifacts/validation/viewer_host_validate_slice_contract.json`
  - `py -3.14 tools/viewer_host_validate_fits_contract.py --contract docs/contracts/runtime_walk_fits.contract.json --out-json artifacts/validation/viewer_host_validate_fits_contract.json`
  - `py -3.14 -m pytest tests/test_fractal_runtime_runtime_walk_viewer.py -q --junitxml artifacts/pytest/test_fractal_runtime_runtime_walk_viewer.junit.xml`
  - `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/per_assertion_machine_proof_code_quality.json`
  - `py -3.14 tools/viewer_host_run_logged_command.py --label per-assertion-build-tests --log artifacts/per_assertion_build_tests.log -- cmd /c ui_app\build_tests_vsdevcmd.cmd`
  - `py -3.14 tools/viewer_host_run_logged_command.py --label per-assertion-runtime-build --log artifacts/per_assertion_runtime_build.log -- cmd /c ui_app\build_vsdevcmd.cmd`
  - `py -3.14 tools/viewer_host_runtime_pytest_lane.py`
