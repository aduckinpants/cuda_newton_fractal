# Runtime-Walk FITS Usability Hostile Review

## Current Phase

Phase 4 - hostile review complete on the repaired FITS runtime-walk control surface and repaired `build_tests` validation rail; one bounded runtime click-automation gap remains

## Phase Checklist

- [x] Phase 1 - reproduce and lock the user-reported control-surface/default-path failures with focused helper/runtime regressions
- [x] Phase 2 - repair default fractal selection, expose usable mapping/transport controls in the FITS import/playback UI, and make the active generated-session artifacts discoverable
- [x] Phase 3 - tune generated transport/mapping behavior so warp-heavy bindings do not destabilize motion, add adjustable transport density/sampling, and surface mapping profile/binding summaries in-viewer
- [x] Phase 4 - repair the flaky `ui_app\build_tests_vsdevcmd.cmd` artifact/output model, rerun at least three deliberate hostile-review passes on the repaired state, and only then reclassify any remaining gaps as bounded/non-blocking

## Notes

- User-reported product failures to treat as first-class defects:
  - no visible/operator-friendly way to understand where the mapping files are or what is being bound to what
  - no in-viewer way to change transport density / sample count for a run
  - no usable control surface for mapping profile / parameter animation behavior
  - default synthesized base state is picking the wrong Explaino fractal family
  - direct warp binding can destabilize the whole image and destroys cohesive smooth motion
- Required hostile-review posture for this slice:
  - do not stop at one fix
  - do not treat helper-only proof as sufficient
  - do three deliberate passes on the repaired state and either close or explicitly report dangling threads
- Likely implementation seams:
  - `ui_app/src/runtime_walk_bootstrap.cpp`
  - `ui/runtime_walk_fits_mapping_profiles_v1.json`
  - `ui_app/src/runtime_walk_viewer_import.cpp`
  - `ui_app/src/runtime_walk_viewer_imgui.cpp`
  - `ui_app/src/main.cpp`
  - `ui_app/tests/test_runtime_walk_bootstrap.cpp`
  - `ui_app/tests/test_runtime_walk_viewer_import.cpp`
  - `tests/test_fractal_runtime_runtime_walk_viewer.py`
- Minimum closure criteria for this slice:
  - the default synthesized FITS path uses the correct Explaino family automatically
  - the import/playback UI exposes where the generated request/bundle/orientation/mapping artifacts live
  - the operator can adjust generated transport density without editing JSON by hand
  - the UI exposes enough mapping/binding summary to stop being blind
  - warp-heavy transport no longer destabilizes default motion
- Hostile-review findings closed in this slice:
  - FITS-only playback was previously visually static in the published runtime even when the session built successfully; runtime playback proof is green again after the repaired activation/default-motion path
  - generated-session identity previously ignored transport options, so sample-count/motion/warp tweaks could collapse onto the same session and feel ineffective; session identity now includes transport controls and is covered by helper regression
  - reopening the FITS import panel previously discarded the active session transport settings and dropped the operator back onto defaults; the panel now reloads the current session settings and shows transport metadata in playback
- Hostile-review/build-tool findings closed in this follow-up:
  - `ui_app\\build_tests_vsdevcmd.cmd` was contaminating itself with shared compiler outputs, which made full-batch native validation intermittently fail with linker/object-file collisions; the script now stages isolated object/PDB outputs under `build_tests` and repeated full-batch runs are green
  - the repaired FITS import panel had regressed the advanced manual override path by disabling `Open FITS` unless a FITS was selected, even when an explicit request/bundle override was present; helper coverage now locks the override path and the panel open-enable logic accepts FITS or advanced manual inputs
- Remaining bounded gap after the final second/third audit passes:
  - no runtime-level UI automation currently drives the `Generated Samples` slider or mapping-profile browse dialog; those paths are covered at helper/session-build level, while runtime acceptance remains FITS-only open-to-playback

## Validation

- `ui_app\build_tests_vsdevcmd.cmd`
- repeated full-batch proof via:
  - `py -3.14 tools/viewer_host_run_logged_command.py --label build-tests-fixed-pass1 --log artifacts/build_tests_fixed_pass1.log -- ui_app\build_tests_vsdevcmd.cmd`
  - `py -3.14 tools/viewer_host_run_logged_command.py --label build-tests-fixed-pass2 --log artifacts/build_tests_fixed_pass2.log -- ui_app\build_tests_vsdevcmd.cmd`
- focused native helper compiles and executions for:
  - `test_runtime_walk.exe`
  - `test_runtime_walk_bootstrap.exe`
  - `test_runtime_walk_viewer_import.exe`
- `ui_app\build_vsdevcmd.cmd`
- focused native helper tests for runtime-walk bootstrap/import/viewer seams
- `py -3.14 -m pytest tests/test_fractal_runtime_runtime_walk_viewer.py -q`
- `py -3.14 tools/viewer_host_runtime_pytest_lane.py`
- `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/runtime_walk_fits_usability_hostile_review_code_quality.json`
- `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`
