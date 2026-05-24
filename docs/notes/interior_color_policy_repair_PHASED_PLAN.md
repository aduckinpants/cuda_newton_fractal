# Interior Color Policy Repair

## Current Phase

Closed - repaired on `master` and ready for final push/status verification.

## Phase Checklist

- [x] Phase 1 - create and lock the checked-in plan/contract
- [x] Phase 2 - reproduce the UX policy failure with black-contrast and source-stack witnesses
- [x] Phase 3 - repair `Interior Strength` semantics to mean black-to-palette interior authority
- [x] Phase 4 - validate native/runtime/color-inventory proof
- [x] Phase 5 - hostile audit, checkpoint, receipts, merge-back, push, and clean-tree closeout

## Explicit User Asks

- [done] Restore predictable interior contrast control; pure black interiors must be possible.
- [done] Avoid inconsistent behavior where the control applies in one color path but not another compatible Smooth Escape path.
- [done] Avoid a boring solid non-black interior with no direct Color Pipeline influence.
- [done] Do not broaden this into palette redesign, fractal formula changes, renderer replacement, or unrelated UI work.

## Scope

In scope:

- Reframe `color_smooth_escape_interior_strength` as Smooth Escape interior color strength: `0` is black, `1` is full palette/source color.
- Keep default behavior colored enough to avoid the prior yellow/flat default regression, while letting users dial all the way to black.
- Make the control apply consistently for Smooth Escape UI mode, including source-stack-backed Color Pipeline states.
- Add native and no-mouse runtime metric proof for black-at-zero and visible colored-at-one.

Out of scope:

- New palette rows.
- Histogram/adaptive normalization.
- New fractal formulas.
- New Color Pipeline editor architecture.
- Physical mouse automation.

## Proof Ledger

- Bootstrap: `py -3.14 tools/viewer_host_session_bootstrap.py --audit --tail-handoff 8` passed on `master` at `cb9367a`; tree clean/even with origin.
- User report: current non-black interior kills contrast, is inconsistent across color paths, and produces a boring solid fill with inadequate direct control.
- Suspected issue: the prior repair made the slider visible/authoritative but kept a weak semantic model: muted-palette anchor instead of explicit black-to-palette interior authority.
- RED: `artifacts/interior_color_policy/red_current/metrics.json` showed `Interior Strength = 0` produced `black_pixel_fraction=0.0`, proving the slider could not restore pure black contrast.
- Native final: `artifacts/validation/interior_color_policy_native_final.json` passed `ui_app\build_tests_vsdevcmd.cmd advanced_color_grading_owner`.
- Runtime publish: `artifacts/validation/interior_color_policy_runtime_publish.json` passed `ui_app\build_vsdevcmd.cmd`.
- Runtime final: `artifacts/pytest/interior_color_policy_runtime.junit.xml` passed `tests/test_fractal_runtime_multibrot_interior_tone.py` with four tests, including Smooth Escape and Escape Magnitude Source stack metric cases.
- Inventory final: `artifacts/validation/interior_color_policy_inventory.json` passed `smooth_escape_color_inventory.py` for 18 cases.
- Hostile audit: `artifacts/validation/interior_color_policy_hostile_audit.json` passed after recording the contrast endpoint and inconsistent source-row findings.
- Checkpoint: `ccf1167` committed the repair through `viewer_host_checkpoint_slice.py`, with validation and contract proof receipts written for that head.
- Merge-back: `master` fast-forwarded to the repaired commit; stale-plan grep found no forbidden stale closeout phrases.

## Hostile Audit

- Status: complete

## Audit Passes

- [x] Pass 1 - proved black interiors are intentionally reachable through `Interior Strength = 0`.
- [x] Pass 2 - clean re-read proved `Interior Strength = 1` remains palette/source controlled and visibly different.
- [x] Pass 3 - clean re-read confirmed Smooth Escape and Escape Magnitude source-stack paths do not bypass the interior control.

## Audit Findings

- [x] Real finding - the previous semantics made `Interior Strength = 0` a muted palette anchor, not black, so the user could not recover the old contrast endpoint.
- [x] Real finding - the previous guard applied the control by `ColorSignal::smooth_escape`, so Smooth Escape mode using an Escape Magnitude Source row could still behave inconsistently.
- [x] Clean follow-up - re-read the repaired state and found no additional real issue in the bounded policy: zero maps to black, one maps to the active palette/source color, and the rule remains limited to Smooth Escape mode/source authority.

## Action Hostile Review

- Action ID: interior-color-policy-repair
- Suspected failure mode: a slider that can change pixels still gives poor UX if it hides the black contrast endpoint, only works in one compatible color path, or leaves non-black interiors as a dull fixed fill.
- Correct owner/action: keep the existing field/control, make its endpoints concrete and predictable, and prove the visible frame behavior with metric runtime tests.
- Proof surface: native escape-time color tests, no-mouse runtime frame metrics, smooth-escape inventory, hostile audit validator, code-quality baseline, diff check.
