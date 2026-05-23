# Fractal Catalog Authority Sprint Setup

## Current Phase

Complete - `master` contains the parameter functionality campaign, and Slice A/Slice B planning docs are ready for clean future implementation branches.

## Phase Checklist

- [x] Phase 0 - merge `codex/parameter-functionality-campaign` into `master` with a fast-forward integration and push `master`.
- [x] Phase 1 - create this docs-only setup plan and contract.
- [x] Phase 2 - create Slice A and Slice B plans/contracts with explicit boundaries.
- [x] Phase 3 - cross-link the prepared slices from deferred/status planning surfaces.
- [x] Phase 4 - validate contract shape, phased-plan sync, hostile audit, code-quality baseline, diff hygiene, checkpoint, receipts, and clean tree.

## Explicit User Asks

- [done] Merge the current parameter-functionality campaign branch into `master` first.
- [done] Set up documentation and plans for Slice A and Slice B so the next work can start cleanly.
- [done] Keep catalog/default authority useful for future fractal additions instead of creating another hand-edited maze.
- [done] Do not start perturbation zoom or new fractal implementation in this setup slice.

## Scope

In scope:
- Fast-forward local `master` to the closed campaign head and push `origin/master`.
- Add prepared plans/contracts for `fractal_catalog_authority_inventory` and `fractal_view_defaults_catalog_migration`.
- Update planning indexes so the next session can find the work without chat archaeology.

Out of scope:
- Product code, schema, renderer, tests, runtime harness, Color Pipeline, pacing, capture behavior, perturbation zoom, and new fractal formulas.
- Claiming Slice A or Slice B implementation is complete.

## Current Repo Truth Inputs

- Bootstrap before merge reported branch `codex/parameter-functionality-campaign`, head `8785da0`, clean tree, and active closed contract `deferred_backlog_reprioritization`.
- `master` was an ancestor of `codex/parameter-functionality-campaign`, so the integration was a fast-forward from `f0d7874` to `8785da0`.
- The backlog keeps catalog/default authority and perturbation expansion as separate follow-ons.
- Current defaults still concentrate in `ui_app/src/fractal_derived_fields.cpp`; perturbation remains Mandelbrot/Julia-only through `ui_app/src/perturbation_reference_orbit.h`.

## Prepared Slice Map

Slice A - Fractal Catalog Authority Inventory:
- Goal: establish a typed, tested metadata row for every current `FractalType` without changing behavior.
- Product value: adding a supported 2D fractal gets a required catalog/default/capability path instead of scattered selector/default archaeology.
- Stop point: defaults preserved, inventory rows complete, missing-row tests fail closed.

Slice B - Fractal View Defaults Catalog Migration:
- Goal: move current single-view defaults out of brittle switch pressure and into the Slice A catalog authority.
- Product value: prepares categorized selector/view-preset UX without changing existing default views.
- Stop point: `ApplyFractalViewPresetDefaults(...)` remains behavior-compatible while reading from catalog-owned defaults.

## Proof Ledger

- Merge proof: `git merge --ff-only codex/parameter-functionality-campaign` fast-forwarded `master` from `f0d7874` to `8785da0`.
- Push proof: `git push origin master` updated `origin/master` to `8785da0`.
- Planning docs prepared: Slice A and Slice B plans/contracts exist and define scope, validation, hostile audit questions, and non-goals.
- Validation target: docs-only contract validation, phased-plan sync, hostile-audit validation for this setup plan, code-quality baseline, and diff hygiene.

## Hostile Audit

- Status: complete
- Did I actually merge the campaign into `master` before preparing new work? Yes: `master` fast-forwarded to `8785da0` and was pushed to `origin/master`.
- Did I create implementation plans rather than pretending implementation is complete? Yes: Slice A and Slice B plans are `Phase 0 - ready to start` with open implementation asks.
- Did I keep perturbation zoom separate? Yes: perturbation is documented only as a later proof campaign and is not included in Slice A/B implementation scope.
- Did I design the catalog work to help future fractal additions? Yes: Slice A requires one metadata row per `FractalType` and fail-closed coverage when future types lack catalog/default/schema coverage.

## Audit Passes

- [done] Pass 1 - found a workflow issue: the closed docs contract did not allow creating new plan files, so a temporary scope preflight was needed before adding Slice A/B docs.
- [done] Pass 2 - clean re-read confirmed the old backlog contract is restored to its original scope in this patch and the new setup contract owns the new planning docs.
- [done] Pass 3 - clean re-read confirmed Slice A does not change product behavior and Slice B does not add selector UX before catalog/default preservation exists.
- [done] Pass 4 - clean re-read confirmed perturbation zoom, new fractal formulas, Color Pipeline, FPS pacing, and capture finding are excluded from this setup slice.

## Audit Findings

- [done] Finding 1: active-contract friction could have left the closed backlog contract broadened. The setup patch restores the prior contract scope and moves future mutation authority to `fractal_catalog_authority_sprint_setup`.
- [done] Clean re-audit: no additional real issue found after separating setup authority from future Slice A/B implementation authority.
- [done] Clean re-read: no implementation scope slipped into this docs-only setup; Slice A/B remain prepared but not started.

## Notes

- The next implementation branch should start from `origin/master` at or after `8785da0`.
- Start with Slice A before Slice B. Slice B depends on the inventory/default-preservation rail from Slice A.
