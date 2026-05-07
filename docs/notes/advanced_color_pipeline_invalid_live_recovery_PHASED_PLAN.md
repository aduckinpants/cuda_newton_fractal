# Advanced Color Pipeline Invalid Live Recovery

## Current Phase

Phase 2 - invalid-live recovery is implemented and validated; checkpoint, receipts, and clean-worktree closure remain

## Phase Checklist

- [x] Phase 0 - prove the current window hides or disables recovery when the live color state is invalid or out of sync, and lock the expected recovery behavior in focused coverage
- [x] Phase 1 - keep the summary controls available and allow a supported draft to repair an invalid live color state
- [ ] Phase 2 - validate the recovery fix on the focused seam, refresh published runtime proof, and close the slice cleanly

## Explicit User Asks

- [open] Stop going in circles around sliders and simple build rules.
- [open] If the checkbox is supposed to exist, keep it present instead of letting it disappear.
- [open] Make the color-pipeline sliders actually affect the runtime on the staged D: build.

## Presumption Loop

The controlling seam is the summary/apply path in `ui_app/src/color_pipeline_window.h`, specifically the interaction between `SyncColorPipelineWindowFromLiveState(...)`, `DescribeColorPipelineDraftApplyState(...)`, and `RenderColorPipelineWindowSummary(...)`. The falsifiable local hypothesis is that when the live color state is invalid or out of sync, the window currently treats that as if no live runtime were available at all. That hides the checkbox and prevents a supported draft from repairing the live state, which makes the sliders look dead. The cheapest discriminating check is a focused regression that starts from an out-of-sync live `coloring_mode`/`color_pipeline` pair and proves the current default supported draft can still classify as applicable and repair the live state through the window.

## Presumption Evidence

- Failure proof: `RenderColorPipelineWindowSummary(...)` only renders the checkbox inside the `live_snapshot.valid` branch.
- Failure proof: `DescribeColorPipelineDraftApplyState(...)` returns `live_unavailable` immediately when `live_snapshot.valid` is false, even if `liveParams` exists and the current draft is a supported tuple that could repair the runtime.
- Structural proof: `SyncColorPipelineWindowFromLiveState(...)` resets `live_snapshot` to empty on invalid live state, which currently cascades into both hidden summary controls and blocked apply behavior.

## Proof Ledger

- GREEN: `ui_app/tests/test_schema_binding.cpp` now proves that an out-of-sync live `coloring_mode` / `color_pipeline` pair leaves the live snapshot invalid, but the default supported draft still classifies as applicable, remains auto-applicable, and repairs the runtime tuple through both direct apply and render-time auto-apply.
- GREEN: `ui_app/src/color_pipeline_window.h` now computes draft apply-state against `liveParams` directly instead of treating an invalid live snapshot as total unavailability, and the summary keeps the auto-apply checkbox visible whenever live runtime params exist while disabling only the reset-from-live path when import is unavailable.
- Validation: the focused native helper rail passed after the new invalid-live recovery regression turned green, `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json` passed with `97/100`, `cmd /c ui_app\build_vsdevcmd.cmd` republished the runtime, staged `fractal_ui.exe --validate-ui` passed, and `py -3.14 tools/viewer_host_runtime_pytest_lane.py` passed with `68 passed`.
- Audit: distrust-first review did not find another local defect in the recovery seam after the helper and summary changes. The repaired behavior is narrowly targeted at the invalid-live state path and does not reopen the earlier timing workaround.

## Notes

- Expected owner files:
  - `docs/notes/advanced_color_pipeline_invalid_live_recovery_PHASED_PLAN.md`
  - `docs/contracts/advanced_color_pipeline_invalid_live_recovery.contract.json`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/tests/test_schema_binding.cpp`
- Non-goals:
  - do not widen the shipped Source / Shape / Palette catalog
  - do not revert the checkbox back to a button
  - do not add another timing-based slider workaround

## Resume Point

Run phased-plan sync and contract validation for `advanced_color_pipeline_invalid_live_recovery`, append the handoff entry with `ck:06fbb088`, checkpoint the slice, write receipts, and finish on a zero-output repo-status check.