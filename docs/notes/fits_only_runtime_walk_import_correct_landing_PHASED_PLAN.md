# FITS-Only Runtime-Walk Import Correct Landing

## Current Phase

Completed - FITS-only runtime-walk import now defaults to synthesized Explaino authority, no longer auto-reuses stale JSON-oriented sessions in the default path, and the published viewer proves FITS-only startup playback from a real corpus artifact

## Phase Checklist

- [x] Phase 1 - reproduce the remaining JSON-prompt/default-path failure and lock red tests around the real operator flow
- [x] Phase 2 - remove default-path request/bundle/state prompts from the viewer import UI and make FITS-only open unconditional
- [x] Phase 3 - tighten synthesized session generation, request/session receipts, and generated transport metadata for FITS-only authority mode
- [x] Phase 4 - add published-runtime proof that FITS-only open reaches visible playback with no JSON prompt, hostile-audit the repaired slice, and checkpoint the validated stop point

## Notes

- Locked product contract:
  - operator input for the normal path is FITS only
  - `Load FITS...` must never require `state.json`, `finding.json`, request JSON, or bundle JSON
  - repo-native state/request/bundle/session artifacts are internal synthesized plumbing only
  - the normal path is `pick FITS -> open FITS -> playback`
- Current failure shape to close:
  - the helper/bootstrap seams can already synthesize a runtime-walk session, but the live viewer path still exposes and can still depend on JSON-oriented request/bundle browsing behavior
  - closure requires the published viewer to prove the real operator path, not just helper seams
- Immediate implementation targets:
  - `ui_app/src/runtime_walk_viewer_imgui.cpp`
  - `ui_app/src/runtime_walk_viewer_import.cpp`
  - `ui_app/src/main.cpp`
  - `ui_app/tests/test_runtime_walk_viewer_import.cpp`
  - `tests/test_fractal_runtime_runtime_walk_viewer.py`
- First hostile-audit targets:
  - `Open FITS` still routing into request/bundle browsing or loaded-state assumptions
  - stale recent/latest reuse masking required fresh synthesis
  - generated session succeeds on disk but playback still does not visibly start
  - UI status text implying another JSON-selection step instead of open-or-error
- Landed repair notes:
  - synth-mode default import no longer auto-reuses recent/sibling request JSON; default FITS-open now forces fresh generated transport unless the operator explicitly browses a request or bundle
  - the import panel now defaults to synthesized authority even when a capture state is loaded, surfaces FITS-first controls, and tucks request/bundle browsing behind advanced overrides
  - a new `--load-runtime-walk-fits <fits>` viewer startup path exercises the same FITS-only import seam for published-runtime acceptance proof
  - hostile audit found and repaired two real issues after the first implementation pass:
    - generated synthesized base state was written as `synthesized_base_state.json`, but the runtime loader only accepts `state.json` / `finding.json`, so activation failed after generation
    - recent/latest bookkeeping still trusted generated-but-never-loaded sessions, so stale JSON-oriented sessions could be reopened as if they were good
  - hostile review follow-up found a third real issue after landing:
    - `Open Latest` only inspected the newest recent-session record, so one newer unloaded or stale generated session poisoned the convenience path even when an older successfully loaded FITS session still existed; the loader now returns the most recent loadable successful session instead
- Remaining explicit coverage gaps after the repaired hostile review:
  - published runtime proof still covers FITS-only playback through the CLI boot path, not the actual file-dialog click path
  - there is still no runtime-level proof for `Open Latest` or manual advanced request/bundle overrides; current coverage for those branches is helper-level only

## Validation

- `ui_app\build_tests_vsdevcmd.cmd`
- `ui_app\build_vsdevcmd.cmd`
- focused native helper tests for runtime-walk bootstrap/import
- focused runtime pytest for runtime-walk viewer FITS-only playback proof
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/fits_only_runtime_walk_import_correct_landing_code_quality.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
