# Reset All Color Pipeline Authority Bug Report

## Title

Reset All can leave stale Color Pipeline UI/receipt state while classic ExplainO Joy basin coloring is the effective render path.

## Severity

P1 - state authority, capture provenance, and reset coherence.

The rendered pixels may be correct. The defect is that UI/report/capture surfaces can tell a false story about which pipeline produced those pixels.

## Affected Subsystems

- Reset runtime state.
- Color Pipeline draft/editor state.
- Runtime-effective `KernelParams` Color Pipeline stacks.
- Color Pipeline graph receipt.
- Capture Finding `fractal-state.json`.
- Diagnostics state save/load.
- Runtime automation report and future agent analysis.

## User-Visible Symptom

After configuring an advanced Color Pipeline and invoking `Reset All`, the viewer can return to classic ExplainO/Joy basin coloring while the advanced Color Pipeline editor remains populated. Capture/report surfaces can then imply that retained SDF/source/shape/palette/grading rows actively executed even though the effective render path is classic:

```text
root_index -> joy -> basin_default
```

## Current Source-Grounded Findings

### Runtime reset clears effective stacks

`ui_app/src/runtime_reset.cpp` clears the runtime-effective stacks:

- `params.color_source_stack_count = 0`;
- `params.color_root_basin_pair_count = 0`;
- `params.color_palette_stack_count = 0`;
- `params.color_grading_stack_count = 0`;

`ui_app/src/main.cpp` calls `ResetRuntimeStateForCurrentFractal(...)` when `resetAllAction` is true.

This supports the hypothesis that the effective runtime pipeline can be reset independently of the Color Pipeline editor draft.

### Effective color source serialization distinguishes flat signal from source stack

`ui_app/src/diagnostics_capture.cpp` writes `color_effective_source.authority` as:

- `source_stack` when `CaptureColorSourceStackCount(params) > 0`;
- `flat_signal` otherwise.

It also writes `legacy_flat_signal` from `params.color_pipeline.signal`.

This means a post-reset classic Joy state can correctly serialize effective source authority as flat `root_index` with zero source stack.

### Draft/editor state is serialized separately

`ui_app/src/diagnostics_capture.cpp` writes `color_pipeline_draft` from `ColorPipelineWindowState`, not directly from the effective runtime stacks.

`ui_app/src/diagnostics_state_io.cpp` parses and restores `color_pipeline_draft` as a separate object and rebuilds a live snapshot from the loaded runtime state.

This is not inherently wrong; preserving authored draft could be a valid product policy. The bug is that the reporting contract does not reliably distinguish retained configured rows from effective execution.

### Graph receipt currently maps active execution to row enabled state

`ui_app/src/color_pipeline_graph_receipt.h` writes:

```text
"active_execution": row.enabled
```

and `fail_closed_reason` is `null` for enabled rows.

That is insufficient for this bug class. A row can be enabled/configured in the editor draft while not contributing to the effective rendered frame.

### Root-basin color path is a separate schedule

`ui_app/src/color_pipeline_window.h` repeatedly states that `root_index` remains on the separate root-basin schedule, paired with `root_classic_palette` or `joy_root_palette`. Generic Source rows explicitly reject `root_index`.

This supports the product interpretation that classic ExplainO/Joy basin coloring is not the same execution path as a generic Source/Shape/Palette/Grading stack.

## Expected Behavior

Regardless of chosen reset policy:

- configured/draft state is distinguishable from effective execution;
- `active_execution=true` is never emitted for a node that did not contribute to the rendered frame;
- graph receipt, top-level color state, runtime stack counts, and frame capture agree;
- bypassed rows carry explicit reason such as `classic_explaino_basin_path`;
- Capture Finding cannot mislead a human or agent into attributing a classic Joy frame to inactive SDF/heatmap rows.

## Actual Behavior

Based on the supplied capture and current source read:

- runtime-effective Color Pipeline stacks can be empty after Reset All;
- effective top-level color can be classic `root_index` / `joy` / `basin_default`;
- the editor draft can remain populated;
- graph receipt can report retained enabled rows as `active_execution=true` because it currently treats row enabled state as active execution;
- Capture Finding can therefore contain contradictory provenance.

## Deterministic Reproduction Recommendation

This pass did not run a live reproduction. Recommended future RED:

1. Launch clean viewer.
2. Select a classic ExplainO/Joy basin lane.
3. Configure an unmistakable advanced Color Pipeline:
   - SDF Source row;
   - non-default palette;
   - non-default grading row.
4. Confirm whether the advanced pipeline is initially effective.
5. Invoke `Reset All`.
6. Wait for a fresh frame generation.
7. Capture:
   - `color_effective_source.authority`;
   - top-level `color_signal`, `color_palette`, `color_grading`;
   - runtime stack counts;
   - `color_pipeline_draft`;
   - `color_pipeline.graph_receipt`;
   - `root_pattern_consumers`;
   - frame hash.
8. Save/reload and repeat the authority check.

The future test should assert that retained draft rows, if preserved, are reported as configured but not effective.

## Ownership Map

| Surface | Owner | Current concern |
| --- | --- | --- |
| Effective runtime source stack | `KernelParams.color_source_stack_count` and entries | Reset clears this. |
| Effective runtime palette/grading stacks | `KernelParams.color_palette_stack_count`, `color_grading_stack_count` | Reset clears these. |
| Classic basin path | `color_pipeline.signal=root_index`, palette `joy`, grading `basin_default`, root-basin schedule | Separate from generic Source stack. |
| Draft/editor rows | `ColorPipelineWindowState.lanes` serialized as `color_pipeline_draft` | Can remain populated after reset. |
| Graph receipt | `color_pipeline_graph_receipt::WriteColorPipelineGraphReceiptJson` | Currently maps `active_execution` to `row.enabled`. |
| Finding sidecar | `diagnostics_capture.cpp` | Can include both effective source summary and stale-looking graph receipt. |
| State load | `diagnostics_state_io.cpp` | Restores draft separately and rebuilds live snapshot. Reload implications need future live proof. |

## Suspected Root Cause

Hypothesis:

```text
Reset All clears runtime-effective Color Pipeline stacks and returns to classic basin coloring,
but ColorPipelineWindowState draft rows remain populated.
The graph receipt then derives active execution from draft row enabled state instead of effective renderer contribution.
```

This is partly source-confirmed:

- reset clearing runtime stacks is confirmed;
- graph receipt `active_execution = row.enabled` is confirmed;
- draft persistence as a separate object is confirmed.

Still unproven in this pass:

- exact live UI sequence that leaves the mismatch;
- reload behavior after the contradictory capture;
- whether the public branch and current local branch differ materially after the PR merge.

## Policy Decision Required

Do not assume Reset All must delete the draft. Pick one policy explicitly in a future repair:

### Option A - Reset configuration completely

Clear both effective runtime stacks and advanced editor draft.

Pros: simplest mental model.

Cons: can destroy an authored draft the user may expect to keep.

### Option B - Preserve authored draft, mark inactive

Keep draft rows, but report:

```text
configured=true
requested=true
effective=false
bypass_reason=classic_explaino_basin_path
```

Pros: preserves work and resolves provenance.

Cons: requires richer UI/report language.

### Option C - Rematerialize draft

Reset retains and reapplies advanced pipeline; classic Joy does not become effective unless explicitly selected.

Pros: draft remains active.

Cons: may conflict with expected Reset All semantics and classic ExplainO defaults.

Recommended direction for future planning: Option B unless product history says Reset All should destroy drafts.

## Impact

- UI truth: user may see enabled rows that are not producing pixels.
- State persistence: saved state can carry both retained draft and effective classic path without clear authority labels.
- Finding captures: reviewers can misattribute the frame to SDF/heatmap/grading rows.
- Graph receipts: `active_execution` is semantically wrong for bypassed rows.
- Automation: no-mouse reports may validate visible/configured rows instead of effective execution.
- Agent analysis: future agents can draw false conclusions from receipt nodes.
- Replay/provenance: reload may reactivate, retain, or differently reconcile stale draft unless tested.

## Existing Tests And Gaps

Existing relevant tests:

- `ui_app/tests/test_diagnostics_capture.cpp` checks finding sidecar and graph receipt presence.
- `ui_app/tests/test_diagnostics_state_io.cpp` checks draft parsing and root-basin state cases.
- `ui_app/tests/test_color_pipeline_window.cpp` checks draft/live snapshot and apply behavior.
- Runtime Color Pipeline tests assert graph receipt presence and frame hashes for configured scenarios.

Gap:

No current test appears to assert post-Reset All effective-vs-configured Color Pipeline truth for classic ExplainO/Joy basin coloring.

## Recommended Future RED Tests

Native:

- `ResetRuntimeStateForCurrentFractal` plus a populated `ColorPipelineWindowState` should produce a receipt where bypassed draft rows are not `active_execution=true`.
- Graph receipt builder should accept effective execution context, not only draft lanes.
- `active_execution` should require actual runtime contribution, not row enabled state.

Runtime/no-mouse:

- Configure advanced Color Pipeline.
- Invoke Reset All through the same action path as the UI button.
- Capture report and finding sidecar.
- Assert:
  - effective source authority is flat/root-basin;
  - runtime stacks are empty or classic as intended;
  - draft rows, if preserved, are marked inactive/bypassed;
  - frame hash is stable through replay;
  - reload does not silently change the effective path.

## Containment Options For Future Repair

- Add an explicit configured/requested/effective tri-state to graph receipt nodes.
- Add bypass reasons such as `classic_explaino_basin_path`.
- Derive graph receipt active execution from materialized runtime stacks plus renderer path, not draft enabled state.
- Reconcile/reset `ColorPipelineWindowState` on Reset All if product policy chooses Option A.
- Add a presentation trace diagnostic so effective render authority can be inspected directly.

## Non-Goals

No repair is authorized in this pass.

Do not:

- alter reset behavior;
- change graph receipt emission;
- change Color Pipeline UI;
- add runtime tests;
- classify rendered pixels as wrong without proof.

## Final Classification

P1 state authority / capture provenance / reset coherence defect. Treat as a reporting and ownership bug first. Renderer output may remain correct.
