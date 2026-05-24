# Smooth-Escape Low-Unique Tuning

## Current Phase

Closed - smooth-escape low-unique tuning implemented, validated, hostile-audited, and checkpointed on the feature branch.

## Phase Checklist

- [x] Phase 1 - open the checked-in plan/contract and lock the active slice
- [x] Phase 2 - add RED runtime/native proof for the remaining measured low-unique rows
- [x] Phase 3 - repair only scoped smooth-escape signal tuning for the measured rows
- [x] Phase 4 - publish runtime and prove the published viewer inventory clears the low-unique rows without mouse automation
- [x] Phase 5 - hostile-audit the touched color seams, Color Pipeline owner controls, and docs
- [x] Phase 6 - validate, checkpoint, receipts, push, merge-back, and clean-tree closeout

## Explicit User Asks

- [done] Continue the next smooth-escape/color cleanup item from the top-five backlog campaign.
- [done] Work the remaining tangible smooth-escape defects instead of starting unrelated SDF, selector, camera, or engine work.
- [done] Preserve Color Pipeline behavior and live smooth-escape owner controls.
- [done] Use no-mouse proof and avoid physical cursor automation.
- [done] Keep this bounded to color tuning; do not turn it into a palette redesign or histogram/adaptive-normalization project.

## Scope

In scope:

- Tune only the remaining measured smooth-escape low-unique rows from the last inventory:
  - `nova`
  - `mcmullen`
  - `magnet`
  - `explaino_nova`
  - `explaino_rational_escape`
- Preserve the closed Collatz-family luma/black-pixel repair.
- Preserve existing Mandelbrot/Multibrot and non-target smooth-escape samples.
- Preserve user-owned `color_smooth_escape_scale`, `color_smooth_escape_bias`, and Source-row smooth-escape scale/bias behavior.
- Add runtime inventory proof that the named rows no longer classify as low unique under the published runtime.
- Keep `SampleFractalPoints(...)` and formula behavior unchanged.

Out of scope:

- Palette redesign.
- Histogram equalization or adaptive normalization.
- New Color Pipeline rows.
- Selector/view preset work.
- Camera/dive behavior.
- SDF pack parser or CUDA SDF evaluator work.
- New fractal types or formula/control changes.
- Physical mouse automation.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on `master` at `89d68ef`; it reported the prior `smooth_escape_collatz_luma` contract as active, so this slice must replace that contract before behavior mutation.
- Evidence basis: `artifacts/smooth_escape_collatz_luma/latest/inventory.md` reports high black-fraction cases `none`, low luma-span cases `none`, and low unique-color cases `nova, mcmullen, magnet, explaino_nova, explaino_rational_escape`.
- Branch: `codex/smooth-escape-low-unique` from clean `master` at `89d68ef`.
- Slice start: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "smooth-escape low-unique tuning" --profile runtime --plan docs/notes/smooth_escape_low_unique_PHASED_PLAN.md --contract docs/contracts/smooth_escape_low_unique.contract.json` opened checkpoint token `ck:2f8a5c89`.
- RED native proof: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_low_unique_native_red --log artifacts/logs/smooth_escape_low_unique_native_red.log --out-json artifacts/validation/smooth_escape_low_unique_native_red.json --heartbeat-seconds 30 --timeout-seconds 600 -- cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner` failed on `Nova smooth-escape tuning should lift the low-unique signal above the shared baseline`.
- RED runtime proof: `py -3.14 -m pytest tests/test_fractal_runtime_smooth_escape_low_unique.py -q --junitxml artifacts/pytest/smooth_escape_low_unique_runtime_red.junit.xml` failed with all five target rows still in `low_unique_color_cases`.
- First native green: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_low_unique_native_green --log artifacts/logs/smooth_escape_low_unique_native_green.log --out-json artifacts/validation/smooth_escape_low_unique_native_green.json --heartbeat-seconds 30 --timeout-seconds 600 -- cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner` passed after scoped scale/bias tuning for target rows.
- First runtime probe after publish still failed: `py -3.14 -m pytest tests/test_fractal_runtime_smooth_escape_low_unique.py -q --junitxml artifacts/pytest/smooth_escape_low_unique_runtime_probe.junit.xml` reported `magnet` and `explaino_rational_escape` below threshold at `48x36`.
- Debug inventory after first tune: `artifacts/smooth_escape_low_unique/debug_after_first_tune/inventory.md` showed `nova`, `mcmullen`, `explaino_nova`, and `explaino_rational_escape` clear at `96x72`, with `magnet` still low unique.
- Magnet angular-detail native proof: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_low_unique_native_magnet_detail --log artifacts/logs/smooth_escape_low_unique_native_magnet_detail.log --out-json artifacts/validation/smooth_escape_low_unique_native_magnet_detail.json --heartbeat-seconds 30 --timeout-seconds 600 -- cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner` passed after Magnet smooth-escape added raw angular detail for collapsed iteration/magnitude cases.
- Rational angular-detail native proof: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_low_unique_native_rational_detail --log artifacts/logs/smooth_escape_low_unique_native_rational_detail.log --out-json artifacts/validation/smooth_escape_low_unique_native_rational_detail.json --heartbeat-seconds 30 --timeout-seconds 600 -- cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner` passed after `explaino_rational_escape` also gained raw angular detail.
- Runtime proof after angular detail: `py -3.14 -m pytest tests/test_fractal_runtime_smooth_escape_low_unique.py -q --junitxml artifacts/pytest/smooth_escape_low_unique_runtime_probe3.junit.xml` passed.
- Hostile owner-control finding: diff review found the first raw angular-detail implementation accidentally used phase-signal owner fields. Repaired by normalizing raw angle directly and adding a native regression that phase controls do not own Magnet smooth-escape detail.
- Owner-control native proof: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_low_unique_native_owner_fix --log artifacts/logs/smooth_escape_low_unique_native_owner_fix.log --out-json artifacts/validation/smooth_escape_low_unique_native_owner_fix.json --heartbeat-seconds 30 --timeout-seconds 600 -- cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner` passed.
- Owner-control runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_low_unique_runtime_publish_owner_fix --log artifacts/logs/smooth_escape_low_unique_runtime_publish_owner_fix.log --out-json artifacts/validation/smooth_escape_low_unique_runtime_publish_owner_fix.json --heartbeat-seconds 30 --timeout-seconds 900 -- cmd /c ui_app\build_vsdevcmd.cmd` passed.
- Owner-control runtime proof: `py -3.14 -m pytest tests/test_fractal_runtime_smooth_escape_low_unique.py -q --junitxml artifacts/pytest/smooth_escape_low_unique_runtime_owner_fix.junit.xml` passed.
- Full inventory proof after owner fix: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_low_unique_inventory_owner_fix --log artifacts/logs/smooth_escape_low_unique_inventory_owner_fix.log --out-json artifacts/validation/smooth_escape_low_unique_inventory_owner_fix.json --heartbeat-seconds 30 --timeout-seconds 240 -- py -3.14 tools/smooth_escape_color_inventory.py --out-dir artifacts/smooth_escape_low_unique/latest --width 96 --height 72 --runtime-lock` passed. The report says high black-fraction cases `none`, low luma-span cases `none`, and low unique-color cases `none`.
- Final native rail: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_low_unique_native_final --log artifacts/logs/smooth_escape_low_unique_native_final.log --out-json artifacts/validation/smooth_escape_low_unique_native_final.json --heartbeat-seconds 30 --timeout-seconds 600 -- cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner` passed.
- Final runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_low_unique_runtime_publish --log artifacts/logs/smooth_escape_low_unique_runtime_publish.log --out-json artifacts/validation/smooth_escape_low_unique_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- cmd /c ui_app\build_vsdevcmd.cmd` passed.
- Final published-runtime proof: `py -3.14 -m pytest tests/test_fractal_runtime_smooth_escape_low_unique.py -q --junitxml artifacts/pytest/smooth_escape_low_unique_runtime.junit.xml` passed.
- Final full inventory: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_low_unique_inventory --log artifacts/logs/smooth_escape_low_unique_inventory.log --out-json artifacts/validation/smooth_escape_low_unique_inventory.json --heartbeat-seconds 30 --timeout-seconds 240 -- py -3.14 tools/smooth_escape_color_inventory.py --out-dir artifacts/smooth_escape_low_unique/latest --width 96 --height 72 --runtime-lock` passed. Final `96x72` inventory reports `none` for high black-fraction, low luma-span, and low unique-color cases across all 18 measured families; target unique counts are Nova `357`, McMullen `403`, Magnet `548`, Explaino-Nova `391`, and Explaino-Rational-Escape `565`.
- Contract validator: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/smooth_escape_low_unique.contract.json --out-json artifacts/validation/smooth_escape_low_unique_contract.json` passed.
- Code quality: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/smooth_escape_low_unique_code_quality.json` passed with score `95/100` and no baseline regression.

## Hostile Audit

- Status: complete

## Audit Passes

- [x] Pass 1 - verify the fix is limited to the measured low-unique rows and does not retune all smooth-escape families blindly.
- [x] Pass 2 - verify Color Pipeline owner fields and Source-row smooth-escape controls still affect target families.
- [x] Pass 3 - verify published-runtime proof uses no physical mouse automation and docs keep larger visual tuning work deferred.

## Audit Findings

- [x] Real finding - The first Magnet/Rational angle-detail implementation reused phase-signal offset/wrap owner fields, which would make phase controls affect smooth_escape on tuned lanes. Repaired by normalizing raw angle directly and adding a native regression that changing phase controls does not change Magnet smooth_escape.
- [x] Real finding - Scale/bias alone cleared Nova, McMullen, and Explaino-Nova but not Magnet at runtime. Repaired with scoped raw angular detail for Magnet and covered by native angle-detail and no-mouse inventory proof.
- [x] Clean re-read - Re-read the final diff and confirmed the slice does not change fractal formulas, add controls, add new Color Pipeline rows, change the inventory threshold, implement selector/camera/SDF work, or use physical mouse automation.

## Action Hostile Review

- Action ID: smooth-escape-low-unique-red
- Suspected failure mode: A uniqueness fix could become a broad palette/pipeline rewrite, hide low-unique rows by changing the measurement tool, or pass only helper proof while the published runtime still has low color variety.
- Correct owner/action: Add native and published-runtime no-mouse proof around the measured rows, then make the smallest scoped smooth-escape signal-tuning repair.
- Proof surface: focused native color tests, runtime publish, published-runtime inventory pytest, focused inventory artifact, plan/contract validators, hostile-audit validator, code-quality baseline, diff check.
- Blocked action: palette redesign, histogram/adaptive normalization, new Color Pipeline rows, selector/view preset work, camera/dive behavior, SDF work, formula/control changes, and physical mouse automation.

## Notes

- This follows `docs/notes/smooth_escape_collatz_luma_PHASED_PLAN.md`.
- Broader smooth-escape/color tuning remains deferred unless this narrow fix directly changes the measured classification.
