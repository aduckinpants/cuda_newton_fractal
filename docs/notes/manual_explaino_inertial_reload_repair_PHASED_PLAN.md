# Manual Explaino Inertial Reload Repair

## Current Phase

Phase 5 checkpoint - actual capture-finding route covered, historical archive still blocked. The reopened investigation reproduced the strict historical failure, ruled out nearby color tuple, parameter, no-root-snap, alternate-runtime, and neighboring-capture recovery paths, and added published-runtime coverage for the real `--capture-finding` archive path. Current captures/finding archives are self-consistent; the old `234919_563__explaino_inertial` frame still has no recoverable runtime authority in the saved artifacts.

## Phase Checklist

- [x] Phase 1 - lock this repair plan/contract, inspect the failing capture path, and add a focused RED that proves saved-state reload does not reproduce the archived render
- [x] Phase 2 - implement the minimal repair at the real serialization/render/reload owner seam
- [x] Phase 3 - publish the active `D:` runtime and prove the current capture/state path through the published runtime, while keeping the unreproduced historical archive as a strict tripwire
- [x] Phase 4 - hostile-audit the repaired state, update the closure matrix evidence, run validation, and checkpoint with an honest historical-xfail/forward-proof result
- [x] Phase 5 - reopen historical recovery: reproduce the archive mismatch, inspect saved artifacts/provenance, identify a recoverable owner or compatibility path with evidence, and add a fresh proof for the real capture-finding path
- [x] Phase 6 - checkpoint the bounded result: current capture-finding archives are now covered by runtime self-replay proof; historical visual recovery remains blocked on absent capture-time authority rather than guessed migration

## Explicit User Asks

- [done] Treat `D:/salt-fractal/cuda_newton_fractal_clone/findings/manual_capture/2026-05-11/234919_563__explaino_inertial` as a real manual reload failure, not a process-exit success.
- [done] Turn the failure into dedicated regression coverage for state serialization/reload drift as advanced-color state grows.
- [done] Prove the repaired forward save/load path against the active published `D:` runtime; the proof is exact-pixel self-replay for a current capture state, not a false claim that the old archive was recovered.
- [done] Keep the historical failure signal alive as a strict `xfail` tripwire instead of weakening the capture-backed proof or hiding the old mismatch.
- [done] Continue the actual historical recovery work for `234919_563__explaino_inertial`; the reopened slice re-ran the failure and searched concrete recovery owners instead of stopping at the forward-proof checkpoint.
- [blocked] Recovering the old archived pixels remains blocked: the archive contains only notes, `finding.json`, `finding.md`, `frame.png`, and `state.json`; color, parameter, no-root-snap, alternate-runtime, and neighboring-capture probes did not find a defensible state migration. The next viable product choice is a viewer/archive UX path for displaying unreplayable historical frames, not a guessed renderer/color patch.

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

- Reopened RED proof: `py -3.14 tools/viewer_host_run_logged_command.py --label "manual explaino historical runxfail repro" --log artifacts/manual_explaino_historical_runxfail_repro.log -- py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_manual_capture_repro.py -k test_manual_explaino_inertial_capture_reloads_to_detailed_frame --runxfail` failed as expected with `22` unique colors and `mean_abs_rgb=61.984`.
- Artifact inventory: the historical directory contains only `field-notes.md`, `finding.json`, `finding.md`, `frame.png`, and `state.json`; the archived PNG has no embedded metadata.
- Neighboring-capture check: same-day manual captures do not provide a matching state/frame pair for `234919_563__explaino_inertial`.
- Color recovery probe: `artifacts/manual_explaino_historical_color_probe_v2/report.json` shows the saved `smooth_escape`/`cyclic_escape` tuple is still the best tested color tuple at `mean_abs_rgb=61.984`; alternate source/palette tuples were worse or invalid.
- Parameter recovery probe: `artifacts/manual_explaino_historical_param_search/report.json` tested bounded seed/phase/damping/root/color-shape/grading parameters; the best candidate was `mean_abs_rgb=55.755`, still far above the `<35` proof threshold.
- No-root-snap probe: `artifacts/manual_explaino_no_root_snap_probe/report.json` showed preserving saved polynomial coefficients with explicit no-root authority still renders the same `mean_abs_rgb=61.984` frame.
- Alternate-runtime probe: available `fractal_ui_probe.exe` rendered the historical state at `mean_abs_rgb=58.760`, closer but still not a recovery; no capture-time executable is preserved in the runtime directory.
- Actual capture-finding route proof: a direct `--capture-finding` run from the historical state created a fresh archive with explicit roots and `poly_coeffs_b`; replaying that archive produced `mean_abs_rgb=0.005` after 256px downsample.
- Regression proof added: `tests/test_fractal_runtime_manual_capture_repro.py` now covers the actual `--capture-finding` archive route in `test_current_explaino_inertial_capture_finding_archive_replays_its_pixels`.
- Focused runtime validation: `py -3.14 tools/viewer_host_runtime_pytest_lane.py tests/test_fractal_runtime_manual_capture_repro.py` passed as `2 passed, 1 xfailed`; the xfail remains the unrecovered historical frame/state mismatch.

## Hostile Audit

- Status: done
- Required posture: assume the forward-proof checkpoint missed the actual operator ask. The reopened audit targeted the old archive again, rejected unproven compatibility guesses, and added proof for the real capture-finding route while keeping the historical archive unrecovered.

## Audit Passes

- [done] Pass 1 - RED audit: the native test proved stale common params under split-color load, and the runtime test checked emitted state, unique colors, and mean RGB delta against the archived frame.
- [done] Pass 2 - first-fix audit: after common-param and `explaino_damping` repair, the historical image still failed as a 22-color orange frame, so the old capture was not claimed as fixed.
- [done] Pass 3 - forward authority audit: explicit roots and `poly_coeffs_b` were added to save/load, recompute was skipped only when saved roots are present and no ExplainO CLI override invalidates them, and the published runtime proved exact self-replay while keeping the historical archive as strict xfail.
- [done] Pass 4 - clean re-read after hostile finding: reviewed direct-load paths and found stale roots/secondary polynomial values could survive when legacy states omitted the new fields; added the regression, cleared absent authority arrays before optional parse, reran native helper tests, republished runtime, and confirmed the repaired state still passes the forward pixel proof.
- [done] Pass 5 - reopened historical audit: re-ran the archived capture mismatch, inventoried capture artifacts/provenance, checked neighboring captures and alternate executables, and ran bounded compatibility probes without finding a recoverable historical state.
- [done] Pass 6 - clean re-read of the product route: proved the current `--capture-finding` archive path creates self-consistent state/frame artifacts and added runtime regression coverage for that route.

## Audit Findings

- [done] Real finding: the loader gated common fractal param assignment under the legacy `coloring_mode` branch. Split-color v3 states parsed `nova_alpha`, `phoenix_p_*`, `multibrot_power`, `multibrot_power_float`, and `lambda_*`, but kept the caller's stale values instead of assigning the saved values.
- [done] Real finding: `explaino_damping` is a live schema-bound render parameter used by ExplainO-Inertial iteration, but diagnostics save/load did not serialize or restore it.
- [done] Real finding: current captures did not persist explicit ExplainO roots or `poly_coeffs_b`, so reload paths could recompute or lose runtime authority that was needed for pixel replay.
- [done] Real finding: legacy states that omit `explaino_roots` or `poly_coeffs_b` could preserve stale caller values in direct load; the loader now clears those arrays before optional parse.
- [blocked] Historical finding: the old `234919_563__explaino_inertial` archive still fails visual reload, and the saved artifacts do not contain enough authority to reconstruct the archived frame through a defensible state migration.
- [clean] Clean re-read: the current capture-diagnostic and capture-finding routes now create replayable state/frame pairs; no further recoverable owner was found for the historical archive after the reopened probes.

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

This follow-up closes as a bounded blocker, not a visual recovery claim. Current `--capture-diagnostic` and `--capture-finding` paths are covered by published-runtime self-replay tests, while the old `234919_563__explaino_inertial` frame remains a strict xfail because its saved artifacts do not contain recoverable runtime authority. The next product-level option is an explicit historical-frame viewing/fallback UX for unreplayable archives, not another guessed renderer or palette migration.

## Action Hostile Review

- Action ID: action-20260512-historical-reload-recovery
- Suspected Failure Mode: the forward replay fix made new captures self-consistent but left the original archived frame/state mismatch unrecovered; the missing owner might have been archived sidecar/provenance, derived ExplainO root reconstruction, load-time defaults, capture-time executable drift, or capture-finding archive behavior.
- Correct Owner/Action: reproduce the archived mismatch, inventory every artifact in the capture directory, trace runtime-field deltas, probe recoverable compatibility candidates, and lock the real current capture-finding route with published-runtime replay proof.
- Proof Surface: `tests/test_fractal_runtime_manual_capture_repro.py` keeps the historical case strict xfail and now also proves current diagnostic and capture-finding archives replay through the active published `D:` runtime.
- Blocked Action: declaring the old pixels recovered without the historical pixel assertion passing, weakening/removing the historical xfail, or changing palette/renderer behavior from a candidate that remains above the proof threshold.
