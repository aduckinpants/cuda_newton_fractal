# Explaino Sidecar Mutation Replay

## Current Phase

Closed - shipped at headless parameter replay; future frame/live proof deferred

## Phase Checklist

- [x] Phase 1 - mutation-history persistence contract
- [x] Phase 2 - shipped headless parameter replay boundary
- [x] Phase 3 - future frame/live replay proof explicitly deferred
- [x] Phase 4 - closure audit and continuity cleanup

## Notes

- Follow-on source: [docs/notes/explaino_sidecar_direct_mutation_PHASED_PLAN.md](docs/notes/explaino_sidecar_direct_mutation_PHASED_PLAN.md)
- Why this plan exists:
  - the direct-mutation plan is complete for controller-policy persistence, but it explicitly deferred mutation-trace persistence or replay beyond the policy snapshot
  - the reflexive sidecar, direct mutation, recovery, and zero-axis optimization plans are all complete, so further Explaino work needs a new bounded plan instead of reopening closed phases
- Scope guardrails:
  - keep `state.json` as the only persisted authority for sidecar mutation history
  - do not introduce a second sidecar-state file or schema surface
  - keep the first slice persistence-only; replay behavior comes after the load/capture contract is proven
- Phase 1 landing note:
  - persisted `sidecar_mutation_history` now round-trips through diagnostics capture, finding/state load, CLI `--load-state-json`, and headless/live sidecar mutation application
  - malformed mutation-history payloads now fail atomically in the native state-load helpers
  - validation for the landed persistence slice: `ui_app\build_tests_vsdevcmd.cmd`, `ui_app\build_vsdevcmd.cmd`, and `py -3.14 -m pytest tests/test_fractal_runtime_explaino_escape_variants.py -q`
- Phase 2A landing note:
  - headless replay now supports deterministic ordered parameter replay from loaded `sidecar_mutation_history` via `--sidecar-replay-mutation-history-count`
  - replay reuses the existing sidecar mutation application semantics, fails fast when the requested replay count exceeds the loaded history, and preserves the persisted history payload during headless capture
  - validation for the landed Phase 2A slice: `ui_app\build_tests_vsdevcmd.cmd`, `ui_app\build_vsdevcmd.cmd`, and `py -3.14 -m pytest tests/test_fractal_runtime_explaino_escape_variants.py -q`
- Feature closure note:
  - this feature now closes cleanly at the shipped headless parameter replay boundary instead of staying half-open as an active spec
  - the previously planned Phase 2B frame-delta proof and live replay proof are explicitly deferred into `DEFERRED_THREADS.md` as a future expansion thread
  - no further code changes are required to treat the current replay behavior as a complete, checkpointed feature on this branch
- Phase 1 exit criteria:
  - diagnostics `state.json` can persist a bounded sidecar mutation-history payload for applied sidecar mutations
  - finding/state load paths round-trip that payload atomically with the rest of the persisted Explaino sidecar state
  - malformed mutation-history payloads fail without partially mutating runtime state
  - later CLI overrides that invalidate a loaded snapshot clear the loaded mutation-history baseline just like persisted orientation already does
- Phase 2 exit criteria:
  - headless runtime proof can replay persisted mutation history deterministically from `--load-state-json`
  - shipped scope closes at deterministic ordered parameter replay
- Phase 3 exit criteria:
  - future expansion thread only: frame-delta and live replay proof remain explicitly deferred
- Phase 4 exit criteria:
  - hostile audit closes any partial-load, stale-history, or type-drift defects
  - continuity surfaces (`_STATUS`, phased plan, handoff log) match the landed replay state