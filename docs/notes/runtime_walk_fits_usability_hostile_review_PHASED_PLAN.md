# Runtime-Walk FITS Usability Hostile Review

## Current Phase

Phase 6 - measured FITS field playback re-closure complete pending checkpoint

## Phase Checklist

- [x] Phase 1 - reproduce and lock the user-reported control-surface/default-path failures with focused helper/runtime regressions
- [x] Phase 2 - repair default fractal selection, expose usable mapping/transport controls in the FITS import/playback UI, and make the active generated-session artifacts discoverable
- [x] Phase 3 - tune generated transport/mapping behavior so warp-heavy bindings do not destabilize motion, add adjustable transport density/sampling, and surface mapping profile/binding summaries in-viewer
- [x] Phase 4 - repair the flaky `ui_app\build_tests_vsdevcmd.cmd` artifact/output model, rerun at least three deliberate hostile-review passes on the repaired state, and only then reclassify any remaining gaps as bounded/non-blocking
- [x] Phase 5 - replace the remaining toy runtime-walk assumptions with a schema-derived FITS binding workbench, live-baseline-plus-offset playback composition, and measured slime-backed field traveler/export proof
- [x] Phase 6 - connect per-frame FITS timelines, live binding offsets, and measured slime overlay into the visible playback loop

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
- Hostile-review findings closed in Phase 5:
  - binding rows were initially only summarized/edited from existing profile rows, leaving no in-viewer add/remove control; the workbench now exposes `Add Binding` and per-row remove controls, and `viewer_host_validate_fits_contract.py` requires those strings in the shipped UI source.
  - adaptive field sampling controls were visible but the field slime population did not actually resize from measured gradient pressure; `TestFieldStepAdaptsMarblePopulationToMeasuredGradient` failed red, then passed after measured finite-gradient counts drive marble population size.
  - field CSV export could be observed as an empty file by the runtime proof while the viewer was writing/truncating it; `WriteRuntimeWalkFieldSlimeCsv` now publishes temp files atomically and the runtime pytest waits for non-empty flow/cell evidence.
  - full native validation could fail after a timeout because stale compiler locks made `build_tests_vsdevcmd.cmd` leave `obj` behind and then fail at `mkdir`; cleanup now retries before failing, and full native validation passed after the repair.
- Hostile-review findings closed in Phase 6:
  - the FITS extractor still treated higher-dimensional inputs as a single aggregate plane, so playback had no current FITS moment; `tests/test_runtime_walk_extract_fits_orientation.py::test_runtime_walk_extract_fits_orientation_preserves_frame_timeline` now proves per-frame timeline output.
  - live FITS bindings were generated/import artifacts, not continuous playback composition; `TestLiveFitsBindingsApplyOffsetsOverBaseline` now proves frame and field bindings compose over the live baseline while preserving default warp.
  - runtime playback could still look static when `ImGuiIO::DeltaTime` was zero/near-zero under the published viewer harness; the playback loop now has a bounded wall-clock delta fallback.
  - stale live CSV exports could satisfy runtime proof after regeneration; generated FITS sessions now remove old flow/cell/binding exports before fresh playback.
  - field and binding CSVs initially overwrote snapshots instead of accumulating time history; field/cell/binding exports now append rows with a single header and runtime proof verifies non-empty current-run history.
  - the first mix binding path could drive `explaino_mix` outside its valid domain; default mapping plus runtime target-domain clamp now keep it in `[0,1]`.
  - live binding row clamps were incorrectly applied to offsets before baseline composition, erasing negative field modulation; the red native regression now proves negative phase-strength offsets survive and clamp only after composition.
  - measured field flow/cell exports lacked active FITS frame context; flow and cell CSVs now include `fits_frame_index`, and the runtime proof plus contract validator check it.
  - the live playback workbench exposed only amount/offset/smoothing, leaving curve/polarity/clamp as read-only or import-only behavior; live rows now expose curve, invert polarity, and clamp min/max controls under a contract assertion.
- Hostile-review passes on repaired Phase 6 state:
  - Pass 1 diff/artifact audit: no new defect after CSV history, stale-export, mix-domain, offset-clamp, and FITS-frame-context repairs; `artifacts/hostile_review/measured_field_artifact_audit_final.json` verifies no warp targets, bounded mix, negative field phase offset, field-step flow rows, finite tangents, nontrivial time history, and FITS frame context.
  - Pass 2 UI/operator review: found and repaired missing live curve/polarity/clamp binding controls; default FITS-only synth mode, advanced overrides, artifact paths, and no default warp UI remain machine-validated.
  - Pass 3 math/data-flow review: no new defect after repaired source/artifact audit; binding math now uses offset first, composed-value clamp second, and synthesis preserves absolute binding clamps.
- Remaining bounded gap after final repaired-state audit passes:
  - runtime-level UI automation still does not drag individual ImGui workbench sliders/buttons; native/session tests, source validators, runtime artifact proof, and published viewer FITS-only playback cover the live binding rows, controls, generated artifacts, measured exports, and no-warp policy. A future UI automation pass should drive the workbench interactively through ImGui/window automation.

## Validation

- Red proof: `py -3.14 tools/viewer_host_run_logged_command.py --label slime-field-adaptive-red-focused --log artifacts/slime_field_adaptive_red_focused.log -- cmd /c %TEMP%\run_field_slime_focused.cmd` failed on adaptive marble population before the fix.
- Focused green proof: `py -3.14 tools/viewer_host_run_logged_command.py --label slime-field-adaptive-green-focused-atomic --log artifacts/slime_field_adaptive_green_focused_atomic.log -- cmd /c %TEMP%\run_field_slime_focused.cmd` passed `test_runtime_walk_field_slime: 24 passed, 0 failed`.
- Native rail: `py -3.14 tools/viewer_host_run_logged_command.py --label slime-workbench-native-final4 --log artifacts/slime_workbench_native_final4.log -- cmd /c ui_app\build_tests_vsdevcmd.cmd` passed.
- Runtime build: `py -3.14 tools/viewer_host_run_logged_command.py --label slime-workbench-runtime-build-final3 --log artifacts/slime_workbench_runtime_build_final3.log -- cmd /c ui_app\build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Runtime viewer proof: `py -3.14 -m pytest tests/test_fractal_runtime_runtime_walk_viewer.py -q --junitxml artifacts/pytest/test_fractal_runtime_runtime_walk_viewer.junit.xml` passed `3 passed`.
- FITS contract validator: `py -3.14 tools/viewer_host_validate_fits_contract.py --contract docs/contracts/runtime_walk_fits.contract.json --out-json artifacts/validation/viewer_host_validate_fits_contract.json` passed with `ok=true`.
- Code-quality audit: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/slime_workbench_code_quality_final.json` passed baseline check.
- Plan sync / whitespace: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed, and `git diff --check` passed.
- Phase 6 red proofs: `artifacts/measured_field_negative_offset_red.log` failed on erased negative field offsets before the clamp fix, `artifacts/measured_field_fits_context_red.log` failed on missing FITS frame context in field CSVs, and `artifacts/validation/viewer_host_validate_fits_contract.red_live_controls.json` failed before live curve/polarity/clamp controls were added.
- Phase 6 native rail: `py -3.14 tools/viewer_host_run_logged_command.py --label measured-field-native-full-pass7 --log artifacts/measured_field_native_full_pass7.log -- cmd /c ui_app\build_tests_vsdevcmd.cmd` passed.
- Phase 6 runtime build: `py -3.14 tools/viewer_host_run_logged_command.py --label measured-field-runtime-build-pass10 --log artifacts/measured_field_runtime_build_pass10.log -- cmd /c ui_app\build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Phase 6 runtime viewer proof: `py -3.14 -m pytest tests/test_fractal_runtime_runtime_walk_viewer.py -q --junitxml artifacts/pytest/test_fractal_runtime_runtime_walk_viewer.junit.xml` passed `3 passed`.
- Phase 6 runtime pytest lane: `py -3.14 tools/viewer_host_runtime_pytest_lane.py` passed `68 passed`.
- Phase 6 extractor proof: `py -3.14 -m pytest tests/test_runtime_walk_extract_fits_orientation.py -q` passed `2 passed`.
- Phase 6 FITS contract validator: `py -3.14 tools/viewer_host_validate_fits_contract.py --contract docs/contracts/runtime_walk_fits.contract.json --out-json artifacts/validation/viewer_host_validate_fits_contract.json` passed with `ok=true`, including `flow_csv_fits_frame_context_wired` and `live_binding_controls_tunable`.
- Phase 6 artifact audit: `artifacts/hostile_review/measured_field_artifact_audit_final.json` passed all generated-artifact invariants.
- Phase 6 code-quality audit: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/measured_field_code_quality_final.json` passed baseline check.
- Phase 6 plan sync / whitespace: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed, and `git diff --check` passed.

## Phase 5 - Slime-Backed Binding Workbench And Field Traveler
### Scope

- Keep the normal FITS operator path FITS-only: repo-native state/request/bundle JSON remains internally synthesized plumbing.
- Use the existing schema/function descriptor/sidecar hypothesis seams as the runtime target catalog; do not create a second hardcoded parameter namespace.
- Compose FITS-driven playback as live operator baseline plus binding offsets, so sliders, pan/zoom, and fractal changes remain responsive during playback.
- Keep shipped/default warp animation forbidden. If warp is exposed later, it must be an explicit unsafe/advanced binding, not a default or hidden transport channel.
- Extend slime as a measured field-particle/traveler substrate using the sidecar measurement host path and deterministic exports.

### Red Tests First

- Add a focused runtime-walk viewer/session regression proving playback composition preserves edited live baseline controls instead of resetting to the loaded/generated base every frame.
- Add a focused runtime-walk regression proving default FITS/runtime-walk playback cannot mutate `explaino_warp_strength` even when transport channels are non-neutral.
- Add a binding workbench regression proving runtime target rows are derived from the schema/sidecar parameter catalog and unknown target paths fail.
- Add field-slime regressions for deterministic marble seeding, finite measured stepping, quantized traveler stability, adaptive sample bounds, and CSV export shape.

### Implementation Notes

- Prefer new testable modules over expanding `main.cpp`.
- The first field-flow implementation may compute tangent/gradient by GPU-batched finite differences through `SidecarMeasurementHost`; do not add decorative CPU-only spokes.
- UI controls should expose sources, targets, enabled/amount/offset/clamp/smoothing/polarity/safety, and adaptive sampling settings without requiring hand-editing JSON.
- Export `runtime_walk_flow_lines.csv` and `runtime_field_cells.csv` with enough source/binding/baseline/composed/sample metadata for downstream RTK tooling.

### Closure Gates

- The named gap for this slice is that runtime-walk playback still behaves as an authoritative snapshot player in places where it must be an offset composer over live controls, while field traveler visualization is not yet tied to measured fractal field state.
- Closure requires the focused red tests, native build-tests rail, runtime FITS-only viewer proof, code-quality audit, phased-plan sync, three hostile-review passes, checkpoint commit, validation receipt, and contract proof.

## Phase 6 - Measured FITS Field Playback Re-Closure
### Scope

- Treat the current visual result as not landed: visible playback must be driven by per-frame FITS signals plus measured field slime, not only by the pre-generated runtime-walk rail.
- Extend the checked-in extractor, native parser, runtime composition, playback UI, measured overlay, and exports under the active FITS contract.
- Keep FITS-only import, Explaino default, and no default warp binding as non-negotiable invariants.

### Red Tests First

- FITS extractor must fail red until multi-frame data produces `frame_count` and `frames[]`.
- Native bootstrap must fail red until frame timelines interpolate and live bindings compose offsets over the live baseline.
- Native viewer overlay must fail red until measured slime marble trajectories can build the visible overlay.

### Closure Gates

- Focused red tests must turn green, then the runtime viewer proof must show FITS-only playback with live controls and measured-flow artifacts.
- Contract assertions and validator JSON must cover the extractor timeline, live binding controls, measured overlay default, and no-warp default policy.
- Closure still requires three hostile-review passes, plan sync, checkpoint validation, handoff, commit, validation receipt, and contract proof.
