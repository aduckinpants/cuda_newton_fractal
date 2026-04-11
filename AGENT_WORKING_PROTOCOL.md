# Agent Working Protocol — cuda_newton_fractal_clone

## Mission

This repo is a CUDA/C++/DX11 fractal viewer-host.
Work here merges back into the mainline Salticid system.
The agent's job: deliver bounded, tested, checkpointed slices that
a successor agent can resume without archaeology.

## Bootstrap Surface

Read `AGENTS.md` first in a new session.

Use it as the concise bootstrap checklist and session-transition surface for this repo.
Treat `AGENT_WORKING_PROTOCOL.md` as the detailed rules document and `AGENTS.md`
as the short operational bootstrap.

Repeatable bootstrap command:
`py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8`

---

## Non-Negotiable Invariants

### 1. Forward TDD

Every behavioral change follows:
1. **Red** — write or extend a focused headless test that fails (proving the gap)
2. **Green** — implement the minimal fix that makes the test pass
3. **Refactor** — clean up only what the test covers
4. **Validate** — run the checked-in build/test scripts (see Build Commands below)
5. **Checkpoint** — commit when tests are green

No behavioral change lands without a test that exercises it.
No test is written after the implementation it covers.
No "I'll add a test later" — that is tech debt, not progress.

### 2. Pit of Success

The right thing must be easy; the wrong thing must be hard or impossible.

Concretely:
- Unknown binding paths, enum ids, or invalid params → fail fast, never silent fallback
- Schema JSON is the UI layout authority; C++ structs are the runtime authority;
  do not create a second editable source of truth
- New float params become animatable via BindFloat + schema JSON entry — no enums,
  no switch/case, no manual dispatch
- Prefer headless testable modules over GUI-only verification
- Prefer deterministic scripts over ad hoc runtime speculation

### 2.1 Mandatory Distrust-First Audit

Every meaningful slice must go through a hostile audit before it is considered complete.

Required posture:
- assume the initial implementation is wrong or incomplete
- assume at least one bug, omission, or workflow mistake still exists
- do not stop at "tests passed" or "build is green"

Required audit loop:
1. Review the diff and touched seams as if there is already a bug in them.
2. Search for at least one real defect, missing regression, misleading checkpoint, or workflow mistake.
3. If a defect is found, add or extend the regression first, fix the issue, revalidate, and audit the repaired state again.
4. Only stop the audit after 2-3 deliberate passes on the repaired state fail to find another real issue.

Audit evidence must be durable:
- record meaningful audit findings or "repair follow-up" checkpoints in `HANDOFF_LOG.md`
- update the phased plan stop point if the audit changes the real phase boundary
- call out workflow/tooling friction found during the audit in the final summary

### 2.2 Deterministic Completion Guard

This repo has a workspace hook guard at `.github/hooks/checkpoint_guard.json` backed by
`tools/viewer_host_checkpoint_guard.py`.

It exists to enforce the closure invariant at runtime, not just in prose:
- `SessionStart` captures the repository dirty-state baseline for the session
- `PreToolUse` denies `task_complete` when the repo state differs from that baseline
- `Stop` blocks the agent from ending while the repo state still differs from that baseline

Do not treat the hook as optional guidance. If it fires, fix the repo state by committing,
reverting to the session baseline, or otherwise resolving the discrepancy before trying to stop.

### 2.3 Validation Receipt Gate

Clean git state alone is not enough to justify closure after a commit in the same session.

If the current `HEAD` differs from the session-start `HEAD`, completion also requires a
validation receipt for the current committed state.

Required flow after the final validation commands pass for the slice:
1. ensure the repo is clean at the final committed `HEAD`
2. write a receipt with `py -3.14 tools\viewer_host_write_validation_receipt.py --summary "<what passed>" --command "<validation cmd>" ...`
3. only then use `task_complete` / stop the slice

The checkpoint guard enforces this deterministically:
- `PostToolUse` reminds immediately when `HEAD` advanced without a receipt
- `PreToolUse` denies `task_complete` for a clean-but-unreceipted committed state
- `Stop` blocks ending the slice until the receipt exists

### 3. No Implicit Fallback

This is a hard architectural rule, not a suggestion:
- If a parameter binding path is unknown → error
- If a fractal type enum is invalid → error
- If a schema resource is missing → error
- If a probe request references an unsupported function → error
- Never silently produce "something" when the intent is unclear

### 4. Schema is the Single Source of Truth

`ui/fractal_binding_surface_v1.ui_schema.json` drives the UI.
C++ structs (`ViewState`, `KernelParams`) drive runtime.
These two must stay in sync via the binding layer.
A second editable layout source WILL drift and WILL cause bugs.

---

## Build and Validation Commands

Use the checked-in scripts. Do not hand-assemble compiler commands.

Public workflow surface:
- `agent: session bootstrap`
- `agent: begin work slice`
- `agent: assert phased plan sync`
- `verify: profile native`
- `verify: profile runtime`
- `verify: profile catalog`
- `verify: profile checkpoint`

| Task | Command | Notes |
|------|---------|-------|
| Session bootstrap | `py -3.14 tools\viewer_host_session_bootstrap.py --audit --tail-handoff 8` | Repeatable new-session start surface |
| Begin work slice | `py -3.14 tools\viewer_host_begin_work_slice.py --intent "<slice>" --profile <native|runtime|catalog|checkpoint|unspecified>` | Repo-specific adapter that delegates to mainline `handoff_append.py` |
| Plan sync check | `py -3.14 tools\viewer_host_assert_phased_plan_sync.py` | Deterministic phased-plan continuity adapter |
| Native helper tests | `ui_app\build_tests_vsdevcmd.cmd` | Must pass before any commit |
| Full viewer build | `ui_app\build_vsdevcmd.cmd` | Must pass before any commit |
| Probe catalog smoke | `py -3.14 tools\reality_toolkit\scripts\run_fractal_catalog_smoke.py --strict --out-dir <task-dir>` | For probe/descriptor changes |
| Python test suite | `py -3.14 -m pytest tests/ -q` | For Python-side changes |
| Specific test file | `py -3.14 -m pytest tests/<test_file>.py -q` | Targeted validation |

For long-running direct terminal commands, prefer:
`py -3.14 tools\viewer_host_run_logged_command.py --label "<label>" --log artifacts/<task>.log -- <command ...>`

This keeps the full transcript in `artifacts/` while emitting a short deterministic summary to stdout, which is more reliable than asking editor-integrated terminal wrappers to carry the entire build/test transcript.

**After both build commands pass**, the runtime is at:
`D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`

**Typical validation sequence** for a code slice:
```
ui_app\build_tests_vsdevcmd.cmd 2>&1
ui_app\build_vsdevcmd.cmd 2>&1
py -3.14 -m pytest tests/<relevant_tests>.py -q
```

---

## Slice Workflow

### Starting a slice

1. Read this protocol, `DEFERRED_THREADS.md`, `KNOWN_ISSUES.md`, and the
   relevant spec intake doc
2. Identify the bounded slice and its exit criteria
3. Write the test first (Red)
4. Implement (Green)
5. Validate with build commands
6. Commit with a descriptive message

### During a slice

- One thing at a time. Do not expand scope mid-slice.
- If you discover a new issue, add it to `KNOWN_ISSUES.md` — do not fix it now
  unless it blocks the current slice.
- If you discover deferred work, add it to `DEFERRED_THREADS.md`.
- Keep the worktree focused; do not touch unrelated files.

### Ending a slice

1. Both build scripts pass
2. Relevant Python tests pass
3. Run the mandatory distrust-first audit loop and repair anything it finds
4. Commit with clear message explaining what landed
5. Write the validation receipt for the final committed `HEAD`
6. Update `HANDOFF_LOG.md` with checkpoint entry
7. Update the active phased plan checklist if one exists

---

## Phased Plans and Checklists

For any multi-slice initiative:
- Maintain a detailed phased plan in `spec_intake/` or `docs/`
- Each phase has explicit exit criteria
- Each turn updates the checklist: mark completed steps, note blockers
- The plan is the resumable state — not chat history

Required continuity rules:
- If you edit a `*_PHASED_PLAN.md` file, keep `## Current Phase` and
  `## Phase Checklist` synchronized in the same edit
- Do not leave a later phase current while earlier phases remain unchecked
- Use `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` or the VS Code task
  `agent: assert phased plan sync` instead of eyeballing plan drift

Reference protocol:
- `docs/PHASED_PLAN_CONTINUITY_PROTOCOL.md`

For the current active specs, see `spec_intake/_STATUS.md`.

## No Chat-History Reliance

Do not treat chat history as a durable planning surface.

For every meaningful work slice:
- create or update a detailed, checklisted, phased plan in the repo
- prefer the nearest existing plan doc; otherwise create
  `docs/notes/<slug>_PHASED_PLAN.md`
- treat `HANDOFF_LOG.md` as the checkpoint index and the phased plan as the
  detailed resumable state

---

## Handoff Discipline

### HANDOFF_LOG.md

Append-only checkpoint log. Each entry records:
- Checkpoint hash
- UTC timestamp
- What landed
- What was validated
- What comes next (optional but helpful)

### DEFERRED_THREADS.md

Intentionally paused work threads. Each entry records:
- What was being done
- Why it was paused
- What conditions allow resuming
- Key files involved

### KNOWN_ISSUES.md

Catalogued bugs and gaps. Prioritized as P0/P1/P2.
When an issue is fixed, mark it resolved with the fixing commit.

### Rule: nothing stays only in chat

If information would be lost when the chat session ends, it belongs in one of:
- `HANDOFF_LOG.md` (what happened)
- `DEFERRED_THREADS.md` (what's paused)
- `KNOWN_ISSUES.md` (what's broken)
- A spec/plan doc (what's planned)

---

## Cousin Repo Boundaries

| Repo | Relationship | Rule |
|------|-------------|------|
| `salticid-cuda` | Read-only reference | Do not edit from this workspace; use handoff docs |
| `nine` | Planning/research reference | Do not edit from this workspace |
| `wizard-test` | Historical/conceptual reference | Read-only |

Cross-repo tooling: the handoff append tool lives in salticid-cuda:
```
py -3.14 C:\code\salticid-cuda\tools\handoff_append.py --repo-root "c:\code\cuda_newton_fractal_clone" "message"
```

Do not fork core workflow tools from mainline into this repo under the same names.
Use the mainline helper directly when it supports `--repo-root`, or use a thin
repo-specific `viewer_host_*` adapter when local repo context is required.

---

## Architecture Rules (from .github/copilot-instructions.md)

- Preserve the distinction between fractal runtime state (`ViewState`, `KernelParams`)
  and viewer model state (`ViewportState`, `IdeWindowModel`, `ViewportWindowModel`)
- Extract logic OUT of `ui_app/src/main.cpp` — do not grow the monolith
- Reuse the mainline headless model layer when possible
- Parameter animation targets are resolved at runtime via BindFloat — do NOT add
  enums, switch/case, or manual dispatch

---

## Terminal Discipline

- Reuse existing terminals; do not proliferate
- Default to `isBackground: false` — wait for output
- Target ~3 active terminals maximum
- If a terminal is stuck or contaminated, refresh that one terminal only
- Use `py -3.14` explicitly for Python commands in this repo

---

## Git Protocol

- Commit when a slice is validated and tests are green
- Do not let uncommitted work drift for hours
- Do not switch branches, force-push, or destructive-reset without user permission
- Commit messages should explain what landed and what was validated
- Push is manual / at user discretion in this repo

---

## Session Start Checklist

When beginning work in a new session:

1. Run `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8`
2. Read `AGENTS.md`
3. Read `AGENT_WORKING_PROTOCOL.md` (this file)
4. Read `spec_intake/_STATUS.md` for current phase
5. Read `DEFERRED_THREADS.md` for paused work
6. Read `KNOWN_ISSUES.md` for known bugs
7. Read `HANDOFF_LOG.md` (last 5-10 entries) for recent context
8. Read the relevant spec intake doc for the current initiative
9. If the slice is meaningful, run `py -3.14 tools/viewer_host_begin_work_slice.py --intent "<slice>" --profile <profile>`
10. Create or update the nearest phased plan and assert sync if you touch it
11. Check git status — if the worktree is dirty, assess and checkpoint carryover
12. Identify the bounded next slice and proceed
