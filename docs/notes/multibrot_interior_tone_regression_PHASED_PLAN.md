# Escape-Time Interior Color Authority Regression

## Current Phase

Closed - escape-time smooth-escape interiors now have bounded public tone authority, representative default interiors are covered by native/runtime smoke, the slice is checkpointed and receipted, and this plan no longer advertises unresolved closeout work.

## Phase Checklist

- [x] Phase 1 - create the checked-in plan/contract and lock the initial slice
- [x] Phase 2 - revise the contract to cover escape-time interior-color authority, schema control exposure, state/capture persistence, and multi-fractal smoke rails
- [x] Phase 3 - add RED native/runtime proof that default smooth-escape interiors are neither forced black nor yellow-dominant across representative escape-time families
- [x] Phase 4 - add a bounded user-facing smooth-escape interior strength control with binding/state/capture coverage
- [x] Phase 5 - repair the escape-time smooth-escape interior tone path without changing formulas, exterior coloring, or Color Pipeline source-stack semantics
- [x] Phase 6 - publish runtime and prove default interior tone plus the new no-mouse control on the published viewer
- [x] Phase 7 - hostile-audit touched color/schema/state/runtime seams and update this plan with findings
- [x] Phase 8 - validate, checkpoint, receipts, push, merge-back, stale-plan grep, and clean-tree closeout

## Explicit User Asks

- [closed] Treat the shown default Multibrot yellow/gold filled-set view as a symptom of a broader escape-time interior-coloring regression.
- [closed] Add deeper native/unit smoke coverage so this does not recur for other escape-time families.
- [closed] Provide better user control if smooth-escape interiors are now colored instead of forced black.
- [closed] Preserve the recent smooth-escape high-black, low-luma, and low-unique repairs.
- [closed] Preserve Color Pipeline behavior; do not break source-stack editing or row automation.
- [closed] Do not start palette redesign, histogram/adaptive normalization, selector/preset, camera, SDF, formula, or broad renderer work.

## Scope

In scope:

- Add a small, explicit smooth-escape interior color strength control in the existing Color panel.
- Bind and persist that control through the same normal schema, diagnostics state, diagnostic capture, runtime-walk bootstrap, and no-mouse automation surfaces as the existing public color controls.
- Add native escape-time color smoke for representative interior samples, including at least Mandelbrot and Multibrot.
- Add runtime default-capture proof that representative published escape-time defaults are not yellow-dominant and not black-dominant under smooth escape.
- Keep smooth-escape interiors non-black and keep live smooth-escape scale/bias responsive.
- Preserve source-stack Color Pipeline authoring semantics; this is not a replacement for the advanced Color Pipeline row model.
- Preserve iteration-count and iteration-band interior black behavior.
- Preserve the Collatz-family and low-unique smooth-escape repairs.

Out of scope:

- Formula changes.
- Palette redesign.
- Histogram equalization or adaptive normalization.
- Selector/view preset work.
- Camera/dive behavior.
- SDF work.
- Renderer replacement.
- Physical mouse automation.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on `codex/multibrot-interior-tone` at `08ca494`; active contract was the initial narrow `multibrot_interior_tone_regression` contract.
- User witness: screenshot shows default Multibrot filled set as a large flat yellow/gold field.
- User correction: this is not Multibrot-only; it needs deeper regression/unit smoke and better control over colored interiors.
- RED: `py -3.14 tools/viewer_host_run_logged_command.py --label escape_time_interior_color_native_red --log artifacts/logs/escape_time_interior_color_native_red.log --out-json artifacts/validation/escape_time_interior_color_native_red.json --heartbeat-seconds 30 --timeout-seconds 600 -- cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner schema_binding diagnostics_state_io diagnostics_capture fractal_types` failed because `KernelParams` had no `color_smooth_escape_interior_strength`.
- Native implementation proof: `escape_time_interior_color_native_impl`, `escape_time_interior_color_schema_bundle_impl`, `escape_time_interior_color_diagnostics_capture_impl`, `escape_time_interior_color_types_impl`, and `escape_time_interior_color_derived_impl` passed.
- Runtime implementation proof: `escape_time_interior_color_runtime_publish_impl` passed; `py -3.14 -m pytest tests/test_fractal_runtime_multibrot_interior_tone.py -q --junitxml artifacts/pytest/escape_time_interior_color_runtime_impl.junit.xml` passed 2 tests.
- Inventory implementation proof: `escape_time_interior_color_inventory_impl` passed; `artifacts/escape_time_interior_color/latest/inventory.md` reports 18 cases with no high-black, low-luma, or low-unique flags.
- Final contract/plan proof: `viewer_host_validate_slice_contract.py` wrote `artifacts/validation/escape_time_interior_color_contract.json`; `viewer_host_assert_phased_plan_sync.py` passed.
- Final native proof: `escape_time_interior_color_native_final`, `escape_time_interior_color_schema_native`, `escape_time_interior_color_capture_native`, `escape_time_interior_color_types_native`, and `escape_time_interior_color_derived_native` passed.
- Final runtime proof: `escape_time_interior_color_runtime_publish` passed, `tests/test_fractal_runtime_multibrot_interior_tone.py` passed 2 tests with no-mouse automation, and `escape_time_interior_color_inventory` passed.
- Final audit/tool proof: `escape_time_interior_color_code_quality` passed baseline after the binding refactor; `escape_time_interior_color_diff_check` passed.
- Checkpoint proof: implementation checkpoint `3d629c8` was committed on `codex/multibrot-interior-tone`, validation and contract-proof receipts were written for that head, the feature branch was pushed, and local `master` fast-forwarded to the repaired head before this stale-plan cleanup.
- Code seam: `ui_app/src/escape_time_coloring.h` sends non-escaped smooth-escape pixels through the same programmable smooth signal path as escaped pixels; current native tests require "not black" and scale sensitivity, but not bounded interior tone or user authority.
- Schema seam: `ui/fractal_binding_surface_v1.ui_schema.json` exposes basic Color controls but no smooth-escape interior authority control.
- Binding seam: `ui_app/src/schema_binding.cpp` binds public `fractal.params.color_*` controls; the new control must use that seam and no-mouse UI automation.
- Persistence seam: `ui_app/src/diagnostics_state_io.cpp`, `ui_app/src/diagnostics_capture.cpp`, and `ui_app/src/runtime_walk_bootstrap.cpp` persist public color fields; old states must default cleanly.
- Default-reset seam: `ui_app/src/fractal_derived_fields.cpp` resets public color fields when applying derived/default fractal state.
- Branch: `codex/multibrot-interior-tone` from clean `master` at `08ca494`.

## Hostile Audit

- Status: complete

## Audit Passes

- [x] Pass 1 - verified the fix is limited to smooth-escape interior tone authority and does not change formulas, selectors, camera, or escaped exterior coloring.
- [x] Pass 2 - verified the new Color-panel control is schema-visible only for smooth_escape, bound through normal float binding, persisted through diagnostics state/capture, and available to no-mouse automation.
- [x] Pass 3 - verified Color Pipeline core/window rails and smooth-escape inventory remain green; source-stack authored source rows are not rewritten by this control because the blend is skipped when a source stack is active.
- [x] Pass 4 - verified runtime proof uses no physical mouse automation and this plan has been updated before final validation.
- [x] Pass 5 - clean re-read the repaired state after the real findings; final native/schema/capture/types/runtime publish/pytest/inventory/code-quality/diff evidence did not expose another real defect or workflow mistake.

## Audit Findings

- [x] Finding 1 - the broadened implementation touched `ui_app/src/fractal_derived_fields.cpp` for default reset, but the first broadened contract omitted that file. Repaired by adding the seam to the contract and re-locking `global_active_contract`.
- [x] Finding 2 - the diagnostics capture test asserted a `0.42f` JSON substring, which can print as a non-exact float. Repaired by using exactly representable `0.5f` while preserving the state-export assertion.
- [x] Finding 3 - code quality caught a real function-size regression in `BindingContext::BindFloat()` after the new color binding was added. Repaired by extracting `BindColorPanelFloat(...)`; `test_schema_binding` and the code-quality baseline both passed after the refactor.
- [x] Finding 4 - the committed implementation plan still described closeout as remaining after the receipts and merge-back. This stale-plan cleanup closes Phase 8 and removes unresolved closeout text on the final head.

## Action Hostile Review

- Action ID: escape-time-interior-color-authority
- Suspected failure mode: A "not black" smooth-escape interior repair can overcorrect into flat yellow interiors, and a quick Multibrot-only fix would leave the same bug class untested on other escape-time families or leave users with no authority over colored interiors.
- Correct owner/action: Add native and runtime proof for bounded smooth-escape interior tone across representative escape-time families; add one explicit bounded public control; tune only the unescaped smooth-escape interior path.
- Proof surface: focused native color test, schema/binding/state/capture tests, runtime publish, no-mouse default/interior-control proof, full smooth-escape inventory, plan/contract validators, hostile-audit validator, code-quality baseline, diff check.
- Blocked action: formula changes, palette redesign, histogram/adaptive normalization, selector/view preset work, camera/dive behavior, SDF work, renderer replacement, and physical mouse automation.
