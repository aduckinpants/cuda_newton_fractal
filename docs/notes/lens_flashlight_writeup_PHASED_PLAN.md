# Lens And Flashlight Writeup

## Current Phase

Complete

## Phase Checklist

- [x] Phase 1 - Trace runtime lens SDF implementation and call path
- [x] Phase 2 - Trace Explaino sidecar lens/action/controller path and resolve the likely "flashlight" meaning
- [x] Phase 3 - Write and verify checked-in documentation

## Notes

- User request for the original thread:
  - produce a clear detailed writeup of exactly how the `lens` SDF and flashlight features work
- Historical terminology note for that thread:
  - there was no checked-in symbol or UI surface literally named `flashlight`
  - that earlier writeup therefore treated "flashlight" as the Explaino sidecar guidance/action/controller path because it was the only matching feature family in the branch at the time
- Primary code seams used for the original descriptive writeup:
  - `ui_app/src/fractal_renderer.cu`
  - `ui_app/src/fractal_family_rules.h`
  - `ui_app/src/lens_sdf.cpp`
  - `ui_app/src/main.cpp`
  - `ui_app/src/explaino_sidecar_lens.cpp`
  - `ui_app/src/explaino_sidecar_energy.cpp`
  - `ui_app/src/explaino_sidecar_action.cpp`
  - `ui_app/src/explaino_sidecar_controller.cpp`
- Historical verification target:
  - phased-plan sync
  - `git diff --check`
- Historical note:
  - this file is preserved as the original descriptive-plan surface only
  - the active thread is now `docs/notes/lens_sdf_report_then_modernization_PHASED_PLAN.md`
  - the current evidence-backed report is `docs/notes/lens_and_flashlight_writeup.md`
