# Smooth-Escape Color Measurement

## Current Phase

Closed - smooth-escape color measurement inventory implemented and validated

## Phase Checklist

- [x] Phase 1 - inspect smooth-escape/color seams and add failing measurement-report tests
- [x] Phase 2 - implement deterministic headless smooth-escape color inventory reporting
- [x] Phase 3 - run representative published-runtime measurement and record the artifact
- [x] Phase 4 - hostile-audit the report, stale docs, and touched tooling seams
- [x] Phase 5 - validate, checkpoint, receipt, push, and clean-tree closeout

## Explicit User Asks

- [active] Continue the five-step plan after diagnostics capture output paths.
- [active] Treat categorized selector/view presets and camera/dive behavior as deferrable for now.
- [active] Continue forward with smooth-escape/color work before returning to SDF pack work.
- [active] Do not change product color behavior in the measurement slice.

## Scope

In scope:

- Confirm diagnostics capture output paths are already closed and sync stale planning/known-issue text if needed.
- Add a deterministic headless measurement/report rail for representative smooth-escape families.
- Record per-family frame metrics, render stats, backend/tier provenance, and current observed black/interior-like pixel fraction.
- Classify which tuning decisions need later catalog metadata or renderer/color changes, without making those changes in this slice.

Out of scope:

- Changing `escape_time_coloring.h` behavior.
- Changing `fractal_renderer.cu` behavior.
- Color Pipeline redesign or new Color Pipeline rows.
- Selector/view preset implementation.
- Camera/dive behavior.
- SDF pack parser, CUDA SDF evaluator, or SDF-native lanes.
- Physical mouse automation.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on `master` at `b9b2ae1`; it reported the prior `lens_semantics_authority` contract as active, so this slice must re-lock before product/tool mutation.
- Preflight: branch `codex/diagnostics-capture-output-paths` already exists at `e2bb164` and is an ancestor of `master`; `docs/notes/diagnostics_capture_output_paths_PHASED_PLAN.md` says diagnostics capture output paths are closed with unique default archives, `--diagnostics-out-dir`, `--out-dir`, native rails, runtime publish, and pytest proof.
- Slice start: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "smooth-escape color measurement" --profile runtime --plan docs/notes/smooth_escape_color_measurement_PHASED_PLAN.md --contract docs/contracts/smooth_escape_color_measurement.contract.json` opened this branch contract with token `ck:11b4122b`.
- RED: `py -3.14 -m pytest tests/test_smooth_escape_color_inventory.py -q --junitxml artifacts/pytest/smooth_escape_color_inventory_unit_red.junit.xml` failed because `tools/smooth_escape_color_inventory.py` did not exist.
- Unit green: `py -3.14 -m pytest tests/test_smooth_escape_color_inventory.py -q --junitxml artifacts/pytest/smooth_escape_color_inventory_unit.junit.xml` passed with 4 tests.
- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_color_measurement_runtime_publish --log artifacts/logs/smooth_escape_color_measurement_runtime_publish.log --out-json artifacts/validation/smooth_escape_color_measurement_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- cmd /c ui_app\build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Published-runtime proof: `py -3.14 -m pytest tests/test_fractal_runtime_smooth_escape_color_inventory.py -q --junitxml artifacts/pytest/smooth_escape_color_inventory_runtime.junit.xml` passed and proves the inventory tool runs against the active published runtime without mouse automation.
- Runtime measurement proof: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_color_measurement_inventory --log artifacts/logs/smooth_escape_color_measurement_inventory.log --out-json artifacts/validation/smooth_escape_color_measurement_inventory.json --heartbeat-seconds 30 --timeout-seconds 240 -- py -3.14 tools/smooth_escape_color_inventory.py --out-dir artifacts/smooth_escape_color_measurement/latest --width 96 --height 72 --runtime-lock` passed with 18 no-mouse capture cases and wrote `artifacts/smooth_escape_color_measurement/latest/inventory.json` plus `inventory.md`.
- Measurement summary:
  - all measured cases use `smooth_escape/cyclic_escape/escape_default` and the same global color tuple
  - high black-fraction cases: `multibrot`, `magnet`
  - low luma-span cases: `collatz`, `mcmullen`, `explaino_collatz_direct`
  - low unique-color cases: `nova`, `collatz`, `mcmullen`, `magnet`, `explaino_nova`, `explaino_rational_escape`, `explaino_collatz_direct`
- Stale-doc sync: `KNOWN_ISSUES.md` now marks diagnostics capture output paths and Lens SDF downsample/control truth resolved, and `docs/notes/top_five_backlog_campaign_PHASED_PLAN.md` records the user-approved continuation order.
- Contract validator: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/smooth_escape_color_measurement.contract.json --out-json artifacts/validation/smooth_escape_color_measurement_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Hostile audit validator: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/smooth_escape_color_measurement_PHASED_PLAN.md --out-json artifacts/validation/smooth_escape_color_measurement_hostile_audit.json` passed.
- Code quality: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/smooth_escape_color_measurement_code_quality.json` passed with score 95/100 and no critical/error findings.
- Diff check: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_color_measurement_diff_check --log artifacts/logs/smooth_escape_color_measurement_diff_check.log --out-json artifacts/validation/smooth_escape_color_measurement_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.
- Parent campaign plan: `docs/notes/top_five_backlog_campaign_PHASED_PLAN.md`.

## Hostile Audit

- Status: complete

## Audit Passes

- [x] Pass 1 - verified the measurement report reads actual published-runtime `--capture-diagnostic --out-dir` bundles, not synthetic or stale files.
- [x] Pass 2 - verified this slice does not change renderer/color behavior, Color Pipeline behavior, selector/view presets, camera/dive behavior, or SDF pack behavior.
- [x] Pass 3 - verified stale diagnostics-capture and Lens SDF backlog text was corrected.

## Audit Findings

- [x] Real finding - The first contract named future tool/test paths that did not exist yet, so `viewer_host_begin_work_slice.py` rejected the slice. Repaired by allowing the existing `tools` and `tests` directories in the contract before locking.
- [x] Real finding - The first runtime smoke showed relative `--out-dir` values were passed to the runtime while the subprocess cwd is the runtime directory, so captures landed outside the repo artifact tree. Repaired by resolving the inventory output directory to an absolute path before launching captures and adding a unit regression.
- [x] Real finding - The first report shape was row-only and did not classify the later tuning queue. Repaired by adding aggregate flags for shared color tuple, high black fraction, low luma span, and low unique-color cases.
- [x] Real finding - `KNOWN_ISSUES.md` still listed diagnostics capture and Lens SDF truth as open after those slices had closed. Repaired by marking both resolved with proof-plan links.
- [x] Real finding - The first receipt attempt was blocked because the viewer-first receipt gate requires a published-runtime pytest/profile command, not only the inventory tool command. Repaired by adding `tests/test_fractal_runtime_smooth_escape_color_inventory.py`, updating the contract, re-locking it, and validating the runtime pytest.
- [x] Real finding - The second receipt attempt was blocked because the full inventory command had no parseable logged-command evidence. Repaired by switching the required inventory command to `viewer_host_run_logged_command.py` with `--out-json` and rerunning it.
- [x] Clean re-read - Re-read the diff and confirmed no renderer, escape-coloring, Color Pipeline, selector/view preset, camera/dive, SDF pack, or physical mouse automation changes were introduced.

## Action Hostile Review

- Action ID: smooth-escape-color-measurement-red
- Suspected failure mode: A measurement-only slice could produce a helper-only report that does not prove the current published runtime or could drift into color/render changes before evidence exists.
- Correct owner/action: Add a deterministic report tool and runtime proof that reads real `--capture-diagnostic` bundles for representative escape-time families.
- Proof surface: focused Python tests for report parsing/metrics, runtime publish, published-runtime no-mouse inventory run, code-quality baseline, diff check.
- Blocked action: product color changes, renderer tuning, Color Pipeline redesign, selector/view preset work, camera/dive behavior, SDF pack work, and physical mouse automation.

## Notes

- This is the behavior-preserving measurement slice before any smooth-escape tuning fix slice.
- Later tuning fixes must be separate and must cite this report or a refreshed replacement report as evidence.
