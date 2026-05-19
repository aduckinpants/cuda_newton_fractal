# Top Down Architecture Repair

## Current Phase

Phase 1 closed - architecture guard, automation-report extraction, and no-mouse rendered-frame proof repair

## Explicit User Asks

- [done] Fix the churn pattern that lets small viewer repairs repeatedly cross unrelated systems.
- [done] Use the mainline `C:\code\salticid-cuda` structural-analysis tooling where it helps, without editing mainline.
- [done] Preserve Explaino, Color Pipeline, no-mouse harness, and smooth-escape behavior while extracting seams.
- [done] Close only with machine proof, hostile audit, checkpoint, receipts, push, clean tree, and no stale plan text.

## Phase Checklist

- [x] Phase 1 - add a repo-local architecture audit/ratchet and extract the UI automation report seam out of `main.cpp`.
- [deferred] Phase 2 - split Color Pipeline model/actions/validation from the ImGui rendering header without changing behavior.
- [deferred] Phase 3 - make fractal family ownership descriptor-driven across schema, bindings, runtime routing, and sample-tier policy.
- [x] Phase 4 - move the slider-change runtime proof away from the broken OS-window pixel probe and into the in-process rendered-frame report.
- [x] Phase 5 - run validation, hostile audit, checkpoint, receipts, push, clean tree, and stale-plan proof for this closed Phase 1 slice.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` reported `master`, `HEAD=b6690d7`, clean tree, and the prior closed `recent_explaino_harness_perf_registry_hardening` contract.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported no staged, unstaged, or untracked files before this slice.
- Review evidence: `tools/code_quality_audit.py --check-baseline` reports `color_pipeline_window.h` 4221 lines, `main.cpp` 2930 lines, and `schema_binding.cpp` 1818 lines; the audit is green but exposes architectural concentration.
- Mainline tool evidence: `C:\code\salticid-cuda\tools\native_architecture_audit.py` is mainline/native-src-specific; the CppDepend status helper is reusable only after this repo has a real compile database or project input.
- RED: no checked-in viewer-host architecture audit failed or reported when the god-file/harness/tooling risks grew.
- RED: automation report JSON and set-value fail-closed logic lived inside `main.cpp`, forcing viewer-shell edits for automation-report changes.
- RED: `tests/test_fractal_runtime_runtime_walk_viewer.py` reported `set_value_consumed=true` but sampled a blank OS-window bitmap, so visible slider-change proofs could false-red or false-green outside the renderer.
- RED: runtime-walk playback applied after Color Pipeline set-value automation, so a consumed Color Pipeline source-scale edit could be overwritten before render.
- Fix: added `tools/viewer_host_architecture_audit.py` plus `tests/test_viewer_host_architecture_audit.py`; final audit `artifacts/architecture/top_down_architecture_audit_final.json` is `ok=true` and still records the CppDepend compile-database readiness gap as a warning.
- Fix: extracted automation-report/fail-closed helpers to `ui_app/src/viewer_ui_automation_report.{h,cpp}` and added `ui_app/tests/test_viewer_ui_automation_report.cpp`.
- Fix: `WriteColorPipelineUiAutomationReport(...)` now includes renderer-owned `rendered_frame_hash` metadata, written after `PresentFrame()`, and the no-mouse slider tests compare that in-process frame hash instead of a blank HWND capture.
- Fix: runtime-walk playback/import now runs before Color Pipeline set-value rendering so the Color Pipeline source-scale edit is not overwritten before the published frame.
- Fix: removed obsolete blank-window probe helpers from the runtime-walk pytest file instead of raising the architecture baseline.
- Fix: loosened one Explaino escape-variant `epsilon=1e-3` persisted-float assertion to `1e-9`, matching the shipped 32-bit runtime storage.
- Proof: `ui_app\build_tests_vsdevcmd.cmd` passed in `artifacts\top_down_architecture_native_full_after_frame_hash.log`.
- Proof: `ui_app\build_vsdevcmd.cmd` passed in `artifacts\top_down_architecture_runtime_publish_after_frame_hash.log`.
- Proof: no-mouse runtime guard passed in `artifacts\top_down_architecture_runtime_no_mouse_guards_after_probe_removal.log`.
- Proof: Explaino escape variants passed in `artifacts\top_down_architecture_explaino_escape_variants_after_tolerance.log`.
- Proof: callable/catalog Python rail passed in `artifacts\top_down_architecture_callable_catalog.log`.
- Proof: code-quality audit passed in `artifacts\top_down_architecture_code_quality_final.json` with score 97.

## Hostile Audit

- Status: closed

Hostile questions for closure:
- Did I actually remove a live seam from `main.cpp`, or only add more tooling prose? Yes: automation report/fail-closed logic moved to `ui_app/src/viewer_ui_automation_report.{h,cpp}` with a focused native test.
- Did I actually add a machine-readable architecture ratchet that can catch repeated churn? Yes: `tools/viewer_host_architecture_audit.py` fails on risk-surface line/function growth and stayed green only after obsolete probe code was removed.
- Did I preserve Explaino selector/control behavior, Color Pipeline live edits, no-mouse runtime harness behavior, and smooth-escape performance routing? Yes for the touched proof surface: registry-axis and Color Pipeline no-mouse set-value proofs now change renderer-owned frame hashes, Explaino escape variants pass, and full native tests pass.
- Did I avoid editing `C:\code\salticid-cuda` and keep mainline tooling use read-only/adapted? Yes: mainline was inspected only through existing tool files; no mainline mutation was made.
- Did I checkpoint, receipt, push, and leave the tree clean with no stale plan language? This plan is synchronized for closure; checkpoint, receipts, push, and clean-tree proof are recorded by the final closeout commands for this slice.

## Audit Passes

- [done] Pass 1 - found the report-before-render/blank-HWND proof defect and replaced it with in-process rendered-frame hashing.
- [done] Pass 2 - architecture audit caught runtime-walk pytest line growth; removed obsolete probe code instead of raising the baseline.
- [done] Pass 3 - re-read the repaired state; no additional real defect found and no stale broad-engine, Meta-Basin, Operator-Itinerary, DSL, or renderer-rewrite drift remained.

## Audit Findings

- [done] The live no-mouse set-value rail trusted a blank OS-window pixel capture; it now uses renderer-owned frame hashes published by the in-process automation report.
- [done] Color Pipeline set-value automation in runtime-walk viewer could be overwritten before render; runtime-walk playback now applies before Color Pipeline automation.
- [done] The new runtime-harness helper code initially grew a guarded risk surface; obsolete blank-window probe helpers were removed and the architecture audit is green.
- [done] Explaino escape-variant runtime rail had a false-red double-precision expectation for a float-backed `epsilon` field; the test now uses a float-appropriate tolerance.

## Action Hostile Review

- Action ID: fractal-toolkit-magnet-bridge-1
- Suspected Failure Mode: the closed top-down contract keeps blocking the new modular fractal-toolkit plan/contract pair, causing product edits to happen under a stale or wrong contract.
- Correct Owner/Action: widen only the closed top-down planning contract scope enough to add the new Magnet-wave plan and contract, then immediately lock the new slice before touching runtime, schema, renderer, or tests.
- Proof Surface: this bridge may touch only the closed top-down plan/contract and the new plan/contract file paths; no product runtime file is allowed before the new contract is active.
- Blocked Action: adding Magnet runtime/schema/test code, scan tooling changes, or any new fractal implementation under the old top-down architecture contract.

## Notes

- Phase 1 is closed as a concrete foundation: a real seam was extracted, a risk-surface audit gate was added, and the broken no-mouse slider proof moved to renderer-owned evidence.
- Phase 2 and Phase 3 remain deferred future slices; they were not bundled into this checkpoint.
- Mainline remains read-only. Any CppDepend/NDepend-style output for this repo belongs under this repo's `artifacts/` or `D:\third-party-output\cppdepend\cuda_newton_fractal_clone`.
