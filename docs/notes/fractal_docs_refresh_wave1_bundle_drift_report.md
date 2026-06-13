# Fractal Docs Refresh Wave 1 Bundle Drift Report

## Live Repo Authority

- Repo: `C:/code/cuda_newton_fractal_clone`
- Bundle path: `D:/salt-output/explaino_novelty_analysis/20260612_000000_viewer_host_fractal_math_refresh_packet_doc_update_working/fractal_docs_bootstrap_bundle`
- Live branch for this report: `codex/fractal-docs-refresh-wave1-foundation`
- Live `HEAD`: `eebc0e3`
- `master`: fast-forwarded and pushed from `571dba4` to `eebc0e3` before this branch was created.
- Rearward review for `eebc0e3`: `ok`, artifact `artifacts/hooks/viewer_host_rearward_review/eebc0e3418002febf7cc5cf01346039c8fa83481.json`.
- Active Wave 1 contract: `docs/contracts/fractal_docs_refresh_wave1_foundation.contract.json`.

## Bundle Stamp

- Bundle generated: `2026-06-12T18:47:52-05:00`.
- Bundle branch stamp: `codex/explaino-root-sdf-field-lane`.
- Bundle head stamp: `24d9d0e`.
- Bundle catalog claim: 48 `FractalType` ids, `0..47`.
- Bundle Root SDF status claim: `closed_not_merged`, `active_repair`, `volatile_context`.

## Drift Findings

### DRIFT-1 - Root SDF merge status is stale

The bundle correctly treated Root SDF as branch-bound at `24d9d0e`, but live repo authority has advanced.

Current truth:

- `master` is now `eebc0e3`.
- `codex/explaino-root-sdf-field-lane` was fast-forward merged and pushed to `master`.
- `eebc0e3` includes the later Root SDF seed-dynamics repair after the bundle stamp.

Disposition: bundle Root SDF `closed_not_merged` and `active_repair` wording is stale and must not be imported as current truth.

### DRIFT-2 - Root SDF volatile-control caveat is resolved for the reported seed-action gap

Live code shows the repair that the bundle could only warn about:

- `FractalType::explaino_root_sdf = 47`.
- `IsFieldPrimarySdfFractal(...)` includes `explaino_root_sdf`.
- `UsesExplainoRootLayoutAuthority(...)` includes `explaino_root_sdf`.
- `SupportsExplainoSeedControls(...)` delegates to root-layout authority instead of broad `IsExplainoFamily(...)`.
- Tests cover the Root SDF seed-control and animation authority paths.

Disposition: preserve the general caution that Root SDF must stay field-primary, but retire the specific active-repair warning.

### DRIFT-3 - Bundle Wave 1 ordering is usable after live verification

The bundle's Wave 1 order is still valid after live repo checks:

1. N-root/root-field authority descriptor with `legacy_quartic_v1` parity.
2. Minimal `preset_core` authority.
3. Deterministic AA V1.
4. Hardening Pass 1 and pause.

Disposition: accepted as the active Wave 1 implementation order.

### DRIFT-4 - Low-hanging idea passes remain out of scope

The bundle includes later low-hanging idea passes and medium enablers, but the user's active goal stops after Hardening Pass 1.

Disposition: do not import or implement Wave 2+ idea work under this contract.

### DRIFT-5 - Current-state docs should be synchronized selectively

The bundle contains broad current-state docs that overlap with checked-in repo planning surfaces. Full-copy import would risk replacing repo authority with a generated snapshot.

Disposition: use selective truth sync only:

- mark Root SDF v1 and seed-dynamics repair as merged to `master`;
- keep Root SDF follow-ons deferred;
- record this Wave 1 foundation campaign as active;
- leave idea registries and sync-tooling specs as side-folder references until a later docs/tooling slice.

## Accepted Imports For This Wave

- Bundle guardrail language: side-folder is `import_candidate`, not authority.
- Wave 1 order: N-root authority, preset core, deterministic AA, hardening pause.
- N-root semantic locks:
  - preserve `legacy_quartic_v1`;
  - distinguish base authority from effective roots;
  - distinguish root layout from fractal family;
  - distinguish root owner from root consumer.
- Preset scope split:
  - implement `preset_core` first;
  - defer gallery polish.
- AA scope:
  - deterministic spatial AA only;
  - AA off must preserve current path exactly;
  - no temporal/random/adaptive AA in V1.
- Hardening decision vocabulary:
  - `FOUNDATION_READY`;
  - `FOUNDATION_READY_WITH_CAVEATS`;
  - `FOUNDATION_BLOCKED`;
  - `ROLLBACK_REQUIRED`.

## Rejected Or Deferred Imports

- Full generated current-state doc replacement.
- Deterministic sync tooling implementation.
- Low-hanging fractal idea passes.
- Medium engine enabler mini-sprint.
- Root-field consumer idea cards.
- New SDF ops, recursive/apollonian packs, or broader SDF-native family growth.
- Broad Color Pipeline UI replacement.
- Any wording that says Root SDF is still unmerged or under active seed-control repair.

## First Implementation Risk List

1. N-root work can silently turn into a renderer rewrite. The first implementation must add descriptor/parity seams before changing consumers.
2. Existing `poly_coeffs[5]` and fixed-root surfaces are four-root-shaped. The first pass should wrap them as `legacy_quartic_v1`, not replace them globally.
3. Root SDF has base/effective roots. Effective phase-modulated roots must not overwrite source authority.
4. Presets can become broad `state.json` dumps. `preset_core` must stay curated, versioned, and fail-closed.
5. AA can mask formula or Color Pipeline bugs. AA off parity is the required first proof.
6. SDF field-primary paths may need a distinct AA policy. The AA slice must name and test that policy.

## Phase 0 Import Decision

Proceed with the Wave 1 foundation plan using the bundle as planning input, not source authority.

Do not start Wave 2+ idea passes until Hardening Pass 1 records a foundation decision.
