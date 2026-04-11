# Code Quality Ratchet Recovery - 2026-04-11

## Current Phase

Complete - Code quality ratchet recovered and checkpointed

## Phase Checklist

- [x] Phase 1 - Reduce top audit offenders and remeasure
- [x] Phase 2 - Reduce secondary parser/binding offenders if audit still misses baseline band
- [x] Phase 3 - Revalidate repaired state and checkpoint

## Notes

- Trigger: user-directed demand to stop discussing the 88/100 regression and fix it.
- Scope: recover the code-quality ratchet toward the stored baseline band without changing the shipped feature set.
- Deterministic setup:
  - `py -3.14 tools/viewer_host_begin_work_slice.py --intent "code quality ratchet recovery from 88 to baseline band" --profile checkpoint`
  - `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json`
- Starting audit state:
  - score `88/100` vs stored baseline `95/100` with tolerance `2`
  - top current offenders include `WinMain()` in `ui_app/src/main.cpp` at 235 lines and `UpdateExplainoPolynomial()` in `ui_app/src/fractal_derived_fields.cpp` at 115 lines
- Phase 1 exit criteria:
  - reduce the largest current function-size regressions via bounded extractions
  - rerun the audit and use the measured result to decide whether a second extraction pass is still required
- Phase 2 candidate seams if needed after remeasurement:
  - `BindingContext::GetEnumId()` / `SetEnumId()` in `ui_app/src/schema_binding.cpp`
  - `ParseViewerCli()` in `ui_app/src/viewer_cli.cpp`
  - `TryParseFractalTypeArg()` in `ui_app/src/cli_args.cpp`
  - `ParseFractalType()` in `ui_app/src/diagnostics_state_io.cpp`

- Phase 1 completion snapshot:
  - extracted command-line dispatch, schema/default initialization, window/ImGui initialization, sidecar refresh, and in-loop capture handling from `WinMain()` in `ui_app/src/main.cpp`
  - extracted Explaino shape/root/polynomial helpers from `UpdateExplainoPolynomial()` in `ui_app/src/fractal_derived_fields.cpp`
  - first remeasurement improved the audit score from `88/100` to `93/100`, but the baseline check still failed on file-specific max-function regressions

- Phase 2 completion snapshot:
  - added shared enum-id helpers in `ui_app/src/enum_id_utils.h` and rewired duplicated enum/string parsing or formatting in `cli_args.cpp`, `diagnostics_state_io.cpp`, `finding_archive_actions.cpp`, `schema_binding.cpp`, and `fractal_probe_runner.cpp`
  - extracted sweep parsing from `ParseViewerCli()` and compacted the safe-mode fractal option construction and fractal family predicate surfaces
  - extracted sequence-results JSON emission from `SerializeFractalProbeResponseJson()`
  - second remeasurement improved the audit score to `96/100`, and `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json` passed the stored baseline check

- Phase 3 completion snapshot:
  - `ui_app/build_tests_vsdevcmd.cmd` passed with `All helper tests passed.`
  - `ui_app/build_vsdevcmd.cmd` passed and published `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`
  - `py -3.14 -m pytest tests/test_fractal_runtime_probe_cli.py tests/test_fractal_runtime_shutdown.py -q` passed with `13 passed in 0.77s`
