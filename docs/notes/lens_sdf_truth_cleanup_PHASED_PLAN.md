# Lens SDF Truth Cleanup

## Current Phase

Closed - Lens SDF downsample truth cleanup repaired and validated

## Phase Checklist

- [x] Phase 1 - inspect Lens SDF downsample authority and write failing focused tests
- [x] Phase 2 - implement shared downsample helpers and route live Lens SDF through them
- [x] Phase 3 - validate focused native/runtime rails and hostile-audit touched seams
- [x] Phase 4 - checkpoint, receipts, push, and clean-tree closeout

## Explicit User Asks

- [done] Continue the top-five mini-sprint after diagnostics capture.
- [done] Work Step 2A first: Lens SDF truth cleanup.
- [done] Keep this cleanup/polish work bounded; do not start the full authored SDF pack system here.
- [done] Preserve current fractal/render behavior outside the Lens SDF downsample path.

## Scope

In scope:

- Make `fractal.lens.downsample` truthful for the live Lens SDF work resolution.
- Consolidate Lens SDF downsample normalization and mask downsampling into the Lens SDF utility surface.
- Preserve default behavior compatibility where possible.
- Add focused native tests for downsample normalization, mask downsampling, signed-distance samples, and output dimensions.
- Keep any runtime proof headless/no-mouse.

Out of scope:

- Authored SDF packs.
- SDF field interface extraction beyond the helper surface needed for this slice.
- Lens semantics authority for every `FractalType`.
- Color Pipeline SDF signals.
- Viewport overlay productization.
- Finding capture, camera/dive behavior, selector/view presets, smooth-escape tuning, or renderer redesign.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on branch `codex/lens-sdf-truth-cleanup` at `e2bb164`.
- Salticid reference inspected:
  - `C:\code\salticid-cuda\ide_ui_dx11\ui_app\src\lens_sdf_chamfer.h`
  - `C:\code\salticid-cuda\ide_ui_dx11\ui_app\src\lens_sdf_chamfer.cpp`
  - `C:\code\salticid-cuda\ide_ui_dx11\ui_app\tests\test_lens_sdf_chamfer.cpp`
- Current local seams inspected:
  - `ui_app/src/lens_sdf.h`
  - `ui_app/src/lens_sdf.cpp`
  - `ui_app/src/main.cpp`
  - `ui_app/src/flashlight_probe.cpp`
  - `ui_app/src/runtime_walk_headless.cpp`
  - `ui_app/tests/test_lens_sdf.cpp`
  - `ui/fractal_binding_surface_v1.ui_schema.json`
- RED tests:
  - `py -3.14 tools/viewer_host_run_logged_command.py --label lens_sdf_truth_cleanup_test_lens_sdf --log artifacts/logs/lens_sdf_truth_cleanup_test_lens_sdf.log --out-json artifacts/validation/lens_sdf_truth_cleanup_test_lens_sdf.json --heartbeat-seconds 30 --timeout-seconds 180 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_lens_sdf` failed with unresolved Lens SDF downsample helper symbols.
- First GREEN:
  - `py -3.14 tools/viewer_host_run_logged_command.py --label lens_sdf_truth_cleanup_test_lens_sdf --log artifacts/logs/lens_sdf_truth_cleanup_test_lens_sdf.log --out-json artifacts/validation/lens_sdf_truth_cleanup_test_lens_sdf.json --heartbeat-seconds 30 --timeout-seconds 180 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_lens_sdf` passed after adding shared downsample helpers and the downsample-aware Lens SDF helper.
  - `py -3.14 tools/viewer_host_run_logged_command.py --label lens_sdf_truth_cleanup_test_flashlight_probe --log artifacts/logs/lens_sdf_truth_cleanup_test_flashlight_probe.log --out-json artifacts/validation/lens_sdf_truth_cleanup_test_flashlight_probe.json --heartbeat-seconds 30 --timeout-seconds 180 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_flashlight_probe` passed after routing flashlight downsample normalization through Lens SDF helpers.
  - `py -3.14 tools/viewer_host_run_logged_command.py --label lens_sdf_truth_cleanup_test_runtime_walk_headless --log artifacts/logs/lens_sdf_truth_cleanup_test_runtime_walk_headless.log --out-json artifacts/validation/lens_sdf_truth_cleanup_test_runtime_walk_headless.json --heartbeat-seconds 30 --timeout-seconds 180 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_runtime_walk_headless` passed after adding the missing focused target and test stubs for the shared helper seam.
- Runtime proof:
  - `py -3.14 tools/viewer_host_run_logged_command.py --label lens_sdf_truth_cleanup_runtime_publish --log artifacts/logs/lens_sdf_truth_cleanup_runtime_publish.log --out-json artifacts/validation/lens_sdf_truth_cleanup_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- cmd /c ui_app\build_vsdevcmd.cmd` passed.
  - `py -3.14 -m pytest tests/test_fractal_runtime_flashlight_probe.py::test_flashlight_probe_reports_lens_downsampled_size -q --junitxml artifacts/pytest/lens_sdf_truth_cleanup_runtime.junit.xml` passed.
- Hostile audit: complete; three real findings were repaired.
- Validation passed:
  - `py -3.14 tools/viewer_host_run_logged_command.py --label lens_sdf_truth_cleanup_test_lens_sdf --log artifacts/logs/lens_sdf_truth_cleanup_test_lens_sdf.log --out-json artifacts/validation/lens_sdf_truth_cleanup_test_lens_sdf.json --heartbeat-seconds 30 --timeout-seconds 180 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_lens_sdf`
  - `py -3.14 tools/viewer_host_run_logged_command.py --label lens_sdf_truth_cleanup_test_flashlight_probe --log artifacts/logs/lens_sdf_truth_cleanup_test_flashlight_probe.log --out-json artifacts/validation/lens_sdf_truth_cleanup_test_flashlight_probe.json --heartbeat-seconds 30 --timeout-seconds 180 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_flashlight_probe`
  - `py -3.14 tools/viewer_host_run_logged_command.py --label lens_sdf_truth_cleanup_test_runtime_walk_headless --log artifacts/logs/lens_sdf_truth_cleanup_test_runtime_walk_headless.log --out-json artifacts/validation/lens_sdf_truth_cleanup_test_runtime_walk_headless.json --heartbeat-seconds 30 --timeout-seconds 180 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_runtime_walk_headless`
  - `py -3.14 tools/viewer_host_run_logged_command.py --label lens_sdf_truth_cleanup_runtime_publish --log artifacts/logs/lens_sdf_truth_cleanup_runtime_publish.log --out-json artifacts/validation/lens_sdf_truth_cleanup_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- cmd /c ui_app\build_vsdevcmd.cmd`
  - `py -3.14 -m pytest tests/test_fractal_runtime_flashlight_probe.py::test_flashlight_probe_reports_lens_downsampled_size -q --junitxml artifacts/pytest/lens_sdf_truth_cleanup_runtime.junit.xml`
  - `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/lens_sdf_truth_cleanup_code_quality.json`
  - `py -3.14 tools/viewer_host_run_logged_command.py --label lens_sdf_truth_cleanup_diff_check --log artifacts/logs/lens_sdf_truth_cleanup_diff_check.log --out-json artifacts/validation/lens_sdf_truth_cleanup_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`

## Hostile Audit

- Status: complete

## Audit Passes

- [x] Pass 1 - verified the live Lens SDF path now consumes `lens.downsample` through `ComputeLensSdfRgbaForMask(...)`.
- [x] Pass 2 - verified helper consolidation did not regress flashlight/runtime-walk headless Lens SDF behavior; clean re-read found no additional real defect.
- [x] Pass 3 - verified no authored SDF pack, field-substrate, Color Pipeline, selector, camera, or renderer redesign slipped into this slice; confirmed the repaired state with focused native and published-runtime proof.

## Audit Findings

- [x] Real finding - The first Step 2A contract touched `runtime_walk_headless.cpp` but did not require the focused runtime-walk headless rail. Repaired by adding a focused build target and required logged validation command.
- [x] Real finding - Because this is viewer-first work, runtime publish alone is not enough. Repaired by adding a published-runtime no-mouse flashlight proof that reads the reported Lens SDF downsampled size.
- [x] Real finding - The first shared live helper downsampled the mask but kept the full-resolution SDF normalization radius. Repaired by normalizing `maxAbsPx` into low-resolution pixels and adding a regression comparison against manual downsample plus chamfer.
- [x] Real finding - The first runtime-walk unit stub compiled the touched helper seam but did not exercise downsample dimensions on a successful walk. Repaired by adding a successful-path test that records normalized low-size dimensions and report completion.
- [x] Clean re-read - Re-read the repaired Lens SDF/live dispatch path and confirmed downsample controls output work dimensions and low-resolution distance scaling.
- [x] Clean re-read - Re-read the diff and confirmed this slice did not add authored SDF packs, field-substrate extraction, Color Pipeline behavior, selector/view preset behavior, camera/dive behavior, smooth-escape tuning, or physical mouse automation.

## Action Hostile Review

- Action ID: lens-sdf-truth-cleanup-red
- Suspected failure mode: The visible `fractal.lens.downsample` control changes saved/bound state but the live Lens SDF path still computes at full render resolution.
- Correct owner/action: Prove the live helper path ignores downsample, then route Lens SDF mask preparation through shared tested helpers.
- Proof surface: Focused native Lens SDF tests, focused dependent headless tests if touched, runtime publish if viewer path changes, and no-mouse command proof where available.
- Blocked action: Authored SDF packs, SDF-native fractal lanes, Color Pipeline SDF signals, selector/view preset work, camera/dive behavior, or broad renderer rewrites.

## Notes

- Parent campaign plan: `docs/notes/top_five_backlog_campaign_PHASED_PLAN.md`.
- Source SDF roadmap: `docs/notes/sdf_field_pack_near_term_TODO.md`.
- This is only Slice 1 / Step 2A from the SDF roadmap.
