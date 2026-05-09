# UI Integration Harness Completion

## Current Phase

Phase 5 in progress - Gate G1 is now closed for the harness thread; the explicit stop-line and exclusions are documented, and the next required work is product planning-authority repair before any advanced-color, preset, composition, or new-fractal implementation resumes

## Phase Checklist

- [x] Phase 1 - split planning authority so the harness-completion thread is its own checked-in plan/contract with a hard gate before product work resumes
- [x] Phase 2 - harvest the remaining low-risk shared-harness seams and close or explicitly defer them
- [x] Phase 3 - audit the public runtime rails and residual workflow-hardening items so the harness closure surface is explicit instead of implicit
- [x] Phase 4 - close the harness thread for now with a documented stop-line, explicit exclusions, and a hard gate that blocks product work until this thread is complete
- [ ] Phase 5 - after the harness gate closes, repair product planning authority before any advanced-color, preset, composition, or new-fractal implementation resumes

## Explicit User Asks

- [done] Stop conflating the color-pipeline/new-fractal thread with the testing-harness and coverage-backfill thread.
- [done] Finish the current testing and harness effort as its own phased set of slices before returning to presets, library completion, composition, or new fractal work.
- [done] Make the harness useful, reusable, and in the right shape for future real work rather than a half-done temporary measure.
- [done] It is acceptable to draw a professional stop-line for now as long as the line is explicit and documented.
- [open] Only after the harness path and the already-defined pending product slices are complete should other backlog work be considered.

## Presumption Loop

The controlling defect is planning authority, not a missing runtime rail or a missing advanced-color row. The harness foundation is already materially real, but its checked-in plan still mixes harness-completion work with product-roadmap restart items. The local hypothesis is that a dedicated harness-completion plan and contract, plus a narrow update to the existing foundation plan, will restore one truthful gate: finish the harness and coverage-backfill thread first, then repair product planning authority, then resume product implementation in the documented order. The cheapest disconfirming checks are the deterministic repo rails: the new contract must validate, the phased plans must stay synchronized, and the updated foundation plan must no longer act as the de facto roadmap for product work.

## Presumption Evidence

- `docs/notes/ui_integration_harness_foundation_PHASED_PLAN.md` already proves the harness foundation itself is complete enough to support a narrower completion program rather than another sprawling mixed roadmap.
- `.vscode/tasks.json` already exposes three explicit runtime proof families plus one non-mandatory sibling rail, which means the remaining harness work is about completion and explicit boundaries, not discovery.
- `HANDOFF_LOG.md` already shows that recent slices have been almost entirely harness/backfill work, while the advanced-color plans remain separately active and partially stale.
- `docs/notes/advanced_color_library_foundation_PHASED_PLAN.md`, `docs/notes/advanced_color_library_foundation_phase5_recovery_PHASED_PLAN.md`, and `docs/notes/advanced_color_library_foundation_phase5_shape_stack_runtime_PHASED_PLAN.md` already demonstrate why product restart needs its own planning-authority repair after the harness gate closes.
- `docs/notes/advanced_color_preset_pit_of_success_PHASED_PLAN.md` is already explicitly parked behind the harness gate, which matches the required ordering and should be preserved.

## Proof Ledger

- Landed: the dedicated harness-completion plan/contract pair now exists, so the restart gate lives in checked-in repo authority rather than only in chat.
- Landed: the existing harness-foundation plan now records the foundation as complete and points all remaining harness-first work at this plan instead of mixing in product restart.
- Landed: `tests/test_function_descriptor_cli.py` and `tests/test_generic_probe_cli.py` now reuse `tests/runtime_harness.py` for active-runtime lookup, eliminating the last obvious low-risk copy of the `fractal_ui_active.txt` reader from the published-runtime CLI test family.
- Landed: the residual "remaining hardening" note is now retired; the public runtime closure surface is explicit and already locked by task-surface regression coverage instead of depending on a chat-era to-do.
- Landed: Gate G1 is closed for the harness thread; the repo now has an explicit stop-line for mandatory rails, named opt-in rails, helper/unit-test exclusions, and the rule that product work must not resume until planning authority is repaired first.
- Validated: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/ui_integration_harness_completion.contract.json --out-json artifacts/validation/ui_integration_harness_completion_contract.json` passed for the new planning-only contract.
- Validated: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed for both the completed foundation plan and this new harness-completion plan.
- Validated: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/ui_integration_harness_completion_PHASED_PLAN.md --out-json artifacts/validation/ui_integration_harness_completion_hostile_audit.json` passed after the explicit stop-line and product gate were recorded.
- Validated: `py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_function_descriptor_cli.py tests/test_generic_probe_cli.py` passed with 23 tests after the helper extraction.
- Validated: the exact runtime/checkpoint profile labels and the named non-mandatory FITS sibling rail are already locked in `tests/test_agent_workflow_tools.py`, so no additional public runtime rail surfaced during the Phase 3 audit.

## Hostile Audit

- Status: done
- Required posture: assume the current planning surfaces will drift back into product work unless the harness gate is explicit enough to block that drift in checked-in repo authority.

## Audit Passes

- [done] Pass 1 - challenge whether the existing harness-foundation plan still mixes harness closure with product restart and therefore needs a dedicated successor plan.
- [done] Pass 2 - challenge whether the new harness-completion plan draws a clear enough professional stop-line for now instead of promising exhaustive future coverage.
- [done] Pass 3 - challenge whether the gate between harness completion and product restart is explicit enough that a later slice cannot casually drift back into presets, composition, or new-fractal work.

## Audit Findings

- [done] Real defect found and repaired: the existing harness-foundation plan was still acting as a mixed roadmap for both harness completion and product restart, which risked resuming advanced-color or preset work before the harness-first gate was actually closed.
- [done] Clean re-read result: the new harness-completion plan now draws a professional stop-line for now by separating reusable mandatory rails and low-risk seam harvest work from future exhaustive GUI automation, IDE/Science Mode expansion, and other non-required near-term coverage.
- [done] Clean re-read result: the gate is now explicit in checked-in authority; presets, advanced-color continuation, composition work, and the new fractal thread are all blocked until the harness-completion plan closes Gate G1 and product planning authority is repaired afterward.

## Notes

- Expected owner files for Phase 1:
  - `docs/notes/ui_integration_harness_completion_PHASED_PLAN.md`
  - `docs/contracts/ui_integration_harness_completion.contract.json`
  - `docs/notes/ui_integration_harness_foundation_PHASED_PLAN.md`
  - `docs/contracts/ui_integration_harness_foundation.contract.json`
- Non-goals for Phase 1:
  - do not resume advanced-color implementation
  - do not reopen preset implementation
  - do not start multi-function composition work
  - do not start the new fractal thread
  - do not create new runtime rails unless the authority split itself proves one is needed
- Phase 2 closure note:
  - the remaining obvious active-runtime metadata duplication in the published-runtime CLI family is now gone
  - the next extraction candidates would need a broader support-module decision rather than another low-risk helper move
- Phase 3 closure note:
  - the mandatory published-runtime closure surface is explicit: `verify: runtime probe/session pytest`, `verify: runtime artifact tools`, and `verify: runtime ui harness`
  - the non-mandatory sibling rail is explicit: `verify: runtime walk FITS witnesses`
  - runtime-adjacent helper/unit tests such as `tests/test_viewer_host_runtime_pytest_lane.py`, `tests/test_runtime_walk_extract_fits_orientation.py`, and `tests/test_fractal_probe_client.py` stay outside the published-runtime rail set because they validate helper/tool seams rather than runtime-visible witness families
- Phase 4 stop-line:
  - Gate G1 is closed once the harness thread has: a dedicated checked-in authority surface, explicit mandatory published-runtime rails, explicit non-mandatory sibling rails, and no remaining obvious low-risk shared-helper duplication in the published-runtime CLI family
  - the stop-line for now is intentional: do not reopen this thread for exhaustive GUI automation, IDE/Science Mode expansion, or new helper-only rail proliferation unless a concrete user-facing regression or closure gap appears
  - product work remains blocked until Phase 5 repairs the advanced-color planning authority so the restart order is explicit: product planning repair first, then the already-defined product slices, and only afterward any broader backlog work

## Resume Point

Open the product planning-authority repair slice next: reconcile the active advanced-color foundation and Phase 5 recovery plans with the already-landed checkpoints before any advanced-color, preset, composition, or new-fractal implementation resumes.
