# Explaino Sidecar Direct Mutation

## Current Phase

Phase 3 - persistence and capture integration

## Phase Checklist

- [x] Phase 1 - explicit policy controls and single-step apply
- [x] Phase 2 - paced autonomous loop
- [ ] Phase 3 - persistence and capture integration

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
- Validation achieved for Phase 2:
	- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/phase2_paced_loop_code_quality.json`
	- `py -3.14 tools/viewer_host_run_logged_command.py --label phase2-native-after-audit-fix --log artifacts/phase2_paced_loop_build_tests.log -- ui_app\build_tests_vsdevcmd.cmd`
	- `py -3.14 tools/viewer_host_run_logged_command.py --label phase2-runtime-final --log artifacts/phase2_paced_loop_runtime_build.log -- ui_app\build_vsdevcmd.cmd`
	- `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe --validate-ui`
	- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- Deferred to later phases:
	- persistence or replay of controller policy / mutation traces
	- headless capture or diagnostics surfaces for explicit mutation actions