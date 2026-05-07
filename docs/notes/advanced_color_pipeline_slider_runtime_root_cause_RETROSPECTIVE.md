# Advanced Color Pipeline Slider Runtime Root Cause Retrospective

## Why This Exists

The advanced color pipeline slider path burned multiple slices because helper-local fixes were treated as product closure while the published `D:` runtime was still broken. This note is the checked-in lesson: what was fixed, what was misdiagnosed, what the real runtime root cause was, and what evidence is now mandatory before declaring any future advanced color pipeline interaction work done.

## User-Visible Failure

- The advanced color pipeline sliders in the published runtime did not behave like the rest of the application.
- Dragging a slider in the advanced Color Pipeline window felt broken in the same way across multiple attempted fixes.
- Several helper and runtime rails stayed green while the actual interactive drag path in the published `D:` viewer was still resetting.

## Wrong Fixes That Burned Time

The following bounded fixes each addressed a real local defect, but they did **not** fix the actual runtime drag-reset path by themselves:

1. apply-affordance stability
   - replaced the disappearing Apply button with a persistent checkbox
   - real issue fixed: layout jump / disappearing control row
   - not sufficient: did not align the runtime drag path with the working sliders

2. auto-apply drag stability / debounced live preview
   - changed when draft apply happened during active drags
   - real issue fixed: reduced one visible timing symptom
   - not sufficient: still trusted a helper-local timing model instead of the real viewer frame lifecycle

3. slider contract parity / active-item smoke
   - aligned the combined slider-plus-input seam with the schema-binding control contract
   - real issue fixed: slider-side activity must be captured before the inline input overwrites the current-item context
   - not sufficient: still did not address per-frame draft rebuilds in the actual window lifecycle

These slices were not worthless. They removed real defects. But they were not the runtime root cause of the repeated user-visible slider failure.

## Real Root Cause

The actual runtime failure lived one frame earlier than the local slider seam.

### Controlling path

- `RenderColorPipelineWindow(...)` in `ui_app/src/color_pipeline_window.h`
- `SyncColorPipelineWindowFromLiveState(...)` in `ui_app/src/color_pipeline_window.h`
- `ResetColorPipelineDraftFromLiveState(...)` in `ui_app/src/color_pipeline_window.h`

### What was happening

1. A supported slider edit in the advanced window applied to live runtime state.
2. After that edit, the programmable draft matched the live snapshot.
3. On the next frame, `RenderColorPipelineWindow(...)` unconditionally called `SyncColorPipelineWindowFromLiveState(...)`.
4. The old sync policy rebuilt the programmable draft whenever `HasColorPipelineDraftEdits(...)` returned false, even if the newly computed live snapshot was **identical** to the existing live snapshot.
5. `ResetColorPipelineDraftFromLiveState(...)` copied the snapshot lanes back into the working draft and reassigned row ids through `EnsureColorPipelineLaneRowsInitialized(...)`.
6. That row-id churn reset the active drag path in the real viewer.

### Why helper-only confidence was misleading

- The focused slider seam tests proved local interaction bookkeeping.
- `--validate-ui` proves startup/schema/CLI safety, not real mouse-drag continuity in the published runtime.
- None of the early bounded regressions locked the frame-to-frame draft identity contract.
- The published runtime was still executing a per-frame draft rebuild path that the earlier tests never modeled.

## Final Repair

The eventual repair was **not** another timing tweak. The fix was to stop rebuilding the programmable draft when live had not actually changed.

### Landed behavior

- `SyncColorPipelineWindowFromLiveState(...)` now compares the previous live snapshot against the newly built snapshot.
- Draft adoption only happens when the live snapshot actually changed, not merely because the draft currently matches live.
- Unchanged live sync preserves row ids across frames.
- The deterministic regression in `ui_app/tests/test_schema_binding.cpp` now fails if two consecutive syncs against identical live params reassign row ids.

## Mandatory Closure Rules For Future Advanced Color Pipeline Interaction Work

These are now non-optional for this feature area.

1. Never close on helper-seam proof alone for viewer-first interaction bugs.
   - Helper coverage is necessary, not sufficient.
   - If the user complaint is about the published runtime interaction path, closure requires a runtime-path explanation and runtime-path proof.

2. `--validate-ui` is not drag proof.
   - Treat it as startup/schema safety only.
   - Do not cite it as evidence that sliders, drags, or per-frame interaction continuity works.

3. For combined-control widgets, test both:
   - the local interaction seam
   - the surrounding frame lifecycle that can invalidate widget identity between frames

4. Any advanced color pipeline change that touches `RenderColorPipelineWindow(...)`, `SyncColorPipelineWindowFromLiveState(...)`, or `ResetColorPipelineDraftFromLiveState(...)` must hostile-review row-id stability.

5. Never widen the shipped catalog or slider surface unless the runtime path is real.
   - runtime-real math
   - runtime-owned parameter storage
   - import/apply round-trip
   - persistence/reset coverage when applicable
   - published-runtime proof

6. When the user reports “still broken in the same way,” assume the previous slice fixed a nearby symptom but not the controlling path.
   - Step outward one frame or one authority seam.
   - Do not keep tuning the same local seam without a new falsifiable runtime hypothesis.

## Concrete Regression Surfaces That Must Stay Green

- `ui_app/tests/test_schema_binding.cpp`
  - slider interaction seam coverage
  - unchanged live-sync row-id stability coverage
- `cmd /c ui_app\build_tests_vsdevcmd.cmd`
- `cmd /c ui_app\build_vsdevcmd.cmd`
- staged `fractal_ui.exe --validate-ui`
- `py -3.14 tools/viewer_host_runtime_pytest_lane.py`

## What This Means For The Next Work

- The next advanced color pipeline slices must widen only runtime-real rows.
- Parameter metadata is already declarative in `FunctionParamDescriptor`; the remaining hard problem is runtime-owned parameter storage and real execution.
- Multi-function-per-lane composition is a separate backend slice. Do not fake it by widening the UI ahead of the runtime executor.

## Bottom Line

The key lesson is simple:

> For viewer-first interaction bugs, the real owner may be the frame-to-frame identity policy around the widget, not the widget itself.

This repo now has a checked-in regression for the real advanced color pipeline drag-reset path. Any future change in this area has to preserve that contract.