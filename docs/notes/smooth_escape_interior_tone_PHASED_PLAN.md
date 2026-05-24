# Smooth-Escape Interior Tone

## Current Phase

Closed - smooth-escape interior-tone repair implemented, validated, hostile-audited, and ready for clean closeout.

## Phase Checklist

- [x] Phase 1 - open the checked-in plan/contract and lock the active slice
- [x] Phase 2 - add RED native/runtime proof for smooth-escape interiors that render black
- [x] Phase 3 - repair only the smooth-escape non-escaped coloring gate
- [x] Phase 4 - publish runtime and prove the published viewer output improves without mouse automation
- [x] Phase 5 - hostile-audit the touched color seams and docs
- [x] Phase 6 - validate, checkpoint, receipts, push, merge-back, and clean-tree closeout

## Explicit User Asks

- [done] Continue the next highest-confidence smooth-escape/color fix after the measurement inventory.
- [done] Do not reopen selector/view presets, camera/dive behavior, SDF packs, or broad Color Pipeline redesign in this slice.
- [done] Preserve Color Pipeline behavior while changing the narrow smooth-escape output path.
- [done] Use no-mouse runtime proof and avoid repeated live viewer open/close loops.

## Scope

In scope:

- Fix the current escape-time smooth-escape path where non-escaped pixels are forced to black before the programmable color path can run.
- Preserve black interiors for `iteration_count` and `iteration_bands`.
- Keep `root_proximity` and `phase_wheel` interior exceptions working.
- Add native proof that smooth-escape interior pixels are non-black and still react to the smooth-escape owner fields.
- Add published-runtime proof that the measured high black-fraction `magnet` case no longer remains high-black under the current smooth-escape tuple.
- Sync the parent top-five campaign and known-issues text only for this narrow interior-black result.

Out of scope:

- Changing the smooth-escape global band period.
- Adding histogram equalization, adaptive normalization, new palette rows, or per-family catalog tuning.
- Selector/view preset work.
- Camera/dive behavior.
- SDF pack parser or CUDA SDF evaluator work.
- Physical mouse automation.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on `master` at `bdfa342`; the prior `smooth_escape_color_measurement` contract was active and must be replaced before behavior mutation.
- Branch: `codex/smooth-escape-interior-tone` from clean `master` at `bdfa342`.
- Inventory basis: `artifacts/smooth_escape_color_measurement/latest/inventory.md` measured the current published runtime and classified `multibrot` and `magnet` as high black-fraction cases under `smooth_escape/cyclic_escape/escape_default`.
- Code seam: `ui_app/src/escape_time_coloring.h` currently returns black for non-escaped exact legacy smooth-escape pixels before the programmable palette path, then repeats the black gate in `MakeEscapeTimeBaseColor`.
- RED native proof: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_interior_tone_native_red --log artifacts/logs/smooth_escape_interior_tone_native_red.log --out-json artifacts/validation/smooth_escape_interior_tone_native_red.json --heartbeat-seconds 30 --timeout-seconds 600 -- cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner` failed on `Smooth-escape interiors should use the programmable color path instead of forced black`.
- RED runtime proof: `py -3.14 -m pytest tests/test_fractal_runtime_smooth_escape_color_inventory.py -q --junitxml artifacts/pytest/smooth_escape_interior_tone_runtime_red.junit.xml` failed with `magnet` black-pixel fraction `0.9594907407407407`.
- Native green: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_interior_tone_native_green --log artifacts/logs/smooth_escape_interior_tone_native_green.log --out-json artifacts/validation/smooth_escape_interior_tone_native_green.json --heartbeat-seconds 30 --timeout-seconds 600 -- cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner` passed.
- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_interior_tone_runtime_publish --log artifacts/logs/smooth_escape_interior_tone_runtime_publish.log --out-json artifacts/validation/smooth_escape_interior_tone_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- cmd /c ui_app\build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Published-runtime proof: `py -3.14 -m pytest tests/test_fractal_runtime_smooth_escape_color_inventory.py -q --junitxml artifacts/pytest/smooth_escape_interior_tone_runtime.junit.xml` passed with no physical mouse automation.
- Full inventory proof: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_interior_tone_inventory --log artifacts/logs/smooth_escape_interior_tone_inventory.log --out-json artifacts/validation/smooth_escape_interior_tone_inventory.json --heartbeat-seconds 30 --timeout-seconds 240 -- py -3.14 tools/smooth_escape_color_inventory.py --out-dir artifacts/smooth_escape_interior_tone/latest --width 96 --height 72 --runtime-lock` passed.
- Post-fix inventory summary: high black-fraction cases are `none`; `mandelbrot`, `multibrot`, and `magnet` report `0.000` black fraction at `96x72`. Remaining low-luma/low-unique cases stay later tuning work.
- Audit hardening: after the first green pass, the diff review found the shared basin gate also changed for smooth-escape; `ui_app/tests/test_escape_time_coloring.cpp` now covers smooth-escape basin interiors and live smooth-escape scale sensitivity.
- Hardened native green: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_interior_tone_native_hardened --log artifacts/logs/smooth_escape_interior_tone_native_hardened.log --out-json artifacts/validation/smooth_escape_interior_tone_native_hardened.json --heartbeat-seconds 30 --timeout-seconds 600 -- cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner` passed.
- Audit hardening 2: hostile re-read found iteration-band interior black was preserved by code but not directly asserted. Repaired by adding an explicit unescaped iteration-band black assertion.
- Parseable final focused native rail: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_interior_tone_native_final --log artifacts/logs/smooth_escape_interior_tone_native_final.log --out-json artifacts/validation/smooth_escape_interior_tone_native_final.json --heartbeat-seconds 30 --timeout-seconds 600 -- cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner` passed.
- Final focused native rail: `cmd /c ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner` passed after the escape-time, basin, iteration-count, and iteration-band assertions were all present.
- Code-quality audit first failed because `TryMakeDiscreteEscapeTimeColor` grew from 27 to 29 lines; repaired by tightening the iteration-count branch without changing behavior.
- Code quality: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/smooth_escape_interior_tone_code_quality.json` passed with score 95/100 and no baseline regression.
- Final runtime publish after the source tightening: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_interior_tone_runtime_publish --log artifacts/logs/smooth_escape_interior_tone_runtime_publish.log --out-json artifacts/validation/smooth_escape_interior_tone_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- cmd /c ui_app\build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Final published-runtime pytest: `py -3.14 -m pytest tests/test_fractal_runtime_smooth_escape_color_inventory.py -q --junitxml artifacts/pytest/smooth_escape_interior_tone_runtime.junit.xml` passed.
- Final full inventory proof: `py -3.14 tools/viewer_host_run_logged_command.py --label smooth_escape_interior_tone_inventory --log artifacts/logs/smooth_escape_interior_tone_inventory.log --out-json artifacts/validation/smooth_escape_interior_tone_inventory.json --heartbeat-seconds 30 --timeout-seconds 240 -- py -3.14 tools/smooth_escape_color_inventory.py --out-dir artifacts/smooth_escape_interior_tone/latest --width 96 --height 72 --runtime-lock` passed after the final publish.

## Hostile Audit

- Status: complete

## Audit Passes

- [x] Pass 1 - verified the fix is limited to smooth-escape non-escaped coloring and does not change iteration-count or iteration-band interior black.
- [x] Pass 2 - verified Color Pipeline core/window/schema rails are green and smooth-escape owner fields remain live on escape-time and basin interiors.
- [x] Pass 3 - verified the runtime proof uses the published runtime without physical mouse automation and the docs keep low-luma/low-unique tuning open.

## Audit Findings

- [x] Real finding - The first green diff also changed the shared basin-coloring gate for smooth-escape interiors, but the initial native RED only covered escape-time interiors. Repaired by adding basin smooth-escape interior and smooth-escape scale-sensitivity assertions, then rerunning the focused native color rail.
- [x] Real finding - The first preservation test only directly asserted iteration-count interiors stayed black. Repaired by adding an explicit unescaped iteration-band black assertion, then rerunning the focused native color rail.
- [x] Real finding - The code-quality audit caught a max-function-length regression in `TryMakeDiscreteEscapeTimeColor`. Repaired by tightening the iteration-count branch and rerunning native, code-quality, runtime publish, runtime pytest, and inventory proof.
- [x] Clean re-read - Re-read the final diff and confirmed the slice does not add palette rows, histogram/adaptive normalization, selector/view preset work, camera/dive behavior, SDF work, or physical mouse automation.

## Action Hostile Review

- Action ID: smooth-escape-interior-tone-red
- Suspected failure mode: A smooth-escape/color fix could accidentally recolor iteration-count interiors, bypass Color Pipeline controls, or only prove helper code while the published runtime remains high-black.
- Correct owner/action: Add native and published-runtime no-mouse proof around the smooth-escape interior gate, then make the smallest coloring-path repair.
- Proof surface: focused native color tests, runtime publish, published-runtime inventory pytest, focused inventory artifact, plan/contract validators, code-quality baseline, diff check.
- Blocked action: palette redesign, per-family tuning metadata, selector/view preset work, camera/dive behavior, SDF work, and physical mouse automation.

## Notes

- This is the first behavior-changing slice after `docs/notes/smooth_escape_color_measurement_PHASED_PLAN.md`.
- The measurement slice also found low-luma and low-unique-color cases; those remain later tuning work unless this narrow fix directly changes their measured classification.
