# Parameter Functionality Campaign

## Explicit User Asks

- Set up a sprint holder branch from `master` for the next parameter functionality campaign.
- Document the campaign through step 9 using the existing ordering.
- Use branch-per-feature execution as each slice starts, rather than letting the sprint become one monolithic feature branch.
- Keep this setup slice limited to campaign documentation, contract setup, validation, checkpoint, push, and clean-tree proof.
- Preserve the current working parameter API baseline from Batch 1; do not reopen or reimplement Batch 1 here.
- Keep physical mouse automation out of future runtime proof.

## Current Phase

Phase 0 is closed: the campaign branch and checked-in campaign plan/contract are set up through step 9. No feature implementation has started.

## Phase Checklist

- [x] Bootstrap and confirm repo branch, head, upstream, and clean state.
- [x] Create sprint holder branch `codex/parameter-functionality-campaign` from current `master`.
- [x] Create this checked-in campaign plan and workflow-only setup contract.
- [x] Lock the active slice to this campaign setup contract.
- [x] Validate contract, phased-plan sync, hostile audit, code-quality baseline, and whitespace.
- [x] Checkpoint the setup branch, write receipts, push the campaign branch, and verify clean tree.
- [x] Gate the first implementation slice so it starts only after the setup branch is pushed and clean.

## Campaign Branch Model

- Sprint holder: `codex/parameter-functionality-campaign`.
- Source: current `master` at setup start, `f0d7874`.
- Feature branches are created from the sprint holder when their slice begins.
- Each feature branch gets its own plan, contract, focused tests, hostile audit, commit, receipts, push, clean-tree closeout, and merge back into the sprint holder.
- The sprint holder is the integration branch for this campaign only. It is not a license to combine unrelated feature work.
- The first feature branch to start after this setup is `codex/pfc-collatz-transition-strength`.

## Campaign Steps Through Step 9

1. `codex/pfc-collatz-transition-strength`: add standalone `collatz_transition_strength` with default-preserving behavior, owner-lane visibility, state IO, validation, native sample proof, and no-mouse runtime proof.
2. `codex/pfc-phoenix-parameterization`: expose the existing Phoenix `p` / seed-like parameterization only if code proves the runtime path already has a stable authority seam; otherwise stop with a documented blocker.
3. `codex/pfc-fixed-family-fold-mix`: add default-preserving fold/mix controls for fixed families, starting with `burning_ship_fold_mix`, `celtic_abs_mix`, and `perpendicular_fold_mix`, with explicit owner-lane visibility and no `explaino_all` ownership leakage.
4. `codex/pfc-parameter-descriptor-export`: add a deterministic parameter surface descriptor/export guardrail so future slider audits compare schema, binding, validation, state IO, runtime params, and animation applicability from one machine-readable surface.
5. `codex/pfc-explaino-y-epsilon`: revisit `explaino_y::epsilon` with a bounded tuned witness search and either prove live sensitivity, narrow the witness, or leave it honestly classified as unresolved/weak.
6. `codex/pfc-branch-dead-explaino-controls`: audit and repair branch-dead Explaino controls where public sliders exist but the active runtime branch does not authoritatively consume them.
7. `codex/pfc-explaino-collatz-direct`: add an Explaino Collatz direct variant only after standalone Collatz control behavior is proven, with explicit selector identity and no umbrella ownership confusion.
8. `codex/pfc-explaino-julia-authority`: design and implement Explaino Julia direct constants behind an explicit seeded/custom authority mode instead of silently repurposing current defaults.
9. `codex/pfc-generated-internal-editors`: expose generated/internal editor authority through a safe override-mode UI, not raw array dumping, and prove saved-state compatibility plus runtime authority.

## Deferred Beyond Step 9

- Equation-pack main viewport integration remains its own feature line.
- Perturbation zoom remains its own major engine/rendering project.
- Broad engine redesign, Color Pipeline changes, capture finding work, and FPS pacing work are not part of this campaign setup.
- IFS, 3D/SDF, density estimators, and other next-gen families stay in later campaigns unless a step above explicitly unblocks a narrow preparatory seam.

## Step Prioritization

- Highest reward / low estimate: Steps 1 and 2. These should provide visible user-facing controls with limited engine risk if runtime authority is already present.
- High reward / moderate estimate: Step 3. It fixes multiple fixed-family exploration gaps with one repeated owner-lane pattern.
- High leverage / moderate estimate: Step 4. It reduces repeat churn by making parameter-surface completeness auditable before future feature code lands.
- Medium reward / small-to-medium estimate: Step 5. It resolves a known weak witness without pretending a control is dead when proof is insufficient.
- High reward / moderate-to-large estimate: Step 6. It repairs the class of regressions where sliders are visible but do nothing.
- High reward / larger estimate: Steps 7 and 8. These add meaningful Explaino variants but need authority-mode clarity.
- Medium-to-high reward / large estimate: Step 9. It unlocks more power, but must wait until safe override authority is specified tightly enough to avoid another raw-control mess.

## Per-Slice Proof Rules

- Native schema/binding/state/validation tests must exist for every new or repaired public control.
- Runtime proof must use in-process/no-mouse automation and keep selector identity on the owning lane.
- A slider is not accepted as working until it changes the rendered/sample output through the shipped public path or is explicitly classified as unresolved with proof.
- Defaults must preserve current visual behavior unless the slice contract says otherwise.
- Owner-specific controls must not be added to `explaino_all` unless the slice explicitly proves they are registry/common-axis controls.
- Color Pipeline, capture finding, and FPS pacing must not regress when shared schema or automation harness paths are touched.

## First Slice Entry Criteria

- Campaign branch pushed and clean.
- Active setup contract closed with receipts.
- New feature branch created from `codex/parameter-functionality-campaign`: `codex/pfc-collatz-transition-strength`.
- New checked-in first-slice plan and contract created before code mutation.
- RED tests prove `collatz_transition_strength` is missing from schema/binding/state/runtime proof before implementation.

## Proof Ledger

- Setup start branch: `master`.
- Setup start head: `f0d7874`.
- Sprint holder branch: `codex/parameter-functionality-campaign`.
- Prior closed baseline: `parameter_functionality_followup_batch1` closed and pushed; this campaign starts after that baseline.
- Contract validation: `artifacts/validation/parameter_functionality_campaign_contract.json`.
- Phased-plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py docs/contracts/parameter_functionality_campaign_PHASED_PLAN.md`.
- Code-quality baseline: `artifacts/validation/parameter_functionality_campaign_code_quality.json`, score 97/100, baseline check passed.
- Whitespace check: `git diff --check`, passed.
- Hostile-audit validation: `artifacts/validation/parameter_functionality_campaign_hostile_audit.json`.

## Hostile Audit

- Status: complete
- Did I document steps 1 through 9 using the established ordering? Yes; the plan lists steps 1 through 9 and explicitly defers equation-pack viewport integration and perturbation zoom beyond this campaign.
- Did I keep this setup to documentation/branch setup instead of feature implementation? Yes; diff scope is the handoff breadcrumb plus this plan and contract.
- Did I avoid pre-creating inactive feature branches? Yes; `git branch --list "codex/pfc-*"` returned no branches.
- Did I preserve Batch 1 as the baseline instead of reopening it? Yes; no Batch 1 implementation files or contracts were edited.
- Did I make the first implementation slice ready without claiming it is already done? Yes; the first slice branch and entry criteria are documented, but no first-slice code work is claimed.

## Audit Passes

- [done] Pass 1: clean re-read confirmed the campaign order covers step 1 through step 9 and defers steps beyond 9.
- [done] Pass 2: no additional real issue found in branch state; the sprint branch exists and no inactive `codex/pfc-*` feature branch was created.
- [done] Pass 3: no additional workflow mistake found after contract, plan-sync, code-quality, and whitespace validation.

## Audit Findings

- [x] Clean re-read confirmed no implementation file was touched during this workflow-only setup slice.
- [x] No additional real issue found in the campaign branch model: feature branches are named and sequenced, but not pre-created.
- [x] No additional workflow mistake found in the setup contract: it requires contract validation, plan sync, hostile audit, code-quality baseline, and whitespace proof.
