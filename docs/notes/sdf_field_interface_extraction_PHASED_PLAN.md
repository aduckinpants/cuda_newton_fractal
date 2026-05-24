# SDF Field Interface Extraction

## Current Phase

Closed - SDF scalar field interface extracted and validated

## Phase Checklist

- [x] Phase 1 - inspect current Lens SDF RGBA authority and write failing scalar-field tests
- [x] Phase 2 - add a reusable scalar signed-distance field result/view beside Lens SDF
- [x] Phase 3 - route Lens SDF RGBA presentation through the scalar field without changing visible behavior
- [x] Phase 4 - validate focused native/runtime rails and hostile-audit touched seams
- [x] Phase 5 - prepare checkpoint, receipt, push, and clean-tree closeout

## Explicit User Asks

- [done] Continue the top-five mini-sprint after Lens SDF truth cleanup.
- [done] Work Step 2B next: SDF field interface extraction.
- [done] Keep this as cleanup/substrate work, not authored SDF packs or a renderer rewrite.
- [done] Preserve existing Lens SDF, flashlight, runtime-walk, and live viewer behavior while extracting scalar field authority.

## Scope

In scope:

- Add a reusable scalar signed-distance field result/view in the Lens SDF utility surface.
- Keep RGBA Lens SDF output as presentation derived from scalar distances, not the only authority.
- Preserve the current sign convention and document/test it.
- Add focused native tests for dimensions, sign convention, diagonal distance behavior, no-boundary cases, invalid input failure, and RGBA parity through the new interface.
- Keep runtime proof no-mouse/headless and limited to regression protection for existing public Lens SDF consumers.

Out of scope:

- Authored SDF packs.
- CUDA SDF pack evaluation.
- SDF-native fractal lanes.
- Color Pipeline SDF signals.
- Viewport overlay productization.
- Lens semantics authority for every `FractalType`.
- Selector/view presets, camera/dive behavior, smooth-escape tuning, or renderer redesign.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on branch `codex/sdf-field-interface-extraction` at `5ebf734`; it reported the prior `lens_sdf_truth_cleanup` contract as active, so this slice must re-lock before product mutation.
- Parent campaign plan: `docs/notes/top_five_backlog_campaign_PHASED_PLAN.md`.
- Source SDF roadmap: `docs/notes/sdf_field_pack_near_term_TODO.md`.
- Prior completed prerequisite: `docs/notes/lens_sdf_truth_cleanup_PHASED_PLAN.md`.
- New contract locked: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "sdf field interface extraction" --profile runtime --plan docs/notes/sdf_field_interface_extraction_PHASED_PLAN.md --contract docs/contracts/sdf_field_interface_extraction.contract.json` produced checkpoint token `ck:2061e36c`.
- RED: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_interface_extraction_test_lens_sdf_red --log artifacts/logs/sdf_field_interface_extraction_test_lens_sdf_red.log --out-json artifacts/validation/sdf_field_interface_extraction_test_lens_sdf_red.json --heartbeat-seconds 30 --timeout-seconds 180 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_lens_sdf` failed because `SdfFieldResult`, `SdfFieldView`, `ComputeSignedDistanceSdfFieldChamfer`, `BuildSignedDistanceSdfRgba`, `SampleSignedDistanceSdfField`, and `ComputeLensSdfFieldForMask` did not exist.
- Corrected RED fixture: the corner-to-center chamfer witness is two diagonal steps in the 5x5 mask fixture, not one; the test now asserts the actual 2.8px diagonal distance band.
- GREEN native:
  - `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_interface_extraction_test_lens_sdf --log artifacts/logs/sdf_field_interface_extraction_test_lens_sdf.log --out-json artifacts/validation/sdf_field_interface_extraction_test_lens_sdf.json --heartbeat-seconds 30 --timeout-seconds 180 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_lens_sdf`
  - `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_interface_extraction_test_flashlight_probe --log artifacts/logs/sdf_field_interface_extraction_test_flashlight_probe.log --out-json artifacts/validation/sdf_field_interface_extraction_test_flashlight_probe.json --heartbeat-seconds 30 --timeout-seconds 180 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_flashlight_probe`
  - `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_interface_extraction_test_runtime_walk_headless --log artifacts/logs/sdf_field_interface_extraction_test_runtime_walk_headless.log --out-json artifacts/validation/sdf_field_interface_extraction_test_runtime_walk_headless.json --heartbeat-seconds 30 --timeout-seconds 180 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_runtime_walk_headless`
- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label sdf_field_interface_extraction_runtime_publish --log artifacts/logs/sdf_field_interface_extraction_runtime_publish.log --out-json artifacts/validation/sdf_field_interface_extraction_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- cmd /c ui_app\build_vsdevcmd.cmd` passed.
- Published no-mouse runtime proof: `py -3.14 -m pytest tests/test_fractal_runtime_flashlight_probe.py::test_flashlight_probe_reports_lens_downsampled_size -q --junitxml artifacts/pytest/sdf_field_interface_extraction_runtime.junit.xml` passed.

## Hostile Audit

- Status: complete

## Audit Passes

- [x] Pass 1 - verified the scalar field interface is real data authority, not a thin RGBA alias; it carries dimensions, pixel scale, sign convention, source kind, and scalar distance storage.
- [x] Pass 2 - verified live Lens SDF, flashlight, and runtime-walk behavior did not regress with focused native rails, runtime publish, and published no-mouse flashlight proof.
- [x] Pass 3 - verified no authored SDF pack, Color Pipeline signal, selector, camera, smooth-escape, or renderer redesign slipped into this slice.

## Audit Findings

- [x] Real finding - The initial RED expected the 5x5 corner-to-center chamfer distance to be one diagonal step; the fixture is two diagonal steps. Repaired by correcting the witness to the actual 2.8px chamfer range before accepting green.
- [x] Real finding - The first implementation added a scalar field API but left flashlight/runtime-walk sampling on the old one-off mask sampler. Repaired by routing those consumers through `SdfFieldResult` and `SampleSignedDistanceSdfField(...)`.
- [x] Clean re-read - Re-read the repaired Lens SDF path and confirmed `ComputeSignedDistanceSdfChamfer(...)` now derives RGBA presentation from scalar field data.
- [x] Clean re-read - Re-read the diff and confirmed this slice did not add authored SDF packs, CUDA SDF pack evaluation, SDF-native fractal lanes, Color Pipeline signals, selector/view preset behavior, camera/dive behavior, smooth-escape tuning, renderer redesign, or physical mouse automation.

## Action Hostile Review

- Action ID: sdf-field-interface-red
- Suspected failure mode: Current Lens SDF exposes only RGBA/debug presentation, forcing downstream consumers to recompute or parse presentation data instead of consuming a reusable scalar field.
- Correct owner/action: Add scalar signed-distance field authority in the Lens SDF utility layer, then derive RGBA presentation from it.
- Proof surface: Focused native Lens SDF tests, dependent flashlight/runtime-walk tests if touched, runtime publish if the live viewer path changes, and no-mouse published-runtime proof for existing public Lens SDF consumers.
- Blocked action: Authored SDF packs, CUDA SDF pack evaluator, SDF-native fractal lanes, Color Pipeline SDF signals, selector/view preset work, camera/dive behavior, smooth-escape tuning, or broad renderer rewrites.

## Notes

- This is Step 2B from the top-five campaign and Slice 2 from `docs/notes/sdf_field_pack_near_term_TODO.md`.
- The expected end state is a reusable scalar field substrate seed, not a user-facing SDF pack feature.
