# PFC Slider Audit Hardening

## Explicit User Asks

- Continue from the hostile review pause point on `codex/parameter-functionality-campaign`.
- Repair the parameter-slider audit gaps found in review instead of widening into new fractal features.
- Make the descriptor/audit surface account for reachable authority-gated sliders, not only default-visible controls.
- Add runtime proof that switching the relevant authority modes through in-process automation reveals and activates the gated controls.
- Sync stale documentation that still describes pre-campaign parameter behavior.
- Keep physical mouse automation out of the proof path.

## Current Phase

Closed: `codex/pfc-slider-audit-hardening` landed at `64b53b3`, validation and contract-proof receipts were written, the feature branch was pushed, and the campaign holder was fast-forwarded to the same head.

## Phase Checklist

- [x] Phase 0 - create this plan/contract, lock the active slice, and record the starting repo state.
- [x] Phase 1 - RED/native proof that the published descriptor misses authority-gated controls reachable through `poly_kind=custom` and `explaino_root_authority=custom`.
- [x] Phase 2 - expand the descriptor to enumerate default and authority-gated parameter surfaces without hardcoded Python subsets.
- [x] Phase 3 - harden descriptor native/runtime tests for custom polynomial controls, Explaino custom root controls, owner-lane boundaries, validation/state hints, and non-leakage.
- [x] Phase 4 - add no-mouse persistent runtime proof for the generated-to-custom Explaino root-authority transition and custom root slider activation.
- [x] Phase 5 - sync stale known-issue/spec/audit docs so they no longer report already-repaired parameter gaps as current.
- [x] Phase 6 - run focused native/runtime validation, hostile audit, full helper validation as feasible, checkpoint, receipts, push, and clean-tree proof.

## Owner Seams

- Descriptor implementation: `ui_app/src/fractal_parameter_surface_descriptor.cpp`, `ui_app/src/fractal_parameter_surface_descriptor.h`.
- Schema and binding authority: `ui/fractal_binding_surface_v1.ui_schema.json`, `ui_app/src/schema_binding.cpp`, `ui_app/src/ui_schema.cpp`.
- Runtime automation/report authority: `tests/runtime_harness.py`, `ui_app/src/main.cpp`, `ui_app/src/viewer_ui_automation_report.cpp`.
- Runtime proof: `tests/test_fractal_parameter_surface_descriptor_cli.py`, `tests/test_fractal_runtime_generated_internal_editors.py`.
- Native proof: `ui_app/tests/test_fractal_parameter_surface_descriptor.cpp`, `ui_app/tests/test_schema_binding.cpp`, `ui_app/tests/test_ui_schema.cpp`.
- Stale documentation: `docs/notes/fractal_control_surface_audit_inventory.md`, `KNOWN_ISSUES.md`, `spec_intake/_STATUS.md`.

## Design Boundary

- This slice changes the audit/proof surface for existing controls only.
- It does not add new fractal types, new formula parameters, Color Pipeline behavior, capture finding behavior, FPS pacing behavior, equation-pack viewport integration, or perturbation zoom.
- Descriptor rows must come from checked-in schema and C++ binding state, not a hardcoded Python list.
- Authority-gated controls must be represented as reachable surfaces with mode metadata so future all-slider sweeps cannot miss them.
- `explaino_all` registry-axis behavior remains unchanged.

## Required REDs

- Current descriptor omits `poly_c0` through `poly_c4` on custom polynomial lanes.
- Current descriptor omits `explaino_custom_root_count` and `explaino_root_*` controls on custom Explaino root-authority lanes.
- Current runtime proof for root editors starts from a preloaded custom state and does not prove the generated-to-custom transition reveals controls.
- Current docs still contain stale pre-campaign parameter claims.

## Proof Ledger

- Starting branch: `codex/pfc-slider-audit-hardening`.
- Source branch/head: `codex/parameter-functionality-campaign` at `a69c27b`.
- Bootstrap status before branch: clean and even with origin.
- Review finding: descriptor reports 46 lanes and 239 default-visible control cells but misses authority-gated custom polynomial and custom root editor controls.
- Native descriptor rail: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_slider_audit_descriptor_native --log artifacts/logs/pfc_slider_audit_descriptor_native.log --out-json artifacts/validation/pfc_slider_audit_descriptor_native.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd test_fractal_parameter_surface_descriptor` passed.
- Runtime publish: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_slider_audit_runtime_publish --log artifacts/logs/pfc_slider_audit_runtime_publish.log --out-json artifacts/validation/pfc_slider_audit_runtime_publish.json --heartbeat-seconds 30 --timeout-seconds 1200 -- ui_app/build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`.
- Runtime proof: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_slider_audit_runtime_pytest --log artifacts/logs/pfc_slider_audit_runtime_pytest.log --out-json artifacts/validation/pfc_slider_audit_runtime_pytest.json --heartbeat-seconds 30 --timeout-seconds 1200 -- py -3.14 -m pytest tests/test_fractal_parameter_surface_descriptor_cli.py tests/test_fractal_runtime_generated_internal_editors.py -q --junitxml artifacts/pytest/pfc_slider_audit_runtime.junit.xml` passed: 4 tests.
- Shared schema/Color Pipeline guard: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_slider_audit_schema --log artifacts/logs/pfc_slider_audit_schema.log --out-json artifacts/validation/pfc_slider_audit_schema.json --heartbeat-seconds 30 --timeout-seconds 900 -- ui_app/build_tests_vsdevcmd.cmd advanced_color_grading_owner` passed.
- Full native helper suite: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_slider_audit_full_native --log artifacts/logs/pfc_slider_audit_full_native.log --out-json artifacts/validation/pfc_slider_audit_full_native.json --heartbeat-seconds 30 --timeout-seconds 2400 -- ui_app/build_tests_vsdevcmd.cmd` passed.
- Code quality: `py -3.14 tools/code_quality_audit.py --check-baseline --out artifacts/validation/pfc_slider_audit_hardening_code_quality.json` passed baseline check, score 95/100.
- Contract schema: `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/pfc_slider_audit_hardening.contract.json --out-json artifacts/validation/pfc_slider_audit_hardening_contract.json` passed.
- Diff hygiene: `py -3.14 tools/viewer_host_run_logged_command.py --label pfc_slider_audit_diff_check --log artifacts/logs/pfc_slider_audit_diff_check.log --out-json artifacts/validation/pfc_slider_audit_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check` passed.

## Hostile Audit

- Status: complete
- Did I actually make the descriptor enumerate reachable gated slider states instead of only default states? Yes: `surface_count`, `visibility_surface_id`, `default_visible`, and gated-surface contexts now expose `poly_custom`, `explaino_roots_custom`, and `explaino_julia_custom` rows.
- Did I avoid replacing a C++ schema/binding-derived descriptor with a hardcoded Python subset? Yes: descriptor expansion runs through `UISchema`, `BindingContext`, `ControlVisibleForContext`, and binding resolution; tests assert representative rows but are not the descriptor authority.
- Did I prove generated-to-custom root-authority transition through no-mouse in-process automation? Yes: `test_explaino_custom_root_editor_changes_live_output_no_mouse` starts generated, switches `fractal.params.explaino_root_authority` to `custom`, checks enum consumption, verifies root controls appear, and proves root edits change the frame hash.
- Did I preserve Explaino-all registry-axis behavior and owner-lane boundaries? Yes: no schema axis ownership changed; descriptor tests continue to assert owner-only controls do not leak to `explaino_all` while common/root-authority surfaces remain explicitly classified.
- Did I avoid Color Pipeline, capture, FPS, equation-pack, perturbation, and broad engine drift? Yes: changed files are descriptor/report/harness/tests/docs/contract only; shared Color Pipeline rails stayed green.
- Did I sync stale docs without claiming deferred future work is complete? Yes: Phoenix and spec status were updated; the old all-fractal inventory is explicitly marked historical, while perturbation/equation-pack/preset-library work remains future.
- Did I close with validation, receipts, push, and clean tree? Yes: commit `64b53b3` has validation and contract-proof receipts, `codex/pfc-slider-audit-hardening` and `codex/parameter-functionality-campaign` were pushed, and the tree was clean afterward.

## Audit Passes

- [x] Pass 1 - review descriptor expansion for missed gated controls or owner-lane leakage.
- [x] Pass 2 - review runtime automation proof for false-positive command consumption or state-preload shortcuts.
- [x] Pass 3 - review stale docs and final diff for unrelated scope expansion.
- [x] Clean re-read of the repaired state: descriptor/report fields, no-mouse enum proof, docs, and validation receipts were reviewed again after fixing the findings; no additional real defect found.

## Audit Findings

- [x] Finding 1: the old descriptor only represented default-visible controls, so all-slider audits could miss reachable `poly_kind=custom`, `explaino_root_authority=custom`, and `explaino_julia_constant_mode=custom` sliders. Fixed by enumerating explicit descriptor surfaces with metadata and native/runtime assertions.
- [x] Finding 2: the first local implementation wired enum command state into the report call but did not emit the enum fields or descriptor surface fields in JSON. Fixed before validation; runtime harness now fails closed unless `requested_enum_path`, `requested_enum_id`, and `enum_consumed` match the command.
- [x] Finding 3: documentation surfaces still treated Phoenix as single-preset and the original audit table as current truth. Fixed by updating `KNOWN_ISSUES.md`, `spec_intake/_STATUS.md`, and adding a 2026-05-23 status note to the historical audit inventory.

## Out Of Scope

- New parameter functionality beyond currently defined controls.
- New fractal types or formula variants.
- Color Pipeline behavior changes.
- Capture finding and FPS pacing behavior changes.
- Equation-pack viewport integration.
- Perturbation zoom.
- Physical mouse or OS cursor automation.
