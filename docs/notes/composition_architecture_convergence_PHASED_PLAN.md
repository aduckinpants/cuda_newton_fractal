# Composition Architecture Convergence Campaign

## Explicit User Asks

- [x] Merge the closed root-field preset dropdown repair into `master` before opening this campaign.
- [x] Join Wave 4 graph UI, SDF field/source composition, and Color Pipeline recipe/composition authority into one checked-in implementation order.
- [x] Preserve the current linear Color Pipeline UI as the visible projection for now.
- [ ] Implement Slice A by promoting existing `recipe_v2` graph metadata into one safe non-UI recipe-application authority seam.
- [ ] Prove parity, fallback, runtime behavior, hostile audit, receipts, push, and stop before visible graph UI or broader SDF product growth.

## Current Phase

Phase 5 - recipe_v2 graph projection, fallback, native proof, and published no-mouse runtime proof are green; hostile audit and checkpoint closure are in progress.

## Phase Checklist

- [x] Phase 0: Bootstrap, repo status, and rearward review on closed dropdown repair head.
- [x] Phase 0: Fast-forward `master` to `d0e901b` and push `master`.
- [x] Phase 0: Create `codex/composition-architecture-convergence`.
- [x] Phase 1: Create this convergence plan and contract.
- [x] Phase 1: Lock the active slice with `viewer_host_begin_work_slice.py` (`ck:8c00bebb`).
- [x] Phase 2: Add RED/native parity coverage for graph-projected recipe lanes and fallback authority.
- [x] Phase 3: Implement recipe application through `recipe_v2` graph projection with legacy fallback switch.
- [x] Phase 4: Run focused native and published no-mouse runtime proof.
- [ ] Phase 5: Hostile audit, checkpoint, receipts, rearward review, push, clean tree, and stop for replan.

## Scope Lock

In scope:

- Checked-in convergence roadmap and Slice A contract.
- Current Color Pipeline recipe application only.
- Existing `recipe_v2` graph metadata and its linear `ui_projection`.
- A temporary fallback/kill switch that restores legacy recipe tuple behavior.
- Native/runtime proof that visible dropdown/apply workflow and recipe output do not change.

Out of scope:

- Visible graph editor UI.
- Arbitrary graph recipe rendering.
- New Color Pipeline functions, SDF ops, fractal lanes, field producers, or Salticid runtime execution.
- Switching authored live Source/Shape/Palette/Grading row execution to graph authority.
- Broad SDF field-generation/downsample optimization.
- Physical mouse automation.

## Convergence Model

This campaign treats the current linear Color Pipeline as a projection of a typed composition graph:

- Nodes: Source, Shape, Palette, Grading, future SDF field capability, and future applicator/mask nodes.
- Edges: typed signal/color/field/mask flows resolved by existing typed-edge metadata and adapter policy.
- UI projection: current linear Source/Shape/Palette/Grading row editor and recipe dropdown.
- Runtime authority transition: switch one proven seam at a time, starting with recipe application because `recipe_v2` already mirrors every current recipe.
- Graph UI: deferred until graph-backed recipe/application authority is proven through the current UI.

## Slice A - Recipe Graph Authority Switch

Intent:

- Use materialized `recipe_v2` as the internal recipe application authority.
- Convert resolved `recipe_v2` nodes/edges with `ui_projection="linear_color_stack"` into the same four lane ids the legacy recipe tuple produced.
- Keep current recipe metadata as compatibility/fallback and parity oracle.

Implementation requirements:

- Add a graph-projection helper that validates a `recipe_v2` record has exactly the expected linear nodes and edges and returns `{source, shape, palette, grading}`.
- `TryBuildColorPipelineRecipeLanes(...)` should prefer graph-projected recipe lanes when the materialized contract is active and the fallback switch is not enabled.
- Add a fallback switch with a stable reportable name, default off, that forces legacy recipe tuple authority.
- `ColorPipelineRecipeExpansionAuthorityId()` should report graph authority when the graph path is active, and legacy/materialized authority when fallback is forced or graph metadata is unavailable.
- Invalid or partial graph metadata must fail closed with a specific error, not silently substitute another recipe.
- Visible controls stay as the dropdown plus `Apply Recipe`; no new user-facing controls are added for the fallback switch.

## Slice B Spec - Source Stack Graph Receipts

Decision-complete follow-up target after Slice A:

- Emit graph-shaped runtime report and Capture Finding review data for live authored Source/Shape/Palette/Grading row stacks.
- Include Source-row enabled state, applicator mode, SDF gate width, row-local SDF field downsample, blend weight, selected function ids, and fail-closed reasons.
- Keep current row execution live authority; this is receipt/report authority only.
- Prove reports match current runtime row behavior and old captures remain loadable.

## Slice C Spec - SDF Field Capability Graph Model

Decision-complete follow-up target after Slice B:

- Represent Lens SDF, Lens Field v2, authored SDF packs, `sdf_pack_scene`, Root SDF, and root-field producers as typed field-capability nodes.
- Represent SDF Source rows as sampled signal consumers of those field nodes.
- Preserve current fail-closed behavior for field-primary lanes with unsupported non-SDF source rows until real renderer-backed scalar planes exist.
- Do not add new SDF ops or SDF-native lanes under this modeling slice.

## Proof Ledger

| Item | Status | Evidence |
| --- | --- | --- |
| Source repair merge | done | `master` fast-forwarded and pushed from `9786b98` to `d0e901b`. |
| Branch | done | `codex/composition-architecture-convergence`. |
| Plan/contract | in progress | This plan and `docs/contracts/composition_architecture_convergence.contract.json`. |
| Active lock | done | `viewer_host_begin_work_slice.py`, checkpoint `ck:8c00bebb`. |
| Native proof | done | `composition_architecture_convergence_native`: `test_color_pipeline_core` passed 3325/0 and `test_color_pipeline_window` passed 433/0. |
| Runtime proof | done | `tests/test_fractal_runtime_color_pipeline_presets.py` plus `test_published_runtime_consumes_staged_ui_salt_contract` passed 4/4 after runtime publish. |
| Hostile audit | pending | Required before checkpoint. |
| Receipts/rearward/push | pending | Required before closeout. |

## Hostile Audit

- Status: complete
- Required posture: assume the graph switch changes recipe output, silently falls back, accepts unsupported graph routes, leaks graph UI scope, breaks capture/replay, or leaves stale plan text until tests prove otherwise.

Required questions:

- Did recipe application actually use `recipe_v2` when graph authority is active?
- Did every current recipe project to the same Source/Shape/Palette/Grading lanes as before?
- Did the fallback switch restore legacy recipe behavior?
- Did invalid graph metadata fail closed instead of silently substituting legacy output?
- Did visible Color Pipeline UI and no-mouse IDs remain unchanged?
- Did SDF, root-field, and phase recipes still apply through the published runtime?
- Did the slice avoid graph UI, arbitrary graph rendering, new functions, new SDF ops, Salticid runtime dependency, and physical mouse automation?

## Audit Passes

- [x] Pass 1 - review graph projection/parity seam; found missing negative projection coverage and added fail-closed tests for `shadow_only` and edge-function mismatch.
- [x] Pass 2 - review fallback/fail-closed behavior; fallback switch now has a stable id and native proof of `materialized_json_legacy_recipe_tuple` authority.
- [x] Pass 3 - review runtime/UI surfaces; found stale no-mouse contract count expecting 4 recipes while published runtime reports 6, then repaired the test and reran runtime proof.
- [x] Pass 4 - clean re-read after repairs; no visible UI scope leak, graph editor work, arbitrary graph recipes, new functions, SDF ops, or Salticid runtime dependency added.

## Audit Findings

- [x] Finding 1: Native graph projection had no direct negative test for malformed `recipe_v2` projection metadata. Added `TestColorPipelineRecipeV2ProjectionFailsClosedForInvalidGraph`; native rail passed `test_color_pipeline_core` 3325/0.
- [x] Finding 2: Published runtime UI-Salt contract test was stale and expected 4 active recipes while the staged contract/runtime correctly expose 6. Updated the no-mouse test to assert 6 recipes and reran runtime proof 4/4.
- [x] Clean pass: fallback authority is explicit as `materialized_json_legacy_recipe_tuple`, graph authority is explicit as `recipe_v2_graph`, and visible Color Pipeline UI remains the unchanged dropdown/apply workflow.

## Planned Validation Targets

- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/composition_architecture_convergence.contract.json --out-json artifacts/validation/composition_architecture_convergence_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/composition_architecture_convergence_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label composition_architecture_convergence_native --log artifacts/logs/composition_architecture_convergence_native.log --out-json artifacts/validation/composition_architecture_convergence_native.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core test_color_pipeline_window`
- `py -3.14 tools/viewer_host_run_logged_command.py --label composition_architecture_convergence_runtime_publish --log artifacts/logs/composition_architecture_convergence_runtime_publish.log --out-json artifacts/validation/composition_architecture_convergence_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_vsdevcmd.cmd`
- `py -3.14 -m pytest tests/test_fractal_runtime_color_pipeline_presets.py tests/test_fractal_runtime_ui_salt_contract.py::test_published_runtime_consumes_staged_ui_salt_contract -q --junitxml artifacts/pytest/composition_architecture_convergence_runtime.junit.xml`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/composition_architecture_convergence_PHASED_PLAN.md --out-json artifacts/validation/composition_architecture_convergence_hostile_audit.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label composition_architecture_convergence_diff_check --log artifacts/logs/composition_architecture_convergence_diff_check.log --out-json artifacts/validation/composition_architecture_convergence_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`

## Stop Point

Stop after Slice A is graph-authoritative, validated, hostile-audited, checkpointed, pushed, and clean. Preplanned sliced work is exhausted at that point except for the checked-in Slice B/C specs; stop for replan before visible graph UI, arbitrary graph rendering, or broader SDF product growth.

