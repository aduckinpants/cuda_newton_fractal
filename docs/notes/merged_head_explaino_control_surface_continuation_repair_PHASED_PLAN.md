# Merged-Head Explaino Control-Surface Continuation Repair

## Current Phase

Closed - the shipped Explaino owner-lane control surface is repaired, the hostile audit is complete, the focused native/runtime proof is green, implementation head `661e0d7` is receipted and pushed on `master`, and this follow-up sync leaves the active plan in truthful closed-state wording.

## Phase Checklist

- [x] Phase 1 - prove current-source selector identity and smooth-escape seams still pass, prove the raw shipped schema strands the seven family-axis controls on `explaino_all`, and prove the current native schema rail masks that artifact by rewriting it inside `LoadUISchemaFromJson(...)`.
- [x] Phase 2 - repair the shipped schema so `ripple_amplitude`, `splice_offset`, `vortex_strength`, `tension_strength`, `balance_void`, `symmetry_tension`, and `field_curvature` live only on their explicit owning Explaino lanes while `explaino_all` stays neutral.
- [x] Phase 3 - repair native and published-runtime proof so the raw schema artifact, loaded schema, and published runtime schema copy all enforce the owner-lane contract while selector identity and the smooth-escape resolver rails stay green.
- [x] Phase 4 - hostile-audit the repaired head, restore the temporary bridge scope, run focused native/runtime proof plus contract validation and phased-plan sync, checkpoint, write receipts, push `master`, and clear stale-plan text.

## Explicit User Asks

- [done] Restore the stranded Explaino family-axis controls to their explicit owner lanes: `explaino_ripple -> ripple_amplitude`, `explaino_splice -> splice_offset`, `explaino_vortex -> vortex_strength`, `explaino_tension -> tension_strength`, and `explaino_balance_void -> balance_void/symmetry_tension/field_curvature`.
- [done] Keep explicit Explaino selector identity intact and keep `explaino_all` as a truthful neutral umbrella that does not claim those family-axis controls.
- [done] Preserve the bounded smooth-escape explicit-lane perf repair and keep `SampleFractalPoints(...)` plus legacy projection callers untouched.
- [done] Repair the native schema/binding proof surface so this exact raw-schema regression is caught instead of normalized away.
- [done] Closed with focused native proof, runtime publish, published-runtime proof, hostile audit, checkpoint commit, machine receipts, push, clean tree, and stale-plan cleanup.

## Proof Ledger

- Bootstrap on 2026-05-18 proved branch=`master`, `HEAD=6937770`, clean tree, the repo is local-only ahead of `origin/master` by two commits, and the active locked contract still points at the already closed `merged_head_explaino_selector_control_smooth_escape_regression_repair` slice.
- Current-source selector identity is still green: rebuilt `artifacts/build_targeted/test_schema_binding_current.exe` from the checked-in `ui_app/tests/test_schema_binding.cpp`, and the run log reports `test_schema_binding: all passed`.
- Current-source smooth-escape resolver protection is still green: rebuilt `artifacts/build_targeted/test_sample_tier_resolver_current.exe` from the checked-in `ui_app/tests/test_sample_tier_resolver.cpp`, and the run log reports `sample_tier_resolver: all tests passed`.
- The raw shipped schema artifact was wrong on the continuation RED head: direct JSON inspection of `ui/fractal_binding_surface_v1.ui_schema.json` showed all seven family-axis controls gated with `visible_if.value == "explaino_all"`.
- The current native schema rail masked that artifact on the RED head instead of catching it: `ui_app/src/ui_schema.cpp` rewrote Explaino axis `visible_if` ownership through `apply_explaino_axis_registry(...)`, so a fresh rebuild of `artifacts/build_targeted/test_ui_schema_current.exe` still passed against the normalized in-memory schema even while the raw file remained wrong.
- `ui_app/src/viewer_schema_load.cpp` routes the live viewer and describe-functions paths through `LoadUISchemaFromJson(...)`, so the bounded continuation repair stayed inside the shipped schema artifact plus proof seams without widening into runtime math or selector-routing changes.
- `SampleFractalPoints(...)` stayed untouched because the contradiction lived in the schema artifact and schema proof masking seams, while the current selector-identity and sample-tier resolver tests remained green on rebuilt current-source binaries.
- The shipped Explaino owner-lane schema artifact is now repaired in `ui/fractal_binding_surface_v1.ui_schema.json`: the four single-axis controls are fenced to `explaino_ripple`, `explaino_splice`, `explaino_vortex`, and `explaino_tension`, while `balance_void`, `symmetry_tension`, and `field_curvature` are fenced to `explaino_balance_void`.
- The native schema rail now proves the raw artifact instead of only the normalized in-memory schema: `ui_app/tests/test_ui_schema.cpp` inspects the raw parsed JSON before `LoadUISchemaFromJson(...)`, and a fresh rebuild of `artifacts/build_targeted/test_ui_schema_current.exe` passes with that raw-file assertion live.
- The temporary bridge scope was restored immediately after the continuation contract locked: `git diff -- docs/contracts/merged_head_explaino_selector_control_smooth_escape_regression_repair.contract.json` is empty on the repaired head.
- Focused current-source proof is green on the repaired head: `test_ui_schema: all passed`, `test_schema_binding: all passed`, and `sample_tier_resolver: all tests passed` from the rebuilt or rerun targeted binaries under `artifacts/build_targeted`.
- Runtime publish completed successfully on the repaired head via `ui_app/build_vsdevcmd.cmd`, and direct inspection of `D:/salt-fractal/cuda_newton_fractal_clone/runtime/ui/fractal_binding_surface_v1.ui_schema.json` now shows the seven Explaino family-axis controls on their explicit owner lanes instead of `explaino_all`.
- Focused published-runtime proof is green on the repaired head: `artifacts/logs/explaino_control_surface_runtime_pytest.log` reports `13 passed in 8.32s` across the new published runtime schema-copy witness, explicit composed-lane identity/default witnesses, the `explaino_all` neutral-umbrella witness, the `explaino_balance_void` owner-lane witness, the smooth-escape usability witness, the float32 backend witness, and the describe-functions catalog witness.
- Hostile audit found one real defect in the first repair pass: the new published runtime schema-copy pytest lacked the Windows-only skip used by the rest of the runtime-published artifact suite. Repaired that guard and reran the entire focused published-runtime set green.
- The implementation checkpoint commit is `661e0d7`, validation receipt `artifacts/hooks/viewer_host_validation_receipts/661e0d7d46b2e17eb2bf914de82c81c07bf039e4.json` and contract proof receipt `artifacts/hooks/viewer_host_contract_proof_receipts/661e0d7d46b2e17eb2bf914de82c81c07bf039e4.json` were written clean, and `master` was pushed to `origin`.
- The mandated stale-plan gate on the pushed implementation head found one lingering open closeout ask in this plan; this continuity follow-up removes that stale text so the active plan matches the actual closed state.

## Hostile Audit

- Status: complete
- Required posture: assume selector identity, owner-lane visibility, raw schema truth, published runtime schema truth, smooth-escape perf preservation, plan text, and my own summary are wrong until machine proof says otherwise.
- Hostile review questions:
  Did I actually restore the controls to the owning explicit Explaino family lanes?
  Did I actually keep explicit selector identity intact?
  Did I actually keep `explaino_all` neutral instead of letting it reclaim family-axis ownership?
  Did I actually preserve the smooth-escape explicit-lane perf repair?
  Did I preserve `SampleFractalPoints(...)` as the shipped legacy projection path?
  Did I silently widen into unrelated architecture work?
  Did I restore the temporary bridge scope and stop with truthful closeout text?

## Audit Passes

- [x] Pass 1 - land the raw-schema RED and the owner-lane schema repair.
- [x] Pass 2 - rerun the focused native and published-runtime proof rails on the repaired head.
- [x] Pass 3 - clean re-read the repaired schema/proof/closeout surfaces after the Windows-only skip fix and confirmed the repaired state without exposing another real defect or workflow mistake.

## Audit Findings

- [x] Hostile audit found one real defect in the first continuation pass: the new published runtime schema-copy pytest was missing the Windows-only skip used by the rest of the runtime-published artifact suite. Repaired the guard and reran the focused published-runtime proof green.
