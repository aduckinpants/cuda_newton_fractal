# Coloring Mode Follow-Up Sprint Overview

## Current Phase

Phase 3 - slice 2 control-surface work is the next active step after slice 1 closure

## Phase Checklist

- [x] Phase 1 - create the durable planning and contract surfaces for the coloring-only thread
- [x] Phase 2 - land slice 1 for runtime split and legacy compatibility
- [ ] Phase 3 - land slice 2 for the control surface and state migration
- [ ] Phase 4 - land slice 3 for family tuning and render polish
- [ ] Phase 5 - run the integration audit and publish the thread back to master

## Explicit User Asks

- [open] Keep the next work separate from camera, view-preset, and responsiveness follow-ups.
- [open] Start implementation on the earlier simpler coloring outline rather than a DSL or Salticid-first path.
- [open] Reuse the last sprint's branch and slice pattern for this new thread.

## Presumption Loop

The likely failure mode is starting the next coloring work from ideas alone instead of durable owner seams. This overview therefore starts with the repo-local planning and contract surfaces, then moves into one compatibility-first runtime slice before any UI/schema migration.

Hostile review assumes the right first move is not a new language surface. The first slice should prove that viewer-host can carry an internal signal/palette/grading split while old `coloring_mode` state, defaults, and legality rules still behave deterministically.

## Presumption Evidence

- Owner Proof: the current shipped color authority already lives in `ui_app/src/fractal_types.h`, `ui_app/src/fractal_family_rules.h`, `ui_app/src/diagnostics_state_io.cpp`, `ui_app/src/fractal_renderer.cu`, and `ui_app/src/escape_time_coloring.h`.
- RED Witness: the first slice-1 regression failed immediately because the repo had no split-color runtime types or synthesized owner field; a second audit regression then proved preset/default application still left the new internal `color_pipeline` stale after `coloring_mode` was reset.
- Fix Proof: the overview plus three slice plans are now checked in, the slice-1 contract is validated and relocked on `feature/coloring-mode-foundation` under `ck:d8074a50`, and the first runtime pass now synthesizes split-color state for family defaults and legacy diagnostics state load.
- Hostile Review Pass 1: the first green build exposed a real defaulting gap in `fractal_derived_fields.cpp`, so the slice contract was widened, relocked, and repaired before continuing.
- Hostile Review Pass 2: a save-authority audit found no new runtime defect, but it added a characterization guard proving diagnostics capture still serializes legacy `coloring_mode` and does not leak the internal `color_pipeline` field during slice 1.

## Proof Ledger

- Manual RED: the coloring thread had no checked-in overview or slice surfaces at all before this Phase 1 foundation work.
- Checked-in regression RED: `ui_app\build_tests_vsdevcmd.cmd` first failed because the new split-color types, helpers, and `KernelParams::color_pipeline` field did not exist; the audit rerun then failed on a focused `test_fractal_derived_fields` regression proving preset defaults left the new field stale.
- First GREEN: the runtime owner seams now define split-color types plus legacy/default synthesis helpers, diagnostics state load populates the new field, and the native helper rail is green again.
- Post-green hostile finding: preset/default application still wrote only `coloring_mode`. That defect is now repaired in `fractal_derived_fields.cpp`, and the same native helper rail is green again on the repaired state.
- Second audit pass: grep and the save-authority characterization guard confirm there are no active runtime consumers of `color_pipeline` outside the synchronization seams, so slice 1 has not accidentally widened into a second live authority surface.

## Notes

- Planned branch topology:
  - `master`
  - `feature/coloring-mode-integration`
  - `feature/coloring-mode-foundation`
  - `feature/coloring-mode-control-surface`
  - `feature/coloring-mode-family-tuning`
- Planned slice sequence:
  - slice 1: `docs/notes/coloring_mode_followup_slice1_runtime_split_PHASED_PLAN.md`
  - slice 2: `docs/notes/coloring_mode_followup_slice2_control_surface_PHASED_PLAN.md`
  - slice 3: `docs/notes/coloring_mode_followup_slice3_family_tuning_PHASED_PLAN.md`
- Scope rules:
  - keep old `coloring_mode` load compatibility throughout this sprint
  - keep Salticid, DSL, AST, and dynamic kernel work out of this thread
  - keep camera/exploration, view presets, and responsiveness as separate later threads
- Phase 1 completion snapshot:
  - the overview plus the three slice plans are checked in
  - `docs/contracts/coloring_mode_followup_sprint_phase0_foundation.contract.json` validates cleanly
  - slice 1 is re-homed off `master` and locked on `feature/coloring-mode-foundation` under `ck:d8074a50`
- Phase 2 completion snapshot:
  - slice 1 now has two deliberate hostile-review passes: the preset/default synchronization repair and the save-authority characterization guard
  - `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/coloring_mode_slice1_code_quality.json` passed at 95/100
  - `py -3.14 tools/viewer_host_run_logged_command.py --label coloring-slice1-final-native --log artifacts/coloring_slice1_final_native.log -- cmd /c ui_app\build_tests_vsdevcmd.cmd` passed on the repaired state

## Resume Point

Phase 3 is next. Checkpoint slice 1 on `feature/coloring-mode-foundation`, merge it into `feature/coloring-mode-integration`, then create `feature/coloring-mode-control-surface` and lock the slice-2 contract before changing the schema/state surface.