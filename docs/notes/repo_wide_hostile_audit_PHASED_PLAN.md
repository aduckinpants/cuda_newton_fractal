# Repo-Wide Hostile Audit

## Current Phase

Complete - First hotspot audit batch checkpointed

## Phase Checklist

- [x] Phase 1 - Deterministic signal collection and hotspot inventory
- [x] Phase 2 - Hostile hotspot review and defect confirmation
- [x] Phase 3 - Focused repairs with forward regressions
- [x] Phase 4 - Repo-wide revalidation and closure

## Notes

- Trigger: user-directed full distrust-first redo after repeated bug escapes.
- Scope: audit the repo from deterministic rails first, then work highest-risk seams down by confirmed defects rather than broad speculative rewrites.
- Required rails:
  - `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json`
  - `ui_app/build_tests_vsdevcmd.cmd`
  - `ui_app/build_vsdevcmd.cmd`
  - targeted Python/runtime rails as findings require

- Phase 1 exit criteria:
  - current code-quality baseline state captured
  - highest-risk files and seams identified from deterministic signals
  - audit order recorded in-repo

- Phase 1 completion snapshot:
  - `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json`
  - baseline check failed at score `89/100` versus stored baseline `95`
  - highest-risk seams recorded from deterministic signals before review work started

- Initial hotspot order:
  - `ui_app/src/main.cpp`
  - `ui_app/src/schema_binding.cpp`
  - `ui_app/src/fractal_derived_fields.cpp`
  - `ui_app/src/fractal_probe_runner.cpp`
  - `ui_app/src/ui_schema.cpp`

- Phase 2 exit criteria:
  - each hotspot reviewed with a distrust-first pass
  - only real defects or missing regressions promoted to repair work
  - false positives and residual risks recorded explicitly

- Phase 2 completion snapshot:
  - promoted real defects from `schema_binding.cpp`, `fractal_derived_fields.cpp`, `fractal_probe_runner.cpp`, and the `main.cpp` finding-capture/sidecar seam
  - reviewed `ui_schema.cpp` as part of schema-surface triage and did not promote a separate blocking defect in this batch
  - rejected one suspected Explaino seed-normalization issue as a false positive after verifying `UpdateExplainoPolynomial(...)` already routes through `ExplainoSeedCombined(...)` and existing regression coverage

- Phase 3 exit criteria:
  - every accepted defect has a focused regression first
  - repairs are minimal and extracted where practical
  - no unrelated cleanup mixed into bug-fix slices

- Phase 3 completion snapshot:
  - added regressions for view/render/lens-only bool binding, invalid `visible_if`, invalid int-combo ids, stale `poly_coeffs_b`, Explaino-Y nonfinite status preservation, and capture-time derived-state prep
  - fixed schema fail-open behavior in `ui_app/src/schema_binding.cpp`
  - cleared stale secondary Explaino splice coefficients in `ui_app/src/fractal_derived_fields.cpp`
  - preserved `FractalProbeSampleStatus::nonfinite` in `ui_app/src/fractal_probe_runner.cpp`
  - extracted `ui_app/src/finding_capture_state.{h,cpp}` and invalidated sidecar + budget caches after finding capture mutates derived runtime state

- Phase 4 exit criteria:
  - relevant rails rerun on repaired state
  - hostile re-audit finds no further blocking defects in touched seams
  - handoff and checkpoint state updated

- Phase 4 progress snapshot:
  - focused native validations passed:
    - `test_schema_binding.exe`
    - `test_fractal_derived_fields.exe`
    - `test_fractal_probe.exe`
    - `test_ui_schema.exe`
    - `test_viewer_schema_load.exe`
    - `test_fractal_sample_pipeline.exe`
    - `test_fractal_probe_coverage.exe`
    - `test_finding_capture_state.exe`
    - `test_finding_archive_actions.exe`
  - runtime publish rerun passed via `ui_app/build_vsdevcmd.cmd`
  - hostile re-audit of the repaired seams found no further blocking defect in the touched surfaces
  - checkpoint commit: `ac16714`
  - handoff updated against the checkpointed batch

- Next-tier audit candidates after this checkpoint:
  - `ui_app/src/diagnostics_state_io.cpp`
  - `ui_app/src/finding_archive_actions.cpp`
  - `ui_app/src/viewer_cli.cpp`
  - `ui_app/src/cli_args.cpp`