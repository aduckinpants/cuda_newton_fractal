# Smooth-Escape Collatz Luma Tuning

## Current Phase

Closed - smooth-escape Collatz luma tuning implemented, validated, hostile-audited, checkpointed, receipted, pushed, and merged to master.

## Phase Checklist

- [x] Phase 1 - open the checked-in plan/contract and lock the active slice
- [x] Phase 2 - add RED native/runtime proof for Collatz-family low-luma and fast-escape black output
- [x] Phase 3 - repair only Collatz-family smooth-escape signal tuning
- [x] Phase 4 - publish runtime and prove the published viewer output improves without mouse automation
- [x] Phase 5 - hostile-audit the touched color seams and docs
- [x] Phase 6 - validate, checkpoint, receipts, push, merge-back, and clean-tree closeout

## Explicit User Asks

- [done] Continue from the remaining smooth/color tuning rows after the interior-tone fix.
- [done] Start with measured, low-risk, tangible improvement rather than broad palette redesign.
- [done] Preserve Color Pipeline controls and current non-Collatz smooth-escape behavior.
- [done] Use no-mouse runtime proof and avoid live viewer open/close churn.

## Scope

In scope:

- Tune the smooth-escape signal for the fast-escaping Collatz map family:
  - `collatz`
  - `explaino_collatz_direct`
- Preserve existing user-owned `color_smooth_escape_scale` and `color_smooth_escape_bias` behavior on those lanes.
- Preserve existing Mandelbrot/Multibrot smooth-escape samples and non-Collatz runtime behavior.
- Add native proof for Collatz-family fast-escape sample luma and live scale sensitivity.
- Add published-runtime proof that Collatz no longer trips the low-luma inventory flag and Collatz-family fast-escape black fraction is reduced.

Out of scope:

- General per-family catalog metadata.
- Histogram equalization or adaptive normalization.
- Palette redesign or new Color Pipeline rows.
- Low-unique tuning for Nova, McMullen, Magnet, Rational Escape, or other non-Collatz families.
- Selector/view preset work.
- Camera/dive behavior.
- SDF pack parser or CUDA SDF evaluator work.
- Physical mouse automation.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on `master` at `fca5f64`; the prior `smooth_escape_interior_tone` contract was active and must be replaced before behavior mutation.
- Branch: `codex/smooth-escape-collatz-luma` from clean `master` at `fca5f64`.
- Evidence basis: `artifacts/smooth_escape_interior_tone/latest/inventory.md` measured the current published runtime after the interior-tone fix. It reports `collatz` with luma range `0.0-61.4`, black fraction `0.123`, and low-luma classification. It also reports `explaino_collatz_direct` with black fraction `0.125`.
- Code seam: `ui_app/src/escape_time_coloring.h` maps smooth escape through `ComputeEscapeTimeNu(...) * 0.025`, so very fast escaping Collatz samples stay near the dark heatmap stop.
- RED native proof: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_collatz_luma_native_red --log artifacts/logs/smooth_escape_collatz_luma_native_red.log --out-json artifacts/validation/smooth_escape_collatz_luma_native_red.json --heartbeat-seconds 30 --timeout-seconds 600 -- cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner` failed on `Collatz fast-escape smooth coloring should not stay trapped in the dark heatmap stop`.
- RED runtime proof: `py -3.14 -m pytest tests/test_fractal_runtime_smooth_escape_collatz_luma.py -q --junitxml artifacts/pytest/smooth_escape_collatz_luma_runtime_red.junit.xml` failed with `collatz` luma span `55.317`.
- First native green: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_collatz_luma_native_green --log artifacts/logs/smooth_escape_collatz_luma_native_green.log --out-json artifacts/validation/smooth_escape_collatz_luma_native_green.json --heartbeat-seconds 30 --timeout-seconds 600 -- cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner` passed after Collatz-family smooth-escape signal scale/bias was added.
- Runtime proof after first tune still failed: `py -3.14 -m pytest tests/test_fractal_runtime_smooth_escape_collatz_luma.py -q --junitxml artifacts/pytest/smooth_escape_collatz_luma_runtime.junit.xml` reported `collatz` black fraction `0.1284722222222222`, proving the black-pixel row was not only a dark-band problem.
- Debug inventory after first tune: `py -3.14 tools/smooth_escape_color_inventory.py --out-dir artifacts/smooth_escape_collatz_luma/debug_after_first_tune --width 96 --height 72 --fractal-type collatz --fractal-type explaino_collatz_direct --runtime-lock` showed luma improved but both Collatz-family black fractions remained about `0.123-0.125`.
- Overflow RED: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_collatz_luma_overflow_red --log artifacts/logs/smooth_escape_collatz_luma_overflow_red.log --out-json artifacts/validation/smooth_escape_collatz_luma_overflow_red.json --heartbeat-seconds 30 --timeout-seconds 600 -- cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner` failed with process exit `-1073741819` after adding an infinity-valued Collatz escape sample.
- Second native green: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_collatz_luma_native_green2 --log artifacts/logs/smooth_escape_collatz_luma_native_green2.log --out-json artifacts/validation/smooth_escape_collatz_luma_native_green2.json --heartbeat-seconds 30 --timeout-seconds 600 -- cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner` passed after Collatz-family nonfinite smooth-escape magnitudes were clamped before `ComputeEscapeTimeNu(...)`.
- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_collatz_luma_runtime_publish --log artifacts/logs/smooth_escape_collatz_luma_runtime_publish.log --out-json artifacts/validation/smooth_escape_collatz_luma_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- cmd /c ui_app\build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Published-runtime proof: `py -3.14 -m pytest tests/test_fractal_runtime_smooth_escape_collatz_luma.py -q --junitxml artifacts/pytest/smooth_escape_collatz_luma_runtime.junit.xml` passed.
- Full inventory proof: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_collatz_luma_inventory --log artifacts/logs/smooth_escape_collatz_luma_inventory.log --out-json artifacts/validation/smooth_escape_collatz_luma_inventory.json --heartbeat-seconds 30 --timeout-seconds 240 -- py -3.14 tools/smooth_escape_color_inventory.py --out-dir artifacts/smooth_escape_collatz_luma/latest --width 96 --height 72 --runtime-lock` passed.
- Post-fix inventory summary: high black-fraction cases are `none`, low luma-span cases are `none`, `collatz` black fraction is `0.000` with luma range `4.6-137.6`, and `explaino_collatz_direct` black fraction is `0.000` with luma range `4.6-138.2`.
- Hostile hardening: final diff review found Collatz normal smooth-escape scale was covered, but Source-row smooth scale was not directly covered on the tuned family path. Repaired by adding a native Source-row scale sensitivity assertion.
- Final native rail: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_collatz_luma_native_final --log artifacts/logs/smooth_escape_collatz_luma_native_final.log --out-json artifacts/validation/smooth_escape_collatz_luma_native_final.json --heartbeat-seconds 30 --timeout-seconds 600 -- cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner` passed.
- Contract validator: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/smooth_escape_collatz_luma.contract.json --out-json artifacts/validation/smooth_escape_collatz_luma_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Code quality: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/smooth_escape_collatz_luma_code_quality.json` passed with score 95/100 and no baseline regression.
- Diff check: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_collatz_luma_diff_check --log artifacts/logs/smooth_escape_collatz_luma_diff_check.log --out-json artifacts/validation/smooth_escape_collatz_luma_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.

## Hostile Audit

- Status: complete

## Audit Passes

- [x] Pass 1 - verified the fix is limited to Collatz-family smooth-escape signal tuning and nonfinite fallback.
- [x] Pass 2 - verified non-Collatz smooth-escape samples and normal/Source-row Color Pipeline controls remain live.
- [x] Pass 3 - verified the runtime proof uses the published runtime without physical mouse automation and docs keep unrelated low-unique rows open.

## Audit Findings

- [x] Real finding - The first Collatz-family signal scale/bias tune fixed the luma span but left fast-escape black fraction around `0.123-0.125`. Repaired by adding a nonfinite overflow RED and clamping Collatz-family nonfinite smooth magnitudes before `ComputeEscapeTimeNu(...)`.
- [x] Real finding - The first native owner-field coverage only tested the legacy smooth-escape scale field. Repaired by adding a Source-row smooth scale sensitivity assertion on the tuned Collatz path.
- [x] Clean re-read - Re-read the final diff and confirmed the slice does not add palette rows, histogram/adaptive normalization, selector/view preset work, camera/dive behavior, SDF work, or physical mouse automation.

## Action Hostile Review

- Action ID: smooth-escape-collatz-luma-red
- Suspected failure mode: A luma fix could become a broad palette rewrite, break non-Collatz smooth-escape expectations, or prove only helper code while the published runtime remains low-luma.
- Correct owner/action: Add native and published-runtime no-mouse proof around Collatz-family smooth-escape luma, then make the smallest signal-tuning repair.
- Proof surface: focused native color tests, runtime publish, published-runtime inventory pytest, focused inventory artifact, plan/contract validators, code-quality baseline, diff check.
- Blocked action: palette redesign, catalog metadata redesign, non-Collatz low-unique tuning, selector/view preset work, camera/dive behavior, SDF work, and physical mouse automation.

## Notes

- This follows `docs/notes/smooth_escape_interior_tone_PHASED_PLAN.md`.
- Non-Collatz low-unique rows remain later tuning work unless this narrow fix directly changes their measured classification.
