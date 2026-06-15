# Color Root Pattern Authority Repair

## Current Phase

Complete - closure validation and hostile audit passed.

## Explicit User Asks

- Fix the Color Pipeline `root_proximity` and `root_phase` sources binding to the wrong ExplainO root system in older presets.
- Restore smooth auto-seed animation behavior instead of color flicker from a mismatched root field.
- Keep this as a bounded repair; do not reopen graph UI, SDF, or broader root-pattern UX work.

## Phase Checklist

- [x] Phase 0 - Branch from clean rearward-OK head and open this repair plan/contract.
- [x] Phase 1 - Add RED/native proof that root-aware color rows must default/import to the active dynamics root field for legacy/older preset paths.
- [x] Phase 2 - Repair the root-aware import/application seam without removing explicit color-root-field support for advanced states.
- [x] Phase 3 - Add no-mouse runtime proof that auto seed changes no longer leave `root_phase` / `root_proximity` bound to the wrong root field.
- [x] Phase 4 - Hostile review, validation receipts, rearward review, push, and stop.

## Proof Ledger

- Rearward review before branch: `py -3.14 tools/viewer_host_rearward_review.py`, status `ok` for `16886abf8ad4504ab5bccc7d8198d0ca21924428`.
- Suspect seam: `ImportSupportedColorPipelineParamsFromLive(...)` currently imports `signal.root_pattern_ref` for `root_phase` and `root_proximity` from `KernelParams::explaino_root_field_pattern_ref`, which is a root-field consumer ref and can be stale or wrong for older Color Pipeline preset paths.
- Native proof: `py -3.14 tools/viewer_host_run_logged_command.py --label color_root_pattern_authority_repair_native ... -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core test_color_pipeline_window`, success; `test_color_pipeline_core` passed 3329 checks and `test_color_pipeline_window` passed 433 checks.
- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label color_root_pattern_authority_repair_runtime_publish ... -- ui_app/build_vsdevcmd.cmd`, success; active runtime staged at `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Runtime proof: `py -3.14 -m pytest tests/test_fractal_runtime_color_pipeline_presets.py::test_color_pipeline_root_pattern_authority_no_mouse -q --junitxml artifacts/pytest/color_root_pattern_authority_repair_runtime.junit.xml`, success.

## Hostile Audit

- Status: done
- Did the fix cover both `root_proximity` and `root_phase`?
- Did older/legacy preset paths default to the dynamics root field instead of a stale color root field?
- Did explicit Color Root Field selection remain possible for advanced/root-aware rows?
- Did the no-mouse proof exercise the same visible/published runtime path the user can hit?
- Did this repair avoid graph UI, SDF, and new fractal work?

## Audit Passes

- [x] Pass 1: Found the exact root-aware import defect. `root_phase` and `root_proximity` imported `signal.root_pattern_ref` from `KernelParams::explaino_root_field_pattern_ref`, which is a root-field consumer selector rather than row-owned Color Pipeline authority.
- [x] Pass 2: Clean re-read of the repaired state confirmed existing source-stack rows with `root_pattern_ref=color_root_field` still load and remain explicit; the repair only stops accidental import/default leakage.
- [x] Pass 3: Clean re-read of the repaired state found no graph UI, SDF behavior, new fractal lane, or Salticid dependency change.

## Audit Findings

- [x] Finding 1: Root-aware Color Pipeline rows could inherit the wrong scoped ExplainO root field from a live root-field consumer ref. Repaired by leaving `signal.root_pattern_ref` row-owned during live-param import while still importing numeric phase/proximity controls.
