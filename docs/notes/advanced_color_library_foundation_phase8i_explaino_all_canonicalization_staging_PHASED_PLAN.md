# Advanced Color Library Foundation Phase 8I - Explaino-all Canonicalization Staging

## Current Phase

Complete - the repo now carries a closed docs-only staging slice for `Explaino-all` canonicalization. This slice does not ship `Explaino-all`; it writes down the bounded execution order, the non-negotiable identity rules, and the reusable launch-anchor surface that future implementation prompts can point at instead of re-encoding the entire family-track doctrine inline.

## Phase Checklist

- [x] Phase 1 - prove the current repo still treats `Explaino-all` as planning inventory only, inspect the existing Explaino selector/default/control surfaces, and identify the exact architectural drift that a canonicalization lane must repair
- [x] Phase 2 - write one checked-in launch-anchor document plus a concrete staged slice map for `Explaino-all` canonicalization while keeping future per-session implementation prompts flexible
- [x] Phase 3 - link the new anchor/slice-map surfaces from the active restart authority, hostile-review the staged plan text, validate the docs-only contract, and close this staging slice cleanly without pretending `Explaino-all` is already shipped

## Explicit User Asks

- [done] Write down the `Explaino-all` forward plan in the repo instead of carrying it only in chat.
- [done] Stage the work as a small repo slice now, but keep the future implementation kickoffs flexible.
- [done] Move the mandatory always-there future prompt content into a checked-in document so later prompts can jump off that file into the rest of the repo.
- [done] Write the actual bounded slices out instead of leaving `Explaino-all` as one vague future bucket.
- [done] Keep this as planning/staging work only; do not accidentally ship `Explaino-all` in the same slice.

## Presumption Loop

The controlling risk is Explaino-family architectural drift, not missing generic advanced-color core work. The current repo still exposes a flat set of separate Explaino identities, still defaults to plain `explaino`, still gates several Explaino controls through per-variant `visible_if fractal_type == ...` checks, and still assigns variant defaults through hand-coded branches. That means adding later Explaino family axes by inertia would keep reintroducing selector drift, default drift, binding drift, and one-off UI/runtime seams.

The bounded hypothesis for this staging slice is that future `Explaino-all` implementation should be split into a small number of explicit slices with one checked-in launch anchor:

1. canonical public identity plus axis registry
2. legacy variant projection plus preset/alias truth
3. enforcement guardrails that stop future drift

An optional cleanup slice exists only if the migration leaves transitional debris that cannot be removed truthfully inside slices 1-3.

## Presumption Evidence

- `ui/fractal_binding_surface_v1.ui_schema.json` still defaults the selector to `explaino`, not `explaino_all`.
- The same schema still lists `explaino_ripple`, `explaino_splice`, `explaino_vortex`, `explaino_tension`, and `explaino_balance_void` as peer selector identities and still exposes several axis controls through per-variant `visible_if fractal_type == ...` gates.
- `ui_app/src/fractal_derived_fields.cpp` still applies Explaino variant defaults through hand-coded `if (fractalType == ...)` branches.
- `docs/notes/advanced_color_library_foundation_oracle_and_inventory.md` already states the desired direction: one meta-family with multiple neutral-default axes, public shipping identity stays neutral, and theme shorthand stays internal only.
- `docs/notes/advanced_color_feature_restart_inventory.md` already lists `Explaino-all` as a later family-track candidate after `ExplainO-BalanceVoid`, but before this slice it had no checked-in execution map or launch-anchor surface.

## Slice Map

### Slice 1 - Canonical Identity + Axis Registry

Goal:

- introduce `explaino_all` as the canonical public Explaino identity
- make it the first Explaino selector entry and default selection on load
- define one shared Explaino axis registry for all neutral-at-zero family axes currently represented by separate variants
- drive the `Explaino-all` control surface from that registry instead of per-variant one-off wiring

Must prove:

- `explaino_all` exists as a real selector/runtime identity
- the selector defaults to `explaino_all`
- all current Explaino family axes are surfaced in one data-driven registry
- zero/default hiding behavior is explicit and testable

Must not do:

- no legacy alias migration yet unless strictly required to make `explaino_all` viable
- no broad cleanup of old variants in this slice

### Slice 2 - Legacy Variant Projection + Preset Truth

Goal:

- make old Explaino variant identities project to `explaino_all` plus preset axis coordinates
- keep old variant ids as compatibility selectors or preset shorthand, not as the architectural center

Must prove:

- selecting a legacy Explaino variant yields the same runtime/persistence truth as the matching `explaino_all` preset vector
- neutral/default collapse remains truthful
- no duplicate runtime ownership path survives just because a legacy label still exists

Must not do:

- no new family axes in this slice unless they are already part of the canonical registry
- no widening into unrelated Explaino or generic color-pipeline work

### Slice 3 - Enforcement Guardrails

Goal:

- make future Explaino-family drift fail closed by architecture instead of by memory

Must prove:

- adding a new Explaino axis without updating the canonical registry fails tests
- adding a new legacy variant or preset without canonical projection fails tests
- neutral-collapse and canonical/default invariants stay enforced

Must not do:

- no broad workflow-tooling redesign
- no fake enforcement that only checks labels while runtime/binding drift remains possible

### Optional Slice 4 - Transitional Cleanup

Only open this if slices 1-3 leave truthful residual cleanup that is too large or risky to absorb in the preceding slice.

Valid targets:

- remove dead per-variant UI branches that are no longer authoritative
- simplify stale helper paths that only existed for the pre-canonical Explaino layout
- tighten docs/help text after the canonicalization behavior is already proven

Invalid targets:

- reopening canonical identity design
- sneaking in new family axes or new public identities

## Launch Anchor

Future implementation prompts should point first at:

- [advanced_color_library_foundation_explaino_all_launch_anchor.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_explaino_all_launch_anchor.md)

That file is the reusable entry surface for:

- mandatory invariants that must always appear in a future `Explaino-all` kickoff
- the slice order
- required owner seams
- required stale-plan gate
- what must remain flexible and decided live at kickoff time

## Proof Ledger

- Current restart authority still lists `Explaino-all` as a later family-track candidate after `ExplainO-BalanceVoid`.
- Current UI selector/default surface still points at plain `explaino` rather than a canonical `explaino_all` identity.
- Current Explaino family controls still mix shared Explaino controls with per-variant `visible_if` wiring for newer family axes.
- Current Explaino defaults still rely on hand-coded variant branches instead of a shared canonical registry.
- This slice adds one reusable launch-anchor doc plus one concrete slice map, and links them from the active restart surfaces so later prompts can be shorter without becoming under-specified.

## Hostile Audit

- Status: complete
- Required posture: assume the staging packet is still too vague, too rigid, or quietly widening scope until the checked-in slice map proves it names the real drift seam, keeps the future kickoff flexible, and does not pretend `Explaino-all` is already underway or shipped.

## Audit Passes

- [x] Pass 1 - re-read the current Explaino selector/default/control surfaces and prove the repo still has separate peer variant identities plus per-variant UI gating.
- [x] Pass 2 - write the staged slice map and launch anchor as if future prompts will try to stay short; ensure the anchor still carries the non-negotiable invariants without freezing every later kickoff into one brittle prompt template.
- [x] Pass 3 - re-read the final docs and active restart links as if they still leave `Explaino-all` as one vague bucket or as if they silently pre-commit all later contracts; neither defect remains in the staged packet.
- [x] Pass 4 - after docs-only validation, perform one clean re-read of the staged plan, launch anchor, and restart-surface links as if the packet still either understates the mandatory invariants or over-hardcodes the future kickoffs; the committed docs now prove the intended middle ground cleanly.

## Audit Findings

- [x] Real staging gap fixed: `Explaino-all` was listed in restart authority but had no checked-in slice map or reusable kickoff anchor, forcing future prompts to restate doctrine from scratch.
- [x] Real architecture seam named: the current drift is default-selector and per-variant control wiring, not missing generic advanced-color core work.
- [x] Flexibility preserved: the staged packet fixes the always-there invariants and slice order, but it does not pre-bake future implementation contracts or pretend later kickoff prompts can ignore live repo state.
- [x] Clean re-read evidence: after contract validation and phased-plan sync, the staged packet still reads truthfully on reread. The launch anchor preserves the non-negotiable Explaino-all identity rules and kickoff floor, the slice map stays bounded to three primary implementation slices plus one optional cleanup slice, and the restart surfaces now point future prompts at checked-in repo authority instead of chat reconstruction.

## Notes

- This slice does not open or ship `Explaino-all`.
- This slice does not reopen generic advanced-color core work.
- This slice does not reopen historical archive compatibility work.
- This slice does not pre-commit future implementation contracts; those should still be created at actual kickoff time for the chosen slice.

## Resume Point

Closed. If `Explaino-all` is chosen as the next product lane, start from [advanced_color_library_foundation_explaino_all_launch_anchor.md](/C:/code/cuda_newton_fractal_clone/docs/notes/advanced_color_library_foundation_explaino_all_launch_anchor.md) and this staged slice map, then open the first real implementation slice for canonical identity plus axis registry.

## Action Hostile Review

- Action ID: action-20260514-explaino-all-canonicalization-staging
- Suspected Failure Mode: the repo gets another planning note that still leaves `Explaino-all` as a vague future bucket, or the launch anchor over-hardcodes future kickoff prompts so tightly that later sessions cannot adapt to live repo drift.
- Correct Owner/Action: stage the invariant doctrine and bounded slice order in checked-in docs, but keep future implementation contracts and per-session kickoff details live and slice-specific.
- Proof Surface: this plan, the launch-anchor doc, the active restart-authority links, docs-only contract validation, phased-plan sync, hostile-audit validation, committed reread, and clean repo state.
- Blocked Action: shipping `Explaino-all`, opening `Explaino-all` aliases/presets/runtime work directly, or widening into generic-core or archive-compatibility work from this staging slice.
