# UI Integration Harness Foundation

## Current Phase

Phase 2 in progress - a checked-in loaded-state scenario driver now powers both advanced-color and sidecar published-runtime proofs; next work should widen that driver to more neighboring workflows and promote it into the mandatory integration rail

## Phase Checklist

- [x] Phase 1 - lock the reusable scenario vocabulary and closure bar: typed action surface, direct native helper coverage, published-runtime proof, and workflow-guard behavior that refuses false recovery/carryover paths.
- [ ] Phase 2 - extract a checked-in scenario driver around runtime publish, load-state/draft hydration, action execution, and deterministic capture so scripted scenarios stop depending on ad hoc CLI seams.
- [ ] Phase 3 - land truthful RED/GREEN scenarios for advanced-color plus at least one neighboring user-facing workflow through the same driver instead of helper-only seams.
- [ ] Phase 4 - wire the scenario driver into a mandatory public validation rail and contract proof chain so future UI slices cannot close without published-runtime proof and hostile-audit evidence.
- [ ] Phase 5 - prove the harness generalizes beyond the current viewer by exercising at least one second host surface or mode family that matters for merge back to mainline.

## Explicit User Asks

- [open] Build a real actual honest integration test harness with no fake greens and no helper-only lies for user-facing UI behavior changes.
- [open] Make the harness robust enough to survive merge into the mainline repo and later exercise the IDE, Science Mode, and other UI surfaces built on the same DX11/ImGui framework.
- [open] Keep the harness deterministic, professional, pure, functional, and generally agent-usable instead of turning it into a one-off advanced-color test hack.
- [open] Make this harness, alongside normal unit coverage, the required deterministic workflow proof for future user-facing UI behavior changes.
- [open] Keep the next harness expansion phase-bounded and explicit in the checked-in plan/contract instead of improvising new one-off seams.
- [done] Hostile review is mandatory closure evidence for this work and must stay machine-enforced instead of being treated as optional prose.

## Immediate Next Slice

- Widen the loaded-state scenario driver to additional neighboring workflows that already use the same publish/load-state/capture pattern instead of leaving them on hand-built command assembly.
- Decide the first extracted helper surface outside the monolithic test file once a second consumer beyond `test_fractal_runtime_explaino_escape_variants.py` is ready.
- Start wiring the driver-backed scenario subset into a named public validation rail so future UI slices can depend on it deterministically.
- Keep hostile-audit validation in the proof chain as the driver expands; do not let the new helper become a loophole for helper-only fake greens.

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
- Done: discovery confirmed the published runtime already supports `--load-state-json` plus `--capture-diagnostic`, while no checked-in headless verb currently scripts an advanced-color draft function change through the same draft/apply helpers the UI uses.
- Done: the first harness-facing RED/GREEN proved an advanced-color draft function switch through the published runtime path, which exposed the need for a reusable multi-action surface.
- Done: CLI startup now preserves loaded `color_pipeline_draft` state for headless runtime proof and headless diagnostic/finding capture now serializes that draft back out.
- Done: hostile audit found and repaired one real defect in the new seam: CLI fractal overrides could invalidate live params while leaving the preserved draft stale; startup now resynchronizes the draft when those overrides change the loaded family path.
- Done: headless runtime proof now accepts repeated `--color-pipeline-action` specs, parses them into typed scenario actions, dispatches them through a visitor-style executor over the draft model, and proves both multi-step function/param mutation and structural Shape-row edits through the published runtime path.
- Done: native helper coverage now exercises the typed action surface directly in `test_viewer_cli` and `test_headless_modes`, so CLI parsing and draft executor behavior both fail fast before the published-runtime rail.
- Done: hostile audit found and repaired a real ownership defect in the typed scenario refactor: pure `--color-pipeline-action` parsing had drifted into `headless_modes.cpp`, which broke the native helper link surface; parsing now lives in `viewer_cli.cpp` while executor tests build typed actions directly.
- Done: hostile audit found and repaired stale `ApplyCliOverrides` helper seams in `test_viewer_state_init` and its native build source list so the native helper profile stays green against the current load-state/color-pipeline signature and JSON dependency set.
- Done: workflow guard hardening now rejects clean-repo crash-recovery adoption and refreshes stale clean baselines only when the current head is already fully receipted, which closes the false-recovery loop without masking receipt debt.
- Done: hostile audit tightened the stale-baseline refresh path again so it validates receipt identity, not just receipt file existence; malformed placeholder receipts can no longer reset the guard.
- Done: the slice contract now requires `viewer_host_validate_hostile_audit.py` and records hostile-audit success as a machine-checked acceptance assertion instead of relying on prose-only discipline.
- Done: the first neighboring workflow now rides the same harness seam: a sidecar mutation-history replay scenario proves a frame-changing user-visible state transition through the same published-runtime load-state/capture pattern as the advanced-color scenarios.
- Done: Phase 2 is now seeded with an explicit `_HeadlessLoadedStateScenario` driver inside the published-runtime test file so advanced-color and sidecar scenarios share one checked-in setup/action/capture flow instead of duplicating CLI assembly by hand.
- Done: the canonical native-helper build rail reran clean on the recovered worktree, so the earlier `ApplyCliOverrides` compile failure was stale log noise rather than a live source mismatch on current disk state.
- Done: the full `tests/test_fractal_runtime_explaino_escape_variants.py` file is green again after fixing the paced-loop stop-threshold proof to compare pumped output against the matching loaded-state baseline instead of the raw startup state.

## Hostile Audit

- Status: done
- Required posture: assume the first proposed harness architecture will still be too viewer-specific, too helper-dependent, or too nondeterministic until a real app-level RED proves otherwise.

## Audit Passes

- [done] Pass 1 - challenged the first headless interaction seam for any path that still bypasses the real draft/apply controller used by the advanced-color UI.
- [done] Pass 2 - challenged the first runtime RED for any path that only proves state-file mutation rather than a real checked-in interaction seam.
- [done] Pass 3 - challenged the first harness helper for any viewer-only assumption that would block reuse in the IDE, Science Mode, or merged mainline host.
- [done] Pass 4 - challenged the new typed scenario refactor against the native helper build surface so parser ownership, stale test seams, and explicit source-list dependencies fail before closure.
- [done] Pass 5 - challenged the workflow guard/recovery path itself so stale baselines and clean-repo misuse cannot produce bogus crash-recovery loops while still preserving receipt enforcement.
- [done] Pass 6 - challenged the stale-baseline refresh path against malformed receipt metadata so placeholder receipt files cannot silently clear carryover debt.
- [done] Pass 7 - challenged the first sidecar replay scenario witness against actual rendered-frame proof instead of accepting a state-only mutation as sufficient harness evidence.
- [done] Pass 8 - challenged the neighboring paced-loop runtime failure against a no-pump control capture so the slice would fix the real baseline defect instead of chasing a nonexistent sidecar mutation bug.

## Audit Findings

- [done] The published runtime already has deterministic load-state plus diagnostics-capture proof rails.
- [done] The missing local seam is a checked-in way to script an advanced-color draft function change through the same draft/apply helpers the UI uses; manual JSON param edits alone are not sufficient harness proof for this slice.
- [done] The CLI startup path currently restores loaded view/params/render plus sidecar state, but it drops the saved `color_pipeline_draft` window state that the interactive load-state path preserves. The first headless advanced-color proof must carry that draft state through startup before it can mutate it honestly.
- [done] After preserving the loaded draft, CLI fractal overrides still left that draft stale even when the live runtime selection had been resynchronized away from a disallowed family-gated tuple. Added a focused published-runtime regression and fixed startup to resynchronize the preserved draft whenever those overrides change the loaded family path.
- [done] Replaced the one-off `--color-pipeline-select-function` seam with a repeated typed `--color-pipeline-action` surface so future headless UI proofs can extend behavior without adding ad hoc per-verb CLI flags.
- [done] The first typed-action implementation accidentally coupled CLI parsing to `headless_modes.cpp`, which broke the lightweight `test_viewer_cli` link surface and would have hidden parser regressions behind a heavier runtime object graph.
- [done] The same hostile audit uncovered stale `test_viewer_state_init` helper seams: the lightweight load-state stub lagged the newer `ColorPipelineWindowState*` import signature and the explicit native helper source list needed `json_min.cpp` once `viewer_state_init.cpp` started resolving color-pipeline defaults through `json_min::Value`.
- [done] The crash-recovery helper would still generate a recovery report for a clean repository, which turned a no-op workflow state into noisy faux recovery and trained the operator to mistrust the guard surface.
- [done] The session-baseline resolver had no safe way to refresh a stale clean baseline after a fully receipted checkpoint, so sticky fallback-session state could keep reporting bogus carryover until someone manually intervened.
- [done] The first stale-baseline refresh fix still trusted receipt file presence alone; malformed receipt placeholders could have refreshed the baseline anyway, so the refresh path now validates receipt head metadata and the active contract identity before accepting the clean head as a new baseline.
- [done] The first sidecar replay scenario choice (`ripple_amplitude`) mutated persisted state but did not move the rendered frame on the published-runtime baseline, so it was not a truthful harness witness; the scenario now replays `explaino_seed`, which does produce a user-visible frame change through the same driver.
- [done] The neighboring paced-loop runtime failure was a false comparison against the raw startup capture: plain `--load-state-json` already changes derived `params.multibrot_power` without any frame change, so the truthful stop-threshold proof must compare pumped output against its matching loaded-state baseline, not the pre-load startup state.

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

Use the now-validated loaded-state scenario driver across more published-runtime workflows, then lift that same truthful interaction pattern into a named mandatory validation rail without relaxing hostile-audit or receipt enforcement.