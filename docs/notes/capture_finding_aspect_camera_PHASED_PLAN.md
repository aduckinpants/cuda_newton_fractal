# Capture Finding Aspect Camera Repair

## Current Phase

Phase 5 complete - preview downscale now requires measured slow-frame evidence; unknown or within-budget interaction remains full resolution.

## Phase Checklist

- [x] Phase 0 - inspect the reported manual capture bundle and identify the concrete capture sizing/camera seam.
- [x] Phase 1 - add focused RED coverage that wide visible viewport capture keeps the same aspect-derived camera framing at high resolution.
- [x] Phase 2 - repair manual finding capture so high-res output derives its aspect from the visible viewport while preserving center/zoom.
- [x] Phase 3 - validate native capture seams, publish runtime, run a no-mouse runtime proof, and complete hostile audit/receipts/checkpoint.
- [x] Phase 4 - repair viewport resolution fighting during slider-driven redraws and add the missing unit/smoke coverage.
- [x] Phase 5 - repair preview downscale policy so common fast/unknown-timing interaction stays full resolution, with focused render-pacing tests.

## Explicit User Asks

- [done] Fix capture finding so it respects the aspect ratio and camera the user sees.
- [done] Preserve high-resolution capture behavior and the existing capture outputs.
- [done] Stop left/right clipping caused by capture using the wrong aspect.
- [done] Keep this slice narrow; do not reopen unrelated fractal, color pipeline, or parameter work.
- [done] Repair the introduced viewport resolution fighting regression when slider interaction redraws the viewport.
- [done] Add the missing unit/smoke coverage for that viewport sizing path.
- [done] Repair lower-resolution preview kicking in too hard when there is no measured FPS loss, especially common low-iteration float32 interaction.

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

## Hostile Audit

- Status: complete
- Did the capture output actually keep the viewport aspect, or did it only change a metadata field?
- Did the repair preserve camera center and zoom instead of recentering or changing the visible frame?
- Did high-resolution capture remain high resolution?
- Did any test use physical mouse automation?
- Did the slice avoid unrelated fractal/color/parameter changes?
- Did preview downscale now require measured slow-frame evidence instead of reducing resolution on common fast or unknown-timing interaction?

## Audit Passes

- [done] Pass 1: reviewed the initial diff and found it fixed headless/configured render aspect but did not explicitly wire the in-loop button path to `RenderedFrameState`.
- [done] Pass 2: repaired that omission by adding `BuildFindingArchiveCaptureRenderForSource(...)` and routing button capture through the rendered frame dimensions when available.
- [done] Pass 3: clean re-read of the repaired diff found no renderer math, color pipeline, parameter schema, or physical mouse automation change.
- [done] Pass 4: regression audit found viewport settle used the whole content region instead of the fitted image rect; repair added headless layout coverage and routed live viewport sizing through it.
- [done] Pass 5: audit preview downscale policy confirmed the repair changes only pacing policy/tests/contract docs, keeps capture/camera/fractal/color paths untouched, and preserves slow-frame preview behavior.

## Audit Findings

- [done] Finding repaired: finding archive capture used a fixed square `4096x4096` output, which changed aspect and clipped wide views horizontally. The helper now derives a 4096-long-edge size from the source render aspect.
- [done] Audit finding repaired: the first implementation still risked button-path mismatch if the current rendered frame dimensions differed from configured render dimensions. The in-loop capture path now uses `RenderedFrameState` dimensions when they are valid.
- [done] Regression finding repaired: slider redraws could make adaptive settle render at the full available content-region aspect instead of the fitted image rect aspect. `g_viewportPixels` now tracks the displayed image rect.
- [done] Regression finding repaired: preview downscale no longer engages for unknown timing or near-budget full-resolution frames; it requires measured slow-frame evidence past the step-down hysteresis.

## Notes

- Expected owner seams: manual capture action code, diagnostics/finding state serialization if dimensions are written there, focused native capture tests, and one no-mouse runtime proof.
- Non-goals: renderer rewrite, color pipeline changes, resolution UX redesign, or changing normal live viewport rendering.
