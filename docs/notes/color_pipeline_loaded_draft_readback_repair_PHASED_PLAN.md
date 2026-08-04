# Color Pipeline Loaded-Draft Readback Repair

## Current Phase

Complete - ready engine PR and separate merge-approval boundary

## Phase Checklist

- [x] Phase 1 - add a focused RED reproducing the captured `banded_signal -> offset_scale -> banded_heatmap -> balance_void_grade` loaded-draft composition and identify the lane that loses authoritative reconstruction
- [x] Phase 2 - repair the smallest central loaded-draft/readback owner without weakening transactional failure or changing Color Pipeline semantics
- [x] Phase 3 - prove native, published-runtime, action-free replay, and the exact Fixture F state-tool workflow against the repaired runtime
- [x] Phase 4 - complete hostile review, checkpoint, push, and prepare a ready engine PR; stop before merge for separate user approval

## Explicit User Asks

- [complete] Investigate and run down the Fixture F loaded Color Pipeline draft failure rather than continuing the paid V8 panel past a known defect.
- [complete] Keep the repair central and API-surface reusable; do not add a state-tool special case or weaken engine-authoritative readback.
- [complete] Preserve repository rigor and require separate user approval before merging the CUDA engine PR.

## Scope And Contract

The triggering evidence is the state-tool Fixture F candidate for `explaino_balance_void`. Its sparse override changes only `palette.band_emphasis` from `1.5` to `1.8` inside the complete captured draft. Packet V8 validation accepts the change, but the published engine exits before rendering with:

```text
Loaded Color Pipeline draft applied in isolation, but the live runtime cannot provide authoritative draft readback.
```

The exact-base `{}` control materializes and replays the same expensive state, so the defect is bounded to explicit loaded-draft application/readback. Current source is unchanged from the earlier transactional readback repair. This is therefore treated as an uncovered composition class until the focused RED proves the exact failing owner.

Allowed product mutation is limited to the canonical Color Pipeline loaded-draft application and live-snapshot reconstruction seams plus focused/native/runtime tests. The repair must preserve:

- compiled UI-Salt function and parameter authority;
- existing lane topology and runtime math;
- exact binary32 normalization/readback behavior;
- transactional failure for genuinely unreconstructable drafts;
- ordinary direct UI compatibility;
- state schema and serialization compatibility;
- the state-tool's engine-authoritative proof boundary.

Forbidden expansion includes new functions, changed recipes, changed compatibility tables, rendering changes, parameter range/default changes, state schema changes, state-tool code changes, generic Color Pipeline redesign, or treating the captured tuple as a one-off exception.

## Proof Ledger

- Repository authority: clean `master` at `deca3d93fac92ad93742e8d47714f91329808ead`, rearward review `ok`; repair branch `codex/color-pipeline-loaded-draft-readback-repair`.
- Triggering state-tool run: `v9-v8-f-luna-high-live-4d246de2-c6fc-4741-a9f9-d4e58aa93056`.
- Triggering proof receipt: `f4721969-1aba-4912-b8eb-addc4f24cfa7`, SHA-256 `5f95ead343923e82acaaa572c7dea853c57cb19538dfcbd64d9c3dc371ec3329`.
- Exact-base control: `df123ce6-8edf-49e5-968e-d144468981fa`, replay proven with identical decoded pixels and stable semantic state.
- Source history: `color_pipeline_loaded_draft.cpp` and its focused test are unchanged from commit `120a244`; the old focused test did not cover the captured four-lane composition.
- Focused RED: the captured four-lane draft failed while every individual lane reader succeeded; schedule-bridge tests then isolated four rejected combinations: the shipped `neutral_default` and `balance_void_default` grading rows on both band and phase tuples.
- Root cause: `fractal_family_rules.h::TryLegacyColoringModeForPipeline` already authorizes those grading rows, but `color_pipeline_core.h::TryBuildColorPipelineScheduleBridgeIds` duplicated a stale, narrower grading check. Full live-snapshot readback uses the latter bridge.
- Central repair: the schedule bridge now delegates tuple validity to `TryMirroredColoringModeForPipeline` and derives its source/palette bridge from the returned legacy mode. No selector, function, parameter, or Fixture F exception was added.
- First focused GREEN: `test_color_pipeline_core` passed 3334 checks; `test_color_pipeline_loaded_draft` passed 48 checks including exact normalized `1.8f` readback.
- Published-runtime RED before rebuild: the new four-lane regression reproduces the exact authoritative-readback error against the previously published executable.
- Full native proof: required suite passed after 1386.6 seconds; focused core and loaded-draft regressions remained green.
- Published-runtime proof: executable SHA-256 `ee97f0983dc625c1c9aedbfb51dbd944577a47332ccfde6bc7499b84944d8911`; the pre-publish RED passed through explicit draft apply and action-free replay.
- Exact Fixture F rerun: proof `66519d57-8239-47db-aee4-70d757e65b8c` is `replay_proven`; materialization and replay share encoded frame SHA-256 `cf818f478efe8a4d6356c6e05f6b7abc7bb77720e2ca5c0c0e75e51645251003` and decoded RGBA SHA-256 `93133508fa0f55fe5c86cc6f2e133faa52384aa4f23b28e454f7a16487d4dc5d`.
- Final engine commit/runtime/PR identity: runtime is recorded above; the checkpoint wrapper, validation receipts, handoff, and ready PR record the derived commit/PR identities after this plan is committed.

## Hostile Audit

- Status: complete

Audit questions:

- Does the proposed fix preserve fail-closed behavior for genuinely unsupported live snapshots?
- Does it repair a shared reconstruction owner rather than whitelist Fixture F?
- Does the test prove all four captured lanes and exact normalized `1.8f` readback?
- Could a flat pipeline mirror overwrite a valid stack entry and make readback lie?
- Are source, shape, palette, and grading stack counts and rows preserved exactly?
- Does published-runtime proof use explicit loaded-draft application rather than a `{}` replay that bypasses it?
- Does state-tool Fixture F reuse the exact already-paid override without another model call?
- Are engine runtime publication and merge authorization kept distinct?

## Audit Passes

- [complete] Pass 1 - focused RED and source-owner classification.
- [complete] Pass 2 - clean re-read of the repaired shared owner and regression surface found no additional real defect; unsupported tuples still fail closed and clear bridge outputs.
- [complete] Pass 3 - published-runtime and exact-Fixture-F re-read confirmed the repaired state; no additional workflow mistake was found, and visual acceptance remains separate.

## Audit Findings

- [complete] The prior transactional readback test covered grading normalization but not the captured band tuple with a reusable grading row. The new unit and published-runtime regressions cover the exact composition class.
- [complete] Four individual lane readers were healthy. The failure was the central schedule bridge's duplicated grading compatibility check, which had drifted narrower than the canonical family rule.

## Notes

- Fixture G remains paused until the engine PR is separately authorized, merged, and republished from exact merged master; branch-local Fixture F proof is complete.
- The state-tool repository stays clean and unmodified during engine repair.
- The ready engine PR is the final autonomous boundary. Engine merge requires separate user approval.

