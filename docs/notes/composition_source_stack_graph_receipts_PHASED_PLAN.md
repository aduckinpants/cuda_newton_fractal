# Composition Source-Stack Graph Receipts Slice

## Explicit User Asks

- [x] Merge the closed Slice A convergence branch to `master` if rearward review remains `ok`.
- [x] Branch from the updated `master` for Slice B.
- [ ] Add graph-shaped receipt/report authority for current live Color Pipeline row stacks.
- [ ] Emit the receipt in runtime/no-mouse automation reports as `color_pipeline_graph_receipt`.
- [ ] Emit the same receipt in Capture Finding `fractal-state.json` under `color_pipeline.graph_receipt`.
- [ ] Keep live execution, UI layout, recipe application, SDF behavior, row semantics, and `state.json` unchanged.
- [ ] Validate, hostile-audit, checkpoint, push, and stop for replan before Slice C, visible graph UI, or broader SDF product growth.

## Current Phase

Phase 5 - hostile audit is complete; checkpoint, receipts, rearward review, push, and clean-tree closure are in progress.

## Phase Checklist

- [x] Phase 0: Bootstrap, status, and rearward review on Slice A head `418d0a8`.
- [x] Phase 0: Fast-forward `master` to Slice A and push upstream.
- [x] Phase 0: Create `codex/composition-source-stack-graph-receipts`.
- [x] Phase 0: Truth-sync the stale Slice A convergence plan text.
- [x] Phase 1: Create and validate this Slice B plan and contract.
- [x] Phase 1: Lock the active slice with `viewer_host_begin_work_slice.py` (`ck:aa02cec8`).
- [x] Phase 2: Add RED/native tests for graph receipt shape, disabled rows, SDF params, source-stack order, and Capture Finding sidecar emission.
- [x] Phase 3: Implement the graph receipt builder and wire report-only emission.
- [x] Phase 4: Run focused native rails and published no-mouse runtime proof.
- [ ] Phase 5: Hostile audit, repair if needed, receipts, rearward review, push, clean tree, and stop for replan.

## Scope Lock

In scope:

- A deterministic Color Pipeline graph receipt builder for current linear row-stack state.
- Runtime/no-mouse automation report field `color_pipeline_graph_receipt`.
- Capture Finding `fractal-state.json` field `color_pipeline.graph_receipt`.
- Receipt nodes for Source, Shape, Palette, and Grading rows, including disabled rows.
- Receipt edges for active sequential row flow only.
- Unsupported-route reporting when the current live validation has failed-closed conditions.

Out of scope:

- Changing Color Pipeline row execution.
- Changing `state.json` replay authority.
- Changing recipe application or `recipe_v2_graph` authority from Slice A.
- Visible graph editor UI or arbitrary graph recipes.
- New Color Pipeline functions, SDF operators, fractal lanes, or Salticid runtime dependency.
- Physical mouse automation.

## Receipt Contract

The new receipt is review/report authority only. It describes the current linear Color Pipeline row-stack as a graph projection without replacing the live row executor.

Required object fields:

- `schema_id: "viewer.color_pipeline_graph_receipt.v1"`
- `execution_authority: "linear_row_stack"`
- `ui_projection: "linear_color_stack"`
- `source_stack_kind`
- `nodes[]`
- `edges[]`
- `unsupported_routes[]`

Node requirements:

- Stable deterministic id: `<lane>.<row_index>`.
- Fields include row index, lane id, function id, enabled state, parameter values, blend weight, SDF gate/applicator, SDF gate width, row-local SDF field downsample, root pattern ref, and fail-closed reason when inactive.
- Disabled rows are present as disabled nodes but do not claim active execution.

Edge requirements:

- Stable deterministic id: `<from_node>-><to_node>`.
- Edges represent active sequential row flow within each lane and lane-to-lane projection flow where a lane has active rows.
- Disabled rows are skipped by active edges.

Unsupported-route requirements:

- Existing validation/fail-closed messages are reported honestly.
- Unsupported field-primary/non-SDF routes remain fail-closed; this slice must not silently substitute graph behavior.

## Proof Ledger

| Item | Status | Evidence |
| --- | --- | --- |
| Slice A merge | done | `master` fast-forwarded to `418d0a8` and pushed before this branch. |
| Branch | done | `codex/composition-source-stack-graph-receipts`. |
| Plan/contract | done | This plan and `docs/contracts/composition_source_stack_graph_receipts.contract.json`; contract validation passed. |
| Active lock | done | `viewer_host_begin_work_slice.py`, checkpoint `ck:aa02cec8`. |
| Native proof | done | `composition_source_stack_graph_receipts_native`: focused native rail passed after audit fix. |
| Runtime proof | done | Runtime published and `test_color_pipeline_source_stack_graph_receipt_is_reported_no_mouse` passed. |
| Hostile audit | done | Pass 1 found invalid sidecar JSON separator and fixed it; passes 2 and 3 found stale ledger text only, then clean report-only diff. |
| Receipts/rearward/push | pending | Required before closeout. |

## Hostile Audit

- Status: complete
- Required posture: assume the graph receipt misrepresents execution, omits disabled rows, claims disabled rows executed, drops SDF gate/downsample/root-pattern data, changes replay state, or masks unsupported routes until proven otherwise.

Required questions:

- Did the receipt include every current Source/Shape/Palette/Grading row with stable ids?
- Did active edges skip disabled rows and preserve author order?
- Did Source-row SDF gate, gate width, row-local downsample, blend weight, and root pattern ref survive into the receipt?
- Did runtime automation report and Capture Finding sidecar emit the same receipt shape?
- Did `state.json`, live row execution, recipe application, SDF behavior, and UI layout remain unchanged?
- Did unsupported routes remain fail-closed and reportable instead of becoming silently accepted?
- Did the slice avoid graph UI, arbitrary graph execution, new functions, SDF ops, fractal lanes, Salticid runtime dependency, and physical mouse automation?

## Audit Passes

- [x] Pass 1 - review receipt builder and serialization seam; found invalid Capture Finding sidecar comma before `graph_receipt`, fixed and covered by native test.
- [x] Pass 2 - review report/capture sidecar parity and stale text; found stale proof-ledger text and synced it.
- [x] Pass 3 - clean re-read the repaired state for runtime/no-mouse proof and unsupported route handling; no additional real defect found, final diff is report-only, `git diff --check` is clean, and unsupported routes remain receipt diagnostics only.

## Audit Findings

- [x] Finding 1: Capture Finding `fractal-state.json` inserted `graph_receipt` without a comma after `color_grading_stack`; fixed separator and added native regression.
- [x] Finding 2: Plan proof ledgers still described completed validation as pending; synced plan truth before closeout.
- [x] Clean pass: confirmed the repaired state: runtime report and Capture Finding sidecar emit graph receipts without changing `state.json`, recipe application, live row execution, SDF behavior, or UI layout.

## Planned Validation Targets

- `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/composition_source_stack_graph_receipts.contract.json --out-json artifacts/validation/composition_source_stack_graph_receipts_contract.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/composition_source_stack_graph_receipts_code_quality.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label composition_source_stack_graph_receipts_native --log artifacts/logs/composition_source_stack_graph_receipts_native.log --out-json artifacts/validation/composition_source_stack_graph_receipts_native.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_color_pipeline_core test_viewer_ui_automation_report test_diagnostics_capture`
- `py -3.14 tools/viewer_host_run_logged_command.py --label composition_source_stack_graph_receipts_runtime_publish --log artifacts/logs/composition_source_stack_graph_receipts_runtime_publish.log --out-json artifacts/validation/composition_source_stack_graph_receipts_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_vsdevcmd.cmd`
- `py -3.14 -m pytest tests/test_fractal_runtime_color_pipeline_presets.py::test_color_pipeline_source_stack_graph_receipt_is_reported_no_mouse -q --junitxml artifacts/pytest/composition_source_stack_graph_receipts_runtime.junit.xml`
- `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/composition_source_stack_graph_receipts_PHASED_PLAN.md --out-json artifacts/validation/composition_source_stack_graph_receipts_hostile_audit.json`
- `py -3.14 tools/viewer_host_run_logged_command.py --label composition_source_stack_graph_receipts_diff_check --log artifacts/logs/composition_source_stack_graph_receipts_diff_check.log --out-json artifacts/validation/composition_source_stack_graph_receipts_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`

## Stop Point

Stop after Slice B emits graph-shaped receipts in runtime report and Capture Finding sidecar, validates, hostile-audits, checkpoints, writes receipts, passes rearward review, pushes, and leaves a clean tree. Preplanned implementation work is exhausted after Slice B; stop for replan before Slice C, visible graph UI, arbitrary graph execution, or broader SDF product growth.
