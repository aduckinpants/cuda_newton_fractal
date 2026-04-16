# Flashlight Donor Branch Audit

## Scope

This audit reviews the non-merged donor branches and nearby ancestry checkpoints relevant to recovering `FlashlightProbe` into the current `feature/explaino-joy` line without merging historical debt wholesale.

Reviewed refs:

- `origin/feature/seed-refactor`
- `working/viewer-host-extraction`
- `master`
- `working/salt-fractal-sweep-viewer`

## Branch Inventory

| Branch / ref | Purpose | Unique surfaces missing from current `HEAD` | Classification | Recommended action |
| --- | --- | --- | --- | --- |
| `origin/feature/seed-refactor` | Historical donor line that carried the original flashlight implementation before the current viewer-host expansion | `--flashlight-probe`, deterministic text seed projection, manifold walk, headless probe JSON, lens/SDF probe sampling, bridge request/prefill/status seams, live flashlight UI, trace/export references | Feature-bearing donor with substantial obsolete coupling | `recover` selected seams only |
| `working/viewer-host-extraction` | Workflow/bootstrap divergence during viewer-host extraction | minor workflow/docs/bootstrap differences; no large runtime-only feature surface found during scan | Workflow/doc branch | `reference only` |
| `master` | Ancestry checkpoint for the current line | no flashlight-specific seam absent from current `HEAD` beyond what is already captured by donor branches | Superseded ancestry checkpoint | `ignore` as donor, keep as ancestry reference |
| `working/salt-fractal-sweep-viewer` | Ancestry checkpoint tied to sweep/viewer evolution | sweep/viewer lineage context only; no flashlight donor surface beyond current `HEAD` plus seed-refactor donor | Superseded ancestry checkpoint | `ignore` as donor, keep as ancestry reference |

## Seed-Refactor Subsystem Classification

### `flashlight-core`

Recover now:

- deterministic text-to-seed projection
- headless `--flashlight-probe <path>`
- bounded multi-band manifold walk
- runtime lens mask downsample plus chamfer SDF sampling
- `flashlight_probe.json`
- final proof bundle:
  - `frame.bmp`
  - `lens_sdf.bmp`
  - `state.json`

Current extraction target:

- `ui_app/src/viewer_cli.*` for CLI surface
- `ui_app/src/main.cpp` headless dispatch only
- `ui_app/src/flashlight_probe.*` for the recovered implementation seam
- runtime pytest lane for published-runtime verification

### `flashlight-bridge`

Defer:

- `flashlight_bridge_request.json`
- `flashlight_bridge_prefill.json`
- `flashlight_bridge_status.json`
- seed modulation / pending auto emit behavior

Reason:

- these seams are bridge policy and workflow integration, not required for restoring the core headless probe
- exporter/watcher ownership is still partially external

### `flashlight-export`

Defer pending provenance proof:

- trace overlay
- STL / OBJ / GIF / vector export references

Current conclusion:

- the repo donor line references bridge request/prefill/status files and per-tick frame emission
- the first-class STL writer is still not located in the current repo surface reviewed so far
- treat exporter logic as external or unresolved until a checked-in writer path is proven

### `legacy-ui-schema`

Never merge wholesale:

- old `ui/fractal_binding_surface_v1.ui_schema.json` copies/backups
- historical control layout
- ad hoc schema survival fixes from the donor line

Reason:

- current schema/startup rails are now materially better than the donor branch state
- the original branch was stopped partly because UI/schema coupling was threatening collapse

### `obsolete-runtime`

Never merge wholesale:

- donor `main.cpp`
- donor build assumptions
- committed diagnostics/build outputs
- live flashlight UI panels in their historical monolithic form

Reason:

- current repo has cleaner ownership boundaries
- restoring the donor runtime shape would reintroduce technical debt instead of extracting the feature

## Recovery Matrix

| Branch / ref | Feature / seam | Current equivalent exists? | Donor quality | Hidden debt risk | Decision | Extraction target |
| --- | --- | --- | --- | --- | --- | --- |
| `origin/feature/seed-refactor` | text seed projection | No | Good, self-contained | Low | Recover now | `ui_app/src/flashlight_probe.*` |
| `origin/feature/seed-refactor` | headless flashlight CLI | No | Good after extraction | Low | Recover now | `viewer_cli.*`, `main.cpp`, `flashlight_probe.*` |
| `origin/feature/seed-refactor` | manifold walk | No | Good, deterministic | Low | Recover now | `ui_app/src/flashlight_probe.*` |
| `origin/feature/seed-refactor` | lens/SDF probe sampling | Partial: current repo already has reusable SDF helpers | Good once rebound to current seams | Low | Recover now | `ui_app/src/flashlight_probe.*` |
| `origin/feature/seed-refactor` | probe JSON / proof bundle | Partial: diagnostics bundle exists, flashlight JSON does not | Good once mapped to current diagnostics surface | Low | Recover now | `diagnostics_capture.*` + `flashlight_probe.*` |
| `origin/feature/seed-refactor` | bridge request/prefill/status | No | Medium; intertwined with workflow assumptions | Medium | Later | separate bridge seam if exporter path is revived |
| `origin/feature/seed-refactor` | trace/STL export | No proven current equivalent | Unresolved | High | Later / unresolved | separate exporter module only after writer path is proven |
| `origin/feature/seed-refactor` | old schema/control layout | No, intentionally | Poor fit for modern line | High | Never | none |
| `origin/feature/seed-refactor` | donor `main.cpp` | No, intentionally | Poor fit for modern line | High | Never | none |
| `working/viewer-host-extraction` | workflow/bootstrap ideas | Largely yes | Narrow value | Low | Reference only | docs/workflow only if a concrete gap appears |

## Current Implementation Decision

The first recovery slice in this repo should remain strictly:

- headless flashlight only
- current schema authoritative
- current viewer-host architecture authoritative
- no live flashlight restore
- no bridge restore
- no exporter restore

## Build / Runtime Hardening Findings

The donor recovery work also exposed a reproducibility gap unrelated to flashlight logic itself:

- older scripts assumed a hardcoded Visual Studio 18/2026 Community path
- this machine works through the installed 2022 Build Tools path
- current recovery also depends on a CUDA target set that includes `sm_86`

The recovery slice therefore includes toolchain discovery hardening and keeps `sm_86` in the CUDA compile targets used by the public runtime/test build scripts.
