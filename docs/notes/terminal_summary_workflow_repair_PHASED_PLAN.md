# Terminal Summary Workflow Repair

## Current Phase

Complete - Terminal summary workflow repair checkpointed

## Phase Checklist

- [x] Phase 1 - Add deterministic logged-command wrapper and focused tests
- [x] Phase 2 - Rewire long validation tasks to the wrapper
- [x] Phase 3 - Validate the repaired workflow on real commands
- [x] Phase 4 - Hostile re-audit and checkpoint prep

## Notes

- Trigger: repeated long-running validation commands completed successfully but `run_in_terminal` returned empty output, forcing manual artifact-log inspection.
- Scope:
  - repo-local workflow helper for long-running commands
  - focused Python tests for helper behavior
  - `.vscode/tasks.json` validation tasks that currently emit large/fragile output
- Phase 1 completion snapshot:
  - added `tools/viewer_host_run_logged_command.py` to run a command, write the full transcript to an artifact log, and print a deterministic short summary to stdout
  - added `tests/test_viewer_host_run_logged_command.py` to cover success and failure summaries with preserved exit codes
- Phase 2 completion snapshot:
  - rewired long-running validation tasks in `.vscode/tasks.json` to use the logged-command helper for native helper tests, runtime publish, runtime probe/session pytest, and catalog smoke
  - documented direct helper usage in `AGENT_WORKING_PROTOCOL.md` so future agents have an explicit repo-local workaround instead of rediscovering the wrapper failure pattern
- Phase 3 completion snapshot:
  - focused helper tests passed via `py -3.14 -m pytest tests/test_viewer_host_run_logged_command.py -q`
  - real-command smoke passed via `py -3.14 tools/viewer_host_run_logged_command.py --label "code quality audit smoke" --log artifacts/verify_code_quality_audit_smoke.log -- py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/verify_code_quality_audit_smoke_report.json`
  - long-build smoke passed via `py -3.14 tools/viewer_host_run_logged_command.py --label "native helper tests smoke" --log artifacts/verify_native_helper_tests_smoke.log -- cmd /c ui_app\build_tests_vsdevcmd.cmd`, with a concise stdout summary and log tail ending in `All helper tests passed.`
- Phase 4 completion snapshot:
  - hostile audit found one real workflow defect in the first helper implementation: missing executables raised a traceback instead of producing the same deterministic summary contract
  - added a focused regression for missing executables and hardened `tools/viewer_host_run_logged_command.py` to log launch failures and return a deterministic nonzero status instead of crashing
  - reran `py -3.14 -m pytest tests/test_viewer_host_run_logged_command.py -q` to green (`3 passed`)
  - validated the hardened failure path via `artifacts/verify_missing_executable_smoke.log`
  - phased-plan sync passed via `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- Exit criteria:
  - long validation commands have a checked-in wrapper that writes full output to an artifact log and prints a deterministic short summary to stdout
  - public validation tasks for long-running rails use that wrapper
  - the helper is covered by focused tests and validated on at least one real repo command
