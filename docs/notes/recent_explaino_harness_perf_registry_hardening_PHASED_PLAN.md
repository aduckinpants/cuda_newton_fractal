# Recent Explaino Harness / Perf / Registry Hardening

## Current Phase

Phase 6 - closed. Native, runtime, Color Pipeline, code-quality, contract, plan-sync, hostile-audit, checkpoint, receipts, push, clean tree, and stale-plan proof are recorded below or by the machine closeout receipts for this committed slice.

## Explicit User Asks

- [closed] Fix the recent Explaino / Explaino-all work without broadening into unrelated fractal features.
- [closed] Replace first-party physical mouse usage with trustworthy in-process automation.
- [closed] Protect Color Pipeline behavior while changing shared automation/report harness code.
- [closed] Replace weak smooth-escape performance proof with a renderer/runtime-owned witness instead of subprocess wall-clock timing.
- [closed] Make Explaino-all axis authority registry-driven across host tests, runtime proof, and device-active-axis behavior.
- [closed] Stop only after focused validation, hostile audit, checkpoint commit, receipts, push, clean tree, and stale-plan cleanup.

## Phase Checklist

- [x] Phase 1 - add REDs for visible schema int/double set-value automation, implement the minimal harness fix, and prove Explaino/Color no-mouse tests still pass.
- [x] Phase 2 - replace the smooth-escape perf witness with renderer/runtime-owned timing and backend/strategy evidence.
- [x] Phase 3 - tighten registry/device parity so every `kExplainoAxisRegistry` axis remains active on canonical `explaino_all` and owner lanes.
- [x] Phase 4 - harden runtime automation report JSON escaping for requested ids, validation messages, lanes, functions, and control ids.
- [x] Phase 5 - repair or explicitly classify the code-quality profile regressions blocking public validation profiles.
- [x] Phase 6 - run final native/runtime/Color/Explaino proof, hostile audit, checkpoint, receipts, push, clean tree, and stale-plan gate.

## Proof Ledger

- Bootstrap proof: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` reports `master`, `HEAD=11cf1e5`, clean tree, prior active contract `explaino_all_standalone_balance_void_axis_effect_repair`, and a red code-quality baseline.
- Repo status proof: `py -3.14 tools/viewer_host_repo_status.py` reports no staged, unstaged, or untracked files before this slice starts.
- Review finding to reproduce first: a visible `slider_int` schema control such as `fractal_control.width.primary` is exported in the automation report but a `--ui-automation-set-control-value` request is not consumed and does not report an error.
- Review finding to protect: Color Pipeline set-value/click automation must remain in-process and keep the smooth-escape ramp source scale, draft/live bridge, enabled/remove rows, and backed rows green.
- Review finding to improve: the current smooth-escape timing proof uses subprocess wall time, which is not a trustworthy render-performance witness.
- Review finding to improve: Explaino axis registry authority is not fully shared with the CUDA device active-axis checks.
- Review finding to improve: automation report JSON still has raw dynamic string fields.
- Review finding to improve: public validation profiles still fail the code-quality baseline audit.
- Phase 1 RED native: `artifacts/recent_explaino_phase1_red_native_schema_binding.log` fails at `test_schema_binding` with `Visible schema-driven set-value automation should apply through the int edit path`.
- Phase 1 RED runtime: `artifacts/recent_explaino_phase1_red_runtime_no_mouse_schema_set_value.log` fails the new visible width set-value and unknown set-value fail-closed runtime tests against the pre-fix published viewer.
- Phase 1 fix: `ui_app/src/schema_binding.cpp` now routes schema set-value automation through int, float, double, and combined Explaino seed edit paths; `ui_app/src/main.cpp` reports unconsumed set-value requests as deterministic visible/unsupported errors after schema and Color Pipeline rects render.
- Phase 1 GREEN native: direct `D:\salt-fractal\cuda_newton_fractal_clone\build_tests\test_schema_binding.exe` passes with `test_schema_binding: all passed`; the longer full native helper run timed out later, after this focused executable already passed.
- Phase 1 runtime publish: `artifacts/recent_explaino_phase1_runtime_publish.log` rebuilds the active runtime at `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Phase 1 GREEN runtime: `artifacts/recent_explaino_phase1_green_runtime_no_mouse_schema_set_value.log` passes the two new no-mouse schema set-value tests.
- Phase 1 adjacent guards: `artifacts/recent_explaino_phase1_guard_runtime_no_mouse_explaino_color.log` passes existing Explaino registry set-value, Color Pipeline set-value, row enable/remove, and no-physical-mouse runtime tests; direct Color Pipeline native executables `test_color_pipeline_core`, `test_color_pipeline_window`, and `test_escape_time_coloring` pass.
- Phase 2 RED: `artifacts/recent_explaino_phase2_red_renderer_owned_perf_witness.log` fails because diagnostic state reports `last_render_ms: 0`, proving the prior test could not measure renderer-owned performance.
- Phase 2 first GREEN attempt: `artifacts/recent_explaino_phase2_green_renderer_owned_perf_witness.log` records positive renderer timings but exposes the old relative-to-neutral baseline threshold as false for heavier seven-axis/owner-lane work.
- Phase 2 fix: headless diagnostic capture now enables `RenderSettings::benchmark` for the diagnostic render, and the runtime test now asserts positive `last_render_ms`, expected backend/strategy, and a 45 ms absolute median renderer budget at 1024x1024 instead of subprocess wall time.
- Phase 2 GREEN: `artifacts/recent_explaino_phase2_green_renderer_owned_perf_budget.log` passes the renderer-owned smooth-escape budget witness; `artifacts/recent_explaino_phase2_guard_smooth_escape_backend_routes.log` passes adjacent backend route tests.
- Phase 3 RED: `artifacts/recent_explaino_phase3_red_registry_single_authority.log` fails to compile `test_fractal_family_rules.cpp` because `ExplainoAxisRegistrySourceEntryCount()` does not exist, proving the registry list was not a single checked source authority.
- Phase 3 fix: `ui_app/src/fractal_family_rules.h` now declares the seven Explaino axes through `EXPLAINO_AXIS_REGISTRY_ENTRIES(...)`, generating `kExplainoAxisRegistry`, source-entry count, CUDA carrier checks, and CUDA any-axis checks from that one list.
- Phase 3 full native attempt: `artifacts/recent_explaino_phase3_green_registry_single_authority.log` timed out later in the broad CUDA sweep after rebuilding the touched registry/device binaries; this is a validation-run limitation, not a green full-suite claim.
- Phase 3 GREEN focused CPU/device proofs: `artifacts/recent_explaino_phase3_green_registry_cpu_direct.log` passes `test_fractal_family_rules.exe`; `artifacts/recent_explaino_phase3_green_registry_device_direct.log` passes `test_escape_time_sample_tier.exe` with the device parity probe for every registry axis.
- Phase 4 RED: `artifacts/recent_explaino_phase4_red_runtime_report_json_escape.log` fails because a quoted requested click id makes the automation report invalid JSON.
- Phase 4 fix: `ui_app/src/main.cpp` now writes all dynamic automation-report strings through `WriteAutomationReportString(...)`, including requested ids, current fractal id, lane/function ids, validation messages, and viewer/Color Pipeline control ids.
- Phase 4 runtime publish: `artifacts/recent_explaino_phase4_runtime_publish_json_escape.log` rebuilds the active runtime after the report writer fix.
- Phase 4 GREEN: `artifacts/recent_explaino_phase4_green_runtime_report_json_escape.log` passes the quoted-id JSON regression; `artifacts/recent_explaino_phase4_guard_runtime_no_mouse_explaino_color_json_report.log` passes eight no-mouse Explaino/schema/Color Pipeline report consumers.
- Phase 5 RED/current audit: `artifacts/recent_explaino_phase5_red_code_quality_current.log` reproduces the bootstrap code-quality failure with six `max_fn_lines` regressions.
- Phase 5 fix: mechanical, behavior-preserving helper splits reduce `fractal_derived_fields.cpp`, `main.cpp`, `escape_time_coloring.h`, `fractal_family_rules.h`, and `fractal_runtime_validation.h`; `fractal_probe_runner.cpp` had a parser false positive from a multiline `if` condition and now keeps the condition on one `if` line.
- Phase 5 GREEN: `artifacts/recent_explaino_phase5_green_code_quality.log` passes `tools/code_quality_audit.py --check-baseline`; `artifacts/recent_explaino_phase5_runtime_publish_after_code_quality_splits.log` rebuilds the active runtime after the splits.
- Phase 6 native: `artifacts/recent_explaino_phase6_native_full_build_tests.log` passes the full `ui_app/build_tests_vsdevcmd.cmd` helper rail, including `test_ui_schema`, `test_schema_binding`, `test_sample_tier_resolver`, `test_color_pipeline_core`, `test_color_pipeline_window`, `test_escape_time_coloring`, `test_fractal_family_rules`, and `test_escape_time_sample_tier`.
- Phase 6 runtime: `artifacts/recent_explaino_phase6_runtime_explaino_smooth_escape_perf_and_backend.log` and `artifacts/recent_explaino_hostile_audit_runtime_explaino_smooth_escape_after_republish.log` pass the renderer-owned smooth-escape backend/perf witnesses after final runtime publish.
- Phase 6 no-mouse runtime: `artifacts/recent_explaino_phase6_runtime_no_mouse_explaino_color_harness.log` and `artifacts/recent_explaino_hostile_audit_guard_runtime_no_mouse_after_json_control_char.log` pass the Explaino/schema/Color Pipeline no-mouse harness set.
- Phase 6 runtime publish: `artifacts/recent_explaino_hostile_audit_runtime_republish_json_control_char_fix.log` rebuilds the normal active runtime at `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe` after clearing a stale viewer process that locked the exe.
- Phase 6 code-quality: `artifacts/recent_explaino_hostile_audit_green_code_quality_after_json_control_char.log` passes the baseline after the hostile-audit JSON control-character fix.
- Phase 6 contract/plan: `artifacts/validation/recent_explaino_harness_perf_registry_hardening_contract.json` validates the contract; `py -3.14 tools/viewer_host_assert_phased_plan_sync.py docs/notes/recent_explaino_harness_perf_registry_hardening_PHASED_PLAN.md` reports OK.

## Hostile Audit

- Status: closed

Hostile questions for closure:
- Did I actually make visible schema numeric controls settable through the no-mouse harness, including int and double paths, without breaking existing float and Color Pipeline paths?
- Did failed set-value automation requests fail closed with a deterministic report error?
- Did I replace the weak perf proof with renderer/runtime-owned evidence rather than another subprocess timing proxy?
- Did I keep `kExplainoAxisRegistry` as the Explaino-all axis authority across tests, host helpers, and device behavior?
- Did I preserve Color Pipeline smooth-escape numeric live behavior and row click automation?
- Did I avoid changing fractal semantics except where a RED proves a current bug?
- Did I close with commit, receipts, push, clean tree, and stale plan text removed?

## Audit Passes

- [x] Pass 1 - diff/seam review found a real JSON-report defect: control characters below U+0020 were still emitted raw after the quote/backslash escape fix.
- [x] Pass 2 - re-read the repaired state after the `\u00XX` control-character fix; no additional real defect found in schema set-value, Explaino registry, Color Pipeline, smooth-escape, or code-quality seams.
- [x] Pass 3 - confirmed the repaired state with full native proof, final runtime publish, focused published-runtime proofs, code-quality, contract validation, and plan sync before checkpoint.

## Audit Findings

- [x] Real finding repaired: `artifacts/recent_explaino_hostile_audit_red_runtime_report_json_control_char.log` proves raw control characters still broke automation-report JSON; `ui_app/src/main.cpp` now escapes remaining control characters as `\u00XX`, and `artifacts/recent_explaino_hostile_audit_green_runtime_report_json_control_char.log` passes.
- [x] Clean re-read after repair: `artifacts/recent_explaino_hostile_audit_guard_runtime_no_mouse_after_json_control_char.log`, `artifacts/recent_explaino_hostile_audit_runtime_explaino_smooth_escape_after_republish.log`, and `artifacts/recent_explaino_hostile_audit_green_code_quality_after_json_control_char.log` confirm the repaired state did not expose another harness, smooth-escape, Color Pipeline, or code-quality defect.

## Boundaries

In scope:
- no-mouse runtime automation correctness for schema numeric controls and Color Pipeline guards
- Explaino/Explaino-all registry authority, runtime proof, and smooth-escape performance evidence
- automation report JSON correctness
- public validation profile quality blockers introduced or exposed by recent work

Out of scope:
- Meta-Basin, Operator-Itinerary, DSL/program-space, new fractal families, renderer replacement, broad UI redesign, or new Color Pipeline features
- changing `SampleFractalPoints(...)` legacy projection semantics except to prove they remain unchanged
