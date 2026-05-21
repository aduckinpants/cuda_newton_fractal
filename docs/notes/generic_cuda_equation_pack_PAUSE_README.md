# Generic CUDA Equation Pack - Pause README

Last updated: 2026-05-21
Branch paused from: `codex/engine-architecture-review`
Merge target: `master`
Validated feature head before pause: `72fa443 Route equation pack viewport color through pipeline`

## What Is Shipped Now

The repo now contains the first usable Generic CUDA equation-pack vertical inside the viewer-host codebase.

Shipped surfaces:

- Pack schema version `1` for AST-authored equation packs.
- Native pack parser/lowerer that produces `GenericFunctionDesc` without using raw expression text as execution authority.
- Workbench runner and example packs under `docs/examples/equation_packs/`.
- Interactive Controls-window workbench entry point.
- No-mouse automation for pack JSON, pack controls, reset defaults, and preview/report surfaces.
- Preview canvas backed by `SampleGenericFunction` output.
- Normal Fractal Type dropdown lane: `generic_equation_pack`.
- Left Controls flow for JSON pack text, apply action, iteration steps, and pack-defined controls.
- Main viewport bridge that evaluates the selected pack through `SampleGenericFunction`.
- Generic equation-pack viewport coloring now routes through the existing Color Pipeline semantics instead of a private palette.

Important boundary: this is real formula execution through the existing Generic CUDA evaluator path. It is not dynamic CUDA kernel registration, not a new live renderer enum switch per user formula, and not Salticid `sample_fn` lowering yet.

## Current Product Shape

The feature is intentionally usable but still an early vertical.

The current intended operator flow is:

1. Select `Generic Equation Pack` from the normal fractal dropdown.
2. Open the Controls surface.
3. Edit or paste a v1 equation-pack JSON document.
4. Apply the pack.
5. Adjust pack controls and the Color Pipeline controls.
6. Use the main viewport for visual feedback.

The detached workbench is scaffolding and diagnostics. The normal dropdown plus left Controls flow is now the product direction.

## Proof Already Recorded

The closed slices recorded machine proof for:

- AST pack parsing/lowering and malformed-pack rejection.
- CPU/CUDA generic sampler parity for representative packs.
- Workbench request/response/gallery artifact creation.
- In-process, no-mouse UI automation.
- Persistent one-process runtime tests for controls and reset defaults.
- Integer normalization for iteration controls.
- Main viewport bridge selection through `generic_equation_pack`.
- Color Pipeline control sensitivity on the generic equation-pack published viewer path.
- Existing Color Pipeline guard rails.
- Full native helper suite on the final Color Pipeline integration slice.

Key recent plans and contracts:

- `docs/notes/generic_cuda_equation_pack_workbench_PHASED_PLAN.md`
- `docs/notes/generic_cuda_equation_pack_interactive_ui_PHASED_PLAN.md`
- `docs/notes/generic_cuda_equation_pack_preview_canvas_PHASED_PLAN.md`
- `docs/notes/generic_cuda_equation_pack_controls_report_reset_PHASED_PLAN.md`
- `docs/notes/generic_cuda_equation_pack_integer_control_repair_PHASED_PLAN.md`
- `docs/notes/generic_cuda_equation_pack_main_viewport_PHASED_PLAN.md`
- `docs/notes/generic_cuda_equation_pack_color_pipeline_PHASED_PLAN.md`

## Paused Work

These are not shipped and should not be described as complete:

- Saved-state persistence for edited equation packs.
- A first-class pack catalog and named pack picker.
- Salticid `sample_fn` / `>>` adapter that emits this v1 pack schema.
- Mainline shared-CUDA ABI integration.
- Dynamic kernel generation or dynamic CUDA registration.
- Rich editor UX for pack AST authoring.
- Performance work beyond the current Generic CUDA evaluator constraints.
- Deeper error recovery and diagnostics for nontechnical users.

## Resume Order

When this feature resumes, use this order unless new evidence changes the tradeoff:

1. Persistence: save and reload the active pack with viewer state.
2. Catalog: add a small curated pack picker instead of requiring JSON paste for every session.
3. Authoring UX: add a safer text/editor surface that still lowers to the v1 AST pack authority.
4. Salticid adapter: lower the limited expression/composition pack surface into this schema.
5. Performance: profile where `SampleGenericFunction` is enough and where a later compiled kernel path is required.
6. Mainline merge work: map the pack schema and descriptor contract into the Salticid/shared-CUDA architecture.

## Do Not Reopen Accidentally

Do not reopen these as part of a small QoL or viewer-polish slice:

- Dynamic kernel registration.
- Generic renderer replacement.
- Salticid parser integration.
- New fractal-family catalog expansion.
- Broad Color Pipeline redesign.
- Physical mouse automation tests.

Small viewer QoL should happen on a fresh branch after the merge and should keep this feature paused unless the QoL item directly affects the shipped generic-pack path.

## Validation Reference

The final Color Pipeline integration slice closed with these relevant rails:

- `test_generic_equation_pack_live`
- `advanced_color_grading_red`
- `ui_app/build_vsdevcmd.cmd`
- `tests/test_fractal_runtime_generic_equation_pack_interactive_ui.py`
- full `ui_app/build_tests_vsdevcmd.cmd`
- plan sync, hostile-audit validation, contract proof, validation receipts, push, and clean-tree proof

The published runtime for the final proof was staged at:

`D:\\salt-fractal\\cuda_newton_fractal_clone\\runtime\\fractal_ui.exe`
