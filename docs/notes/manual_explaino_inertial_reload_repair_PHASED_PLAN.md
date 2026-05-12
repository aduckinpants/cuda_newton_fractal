# Manual Explaino Inertial Reload Repair

## Current Phase

Phase 3 - blocked published-runtime reproduction. The split-color common-param loader bug is fixed, `explaino_damping` now saves/loads for forward captures, native helper tests are green, and the active `D:` runtime has been republished. The historical `234919_563__explaino_inertial` state still reloads as a 22-color orange frame, so foundation closure remains blocked by apparent historical save-time data loss.

## Phase Checklist

- [x] Phase 1 - lock this repair plan/contract, inspect the failing capture path, and add a focused RED that proves saved-state reload does not reproduce the archived render
- [x] Phase 2 - implement the minimal repair at the real serialization/render/reload owner seam
- [ ] Phase 3 - publish the active `D:` runtime and prove the failing capture re-renders from saved state through the published runtime
- [ ] Phase 4 - hostile-audit the repaired state, update the closure matrix, write receipts if green, checkpoint the blocked state if not, and leave the worktree clean

## Explicit User Asks

- [open] Treat `D:/salt-fractal/cuda_newton_fractal_clone/findings/manual_capture/2026-05-11/234919_563__explaino_inertial` as a real manual reload failure, not a process-exit success.
- [done] Turn the failure into a dedicated regression for state serialization/reload drift as advanced-color state grows.
- [open] Prove the fix against the active published `D:` runtime, because viewer-visible reproduction is the point of this path.
- [done] Keep the prior closure matrix blocked until this capture-backed reproduction proof is green.

## Presumption Loop

Presumption: the implementation is guilty until proven innocent. The archived repro command exiting 0 is not proof because the fresh frame is visually wrong. Forward TDD is mandatory: add the RED first, then repair the smallest owner seam that makes the RED and runtime repro pass. Hostile review must assume the first fix is incomplete and must explicitly check for topic/owner change reset before widening from diagnostics serialization into renderer, color pipeline, or finding-archive behavior.

The likely owner is in the diagnostics state load/save/render boundary, but that is only a starting hypothesis. The next step must distinguish four possibilities with evidence: saved JSON loses a runtime field, load mutates a field before render, the headless render path recomputes derived ExplainO state differently than the capture path, or the archived frame/state pair was produced from mismatched state. Do not patch color appearance blindly.

## Presumption Evidence

- Owner Proof: The focused native RED isolates the first concrete owner to `ui_app/src/diagnostics_state_io.cpp`: explicit split-color states parse common params but do not assign them back to `KernelParams`.
- RED Witness: `artifacts/manual_explaino_inertial_red_native.log` fails with `Expected split-color diagnostics state to preserve common fractal params`; `artifacts/manual_explaino_inertial_red_runtime.log` fails with `multibrot_power reloaded as 2`, `22` unique colors, and `mean_abs_rgb=61.984`.
- Fix Proof: Partial; `artifacts/manual_explaino_inertial_native.log` is green after the common-param and `explaino_damping` fixes, but `artifacts/manual_explaino_inertial_runtime_proof.log` still fails on unique colors and mean RGB delta.
- Hostile Review Result: The second real defect was missing `explaino_damping` persistence. After repairing it, damping/spread/color/seed probes still did not recover the old archived image, so the old capture remains blocked instead of being claimed as fixed.

## Proof Ledger

- Pre-slice blocker: `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md` now records `234919_563__explaino_inertial` as a closure blocker.
- Observed failure before this slice: `D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.cmd --load-state-json D:/salt-fractal/cuda_newton_fractal_clone/findings/manual_capture/2026-05-11/234919_563__explaino_inertial/state.json --capture-diagnostic` exits 0, but the fresh render is a solid orange frame instead of the archived detailed fractal.
- Observed state drift before this slice: the freshly emitted diagnostics state mostly matches the archived state, with `params.multibrot_power` observed changing from `3` to `2`; this may be a symptom or a red herring and must be proven before fixing.
- Stale-artifact note: `runtime/diagnostics/last/capture_finding_error.txt` is from April and is not accepted as evidence for the current failure.
- RED native witness: `py -3.14 tools/viewer_host_run_logged_command.py --label "manual explaino inertial red native" --log artifacts/manual_explaino_inertial_red_native.log -- ui_app\build_tests_vsdevcmd.cmd` failed as expected on the split-color common-param loader regression.
- RED runtime witness: `py -3.14 tools/viewer_host_run_logged_command.py --label "manual explaino inertial red runtime proof" --log artifacts/manual_explaino_inertial_red_runtime.log -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_manual_capture_repro.py` failed as expected with state and image mismatch.
- First repair proof: `py -3.14 tools/viewer_host_run_logged_command.py --label "manual explaino inertial native helper tests" --log artifacts/manual_explaino_inertial_native.log -- ui_app\build_tests_vsdevcmd.cmd` passed after assigning v2/v3 common fractal params outside the legacy color branch.
- Remaining runtime blocker: `py -3.14 tools/viewer_host_run_logged_command.py --label "manual explaino inertial runtime proof" --log artifacts/manual_explaino_inertial_runtime_proof.log -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_manual_capture_repro.py` still fails after publish with `22` unique colors and `mean_abs_rgb=61.984`; emitted JSON now matches the archived state for checked fields.
- Second repair proof: `explaino_damping` now serializes in `ui_app/src/diagnostics_capture.cpp`, reloads in `ui_app/src/diagnostics_state_io.cpp`, and is covered by `ui_app/tests/test_diagnostics_state_io.cpp`; `artifacts/manual_explaino_inertial_native.log` passed after that repair.
- Runtime publish proof: `artifacts/manual_explaino_inertial_runtime_publish.log` passed after the `explaino_damping` repair and republished `D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe`.
- Post-damping blocker: `artifacts/manual_explaino_inertial_runtime_proof.log` still fails the historical capture with `22` unique colors and `mean_abs_rgb=61.984`; the old `state.json` has no saved `explaino_damping`, so that value cannot be recovered from the JSON.
- Probe evidence: formula/color tuple probes, damping probe best `mean_abs_rgb=58.497`, spread+damping probe best `mean_abs_rgb=57.398`, color-param probe best `mean_abs_rgb=57.415`, and seed/tween/phase probe best `mean_abs_rgb=56.941`; none reproduced the archived 4096x4096 frame.

## Hostile Audit

- Status: complete
- Required posture: assume the first apparent fix only makes JSON fields match while the viewer still renders the wrong image. Audit both state and pixels, and keep the closure blocker open unless the published runtime reproduces the archived pixels.

## Audit Passes

- [done] Pass 1 - RED audit: the native test proves stale common params under split-color load, and the runtime test also checks emitted state, unique colors, and mean RGB delta against the archived frame.
- [done] Pass 2 - first-fix audit: re-ran the published-runtime manual capture proof after publish; the common-param state drift disappeared, but the image still failed as a 22-color orange frame.
- [done] Pass 3 - re-read the repaired state after adding `explaino_damping`, republished the runtime, and confirmed the repaired state still does not reproduce the historical frame; no additional real issue found in the covered save/load seams, and the unresolved evidence is now recorded as a closure blocker rather than closure proof.

## Audit Findings

- [done] Real finding: the loader gates common fractal param assignment under the legacy `coloring_mode` branch. Split-color v3 states parse `nova_alpha`, `phoenix_p_*`, `multibrot_power`, `multibrot_power_float`, and `lambda_*`, but keep the caller's stale values instead of assigning the saved values.
- [done] Real finding: `explaino_damping` is a live schema-bound render parameter used by ExplainO-Inertial iteration, but diagnostics save/load did not serialize or restore it.
- [done] Real finding: the historical `234919_563__explaino_inertial` capture remains unreproduced after the covered repairs; the old JSON omitted at least one render-affecting value, and compatibility probes did not find a defensible default or migration.
- [clean] Clean re-read: no additional real defect found in the implemented save/load coverage after the post-damping runtime proof and probes; closure remains blocked because the published runtime has not reproduced the old archived pixels.

## Notes

- Expected owner surfaces for this repair slice:
  - `ui_app/src/diagnostics_state_io.cpp`
  - `ui_app/src/diagnostics_capture.cpp`
  - `ui_app/src/main.cpp`
  - `ui_app/src/finding_capture_state.cpp`
  - `ui_app/src/fractal_derived_fields.cpp`
  - `ui_app/src/fractal_types.h`
  - `ui_app/tests/test_diagnostics_state_io.cpp`
  - `ui_app/tests/test_finding_archive_actions.cpp`
  - `tests/test_fractal_runtime_manual_capture_repro.py`
- Non-goals:
  - do not implement Source signal composition
  - do not promote deferred Grading rows
  - do not claim UI/UX behavior from a headless test
  - do not commit the full 4096x4096 frame as a test fixture unless a smaller deterministic witness cannot prove the bug

## Resume Point

The forward serialization defects found in this slice are repaired, but the original `234919_563__explaino_inertial` capture remains a viewer-visible blocker. Continue only by either finding a new recoverable missing owner with evidence, or explicitly reclassifying this historical capture as unrecoverable data loss and adding a new complete forward capture proof; do not weaken the current runtime regression silently.

## Action Hostile Review

- Action ID: action-20260512-inertial-render-state-completeness
- Suspected Failure Mode: fixing only the visible `multibrot_power` drift leaves a second saved-state data-loss bug where render-affecting ExplainO params are absent from diagnostics JSON.
- Correct Owner/Action: serialize and restore `explaino_damping` through diagnostics state IO, add focused native coverage, and then test whether the historical manual capture can be reproduced or must be recorded as unrecoverable data loss.
- Proof Surface: native diagnostics-state coverage, runtime publish, published-runtime manual capture repro, and explicit closure-matrix status based on that proof.
- Blocked Action: changing renderer/color math to chase the image before the missing state field is covered and tested.
