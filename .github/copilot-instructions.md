# Viewer Host Repo Guidelines

## Session Start
- Read `AGENT_WORKING_PROTOCOL.md` first. It contains the full working rules,
  build commands, slice workflow, and handoff discipline.
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
- Follow strict TDD for behavioral changes: add or extend the focused test first, then implement the minimal fix, then refactor.
- Prefer deterministic scripts, tests, and generated reports over ad hoc runtime speculation.
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