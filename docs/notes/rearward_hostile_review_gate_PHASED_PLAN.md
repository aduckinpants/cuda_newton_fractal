# Rearward Hostile Review Gate

## Current Phase

Validated - the workflow-only rearward hostile review gate is implemented, audited, and contract-proof evidence is parseable.

## Phase Checklist

- [x] Phase 1 - create and lock this checked-in plan/contract
- [x] Phase 2 - add RED workflow tests for missing, ok, and needs-repair rearward review states
- [x] Phase 3 - implement the rearward review artifact producer and mutation gate
- [x] Phase 4 - update workflow docs and prove focused workflow rails
- [x] Phase 5 - hostile-audit and final validation closeout

## Explicit User Asks

- [done] Automate the useful "first review and fix previous turn" pattern so it does not need to be repeated every session.
- [done] Make the rearward hostile-review pattern hard-gated rather than advisory.
- [done] Keep read-only exploration available, but block meaningful mutation until the previous committed head has an ok rearward review artifact.
- [done] Preserve the existing checkpoint, receipt, contract, explicit-ask, and hostile-audit guard behavior.

## Scope

In scope:

- Add `tools/viewer_host_rearward_review.py`.
- Add focused workflow tests for the rearward review artifact and mutation gating.
- Gate mutation through existing checkpoint guard surfaces.
- Add a `viewer_host_begin_work_slice.py --rearward-repair-for <head>` escape hatch for repair-only slices.
- Update repo workflow docs to require bootstrap, repo status, rearward review, repair if needed, then new work.

Out of scope:

- GPU Lens SDF implementation.
- SDF roadmap truth repair beyond seeding it as the next dogfood slice.
- Product/runtime viewer behavior.
- Relaxing existing checkpoint, receipt, contract-proof, explicit-ask, or hostile-audit rules.

## Proof Ledger

- Bootstrap authority: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` reported `branch=master`, `HEAD=08378f9`, clean worktree, and active closed contract `sdf_runtime_walk_signals`.
- Repo status authority: `py -3.14 tools/viewer_host_repo_status.py` reported no staged, unstaged, or untracked files before this slice.
- Gap authority: the existing workflow enforces hostile audit before closeout, but it does not hard-block the next implementation turn until the previous committed head has been reread through a rearward review artifact.
- RED authority: `py -3.14 -m pytest tests/test_viewer_host_rearward_review.py tests/test_viewer_host_checkpoint_guard.py -q` failed on the placeholder review tool and old begin-slice guard path, proving the missing artifact and gate behavior.
- GREEN authority: `py -3.14 -m pytest tests/test_viewer_host_rearward_review.py tests/test_viewer_host_checkpoint_guard.py -q --junitxml artifacts/pytest/rearward_hostile_review_gate_workflow.junit.xml` passed with `87 passed` after adding `tools/viewer_host_rearward_review.py`, begin-slice `--rearward-repair-for`, mutation/completion guard checks, and workflow docs.
- Receipt repair authority: first receipt generation exposed that the focused pytest command lacked a machine-parseable JUnit artifact, so the contract and proof ledger now require the same focused rail with `--junitxml`.
- Contract authority: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/rearward_hostile_review_gate.contract.json --out-json artifacts/validation/rearward_hostile_review_gate_contract.json` passed.
- Plan-sync authority: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Hostile-audit authority: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/rearward_hostile_review_gate_PHASED_PLAN.md --out-json artifacts/validation/rearward_hostile_review_gate_hostile_audit.json` passed.
- Code-quality authority: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/rearward_hostile_review_gate_code_quality.json` passed with baseline score `96/100`.
- Diff hygiene authority: `py -3.14 tools/viewer_host_run_logged_command.py --label rearward_hostile_review_gate_diff_check --log artifacts/logs/rearward_hostile_review_gate_diff_check.log --out-json artifacts/validation/rearward_hostile_review_gate_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.

## Hostile Audit

- Status: done
- Required posture: assume the first gate either blocks too much, allows mutation without proof, lets stale repair heads pass as normal work, or weakens existing checkpoint/receipt/contract rules until tests prove otherwise.

## Audit Passes

- [done] Pass 1 - found and repaired a real guard-order defect: the first completion gate preempted the older missing-validation-receipt denial.
- [done] Pass 2 - re-read the guard integration after the focused GREEN rail and confirmed read-only commands still allow while begin-slice mutation requires an ok artifact or exact repair head.
- [done] Pass 3 - re-read the existing checkpoint, receipt, contract, explicit-ask, and hostile-audit closure order and confirmed the rearward completion gate runs only after those blockers are clear.
- [done] Pass 4 - attempted receipt writing and repaired the contract evidence gap when proof generation rejected the non-JUnit pytest command.

## Audit Findings

- [done] Real defect found and repaired: the first completion integration made rearward review denial fire before the established validation-receipt denial, which would have obscured the primary proof blocker.
- [done] Real closeout defect found and repaired: contract-proof receipt generation rejected the focused pytest command because it had no JUnit artifact.
- [done] No additional real defect found after the focused re-audit and validation pass.

## Action Hostile Review

- Action ID: rearward-hostile-review-gate-start
- Suspected failure mode: future sessions can begin new implementation work without first rereading the just-closed previous head, so stale plan text or workflow mistakes survive until the user catches them manually.
- Correct owner/action: add a machine-written rearward review artifact and block mutation until the current clean head has an ok artifact or is being repaired under an explicit repair slice.
- Proof surface: focused workflow tests, contract validation, phased-plan sync, hostile-audit validation, code-quality baseline, diff hygiene, and a dogfood rearward review run against the current head.
- Blocked action: GPU Lens SDF, SDF roadmap repair implementation, live viewer behavior changes, or weakening existing guard rails.
