# CLI Bridge V2 Named-Pipe Transport

## Current Phase

Complete - Windows named-pipe transport landed, validated, and ready to checkpoint

## Phase Checklist

- [x] Phase 1 - Define the V2-G transport surface and add focused failing tests
- [x] Phase 2 - Implement Windows named-pipe session transport on top of the existing line protocol
- [x] Phase 3 - Revalidate the repaired state, hostile-audit the slice, and checkpoint

## Notes

- Trigger: `spec_intake/_STATUS.md` marks CLI Bridge V2 alternate transport as the next protocol-only unblocked slice.
- Scope: land a Windows named-pipe transport for session mode without changing request/response semantics or widening the protocol beyond V2-G.
- Exit criteria:
  - CLI parsing exposes a bounded named-pipe session surface.
  - The runtime serves the existing one-line JSON session protocol over a Windows named pipe.
  - Focused native tests cover the CLI and headless transport helpers.
  - Focused runtime tests prove a client can open the pipe, exchange session lines, and close cleanly.
- Non-goals for this slice:
  - no socket transport yet
  - no binary protocol
  - no changes to V1 single-request sample mode
- Deterministic setup:
  - `py -3.14 tools/viewer_host_begin_work_slice.py --intent "V2-G named-pipe session transport" --profile runtime`
  - `ui_app/build_tests_vsdevcmd.cmd`
  - `ui_app/build_vsdevcmd.cmd`
  - `py -3.14 -m pytest tests/test_fractal_runtime_session.py -q`
- Phase 1 completion snapshot:
  - added parser coverage for `--sample-session-pipe <name>` in `ui_app/tests/test_viewer_cli.cpp`
  - added focused native transport tests in `ui_app/tests/test_headless_modes.cpp`
  - added a runtime named-pipe round-trip test in `tests/test_fractal_runtime_session.py`
- Phase 2 completion snapshot:
  - added a bounded `sample_session_pipe_name` CLI surface and dispatch helper in `ui_app/src/viewer_cli.*` and `ui_app/src/main.cpp`
  - implemented strict pipe-name normalization plus a Windows named-pipe session loop in `ui_app/src/headless_modes.*`
- Phase 3 audit snapshot so far:
  - hostile review found a real race in the new native named-pipe test helper; fixed `ConnectSessionPipe()` to retry until the server actually creates the pipe instead of relying on a single `WaitNamedPipeA()` call
  - hostile review found a real structural splice error in `tests/test_fractal_runtime_session.py`; restored the existing NDJSON test body and moved the new named-pipe class to the end of the file
  - hostile re-audit found a continuity defect in the surviving V2-G spec text: the implementation is a bounded Windows named-pipe transport for one external client/session per process, but the spec still advertised concurrent callers/external consumers; narrowed the spec to the shipped scope and recorded broader multi-client/socket transport as deferred
  - revalidated focused seams with `D:\salt-fractal\cuda_newton_fractal_clone\build_tests\test_headless_modes.exe`, `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json`, `ui_app/build_vsdevcmd.cmd`, and `py -3.14 -m pytest tests/test_fractal_runtime_session.py -q`
  - canonical native helper validation completed with `ui_app/build_tests_vsdevcmd.cmd` ending in `All helper tests passed.` after the repaired test helper rerun
