# UI Integration Harness Foundation

## Current Phase

Phase 1 in progress - define the merge-grade harness contract, scenario model, and first truthful RED targets before any one-off viewer test code lands

## Phase Checklist

- [ ] Phase 1 - define the reusable app-level harness contract, scenario vocabulary, and workflow proof bar for the viewer host, future IDE surfaces, Science Mode, and other merged DX11/ImGui clients
- [ ] Phase 2 - formalize the driver/session seams around runtime publish, diagnostics capture/load-state, and real app frame execution so scripted scenarios can drive the actual host deterministically
- [ ] Phase 3 - land the first truthful RED/GREEN scenarios for the advanced-color UX regression through the real app/runtime path instead of helper-only seams
- [ ] Phase 4 - wire the harness into a mandatory public validation rail and document the agent-facing usage pattern so future UI slices cannot close on fake greens
- [ ] Phase 5 - prove the harness generalizes beyond the current viewer by exercising at least one second host surface or mode family that will matter during merge back to mainline

## Explicit User Asks

- [open] Build a real actual honest integration test harness with no fake greens and no helper-only lies for user-facing UI behavior changes.
- [open] Make the harness robust enough to survive merge into the mainline repo and later exercise the IDE, Science Mode, and other UI surfaces built on the same DX11/ImGui framework.
- [open] Keep the harness deterministic, professional, pure, functional, and generally agent-usable instead of turning it into a one-off advanced-color test hack.
- [open] Make this harness, alongside normal unit coverage, the required deterministic workflow proof for future user-facing UI behavior changes.

## Presumption Loop

The local hypothesis is that this repo already contains enough real app/runtime seams to build a durable interaction harness without resorting to screenshot-only smoke tests or helper-level self-deception. The cheapest disconfirming path is to anchor the harness around existing runtime publish, diagnostics capture/load-state, and frame-driving seams, then write a first RED for the advanced-color UX failure through the actual host path. If that RED still depends on helper-only state mutation or cannot capture deterministic state plus frame evidence, the harness architecture is too shallow and the slice is not ready to proceed.

## Presumption Evidence

- `ui_app/src/main.cpp` already owns the real app frame loop, viewport render path, and runtime dispatch seam that a truthful harness must eventually drive.
- `ui_app/src/headless_modes.h` and the published runtime path already provide deterministic process-entry surfaces that can support scripted sessions instead of ad hoc GUI clicking.
- `ui_app/src/diagnostics_capture.cpp` and `ui_app/src/diagnostics_state_io.cpp` already round-trip rich runtime state, which is the right base for deterministic setup and post-action assertions.
- `ui_app/src/finding_state_actions.h` already exposes repo-owned restore seams that can hydrate runtime state from durable artifacts without bespoke test-only plumbing.
- `tools/viewer_host_runtime_pytest_lane.py` already proves the repo can enforce published-runtime proof on a checked-in validation rail; Slice 1 should extend that discipline instead of inventing a parallel ritual.
- The current advanced-color helper tests prove model behavior in `ui_app/tests/test_schema_binding.cpp`, but they do not yet prove real app interaction behavior; that gap is the right first RED target for this slice.

## Proof Ledger

- Done: this checked-in Slice 1 phased plan now captures the merge-to-mainline requirement explicitly so the harness is designed as shared infrastructure instead of a viewer-only patch.
- Done: the bootstrap contract for this slice now reserves the planning surface needed to start the harness on approved repo rails instead of leaving it as chat-only intent.
- Done: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/ui_integration_harness_foundation.contract.json --out-json artifacts/validation/ui_integration_harness_foundation_contract.json` and `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` both passed for this planning bootstrap slice.
- Planned: add the first harness-facing RED that proves the advanced-color support-change UX defect through the real app/runtime path.

## Hostile Audit

- Status: done
- Required posture: assume the first proposed harness architecture will still be too viewer-specific, too helper-dependent, or too nondeterministic until a real app-level RED proves otherwise.

## Audit Passes

- [done] Pass 1 - challenged the initial harness shape for any hidden dependence on helper-only state mutation, screenshot-only proof, or per-feature special cases.
- [done] Pass 2 - challenged the workflow plan for any path that would still allow user-facing UI slices to close without the harness receipt.
- [done] Pass 3 - challenged the reuse story for any assumption that only works in this repo but would break once merged into the IDE/mainline host.

## Audit Findings

- [done] No additional planning-scope defect was found after re-reading the harness bootstrap docs for viewer-only drift, fake-proof loopholes, or premature preset-scope expansion.

## Notes

- Expected owner files for this slice:
  - `docs/contracts/ui_integration_harness_foundation.contract.json`
  - `docs/notes/ui_integration_harness_foundation_PHASED_PLAN.md`
  - `docs/notes/advanced_color_preset_pit_of_success_PHASED_PLAN.md`
  - `ui_app/src/main.cpp`
  - `ui_app/src/headless_modes.h`
  - `ui_app/src/headless_modes.cpp`
  - `ui_app/src/diagnostics_capture.cpp`
  - `ui_app/src/diagnostics_state_io.cpp`
  - `ui_app/src/finding_state_actions.h`
  - `tools/viewer_host_runtime_pytest_lane.py`
  - `.vscode/tasks.json`
  - future repo-local harness helpers/tests under checked-in `tools/` and `tests/` surfaces
- Non-goals for this slice:
  - do not build a viewer-only throwaway test harness that cannot survive merge back to mainline
  - do not accept helper-only or screenshot-only UI proof as the long-term answer
  - do not start preset implementation here; Slice 2 stays gated until this harness is real and mandatory

## Resume Point

Start with the smallest real-host RED for the advanced-color support-change UX failure, using the existing runtime publish plus diagnostics state/capture seams to prove state and frame outcomes through the actual app/runtime path.