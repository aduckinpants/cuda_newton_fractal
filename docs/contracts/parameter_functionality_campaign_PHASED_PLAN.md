# Parameter Functionality Campaign

## Explicit User Asks

- Set up a sprint holder branch from `master` for the next parameter functionality campaign.
- Document the campaign through step 9 using the existing ordering.
- Use branch-per-feature execution as each slice starts, rather than letting the sprint become one monolithic feature branch.
- Keep this setup slice limited to campaign documentation, contract setup, validation, checkpoint, push, and clean-tree proof.
- Preserve the current working parameter API baseline from Batch 1; do not reopen or reimplement Batch 1 here.
- Keep physical mouse automation out of future runtime proof.

## Current Phase

Sprint holder is integrated through Step 6 at `32dd3e9`. The next implementation slice is Step 7, `codex/pfc-explaino-collatz-direct`.

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
- The next feature branch to start after the current integrated holder is `codex/pfc-explaino-collatz-direct`.

## Campaign Steps Through Step 9

1. [closed] `codex/pfc-collatz-transition-strength`: add standalone `collatz_transition_strength` with default-preserving behavior, owner-lane visibility, state IO, validation, native sample proof, and no-mouse runtime proof.
2. [closed] `codex/pfc-phoenix-parameterization`: expose the existing Phoenix `p` / seed-like parameterization only if code proves the runtime path already has a stable authority seam; otherwise stop with a documented blocker.
3. [closed] `codex/pfc-fixed-family-fold-mix`: add default-preserving fold/mix controls for fixed families, starting with `burning_ship_fold_mix`, `celtic_abs_mix`, and `perpendicular_fold_mix`, with explicit owner-lane visibility and no `explaino_all` ownership leakage.
4. [closed] `codex/pfc-parameter-descriptor-export`: add a deterministic parameter surface descriptor/export guardrail so future slider audits compare schema, binding, validation, state IO, runtime params, and animation applicability from one machine-readable surface.
5. [closed] `codex/pfc-explaino-y-epsilon`: revisited `explaino_y::epsilon`; current proof shows live sensitivity on the tuned native sample surface and a preview-stable no-mouse published runtime surface.
6. [closed] `codex/pfc-branch-dead-explaino-controls`: repaired `explaino_nova` warp/damping as authoritative visible controls and kept ambiguous branch-dead Explaino rows classified/deferred instead of re-exposing inert sliders.
7. [next] `codex/pfc-explaino-collatz-direct`: add an Explaino Collatz direct variant only after standalone Collatz control behavior is proven, with explicit selector identity and no umbrella ownership confusion.
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

## Next Slice Entry Criteria

- Campaign branch pushed and clean at the integrated holder head.
- Active Step 6 contract closed with receipts.
- New feature branch to create from `codex/parameter-functionality-campaign`: `codex/pfc-explaino-collatz-direct`.
- New checked-in Step 7 plan and contract must be created before code mutation.
- Step 7 must preserve standalone Collatz behavior and Explaino-all registry/common-axis behavior while adding only the explicitly bounded Explaino Collatz direct variant.

## Proof Ledger

- Setup start branch: `master`.
- Setup start head: `f0d7874`.
- Sprint holder branch: `codex/parameter-functionality-campaign`.
- Prior closed baseline: `parameter_functionality_followup_batch1` closed and pushed; this campaign starts after that baseline.
- Contract validation: `artifacts/validation/parameter_functionality_campaign_contract.json`.
- Phased-plan sync: `py -3.14 tools/viewer_host_assert_phased_plan_sync.py`.
- Code-quality baseline: `artifacts/validation/parameter_functionality_campaign_code_quality.json`, score 97/100, baseline check passed.
- Whitespace check: `py -3.14 tools/viewer_host_run_logged_command.py --label parameter_functionality_campaign_diff_check --log artifacts/logs/parameter_functionality_campaign_diff_check.log --out-json artifacts/validation/parameter_functionality_campaign_diff_check.json --heartbeat-seconds 30 --timeout-seconds 120 -- git diff --check`, passed.
- Receipt gate finding: the first receipt attempt found that explicit-path plan sync and raw `git diff --check` were not parseable evidence commands for contract proof; repaired by switching to the repo-recognized plan-sync command and a logged diff-check command.
- Hostile-audit validation: `artifacts/validation/parameter_functionality_campaign_hostile_audit.json`.
- Integrated through Step 5: `5b6950c` closed `pfc_explaino_y_epsilon` with native viewport-mapped sample proof and preview-stable no-mouse runtime proof.
- Integrated through Step 6: `32dd3e9` closed `pfc_branch_dead_explaino_controls`; `explaino_nova` now exposes and consumes `explaino_warp_strength` and `explaino_damping`, the native/full-helper/published-runtime proof is green, and ambiguous branch-dead rows remain classified/deferred.

## Hostile Audit

- Status: complete
- Did I document steps 1 through 9 using the established ordering? Yes; the plan lists steps 1 through 9 and explicitly defers equation-pack viewport integration and perturbation zoom beyond this campaign.
- Did I keep this setup to documentation/branch setup instead of feature implementation? Yes; diff scope is the handoff breadcrumb plus this plan and contract.
- Did I avoid pre-creating inactive feature branches? Yes; `git branch --list "codex/pfc-*"` returned no branches.
- Did I preserve Batch 1 as the baseline instead of reopening it? Yes; no Batch 1 implementation files or contracts were edited.
- Did I keep the holder plan current after integrated feature slices? Yes; it now names Step 7 as the next implementation branch and records Step 6 as closed at `32dd3e9`.

## Audit Passes

- [done] Pass 1: clean re-read confirmed the campaign order covers step 1 through step 9 and defers steps beyond 9.
- [done] Pass 2: no additional real issue found in branch state; the sprint branch exists and no inactive `codex/pfc-*` feature branch was created.
- [done] Pass 3: receipt-gate review found a real workflow mistake: two required validation commands did not have parseable contract-proof evidence. Repaired the contract command list and re-ran validation.
- [done] Pass 4: clean re-read confirmed the repaired command list has parseable evidence for contract validation, plan sync, hostile audit, code-quality, and diff-check proof.

## Audit Findings

- [x] Clean re-read confirmed no implementation file was touched during this workflow-only setup slice.
- [x] No additional real issue found in the campaign branch model: feature branches are named and sequenced, but not pre-created.
- [x] Real finding 1: the initial setup contract required explicit-path plan sync and raw `git diff --check`, which the contract-proof receipt could not parse as evidence. Repaired with the recognized plan-sync command and a logged diff-check command.
- [x] Clean re-read confirmed the repaired setup contract requires parseable evidence for every required validation command.
