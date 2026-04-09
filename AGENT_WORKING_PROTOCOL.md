# Agent Working Protocol — cuda_newton_fractal_clone

## Mission

This repo is a CUDA/C++/DX11 fractal viewer-host.
Work here merges back into the mainline Salticid system.
The agent's job: deliver bounded, tested, checkpointed slices that
a successor agent can resume without archaeology.

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

| Task | Command | Notes |
|------|---------|-------|
| Native helper tests | `ui_app\build_tests_vsdevcmd.cmd` | Must pass before any commit |
| Full viewer build | `ui_app\build_vsdevcmd.cmd` | Must pass before any commit |
| Probe catalog smoke | `py -3.14 tools\reality_toolkit\scripts\run_fractal_catalog_smoke.py --strict --out-dir <task-dir>` | For probe/descriptor changes |
| Python test suite | `py -3.14 -m pytest tests/ -q` | For Python-side changes |
| Specific test file | `py -3.14 -m pytest tests/<test_file>.py -q` | Targeted validation |

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
3. Commit with clear message explaining what landed
4. Update `HANDOFF_LOG.md` with checkpoint entry
5. Update the active phased plan checklist if one exists

---

## Phased Plans and Checklists

For any multi-slice initiative:
- Maintain a detailed phased plan in `spec_intake/` or `docs/`
- Each phase has explicit exit criteria
- Each turn updates the checklist: mark completed steps, note blockers
- The plan is the resumable state — not chat history

For the current active specs, see `spec_intake/_STATUS.md`.

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

1. Read `AGENT_WORKING_PROTOCOL.md` (this file)
2. Read `spec_intake/_STATUS.md` for current phase
3. Read `DEFERRED_THREADS.md` for paused work
4. Read `KNOWN_ISSUES.md` for known bugs
5. Read `HANDOFF_LOG.md` (last 5-10 entries) for recent context
6. Read the relevant spec intake doc for the current initiative
7. Check git status — if worktree is dirty, assess and checkpoint carryover
8. Identify the bounded next slice and proceed
