# FPS Debounce Latency Repair

## Current Phase

Phase 4 complete - FPS debounce latency repair is implemented and validated. Closeout requires the checkpoint commit, receipts, push, and clean-tree proof recorded by the repo workflow, with no additional repair work pending in this plan.

## Phase Checklist

- [x] Phase 0 - open a dedicated checked-in plan/contract for FPS debounce latency repair and lock the active slice to it.
- [x] Phase 1 - measure slow-render interaction latency in the published runtime: command round-trip, render ms, preview scale, rendered dimensions, and settle behavior.
- [x] Phase 2 - add RED coverage that fails when severe slow interaction remains hundreds of ms per edit despite preview being active.
- [x] Phase 3 - repair the narrow pacing/default scale seam so severe slow f64 gets materially lower latency while moderate f32 stays full resolution.
- [x] Phase 4 - validate native/runtime rails, publish runtime, hostile-audit, checkpoint, receipts, push, and clean tree.

## Explicit User Asks

- [addressed] User-tested result still does not work: resolution changes are visible in the slider area, but responsiveness does not improve when things are really slow.
- [addressed] Do not claim preview activation or lower render dimensions as enough proof.
- [addressed] Measure actual interaction responsiveness and latency.
- [addressed] Preserve the previous fix where moderate f32/full-res interaction does not downscale unnecessarily.
- [addressed] Preserve slow f64 preview activation, but make it materially useful.
- [addressed] No physical mouse automation and no repeated viewer open/close loop.

## Proof Ledger

- Starting point: `550f4c7` proved default-target fast/f32 stays full resolution and slow/f64 enters preview, but did not prove interaction latency improves enough for the user.
- Suspected failing seam: default `preview_min_scale=0.5` can still leave severe f64 preview frames around hundreds of milliseconds, so the UI remains blocked even though resolution visibly changes.
- Measurement RED artifact: `artifacts/runtime_measure/fps_debounce_latency_red/measurement.json` showed a 2048x1536 standard Multibrot baseline at `1490.143 ms`; preview at scale `0.5` still rendered `1024x768` in about `396-405 ms` and command round-trips stayed about `808 ms`.
- Candidate measurement: `preview_min_scale=0.125` rendered about `293x220` in `62 ms`, but the old code still lacked a guard against stale slow-frame delta expiring new input.
- Required RED: `fps_debounce_latency_native_red` failed with `Expected a newly observed interaction to enter preview even when the previous frame was very slow`, proving preview-active/dimension-only coverage was insufficient.
- Final runtime measurement: `artifacts/runtime_measure/fps_debounce_latency_final/measurement.json` shows full-resolution baseline `1497.046 ms`, preview edit round-trip `101.280 ms`, preview render `62.381 ms`, preview scale `0.149218`, and rendered dimensions `306x229` for target `2048x1536`.
- Final validation: focused native pacing, runtime publish, and all four `tests/test_fractal_runtime_resolution_pacing.py` tests passed after the final code-quality split.
- Non-goals preserved: renderer rewrite, fractal math changes, color pipeline changes, capture finding changes, or OS mouse automation.

## Hostile Audit

- Status: complete
- Did I measure command/input round-trip latency, not just render dimensions? Yes: RED `808 ms`, intermediate `455/506 ms`, final `101 ms`.
- Did the repair materially reduce slow f64 interaction latency? Yes: final published-runtime witness reduced the severe edit round-trip from about `808 ms` to about `101 ms`.
- Did the repair preserve moderate f32 full-resolution behavior? Yes: native pacing and `test_default_target_fast_f32_stays_full_but_slow_f64_enters_preview_no_mouse` passed.
- Did the repair preserve one settle render after interaction? Yes: `test_camera_center_edits_enter_preview_when_measured_frames_are_slow_no_mouse` passed and waits for preview to settle to full quality.
- Did I avoid physical mouse automation and repeated viewer process churn? Yes: proofs use in-process set-value automation; no OS cursor path was added.
- Did I avoid unrelated renderer/fractal/color/capture changes? Yes: changes are limited to pacing state, sidecar-refresh timing during preview, and focused tests/docs.

## Audit Passes

- [complete] Pass 1: reviewed latency measurement as if preview dimensions were a false positive; found default 0.5 preview renders still cost about 400 ms and command latency stayed about 808 ms.
- [complete] Pass 2: tightened runtime threshold; found sidecar refresh still left about 506 ms round-trip after render-scale repair.
- [complete] Pass 3: clean re-read found active-preview recovery/report scale snapped back to 0.5; added native coverage and repaired recovery before final validation.

## Audit Findings

- [x] Published-runtime measurement proved the user report: default severe f64 preview changed dimensions but still left about `808 ms` command latency and about `400 ms` preview renders.
- [x] Native RED proved stale previous-frame delta could expire newly observed interaction before preview had a chance to help.
- [x] Fixed runtime measurement still showed `455 ms` external edit round-trip after render dropped to `62.8 ms`, pointing at non-render per-edit work in the frame loop.
- [x] Tightened runtime RED failed at `506 ms`, proving per-edit side work still made the preview materially unresponsive after render-scale repair.
- [x] Sidecar deferral reduced measured command round-trip to about `102 ms`, but exposed stale report/pacing recovery where active preview scale snapped back to `0.5` without a matching rendered frame.
- [x] Active-preview recovery now keeps the emergency scale while the preview frame is still slow instead of snapping back to the normal floor.
- [x] Final published-runtime measurement: baseline `1497.046 ms`, preview edit round-trip `101.280 ms`, preview render `62.381 ms`, scale `0.149218`, dimensions `306x229`.

## Measurement Requirements

- Severe slow witness: slow f64/high-resolution interaction must produce bounded command round-trip latency, not merely a smaller frame.
- Moderate fast witness: 4096x4096 fast/f32 around the previous 55ms case must remain full resolution.
- Settle witness: one full-quality settle render remains after interaction.
- Harness witness: proof uses one persistent viewer and in-process commands.
