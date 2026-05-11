# Smooth Escape Backend Signal Classification

## Current Phase

Phase 4 - hostile-audit the repaired state, write receipts, and checkpoint cleanly

## Phase Checklist

- [x] Phase 1 - classify the smooth_escape visual failure with a focused RED witness
- [x] Phase 2 - repair the proven owner with the smallest runtime-authoritative change
- [x] Phase 3 - prove the repaired behavior through native and published-runtime rails
- [ ] Phase 4 - hostile-audit the repaired state, write receipts, and checkpoint cleanly

## Explicit User Asks

- [done] Start implementation without blending this slice into the closed advanced-color band_finish work.
- [done] Prove the smooth_escape f32/f64 visual failure before choosing a fix owner.
- [done] Preserve mandatory forward TDD and integration TDD; native RED/GREEN and published-runtime proof are recorded before closure.
- [open] Keep the slice bounded, hostile-reviewed, checkpointed, and honest.

## Presumption Loop

The local hypothesis is deliberately unsettled: the reported smooth_escape failure may be caused by backend selection, residual-sensitive signal semantics, normalization/tuning, family-specific precision limits, or an interaction between those seams. The first implementation step is therefore classification, not a precision-policy patch. The RED witness must compare current `fast`, `standard`, and `tier_auto` behavior and record enough evidence to name the owner before any runtime fix lands.

## Presumption Evidence

- Owner Proof: done. The native RED shows ExplainO programmable `smooth_escape` has a material fast-vs-standard delta (`2242` pixels at 64x64), while `tier_auto` resolves to `float32` and exactly matches `fast`; the first owner is backend routing policy missing signal context, not an unproven renderer rewrite.
- RED Witness: done via `py -3.14 tools/viewer_host_run_logged_command.py --label "smooth_escape red native helper tests rerun" --log artifacts/smooth_escape_red_native_rerun.log -- ui_app\build_tests_vsdevcmd.cmd` exiting 1 at `test_escape_time_sample_tier.exe` with `auto backend=float32, fast_standard_diffs=2242, auto_fast_diffs=0, auto_standard_diffs=2242`.
- Integration Witness: done. `artifacts/smooth_escape_runtime_backend_pytest.log` exits 0 with the active `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`; the focused pytest asserts captured diagnostics for ExplainO smooth_escape report `render.sample_tier == tier_auto`, `stats.resolved_backend == float64`, and `stats.resolved_strategy == direct`.
- Fix Proof: done. `ResolveSampleEvalModeForRender` promotes only basin-family render-context smooth_escape auto requests to `float64`, keeps explicit `fast`/`standard` behavior unchanged, and leaves unrelated root-index auto behavior at `float32` in resolver coverage.
- Hostile Review Pass 1: done. Found a real diagnostics round-trip defect: the provenance patch wrote `render.sample_tier` but the loader ignored it. Added `artifacts/smooth_escape_sample_tier_loader_red_native_rerun.log` as behavioral RED and repaired the loader.
- Hostile Review Pass 2: done. Re-read the repaired resolver/renderer/state-loader diff; no additional real defect found after `artifacts/smooth_escape_sample_tier_loader_green_native.log` passed.
- Hostile Review Pass 3: done. Re-read the published-runtime proof and closure chain after republish; no additional workflow mistake found after `artifacts/smooth_escape_loader_runtime_backend_pytest.log` passed.

## Proof Ledger

- Bootstrap: session bootstrap reported clean after carryover cleanup and active advanced-color contract was still locked before this new plan/contract was created.
- Manual RED: done. Initial native run `artifacts/smooth_escape_red_native.log` failed for malformed string literals, which was not a valid behavioral RED and was repaired before proceeding.
- Checked-in regression RED: done. `artifacts/smooth_escape_red_native_rerun.log` exits 1 because `tier_auto` remains `float32` while explicit `fast` vs `standard` differs by 2242 pixels.
- First GREEN: done. `artifacts/smooth_escape_green_native.log` exits 0 after the render-context resolver repair.
- Native validation: done. `artifacts/smooth_escape_diagnostics_green_native.log` exits 0 after adding diagnostics provenance coverage.
- Audit RED: done. `artifacts/smooth_escape_sample_tier_loader_red_native_rerun.log` exits 1 on the behavioral assertion `render sample_tier should load from saved diagnostics state`; the earlier `artifacts/smooth_escape_sample_tier_loader_red_native.log` was invalid compile noise and is not counted as behavioral proof.
- Audit repair validation: done. `artifacts/smooth_escape_sample_tier_loader_green_native.log` exits 0 after adding sample-tier loader support and invalid-id coverage.
- Runtime publish: done. `artifacts/smooth_escape_runtime_publish.log` exits 0 and stages the active runtime to `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`; `artifacts/smooth_escape_loader_runtime_publish.log` exits 0 after the loader repair and restages the active runtime.
- Published-runtime proof: done. `artifacts/smooth_escape_runtime_backend_pytest.log` exits 0 with `1 passed, 21 deselected` for the focused ExplainO smooth_escape auto-backend test; `artifacts/smooth_escape_loader_runtime_backend_pytest.log` repeats the same proof after the loader repair.
- Code quality audit: done. `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json` exits 0 with score 97/100 and baseline passed.
- Contract validation: done. `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/smooth_escape_backend_signal_classification.contract.json --out-json artifacts/validation/smooth_escape_backend_signal_classification_contract.json` exits 0.
- Plan sync: done. `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` exits 0 after hostile-audit updates.
- Hostile audit validator: done. `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/smooth_escape_backend_signal_classification_PHASED_PLAN.md --out-json artifacts/validation/smooth_escape_backend_signal_classification_hostile_audit.json` exits 0.
- Checkpoint: pending.

## Hostile Audit

- Status: done
- Required posture: assume the first explanation is wrong until the witness distinguishes backend routing, signal semantics, tuning, and family precision. Assume a helper-only or hash-only proof is insufficient unless it proves the user-visible failure mode.

## Audit Passes

- [done] Pass 1 - inspected the classification witness and diagnostics proof for hidden bias; found the real `sample_tier` write-without-load defect and repaired it with RED/GREEN coverage.
- [done] Pass 2 - re-read the repaired state across resolver, renderer, diagnostics capture, and diagnostics loader; no additional real defect found and explicit `fast` semantics remain preserved by tests.
- [done] Pass 3 - re-read the published-runtime proof and receipt chain after republish; no additional workflow mistake found and the focused runtime test proves the active executable reports auto plus `float64`.

## Audit Findings

- [done] Real finding: diagnostics capture wrote `render.sample_tier` for provenance while `LoadDiagnosticsStateFile` ignored that field, so explicit-tier captured states would reload as default auto. Fixed by parsing `sample_tier` through `TryParseSampleTierId` and failing unknown string ids.
- [done] Clean re-read evidence: `artifacts/smooth_escape_sample_tier_loader_green_native.log` confirmed the repaired state after the loader fix; no additional real issue found in the state round-trip seam.
- [done] Clean re-read evidence: `artifacts/smooth_escape_loader_runtime_backend_pytest.log` confirmed the repaired published-runtime path after republish; no additional workflow mistake found in runtime proof.

## Action Hostile Review

- Action ID: action-20260511-smooth-escape-sample-tier-roundtrip
- Suspected Failure Mode: diagnostics may write `render.sample_tier` for runtime proof while the state loader silently ignores it, causing explicit-tier captures to reload as default auto.
- Expected Proof: a focused diagnostics-state regression proves `sample_tier` reloads as `standard`, invalid ids fail, diagnostics capture still writes the field, and the published-runtime smooth_escape proof still reports auto plus `float64`.
- Blocked Action: any closure that writes `sample_tier` into state JSON without loader support or explicit validation failure for bad ids.

## Notes

- Expected owner files:
  - `docs/notes/smooth_escape_backend_signal_classification_PHASED_PLAN.md`
  - `docs/contracts/smooth_escape_backend_signal_classification.contract.json`
  - `ui_app/src/sample_tier_resolver.cpp`
  - `ui_app/src/escape_time_coloring.h`
  - `ui_app/src/fractal_renderer.cu`
  - `ui_app/src/fractal_sample_device.inl`
  - `ui_app/src/fractal_types.h`
  - `ui_app/tests/test_escape_time_sample_tier.cu`
  - `tests/test_fractal_runtime_explaino_escape_variants.py`
- Non-goals:
  - do not reopen advanced-color catalog completeness
  - do not add Capture/Export or Fractal Lab UI work in this slice
  - do not change generic.sample or Reality Toolkit surfaces here
  - do not implement broad perturbation support unless the witness proves that is the owner for this bug
- Exit criteria:
  - a real named owner is proven by a RED witness
  - the repair is minimal and runtime-authoritative
  - native and published-runtime proof pass
  - hostile audit records three repaired-state passes or a real finding plus clean re-audit evidence
  - validation and contract proof receipts exist for the final committed HEAD
