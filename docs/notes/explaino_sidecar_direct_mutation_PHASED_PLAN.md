# Explaino Sidecar Direct Mutation

## Current Phase

Complete - direct-mutation persistence slice closed; use a new follow-on plan for any further replay or mutation-history work

## Phase Checklist

- [x] Phase 1 - explicit policy controls and single-step apply
- [x] Phase 2 - paced autonomous loop
- [x] Phase 3 - persistence and capture integration

## Notes

- Spec source: spec_intake/ExplainoAll_SmartSidecar_SpecIntake.md (R3-C auto-demonstration mode)
- This follow-on plan deliberately starts with the smallest safe runtime-mutation seam:
	- expose the controller policy in the sidecar UI
	- allow a single explicit apply of the currently armed controller decision
	- keep the continuous loop deferred until mutation semantics are proven safe
- Exit criteria for Phase 1:
	- sidecar UI exposes enablement, mutation opt-in, and stop-threshold policy controls
	- an armed controller decision can mutate the bound runtime path exactly once when explicitly requested
	- focused native coverage proves the mutation helper updates float, double, and int paths and rejects unsupported or targetless decisions
	- runtime build stays green after wiring the new sidecar interaction seam
- Delivered in Phase 1:
	- `ui_app/src/explaino_sidecar_controller.h/.cpp`
	- `ui_app/src/explaino_sidecar_window.h/.cpp`
	- `ui_app/src/main.cpp`
	- `ui_app/tests/test_explaino_sidecar_controller.cpp`
	- `ui_app/build_tests_vsdevcmd.cmd`
	- the sidecar window now exposes explicit auto-demonstration policy controls for enablement, mutation opt-in, and stop thresholds
	- the viewer can apply one armed controller recommendation explicitly through the sidecar UI without enabling a free-running mutation loop
	- the controller mutation helper now supports float, double, and int binding paths and fails fast on targetless or unsupported decisions
- Audit repair after the initial Phase 1 close:
	- explicit sidecar apply now requires an armed mutation decision instead of bypassing proposal-only controller states
	- the mutation helper now fails fast on declared type/path drift instead of silently falling back across numeric binders
	- the sidecar window no longer advertises an apply button while the controller remains proposal-only
- Validation achieved for Phase 1:
	- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json`
	- `ui_app/build_tests_vsdevcmd.cmd`
	- `ui_app/build_vsdevcmd.cmd`
	- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- Validation achieved for the audit repair:
	- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/sidecar_audit_fix_code_quality.json`
	- `ui_app/build_tests_vsdevcmd.cmd`
	- `ui_app/build_vsdevcmd.cmd`
	- `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe --validate-ui`
- Phase 2 target:
	- add a separate paced-loop opt-in instead of overloading the existing mutation opt-in
	- add a dwell/interval control so autonomous mutation happens at a visible, bounded cadence
	- keep the pacing decision in a headless controller seam rather than pushing more state logic into `main.cpp`
	- reset the paced-loop timer on explicit user interaction and on controller-decision changes so a new armed recommendation does not fire immediately
	- prove the loop behavior with focused native tests before wiring it into the viewer runtime
- Delivered in Phase 2:
	- `ui_app/src/explaino_sidecar_controller.h/.cpp`
	- `ui_app/src/explaino_sidecar_window.cpp`
	- `ui_app/src/main.cpp`
	- `ui_app/tests/test_explaino_sidecar_controller.cpp`
	- the controller policy now separates explicit mutation opt-in from a paced autonomous-loop opt-in and dwell interval
	- the paced-loop timer lives in the controller seam and only auto-applies armed decisions after the configured dwell interval
	- the loop timer resets on explicit user interaction, on controller-decision changes, and after any successful sidecar mutation so a prior dwell window cannot leak into the next step
	- `main.cpp` now delegates the sidecar render/loop/apply block to a focused helper instead of expanding `WinMain()` further
- Audit repair after the initial Phase 2 implementation:
	- preserved the current frame's dwell time when a newly armed decision first appears so the loop fires on the configured cadence instead of one tick late
	- reset the paced-loop state after manual applies so a previous dwell window cannot make the next autonomous step fire early
	- repaired the temporary `WinMain()` line-count ratchet regression by extracting the per-frame sidecar auto-demo block into a dedicated helper
	- repaired the cached sidecar refresh gate so controller-policy changes now rebuild the sidecar state even when the fractal runtime itself is otherwise clean
- Validation achieved for Phase 2:
	- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/phase2_paced_loop_code_quality.json`
	- `py -3.14 tools/viewer_host_run_logged_command.py --label phase2-native-after-audit-fix --log artifacts/phase2_paced_loop_build_tests.log -- ui_app\build_tests_vsdevcmd.cmd`
	- `py -3.14 tools/viewer_host_run_logged_command.py --label phase2-runtime-final --log artifacts/phase2_paced_loop_runtime_build.log -- ui_app\build_vsdevcmd.cmd`
	- `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe --validate-ui`
	- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- Validation achieved for the control-surface repair:
	- `ui_app/build_tests_vsdevcmd.cmd`
	- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json`
	- `ui_app/build_vsdevcmd.cmd`
	- `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe --validate-ui`
- Phase 3 target:
	- persist the sidecar auto-demo controller policy through the existing diagnostics `state.json` contract anywhere `sidecar_orientation` already round-trips
	- keep the persisted state on the existing diagnostics/finding/load-state seam instead of inventing a second sidecar-state authority
	- preserve atomic load behavior so malformed persisted policy blocks fail without partially mutating the runtime
- Delivered in Phase 3:
	- `ui_app/src/diagnostics_capture.h/.cpp`
	- `ui_app/src/diagnostics_state_io.h/.cpp`
	- `ui_app/src/finding_archive_actions.h/.cpp`
	- `ui_app/src/finding_state_actions.h/.cpp`
	- `ui_app/src/viewer_state_init.h/.cpp`
	- `ui_app/src/main.cpp`
	- `ui_app/tests/test_diagnostics_state_io.cpp`
	- `ui_app/tests/test_finding_state_actions.cpp`
	- `ui_app/tests/test_viewer_state_init.cpp`
	- `tests/test_fractal_runtime_explaino_escape_variants.py`
	- diagnostics and finding captures now persist `sidecar_auto_demo_policy` alongside `sidecar_orientation`
	- CLI `--load-state-json` startup, in-app state/finding load, headless captures, and archived finding captures now all thread the persisted controller policy through the same `state.json` seam
	- legacy states without a persisted controller policy now reset the live controller policy to defaults instead of leaking stale in-memory settings across loads
	- diagnostics-state integer fields now fail fast on fractional JSON numbers instead of silently truncating during load
- Validation achieved for Phase 3:
	- `py -3.14 tools/viewer_host_run_logged_command.py --label phase3-native-post-audit --log artifacts/phase3_native_post_audit.log -- .\ui_app\build_tests_vsdevcmd.cmd`
	- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json`
	- `py -3.14 -m pytest tests/test_fractal_runtime_explaino_escape_variants.py -q`
	- targeted published-runtime republish of changed C++ units against the existing CUDA objects via the temporary `phase3_publish_cpp_only.cmd` workaround after `ui_app/build_vsdevcmd.cmd` hung without reaching `nvcc`/`cl` output in this session
- Deferred to later phases:
	- mutation-trace persistence or replay beyond the controller policy snapshot
	- dedicated diagnostics or archive surfaces for explicit mutation-action history
	- a checked-in reliable C++-only publish helper or a repaired `ui_app/build_vsdevcmd.cmd` path so future non-CUDA slices do not need the temporary targeted republish workaround