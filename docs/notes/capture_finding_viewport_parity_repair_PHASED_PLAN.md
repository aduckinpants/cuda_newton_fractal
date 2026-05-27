# Capture Finding Viewport Parity And SDF Pacing Repair

## Current Phase

Closed - Capture Finding SDF replay authority and SDF-heavy pacing stability repaired and validated.

## Phase Checklist

- [x] Phase 0 - bootstrap clean `codex/sdf-adaptive-preview-pacing` head `a3bf6c9`, confirm rearward review is ok, and inspect the reported Julia finding.
- [x] Phase 1 - open this plan/contract, lock the active slice, and record RED proof that high-res Capture Finding recomputes SDF color sources at the capture resolution instead of the live/source viewport resolution.
- [x] Phase 2 - add RED proof for SDF-heavy preview flicker where a very slow full-quality frame is rescheduled after a fixed short debounce and causes visible high/low thrash.
- [x] Phase 3 - add replay authority for SDF field source resolution and route in-loop Capture Finding through it without downgrading high-res output or standard/f64 capture quality.
- [x] Phase 4 - add measured pacing stability so very slow full-quality SDF work holds preview long enough to avoid rapid full/preview flicker while still allowing settle.
- [x] Phase 5 - add native state IO/pacing coverage and no-mouse runtime regressions for the Julia Capture Finding and SDF-heavy pacing cases.
- [x] Phase 6 - hostile audit, validation receipts, contract proof receipt, rearward review, push, and clean-tree closeout.

## Explicit User Asks

- [closed] Fix the Capture Finding regression reported for `D:\salt-fractal\cuda_newton_fractal_clone\findings\manual_capture\2026-05-26\185950_922__julia`.
- [closed] Compare against the actual viewport color/control state, not a proxy.
- [closed] Close the missing unit/runtime smoke gap for this important path.
- [closed] Fix the SDF-heavy preview pacing flicker reported for `D:\salt-fractal\cuda_newton_fractal_clone\findings\manual_capture\2026-05-26\191229_004__explaino_all`.
- [closed] Preserve high-resolution Capture Finding behavior and the existing Color Pipeline/SDF feature semantics.

## Scope

In scope:
- Capture Finding rendering and replay authority for SDF Color Pipeline source rows.
- Persisting and loading any missing state needed for SDF source fields to reproduce the archived frame.
- SDF-heavy preview pacing stability when full-quality frames are far slower than the configured debounce.
- Focused native diagnostics-state/pacing tests and persistent no-mouse runtime regressions.

Out of scope:
- Color Pipeline redesign, phase-signal redesign, per-source SDF downsample, GPU Color Pipeline postprocess, SDF-native lanes, or animation system changes.
- Physical mouse automation.
- Capture quality downgrade, resolution cap reduction, or sample-tier downgrade.

## Investigation Notes

- The reported finding archived `frame.png` and the diagnostic `frame.bmp` under `runtime/diagnostics/20260526_185950_817__diagnostic_53988`; those two files are internally consistent.
- The saved `state.json` does persist the reported five SDF Source rows plus the two Phase Wheel palette rows and grading rows shown in the user's screenshots.
- A controlled headless diagnostic of the same state at `2048x1280` matches the user's viewport much more closely than the archived `4096x2560` finding. The archived frame shows stronger red/yellow phase-wheel rings because Capture Finding recomputed the Lens SDF field at the archive resolution.
- For SDF normal-angle and curvature rows, the SDF field resolution is part of the color source authority. Recomputing that field at a different output resolution is not a neutral high-resolution upscale.
- The second reported `explaino_all` finding has `last_render_ms` around `4988 ms` at `4096x2542`, f64/standard, SDF-heavy source rows, `lens.downsample = 1`, and `interaction_debounce_ms = 200`. A fixed 200 ms debounce is far shorter than the measured full-quality cost, so the viewer can try to settle full quality during active exploration and visibly flicker between low and high resolution.

## Implementation Direction

Keep Capture Finding high-resolution output. For SDF Color Pipeline captures, archive rendering should render the high-resolution output frame but compute the SDF field from the live/source viewport resolution, then apply the existing downsampled-field expansion path to the archive frame. Persist that source SDF field resolution into `state.json` so headless replay of the finding uses the same color-source authority.

Old states without the source-resolution field must continue to load with the current default behavior: SDF field source resolution follows the render target resolution.

For pacing, keep the adaptive preview scale policy but make settle timing aware of measured full-quality cost for SDF-heavy paths. The fix should avoid full/preview thrashing when a full-quality frame is seconds long, while still producing a full-quality settle after interaction truly stops.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on `codex/sdf-adaptive-preview-pacing` at `a3bf6c9`.
- Repo status: `py -3.14 tools/viewer_host_repo_status.py` reported a clean tree before this repair slice.
- Rearward review: `py -3.14 tools/viewer_host_rearward_review.py` returned `status=ok` for `a3bf6c9`.
- Reported finding read: `finding.json` repro command points at `--load-state-json ...\state.json --capture-diagnostic`; the finding frame and diagnostic frame are both `4096x2560`.
- User screenshots show the same serialized Color Pipeline rows as `state.json`, so the regression is not a missing row-stack serialization problem.
- Manual RED witness: the saved state rendered at `2048x1280` produces the viewport-like cyan/green/magenta image, while the high-res `4096x2560` Capture Finding produces the divergent red/yellow phase-wheel bands.
- Slice lock: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "Capture Finding viewport parity repair" --profile runtime --plan docs/notes/capture_finding_viewport_parity_repair_PHASED_PLAN.md --contract docs/contracts/capture_finding_viewport_parity_repair.contract.json` appended `ck:1be50ec3` and locked the active contract.
- Second reported issue read: `D:\salt-fractal\cuda_newton_fractal_clone\findings\manual_capture\2026-05-26\191229_004__explaino_all\state.json` records `last_render_ms = 4987.740234375`, `4096x2542`, `sample_tier = standard`, SDF-heavy source stack, and `lens.downsample = 1`.
- Native state IO proof: `capture_finding_native_state_io` passed `test_diagnostics_state_io`, including Lens SDF field source resolution round-trip and old-state defaults.
- Native pacing proof: `capture_finding_native_pacing` passed `test_viewer_render_pacing`, including the slow full-quality settle hold.
- Runtime publish proof: `capture_finding_runtime_publish` passed and staged the published runtime.
- Published Capture Finding proof: `capture_finding_runtime_sdf_rows` passed 7 no-mouse tests, including the new Julia multi-row SDF Capture Finding source-resolution replay test.
- Published SDF pacing proof: `capture_finding_runtime_sdf_pacing` passed. An earlier run failed on one pre-existing wall-clock report-observation threshold, then the same required label passed on rerun; the product assertions remained preview-active/faster-than-baseline.
- Diff check: `capture_finding_diff_check` passed `git diff --check`.
- Code quality: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/capture_finding_code_quality.json` passed.
- Contract validation: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/capture_finding_viewport_parity_repair.contract.json --out-json artifacts/validation/capture_finding_contract.json` passed.
- Phased-plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Hostile-audit validation: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/capture_finding_viewport_parity_repair_PHASED_PLAN.md --out-json artifacts/validation/capture_finding_hostile_audit.json` passed.

## Hostile Audit

- Status: complete
- Required posture: assume Capture Finding still ignores live viewport authority, replay still diverges, high-res output was silently downgraded, Color Pipeline row semantics changed, pacing still flickers between full and preview, or the test only proves headless replay instead of the in-loop Capture Finding button path.
- Did Capture Finding preserve the live/source SDF field authority? Yes; in-loop Capture Finding now stores the capture-time rendered frame size as `lens.sdf_field_source_width/height`, and headless replay uses that source field while keeping high-resolution archive output.
- Did high-resolution finding output stay high quality? Yes; `BuildFindingArchiveCaptureRenderForSource(...)` still selects the archive resolution and `sample_tier = standard`.
- Did the runtime proof exercise the in-loop Capture Finding button path? Yes; the new no-mouse test uses the persistent viewer `capture_finding` control and then validates the archived `state.json` and high-res `frame.png`.
- Did pacing avoid the fixed-debounce flicker case? Yes; very slow full-quality timing now extends the settle window up to a bounded hold, while the native test proves a full-quality settle is still issued.
- Did this change Color Pipeline row semantics, SDF phase behavior, GPU postprocess, or per-source downsample? No.

## Audit Passes

- [done] Pass 1 - inspected the implementation diff for source-resolution authority, replay authority, and high-res preservation.
- [done] Pass 2 - verified runtime proof exercises the in-process Capture Finding button path with no physical mouse.
- [done] Pass 3 - verified old states still default Lens SDF source resolution to zero and existing SDF source rows remain green.
- [done] Pass 4 - verified pacing fix does not become over-aggressive for cheap paths covered by existing native pacing cases and does not suppress eventual full-quality settle.
- [done] Pass 5 - repaired the initial runtime test mistake that compared against the first ready report instead of the capture-time rendered frame report.
- [done] Pass 6 - clean re-read after the focused native/runtime proofs, code-quality audit, and diff check found no additional real defect in the touched Capture Finding, Lens SDF replay, or pacing seams.

## Audit Findings

- [done] Finding 1: Capture Finding preserved aspect and high output resolution, but for SDF source rows it recomputed the Lens SDF field at the archive resolution. Repaired by persisting `lens.sdf_field_source_width/height` and using that field source for SDF postprocess replay.
- [done] Finding 2: SDF-heavy full-quality frames can cost multiple seconds while the settle debounce stays 200 ms, causing the viewer to re-enter full-quality too soon and visibly flicker between preview and full-quality resolutions. Repaired by adding a bounded slow full-quality settle hold to pacing state.
- [done] Finding 3: The first runtime regression proof used the initial ready report dimensions instead of the capture-time rendered-frame dimensions. Repaired by using the `capture_finding` click report as the source-resolution authority.
- [done] Clean re-read: no additional real defect found after rerunning focused native state/pacing rails, runtime publish, SDF Capture Finding runtime proof, SDF pacing runtime proof, code-quality baseline, and diff check.
