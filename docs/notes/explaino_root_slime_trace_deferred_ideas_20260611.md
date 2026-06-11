# ExplainO Root And Slime Trace Deferred Ideas 20260611

Status: deferred planning and grooming. Do not implement anything in this note without a fresh checked-in phased plan and contract.

## Summary

This note parks the useful ideas around ExplainO captured-root authority, root-layout analysis, seed hunting, sidecar/slime trace tooling, FITS/flashlight reuse, and RTK follow-ups before the next implementation campaign is finalized.

The near-term implementation candidate is intentionally narrow:

1. Repair analyzer authority so captured `params.explaino_roots` win over coefficient re-solving.
2. Harden sidecar mutation trace receipts enough to support future parameter-space slime analysis.
3. Only then build trace/analyzer/report tooling.

The rest of this note is deferred opportunity grooming, not permission to start a broad research/tooling campaign.

## Current Authority Facts

- Capture/state JSON already persists `params.explaino_root_authority`, `params.explaino_root_count`, and `params.explaino_roots`.
- `tools/reality_toolkit/fractal_explorer/finding_analyzer.py` still has paths that re-solve roots from coefficients when analyzing findings.
- Current `sidecar_mutation_history` is enough for ordered target replay, but not enough for rigorous golden-thread proof.
- Runtime-walk field slime and ExplainO sidecar slime are different systems:
  - runtime-walk field slime is world/image-field traversal;
  - ExplainO sidecar slime is parameter-policy traversal.
- FITS/flashlight tooling is useful as an input/visualization/reporting layer, not as the first root-authority repair seam.

## Recommended Near-Term Campaign Shape

### Slice 1 - Captured Root Analyzer Authority

Priority: P0, high benefit, low/medium effort.

Implement first because every later root-layout or seed-hunting tool depends on truthful root authority.

Acceptance direction:

- If `params.explaino_roots` exists and `params.explaino_root_count > 0`, analyzer output uses those exact roots.
- Preserve/report `params.explaino_root_authority`.
- Coefficient-derived roots are fallback-only and labeled `legacy_coefficients` or equivalent.
- No render behavior changes.

### Slice 2 - Sidecar Trace Receipt Inventory

Priority: P1, high benefit, low/medium effort.

Before building new slime/RTK tooling, inspect the current mutation history and define what is missing for deterministic trace analysis.

Likely missing receipt fields:

- `step_index`
- previous value
- actual value after apply
- pre/post state hash
- frame/sample measurement hash
- root authority and roots at step
- selection reason
- policy/genome id when applicable
- scene id and RNG seed

### Slice 3 - Headless Parameter-Space Slime Trace Runner

Priority: P1/P2, high benefit, medium effort.

This is the first real trace tooling slice after authority and receipts. It should not use mouse automation or live UI.

Desired output artifacts:

- `slime_trace_manifest.json`
- `initial_state.json`
- `final_state.json`
- `mutation_trace.jsonl`
- `root_samples.jsonl`
- `measurement_samples.jsonl`
- `trace_summary.json`

### Slice 4 - RTK/Analyzer Bridge

Priority: P2, medium/high benefit, medium effort.

Analyze serialized parameter-space traces, not only final findings.

Useful outputs:

- parameter utility timeline
- root drift timeline
- conjugate-pair separation timeline
- reactivation and exhaustion markers
- replay status

### Slice 5 - Optional FITS/Flashlight Reuse

Priority: P3, medium benefit, medium effort.

Use after trace artifacts are truthful.

Good reuse:

- FITS as deterministic external scene/observer input corpus.
- Flashlight-style overlay/CSV/OBJ/STL artifact writing for trace visualization.
- Runtime-walk frame context when comparing field trajectories to parameter trajectories.

Do not use FITS/flashlight as the root-authority source.

## Deferred Idea Grooming

| Rank | Idea | Effort | Benefit | Recommendation |
|------|------|--------|---------|----------------|
| 1 | Captured ExplainO roots as analyzer truth | Low/Medium | High | Do first. This is a correctness repair, not research. |
| 2 | Sidecar trace receipt hardening | Low/Medium | High | Do before new slime analysis so later tools do not build on weak history. |
| 3 | ExplainO capability atlas | Low/Medium | Medium/High | Good early tool after root authority. It reduces future archaeology. |
| 4 | Headless parameter-space slime trace runner | Medium | High | Strong next step after receipts. Keep headless/offline first. |
| 5 | Continuity witness around `explaino_seed_curve.h` | Medium | Medium/High | Good diagnostic rail after root authority and atlas. |
| 6 | Seed sweep as interesting-seed hunter | Medium | Medium/High | Promising, but should wait until root-aware metrics are authoritative. |
| 7 | Root-history charts in finding charts | Low/Medium | Medium | Nice bridge once trace or sweep data exists. Not first. |
| 8 | RTK v3 trace-first analyzer | Medium/High | High strategic | Good architecture direction, but needs trace receipts first. |
| 9 | FITS/flashlight corpus and trace visualization reuse | Medium | Medium | Useful later; not the owner for root/slime authority. |
| 10 | Runtime-walk preset pack | Low/Medium | Low/Medium | Fun/product-polish value, but defer behind authority and trace tooling. |
| 11 | Slime GA policy evolution | High | High but risky | Defer. Requires deterministic headless evaluator and strong receipts. |
| 12 | Adaptive viewport presentation for fastest slime runs | Medium/High | Medium/High | Already documented separately; do only after trace/capture authority is clear. |

## External Review Ideas

### 1. Captured ExplainO Roots As Single Analysis Truth

Verdict: accept as first implementation slice.

Why:

- It closes a real authority mismatch.
- It improves current finding analysis immediately.
- It is bounded and testable.

Blocking concerns:

- The analyzer may still need coefficients for basin/Newton calculations. The fix must not delete coefficient use entirely; it must only make root selection authoritative.

### 2. ExplainO Capability Atlas

Verdict: accept and defer behind root-authority repair.

Useful fields:

- fractal type id and label
- whether generated roots exist
- whether custom roots exist
- whether captured runtime roots are expected
- known conjugate-pair semantics
- whether a second polynomial or secondary root family exists
- special branches in `fractal_family_rules.h` / `fractal_derived_fields.cpp`
- known analyzer limitations

Benefit:

- Prevents future archaeology around ExplainO variants.
- Provides a stable matrix for seed sweeps and trace runners.

Show-stopper:

- None, but it should be generated from repo authority rather than hand-maintained prose if it becomes more than a quick report.

### 3. Seed Sweep As Interesting Seed Hunter

Verdict: promising; defer.

Root-aware scores to consider:

- conjugate-pair separation drift
- centroid drift
- symmetry error
- pair identity stability
- root velocity under seed delta
- frame/root disagreement score
- "weird but stable" score
- "jumpy/unstable" score

Bundle output:

- top weird seeds
- top smooth seeds
- top symmetry-break seeds
- top reactivation candidates for sidecar exploration

Show-stoppers:

- Must use captured/runtime-authoritative roots where available.
- Needs stable scoring definitions or it will reward noise.
- Can become expensive quickly if it renders too much; start with small sample/probe data before image-heavy sweeps.

### 4. Root-History Charts

Verdict: accept as later reporting polish.

Likely charts:

- root x/y over seed or slime step
- conjugate-pair separation
- centroid drift
- symmetry error
- root authority/source timeline

Best dependency:

- Wait until root-authority repair plus either seed sweep or trace runner outputs exist.

### 5. Continuity Witness Around `explaino_seed_curve.h`

Verdict: accept as a diagnostic rail after the root-authority repair.

Why:

- It is repo-native.
- It can answer whether a lane is naturally jumpy or has a discontinuity/bug.
- It can become an early warning rail for future ExplainO root work.

Candidate metrics:

- `seed_delta`
- max root displacement
- mean root displacement
- pair assignment stability
- frame/sample delta
- discontinuity flag

Show-stopper:

- Pair assignment is not trivial if roots reorder or merge. Start with conservative matching and report ambiguous pairings instead of inventing certainty.

### 6. ExplainO Runtime-Walk Preset Pack

Verdict: defer as product polish.

Why it is useful:

- It would make runtime-walk/FITS work easier to explore.
- It can provide curated examples for later RTK traces.

Why not now:

- It does not fix root authority.
- It does not harden slime trace receipts.
- It risks becoming visual/toy work before the measurement contract is stable.

## Other Related Deferred Ideas

### FITS/Flashlight Reuse For Trace Work

Verdict: keep as later support layer.

Use for:

- deterministic external scene corpus;
- observer/context inputs;
- trace overlays and companion artifacts;
- comparing parameter-space slime traces with world-field trajectories.

Do not use for:

- deciding root authority;
- replacing captured roots;
- driving the first analyzer repair.

### RTK v3 Trace-First Reorientation

Verdict: good architecture direction; defer until trace receipts are strong enough.

Preferred model:

```text
scene definition
-> headless runtime loads exact state
-> sidecar/slime runs bounded policy walk
-> serialized trace and sample receipts
-> RTK/analyzer consumes trace artifacts
-> charts/reports are derived views
```

This is better than a side repo owning the core truth because the trace depends on repo-private state, sidecar semantics, and runtime-derived roots.

### Slime Policy GA And Vein Stopping

Verdict: already documented; defer.

Reference: `docs/notes/slime_policy_ga_and_vein_stopping_DEFERRED_NOTE.md`.

Dependency:

- Do not start GA before trace receipt hardening and deterministic headless evaluator.

### Adaptive Viewport Presentation For Fast Slime Runs

Verdict: already documented; defer.

Reference: `docs/notes/slime_adaptive_viewport_presentation_DEFERRED_NOTE.md`.

Dependency:

- Needs explicit stale-view reporting and forced latest-state render before capture.

## Contradictions And Show-Stoppers

- Captured root authority and coefficient fallback are not the same thing. Analyzer output must say which source was used.
- Root analysis tools must not silently replace runtime-captured roots with coefficient-derived guesses.
- Root-history charts can lie if root ordering changes. Any charting slice needs explicit pair matching and ambiguity reporting.
- Seed hunting can reward visual noise unless stability and degeneracy penalties are included.
- Runtime-walk field slime and ExplainO sidecar slime share vocabulary but not authority. A future campaign must keep their receipt formats distinct until a deliberate bridge exists.
- FITS/flashlight can help visualize or seed traces, but making them root authority would reproduce the same class of stale-analysis bug.
- GA work is premature until mutation traces include enough immutable context to replay and audit decisions.

## Suggested Next Implementation Order

1. Root analyzer authority repair.
2. Trace receipt hardening.
3. ExplainO capability atlas.
4. Headless parameter-space slime trace runner.
5. Continuity witness.
6. Root-aware seed hunter.
7. Root-history charts.
8. RTK/FITS/flashlight trace visualization bridge.
9. Stopping policy v1-v3.
10. GA policy evaluator.

Stop and replan after item 4 before adding charts, seed hunting, GA, or product UI.
