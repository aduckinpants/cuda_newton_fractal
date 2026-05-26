# Animation Pacing Integration

## Current Phase

Phase 4 - hostile audit, validation receipts, contract proof receipt, rearward review, push, and clean-tree closeout.

## Phase Checklist

- [x] Phase 0 - start from the clean pushed adaptive pacing head `92d5210` on `codex/sdf-adaptive-preview-pacing`.
- [x] Phase 1 - open this runtime/viewer-first plan and contract, lock the active slice, and add RED proof for animation-driven pacing.
- [x] Phase 2 - route generic parameter animation and Explaino seed auto-increment changes through the same interaction/pacing notification seam as manual controls.
- [x] Phase 3 - publish runtime and prove persistent no-mouse animation enters preview under measured slow/SDF load while still settling when animation is disabled.
- [ ] Phase 4 - hostile audit, validation receipts, contract proof receipt, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [done] Make the simple animation system play nicely with the pacing/recovery system.
- [done] Treat animation bypassing the recovery system as an API/integration surface problem, not as a broad animation overhaul.
- [done] Keep the existing simple animation behavior and controls; this slice should only fix pacing integration.
- [done] Preserve manual control pacing behavior, SDF source behavior, capture/full-quality settle, and the adaptive preview work already landed on this branch.

## Scope

In scope:
- Main viewer integration around `ApplyParamAnimDynamics(...)` and `ApplyExplainoSeedDynamics(...)`.
- Focused native or runtime tests proving animation-driven mutation is treated as interaction for pacing.
- Persistent no-mouse runtime proof; no physical mouse automation.
- Plan, contract, handoff, receipts, and validation artifacts.

Out of scope:
- Animation UI redesign or overhaul.
- New animation targets.
- Per-source/per-row SDF downsample.
- GPU Color Pipeline postprocess.
- SDF source semantics, gates, phase behavior, capture/replay, or Color Pipeline UI changes.
- SDF-native lanes, authored SDF pack UI, or Salticid runtime dependency changes.

## Implementation Direction

The narrow fix should keep `ApplyParamAnimDynamics(...)` and `ApplyExplainoSeedDynamics(...)` as the mutation authority, but the caller must treat a returned `true` as a viewer interaction for pacing purposes. Manual schema/control edits, viewport drag, Color Pipeline edits, sidecar mutations, and runtime-walk UI already reach `NoteViewerInteraction(...)`; animation changes should join that same seam instead of inventing a second pacing API.

Runtime proof should use one persistent viewer process, enable an animatable parameter under a slow SDF Color Pipeline workload, and assert that the animation-driven frame enters preview without an explicit control mutation. It should then disable animation and prove the next settle path returns to full quality.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on `codex/sdf-adaptive-preview-pacing` at `92d5210`.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported a clean tree.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `92d5210`.
- Initial code read: `RunViewerFrame(...)` sets `dirty` when `ApplyExplainoSeedDynamics(...)` or `ApplyParamAnimDynamics(...)` returns true, but it does not set `actions.interactionChanged` or call `NoteViewerInteraction(...)` for those animation mutations.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "Animation pacing integration repair" --profile runtime --plan docs/notes/animation_pacing_integration_PHASED_PLAN.md --contract docs/contracts/animation_pacing_integration.contract.json` appended `ck:692e2b28` and locked the active contract.
- Native pacing proof: `animation_pacing_native` passed `test_viewer_render_pacing`.
- Runtime publish proof: `animation_pacing_runtime_publish` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Published SDF runtime proof: `animation_pacing_runtime_sdf` passed; the persistent no-mouse viewer enables `fractal.view.param_anim_target = ripple_amplitude`, waits beyond the debounce window, proves animation itself keeps preview active under SDF load, then disables animation and proves full-quality settle.
- Published pacing preservation proof: `animation_pacing_runtime_resolution` passed with 4 tests.

## Hostile Audit

- Status: complete
- Required posture: assume animation updates are still bypassing pacing, preview never settles after animation is disabled, manual controls regress, or runtime proof only exercises command-driven control edits instead of autonomous animation.
- Did animation-driven mutation enter the same pacing path as manual edits? Yes; `RunViewerFrame(...)` now treats returned animation mutations as `actions.interactionChanged`, which reaches the existing `NoteViewerInteraction(...)` seam.
- Did the runtime proof exercise animation itself rather than only a set-value command? Yes; after enabling `ripple_amplitude` animation, the test waits beyond the debounce window with the same automation command sequence and requires preview to remain active from autonomous animation frames.
- Did disabling animation settle back to full quality? Yes; the runtime test then sets the animation target to `none` and waits for preview inactive, scale `1.0`, target dimensions, and SDF postprocess step `1`.
- Did this become an animation overhaul? No; the animation target resolver, schema options, and parameter mutation logic were not redesigned.

## Audit Passes

- [done] Pass 1 - identified the exact animation/pacing integration gap before implementation: animation changes set `dirty` but did not mark the viewer as interacted for pacing.
- [done] Pass 2 - re-read the implementation diff for false interaction spam, missed settle path, and manual-control pacing regressions.
- [done] Pass 3 - re-read runtime proof to ensure animation itself, not a manual set-value command, triggers preview.
- [done] Pass 4 - clean re-read found no animation UI redesign, new target exposure, SDF source behavior change, GPU postprocess work, or per-source downsample drift.

## Audit Findings

- [done] Finding 1: generic parameter animation and Explaino seed auto-increment returned `true` from their mutation helpers, but the caller only set `dirty`; it did not set `actions.interactionChanged`, so pacing could miss animation-driven work. Repaired by routing animation mutations into the existing interaction notification seam.
- [done] Finding 2: a runtime proof that only set the animation target would be insufficient because the set command itself can dirty the viewer. Repaired by waiting beyond the debounce window with the same command sequence and requiring preview to remain active from autonomous animation frames.
- [done] Clean re-read confirmed focused native proof, published animation/SDF runtime proof, and published resolution pacing preservation proof pass on the repaired state.
