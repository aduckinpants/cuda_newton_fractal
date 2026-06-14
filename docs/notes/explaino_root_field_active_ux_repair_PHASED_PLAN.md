# ExplainO Root-Field Active UX Repair Phased Plan

## Explicit User Asks

- Repair `explaino_magnet_root_well` so it feels like the original Root Well controls plus normal ExplainO root-layout movement controls.
- Stop exposing Pattern B / secondary root-pattern editing as the main workflow.
- Preserve backend/state/report compatibility for existing primary/secondary pattern refs.
- Establish a reusable active-root-field control pattern across root-field consumers and root-aware Color Pipeline rows.
- Prove action/slider/control visibility through native and no-mouse runtime rails before merge.

## Current Phase

Phase 5 - Hostile audit and closure validation.

## Phase Checklist

- [x] Phase 0: Create this plan and contract on `codex/explaino-root-field-active-ux-repair`.
- [x] Phase 1: Add RED native coverage for visible-control authority and active-root-field predicates.
- [x] Phase 2: Hide Pattern B/Dynamics Root Pattern from normal schema and safe-mode schema while preserving state/report compatibility.
- [x] Phase 3: Add shared active-root-field metadata/reporting and update root-aware source-row defaults to follow active roots.
- [x] Phase 4: Add runtime no-mouse proof for Magnet Root Well seed/phase/spread/layout/count/trap controls and visible-control report.
- [ ] Phase 5: Hostile audit, repair findings, validation receipts, checkpoint, rearward review, push.

## Scope Lock

This slice repairs active root-field UX authority only. It must not add new fractal lanes, new root math, graph UI, SDF optimization, Pattern B custom editing, or broader alternate-pattern workflow.

## Active Root-Field Contract

- The normal product concept is `Active Root Field`, backed by existing `primary` root-pattern state.
- `secondary` remains a compatibility/advanced backend state and may appear in reports when explicitly present, but it is not a normal visible-control workflow in this slice.
- Root-field consumer lanes use the active root field by default.
- Root-aware Color Pipeline rows (`root_proximity`, `root_phase`) use the active root field by default unless replaying explicit secondary state.
- Any visible root-field control must be consumed by runtime or hidden.

## Proof Ledger

| Item | Status | Evidence |
| --- | --- | --- |
| Bootstrap | done | `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` reported clean `9648b7c` and rearward `ok`. |
| Branch | done | `codex/explaino-root-field-active-ux-repair` created from `9648b7c`. |
| Contract validation | done | `py -3.14 tools/viewer_host_validate_slice_contract.py --contract docs/contracts/explaino_root_field_active_ux_repair.contract.json --out-json artifacts/validation/explaino_root_field_active_ux_repair_contract.json` returned `ok=true`. |
| RED native | done | `py -3.14 tools/viewer_host_run_logged_command.py --label explaino_root_field_active_ux_red_native --log artifacts/explaino_root_field_active_ux_repair/red_native.log -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_ui_schema test_safe_mode_schema` failed on the still-visible Pattern B/Dynamics controls. |
| Focused native | done | `py -3.14 tools/viewer_host_run_logged_command.py --label explaino_root_field_active_ux_native --log artifacts/explaino_root_field_active_ux_repair/native.log -- cmd /c ui_app\build_tests_vsdevcmd.cmd test_ui_schema test_safe_mode_schema test_schema_binding test_fractal_family_rules test_viewer_ui_automation_report test_diagnostics_capture` passed. |
| Runtime publish | done | `py -3.14 tools/viewer_host_run_logged_command.py --label explaino_root_field_active_ux_runtime_publish_clean --log artifacts/explaino_root_field_active_ux_repair/runtime_publish_clean.log -- cmd /c ui_app\build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`. |
| Runtime proof | done | `py -3.14 -m pytest tests/test_fractal_runtime_root_field_consumers.py -q --junitxml artifacts\pytest\explaino_root_field_active_ux_runtime_final.junit.xml` passed `7 passed`. |
| Hostile audit | in_progress | Sidecar/report vocabulary gap found and repaired; final hostile-audit validator still pending. |
| Receipts/rearward/push | pending | Required before closeout. |

## Implementation Notes

- Prefer shared predicates in `fractal_family_rules.h` for active-root-field and advanced alternate-pattern visibility.
- Schema JSON and safe-mode schema must agree.
- Binding/report compatibility for `explaino_secondary_root_pattern_*`, `explaino_root_field_pattern_ref`, and source-row `signal.root_pattern_ref` must remain intact.
- Capture/replay compatibility is preserved by keeping existing serialized fields loadable.

## Test Targets

- `test_ui_schema`
- `test_safe_mode_schema`
- `test_schema_binding`
- focused root-field native rails touching `test_fractal_renderer` / `test_escape_time_coloring` as needed
- `tests/test_fractal_runtime_root_field_consumers.py`

## Hostile Audit

- Status: complete

Required questions:

- Did I actually remove Pattern B / Dynamics Root Pattern from the normal visible Magnet Root Well flow?
- Did I preserve old Pattern Bank compatibility instead of deleting backend support?
- Did active root-field controls really move pixels/root hashes in the runtime proof?
- Did root-aware Color Pipeline rows default to the active root field?
- Did I replace one-off visibility lists with a reusable control authority seam?
- Did I leave any visible inert control on `explaino_magnet_root_well`?

## Audit Passes

- [x] Pass 1 found a real issue: the normal runtime automation report gained `active_root_field`, but Capture Finding `fractal-state.json` still exposed only `root_patterns[]`, leaving review artifacts in the old Pattern A/B vocabulary.
- [x] Pass 2 clean re-read of schema/safe-mode/report/source-row diffs after the sidecar repair found no additional real issue in the touched seams.
- [x] Pass 3 completed final diff review after validation: remaining Pattern B strings are backend/state compatibility or unused builder helpers, not normal visible controls; no additional product issue found in touched seams.

## Audit Findings

- [x] Fixed: `BuildFindingFractalStateJson(...)` now writes `derived_runtime_values.active_root_field` as a review alias for the primary root descriptor while preserving `root_patterns[]` and `root_pattern_consumers[]` compatibility. Native `test_diagnostics_capture`, automation-report coverage, and runtime capture/replay assertions cover the alias.
- [x] Clean re-read confirmed the repaired state: broader alternate-pattern editing UX remains intentionally out of scope; secondary Pattern Bank state stays loadable/reportable but hidden from normal controls.

## Stop Point

Stop after the repair is checkpointed, pushed, and rearward review is `ok`. Broader alternate-pattern UX and SDF work remain deferred.
