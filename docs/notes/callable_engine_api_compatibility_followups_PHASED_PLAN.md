# Callable Engine API Compatibility Follow-Ups

## Current Phase

Complete - deferred API compatibility follow-ups are documented, parked behind the active SDF work, and no product/API behavior changed.

## Phase Checklist

- [x] Phase 0 - confirm clean repo state and rearward review `ok`.
- [x] Phase 1 - perform one more read-only review of the viewer-side callable/sample API seams.
- [x] Phase 2 - cross-check the Salticid `sample_fn` operator surface enough to make concrete resync recommendations.
- [x] Phase 3 - write the deferred follow-up note and backlog pointer.
- [x] Phase 4 - validate plan/contract, hostile audit, diff check, checkpoint, receipts, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [closed] Write the callable/sample API review down while it is fresh.
- [closed] Keep all follow-ups deferred behind SDF work; nothing here should preempt the SDF thread.
- [closed] Include a sixth item with concrete Salticid operator-surface contract recommendations for later resync work.

## Scope

In scope:

- Documentation-only note for low-hanging callable/sample API compatibility follow-ups.
- Deferred backlog pointer.
- Salticid-side resync recommendations based on read-only review.

Out of scope:

- Runtime behavior changes.
- SDF behavior changes.
- Salticid repo edits.
- API contract implementation.
- Test additions beyond documentation validation.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on clean `codex/sdf-measurement-replan-20260611` at `322ff14`.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported clean tree.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `ok` for `322ff14`.
- Read-only viewer review inspected `fractal_probe_contract.cpp`, `fractal_probe_runner.cpp`, `function_descriptor.cpp`, callable-engine docs, and runtime tests.
- Read-only Salticid review inspected `sample_fn` docs/tests and `lower_sample_fn(...)` behavior.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "defer callable engine API compatibility follow-ups" --profile native --plan docs/notes/callable_engine_api_compatibility_followups_PHASED_PLAN.md --contract docs/contracts/callable_engine_api_compatibility_followups.contract.json` appended `ck:15f26bfe` and locked the active contract.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/callable_engine_api_compatibility_followups.contract.json --out-json artifacts/validation/callable_engine_api_compatibility_followups_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Hostile-audit validation: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/callable_engine_api_compatibility_followups_PHASED_PLAN.md --out-json artifacts/validation/callable_engine_api_compatibility_followups_hostile_audit.json` passed.
- Code-quality baseline: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/callable_engine_api_compatibility_followups_code_quality.json` passed.
- Diff check: `py -3.14 tools/viewer_host_run_logged_command.py --label callable_engine_api_compatibility_followups_diff_check --log artifacts/logs/callable_engine_api_compatibility_followups_diff_check.log --out-json artifacts/validation/callable_engine_api_compatibility_followups_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.

## Hostile Audit

- Status: complete

Questions:

- Did this slice change only documentation?
- Did it clearly defer all API work behind SDF?
- Did it separate viewer-side issues from Salticid-side resync recommendations?
- Did it avoid claiming this repo is the root cause of the active Salticid crash?
- Did it avoid touching runtime code, SDF behavior, or Salticid files?

## Audit Passes

- [closed] Pass 1 - challenged whether any finding was urgent enough to interrupt SDF; none were.
- [closed] Pass 2 - challenged whether the Salticid recommendations were repo-grounded; read-only Salticid review found concrete `sample_fn` default and generic-sample metadata seams.
- [closed] Pass 3 - clean re-read confirmed this is a deferred documentation slice only.

## Audit Findings

- [closed] Finding 1 - The viewer docs already tell callers to set `fractal.view.fractal_type` explicitly, so the default mismatch is not an immediate production stop-line. The deferred note records it as descriptor/default hardening rather than an urgent bug.
