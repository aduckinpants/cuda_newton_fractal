# Hard Denial Workflow Lock And FITS Re-Closure

## Current Phase

Phase 4 - hostile-audit the hardened workflow and checkpoint only after the required proof rails are green

## Phase Checklist

- [x] Phase 1 - wire active contract state, mutation denial, and strict banner surfaces
- [x] Phase 2 - add contract-proof receipts, validators, and wrapper-based closure flow
- [x] Phase 3 - re-baseline the FITS defaults under the enforced contract and repair focused regressions
- [ ] Phase 4 - hostile-audit the hardened workflow and checkpoint only after the required proof rails are green

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
- Remaining bounded gap before claiming the enforcement rewrite is fully airtight:
  - `viewer_host_write_contract_proof_receipt.py` still derives proof from validated command lists plus validators; it does not yet parse per-assertion runtime evidence artifacts for every `required_acceptance_assertions` entry
- Required proof rails for this initiative:
  - `py -3.14 -m pytest tests/test_viewer_host_checkpoint_guard.py -q`
  - `py -3.14 -m pytest tests/test_agent_workflow_tools.py -q`
  - `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/hard_denial_workflow_lock_and_fits_reclosure.contract.json`
  - `py -3.14 tools/viewer_host_validate_fits_contract.py --contract docs/contracts/runtime_walk_fits.contract.json`
  - `py -3.14 -m pytest tests/test_fractal_runtime_runtime_walk_viewer.py -q`
