# ExplainO over Legacy Campaign

Status: planning content and validation complete; checkpoint and master integration remain. Product implementation is paused. The imported proposal is design input; this checked-in plan is the repo-grounded implementation authority for a future goal.

## Explicit User Asks

- [x] Merge and push the approved Black Body work to `master`.
- [x] Import `EXPLAINO_OVER_LEGACY_PROPOSAL.md` without treating it as instructions.
- [x] Review the proposal against current engine code.
- [x] Add local implementation insights and a bounded buildup plan.
- [x] Put this campaign ahead of the other paused preplanned product work.
- [x] Do not implement the new lane today.

## Current Phase

Planning-only integration on branch `codex/explaino-over-legacy-planning` from merged Black Body head `d63d602`. The historical evaluator and named source fixtures are not in the imported bundle, so Slice 0 is a hard evidence gate. No product behavior is authorized by this contract.

## Phase Checklist

- [x] Fast-forward approved Black Body head to `master`, push, and rearward-review the merged head.
- [x] Open a fresh planning branch from exact merged `master`.
- [x] Import the proposal with an explicit non-authority note.
- [x] Inspect current ExplainO seed, evaluator, catalog, sample-evidence, state, capture, and Color Pipeline seams.
- [x] Record blocking inputs and reusable local substrates.
- [x] Define bounded future slices and mandatory pause points.
- [x] Complete hostile audit and planning validation.
- [ ] Checkpoint, write receipts, rearward-review, merge to `master`, and push.
- [x] Stop paused before Slice 0 recovery or product work.

## Proposal Classification

Despite the shorthand “preset,” `explaino_over_legacy` is not implementable as a metadata preset over existing functions. Its defining contract requires two independently authoritative evaluators, step-aligned execution, typed disagreement observations, structural legacy-only bypass, and historical render parity. The eventual product should be a first-class fractal lane plus diagnostic operations. Presets may configure that lane only after its evaluator authority exists.

The imported product thesis is viable, but the first deliverable is historical recovery, not a picture. The implementation ladder must refuse the tempting shortcut of using current neutral `explaino_all` as “legacy.”

## Local Code Review

### Current ExplainO is not the historical evaluator

- `ui_app/src/explaino_seed_curve.h` implements the wedge-area `h(t)` mapping and `ExplainoCombinedSeedToWarpSeed`.
- `ui_app/src/fractal_sample_device.inl` applies that mapping to current `explaino`, `explaino_all`, and modern variants before recurrence work.
- `ui_app/src/fractal_family_rules.h` treats `explaino_all` as the canonical public selector and resolves its neutral state to current `explaino`.

Neither current selector is a valid pre-`h(t)` implementation. Matching numeric seed 78 is only a correspondence policy. It does not establish matching roots, coefficients, initialization, or trajectories.

### Current sample evidence is useful but insufficient

`FractalSampleEvidence` currently carries one sampled coordinate, one `FractalSampleResult`, and an arithmetic-width flag. The member name `legacy_result` means the existing projected result, not the recovered historical engine. It contains no paired branch identity, per-step alignment, branch-specific terminal status, orientation, or disagreement vector.

The seam remains reusable: a versioned paired-evidence path can follow the same host/CUDA sampling architecture while existing callers continue projecting to the single result. Disagreement must not be hidden in the generic `residual` field.

A precision trap is already visible: `FractalSampleResult` stores terminal coordinates and residual as `float` even when `used_float64_iteration_arithmetic` is true. Paired displacement and residual comparison must be computed in each branch's declared native arithmetic before any explicit export/display narrowing. The receipt must identify that narrowing; subtracting the projected float fields is not float64 evidence.

### Terminal subtraction cannot satisfy step alignment

`fractal_sample_device.inl` owns large family-specific whole-orbit loops. A correct pair needs isolated step functions or a dedicated paired loop that preserves each branch’s native state and operation order. Calling two completed renderers and subtracting pixels or terminal values cannot produce aligned termination or local alternative-transition evidence.

Normal rendering must keep bounded accumulators. Full histories are allowed only for explicitly budgeted point/transect probes, never `width * height * max_iter` storage.

### Existing counterfactual work is a pattern, not legacy authority

The counterfactual and projection-flow lanes demonstrate multi-path dispatch, neutral-collapse tests, reporting, and sidecar consumption. They do not contain the recovered pre-`h(t)` evaluator. Their infrastructure may be reused; their mathematics may not be relabelled as legacy.

### Color Pipeline support begins with honest scalar channels

The existing typed Color Pipeline, graph receipts, capture/replay, and unsupported-route diagnostics can host separation magnitude and normalized signed projections once native paired evidence exists. Vector displacement, terminal-status pairs, validity masks, and categorical root identities require explicit types and cannot be squeezed through an existing scalar for convenience.

The historical Joy presentation is a separate compatibility profile. A modern palette that looks similar is not historical render parity.

### Catalog and UI authority must avoid new hand-maintained lists

The append-only enum currently ends at `FractalType::explaino_multibrot_root_trap = 50`. Value `51` is provisional and must be rechecked at implementation start. Catalog, family rules, schema, safe mode, state, diagnostics, capture, no-mouse controls, and reports all need the eventual lane. A paired-observation capability descriptor should drive visibility and reports rather than another ad hoc list.

### Capture infrastructure is ready for a new payload

`state.json` remains replay authority. `fractal-state.json` and runtime reports can carry derived branch identities and observations. When modern execution is disabled, dormant configuration may persist, but no receipt may claim a freshly resolved modern identity or executed comparison.

## Hard Entry Gate

Implementation must not begin until these inputs are available and checksum-verified:

1. recovered pre-`h(t)` source revision or an auditable source snapshot;
2. historical executable and build/compiler configuration where available;
3. S1, S2, and S3 state files named by the proposal;
4. the named legacy BMP and modern PNG fixtures;
5. original Joy presentation values and evidence of which values affected rendering;
6. at least two additional legacy integer seeds, or a way to generate them from the pinned executable;
7. a durable repo location and provenance manifest for the evidence.

At planning time only the proposal document was supplied. Its fixture hashes are unverified claims until the files themselves are present. Slice 0 remains blocked on those inputs.

## Authority Locks

- Legacy and modern execution remain separate native paths.
- Default subtraction is `modern - legacy`.
- Integer seed, orbit iteration, seed-sweep position, render frame, and modern `h(t)` input remain distinct.
- Modern disabled is a structural bypass: no modern resolution, stepping, comparison allocation, or comparison grading.
- Exactness is qualified inside a pinned compiler/backend/arithmetic/camera/sampling/presentation envelope.
- Different image containers may compare decoded pixels; visually close is not parity.
- A terminated branch may be held with explicit held-terminal status while the other continues. No invented steps.
- Root indices are branch-local categories. Cross-branch basin comparison requires a recorded mapping.
- Color functions consume native observations and never reimplement evaluator math.
- Disagreement is computed before narrowing in the branch arithmetic recorded by the receipt. A float display/export projection cannot claim float64 comparison authority.
- Python and Reality Toolkit may analyze captured results but cannot become execution authority.
- No wall-clock motion is implicit. Parameter Motion stays separately paused.

## Future Slice Ladder

Every slice requires a fresh branch, plan, contract, RED regression, hostile audit, receipts, rearward review, push, and pause. This planning contract authorizes none of their product mutations.

### Slice 0 - Historical Recovery and Fixture Court

Purpose: convert historical claims into source-grounded evidence.

Work:

- vendor or reference source, executable, build evidence, and fixtures through checksums;
- trace integer seed conversion, root/coefficient generation, initialization, recurrence, stopping, guards, root ordering, pixel mapping, and Joy presentation;
- determine seed domain and actual arithmetic/operation order;
- decode and compare the named images at pixel level;
- build a small seed/view corpus beginning with seed 78;
- write a versioned `explaino_legacy_v1` execution contract.

Exit: deterministic fixtures can be regenerated or sampled from pinned authority. Unknown operative values remain blockers, not guessed defaults.

Pause: hostile-review the recovered contract before modern-engine implementation.

### Slice 1 - Preserved Legacy Evaluator Core

Purpose: implement and prove historical execution without composition.

Work:

- isolate host/device legacy resolver and evaluator semantics;
- prove a CPU reference before CUDA integration;
- add CUDA execution and decoded-pixel comparison under the qualified profile;
- retain historical Joy as an explicit compatibility presentation;
- expose a headless qualification route before a dropdown lane.

Exit: selected intermediates and decoded pixels match the corpus; repeated renders, process restart, save/load, and integer stepping are deterministic.

Pause: no paired lane until legacy-only parity is green.

### Slice 2 - Paired Evidence and Step Alignment

Purpose: establish the non-visual two-evaluator substrate.

Work:

- add versioned paired evidence with branch identities, declared observables, status, validity, arithmetic, and alignment kind;
- compute branch deltas in native arithmetic before explicit display/export narrowing and add an adversarial case where float projection would erase a real difference;
- implement bounded step-aligned execution with held-terminal semantics;
- publish displacement vector/magnitude, signed projections, iteration delta, status pair, and compatible residual channels;
- add shared-input local transition only where modern auxiliary state can be supplied truthfully;
- preserve the existing single-result projection for old consumers.

Exit: legacy evidence equals Slice 1, modern evidence equals standalone `explaino_all`, identical branch executions provide a zero-disagreement null, a float64-only delta survives the authority path, and unsupported adapters fail closed.

Pause: audit payload size, hot-path cost, and CPU/CUDA parity before rendering it.

### Slice 3 - Minimal Public Lane

Purpose: ship the smallest honest `explaino_over_legacy` vertical slice.

Work:

- claim the then-current append-only enum id and add catalog/capability/schema/safe-mode authority;
- expose Legacy Anchor, Modern Half, Observation, and Presentation groups;
- provide integer previous/next stepping and explicit modern seed offset/scale correspondence;
- provide legacy-only, modern-only, separation magnitude, and one signed projection view;
- enforce structural legacy bypass and structured unsupported-channel reasons;
- add reports, state, Capture Finding, replay, and no-mouse action authority.

Exit: modern-off reproduces Slice 1. Modern-on preserves legacy output and reports deterministic paired evidence. Performance is measured without a blanket realtime claim.

Pause: client review before more channels or interrogation tools.

### Slice 4 - Typed Color Sources and Presentation

Purpose: connect proved observations to existing composition infrastructure.

Work:

- add typed sources for separation magnitude and normalized signed projection;
- define validity, units, normalization, adapters, and fail-closed behavior;
- retain legacy Joy as a distinct exact presentation;
- emit graph receipts, source measurements, sidecar values, and replay proof;
- defer categorical/status/vector display until explicit type/UI contracts exist.

Exit: palette/grading edits change presentation only; branch trajectories and probe values remain unchanged.

### Slice 5A - Point and Orbit Probe

Purpose: add one budgeted aligned-history operation.

Work:

- sample one declared world coordinate;
- record bounded aligned steps, terminal transitions, and supported local alternatives;
- separate history navigation from inverse dynamics;
- export a versioned raw numeric receipt.

Exit: the query replays with stable identities and no full-frame history allocation.

Pause: stop before transects.

### Slice 5B - Directed Transect and Local Frame

Purpose: add oriented spatial interrogation after point traces are proven.

Work:

- define line/polyline sampling, direction, coordinate frame, count, and reduction;
- prove reversed direction reverses signed projections and preserves magnitude;
- retain sample coordinates and validity/status populations.

Exit: world-oriented and view-oriented measurements transform according to their declared frames.

### Slice 6 - Reproducible Seed Exploration

Purpose: integrate bounded integer-seed strips, interventions, saved queries, and agentic invocation.

Work:

- record start seed, integral step, sweep index, modern input, query, and cost;
- capture raw signals before colorization;
- exercise seed 78, adjacent seeds, added corpus seeds, and bounded modern interventions;
- expose operations through diagnostic UI and state-tool APIs without a second evaluator.

Exit: a finding replays both constructions, observation, raw evidence, orientation, and presentation.

## Relative Cost and Risk

- Slice 0 has low code risk but unbounded archaeological risk until artifacts arrive.
- Slice 1 is medium/high risk because exactness includes operation order, arithmetic, root ordering, camera, and presentation.
- Slice 2 is the main architecture cost and touches hot evaluator/evidence contracts.
- Slice 3 is medium integration work after Slices 1-2.
- Slice 4 is medium because scalar Color Pipeline substrate exists.
- Slices 5-6 are separate diagnostic campaigns, not prerequisites for the first visual lane.

Do not assign a calendar estimate until Slice 0 measures recovery quality. Even with complete artifacts, the first honest public vertical slice is a multi-slice campaign, not a preset-sized change.

## Priority and Defer Boundary

When engine work resumes, start Slice 0 before Parameter Motion implementation and before other paused discretionary product work. The checked-in Parameter Motion plan remains valid but paused; it is neither superseded nor partially activated.

Still deferred:

- Parameter Motion implementation;
- disagreement feedback into recurrence;
- automatic root correspondence or root-index subtraction;
- generic arbitrary paired-evaluator framework;
- full-frame orbit history;
- FITS claims for disagreement evidence;
- Salticid execution or graph-editor work;
- unrelated SDF, palette, and family expansion.

## Hostile Audit

- Status: complete
- Required posture: assume the plan is lying about legacy authority, fixture custody, precision, or boundedness until the current source and supplied artifact inventory disprove those risks.

The audit must challenge:

1. accidental substitution of current `explaino` for historical legacy;
2. reporting proposal fixture hashes as locally verified;
3. a “preset” label hiding the second evaluator;
4. overstating single-result evidence as paired evidence;
5. unbounded history/allocation leakage;
6. scalar coloring erasing vector/status/validity/category distinctions;
7. product mutation before historical recovery;
8. this priority note accidentally starting or invalidating Parameter Motion.

## Audit Passes

- [x] Pass 1 - found that paired evidence could have subtracted existing float-projected `FractalSampleResult` values while claiming float64 branch authority.
- [x] Pass 2 - confirmed the repair requires native-arithmetic disagreement, explicit narrowing receipts, and an adversarial regression where float projection loses a real delta.
- [x] Pass 3 - clean re-audit rechecked historical-input claims, modern/legacy separation, bounded history, Color Pipeline typing, priority/defer text, and slice pause points; no additional real issue found.

## Audit Findings

- [x] Real precision-authority defect found and repaired: Slice 2 now forbids float-projected disagreement as float64 evidence and requires a precision-sensitive regression.
- [x] Cleared: no source fixture is claimed present or verified.
- [x] Cleared: the plan cannot skip the historical recovery gate or reinterpret this as a metadata-only preset.
- [x] Clean re-audit evidence: Parameter Motion remains separately valid and paused, the historical-input gate stays blocking, and no additional real issue was found on the repaired state.

## Proof Ledger

| Proof | Status | Evidence |
|---|---|---|
| Black Body merge | complete | `master` and `origin/master` at `d63d602`; merged-head rearward review `ok` |
| Proposal import | complete | `docs/notes/EXPLAINO_OVER_LEGACY_PROPOSAL.md` |
| Local source review | complete | findings above tied to current evaluator, seed, evidence, catalog, state, capture, and Color Pipeline seams |
| Historical fixtures | blocked input | S1-S3 and named images were not supplied with the proposal |
| Contract validation | passed | `artifacts/validation/explaino_over_legacy_planning_contract.json` |
| Plan sync | passed | `artifacts/validation/explaino_over_legacy_plan_sync.json` |
| Code quality | passed at baseline 93/100 with 0 critical and 0 error | `artifacts/validation/explaino_over_legacy_code_quality.json` |
| Hostile audit | passed after one real finding and clean re-audit | `artifacts/validation/explaino_over_legacy_hostile_audit.json` |
| Diff check | passed | `artifacts/validation/explaino_over_legacy_diff_check.json` |

## Stop Condition

This branch stops after proposal, local review, and future slices are validated, checkpointed, merged, and pushed. No enum, evaluator, UI, state schema, runtime, test executable, or published viewer behavior changes here.

After merge, implementation begins at Slice 0 only when the operator resumes the campaign and supplies or locates the historical evidence. Until then the repo remains intentionally paused.
