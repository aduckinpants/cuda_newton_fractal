# Color Pipeline Composition And Preset UX Review

## Current Phase

Complete - Color Pipeline composition/preset review is documented, synced, validated, checkpointed, and later truth-synced after the effective Source summary shipped.

## Phase Checklist

- [x] Phase 1 - create this checked-in plan/contract and open the slice
- [x] Phase 2 - record current repo truth for Color Pipeline stack authority, SDF phase signals, and function-library limits
- [x] Phase 3 - define the smallest implementation-ready follow-up slices for preset workflow, source-stack authority summaries, and function-library expansion
- [x] Phase 4 - sync SDF and Color Pipeline roadmap surfaces with the selected follow-up order
- [x] Phase 5 - validate plan/contract sync, hostile audit, diff hygiene, checkpoint, receipts, rearward review, push, and clean tree

## Explicit User Asks

- [done] Add the SDF normal-angle / branch-cut interpretation to the plan without treating visually useful diagnostic output as a kernel bug.
- [done] Review the Color Pipeline function-library shape because the current function set and composition flow are useful but still limited.
- [done] Consider whether SDF fields should feed other Color Pipeline segments or become building blocks for more advanced composition math.
- [done] Keep SDF-native lanes, authored SDF UI, and broad Color Pipeline redesign separate until the smaller UX authority gaps are planned clearly.
- [done] Continue real work after the planning truth is captured, instead of leaving the repo in discussion-only state. The selected next code seam, source-stack authority summary/report clarity, later shipped at `ck:6b8e3f61`.

## Scope

In scope:

- Document the product interpretation of `sdf_normal_angle` as full-field diagnostic phase data whose wrap and medial-axis seams are expected unless a boundary-masked beauty mode is selected.
- Document that top-level `color_signal` is not a truthful effective-source summary when a non-empty `color_source_stack` is active.
- Classify near-term Color Pipeline UX follow-ups by size and dependency order.
- Decide where SDF fields may reasonably act as reusable Color Pipeline operands: Source signals now, Source masks/weights next, Shape modulation later, and SDF-native fractal lanes only after separate field/renderer proof.
- Keep the current branch's shipped SDF source rows, capture/replay authority, phase metadata, and fractal-switch preservation intact.

Out of scope:

- Implementing a full preset manager in this slice.
- Implementing a Factorio-style composition UI in this slice.
- Adding SDF-native selectable fractal lanes.
- Adding authored SDF pack live viewport integration.
- Adding arbitrary shader/function execution.
- Replacing the renderer or Color Pipeline architecture.
- Physical mouse automation.

## Current Repo Truth To Preserve

- `sdf_normal_angle` is already classified as `phase` data; it is not an ordinary scalar SDF source.
- Full-field normal angle can produce visually coherent wrap seams and nearest-feature/medial-axis partitions. That is useful diagnostic output, not automatically a fractal-kernel defect.
- Boundary-masked normal-angle is the likely beauty-mode follow-up because it can preserve local normal color near the field boundary while suppressing broad off-boundary planes.
- A non-empty `color_source_stack` is effective render authority even if legacy top-level fields such as `color_signal` still contain a simpler summary.
- The current function library is segment-shaped: Source, Shape, Palette, and Grading rows have different contracts. SDF values should not be smuggled into every segment as raw scalar hacks.

## Function Library Shape Review

Current segment contracts:

- Source rows produce the upstream signal. Signals are now typed as scalar, phase, or categorical.
- Shape rows transform a normalized signal before palette lookup.
- Palette rows map a signal to RGB.
- Grading rows modify final RGB.
- Source-stack composition is currently weighted signal blending. It is useful, but it is still crude compared to a user-authored rule/composition surface.

Current useful function families:

- scalar Source: smooth escape, banded signal, escape magnitude, root proximity, SDF signed distance, SDF boundary band, SDF curvature
- phase Source: orbit phase, orbit stripe, SDF normal angle
- categorical Source: root index, SDF inside/outside
- Shape: identity, offset/scale, repeat, mirror repeat, posterize, bias/gain, smooth window
- Palette: heatmap, phase wheel, banded heatmap, Explaino cmap, root palettes
- Grading: contrast/tone/phase/band/neutral/glow/balance-void style finish rows

Current gaps:

- Effective Source-stack summary/report clarity is shipped. Product-facing preset/projection semantics are shipped, and the current composition UI cleanup removes the remaining visible draft/live-bridge wording without changing the internal row-stack implementation.
- Signal kind metadata exists, but phase/categorical kinds do not yet strongly guide row affordances, warning text, or phase-safe defaults.
- There is no first-class mask/gate segment. Users can blend rows, but cannot easily say "apply this source only near an SDF boundary" or "use this inside/outside field as a rule".
- Function picker/library presentation is flat enough that growing the library will become confusing.
- Visible draft/live wording cleanup is shipped; internal draft/live names remain as implementation seams and persisted-state compatibility.
- Preset behavior now has an explicit authored-intent versus live-projection model so supported fractal switches preserve work while unsupported rows project safely.

## SDF Operand Review

SDF fields can be useful beyond plain Source rows, but each use needs a segment contract:

- Shipped now: SDF fields as Source signals.
- Low-risk next: SDF fields as masks/gates for Source rows, especially `sdf_boundary_band` and `sdf_inside_outside`.
- Useful next beauty control: boundary-masked `sdf_normal_angle`, preserving full-field normal angle as a diagnostic mode.
- Later: SDF fields as Shape modulators, Palette selectors, or Grading intensity masks.
- Deferred: authored analytic SDF packs as field producers and SDF-native selectable fractal lanes.

The important boundary is that SDF should become a typed operand in the composition model, not an untyped scalar escape hatch that makes every segment harder to reason about.

## Proposed Follow-Up Order

1. Source-stack authority summary - shipped.
   - Size/reward: small / high trust.
   - Diagnostic state exports now identify flat-signal versus source-stack authority with source row signal kinds and blend weights.
   - Keep legacy top-level fields for compatibility, but do not treat them as the only product-facing summary.

2. Preset workflow truth - shipped.
   - Size/reward: medium / high user value.
   - Separate saved author intent from target-fractal projection.
   - Preserve compatible pipelines across fractal switches.
   - Treat unsupported rows as explicit projections, not silent resets.

3. Composition workflow UI cleanup - shipped for visible implementation wording.
   - Size/reward: medium / high user value.
   - Replace confusing draft-state labels and row affordances with a clearer authoring model before adding more functions.
   - Use the existing Source/Shape/Palette/Grading stack model as the authority; do not add a second composition system.
   - Later UI layout inspired by schedule/ordered-rule editors remains deferred: ordered rows, enabled state, function picker, row params, and per-row status.

4. Boundary-masked phase source.
   - Size/reward: small/medium / medium-high visual value.
   - Keep current full-field `sdf_normal_angle` as an intentional diagnostic source.
   - Add or plan a beauty-oriented boundary-masked normal-angle source/mode that suppresses broad off-boundary planes.

5. SDF as reusable operands.
   - Size/reward: medium/high / high strategic value.
   - Current: SDF fields provide Source signals.
   - Next likely: SDF boundary/inside masks can become blend weights or gates for Source rows.
   - Later: SDF fields can modulate Shape rows, Palette selection, or Grading intensity, but only with explicit signal-kind metadata.
   - Deferred: authored analytic SDF packs and SDF-native fractal lanes.

6. Function-library expansion taxonomy.
   - Size/reward: ongoing / high long-term value.
   - Add functions only after their segment contract is clear: scalar Source, phase Source, categorical Source, Shape transform, Palette mapping, Grading finish, mask/gate, or field producer.
   - Require no-mouse runtime proof that each new function materially changes pixels through the normal viewer path.

## Proof Ledger

- Start authority: current branch `codex/color-pipeline-sdf-source-rows` at `020ba56` is clean and rearward review returned `status=ok`.
- Prior shipped SDF evidence: live SDF Source rows, SDF Source row customization, capture/replay authority, phase-signal metadata, viewport overlay, and Color Pipeline fractal-switch preservation are closed on this branch.
- Investigation finding: the referenced `152625_625__halley` capture is Halley with a four-row SDF source stack, not a Multibrot/signed-distance-only kernel repro.
- Investigation finding: `sdf_normal_angle` contributes to that stack but is not the full-weight dominant row in that capture.
- Investigation finding: the real product risk is effective-source/state-summary clarity plus phase-safe/beauty-mode follow-up, not deletion of full-field normal-angle diagnostics.
- Current follow-up selection at review close was source-stack authority summary/report clarity; this shipped later at `ck:6b8e3f61`. UI-Salt backend authority for catalog descriptors, compatibility, companion suggestions, and recipe expansion shipped through `d908c54`. Product-facing preset workflow truth is shipped, and the current `color_pipeline_composition_ui_cleanup` slice ships visible implementation-wording cleanup. Remaining later composition follow-ups are boundary-masked phase source, SDF masks/gates, and function-library taxonomy/layout work.
- Roadmap sync: `spec_intake/_STATUS.md`, `DEFERRED_THREADS.md`, `KNOWN_ISSUES.md`, `docs/notes/sdf_field_pack_near_term_TODO.md`, and the parked preset pit-of-success plan now point at this review and its selected follow-up order. A later truth-sync marks source-stack summary as shipped and moves SDF performance work to field quality/downsample policy.

## Hostile Audit

- Status: complete
- Required posture: assume this plan accidentally over-promises implementation, hides a real state-authority bug as design discussion, or turns SDF work into a broad Color Pipeline rewrite until the roadmap and scope prove otherwise.

## Audit Passes

- [done] Pass 1 - re-read current SDF roadmap/status docs and ensure shipped work is not reopened.
- [done] Pass 2 - re-read Color Pipeline catalog/function surfaces and separate shipped functions from proposed library expansion.
- [done] Pass 3 - re-read capture/state authority surfaces and identify the smallest testable source-summary follow-up.
- [done] Pass 4 - ensure SDF-native lanes and authored SDF UI remain deferred.

## Audit Findings

- [done] Real roadmap gap found: the active SDF TODO mentioned Color Pipeline composition/preset UX review but did not spell out the source-stack authority summary, full-field diagnostic versus boundary-masked normal-angle split, or SDF-as-mask/gate follow-up.
- [done] Real older-plan drift found: the preset pit-of-success note remained parked behind an old harness gate and did not identify this active composition review as the current refinement surface.
- [done] Clean re-read: this review does not implement or claim a full preset manager, Factorio-style UI, authored SDF live viewport integration, or SDF-native lanes.
- [done] Later truth-sync finding: source-stack authority summary/report clarity is shipped and should not remain listed as the next pending implementation seam.
- [done] Later truth-sync finding: UI-Salt backend authority is shipped through recipe expansion; preset workflow truth is the active product-facing gap, not another backend metadata switch.

## Action Hostile Review

- Action ID: color-pipeline-composition-preset-ux-review-open
- Suspected failure mode: treating `sdf_normal_angle` seams as a kernel bug, or conversely ignoring the real product issue that phase/stack authority is not exposed clearly enough.
- Correct owner/action: Color Pipeline planning surfaces must distinguish diagnostic phase views, beauty/masked phase views, effective Source-stack summaries, and future function-library expansion.
- Proof surface: checked-in plan/contract, roadmap sync, phased-plan sync, hostile-audit validation, contract validation, diff hygiene, rearward review, and clean tree.
- Blocked action: full preset manager, Factorio-style UI implementation, SDF-native lanes, authored SDF live viewport integration, arbitrary shader execution, renderer replacement, or physical mouse automation.
