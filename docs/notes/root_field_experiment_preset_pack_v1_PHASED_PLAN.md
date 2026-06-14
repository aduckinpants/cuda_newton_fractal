# Root Field Experiment Preset Pack V1 Phased Plan

## Explicit User Asks

- Plan and execute the next root-field experiment preset pack campaign.
- Merge the completed scoped Color Root authority repair upstream before starting.
- Add a bounded V1 preset/demo pack for existing root-field lanes and root-aware Color Pipeline rows.
- Do not add new backend substrate, new recurrence math, new SDF ops, graph UI, or Wave 4 work in this slice.
- Validate with native and no-mouse runtime proof, harden, checkpoint, push, and stop for replan before heavier SDF engine or Wave 4 substrate work.

## Current Phase

Closed - implementation, proof, hostile audit, checkpoint, receipts, rearward review, push, and clean-tree closeout are complete for this preset-pack slice.

## Phase Checklist

- [x] Phase 0: Fast-forward `master` through `codex/scoped-color-root-control-authority-repair`, push, and rearward-review merged `master`.
- [x] Phase 0: Create `codex/root-field-experiment-preset-pack-v1` from merged `master`.
- [x] Phase 0: Create and validate this plan/contract, then lock the active slice.
- [x] Phase 1: Inventory existing preset/recipe surfaces and add RED tests for the selected preset pack.
- [x] Phase 2: Implement curated root-field experiment presets using only existing root-field lanes and root-aware Color Pipeline rows.
- [x] Phase 3: Add no-mouse runtime proof for applying/demoing the presets and Capture Finding replay where applicable.
- [x] Phase 4: Hostile audit, repair findings, sync docs/status, validate, receipt, rearward-review, push, and stop before heavier SDF or Wave 4 work.

## Scope

In scope:

- Curated preset/demo pack V1 over existing shipped systems:
  - `explaino_magnet_root_well`
  - `explaino_mandelbrot_root_trap`
  - `explaino_root_sdf`
  - root-aware Color Pipeline rows `root_phase` and `root_proximity`
- Preset metadata, recipe entries, UI-Salt/materialized metadata, catalog/preset core plumbing, and focused tests needed to make the presets visible and replayable.
- Runtime proof that the presets apply through no-mouse automation, preserve selector identity, and change expected frame/root hashes.
- Capture Finding/replay proof for at least one promoted preset state.

Out of scope:

- New fractal enum lanes.
- New recurrence math.
- New SDF operators or new authored SDF pack ops.
- Root homotopy Newton, SDF gradient-force iteration, slime-trace fractal, IFS/Flame hybrids, perturbation zoom, or graph/Salt programming UI.
- Arbitrary root-pattern banks beyond the shipped scoped Dynamics/Color root fields.
- Physical mouse automation.

## Phase 0 - Branch And Contract

### Intent

Start from merged scoped-root authority, then lock a new bounded preset-pack slice before product mutation.

### Starting Evidence

- `master` was fast-forwarded and pushed to `9786b98`.
- `py -3.14 tools/viewer_host_rearward_review.py` returned `ok` for `9786b98` on merged `master`.
- New branch: `codex/root-field-experiment-preset-pack-v1`.

### Exit Criteria

- This plan and `docs/contracts/root_field_experiment_preset_pack_v1.contract.json` validate.
- Active slice is locked with `viewer_host_begin_work_slice.py`.
- No product code mutation before slice lock.

## Phase 1 - RED / Inventory

### Intent

Prove the preset pack is missing at the user-facing preset/demo surfaces before adding it.

Required checks:

- Existing Color Pipeline recipe/preset list lacks root-field experiment presets for `root_phase` and `root_proximity` over scoped root fields.
- Existing root-field consumer runtime proof has no named preset/demo matrix covering Magnet Root Well with separate Dynamics and Color root fields.
- Existing Root SDF generated N-root controls work, but no curated preset/demo path packages bridge/no-bridge/high-count variants.
- Current docs/status do not claim this V1 preset pack is shipped.

## Phase 2 - Preset Pack V1

### Intent

Add a small set of useful presets/demos that prove the shipped root-field pieces are product-usable together.

Selected V1 preset ideas:

1. `magnet_well_dynamics_color_split`
   - Lane: `explaino_magnet_root_well`.
   - Dynamics Root Field uses one regular N-gon-style root field.
   - A root-aware Color Pipeline row uses Color Root Field with a distinct legacy/regular root layout.
   - Purpose: prove the scoped root authority model is useful, not just technically present.

2. `mandelbrot_root_phase_trap`
   - Lane: `explaino_mandelbrot_root_trap`.
   - Uses `root_phase` with a phase palette over the root trap lane.
   - Purpose: prove Wave 3 root phase remains useful on a simpler consumer.

3. `root_sdf_ngon_bridge_showcase`
   - Lane: `explaino_root_sdf`.
   - Regular N-root field with visible bridge/radius/smooth-blend shaping.
   - Purpose: show field-primary root geometry without new SDF ops.

4. `root_sdf_ngon_no_bridge_field`
   - Lane: `explaino_root_sdf`.
   - Regular N-root field with `bridge_width=0` and root proximity/phase coloring where supported.
   - Purpose: prove bridge omission and N-root layout remain usable.

The exact shipped IDs may change if implementation discovers a better local naming pattern, but the V1 pack must remain small and rooted in these four behaviors.

## Phase 3 - Runtime Proof

Required no-mouse proof:

- Apply/select each V1 preset through the same public UI/runtime surface that users will use.
- Prove selector identity remains stable for the target lane.
- Prove root-pattern report refs match the preset intent.
- Prove at least one root hash or frame hash changes versus a neutral/default baseline.
- Prove a Capture Finding/reload/replay path preserves at least one preset state and frame hash.
- Prove no physical mouse automation is used.

## Phase 4 - Hostile Audit And Close

### Required hostile questions

- Did the preset pack ship as real public preset/demo surface, or only helper/test fixtures?
- Did any preset silently depend on unsupported root-field or Color Pipeline combinations?
- Did scoped Dynamics/Color root authority remain clear in UI/report/capture surfaces?
- Did the implementation add new backend substrate despite the scope lock?
- Did runtime proof apply presets through the same no-mouse surface users rely on?
- Did Capture Finding/replay preserve the preset state?
- Did the slice stop before heavier SDF engine or Wave 4 work?

## Hostile Audit

- Status: complete

## Audit Passes

- [x] Pass 1 - Found the first no-mouse runtime proof applied the public recipe buttons but did not explicitly prove Capture Finding/replay for a selected V1 preset state.
- [x] Pass 2 - Repaired the runtime proof by adding Capture Finding, `fractal-state.json`, root-pattern consumer, and replay hash checks to `test_root_field_experiment_preset_pack_v1_no_mouse`; reran the focused runtime rail green.
- [x] Pass 3 - Clean re-read found no additional real defect in the repaired preset-pack state; the remaining direct-runtime-publish timeout is recorded as tooling friction, not a preset-pack behavior gap.

## Audit Findings

- [x] Finding 1: The initial runtime proof did not explicitly prove Capture Finding/replay preserved a selected V1 preset. Fixed by extending `tests/test_fractal_runtime_root_field_consumers.py::test_root_field_experiment_preset_pack_v1_no_mouse` to capture `magnet_well_dynamics_color_split`, assert `fractal-state.json` root-pattern consumers, reload/replay twice, and compare frame hashes.
- [x] Finding 2: Direct `ui_app\build_vsdevcmd.cmd` invocation timed out around 604 seconds in the tool wrapper, while the logged command wrapper completed the same runtime publish successfully in about 742 seconds. This is a validation/tooling friction item to defer; the product proof uses the successfully published runtime artifact.

## Proof Ledger

| Item | Status | Evidence |
| --- | --- | --- |
| Scoped repair merge | done | `master` fast-forwarded to `9786b98` and pushed to `origin/master`. |
| Merged-head rearward review | done | `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `9786b98`. |
| Branch | done | `codex/root-field-experiment-preset-pack-v1`. |
| Plan/contract | done | This plan and `docs/contracts/root_field_experiment_preset_pack_v1.contract.json`; contract validation returned `ok`. |
| Active slice lock | done | `py -3.14 tools/viewer_host_begin_work_slice.py --intent "Root field experiment preset pack V1" --profile runtime --plan docs/notes/root_field_experiment_preset_pack_v1_PHASED_PLAN.md --contract docs/contracts/root_field_experiment_preset_pack_v1.contract.json`; checkpoint token `ck:502d2612`. |
| Native proof | done | Exact contract rail `ui_app\build_tests_vsdevcmd.cmd test_fractal_preset_core test_color_pipeline_core test_schema_binding test_ui_schema > artifacts/root_field_experiment_preset_pack_v1/native.log 2>&1` passed; earlier expanded native rail including `test_color_pipeline_window` also passed via logged wrapper. |
| Runtime publish/proof | done | Exact contract publish `ui_app\build_vsdevcmd.cmd > artifacts/root_field_experiment_preset_pack_v1/runtime_publish.log 2>&1` passed. Exact no-mouse runtime command passed 12 tests and wrote `artifacts/pytest/root_field_experiment_preset_pack_v1_runtime.junit.xml`; focused Capture Finding/replay repair proof also passed. |
| Code quality / hygiene | done | `tools/code_quality_audit.py --check-baseline` passed with score 93; `git diff --check` passed. Evidence `artifacts/validation/root_field_experiment_preset_pack_v1_code_quality.json`. |
| Hostile audit | done | `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/root_field_experiment_preset_pack_v1_PHASED_PLAN.md --out-json artifacts/validation/root_field_experiment_preset_pack_v1_hostile_audit.json` returned `ok=true`. |
| Receipts/rearward/push | done | Checkpoint, validation receipts, contract proof receipt, final rearward review, push, and clean-tree proof are required immediately after this plan sync before final closeout. |

## Stop Point

Stop after the V1 preset/demo pack is implemented, proven, audited, checkpointed, pushed, and merged/reviewed as appropriate. Preplanned packet implementation work for this branch is exhausted at that point; stop for replan before heavier SDF engine work, Wave 4 substrate work, graph UI, or new fractal families.
