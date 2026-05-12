# Manual Explaino Inertial Reload Repair

## Current Phase

Closed at forward capture/state authority repair checkpoint. The historical `234919_563__explaino_inertial` archive remains an explicit strict xfail tripwire because its archived `frame.png` and `state.json` still do not replay to the same pixels, but current captures now serialize enough ExplainO runtime authority (`explaino_damping`, explicit `explaino_roots`, and `poly_coeffs_b`) for the published `D:` runtime to replay a saved state to exact pixels. Hostile audit found and repaired one additional stale-value loader bug for older states that omit the new fields.

## Phase Checklist

- [x] Phase 1 - lock this repair plan/contract, inspect the failing capture path, and add a focused RED that proves saved-state reload does not reproduce the archived render
- [x] Phase 2 - implement the minimal repair at the real serialization/render/reload owner seam
- [x] Phase 3 - publish the active `D:` runtime and prove the current capture/state path through the published runtime, while keeping the unreproduced historical archive as a strict tripwire
- [x] Phase 4 - hostile-audit the repaired state, update the closure matrix evidence, run validation, and checkpoint with an honest historical-xfail/forward-proof result

## Explicit User Asks

- [done] Treat `D:/salt-fractal/cuda_newton_fractal_clone/findings/manual_capture/2026-05-11/234919_563__explaino_inertial` as a real manual reload failure, not a process-exit success.
- [done] Turn the failure into dedicated regression coverage for state serialization/reload drift as advanced-color state grows.
- [done] Prove the repaired forward save/load path against the active published `D:` runtime; the proof is exact-pixel self-replay for a current capture state, not a false claim that the old archive was recovered.
- [done] Keep the historical failure signal alive as a strict `xfail` tripwire instead of weakening the capture-backed proof or hiding the old mismatch.
- [done] Keep the prior closure matrix honest: historical recovery is not green; the forward serialization authority repair is green.

## Presumption Loop

Presumption: the implementation is guilty until proven innocent. The archived repro command exiting 0 is not proof because the fresh frame is visually wrong. Forward TDD is mandatory: add the RED first, then repair the smallest owner seam that makes the RED and runtime repro pass. Hostile review must assume the first fix is incomplete and must explicitly check for topic/owner change reset before widening from diagnostics serialization into renderer, color pipeline, or finding-archive behavior.

The likely owner is in the diagnostics state load/save/render boundary, but that is only a starting hypothesis. The next step must distinguish four possibilities with evidence: saved JSON loses a runtime field, load mutates a field before render, the headless render path recomputes derived ExplainO state differently than the capture path, or the archived frame/state pair was produced from mismatched state. Do not patch color appearance blindly.

## Presumption Evidence

- Owner Proof: The focused native RED isolated the first concrete owner to `ui_app/src/diagnostics_state_io.cpp`: explicit split-color states parsed common params but did not assign them back to `KernelParams`.
- RED Witness: `artifacts/manual_explaino_inertial_red_native.log` failed with `Expected split-color diagnostics state to preserve common fractal params`; `artifacts/manual_explaino_inertial_red_runtime.log` failed with `multibrot_power reloaded as 2`, `22` unique colors, and `mean_abs_rgb=61.984`.
- Second Owner Proof: `explaino_damping` was a live ExplainO-Inertial runtime parameter but was absent from diagnostics save/load, so current captures could not faithfully round-trip that control.
- Forward Authority Proof: The save path now emits explicit `explaino_roots` and `poly_coeffs_b`; loaders preserve them and skip ExplainO polynomial recompute when explicit saved roots are present and no CLI override invalidates them.
- Historical Classification: Compatibility probes across damping, root spread, phase, seed, color/source signals, sample tier, max-iter/color scale, and camera perturbations did not recover the old archived frame; source-delta checks also showed render/sample/color/derived owners unchanged from capture-time `f9d26f3`. The old archive is therefore retained as a strict recovery tripwire rather than declared fixed.
- Hostile Review Result: The audit found an additional direct-load stale-value defect for legacy states that omit the new authority fields. That defect is now covered by a regression and repaired by clearing roots and secondary polynomial coefficients before optional parse.

## Proof Ledger

- Pre-slice blocker: `docs/notes/advanced_color_library_foundation_CLOSURE_MATRIX.md` records `234919_563__explaino_inertial` as a closure blocker.
- Observed failure before this slice: loading the archived `state.json` through the published runtime exited 0 but rendered a 22-color orange frame instead of the archived detailed fractal.
- RED native witness: `py -3.14 tools/viewer_host_run_logged_command.py --label "manual explaino inertial red native" --log artifacts/manual_explaino_inertial_red_native.log -- ui_app\build_tests_vsdevcmd.cmd` failed as expected on split-color common-param loader drift.
- RED runtime witness: `py -3.14 tools/viewer_host_run_logged_command.py --label "manual explaino inertial red runtime proof" --log artifacts/manual_explaino_inertial_red_runtime.log -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_manual_capture_repro.py` failed as expected with state and image mismatch.
- First repair proof: `py -3.14 tools/viewer_host_run_logged_command.py --label "manual explaino inertial native helper tests" --log artifacts/manual_explaino_inertial_native.log -- ui_app\build_tests_vsdevcmd.cmd` passed after assigning v2/v3 common fractal params outside the legacy color branch and adding `explaino_damping` save/load.
- Historical replay proof after the first repairs: `artifacts/manual_explaino_inertial_runtime_proof.log` still failed with `22` unique colors and `mean_abs_rgb=61.984`, proving the old archive was not recovered.
- Probe evidence: damping/epsilon/root/seed/color/source/fractal-type/sample-tier/max-iter/camera searches did not recover the archived frame; best candidates remained far above the capture-backed threshold and did not justify a compatibility migration.
- Capture-time source delta proof: `git diff --name-status f9d26f3..HEAD -- ui_app/src/fractal_renderer.cu ui_app/src/fractal_sample_device.inl ui_app/src/escape_time_coloring.h ui_app/src/fractal_derived_fields.cpp ui_app/src/finding_state_actions.cpp` produced no render/sample/color/derived/finding-action owner changes, while diagnostics save/load/test files did change.
- Forward authority RED witness: `artifacts/manual_explaino_capture_authority_red_native.log` failed before the explicit roots/secondary polynomial repair, proving current saves did not persist enough ExplainO runtime authority.
- Forward native proof: `py -3.14 tools/viewer_host_run_logged_command.py --label "manual explaino capture authority native retry" --log artifacts/manual_explaino_capture_authority_native_retry.log -- ui_app\build_tests_vsdevcmd.cmd` passed after saving/loading `explaino_roots` and `poly_coeffs_b` and guarding recompute.
- Forward runtime publish proof: `py -3.14 tools/viewer_host_run_logged_command.py --label "manual explaino capture authority runtime publish" --log artifacts/manual_explaino_capture_authority_runtime_publish.log -- ui_app\build_vsdevcmd.cmd` passed and staged `D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe`.
- Forward published-runtime proof: `py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_manual_capture_repro.py` passed as `1 passed, 1 xfailed`; the passing test renders from the historical state, copies the newly emitted state/frame, verifies explicit roots and `poly_coeffs_b`, then replays the emitted state to exact RGB pixels. The strict xfail is the unrecovered historical archive.
- Hostile audit repair proof: `py -3.14 tools/viewer_host_run_logged_command.py --label "manual explaino stale authority native" --log artifacts/manual_explaino_stale_authority_native.log -- ui_app\build_tests_vsdevcmd.cmd` passed after adding the legacy-state stale-authority regression and clearing absent roots/secondary coefficients.
- Final runtime proof after audit repair: `py -3.14 tools/viewer_host_run_logged_command.py --label "manual explaino stale authority runtime publish" --log artifacts/manual_explaino_stale_authority_runtime_publish.log -- ui_app\build_vsdevcmd.cmd` passed, and `py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_manual_capture_repro.py` again passed as `1 passed, 1 xfailed`.
- Code quality proof: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/code_quality_report.json` passed with score `97/100` and no critical/error findings.
- Diff sanity proof: `git diff --check` reported only the existing `HANDOFF_LOG.md` line-ending warning and no whitespace errors in the code/test diff.

## Hostile Audit

- Status: done
- Required posture: assume the implementation is wrong until the state and pixel evidence say otherwise. The old archive is not recovered; the repaired guarantee is forward capture/state authority plus exact published-runtime self-replay, with the historical mismatch retained as a strict tripwire.

## Audit Passes

- [done] Pass 1 - RED audit: the native test proved stale common params under split-color load, and the runtime test checked emitted state, unique colors, and mean RGB delta against the archived frame.
- [done] Pass 2 - first-fix audit: after common-param and `explaino_damping` repair, the historical image still failed as a 22-color orange frame, so the old capture was not claimed as fixed.
- [done] Pass 3 - forward authority audit: explicit roots and `poly_coeffs_b` were added to save/load, recompute was skipped only when saved roots are present and no ExplainO CLI override invalidates them, and the published runtime proved exact self-replay while keeping the historical archive as strict xfail.
- [done] Pass 4 - clean re-read after hostile finding: reviewed direct-load paths and found stale roots/secondary polynomial values could survive when legacy states omitted the new fields; added the regression, cleared absent authority arrays before optional parse, reran native helper tests, republished runtime, and confirmed the repaired state still passes the forward pixel proof.

## Audit Findings

- [done] Real finding: the loader gated common fractal param assignment under the legacy `coloring_mode` branch. Split-color v3 states parsed `nova_alpha`, `phoenix_p_*`, `multibrot_power`, `multibrot_power_float`, and `lambda_*`, but kept the caller's stale values instead of assigning the saved values.
- [done] Real finding: `explaino_damping` is a live schema-bound render parameter used by ExplainO-Inertial iteration, but diagnostics save/load did not serialize or restore it.
- [done] Real finding: current captures did not persist explicit ExplainO roots or `poly_coeffs_b`, so reload paths could recompute or lose runtime authority that was needed for pixel replay.
- [done] Real finding: legacy states that omit `explaino_roots` or `poly_coeffs_b` could preserve stale caller values in direct load; the loader now clears those arrays before optional parse.
- [done] Real finding: the historical `234919_563__explaino_inertial` capture remains unreproduced after the covered repairs; the old JSON omitted at least one render-affecting value, and compatibility probes did not find a defensible default or migration.
- [clean] Clean re-read: no additional real defect found in the repaired save/load/recompute guards after the stale-authority regression, native helper rerun, runtime republish, and final forward runtime pixel proof.

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

This slice closes with an honest bounded result: the old `234919_563__explaino_inertial` archive remains a strict xfail recovery tripwire, while current capture/save/load now persists enough ExplainO runtime authority to replay exact pixels through the published runtime. Future work should either find new evidence for historical recovery or leave the old archive classified as unrecoverable frame/state mismatch; do not weaken the strict xfail without actual historical pixel recovery.

## Action Hostile Review

- Action ID: action-20260512-capture-state-authority
- Suspected Failure Mode: the archive path can persist a rendered frame and a `state.json` snapshot whose saved runtime authority does not reproduce the rendered dynamics; historical evidence is `stats.last_iters_avg=19` in the archive versus about `23` on saved-state replay, with unchanged renderer/color/sample code.
- Correct Owner/Action: make diagnostics capture save/load carry the runtime authority required for current ExplainO replay, guard recompute when explicit saved roots are authoritative, and keep the old archive as an explicit strict xfail unless real recovery evidence appears.
- Proof Surface: focused native helper tests around diagnostics/finding load state authority, `ui_app/build_tests_vsdevcmd.cmd`, runtime publish to `D:`, and `tests/test_fractal_runtime_manual_capture_repro.py` proving exact forward self-replay plus strict historical xfail.
- Blocked Action: tweaking palette/grading constants, changing renderer semantics without a new RED, or weakening the historical pixel proof without adding actual historical pixel recovery.
