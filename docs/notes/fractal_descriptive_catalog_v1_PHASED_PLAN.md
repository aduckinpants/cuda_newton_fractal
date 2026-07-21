# Fractal Descriptive Catalog V1 Phased Plan

## Explicit User Asks

- Add an engine-owned deterministic descriptive catalog export for every live fractal selector.
- Review mathematical prose only for `explaino_all` and `explaino_magnet_root_well`; every other selector remains explicitly unavailable.
- Keep the live catalog authoritative for selector identity, catalog order, formula-growth identity, and existing capability/runtime flags.
- Ground every reviewed sentence in current engine evidence through a tracked build/test ledger.
- Treat the historical side-folder catalog only as editorial seed evidence; production and the published runtime must not depend on it.
- Merge and publish the engine feature cleanly before the state tool consumes it.
- Do not change formulas, rendering, state, parameter applicability, Color Pipeline behavior, selector names, or flag taxonomy.

## Starting Authority

- Repository: `C:\\code\\cuda_newton_fractal_clone`
- Branch: `codex/fractal-descriptive-catalog-v1`
- Exact starting commit: `1ec313244d529cdee588ad12aecff37e4390a903`
- Starting base: exact merged `origin/master`
- Rearward review: `artifacts/hooks/viewer_host_rearward_review/1ec313244d529cdee588ad12aecff37e4390a903.json` reports `ok`.
- Historical editorial seed: `D:\\salt-output\\explaino_novelty_analysis\\20260612_000000_viewer_host_fractal_math_refresh_packet_doc_update_working\\fractal_catalog_current.json`
- Historical seed SHA-256 observed during preflight: `cf5446b7d899f9a467186fc5d514e80b2d6de72c3992dca3830d7f7b91169d1d`.

## Current Phase

Phase 4 complete - engine implementation is validation-complete and awaiting the separately authorized feature-merge boundary. Phase 5 remains blocked until exact merged-master publication.

## Phase Checklist

- [x] Phase 0 - validated and locked this engine-native plan/contract at slice token `ck:35474d4a`.
- [x] Phase 1 - captured focused RED tests for missing descriptive-catalog implementation and CLI ownership, plus runtime REDs for Windows stdout/file byte mismatch.
- [x] Phase 2 - implemented the metadata-only compiled overlay, deterministic serializer, and headless CLI/file surfaces.
- [x] Phase 3 - completed the sentence evidence ledger and reproducible historical editorial audit for the two reviewed selectors.
- [x] Phase 4 - ran focused tests, full native/public catalog rails, deterministic cross-mode proof, hostile audit, checkpoint preparation, and merge-ready closure.
- [ ] Phase 5 - after separately authorized merge, fast-forward merged master, publish the runtime, and prove cross-mode deterministic export bytes.

## Accepted Public Contract

The published viewer adds:

```text
fractal_ui.exe --describe-fractal-catalog
fractal_ui.exe --describe-fractal-catalog-json <path>
```

Stdout mode writes only deterministic UTF-8 JSON to stdout. File mode writes the identical JSON bytes to the requested path. Diagnostics use stderr and failures return nonzero.

The JSON root is:

```json
{
  "schema_version": 1,
  "entries": []
}
```

Every `kFractalCatalog` row produces one entry, in catalog order, with fixed field order:

1. `selector_id`
2. `display_name`
3. `category`
4. `family`
5. `formula_growth_surface`
6. `capability_flags`
7. `runtime_flags`
8. `description_status`
9. `description`

Flags are deterministic strings in declared enum-bit order and retain their current live meanings. No flag, selector, category, family, or growth-surface taxonomy repair is authorized.

A reviewed `description` contains fixed-order fields:

1. `math_summary`
2. `recurrence_or_field_model`
3. `state_order`
4. `termination_or_classification`
5. `interpretation_notes`
6. `source_refs`

Only `explaino_all` and `explaino_magnet_root_well` are reviewed in V1. Other rows emit `description_status: "unavailable"` and `description: null`.

The exported bytes contain no timestamps, local paths, branch or commit data, build-machine identity, or nondeterministically ordered containers. Runtime/commit provenance belongs to external receipts.

## File-Output Semantics

The closest established atomic report convention is `ui_app/src/viewer_ui_automation_report.cpp`: a same-directory temporary file, close/flush, replacement, rename, and cleanup. This slice adopts that convention without introducing a new transaction framework:

- The parent directory must already exist; the command does not create it.
- The temporary path is the exact target path plus `.tmp`, in the same directory.
- An existing stale temporary file is replaced before writing.
- A successful closed temporary file replaces an existing target.
- Replacement first uses the established remove-plus-rename path, with the same copy-overwrite fallback used by the analogous engine convention.
- Any open, write, close, remove, rename, or fallback-copy failure returns nonzero, reports the exact stage to stderr, and attempts to remove only the owned `.tmp` file.
- Failure must not report success or leave the owned temporary file behind.
- Tests cover missing parents, existing targets, stale temporary files, replacement failure, cleanup, diagnostics, and exit status.

If implementation proves this analogous convention cannot satisfy deterministic identical bytes or safe cleanup without a new transaction design, stop for plan revision.

## Runtime And Evidence Design

- `kFractalCatalog` remains the only live selector/flag/catalog-order authority.
- A compiled descriptive overlay stores reviewed fields for the two approved selectors only.
- The overlay may retain internal sentence-to-claim identifiers for tests, but those identifiers never enter public JSON.
- `docs/fractal_descriptive_catalog_evidence.v1.json` is build/test authority only and is not loaded by the executable.
- Focused tests join compiled overlay sentences to accepted ledger records and reject missing, ambiguous, rejected, or superseded authority.
- Public `source_refs` are concise repository-relative references derived only from accepted evidence mappings.
- The published executable has no dependency on the ledger or historical side folder.

## Editorial Review Boundary

Current engine code and tests override historical prose.

For `explaino_all`, verify and describe only source-grounded facts about:

- the canonical public selector and neutral-axis collapse to baseline ExplainO;
- the degree-four real-coefficient Newton basis used by the active composed branch;
- the active splice, ripple, vortex, tension, balance/void, symmetry-tension, field-curvature, damping, and optional Phoenix-memory terms;
- first-order state when the Phoenix memory term is neutral and second-order state when it is active;
- residual convergence, finite-state handling, bounded-radius rescaling, and nearest-root classification when residual convergence is not reached.

For `explaino_magnet_root_well`, verify and describe only source-grounded facts about:

- the shared Magnet Type-I rational recurrence with pixel coordinate as `c` and configured Magnet seed as initial `z`;
- the bounded relaxation applied to the rational update;
- residual-to-`z=1` convergence, Magnet bailout, nonfinite termination, and iteration exhaustion;
- the separation between the base Magnet recurrence and the root-field distance/nearest-root signals used by coloring.

Do not claim that serialized roots establish visible symmetry, that root proximity establishes basins, or that any active/nonzero control visibly contributes without frame evidence.

## Historical Seed Audit

Track a reproducible audit under `docs/notes/` containing the review source identity, repository-independent source path notation, file SHA-256, audit date, observed live and historical counts, aliases, per-entry mapping/disposition, unmatched/duplicate/ambiguous entries, and live entries without seeds.

The starting hypothesis is 51 live selectors, 48 historical entries, the editorial alias `lambda_map -> lambda`, and three newer root-field selectors without historical seeds. The audit reports observed deviations instead of forcing those values.

## Mutation Surface

Allowed:

- reviewed descriptive metadata overlay;
- deterministic serialization and mechanical existing-flag string conversion;
- headless CLI parsing/dispatch;
- focused native/runtime tests and build wiring;
- evidence ledger, historical audit, plan/contract, handoff, and validation artifacts;
- publication through established engine tooling.

Forbidden without a new user-approved engine plan:

- formulas, samplers, rendering, UI, or runtime iteration behavior;
- state serialization/deserialization/loading;
- Color Pipeline behavior;
- parameter applicability or control-surface changes;
- selector or flag additions, renaming, reinterpretation, or taxonomy repair;
- unrelated compatibility repair.

## TDD And Rollback

- Phase 1 must capture focused RED failures before implementation.
- Each implementation step closes the smallest corresponding RED test first.
- Dirty experiments remain on this branch, within contract scope, and are either promoted into coherent tested code or reverted through an approved mutation wrapper.
- Any need to touch a forbidden surface, invent a new file transaction framework, parse docs at runtime, or depend on the historical folder is a design blocker and requires plan revision.
- Rollback is the branch checkpoint before product mutation plus deletion of newly added metadata-only files through approved repository mutation tooling; never reset or overwrite unrelated work.

## Validation And Publication Gates

Run every required command in `docs/contracts/fractal_descriptive_catalog_v1.contract.json`, plus any stronger current repository-mandated rail discovered during execution. The minimum closure set includes:

- focused native descriptive-catalog and CLI tests;
- focused published-runtime CLI tests;
- full native helper suite;
- public catalog profile, including runtime build/publish and catalog smoke;
- repeated stdout and file export byte comparisons from the published runtime;
- contract validation, phased-plan sync, code-quality baseline, diff check, hostile audit, receipts, checkpoint, and rearward review.

Feature merge is not inferred from this plan. Stop at a clean pushed merge-ready checkpoint unless current engine protocol grants autonomous merge or the user separately authorizes that feature PR merge. Publication proof occurs only from exact merged master.

## Proof Ledger

| Item | Evidence |
| --- | --- |
| Exact starting base | `1ec313244d529cdee588ad12aecff37e4390a903` equals merged `origin/master`. |
| Rearward authority | `artifacts/hooks/viewer_host_rearward_review/1ec313244d529cdee588ad12aecff37e4390a903.json` reports `ok`. |
| Live catalog authority | Focused native proof exports exactly 51 unique entries in `kFractalCatalog` order, preserves `lambda`, and rejects `lambda_map`. |
| Historical source hash | `docs/notes/fractal_descriptive_catalog_historical_audit_v1.json` records and reproduces `cf5446b7d899f9a467186fc5d514e80b2d6de72c3992dca3830d7f7b91169d1d`, 48 historical entries, 47 exact matches, one corrected alias, and three live selectors without seeds. |
| File convention | `ui_app/src/viewer_ui_automation_report.cpp` supplies the closest established same-directory temp/replace convention. |
| Slice lock | `ck:35474d4a`; the active contract is `fractal_descriptive_catalog_v1`. |
| RED receipts | `artifacts/logs/fractal_descriptive_catalog_red_native.log`, `artifacts/logs/fractal_descriptive_catalog_red_viewer_cli.log`, and `artifacts/validation/fractal_descriptive_catalog_runtime_pytest_red_crossmode.json` preserve the missing implementation/CLI and CRLF cross-mode failures. |
| Focused native proof | `artifacts/validation/fractal_descriptive_catalog_focused_native.json` passes schema/order/identity, fixed flag meanings and unknown-bit rejection, reviewed/unavailable coverage, sentence-ledger joins, source-ref derivation, and file semantics. |
| CLI proof | `artifacts/validation/fractal_descriptive_catalog_viewer_cli.json` passes 227/227 parser and conflict cases. |
| Full native proof | `artifacts/validation/fractal_descriptive_catalog_full_native.json` reports success and `All helper tests passed` in `1335.646s`. |
| Runtime publish proof | `artifacts/validation/fractal_descriptive_catalog_runtime_publish.json` reports success and stages the active runtime at `D:/salt-fractal/cuda_newton_fractal_clone/runtime/fractal_ui.exe`. This is branch validation only, not the Phase 5 merged-master handoff. |
| Published-runtime catalog proof | `artifacts/validation/fractal_descriptive_catalog_runtime_pytest.json` reports 4 passed with non-ambiguous counts; `artifacts/validation/fractal_descriptive_catalog_public_catalog_smoke.json` reports 43/43 strict catalog rows. |
| Deterministic export proof | Two stdout exports and the file export are each 28,487 bytes with SHA-256 `21184b8af87d3fe2cc7e652cba5a125b54876a265ae2fe99595dc7f4a46262c0`. |
| Pre-merge validation runtime | Active executable SHA-256 after the final branch publish is `c36418ed6d67a773f4dbedb919cdeab6662904decfd64c89743734aee8526303`; this is not the merged-master runtime identity required by Phase 5. |

## Hostile Audit

- Status: complete

Audit questions:

- Did the implementation remain metadata-only and preserve every existing catalog meaning?
- Can identical compiled metadata produce byte-different output across invocations or modes?
- Can any reviewed sentence reach public JSON without current accepted evidence?
- Can rejected or superseded evidence authorize prose?
- Does runtime behavior depend on the ledger, docs, historical folder, local paths, or machine identity?
- Can file-mode failure corrupt an existing target or leave an owned temporary file?
- Did any historical wording outrank current source or tests?
- Did this slice claim merge/publication/closure without the required authority and receipts?

## Audit Passes

- [x] Pass 1 - implementation and contract-scope audit confirmed the product mutation is limited to compiled descriptive metadata, deterministic serialization, headless CLI/build wiring, tests, and tracked evidence; no formula, sampler, render, state, Color Pipeline, parameter, selector, or flag-taxonomy behavior changed.
- [x] Pass 2 - determinism, evidence-linkage, CLI, and file-failure audit found and repaired Windows stdout newline translation, missing link wiring, weak source-ref derivation coverage, and silent future taxonomy/flag omission risk.
- [x] Pass 3 - clean re-read of the repaired state plus focused, full-native, runtime-publish, runtime-pytest, strict-catalog, and byte-hash proof found no additional real defect.

## Audit Findings

- [x] The inherited contract lock could not authorize creation of its successor contract; a minimal contract-only bootstrap correction was made before product mutation, the successor contract was immediately locked, and the inherited contract was restored with no lingering diff.
- [x] Runtime build attempt 1 exposed Windows `min` macro contamination and a missing production link object; `NOMINMAX` and the explicit catalog object link repaired both seams.
- [x] The first runtime cross-mode proof exposed CRLF translation on stdout while file mode emitted LF; binary stdout mode repaired byte identity, and the 4-test runtime lane now passes.
- [x] The first broad native run exposed a missing catalog source in `test_headless_modes`; the established target wiring was repaired and the complete helper suite subsequently passed.
- [x] The initial engine-native contract named a nonexistent `Invoke-Build` command and duplicated pytest quiet mode; the locked contract now uses the exact checked-in catalog-profile dependent script and a single helper-owned quiet flag.
- [x] The first evidence test only checked public source-ref shape; it now proves every public reference derives from an accepted, used, current claim and every accepted reviewed claim maps exactly once.
- [x] The initial taxonomy tests could permit a future unknown enum string or silently omitted flag bit; focused coverage now fails closed on either condition.

## Decisive Stop Conditions

Stop before product mutation if contract lock, exact-head authority, or RED-test setup fails. Stop during implementation if the feature requires a forbidden runtime/state/rendering change or a new transaction framework. Stop at merge-ready if feature merge authority is absent. Stop before state-tool integration until exact merged-master publication, executable hash, runtime identity, deterministic stdout/file export, catalog hash, clean-tree proof, and engine closure receipts all exist.
