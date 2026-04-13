# Headless Exploration Advisor

## Current Phase

Closed - headless advisor v1 shipped

## Phase Checklist

- [x] Phase 1 - deterministic advisor report seam
- [x] Phase 2 - headless CLI/report mode
- [x] Phase 3 - focused runtime artifact proof
- [x] Phase 4 - closure audit and continuity cleanup

## Notes

- Why this plan exists:
  - the shipped Explaino replay work proved deterministic load-and-apply behavior, but it does not yet help the user decide what to inspect next
  - the existing sidecar seams already compute the ingredients for a useful advisor: hypothesis space, measurement, budget, completeness, lens, energy, and one best recommendation
  - the first useful product should be a deterministic headless artifact, not a new live-window workflow
- Product boundary:
  - first shipped job: "given this Explaino state, recommend the next most informative observation and explain why"
  - first shipped surface: headless JSON artifact from a CLI mode
  - keep the first version Explaino-only and read-only; no autonomous mutation loop, no live playback, no new sidecar UI obligations
- Phase 1 exit criteria:
  - a focused native seam can build a deterministic advisor report from a fixed Explaino state using the existing sidecar intelligence path
  - the report includes at least one ranked recommendation plus the selected top recommendation and enough metadata to review it later
  - the report fails fast on unsupported/non-Explaino states instead of silently producing a generic fallback
- Phase 2 exit criteria:
  - a new headless CLI/report mode can load a state or use the current CLI-configured state and emit the advisor JSON artifact deterministically
  - the first mode can write to an explicit JSON path and/or stdout without mutating runtime state
- Phase 3 exit criteria:
  - focused runtime coverage proves end-to-end state-load plus report generation from a saved Explaino state
  - artifact content is stable enough to compare fields deterministically in pytest
- Phase 4 exit criteria:
  - audit closes any silent eligibility drift, stale-state leakage, or recommendation-order instability defects
  - continuity surfaces (`_STATUS`, phased plan, handoff log) match the landed advisor scope

## Closure Notes

- Shipped scope:
  - `--explore-recommend-json <path>` builds a deterministic Explaino-only advisor report from the current or loaded state
  - the report reuses the existing sidecar measurement, budget, completeness, energy, and action recommendation seams instead of introducing a second ranking pipeline
  - unsupported/non-Explaino states fail fast instead of falling back silently
- Validation run during closure:
  - focused native binaries: `test_viewer_cli.exe`, `test_explaino_exploration_advisor.exe`, `test_headless_modes.exe`
  - runtime publish: `ui_app/build_vsdevcmd.cmd`
  - focused runtime pytest: `py -3.14 -m pytest tests/test_fractal_runtime_explaino_escape_variants.py -q`