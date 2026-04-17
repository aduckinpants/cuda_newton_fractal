# FITS-Driven Explaino Synthesis And Playback V1

## Current Phase

Completed - FITS-driven Explaino synthesis/import/bootstrap v1 is landed and validated; follow-on work should build on the new synth authority path rather than the old FITS-as-evidence-only import flow

## Phase Checklist

- [x] Phase 1 - lock the synthesized-session contract, mapping profile surface, and red-test boundary
- [x] Phase 2 - add focused red tests for mapping-profile parsing, synthesized base-state generation, and import-session authority modes
- [x] Phase 3 - land the native bootstrap seam plus generated synthesized-state/session artifacts without regressing existing runtime-walk replay
- [x] Phase 4 - wire the viewer import flow, authority messaging, and synth/open affordances into the live runtime
- [x] Phase 5 - add focused runtime proof, hostile-audit the repaired slice, and checkpoint the validated stop point

## Notes

- Why this plan exists:
  - flashlight already proves this repo can bootstrap a deterministic Explaino runtime from external structured input without a pre-existing `state.json`
  - runtime-walk/FITS import still treats `state.json` as mandatory authority, which is the wrong product boundary for the Explaino FITS workflow
  - the user clarified that "data-driven" here means schema/decorator/JSON-driven mappings from FITS-derived orientation signals onto runtime parameters, not just replaying saved snapshots
- Default operator contract for this thread:
  - the FITS comes from another program
  - the viewer must accept that FITS directly
  - any repo-native `state.json`, request JSON, or bundle JSON needed for playback must be synthesized internally
- Locked implementation stance for this slice:
  - keep runtime-walk transport itself intact; the change is at the import/bootstrap/session-generation layer
  - add a checked-in JSON mapping-profile contract that is broad-model for the full fractal catalog but ships Explaino-first plus `all` aggregate mappings in v1
  - add a native synthesized-base seam that builds a minimal valid Explaino-family state from defaults plus resolved FITS-derived orientation signals
  - for FITS-driven Explaino import, generated synthesized base-state artifacts are first-class authority; existing loaded-state replay remains valid and unchanged for other workflows
  - use the existing runtime metadata / repo-root / `py -3.14` launch pattern for any repo-local FITS feature-extraction helper rather than inventing a second ad hoc process seam
- Expected generated artifacts under runtime-walk import sessions:
  - generated runtime-walk request JSON
  - generated synthesized base-state JSON when synth mode is used
  - FITS-derived orientation-input JSON
  - import selection manifest / receipt
  - recent/latest session index updates
- Minimum shipped mapping targets in v1:
  - combined Explaino seed
  - phase
  - seed drift
  - mix
  - warp strength
  - supported view transform controls
  - `all` aggregate profile using the same transport surface
- Non-goals for this slice:
  - no FITS frame-by-frame playback
  - no full 37-family specialized mappings in the first landing
  - no replacement of existing `state.json` replay for non-FITS workflows
  - no new closed-loop field-flow/spline redesign in this slice; that remains follow-on polish once the authority model is correct
- First hostile-audit targets:
  - synth mode accidentally falling back to stale recent/latest state sessions
  - invalid mapping profile silently ignored instead of failing fast
  - synthesized state not actually Explaino-family safe
  - viewer text still implying FITS is mere evidence in synth mode after the authority model changes
  - flashlight regressions from shared bootstrap extraction
  - gradient-flow overlay collapsing into only a few symmetric spokes instead of a denser local flow-map field

## Validation

- `ui_app\build_tests_vsdevcmd.cmd`
- `ui_app\build_vsdevcmd.cmd`
- focused native helper tests for synthesized runtime-walk bootstrap and import/session authority modes
- focused runtime pytest coverage for synthesized-session viewer playback
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
