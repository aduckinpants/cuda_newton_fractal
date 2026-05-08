# Checkpoint Guard Crash Recovery Playbook

## Purpose

This repo's checkpoint guard is intentionally fail-closed. If VS Code, the extension host, or the chat session crashes while the repo is dirty, the guard should still prevent the next session from pretending the stranded slice is a fresh clean start. The missing piece this playbook covers is how to diagnose that exact state and resume it safely.

## Failure Signature

Expect this playbook when all of the following are true:

- `UserPromptSubmit` or `SessionStart` says the repo is dirty but the session has no checkpoint baseline.
- The hook output tells you to run `py -3.14 tools/viewer_host_recover_crash_state.py --summary "<operator note>" --adopt-current-state`.
- The repo was already dirty before the new session started, usually because VS Code crashed, the extension host rolled back, or the chat thread was replaced mid-slice.

Typical prompt-hook message shape:

- crash recovery required
- repository dirty
- session has no checkpoint baseline
- inspect `artifacts/hooks/viewer_host_checkpoint_guard/recovery/`
- retry only after explicit recovery adoption

## Root-Cause Sequence

The normal bad path is:

1. A slice begins and dirties the repo.
2. VS Code, the extension host, or the chat session dies before the slice is checkpointed or the session baseline can be reused.
3. A new session starts against the same dirty repo, but there is no matching baseline file for the new session id.
4. `UserPromptSubmit` sees `dirty repo + missing baseline` and now blocks with an explicit crash-recovery instruction instead of silently carrying on.

This is not the same as a normal dirty-carryover prompt block. The important difference is that the current session cannot prove where its baseline should have been, so explicit operator adoption is required before resume.

## Exact Recovery Flow

Run the helper from the repo root:

```text
py -3.14 tools/viewer_host_recover_crash_state.py --summary "<operator note>" --adopt-current-state
```

What the helper does:

- captures the current dirty snapshot
- writes a durable report under `artifacts/hooks/viewer_host_checkpoint_guard/recovery/`
- records current branch, `HEAD`, dirty-path summary, active contract state, latest handoff context, and receipt state
- splits changed paths into `in_contract_scope` and `out_of_contract_scope`
- writes `artifacts/hooks/viewer_host_checkpoint_guard/recovery/active_recovery_adoption.json` only if the diagnosed snapshot still matches the current repo state

What to do next:

1. Read the new recovery report JSON in `artifacts/hooks/viewer_host_checkpoint_guard/recovery/`.
2. Confirm the dirty files and active contract still describe the stranded slice you intend to resume.
3. Start a fresh prompt or fresh chat session in the same repo.
4. Let the hook materialize a new session baseline from the adopted dirty snapshot.
5. Continue the stranded slice in crash-safe mode: one bounded command at a time, no parallel terminal work, and prefer logged-command wrappers for large output.

Important constraints:

- Adoption is explicit and one-shot. The hook consumes `active_recovery_adoption.json` after it materializes the new session baseline.
- If the repo changes between diagnosis and adoption, the helper refuses adoption and you must rerun it.
- Recovery adoption authorizes resume of the stranded slice only. It is not permission to treat the repo as freshly clean or to skip normal checkpoint, validation, receipt, or handoff closure.

## Artifact Locations

- Baselines: `artifacts/hooks/viewer_host_checkpoint_guard/`
- Recovery reports: `artifacts/hooks/viewer_host_checkpoint_guard/recovery/recovery_<timestamp>_<digest>.json`
- Active one-shot adoption artifact: `artifacts/hooks/viewer_host_checkpoint_guard/recovery/active_recovery_adoption.json`
- Validation receipts: `artifacts/hooks/viewer_host_validation_receipts/<head>.json`
- Contract-proof receipts: `artifacts/contract_proof_receipts/<head>.json`
- Active locked contract state: `artifacts/hooks/viewer_host_contract_guard/global_active_contract.json`

## How To Tell Similar Failures Apart

### Missing baseline after crash or rollback

Signals:

- dirty repo
- no baseline for the current session
- prompt/session-start output explicitly tells you to run the crash-recovery helper

Fix:

- run `py -3.14 tools/viewer_host_recover_crash_state.py --summary "<operator note>" --adopt-current-state`
- inspect the report
- retry in a fresh session

### Missing validation or contract-proof receipt

Signals:

- `HEAD` advanced compared to the session baseline
- repo may already be clean
- prompt or completion surfaces complain about missing receipts rather than missing baseline

Fix:

- finish the prior slice normally
- write the validation receipt and contract-proof receipt for the committed clean `HEAD`
- do not use the crash helper for a receipt-only problem

### Contract drift or out-of-scope dirty files

Signals:

- recovery report shows `out_of_contract_scope` paths
- locked contract validation reports an error, or the active contract no longer matches the intended slice

Fix:

- decide whether the dirty files are legitimate slice expansion or accidental spill
- if the work is legitimate, revise and re-lock the contract before further mutation
- if not, treat the drift as workflow debt to resolve before broadening the slice

## Safe Resume Checklist

After adoption succeeds:

1. Reopen the stranded phased plan and confirm the real current phase.
2. Reopen the recovery report and note any `out_of_contract_scope` files.
3. Resume in crash-safe mode:
   - one terminal command at a time
   - no parallel heavy validation
   - prefer `py -3.14 tools/viewer_host_run_logged_command.py --label ... --log artifacts/... -- <command ...>` for large output
4. Finish the slice with the normal validation, handoff, commit, and receipt flow.

## Portable Pattern For Similar Repos

If another repo carries session-baseline hooks, copy the pattern rather than the exact filenames:

1. Keep prompt/completion hooks fail-closed for dirty carryover.
2. Add one repo-local recovery helper that records the stranded dirty snapshot durably.
3. Require explicit operator adoption before a new session can reuse that dirty snapshot as its baseline.
4. Make adoption one-shot and consume it after baseline materialization.
5. Record enough detail for forensics:
   - branch
   - `HEAD`
   - dirty-path summary
   - active contract or task scope
   - receipt state
   - latest handoff or continuity context
   - in-scope versus out-of-scope changed paths
6. Document the exact operator command in the repo bootstrap docs, not only in chat.

## External-Agent Repair Note

This playbook exists because an external agent had to repair the first real crash-lockout incident in this repo after VS Code extension-host instability plus rollback left a dirty slice without a session baseline. Keep that history in mind when similar repos adopt these hooks: the dangerous edge case is not just "editor crash", it is "editor crash while dirty, followed by a new session that has no baseline and no recovery seam."
