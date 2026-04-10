# Phased Plan Continuity Protocol

## Purpose

Chat history is not the durable source of truth for this repo.

For any meaningful slice, the resumable state must live in repo documents that a fresh agent can reopen without archaeology.

## Required Surface

Each meaningful multi-step slice should use a phased plan document with, at minimum, these sections:

- `## Current Phase`
- `## Phase Checklist`
- `## Notes` or another short evidence section for blockers, proof, or next actions

Preferred location:

- reuse the nearest existing plan doc first
- otherwise create `docs/notes/<slug>_PHASED_PLAN.md`

## Sync Rules

When editing a phased plan:

1. Update `## Current Phase` and `## Phase Checklist` in the same edit.
2. The current phase must match the first unchecked checklist phase.
3. Do not leave earlier phases unchecked while claiming a later phase is current.
4. When all phases are checked, `## Current Phase` should say `Complete`, `Completed`, or `Done`.
5. Remove or rewrite stale blockers once a later checked-in slice has already closed them.

## Deterministic Check

Use the local repo adapter instead of visually eyeballing plan drift:

- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py --all`
- task: `agent: assert phased plan sync`

Default behavior:

- if phased-plan files are dirty, the tool checks those
- otherwise, if the most recent commit touched phased-plan files, the tool checks those
- otherwise, it exits cleanly with a no-op message

## Minimal Template

```md
# <Slice Title>

## Current Phase

Phase 1 - <short label>

## Phase Checklist

- [ ] Phase 1 - <short label>
- [ ] Phase 2 - <short label>
- [ ] Phase 3 - <short label>

## Notes

- Exit criteria:
- Validation:
- Deferred questions:
```

## Session Transition Usage

At session start:

1. Run `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8`.
2. Reopen the active phased plan, if one exists.
3. Confirm that `## Current Phase` and `## Phase Checklist` still agree.
4. Append a pending slice breadcrumb with `py -3.14 tools/viewer_host_begin_work_slice.py ...` before broad new work.

At session end:

1. Update the phased plan with the real stop point.
2. Append `HANDOFF_LOG.md` with the validated checkpoint or pending breadcrumb.
3. Leave enough evidence in the plan that a fresh agent can continue from the repo alone.