# PFC Generated/Internal Editors

## Explicit User Asks

- Start Step 9 from `codex/parameter-functionality-campaign` after the seed-responsiveness repair is closed and pushed.
- Expose generated/internal editor authority through safe override-mode UI, not raw array dumping.
- Keep the slice cleanup/polish scoped: no new fractal type, no equation-pack viewport integration, no perturbation, no Color Pipeline rewrite, no capture/fps pacing work.
- Preserve existing generated defaults, saved-state compatibility, and runtime authority.
- Use no-mouse runtime proof.

## Current Phase

Phase 5 is validation-complete on `codex/pfc-generated-internal-editors`. The feature branch is ready for the repo checkpoint wrapper and holder-branch integration workflow.

## Phase Checklist

- [x] Start from clean pushed holder `codex/parameter-functionality-campaign` at `680a828`.
- [x] Create feature branch `codex/pfc-generated-internal-editors`.
- [x] Create this checked-in plan and contract.
- [x] Lock the active slice to this Step 9 plan/contract.
- [x] RED: prove existing polynomial coefficient controls are exposed without an explicit custom/override mode gate.
- [x] RED: prove Explaino generated roots cannot be edited authoritatively through a safe owner-lane override surface.
- [x] Implement explicit generated/custom authority for Explaino root editing without raw array dumping.
- [x] Gate Newton/Nova/Halley coefficient controls behind `poly_kind=custom` so preset modes stay truthful.
- [x] Add schema/binding/state/runtime validation tests for authority modes and bounds.
- [x] Add no-mouse runtime proof that a custom root edit changes `explaino` output and that generated mode still regenerates from Seed.
- [x] Validate focused native rails, runtime publish, no-mouse runtime proof, full native helper suite, hostile audit, and diff check.

## Owner Seams

- Type/state authority: `ui_app/src/fractal_types.h`, `ui_app/src/enum_id_utils.h`.
- Generated/default authority: `ui_app/src/fractal_derived_fields.cpp`.
- UI/schema authority: `ui/fractal_binding_surface_v1.ui_schema.json`, `ui_app/src/ui_schema.cpp`, `ui_app/src/schema_binding.cpp`.
- Saved state/capture authority: `ui_app/src/diagnostics_state_io.cpp`, `ui_app/src/diagnostics_capture.cpp`, `ui_app/src/runtime_walk_bootstrap.cpp`.
- Runtime/sample authority: `ui_app/src/fractal_sample_device.inl`, `ui_app/src/fractal_probe_runner.cpp`.
- Native tests: `ui_app/tests/test_ui_schema.cpp`, `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_diagnostics_state_io.cpp`, `ui_app/tests/test_fractal_runtime_validation.cpp`, `ui_app/tests/test_fractal_derived_fields.cpp`, `ui_app/tests/test_fractal_sample_kernel.cu`.
- Runtime proof: `tests/test_fractal_runtime_generated_internal_editors.py` plus the existing `tests/test_fractal_runtime_explaino_julia_authority.py` no-mouse proof.

## Design Boundary

- Explaino root editing gets an explicit authority mode: `generated` keeps current seed/phase/root-spread generation; `custom` lets bounded root-count/root-coordinate controls drive the carrier polynomial.
- The editor exposes individual bounded root coordinate controls only behind the explicit custom authority mode and only on owning Explaino lanes that already use custom polynomial authority.
- Generated mode remains the default and preserves existing visuals.
- Newton/Nova/Halley coefficient controls remain available only under `poly_kind=custom`; preset polynomial modes do not imply the raw coefficients are authoritative user controls.
- `explaino_all` receives only the shared generated/root-authority editor, with custom fields hidden until explicit custom authority is active.
- `explaino_julia` remains governed by its seeded/custom constant authority; the existing no-mouse runtime proof confirms Seed still changes its live output.
- No `poly_coeffs_b` or other secondary/generated arrays are exposed in this slice.
- No equation-pack viewport integration, perturbation, Color Pipeline, capture, or FPS pacing work is included.

## Proof Ledger

- Source holder branch: `codex/parameter-functionality-campaign`.
- Starting head: `680a828`.
- Feature branch: `codex/pfc-generated-internal-editors`.
- Contract lock: `ck:14dbe91e` via `viewer_host_begin_work_slice.py`.
- RED proof: `artifacts/validation/pfc_generated_internal_editors_red_schema.json` failed on missing generated root authority.
- Focused schema/binding rail: `artifacts/validation/pfc_generated_internal_editors_schema.json` passed.
- Runtime validation rail: `artifacts/validation/pfc_generated_internal_editors_runtime_validation.json` passed.
- Sample rail: `artifacts/validation/pfc_generated_internal_editors_sample.json` passed.
- Code quality: `artifacts/validation/pfc_generated_internal_editors_code_quality.json` passed after extracting `BindExplainoRootCoordinate`.
- Runtime publish: `artifacts/validation/pfc_generated_internal_editors_runtime_publish.json` passed.
- No-mouse runtime proof: `artifacts/validation/pfc_generated_internal_editors_runtime_pytest.json` passed with `tests/test_fractal_runtime_generated_internal_editors.py` and `tests/test_fractal_runtime_explaino_julia_authority.py`.
- Full native helper suite: `artifacts/validation/pfc_generated_internal_editors_full_native.json` passed.
- Diff check: `artifacts/validation/pfc_generated_internal_editors_diff_check.json` passed.

## Hostile Audit

- Status: complete
- Did I actually add explicit override authority instead of dumping raw arrays into the UI? Yes: `explaino_root_authority` is the default-generated gate and root fields are hidden behind `explaino_custom_roots_active`.
- Did I preserve generated/default behavior when custom override is off? Yes: generated remains default, generated authority clears custom roots, and sample/native tests cover default parity.
- Did I keep preset polynomial controls truthful by hiding coeff editors unless `poly_kind=custom`? Yes: schema visibility and binding tests require the custom predicate.
- Did custom Explaino roots survive derived-field refresh and actually drive the runtime carrier polynomial? Yes: derived-field, sample, and no-mouse runtime tests cover custom-root sensitivity.
- Did I keep `explaino_all` and owner lanes truthful? Yes: the non-axis allowlist explicitly classifies shared root authority, and registry-axis behavior is unchanged.
- Did I avoid equation-pack, perturbation, Color Pipeline, capture, FPS pacing, and broad engine drift? Yes: touched files are limited to schema/binding/state/runtime validation/tests and the active plan/contract.
- Did I close with receipts, push, clean tree, and no stale plan text? This plan is synchronized for the checkpoint workflow; receipts and push are produced by the repo closeout tools after commit.

## Audit Passes

- [x] Pass 1: RED/diff review found the missing custom-root runtime validation and the missing `explaino_all` non-axis classification for `explaino_root_authority`; both were fixed and revalidated.
- [x] Pass 2: repaired-state review found stale `poly_c4` visibility expectations and an invalid `UpdateExplainoPolynomial` link assumption in `test_fractal_sample_kernel`; both were corrected and revalidated.
- [x] Pass 3: final clean re-read found the `BindFloat()` line-count regression, extracted `BindExplainoRootCoordinate`, and no additional real issue was found after rerunning schema, quality, full native, runtime publish, runtime pytest, and diff check.

## Audit Findings

- [x] Real finding: custom Explaino root authority initially lacked runtime validation for invalid root count and non-finite root coordinates; `test_fractal_runtime_validation` now covers both failures.
- [x] Real finding: `explaino_root_authority` was visible on `explaino_all` but missing from the explicit non-axis classification allowlist; `test_ui_schema` now classifies it deliberately.
- [x] Real finding: the legacy `poly_c4` test expected unconditional Nova visibility after the slice made coefficient editors custom-authority-only; the test now enforces the authority gate.
- [x] Real finding: the first sample proof crossed a helper-target link boundary by calling `UpdateExplainoPolynomial`; the sample proof now stays inside `test_fractal_sample_kernel` link scope.
- [x] Real finding: the new root binding paths pushed `BindFloat()` past the code-quality baseline; `BindExplainoRootCoordinate` restored the baseline.
- [x] Clean re-read: final validation did not expose another generated/internal editor, Explaino-Julia seed, schema, binding, runtime, quality, or diff issue.

## Out Of Scope

- Equation-pack viewport integration.
- Perturbation zoom.
- Color Pipeline changes.
- Capture finding and FPS pacing changes.
- New fractal types.
- Raw `poly_coeffs_b` editor exposure.
