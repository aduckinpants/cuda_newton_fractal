# Explaino Magnet Research And Low-Hanging Fruit Note

## Current Phase

Complete - docs-only research note recorded for later; no product code is changed in this slice.

## Explicit User Asks

- [x] Document the `explaino_magnet` feasibility research for later rather than implementing it now.
- [x] Identify lower-effort nearby work that is less risky than biting off `explaino_magnet` immediately.
- [x] Keep this doc-only; do not start a new fractal implementation under this slice.

## Phase Checklist

- [x] Phase 0 - confirm clean `master` and inspect Magnet / Explaino owner seams.
- [x] Phase 1 - write the durable research note and low-hanging-fruit ranking.
- [x] Phase 2 - validate contract, plan sync, hostile audit, checkpoint, receipts, push, clean tree, and stale-plan grep.

## Proof Ledger

- Repo state before this doc slice was clean on `master` / `origin/master` at `04359a9`.
- Current Magnet authority: `FractalType::magnet` is a real escape-time family with `magnet_seed_real`, `magnet_seed_imag`, `magnet_relaxation`, and `magnet_bailout` controls in schema/binding/runtime surfaces.
- Current Explaino authority: `kExplainoAxisRegistry` owns seven canonical axes across the explicit Explaino projection lanes and `explaino_all`.
- Research conclusion: `explaino_magnet` is feasible, but not low-hanging because the safe design needs an escape-time Explaino-axis modulation seam rather than pushing Magnet through legacy basin/root `IsExplainoFamily(...)` assumptions.
- Durable note written at `docs/notes/explaino_magnet_variant_research_and_low_hanging_fruit.md`.

## Hostile Audit

- Status: complete
- Required posture: assume the note is too vague, accidentally green-lights a risky implementation, or confuses low-hanging polish with new architecture unless it explicitly separates those categories.

## Audit Passes

- [x] Pass 1 - re-read current Magnet and Explaino seams before ranking effort; no product-code mutation was made.
- [x] Pass 2 - reviewed the note for scope creep and separated low-risk Magnet polish from medium-risk `explaino_magnet` architecture.
- [x] Pass 3 - clean re-read found no additional stale implementation claim, no fake closure claim, and no instruction to mutate code under this docs-only slice.

## Audit Findings

- [x] Real finding: the first instinct to classify `explaino_magnet` as low hanging would be misleading; the current code makes it a medium slice because legacy Explaino carries basin/root/sidecar assumptions that Magnet should not inherit.
- [x] Clean re-read: the final note keeps `explaino_magnet` deferred and ranks safer adjacent work first.

## Action Hostile Review

- Action ID: explaino-magnet-research-note-1
- Suspected Failure Mode: The documentation could be mistaken for implementation approval, or it could understate the architectural risk of mixing escape-time Magnet with legacy basin-style Explaino machinery.
- Correct Owner/Action: Save only a research note with clear feasibility, risk, and low-hanging-fruit ranking; no product code, schema, renderer, or test harness changes.
- Proof Surface: contract validation, phased-plan sync, hostile-audit validation, committed note, push, clean tree.
- Blocked Action: adding `FractalType::explaino_magnet`, changing Magnet runtime math, changing `kExplainoAxisRegistry`, or reopening Color Pipeline work.
