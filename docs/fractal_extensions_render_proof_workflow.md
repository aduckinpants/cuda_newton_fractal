# Fractal Extensions Render-Proof Workflow

This note documents the manual workflow we are using right now to prove that a fractal-extensions run produced genuinely different rendered frames rather than one reused hero image plus different JSON sidecars.

The immediate goal is auditability, not product polish. Later this can be formalized into a dedicated tool or reporter script.

## Why This Exists

The fractal-extensions composite seam has two different proof modes:

- scene-frame proof: capture one or more real runtime frames from `state.json` scenes, archive them, and attach `generic.sample` sidecars plus analysis output
- sidecar proof: keep one base frame fixed and vary only the sidecar math requests and responses

Those are both valid, but they prove different things.

If we need to prove “these renders are truly different images,” a single-base-state manifest is not enough. We need a manifest whose scenes point at different `state.json` captures, then we need a cheap, deterministic audit over the resulting `frame.png` files.

## Current Seams

The current shipped pieces are:

- manifest loader and composite runner: `tools/reality_toolkit/fractal_explorer/fractal_extensions.py`
- CLI wrapper: `tools/reality_toolkit/scripts/run_fractal_extensions_composite.py`
- finding archive seam: `tools/reality_toolkit/fractal_explorer/finding_capture.py`
- gallery manifest with five distinct scene states: `docs/manifests/gallery_manifest.json`
- single-base-state sidecar manifests:
  - `docs/manifests/maxwell_rh_probe_manifest.json`
  - `docs/manifests/transcendental_lambda_probe_manifest.json`

The composite runner does three important things for each scene:

1. load the published runtime with `--load-state-json <scene state>` and run `--capture-diagnostic`
2. archive the resulting `frame.bmp` + `state.json` into a stable finding directory and convert the frame to `frame.png`
3. run the configured `generic.sample` sidecars and optional analysis output against that archived scene

That means the rendered image comes from the scene `state.json`, not from the sidecar requests.

## Manifest Selection Rule

Use `docs/manifests/gallery_manifest.json` when the claim is:

- “show me several different rendered fractal scenes”
- “prove the composite run did not reuse one image for every result”
- “give me a gallery audit surface for visual comparison”

Do not use `docs/manifests/maxwell_rh_probe_manifest.json` or `docs/manifests/transcendental_lambda_probe_manifest.json` for that claim. Those are valid sidecar/math probes, but each is anchored to one base `state.json`, so they prove callable math coverage, not multiple distinct rendered frames.

## Manual Proof Workflow

### 1. Rerun the gallery manifest to a fresh explicit output directory

Use a new timestamped output directory every time. Do not overwrite a shared proof folder whose contents might be cited later.

Example command used in the 2026-04-14 audit:

```powershell
py -3.14 tools/viewer_host_run_logged_command.py \
  --label render-proof-gallery \
  --log artifacts/render_proof_gallery.log \
  -- py -3.14 tools/reality_toolkit/scripts/run_fractal_extensions_composite.py \
       --manifest-json docs/manifests/gallery_manifest.json \
       --out-dir D:/salt-fractal/cuda_newton_fractal_clone/findings/render_proof_gallery_2026-04-14_1350utc \
       --overwrite
```

Expected result:

- one top-level `fractal_extensions_summary.json`
- one directory per scene
- inside each scene: `frame.png`, `state.json`, `finding.json`, `finding.md`, `field-notes.md`, `extension_sidecars.json`, `sidecars/...`, and `analysis/...`

### 2. Confirm the run really captured multiple scenes

Read the top-level `fractal_extensions_summary.json` and check:

- `scene_count` matches the manifest
- each `scene_summaries[*].captured` is `true`
- each scene has its own `finding_dir`

This is the structural proof that the runner did not silently collapse to one scene.

### 3. Compute a cheap duplicate-image audit over the archived `frame.png` files

Current manual method: hash each scene `frame.png`, collect a few summary metrics from `analysis/analysis.json`, and write a local proof artifact.

Example command used in the 2026-04-14 audit:

```powershell
$root = 'D:\salt-fractal\cuda_newton_fractal_clone\findings\render_proof_gallery_2026-04-14_1350utc'
$scenes = Get-ChildItem -Path $root -Directory
$report = foreach ($scene in $scenes) {
  $frame = Join-Path $scene.FullName 'frame.png'
  $analysisPath = Join-Path $scene.FullName 'analysis\analysis.json'
  $analysis = Get-Content -Path $analysisPath -Raw | ConvertFrom-Json
  [pscustomobject]@{
    scene = $scene.Name
    fractal_type = $analysis.state.fractal_type
    frame_sha256 = (Get-FileHash -Path $frame -Algorithm SHA256).Hash
    frame_bytes = (Get-Item $frame).Length
    mean_luma = $analysis.frame_metrics.mean_luma
    edge_energy = $analysis.frame_metrics.edge_energy
    unique_colors_4bit = $analysis.frame_metrics.unique_colors_4bit
    top_cluster_fraction = if ($analysis.color_clusters.clusters.Count -gt 0) {
      $analysis.color_clusters.clusters[0].fraction
    } else {
      $null
    }
  }
}
$dups = @(
  $report |
    Group-Object -Property frame_sha256 |
    Where-Object { $_.Count -gt 1 } |
    ForEach-Object {
      [pscustomobject]@{
        frame_sha256 = $_.Name
        count = $_.Count
        scenes = @($_.Group | ForEach-Object { $_.scene })
      }
    }
)
$result = [pscustomobject]@{
  out_dir = $root
  scene_count = $report.Count
  duplicate_hash_groups = $dups
  scenes = @($report)
}
$result | ConvertTo-Json -Depth 6 | Set-Content -Path 'artifacts/render_proof_gallery_distinctness.json'
```

Acceptance rule:

- `duplicate_hash_groups` must be empty

The metrics are secondary evidence. The hash audit is the fast deterministic check.

### 4. Inspect representative frames visually

Even with unique hashes, do a small spot-check because visually broken captures can still be unique.

Current expectation:

- check at least three scenes with clearly different geometry or palette behavior
- include at least one scene with strong structure and one scene with degenerate or near-flat structure if present

Examples from the 2026-04-14 audit:

- `joy_scallop` showed scalloped basin boundaries
- `phoenix_bifurcation` showed a four-basin split
- `dual_explaino` showed the dual-seed butterfly-like composite
- `rational_escape_globe` collapsed to a mostly flat ochre globe frame, which is still useful because it is obviously not the same image as the structured scenes

### 5. Record the boundary explicitly

When closing the audit, write down whether the manifest proved:

- multiple distinct rendered frames
- one rendered frame plus multiple sidecar math probes

Do not let those two claims blur together in later summaries.

## Output Contract To Rely On Today

The minimum durable proof bundle is:

- run log: `artifacts/<label>.log`
- top-level run summary: `<out-dir>/fractal_extensions_summary.json`
- per-scene archive:
  - `<scene>/frame.png`
  - `<scene>/state.json`
  - `<scene>/finding.json`
  - `<scene>/extension_sidecars.json`
  - `<scene>/analysis/analysis.json`
- duplicate-image audit artifact: `artifacts/render_proof_gallery_distinctness.json`

If any of those are missing, the proof is incomplete.

## What This Workflow Does Not Prove

This workflow does not prove:

- that every sidecar response is mathematically correct
- that a visually flat frame is a bug rather than a legitimate capture
- that two distinct frames are meaningfully far apart in image space beyond exact-file inequality
- that the gallery manifest is the best mathematical selection of scenes

It only proves that the archived render outputs were freshly captured per scene and were not exact duplicate images.

## Future Tool Boundary

If this becomes a first-class tool later, the likely shape is:

- input: manifest path, explicit output directory, optional strict mode
- execution: rerun the composite capture, compute frame hashes, inspect per-scene output completeness
- output: one machine-readable proof report plus one short human-readable markdown summary

Likely responsibilities for a dedicated tool:

1. enforce the manifest-selection rule, or at least warn when all scenes share one `state_json`
2. emit the hash audit automatically instead of relying on an ad hoc PowerShell snippet
3. summarize per-scene frame metrics and duplicate status in one report
4. fail non-zero if required proof artifacts are missing or duplicate hashes are found

Likely non-goals for v1:

- perceptual image similarity scoring
- automated aesthetic ranking
- replacing the existing composite runner

The composite runner should stay the scene-capture surface. The future tool should sit on top as an audit/report layer.