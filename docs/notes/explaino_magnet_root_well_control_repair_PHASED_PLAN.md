# ExplainO Magnet Root Well Control Repair Phased Plan

## Explicit User Asks

- Repair the real remaining UX bug: `explaino_magnet_root_well` must expose the same base Magnet manipulation controls as the normal Magnet lane.
- Do not treat Pattern B removal as sufficient; the expected workflow is Magnet Root Well controls plus normal root-layout movement controls.
- Keep this as a focused repair, not a new root-pattern UX redesign or SDF feature slice.

## Current Phase

Phase 4 - Closure validation complete; checkpoint, receipts, rearward review, and push are the remaining workflow steps.

## Phase Checklist

- [x] Phase 0: Create this plan and contract on `codex/explaino-magnet-root-well-control-repair`.
- [x] Phase 1: Add RED tests proving base Magnet controls are missing on `explaino_magnet_root_well`.
- [x] Phase 2: Add a shared base-fractal control visibility rule for Magnet-backed root-field consumers.
- [x] Phase 3: Repair schema/safe-mode/binding/runtime visibility and no-mouse coverage.
- [x] Phase 4: Hostile audit and validation completed; checkpoint, receipts, rearward review, and push are the remaining workflow steps outside product mutation.

## Scope Lock

This slice may repair only the Magnet Root Well control surface and tests/proof around that surface. It must not add Pattern B UI, change root-field math, change Magnet recurrence defaults, start SDF work, or redesign ExplainO Warp.

## Concrete Bug

`explaino_magnet_root_well` is implemented as a Magnet-backed root-field consumer, and runtime/state paths already understand Magnet params for it, but the normal UI still hides the base Magnet controls that make Magnet Type I manipulable:

- `magnet_seed_real`
- `magnet_seed_imag`
- `magnet_relaxation`
- `magnet_bailout`

The previous Active UX repair removed awkward Pattern B controls but failed the real workflow ask because the lane still lacks the base Magnet control surface.

## Desired Behavior

- Selecting `explaino_magnet_root_well` shows Root Well controls, active root-layout controls, seed/phase/spread controls, and base Magnet controls.
- The base Magnet controls remain visible on normal `magnet`.
- Unrelated lanes do not gain Magnet controls.
- Pattern B and Dynamics Root Pattern remain hidden from the normal Magnet Root Well UI.
- Visible controls are consumed by runtime/state/report paths or intentionally hidden.

## Proof Ledger

| Item | Status | Evidence |
| --- | --- | --- |
| Bootstrap | done | `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` on clean tooling head before branch. |
| Rearward review | done | `py -3.14 tools/viewer_host_rearward_review.py` returned `ok` for `4c57d3d`. |
| RED tests | done | `ui_app\build_tests_vsdevcmd.cmd test_ui_schema test_safe_mode_schema test_schema_binding` failed before repair with `test_ui_schema` missing Magnet owner controls for the Root Well lane. |
| Native/tool validation | done | `ui_app\build_tests_vsdevcmd.cmd test_ui_schema test_safe_mode_schema test_schema_binding` passed; extra `ui_app\build_tests_vsdevcmd.cmd test_param_anim_generic` passed; contract validation, plan sync, code-quality baseline, hostile-audit validation, and diff check passed. |
| Runtime publish | done | `ui_app\build_vsdevcmd.cmd` passed and staged `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`. |
| Runtime proof | done | `py -3.14 -m pytest tests/test_fractal_runtime_root_field_consumers.py -q --junitxml artifacts\pytest\explaino_magnet_root_well_control_repair_runtime.junit.xml` passed 8 tests. |
| Hostile audit | done | Audit found and repaired proof gaps; `py -3.14 tools/viewer_host_validate_hostile_audit.py --plan docs/notes/explaino_magnet_root_well_control_repair_PHASED_PLAN.md --out-json artifacts/validation/explaino_magnet_root_well_control_repair_hostile_audit.json` passed. |
| Receipts/rearward/push | pending | Required after checkpoint commit before final closeout. |

## Test Targets

- `test_ui_schema`: Magnet controls include `explaino_magnet_root_well` where the base Magnet control contract requires it, while Pattern B controls remain absent.
- `test_safe_mode_schema`: safe-mode schema exposes the same base Magnet controls for `explaino_magnet_root_well`.
- `test_schema_binding`: binding remains authoritative for Magnet params and visible-control predicates cover the new lane.
- Runtime no-mouse: select `explaino_magnet_root_well`, set `magnet_seed_real`, `magnet_seed_imag`, `magnet_relaxation`, and `magnet_bailout`, prove selector identity and frame/report change.

## Hostile Audit

- Status: complete

Required questions:

- Did I actually expose the base Magnet controls on `explaino_magnet_root_well`?
- Did those controls actually affect the runtime/report path?
- Did I preserve normal Magnet behavior?
- Did I keep Pattern B/Dynamics controls hidden in the normal workflow?
- Did I avoid a one-off list that will repeat this bug for the next base-fractal consumer?
- Did runtime proof cover action through the no-mouse UI harness, not just direct param mutation?

## Audit Passes

- [x] Pass 1: Re-read schema/safe-mode/runtime diffs after GREEN native proof. Found that animation-target visibility was not covered by the required schema rails.
- [x] Pass 2: Ran added `test_param_anim_generic` rail. Found the runtime test was asserting control values that schema automation reports do not publish.
- [x] Pass 3: Re-ran focused and full no-mouse runtime proof after fixing the test to assert visible controls, consumed set-value commands, stable selector identity, and final frame delta.
- [x] Pass 4: Clean re-read of the repaired state confirmed the four Magnet controls are visible on `explaino_magnet_root_well`, Pattern B controls remain hidden, and no additional real defect found.

## Audit Findings

- [x] The first repair shape would have exposed sliders while still leaving animation target parity unproven; added `test_param_anim_generic` coverage for `explaino_magnet_root_well`.
- [x] The first runtime proof incorrectly expected numeric `value` fields in schema control reports; corrected it to use the harness-supported evidence surface: control visibility, consumed set-value commands, selector stability, and frame hash change.
- [x] Clean re-read the repaired state after both fixes; no additional real issue found in the touched control visibility, animation target, or no-mouse runtime proof seams.

## Stop Point

Stop after the repair is validated, checkpointed, receipted, rearward-reviewed, pushed, and the next product direction is left for user review.
