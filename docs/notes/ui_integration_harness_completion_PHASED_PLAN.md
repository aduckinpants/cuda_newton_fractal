# UI Integration Harness Completion

## Current Phase

Phase 3 in progress - the low-risk active-runtime lookup seam harvest is complete; next audit the public runtime rails and residual workflow-hardening items so the harness closure surface is explicit before product work can resume

## Phase Checklist

- [x] Phase 1 - split planning authority so the harness-completion thread is its own checked-in plan/contract with a hard gate before product work resumes
- [x] Phase 2 - harvest the remaining low-risk shared-harness seams and close or explicitly defer them
- [ ] Phase 3 - audit the public runtime rails and residual workflow-hardening items so the harness closure surface is explicit instead of implicit
- [ ] Phase 4 - close the harness thread for now with a documented stop-line, explicit exclusions, and a hard gate that blocks product work until this thread is complete
- [ ] Phase 5 - after the harness gate closes, repair product planning authority before any advanced-color, preset, composition, or new-fractal implementation resumes

## Explicit User Asks

- [open] Stop conflating the color-pipeline/new-fractal thread with the testing-harness and coverage-backfill thread.
- [open] Finish the current testing and harness effort as its own phased set of slices before returning to presets, library completion, composition, or new fractal work.
- [open] Make the harness useful, reusable, and in the right shape for future real work rather than a half-done temporary measure.
- [open] It is acceptable to draw a professional stop-line for now as long as the line is explicit and documented.
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
- Validated: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/ui_integration_harness_completion.contract.json --out-json artifacts/validation/ui_integration_harness_completion_contract.json` passed for the new planning-only contract.
- Validated: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed for both the completed foundation plan and this new harness-completion plan.
- Validated: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/ui_integration_harness_completion_PHASED_PLAN.md --out-json artifacts/validation/ui_integration_harness_completion_hostile_audit.json` passed after the explicit stop-line and product gate were recorded.
- Validated: `py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_function_descriptor_cli.py tests/test_generic_probe_cli.py` passed with 23 tests after the helper extraction.

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

## Resume Point

Audit the public runtime rails and residual workflow-hardening items for any still-implicit closure requirement, then either make the rail explicit or document the deferral before moving to the final harness stop-line phase.
