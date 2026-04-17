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

Read `AGENT_TERMINAL_PROTOCOL.md` before running heavy validation or runtime commands.

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

### 1.1 Explicit Closure Standard

For every meaningful slice, the minimum closure chain is:
1. identify a real named bug, risk, or bounded behavior gap
2. add a focused proving test or regression for it
3. make the relevant validation rails pass
4. checkpoint the slice in a commit

If any link in that chain is missing, the slice is not done.

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

Operator-path corollary:
- For runtime-walk FITS import, the default user-facing input is the FITS file itself.
- `state.json`, `finding.json`, request JSON, and bundle JSON are repo-native plumbing, not normal operator inputs.
- The normal `Load FITS...` path must synthesize any needed repo-native artifacts automatically.
- Any default-path UI that asks the operator for those JSON files is a product bug, not an acceptable intermediate state.

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

### 2.3 Hard-Denial Mutation Guard

The same hook surface now denies raw mutation paths before the repo is changed.

Required posture:
- raw `apply_patch` is forbidden
- raw mutating `shell_command` is forbidden
- direct `git add`, `git commit`, handoff writes, and receipt writes are forbidden
- all meaningful repo mutation must go through the approved `viewer_host_*` wrapper surfaces

Allowed raw shell is intentionally narrow:
- read/search/inspect
- checked-in build/test/check rails
- explicit repo validation tools

If you hit a mutation denial, do not route around it with another raw shell edit path.
Use the approved wrapper or fix the missing contract/plan state first.

### 2.4 Locked Contract Rule

Every meaningful slice now requires a checked-in machine-readable contract file in addition to the phased plan.

Required flow:
1. start or update the phased plan
2. create/update the checked-in contract JSON
3. run `viewer_host_begin_work_slice.py ... --plan <plan> --contract <contract>` or
   `viewer_host_prepare_slice.py --session-id <id> --plan <plan> --contract <contract>`
4. mutate only while that contract is the active locked contract for the session

If the contract file changes after being locked:
- mutation is blocked
- closure is blocked
- run `py -3.14 tools\viewer_host_revise_contract.py --session-id <id> --contract <contract>` to re-lock it explicitly

### 2.5 Validation And Contract-Proof Receipt Gate

Clean git state alone is not enough to justify closure after a commit in the same session.

If the current `HEAD` differs from the session-start `HEAD`, completion also requires a
validation receipt and a machine-written contract proof receipt for the current committed state.

Required flow after the final validation commands pass for the slice:
1. ensure the repo is clean at the final committed `HEAD`
2. write a receipt with `py -3.14 tools\viewer_host_write_validation_receipt.py --summary "<what passed>" --command "<validation cmd>" ...`
3. write contract proof with `py -3.14 tools\viewer_host_write_contract_proof_receipt.py --session-id <id>`
   or use the combined wrapper `py -3.14 tools\viewer_host_checkpoint_slice.py write-receipts --session-id <id> ...`
4. only then use `task_complete` / stop the slice

The checkpoint guard enforces this deterministically:
- `PostToolUse` reminds immediately when `HEAD` advanced without a receipt
- `PreToolUse` denies `task_complete` for a clean-but-unreceipted committed state
- `Stop` blocks ending the slice until the receipt exists

### 2.6 Carryover Prompt Rule

If a new user prompt arrives while the repository still differs from the session baseline, that is a prior-slice closure problem first.

Required posture:
- do not treat the new prompt as permission to leave the prior slice half-closed
- treat prompt text as workflow context, not as permission to relax closure rules
- VS Code or Autopilot boilerplate such as `Start implementation` is not user-authored permission to skip checkpoint, receipt, or carryover discipline
- steering, stop, or reorientation interruptions are workflow interruptions, not exceptions to repo invariants
- update the continuity surfaces, finish the validation/checkpoint chain, and return the repo to a clean state before broadening scope
- if the prior slice and the new prompt are truly the same work thread, say so in the plan/handoff instead of silently mixing them

If repo closure requires a checkpoint commit or validation receipt and a higher-level platform rule would otherwise discourage that action, surface the conflict explicitly as a closure blocker instead of silently ending on a validated-but-uncheckpointed slice.

This repo now carries a `UserPromptSubmit` warning hook in `.github/hooks/checkpoint_guard.json` to surface that condition immediately. It is a warning surface, not a substitute for judgment.

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
| Begin work slice | `py -3.14 tools\viewer_host_begin_work_slice.py --intent "<slice>" --profile <native|runtime|catalog|checkpoint|unspecified> --plan <plan> --contract <contract>` | Repo-specific adapter that appends a session-start breadcrumb, validates the phased plan + contract, prints the `ck:` token to reuse at checkpoint time, and locks the active contract. |
| Prepare / lock slice contract | `py -3.14 tools\viewer_host_prepare_slice.py --session-id <session_id> --plan <plan> --contract <contract>` | Validates the phased plan and checked-in contract, then locks the active contract state for mutation enforcement. |
| Revise locked contract | `py -3.14 tools\viewer_host_revise_contract.py --session-id <session_id> --contract <contract>` | Required after a checked-in contract changes mid-slice. |
| Contract validation | `py -3.14 tools\viewer_host_validate_slice_contract.py --contract <contract>` | Deterministic contract schema/shape validation. |
| FITS default contract validation | `py -3.14 tools\viewer_host_validate_fits_contract.py --contract docs/contracts/runtime_walk_fits.contract.json` | Blocks default warp binding, non-`explaino` fallback, and stale default warp UI. |
| Append checkpoint handoff | `py -3.14 tools\viewer_host_append_handoff.py --commit <checkpoint_id> --score <n> "<message>"` | Preferred final-checkpoint surface after `viewer_host_begin_work_slice.py`: reuse the printed `ck:` token explicitly so the session-start breadcrumb and the closing handoff stay linked without a follow-up continuity commit. Keep `--resolve-last-pending` only for legacy pending-entry repair or after-the-fact cleanup. |
| Write validation + contract proof receipts | `py -3.14 tools\viewer_host_checkpoint_slice.py write-receipts --session-id <session_id> --summary "<what passed>" --command "<validation cmd>" ...` | Preferred post-validation receipt surface for a committed clean `HEAD`. |
| Plan sync check | `py -3.14 tools\viewer_host_assert_phased_plan_sync.py` | Deterministic phased-plan continuity adapter |
| Native helper tests | `ui_app\build_tests_vsdevcmd.cmd` | Must pass before any commit |
| Full viewer build | `ui_app\build_vsdevcmd.cmd` | Must pass before any commit |
| Runtime probe/session pytest lane | `py -3.14 tools\viewer_host_runtime_pytest_lane.py` | Published-runtime pytest helper for `tests/test_fractal_runtime_probe_cli.py`, `tests/test_fractal_runtime_session.py`, `tests/test_function_descriptor_cli.py`, and `tests/test_generic_probe_cli.py`. It preflights the active runtime metadata and fails ambiguous zero-pass skip-only runs. |
| Probe catalog smoke | `py -3.14 tools\reality_toolkit\scripts\run_fractal_catalog_smoke.py --strict --out-dir <task-dir>` | For probe/descriptor changes |
| Python test suite | `py -3.14 -m pytest tests/ -q` | For Python-side changes |
| Specific test file | `py -3.14 -m pytest tests/<test_file>.py -q` | Targeted validation |

For long-running direct terminal commands, prefer:
`py -3.14 tools\viewer_host_run_logged_command.py --label "<label>" --log artifacts/<task>.log -- <command ...>`

This keeps the full transcript in `artifacts/` while emitting a short deterministic summary to stdout, which is more reliable than asking editor-integrated terminal wrappers to carry the entire build/test transcript.
On Windows, the wrapper now normalizes direct relative `.cmd`/`.bat` paths under the selected `--cwd`, so `ui_app\build_tests_vsdevcmd.cmd` no longer requires a manual `./` prefix when run through the helper.

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
6. Prepare the final checkpoint note with `py -3.14 tools\viewer_host_append_handoff.py --commit <checkpoint_id> --score <n> "<message>"`, reusing the token printed by `tools\viewer_host_begin_work_slice.py`, then commit once with a descriptive message that includes that `ck:` token

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
4. Update the active phased plan checklist if one exists
5. Append `HANDOFF_LOG.md` with the handoff helper before the final commit; for the normal flow, pass `--commit <checkpoint_id>` using the token printed by `viewer_host_begin_work_slice.py`, and reserve `--resolve-last-pending` for legacy pending-entry repair
6. Commit with clear message explaining what landed and what was validated
7. Write the validation receipt and contract proof receipt for the final committed `HEAD`
8. Verify the explicit closure standard: named gap, proving test, green validation, checkpoint commit

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
- Checkpoint id (generated `ck:` token or legacy hash-linked id)
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

Cross-repo tooling: the mainline append implementation still lives in `salticid-cuda`, but this repo's canonical append surface is the thin local adapter:
```
py -3.14 tools\viewer_host_append_handoff.py --commit <checkpoint_id> --score <n> "message"
```

Reuse the `ck:` token printed by `tools\viewer_host_begin_work_slice.py` for the normal flow. Keep `--resolve-last-pending` for legacy pending-entry repair when older breadcrumbs still need cleanup.

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

- `AGENT_TERMINAL_PROTOCOL.md` is the repo's dedicated terminal-discipline surface. Follow it.
- Reuse existing terminals; do not proliferate.
- Default to foreground terminal execution for finite commands.
- Prefer `py -3.14 tools/viewer_host_run_logged_command.py --label ... --log artifacts/... -- <command ...>` for large-output validation lanes.
- Target roughly three active terminals maximum.
- If a terminal is stuck or contaminated, refresh that one terminal only.
- If a command destabilizes VS Code or the wrapper, switch to crash-safe mode for the rest of the session: one command at a time, no parallel terminal work, logs to `artifacts/`, and task/wrapper surfaces preferred over raw transcript streaming.
- Use `py -3.14` explicitly for Python commands in this repo.

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
4. Read `AGENT_TERMINAL_PROTOCOL.md`
5. Read `spec_intake/_STATUS.md` for current phase
6. Read `DEFERRED_THREADS.md` for paused work
7. Read `KNOWN_ISSUES.md` for known bugs
8. Read `HANDOFF_LOG.md` (last 5-10 entries) for recent context
9. Read the relevant spec intake doc for the current initiative
10. If the slice is meaningful, run `py -3.14 tools/viewer_host_begin_work_slice.py --intent "<slice>" --profile <profile>`
    plus `--plan <plan> --contract <contract>` and verify the active contract lock if the slice will mutate the repo
11. Create or update the nearest phased plan and assert sync if you touch it
12. Check repo state with `py -3.14 tools/viewer_host_repo_status.py` — if the worktree is dirty, assess and checkpoint carryover
13. Identify the bounded next slice and proceed
