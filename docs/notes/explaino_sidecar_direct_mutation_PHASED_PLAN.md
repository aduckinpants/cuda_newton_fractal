# Explaino Sidecar Direct Mutation

## Current Phase

Complete - Phase 1 explicit policy controls and single-step apply with audit repair

## Phase Checklist

- [x] Phase 1 - explicit policy controls and single-step apply
- [ ] Phase 2 - paced autonomous loop
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
- Deferred to later phases:
	- paced autonomous mutation loops
	- persistence or replay of controller policy / mutation traces
	- headless capture or diagnostics surfaces for explicit mutation actions