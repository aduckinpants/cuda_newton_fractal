# Capture Finding Aspect Camera Repair

## Current Phase

Phase 8 complete - no-mouse camera-center pacing proof is green, runtime report exposes pacing/timing evidence, and late viewport drag interaction now feeds the next pacing decision.

## Phase Checklist

- [x] Phase 0 - inspect the reported manual capture bundle and identify the concrete capture sizing/camera seam.
- [x] Phase 1 - add focused RED coverage that wide visible viewport capture keeps the same aspect-derived camera framing at high resolution.
- [x] Phase 2 - repair manual finding capture so high-res output derives its aspect from the visible viewport while preserving center/zoom.
- [x] Phase 3 - validate native capture seams, publish runtime, run a no-mouse runtime proof, and complete hostile audit/receipts/checkpoint.
- [x] Phase 4 - repair viewport resolution fighting during slider-driven redraws and add the missing unit/smoke coverage.
- [x] Phase 5 - repair preview downscale policy so common fast/unknown-timing interaction stays full resolution, with focused render-pacing tests.
- [x] Phase 6 - harden active-preview UX when debounce is needed, repair the code-quality baseline regression, add the missing focused pacing harness target, and run extended validation.
- [x] Phase 7 - restore Capture Finding standard/f64 output and make live preview recovery see non-benchmark frame time during slow interaction.
- [x] Phase 8 - prove camera center changes enter adaptive preview through no-mouse runtime automation, expose truthful pacing report fields, and repair lost viewport interaction feedback if confirmed.

## Explicit User Asks

- [done] Fix capture finding so it respects the aspect ratio and camera the user sees.
- [done] Preserve high-resolution capture behavior and the existing capture outputs.
- [done] Stop left/right clipping caused by capture using the wrong aspect.
- [done] Keep this slice narrow; do not reopen unrelated fractal, color pipeline, or parameter work.
- [done] Repair the introduced viewport resolution fighting regression when slider interaction redraws the viewport.
- [done] Add the missing unit/smoke coverage for that viewport sizing path.
- [done] Repair lower-resolution preview kicking in too hard when there is no measured FPS loss, especially common low-iteration float32 interaction.
- [done] Hostile-review the viewport preview UX when debounce is actually needed, and fix low-hanging defects not already documented/deferred.
- [done] Restore Capture Finding to f64/standard quality after the aspect/camera repair.
- [done] Fix the slow-interaction FPS recovery path so high-resolution low-FPS drag and slider edits actually enter preview.
- [done] Emulate the remaining pacing regression by changing camera X/Y through in-process automation, reading FPS/timing and live resolution, and responding with a fix when the report proves pacing is still wrong.

## Presumption Loop

The reported screenshot is wide, while the capture bundle appears to be rendered through a fixed or default capture resolution/aspect rather than the live viewport aspect. If the renderer maps camera zoom through render resolution, reusing the same center/zoom with a narrower capture aspect will visibly clip horizontal content. The cheapest proof is a focused capture-setting regression that builds the high-resolution capture size from a wide visible viewport and asserts that the resulting capture aspect, center, and zoom match the visible camera contract.

## Proof Ledger

- Found: `D:\salt-fractal\cuda_newton_fractal_clone\findings\manual_capture\2026-05-21\221756_601__multibrot\frame.png` is `4096x4096`, while the user-visible screenshot is wide; the captured `state.json` also persisted `render.width=4096` and `render.height=4096`.
- Found: `ui_app/src/finding_archive_actions.cpp` forced `BuildFindingArchiveCaptureRender(...)` to `4096x4096` for every finding archive capture.
- RED: `ui_app/tests/test_finding_archive_actions.cpp` was changed first to reject square capture and require source-aspect high-resolution capture, including `2048x1152 -> 4096x2304`; it failed on the old implementation.
- Landed: `BuildFindingArchiveCaptureRender(...)` now preserves source aspect with a 4096-pixel long edge and still enables benchmark timing without changing other render settings.
- Landed: in-loop Capture Finding now calls `BuildFindingArchiveCaptureRenderForSource(...)` with the actual `RenderedFrameState` dimensions when available, so the button path follows the frame the viewer just rendered instead of falling back to stale/default configured dimensions.
- Runtime proof: `tests/test_fractal_runtime_manual_capture_repro.py::test_capture_finding_preserves_wide_viewport_aspect_at_high_resolution` uses headless `--capture-finding` with no mouse and asserts archived `state.json` plus `frame.png` are `4096x2304` while a seeded nonzero center/zoom camera remains intact.
- Validated: `ui_app\build_tests_vsdevcmd.cmd test_finding_archive_actions` passed.
- Validated: `ui_app\build_vsdevcmd.cmd` passed and published `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Validated: `py -3.14 -m pytest tests/test_fractal_runtime_manual_capture_repro.py::test_capture_finding_preserves_wide_viewport_aspect_at_high_resolution -q` passed.
- Validated: `ui_app\build_tests_vsdevcmd.cmd serializer_owner_fast` passed.
- Validated: `py -3.14 tools/code_quality_audit.py --out artifacts/validation/capture_finding_aspect_camera_code_quality.json` passed with score `97/100`, no critical/errors.
- Validated: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/capture_finding_aspect_camera.contract.json --out-json artifacts/validation/capture_finding_aspect_camera_contract.json` passed.
- Validated: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Regression report: after the first pushed capture fix, slider-driven redraws visibly fought over viewport resolution.
- Found: `RenderFractalViewport(...)` tracked `g_viewportPixels` from the entire available content region, while the actual image draw rect was fitted to the render/frame aspect. That allowed settle renders to adopt the wrong available-region aspect and fight the displayed image aspect.
- Landed: `ComputeViewportDisplayLayout(...)` now makes the fitted displayed image rect a headless-testable seam.
- Landed: `RenderFractalViewport(...)` now records `g_viewportPixels` from the fitted image rect and uses that same rect for zoom/pan interaction math.
- RED/green: `ui_app/tests/test_viewport_interaction.cpp` now proves a wide available region with a 4:3 source settles to `1536x1152`, not `2048x1152`, and that preview frames scale into the same displayed rect.
- Validated: direct one-shell focused build/run of `test_viewport_interaction.exe` and `test_viewer_render_pacing.exe` passed.
- Validated: `ui_app\build_tests_vsdevcmd.cmd test_finding_archive_actions` passed after the viewport repair.
- Validated: `ui_app\build_vsdevcmd.cmd` passed after the viewport repair and republished `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Validated: `py -3.14 -m pytest tests/test_fractal_runtime_manual_capture_repro.py::test_capture_finding_preserves_wide_viewport_aspect_at_high_resolution -q` passed after the viewport repair.
- Validated: `py -3.14 tools/code_quality_audit.py --out artifacts/validation/capture_finding_aspect_camera_code_quality.json` passed after the viewport repair with score `97/100`, no critical/errors.
- Blocked rail: a full `ui_app\build_tests_vsdevcmd.cmd` run exceeded a 15-minute timeout during CUDA-heavy native tests; orphaned compiler processes were stopped and this slice did not claim full-native-suite closure.
- New regression report: lower-resolution preview is visibly kicking in during interaction even when common low-iteration float32 views have no FPS loss to recover.
- RED target: `test_viewer_render_pacing` must reject unknown-timing and within-budget interaction downscale while preserving slow-frame budget downscale.
- RED observed: after updating the focused pacing unit rail first, current code failed with `Expected unknown timing to stay full resolution until a slow frame is measured`.
- Landed: `TargetPreviewScale(...)` now returns full scale for unknown timing and only computes a lower preview scale after `last_render_ms` exceeds `target_frame_ms * step_down_hysteresis`.
- Landed: slow-frame behavior still uses the bounded `sqrt(target_frame_ms / last_render_ms)` preview scale and active preview recovery still eases upward gradually.
- Validated: direct one-shell focused build/run of `test_viewer_render_pacing.exe` and `test_viewport_interaction.exe` passed after the preview policy repair.
- Validated: `ui_app\build_vsdevcmd.cmd` passed and republished `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe` after the preview policy repair.
- Validated: `py -3.14 -m pytest tests/test_fractal_runtime_manual_capture_repro.py::test_capture_finding_preserves_wide_viewport_aspect_at_high_resolution -q` passed after the preview policy repair.
- Validated: `py -3.14 tools/code_quality_audit.py --out artifacts/validation/capture_finding_aspect_camera_code_quality.json` passed after the preview policy repair with score `97/100`, no critical/errors.
- Noted validation miss: `ui_app\build_tests_vsdevcmd.cmd test_viewer_render_pacing` is not a supported focused target in the checked-in script, so the focused pacing rail was run through the direct one-shell MSVC command instead.
- Phase 6 review found: bootstrap code-quality baseline rejects the previous helper shape with `viewer_render_pacing.cpp max_fn_lines 9 -> 10`.
- Phase 6 review found: active preview recovery ignores `step_up_hysteresis` and treats preview-frame timing as if it were full-resolution timing, which can pump resolution during needed debounce.
- Phase 6 review found: the checked native helper script compiles `test_viewer_render_pacing` in the full suite but lacks a focused `test_viewer_render_pacing` target, forcing ad hoc validation commands for this seam.
- Phase 6 RED observed: the added active-preview tests failed first with `Expected active preview with unknown timing to hold the current preview scale`.
- Landed: active preview now interprets render timing relative to the preview scale that produced it; unknown active-preview timing holds scale, middling timing holds through the recovery hysteresis band, and clearly fast preview frames recover gradually.
- Landed: `TargetPreviewScale(...)` was split into smaller helpers so `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/capture_finding_aspect_camera_code_quality.json` passes the baseline.
- Landed: `ui_app/build_tests_vsdevcmd.cmd test_viewer_render_pacing` is now a checked focused target for the pacing seam.
- Validated: `ui_app\build_tests_vsdevcmd.cmd test_viewer_render_pacing` passed.
- Validated: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/capture_finding_aspect_camera_code_quality.json` passed with score `97/100` and baseline check passed.
- Validated: `py -3.14 tools/viewer_host_run_logged_command.py --label pacing_qol_native_full --log artifacts/logs/pacing_qol_native_full.log -- ui_app/build_tests_vsdevcmd.cmd` passed with `All helper tests passed`.
- Validated: `ui_app\build_vsdevcmd.cmd` passed and republished `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Validated: `py -3.14 -m pytest tests/test_fractal_runtime_resolution_pacing.py tests/test_fractal_runtime_manual_capture_repro.py::test_capture_finding_preserves_wide_viewport_aspect_at_high_resolution -q` passed with `2 passed`.
- Validated: `git diff --check` passed.
- Phase 7 regression report: Capture Finding should remain f64/standard, but the current high-resolution aspect repair preserves the live viewport sample tier, so a fast/f32 viewport archives a fast/f32 finding.
- Phase 7 regression report: high-resolution slow interaction still does not recover; renderer inspection found normal non-benchmark `RenderFractalCUDA(...)` writes `last_render_ms = 0`, leaving preview pacing blind outside benchmark/capture paths.
- Phase 7 RED targets: native capture helper must reject f32 Capture Finding output, native renderer stats must reject zero non-benchmark live timing, and runtime Capture Finding must archive `sample_tier=standard` with `resolved_backend=float64`.
- Phase 7 RED observed: `ui_app\build_tests_vsdevcmd.cmd test_finding_archive_actions` failed with `Expected finding archive capture to force the standard float64 sample tier even from a fast live viewport`.
- Phase 7 RED observed: `ui_app\build_tests_vsdevcmd.cmd test_fractal_renderer` failed with `FAIL: Non-benchmark live render reports elapsed milliseconds for viewport pacing`.
- Landed: `BuildFindingArchiveCaptureRenderForSource(...)` now forces `captureRender.sample_tier = SampleTier::standard` while preserving high-resolution aspect, benchmark timing, block size, device id, and preview settings.
- Landed: `RenderFractalCUDA(...)` now reports elapsed frame time for non-benchmark live renders; benchmark renders still prefer CUDA event timing when available.
- Landed: the status panel no longer labels all `last_render_ms` values as benchmark-only.
- Validated: `ui_app\build_tests_vsdevcmd.cmd test_finding_archive_actions` passed.
- Validated: `ui_app\build_tests_vsdevcmd.cmd test_fractal_renderer` passed with `test_fractal_renderer: passed=76 failed=0`.
- Validated: `ui_app\build_tests_vsdevcmd.cmd test_viewer_render_pacing` passed.
- Validated: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/capture_finding_aspect_camera_code_quality.json` passed with score `97/100` and baseline check passed.
- Validated: `ui_app\build_vsdevcmd.cmd` passed and republished `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Validated: `py -3.14 -m pytest tests/test_fractal_runtime_manual_capture_repro.py::test_capture_finding_preserves_wide_viewport_aspect_at_high_resolution -q` passed with `1 passed`; the runtime archive asserted `sample_tier=standard`, `resolved_backend=float64`, nonzero `last_render_ms`, aspect, and camera.
- Validated: `py -3.14 tools/viewer_host_run_logged_command.py --label capture_pacing_phase7_native_full --log artifacts/logs/capture_pacing_phase7_native_full.log -- ui_app/build_tests_vsdevcmd.cmd` passed with `All helper tests passed`.
- Validated: `py -3.14 -m pytest tests/test_fractal_runtime_resolution_pacing.py -q` passed with `1 passed`.
- Phase 8 RED target: runtime automation must change `fractal_control.center_x.primary` / `center_y` without physical mouse input, observe measured `last_render_ms`, and prove slow measured frames reduce the live rendered frame below the target render resolution while fast/unknown cases remain full resolution.
- Phase 8 inspection finding: `RenderFractalViewport(...)` reports wheel/drag interaction after the pacing decision and render dispatch, and the resulting flag is not fed back into `NoteViewerInteraction(...)`, so direct viewport drag can be lost before the next frame's pacing decision.
- Landed: the UI automation report now publishes `target_render_width`, `target_render_height`, `last_render_ms`, `last_render_fps`, `render_pacing_preview_active`, `render_pacing_preview_scale`, and pacing render dimensions so runtime tests can read actual FPS/pacing state.
- Landed: `RunViewerFrame(...)` now records viewport wheel/drag interaction after `RenderFractalViewport(...)` and calls `NoteViewerInteraction(...)` so direct camera drag influences the next frame's pacing decision instead of being discarded.
- Runtime proof: `tests/test_fractal_runtime_resolution_pacing.py::test_camera_center_edits_enter_preview_when_measured_frames_are_slow_no_mouse` launches one viewer, edits `center_x` and `center_y` via set-value automation, observes a measured slow full-resolution frame, proves preview active with lower live dimensions, then observes non-preview settle.
- Validated: `ui_app\build_tests_vsdevcmd.cmd test_viewer_ui_automation_report` passed.
- Validated: `ui_app\build_tests_vsdevcmd.cmd test_viewer_render_pacing` passed.
- Validated: `ui_app\build_vsdevcmd.cmd` passed and republished `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Validated: `py -3.14 -m pytest tests/test_fractal_runtime_resolution_pacing.py::test_camera_center_edits_enter_preview_when_measured_frames_are_slow_no_mouse -q` passed with `1 passed`.
- Validated: `py -3.14 -m pytest tests/test_fractal_runtime_resolution_pacing.py -q` passed with `2 passed`.
- Validated: `py -3.14 -m pytest tests/test_fractal_runtime_persistent_viewer_harness.py::test_persistent_runtime_viewer_batches_set_value_proofs_in_one_process -q` passed with `1 passed`.
- Validated: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/capture_finding_aspect_camera_code_quality.json` passed with score `97/100` and baseline check passed.
- Validated: `ui_app\build_tests_vsdevcmd.cmd test_finding_archive_actions` passed.
- Validated: `py -3.14 -m pytest tests/test_fractal_runtime_manual_capture_repro.py::test_capture_finding_preserves_wide_viewport_aspect_at_high_resolution -q` passed with `1 passed`.
- Validated: `ui_app\build_tests_vsdevcmd.cmd test_fractal_renderer` passed with `test_fractal_renderer: passed=76 failed=0`.
- Attempted but not green: `py -3.14 tools/viewer_host_run_logged_command.py --label capture_pacing_phase8_native_full --log artifacts/logs/capture_pacing_phase8_native_full.log -- ui_app/build_tests_vsdevcmd.cmd` exceeded a 20-minute timeout while compiling CUDA-heavy tests; leftover child build processes were stopped.
- Validated: `git diff --check` passed with only the existing line-ending warning for `HANDOFF_LOG.md`.

## Hostile Audit

- Status: complete
- Did the capture output actually keep the viewport aspect, or did it only change a metadata field?
- Did the repair preserve camera center and zoom instead of recentering or changing the visible frame?
- Did high-resolution capture remain high resolution?
- Did any test use physical mouse automation?
- Did the slice avoid unrelated fractal/color/parameter changes?
- Did preview downscale now require measured slow-frame evidence instead of reducing resolution on common fast or unknown-timing interaction?
- When preview is already active, does recovery use measured preview scale and `step_up_hysteresis` so needed debounce does not pump resolution?
- Does the pacing seam satisfy the repo code-quality baseline after the QoL pass?
- Can `test_viewer_render_pacing` run through the checked-in native helper harness instead of an ad hoc MSVC command?
- Does Capture Finding force standard/f64 independently of the current live viewport tier?
- Does the normal live renderer report nonzero frame timing so pacing can react without `benchmark=true`?
- Does no-mouse camera-center automation prove slow measured interaction enters preview and later settles back to full quality?
- Does direct viewport wheel/drag interaction update pacing state for the next frame instead of being detected too late and discarded?

## Audit Passes

- [done] Pass 1: reviewed the initial diff and found it fixed headless/configured render aspect but did not explicitly wire the in-loop button path to `RenderedFrameState`.
- [done] Pass 2: repaired that omission by adding `BuildFindingArchiveCaptureRenderForSource(...)` and routing button capture through the rendered frame dimensions when available.
- [done] Pass 3: clean re-read of the repaired diff found no renderer math, color pipeline, parameter schema, or physical mouse automation change.
- [done] Pass 4: regression audit found viewport settle used the whole content region instead of the fitted image rect; repair added headless layout coverage and routed live viewport sizing through it.
- [done] Pass 5: audit preview downscale policy confirmed the repair changes only pacing policy/tests/contract docs, keeps capture/camera/fractal/color paths untouched, and preserves slow-frame preview behavior.
- [done] Pass 6: audit confirmed active-preview recovery now uses measured preview scale plus step-up/step-down hysteresis, code quality baseline passes, and the focused pacing harness target runs through the checked native helper.
- [done] Pass 7: audit confirmed Capture Finding forces standard/f64 independent of live tier, normal live renders report nonzero timing for pacing, no OS-mouse automation was added, and capture aspect/camera behavior stayed covered.
- [done] Pass 8: audit confirmed no-mouse camera-center automation enters preview on slow measured frames, the report carries timing/pacing evidence, direct viewport interaction now feeds the next pacing decision, and no OS-mouse automation was added.

## Audit Findings

- [done] Finding repaired: finding archive capture used a fixed square `4096x4096` output, which changed aspect and clipped wide views horizontally. The helper now derives a 4096-long-edge size from the source render aspect.
- [done] Audit finding repaired: the first implementation still risked button-path mismatch if the current rendered frame dimensions differed from configured render dimensions. The in-loop capture path now uses `RenderedFrameState` dimensions when they are valid.
- [done] Regression finding repaired: slider redraws could make adaptive settle render at the full available content-region aspect instead of the fitted image rect aspect. `g_viewportPixels` now tracks the displayed image rect.
- [done] Regression finding repaired: preview downscale no longer engages for unknown timing or near-budget full-resolution frames; it requires measured slow-frame evidence past the step-down hysteresis.
- [done] QoL finding repaired: active-preview recovery now uses current preview scale and `step_up_hysteresis`, holding scale for unknown/middling timings and recovering only on clearly fast preview frames.
- [done] QoL finding repaired: pacing helpers were split so the repo code-quality baseline passes.
- [done] Test-harness finding repaired: `ui_app/build_tests_vsdevcmd.cmd test_viewer_render_pacing` now compiles and runs the focused pacing unit rail.
- [done] Regression finding repaired: Capture Finding now forces `SampleTier::standard`, and runtime archive proof confirms `sample_tier=standard` with `resolved_backend=float64`.
- [done] Regression finding repaired: normal non-benchmark live renders now report elapsed frame time, so preview pacing can react to slow drag/slider frames without requiring benchmark mode.
- [done] Regression finding repaired: camera X/Y set-value automation now proves slow measured interaction enters preview and settles; late viewport wheel/drag interaction is fed into pacing state for the next frame instead of being dropped after render dispatch.

## Notes

- Expected owner seams: manual capture action code, diagnostics/finding state serialization if dimensions are written there, focused native capture tests, and one no-mouse runtime proof.
- Non-goals: renderer rewrite, color pipeline changes, resolution UX redesign, or changing normal live viewport rendering.
