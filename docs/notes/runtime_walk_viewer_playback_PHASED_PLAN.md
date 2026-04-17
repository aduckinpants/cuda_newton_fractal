# Runtime-Walk Viewer Playback V1

## Current Phase

Complete - runtime-walk viewer playback v1 is landed, hostile-audited, and validated

## Phase Checklist

- [x] Phase 1 - lock the viewer playback contract, overlay boundary, and red-test surface
- [x] Phase 2 - add focused red tests for viewer playback state, spline/closed-loop path generation, and runtime-local gradient overlay behavior
- [x] Phase 3 - land extracted viewer playback and overlay seams without expanding the `main.cpp` monolith
- [x] Phase 4 - wire the viewer panel, viewport overlays, and runtime request loading into the live runtime
- [x] Phase 5 - add live/runtime proof, hostile-review the repaired slice, and checkpoint the validated stop point

## Notes

- Why this plan exists:
  - the repo already has a validated headless Explaino saved-runtime walker, trace export, and RTK v3 measurement lane
  - there is still no in-viewer playback or scrub surface for runtime-walk requests, so the saved-runtime work is not yet semi-usable from the live viewer
  - the user explicitly wants this slice to replay the actual fractal state through interpolated multi-parameter motion, not frame-playback from FITS
- Locked implementation stance for V1:
  - replay authority remains `state.json` plus the runtime-walk / Mr Zipper bundle
  - FITS and harvested RTK artifacts are companion evidence only
  - first viewer target is Explaino-family only
  - use the existing `EvaluateRuntimeWalkSnapshot(...)` and `ApplyRuntimeWalkSnapshot(...)` seams as the transport authority
  - keep playback as a dedicated subsystem rather than bolting it onto sweep mode
  - extract new viewer playback logic out of `main.cpp`; do not grow the monolith
- Required V1 operator behavior:
  - load a runtime-walk request or equivalent resolved state+bundle pair
  - play/pause and scrub continuous `t`
  - step next/prev by tick
  - show nearest branch marker and sticky-region status
  - render the live fractal POV through interpolated multi-parameter state updates
  - render raw path by default, with optional spline smoothing and optional closed-loop fit
  - render a runtime-local gradient-flow overlay from the current marker toward nearby strong gradients / nearby almosts
  - surface companion FITS / RTK evidence paths without making them authoritative
- Planned implementation layers:
  - Layer A: authoritative state transport
    - resolved runtime-walk viewer asset
    - continuous `t` playback state
    - snapshot evaluation and application into live `ViewState` / `KernelParams`
  - Layer B: path visualization
    - raw polyline
    - deterministic smoothed spline fit
    - optional closed-loop display that reconnects final point to first point without changing transport behavior
  - Layer C: overlay provider model
    - shipped provider: runtime-local gradient provider
    - reserved future provider slot for imported RTK / FITS hotspot overlays
- Red-test boundary for this slice:
  - viewer playback state can load from the existing runtime-walk request shape
  - continuous viewer interpolation matches the headless runtime-walk evaluation seam
  - branch-marker nearest/sticky annotation matches headless behavior
  - spline and closed-loop display generation is deterministic from a fixed point set
  - runtime-local gradient overlay produces finite guide strokes, stops below threshold, and does not mutate authoritative transport state
- First runtime proof target:
  - published runtime opens a real runtime-walk request into a viewer playback session
  - play/pause, stepping, and loop visibly move the fractal state
  - overlay toggles affect rendering only, not transport state
  - sparse runtime-walk requests still render a usable raw path
- Landed behavior in this slice:
  - viewer CLI now accepts `--load-runtime-walk-request-json <request.json>` for interactive playback while preserving the existing headless `--runtime-walk-request-json` mode
  - runtime-walk playback is now implemented as extracted seams in:
    - `ui_app/src/runtime_walk_viewer.*`
    - `ui_app/src/runtime_walk_viewer_session.*`
    - `ui_app/src/runtime_walk_viewer_imgui.*`
  - the live viewer now:
    - loads a runtime-walk request into a resolved Explaino-only playback session
    - replays interpolated multi-parameter state through the live fractal runtime
    - exposes play/pause, step, speed, loop, and `t` scrubbing controls
    - renders raw path, spline path, optional closed-loop fit, branch markers, and a runtime-local gradient-flow overlay in the viewport
    - surfaces companion FITS / RTK paths in the playback panel without making them authoritative
  - focused proving coverage now includes:
    - `ui_app/tests/test_runtime_walk_viewer.cpp`
    - `tests/test_fractal_runtime_runtime_walk_viewer.py`
- Hostile-audit defects found and repaired:
  - first implementation defect:
    - symmetric sample neighborhoods could collapse the local tangent to zero, leaving the runtime-local gradient provider with no guide-stroke directions and producing an empty overlay
    - repaired by falling back to local channel-gradient direction estimates when the path tangent degenerates
  - second implementation/workflow defect:
    - wiring the feature directly through `WinMain` pushed `main.cpp` max function length from the 189-line baseline to 309 lines, failing the repo code-quality ratchet
    - repaired by extracting the runtime-walk viewer lifecycle into dedicated per-frame/session helpers and restoring the code-quality baseline to passing

## Validation

- `ui_app\build_tests_vsdevcmd.cmd`
- `ui_app\build_vsdevcmd.cmd`
- focused native helper tests for runtime-walk viewer playback
- focused runtime pytest coverage for viewer playback / scrub behavior
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`

## Evidence / References

- Existing headless runtime-walk seams:
  - `ui_app/src/runtime_walk.h`
  - `ui_app/src/runtime_walk.cpp`
  - `ui_app/src/runtime_walk_headless.cpp`
- Existing viewer interaction patterns:
  - `ui_app/src/main.cpp`
  - `ui_app/src/viewer_sweep.cpp`
  - `ui_app/tests/test_viewer_sweep.cpp`
- Existing runtime playback proof surfaces:
  - `tests/test_fractal_runtime_explaino_runtime_walk.py`
  - `tools/explaino_runtime_walk.py`
