# Capture Replay Authority Smoke Matrix

## Current Phase

Phase 7 complete - capture/replay authority repair is implemented and validated.

## Phase Checklist

- [x] Phase 1 - create this checked-in plan/contract and open the slice
- [x] Phase 2 - sync SDF roadmap truth so completed Source customization is no longer active future work
- [x] Phase 3 - add RED native/runtime proof for missing Lens state replay authority and strict CLI state filename friction
- [x] Phase 4 - persist/load Lens settings in diagnostics state and route headless replay through loaded Lens authority
- [x] Phase 5 - split explicit `--load-state-json` loading from finding-picker filename rules
- [x] Phase 6 - add and prove the fast data-driven capture/replay smoke matrix
- [x] Phase 7 - hostile audit, preservation rails, receipts, rearward review, push, and clean-tree closeout

## Explicit User Asks

- [done] Pause forward SDF feature work until capture replay authority is repaired.
- [done] Make `state.json` fully reproduce SDF Color Pipeline captures, including Lens SDF field settings.
- [done] Fix explicit `--load-state-json` so it accepts any valid state JSON filename.
- [done] Add a fast data-driven smoke matrix that catches future capture/replay authority regressions without 4K renders or heavy test time.
- [done] Keep authored SDF pack UI, SDF-native lanes, and broad Color Pipeline redesign paused.
- [open] After replay authority is green, continue with a separate SDF phase-signal metadata slice.

## Scope

In scope:

- Persist and load top-level Lens settings in diagnostics `state.json` with old-state defaults.
- Ensure headless diagnostic/finding replay uses loaded Lens settings for SDF postprocess.
- Allow CLI `--load-state-json` to load arbitrary valid JSON state filenames while preserving strict finding-picker behavior.
- Add a compact capture/replay matrix covering non-SDF, SDF signed distance, SDF normal angle, SDF boundary band, and a two-row SDF stack with non-default downsample.
- Sync active SDF roadmap/status text for the already-shipped Source customization slice.

Out of scope:

- SDF-native selectable fractal lanes.
- Authored SDF pack UI/live viewport integration.
- Phase-safe shape transforms or vector normal sources.
- Renderer replacement or CUDA kernel redesign.
- Physical mouse automation.

## Proof Ledger

- Start authority: branch `codex/color-pipeline-sdf-source-rows` at `da8a04c` is clean and rearward review returned `status=ok`.
- Pre-slice investigation proved `D:\salt-fractal\cuda_newton_fractal_clone\findings\manual_capture\2026-05-25\130840_767__multibrot\frame.png` matched its original diagnostic BMP exactly, while current replay of the same saved state produced a different but deterministic frame because Lens SDF authority was not serialized in state.
- RED targets: diagnostics state IO Lens round-trip/defaults, explicit arbitrary filename state-load acceptance, finding-picker filename strictness preservation, and fast capture/replay hash parity.
- RED result: `capture_replay_authority_red_native` failed on missing Lens-aware capture/load APIs before implementation.
- Native proof: `capture_replay_authority_test_diagnostics_state_io`, `capture_replay_authority_test_finding_state_actions`, `capture_replay_authority_test_viewer_state_init`, and `capture_replay_authority_test_lens_sdf` passed.
- Runtime proof: `capture_replay_authority_runtime_publish` published `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Published runtime proof: `capture_replay_authority_runtime_pytest` passed `6 passed`, covering the new replay matrix plus SDF Source rows and viewport overlay preservation.
- Harness proof: `ui_app/build_tests_vsdevcmd.cmd` now has focused `test_finding_state_actions` and `test_viewer_state_init` targets so the repaired seams are not hidden behind the full native sweep only.
- Closeout validators: contract validation, phased-plan sync, hostile-audit validation, code-quality baseline, and diff hygiene all passed.

## Hostile Audit

- Status: complete
- Required posture: assume `state.json` still omits hidden replay authority, SDF replay still uses a default Lens setting, arbitrary filename support weakens finding-picker safety, the smoke matrix is too narrow or too slow, or future SDF source additions can bypass replay proof until focused tests disprove each risk.

## Audit Passes

- [done] Pass 1 - re-read diagnostics capture/state load seams; hidden Lens replay authority was the missing state surface for SDF postprocess replay.
- [done] Pass 2 - re-read CLI state-load versus finding-picker split; explicit state JSON loading accepts arbitrary names while the finding picker still rejects arbitrary filenames.
- [done] Pass 3 - re-run and review the smoke matrix; it stays small at 192x144, data-driven by row descriptors, and covers non-SDF plus signed distance, normal angle, boundary band, and a two-row SDF stack with non-default downsample.
- [done] Pass 4 - clean re-read of the repaired state found no additional real defect in diagnostics capture/state IO, explicit state loading, finding-picker strictness, or replay-matrix coverage.

## Audit Findings

- [done] Real harness defect found: `test_finding_state_actions` existed in the full native suite but had no focused build target, so the exact new arbitrary-filename unit rail could not be run directly. Added the focused target.
- [done] Real follow-on harness defect found: moving CLI explicit state loading to direct diagnostics state IO would have broken `test_viewer_state_init` because the lightweight test still stubbed only the old finding-selection loader. Added the direct state-load stub and focused target.
- [done] Real runtime matrix defect found: the initial `sdf_normal_angle` row used an invalid flat tuple with the cyclic palette; corrected it to `phase_wheel`/`phase_default` while keeping the normal-angle source-stack proof.
- [done] Real code-quality defect found: the first Lens parser implementation exceeded the `diagnostics_state_io.cpp` function-length baseline. Split the parser into small helpers and reran diagnostics state IO plus code-quality baseline successfully.

## Action Hostile Review

- Action ID: capture-replay-authority-smoke-matrix-closeout
- Suspected failure mode: SDF captures can look correct at capture time while saved `state.json` cannot reproduce them because Lens SDF settings remain hidden runtime authority.
- Correct owner/action: diagnostics state IO, explicit state-load routing, headless SDF postprocess capture/replay, and fast runtime smoke matrix must move together.
- Proof surface: focused native diagnostics/finding-state tests, runtime publish, published-runtime capture/replay smoke matrix, Color Pipeline SDF rows preservation, viewport overlay preservation, Lens SDF preservation, contract validation, phased-plan sync, hostile-audit validation, code-quality baseline, and diff hygiene.
- Blocked action: authored SDF UI, SDF-native lanes, phase-safe shape redesign, vector normal sources, renderer replacement, or physical mouse automation.
