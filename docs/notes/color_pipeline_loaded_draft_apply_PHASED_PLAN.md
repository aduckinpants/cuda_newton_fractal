# Loaded Color Pipeline Draft Application Phased Plan

## Current Phase

Phase 3 - complete validation, checkpoint receipts, rearward review, push, and PR-ready handoff.

## Phase Checklist

- [x] Phase 0 - locked this plan/contract at `ck:5d3685da` and captured focused RED tests for the missing reusable application operation and CLI gate.
- [x] Phase 1 - implemented the central loaded-draft application API and thin CLI adapter; all three focused native rails are green.
- [x] Phase 2 - proved explicit materialization, emitted-state persistence, action-free replay, ordinary-load compatibility, and rendered effect through the published runtime.
- [ ] Phase 3 - complete hostile audit, validation, checkpoint, receipts, rearward review, push, and PR-ready handoff.

## Explicit User Asks

- [open] Repair the engine gap where a deserialized `color_pipeline_draft` remains pending and does not affect rendering unless a later action happens to apply it.
- [open] Keep the repair central, reusable, and suitable for later callable/API exposure rather than embedding a one-off state-tool or capture-path hack.
- [open] Preserve the distinction between loading a draft and deliberately applying it; generic state loading must not silently change semantics.
- [open] Follow the full CUDA engine repository rigor used by the preceding engine campaign.

## Starting Authority

- Repository: `C:\\code\\cuda_newton_fractal_clone`
- Branch: `codex/color-pipeline-loaded-draft-apply`
- Exact starting commit: `b8c19dfa94c7eef136dc4e5eee6a1df67d57daba`
- Starting base: exact clean `origin/master` with upstream counts `0 0`.
- Rearward review: `artifacts/hooks/viewer_host_rearward_review/b8c19dfa94c7eef136dc4e5eee6a1df67d57daba.json` reports `ok`.
- The state-tool repository remains clean and out of mutation scope for this engine slice.

## Named Gap

`LoadDiagnosticsStateFile(...)` correctly restores `color_pipeline_draft` into `ColorPipelineWindowState` while leaving live `KernelParams` unchanged. The only existing promotion path is `ApplyColorPipelineDraftToLiveState(...)`, currently reached by UI interactions and headless action execution. A complete loaded draft therefore survives serialization but can remain visually inert when a caller wants to materialize that exact draft without inventing an action sequence.

The repair is an explicit operation, not a change to ordinary deserialization:

```text
load complete state
-> retain loaded draft as draft
-> caller explicitly requests apply-loaded-draft
-> engine-owned lowering updates live KernelParams
-> emitted complete state replays without the request
```

## Accepted API And CLI Contract

Add a dedicated engine-owned operation named `ApplyLoadedColorPipelineDraftToRuntime` (or the closest name justified by implementation) in a small reusable module outside `main.cpp` and outside the state loader. It accepts the loaded `ColorPipelineWindowState`, live fractal identity, and live `KernelParams`; it returns structured changed/not-changed status plus an exact error string. It delegates all tuple, row, parameter, compatibility, and live-stack lowering to the existing engine authority. It must not duplicate UI-Salt metadata or Color Pipeline mapping logic.

Expose the operation through a thin CLI adapter:

```text
fractal_ui.exe --load-state-json <state.json> --apply-loaded-color-pipeline-draft ...
```

Rules:

- The flag requires `--load-state-json`.
- The flag is rejected with `--color-pipeline-action` and with CLI overrides that invalidate or replace the loaded draft identity.
- Resolution/capture-output arguments may remain valid because they do not alter draft meaning.
- Applying a valid draft may report changed or already-matched; both are successful materializations.
- Invalid, unsupported, or incompatible drafts fail closed with engine-owned validation detail.
- Without the flag, loaded-draft behavior remains byte/semantic compatible: the draft is restored but is not implicitly promoted.
- The normal viewer, capture-finding, and capture-diagnostic routes share the same initialization adapter; no capture-only application path is allowed.
- The engine-emitted complete state after application is the replay authority. Replaying it without the flag must preserve state and decoded pixels.

## Mutation Surface

Allowed:

- one reusable loaded-draft application module;
- CLI parsing, mode validation, and state-initialization adapter wiring;
- focused native/runtime regressions and necessary checked-in build wiring;
- this plan, contract, handoff, and validation artifacts.

Forbidden without plan revision:

- formula, sampler, renderer, palette, grading, or Color Pipeline semantics changes;
- automatic application during ordinary state load;
- state JSON schema or serialization changes;
- UI redesign or new Color Pipeline authoring behavior;
- action synthesis, Python lowering, or state-tool workarounds;
- selector, flag, parameter-surface, or UI-Salt contract changes;
- engine API expansion beyond the bounded reusable operation and thin CLI adapter.

## TDD And Rollback

- Phase 0 captures RED tests before production code changes.
- Native tests first prove the reusable operation, CLI requirements, no-flag preservation, and error propagation.
- Runtime tests then prove a real loaded draft changes effective serialized color state and decoded pixels, and that the emitted state replays action-free.
- Dirty experiments stay on this branch and within the locked contract scope.
- If the existing promotion authority cannot be safely reused without changing Color Pipeline semantics, stop for plan revision.
- Rollback is removal of the new bounded operation/adapter through approved mutation wrappers; never reset or overwrite unrelated work.

## Proof Ledger

- Exact base and rearward review: proven at `b8c19dfa94c7eef136dc4e5eee6a1df67d57daba`.
- Manual gap evidence: a draft-only grading saturation edit survived state emission and replay but rendered decoded pixels identical to base; aligning all existing live representations changed decoded pixels materially.
- Focused native RED: unresolved operation symbol, missing CLI field, and missing initialization field are preserved under `artifacts/validation/color_pipeline_loaded_draft_red_*.json`.
- Focused runtime RED: the existing published runtime left `color_saturation` at `1.0` instead of applying the loaded draft value `0.25`; evidence is `artifacts/validation/color_pipeline_loaded_draft_red_runtime.json`.
- Hostile-review RED: the first API version could combine stale prior diagnostics with the current application failure; `artifacts/validation/color_pipeline_loaded_draft_hostile_stale_validation_red.json` records 19 passing checks and the one expected failing stale-detail check.
- Repaired GREEN: the central operation clears prior validation messages before authoritative revalidation; API `20/20`, viewer CLI `234/234`, and state initialization `55/55` pass.
- Full native GREEN: `artifacts/validation/color_pipeline_loaded_draft_full_native.json` reports all helper tests passed.
- Published runtime GREEN: `artifacts/validation/color_pipeline_loaded_draft_runtime_publish.json` publishes executable SHA-256 `7ac4731398c931bd31458765ec8d81840a82375a9449cc9745a291478b64c350`.
- Runtime materialization/replay GREEN: `artifacts/validation/color_pipeline_loaded_draft_runtime_pytest.json` proves ordinary load remains non-applying and frame-identical, explicit apply changes live saturation and decoded pixels, and emitted state replays action-free with matching params, draft, and frame.
- Final receipts/rearward/push: pending.

## Hostile Audit

- Status: complete

Audit questions:

- Did any ordinary load path begin applying drafts implicitly?
- Is the CLI only an adapter over a reusable engine operation?
- Does the operation reuse the existing authoritative lowering without duplicating metadata or mappings?
- Can incompatible or malformed loaded drafts fail with a vague or silent fallback?
- Does the emitted complete state replay without the flag and with identical decoded pixels?
- Did test fixtures accidentally prove only scalar mirror changes rather than draft-to-live lowering?
- Did branch publication get mistaken for merged-master authority?

## Audit Passes

- [x] Pass 1 - traced success and failure state across repeated calls and found that validation messages from an earlier operation could leak into a later failure result.
- [x] Pass 2 - added a RED regression, repaired the central boundary by clearing prior diagnostics before authoritative validation, and reran focused plus published-runtime proofs.
- [x] Pass 3 - clean re-read confirmed ordinary load remains non-applying, the CLI is a thin explicit adapter, all lowering stays in `ApplyColorPipelineDraftToLiveState`, invalid drafts fail closed with current detail, and emitted state replays without the flag.

## Audit Findings

- [fixed] Stale validation leakage could make a current loaded-draft rejection misleading. The central operation now begins each non-null application attempt with a clean diagnostic set; the regression proves stale text is excluded while current engine validation detail remains available.
- [closed] No implicit application path, duplicate metadata table, capture-only shortcut, action synthesis, state-schema mutation, or merged-master claim was introduced.

## Validation And Stop Boundary

Run every command in `docs/contracts/color_pipeline_loaded_draft_apply.contract.json` plus any stronger current repository-required rail. Viewer-first closure requires the checked-in runtime publish and a published-runtime pytest proving the real flag path.

Stop at a clean pushed PR-ready engine checkpoint. This campaign does not authorize merging the new feature. State-tool integration remains blocked until separate merge authorization, exact merged-master publication, runtime executable identity, and clean engine closure are established.
