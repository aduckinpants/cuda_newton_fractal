# Capture Finding Aspect Camera Repair

## Current Phase

Phase 3 complete - capture finding now derives high-resolution archive aspect from the visible/rendered source frame when available and from the requested render size for headless capture; checkpoint, receipts, and push remain.

## Phase Checklist

- [x] Phase 0 - inspect the reported manual capture bundle and identify the concrete capture sizing/camera seam.
- [x] Phase 1 - add focused RED coverage that wide visible viewport capture keeps the same aspect-derived camera framing at high resolution.
- [x] Phase 2 - repair manual finding capture so high-res output derives its aspect from the visible viewport while preserving center/zoom.
- [x] Phase 3 - validate native capture seams, publish runtime, run a no-mouse runtime proof, and complete hostile audit/receipts/checkpoint.

## Explicit User Asks

- [done] Fix capture finding so it respects the aspect ratio and camera the user sees.
- [done] Preserve high-resolution capture behavior and the existing capture outputs.
- [done] Stop left/right clipping caused by capture using the wrong aspect.
- [done] Keep this slice narrow; do not reopen unrelated fractal, color pipeline, or parameter work.

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

## Hostile Audit

- Status: complete
- Did the capture output actually keep the viewport aspect, or did it only change a metadata field?
- Did the repair preserve camera center and zoom instead of recentering or changing the visible frame?
- Did high-resolution capture remain high resolution?
- Did any test use physical mouse automation?
- Did the slice avoid unrelated fractal/color/parameter changes?

## Audit Passes

- [done] Pass 1: reviewed the initial diff and found it fixed headless/configured render aspect but did not explicitly wire the in-loop button path to `RenderedFrameState`.
- [done] Pass 2: repaired that omission by adding `BuildFindingArchiveCaptureRenderForSource(...)` and routing button capture through the rendered frame dimensions when available.
- [done] Pass 3: clean re-read of the repaired diff found no renderer math, color pipeline, parameter schema, or physical mouse automation change.

## Audit Findings

- [done] Finding repaired: finding archive capture used a fixed square `4096x4096` output, which changed aspect and clipped wide views horizontally. The helper now derives a 4096-long-edge size from the source render aspect.
- [done] Audit finding repaired: the first implementation still risked button-path mismatch if the current rendered frame dimensions differed from configured render dimensions. The in-loop capture path now uses `RenderedFrameState` dimensions when they are valid.

## Notes

- Expected owner seams: manual capture action code, diagnostics/finding state serialization if dimensions are written there, focused native capture tests, and one no-mouse runtime proof.
- Non-goals: renderer rewrite, color pipeline changes, resolution UX redesign, or changing normal live viewport rendering.
