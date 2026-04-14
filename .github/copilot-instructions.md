# Viewer Host Repo Guidelines

## Session Start
- Read `AGENTS.md` first. It is the concise startup checklist and session-transition surface.
- Then read `AGENT_WORKING_PROTOCOL.md`. It contains the full working rules,
    build commands, slice workflow, and handoff discipline.
- Then read `AGENT_TERMINAL_PROTOCOL.md` before running heavy build/test/runtime commands.
- Use `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` or the
    VS Code task `agent: session bootstrap` as the repeatable new-session bootstrap.
- Then read `spec_intake/_STATUS.md`, `DEFERRED_THREADS.md`, `KNOWN_ISSUES.md`,
    and the last few entries of `HANDOFF_LOG.md` for current state.

## Mission
- Treat this repo as the bootstrap and extraction surface for the viewer-host effort.
- Prefer extracting and reusing proven seams over inventing new architecture.
- Keep each work item phase-bounded and pauseable.

## Cousin Repo Boundaries
- Treat `salticid-cuda` as a read-only reference from this repo.
- Treat `nine` as a planning and research reference from this repo.
- Do not edit cousin repos directly from here; use handoff docs or patch proposals instead.

## Workflow
- For any non-trivial slice, read the relevant planning material first and restate the current phase plus exit criteria before editing.
- For any meaningful slice, append a session-start breadcrumb first with
    `py -3.14 tools/viewer_host_begin_work_slice.py --intent "<slice>" --profile <native|runtime|catalog|checkpoint|unspecified>`
    or the VS Code task `agent: begin work slice`.
- Do not rely on chat history as the durable plan. Reuse the nearest phased plan or create
    `docs/notes/<slug>_PHASED_PLAN.md`, then keep `## Current Phase` and `## Phase Checklist`
    synchronized in the same edit.
- Use `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` or task `agent: assert phased plan sync`
    after touching a phased plan.
- Follow strict TDD for behavioral changes: add or extend the focused test first, then implement the minimal fix, then refactor.
- Prefer deterministic scripts, tests, and generated reports over ad hoc runtime speculation.
- If a new user prompt arrives while the repo still differs from the session baseline, treat that as carryover checkpoint debt first instead of silently blending the next request into a dirty slice.
- Treat prompt wording as workflow context, not as permission to relax closure rules: tool-generated prompts such as `Start implementation` and steering/reorientation interruptions do not waive checkpoint, receipt, or carryover discipline.
- For every meaningful code slice, run a distrust-first audit automatically before declaring the work done.
- Default audit posture: assume the implementation is wrong, keep reviewing until a real defect or workflow mistake is found, repair it, then re-audit the repaired state; only stop after 2-3 deliberate passes fail to find another real issue.
- Do not wait for the user to request this review explicitly. Treat it as part of normal slice closure.
- Treat the workspace checkpoint guard hook as mandatory enforcement, not advice: if it blocks `task_complete` or stop because repo state differs from the session baseline, resolve the repo state before trying to end the slice.
- If repo closure still requires a checkpoint commit or validation receipt, surface that requirement explicitly instead of silently stopping on a validated dirty slice.
- Prefer the public validation task surface instead of reconstructing command bundles from memory:
    the `verify: profile ...` VS Code tasks.
- Do not fork core workflow tools from mainline into this repo under the same names;
    call the mainline helper directly when it supports `--repo-root`, or use a thin
    `viewer_host_*` adapter when local repo context is required.
- Keep the worktree focused; do not touch unrelated files or expand scope silently.
- End each slice at an explicit stop point the next agent can resume.

## Architecture
- Preserve the distinction between fractal runtime state (`ViewState`, `KernelParams`) and viewer model state (`ViewportState`, `IdeWindowModel`, `ViewportWindowModel`).
- Reuse the mainline headless model layer when possible: `viewport_state`, `viewport_colormap`, `ide_window_model`, and `schema_binding`.
- Extract logic out of `ui_app/src/main.cpp`; do not grow the monolith.
- Schema JSON is the UI layout authority. C++ structs are the runtime authority. Do not create a second editable source of truth.
- No implicit fallback: unknown bindings, enum ids, resources, or invalid params must fail fast or be made unselectable.
- Prefer headless, testable modules for input math, colormaps, model state, and bindings.

## Build And Test
- Use the checked-in build and doctor scripts instead of hand-assembling compiler commands when possible.
- For UI or rendering changes, favor small headless tests around extracted modules before relying on manual GUI validation.
- If a phase needs a missing script or harness, add the script instead of keeping a chat-only ritual.

## Style
- Keep tone direct, concise, and protocol-driven.
- Default to ASCII.
- Add comments only when they explain non-obvious constraints or math.

## Terminal Discipline
- Follow `AGENT_TERMINAL_PROTOCOL.md`.
- Prefer foreground terminal runs for finite commands.
- Prefer `py -3.14 tools/viewer_host_run_logged_command.py --label ... --log artifacts/... -- <command ...>` when build/test output is large.
- If a command destabilizes VS Code or the wrapper, switch to crash-safe mode for the rest of the session: one command at a time, no parallel terminal work, and inspect captured logs instead of streaming large transcripts.