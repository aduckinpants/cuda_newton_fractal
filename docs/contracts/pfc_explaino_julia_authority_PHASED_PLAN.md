# PFC Explaino Julia Authority

## Explicit User Asks

- Continue the parameter functionality campaign from the pushed holder branch.
- Implement Step 8 on feature branch `codex/pfc-explaino-julia-authority`.
- Add Explaino Julia direct constants behind an explicit seeded/custom authority mode instead of silently repurposing current defaults.
- Preserve existing `explaino_julia` selector identity and existing Explaino-all registry/common-axis behavior.
- Keep owner-specific Julia authority controls off `explaino_all`.
- Use no-mouse persistent runtime proof and avoid repeated viewer relaunch loops.
- Do not touch Color Pipeline, capture finding, FPS pacing, equation-pack viewport integration, perturbation zoom, generated editors, or broad engine redesign in this slice.

## Current Phase

Phase 4 is open: implementation and validation are green; checkpoint, receipts, push, and holder integration remain pending.

## Phase Checklist

- [x] Bootstrap and confirm the campaign holder was clean before this feature branch.
- [x] Create feature branch `codex/pfc-explaino-julia-authority` from `codex/parameter-functionality-campaign`.
- [x] Create this checked-in plan and contract.
- [x] Lock the active slice to this Explaino Julia authority contract.
- [x] Inspect the existing standalone Julia, existing Explaino Julia path, selector identity, schema, binding, validation, probe, sample, state IO, descriptor, and safe-mode seams.
- [x] RED: prove the public `explaino_julia` lane lacks an explicit seeded/custom Julia constant authority mode.
- [x] RED: prove native schema/sample/probe/state rails do not yet cover Explaino Julia custom constants.
- [x] Implement the narrow authority repair while preserving default behavior.
- [x] Make custom/direct constants authoritative on `explaino_julia` through the shipped public path and hidden from `explaino_all`.
- [x] Wire schema/binding, safe-mode schema, validation, diagnostics state IO, descriptor/catalog, native sample/probe, and runtime math.
- [x] Validate focused schema/native/sample/probe/function-descriptor/state rails.
- [x] Publish runtime and run one persistent no-mouse viewer proof for selector identity plus frame-hash sensitivity.
- [x] Run full native helper suite if focused rails pass.
- [x] Hostile audit the diff and repair real findings.
- [ ] Prepare checkpoint, validation receipts, contract-proof receipt, push, and clean-tree closure.

## Owner Seams

- UI schema authority: `ui/fractal_binding_surface_v1.ui_schema.json`.
- Binding authority: `ui_app/src/schema_binding.cpp`.
- Fractal params/state authority: `ui_app/src/fractal_types.h`, `ui_app/src/diagnostics_state_io.cpp`.
- Enum/string authority: `ui_app/src/enum_id_utils.h`.
- Family/selector/coloring authority: `ui_app/src/fractal_family_rules.h`, `ui_app/src/fractal_derived_fields.cpp`.
- Runtime validation: `ui_app/src/fractal_runtime_validation.h`.
- Device runtime: `ui_app/src/fractal_sample_device.inl`.
- Host/probe runtime: `ui_app/src/fractal_probe_runner.cpp`.
- Descriptor/catalog visibility: `ui_app/src/function_descriptor.cpp`.
- Safe-mode schema: `ui_app/src/safe_mode_schema.cpp`.
- Native proof: `ui_app/tests/test_ui_schema.cpp`, `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_safe_mode_schema.cpp`, `ui_app/tests/test_diagnostics_state_io.cpp`, `ui_app/tests/test_fractal_runtime_validation.cpp`, `ui_app/tests/test_fractal_sample_kernel.cu`, and probe/descriptor tests.
- Runtime proof: `tests/test_fractal_runtime_explaino_julia_authority.py` and related published-runtime descriptor checks.

## Design Boundary

- `explaino_julia` remains the existing selector; this slice does not add a new `FractalType`.
- Existing default visual behavior must remain compatible unless the RED proof shows a stricter authority default is required.
- The new authority mode must make the source of the Julia constant explicit: seeded/root-derived mode versus custom/direct constants.
- Owner-specific Julia authority controls belong on `explaino_julia`, not `explaino_all`.
- Existing standalone `julia` behavior remains unchanged.
- Common Explaino registry/common-axis controls and `explaino_all` behavior must not be changed.

## Proof Ledger

- Slice source branch: `codex/parameter-functionality-campaign`.
- Slice branch: `codex/pfc-explaino-julia-authority`.
- Starting head: `e784817`.
- Campaign holder prerequisite: Step 7 closed and integrated at `3d214c3`; holder cleanup head is `e784817`.
- Contract lock: `viewer_host_begin_work_slice.py` opened `ck:c342fa74` for `docs/contracts/pfc_explaino_julia_authority_PHASED_PLAN.md` and `docs/contracts/pfc_explaino_julia_authority.contract.json`.
- RED native coverage proof: `pfc_explaino_julia_authority_red_sample` failed as expected because `KernelParams` had no `explaino_julia_c_real` / `explaino_julia_c_imag` authority fields yet.
- Implementation facts: added `ExplainoJuliaConstantMode::{seeded,custom}`, owner params `explaino_julia_c_real` / `explaino_julia_c_imag`, computed visibility predicate `fractal.params.explaino_julia_custom_constants_active`, schema/safe-mode controls, binding/state/capture export, descriptor visibility, native/probe runtime routing, validation, and no-mouse runtime proof.
- Default behavior: `seeded` mode is the default and still uses the seeded/root-derived Explaino Julia constant path; standalone `julia_c_real` / `julia_c_imag` remain standalone Julia only.
- Owner visibility: `explaino_julia_constant_mode` is visible only on `explaino_julia`; custom constants are visible only when `fractal.params.explaino_julia_custom_constants_active` is true, and binding tests prove that predicate is false on `explaino_all`.
- Focused native rails:
  - `pfc_explaino_julia_authority_sample`: passed, `test_fractal_sample_kernel: passed=1031 failed=0`.
  - `pfc_explaino_julia_authority_schema`: passed Color Pipeline/schema/state rail bundle.
  - `pfc_explaino_julia_authority_descriptor`: passed.
  - `pfc_explaino_julia_authority_safe_mode`: passed after repairing the stale safe-mode panel count.
  - `pfc_explaino_julia_authority_state`: passed.
  - `pfc_explaino_julia_authority_generic_probe`: passed.
  - `pfc_explaino_julia_authority_sample_tier`: passed.
  - `pfc_explaino_julia_authority_ui_schema_bundle_after_safe_mode_fix`: passed after the duplicate `test_ui_schema` safe-mode count was updated.
- Runtime publish: `pfc_explaino_julia_authority_runtime_publish_final` passed; active runtime rebuilt.
- No-mouse runtime proof: `pfc_explaino_julia_authority_runtime_pytest_final` passed, 10 pytest cases including `tests/test_fractal_runtime_explaino_julia_authority.py`.
- Full native suite: first `pfc_explaino_julia_authority_full_native_final` failed on stale duplicate `test_ui_schema` safe-mode panel count; repaired and reran `pfc_explaino_julia_authority_full_native_final_retry`, which passed with `All helper tests passed`.
- Code quality: `tools/code_quality_audit.py --check-baseline` passed with score 96/100.
- Hostile audit repair findings: stale duplicate safe-mode schema count was found by full native and repaired; unsupported focused target names were identified as workflow friction and not claimed as passing proof.

## Hostile Audit

- Status: complete
- Did I actually add explicit seeded/custom authority for `explaino_julia` constants instead of silently borrowing another lane's defaults?
- Did I keep existing `explaino_julia` selector identity intact?
- Did I keep owner-specific Julia controls off `explaino_all`?
- Did I preserve standalone `julia` behavior?
- Did I preserve existing Explaino-all registry/common-axis behavior?
- Did I prove visible custom constants change the published runtime frame through no-mouse automation?
- Did I avoid Color Pipeline, capture finding, FPS pacing, equation-pack, perturbation, generated-editor, or broad engine drift?
- Did I close with receipts, push, clean tree, and no stale plan text?

## Audit Passes

- [x] Pass 1: diff review confirmed the new controls are owner-gated and the custom constant predicate stays false on `explaino_all`.
- [x] Pass 2: repaired-state review found the stale duplicate safe-mode panel count in `test_ui_schema`; fixed and revalidated through the schema bundle and full native retry.
- [x] Pass 3: final clean re-read found no Color Pipeline, capture-finding, FPS pacing, equation-pack, perturbation, renderer, sample-tier, or physical mouse automation drift.

## Audit Findings

- [x] Finding 1: `test_ui_schema` carried a duplicate safe-mode fractal-panel count that was stale after adding the three safe-mode Explaino Julia authority controls. Fixed the count from 26 to 29 and proved it through `pfc_explaino_julia_authority_ui_schema_bundle_after_safe_mode_fix` plus the full native retry.
- [x] Finding 2: the build script does not expose standalone focused targets named `test_ui_schema`, `test_fractal_runtime_validation`, or `test_param_anim_generic`; those failed before executing tests. The slice now records only proof rails that actually ran, and uses the existing `test_explaino_counterfactual_repair` schema bundle when a focused `test_ui_schema` witness is needed.

## Out Of Scope

- Adding a new `FractalType` for Julia authority.
- Changing standalone `julia` semantics.
- Adding owner-specific Julia constants to `explaino_all`.
- Generated/internal editors.
- Equation-pack viewport integration.
- Perturbation zoom.
- Color Pipeline, capture finding, and FPS pacing changes.
