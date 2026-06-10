# Deferred Note: SDF Pack Packaging And Hybrid Substrate Readiness

Status: deferred SDF follow-up and longer-term substrate planning. This is likely the most relevant note when SDF work resumes, but it is not active today.

## Summary

The recent SDF substrate is materially real: shared SDF field surfaces, authored pack parsing/evaluation, CPU/CUDA evaluation, Color Pipeline SDF consumers, row-local field downsample, `sdf_pack_scene`, and curated built-in packs are now part of the product direction.

The short-term SDF risk is packaging and recovery, not new math:

- built-in `sdf_pack_scene` packs are still reported as repo-dependent in the published runtime path;
- default built-in load failure may leave the normal UI without a recovery selector;
- those are product hardening items before broader SDF catalog growth.

The longer-term hybrid question splits cleanly:

- `IFS + SDF` is plausible after a dedicated IFS/map-set/chaos-game substrate exists.
- `Flame + SDF` is farther because it needs attractor-density accumulation, histogram/normalization, and a render-state contract before an SDF bridge is meaningful.

## Short-Term SDF Packaging Issues

Known review findings to preserve:

1. Built-in `sdf_pack_scene` packs are still repo-dependent in the published runtime path.
   - Reported seam: `ui_app/src/sdf_pack_viewer_ui.cpp` resolves pack JSON through `fractal_ui_repo_root.txt` or process CWD.
   - Reported seam: `ui_app/build_vsdevcmd.cmd` stages repo-root metadata but not the built-in pack JSON files beside the runtime.
   - Risk: a copied published runtime can lose built-in packs even though `sdf_pack_scene` is now a normal viewer lane.

2. Default built-in load failure is not recoverable enough from normal UI.
   - Startup default load failure can leave `have_pack=false`.
   - If the UI returns early after only showing the error, the built-in selector may not render.
   - Risk: the exact packaging failure gives the operator no in-app retry/choose-another-pack path.

These should be considered high-priority SDF hardening when SDF work resumes.

## SDF Hybrid Readiness

SDF half:

- shared field interface exists;
- authored-pack field production exists;
- Color Pipeline and overlay consumers exist;
- normal selectable `sdf_pack_scene` lane exists;
- capture/replay/report authority has been improved.

Missing non-SDF half for IFS:

- map-set/subdivision or chaos-game contract;
- deterministic render-state authority;
- sample/probe semantics;
- bridge from IFS geometry/density output into SDF field interface;
- catalog/family defaults and controls.

Missing non-SDF half for Flame/attractor density:

- accumulation renderer;
- histogram/density normalization;
- exposure/color-mapping authority;
- determinism and progressive accumulation policy;
- GPU memory/performance story;
- bridge from density or derived fields into SDF composition.

## Contradictions And Show-Stoppers

- `sdf_pack_scene` cannot be considered self-contained product if built-in pack files are not staged with the published runtime.
- A normal viewer lane cannot fail into an unrecoverable UI state when its default pack is missing.
- `IFS + SDF` is not "almost done" merely because the SDF side exists. The IFS substrate is separate.
- `Flame + SDF` is farther than IFS because density/histogram rendering is a distinct substrate, not a small SDF operator.
- Mixing SDF-native fields with unsupported non-SDF source rows must stay fail-closed until a real scalar/source-signal producer exists.
- New SDF ops, recursive/apollonian packs, IFS, Flame, and 3D DE should not be bundled into one "SDF expansion" slice.

## Future Slice Shape

Recommended short-term SDF return order:

1. Published-runtime built-in pack staging.
   - Stage built-in pack JSON beside runtime or via explicit manifest.
   - Remove normal runtime reliance on repo-root fallback.
2. Default-pack load failure recovery.
   - The built-in selector and retry path must render even when default load fails.
   - Report a clear fail-closed error.
3. Pack resolution tests.
   - Published runtime without repo checkout still loads built-ins.
   - Missing default pack can be recovered from the UI/report path.
4. Field capability/report consistency.
   - Keep producer kind, dimensions, downsample, backend, and fail reason visible.
5. Only then revisit new SDF ops or pack catalog expansion.

Recommended longer-term hybrid order:

1. IFS substrate contract.
2. IFS to SDF field bridge.
3. First curated IFS+SDF pack/lane.
4. Attractor-density/Flame substrate contract.
5. Flame/density to SDF bridge.
6. Hybrid composition productization.

## Proof Gates For Any Future Implementation

- Published runtime test runs from a copied runtime folder without repo-root access and still finds built-in SDF packs.
- Missing built-in default pack surfaces a recoverable UI/report error.
- Capture Finding and replay preserve selected pack id and controls.
- Field capability report names producer kind, backend, dimensions, downsample, and fail-closed reason.
- Mixed Source rows for `sdf_pack_scene` remain denied unless a real renderer-backed non-SDF signal plane exists.
- IFS/Flame work starts with separate substrate contracts, not with SDF Color Pipeline glue.

