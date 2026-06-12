# Progress Visibility Protocol

## Current Phase

Closed - workflow rule update implemented and ready for checkpoint validation.

## Phase Checklist

- [x] Phase 0 - bootstrap, repo status, rearward review, and fresh branch from `master`.
- [x] Phase 1 - create a bounded workflow-only plan and contract.
- [x] Phase 2 - update repo rules for detailed next-step summaries, preplanned-work exhaustion notices, and better moment-to-moment progress.
- [x] Phase 3 - validate contract, plan sync, hostile audit, code quality, rule-text proof, and diff check before checkpoint.

## Explicit User Asks

- [x] Make future next steps more detailed and visible.
- [x] Require a note when preplanned sliced work is exhausted.
- [x] Put the rule in the repo workflow docs, not only in chat history.
- [x] Improve moment-to-moment progress insight.

## Scope

In scope:

- `AGENTS.md` concise workflow rule.
- `AGENT_WORKING_PROTOCOL.md` detailed workflow rule.
- This plan and the matching contract.

Out of scope:

- Product/runtime behavior changes.
- Hook/tool implementation changes.
- SDF, ExplainO, Color Pipeline, or analyzer changes.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on clean branch `codex/explaino-root-slime-trace-campaign` at `c18a28e`.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported clean tree.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `ok` for `c18a28e`.
- Branch: switched from `master` to `codex/progress-visibility-protocol`; rearward review returned `ok` for `edf2bf5`.
- Contract lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "progress visibility and preplanned-work exhaustion repo rule" --profile native --plan docs/notes/progress_visibility_protocol_PHASED_PLAN.md --contract docs/contracts/progress_visibility_protocol.contract.json` opened checkpoint `ck:9dcdfe2f`.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/progress_visibility_protocol.contract.json --out-json artifacts/validation/progress_visibility_protocol_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Hostile audit validation: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/progress_visibility_protocol_PHASED_PLAN.md --out-json artifacts/validation/progress_visibility_protocol_hostile_audit.json` passed.
- Code-quality baseline: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/progress_visibility_protocol_code_quality.json` passed with baseline score 93/100.
- Rule-text proof: `py -3.14 tools/viewer_host_run_logged_command.py --label progress_visibility_protocol_rule_text --log artifacts/logs/progress_visibility_protocol_rule_text.log --out-json artifacts/validation/progress_visibility_protocol_rule_text.json --heartbeat-seconds 30 --timeout-seconds 120 -- rg -n "Progress Visibility Rule|Preplanned Work Exhaustion" AGENTS.md AGENT_WORKING_PROTOCOL.md` passed with exit code 0.
- Diff check: `py -3.14 tools/viewer_host_run_logged_command.py --label progress_visibility_protocol_diff_check --log artifacts/logs/progress_visibility_protocol_diff_check.log --out-json artifacts/validation/progress_visibility_protocol_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed with exit code 0.

## Hostile Audit

- Status: complete

Questions:

- Did the repo rules explicitly require detailed next steps instead of vague closeouts? Yes.
- Did the repo rules explicitly require saying when preplanned sliced work is exhausted? Yes.
- Did the repo rules cover moment-to-moment progress, not only final summaries? Yes.
- Did this slice avoid product behavior changes? Yes.

## Audit Passes

- [x] Pass 1 - found the workflow gap: existing rules required plans/handoffs but did not require explicit next-step detail or exhaustion notices.
- [x] Pass 2 - re-read the repaired state and no additional real issue found in the bounded docs-only scope.
- [x] Pass 3 - confirmed the repaired state with rule-text proof and no additional workflow mistake found.

## Audit Findings

- [x] Workflow gap: closeouts and progress updates could omit detailed next steps or fail to say when preplanned sliced work was exhausted.
- [x] Clean re-read the repaired state; no additional real issue found in this docs-only slice.
