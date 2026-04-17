# Zero-Blocker FITS Open For Runtime-Walk

## Current Phase

Completed - zero-blocker FITS open now synthesizes the full runtime-walk session path when authored transport artifacts are absent, with recent/latest guarded against stale generated bundles

## Phase Checklist

- [x] Phase 1 - lock the zero-blocker invariant, identify the current hard-stop seam, and add red-test coverage for FITS-only import with no authored request/bundle
- [x] Phase 2 - add deterministic runtime-walk transport synthesis plus generated bundle/request artifacts for FITS-only import
- [x] Phase 3 - rework the viewer import flow so `Load FITS...` succeeds through the basic path without a second blocker
- [x] Phase 4 - add runtime proof for generated-session playback, hostile-audit the repaired slice, and checkpoint the validated stop point

## Notes

- Why this plan exists:
  - the previous FITS synthesis slice fixed base-state authority, but not the full operator path
  - `Load FITS...` can still fail after the first step with "could not discover a compatible runtime-walk request or bundle"
  - that is a direct Pit-of-Success violation because the default path still expects authored transport artifacts
- Locked implementation stance for this slice:
  - FITS-only import must auto-generate the full session when no authored request/bundle exists
  - the generated session must include synthesized base-state JSON, orientation-input JSON, synthesized bundle JSON, synthesized request JSON, and import receipt/manifest
  - authored request/bundle selection remains supported as an advanced override
  - recent/latest reuse is allowed only when the matching generated session is still valid; stale reuse must not mask needed synthesis
- Default generated transport requirements:
  - valid `mr_zipper_branch` bundle
  - at least two samples
  - deterministic `t` schedule
  - branch-marker surface present even if minimal
  - multi-parameter transport, not camera-only motion
  - open-path transport by default and compatible with existing spline/closed-loop display overlays
- First hostile-audit targets:
  - FITS-only import still encountering a second blocker anywhere in the default path
  - synthesized bundle degenerating into a single-parameter or visually static path
  - stale recent/latest session being silently reused instead of generating a new one
  - explicit request/bundle overrides being ignored once synthesis exists
  - generated session artifacts not being durable/reopenable through recent/latest

## Validation

- `ui_app\build_tests_vsdevcmd.cmd`
- `ui_app\build_vsdevcmd.cmd`
- focused native helper tests for bootstrap transport synthesis and runtime-walk viewer import
- focused runtime pytest coverage for FITS-only generated-session playback
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/zero_blocker_fits_open_code_quality.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
