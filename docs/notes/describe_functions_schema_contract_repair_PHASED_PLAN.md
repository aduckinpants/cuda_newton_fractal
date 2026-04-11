# Describe-Functions Schema Contract Repair

## Current Phase

Complete - Describe-functions schema contract repair checkpointed

## Phase Checklist

- [x] Phase 1 - Red regressions and bounded helper extraction
- [x] Phase 2 - Safe-mode parity and schema-candidate repair
- [x] Phase 3 - Public validation rail repair and full revalidation
- [x] Phase 4 - Hostile re-audit and checkpoint prep

## Notes

- Trigger: repo-wide hostile review confirmed three real defects in the describe-functions/schema contract.
- Scope:
  - `ui_app/src/headless_modes.cpp`
  - `ui_app/src/viewer_schema_load.h/.cpp`
  - `ui_app/src/main.cpp`
  - focused native/runtime tests and the public VS Code runtime validation rail
- Confirmed defects to repair in this slice:
  - `--describe-functions` bypasses validated schema loading and can advertise parameters from a schema the viewer would safe-mode away.
  - runtime schema candidate resolution still includes a current-working-directory relative fallback.
  - the public runtime verification profile no longer exercises the describe-functions runtime tests named by the checked-in spec.
- Phase 1 completion snapshot:
  - added a focused headless regression proving `RunDescribeFunctionsMode(...)` must not leak an invalid schema binding into the advertised catalog surface
  - added a focused schema-load regression proving viewer schema candidates stay exe-relative and do not include the old cwd-relative fallback
  - red state was confirmed on the checked-in helper rail via `artifacts/describe_schema_red_build_tests.log`
- Phase 2 completion snapshot:
  - extracted `BuildViewerSchemaCandidates(...)` into `viewer_schema_load.{h,cpp}` so schema path resolution lives next to the schema load contract instead of inside `main.cpp`
  - removed the old plain `..\ui\...` cwd-relative schema fallback from runtime candidate resolution
  - rewired `RunDescribeFunctionsMode(...)` to use `LoadAndValidateViewerSchema(...)` so invalid bindings now safe-mode instead of advertising impossible parameters
- Phase 3 completion snapshot:
  - widened `.vscode/tasks.json` runtime pytest coverage to include `tests/test_function_descriptor_cli.py` and `tests/test_generic_probe_cli.py`
  - validated the repaired native/helper seam via `ui_app/build_tests_vsdevcmd.cmd` with log at `artifacts/describe_schema_full_build_tests.log`
  - validated runtime publish via `ui_app/build_vsdevcmd.cmd` with log at `artifacts/describe_schema_runtime_build.log`
  - validated the widened runtime Python lane via `py -3.14 -m pytest tests/test_fractal_runtime_probe_cli.py tests/test_fractal_runtime_session.py tests/test_function_descriptor_cli.py tests/test_generic_probe_cli.py -q` with log at `artifacts/describe_schema_runtime_pytests.log` (`47 passed`)
- Phase 4 completion snapshot:
  - phased-plan sync passed via `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
  - code-quality baseline audit passed at `96/100` via `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/describe_schema_code_quality_report.json`
  - hostile re-audit of `viewer_schema_load.cpp`, `headless_modes.cpp`, `main.cpp`, the new focused tests, and the widened runtime task surface did not find another blocking defect
- Exit criteria:
  - describe-functions uses the same validated schema load contract as the viewer.
  - published-runtime schema lookup is deterministic and no longer depends on cwd-relative fallback.
  - the public runtime validation profile covers the describe-functions runtime tests.
