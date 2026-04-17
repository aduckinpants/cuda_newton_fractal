# Runtime-Walk Viewer Pit-of-Success Polish V2

## Current Phase

Complete - runtime-walk viewer Pit-of-Success polish v2 is landed, hostile-audited, and validated

## Phase Checklist

- [x] Phase 1 - hostile review the current viewer/operator flow and lock red tests for the import/session gap
- [x] Phase 2 - add a shared runtime-walk viewer import/session seam with recent/latest session persistence
- [x] Phase 3 - wire `Load FITS...`, import panel, and recent/latest affordances into the viewer without regressing the authority boundary
- [x] Phase 4 - add runtime proof, hostile-audit the repaired slice, and checkpoint the validated stop point

## Notes

- Why this plan exists:
  - runtime-walk playback is live and usable once a request JSON already exists
  - the operator still has no practical in-viewer path for starting from a loaded capture state plus a companion FITS
  - the current surface violates Pit of Success because `Open Request...` is still the only practical entrypoint
- Operator-contract rule:
  - `Load FITS...` exists so operators coming from other programs do not need repo-native JSON inputs
  - request/bundle/base-state artifacts may exist internally, but the default path must never ask the operator to supply them
- Locked stance for this slice:
  - replay authority remains `state.json` plus the runtime-walk / Mr Zipper bundle
  - FITS remains companion evidence only
  - `Load FITS...` must sit beside the existing capture-state flow and open a dedicated import panel
  - this historical slice stopped at import-panel/session plumbing; later slices supersede the old "ask for bundle/request" fallback with full default-path synthesis
  - recent/latest convenience is runtime-local only
- Intended recovery surface:
  - persist the current resolved base-state path after capture-state load, startup load, and runtime-walk viewer request load
  - add a shared import/session seam that generates a runtime-local request plus durable receipt/manifest under a diagnostics session root
  - add a latest/recent index so the viewer can reopen prior imported sessions without raw-path browsing
  - expose `Load FITS...`, `Open Latest`, and recent-session actions through a dedicated import panel
- First hostile-audit targets:
  - no loaded capture state
  - loaded non-Explaino capture state
  - FITS selected with no request/bundle
  - stale generated latest session
  - mismatched state/request/bundle/FITS combinations
  - misleading UI language that implies FITS is authoritative
- Landed behavior in this slice:
  - the Controls window now renders `Load FITS...` beside the existing capture-state action and disables it with a direct tooltip when no authoritative Explaino base state is available
  - runtime-walk viewer import sessions are now generated under a dedicated runtime diagnostics root with:
    - generated request JSON
    - selection manifest
    - receipt
    - recent/latest index
  - the viewer now exposes a dedicated FITS import panel with:
    - base-state authority messaging
    - FITS/request/bundle browsing
    - `Open Latest`
    - recent-session reopen
  - runtime-walk activation now updates the tracked authoritative base-state path and loading a fresh capture state clears the old runtime-walk session instead of silently letting old playback keep control
- Hostile-audit defects found and repaired:
  - first implementation/workflow defect:
    - the import seam initially targeted the wrong `json_min` parser API and failed the first native helper pass
    - repaired by switching the seam to the repo’s real `json_min::Parse(...)` / `error.empty()` contract
  - second implementation/runtime-proof defect:
    - the focused runtime viewer proof could compare captures before the window client size settled, producing false failures from mismatched frame sizes
    - repaired by stabilizing the capture helper before diffing
  - third behavioral defect:
    - the initial `Load FITS...` guard keyed off the current live fractal family instead of the authoritative loaded base-state family, so a user could load a valid Explaino base state, tweak the live view to another family, and lose the import path even though the saved base state remained valid
    - repaired by tracking the authoritative loaded base-state family separately and locking it with focused helper coverage

## Validation

- `ui_app\build_tests_vsdevcmd.cmd`
- `ui_app\build_vsdevcmd.cmd`
- focused native helper tests for runtime-walk viewer import/session persistence
- focused runtime pytest for runtime-walk viewer playback / import-flow proof
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
