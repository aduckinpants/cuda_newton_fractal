# PFC Explaino Seed Responsiveness Repair

## Explicit User Asks

- Triage the reported issue that `explaino_julia`, and possibly other Explaino lanes, do not respond to the visible Seed value.
- Do this before starting Step 9 so the campaign does not build on a reported regression from Step 8.
- Keep the repair narrow; do not start generated/internal editors, equation-pack work, Color Pipeline work, capture finding work, FPS pacing, or broad engine redesign in this slice.
- Use no-mouse runtime proof.

## Current Phase

Phase 5 is open: closure validators, checkpoint, receipts, push, and holder integration are next.

## Phase Checklist

- [x] Bootstrap on clean holder branch `codex/parameter-functionality-campaign` at `a05bb8d`.
- [x] Create feature branch `codex/pfc-explaino-seed-responsiveness-repair`.
- [x] Create this checked-in plan and contract.
- [x] Lock the active slice to this seed-responsiveness repair.
- [x] RED: prove `explaino_julia` Seed can be inert through the shipped public path.
- [x] RED/audit: classify other direct Explaino lanes for seed responsiveness without broadening the slice.
- [x] Implement the narrowest default/authority repair.
- [x] Add native sample/default tests proving Seed affects `explaino_julia` in seeded and custom constant modes.
- [x] Add or update no-mouse published-runtime proof for the visible Seed slider on `explaino_julia`.
- [ ] Validate focused native rails, runtime publish, no-mouse runtime proof, full native helper suite if focused rails pass, hostile audit, receipts, push, and holder integration. Focused/native/runtime rails are green; hostile-audit validators, receipts, push, and holder integration remain.

## Owner Seams

- Defaults and derived roots: `ui_app/src/fractal_derived_fields.cpp`.
- Runtime math: `ui_app/src/fractal_sample_device.inl`, `ui_app/src/fractal_probe_runner.cpp`.
- Binding/edit path: `ui_app/src/schema_binding.cpp`.
- UI schema: `ui/fractal_binding_surface_v1.ui_schema.json`.
- Native tests: `ui_app/tests/test_fractal_derived_fields.cpp`, `ui_app/tests/test_fractal_sample_kernel.cu`, `ui_app/tests/test_schema_binding.cpp`.
- Runtime proof: `tests/test_fractal_runtime_explaino_julia_authority.py` or a dedicated seed-responsiveness runtime pytest.

## Design Boundary

- Seed responsiveness means the visible `explaino_seed` control changes rendered/sample output on an owning Explaino lane.
- Custom Explaino Julia constants may keep their custom `c`, but Seed must still have an authoritative effect through the Explaino warp/start surface if the Seed control remains visible.
- Do not add new fractal types.
- Do not move owner-specific controls onto `explaino_all`.
- Do not change standalone `julia`.
- Do not change Color Pipeline, capture finding, FPS pacing, equation-pack viewport integration, perturbation, or generated/internal editor behavior.

## Proof Ledger

- Source branch: `codex/parameter-functionality-campaign`.
- Starting head: `a05bb8d`.
- Feature branch: `codex/pfc-explaino-seed-responsiveness-repair`.
- Contract lock: `pfc_explaino_seed_responsiveness_repair` locked with checkpoint `ck:0b543bf6`.
- RED proof: `pfc_explaino_seed_responsiveness_repair_red_derived` failed because `explaino_julia` defaulted `explaino_warp_strength` to `0.0`, making the visible Seed control inert once custom Julia constants stopped deriving `c` from Seed.
- Additional RED/audit proof: `pfc_explaino_seed_responsiveness_repair_sample` failed on integer Seed values because warp-only paths used `LogisticAreaUToSeed`, whose documented contract intentionally ignores integer components.
- Implementation facts: `explaino_julia` now defaults seed-driven warp to `0.25`, and Explaino warp-start paths use `ExplainoCombinedSeedToWarpSeed` so integer Seed changes affect warp without changing the legacy fractional `LogisticAreaUToSeed` contract.
- Other-lane classification: root-surface Explaino lanes still derive authority through `UpdateExplainoPolynomial`; direct warp-start lanes now consume full combined Seed through the shared warp seed helper; explicitly warp-disabled defaults such as Nova/DualSeed remain covered by existing default-authority tests and were not widened in this repair.
- Focused native rails: `pfc_explaino_seed_responsiveness_repair_derived`, `pfc_explaino_seed_responsiveness_repair_sample`, and `pfc_explaino_seed_responsiveness_repair_schema` passed.
- Runtime publish: `pfc_explaino_seed_responsiveness_repair_runtime_publish` passed.
- No-mouse runtime proof: `pfc_explaino_seed_responsiveness_repair_runtime_pytest` passed.
- Full native helper suite: `pfc_explaino_seed_responsiveness_repair_full_native` passed.
- Hostile audit findings: two real findings recorded below; repaired-state clean re-read complete; final validators pending.

## Hostile Audit

- Status: complete
- Did I actually prove and repair the reported Seed inertness instead of starting Step 9 over it?
- Did I preserve Step 8 Explaino Julia seeded/custom constant authority?
- Did I keep `explaino_all` behavior unchanged?
- Did I preserve standalone `julia` behavior?
- Did I classify other likely affected Explaino lanes without broadening into unrelated work?
- Did I avoid Color Pipeline, capture finding, FPS pacing, equation-pack, perturbation, generated-editor, or broad engine drift?
- Did I close with receipts, push, clean tree, and no stale plan text?

## Audit Passes

- [x] Pass 1: confirmed the original default-authority bug in `ApplyExplainoPresetDefaults`; Julia custom constants preserved `c`, but visible Seed had no active output path because warp was disabled.
- [x] Pass 2: confirmed a second seed-routing bug in warp-only Explaino paths; integer Seed changes were discarded by the fractional logistic helper.
- [x] Pass 3: re-read the repaired state after focused, runtime, and full native validation; no additional real defect found in the bounded seed-responsiveness slice.

## Audit Findings

- [x] Finding 1: `explaino_julia` defaulted `explaino_warp_strength` to `0.0`, so the visible Seed slider could be inert in custom constant mode. Fixed by enabling the same seed-driven warp default already used by the direct Explaino Julia-like lanes.
- [x] Finding 2: `LogisticAreaUToSeed` intentionally ignores integer seed components, so integer Seed edits such as `2.0` to `7.0` could remain visually inert on warp-only paths. Fixed by adding `ExplainoCombinedSeedToWarpSeed` for warp routing while preserving the existing fractional logistic contract and tests.

## Out Of Scope

- Step 9 generated/internal editor implementation.
- Equation-pack viewport integration.
- Perturbation zoom.
- Color Pipeline, capture finding, and FPS pacing changes.
- Standalone Julia changes.
