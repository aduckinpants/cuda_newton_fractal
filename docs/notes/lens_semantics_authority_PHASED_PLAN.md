# Lens Semantics Authority

## Current Phase

Closed - Lens semantics authority is implemented, validated, checkpointed, receipted, and pushed

## Phase Checklist

- [x] Phase 1 - inspect renderer mask semantics and add failing descriptor/completeness tests
- [x] Phase 2 - add explicit lens semantics descriptors for every shipped `FractalType`
- [x] Phase 3 - route renderer/probe/report vocabulary through the descriptor surface
- [x] Phase 4 - validate focused native/runtime rails and hostile-audit touched seams
- [x] Phase 5 - prepare checkpoint, receipt, push, and clean-tree closeout

## Explicit User Asks

- [active] Start the next step after SDF field interface extraction.
- [active] Work Step 2C next: Lens semantics authority.
- [active] Keep this bounded to cleanup/substrate work, not authored SDF packs or renderer redesign.
- [active] Preserve current Lens SDF, flashlight, runtime-walk, and live viewer behavior while making mask semantics explicit.

## Scope

In scope:

- Centralize what the Lens mask means for every shipped `FractalType`.
- Preserve current basin root-parity behavior while naming it explicitly as a synthetic basin-parity partition.
- Preserve current escape/interior behavior while naming it explicitly as escape-interior membership.
- Add focused tests proving every current fractal maps to one descriptor and unsupported future values fail closed.
- Route renderer/probe/report vocabulary through one Lens semantics helper surface where this slice touches the seam.

Out of scope:

- Authored SDF packs.
- CUDA SDF pack evaluation.
- SDF-native fractal lanes.
- Color Pipeline SDF signals.
- Viewport overlay productization.
- Changing visual Lens output semantics except to preserve and name the existing behavior.
- Selector/view presets, camera/dive behavior, smooth-escape tuning, or renderer redesign.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on `master` at `0f8fc28`; it reported the prior `sdf_field_interface_extraction` contract as active, so this slice must re-lock before product mutation.
- Parent campaign plan: `docs/notes/top_five_backlog_campaign_PHASED_PLAN.md`.
- Source SDF roadmap: `docs/notes/sdf_field_pack_near_term_TODO.md`.
- Slice start: `py -3.14 tools/viewer_host_begin_work_slice.py --intent "lens semantics authority" --profile runtime --plan docs/notes/lens_semantics_authority_PHASED_PLAN.md --contract docs/contracts/lens_semantics_authority.contract.json` opened this branch contract with token `ck:ff90e1aa`.
- RED: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_semantics_authority_test_fractal_family_rules_red --log artifacts/logs/lens_semantics_authority_test_fractal_family_rules_red.log --out-json artifacts/validation/lens_semantics_authority_test_fractal_family_rules_red.json --heartbeat-seconds 30 --timeout-seconds 180 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_fractal_family_rules` failed because the Lens semantics descriptor API did not exist.
- Native green: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_semantics_authority_test_fractal_family_rules --log artifacts/logs/lens_semantics_authority_test_fractal_family_rules.log --out-json artifacts/validation/lens_semantics_authority_test_fractal_family_rules.json --heartbeat-seconds 30 --timeout-seconds 180 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_fractal_family_rules` passed.
- Native green: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_semantics_authority_test_fractal_renderer --log artifacts/logs/lens_semantics_authority_test_fractal_renderer.log --out-json artifacts/validation/lens_semantics_authority_test_fractal_renderer.json --heartbeat-seconds 30 --timeout-seconds 240 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_fractal_renderer` passed.
- Native green: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_semantics_authority_test_flashlight_probe --log artifacts/logs/lens_semantics_authority_test_flashlight_probe.log --out-json artifacts/validation/lens_semantics_authority_test_flashlight_probe.json --heartbeat-seconds 30 --timeout-seconds 180 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_flashlight_probe` passed.
- Native green: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_semantics_authority_test_runtime_walk_headless --log artifacts/logs/lens_semantics_authority_test_runtime_walk_headless.log --out-json artifacts/validation/lens_semantics_authority_test_runtime_walk_headless.json --heartbeat-seconds 30 --timeout-seconds 180 -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_runtime_walk_headless` passed.
- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_semantics_authority_runtime_publish --log artifacts/logs/lens_semantics_authority_runtime_publish.log --out-json artifacts/validation/lens_semantics_authority_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 900 -- cmd /c ui_app\build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Runtime proof: `py -3.14 -m pytest tests/test_fractal_runtime_flashlight_probe.py::test_flashlight_probe_reports_lens_downsampled_size -q --junitxml artifacts/pytest/lens_semantics_authority_runtime.junit.xml` passed after stale-output hardening.
- Contract validator: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/lens_semantics_authority.contract.json --out-json artifacts/validation/lens_semantics_authority_contract.json` passed.
- Plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py` passed.
- Hostile audit validator: `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/lens_semantics_authority_PHASED_PLAN.md --out-json artifacts/validation/lens_semantics_authority_hostile_audit.json` passed.
- Code quality: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/lens_semantics_authority_code_quality.json` passed with score 95/100 and no critical/error findings.
- Diff check: `py -3.14 tools/viewer_host_run_logged_command.py --label lens_semantics_authority_diff_check --log artifacts/logs/lens_semantics_authority_diff_check.log --out-json artifacts/validation/lens_semantics_authority_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.
- Prior completed prerequisites:
  - `docs/notes/lens_sdf_truth_cleanup_PHASED_PLAN.md`
  - `docs/notes/sdf_field_interface_extraction_PHASED_PLAN.md`

## Hostile Audit

- Status: complete

## Audit Passes

- [x] Pass 1 - verified every shipped `FractalType` has exactly one explicit Lens semantics descriptor, no shipped descriptor resolves to `unsupported`, and the registry preserves the old `SupportsBasinColoring` / `IsEscapeTimeFamily` behavior split.
- [x] Pass 2 - verified the renderer, flashlight probe, runtime-walk report, and runtime proof now use the Lens semantics vocabulary where this slice touched them.
- [x] Pass 3 - verified the diff did not add authored SDF packs, Color Pipeline signals, selector/view presets, camera/dive behavior, smooth-escape tuning, or renderer redesign.

## Audit Findings

- [x] First implementation tried to route the CUDA renderer through the host registry lookup, which is not device-safe. `test_fractal_renderer` caught the CUDA compile failure; the repair keeps the host registry for reports/tests and uses a macro-generated `LensMaskPartitionForFractal(...)` switch for device code.
- [x] `flashlight_probe.cpp` initially used `LensMaskSemanticId(...)` without including the owning header. `test_flashlight_probe` caught the compile failure; the repair includes `fractal_family_rules.h`.
- [x] The targeted runtime proof was reading `diagnostics\last\flashlight_probe.json` without proving the file was regenerated, and direct inspection showed the file could be stale. The repair mirrors flashlight companion artifacts into `diagnostics\last`, wraps the shared runtime command with the automation lock, deletes stale proof artifacts before the lens semantics check, and asserts regeneration before reading JSON.
- [x] The initial Lens semantics completeness test did not explicitly prove the new registry preserved the old basin-vs-escape mask split. The repair added per-fractal checks against `SupportsBasinColoring(...)` and `IsEscapeTimeFamily(...)`.
- [x] Clean re-read of the repaired state found no additional real defect in the Lens semantics registry, renderer mask branch, flashlight report JSON, runtime-walk report JSON, or stale-output runtime proof.

## Action Hostile Review

- Action ID: lens-semantics-authority-red
- Suspected failure mode: The renderer knows about basin root-parity masks and escape/interior masks, but the meaning is split across ad hoc branches instead of one explicit descriptor surface.
- Correct owner/action: Add a Lens semantics descriptor layer in `fractal_family_rules.h`, test every `FractalType`, then route mask generation and report vocabulary through that layer.
- Proof surface: Focused native `test_fractal_family_rules`, focused renderer/Lens dependent tests if touched, runtime publish, and no-mouse published-runtime proof for public Lens SDF consumers.
- Blocked action: Authored SDF packs, CUDA SDF pack evaluator, SDF-native fractal lanes, Color Pipeline SDF signals, selector/view preset work, camera/dive behavior, smooth-escape tuning, or broad renderer rewrites.

## Notes

- This is Step 2C from the top-five campaign and Slice 3 from `docs/notes/sdf_field_pack_near_term_TODO.md`.
- Expected end state: Lens SDF, probes, and reports can use one explicit semantics vocabulary without changing the current user-visible mask behavior.
