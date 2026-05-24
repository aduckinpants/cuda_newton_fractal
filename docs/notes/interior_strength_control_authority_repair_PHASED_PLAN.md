# Interior Strength Control Authority Repair

## Current Phase

Phase 5 - hostile audit, checkpoint, receipts, push, merge-back, stale-plan grep, and clean-tree closeout.

## Phase Checklist

- [x] Phase 1 - create and lock the checked-in continuation plan/contract
- [x] Phase 2 - reproduce the weak/no-op user-facing slider behavior with a stronger native/runtime witness
- [x] Phase 3 - repair the `Interior Strength` control so low-to-high edits visibly affect smooth-escape interiors
- [x] Phase 4 - validate native color/schema/runtime proof and preserve smooth-escape inventory guardrails
- [ ] Phase 5 - hostile audit, checkpoint, receipts, push, merge-back, stale-plan grep, and clean-tree closeout

## Explicit User Asks

- [done] Treat the reported `Interior Strength` slider as not working from the user-facing perspective.
- [done] Do not rely on weak hash-only proof if the visible change is too small to see.
- [done] Keep the broader escape-time interior smoke and recent smooth-escape inventory repairs intact.
- [done] Keep Color Pipeline behavior intact; do not break source-stack editing or row automation.
- [done] Do not start palette redesign, formula changes, camera work, SDF work, or broad renderer work.

## Scope

In scope:

- Strengthen tests to measure visible pixel/color-distance impact, not only frame-hash movement.
- Repair the smooth-escape interior-strength mapping so the control has an obvious effect on interior-dominant views.
- Preserve the public Color panel control id and persisted field name.
- Preserve source-stack and advanced Color Pipeline semantics unless current code proves that is exactly the blocker.
- Preserve previous smooth-escape inventory guardrails: no high black, no low luma span, no low unique-color cases.

Out of scope:

- New palettes.
- Histogram/adaptive normalization.
- Formula changes.
- Selector/view preset work.
- SDF work.
- Physical mouse automation.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on `master` at `38ee801`; tree clean/even with origin.
- User report: the visible `Interior Strength` slider "does nothing that I can tell."
- Suspected proof gap: prior runtime test only required frame hash change, which can pass on an imperceptible or incidental frame difference.
- RED: `artifacts/interior_strength_control/source_stack_repro/metrics.json` showed a one-row smooth-escape Source stack made `Interior Strength` fully inert: `avg_abs_rgb_delta_per_channel=0.0`, `changed_pixel_fraction=0.0`, `max_rgb_sum_delta=0`.
- Bound correction: keep the control gated to smooth-escape source authority, but stop bypassing it only because the Color Pipeline Source stack is active.
- Native final: `artifacts/validation/interior_strength_control_native_final.json` passed `ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner`.
- Runtime publish: `artifacts/validation/interior_strength_control_runtime_publish.json` passed `ui_app\build_vsdevcmd.cmd`.
- Runtime final: `artifacts/pytest/interior_strength_control_runtime.junit.xml` passed `tests/test_fractal_runtime_multibrot_interior_tone.py`; the test now covers no-mouse set-value on a source-stack state and a pixel-delta proof.
- Inventory final: `artifacts/validation/interior_strength_control_inventory.json` passed `smooth_escape_color_inventory.py` for 18 cases.
- Primary seams: `ui_app/src/escape_time_coloring.h`, `ui_app/tests/test_escape_time_coloring.cpp`, `tests/test_fractal_runtime_multibrot_interior_tone.py`, and any schema/binding files only if live proof shows the control is not actually wired.

## Hostile Audit

- Status: complete

## Audit Passes

- [x] Pass 1 - verified the repaired control changes actual visible interior color/luma/chroma by a bounded threshold.
- [x] Pass 2 - clean re-read of the repaired state verified the repair does not reintroduce black interiors or yellow-dominant defaults.
- [x] Pass 3 - clean re-read confirmed the repaired state keeps Color Pipeline source-stack and recent smooth-escape inventory guardrails green.

## Audit Findings

- [x] Real finding - the first implementation made `Interior Strength` conditional on `color_source_stack_count == 0`, so a Smooth Escape Source stack made the slider inert even though the Color panel still exposed it.
- [x] Test finding - the prior runtime proof accepted a frame-hash change and did not measure pixel/color distance, so it could not prove a visible control.
- [x] Clean follow-up - re-read the repaired state and found no additional real defect in the bounded guard; it remains limited to `ColorSignal::smooth_escape`, so non-smooth final Source rows are not widened into this control path.

## Action Hostile Review

- Action ID: interior-strength-visible-authority
- Suspected failure mode: a slider can technically alter a frame hash while remaining visually useless, especially if the muted anchor and full interior color are too close, source-stack semantics skip the blend, or the user-facing view is dominated by interior pixels whose signal collapses to a narrow range.
- Correct owner/action: prove visible impact with numeric frame/color metrics, repair only the interior-strength mapping or binding seam that is actually failing, and keep prior color inventory rails green.
- Proof surface: stronger native color-distance assertions, no-mouse runtime frame metric proof, runtime publish, smooth-escape inventory, hostile-audit validator, code-quality baseline, diff check.
