# Explaino Saved-Runtime Walker V1

## Current Phase

Complete - headless Explaino saved-runtime walker v1 is landed and validated; next follow-on is deeper mainline gold-subset continuation parity once broker-session inputs are wired

## Phase Checklist

- [x] Phase 1 - lock the walker contract, corpus posture, and first recovery slice
- [x] Phase 2 - add red tests for bundle parsing, deterministic state mapping, branch annotations, and runtime artifact regeneration
- [x] Phase 3 - land the native headless walker seam plus headless artifact bundle generation
- [x] Phase 4 - add the repo-local FITS corpus intake and mainline parity harness surface
- [x] Phase 5 - hostile-review the slice, repair at least one real defect if found, and checkpoint the validated stop point

## Notes

- Why this plan exists:
  - the recovered flashlight stack now gives this repo a deterministic headless trace/export pattern, but there is still no saved-runtime walker that replays Explaino state from `state.json` while using fresh FITS as an acceptance corpus
  - the old FITS playback note in `spec_intake/FitsSolutionSpacePlayback_DesignNote_2026-04-07.md` was design-only and predated the current flashlight, sidecar persistence, bridge/export, and sample-function seams
  - the user clarified two important constraints for this slice:
    - `state.json` remains the only replay authority in this repo for v1
    - `C:\code\hat-rack-v2\salts` is a real Explaino checkpoint corpus, but its folder layout is not stable and must not be hardcoded, especially `archive/`
- Locked implementation stance for v1:
  - add a native headless walker seam inside this repo's current runtime instead of reviving legacy branch code or building a second replay authority
  - keep the walker Explaino-family only
  - model one main line with explicit branch markers and sticky branch regions
  - keep slime observe-only
  - treat FITS as acceptance/comparison inputs and future bridge targets, not authoritative replay state
- First recovery slice boundary:
  - native request/result types for:
    - normalized Mr Zipper branch bundle parsing
    - `t -> state snapshot` mapping
    - branch-marker / sticky-region annotation
  - headless runtime mode that consumes a request JSON and writes:
    - `runtime_walk_report.json`
    - `runtime_walk_branch_manifest.json`
    - `runtime_walk_trace.csv`
    - `runtime_walk_trace.obj`
    - `runtime_walk_trace.stl`
    - `runtime_walk_trace_overlay.bmp`
    - per-tick diagnostics bundles under a stable output root
  - repo-local FITS corpus/parity tooling that:
    - accepts explicit FITS paths first
    - optionally scans shallowly for `checkpoint_final.fits`
    - never encodes `/archive/` or any other path pattern as meaning
- Explicit non-goals for this slice:
  - no branch-web solver
  - no active slime steering or prompt mutation loop
  - no attempt to promote FITS to replay authority inside this repo
  - no UI scrubber/timeline surface yet
- Landed behavior in this slice:
  - native `--runtime-walk-request-json` headless mode now replays Explaino-family state from `state.json` plus a canonical `mr_zipper_branch` bundle and writes:
    - `runtime_walk_report.json`
    - `runtime_walk_branch_manifest.json`
    - per-tick diagnostics bundles
  - repo-local `tools/explaino_runtime_walk.py` now:
    - regenerates `runtime_walk_trace.csv` / `.obj` / `.stl` / overlay BMP outputs from the native report
    - discovers checkpoint FITS by explicit path first and by shallow bounded search second, with no `/archive/` special casing
    - emits corpus and structural-parity JSON summaries against fresh `C:\code\hat-rack-v2\salts` inputs
  - hostile-review repairs already folded into the landed slice:
    - parser/runtime mismatch against the local `json_min` API
    - stale status-path creation failure in the Python tool
    - 24-bit BMP incompatibility in the shared trace exporter
    - missing branch-manifest freshness check in the runtime tool
    - corpus discovery default depth too shallow for the real `salts` tree
- Bounded gap kept explicit:
  - the read-only mainline continuation helper surface exists, but a direct April 16, 2026 probe showed `C:\code\salticid-cuda\scripts\run_salticid.py` still requires `--broker-session` alongside `--checkpoint-in/--checkpoint-out`
  - exact gold-subset continuation parity is therefore scaffolded but not yet fully automated in this repo without that additional session context
- Anticipated architecture:
  - reuse the current `--load-state-json` state-init path and diagnostics capture contract
  - reuse the flashlight trace/export geometry pattern instead of inventing a second STL/overlay writer
  - keep the parity harness read-only against `C:\code\salticid-cuda\scripts\run_salticid.py --checkpoint-in/--checkpoint-out`

## Validation

- `ui_app\build_tests_vsdevcmd.cmd`
- `ui_app\build_vsdevcmd.cmd`
- focused native walker/helper tests
- focused Python tests for corpus discovery and parity harnessing
- focused runtime walker pytest coverage
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`

## Evidence / References

- Existing runtime authority:
  - `ui_app/src/viewer_state_init.cpp`
  - `ui_app/src/diagnostics_capture.cpp`
  - `ui_app/src/diagnostics_state_io.cpp`
- Existing headless trace/export pattern:
  - `ui_app/src/flashlight_probe.cpp`
  - `tools/flashlight_bridge_runner.py`
- Mainline checkpoint / FITS reference:
  - `C:\code\salticid-cuda\salticid_runner\checkpoint.py`
  - `C:\code\salticid-cuda\scripts\run_salticid.py`
  - `C:\code\salticid-cuda\tests\test_checkpoint_py.py`
  - `C:\code\salticid-cuda\tests\test_mr_zipper_branch_seed_bridge.py`
- Acceptance corpus:
  - `C:\code\hat-rack-v2\salts\README.md`
  - `C:\code\hat-rack-v2\salts\scripts\salts_session.py`
