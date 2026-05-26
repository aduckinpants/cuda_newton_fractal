# Advanced Color Preset Pit Of Success

> Historical planning surface. The Color Pipeline composition/preset UX review is
> complete; future preset implementation should start from that review's
> remaining follow-up order, with effective Source-stack summary already shipped.
> 
> This is intentionally a design-only planning slice for now. Do not start preset implementation until the Slice 1 UI integration harness is real, trusted, and mandatory workflow proof for user-facing UI behavior changes.

## Current Phase

Phase 0 parked - design placeholder only. Do not implement from this document directly; open a fresh preset workflow slice using the completed Color Pipeline composition/preset UX review as context.

## Phase Checklist

- [ ] Phase 1 - define the preset product invariants, normalization rules, and safety defaults around authored draft preservation versus live-supported projection
- [ ] Phase 2 - define preset v0 as a deterministic default-preset path with minimal or no UI chrome if UI exposure would slow the safety work down
- [ ] Phase 3 - define the future composition model and simple chain rule so later add/blend/compose behavior can land without chaos
- [ ] Phase 4 - reopen this slice only through a fresh checked-in preset workflow slice; the composition/preset review is complete and source-stack summary/report clarity is already shipped

## Explicit User Asks

- [open] Fix the awful current UX where changing supported functions can blow away other slider customizations.
- [open] Favor pit-of-success behavior: keep values when possible, disable unsupported rows instead of deleting them, and use neutral deterministic fill behavior instead of chaos.
- [open] Allow preset v0 to be just a default preset with no required UI if that keeps the implementation simple and testable.
- [open] Set the system up for near-future combined function chains and a small chain-oriented UI without forcing that complexity into the first preset implementation.

## Presumption Loop

The local hypothesis is that presets should preserve authored draft intent separately from the currently-applicable live runtime projection. The cheapest disconfirming path, once Slice 1 exists, is a real app/runtime RED that proves a saved preset can survive support changes without silently deleting authored values while still projecting a deterministic supported live state. If the first preset design still conflates authored draft state with the live bridge state, it will reproduce the same destructive UX under a different label and the slice is not ready.

## Presumption Evidence

- `ui_app/src/color_pipeline_window.h` already contains the authored draft model, the live snapshot model, and the current live-apply bridge, which is the right structural seam for future preset work.
- `ui_app/src/diagnostics_capture.cpp` already persists `color_pipeline_draft`, which means the repo already has a concrete serialization seam that a preset system can reuse.
- `ui_app/src/diagnostics_state_io.cpp` already restores saved drafts and validates them against the saved live tuple, which is useful evidence for future preset load rules.
- The advanced-color model already treats `identity` as a first-class Shape function, which is the natural neutral-fill building block for future support-gap normalization.
- The current function-switch behavior in `ui_app/src/color_pipeline_window.h` resets row parameter values from descriptor defaults, which is exactly why the future preset slice must separate authored state from live-supported projection.

## Proof Ledger

- Done: this checked-in plan captures the original preset safety gate; current refinement now routes through the completed Color Pipeline composition/preset UX review and later implementation slices.
- Done: the existing draft persistence and live-apply seams are named explicitly here so future work starts from real code surfaces instead of chat memory.
- Planned: when preset workflow becomes the active code slice, write the first real app/runtime REDs for preset normalization, authored-value preservation, and deterministic live projection.

## Hostile Audit

- Status: complete
- Required posture: assume the first preset design will accidentally recreate the current destructive UX unless the active composition/preset review and later harness-backed REDs disprove that risk.

## Audit Passes

- [done] Pass 1 - challenge the future preset model for any path that still destroys authored values when support changes.
- [done] Pass 2 - challenge the future normalization rules for any hidden nondeterminism or family-specific chaos.
- [done] Pass 3 - challenge the future composition rule for any path that makes later chain UI harder instead of easier.

## Audit Findings

- [done] Real stale-surface finding repaired: this historical preset note still pointed at the old Slice 1 harness gate instead of the active Color Pipeline composition/preset UX review.
- [done] Clean boundary finding: this historical plan remains design context only and still must not be used to implement a preset manager directly.
- [done] Clean re-read confirmed the repaired state: no additional real issue found in this historical preset planning surface.

## Notes

- Expected owner files when this slice reopens:
  - `docs/notes/advanced_color_preset_pit_of_success_PHASED_PLAN.md`
  - `ui_app/src/color_pipeline_window.h`
  - `ui_app/src/diagnostics_capture.cpp`
  - `ui_app/src/diagnostics_state_io.cpp`
  - future preset storage/UI surfaces to be named once Slice 1 proves the first real REDs
- Non-goals for this parked slice:
  - do not start preset code before the Slice 1 harness gate is satisfied
  - do not force a full preset UI in v0 if a safe default-preset path is enough
  - do not implement future composition-chain math here until the simple rules are refined against real harness-backed tests

## Next Use

Do not implement from this document directly. Use the completed `docs/notes/color_pipeline_composition_preset_ux_review_PHASED_PLAN.md` as context: effective Source-stack summary/report clarity is shipped, while authored intent versus live projection, nondestructive preset workflow, composition UI cleanup, boundary-masked phase sources, and SDF masks/gates remain separate follow-up slices.
