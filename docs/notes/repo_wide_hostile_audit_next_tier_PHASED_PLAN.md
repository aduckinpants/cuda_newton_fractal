# Repo-Wide Hostile Audit - Next-Tier Hotspots

## Current Phase

Complete - Next-tier audit batch checkpointed

## Phase Checklist

- [x] Phase 1 - Next-tier hotspot selection and direct code review
- [x] Phase 2 - Regression expansion for confirmed defects
- [x] Phase 3 - Minimal repairs with no-implicit-fallback enforcement
- [x] Phase 4 - Revalidation, hostile re-audit, and checkpoint prep

## Notes

- Trigger: continue the repo-wide hostile audit after the first hotspot batch checkpoint.
- Scope: `ui_app/src/viewer_cli.cpp` and `ui_app/src/diagnostics_state_io.cpp`, with their focused native tests.
- Hotspot selection basis:
  - next-tier file candidates recorded in the first hostile-audit plan
  - direct hostile review of CLI parsing and diagnostics/finding-state load seams

- Phase 1 completion snapshot:
  - audited `viewer_cli.cpp`, `diagnostics_state_io.cpp`, `finding_archive_actions.cpp`, and adjacent tests
  - promoted real defects only from `viewer_cli.cpp` and `diagnostics_state_io.cpp`
  - deferred non-blocking findings outside this batch rather than widening scope silently

- Confirmed defects for this batch:
  - CLI sweep config accepted zero/negative step values and non-ascending bounds
  - diagnostics state loading accepted `max_iter <= 0`
  - diagnostics state loading accepted non-positive render width/height
  - `finding.json` state references could escape the finding directory or point at `.`
  - optional `transcendental_func`, `momentum_beta`, and `mcmullen_preset` fields failed open on bad types or unknown ids

- Phase 2 completion snapshot:
  - added focused viewer CLI regressions for zero step, negative step, equal bounds, and descending bounds
  - added diagnostics-state regressions for zero `max_iter`, invalid dimensions, traversal attempts, directory references, and optional-field fail-fast behavior

- Phase 3 completion snapshot:
  - `ParseViewerCli(...)` now rejects invalid sweep ranges before enabling sweep mode
  - `LoadDiagnosticsStateJson(...)` now rejects non-positive `max_iter`, width, and height
  - `ResolveFindingStateJsonPath(...)` now rejects absolute, escaping, and directory-valued `state_file` references
  - optional diagnostics fields now fail fast on bad types or unknown enum ids instead of silently inheriting defaults

- Phase 4 completion snapshot:
  - focused native regressions passed:
    - `test_viewer_cli.exe`
    - `test_diagnostics_state_io.exe`
  - full helper rail passed via `ui_app/build_tests_vsdevcmd.cmd`
  - runtime publish passed via `ui_app/build_vsdevcmd.cmd`
  - hostile re-audit of the final touched seams found no blocking defect
  - post-repair code quality audit refreshed `artifacts/code_quality_report.json` with score `88/100`

- Residual non-blocking risks noted during final audit:
  - diagnostics-state tests now lock negative paths but still do not assert successful round-trip for valid `transcendental_func` and `mcmullen_preset` values
  - code quality score dropped from `89` to `88` because the extra validation logic pushed `ParseViewerCli()` and `diagnostics_state_io.cpp` size metrics slightly upward; no new behavioral regression was found

- Likely next hostile-audit candidates after this checkpoint:
  - `ui_app/src/finding_archive_actions.cpp`
  - `ui_app/src/viewer_cli.cpp` dwell/range-adjacent validation follow-up only if a concrete defect is observed
  - `ui_app/src/cli_args.cpp`