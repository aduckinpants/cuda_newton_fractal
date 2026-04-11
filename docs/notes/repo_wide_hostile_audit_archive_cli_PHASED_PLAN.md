# Repo-Wide Hostile Audit - Archive And CLI Seams

## Current Phase

Complete - Archive and CLI audit batch checkpointed

## Phase Checklist

- [x] Phase 1 - Archive and CLI seam review
- [x] Phase 2 - Regression expansion for confirmed defects
- [x] Phase 3 - Minimal repairs and fail-fast hardening
- [x] Phase 4 - Revalidation, hostile re-audit, and checkpoint prep

## Notes

- Trigger: continue the repo-wide hostile audit after the next-tier CLI and diagnostics-state checkpoint.
- Scope:
  - `ui_app/src/cli_args.cpp`
  - `ui_app/src/finding_archive_actions.cpp`
  - adjacent focused tests

- Phase 1 completion snapshot:
  - hostile review confirmed real defects rather than speculative cleanup
  - promoted findings:
    - `TryParseDoubleArg(...)` fails open on `ERANGE` overflow/underflow
    - `TryParseIntArg(...)` ignores `ERANGE` and `int` bounds
    - `TryParseDoubleArg(...)` accepts empty strings and nonfinite values such as `inf`/`nan`
  - lower-priority follow-ups noted but not yet promoted:
    - temp-file deletion visibility in archive script failures
  - false positive rejected after direct regression:
    - `AppendCommandLineArg(...)` already round-trips trailing backslashes correctly under `CommandLineToArgvW`

- Phase 2 exit criteria:
  - focused regressions added for promoted defects
  - red state proven before implementation

- Phase 2 completion snapshot:
  - added CLI regressions for double overflow, underflow, empty string, `inf`, `nan`, int overflow, empty int values, and null `TryParseFractalTypeArg(...)` output pointers
  - added an archive command-line round-trip regression for quoted trailing-backslash paths to verify the suspected quoting defect directly
  - red state confirmed only in `test_cli_args.exe`; `test_finding_archive_actions.exe` stayed green and closed the quoting suspicion as a false positive

- Phase 3 exit criteria:
  - CLI numeric parsing rejects out-of-range values explicitly
  - Windows command-line quoting round-trips trailing backslashes correctly
  - no unrelated archive/CLI cleanup mixed into the slice

- Phase 3 completion snapshot:
  - `TryParseDoubleArg(...)` now rejects empty strings, `ERANGE`, and nonfinite results
  - `TryParseIntArg(...)` now rejects empty strings, `ERANGE`, and `int` overflow/underflow
  - `TryParseFractalTypeArg(...)` now rejects null output pointers explicitly
  - no production change was needed in `finding_archive_actions.cpp` because the promoted quoting suspicion did not reproduce under regression

- Phase 4 exit criteria:
  - focused binaries pass
  - full helper and runtime rails pass
  - hostile re-audit finds no blocking defect in touched seams

- Phase 4 completion snapshot:
  - focused binaries passed:
    - `test_cli_args.exe`
    - `test_finding_archive_actions.exe`
  - full helper rail passed via `ui_app/build_tests_vsdevcmd.cmd`
  - runtime publish passed via `ui_app/build_vsdevcmd.cmd`
  - hostile re-audit found no blocking defect in `cli_args.cpp`, `test_cli_args.cpp`, or `test_finding_archive_actions.cpp`
  - post-repair code quality audit remained `88/100`