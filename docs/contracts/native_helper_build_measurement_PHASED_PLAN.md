# Native Helper Build Measurement Repair

## Current Phase

Phase 4 complete - native helper measurement repair is validated and closed; FPS debounce measurement remains the next queued slice.

## Phase Checklist

- [x] Phase 0 - open a dedicated checked-in plan/contract for native helper build measurement and lock the active slice to it.
- [x] Phase 1 - measure the current native helper build path with bounded focused targets, elapsed timing, timeout behavior, and process-tree cleanup evidence.
- [x] Phase 2 - repair the smallest confirmed harness gaps: timeout/process-tree handling, missing focused targets, and deterministic timing artifacts.
- [x] Phase 3 - validate focused native helper rails and the measurement harness without deleting CUDA/native coverage or hiding the issue behind a timeout-only change.
- [x] Phase 4 - hostile-audit the tooling repair, checkpoint with receipts, push, and leave the FPS debounce measurement slice queued next.

## Explicit User Asks

- [done] Do a dedicated unit of work for the compiler/native helper timeout problem.
- [done] Do not tune or guess at the FPS debounce behavior inside this tooling slice.
- [done] Measure instead of guessing: per-command timing, whether the full helper is slow or wedged, and whether timeout cleanup leaves compiler/test processes behind.
- [done] After this slice is complete, return to the debounce/FPS issue with measurement-driven proof.
- [done] Preserve coverage; do not skip CUDA/native rails just to make the suite look green.
- [done] Keep the machine usable by avoiding repeated open/close viewer loops and physical mouse automation.

## Proof Ledger

- Starting point: branch `codex/native-helper-build-measurement` starts from clean `7154b69`, immediately after the capture/pacing runtime proof slice.
- RED/proof: `test_sample_tier_resolver` was compiled in the full helper but rejected as an unknown focused target before this slice (`artifacts/logs/native_helper_measure_missing_sample_tier_target.log`).
- Measurement: `native_measure_pacing_target` passed through `ui_app/build_tests_vsdevcmd.cmd test_viewer_render_pacing` in `2.097s` with JSON at `artifacts/validation/native_helper_measure_pacing_target.json`.
- Measurement: `native_measure_sample_tier_target` passed through the newly exposed focused target in `2.042s` with JSON at `artifacts/validation/native_helper_measure_sample_tier_target.json`.
- Measurement: `native_measure_fractal_renderer_target` passed through `ui_app/build_tests_vsdevcmd.cmd test_fractal_renderer` in `136.608s` with JSON at `artifacts/validation/native_helper_measure_fractal_renderer_target.json`.
- Measurement: full `ui_app/build_tests_vsdevcmd.cmd` passed in one shell with `All helper tests passed` in `1625.240s` (`27.1 min`) with JSON at `artifacts/validation/native_helper_full_measured.json`.
- Measurement interpretation: the timeout was not evidence of a hung suite; the full helper has 18 `nvcc` invocations, each targeting `sm_86`, `sm_120`, and `sm_121`, and repeatedly compiles `fractal_renderer.cu`, `fractal_sample_core.cu`, and `generic_sample_core.cu` from a clean object directory.
- Landed: `tools/viewer_host_run_logged_command.py` now writes optional structured JSON with elapsed time, timeout state, exit code, log path, and cleanup metadata.
- Landed: timeout handling now has an explicit child-process-tree regression: `tests/test_viewer_host_run_logged_command.py::test_timeout_kills_child_process_tree`.
- Landed: non-Windows timeout handling starts the child in a separate session and kills the process group, matching the existing Windows `taskkill /T /F` intent.
- Landed: `ui_app/build_tests_vsdevcmd.cmd test_sample_tier_resolver` is now a checked focused target.
- Landed: `tools/viewer_host_validate_native_helper_build.py` validates the focused target surface, focused measurement JSON, full-helper measurement JSON, CUDA compile profile, and the explicit FPS-debounce deferral.
- Validated: `py -3.14 -m pytest tests/test_viewer_host_run_logged_command.py -q --junitxml artifacts/pytest/test_viewer_host_run_logged_command.junit.xml` passed with `6 passed`.
- Validated: `py -3.14 tools/viewer_host_validate_native_helper_build.py --out-json artifacts/validation/native_helper_build_measurement.json` passed.
- Validated: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/native_helper_build_measurement_code_quality.json` passed with score `97/100` and baseline passed.
- Validated: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/native_helper_build_measurement.contract.json --out-json artifacts/validation/native_helper_build_measurement_contract.json` passed.
- Validated: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Validated: `git diff --check` passed with only the existing `HANDOFF_LOG.md` line-ending warning.

## Hostile Audit

- Status: complete
- Did I measure current build/helper timing before changing the harness?
- Did I prove whether the timeout is slow compilation, a hung test, child-process leakage, or command-shape overhead?
- Did I preserve CUDA/native coverage instead of deleting, skipping, or silently narrowing it?
- Did timeout handling kill the full child process tree and write evidence?
- Did the focused helper target surface improve the next repair loop without replacing the full suite?
- Did I avoid FPS debounce tuning or renderer behavior changes in this tooling slice?
- Did I leave a concrete next-step handoff back to the FPS debounce measurement repair?

## Audit Passes

- [done] Pass 1: reviewed the measured artifacts and confirmed the previous 20-minute timeout was lower than the measured full native helper duration, while the full helper passes when allowed to complete.
- [done] Pass 2: reviewed the landed diff and found avoidable test-file polish issues; repaired import order, long cleanup call formatting, and blank-line churn before validation.
- [done] Pass 3: clean re-read the repaired state, plan, contract, and closeout text; no additional real defect found, and FPS debounce remains queued next rather than modified here.

## Audit Findings

- [done] Finding repaired: `test_sample_tier_resolver` was a real focused-target gap. The helper now supports `ui_app/build_tests_vsdevcmd.cmd test_sample_tier_resolver`.
- [done] Finding repaired: logged validation previously emitted only text summaries, making timeout diagnosis harder. The wrapper now writes structured JSON timing/cleanup receipts.
- [done] Finding repaired: timeout cleanup had no unit proof. The new pytest regression spawns a child process, forces wrapper timeout, and proves the child process is gone.
- [done] Finding clarified: the full helper is slow, not wedged, on this machine. The measured pass took `1625.240s`; validation timeouts below that are harness limits, not proof of test failure.
- [done] Closure finding repaired: the contract lists the required measured commands and validators, hostile audit is complete, and final checkpoint/receipt/push handling belongs to the repo workflow after this plan update.

## Measurement Questions

- Which native helper targets consume the most wall-clock time? Current bounded witness: `test_fractal_renderer` is `136.608s`, while small CPU targets are about `2s`.
- Which CUDA sources are compiled repeatedly by the helper, and how often? Current validator reports `fractal_renderer.cu=10`, `fractal_sample_core.cu=10`, `generic_sample_core.cu=9` textual appearances and 18 total `nvcc` invocations.
- Does a timeout terminate `cmd`, `cl`, `link`, `nvcc`, and test child processes, or only the parent process? The new unit proof covers child-process-tree cleanup; full compiler-child cleanup remains best represented by the same wrapper path and Windows `taskkill /T /F` behavior.
- Which focused native targets are missing and forcing broad helper runs for small seams? `test_sample_tier_resolver` was missing and is now repaired.
- Can the harness emit stable JSON/timing artifacts suitable for receipts and future hostile review? Yes; focused and full helper measurements now write JSON under `artifacts/validation/`.

## Deferred Next Slice

- FPS debounce/pacing remains the immediate next work after this slice. It must be measurement-driven: capture live render timing, preview scale, target dimensions, interaction source, sample tier/backend, and user-visible smoothness indicators before policy changes.
- The current reported debounce state is not accepted as good: it is too aggressive when no FPS problem exists and ineffective in slow f64/high-resolution scenarios. That behavior is queued, not fixed here.
