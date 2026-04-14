# Callable Engine Surface Wrap

## Current Phase

Phase 4 - handoff to transpiler/kernel registration thread

## Phase Checklist

- [x] Phase 1 - wrap current callable surface and advisor stdout
- [x] Phase 2 - document generic/lambda request-composition boundary
- [x] Phase 3 - design registry-backed callable-surface generalization
- [ ] Phase 4 - handoff to transpiler/kernel registration thread

## Notes

- Why this plan exists:
  - the engine already exposes multiple callable headless seams, but they are wrapped unevenly: `fractal.sample`, `generic.sample`, `--describe-functions`, `--sample-request-*`, `--sample-session`, and the new advisor report mode
  - the current generic function sampler proves that incoming sample requests can carry function expressions, but that surface is not yet clearly documented as the preview boundary for broader callable-engine generalization
  - the user wants the near-term surface wrapped cleanly now, with a clear path toward lambda/kernel compositions built from incoming sample requests later
- Current reality:
  - shipped callable functions: `fractal.sample`, `generic.sample`
  - shipped discovery surface: `--describe-functions`, `--describe-functions-json`
  - shipped sample transport: `--sample-request-stdin/json`, `--sample-response-stdout/json`, `--sample-session`
  - shipped advisor surface: `--explore-recommend`, `--explore-recommend-json`
  - landed wrap/doc surface: advisor stdout symmetry plus [docs/callable_engine_surface.md](docs/callable_engine_surface.md) as the current callable-surface reference and generic/lambda boundary note
  - current operator-facing cheatsheet: [docs/callable_engine_dynamic_function_cheatsheet.md](docs/callable_engine_dynamic_function_cheatsheet.md) summarizes what `generic.sample` can actually solve and sweep today, plus the current POC limits
  - current runtime-probe companion docs: [docs/callable_engine_fractal_sample_cheatsheet.md](docs/callable_engine_fractal_sample_cheatsheet.md) plus [docs/examples/callable_engine/README.md](docs/examples/callable_engine/README.md) give the matching `fractal.sample` can/cannot-do view and copy-paste request examples
  - current transport companion docs: [docs/callable_engine_transport_session_cheatsheet.md](docs/callable_engine_transport_session_cheatsheet.md) plus the transport examples under [docs/examples/callable_engine/README.md](docs/examples/callable_engine/README.md) cover stdin/stdout, file I/O, batch arrays, NDJSON, session `state_token` diffs, and the named-pipe alternate transport
  - phase-3 registry result: a built-in callable registry now acts as the single authority for shipped function ids, execution kinds, and descriptor builders, so descriptor emission, fail-fast `function_id` validation, and fractal-vs-generic dispatch no longer duplicate `fractal.sample` / `generic.sample` in separate seams
  - current Phase 4 wrap target: `generic.sample` now grows through a low-prominence execution seam rather than a new math parameter: request-side `execution.backend_preference = default | cpu | cuda`, response-side `runtime.backend_used`, and an internal transport-agnostic backend dispatcher in the runner that later in-process callers can reuse directly
- Phase 1 exit criteria:
  - advisor mode supports stdout and optional file output with the same deterministic payload
  - focused native/runtime tests cover the stdout path
  - a checked-in reference doc explains the current callable-engine surface, the function ids it ships today, and representative request examples
- Phase 2 exit criteria:
  - a checked-in design note explains how incoming sample requests should express generic/lambda compositions without pretending dynamic kernel registration already exists
  - the note marks the boundary between the shipped `generic.sample` preview surface and the later registry/transpiler work
- Phase 3 exit criteria:
  - the repo has a concrete design for a registry-backed callable surface that can host more than `fractal.sample` and `generic.sample` without forking the request contract
  - fail-fast behavior, descriptor derivation rules, and function-id authority are explicit
  - landed: the static registry now owns descriptor builders as well as execution kinds, making additional built-in callable registrations a single-registry-surface change instead of a registry row plus a separate descriptor switch
- Phase 4 exit criteria:
  - the follow-on handoff into the Salticid transpiler/kernel-registration thread is documented with the engine-side responsibilities clearly separated from the sibling-repo CUDA backend work
  - the repo continuity docs make it explicit that the new backend-preference surface is execution metadata, not a new function parameter or a CLI-owned contract

## Validation

- `ui_app/build_tests_vsdevcmd.cmd`
- `ui_app/build_vsdevcmd.cmd`
- `py -3.14 -m pytest tests/test_fractal_runtime_explaino_escape_variants.py tests/test_function_descriptor_cli.py tests/test_generic_probe_cli.py -q`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json`