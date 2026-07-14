# ExplainO Multibrot Root Trap Phased Plan

## Explicit User Asks

- Merge the current Color Pipeline root-pattern authority repair to `master` before starting this slice.
- Add exactly one new experimental lane: `explaino_multibrot_root_trap`.
- Preserve existing Multibrot recurrence behavior, including non-integer real exponent and complex exponent support.
- Prove zero trap strength matches baseline `multibrot` for integer, non-integer, above-old-cap, and complex exponent cases.
- Prove root-field controls and Multibrot controls are visible and active through no-mouse runtime tests.
- Validate, checkpoint, push, and stop before adapter work, SDF work, or additional fractal families.

## Current Phase

Phase 6 - hostile audit, receipt, rearward review, push, and clean closeout.

## Phase Checklist

- [x] Merge/push `codex/color-root-pattern-authority-repair` to `master`.
- [x] Branch `codex/explaino-multibrot-root-trap` from merged `master`.
- [x] Validate this plan/contract and lock the active slice.
- [x] Phase 1: add RED tests proving `explaino_multibrot_root_trap` selector/control/formula support is absent.
- [x] Phase 2: implement enum/catalog/schema/binding/report/state surfaces for the new lane.
- [x] Phase 3: route recurrence through the existing Multibrot direct formula with `multibrot_power_float` and `multibrot_power_imag`.
- [x] Phase 4: add native proof for zero-strength parity and nonzero root-trap sensitivity.
- [x] Phase 5: publish runtime and add no-mouse proof for selector identity, Multibrot exponent controls, root controls, capture/replay, and trap sensitivity.
- [ ] Phase 6: hostile audit, validation receipts, contract proof receipt, rearward review, push, clean tree, and stop.

## Scope

In scope:

- New append-only selector `explaino_multibrot_root_trap` in `Explaino Experiments`.
- Multibrot controls on the new lane: `multibrot_power_float` and `multibrot_power_imag`.
- Scoped Dynamics Root Field controls/actions and Root Trap Strength/Scale.
- Reports and Capture Finding sidecar truth for base fractal `multibrot`, exponent values, root-field consumer state, and root hashes.

Out of scope:

- Adapter-library expansion, graph UI, SDF operator work, perturbation work, extra root-field consumers, and new non-SDF source producers.
- Any recurrence modification beyond using the existing Multibrot formula path.
- Any physical mouse automation.

## Implementation Notes

- Existing root consumers map `explaino_mandelbrot_root_trap` to `mandelbrot` and `explaino_magnet_root_well` to `magnet` inside the shared direct formula helpers.
- This slice must add the same base-type mapping for `explaino_multibrot_root_trap -> multibrot`.
- Zero trap strength must be exact against baseline Multibrot for:
  - `multibrot_power_float=3.0`, `multibrot_power_imag=0.0`;
  - `multibrot_power_float=1.5`, `multibrot_power_imag=0.0`;
  - `multibrot_power_float=16.0`, `multibrot_power_imag=0.0`;
  - a nonzero imaginary exponent case.
- Nonzero trap strength must change the output without changing recurrence ownership.

## Proof Ledger

| Item | Evidence |
| --- | --- |
| Preflight merge | `master` fast-forwarded from `418d0a8` to `72057da` and pushed to `origin/master`. |
| Branch | `codex/explaino-multibrot-root-trap` from `72057da`. |
| Contract bootstrap | `artifacts/validation/explaino_multibrot_root_trap_contract_bootstrap.json`; `tools/viewer_host_assert_phased_plan_sync.py` passed. |
| RED proof | `artifacts/logged_commands/explaino_multibrot_root_trap_red_focused.json` failed on missing `FractalType::explaino_multibrot_root_trap`, proving the lane/control/formula support was absent before implementation. |
| Focused native proof | `artifacts/logged_commands/explaino_multibrot_root_trap_green_focused_6.json` passed schema binding, runtime validation, escape-time coloring, UI schema, safe-mode schema, and param animation rails. |
| Adjacent native proof | `artifacts/logged_commands/explaino_multibrot_root_trap_green_adjacent_native_2.json` passed diagnostics state IO, family rules, catalog authority, and fractal types after the catalog audit fix. |
| Capture/report proof | `artifacts/logged_commands/explaino_multibrot_root_trap_green_capture_report.json` passed diagnostics capture and automation report; `artifacts/logged_commands/explaino_multibrot_root_trap_green_report_after_audit_2.json` passed the report rail after explicit exponent fields were added. |
| Full native helper proof | `artifacts/logged_commands/explaino_multibrot_root_trap_native_full.json` passed the full `ui_app/build_tests_vsdevcmd.cmd` helper suite, including `test_escape_time_direct_formulas`. |
| Runtime publish | `artifacts/logged_commands/explaino_multibrot_root_trap_runtime_publish_after_audit.json` passed `ui_app/build_vsdevcmd.cmd`, publishing `D:\salt-fractal\cuda_newton_fractal_clone\runtime\fractal_ui.exe`. |
| Published runtime proof | `artifacts/logged_commands/explaino_multibrot_root_trap_runtime_pytest_after_audit.json` passed 12 no-mouse root-field consumer tests in `tests/test_fractal_runtime_root_field_consumers.py`, including Multibrot exponent report assertions and capture/replay cases. |
| Contract validation | `artifacts/validation/explaino_multibrot_root_trap_contract.json` passed. |
| Code quality | `artifacts/validation/explaino_multibrot_root_trap_code_quality.json` passed after removing the `sample_tier_resolver.cpp` and `fractal_family_rules.h` function-length regressions. |
| Diff check | `artifacts/validation/explaino_multibrot_root_trap_diff_check.json` passed. |
| Contract native proof | `artifacts/validation/explaino_multibrot_root_trap_native.json` passed the contract-focused native rail: family rules, diagnostics state IO, runtime validation, escape-time coloring, renderer, schema binding, UI schema, and automation report. |
| Contract runtime publish proof | `artifacts/validation/explaino_multibrot_root_trap_runtime_publish.json` passed `ui_app/build_vsdevcmd.cmd`. |
| Contract runtime JUnit proof | `artifacts/pytest/explaino_multibrot_root_trap_runtime.junit.xml` passed 12 no-mouse root-field consumer tests. |

## Hostile Audit

- Status: complete

Audit questions:

- Did `explaino_multibrot_root_trap` actually use the existing Multibrot direct formula instead of falling back to Mandelbrot?
- Did the new lane preserve real and complex Multibrot exponent controls, validation, state, and visible authority?
- Did zero-strength parity cover integer, non-integer, above-old-cap, and complex exponent cases?
- Did the no-mouse runtime proof exercise real visible controls rather than only internal params?
- Did the slice avoid adapter work, SDF work, graph UI, perturbation work, and extra fractal lanes?

## Audit Passes

- [x] Pass 1 - found missing catalog rows and missing explicit Multibrot exponent report fields.
- [x] Pass 2 - found code-quality regressions in root-consumer helper surfaces and repaired them without raising the baseline.
- [x] Pass 3 - reread the final diff/proof ledger and found no remaining selector, exponent, trap, report, or scope drift.
- [x] Pass 4 - clean re-read of the repaired state after contract native proof, runtime publish proof, runtime JUnit proof, code-quality baseline, and diff check found no additional real defect.

## Audit Findings

- [x] Finding 1: `explaino_mandelbrot_root_trap` and `explaino_magnet_root_well` had enum IDs but no catalog rows; the new Multibrot lane would have landed on top of a broken catalog authority rail. Fixed by adding catalog rows for both existing root consumers plus the new Multibrot root trap, then reran catalog authority successfully.
- [x] Finding 2: capture `fractal-state.json` wrote Multibrot exponent values, but the automation/runtime report did not expose them explicitly for root-field consumers. Fixed by adding `root_field_consumer_multibrot_power_float` and `root_field_consumer_multibrot_power_imag` report fields plus native and published-runtime assertions.
- [x] Finding 3: adding the root-consumer lane initially increased max function lengths in `sample_tier_resolver.cpp` and `fractal_family_rules.h`. Fixed by routing through the base-fractal helper instead of expanding line-heavy switch/list surfaces; code-quality baseline now passes.

## Stop Point

Stop after the new lane is implemented, validated, hostile-audited, checkpointed, receipted, rearward-reviewed, pushed, and clean. Preplanned work for this slice is exhausted at that point; stop for replan before safe-adapter work, SDF field-generation optimization, new SDF ops, or additional fractal ideas.
